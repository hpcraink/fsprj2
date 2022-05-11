#include <stdio.h>
#include <assert.h>
#include "scerb.h"


/* -- Functions -- */
/* -  Debugging  - */
void fnres_scerb_debug_print_scevent(scevent_t* event, FILE* output_stream) {
    fprintf((output_stream) ? (output_stream) : (stdout), "`ts_in_ns`=%lu, `fd`=%d, `event_type`=%s, `filename_len`=%zu, `filename`=\"%s\"",
            event->ts_in_ns,
            event->fd,
            OPEN == event->type ? "OPEN" : "CLOSE",
            event->filename_len,
            event->filename);
}
