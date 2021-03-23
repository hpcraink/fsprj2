#include "libiotrace_config.h"

//##################

#include <sys/types.h>
#include <sys/socket.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <limits.h>
#include <ctype.h>
#include <inttypes.h>
#include <sys/time.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s)          \
	CALL_REAL_POSIX_SYNC(close) \
	(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#include <stdio.h>
#include <string.h>

#include <sys/ioctl.h>
#include <net/if.h>

//##################

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#else
#error HAVE_UNISTD_H not defined
#endif

#ifdef HAVE_STDLIB_H
#include <stdlib.h>
#endif

#include <sys/syscall.h>
#include <sys/stat.h>

#ifdef HAVE_PTHREAD_H
#include <pthread.h>
#endif

#include <stdio.h>
#ifdef HAVE_MNTENT_H
#  include <mntent.h>
#endif
#include <stdarg.h>
#include <fcntl.h>

#include <execinfo.h>

#include "error.h"

#include "llhttp/llhttp.h"

#include "os.h"
#include "event.h"

#include "libiotrace.h"
#include "libiotrace_include_function.h"

#include "wrapper_name.h"

/* defines for exec-functions */
#ifndef MAX_EXEC_ARRAY_LENGTH
#define MAX_EXEC_ARRAY_LENGTH 1000
#endif

/* defines for influxdb connection */
#ifndef MAX_INFLUX_TOKEN
#define MAX_INFLUX_TOKEN 200
#endif

#ifndef MAX_INFLUX_BUCKET
#define MAX_INFLUX_BUCKET 200
#endif

#ifndef MAX_INFLUX_ORGANIZATION
#define MAX_INFLUX_ORGANIZATION 200
#endif

#ifndef MAX_DATABSE_IP
#define MAX_DATABASE_IP 200
#endif

#ifndef MAX_DATABSE_PORT
#define MAX_DATABASE_PORT 200
#endif

/* defines for control connection */
#ifndef PORT_RANGE_MIN
#define PORT_RANGE_MIN 50000
#endif

#ifndef PORT_RANGE_MAX
#define PORT_RANGE_MAX 60000
#endif

/* defines for all connections */
#ifndef SELECT_TIMEOUT_SECONDS
#define SELECT_TIMEOUT_SECONDS 1
#endif

/* flags and values to control logging */
#ifdef LOGGING
static ATTRIBUTE_THREAD char no_logging = 0;
#else
static ATTRIBUTE_THREAD char no_logging = 1;
#endif
#ifdef SENDING
static ATTRIBUTE_THREAD char no_sending = 0;
#else
static ATTRIBUTE_THREAD char no_sending = 1;
#endif
#ifdef STACKTRACE_DEPTH
static ATTRIBUTE_THREAD int stacktrace_depth = STACKTRACE_DEPTH;
#else
static ATTRIBUTE_THREAD int stacktrace_depth = 0;
#endif
#ifdef STACKTRACE_PTR
static ATTRIBUTE_THREAD char stacktrace_ptr = STACKTRACE_PTR;
#else
static ATTRIBUTE_THREAD char stacktrace_ptr = 0;
#endif
#ifdef STACKTRACE_SYMBOL
static ATTRIBUTE_THREAD char stacktrace_symbol = STACKTRACE_SYMBOL;
#else
static ATTRIBUTE_THREAD char stacktrace_symbol = 0;
#endif

/* log file write Buffer */
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1048576 // 1 MB
#endif
static char data_buffer[BUFFER_SIZE];
static const char *endpos = data_buffer + BUFFER_SIZE;
static char *pos;
static int count_basic;

#if !defined(HAVE_HOST_NAME_MAX)
int host_name_max;
#endif

typedef struct libiotrace_sockets {
	SOCKET socket;
	llhttp_t parser;
} libiotrace_socket;

/* Mutex */
static pthread_mutex_t lock;
static pthread_mutex_t socket_lock;

/* environment variables */
static const char *env_log_name = "IOTRACE_LOG_NAME";
static const char *env_influx_token = "IOTRACE_INFLUX_TOKEN";
static const char *env_influx_organization = "IOTRACE_INFLUX_ORGANIZATION";
static const char *env_influx_bucket = "IOTRACE_INFLUX_BUCKET";
static const char *env_database_ip = "IOTRACE_DATABASE_IP";
static const char *env_database_port = "IOTRACE_DATABASE_PORT";
static const char *env_wrapper_whitelist = "IOTRACE_WHITELIST";
#ifndef IO_LIB_STATIC
static const char *env_ld_preload = "LD_PRELOAD";
#endif

// once per process
static pid_t pid;
static char *hostname;
static char log_name[MAXFILENAME];
static char filesystem_log_name[MAXFILENAME];
static char working_dir_log_name[MAXFILENAME];
static char control_log_name[MAXFILENAME];
static char influx_token[MAX_INFLUX_TOKEN];
static int influx_token_len;
static char influx_organization[MAX_INFLUX_ORGANIZATION];
static int influx_organization_len;
static char influx_bucket[MAX_INFLUX_BUCKET];
static int influx_bucket_len;
static char database_ip[MAX_DATABASE_IP];
static char database_port[MAX_DATABASE_PORT];
static char whitelist[MAXFILENAME];
static char event_cleanup_done = 0;
#ifndef IO_LIB_STATIC
static char ld_preload[MAXFILENAME + sizeof(env_ld_preload)];
static char log_name_env[MAXFILENAME + sizeof(env_log_name)];
static char database_port_env[MAX_DATABASE_PORT + sizeof(env_database_port)];
static char database_ip_env[MAX_DATABASE_IP + sizeof(env_database_ip)];
static char influx_token_env[MAX_INFLUX_TOKEN + sizeof(env_influx_token)];
static char influx_organization_env[MAX_INFLUX_ORGANIZATION + sizeof(env_influx_organization)];
static char influx_bucket_env[MAX_INFLUX_BUCKET + sizeof(env_influx_bucket)];
static char whitelist_env[MAXFILENAME + sizeof(env_wrapper_whitelist)];
static char has_whitelist;
#endif
#ifndef REALTIME
static u_int64_t system_start_time;
#endif
static libiotrace_socket **recv_sockets = NULL;
static int recv_sockets_len = 0;
static libiotrace_socket **open_control_sockets = NULL;
static int open_control_sockets_len = 0;
static SOCKET socket_control;
static llhttp_settings_t settings;
struct wrapper_status active_wrapper_status;

// once per thread
static ATTRIBUTE_THREAD pid_t tid = -1;
static ATTRIBUTE_THREAD SOCKET socket_peer;

void cleanup() ATTRIBUTE_DESTRUCTOR;
void *communication_thread(void *arg);

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

libiotrace_socket *create_libiotrace_socket(SOCKET s, llhttp_type_t type) {
	libiotrace_socket *socket = malloc(sizeof(libiotrace_socket));
	if (NULL == socket)
	{
		LIBIOTRACE_ERROR("malloc failed, errno=%d", errno);
	}

	socket->socket = s;

	llhttp_init(&(socket->parser), type, &settings);

	return socket;
}

/**
 * Save a socket in a global array.
 *
 * Adds "socket" to "array". If "array" is NULL and "len" points to
 * the value 0 a new array with length 1 is allocated. Else the
 * existing array is increased by 1. In both cases "*len" is
 * incremented by 1 and the "socket" is added to "array" as a new
 * element.
 *
 * @param[in] socket    The socket to save (a pointer to
 *                      "libiotrace_socket")
 * @param[in] lock      Mutex used to make the array manipulation
 *                      thread safe, or NULL if no concurrent access
 *                      is possible
 * @param[in,out] len   Pointer to count of sockets in "array". Is
 *                      incremented by 1 after function returns.
 * @param[in,out] array Pointer to dynamically allocated array of
 *                      sockets with length "*len", or NULL if array
 *                      should be created during function call.
 *                      After function call "array" is increased by
 *                      one element. This new element holds the
 *                      value of "socket".
 */
void save_socket(libiotrace_socket *socket, pthread_mutex_t *lock, int *len, libiotrace_socket ***array)
{
	void *ret;

	if (NULL != lock)
	{
		pthread_mutex_lock(lock);
	}

	(*len)++;
	ret = realloc(*array, sizeof(libiotrace_socket*) * (*len));
	if (NULL == ret)
	{
		free(*array);
		LIBIOTRACE_ERROR("realloc() failed");
	}
	*array = ret;
	(*array)[*len - 1] = socket;

	if (NULL != lock)
	{
		pthread_mutex_unlock(lock);
	}
}

/**
 * Delete a socket from a global array.
 *
 * Removes "socket" from "array". If "socket" was not found in
 * "array" the array is left untouched. Deletes only the pointer
 * to "libiotrace_socket" from the array. The "libiotrace_socket"
 * itself is left untouched.
 *
 * @param[in] socket    The socket to delete
 * @param[in] lock      Mutex used to make the array manipulation
 *                      thread safe, or NULL if no concurrent access
 *                      is possible
 * @param[in,out] len   Pointer to count of sockets in "array". Is
 *                      decremented by 1 after function returns.
 * @param[in,out] array Pointer to dynamically allocated array of
 *                      sockets with length "*len".
 *                      After function call "array" is decreased by
 *                      one element (if "socket" was in "array").
 */
void delete_socket(SOCKET socket, pthread_mutex_t *lock, int *len, libiotrace_socket ***array)
{
	void *ret;
	int i;

	if (NULL != lock)
	{
		pthread_mutex_lock(lock);
	}

	(*len)--;
	if ((*array)[*len]->socket == socket)
	{
		//Delete last element if last element is current socket
		ret = realloc(*array, sizeof(libiotrace_socket*) * (*len));
		if (*len == 0)
		{
			if (ret != NULL)
			{
				free(ret);
			}
			*array = NULL;
		}
		else if (NULL == ret)
		{
			free(*array);
			LIBIOTRACE_ERROR("realloc() failed");
		}
		else
		{
			*array = ret;
		}
	}
	else
	{
		for (i = 0; i < *len; i++)
		{
			if ((*array)[i]->socket == socket)
			{
				(*array)[i] = (*array)[*len];
				break;
			}
		}
		if (i >= *len)
		{
			// socket not found
			return;
		}
		ret = realloc(*array, sizeof(libiotrace_socket*) * (*len));
		if (*len == 0)
		{
			if (ret != NULL)
			{
				free(ret);
			}
			*array = NULL;
		}
		else if (NULL == ret)
		{
			free(*array);
			LIBIOTRACE_ERROR("realloc() failed");
		}
		else
		{
			*array = ret;
		}
	}

	if (NULL != lock)
	{
		pthread_mutex_unlock(lock);
	}
}

#ifndef IO_LIB_STATIC
static char event_init_done = 0;

/**
 *  Initialize pointers for glibc functions.
 *
 *  Wrappers use them to call the real functions.
 */
void event_init()
{
	if (!event_init_done)
	{

#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_DLSYM
#include "event_wrapper.h"

		event_init_done = 1;
	}
}
#endif

/**
 * Changes the status of the wrapper for a function
 * with name equals "line" to "toggle"
 *
 * An active wrapper logs/sends collected data an inactive
 * wrapper doesn't.
 * Status of a wrapper (active/inactive) is saved per process.
 * Changing the status affects all threads in the process.
 *
 * @param[in] line   "\0" terminated name of the wrapped function.
 * @param[in] toggle 1 for set wrapper active
 *                   0 for set wrapper inactive
 */
void toggle_wrapper(const char *line, const char toggle)
{
	char ret = 1;

	if (!strcmp(line, ""))
	{
		ret = 0;
	}
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_SET_VARIABLE
#include "event_wrapper.h"
	else
	{
		ret = 0;
	}
#ifdef WITH_POSIX_IO
	if (!ret)
	{
		ret = toggle_posix_wrapper(line, toggle);
	}
#endif
#ifdef WITH_MPI_IO
	if (!ret)
	{
		ret = toggle_mpi_wrapper(line, toggle);
	}
#endif
#ifdef WITH_POSIX_AIO
	if (!ret)
	{
		ret = toggle_posix_aio_wrapper(line, toggle);
	}
#endif
#ifdef WITH_DL_IO
	if (!ret)
	{
		ret = toggle_dl_wrapper(line, toggle);
	}
#endif
}

#ifndef IO_LIB_STATIC
/**
 * Initializes all needed function pointers.
 *
 * Each wrapper needs a function pointer to the
 * wrapped function. "dlsym" is used to get
 * these pointers.
 */
void init_wrapper()
{
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

/**
 * Creates a new socket for sending data to influxdb.
 *
 * Is called once per thread. Creates a new socket and stores
 * it in the thread local storage "socket_peer". The new
 * socket is additionally stored in the array "recv_sockets".
 * "socket_peer" is used inside each wrapper to send data
 * to influxdb. Because each thread has it's own socket
 * inside "socket_peer" no synchronization between calls of
 * send function is needed. The responses from influxdb are
 * read inside "communication_thread" from all sockets
 * stored in the array "recv_sockets".
 */
void prepare_socket()
{
	/* Call of getaddrinfo calls other posix functions. These other
	 * functions could be wrapped. Call of a wrapper out of getaddrinfo
	 * is done during initialization of a thread (because
	 * prepare_socket is only called during initialization of the
	 * thread). Because at this time the thread is not initialized
	 * the call of the wrapper calls prepare_socket. Now we have a
	 * endless recursion of function calls that exceeds the stack size.
	 * => getaddrinfo should only be called if POSIX wrapper are not
	 *    build */
#ifdef WITH_POSIX_IO
	socket_peer = CALL_REAL_POSIX_SYNC(socket)(AF_INET, SOCK_STREAM, 0);
#else
	//Configure remote address for socket
	struct addrinfo hints;
	memset(&hints, 0, sizeof(hints));
	hints.ai_socktype = SOCK_STREAM;
	struct addrinfo *peer_address;
	if (getaddrinfo(database_ip, database_port, &hints, &peer_address))
	{
		LIBIOTRACE_WARN("getaddrinfo() failed. (%d)", GETSOCKETERRNO());
		return;
	}
	socket_peer = CALL_REAL_POSIX_SYNC(socket)(peer_address->ai_family,
											   peer_address->ai_socktype, peer_address->ai_protocol);

#endif

	if (!ISVALIDSOCKET(socket_peer))
	{
		LIBIOTRACE_WARN("socket() failed. (%d)", GETSOCKETERRNO());
		return;
	}

	//Set socket option TCP_NODELAY
	// int option = 0;
	// if (setsockopt(socket_peer, IPPROTO_TCP, TCP_NODELAY, (void *)&option, sizeof(option)))
	// {
	// 	CALL_REAL_POSIX_SYNC(fprintf)
	// 	(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
	// 	return;
	// }

	//Set socket option REUSEADDR
	// int option2 = 0;
	// if (setsockopt(socket_peer, SOL_SOCKET, SO_REUSEADDR, (void *)&option2, sizeof(option2)))
	// {
	// 	CALL_REAL_POSIX_SYNC(fprintf)
	// 	(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
	// 	return;
	// }

#ifdef WITH_POSIX_IO
	//MAP PORT FROM ENV TO SA_DATA(IPv4)
	unsigned short database_port_short = (unsigned short)atoi(database_port);
	unsigned char *database_port_short_p = (unsigned char *)&database_port_short;

	//MAP IP FROM ENV TO SA_DATA(IPv4)
	char *str = database_ip, *str2;
	unsigned char database_ip_char[4] = {0};
	size_t index = 0;

	str2 = str;
	while (*str)
	{
		if (isdigit((unsigned char)*str))
		{
			database_ip_char[index] *= 10;
			database_ip_char[index] += *str - '0';
		}
		else
		{
			index++;
		}
		str++;
	}

	struct sockaddr own_ai_addr;
	own_ai_addr.sa_data[0] = database_port_short_p[1];
	own_ai_addr.sa_data[1] = database_port_short_p[0];
	own_ai_addr.sa_data[2] = database_ip_char[0];
	own_ai_addr.sa_data[3] = database_ip_char[1];
	own_ai_addr.sa_data[4] = database_ip_char[2];
	own_ai_addr.sa_data[5] = database_ip_char[3];
	own_ai_addr.sa_data[6] = 0x00;
	own_ai_addr.sa_data[7] = 0x00;
	own_ai_addr.sa_data[8] = 0x00;
	own_ai_addr.sa_data[9] = 0x00;
	own_ai_addr.sa_data[10] = 0x00;
	own_ai_addr.sa_data[11] = 0x00;
	own_ai_addr.sa_data[12] = 0x00;
	own_ai_addr.sa_data[13] = 0x00;
	own_ai_addr.sa_family = 2;

	if (CALL_REAL_POSIX_SYNC(connect)(socket_peer, &own_ai_addr, 16))
	{
		LIBIOTRACE_WARN("connect() failed. (%d)", GETSOCKETERRNO());
		return;
	}
#else
	if (connect(socket_peer,
				peer_address->ai_addr, peer_address->ai_addrlen))
	{
		LIBIOTRACE_WARN("connect() failed. (%d)", GETSOCKETERRNO());
		return;
	}
	freeaddrinfo(peer_address);
#endif

	// save socket globally to create thread that listens to / reads from all sockets
	libiotrace_socket *socket = create_libiotrace_socket(socket_peer, HTTP_RESPONSE);
	save_socket(socket, &socket_lock, &recv_sockets_len, &recv_sockets);
}

void libiotrace_start_log()
{
	no_logging = 0;
}

void libiotrace_end_log()
{
	no_logging = 1;
}

void libiotrace_start_send()
{
	no_sending = 0;
}

void libiotrace_end_send()
{
	no_sending = 1;
}

void libiotrace_start_stacktrace_ptr()
{
	stacktrace_ptr = 1;
}

void libiotrace_end_stacktrace_ptr()
{
	stacktrace_ptr = 0;
}

void libiotrace_start_stacktrace_symbol()
{
	stacktrace_symbol = 1;
}

void libiotrace_end_stacktrace_symbol()
{
	stacktrace_symbol = 0;
}

void libiotrace_set_stacktrace_depth(int depth)
{
	stacktrace_depth = depth;
}

int libiotrace_get_stacktrace_depth()
{
	return stacktrace_depth;
}

void libiotrace_set_wrapper_active(const char *wrapper) {
	toggle_wrapper(wrapper, 1);
}

void libiotrace_set_wrapper_inactive(const char *wrapper) {
	toggle_wrapper(wrapper, 0);
}

/**
 * Prints the filesystem to a file.
 *
 * The file is given in the global variable #filesystem_log_name which is
 * set from environment variable #env_log_name. Each mount point is printed
 * on a new line. The printed line shows the device, the mount point, the
 * file-system type and the mount options, the dump frequency in days and
 * the mount passno as a json object.
 */
#if 0 // RAY MacOS
void print_filesystem()
{
	FILE *file;
#ifdef HAVE_GETMNTENT_R
	struct mntent filesystem_entry;
	char buf[4 * MAXFILENAME];
#endif
	struct mntent *filesystem_entry_ptr;
	char buf_filesystem[libiotrace_struct_max_size_filesystem() + 1]; /* +1 for trailing null character */
	struct filesystem filesystem_data;
	struct stat stat_data;
	char mount_point[MAXFILENAME];
	int fd;
	int ret;

	fd = CALL_REAL_POSIX_SYNC(open)(filesystem_log_name,
									O_WRONLY | O_CREAT | O_EXCL,
									S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (-1 == fd)
	{
		if (errno == EEXIST)
		{
			return;
		}
		else
		{
			LIBIOTRACE_ERROR("open() returned %d", fd);
		}
	}

	file = setmntent("/proc/mounts", "r");
	if (NULL == file)
	{
		LIBIOTRACE_ERROR("setmntent() returned NULL with errno=%d", errno);
	}

#ifdef HAVE_GETMNTENT_R
	while (getmntent_r(file, &filesystem_entry, buf, sizeof(buf)))
	{
		filesystem_entry_ptr = &filesystem_entry;
#else
	while (filesystem_entry_ptr = getmntent(file))
	{
#endif
		ret = strlen(filesystem_entry_ptr->mnt_dir);
		if (MAXFILENAME < ret + 4)
		{
			LIBIOTRACE_ERROR("getmntent() returned mnt_dir too long (%d bytes) for buffer", ret);
		}
		strcpy(mount_point, filesystem_entry_ptr->mnt_dir);
		// get mounted directory, not the mount point in parent filesystem
		strcpy(mount_point + ret, "/./");
		ret = stat(mount_point, &stat_data);
		if (-1 == ret)
		{
			filesystem_data.device_id = 0;
		}
		else
		{
			filesystem_data.device_id = stat_data.st_dev;
		}
		filesystem_data.name = filesystem_entry_ptr->mnt_fsname;
		filesystem_data.path_prefix = filesystem_entry_ptr->mnt_dir;
		filesystem_data.mount_type = filesystem_entry_ptr->mnt_type;
		filesystem_data.mount_options = filesystem_entry_ptr->mnt_opts;
		filesystem_data.dump_frequency_in_days = filesystem_entry_ptr->mnt_freq;
		filesystem_data.pass_number_on_parallel_fsck =
			filesystem_entry_ptr->mnt_passno;
		libiotrace_struct_print_filesystem(buf_filesystem, sizeof(buf_filesystem),
									 &filesystem_data);
		ret = dprintf(fd, "%s" LINE_BREAK, buf_filesystem);
		if (0 > ret)
		{
			LIBIOTRACE_ERROR("dprintf() returned %d with errno=%d", ret, errno);
		}
	}

	endmntent(file);

	CALL_REAL_POSIX_SYNC(close)
	(fd);
}
#endif

/**
 * Get "device_id" and "inode_nr" for given file descriptor "fd".
 *
 * @param[in]  filename File descriptor of a file.
 * @param[out] data     Pointer to a struct file_id. Function fills
 *                      "device_id" and "inode_nr" of "fd" into
 *                      this struct.
 */
void get_file_id(int fd, struct file_id *data)
{
	struct stat stat_data;
	int ret;

	if (0 > fd)
	{
		data->device_id = 0;
		data->inode_nr = 0;
	}
	else
	{
		ret = fstat(fd, &stat_data);
		if (0 > ret)
		{
			LIBIOTRACE_ERROR("fstat() returned %d with errno=%d", ret, errno);
		}

		data->device_id = stat_data.st_dev;
		data->inode_nr = stat_data.st_ino;
	}
}

/**
 * Get "device_id" and "inode_nr" for given path "filename".
 *
 * @param[in]  filename Pointer to a "\0" terminated path to a file.
 * @param[out] data     Pointer to a struct file_id. Function fills
 *                      "device_id" and "inode_nr" of "filename"
 *                      into this struct.
 */
void get_file_id_by_path(const char *filename, struct file_id *data)
{
	struct stat stat_data;
	int ret;

	ret = stat(filename, &stat_data);
	if (0 > ret)
	{
		LIBIOTRACE_ERROR("stat() returned %d with errno=%d", ret, errno);
	}

	data->device_id = stat_data.st_dev;
	data->inode_nr = stat_data.st_ino;
}

/**
 * Prints the working dir of the actual process to a file.
 *
 * The file is given in the global variable #working_dir_log_name which is
 * set from environment variable #env_log_name. The working dir is printed
 * on a new line. The printed line shows the working dir, a timestamp, the
 * hostname and the process id as a json object.
 */
void print_working_directory()
{
	char buf_working_dir[libiotrace_struct_max_size_working_dir() + 1]; /* +1 for trailing null character */
	struct working_dir working_dir_data;
	char cwd[MAXFILENAME];
	char *ret;
	int fd;
	int ret_int;

	ret = getcwd(cwd, sizeof(cwd));
	if (NULL == ret)
	{
		LIBIOTRACE_ERROR("getcwd() returned NULL with errno=%d", errno);
	}

	working_dir_data.time = gettime();
	working_dir_data.hostname = hostname;
	working_dir_data.process_id = pid;
	working_dir_data.dir = cwd;

	fd = CALL_REAL_POSIX_SYNC(open)(working_dir_log_name,
									O_WRONLY | O_CREAT | O_APPEND,
									S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (-1 == fd)
	{
		LIBIOTRACE_ERROR("open() of file %s returned %d", working_dir_log_name, fd);
	}

	libiotrace_struct_print_working_dir(buf_working_dir, sizeof(buf_working_dir),
								  &working_dir_data);
	ret_int = dprintf(fd, "%s" LINE_BREAK, buf_working_dir);
	if (0 > ret_int)
	{
		LIBIOTRACE_ERROR("dprintf() returned %d with errno=%d", ret_int, errno);
	}

	CALL_REAL_POSIX_SYNC(close)
	(fd);
}

/**
 * Reset values in forked process before forked process starts.
 *
 * If a process forks another process the new process is a copy of the old
 * process. The copy inherits all values from the old process. Some values
 * are only valid for the old process. These values must be reset.
 */
void reset_values_in_forked_process()
{
	init_done = 0;
	tid = -1;
	recv_sockets = NULL;
	recv_sockets_len = 0;
	open_control_sockets = NULL;
	open_control_sockets_len = 0;
}

/**
 * Adds an simulated wrapper call for opening "fd" to the central buffer.
 *
 * Is used for STDIN_FILENO, STDOUT_FILENO and STDERR_FILENO. These three
 * file descriptors are already open as the process is started. The
 * simulated wrapper call enables handling of these descriptors without
 * additional logic during reading of the log file.
 *
 * @param[in] fd File descriptor to add simulated open for.
 */
void open_std_fd(int fd)
{
	struct basic data;
	struct file_descriptor file_descriptor_data;

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	file_descriptor_data.descriptor = fd;

	data.time_start = gettime();
	data.time_end = data.time_start;
#ifdef LOG_WRAPPER_TIME
	data.wrapper.time_start = data.time_start;
	//data.wrapper.time_end = 0;
#endif

	data.return_state = ok;
	data.return_state_detail = NULL;

	write_into_buffer(&data);
	WRAP_FREE(&data)
}

/**
 * Adds an simulated wrapper call for opening "file" to the central buffer.
 *
 * Is used for stdin, stdout and stderr. These three file streams
 * are already open as the process is started. The simulated wrapper
 * call enables handling of these streams without additional logic
 * during reading of the log file.
 *
 * @param[in] file File stream to add simulated open for.
 */
void open_std_file(FILE *file)
{
	struct basic data;
	struct file_stream file_stream_data;

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)
	file_stream_data.stream = file;

	data.time_start = gettime();
	data.time_end = data.time_start;
#ifdef LOG_WRAPPER_TIME
	data.wrapper.time_start = data.time_start;
	//data.wrapper.time_end = 0;
#endif

	data.return_state = ok;
	data.return_state_detail = NULL;

	write_into_buffer(&data);
	WRAP_FREE(&data)
}

void init_on_load() ATTRIBUTE_CONSTRUCTOR;

/**
 * Calls "init_process" during execution of "ctor" section.
 *
 * Guarantees that "init_process" is called before main() of
 * the observed program is started.
 */
void init_on_load()
{
#ifdef LOG_WRAPPER_TIME
	struct basic data;
	data.time_start = 0;
	data.time_end = 0;
	data.return_state = ok;
	data.return_state_detail = NULL;
#endif

	WRAPPER_TIME_START(data)

	init_process();

#ifdef LOG_WRAPPER_TIME
	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)
#endif

	WRAPPER_TIME_END(data);

#ifdef LOG_WRAPPER_TIME
	write_into_buffer(&data);
	WRAP_FREE(&data)
#endif
}

/**
 * Sends a "message" to the "socket".
 *
 * Returns if the whole "message" is send to "socket".
 * Does not wait for or check responses.
 *
 * @param[in] message "\0" terminated message to send.
 * @param[in] socket  Socket to send to.
 */
void send_data(const char *message, SOCKET socket)
{
	size_t bytes_to_send = strlen(message);
	const char *message_to_send = message;

	while (bytes_to_send > 0)
	{
		int bytes_sent = send(socket, message_to_send, bytes_to_send, 0);

		if (-1 == bytes_sent)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				LIBIOTRACE_WARN("Send buffer is full. Please increase your limit.");
			}
			else
			{
				LIBIOTRACE_ERROR("send() returned %d, errno: %d", bytes_sent, errno);
			}
		}
		else
		{
			if (bytes_sent < bytes_to_send)
			{
				bytes_to_send -= bytes_sent;
				message_to_send += bytes_sent;
			}
			else
			{
				bytes_to_send = 0;
			}
		}
	}
}

/**
 * Callback for each http response from influxdb
 *
 * Only checks if status was 204.
 *
 * @param[in] parser A pointer to the parser which has called
 *                   this callback. Gives access to the parser
 *                   state (like method of the request).
 * @param[in] at     Start pointer of the parsed status as a char
 *                   array. The array is not terminated by "\0"
 *                   (see "length").
 * @param[in] length Count of chars in the status (in the array
 *                   given by "at").
 *
 * @return Error state of the callback (not used; gives "0" back).
 */
int url_callback_responses(llhttp_t *parser, const char *at, size_t length) {
	if (parser->status_code != 204) {
		LIBIOTRACE_WARN("unknown status (%d) in response from influxdb", parser->status_code);
	} else {
		//LIBIOTRACE_WARN("known status (%d) in response from influxdb", parser->status_code);
	}

	return 0;
}

/**
 * Callback for each http request to control libiotrace.
 *
 * If a complete URL is read this callback is called. It accepts
 * the following requests:
 *
 * POST: An "/" followed by a function name, followed by a "/",
 *       followed by a "1" or a "0".
 *       If the last sign in the URL is a "1" and a wrapper for
 *       the function name exists this wrapper is set to active.
 *       Else if the function name exists the wrapper is set to
 *       inactive.
 * GET:  Requests an json object with all known wrappers as keys
 *       and the status (active/inactive) as value (1/0).
 *
 * The responses to the requests are send to the connection the
 * request came in. For that the global thread local variable
 * "socket_peer" has to be set to the corresponding socket
 * before the http parser calls this callback.
 * The callback is only used inside "communication_thread" which
 * exists once per process. So the "socket_peer" is unique and
 * thread safe.
 *
 * @param[in] parser A pointer to the parser which has called
 *                   this callback. Gives access to the parser
 *                   state (like method of the request).
 * @param[in] at     Start pointer of the parsed URL as a char
 *                   array. The array is not terminated by "\0"
 *                   (see "length").
 * @param[in] length Count of chars in the URL (in the array
 *                   given by "at").
 *
 * @return Error state of the callback (not used; gives "0" back).
 */
int url_callback_requests(llhttp_t *parser, const char *at, size_t length) {
	if (parser->method == HTTP_POST)
	{
		char *slash1 = NULL;
		char *slash2 = NULL;
		slash1 = (char *)memchr(at, '/', length);
		slash2 = (char *)memrchr(at, '/', length);
		if (slash1 != NULL && slash1 != slash2 && at + length - 1 == slash2 + 1)
		{
			char functionname[slash2 - slash1];
			strncpy(functionname, slash1 + 1, slash2 - slash1 - 1);
			functionname[slash2 - slash1 - 1] = '\0';
			if (at[length - 1] == '1')
			{
				toggle_wrapper(functionname, 1);
			}
			else
			{
				toggle_wrapper(functionname, 0);
			}
			send_data("HTTP/1.1 204 No Content" LINE_BREAK LINE_BREAK LINE_BREAK, socket_peer);
		}
		else
		{
			send_data("HTTP/1.1 400 Bad Request" LINE_BREAK LINE_BREAK LINE_BREAK, socket_peer);
		}
	}
	else if (parser->method == HTTP_GET)
	{
		const char *message_header = "HTTP/1.1 200 OK" LINE_BREAK
				"Content-Length: %ld" LINE_BREAK
				"Content-Type: application/json" LINE_BREAK
				LINE_BREAK
				"%s";

		// buffer for body
		char buf[libiotrace_struct_max_size_wrapper_status() + 1];
		int ret = libiotrace_struct_print_wrapper_status(buf, sizeof(buf), &active_wrapper_status);
		if (0 > ret)
		{
			LIBIOTRACE_ERROR("libiotrace_struct_print_wrapper_status() returned %d", ret);
		}

		const int message_len = strlen(message_header) + COUNT_DEC_AS_CHAR(ret) + ret;
		char message[message_len + 1];

		snprintf(message, sizeof(message), message_header, ret, buf);

		send_data(message, socket_peer);
	}
	else
	{
		send_data("HTTP/1.1 405 Method Not Allowed" LINE_BREAK LINE_BREAK LINE_BREAK, socket_peer);
	}

	return 0;
}

/**
 * Callback for each http request/response.
 *
 * If a complete URL/status is read this callback is called. It calls
 * for responses
 *     url_callback_responses
 * and for requests
 *     url_callback_requests
 *
 * @param[in] parser A pointer to the parser which has called
 *                   this callback. Gives access to the parser
 *                   state (like method of the request).
 * @param[in] at     Start pointer of the parsed URL/status as a char
 *                   array. The array is not terminated by "\0"
 *                   (see "length").
 * @param[in] length Count of chars in the URL/status (in the array
 *                   given by "at").
 *
 * @return Error state of the callback (not used; gives "0" back).
 */
int url_callback(llhttp_t *parser, const char *at, size_t length)
{
	switch(parser->type) {
	case HTTP_RESPONSE:
		url_callback_responses(parser, at, length);
		break;
	case HTTP_REQUEST:
		url_callback_requests(parser, at, length);
		break;
	default:
		LIBIOTRACE_ERROR("unknown parser type");
	}

	return 0;
}

/**
 * Thread for reading answers from influxdb and receiving commands
 * from http connections (WebServices).
 *
 * Started as a separate thread via "pthread_create" during call
 * of "init_process".
 *
 * Checks for responses from influxdb on existing connections
 * (each thread of the monitored program opens a new socket to
 * communicate with influxdb during call of "init_thread" with the
 * function "prepare_socket"; each such thread uses it's socket to
 * send data to influxdb but doesn't wait for an response;
 * responses are read in this separate thread).
 *
 * Also waits for incoming connections on a additional socket.
 * Each such connection is handled inside this separate thread and
 * enables controlling which wrapper is active. The additional
 * socket for incoming connections is bound to a PORT in the range
 * from PORT_RANGE_MIN to PORT_RANGE_MAX. Which port is bound is
 * written to a file. Incoming http requests are handled via
 * callback function "url_callback".
 *
 * This thread runs and reads/listens the multiple sockets until
 * "event_cleanup_done" is set to "true". This is done during call
 * of "cleanup" if the program exits.
 *
 * @param[in] arg Not used.
 * @return Not used (allways NULL)
 */
void *communication_thread(void *arg)
{
	struct timeval select_timeout;

	// Open Socket to receive control information
	struct sockaddr_in addr;
	socket_control = CALL_REAL_POSIX_SYNC(socket)(PF_INET, SOCK_STREAM, 0);
	if (socket_control < 0)
	{
		LIBIOTRACE_ERROR("could not open socket, errno %d", errno);
	}
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_ANY);

	// Look up free ports in specific range and bind socket to free port
	int i;
	for (i = PORT_RANGE_MIN; i <= PORT_RANGE_MAX; i++)
	{
		addr.sin_port = htons(i);
		if (!CALL_REAL_POSIX_SYNC(bind)(socket_control, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)))
		{
			//Write PORT and IP for control commands to file
			struct ifreq ifreqs[20];
			struct ifconf ic;

			ic.ifc_len = sizeof ifreqs;
			ic.ifc_req = ifreqs;

			if (ioctl(socket_control, SIOCGIFCONF, &ic) < 0)
			{
				LIBIOTRACE_ERROR("ioctl returned -1, errno %d", errno);
			}

			int fd = CALL_REAL_POSIX_SYNC(open)(control_log_name,
												O_WRONLY | O_CREAT | O_APPEND,
												S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
			if (-1 == fd)
			{
				LIBIOTRACE_ERROR("open() of file %s returned %d", control_log_name, fd);
			}

			for (int l = 0; l < ic.ifc_len / sizeof(struct ifreq); ++l)
			{
				int ret = dprintf(fd, "(%u) %s: %s:%d" LINE_BREAK, pid, ifreqs[l].ifr_name,
								  inet_ntoa(((struct sockaddr_in *)&ifreqs[l].ifr_addr)->sin_addr), i);

				if (0 > ret)
				{
					LIBIOTRACE_ERROR("dprintf() returned %d with errno=%d", ret, errno);
				}
			}

			CALL_REAL_POSIX_SYNC(close)
			(fd);

			break;
		}
	}
	if (i > PORT_RANGE_MAX)
	{
		CALL_REAL_POSIX_SYNC(close)
		(socket_control);
		LIBIOTRACE_ERROR("unable to bind socket");
	}

	// Listen to socket
	int ret = listen(socket_control, 10);
	if (0 > ret)
	{
		CALL_REAL_POSIX_SYNC(close)
		(socket_control);
		LIBIOTRACE_ERROR("unable to listen to socket, errno=%d", errno);
	}
	// Now wait for connection in select below

	// Read responses from influxdb and read control messages
	while (!event_cleanup_done)
	{
		fd_set fd_recv_sockets;
		FD_ZERO(&fd_recv_sockets);
		SOCKET socket_max = -1;
		pthread_mutex_lock(&socket_lock);
		for (int i = 0; i < recv_sockets_len; i++)
		{
			FD_SET(recv_sockets[i]->socket, &fd_recv_sockets);
			if (recv_sockets[i]->socket > socket_max)
			{
				socket_max = recv_sockets[i]->socket;
			}
		}
		pthread_mutex_unlock(&socket_lock);
		//Add listening socket for establishing control connections to fd_set
		FD_SET(socket_control, &fd_recv_sockets);
		if (socket_control > socket_max)
		{
			socket_max = socket_control;
		}
		//Add active control connections to fd_set
		for (int i = 0; i < open_control_sockets_len; i++)
		{
			FD_SET(open_control_sockets[i]->socket, &fd_recv_sockets);
			if (open_control_sockets[i]->socket > socket_max)
			{
				socket_max = open_control_sockets[i]->socket;
			}
		}

		select_timeout.tv_sec = SELECT_TIMEOUT_SECONDS;
		select_timeout.tv_usec = 0;
		int ret = CALL_REAL_POSIX_SYNC(select)(socket_max + 1, &fd_recv_sockets, NULL, NULL, &select_timeout);
		// Select: At least one socket is ready to be processed
		if (-1 == ret)
		{
			LIBIOTRACE_WARN("select() returned -1, errno=%d.", errno);
			break;
		} else if (0 < ret) {
			/* check which descriptor/socket is ready to read if there was new data before timeout expired */

			// receive responses from influxdb
			pthread_mutex_lock(&socket_lock);
			for (int i = 0; i < recv_sockets_len; i++)
			{
				//Which sockets are ready to read
				if (FD_ISSET(recv_sockets[i]->socket, &fd_recv_sockets))
				{
					char read[4096];
					ssize_t bytes_received = recv(recv_sockets[i]->socket, read, 4096, 0);
					if (1 > bytes_received)
					{
						//Socket is destroyed or closed by peer
						//close(recv_sockets[i]);
						//delete_socket(recv_sockets[i]);
						//i--;
					} else {
						// TODO: parse/interpret responses (each socket needs it's own llhttp_t parser)
						enum llhttp_errno err = llhttp_execute(&(recv_sockets[i]->parser), read, bytes_received);
						if (err != HPE_OK)
						{
							const char *errno_text = llhttp_errno_name(err);
							LIBIOTRACE_ERROR("error parsing influxdb response: %s: %s", errno_text, recv_sockets[i]->parser.reason);
						}
					}
				}
			}
			pthread_mutex_unlock(&socket_lock);

			// If socket to establish new connections is ready
			if (FD_ISSET(socket_control, &fd_recv_sockets))
			{
				//Accept one new socket but more could be ready at this point
				SOCKET socket = accept(socket_control, NULL, NULL);
				if (0 > socket)
				{
					LIBIOTRACE_ERROR("accept returned -1, errno=%d", errno);
				}
				//Connection established; write all established sockets in array
				libiotrace_socket *s = create_libiotrace_socket(socket, HTTP_REQUEST);
				save_socket(s, NULL, &open_control_sockets_len, &open_control_sockets);
			}

			// receive control requests
			for (int i = 0; i < open_control_sockets_len; i++)
			{
				//Which sockets are ready to read
				if (FD_ISSET(open_control_sockets[i]->socket, &fd_recv_sockets))
				{
					char read[4096];
					ssize_t bytes_received = recv(open_control_sockets[i]->socket, read, 4096, 0);
					if (1 > bytes_received)
					{
						//Socket is destroyed or closed by peer
						CLOSESOCKET(open_control_sockets[i]->socket);
						libiotrace_socket *s = open_control_sockets[i];
						delete_socket(open_control_sockets[i]->socket, NULL, &open_control_sockets_len, &open_control_sockets);
						free(s);
						i--;
					}
					else
					{
						socket_peer = open_control_sockets[i]->socket; // is needed by callback => must be set before llhttp_execute()
						// TODO: to handle more than one connection a separate parser for each socket/connection is needed
						enum llhttp_errno err = llhttp_execute(&(open_control_sockets[i]->parser), read, bytes_received);
						if (err != HPE_OK)
						{
							const char *errno_text = llhttp_errno_name(err);
							snprintf(read, sizeof(read), "HTTP/1.1 400 Bad Request" LINE_BREAK "Content-Length: %ld" LINE_BREAK "Content-Type: application/json" LINE_BREAK LINE_BREAK "%s: %s",
									strlen(errno_text) + 2 + strlen(open_control_sockets[i]->parser.reason), errno_text, open_control_sockets[i]->parser.reason);
							send_data(read, socket_peer);
						}
					}
				}
			}
		}
	}
	CLOSESOCKET(socket_control);
	for (int i = 0; i < open_control_sockets_len; i++)
	{
		CLOSESOCKET(open_control_sockets[i]->socket);
		free(open_control_sockets[i]);
	}
	free(open_control_sockets);
	return NULL;
}

