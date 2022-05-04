#include <stdio.h>
#include <assert.h>
#include "scerb.h"


/* -- Functions -- */
/* - Misc. - */
bool fnres_scerb_is_inited(sm_scerb_t* sm_scerb) {
    assert( sm_scerb && "params may not be `NULL`" );
    return NULL != sm_scerb;
}

/* -  Debugging  - */
void fnres_scerb_debug_print_scevent(scevent_t* event, FILE* output_stream) {
    fprintf((output_stream) ? (output_stream) : (stdout), "`fd`=%d, `event_type`=%s, `filename_len`=%zu, `filename`=\"%s\"",
            event->fd,
            OPEN == event->type ? "OPEN" : "CLOSE",
            event->filename_len,
            event->filename);
}
