#ifndef FNRES_SCERB_RMIND_COMMON_H_
#define FNRES_SCERB_RMIND_COMMON_H_

#include <stdint.h>
#include "../rmind-ringbuf/ringbuf.h"

/* -- Consts -- */
#define BUFFER_SIZE_IN_BYTES (10000)


/* -- Data types -- */
struct sm_scerb {
    ringbuf_t ringbuf;
    uint8_t buf[BUFFER_SIZE_IN_BYTES];
};

#endif /* FNRES_SCERB_RMIND_COMMON_H_ */
