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


int str_to_long(char* str, long* num);

char* get_libiotrace_so_file_path(void);

int dirname_r(const char* path, char* buffer, size_t bufflen);
int basename_r(const char* path, char* buffer, size_t bufflen);


END_C_DECLS

#endif /* LIBIOTRACE_UTILS_H_ */
