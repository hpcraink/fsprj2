#include <argp.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

#include "cli.h"
#include "common/utils.h"
#include "trace/syscalls.h"
#include "../common/stracer_consts.h"

#include <errno.h>
//#define DEV_DEBUG_ENABLE_LOGS
#include "common/error.h"


/* -- Functions -- */
static error_t parse_cli_opt(int key, char *always_use_arg_and_not_me, struct argp_state *state) {
    cli_args_t *arguments = state->input;

    char* const arg = (always_use_arg_and_not_me && '=' == always_use_arg_and_not_me[0]) ?
            (always_use_arg_and_not_me + 1) :
            (always_use_arg_and_not_me);    /* CLI args may be passed w/ equals sign, e.g., `-key=value`, which messes up parsing */
    switch (key) {
    /* Fildes of socket 2 be used for tracing */
        case STRACER_CLI_OPTION_SOCKFD:
        {
            long tmp_parsed_sockfd;
            if (-1 == str_to_long(arg, &tmp_parsed_sockfd) || tmp_parsed_sockfd < 0) {
                DEV_DEBUG_PRINT_MSG("CLI -- option -%c: Couldn't parse socket fd: \"%s\"",
                                    STRACER_CLI_OPTION_SOCKFD, arg);
                argp_usage(state);
            }
            arguments->uxd_reg_sock_fd = (int)tmp_parsed_sockfd;
        }
            break;

    /* Trace only subset of syscalls */
        case STRACER_CLI_OPTION_SSUBSET:
        {
            arguments->trace_only_syscall_subset = true;
            memset(arguments->syscall_subset_to_be_traced, 0,
                   SYSCALLS_ARR_SIZE * sizeof(*(arguments->syscall_subset_to_be_traced)));

            if (strchr(arg, ',')) {
                char* arg_copy = DIE_WHEN_ERRNO_VPTR( strdup(arg) );

                char* pch = NULL;
                while ((pch = strtok((!pch) ? (arg_copy) : (NULL), ","))) {
                    const long scall_nr = syscalls_get_nr(pch);
                    if (-1 == scall_nr) {
                        DEV_DEBUG_PRINT_MSG("CLI -- option -%c: Invalid syscall name \"%s\"",
                                            STRACER_CLI_OPTION_SSUBSET, arg);
                        argp_usage(state);
                    }
                    arguments->syscall_subset_to_be_traced[scall_nr] = true;
                }

                free(arg_copy);
            } else {
                const long scall_nr = syscalls_get_nr(arg);
                if (-1 == scall_nr) {
                    DEV_DEBUG_PRINT_MSG("CLI -- option -%c: Invalid syscall name \"%s\"",
                                        STRACER_CLI_OPTION_SSUBSET, arg);
                    argp_usage(state);
                }
                arguments->syscall_subset_to_be_traced[scall_nr] = true;
            }
        }
            break;

    /* Linkage information  (expected format: "s:path/to/exec") */
        case STRACER_CLI_OPTION_LIBIOTRACE_LINKAGE:
            if (
                    strlen(arg) < 3 ||
                    (STRACER_CLI_LIBIOTRACE_LINKAGE_STATIC != arg[0] && STRACER_CLI_LIBIOTRACE_LINKAGE_SHARED != arg[0]) ||
                    ':' != arg[1]
                ) {
                DEV_DEBUG_PRINT_MSG("CLI -- option -%c: Invalid arg format for linkage information: \"%s\"",
                                    STRACER_CLI_OPTION_LIBIOTRACE_LINKAGE, arg);
                argp_usage(state);
            }
            arguments->unwind_static_linkage = STRACER_CLI_LIBIOTRACE_LINKAGE_STATIC == arg[0];
            arguments->unwind_module_name = arg + 2;
            break;

    /* TASK: Warn when function call wasn't traced by libiotrace */
        case STRACER_CLI_OPTION_TASK_WARN:
            arguments->task_warn_not_traced_ioevents = true;
            break;

    /* TASK: Write syscall event in shared buffer */
        case STRACER_CLI_OPTION_TASK_LSEP:
            arguments->task_lsep = true;
            break;


        case ARGP_KEY_ARG:
          /* Too many arguments */
          break;

        case ARGP_KEY_END:
          /* Validate ALWAYS required args (sockfd  +  at least 1 selected task must be selected) */
            if ( -1 == arguments->uxd_reg_sock_fd ||
                 (!arguments->task_warn_not_traced_ioevents && !arguments->task_lsep) ) {
                DEV_DEBUG_PRINT_MSG("CLI validation: UXD fd + at least 1 task may be selected");
                argp_usage(state);
            }

        /* Validate arg(s) which are required depending on selected task(s) */
            if (arguments->task_warn_not_traced_ioevents && !arguments->unwind_module_name) {
                DEV_DEBUG_PRINT_MSG("CLI validation: The warning task requires linkage information (for unwinding)");
                argp_usage(state);
            }
          break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


void parse_cli_args(int argc, char** argv,
                    cli_args_t* parsed_cli_args_ptr) {
    static const struct argp_option cli_options[] = {
        { NULL, STRACER_CLI_OPTION_SOCKFD,             "fd",           0, "fd of IPC UXD socket",                                                                1 },
        { NULL, STRACER_CLI_OPTION_SSUBSET,            "syscall_set",  0, "To be traced, comma-list separated, syscall subset",                                  2 },
        { NULL, STRACER_CLI_OPTION_LIBIOTRACE_LINKAGE, "linkage_info", 0, "Linkage information (required for unwinding)",                                        3 },
        { NULL, STRACER_CLI_OPTION_TASK_WARN, NULL,                    0, "TASK warn (alert when ioevent wasn't traced by libiotrace)",                          4 },
        { NULL, STRACER_CLI_OPTION_TASK_LSEP, NULL,                    0, "TASK lsep (write scevents into shared buffer, which then can be read by libiotrace)", 4 },
        {0}
    };

  /* Defaults */
    parsed_cli_args_ptr->uxd_reg_sock_fd = -1;
    parsed_cli_args_ptr->trace_only_syscall_subset = false;
    parsed_cli_args_ptr->task_warn_not_traced_ioevents = false;
    parsed_cli_args_ptr->task_lsep = false;
    parsed_cli_args_ptr->unwind_static_linkage = false;
    parsed_cli_args_ptr->unwind_module_name = NULL;

    static const struct argp argp = {
        cli_options, parse_cli_opt,
        "program",
        "Syscall-Tracer used in conjunction with libiotrace",
        .children = NULL, .help_filter = NULL, .argp_domain = NULL
    };

    argp_parse(&argp, argc, argv, 0, 0, parsed_cli_args_ptr);
}

void print_parsed_cli_args(cli_args_t* parsed_cli_args_ptr) {
    puts("Parsed CLI args:");
    printf("\t`uxd_reg_sock_fd`=%d\n", parsed_cli_args_ptr->uxd_reg_sock_fd);
    printf("\t`trace_only_syscall_subset`=%s", parsed_cli_args_ptr->trace_only_syscall_subset ? ("true") : ("false"));
    if (parsed_cli_args_ptr->trace_only_syscall_subset) {
        printf(" { ");
        for (int i = 0; i < SYSCALLS_ARR_SIZE; i++) {
            const syscall_entry_t* const scall = &syscalls[i];
            if (!parsed_cli_args_ptr->syscall_subset_to_be_traced[i] || !scall->name) {
                continue;
            }
            printf("%s(%d), ", scall->name, i);
        }
        printf("}");
    }
    printf("\n\t`task_warn_not_traced_ioevent`=%s\n", parsed_cli_args_ptr->task_warn_not_traced_ioevents ? ("true") : ("false"));
    printf("\t`task_lsep`=%s\n", parsed_cli_args_ptr->task_lsep ? ("true") : ("false"));
    printf("\t`unwind_static_linkage`=%s\n", parsed_cli_args_ptr->unwind_static_linkage ? ("true") : ("false"));
    printf("\t`unwind_module_name`=%s\n", __extension__( parsed_cli_args_ptr->unwind_module_name ? : ("N/A") ));
}
