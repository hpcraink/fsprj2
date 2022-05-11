#ifndef COMMON_FNRES_STRACING_SCERB_H_
#define COMMON_FNRES_STRACING_SCERB_H_

#include <stdio.h>
#include <stdint.h>


/* -- Data types -- */
typedef struct sm_scerb sm_scerb_t;

typedef struct {
    uint64_t ts_in_ns;
    int fd;
    enum { OPEN, CLOSE } type;
/* !!!  DON'T CHANGE THE FOLLOWING LAST 2 MEMBERS (otherwise reading in 2 steps via `offsetof` won't work anymore)  !!! */
    size_t filename_len;
    char filename[];        /* Flexible Array Member w/ max len = `FILENAME_MAX` */
} scevent_t;


/* -- Macros -- */
#define FNRES_SCEVENT_SIZEOF(STRUCT_PTR)       ( sizeof(*(STRUCT_PTR)) + (STRUCT_PTR)->filename_len )
#define FNRES_SCEVENT_MAX_SIZE                 ( sizeof(scevent_t) + FILENAME_MAX )


/* -- Function prototypes -- */
/* - Debugging - */
void fnres_scerb_debug_print_scevent(scevent_t*, FILE*);

#endif /* COMMON_FNRES_STRACING_SCERB_H_ */
