#include <errno.h>
#include <stdlib.h>
#include <string.h>

#include <execinfo.h>
#include <limits.h>

#include "utils.h"

/**
 * Generates environment variable from "key" and "value".
 *
 * @param[out] env       Pointer to buffer in which environment variable
 *                       is stored. Buffer must have sufficient length
 *                       (strlen(key) + strlen(value) + 2).
 * @param[in] key        "\0" terminated key of the environment variable.
 * @param[in] key_length Char count of "key" excluding terminating "\0"
 *                       (strlen(key)).
 * @param[in] value      "\0" terminated value of the environment
 *                       variable.
 */
void generate_env(char *env, const char *key, const int key_length,
		const char *value) {
	strcpy(env, key);
	strcpy(env + key_length, "=");
	strcpy(env + key_length + 1, value);
}

/**
 * Get next line out of buffer buf.
 *
 * Each line break in buf is changed to '\0' and each call to read_line returns
 * a pointer to the now '\0' terminated next line in buf.
 *
 * @param[in,out] buf  Pointer to '\0' terminated buffer. The pointer is const
 *                     but the buffer itself will be manipulated (line breaks
 *                     get changed to '\0').
 * @param[in]     len  Length of buffer buf. Can but must not include the
 *                     terminating '\0'.
 * @param[in,out] pos  Pointer to a pointer to the current position in buf.
 *                     *pos must be equal to buf for first call of read_line for
 *                     one buffer. pos should not be changed outside of
 *                     read_line between subsequent calls to read_line for one
 *                     buffer.
 * @return Pointer to next '\0' terminated line in buf or NULL if no next line
 *         exists.
 */
char* read_line(const char *buf, const size_t len, char **pos) {
	char *tmp_pos = *pos;

	if (**pos == '\0') {
		return NULL;
	}

	for (; **pos != '\0' && *pos < buf + len; (*pos)++) {
		if ('\r' == **pos) {
			**pos = '\0';
			(*pos)++;
			if ('\n' == **pos) {
				**pos = '\0';
				(*pos)++;
			}
			return tmp_pos;
		}
		if ('\n' == **pos) {
			**pos = '\0';
			(*pos)++;
			return tmp_pos;
		}
	}

	if (tmp_pos < buf + len) {
		return tmp_pos;
	}
	return NULL;
}

/**
 * Get short version of log_name.
 *
 * Writes a short version of log_name to short_log_name. If log_name_len is
 * less or equal to short_log_name_len log_name is copied to short_log_name.
 * Else the last short_log_name_len bytes of log_name are copied to
 * short_log_name.
 *
 * @param[out] short_log_name     Buffer to write short version of log_name to.
 *                                Shortened version includes terminating '\0'.
 * @param[in]  short_log_name_len Length of buffer short_log_name.
 * @param[in]  log_name           '\0' terminated log_name to shorten.
 * @param[in]  log_name_len       Length of log_name.
 */
void shorten_log_name(char *short_log_name, const int short_log_name_len,
		const char *log_name, const int log_name_len) {
	if (log_name_len <= short_log_name_len) {
		strncpy(short_log_name, log_name, short_log_name_len);
	} else {
		strncpy(short_log_name,
				log_name + (log_name_len - short_log_name_len),
				short_log_name_len);
	}
}


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


/**
 * Parses the path to the libiotrace so file from the current stacktrace
 * NOTE: Works ONLY on GNU/Linux (since the returned format on e.g., macOS is different)
 *
 * @return                   Pointer to `malloc`'ed path string or `NULL` on failure
 */
char* get_libiotrace_so_file_path(void) {
    char* so_filename;

    void* backtrace_rtn_addr[1];
    char** backtrace_fct_names = NULL;
    char* strtok_r_saveptr = NULL;
    if (1 != backtrace(backtrace_rtn_addr, sizeof backtrace_rtn_addr / sizeof backtrace_rtn_addr[0]) ||
        ! (backtrace_fct_names = backtrace_symbols(backtrace_rtn_addr, 1)) ||
        ! strtok_r(backtrace_fct_names[0], "(", &strtok_r_saveptr) ||
        ! (so_filename = strdup(backtrace_fct_names[0])) ) {

        if (backtrace_fct_names) { free(backtrace_fct_names); }
        return NULL;
    }

    free(backtrace_fct_names);
    return so_filename;
}

/**
 * Implementation of `dirname`(3), taken from the Android NDK, which ALWAYS uses
 * the provided buffer
 *
 * BACKGROUND: The glibc implementation MAY modify the provided buffer OR use
 *             an internal static buffer (making it thus, not thread safe)
 *
 * @param[in]  path          Path from which the dirname shall be derived
 * @param[out] buffer        Buffer which will contain the dirname
 * @param[in] bufflen        Size of provided buffer in bytes
 * @return                   Length of derived dirname, -1 on failure
 */
int
dirname_r(const char*  path, char*  buffer, size_t  bufflen)
{
    const char *endp, *startp;
    int         result, len;

    /* Empty or NULL string gets treated as "." */
    if (path == NULL || *path == '\0') {
        startp = ".";
        len  = 1;
        goto Exit;
    }

    /* Strip trailing slashes */
    endp = path + strlen(path) - 1;
    while (endp > path && *endp == '/')
        endp--;

    /* Find the start of the dir */
    while (endp > path && *endp != '/')
        endp--;

    /* Either the dir is "/" or there are no slashes */
    if (endp == path) {
        startp = (*endp == '/') ? "/" : ".";
        len  = 1;
        goto Exit;
    }

    do {
        endp--;
    } while (endp > path && *endp == '/');

    startp = path;
    len = endp - startp +1;

Exit:
    result = len;
    if (len+1 > PATH_MAX) {
        errno = ENAMETOOLONG;
        return -1;
    }
    if (buffer == NULL)
        return result;

    if (len > (int)bufflen-1) {
        len    = (int)bufflen-1;
        result = -1;
        errno  = ERANGE;
    }

    if (len >= 0) {
        memcpy( buffer, startp, len );
        buffer[len] = 0;
    }
    return result;
}

/**
 * Implementation of `basename`(3), taken from the Android NDK, which ALWAYS uses
 * the provided buffer
 *
 * BACKGROUND: The glibc implementation MAY modify the provided buffer OR use
 *             an internal static buffer (making it thus, not thread safe)
 *
 * @param[in]  path          Path from which the basename shall be derived
 * @param[out] buffer        Buffer which will contain the basename
 * @param[in] bufflen        Size of provided buffer in bytes
 * @return                   Length of derived basename, -1 on failure
 */
int
basename_r(const char* path, char*  buffer, size_t  bufflen)
{
    const char *endp, *startp;
    int         len, result;

    /* Empty or NULL string gets treated as "." */
    if (path == NULL || *path == '\0') {
        startp  = ".";
        len     = 1;
        goto Exit;
    }

    /* Strip trailing slashes */
    endp = path + strlen(path) - 1;
    while (endp > path && *endp == '/')
        endp--;

    /* All slashes becomes "/" */
    if (endp == path && *endp == '/') {
        startp = "/";
        len    = 1;
        goto Exit;
    }

    /* Find the start of the base */
    startp = endp;
    while (startp > path && *(startp - 1) != '/')
        startp--;

    len = endp - startp +1;

Exit:
    result = len;
    if (buffer == NULL) {
        return result;
    }
    if (len > (int)bufflen-1) {
        len    = (int)bufflen-1;
        result = -1;
        errno  = ERANGE;
    }

    if (len >= 0) {
        memcpy( buffer, startp, len );
        buffer[len] = 0;
    }
    return result;
}
