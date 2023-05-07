/**
 * stracer (aka. 'System call Tracer')
 *   - Lifecycle:
 *     - Gets launched by libiotrace as separate process
 *     - Tracee's request tracing by sending a tracing request (via an Unix Domain Socket)
 *        - NOTE: We could also set the ptrace option `PTRACE_O_TRACECLONE`, which would automatically attach
 *                newly `clone`(2)d child processes (therefore, making this registration superfluous)
 *                HOWEVER this would require us to trace the MPI ORTED process
 *     - Tracer performs selected 'Task's on tracee's (e.g., checking whether an ioevent has been traced by libiotrace)
 *     - Tracer terminates as soon as the last tracee has exited and timeout has expired
 *
 *   - Supported 'Tasks': The tracer may perform the following Tasks on tracee's during tracing:
 *     - warn: Check whether the provided syscalls have been traced by libiotrace, if not, print a warning
 *     - lsep: "Libiotrace Syscall-Event Passing" -- Interface (i.e., IPC mechanism) for passing traced syscalls to libiotrace
 *
 *   - Known ISSUEs:
 *     - Attaching issues when functions like `pthread_create`(3) are statically linked (for more details, see comments in libiotrace/entrypoint.c)
 *     - lsep offers only "narrow" -- per thread "view" of traced syscalls (for more details, see comments in libiotrace/tasks/lsep/stracing_lsep.c)
 *
 *   - TODOs:
 *     - REUSE CODE B/W libiotrace & stracer & tests:
 *        - gettime    (utils.c ?? --> gettime + str_to_long; -> how2: COMPILE_OPTIONS -> include + add source)
 *        - error.h
 *        - scerb_ipc_utils
 *     - (OPTIONAL: Pass job-name via CLI args from libiotrace (logfile could then also contain the jobname))
 *     - aarch64 support (returns currently wrong syscall-nr, hence disabled)
 */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "ipc/uxd_socket.h"
#include "trace/tracing.h"
#include "trace/ptrace_utils.h"
#include "tasks/task_hooks.h"
#include "cli.h"

//#define DEV_DEBUG_ENABLE_LOGS
#include "common/error.h"



/* -- Consts -- */
#ifdef USE_LOGFILE
#  define LOGFILE_NAME "libiotrace_stracer.log"
#endif /* USE_LOGFILE */



int main(int argc, char** argv) {
/* 0. Setup stracer process */
    DIE_WHEN_ERRNO( close(STDIN_FILENO) );

#ifdef USE_LOGFILE
/* 0.1. Set up a logfile (since we can't log to console; must happen asap to ensure ALL logs get 'routed' to correct location) */
    FILE* stdout_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(LOGFILE_NAME, "a+", stdout) );
    FILE* stderr_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(LOGFILE_NAME, "a+", stderr) );
    if (0 != setvbuf(stdout_logfile, NULL, _IONBF, 0) ||
        0 != setvbuf(stderr_logfile, NULL, _IONBF, 0)) {
        LOG_ERROR_AND_DIE("Couldn't set buffering options for logfile");
    }
#endif /* USE_LOGFILE */

/* 0.2. Parse CLI args */
    cli_args_t parsed_cli_args;
    parse_cli_args(argc, argv, &parsed_cli_args);
#ifdef DEV_DEBUG_ENABLE_LOGS
    print_parsed_cli_args(&parsed_cli_args);
#endif
    const int uxd_reg_sock_fd = parsed_cli_args.uxd_reg_sock_fd;

/* 0.3. Check whether fildes is valid */
    if (fcntl(uxd_reg_sock_fd, F_GETFL) < 0 && EBADF == errno) {
        LOG_ERROR_AND_DIE("Invalid uxd socket fd");
    }


/* 0.4. Daemonize tracer, i.e., fork grandchild (which will be the actual tracer) */
    if (DIE_WHEN_ERRNO( fork() )) {     /* Child -> not used */
#ifdef USE_LOGFILE
        fclose(stdout_logfile);
        fclose(stderr_logfile);
#endif /* USE_LOGFILE */
        close(uxd_reg_sock_fd);
        pause();
        _exit(0);
    }
    kill(getppid(), SIGKILL);   /* Grandchild = Tracer */

/* 0.5. Leave tracee's process group (otherwise we also get signals which are sent to that group (e.g., Console -> Ctrl-C -> SIGINT)) */
    DIE_WHEN_ERRNO( setpgid(0, 0) );


/* 1. Init tracing tasks */
    tasks_on_stracer_init(&parsed_cli_args);


/* --------------------- --------------------- --------------------- --------------------- --------------------- */
/* 2. Start tracing .. */
#ifdef DEV_DEBUG_ENABLE_LOGS
    const pid_t tracer_pid = getpid();
