#include "libiotrace_config.h"

//##################

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sysinfo.h>
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

//##################
#include <assert.h>

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

#include "os.h"
#include "event.h"

#include "libiotrace.h"
#include "json_include_function.h"

/* defines for exec-functions */
#ifndef MAX_EXEC_ARRAY_LENGTH
#define MAX_EXEC_ARRAY_LENGTH 1000
#endif

#ifndef MAX_INFLUX_TOKEN
#define MAX_INFLUX_TOKEN 200
#endif

#ifndef MAX_DATABSE_IP
#define MAX_DATABASE_IP 200
#endif

#ifndef MAX_DATABSE_PORT
#define MAX_DATABASE_PORT 200
#endif

/* flags and values to control logging */
#ifdef LOGGING
static ATTRIBUTE_THREAD char no_logging = 0;
#else
static ATTRIBUTE_THREAD char no_logging = 1;
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
static char influx_token[MAX_INFLUX_TOKEN];
static char database_ip[MAX_DATABASE_IP];
static char database_port[MAX_DATABASE_PORT];
static char whitelist[MAXFILENAME];
static char event_cleanup_done = 0;
#ifndef IO_LIB_STATIC
static char ld_preload[MAXFILENAME + sizeof(env_ld_preload)];
static char log_name_env[MAXFILENAME + sizeof(env_log_name)];
static char database_port_env[MAX_DATABASE_PORT + sizeof(env_database_port)];
static char database_ip_env[MAX_DATABASE_PORT + sizeof(env_database_ip)];
static char influx_token_env[MAX_DATABASE_PORT + sizeof(env_influx_token)];
static char whitelist_env[MAXFILENAME + sizeof(env_wrapper_whitelist)];
#endif
static long long system_start_time;
static SOCKET *recv_sockets = NULL;
static int recv_sockets_len = 0;
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

char libio_execve = WRAPPER_ACTIVE;
char libio_execv = WRAPPER_ACTIVE;
char libio_execl = WRAPPER_ACTIVE;
char libio_execvp = WRAPPER_ACTIVE;
char libio_execlp = WRAPPER_ACTIVE;
char libio_execvpe = WRAPPER_ACTIVE;
char libio_execle = WRAPPER_ACTIVE;
char libio__exit = WRAPPER_ACTIVE;
char libio__Exit = WRAPPER_ACTIVE;
char libio_exit_group = WRAPPER_ACTIVE;

void save_socket(SOCKET socket)
{
	void *ret;
	pthread_mutex_lock(&socket_lock);
	recv_sockets_len++;
	ret = realloc(recv_sockets, sizeof(SOCKET) * recv_sockets_len);
	if (NULL == ret)
	{
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr, "realloc() failed. (%d)\n", ret);
		free(recv_sockets);
		assert(0);
	}
	recv_sockets = ret;
	recv_sockets[recv_sockets_len - 1] = socket;
	pthread_mutex_unlock(&socket_lock);
}

//Attention: Function not thread-safe
//Should only be used inside of block guarded by mutex 'socket_lock'
void delete_socket(SOCKET socket)
{
	void *ret;
	recv_sockets_len--;
	if (recv_sockets[recv_sockets_len] == socket)
	{
		//Delete last element if last element is current socket
		ret = realloc(recv_sockets, sizeof(SOCKET) * recv_sockets_len);
		if (recv_sockets_len == 0)
		{
			if (ret != NULL)
			{
				free(ret);
			}
			recv_sockets = NULL;
		}
		else if (NULL == ret)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr, "realloc() failed. (%d)\n", ret);
			free(recv_sockets);
			assert(0);
		}
		else
		{
			recv_sockets = ret;
		}
	}
	else
	{
		for (int i = 0; i < recv_sockets_len; i++)
		{
			if (recv_sockets[i] == socket)
			{
				recv_sockets[i] = recv_sockets[recv_sockets_len];
				break;
			}
		}
		ret = realloc(recv_sockets, sizeof(SOCKET) * recv_sockets_len);
		if (recv_sockets_len == 0)
		{
			if (ret != NULL)
			{
				free(ret);
			}
			recv_sockets = NULL;
		}
		else if (NULL == ret)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr, "realloc() failed. (%d)\n", ret);
			free(recv_sockets);
			assert(0);
		}
		else
		{
			recv_sockets = ret;
		}
	}
}

