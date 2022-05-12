/**
 * Functions for creating, attaching & destroying shared memory segments
 */
#ifndef COMMON_SM_HELPERS_H_
#define COMMON_SM_HELPERS_H_

#include <stdbool.h>


/* -- Function prototypes -- */
/**
 * @brief                                 Creates (optional) & attaches shared memory object (smo)
 *
 * @param[out] smo_name                   Handle which shall be used for smo
 * @param[in] smo_min_len                 Minimum size of smo (NOTE: will be in most cases larger (due to kernel rounding to page size))
 * @param[out] shared_mem_addr_ptr        Will contain start address of smo
 * @param[out] shared_mem_len_ptr         Will contain actual length of smo
 * @param[in] o_creat                     Whether smo shall be created, if it doesn't exist yet
 * @return int                            Returns `-1` on failure and `0` on success
 */
int sm_ipc_attach_create_smo(
        char* smo_name, off_t smo_min_len,
        void** shared_mem_addr_ptr, unsigned long long* shared_mem_len_ptr,
        bool o_creat);

/**
 * @brief                                 Detaches & unlinks smo
 *
 * @param[in,out] shared_mem_addr_ptr     Address of smo, which will be set to `NULL`
 * @param[in] smo_name                    Handle for smo (which was used during creation)
 * @return int                            Returns `-1` on failure and `0` on success
 */
int sm_ipc_detach_smo(void** shared_mem_addr_ptr, char* smo_name);

#endif /* COMMON_SM_HELPERS_H_ */
