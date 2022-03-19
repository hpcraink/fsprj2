#ifndef LIBIOTRACE_STRACER_IPC_H
#define LIBIOTRACE_STRACER_IPC_H


/* -- Data types -- */
typedef struct {
    enum {
        PROBE_TRACER_RUNNING,
        TRACEE_REQUEST_TRACING
    } request;
} uxd_sock_ipc_requests_t;


#endif /* LIBIOTRACE_STRACER_IPC_H */
