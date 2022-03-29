/**
 * stracer -- System call Tracer
 *   - Lifecycle:
 *     - Gets launched by libiotrace as separate process
 *     - Tracees request to be traced by sending a tracing request (via a Unix Domain Socket)
 *     - Tracer performs selected 'Task's on tracees (e.g., checking whether syscall has been traced)
 *     - Tracer terminates itself as soon as last tracee has exited and timeout has expired
 *
 *   - Supported 'Tasks': The tracer may perform the following tasks on tracees during tracing
 *     - Warn: Check whether the provided syscalls have been traced by libiotrace, if not, print a warning
 *     - fnres: ...
 *
 *   - Things to keep in mind:
 *     - stracer may NOT include any functionality provided by libiotrace (since it's a independent process)
 */
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <signal.h>

#include "cli.h"
#include "ipc/uxd_socket.h"
#include "trace/tracing.h"
#include "trace/ptrace_utils.h"
#include "trace/syscalls.h"

#include "common/error.h"
#define DEV_DEBUG_ENABLE_LOGS
#include "common/debug.h"

/* -- Consts -- */
#ifdef USE_LOGFILE
#  define LOGFILE_NAME "libiotrace_stracer.log"
#endif /* USE_LOGFILE */



int main(int argc, char** argv) {
/* (0) Init */
    DIE_WHEN_ERRNO( close(STDIN_FILENO) );

#ifdef USE_LOGFILE
/* (0.1) Setup a logfile (since we can't log to console; must happen asap to ensure ALL logs get 'routed' to correct location) */
    FILE* stdout_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(LOGFILE_NAME, "a+", stdout) );
    FILE* stderr_logfile = (FILE*)DIE_WHEN_ERRNO_VPTR( freopen(LOGFILE_NAME, "a+", stderr) );
    if (0 != setvbuf(stdout_logfile, NULL, _IONBF, 0) ||
        0 != setvbuf(stderr_logfile, NULL, _IONBF, 0)) {
        LOG_ERROR_AND_EXIT("Couldn't set buffering options for logfile");
    }
#endif /* USE_LOGFILE */

/* (0.2) Parse CLI args */
    cli_args_t parsed_cli_args;
    parse_cli_args(argc, argv, &parsed_cli_args);
#ifdef DEV_DEBUG_ENABLE_LOGS
    print_parsed_cli_args(&parsed_cli_args);
#endif
    const int uxd_reg_sock_fd = parsed_cli_args.uxd_reg_sock_fd;

/* (0.3) Check whether fildes is valid  (TODO: check whether socket) */
    if (fcntl(uxd_reg_sock_fd, F_GETFL) < 0 && EBADF == errno) {
        LOG_ERROR_AND_EXIT("Invalid socket fildes");
    }


/* (0.4) Daemonize tracer, i.e., fork grandchild (which will be the actual tracer) */
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


/* (2) Setup */
//    unwind_init();


