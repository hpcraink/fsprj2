#ifndef LIBIOTRACE_EVENT_H
#define LIBIOTRACE_EVENT_H

#include <stdio.h>
#include <string.h>
#include <dlfcn.h>
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
#include <sys/types.h>
#include <limits.h>
#include "libiotrace_config.h"

#define MAXFILENAME PATH_MAX /* get length filename from limits.h */
#define MAXFUNCTIONNAME 40

enum file_type {
	stream,
	descriptor
};

enum access_mode {
	read_only,        //O_RDONLY or r
	write_only,       //O_WRONLY or w, a
	read_and_write,   //O_RDWR or r+, w+, a+
	unknown
};

struct creation_flags {
	int cloexec;
	int creat;
	int directory;
	int excl;
	int noctty;
	int nofollow;
	int tmpfile;
	int trunc;
};

struct status_flags {
	int append;
	int async;
	int direct;
	int dsync;
	int largefile;
	int noatime;
	int nonblock;
	int ndelay;
	int path;
	int sync;
};

/* basic struct for every call */
struct basic {
	pid_t process_id;
	pid_t thread_id;
	char function_name[MAXFUNCTIONNAME];
	clock_t time_start;
	clock_t time_end;
	enum file_type type;
	union {
		FILE* file_stream;
		int file_descriptor;
	};
};

/* struct for file open */
struct open {
	char file_name[MAXFILENAME];
	enum access_mode mode;
	struct creation_flags creation;
	struct status_flags status;
};

/* struct for file close */
struct close {
	int return_value;
};

void get_basic(struct basic data);

void print_basic(struct basic data);

void print_open(struct open data);

void print_close(struct close data);

#endif /* LIBIOTRACE_EVENT_H */
