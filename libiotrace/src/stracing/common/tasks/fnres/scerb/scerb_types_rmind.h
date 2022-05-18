#ifndef COMMON_STRACING_FNRES_SCERB_RMIND_H_
#define COMMON_STRACING_FNRES_SCERB_RMIND_H_

#include <stdint.h>
#include "../../../../../libs/rmind-ringbuf/ringbuf.h"


/* -- Data types -- */
struct sm_scerb {
    ringbuf_t ringbuf;
    uint8_t buf[STRACING_FNRES_RB_SIZE];
};

#endif /* COMMON_STRACING_FNRES_SCERB_RMIND_H_ */
