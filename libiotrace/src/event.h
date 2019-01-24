#ifndef LIBIOTRACE_EVENT_H
#define LIBIOTRACE_EVENT_H

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <pthread.h>
#include <time.h>
//#include <sys/types.h>
#include <limits.h>
#include "libiotrace_config.h"

#define MAXFILENAME PATH_MAX /* get length filename from limits.h */

static void init() ATTRIBUTE_CONSTRUCTOR;

enum file_type {
	stream,
	discriptor
};

/* basic struct for every call */
struct basic {
	pid_t pid;
	pthread_t pthread;
	time_t time_start;
	time_t time_end;
	file_type type;
	union file {
		FILE* file_stream;
		int file_discriptor;
	};
};

/* struct for file open */
struct open {
	char file_name[MAXFILENAME];
};

/* Function pointers for glibc functions */
static FILE * (*real_fopen) (const char *filename, const char *opentype) = NULL;

/* initialize pointers for glibc functions */
static void init( ){
	real_fopen = dlsym(RTLD_NEXT, "fopen");
}

inline void get_basic(basic data) {
	data.pid = getpid();
	data.pthread = pthread_self();
}

void print_basic(basic data) {
	printf("basic: pid:%lu;pthread:%lu;start:%lu;end:%lu;", data.pid, data.pthread, data.time_start, data.time_end);
	switch(data.type) {
		case stream:     printf("stream:%u;",data.file); break;
		case discriptor: printf("discriptor:%p;",data.file); break;
		default:         printf("Error;"); break;
	}
	printf("\n");
}

void print_open(open data) {
	printf("open:%s\n",data.file_name);
}

FILE * fopen (const char *filename, const char *opentype) {
	FILE * file;
	basic data;
	open open_data;

	get_basic(data);
	data.type = stream;
	data.file = *file;

	strncpy(open_data.file_name, filename, MAXFILENAME);

	data.time_start = time(NULL);
	file = real_fopen (filename, opentype);
	data.time_end = time(NULL);

	print_basic(data);

	return file;
}

#endif /* LIBIOTRACE_EVENT_H */
