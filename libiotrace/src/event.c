#include "libiotrace_config.h"

//##################

#include <sys/types.h>
#include <sys/socket.h>
//#include <sys/sysinfo.h>
#include <time.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>
#include <limits.h>
#include <ctype.h>

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
#include <mntent.h>
#include <stdarg.h>
#include <fcntl.h>

#include <execinfo.h>

#include "error.h"

#include "llhttp/llhttp.h"

#include "os.h"
#include "event.h"

#include "libiotrace.h"
#include "json_include_function.h"

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

/* Buffer */
#ifndef BUFFER_SIZE
#define BUFFER_SIZE 1048576 // 1 MB
#endif
static char data_buffer[BUFFER_SIZE];
static const char *endpos = data_buffer + BUFFER_SIZE;
static char *pos;
static int count_basic;

// Todo: multiple definition of host_name_max in libiotrace_config.h and here?
#if !defined(HAVE_HOST_NAME_MAX)
static int host_name_max;
#endif

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
static char hostname[HOST_NAME_MAX];
static char log_name[MAXFILENAME];
static char filesystem_log_name[MAXFILENAME];
static char working_dir_log_name[MAXFILENAME];
static char control_log_name[MAXFILENAME];
static char influx_token[MAX_INFLUX_TOKEN];
static char influx_organization[MAX_INFLUX_ORGANIZATION];
static char influx_bucket[MAX_INFLUX_BUCKET];
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
#endif
static u_int64_t system_start_time;
static SOCKET *recv_sockets = NULL;
static int recv_sockets_len = 0;
static SOCKET *open_control_sockets = NULL;
static int open_control_sockets_len = 0;
static SOCKET socket_control;

// once per thread
static ATTRIBUTE_THREAD pid_t tid = -1;
static ATTRIBUTE_THREAD SOCKET socket_peer;

void cleanup() ATTRIBUTE_DESTRUCTOR;
void *recvData(void *arg);

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

struct wrapper_status active_wrapper_status;