/**
 * Reads an environment variable.
 *
 * @param[in]  env_name            Name of the environment variable to
 *                                 read as "\0" terminated char array.
 * @param[out] dst                 Pointer to a buffer in which the read
 *                                 environment variable is stored.
 * @param[in]  max_len             Length of the buffer "dst".
 * @param[in]  error_if_not_exists If set to "true" and if no variable
 *                                 with "env_name" as a name was found
 *                                 an error is printed and "exit" is
 *                                 called.
 * @return The length of the read variable (without the terminating
 *         "\0") or 0 if no variable with "env_name" as name was found.
 */
int libiotrace_get_env(const char *env_name, char *dst, const int max_len, const char error_if_not_exists)
{
	char *log;
	int length;

	log = getenv(env_name);
	if (NULL == log)
	{
		if (error_if_not_exists)
		{
			LIBIOTRACE_ERROR("getenv(\"%s\") returned NULL", env_name);
		}
		else
		{
			return 0;
		}
	}
	length = strlen(log);
	if (max_len < length)
	{
		LIBIOTRACE_ERROR("getenv() returned %s too long (%d bytes) for buffer", env_name, length);
	}

	strcpy(dst, log);

	return length;
}

/**
 * Reads a whitelist from a file.
 *
 * If a environment variable holds the path of a whitelist
 * this path is set to the variable "whitelist" and this
 * function is called. This function reads every line of
 * the file and interprets each line not starting with "#"
 * as the name of a function (leading and trailing white
 * spaces are removed). For each function name the function
 * "toggle_event_wrapper" is called. If a wrapper for a
 * function with a corresponding name exists that wrapper
 * is set to active. An active wrapper logs/sends his data.
 */
