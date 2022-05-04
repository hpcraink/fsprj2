/**
 * Syscall Event RingBuffer (aka., `scerb`) storing syscall events (aka., `scevent`) for the filename resolution (aka., `fnres`)
 */
#ifndef FNRES_SCERB_COMMON_H_
#define FNRES_SCERB_COMMON_H_

#include <stdio.h>
#include <stdint.h>
#include <stdbool.h>


/* -- Data types -- */
typedef struct sm_scerb sm_scerb_t;

typedef struct {
    uint64_t ts_in_ns;
    int fd;
    enum { OPEN, CLOSE } type;
    size_t filename_len;
    char filename[];     // Flexible Array Member; max = FILENAME_MAX
} scevent_t;


/* -- Macros -- */
#define FNRES_SCEVENT_SIZE_OF_INST(STRUCT_PTR) ( sizeof(*(STRUCT_PTR)) + (STRUCT_PTR)->filename_len )
#define FNRES_SCEVENT_MAX_SIZE                 ( sizeof(scevent_t) + FILENAME_MAX )


/* -- Function prototypes -- */
/* - Misc. - */
/**
 * @brief                                 Allows checking whether init has been performed
 *
 * @param[in] sm_scerb                    Handle (i.e., pointer) to shared memory segment containing scerb + buffer
 * @return                                Returns `1` if inited, `0` otherwise
 */
bool fnres_scerb_is_inited(sm_scerb_t* sm_scerb);



/* -- Debugging -- */
void fnres_scerb_debug_print_scevent(scevent_t*, FILE*);

#endif /* FNRES_SCERB_COMMON_H_ */