void save_socket(SOCKET socket, pthread_mutex_t *lock, int *len, SOCKET **array)
{
	void *ret;

	if (NULL != lock)
	{
		pthread_mutex_lock(lock);
	}

	(*len)++;
	ret = realloc(*array, sizeof(SOCKET) * (*len));
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

void delete_socket(SOCKET socket, pthread_mutex_t *lock, int *len, SOCKET **array)
{
	void *ret;

	if (NULL != lock)
	{
		pthread_mutex_lock(lock);
	}

	(*len)--;
	if ((*array)[*len] == socket)
	{
		//Delete last element if last element is current socket
		ret = realloc(*array, sizeof(SOCKET) * (*len));
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
		for (int i = 0; i < *len; i++)
		{
			if ((*array)[i] == socket)
			{
				(*array)[i] = (*array)[*len];
				break;
			}
		}
		ret = realloc(*array, sizeof(SOCKET) * (*len));
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

/* Initialize pointers for glibc functions. */
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

void toggle_event_wrapper(char *line, char toggle)
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

// Create socket per thread
void prepare_socket()
{
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
		fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
		return;
	}
	socket_peer = CALL_REAL_POSIX_SYNC(socket)(peer_address->ai_family,
											   peer_address->ai_socktype, peer_address->ai_protocol);

#endif

	if (!ISVALIDSOCKET(socket_peer))
	{
		fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
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
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
		return;
	}
#else
	if (connect(socket_peer,
				peer_address->ai_addr, peer_address->ai_addrlen))
	{
		fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
		return;
	}
	freeaddrinfo(peer_address);
#endif

	// save socket globally to create thread that listens to all sockets
	save_socket(socket_peer, &socket_lock, &recv_sockets_len, &recv_sockets);
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

/**
 * Prints the filesystem to a file.
 *
 * The file is given in the global variable #filesystem_log_name which is
 * set from environment variable #env_log_name. Each mount point is printed
 * on a new line. The printed line shows the device, the mount point, the
 * file-system type and the mount options, the dump frequency in days and
 * the mount passno as a json object.
 */
void print_filesystem()
{
	FILE *file;
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
		json_struct_print_filesystem(buf_filesystem, sizeof(buf_filesystem),
									 &filesystem_data);
		ret = dprintf(fd, "%s\n", buf_filesystem); //TODO: CALL_REAL_POSIX_SYNC(dprintf)
		if (0 > ret)
		{
			LIBIOTRACE_ERROR("dprintf() returned %d with errno=%d", ret, errno);
		}
	}

	endmntent(file);

	CALL_REAL_POSIX_SYNC(close)
	(fd);
}

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
	char buf_working_dir[json_struct_max_size_working_dir() + 1]; /* +1 for trailing null character */
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

	json_struct_print_working_dir(buf_working_dir, sizeof(buf_working_dir),
								  &working_dir_data);
	ret_int = dprintf(fd, "%s\n", buf_working_dir); //TODO: CALL_REAL_POSIX_SYNC(dprintf)
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

void open_std_fd(int fd)
{
	struct basic data;
	struct file_descriptor file_descriptor_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
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

	writeData(&data);
	WRAP_FREE(&data)
}

void open_std_file(FILE *file)
{
	struct basic data;
	struct file_stream file_stream_data;

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)
	file_stream_data.stream = file;

	data.time_start = gettime();
	data.time_end = data.time_start;
#ifdef LOG_WRAPPER_TIME
	data.wrapper.time_start = data.time_start;
	//data.wrapper.time_end = 0;
#endif

	data.return_state = ok;
	data.return_state_detail = NULL;

	writeData(&data);
	WRAP_FREE(&data)
}

//Compiler --> Add this function to CTOR section in elf binary
// Execute this function before main() of program observing
void init_on_load() ATTRIBUTE_CONSTRUCTOR;

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
#endif

	WRAPPER_TIME_END(data);

#ifdef LOG_WRAPPER_TIME
	writeData(&data);
	WRAP_FREE(&data)
#endif
}

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
				CALL_REAL_POSIX_SYNC(fprintf)
				(stderr,
				 "Send buffer is full. Please increase your limit.\n");
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

int my_url_callback(llhttp_t *parser, const char *at, size_t length)
{

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
				toggle_event_wrapper(functionname, 1);
			}
			else
			{
				toggle_event_wrapper(functionname, 0);
			}
			// TODO: \r\n instead of \n
			send_data("HTTP/1.1 204 No Content\n\n\n", socket_peer);
		}
		else
		{
			send_data("HTTP/1.1 400 Bad Request\n\n\n", socket_peer);
		}
	}
	else if (parser->method == HTTP_GET)
	{
		const char *message_header = "HTTP/1.1 200 OK\r\nContent-Length: 0123456789\r\nContent-Type: application/json\r\n\r\n";
		const int message_header_len = strlen(message_header);
		char message[message_header_len + json_struct_max_size_wrapper_status() + 1];

		// buffer for body
		char buf[json_struct_max_size_wrapper_status() + 1];
		int ret = json_struct_print_wrapper_status(buf, sizeof(buf), &active_wrapper_status);
		if (0 > ret)
		{
			LIBIOTRACE_ERROR("json_struct_print_wrapper_status() returned %d", ret);
		}

		// TODO: use message_header instead of string
		snprintf(message, sizeof(message), "HTTP/1.1 200 OK\r\nContent-Length: %ld\nContent-Type: application/json\n\n%s", strlen(buf), buf);

		send_data(message, socket_peer);
	}
	else
	{
		send_data("HTTP/1.1 405 Method Not Allowed\n\n\n", socket_peer);
	}

	return 0;
}

