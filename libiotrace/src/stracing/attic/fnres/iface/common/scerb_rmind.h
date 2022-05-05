#ifndef FNRES_SCERB_RMIND_COMMON_H_
#define FNRES_SCERB_RMIND_COMMON_H_

#include <stdint.h>
#include "../rmind-ringbuf/ringbuf.h"


/* -- Data types -- */
struct sm_scerb {
    ringbuf_t ringbuf;
    uint8_t buf[STRACE_FNRES_RB_SIZE];
};

#endif /* FNRES_SCERB_RMIND_COMMON_H_ */
