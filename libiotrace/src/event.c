#include "libiotrace_config.h"

#include <assert.h>

#ifdef HAVE_UNISTD_H
#  include <unistd.h>
#else
#  error HAVE_UNISTD_H not defined
#endif

#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif

#include <sys/syscall.h>
#include <sys/stat.h>

#ifdef HAVE_PTHREAD_H
#  include <pthread.h>
#endif

#include <stdio.h>
#include <mntent.h>
#include <stdarg.h>
#include <fcntl.h>

#include "os.h"
#include "event.h"

#include "json_include_function.h"

/* defines for exec-functions */
#define MAX_EXEC_ARRAY_LENGTH 1000

/* Buffer */
#define BUFFER_SIZE 1048576 // 1 MB
static char data_buffer[BUFFER_SIZE];
static const char* endpos = data_buffer + BUFFER_SIZE;
static char* pos;
static int count_basic;

// Todo: multiple definition of host_name_max in libiotrace_config.h and here?
#if !defined (HAVE_HOST_NAME_MAX)
static int host_name_max;
#endif

/* Mutex */
static pthread_mutex_t lock;

/* environment variables */
static const char * env_log_name = "IOTRACE_LOG_NAME";
#ifndef IO_LIB_STATIC
static const char * env_ld_preload = "LD_PRELOAD";
#endif

// once per process
static pid_t pid;
static char hostname[HOST_NAME_MAX];
static char log_name[MAXFILENAME];
static char filesystem_log_name[MAXFILENAME];
static char working_dir_log_name[MAXFILENAME];
#ifndef IO_LIB_STATIC
static char ld_preload[MAXFILENAME + sizeof(env_ld_preload)];
static char log_name_env[MAXFILENAME + sizeof(env_log_name)];
#endif
// once per thread
static ATTRIBUTE_THREAD pid_t tid = -1;

#ifndef IO_LIB_STATIC
REAL_DEFINITION_TYPE int REAL_DEFINITION(execve)(const char *filename, char *const argv[], char *const envp[]) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(execv)(const char *path, char *const argv[]) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(execl)(const char *path, const char *arg, ... /* (char  *) NULL */) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(execvp)(const char *file, char *const argv[]) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(execlp)(const char *file, const char *arg, ... /* (char  *) NULL */) REAL_DEFINITION_INIT;
#ifdef HAVE_EXECVPE
REAL_DEFINITION_TYPE int REAL_DEFINITION(execvpe)(const char *file, char *const argv[], char *const envp[]) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(execle)(const char *path, const char *arg, ... /*, (char *) NULL, char * const envp[] */) REAL_DEFINITION_INIT;

REAL_DEFINITION_TYPE void REAL_DEFINITION(_exit)(int status) REAL_DEFINITION_INIT;
#ifdef HAVE_EXIT
REAL_DEFINITION_TYPE void REAL_DEFINITION(_Exit)(int status) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_EXIT_GROUP
REAL_DEFINITION_TYPE void REAL_DEFINITION(exit_group)(int status) REAL_DEFINITION_INIT;
#endif
#endif

#ifndef IO_LIB_STATIC
char event_init_done = 0;
/* Initialize pointers for glibc functions. */
void event_init() {
	if (!event_init_done) {

		DLSYM(execve);
		DLSYM(execv);
		DLSYM(execl);
		DLSYM(execvp);
		DLSYM(execlp);
#ifdef HAVE_EXECVPE
		DLSYM(execvpe);
#endif
		DLSYM(execle);

		DLSYM(_exit);
#ifdef HAVE_EXIT
		DLSYM(_Exit);
#endif
#ifdef HAVE_EXIT_GROUP
		DLSYM(exit_group);
#endif

		event_init_done = 1;
	}
}
#endif

#ifndef IO_LIB_STATIC
void init_wrapper() {
	event_init();

#ifdef WITH_POSIX_IO
	posix_io_init(); // initialize of function pointers is necessary
#endif
#ifdef WITH_POSIX_AIO
	posix_aio_init(); // initialize of function pointers is necessary
#endif
#ifdef WITH_DL_IO
	dl_io_init(); // initialize of function pointers is necessary
#endif
}
#endif