#ifndef IO_LIB_STATIC
static char event_init_done = 0;

/* Initialize pointers for glibc functions. */
void event_init()
{
	if (!event_init_done)
	{

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

void activate_event_wrapper(char *line)
{
	char ret = 1;

	if (!strcmp(line, "")) {
		ret = 0;
	}
	WRAPPER_ACTIVATE(line, execve)
	WRAPPER_ACTIVATE(line, execv)
	WRAPPER_ACTIVATE(line, execl)
	WRAPPER_ACTIVATE(line, execvp)
	WRAPPER_ACTIVATE(line, execlp)
	WRAPPER_ACTIVATE(line, execvpe)
	WRAPPER_ACTIVATE(line, execlp)
	WRAPPER_ACTIVATE(line, execle)
	WRAPPER_ACTIVATE(line, _exit)
	WRAPPER_ACTIVATE(line, _Exit)
	WRAPPER_ACTIVATE(line, exit_group)
	else
	{
		ret = 0;
	}
#ifdef WITH_POSIX_IO
	if (!ret)
	{
		ret = activate_posix_wrapper(line);
	}
#endif
#ifdef WITH_MPI_IO
	if (!ret)
	{
		ret = activate_mpi_wrapper(line);
	}
#endif
#ifdef WITH_POSIX_AIO
	if (!ret)
	{
		ret = activate_posix_aio_wrapper(line);
	}
#endif
#ifdef WITH_DL_IO
	if (!ret)
	{
		ret = activate_dl_wrapper(line);
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

//Create socket per thread
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
	save_socket(socket_peer);
}

void libiotrace_start_log()
{
	no_logging = 0;
}

void libiotrace_end_log()
{
	no_logging = 1;
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

void get_filesystem()
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: open() returned %d.\n", __func__, fd);
			assert(0);
		}
	}

	file = setmntent("/proc/mounts", "r");
	if (NULL == file)
	{
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: setmntent() returned NULL with errno=%d.\n",
		 __func__, errno);
		assert(0);
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: getmntent() returned mnt_dir too long (%d bytes) for buffer.\n",
			 __func__, ret);
			assert(0);
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: dprintf() returned %d with errno=%d.\n",
			 __func__, ret, errno);
			assert(0);
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: fstat() returned %d with errno=%d.\n",
			 __func__, ret, errno);
			assert(0);
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
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: stat() returned %d with errno=%d.\n", __func__,
		 ret, errno);
		assert(0);
	}

	data->device_id = stat_data.st_dev;
	data->inode_nr = stat_data.st_ino;
}

void get_directories()
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
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: getcwd() returned NULL with errno=%d.\n",
		 __func__, errno);
		assert(0);
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
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: open() of file %s returned %d.\n", __func__,
		 working_dir_log_name, fd);
		assert(0);
	}

	json_struct_print_working_dir(buf_working_dir, sizeof(buf_working_dir),
								  &working_dir_data);
	ret_int = dprintf(fd, "%s\n", buf_working_dir); //TODO: CALL_REAL_POSIX_SYNC(dprintf)
	if (0 > ret_int)
	{
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: dprintf() returned %d with errno=%d.\n",
		 __func__, ret_int, errno);
		assert(0);
	}

	CALL_REAL_POSIX_SYNC(close)
	(fd);
}

void clear_init()
{
	init_done = 0;
	tid = -1;
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

	init_basic();

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

void *recvData(void *arg)
{
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
		int ret = CALL_REAL_POSIX_SYNC(select)(socket_max + 1, &fd_recv_sockets, NULL, NULL, NULL); //Wait for data
		if (-1 == ret)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: Select returned -1 (%d).\n",
			 __func__,
			 errno);
			break;
		}
		pthread_mutex_lock(&socket_lock);
		for (int i = 0; i < recv_sockets_len; i++)
		{
			//Which sockets are ready to read
			if (FD_ISSET(recv_sockets[i], &fd_recv_sockets))
			{
				char read[4096];
				ssize_t bytes_received = recv(socket_peer, read, 4096, 0);
				if (1 > bytes_received)
				{
					//Socket is destroyed or closed by peer
					//close(recv_sockets[i]);
					delete_socket(recv_sockets[i]);
					i--;
				}
				else
				{
					//TODO: Read requests to control wrappers
				}
			}
		}
		pthread_mutex_unlock(&socket_lock);
	}
	return NULL;
}

