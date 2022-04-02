#include <stdio.h>
#include "scerb.h"


/* --  Functions  -- */
/* -  Debugging  - */
void fnres_scerb_debug_print_scevent(fnres_scevent* event, FILE* output_stream) {
    fprintf((output_stream) ? (output_stream) : (stdout), "`fd`=%d, `event_type`=%s, `filename_len`=%zu, `filename`=\"%s\"",
            event->fd,
            OPEN == event->type ? "OPEN" : "CLOSE",
            event->filename_len,
            event->filename);
}