void cleanup() ATTRIBUTE_DESTRUCTOR;

void get_filesystem() {
	FILE * file;
#ifdef HAVE_GETMNTENT_R
	struct mntent filesystem_entry;
	char buf[4 * MAXFILENAME];
#endif
	struct mntent *filesystem_entry_ptr;
	char buf_filesystem[json_struct_max_size_filesystem() + 1]; /* +1 for trailing null character */
	struct filesystem filesystem_data;
	struct stat stat_data;
	char mount_point[MAXFILENAME];
	int fd;
	int ret;

	fd = CALL_REAL_POSIX_SYNC(open)(filesystem_log_name,
	O_WRONLY | O_CREAT | O_EXCL,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (-1 == fd) {
		if (errno == EEXIST) {
			return;
		} else {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: open() returned %d.\n", __func__, fd);
			assert(0);
		}
	}

	file = setmntent("/proc/mounts", "r");
	if (NULL == file) {
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: setmntent() returned NULL with errno=%d.\n",
				__func__, errno);
		assert(0);
	}

#ifdef HAVE_GETMNTENT_R
	while (getmntent_r(file, &filesystem_entry, buf, sizeof(buf))) {
		filesystem_entry_ptr = &filesystem_entry;
#else
		while (filesystem_entry_ptr = getmntent(file)) {
#endif
		ret = strlen(filesystem_entry_ptr->mnt_dir);
		if (MAXFILENAME < ret + 4) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: getmntent() returned mnt_dir too long (%d bytes) for buffer.\n",
					__func__, ret);
			assert(0);
		}
		strcpy(mount_point, filesystem_entry_ptr->mnt_dir);
		// get mounted directory, not the mount point in parent filesystem
		strcpy(mount_point + ret, "/./");
		ret = stat(mount_point, &stat_data);
		if (-1 == ret) {
			filesystem_data.device_id = 0;
		} else {
			filesystem_data.device_id = stat_data.st_dev;
		}
		filesystem_data.name = filesystem_entry_ptr->mnt_fsname;
		filesystem_data.path_prefix = filesystem_entry_ptr->mnt_dir;
		filesystem_data.mount_type = filesystem_entry_ptr->mnt_type;
		filesystem_data.mount_options = filesystem_entry_ptr->mnt_opts;
		filesystem_data.dump_frequency_in_days = filesystem_entry_ptr->mnt_freq;
		filesystem_data.pass_number_on_parallel_fsck =
				filesystem_entry_ptr->mnt_passno;
		json_struct_print_filesystem(buf_filesystem, sizeof(buf_filesystem),
				&filesystem_data);
		ret = dprintf(fd, "%s\n", buf_filesystem); //TODO: CALL_REAL_POSIX_SYNC(dprintf)
		if (0 > ret) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: dprintf() returned %d with errno=%d.\n",
					__func__, ret, errno);
			assert(0);
		}
	}

	endmntent(file);

	CALL_REAL_POSIX_SYNC(close)(fd);
}

void get_file_id(int fd, struct file_id *data) {
	struct stat stat_data;
	int ret;

	if (0 > fd) {
		data->device_id = 0;
		data->inode_nr = 0;
	} else {
		ret = fstat(fd, &stat_data);
		if (0 > ret) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: fstat() returned %d with errno=%d.\n",
					__func__, ret, errno);
			assert(0);
		}

		data->device_id = stat_data.st_dev;
		data->inode_nr = stat_data.st_ino;
	}
}

void get_file_id_by_path(const char *filename, struct file_id *data) {
	struct stat stat_data;
	int ret;

	ret = stat(filename, &stat_data);
	if (0 > ret) {
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: stat() returned %d with errno=%d.\n", __func__,
				ret, errno);
		assert(0);
	}

	data->device_id = stat_data.st_dev;
	data->inode_nr = stat_data.st_ino;
}

