#ifndef LIBIOTRACE_UTILS_H_
#define LIBIOTRACE_UTILS_H_

#include <sys/types.h>

#include "libiotrace_config.h"

BEGIN_C_DECLS

void generate_env(char *env, const char *key, const int key_length,
		const char *value);
char* read_line(const char *buf, const size_t len, char **pos);
void shorten_log_name(char *short_log_name, const int short_log_name_len,
		const char *log_name, const int log_name_len);
// ToDo: as macro with return value?
u_int64_t gettime(void);

END_C_DECLS

#endif /* LIBIOTRACE_UTILS_H_ */
