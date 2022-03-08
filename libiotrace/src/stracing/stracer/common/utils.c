#include <errno.h>
#include <stdlib.h>

/**
 * Parse C string (w/ number in base 10) as signed long
 *
 * @param[in]  str String to be parsed
 * @param[out] num Parsed long
 * @return     0 on success, -1 on failure
 */
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
