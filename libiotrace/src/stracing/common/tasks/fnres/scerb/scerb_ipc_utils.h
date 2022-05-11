/**
 * Functions for creating, attaching & destroying shared memory segments
 */
#ifndef COMMON_SM_HELPERS_H_
#define COMMON_SM_HELPERS_H_

#include <stdbool.h>


/* -- Function prototypes -- */
int sm_ipc_attach_create_smo(
        char* smo_name, off_t smo_min_len,
        void** shared_mem_addr_ptr, unsigned long long* shared_mem_len_ptr,
        bool o_creat);

int sm_ipc_detach_smo(void** shared_mem_addr_ptr, char* smo_name);

#endif /* COMMON_SM_HELPERS_H_ */