void read_whitelist() {
	FILE *stream;
	char *line = NULL;
	char *clean_line = NULL;
	char *end_clean_line = NULL;
	size_t len = 0;
	ssize_t nread;
	stream = CALL_REAL_POSIX_SYNC(fopen)(whitelist, "r");
	if (stream == NULL)
	{
		LIBIOTRACE_ERROR("fopen() failed, errno=%d", errno);
	}

	while ((nread = CALL_REAL_POSIX_SYNC(getline)(&line, &len, stream)) != -1)
	{
		size_t byte_count = strlen(line);

		// remove trailing linebreak
		if (byte_count > 0 && line[byte_count - 1] == '\n')
		{
			line[byte_count - 1] = '\0';
		}
		if (byte_count > 0 && line[byte_count - 2] == '\r')
		{
			line[byte_count - 2] = '\0';
		}
		clean_line = line;

		// remove leading spaces
		while (isspace((unsigned char)*clean_line))
		{
			clean_line++;
		}

		// not a comment and not only spaces
		if (*clean_line != '#' && *clean_line != '\0')
		{
			end_clean_line = clean_line + strlen(clean_line) - 1;

			// remove trailing spaces
			while (end_clean_line > clean_line && isspace((unsigned char)*end_clean_line))
			{
				end_clean_line--;
			}
			end_clean_line[1] = '\0';

			toggle_wrapper(clean_line, 1);
		}
	}

	free(line);
	CALL_REAL_POSIX_SYNC(fclose)
	(stream);
}

