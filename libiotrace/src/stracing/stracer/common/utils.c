#include <errno.h>
#include <time.h>
#include <stdlib.h>

#include "utils.h"
#include "error.h"


/* -- Functions -- */
int str_to_long(char* str, long* num) {
    char* parse_end_ptr = NULL;
    if (NULL != (parse_end_ptr = str) && NULL != num) {
        char* p_end_ptr = NULL;
        const long parsed_number = (int)strtol(parse_end_ptr, &p_end_ptr, 10);

        if (parse_end_ptr != p_end_ptr && ERANGE != errno) {
            *num = parsed_number;
            return 0;
        }
    }
    return -1;
}

uint64_t gettime(void) {
    struct timespec t;
    DIE_WHEN_ERRNO( clock_gettime(CLOCK_REALTIME, &t) );
    return (u_int64_t)t.tv_sec * 1000000000ll + (u_int64_t)t.tv_nsec;
}