char init_done = 0;
void init_basic()
{
	char *log;
	int length;

	if (!init_done)
	{
		pos = data_buffer;
		count_basic = 0;

#if !defined(IO_LIB_STATIC)
		init_wrapper();
#endif

#if !defined(HAVE_HOST_NAME_MAX)
		host_name_max = sysconf(POSIX_HOST_NAME_MAX);
#endif

		pid = getpid();
		gethostname(hostname, HOST_NAME_MAX);

		log = getenv(env_log_name);
		if (NULL == log)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: function getenv(\"%s\") returned NULL.\n",
			 __func__, env_log_name);
			assert(0);
		}
		length = strlen(log);
		if (MAXFILENAME < length + 17 + strlen(hostname))
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: getenv() returned %s too long (%d bytes) for buffer.\n",
			 __func__, env_log_name, length);
			assert(0);
		}
		strcpy(log_name, log);
		strcpy(filesystem_log_name, log);
		strcpy(working_dir_log_name, log);
		strcpy(log_name + length, "_iotrace.log");
		strcpy(filesystem_log_name + length, "_filesystem_");
		strcpy(filesystem_log_name + length + 12, hostname);
		strcpy(filesystem_log_name + length + 12 + strlen(hostname), ".log");
		strcpy(working_dir_log_name + length, "_working_dir.log");

		// get token from environment
#ifndef IO_LIB_STATIC
		strcpy(log_name_env, env_log_name);
		strcpy(log_name_env + length, "=");
		strcpy(log_name_env + length + 1, log);
#endif
		log = getenv(env_influx_token);
		if (NULL == log)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: function getenv(\"%s\") returned NULL.\n",
			 __func__, env_influx_token);
			assert(0);
		}
		length = strlen(log);
		if (MAX_INFLUX_TOKEN < length)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: getenv() returned %s too long (%d bytes) for buffer.\n",
			 __func__, env_influx_token, length);
			assert(0);
		}
		strcpy(influx_token, log);

#ifndef IO_LIB_STATIC
		strcpy(influx_token_env, env_influx_token);
		strcpy(influx_token_env + length, "=");
		strcpy(influx_token_env + length + 1, influx_token);
#endif

		// get database ip from environment
		log = getenv(env_database_ip);
		if (NULL == log)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: function getenv(\"%s\") returned NULL.\n",
			 __func__, env_database_ip);
			assert(0);
		}
		length = strlen(log);
		if (MAX_DATABASE_IP < length)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: getenv() returned %s too long (%d bytes) for buffer.\n",
			 __func__, env_database_ip, length);
			assert(0);
		}
		strcpy(database_ip, log);

#ifndef IO_LIB_STATIC
		strcpy(database_ip_env, env_database_ip);
		strcpy(database_ip_env + length, "=");
		strcpy(database_ip_env + length + 1, database_ip);
#endif
		// get database port from environment
		log = getenv(env_database_port);
		if (NULL == log)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: function getenv(\"%s\") returned NULL.\n",
			 __func__, env_database_port);
			assert(0);
		}
		length = strlen(log);
		if (MAX_DATABASE_IP < length)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: getenv() returned %s too long (%d bytes) for buffer.\n",
			 __func__, env_database_port, length);
			assert(0);
		}
		strcpy(database_port, log);
#ifndef IO_LIB_STATIC
		strcpy(database_port_env, env_database_port);
		strcpy(database_port_env + length, "=");
		strcpy(database_port_env + length + 1, database_port);