/**
 * Generates environment variable from "key" and "value".
 *
 * @param[out] env       Pointer to buffer in which environment variable
 *                       is stored. Buffer must have sufficient length
 *                       (strlen(key) + strlen(value) + 2).
 * @param[in] key        "\0" terminated key of the environment variable.
 * @param[in] key_length Char count of "key" excluding terminating "\0"
 *                       (strlen(key)).
 * @param[in] value      "\0" terminated value of the environment
 *                       variable.
 */
void generate_env(char *env, const char *key, const int key_length, const char *value) {
	strcpy(env, key);
	strcpy(env + key_length, "=");
	strcpy(env + key_length + 1, value);
}

char init_done = 0;
/**
 * Initialize libiotrace for current process.
 *
 * Is called from init_on_load() out of the ctor section before main() is called.
 * If another ctor entry calls a wrapper before init_on_load() was called, the
 * wrapper calls init_process(). So init_process() is called before the first
 * wrapper starts collecting data.
 * Because init_process() is called before main() is called, no other thread then
 * the process itself is running => no synchronization is needed.
 */
void init_process()
{
	char *log;
	int length;

	if (!init_done)
	{
		pos = data_buffer;
		count_basic = 0;

#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_VARIABLE
#include "event_wrapper.h"

#ifdef WITH_MPI_IO
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_VARIABLE
#include "mpi_io_wrapper.h"
#endif

#ifdef WITH_POSIX_IO
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_VARIABLE
#include "posix_io_wrapper.h"
#endif

#ifdef WITH_POSIX_AIO
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_VARIABLE
#include "posix_aio_wrapper.h"
#endif

#ifdef WITH_DL_IO
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_VARIABLE
#include "dl_io_wrapper.h"
#endif

#if !defined(IO_LIB_STATIC)
		init_wrapper();
#endif

#if !defined(HAVE_HOST_NAME_MAX)
#if defined(HAVE__POSIX_HOST_NAME_MAX)
		host_name_max = _POSIX_HOST_NAME_MAX;
#else
		host_name_max = sysconf(_SC_HOST_NAME_MAX);
#endif
#endif

		pid = getpid();
		hostname = malloc(HOST_NAME_MAX);
		if (NULL == hostname)
			LIBIOTRACE_ERROR("malloc failed, errno=%d", errno);

		gethostname(hostname, HOST_NAME_MAX);

		char filesystem_postfix[] = "_filesystem_";
		char filesystem_extension[] = ".log";
		length = libiotrace_get_env(env_log_name, log_name, MAXFILENAME - strlen(filesystem_extension) - strlen(filesystem_postfix) - strlen(hostname), 1);
		strcpy(filesystem_log_name, log_name);
		strcpy(working_dir_log_name, log_name);
		strcpy(control_log_name, log_name);
		strcpy(log_name + length, "_iotrace.log");
		strcpy(filesystem_log_name + length, filesystem_postfix);
		strcpy(filesystem_log_name + length + strlen(filesystem_postfix), hostname);
		strcpy(filesystem_log_name + length + strlen(filesystem_postfix) + strlen(hostname), filesystem_extension);
		strcpy(working_dir_log_name + length, "_working_dir.log");
		strcpy(control_log_name + length, "_control.log");

#ifndef IO_LIB_STATIC
		generate_env(log_name_env, env_log_name, length, log_name);
#endif

		// get token from environment
		length = libiotrace_get_env(env_influx_token, influx_token, MAX_INFLUX_TOKEN, 1);
		influx_token_len = strlen(influx_token);

#ifndef IO_LIB_STATIC
		generate_env(influx_token_env, env_influx_token, length, influx_token);
#endif

		// get bucket name from environment
		length = libiotrace_get_env(env_influx_bucket, influx_bucket, MAX_INFLUX_BUCKET, 1);
		influx_bucket_len = strlen(influx_bucket);

#ifndef IO_LIB_STATIC
		generate_env(influx_bucket_env, env_influx_bucket, length, influx_bucket);
#endif

		// get organization name from environment
		length = libiotrace_get_env(env_influx_organization, influx_organization, MAX_INFLUX_ORGANIZATION, 1);
		influx_organization_len = strlen(influx_organization);

#ifndef IO_LIB_STATIC
		generate_env(influx_organization_env, env_influx_organization, length, influx_organization);
#endif

		// get database ip from environment
		length = libiotrace_get_env(env_database_ip, database_ip, MAX_DATABASE_IP, 1);

#ifndef IO_LIB_STATIC
		generate_env(database_ip_env, env_database_ip, length, database_ip);
#endif

		// get database port from environment
		length = libiotrace_get_env(env_database_port, database_port, MAX_DATABASE_PORT, 1);

#ifndef IO_LIB_STATIC
		generate_env(database_port_env, env_database_port, length, database_port);
#endif

		// Path to wrapper whitelist
		length = libiotrace_get_env(env_wrapper_whitelist, whitelist, MAXFILENAME, 0);
		if (0 != length)
		{
#ifndef IO_LIB_STATIC
			has_whitelist = 1;
			generate_env(whitelist_env, env_wrapper_whitelist, length, whitelist);
#endif

			read_whitelist();
		} else {
#ifndef IO_LIB_STATIC
			has_whitelist = 0;
#endif
		}

#ifndef IO_LIB_STATIC
		length = strlen(env_ld_preload);
		strcpy(ld_preload, env_ld_preload);
		strcpy(ld_preload + length, "=");
		length = libiotrace_get_env(env_ld_preload, ld_preload + length + 1, MAXFILENAME, 1);
#endif

#ifndef REALTIME
		system_start_time = iotrace_get_boot_time();
#endif

		pthread_mutex_init(&lock, NULL);
		pthread_mutex_init(&socket_lock, NULL);

		pthread_atfork(NULL, NULL, reset_values_in_forked_process);
#if 0 // RAY MacOS
		print_filesystem();
#endif
		print_working_directory();

#ifdef WITH_STD_IO
		open_std_fd(STDIN_FILENO);
		open_std_fd(STDOUT_FILENO);
		open_std_fd(STDERR_FILENO);

		open_std_file(stdin);
		open_std_file(stdout);
		open_std_file(stderr);
#endif

		/* Initialize user callbacks and settings */
		llhttp_settings_init(&settings);
		/* Set user callback */
		//TODO: direct call of url_callback_responses and url_callback_requests
		settings.on_url = url_callback;
		settings.on_status = url_callback;

		//Create receive thread per process
		pthread_t recv_thread;

		int ret = pthread_create(&recv_thread, NULL, communication_thread, NULL);
		if (0 != ret)
		{
			LIBIOTRACE_WARN("pthread_create() failed. (%d)", ret);
			return;
		}
		init_done = 1;
	}
}

