#ifndef LIBIOTRACE_UTILS_H_
#define LIBIOTRACE_UTILS_H_

#include "libiotrace_config.h"

BEGIN_C_DECLS

void generate_env(char *env, const char *key, const int key_length, const char *value);
char *read_line(const char *buf, const size_t len, char **pos);

END_C_DECLS

#endif /* LIBIOTRACE_UTILS_H_ */