/* (3) Start tracing .. */
    const pid_t tracer_pid = getpid();
    DEV_DEBUG_PRINT_MSG("Ready for tracing requests (running under pid=%d) ..", tracer_pid);

    for (int tracee_count = 0; ; ) {
                                #include <time.h>
                                nanosleep((const struct timespec[]){{3, 250000000L}}, NULL);            // TESTING

        pid_t new_tracee_tid = -1;

    /* (3.0) Check for new ipc requests */
        DEV_DEBUG_PRINT_MSG(">> IPC requests: Checking");
        for (uxd_sock_ipc_requests_t ipc_request; ; ) {
            const int status = uxd_ipc_receive_new_request(uxd_reg_sock_fd,&ipc_request, NULL);
            if (-2 == status) { continue; }
            else if (-1 == status) { break; }

            switch (ipc_request.request_type) {
                case PROBE_TRACER_RUNNING:
                    DEV_DEBUG_PRINT_MSG(">>> IPC requests: I'm still running (pid=%d)..", tracer_pid);
                    continue;

                case TRACEE_REQUEST_TRACING:
                    new_tracee_tid = ipc_request.payload.tracee_tid;
                    DEV_DEBUG_PRINT_MSG(">>> IPC requests: Received tracing request from tid=%d", new_tracee_tid);
                    tracing_attach_tracee(new_tracee_tid);
                    tracee_count++;
                    DEV_DEBUG_PRINT_MSG(">>> IPC requests: +++ Attached tracee w/ tid=%d +++", new_tracee_tid);
                    break;  /* We must exit loop here to set breakpoint for just attached tracee .. */

                default:
                    LOG_WARN("Invalid ipc-request");
            }
        }


    /* (3.1) Check whether we can exit */
        DEV_DEBUG_PRINT_MSG(">> Exit condition: Checking tracee count");
        if (0 == tracee_count) {
            DEV_DEBUG_PRINT_MSG(">>> Exit condition -- TIMEOUT: No tracees, will terminate in %d ms", EXIT_TIMEOUT_IN_MS);
            if (uxd_ipc_block_until_request_or_timeout(uxd_reg_sock_fd, EXIT_TIMEOUT_IN_MS)) { continue; }
            DEV_DEBUG_PRINT_MSG(">>> Exit condition -- TIMEOUT: Has lapsed, terminating ...");
            break;
        }


    /* (3.2) Trace */
        DEV_DEBUG_PRINT_MSG(">> Tracing: Setting bp for attached tracees + Checking for new trap");
        for (pid_t trapped_tracee_sttid = new_tracee_tid; ; ) {     /* `sttid`, aka., "status tid" = tid which contains status information in sign bit (has stopped = positive, has terminated = negative) */

        /* 3.2.1. Check whether there's a trapped tracee */
            DEV_DEBUG_PRINT_MSG(">>> Tracing: Setting next bp for %d", trapped_tracee_sttid);
            if (0 == (trapped_tracee_sttid = tracing_set_bp_and_check_trap(trapped_tracee_sttid)) ) {
                DEV_DEBUG_PRINT_MSG(">>> Tracing: No pending trapped tracees");
                break;
            }


        /* 3.2.2. Check status */
            /*   -> Tracee terminated */
            if (0 > trapped_tracee_sttid) {
                DEV_DEBUG_PRINT_MSG(">>> Tracing: +++ [%d] terminated +++\n", -(trapped_tracee_sttid));
                tracee_count--;
                break;

            /*   -> Tracee stopped (i.e., hit breakpoint) */
            } else {
                DEV_DEBUG_PRINT_MSG(">>> Tracing: %d hit a breakpoint", trapped_tracee_sttid);
                struct user_regs_struct_full regs;
                ptrace_get_regs_content(trapped_tracee_sttid, &regs);

                const long syscall_nr = USER_REGS_STRUCT_SC_NO(regs);
                if (NO_SYSCALL == syscall_nr ||                                       /* "Trap" was, e.g., a signal */
                    (parsed_cli_args.trace_only_syscall_subset &&
                     !(parsed_cli_args.syscall_subset_to_be_traced[syscall_nr]))) {   /* Current "trapped" syscall shall not be traced */
                    continue;
                }

                /* >> SYSCALL-EXIT << */
                if (USER_REGS_STRUCT_SC_HAS_RTNED(regs)) {
                    const char* scall_name = syscalls_get_name(syscall_nr);

                    fprintf(stderr, "%s(", (scall_name) ? (scall_name) : ("UNKNOWN"));
                    syscalls_print_args(trapped_tracee_sttid, &regs);
                    fprintf(stderr, ")");

                    const long syscall_rtn_val = USER_REGS_STRUCT_SC_RTNVAL(regs);
                    fprintf(stderr, " = %ld\n", syscall_rtn_val);

// TODO: WARN + FNRES
//                    unwind_print_backtrace_of_proc(trapped_tracee_sttid);
                }
            }
        }

    }


/* (3) Cleanup */
    uxd_ipc_sock_fin(uxd_reg_sock_fd, UXD_SOCKET_FILEPATH);
#ifdef USE_LOGFILE
    fclose(stdout_logfile);
    fclose(stderr_logfile);
#endif /* USE_LOGFILE */
//    unwind_fin();

    return 0;
}