/**
 * Fills stacktrace information to given struct basic.
 *
 * @param[out] data A pointer to a struct basic structure
 */
void get_stacktrace(struct basic *data)
{
	int size;
	void *trace = malloc(sizeof(void *) * (stacktrace_depth + 3));
	char **messages = (char **)NULL;

	if (NULL == trace)
	{
		LIBIOTRACE_ERROR("malloc() returned NULL");
	}

	size = backtrace(trace, stacktrace_depth + 3);
	if (0 >= size)
	{
		LIBIOTRACE_ERROR("backtrace() returned %d", size);
	}

	if (stacktrace_ptr)
	{
		LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY((*data), stacktrace_pointer, trace, 3,
										 size)
	}
	else
	{
		LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL((*data), stacktrace_pointer)
	}

	if (stacktrace_symbol)
	{
		messages = backtrace_symbols(trace, size);
		if (NULL == messages)
		{
			LIBIOTRACE_ERROR("backtrace_symbols() returned NULL with errno=%d", errno);
		}

		LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY((*data), stacktrace_symbols,
											messages, 3, size)
	}
	else
	{
		LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL((*data), stacktrace_symbols)
	}

	if (!stacktrace_ptr)
	{
		free(trace);
	}
}

/**
 * Initialize libiotrace for current thread.
 *
 * Is called from get_basic() during first call of a wrapper in a thread.
 */
