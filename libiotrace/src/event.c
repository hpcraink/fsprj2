#include "libiotrace_config.h"

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#endif
//#include <sys/types.h>
#include <sys/syscall.h>
#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#ifdef HAVE_JSONC_H
#  include <json-c/json.h>
#endif

#include "event.h"

/* Buffer */
#define BUFFER_SIZE 10
// ToDo: Buffer for structs with different size (size as first element in each struct?)
static struct basic data_buffer[BUFFER_SIZE];
static struct basic* endpos;
static struct basic* pos;

/* Mutex */
static pthread_mutex_t lock;

static void init()__attribute__((constructor));
// ToDo: test for destructor in cmake
static void cleanup()__attribute__((destructor));

static void init() {
	endpos = data_buffer + BUFFER_SIZE - 1;
	pos = data_buffer;

	pthread_mutex_init(&lock, NULL);

	// don't use /dev/urandom for generating hash in json-c
	// (because we don't want to have additional file IO)
	json_global_set_string_hash(JSON_C_STR_HASH_PERLLIKE);
}

void get_basic(struct basic data) {
	data.process_id = getpid();
	//data.pthread = pthread_self();
	//data.pthread = gettid();

	// call gettid() as syscall because there is no implementation in glibc
	data.thread_id = syscall(__NR_gettid);
}

void print_basic(struct basic data) {
	int ret;

	json_object* root_object = json_object_new_object();
	ret = json_object_object_add(root_object, "process_id",
			json_object_new_int64(data.process_id));
	ret = json_object_object_add(root_object, "thread_id",
			json_object_new_int64(data.thread_id));
	ret = json_object_object_add(root_object, "function_name",
			json_object_new_string(data.function_name));
	ret = json_object_object_add(root_object, "start",
			json_object_new_int64(data.time_start));
	ret = json_object_object_add(root_object, "end",
			json_object_new_int64(data.time_end));

	json_object* file_type = json_object_new_object();
	switch (data.type) {
	case stream:
		ret = json_object_object_add(file_type, "stream",
				json_object_new_int64(data.file_stream));
		break;
	case descriptor:
		ret = json_object_object_add(file_type, "descriptor",
				json_object_new_int64(data.file_descriptor));
		break;
	default:
		ret = json_object_object_add(file_type, "file_type",
				json_object_new_string("not implemented"));
	}
	ret = json_object_object_add(root_object, "file_type", file_type);

	json_object* function_data = json_object_new_object();
	switch (data.func_type) {
	case open_function:
		ret = json_object_object_add(function_data, "function_type",
				json_object_new_string("open_function"));

		json_object* mode;
		switch (data.open_data.mode) {
		case read_only:
			mode = json_object_new_string("read_only");
			break;
		case write_only:
			mode = json_object_new_string("write_only");
			break;
		case read_and_write:
			mode = json_object_new_string("read_and_write");
			break;
		case unknown:
		default:
			mode = json_object_new_string("unknown");
		}
		ret = json_object_object_add(function_data, "mode", mode);
		ret = json_object_object_add(function_data, "file_name",
				json_object_new_string(data.open_data.file_name));

		json_object* creation_flags = json_object_new_object();
		ret = json_object_object_add(creation_flags, "cloexec",
				json_object_new_boolean(data.open_data.creation.cloexec));
		ret = json_object_object_add(creation_flags, "creat",
				json_object_new_boolean(data.open_data.creation.creat));
		ret = json_object_object_add(creation_flags, "directory",
				json_object_new_boolean(data.open_data.creation.directory));
		ret = json_object_object_add(creation_flags, "excl",
				json_object_new_boolean(data.open_data.creation.excl));
		ret = json_object_object_add(creation_flags, "noctty",
				json_object_new_boolean(data.open_data.creation.noctty));
		ret = json_object_object_add(creation_flags, "nofollow",
				json_object_new_boolean(data.open_data.creation.nofollow));
		ret = json_object_object_add(creation_flags, "tmpfile",
				json_object_new_boolean(data.open_data.creation.tmpfile));
		ret = json_object_object_add(creation_flags, "trunc",
				json_object_new_boolean(data.open_data.creation.trunc));
		ret = json_object_object_add(function_data, "creation_flags",
				creation_flags);

		json_object* status_flags = json_object_new_object();
		ret = json_object_object_add(status_flags, "append",
				json_object_new_boolean(data.open_data.status.append));
		ret = json_object_object_add(status_flags, "append",
				json_object_new_boolean(data.open_data.status.append));
		ret = json_object_object_add(status_flags, "async",
				json_object_new_boolean(data.open_data.status.async));
		ret = json_object_object_add(status_flags, "direct",
				json_object_new_boolean(data.open_data.status.direct));
		ret = json_object_object_add(status_flags, "dsync",
				json_object_new_boolean(data.open_data.status.dsync));
		ret = json_object_object_add(status_flags, "largefile",
				json_object_new_boolean(data.open_data.status.largefile));
		ret = json_object_object_add(status_flags, "ndelay",
				json_object_new_boolean(data.open_data.status.ndelay));
		ret = json_object_object_add(status_flags, "noatime",
				json_object_new_boolean(data.open_data.status.noatime));
		ret = json_object_object_add(status_flags, "nonblock",
				json_object_new_boolean(data.open_data.status.nonblock));
		ret = json_object_object_add(status_flags, "path",
				json_object_new_boolean(data.open_data.status.path));
		ret = json_object_object_add(status_flags, "sync",
				json_object_new_boolean(data.open_data.status.sync));
		ret = json_object_object_add(function_data, "status_flags",
				status_flags);
		break;
	case close_function:
		ret = json_object_object_add(function_data, "function_type",
				json_object_new_string("close_function"));
		ret = json_object_object_add(function_data, "return_value",
				json_object_new_int64(data.close_data.return_value));
		break;
	default:
		ret = json_object_object_add(function_data, "function_type",
				json_object_new_string("not implemented"));
	}
	ret = json_object_object_add(root_object, "function_data", function_data);

	printf("%s\n", json_object_to_json_string(root_object));

	// free allocated memory
	ret = json_object_put(root_object);
	printf("%d\n", ret);
}

void printData() {
	for (struct basic* d = data_buffer; d < pos; d++) {
		struct basic data = *d;
		print_basic(data);
	}
	pos = data_buffer;
}

//void writeData(char *data) {
void writeData(struct basic data) {
	/* write (synchronized) */
	pthread_mutex_lock(&lock);
	if (pos > endpos) {
		printData();
	}
	*pos = data;
	//memcpy(pos, &data, sizeof(data));
	pos++;

	pthread_mutex_unlock(&lock);
}

void cleanup() {
	printData();

	pthread_mutex_destroy(&lock);
}