void *recvData(void *arg)
{

	llhttp_t parser;
	llhttp_settings_t settings;

	/* Initialize user callbacks and settings */
	llhttp_settings_init(&settings);

	/* Set user callback */
	settings.on_url = my_url_callback;

	llhttp_init(&parser, HTTP_BOTH, &settings);

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
				int ret = dprintf(fd, "(%u) %s: %s:%d\n", pid, ifreqs[l].ifr_name,
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
			FD_SET(recv_sockets[i], &fd_recv_sockets);
			if (recv_sockets[i] > socket_max)
			{
				socket_max = recv_sockets[i];
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
			FD_SET(open_control_sockets[i], &fd_recv_sockets);
			if (open_control_sockets[i] > socket_max)
			{
				socket_max = open_control_sockets[i];
			}
		}

		int ret = CALL_REAL_POSIX_SYNC(select)(socket_max + 1, &fd_recv_sockets, NULL, NULL, NULL);
		// Select: At least one socket is ready to be processed
		if (-1 == ret)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: Select returned -1 (%d).\n",
			 __func__,
			 errno);
			break;
		}

		// receive responses from influxdb
		pthread_mutex_lock(&socket_lock);
		for (int i = 0; i < recv_sockets_len; i++)
		{
			//Which sockets are ready to read
			if (FD_ISSET(recv_sockets[i], &fd_recv_sockets))
			{
				char read[4096];
				ssize_t bytes_received = recv(recv_sockets[i], read, 4096, 0);
				if (1 > bytes_received)
				{
					//Socket is destroyed or closed by peer
					//close(recv_sockets[i]);
					//delete_socket(recv_sockets[i]);
					//i--;
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
			save_socket(socket, NULL, &open_control_sockets_len, &open_control_sockets);
		}

		// receive control requests
		for (int i = 0; i < open_control_sockets_len; i++)
		{
			//Which sockets are ready to read
			if (FD_ISSET(open_control_sockets[i], &fd_recv_sockets))
			{
				char read[4096];
				ssize_t bytes_received = recv(open_control_sockets[i], read, 4096, 0);
				if (1 > bytes_received)
				{
					//Socket is destroyed or closed by peer
					close(open_control_sockets[i]);
					delete_socket(open_control_sockets[i], NULL, &open_control_sockets_len, &open_control_sockets);
					i--;
				}
				else
				{
					socket_peer = open_control_sockets[i]; // is needed by callback => must be set before llhttp_execute()
					enum llhttp_errno err = llhttp_execute(&parser, read, bytes_received);
					if (err != HPE_OK)
					{
						const char *errno_text = llhttp_errno_name(err);
						snprintf(read, sizeof(read), "HTTP/1.1 400 Bad Request\nContent-Length: %ld\nContent-Type: application/json\n\n%s: %s",
								 strlen(errno_text) + 2 + strlen(parser.reason), errno_text, parser.reason);
						send_data(read, socket_peer);
					}
				}
			}
		}
	}
	CLOSESOCKET(socket_control);
	for (int i = 0; i < open_control_sockets_len; i++)
	{
		CLOSESOCKET(open_control_sockets[i]);
	}
	return NULL;
}