void init_thread()
{
	tid = iotrace_get_tid();
	prepare_socket();
}

/**
 * Sets some basic information to the given structure.
 *
 * Fills the "process_id", "thread_id", "hostname" and if
 * needed the stacktrace with the current values.
 * If "get_basic" is called for the first time in a new
 * thread the "init_thread" function is called to
 * initialize libiotrace for the current thread.
 *
 * @param[out] data A pointer to a struct basic structure
 */
void get_basic(struct basic *data)
{
	/* tid is thread local storage => no synchronization with
	 * other threads is needed */
	if (tid == -1)
	{
		/* call once per new thread */
		init_thread();
	}

	data->process_id = pid;
	data->thread_id = tid;

	data->hostname = hostname;

	if (0 < stacktrace_depth && (stacktrace_ptr || stacktrace_symbol))
	{
		get_stacktrace(data);
	}
	else
	{
		LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL((*data), stacktrace_symbols)
		LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL((*data), stacktrace_pointer)
	}
}

/**
 * Gets actual time in nano seconds.
 *
 * @return time in nano seconds
 */
inline u_int64_t gettime(void)
{
	struct timespec t;
	u_int64_t time;
#ifdef REALTIME
	clock_gettime(CLOCK_REALTIME, &t);
#else
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
#endif
	time = (u_int64_t)t.tv_sec * 1000000000ll + (u_int64_t)t.tv_nsec;
	return time;
}