#endif
		// Path to wrapper whitelist
		log = getenv(env_wrapper_whitelist);
		if (NULL != log)
		{
			length = strlen(log);
			if (MAXFILENAME < length)
			{
				CALL_REAL_POSIX_SYNC(fprintf)
				(stderr,
				 "In function %s: getenv() returned %s too long (%d bytes) for buffer.\n",
				 __func__, env_wrapper_whitelist, length);
				assert(0);
			}
			strcpy(whitelist, log);
#ifndef IO_LIB_STATIC
			strcpy(whitelist_env, env_wrapper_whitelist);
			strcpy(whitelist_env + length, "=");
			strcpy(whitelist_env + length + 1, whitelist);
#endif
			//Hier Auslesen der Whitelist
			FILE *stream;
			char *line = NULL;
			size_t len = 0;
			ssize_t nread;
			stream = CALL_REAL_POSIX_SYNC(fopen)(whitelist, "r");
			if (stream == NULL)
			{
				CALL_REAL_POSIX_SYNC(fprintf)
				(stderr,
				 "In function %s: fopen() failed (ERRNO: %d)\n",
				 __func__, errno);
				assert(0);
			}

			while ((nread = CALL_REAL_POSIX_SYNC(getline)(&line, &len, stream)) != -1)
			{
				size_t byte_count = strlen(line);
				if (byte_count > 0 && line[byte_count - 1] == '\n')
				{
					line[byte_count - 1] = '\0';
				}
				activate_event_wrapper(line);
			}

			free(line);
			CALL_REAL_POSIX_SYNC(fclose)
			(stream);
		}

#ifndef IO_LIB_STATIC

		log = getenv(env_ld_preload);
		if (NULL == log)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: function getenv(\"%s\") returned NULL.\n",
			 __func__, env_ld_preload);
			assert(0);
		}
		length = strlen(env_ld_preload);
		strcpy(ld_preload, env_ld_preload);
		strcpy(ld_preload + length, "=");
		strcpy(ld_preload + length + 1, log);
#endif

		struct sysinfo info;
		int errret = sysinfo(&info);
		if (errret != 0)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: sysinfo() returned %d.\n", __func__, errret);
			assert(0);
		}
		time_t current_time = time(NULL);
		system_start_time = (current_time - info.uptime) * 1000000000;

		pthread_mutex_init(&lock, NULL);
		pthread_mutex_init(&socket_lock, NULL);

		pthread_atfork(NULL, NULL, clear_init);

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
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: malloc() returned NULL.\n", __func__);
		assert(0);
	}

	size = backtrace(trace, stacktrace_depth + 3);
	if (0 >= size)
	{
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: backtrace() returned %d.\n", __func__, size);
		assert(0);
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: backtrace_symbols() returned NULL with errno=%d.\n",
			 __func__, errno);
			assert(0);
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

