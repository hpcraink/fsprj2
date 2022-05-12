/* For libdw usage examples / internals, see
 *   - https://github.com/strace/strace/blob/master/src/unwind-libdw.c
 *   - https://github.com/ganboing/elfutils/tree/master/libdwfl
 * Prerequisites: libunwind-dev, libdw-dev & libiberty-dev
 * TODO: Performance optimizations (see e.g., strace's `unwind-libdw.c`)
 */

#include <elfutils/libdwfl.h>
#define UNW_REMOTE_ONLY
#include <libunwind-ptrace.h>

#include <stdio.h>
#include <stdlib.h>

#include "unwind.h"

#include <assert.h>
#include "../common/error.h"
//#define DEV_DEBUG_ENABLE_LOGS
#include "../common/debug.h"


/* -- Macros / Globals  -- */
#define LIBIOTRACE_PRECLUDED_PTHREAD_FCT "pthread_create_start_routine"

static unw_addr_space_t g_unw_as;


/* -- Function prototypes -- */
static Dwfl* init_ldw_for_proc(pid_t tid);


/* -- Functions -- */
bool unwind_is_inited(void) {
    return !! g_unw_as;
}

void unwind_init(void) {
    /* ELUCIDATION:
     *   `unw_create_addr_space`(3): Create a new remote unwind address-space; args:
     *      - `ap` pointer (= set of callback routines to access information required to unwind a chain of stackframes) +
     *      - specified byteorder (`0` = default byte-order of unwind target)
     */
    if (! (g_unw_as = unw_create_addr_space(&_UPT_accessors, 0)) ) {
        LOG_ERROR_AND_EXIT("libunwind -- failed to create address space for stack unwinding");
    }

    /* ELUCIDATION:
     *   `unw_set_caching_policy`(3): Sets the caching policy of address space, may be either ...
     *     - `UNW_CACHE_NONE`, `UNW_CACHE_GLOBAL`, `UNW_CACHE_PER_THREAD`
     *     WARNING: Caching requires appropriate calls to unw_flush_cache() to ensure cache validity
     */
    // unw_set_caching_policy(_unw_as, UNW_CACHE_GLOBAL);

    DEV_DEBUG_PRINT_MSG(">>> Unwind: Got init");
}

void unwind_fin(void) {
    unw_destroy_addr_space(g_unw_as);        // ?? TODO: Necessary ??
    g_unw_as = NULL;

    DEV_DEBUG_PRINT_MSG(">>> Unwind: Got fin");
}


bool unwind_ioevent_was_traced(pid_t tid,
                               char* stacktrace_module_name, char* stacktrace_fct_name) {
    assert(g_unw_as && "Unwind context may be inited prior usage." );

    DEV_DEBUG_PRINT_MSG(">>> Unwind: Unwinding stack for %d, searching for \"%s:%s*\"",
                        tid, stacktrace_module_name, __extension__( stacktrace_fct_name ? : "*" ));


/* 0. Init  */
  /* 0.1. libunwind */
    unw_context_t *unw_ctx = _UPT_create(tid);
    unw_cursor_t unw_cursor;
    if (0 > unw_init_remote(&unw_cursor, g_unw_as, unw_ctx)) {
        LOG_ERROR_AND_EXIT("libunwind -- failed to init context");
    }

  /* 0.2. libdw */
    Dwfl* dwfl = init_ldw_for_proc(tid);


/* 1. Search ... */
    bool found_stack_entry = false;
    do {
    /* 1.1. Get IP-address */
        unw_word_t ip = 0;
        if (0 > unw_get_reg(&unw_cursor, UNW_REG_IP, &ip)) {
            LOG_ERROR_AND_EXIT("libunwind -- failed to walk the stack of process %d", tid);
        }

    /* 1.2. Check module name in stacktrace */
        Dwfl_Module* module = dwfl_addrmodule(dwfl, (uintptr_t)ip);
        const char *module_name = dwfl_module_info(module, NULL, NULL, NULL, NULL, NULL, NULL, NULL);
        if (! strstr(module_name, stacktrace_module_name) ) {                            /* 'Module name' doesn't match -> proceed ... */
            DEV_DEBUG_PRINT_MSG(">>> Unwind: No match (%s:xxx)", module_name);
            continue;
        }
        /* NOTE: Even if the module name matches, and we don't search for fct-names, we still have to ALWAYS check for `LIBIOTRACE_PRECLUDED_PTHREAD_FCT` */


    /* 1.3. Retrieve & check function- (i.e., symbol) name */
        unw_word_t offset = 0;
        char symbol_buf[4096];
        const int status_get_fct_name = unw_get_proc_name(&unw_cursor, symbol_buf, sizeof(symbol_buf), &offset);
      /* Retrieved fct name successfully */
        if (! status_get_fct_name) {
            if (!strcmp(symbol_buf, LIBIOTRACE_PRECLUDED_PTHREAD_FCT)) {                /* !!!  ALWAYS ignore libiotrace's `pthread_create` routine wrapper (even when not searching for fct-names)  !!! */
                DEV_DEBUG_PRINT_MSG(">>> Unwind: IGNORED FCT (%s:%s)", module_name, symbol_buf);
                continue;
            }

            if (stacktrace_fct_name && strstr(symbol_buf, stacktrace_fct_name)) {       /* We're searching for fct-names AND fct-name matches */
                goto found_entry;
            }
        }

        if (!stacktrace_fct_name) {                                                     /* We're not searching for fct-names  (and fct-name != `LIBIOTRACE_PRECLUDED_PTHREAD_FCT`) */
            goto found_entry;
        }

        DEV_DEBUG_PRINT_MSG(">>> Unwind: No match (%s:%s)", module_name, (!status_get_fct_name) ? symbol_buf : "?");
        continue;


    found_entry:
        DEV_DEBUG_PRINT_MSG(">>> Unwind: Found match (%s:%s)", module_name, (!status_get_fct_name) ? symbol_buf : "?");
        found_stack_entry = true;
        break;
    } while (unw_step(&unw_cursor) > 0);


/* 2. Cleanup (destroy unwinding context of process) */
    dwfl_end(dwfl);
    _UPT_destroy(unw_ctx);


    return found_stack_entry;
}


/* - Helpers - */
static Dwfl* init_ldw_for_proc(pid_t tid) {
    static const Dwfl_Callbacks dwfl_callbacks = {
        .find_elf = dwfl_linux_proc_find_elf,
        .find_debuginfo = dwfl_standard_find_debuginfo
    };

    Dwfl* dwfl;
    if ( (dwfl = dwfl_begin(&dwfl_callbacks))      &&
          !dwfl_linux_proc_attach(dwfl, tid, true) &&
          !dwfl_linux_proc_report(dwfl, tid)       &&
          !dwfl_report_end(dwfl, NULL, NULL) ) {
        return dwfl;
    }

    dwfl_end(dwfl);
    LOG_ERROR_AND_EXIT("libdw -- failed to init for process %d", tid);
}
