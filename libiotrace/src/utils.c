#include <string.h>
#include <time.h>

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
 * Gets actual time in nano seconds.
 *
 * @return time in nano seconds
 */
inline u_int64_t gettime(void)
{
	struct timespec t;
	u_int64_t time;
#ifdef REALTIME
	clock_gettime(CLOCK_REALTIME, &t);
#else
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
#endif
	time = (u_int64_t)t.tv_sec * 1000000000ll + (u_int64_t)t.tv_nsec;
	return time;
}