int libiotrace_get_env(const char *env_name, char *dst, const int max_len, const char error_if_not_exists) {
	char *log;
	int length;

	log = getenv(env_name);
	if (NULL == log)
	{
		if (error_if_not_exists) {
			LIBIOTRACE_ERROR("getenv(\"%s\") returned NULL", env_name);
		} else {
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

char init_done = 0;
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

#if !defined(IO_LIB_STATIC)
		init_wrapper();
#endif

#if !defined(HAVE_HOST_NAME_MAX)
		host_name_max = sysconf(POSIX_HOST_NAME_MAX);
#endif

		pid = getpid();
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
		strcpy(log_name_env, env_log_name);
		strcpy(log_name_env + length, "=");
		strcpy(log_name_env + length + 1, log_name);
#endif

		// get token from environment
		length = libiotrace_get_env(env_influx_token, influx_token, MAX_INFLUX_TOKEN, 1);

#ifndef IO_LIB_STATIC
		strcpy(influx_token_env, env_influx_token);
		strcpy(influx_token_env + length, "=");
		strcpy(influx_token_env + length + 1, influx_token);
#endif

		// get bucket name from environment
		length = libiotrace_get_env(env_influx_bucket, influx_bucket, MAX_INFLUX_BUCKET, 1);

#ifndef IO_LIB_STATIC
		strcpy(influx_bucket_env, env_influx_bucket);
		strcpy(influx_bucket_env + length, "=");
		strcpy(influx_bucket_env + length + 1, influx_bucket);
#endif

		// get organization name from environment
		length = libiotrace_get_env(env_influx_organization, influx_organization, MAX_INFLUX_ORGANIZATION, 1);

#ifndef IO_LIB_STATIC
		strcpy(influx_organization_env, env_influx_organization);
		strcpy(influx_organization_env + length, "=");
		strcpy(influx_organization_env + length + 1, influx_organization);
#endif

		// get database ip from environment
		length = libiotrace_get_env(env_database_ip, database_ip, MAX_DATABASE_IP, 1);

#ifndef IO_LIB_STATIC
		strcpy(database_ip_env, env_database_ip);
		strcpy(database_ip_env + length, "=");
		strcpy(database_ip_env + length + 1, database_ip);
#endif
		// get database port from environment
		length = libiotrace_get_env(env_database_port, database_port, MAX_DATABASE_PORT, 1);

#ifndef IO_LIB_STATIC
		strcpy(database_port_env, env_database_port);
		strcpy(database_port_env + length, "=");
		strcpy(database_port_env + length + 1, database_port);
#endif
		// Path to wrapper whitelist
		length = libiotrace_get_env(env_wrapper_whitelist, whitelist, MAXFILENAME, 0);
		if (0 != length)
		{
#ifndef IO_LIB_STATIC
			strcpy(whitelist_env, env_wrapper_whitelist);
			strcpy(whitelist_env + length, "=");
			strcpy(whitelist_env + length + 1, whitelist);
#endif
			//Hier Auslesen der Whitelist
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

					toggle_event_wrapper(clean_line, 1);
				}
			}

			free(line);
			CALL_REAL_POSIX_SYNC(fclose)
			(stream);
		}

#ifndef IO_LIB_STATIC
		length = strlen(env_ld_preload);
		strcpy(ld_preload, env_ld_preload);
		strcpy(ld_preload + length, "=");
		length = libiotrace_get_env(env_ld_preload, ld_preload + length + 1, MAXFILENAME, 1);
#endif

		// TODO: time() - uptime is false if there was a sleep
		// instead read /var/run/utmp as sequence of utmp sturcts from the newest to the oldest entry
		// until ut_type has the value BOOT_TIME => in the last struct ut_tv holds the boot_time
		// see https://stackoverflow.com/questions/26333279/reading-the-linux-utmp-file-without-using-fopen-and-fread
//		struct sysinfo info;
//		int errret = sysinfo(&info);
//		if (errret != 0)
//		{
//			LIBIOTRACE_ERROR("sysinfo() returned %d", errret);
//		}
//		time_t current_time = time(NULL);
//		system_start_time = (current_time - info.uptime) * 1000000000;
		system_start_time = iotrace_get_boot_time();

		pthread_mutex_init(&lock, NULL);
		pthread_mutex_init(&socket_lock, NULL);

		pthread_atfork(NULL, NULL, reset_values_in_forked_process);

		print_filesystem();
		print_working_directory();

#ifdef WITH_STD_IO
		open_std_fd(STDIN_FILENO);
		open_std_fd(STDOUT_FILENO);
		open_std_fd(STDERR_FILENO);

		open_std_file(stdin);
		open_std_file(stdout);
		open_std_file(stderr);
#endif

		//Create receive thread per process
		pthread_t recv_thread;

		int ret = pthread_create(&recv_thread, NULL, recvData, NULL);
		if (0 != ret)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr, "pthread_create() failed. (%d)\n", ret);
			return;
		}
		init_done = 1;
	}
}

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
		JSON_STRUCT_SET_MALLOC_PTR_ARRAY((*data), stacktrace_pointer, trace, 3,
										 size)
	}
	else
	{
		JSON_STRUCT_SET_MALLOC_PTR_ARRAY_NULL((*data), stacktrace_pointer)
	}

	if (stacktrace_symbol)
	{
		messages = backtrace_symbols(trace, size);
		if (NULL == messages)
		{
			LIBIOTRACE_ERROR("backtrace_symbols() returned NULL with errno=%d", errno);
		}

		JSON_STRUCT_SET_MALLOC_STRING_ARRAY((*data), stacktrace_symbols,
											messages, 3, size)
	}
	else
	{
		JSON_STRUCT_SET_MALLOC_STRING_ARRAY_NULL((*data), stacktrace_symbols)
	}

	if (!stacktrace_ptr)
	{
		free(trace);
	}
}

