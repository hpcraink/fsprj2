#ifndef LIBIOTRACE_STRACER_UXD_IPC_TYPES_H
#define LIBIOTRACE_STRACER_UXD_IPC_TYPES_H


/* -- Data types -- */
typedef struct {
    enum {
        PARENT_PROBE_TRACER_RUNNING,

        PARENT_REQUEST_TRACING,
        TRACER_REQUEST_ACCEPTED

        // ...
    } msg_type;
    union {
        pid_t tracee_tid;
    } msg_payload;
} uxd_sock_ipc_msg_t;


#endif /* LIBIOTRACE_STRACER_UXD_IPC_TYPES_H */