void get_basic(struct basic *data)
{
	// lock write on tid
	if (tid == -1)
	{
		tid = iotrace_gettid();
		prepare_socket();
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
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: open() of file %s returned %d.\n", __func__,
		 log_name, fd);
		assert(0);
	}

	for (int i = 0; i < count_basic; i++)
	{
		data = (struct basic *)((void *)pos);

		ret = json_struct_print_basic(buf, sizeof(buf), data); //Function is present at runtime, built with macros from json_defines.h
		ret = dprintf(fd, "%s\n", buf);						   //TODO: CALL_REAL_POSIX_SYNC(dprintf)
		if (0 > ret)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: dprintf() returned %d.\n", __func__, ret);
			assert(0);
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
	if (event_cleanup_done)
	{
		return;
	}
	//printf("Ich lauf rein\n");
	//riesen buffer fuer alles

	char message[400 + json_struct_push_max_size_basic(0) + 1];

	//buffer fuer body
	char buf[json_struct_push_max_size_basic(0) + 1]; /* +1 for trailing null character   Funktion wird von Markos gebaut   Groesse vom Body zum senden*/
	int ret = json_struct_push_basic(buf, sizeof(buf), data, "");
	buf[strlen(buf) - 1] = '\0'; /*remove last comma*/

	if (0 > ret)
	{
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: json_struct_push_basic() returned %d.\n", __func__, ret);
		assert(0);
	}
	//printf("Testlauf 1\n");
	char labels[200];
	char short_log_name[50];
	strncpy(short_log_name, log_name, sizeof(short_log_name));
	snprintf(labels, sizeof(labels), "libiotrace,jobname=%s,hostname=%s,processid=%u,thread=%u,functionname=%s", short_log_name, data->hostname, data->process_id, data->thread_id, data->function_name);

	char timestamp[50];

	snprintf(timestamp, sizeof(timestamp), "%lld", system_start_time + data->time_end);

	snprintf(message, sizeof(message), "POST /api/v2/write?bucket=hsebucket&precision=ns&org=hse HTTP/1.1\nHost: localhost:8086\nAccept: */*\n"
									   "Authorization: Token %s\nContent-Length: %ld\nContent-Type: application/x-www-form-urlencoded\n\n%s %s %s",
			 influx_token, strlen(labels) + 1 /*space*/ + ret - 1 /*last comma in ret*/ + 1 + strlen(timestamp), labels, buf, timestamp);
	//printf("Testlauf 2\n");

	// CALL_REAL_POSIX_SYNC(fprintf)
	// 		(stderr,
	// 		 "Socket: %d\n", socket_peer);
	// CALL_REAL_POSIX_SYNC(fprintf)
	// (stderr,
	//  "Message content: %s\n", message);

	char *message_to_send = message;
	size_t bytes_to_send = strlen(message);

	while (bytes_to_send > 0)
	{

		int bytes_sent = send(socket_peer, message_to_send, bytes_to_send, 0);
		if (-1 == bytes_sent)
		{
			if (errno == EWOULDBLOCK || errno == EAGAIN)
			{
				CALL_REAL_POSIX_SYNC(fprintf)
				(stderr,
				 "Send buffer is full. Please increase your limit. Data is lost.\n");
			}
			else
			{

				CALL_REAL_POSIX_SYNC(fprintf)
				(stderr,
				 "In function %s: send() returned %d, errno: %d.\n", __func__, bytes_sent, errno);
				assert(0);
			}
		}
		else
		{
			if (bytes_sent < bytes_to_send)
			{
				// CALL_REAL_POSIX_SYNC(fprintf)
				// (stderr,
				//  "Seems that send doesn't return enough bytes.....\n");
				// CALL_REAL_POSIX_SYNC(fflush)
				// (stderr);
				bytes_to_send -= bytes_sent;
				message_to_send += bytes_sent;
			}
			else
			{
				bytes_to_send = 0;
			}
		}
		//printf("errno:%d\n", errno);
		//printf("Sent %d bytes.\n", bytes_sent);
	}
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
		// ToDo: solve circular dependency
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: buffer (%ld bytes) not big enough for even one struct basic (%d bytes).\n",
		 __func__, sizeof(data_buffer), length);
		assert(0);
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

	pthread_mutex_destroy(&lock);
	pthread_mutex_destroy(&socket_lock);

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
		CALL_REAL_POSIX_SYNC(fprintf)
		(stderr,
		 "In function %s: envp[] has more elements then buffer (%d).\n",
		 func,
		 MAX_EXEC_ARRAY_LENGTH);
		assert(0);
	}

	if (!has_ld_preload)
	{
		if (MAX_EXEC_ARRAY_LENGTH <= env_element + 6)
		{
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
			 "In function %s: envp[] with added libiotrace-variables has more elements then buffer (%d).\n",
			 func,
			 MAX_EXEC_ARRAY_LENGTH);
			assert(0);
		}
		env[env_element] = &ld_preload[0];
		env[++env_element] = &log_name_env[0];
		env[++env_element] = &database_ip_env[0];
		env[++env_element] = &database_port_env[0];
		env[++env_element] = &influx_token_env[0];
		env[++env_element] = &whitelist_env[0];
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
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
	CALL_REAL_POSIX_SYNC(fprintf)
	(stderr,
	 "In function %s: wrapper needs function execvpe() to work properly.\n",
	 __func__);
	assert(0);
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
			CALL_REAL_POSIX_SYNC(fprintf)
			(stderr,
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