void init_thread() {
	tid = iotrace_get_tid();
	prepare_socket();
}

void get_basic(struct basic *data)
{
	// lock write on tid
	if (tid == -1)
	{
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
		JSON_STRUCT_SET_MALLOC_STRING_ARRAY_NULL((*data), stacktrace_symbols)
		JSON_STRUCT_SET_MALLOC_PTR_ARRAY_NULL((*data), stacktrace_pointer)
	}
}

inline u_int64_t gettime(void)
{
	struct timespec t;
	u_int64_t time;
	clock_gettime(CLOCK_MONOTONIC_RAW, &t);
	time = t.tv_sec * (1000 * 1000 * 1000) + t.tv_nsec;
	return time;
}

void printData()
{
	struct basic *data;
	int ret;
	char buf[json_struct_max_size_basic() + 1]; /* +1 for trailing null character */
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

		ret = json_struct_print_basic(buf, sizeof(buf), data); //Function is present at runtime, built with macros from json_defines.h
		ret = dprintf(fd, "%s\n", buf);						   //TODO: CALL_REAL_POSIX_SYNC(dprintf)
		if (0 > ret)
		{
			LIBIOTRACE_ERROR("dprintf() returned %d", ret);
		}
		ret = json_struct_sizeof_basic(data);

		pos += ret;
	}

	CALL_REAL_POSIX_SYNC(close)
	(fd);

	pos = data_buffer;
	count_basic = 0;
}

void pushData(struct basic *data)
{
	if (event_cleanup_done || no_sending)
	{
		return;
	}

	//buffer all (header + body)
	char message[400 + json_struct_push_max_size_basic(0) + 1];

	//buffer for body
	char buf[json_struct_push_max_size_basic(0) + 1]; /* +1 for trailing null character (function build by macros; gives length of body to send) */
	int ret = json_struct_push_basic(buf, sizeof(buf), data, "");
	buf[strlen(buf) - 1] = '\0'; /*remove last comma*/

	if (0 > ret)
	{
		LIBIOTRACE_ERROR("json_struct_push_basic() returned %d", ret);
	}

	char labels[200];
	char short_log_name[50];
	strncpy(short_log_name, log_name, sizeof(short_log_name));
	snprintf(labels, sizeof(labels), "libiotrace,jobname=%s,hostname=%s,processid=%u,thread=%u,functionname=%s", short_log_name, data->hostname, data->process_id, data->thread_id, data->function_name);

	char timestamp[50];

	snprintf(timestamp, sizeof(timestamp), "%lld", system_start_time + data->time_end);

	snprintf(message, sizeof(message), "POST /api/v2/write?bucket=%s&precision=ns&org=%s HTTP/1.1\nHost: localhost:8086\nAccept: */*\n"
									   "Authorization: Token %s\nContent-Length: %ld\nContent-Type: application/x-www-form-urlencoded\n\n%s %s %s",
			 influx_bucket, influx_organization, influx_token, strlen(labels) + 1 /*space*/ + ret - 1 /*last comma in ret*/ + 1 + strlen(timestamp), labels, buf, timestamp);

	send_data(message, socket_peer);
}

