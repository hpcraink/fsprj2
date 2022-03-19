#include <argp.h>

#include <stdbool.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include "cli.h"
#include "common/error.h"
#include "common/utils.h"
#include "trace/syscalls.h"
#include "../common/stracer.h"


/* -- Functions -- */
static error_t parse_cli_opt(int key, char *arg, struct argp_state *state) {
    cli_args *arguments = state->input;

    char* const arg_str_start = (arg && '=' == arg[0]) ? (arg +1) : (arg);    /* CLI args may be passed w/ equals sign, e.g., `-key=value`, which messes up parsing */
    switch (key) {
    /* Fildes of socket 2 be used for tracing */
        case STRACER_CLI_OPTION_SOCKFD:
        {
            long tmp_parsed_sockfd;
            if (-1 == str_to_long(arg_str_start, &tmp_parsed_sockfd) || tmp_parsed_sockfd < 0) {
                LOG_ERROR_AND_EXIT("Couldn't parse supplied socket fildes");
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

            if (strchr(arg_str_start, ',')) {
                char* arg_copy = DIE_WHEN_ERRNO_VPTR( strdup(arg_str_start) );

                char* pch = NULL;
                while ((pch = strtok((!pch) ? (arg_copy) : (NULL), ","))) {
                    const long scall_nr = syscalls_get_nr(pch);
                    if (-1 == scall_nr) {
                        argp_usage(state);
                    }
                    arguments->syscall_subset_to_be_traced[scall_nr] = true;
                }

                free(arg_copy);
            } else {
                const long scall_nr = syscalls_get_nr(arg_str_start);
                if (-1 == scall_nr) {
                    argp_usage(state);
                }
                arguments->syscall_subset_to_be_traced[scall_nr] = true;
            }
        }
            break;

    /* Warn when function call wasn't traced by libiotrace */
        case STRACER_CLI_OPTION_WARN:
            arguments->warn_not_traced_syscalls = true;
            break;


        case ARGP_KEY_ARG:
          /* Too many arguments */
          break;

        case ARGP_KEY_END:
          /* Not enough arguments */
          if (-1 == arguments->uxd_reg_sock_fd) {
            argp_usage(state);
          }
          break;

        default:
            return ARGP_ERR_UNKNOWN;
    }

    return 0;
}


void parse_cli_args(int argc, char** argv,
                    cli_args* parsed_cli_args_ptr) {
    static const struct argp_option cli_options[] = {
        {"sockfd", STRACER_CLI_OPTION_SOCKFD,  "fildes",      0, "File descriptor of opened socket which shall be used for tracing",       1},
        {"trace",  STRACER_CLI_OPTION_SSUBSET, "syscall_set", 0, "Trace only the specified (as comma-list seperated) set of system calls", 2},
        {"warn",   STRACER_CLI_OPTION_WARN,    NULL,          0, "Warn when function call wasn't traced by libiotrace",                    3},
        {0}
    };

  /* Defaults */
    parsed_cli_args_ptr->uxd_reg_sock_fd = -1;
    parsed_cli_args_ptr->trace_only_syscall_subset = false;
    parsed_cli_args_ptr->warn_not_traced_syscalls = false;

    static const struct argp argp = {
        cli_options, parse_cli_opt,
        "program",
        "Syscall-Tracer used in conjunction with libiotrace",
        .children = NULL, .help_filter = NULL, .argp_domain = NULL
    };

    argp_parse(&argp, argc, argv, 0, 0, parsed_cli_args_ptr);
}

void print_parsed_cli_args(cli_args* parsed_cli_args_ptr) {
    static const char* const TRUE_STR = "true";
    static const char* const FALSE_STR = "false";

    puts("Parsed CLI args:");

    printf("\t`uxd_reg_sock_fd`=%d\n", parsed_cli_args_ptr->uxd_reg_sock_fd);
    printf("\t`trace_only_syscall_subset`=%s", parsed_cli_args_ptr->trace_only_syscall_subset ? (TRUE_STR) : (FALSE_STR));
    if (parsed_cli_args_ptr->trace_only_syscall_subset) {
        printf(" { ");
        for (int i = 0; i < SYSCALLS_ARR_SIZE; i++) {
            const syscall_entry* const scall = &syscalls[i];
            if (!parsed_cli_args_ptr->syscall_subset_to_be_traced[i] || !scall->name) {
                continue;
            }
            printf("%s(%d), ", scall->name, i);
        }
        printf("}");
    }
    printf("\n\t`warn_not_traced_syscalls`=%s\n", parsed_cli_args_ptr->warn_not_traced_syscalls ? (TRUE_STR) : (FALSE_STR));
}
