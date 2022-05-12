/**
 * Data types used by stracer, which are also pertinent for libiotrace
 */
#ifndef COMMON_STRACING_TYPES_H_
#define COMMON_STRACING_TYPES_H_

#include <stdio.h>
#include <stdint.h>


/* -- Data types -- */
/**
 * Describes a syscall event, traced by stracer
 */
typedef struct {
    uint64_t ts_in_ns;
    bool succeeded;
    int fd;
    enum { OPEN, CLOSE } type;
/* !!!  DON'T CHANGE THE FOLLOWING LAST 2 MEMBERS (otherwise reading in 2 steps via `offsetof` won't work anymore)  !!! */
    size_t filename_len;
    char filename[];        /* Flexible Array Member w/ max len = `FILENAME_MAX` */
} scevent_t;


/* -- Macros -- */
#define SCEVENT_FILENAME_MAX             FILENAME_MAX
#define SCEVENT_SIZEOF(STRUCT_PTR)       ( sizeof(*(STRUCT_PTR)) + (STRUCT_PTR)->filename_len )
#define SCEVENT_MAX_SIZE                 ( sizeof(scevent_t) + SCEVENT_FILENAME_MAX )

#endif /* COMMON_STRACING_TYPES_H_ */
