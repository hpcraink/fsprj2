#ifndef LIBIOTRACE_EVENT_H
#define LIBIOTRACE_EVENT_H

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include "libiotrace_config.h"

#include "json_include_struct.h"


void get_basic(struct basic *data);

void writeData(struct basic *data);

#endif /* LIBIOTRACE_EVENT_H */
