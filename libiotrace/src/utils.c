#include <errno.h>
#include <stdlib.h>
#include <string.h>

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
 * Rudimentary TODO: and NOT PROPERLY VALIDATED implementation of `dirname`(3),
 * which modifies the provided buffer
 *
 * BACKGROUND: The glibc implementation MAY modify the provided buffer OR use
 *             an internal static buffer (making it not thread safe)
 *
 * @param[in,out]  path      Path buffer containing the path from which the
 *                           dirname shall be derived
 * @param[out]     path_size Size of allocated path buffer (incl. `\0`)
 * @return                   0 on success, -1 on failure
 */
int dirname_n(char* path, int path_size) {
  if (!path || path_size < 2) {   // For cwd ('.') we need at least 2 bytes
    return -1;
  }

  char* last_slash = NULL;
  for (char* p = path; *p &&
                            (p - path +1 <= path_size); // Make sure we're not reading more than we're supposed to (in case of an unterminated string)
                            p++) {
    if ('/' == *p) {
      last_slash = p;
    }
  }

  if (!last_slash || path == last_slash) {
    path[0] = '.';
    last_slash = path+ 1;
  }

  *last_slash = '\0';
  return 0;
}