void writeData(struct basic *data)
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

	int length = json_struct_sizeof_basic(data);

	if (pos + length > endpos)
	{
		printData();
	}
	if (pos + length > endpos)
	{
		// ToDo: solve circular dependency of fprintf
		LIBIOTRACE_ERROR("buffer (%ld bytes) not big enough for even one struct basic (%d bytes)", sizeof(data_buffer), length);
	}

#ifdef LOG_WRAPPER_TIME
	old_pos = pos;
#endif
	pos = (void *)json_struct_copy_basic((void *)pos, data);
	count_basic++;
	// insert end time for wrapper in buffer
	WRAPPER_TIME_END((*((struct basic *)((void *)old_pos))))

	pthread_mutex_unlock(&lock);
}

void freeMemory(struct basic *data)
{
	json_struct_free_basic(data);
}

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
	printData();
	pthread_mutex_unlock(&lock);

#ifdef LOG_WRAPPER_TIME
	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
#endif

	WRAPPER_TIME_END(data);

#ifdef LOG_WRAPPER_TIME
	writeData(&data);
	pthread_mutex_lock(&lock);
	printData();
	pthread_mutex_unlock(&lock);
	WRAP_FREE(&data)
#endif

	pthread_mutex_lock(&socket_lock);
	for (int i = 0; i < recv_sockets_len; i++)
	{
		shutdown(recv_sockets[i], SHUT_WR);
		while (1)
		{
			fd_set reads;
			FD_ZERO(&reads);
			FD_SET(recv_sockets[i], &reads);

			int ret = CALL_REAL_POSIX_SYNC(select)(recv_sockets[i] + 1, &reads, NULL, NULL, NULL);
			if (-1 == ret)
			{
				CALL_REAL_POSIX_SYNC(fprintf)
				(stderr,
				 "In function %s: Select returned -1 (%d).\n",
				 __func__,
				 errno);
				break;
			}
			if (FD_ISSET(recv_sockets[i], &reads))
			{
				char read[4096];
				int bytes_received = recv(recv_sockets[i], read, 4096, 0);
				if (bytes_received < 1)
				{
					//printf("Connection closed by peer...\n");
					break;
				}
			}
		}
		CLOSESOCKET(recv_sockets[i]);
	}
	pthread_mutex_unlock(&socket_lock);

	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&socket_lock);
}

#ifndef IO_LIB_STATIC
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
		LIBIOTRACE_ERROR("envp[] has more elements then buffer (%d)", MAX_EXEC_ARRAY_LENGTH);
	}

	if (!has_ld_preload)
	{
		if (MAX_EXEC_ARRAY_LENGTH <= env_element + 6)
		{
			LIBIOTRACE_ERROR("envp[] with added libiotrace-variables has more elements then buffer (%d)", MAX_EXEC_ARRAY_LENGTH);
		}
		env[env_element] = &ld_preload[0];
		env[++env_element] = &log_name_env[0];
		env[++env_element] = &database_ip_env[0];
		env[++env_element] = &database_port_env[0];
		env[++env_element] = &influx_token_env[0];
		env[++env_element] = &whitelist_env[0];
		// TODO: influx organization and bucket
		env[++env_element] = NULL;
	}
}
#endif

int WRAP(execve)(const char *filename, char *const argv[], char *const envp[])
{
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

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
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	data.return_state = ok;
	CALL_REAL_FUNCTION_NO_RETURN(data, _exit, status)
}

#ifdef HAVE_EXIT
void WRAP(_Exit)(int status)
{
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
void WRAP(exit_group)(int status)
{
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
