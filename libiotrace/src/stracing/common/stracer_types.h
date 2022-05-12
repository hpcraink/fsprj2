#ifndef COMMON_STRACING_TYPES_H_
#define COMMON_STRACING_TYPES_H_

#include <stdio.h>
#include <stdint.h>


/* -- Data types -- */
typedef struct {
    uint64_t ts_in_ns;
    bool success;
    int fd;
    enum { OPEN, CLOSE } type;
/* !!!  DON'T CHANGE THE FOLLOWING LAST 2 MEMBERS (otherwise reading in 2 steps via `offsetof` won't work anymore)  !!! */
    size_t filename_len;
    char filename[];        /* Flexible Array Member w/ max len = `FILENAME_MAX` */
} scevent_t;


/* -- Macros -- */
#define FNRES_SCEVENT_MAX_FILENAME             FILENAME_MAX
#define FNRES_SCEVENT_SIZEOF(STRUCT_PTR)       ( sizeof(*(STRUCT_PTR)) + (STRUCT_PTR)->filename_len )
#define FNRES_SCEVENT_MAX_SIZE                 ( sizeof(scevent_t) + FNRES_SCEVENT_MAX_FILENAME )


/* - Debugging - */
void fnres_scerb_debug_print_scevent(scevent_t*, FILE*);

#endif /* COMMON_STRACING_TYPES_H_ */