void get_directories() {
	char buf_working_dir[json_struct_max_size_working_dir() + 1]; /* +1 for trailing null character */
	struct working_dir working_dir_data;
	char cwd[MAXFILENAME];
	char * ret;
	int fd;
	int ret_int;

	ret = getcwd(cwd, sizeof(cwd));
	if (NULL == ret) {
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: getcwd() returned NULL with errno=%d.\n",
				__func__, errno);
		assert(0);
	}

	working_dir_data.time = gettime();
	working_dir_data.process_id = pid;
	working_dir_data.dir = cwd;

	fd = CALL_REAL_POSIX_SYNC(open)(working_dir_log_name,
	O_WRONLY | O_CREAT | O_APPEND,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (-1 == fd) {
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: open() of file %s returned %d.\n", __func__,
				working_dir_log_name, fd);
		assert(0);
	}

	json_struct_print_working_dir(buf_working_dir, sizeof(buf_working_dir),
			&working_dir_data);
	ret_int = dprintf(fd, "%s\n", buf_working_dir); //TODO: CALL_REAL_POSIX_SYNC(dprintf)
	if (0 > ret_int) {
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: dprintf() returned %d with errno=%d.\n",
				__func__, ret_int, errno);
		assert(0);
	}

	CALL_REAL_POSIX_SYNC(close)(fd);
}

void clear_init() {
	init_done = 0;
	tid = -1;
}

void open_std_fd(int fd) {
	struct basic data;
	struct file_descriptor file_descriptor_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
			file_descriptor_data)
	file_descriptor_data.descriptor = fd;

	data.time_start = gettime();
	data.time_end = gettime();

	data.return_state = ok;

	writeData(&data);
}

void open_std_file(FILE * file) {
	struct basic data;
	struct file_stream file_stream_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream,
			file_stream_data)
	file_stream_data.stream = file;

	data.time_start = gettime();
	data.time_end = gettime();

	data.return_state = ok;

	writeData(&data);
}

char init_done = 0;
void init_basic() {
	char * log;
	int length;

	if (!init_done) {
		pos = data_buffer;
		count_basic = 0;

#if !defined(HAVE_HOST_NAME_MAX)
		host_name_max = sysconf(POSIX_HOST_NAME_MAX);
#endif

		pid = getpid();
		gethostname(hostname, HOST_NAME_MAX);

		log = getenv(env_log_name);
		if (NULL == log) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: function getenv(\"%s\") returned NULL.\n",
					__func__, env_log_name);
			assert(0);
		}
		length = strlen(log);
		if (MAXFILENAME < length + 17) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: getenv() returned %s too long (%d bytes) for buffer.\n",
					__func__, env_log_name, length);
			assert(0);
		}
		strcpy(log_name, log);
		strcpy(filesystem_log_name, log);
		strcpy(working_dir_log_name, log);
		strcpy(log_name + length, "_iotrace.log");
		strcpy(filesystem_log_name + length, "_filesystem.log");
		strcpy(working_dir_log_name + length, "_working_dir.log");

#ifndef IO_LIB_STATIC
		strcpy(log_name_env, env_log_name);
		strcpy(log_name_env + length, "=");
		strcpy(log_name_env + length + 1, log);

		log = getenv(env_ld_preload);
		if (NULL == log) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: function getenv(\"%s\") returned NULL.\n",
					__func__, env_ld_preload);
			assert(0);
		}
		length = strlen(env_ld_preload);
		strcpy(ld_preload, env_ld_preload);
		strcpy(ld_preload + length, "=");
		strcpy(ld_preload + length + 1, log);
#endif

		pthread_mutex_init(&lock, NULL);

		pthread_atfork(NULL, NULL, clear_init);

#if !defined(IO_LIB_STATIC)
		init_wrapper();
#endif

		get_filesystem();
		get_directories();

