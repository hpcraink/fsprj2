#ifndef LIBIOTRACE_EVENT_H
#define LIBIOTRACE_EVENT_H

#include <stdio.h>
#include <string.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "libiotrace_config.h"

#include "json_include_struct.h"


static char init_done = 0;
// ToDo: use macro ATTRIBUTE_CONSTRUCTOR
void init()__attribute__((constructor));

void get_basic(struct basic *data);

// ToDo: as macro with return value?
u_int64_t gettime(void);

void writeData(struct basic *data);

#endif /* LIBIOTRACE_EVENT_H */
