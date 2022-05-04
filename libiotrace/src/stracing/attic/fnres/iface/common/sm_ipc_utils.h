#ifndef FNRES_SCERB_SM_HELPERS_H_
#define FNRES_SCERB_SM_HELPERS_H_

#include <stdbool.h>
#include <unistd.h>


/* -- Function prototypes -- */
int sm_ipc_attach_create_map_smo(
        char* smo_name, off_t smo_min_len,
        void** shared_mem_addr_ptr, unsigned long long* shared_mem_len_ptr,
        bool o_creat);

int sm_ipc_detach_smo(void** shared_mem_addr_ptr, char* smo_name);

#endif /* FNRES_SCERB_SM_HELPERS_H_ */