/**
 * Prints all structures from buffer to file.
 *
 * Each structure is serialized to json and written to
 * a file. After that the buffer is empty. Serialization
 * and clearing of the buffer is not synchronized with
 * other threads. So "print_buffer" should only be
 * called from a synchronized code.
 */
void print_buffer()
{
	struct basic *data;
	int ret;
	char buf[libiotrace_struct_max_size_basic() + 1]; /* +1 for trailing null character */
	int fd;
	pos = data_buffer;

	fd = CALL_REAL_POSIX_SYNC(open)(log_name, O_WRONLY | O_CREAT | O_APPEND,
									S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
	if (-1 == fd)
	{
		LIBIOTRACE_ERROR("open() of file %s returned %d", log_name, fd);
	}

	for (int i = 0; i < count_basic; i++)
	{
		data = (struct basic *)((void *)pos);

		ret = libiotrace_struct_print_basic(buf, sizeof(buf), data); //Function is present at runtime, built with macros from libiotrace_defines.h
		ret = dprintf(fd, "%s" LINE_BREAK, buf);
		if (0 > ret)
		{
			LIBIOTRACE_ERROR("dprintf() returned %d", ret);
		}
		ret = libiotrace_struct_sizeof_basic(data);

		pos += ret;
	}

	CALL_REAL_POSIX_SYNC(close)
	(fd);

	pos = data_buffer;
	count_basic = 0;
}

/**
 * Sends a struct basic to influxdb.
 *
 * @param[in] data Pointer to struct basic
 */
void write_into_influxdb(struct basic *data)
{
	if (event_cleanup_done || no_sending)
	{
		return;
	}

	//buffer for body
	int body_length = libiotrace_struct_push_max_size_basic(0) + 1; /* +1 for trailing null character (function build by macros; gives length of body to send) */
	char body[body_length];
	body_length = libiotrace_struct_push_basic(body, body_length, data, "");
	if (0 > body_length)
	{
		LIBIOTRACE_ERROR("libiotrace_struct_push_basic() returned %d", body_length);
	}
	body_length--; /*last comma in ret*/
	body[body_length] = '\0'; /*remove last comma*/

	char short_log_name[50];
	strncpy(short_log_name, log_name, sizeof(short_log_name));

	const char labels[] = "libiotrace,jobname=%s,hostname=%s,processid=%u,thread=%u,functionname=%s";
	int body_labels_length = strlen(labels)
			+ sizeof(short_log_name) /* jobname */
			+ HOST_NAME_MAX /* hostname */
			+ COUNT_DEC_AS_CHAR(data->process_id) /* processid */
			+ COUNT_DEC_AS_CHAR(data->thread_id) /* thread */
			+ MAX_FUNCTION_NAME; /* functionname */
	char body_labels[body_labels_length];
	snprintf(body_labels, sizeof(body_labels), labels, short_log_name, data->hostname, data->process_id, data->thread_id, data->function_name);
	body_labels_length = strlen(body_labels);

	int timestamp_length = COUNT_DEC_AS_CHAR(data->time_end);
	char timestamp[timestamp_length];
#ifdef REALTIME
	snprintf(timestamp, sizeof(timestamp), "%" PRIu64, data->time_end);
#else
	snprintf(timestamp, sizeof(timestamp), "%" PRIu64, system_start_time + data->time_end);
#endif
	timestamp_length = strlen(timestamp);

	const int content_length = body_labels_length + 1 /*space*/ + body_length + 1 /*space*/ + timestamp_length;

	const char header[] = "POST /api/v2/write?bucket=%s&precision=ns&org=%s HTTP/1.1" LINE_BREAK
			"Host: localhost:8086" LINE_BREAK /* TODO: localhost vs. real host */
			"Accept: */*" LINE_BREAK
			"Authorization: Token %s" LINE_BREAK
			"Content-Length: %d" LINE_BREAK
			"Content-Type: application/x-www-form-urlencoded" LINE_BREAK
			LINE_BREAK
			"%s %s %s";
	const int message_length = strlen(header)
			+ influx_bucket_len
			+ influx_organization_len
			+ influx_token_len
			+ COUNT_DEC_AS_CHAR(content_length) /* Content-Length */
			+ body_labels_length
			+ body_length
			+ timestamp_length;

	//buffer all (header + body)
	char message[message_length + 1];
	snprintf(message, sizeof(message), header, influx_bucket, influx_organization, influx_token,
			content_length, body_labels, body, timestamp);

	send_data(message, socket_peer);
}

/**
 * Writes a struct basic to the buffer for this process.
 *
 * A deep copy of all values from "data" and all in "data"
 * referenced structures and arrays is synchronized written
 * to the central buffer.
 * If the buffer hasn't enough free space for the deep copy
 * the buffer is cleared with a call to "print_buffer"
 * first.
 *
 * @param[in] data Pointer to struct basic
 */
void write_into_buffer(struct basic *data)
{
#ifdef LOG_WRAPPER_TIME
	static char *old_pos;
#endif

	if (no_logging)
	{
		return;
	}

	/* write (synchronized) */
	pthread_mutex_lock(&lock);

	int length = libiotrace_struct_sizeof_basic(data);

	if (pos + length > endpos)
	{
		print_buffer();
	}
	if (pos + length > endpos)
	{
		// ToDo: solve circular dependency of fprintf
		LIBIOTRACE_ERROR("buffer (%ld bytes) not big enough for even one struct basic (%d bytes)", sizeof(data_buffer), length);
	}

#ifdef LOG_WRAPPER_TIME
	old_pos = pos;
#endif
	pos = (void *)libiotrace_struct_copy_basic((void *)pos, data);
	count_basic++;
	// insert end time for wrapper in buffer
	WRAPPER_TIME_END((*((struct basic *)((void *)old_pos))))

	pthread_mutex_unlock(&lock);
}

/**
 * Frees dynamically allocated memory in struct basic
 *
 * @aram[in] data Pointer to struct basic
 */
void free_memory(struct basic *data)
{
	libiotrace_struct_free_basic(data);
}

/**
 * Write buffer and close sockets before process terminates.
 *
 * Is called after main() via "dtor" section or during a call of a
 * wrapper of a "exit*" function. Writes buffer contents to file,
 * closes open connections (sockets) and destroys mutexes.
 */
void cleanup()
{
	event_cleanup_done = 1;
#ifdef LOG_WRAPPER_TIME
	struct basic data;
	data.time_start = 0;
	data.time_end = 0;
	data.return_state = ok;
	data.return_state_detail = NULL;
#endif

	WRAPPER_TIME_START(data)

	pthread_mutex_lock(&lock);
	print_buffer();
	pthread_mutex_unlock(&lock);

#ifdef LOG_WRAPPER_TIME
	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)
#endif

	WRAPPER_TIME_END(data);

#ifdef LOG_WRAPPER_TIME
	write_into_buffer(&data);
	pthread_mutex_lock(&lock);
	print_buffer();
	pthread_mutex_unlock(&lock);
	WRAP_FREE(&data)
#endif

	pthread_mutex_lock(&socket_lock);
	for (int i = 0; i < recv_sockets_len; i++)
	{
		shutdown(recv_sockets[i]->socket, SHUT_WR);
		while (1)
		{
			fd_set reads;
			FD_ZERO(&reads);
			FD_SET(recv_sockets[i]->socket, &reads);

			int ret = CALL_REAL_POSIX_SYNC(select)(recv_sockets[i]->socket + 1, &reads, NULL, NULL, NULL);
			if (-1 == ret)
			{
				LIBIOTRACE_WARN("select() returned -1, errno=%d.", errno);
				break;
			}
			if (FD_ISSET(recv_sockets[i]->socket, &reads))
			{
				char read[4096];
				int bytes_received = recv(recv_sockets[i]->socket, read, 4096, 0);
				if (bytes_received < 1)
				{
					// Connection closed by peer
					break;
				}
			}
		}
		CLOSESOCKET(recv_sockets[i]->socket);
	}
	pthread_mutex_unlock(&socket_lock);

	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&socket_lock);
}