#endif /* DEV_DEBUG_ENABLE_LOGS */
    DEV_DEBUG_PRINT_MSG("Ready for tracing requests (running under pid=%d) ..", tracer_pid);

    for (unsigned int tracee_count = 0; ; ) {

    /* 2.1. Check for new IPC requests */
        for (;;) {
            int tracee_conn_fd;
            uxd_sock_ipc_msg_t ipc_request;
            const int status = uxd_ipc_tracer_recv_new_request(uxd_reg_sock_fd, &tracee_conn_fd,
                                                               &ipc_request);
            if (-1 == status)      { break; }
            else if (-2 == status) { goto cleanup; }

            switch (ipc_request.msg_type) {
                case PARENT_PROBE_TRACER_RUNNING:
                    DEV_DEBUG_PRINT_MSG(">>> IPC requests: Got probed; I'm still running (pid=%d)..", tracer_pid);
                    break; /* APPLIES TO SWITCH-CASE !! */

                case PARENT_REQUEST_TRACING:
                    DEV_DEBUG_PRINT_MSG(">>> IPC requests: Received tracing request from tid=%d", ipc_request.msg_payload.tracee_tid);
                    const pid_t new_tracee_tid = tracing_attach_tracee(ipc_request.msg_payload.tracee_tid);
                    if (-1 != new_tracee_tid) {
                        tracee_count++;
                        DEV_DEBUG_PRINT_MSG(">>> IPC requests/Tracing: +++ Attached tracee + set 1st bp for %d +++", new_tracee_tid);
                        tasks_on_event_attached_tracee(new_tracee_tid, &ipc_request);
                        uxd_ipc_tracer_send_tracing_ack(tracee_conn_fd);
                    }
                    break; /* PERTAINS TO THE SWITCH-CASE !! */


                /* NOTE: We do NOT implement a detach IPC request (e.g., during libiotrace's cleanup) since
                 * it's more efficient for the tracee to 'just terminate' */
                case TRACER_REQUEST_ACCEPTED:
                default:
                    LOG_WARN("Invalid ipc-request");
            }

        cleanup:
            DIE_WHEN_ERRNO( close(tracee_conn_fd) );
        }


    /* 2.2. Check whether we can exit */
        if (0 == tracee_count) {
            DEV_DEBUG_PRINT_MSG(">>> Exit condition: TIMEOUT -- No tracees, will terminate in %d ms", EXIT_TIMEOUT_IN_MS);
            if (uxd_ipc_tracer_block_until_request_or_timeout(uxd_reg_sock_fd, EXIT_TIMEOUT_IN_MS)) { continue; }
            DEV_DEBUG_PRINT_MSG(">>> Exit condition: TIMEOUT -- Has lapsed, terminating ..");
            break;
        }


    /* 2.3. Trace */
        for (pid_t trapped_tracee_sttid = -1; ; ) {     /* `sttid`, aka., "status tid" = tid which contains status information in sign bit (has stopped = positive, has terminated = negative) */

        /* 2.3.1. Check whether there's a trapped tracee */
            if (! (trapped_tracee_sttid = tracing_set_next_bp_and_check_trap(trapped_tracee_sttid)) ) { break; }

        /* 2.3.2. Check status */
            /*   -> Tracee terminated */
            if (0 > trapped_tracee_sttid) {
                DEV_DEBUG_PRINT_MSG(">>> Tracing: +++ [%d] terminated +++", -(trapped_tracee_sttid));
                tracee_count--;
                tasks_on_event_tracee_exit( -(trapped_tracee_sttid) );
                break;

            /*   -> Tracee stopped (i.e., hit breakpoint) */
            } else {
                struct user_regs_struct_full regs;
                if (-1 == ptrace_get_regs_content(trapped_tracee_sttid, &regs)) {
                    DEV_DEBUG_PRINT_MSG("Couldn't read register contents -- process got probably `SIGKILL`ed");
                    trapped_tracee_sttid = -1;
                    continue;
                }

                const long syscall_nr = USER_REGS_STRUCT_SC_NO(regs);
                if (NO_SYSCALL == syscall_nr ||                                       /* "Trap" was, e.g., a signal */
                    (parsed_cli_args.trace_only_syscall_subset &&
                     !(parsed_cli_args.syscall_subset_to_be_traced[syscall_nr]))) {   /* Current "trapped" syscall shall not be traced */
                    continue;
                }

                if (USER_REGS_STRUCT_SC_HAS_RTNED(regs)) {   /* SYSCALL-EXIT */
                    DEV_DEBUG_PRINT_MSG(">>> Tracing: [%d] performed syscall=%ld", trapped_tracee_sttid, syscall_nr);
                    tasks_on_event_syscall(trapped_tracee_sttid, &regs);
                }
            }
        }
    }
/* --------------------- --------------------- --------------------- --------------------- --------------------- */


/* 3. Cleanup */
    uxd_ipc_tracer_sock_fin(uxd_reg_sock_fd, UXD_SOCKET_FILEPATH);

#ifdef USE_LOGFILE
    fclose(stdout_logfile);
    fclose(stderr_logfile);
#endif /* USE_LOGFILE */

    tasks_on_stracer_fin();

    return 0;
}