#ifdef WITH_STD_IO
		open_std_fd(STDIN_FILENO);
		open_std_fd(STDOUT_FILENO);
		open_std_fd(STDERR_FILENO);

		open_std_file(stdin);
		open_std_file(stdout);
		open_std_file(stderr);
#endif

		init_done = 1;
	}
}

void get_basic(struct basic *data) {
	// lock write on tid
	if (tid == -1) {
		tid = iotrace_gettid();
	}

	data->process_id = pid;
	data->thread_id = tid;

	data->hostname = hostname;
}

inline u_int64_t gettime(void) {
	struct timespec t;
	u_int64_t time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	time = t.tv_sec * (1000 * 1000 * 1000) + t.tv_nsec;
	return time;
}

void printData() {
	struct basic *data;
	int ret;
	char buf[json_struct_max_size_basic() + 1]; /* +1 for trailing null character */
	int fd;
	pos = data_buffer;

	fd = CALL_REAL_POSIX_SYNC(open)(log_name, O_WRONLY | O_CREAT | O_APPEND,
	S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (-1 == fd) {
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: open() of file %s returned %d.\n", __func__,
				log_name, fd);
		assert(0);
	}

	for (int i = 0; i < count_basic; i++) {
		data = (struct basic *) ((void *) pos);

		ret = json_struct_print_basic(buf, sizeof(buf), data);
		ret = dprintf(fd, "%s\n", buf); //TODO: CALL_REAL_POSIX_SYNC(dprintf)
		if (0 > ret) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: dprintf() returned %d.\n", __func__, ret);
			assert(0);
		}
		ret = json_struct_sizeof_basic(data);

		pos += ret;
	}

	CALL_REAL_POSIX_SYNC(close)(fd);

	pos = data_buffer;
	count_basic = 0;
}

void writeData(struct basic *data) {
	/* write (synchronized) */
	pthread_mutex_lock(&lock);

	int length = json_struct_sizeof_basic(data);

	if (pos + length > endpos) {
		printData();
	}
	if (pos + length > endpos) {
		// ToDo: solve circular dependency
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: buffer (%ld bytes) not big enough for even one struct basic (%d bytes).\n",
				__func__, sizeof(data_buffer), length);
		assert(0);
	}

	pos = (void*) json_struct_copy_basic((void*) pos, data);
	count_basic++;

	pthread_mutex_unlock(&lock);
}

void cleanup() {
	pthread_mutex_lock(&lock);
	printData();
	pthread_mutex_unlock(&lock);

	pthread_mutex_destroy(&lock);
}

#ifndef IO_LIB_STATIC
void check_ld_preload(char * env[], char * const envp[], const char * func) {
	int env_element;
	char has_ld_preload = 0;
	char envp_null = 0;

	for (env_element = 0; env_element < MAX_EXEC_ARRAY_LENGTH; env_element++) {
		env[env_element] = envp[env_element];

		if (NULL != envp[env_element]) {
			if (strcmp(ld_preload, envp[env_element]) == 0) {
				has_ld_preload = 1;
			}
		} else {
			envp_null = 1;
			break;
		}
	}

	if (!envp_null) {
		CALL_REAL_POSIX_SYNC(fprintf)(stderr,
				"In function %s: envp[] has more elements then buffer (%d).\n",
				func,
				MAX_EXEC_ARRAY_LENGTH);
		assert(0);
	}

	if (!has_ld_preload) {
		if (MAX_EXEC_ARRAY_LENGTH <= env_element + 2) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: envp[] with added libiotrace-variables has more elements then buffer (%d).\n",
					func,
					MAX_EXEC_ARRAY_LENGTH);
			assert(0);
		}
		env[env_element] = &ld_preload[0];
		env[++env_element] = &log_name_env[0];
		env[++env_element] = NULL;
	}
}
#endif

int WRAP(execve)(const char *filename, char * const argv[], char * const envp[]) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
#ifndef IO_LIB_STATIC
	char * env[MAX_EXEC_ARRAY_LENGTH];
	check_ld_preload(env, envp, __func__);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execve, filename, argv, env)