#ifndef IO_LIB_STATIC
/**
 * Ensure that an array of environment variables contains all libiotrace variables.
 *
 * A call to an "exec*" function can manipulate the environment variables. To ensure
 * that all new processes are under the control of libiotrace the manipulated
 * environment variables must contain all necessary variables.
 * The given array "envp" is checked for LD_PRELOAD. If LD_PRELOAD isn't contained
 * in "envp" all variables from "envp" and all variables used by libiotrace are
 * added to "env". Else "env" is a copy of "envp".
 *
 * @param[in,out] env  An empty array of char arrays as input. Is filled with all
 *                     variables from "envp", all missing necessary variables
 *                     used by libiotrace and a terminating NULL as last array
 *                     during function call of "check_ld_preload".
 * @param[in]     envp Environment variables of the new process as a NULL
 *                     terminated array of strings ("\0" terminated char arrays)
 * @param[in]     func Function name of the calling function as "\0" terminated
 *                     char array. Used for error handling.
 */
void check_ld_preload(char *env[], char *const envp[], const char *func)
{
	int env_element;
	char has_ld_preload = 0;
	char envp_null = 0;

	for (env_element = 0; env_element < MAX_EXEC_ARRAY_LENGTH; env_element++)
	{
		env[env_element] = envp[env_element];

		if (NULL != envp[env_element])
		{
			if (strcmp(ld_preload, envp[env_element]) == 0)
			{
				has_ld_preload = 1;
			}
		}
		else
		{
			envp_null = 1;
			break;
		}
	}

	if (!envp_null)
	{
		LIBIOTRACE_ERROR("during call of %s envp[] has more elements then buffer (%d)", func, MAX_EXEC_ARRAY_LENGTH);
	}

	if (!has_ld_preload)
	{
		int count_libiotrace_env = 7;
		if (has_whitelist)
		{
			count_libiotrace_env++;
		}

		if (MAX_EXEC_ARRAY_LENGTH <= env_element + count_libiotrace_env)
		{
			LIBIOTRACE_ERROR("during call if %s envp[] with added libiotrace-variables has more elements then buffer (%d)", func, MAX_EXEC_ARRAY_LENGTH);
		}
		env[env_element] = &ld_preload[0];
		env[++env_element] = &log_name_env[0];
		env[++env_element] = &database_ip_env[0];
		env[++env_element] = &database_port_env[0];
		env[++env_element] = &influx_token_env[0];
		if (has_whitelist)
		{
			env[++env_element] = &whitelist_env[0];
		}
		env[++env_element] = &influx_bucket_env[0];
		env[++env_element] = &influx_organization_env[0];
		env[++env_element] = NULL;
	}
}
#endif

/*******************************************************************************/
/* exec and exit function wrapper                                              */

int WRAP(execve)(const char *filename, char *const argv[], char *const envp[])
{
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
#ifndef IO_LIB_STATIC
	char *env[MAX_EXEC_ARRAY_LENGTH];
	check_ld_preload(env, envp, __func__);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execve, filename, argv, env)
#else
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execve, filename, argv, envp)
#endif

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, execve)
	return ret;
}

int WRAP(execv)(const char *path, char *const argv[])
{
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execv, path, argv)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, execv)
	return ret;
}

int WRAP(execl)(const char *path, const char *arg, ... /* (char  *) NULL */)
{
	int ret;
	struct basic data;
	char *argv[MAX_EXEC_ARRAY_LENGTH];
	int count = 0;
	char *element;
	va_list ap;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	va_start(ap, arg);
	element = (char *)((void *)arg);
	while (NULL != element)
	{
		if (count >= MAX_EXEC_ARRAY_LENGTH - 1)
		{
			LIBIOTRACE_ERROR("buffer (%d elements) not big enough for argument array", MAX_EXEC_ARRAY_LENGTH);
		}
		argv[count] = element;
		count++;

		element = va_arg(ap, char *);
	}
	argv[count] = element;
	va_end(ap);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execv, path, argv)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, execl)
	return ret;
}

int WRAP(execvp)(const char *file, char *const argv[])
{
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvp, file, argv)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, execvp)
	return ret;
}

int WRAP(execlp)(const char *file, const char *arg, ... /* (char  *) NULL */)
{
	int ret;
	struct basic data;
	char *argv[MAX_EXEC_ARRAY_LENGTH];
	int count = 0;
	char *element;
	va_list ap;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	va_start(ap, arg);
	element = (char *)((void *)arg);
	while (NULL != element)
	{
		if (count >= MAX_EXEC_ARRAY_LENGTH - 1)
		{
			LIBIOTRACE_ERROR("buffer (%d elements) not big enough for argument array", MAX_EXEC_ARRAY_LENGTH);
		}
		argv[count] = element;
		count++;

		element = va_arg(ap, char *);
	}
	argv[count] = element;
	va_end(ap);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvp, file, argv)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, execlp)
	return ret;
}

#ifdef HAVE_EXECVPE
int WRAP(execvpe)(const char *file, char *const argv[], char *const envp[])
{
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
#ifndef IO_LIB_STATIC
	char *env[MAX_EXEC_ARRAY_LENGTH];
	check_ld_preload(env, envp, __func__);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, file, argv, env)
#else
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, file, argv, envp)
#endif

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, execvpe)
	return ret;
}
#endif

int WRAP(execle)(const char *path, const char *arg,
				 ... /*, (char *) NULL, char * const envp[] */)
{
#ifndef HAVE_EXECVPE
	LIBIOTRACE_ERROR("wrapper needs function execvpe() to work properly");
#endif
	int ret;
	struct basic data;
	char *argv[MAX_EXEC_ARRAY_LENGTH];
	int count = 0;
	char *element;
	char **envp;
	va_list ap;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	va_start(ap, arg);
	element = (char *)((void *)arg);
	while (NULL != element)
	{
		if (count >= MAX_EXEC_ARRAY_LENGTH - 1)
		{
			LIBIOTRACE_ERROR("buffer (%d elements) not big enough for argument array", MAX_EXEC_ARRAY_LENGTH);
		}
		argv[count] = element;
		count++;

		element = va_arg(ap, char *);
	}
	argv[count] = element;
	envp = va_arg(ap, char **);
	va_end(ap);

#ifndef IO_LIB_STATIC
	char *env[MAX_EXEC_ARRAY_LENGTH];
	check_ld_preload(env, envp, __func__);
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, path, argv, env)
#else
	CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvpe, path, argv, envp)
#endif

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, execle)
	return ret;
}

void WRAP(_exit)(int status)
{
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_NO_RETURN(data, _exit, status)
}

#ifdef HAVE_EXIT
void WRAP(_Exit)(int status)
{
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_NO_RETURN(data, _Exit, status)
}
#endif

#ifdef HAVE_EXIT_GROUP
void WRAP(exit_group)(int status)
{
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_NO_RETURN(data, exit_group, status)
}
#endif
