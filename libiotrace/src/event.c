#include "libiotrace_config.h"
#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
//#include <pthread.h>
//#include <sys/types.h>
#include <sys/syscall.h>

#include "event.h"
#include "os.h"

void get_basic(struct basic data) {
	data.process_id = getpid();
	//data.pthread = pthread_self();
	//data.pthread = gettid();

	// call gettid() as syscall because there is no implementation in glibc
	data.thread_id = iotrace_gettid();
}

void print_basic(struct basic data) {
	printf("basic: process_id:%u;thread_id:%u;function_name:%s;start:%lu;end:%lu;", data.process_id, data.thread_id, data.function_name, data.time_start, data.time_end);
	switch(data.type) {
		case stream:     printf("stream:%p;",data.file_stream); break;
		case descriptor: printf("descriptor:%d;",data.file_descriptor); break;
		default:         printf("Error;"); break;
	}
	printf("\n");
}

void print_open(struct open data) {
	printf("  open: file:%s;", data.file_name);
	switch(data.mode) {
		case read_only:      printf("mode:read_only;"); break;
		case write_only:     printf("mode:write_only;"); break;
		case read_and_write: printf("mode:read_and_write;"); break;
		case unknown:        printf("mode:unknown;"); break;
		default:             printf("Error;"); break;
	}
	printf("creation:");
	if (data.creation.cloexec) printf("cloexec,");
	if (data.creation.creat) printf("creat,");
	if (data.creation.directory) printf("directory,");
	if (data.creation.excl) printf("excl,");
	if (data.creation.noctty) printf("noctty,");
	if (data.creation.nofollow) printf("nofollow,");
	if (data.creation.tmpfile) printf("tmpfile,");
	if (data.creation.trunc) printf("trunc,");
	printf(";status:");
	if (data.status.append) printf("append,");
	if (data.status.async) printf("async,");
	if (data.status.direct) printf("direct,");
	if (data.status.dsync) printf("dsync,");
	if (data.status.largefile) printf("largefile,");
	if (data.status.ndelay) printf("ndelay,");
	if (data.status.noatime) printf("noatime,");
	if (data.status.nonblock) printf("nonblock,");
	if (data.status.path) printf("path,");
	if (data.status.sync) printf("sync,");
	printf(";\n");
}

void print_close(struct close data) {
	printf("  close: return:%d;\n", data.return_value);
}
