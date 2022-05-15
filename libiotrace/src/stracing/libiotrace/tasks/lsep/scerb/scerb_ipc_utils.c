#include "../../../../../event.h"
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <unistd.h>

#include "scerb_ipc_utils.h"


/* -- Macros -- */
#define RTN_VAL_WHEN_ERRNO(FUNC) __extension__({ ({ \
    int __val = (FUNC);                             \
    (-1 == __val ? ({ return -1; -1; }) : __val);   \
  }); })


/* -- Functions -- */
int sm_ipc_attach_create_smo(
        char* smo_name, off_t smo_min_len,
        void** shared_mem_addr_ptr, unsigned long long* shared_mem_len_ptr,
        bool o_creat) {
/* Create smo */
    int smo_fd = RTN_VAL_WHEN_ERRNO( CALL_REAL_ALLOC_SYNC(shm_open)(smo_name, O_RDWR | ((o_creat) ? (O_CREAT) : (0)), S_IRUSR | S_IWUSR) );
    if (o_creat) {
        RTN_VAL_WHEN_ERRNO( ftruncate(smo_fd, smo_min_len) );
    }

/* Get actual length */
    struct stat stat_info;
    RTN_VAL_WHEN_ERRNO( fstat(smo_fd, &stat_info) );
    *shared_mem_len_ptr = stat_info.st_size;         // NOTE: What we get depends on the page size (which may be retrieved via `getconf PAGE_SIZE` or `pagesize`)

/* Map it in address space of caller */
    if (MAP_FAILED == (*shared_mem_addr_ptr = CALL_REAL_POSIX_SYNC(mmap)(NULL, *shared_mem_len_ptr, PROT_READ | PROT_WRITE, MAP_SHARED, smo_fd, 0))) {
        return -1;
    }
    CALL_REAL_POSIX_SYNC(close)(smo_fd);

    return 0;
}


int sm_ipc_detach_smo(void** shared_mem_addr_ptr, char* smo_name) {
    int smo_fd = RTN_VAL_WHEN_ERRNO( CALL_REAL_ALLOC_SYNC(shm_open)(smo_name, O_RDONLY, 0) );     // NOTE: `mode` flags are required (not a variadic fct on Linux)
    struct stat stat_info;
    RTN_VAL_WHEN_ERRNO( fstat(smo_fd, &stat_info) );

    RTN_VAL_WHEN_ERRNO( CALL_REAL_POSIX_SYNC(munmap)(*shared_mem_addr_ptr, stat_info.st_size) );
    *shared_mem_addr_ptr = NULL;

    CALL_REAL_POSIX_SYNC(close)(smo_fd);

    return 0;
}
