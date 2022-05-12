#ifndef STRACER_STRACING_FNRES_H_
#define STRACER_STRACING_FNRES_H_

#include <sys/types.h>
#include <stdbool.h>
#include "../../../common/stracer_types.h"


/* -- Function prototypes -- */
bool stracing_fnres_is_inited(void);

void stracing_fnres_init(long scerbmap_max_size);
void stracing_fnres_fin(void);

void stracing_fnres_attach_sm(pid_t tid);
void stracing_fnres_write_scevent(pid_t tid, scevent_t* event_buf_ptr);
void stracing_fnres_destroy_sm(pid_t tid);

#endif /* STRACER_STRACING_FNRES_H_ */