#else
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execve, filename, argv, envp)
#endif

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(execv)(const char *path, char * const argv[]) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execv, path, argv)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(execl)(const char *path, const char *arg, ... /* (char  *) NULL */) {
	int ret;
	struct basic data;
	char * argv[MAX_EXEC_ARRAY_LENGTH];
	int count = 0;
	char * element;
	va_list ap;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	va_start(ap, arg);
	element = (char *) ((void *) arg);
	while (NULL != element) {
		if (count >= MAX_EXEC_ARRAY_LENGTH - 1) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: buffer (%d elements) not big enough for argument array.\n",
					__func__, MAX_EXEC_ARRAY_LENGTH);
			assert(0);
		}
		argv[count] = element;
		count++;

		element = va_arg(ap, char *);
	}
	argv[count] = element;
	va_end(ap);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execv, path, argv)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(execvp)(const char *file, char * const argv[]) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvp, file, argv)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

int WRAP(execlp)(const char *file, const char *arg, ... /* (char  *) NULL */) {
	int ret;
	struct basic data;
	char * argv[MAX_EXEC_ARRAY_LENGTH];
	int count = 0;
	char * element;
	va_list ap;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	va_start(ap, arg);
	element = (char *) ((void *) arg);
	while (NULL != element) {
		if (count >= MAX_EXEC_ARRAY_LENGTH - 1) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: buffer (%d elements) not big enough for argument array.\n",
					__func__, MAX_EXEC_ARRAY_LENGTH);
			assert(0);
		}
		argv[count] = element;
		count++;

		element = va_arg(ap, char *);
	}
	argv[count] = element;
	va_end(ap);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvp, file, argv)

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

#ifdef HAVE_EXECVPE
int WRAP(execvpe)(const char *file, char * const argv[], char * const envp[]) {
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
#ifndef IO_LIB_STATIC
	char * env[MAX_EXEC_ARRAY_LENGTH];
	check_ld_preload(env, envp, __func__);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, file, argv, env)
#else
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, file, argv, envp)
#endif

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}
#endif

int WRAP(execle)(const char *path, const char *arg,
		... /*, (char *) NULL, char * const envp[] */) {
#ifndef HAVE_EXECVPE
	CALL_REAL_POSIX_SYNC(fprintf)(stderr,
			"In function %s: wrapper needs function execvpe() to work properly.\n",
			__func__);
	assert(0);
#endif
	int ret;
	struct basic data;
	char * argv[MAX_EXEC_ARRAY_LENGTH];
	int count = 0;
	char * element;
	char ** envp;
	va_list ap;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	va_start(ap, arg);
	element = (char *) ((void *) arg);
	while (NULL != element) {
		if (count >= MAX_EXEC_ARRAY_LENGTH - 1) {
			CALL_REAL_POSIX_SYNC(fprintf)(stderr,
					"In function %s: buffer (%d elements) not big enough for argument array.\n",
					__func__, MAX_EXEC_ARRAY_LENGTH);
			assert(0);
		}
		argv[count] = element;
		count++;

		element = va_arg(ap, char *);
	}
	argv[count] = element;
	envp = va_arg(ap, char **);
	va_end(ap);

#ifndef IO_LIB_STATIC
	char * env[MAX_EXEC_ARRAY_LENGTH];
	check_ld_preload(env, envp, __func__);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, path, argv, env)
#else
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, path, argv, envp)
#endif

	if (-1 == ret) {
		data.return_state = error;
	} else {
		data.return_state = ok;
	}

	WRAP_END(data)
	return ret;
}

void WRAP(_exit)(int status) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_NO_RETURN(data, _exit, status)
}

#ifdef HAVE_EXIT
void WRAP(_Exit)(int status) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_NO_RETURN(data, _Exit, status)
}
#endif

#ifdef HAVE_EXIT_GROUP
void WRAP(exit_group)(int status) {
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_NO_RETURN(data, exit_group, status)
}
#endif
