#include "libiotrace_config.h"

//##################

#include <sys/types.h>
#include <sys/socket.h>
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

#include "common/error.h"

#include "libs/llhttp/llhttp.h"

#include "os.h"
#include "event.h"
#include "common/gettime.h"
#include "common/utils.h"

#include "libiotrace.h"
#include "libiotrace_functions.h"

#include "wrapper_name.h"


#if !defined(WITH_ALLOC) && !defined(WITH_DL_IO) && !defined(WITH_MPI_IO) && !defined(WITH_POSIX_AIO) && !defined(WITH_POSIX_IO)
#  error "at least one group of wrappers must be included"
#endif

#if defined(STRACING_ENABLED) && !defined(WITH_POSIX_IO)
#  error "`STRACING_ENABLED` requires `WITH_POSIX_IO`"
#endif


/* defines for exec-functions */
#ifndef MAX_EXEC_ARRAY_LENGTH
#define MAX_EXEC_ARRAY_LENGTH 1000
#endif

/* defines for socket handling */
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
#define CLOSESOCKET(s)          \
    CALL_REAL_POSIX_SYNC(close) \
    (s)
#define SOCKET int
#endif
#if defined(IOTRACE_ENABLE_INFLUXDB)
#define ISVALIDSOCKET(s) ((s) >= 0)
#define GETSOCKETERRNO() (errno)
#endif

/* defines for influxdb connection */
#ifdef IOTRACE_ENABLE_INFLUXDB

#ifndef MAX_INFLUX_TOKEN
#define MAX_INFLUX_TOKEN 200
#endif

#ifndef MAX_INFLUX_BUCKET
#define MAX_INFLUX_BUCKET 200
#endif

#ifndef MAX_INFLUX_ORGANIZATION
#define MAX_INFLUX_ORGANIZATION 200
#endif

#ifndef MAX_DATABASE_IP
#define MAX_DATABASE_IP 200
#endif

#ifndef MAX_DATABASE_PORT
#define MAX_DATABASE_PORT 200
#endif

#endif

/* defines for control connection */
#ifdef ENABLE_REMOTE_CONTROL

#ifndef PORT_RANGE_MIN
#define PORT_RANGE_MIN 50000
#endif

#ifndef PORT_RANGE_MAX
#define PORT_RANGE_MAX 60000
#endif

#endif

/* defines for all connections */
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
#ifndef SELECT_TIMEOUT_SECONDS
#define SELECT_TIMEOUT_SECONDS 1
#endif
#endif

/* flags and values to control logging */
#ifdef IOTRACE_ENABLE_LOGFILE
#  ifdef LOGGING
static ATTRIBUTE_THREAD char no_logging = 0;
#  else
static ATTRIBUTE_THREAD char no_logging = 1;
#  endif
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
#  ifdef SENDING
static ATTRIBUTE_THREAD char no_sending = 0;
#  else
static ATTRIBUTE_THREAD char no_sending = 1;
#  endif
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

/* max length host name */
#if !defined(HAVE_HOST_NAME_MAX)
int host_name_max;
#endif

/* struct for socket and corresponding parser */
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
typedef struct libiotrace_sockets {
    SOCKET socket;
    llhttp_t parser;
} libiotrace_socket;
#endif

/* Mutex */
#ifdef IOTRACE_ENABLE_INFLUXDB
static pthread_mutex_t socket_lock;
#endif
#if defined(IOTRACE_ENABLE_INFLUXDB) && defined(ENABLE_REMOTE_CONTROL)
static pthread_mutex_t ip_lock;
#endif

/* environment variables   (NOTE: Uses const char array to ensure `sizeof` returns correct size) */
#if defined(IOTRACE_ENABLE_LOGFILE) \
    || defined(IOTRACE_ENABLE_INFLUXDB) /* log_name is also used to build short_log_name */
static const char ENV_LOG_NAME[] =  "IOTRACE_LOG_NAME";
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
static const char ENV_INFLUX_TOKEN[] =  "IOTRACE_INFLUX_TOKEN";
static const char ENV_INFLUX_ORGANIZATION[] =  "IOTRACE_INFLUX_ORGANIZATION";
static const char ENV_INFLUX_BUCKET[] =  "IOTRACE_INFLUX_BUCKET";
static const char ENV_DATABASE_IP[] =  "IOTRACE_DATABASE_IP";
static const char ENV_DATABASE_PORT[] =  "IOTRACE_DATABASE_PORT";
#endif
static const char ENV_WRAPPER_WHITELIST[] =  "IOTRACE_WHITELIST";
#ifndef IO_LIB_STATIC
static const char ENV_LD_PRELOAD[] =  "LD_PRELOAD";
#endif

// once per process
pid_t pid;
static char *hostname;

#if defined(IOTRACE_ENABLE_LOGFILE) \
    || defined(IOTRACE_ENABLE_INFLUXDB) /* log_name is also used to build short_log_name */
static char log_name[MAXFILENAME];
static int log_name_len;
#endif
#if defined(ENABLE_REMOTE_CONTROL) && defined(IOTRACE_ENABLE_LOGFILE)
static char control_log_name[MAXFILENAME];
#endif

#ifdef IOTRACE_ENABLE_LOGFILE
static char filesystem_log_name[MAXFILENAME];
static char working_dir_log_name[MAXFILENAME];
#endif

#ifdef IOTRACE_ENABLE_INFLUXDB
static char influx_token[MAX_INFLUX_TOKEN];
static int influx_token_len;
static char influx_organization[MAX_INFLUX_ORGANIZATION];
static int influx_organization_len;
static char influx_bucket[MAX_INFLUX_BUCKET];
static int influx_bucket_len;
static char database_ip[MAX_DATABASE_IP];
static int database_ip_len;
static char database_port[MAX_DATABASE_PORT];
static int database_port_len;

static libiotrace_socket **recv_sockets = NULL;
static int recv_sockets_len = 0;
#endif

static char whitelist[MAXFILENAME];

#if defined(IOTRACE_ENABLE_LOGFILE) || defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
static char event_cleanup_done = 0;
#endif

#ifndef IO_LIB_STATIC

static char ld_preload[MAXFILENAME + sizeof(ENV_LD_PRELOAD)];

#if defined(IOTRACE_ENABLE_LOGFILE) \
    || defined(IOTRACE_ENABLE_INFLUXDB) /* log_name is also used to build short_log_name */
static char log_name_env[MAXFILENAME + sizeof(ENV_LOG_NAME)];
#endif

#ifdef IOTRACE_ENABLE_INFLUXDB
static char database_port_env[MAX_DATABASE_PORT + sizeof(ENV_DATABASE_PORT)];
static char database_ip_env[MAX_DATABASE_IP + sizeof(ENV_DATABASE_IP)];
static char influx_token_env[MAX_INFLUX_TOKEN + sizeof(ENV_INFLUX_TOKEN)];
static char influx_organization_env[MAX_INFLUX_ORGANIZATION
        + sizeof(ENV_INFLUX_ORGANIZATION)];
static char influx_bucket_env[MAX_INFLUX_BUCKET + sizeof(ENV_INFLUX_BUCKET)];
#endif

static char whitelist_env[MAXFILENAME + sizeof(ENV_WRAPPER_WHITELIST)];
static char has_whitelist;

#endif

#ifndef REALTIME
static u_int64_t system_start_time;
#endif

#ifdef ENABLE_REMOTE_CONTROL

static libiotrace_socket **open_control_sockets = NULL;
static int open_control_sockets_len = 0;
static SOCKET socket_control;

#ifdef IOTRACE_ENABLE_INFLUXDB
static int libiotrace_control_port = -1;
static char local_ip[39] = "";
#endif

#endif

#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
static llhttp_settings_t settings;
#endif

struct wrapper_status active_wrapper_status;

// once per thread
ATTRIBUTE_THREAD pid_t tid = -1;

#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
static ATTRIBUTE_THREAD SOCKET socket_peer = -1;
#endif

#if defined(IOTRACE_ENABLE_LOGFILE) || defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
void cleanup_process(void) ATTRIBUTE_DESTRUCTOR;
#endif

#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
void* communication_thread(void *arg);
#endif

#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
void send_data(const char *message, SOCKET socket);
#endif

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
REAL_DEFINITION_TYPE int REAL_DEFINITION(pthread_create)(pthread_t *restrict thread,
const pthread_attr_t *restrict attr,
void *(*start_routine)(void *),
void *restrict arg) REAL_DEFINITION_INIT;
#endif

#ifdef FILENAME_RESOLUTION_ENABLED
#  include "fnres/fnres.h"

static const char* const FNRES_ENV_FNMAP_MAX_FNAMES = "IOTRACE_FNRES_MAX_FILENAMES";
static const long FNRES_DEFAULT_FNMAP_MAX_FNAMES = 100;
static const long FNRES_MAX_FNMAP_MAX_FNAMES = 10000;
#endif

#ifdef STRACING_ENABLED
#  include "stracing/libiotrace/entrypoint.h"
#endif




/*
 * POWER MEASUREMENT HEADER
 */
#ifdef ENABLE_POWER_MEASUREMENT

#include "power_measurement/defines.h"

void power_measurement_init(void);
void power_measurement_step(void);
void power_measurement_cleanup(void);

void get_cpu_info(int *family, int *model);
int power_measurement_get_number_of_max_cpu_count(void);
void power_measurement_load_cpu_info(void);
void power_measurement_free_cpu_info(void);

#ifdef  ENABLE_POWER_MEASUREMENT_RAPL
int rapl_init(int cpu_family, int cpu_model);
void rapl_create_task(CPUMeasurementTask *cpu_measurement_task, unsigned int cpu_id, unsigned int cpu_package_id, char *name, unsigned int offset_in_file, unsigned int type);
void rapl_free(void);
int rapl_open_file(unsigned int offset);
void rapl_measurement(void);
long long rapl_read_msr(int file_descriptor, unsigned int offset_in_file);
long long rapl_convert_energy(int type, long long value);
#endif
#endif



/**
 * Create a new libiotrace_socket.
 *
 * Dynamically allocates a new libiotrace_socket. Saves the
 * socket "s" and a new created parser for this socket in the
 * new libiotrace_socket and returns it.
 *
 * @param[in] s    The socket to save in a new libiotrace_socket.
 * @param[in] type llhttp_type_t for creating the new parser for "s".
 *
 * @return Pointer to a new created libiotrace_socket. Must be freed
 *         with "free" if no longer used.
 */
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
libiotrace_socket* create_libiotrace_socket(SOCKET s, llhttp_type_t type) {
    libiotrace_socket *socket = CALL_REAL_ALLOC_SYNC(malloc)(
    sizeof(libiotrace_socket));
    if (NULL == socket) {
        LOG_ERROR_AND_DIE("malloc failed, errno=%d", errno);
    }

    socket->socket = s;

    llhttp_init(&(socket->parser), type, &settings);

    return socket;
}
#endif

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
 *                      is possible and no synchronization is needed
 * @param[in,out] len   Pointer to count of sockets in "array". Is
 *                      incremented by 1 after function returns.
 * @param[in,out] array Pointer to dynamically allocated array of
 *                      sockets with length "*len", or NULL if array
 *                      should be created during function call.
 *                      After function call "array" is increased by
 *                      one element. This new element holds the
 *                      value of "socket".
 */
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
void save_socket(libiotrace_socket *socket, pthread_mutex_t *lock, int *len,
libiotrace_socket ***array) {
    void *ret;

    if (NULL != lock) {
        pthread_mutex_lock(lock);
    }

    (*len)++;
    ret = CALL_REAL_ALLOC_SYNC(realloc)(*array,
    sizeof(libiotrace_socket*) * (*len));
    if (NULL == ret) {
        CALL_REAL_ALLOC_SYNC(free)(*array);
        LOG_ERROR_AND_DIE("realloc() failed");
    }
    *array = ret;
    (*array)[*len - 1] = socket;

    if (NULL != lock) {
        pthread_mutex_unlock(lock);
    }
}
#endif

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
 *                      is possible and no synchronization is needed
 * @param[in,out] len   Pointer to count of sockets in "array". Is
 *                      decremented by 1 after function returns.
 * @param[in,out] array Pointer to dynamically allocated array of
 *                      sockets with length "*len".
 *                      After function call "array" is decreased by
 *                      one element (if "socket" was in "array").
 */
#if defined(ENABLE_REMOTE_CONTROL) || defined(IOTRACE_ENABLE_INFLUXDB)
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
        ret = CALL_REAL_ALLOC_SYNC(realloc)(*array, sizeof(libiotrace_socket*) * (*len));
        if (*len == 0)
        {
            if (ret != NULL)
            {
                CALL_REAL_ALLOC_SYNC(free)(ret);
            }
            *array = NULL;
        }
        else if (NULL == ret)
        {
            CALL_REAL_ALLOC_SYNC(free)(*array);
            LOG_ERROR_AND_DIE("realloc() failed");
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
        ret = CALL_REAL_ALLOC_SYNC(realloc)(*array, sizeof(libiotrace_socket*) * (*len));
        if (*len == 0)
        {
            if (ret != NULL)
            {
                CALL_REAL_ALLOC_SYNC(free)(ret);
            }
            *array = NULL;
        }
        else if (NULL == ret)
        {
            CALL_REAL_ALLOC_SYNC(free)(*array);
            LOG_ERROR_AND_DIE("realloc() failed");
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
#endif

#ifndef IO_LIB_STATIC
static char event_init_done = 0;

/**
 *  Initialize pointers for glibc functions.
 *
 *  Wrappers use them to call the real functions.
 */
void event_init(void) {
    if (!event_init_done) {

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
void toggle_wrapper(const char *line, const char toggle) {
    char ret = 1;

    if (!strcmp(line, "")) {
        ret = 0;
    }
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_SET_VARIABLE
#include "event_wrapper.h"
#include "event_sim_wrapper.h"
    else {
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
#ifdef WITH_ALLOC
    if (!ret)
    {
        ret = toggle_alloc_wrapper(line, toggle);
    }
#endif
}

/**
 * Initializes all needed function pointers.
 *
 * Each wrapper needs a function pointer to the
 * wrapped function. "dlsym" is used to get
 * these pointers.
 */
#ifndef IO_LIB_STATIC
void init_wrapper(void) {
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
#ifdef WITH_ALLOC
    alloc_init(); // initialize of function pointers is necessary
#endif
}
#endif

/**
 * Creates a new socket to POST data to influxdb.
 *
 * @return the newly created socket
 */
#ifdef IOTRACE_ENABLE_INFLUXDB
SOCKET create_socket(void) {
    // TODO: close on exec needed (FD O_CLOEXEC)???
    SOCKET new_socket = -1;
    /* Call of getaddrinfo calls other posix functions. These other
     * functions could be wrapped. Call of a wrapper out of getaddrinfo
     * is done during initialization of a thread (because
     * prepare_socket is only called during initialization of the
     * thread). Because at this time the thread is not initialized
     * the call of the wrapper calls prepare_socket. Now we have a
     * endless recursion of function calls that exceeds the stack size.
     * => getaddrinfo should only be called if POSIX wrapper are not
     *    build */
#if defined(WITH_POSIX_IO) || defined(WITH_ALLOC)
    new_socket = CALL_REAL_POSIX_SYNC(socket)(AF_INET, SOCK_STREAM, 0);
#else
    //Configure remote address for socket
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(database_ip, database_port, &hints, &peer_address)) {
        LOG_WARN("getaddrinfo() failed. (%d)", GETSOCKETERRNO());
        return new_socket;
    }
    new_socket = CALL_REAL_POSIX_SYNC(socket)(peer_address->ai_family,
            peer_address->ai_socktype, peer_address->ai_protocol);

#endif

    if (!ISVALIDSOCKET(new_socket)) {
        LOG_WARN("socket() failed. (%d)", GETSOCKETERRNO());
        return new_socket;
    }

    //Set socket option TCP_NODELAY
    // int option = 0;
    // if (setsockopt(new_socket, IPPROTO_TCP, TCP_NODELAY, (void *)&option, sizeof(option)))
    // {
    //     CALL_REAL_POSIX_SYNC(fprintf)
    //     (stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
    //     return;
    // }

    //Set socket option REUSEADDR
    // int option2 = 0;
    // if (setsockopt(new_socket, SOL_SOCKET, SO_REUSEADDR, (void *)&option2, sizeof(option2)))
    // {
    //     CALL_REAL_POSIX_SYNC(fprintf)
    //     (stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
    //     return;
    // }

#if defined(WITH_POSIX_IO) || defined(WITH_ALLOC)
    //MAP PORT FROM ENV TO SA_DATA(IPv4)
    unsigned short database_port_short = (unsigned short)atoi(database_port);
    unsigned char *database_port_short_p = (unsigned char *)&database_port_short;

    //MAP IP FROM ENV TO SA_DATA(IPv4)
    char *str = database_ip;
    unsigned char database_ip_char[4] = {0};
    size_t index = 0;

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

    if (CALL_REAL_POSIX_SYNC(connect)(new_socket, &own_ai_addr, 16))
    {
        LOG_WARN("connect() failed. (%d)", GETSOCKETERRNO());
        return new_socket;
    }
#else
    if (connect(new_socket, peer_address->ai_addr, peer_address->ai_addrlen)) {
        LOG_WARN("connect() failed. (%d)", GETSOCKETERRNO());
        return new_socket;
    }
    freeaddrinfo(peer_address);
#endif

    return new_socket;
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
#ifdef IOTRACE_ENABLE_INFLUXDB
void prepare_socket(void) {
    socket_peer = create_socket();

    // save socket globally to create thread that listens to / reads from all sockets
    libiotrace_socket *socket = create_libiotrace_socket(socket_peer,
            HTTP_RESPONSE);
    save_socket(socket, &socket_lock, &recv_sockets_len, &recv_sockets);

#if defined(ENABLE_REMOTE_CONTROL)
    // save local ip (for sending ip to influx)
    pthread_mutex_lock(&ip_lock);
    if ('\0' == local_ip[0]) {
        struct sockaddr_in local_addr;
        memset(&local_addr, 0, sizeof(local_addr));
        socklen_t len = sizeof(local_addr);
        if (0 > getsockname(socket_peer, (struct sockaddr *) &local_addr, &len)) {
            LOG_WARN("getsockname returned -1, errno %d", errno);
        }
        // TODO: AF_INET is IPv4, support AF_INET6 for IPv6
        if (NULL == inet_ntop(AF_INET, &local_addr.sin_addr, local_ip, sizeof(local_ip))) {
            LOG_WARN("inet_ntop returned NULL, errno %d", errno);
        }
    }
    pthread_mutex_unlock(&ip_lock);
#endif
}
#endif

void libiotrace_start_log(void) {
#ifdef IOTRACE_ENABLE_LOGFILE
    no_logging = 0;
#endif
}

void libiotrace_end_log(void) {
#ifdef IOTRACE_ENABLE_LOGFILE
    no_logging = 1;
#endif
}

void libiotrace_start_send(void) {
#ifdef IOTRACE_ENABLE_INFLUXDB
    no_sending = 0;
#endif
}

void libiotrace_end_send(void) {
#ifdef IOTRACE_ENABLE_INFLUXDB
    no_sending = 1;
#endif
}

void libiotrace_start_stacktrace_ptr(void) {
    stacktrace_ptr = 1;
}

void libiotrace_end_stacktrace_ptr(void) {
    stacktrace_ptr = 0;
}

void libiotrace_start_stacktrace_symbol(void) {
    stacktrace_symbol = 1;
}

void libiotrace_end_stacktrace_symbol(void) {
    stacktrace_symbol = 0;
}

void libiotrace_set_stacktrace_depth(int depth) {
    stacktrace_depth = depth;
}

int libiotrace_get_stacktrace_depth(void) {
    return stacktrace_depth;
}

void libiotrace_set_wrapper_active(const char *wrapper) {
    toggle_wrapper(wrapper, 1);
}

void libiotrace_set_wrapper_inactive(const char *wrapper) {
    toggle_wrapper(wrapper, 0);
}

/*
* Writes one power_measurement_data-entry to influxdb.
*
* @param[in]  data power_measurement_data-entry.
*/
#if defined(IOTRACE_ENABLE_INFLUXDB) && defined(ENABLE_POWER_MEASUREMENT)
void write_power_measurement_data_into_influxdb(struct power_measurement_data *data, int method) {
    //buffer for body
    int body_length = libiotrace_struct_push_max_size_power_measurement_data(0) + 1; /* +1 for trailing null character (function build by macros; gives length of body to send) */
    char body[body_length];
    body_length = libiotrace_struct_push_power_measurement_data(body, body_length, data, "");
    if (0 > body_length)
    {
        LOG_ERROR_AND_DIE("libiotrace_struct_push_power_measurement_data() returned %d", body_length);
    }
    body_length--; /*last comma in ret*/
    body[body_length] = '\0'; /*remove last comma*/

    char short_log_name[50];
    shorten_log_name(short_log_name, sizeof(short_log_name), log_name, log_name_len);

    const char labels[] = "libiotrace_power_measurement,jobname=%s,hostname=%s,pid=%d,cpu_package=%d,cpu_id=%d,type=%u,name=%s,method=%d";
    int body_labels_length = strlen(labels)
                             + sizeof(short_log_name) /* jobname */
                             + HOST_NAME_MAX /* hostname */
                             + COUNT_DEC_AS_CHAR(data->pid) /* pid */
                             + COUNT_DEC_AS_CHAR(data->cpu_package) /* cpu_package */
                             + COUNT_DEC_AS_CHAR(data->cpu_id) /* cpu_id */
                             + COUNT_DEC_AS_CHAR(data->type) /* type */
                             + strlen(data->name) /* name */
                             + COUNT_DEC_AS_CHAR(method); /* name */

    char body_labels[body_labels_length];
    snprintf(body_labels, sizeof(body_labels), labels, short_log_name, hostname, data->pid, data->cpu_package, data->cpu_id, (uint)data->type, data->name, method);
    body_labels_length = strlen(body_labels);

    u_int64_t current_time = gettime();
    int timestamp_length = COUNT_DEC_AS_CHAR(current_time);
    char timestamp[timestamp_length];
#ifdef REALTIME
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64, current_time);
#else
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64, system_start_time + current_time);
#endif
    timestamp_length = strlen(timestamp);

    const int content_length = body_labels_length + 1 /*space*/ + body_length + 1 /*space*/ + timestamp_length;

    const char header[] = "POST /api/v2/write?bucket=%s&precision=ns&org=%s HTTP/1.1" LINE_BREAK
                          "Host: %s:%s" LINE_BREAK
                          "Accept: */*" LINE_BREAK
                          "Authorization: Token %s" LINE_BREAK
                          "Content-Length: %d" LINE_BREAK
                          "Content-Type: application/x-www-form-urlencoded" LINE_BREAK
                          LINE_BREAK
                          "%s %s %s";
    const int message_length = strlen(header)
                               + influx_bucket_len
                               + influx_organization_len
                               + database_ip_len
                               + database_port_len
                               + influx_token_len
                               + COUNT_DEC_AS_CHAR(content_length) /* Content-Length */
                               + body_labels_length
                               + body_length
                               + timestamp_length;

    //buffer all (header + body)
    char message[message_length + 1];
    snprintf(message, sizeof(message), header, influx_bucket, influx_organization, database_ip, database_port, influx_token,
             content_length, body_labels, body, timestamp);


    send_data(message, socket_peer);
}
#endif

/**
 * Writes one filesystem-entry to influxdb.
 *
 * @param[in]  data filesystem-entry.
 */
#if defined(IOTRACE_ENABLE_INFLUXDB) && defined(ENABLE_FILESYSTEM_METADATA)
void write_filesystem_into_influxdb(struct filesystem *data) {
    //buffer for body
    int body_length = libiotrace_struct_push_max_size_filesystem(0) + 1; /* +1 for trailing null character (function build by macros; gives length of body to send) */
    char body[body_length];
    body_length = libiotrace_struct_push_filesystem(body, body_length, data, "");
    if (0 > body_length)
    {
        LOG_ERROR_AND_DIE("libiotrace_struct_push_filesystem() returned %d", body_length);
    }
    body_length--; /*last comma in ret*/
    body[body_length] = '\0'; /*remove last comma*/

    char short_log_name[50];
    shorten_log_name(short_log_name, sizeof(short_log_name), log_name, log_name_len);

    const char labels[] = "libiotrace_filesystem,jobname=%s,hostname=%s,device_id=%lu";
    int body_labels_length = strlen(labels)
                             + sizeof(short_log_name) /* jobname */
                             + HOST_NAME_MAX /* hostname */
                             + COUNT_DEC_AS_CHAR(data->device_id); /* deviceid */
    char body_labels[body_labels_length];
    snprintf(body_labels, sizeof(body_labels), labels, short_log_name, hostname, data->device_id);
    body_labels_length = strlen(body_labels);

    u_int64_t current_time = gettime();
    int timestamp_length = COUNT_DEC_AS_CHAR(current_time);
    char timestamp[timestamp_length];
#ifdef REALTIME
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64, current_time);
#else
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64, system_start_time + current_time);
#endif
    timestamp_length = strlen(timestamp);

    const int content_length = body_labels_length + 1 /*space*/ + body_length + 1 /*space*/ + timestamp_length;

    const char header[] = "POST /api/v2/write?bucket=%s&precision=ns&org=%s HTTP/1.1" LINE_BREAK
                          "Host: %s:%s" LINE_BREAK
                          "Accept: */*" LINE_BREAK
                          "Authorization: Token %s" LINE_BREAK
                          "Content-Length: %d" LINE_BREAK
                          "Content-Type: application/x-www-form-urlencoded" LINE_BREAK
                          LINE_BREAK
                          "%s %s %s";
    const int message_length = strlen(header)
                               + influx_bucket_len
                               + influx_organization_len
                               + database_ip_len
                               + database_port_len
                               + influx_token_len
                               + COUNT_DEC_AS_CHAR(content_length) /* Content-Length */
                               + body_labels_length
                               + body_length
                               + timestamp_length;

    //buffer all (header + body)
    char message[message_length + 1];
    snprintf(message, sizeof(message), header, influx_bucket, influx_organization, database_ip, database_port, influx_token,
             content_length, body_labels, body, timestamp);

    send_data(message, socket_peer);
}
#endif



/**
 * Prints the filesystem to a file.
 *
 * The file is given in the global variable #filesystem_log_name which is
 * set from environment variable #ENV_LOG_NAME. Each mount point is printed
 * on a new line. The printed line shows the device, the mount point, the
 * file-system type and the mount options, the dump frequency in days and
 * the mount passno as a json object.
 */
#if defined(ENABLE_FILESYSTEM_METADATA) && \
        (defined(IOTRACE_ENABLE_LOGFILE) \
                || defined(IOTRACE_ENABLE_INFLUXDB))
#ifdef __linux__ // TODO: RAY MacOS; Windows?
void print_filesystem(void)
{
    FILE *file;
#ifdef HAVE_GETMNTENT_R
    struct mntent filesystem_entry;
    char buf[4 * MAXFILENAME];
#endif
    struct mntent *filesystem_entry_ptr;
    struct filesystem filesystem_data;
    struct stat stat_data;
    char mount_point[MAXFILENAME];
#ifdef IOTRACE_ENABLE_LOGFILE
    char buf_filesystem[libiotrace_struct_max_size_filesystem() + sizeof(LINE_BREAK)];
    int fd;
    int count;
#endif
    int ret;

#ifdef IOTRACE_ENABLE_LOGFILE
    fd = CALL_REAL_POSIX_SYNC(open)(filesystem_log_name,
                                    O_WRONLY | O_CREAT | O_EXCL,
                                    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (-1 == fd)
    {
        if (errno == EEXIST) /* print filesystem only once */
        {
            return;
        }
        else
        {
            LOG_ERROR_AND_DIE("open() returned %d with errno=%d", fd, errno);
        }
    }
#endif

    file = setmntent("/proc/mounts", "r");
    if (NULL == file)
    {
        LOG_ERROR_AND_DIE("setmntent() returned NULL with errno=%d", errno);
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
            LOG_ERROR_AND_DIE("getmntent() returned mnt_dir too long (%d bytes) for buffer", ret);
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
#ifdef IOTRACE_ENABLE_LOGFILE
        ret = libiotrace_struct_print_filesystem(buf_filesystem, sizeof(buf_filesystem),
                                     &filesystem_data);
        strcpy(buf_filesystem + ret, LINE_BREAK);
        count = ret + sizeof(LINE_BREAK) - 1;
        ret = CALL_REAL_POSIX_SYNC(write)(fd, buf_filesystem, count);
        if (0 > ret) {
            LOG_ERROR_AND_DIE("write() returned %d", ret);
        }
        if (ret < count) {
            LOG_ERROR_AND_DIE("incomplete write() occurred");
        }
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
        write_filesystem_into_influxdb(&filesystem_data);
#endif
    }

    endmntent(file);

#ifdef IOTRACE_ENABLE_LOGFILE
    CALL_REAL_POSIX_SYNC(close)
    (fd);
#endif
}
#endif
#endif

/**
 * Get "device_id" and "inode_nr" for given file descriptor "fd".
 *
 * @param[in]  filename File descriptor of a file.
 * @param[out] data     Pointer to a struct file_id. Function fills
 *                      "device_id" and "inode_nr" of "fd" into
 *                      this struct.
 */
void get_file_id(int fd, struct file_id *data) {
    struct stat stat_data;
    int ret;

    if (0 > fd) {
        data->device_id = 0;
        data->inode_nr = 0;
    } else {
        ret = fstat(fd, &stat_data);
        if (0 > ret) {
            LOG_ERROR_AND_DIE("fstat() returned %d with errno=%d", ret, errno);
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
void get_file_id_by_path(const char *filename, struct file_id *data) {
    struct stat stat_data;
    int ret;

    ret = stat(filename, &stat_data);
    if (0 > ret) {
        LOG_ERROR_AND_DIE("stat() returned %d with errno=%d", ret, errno);
    }

    data->device_id = stat_data.st_dev;
    data->inode_nr = stat_data.st_ino;
}

/**
 * Prints the working dir of the actual process to a file.
 *
 * The file is given in the global variable #working_dir_log_name which is
 * set from environment variable #ENV_LOG_NAME. The working dir is printed
 * on a new line. The printed line shows the working dir, a timestamp, the
 * hostname and the process id as a json object.
 */
#ifdef IOTRACE_ENABLE_LOGFILE
void print_working_directory(void) {
    char buf_working_dir[libiotrace_struct_max_size_working_dir()
            + sizeof(LINE_BREAK)];
    struct working_dir working_dir_data;
    char cwd[MAXFILENAME];
    char *ret;
    int fd;
    int ret_int;
    int count;

    ret = getcwd(cwd, sizeof(cwd));
    if (NULL == ret) {
        LOG_ERROR_AND_DIE("getcwd() returned NULL with errno=%d", errno);
    }

    working_dir_data.time = gettime();
    working_dir_data.hostname = hostname;
    working_dir_data.pid = pid;
    working_dir_data.dir = cwd;

    fd = CALL_REAL_POSIX_SYNC(open)(working_dir_log_name,
    O_WRONLY | O_CREAT | O_APPEND,
    S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
    if (-1 == fd) {
        LOG_ERROR_AND_DIE("open() of file %s returned %d", working_dir_log_name,
                           fd);
    }

    ret_int = libiotrace_struct_print_working_dir(buf_working_dir,
            sizeof(buf_working_dir), &working_dir_data);
    strcpy(buf_working_dir + ret_int, LINE_BREAK);
    count = ret_int + sizeof(LINE_BREAK) - 1;
    ret_int = CALL_REAL_POSIX_SYNC(write)(fd, buf_working_dir, count);
    if (0 > ret_int) {
        LOG_ERROR_AND_DIE("write() returned %d", ret_int);
    }
    if (ret_int < count) {
        LOG_ERROR_AND_DIE("incomplete write() occurred");
    }

    CALL_REAL_POSIX_SYNC(close)(fd);
}
#endif

/**
 * Reset values in forked process before forked process starts.
 *
 * If a process forks another process the new process is a copy of the old
 * process. The copy inherits all values from the old process. Some values
 * are only valid for the old process. These values must be reset.
 *
 * Is registered by pthread_atfork. Must be async-signal-safe. Should only
 * make assignments to constant values.
 */
void reset_values_in_forked_process(void) {
    init_done = 0;
    tid = -1;
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
    socket_peer = -1;
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
    recv_sockets = NULL;
    recv_sockets_len = 0;
#endif
#ifdef ENABLE_REMOTE_CONTROL
    open_control_sockets = NULL;
    open_control_sockets_len = 0;
#endif
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
#ifdef WITH_STD_IO
void open_std_fd(int fd)
{
    struct basic data;
    struct file_descriptor file_descriptor_data;

    if (!active_wrapper_status.open_std_fd) {
        return;
    }

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


#ifdef FILENAME_RESOLUTION_ENABLED
    fnres_trace_ioevent(&data);
#endif

#ifdef IOTRACE_ENABLE_LOGFILE
    if (!no_logging) {
        io_log_file_buffer_write(&data);
    }
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
    write_into_influxdb(&data);
#endif
    WRAP_FREE(&data)
}
#endif

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
#ifdef WITH_STD_IO
void open_std_file(FILE *file)
{
    struct basic data;
    struct file_stream file_stream_data;

    if (!active_wrapper_status.open_std_file) {
        return;
    }

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


#ifdef FILENAME_RESOLUTION_ENABLED
    fnres_trace_ioevent(&data);
#endif


#ifdef IOTRACE_ENABLE_LOGFILE
    if (!no_logging) {
        io_log_file_buffer_write(&data);
    }
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
    write_into_influxdb(&data);
#endif
    WRAP_FREE(&data)
}
#endif

void init_on_load(void) ATTRIBUTE_CONSTRUCTOR;

/**
 * Calls "init_process" during execution of "ctor" section.
 *
 * Guarantees that "init_process" is called before main() of
 * the observed program is started.
 */
void init_on_load(void) {
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
#  ifdef FILENAME_RESOLUTION_ENABLED
    fnres_trace_ioevent(&data);
#  endif

#  ifdef IOTRACE_ENABLE_LOGFILE
    if (active_wrapper_status.init_on_load && !no_logging) {
        io_log_file_buffer_write(&data);
    }
#  endif
#  ifdef IOTRACE_ENABLE_INFLUXDB
    if (active_wrapper_status.init_on_load) {
        write_into_influxdb(&data);
    }
#  endif
    WRAP_FREE(&data)
#endif /* LOG_WRAPPER_TIME */
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
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
void send_data(const char *message, SOCKET socket) {
    size_t bytes_to_send = strlen(message);
    const char *message_to_send = message;

    while (bytes_to_send > 0) {
        int bytes_sent = send(socket, message_to_send, bytes_to_send, 0);

        if (-1 == bytes_sent) {
            if (errno == EWOULDBLOCK || errno == EAGAIN) {
                LOG_WARN(
                        "Send buffer is full. Please increase your limit.");
            } else {
                LOG_ERROR_AND_DIE("send() returned %d, errno: %d, socket: %d", bytes_sent,
                        errno, socket);
            }
        } else {
            if ((size_t)bytes_sent < bytes_to_send) {
                bytes_to_send -= bytes_sent;
                message_to_send += bytes_sent;
            } else {
                bytes_to_send = 0;
            }
        }
    }
}
#endif

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
#ifdef IOTRACE_ENABLE_INFLUXDB
int url_callback_responses(llhttp_t *parser, const char *at ATTRIBUTE_UNUSED, size_t length ATTRIBUTE_UNUSED) {
    if (parser->status_code != 204) {
        LOG_WARN("unknown status (%d) in response from influxdb",
                parser->status_code);
    } else {
        //LOG_WARN("known status (%d) in response from influxdb", parser->status_code);
    }

    return 0;
}
#endif

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
#ifdef ENABLE_REMOTE_CONTROL
int url_callback_requests(llhttp_t *parser, const char *at, size_t length) {
    // TODO: parser will call callback multiple times for partial messages:
    //       if (url_chunk) { build TLS_URL } else
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
        const char message_header[] = "HTTP/1.1 200 OK" LINE_BREAK \
                "Content-Length: %d" LINE_BREAK \
                "Content-Type: application/json" LINE_BREAK \
                LINE_BREAK \
                "%s";

        // buffer for body
        char buf[libiotrace_struct_max_size_wrapper_status() + 1];
        int ret = libiotrace_struct_print_wrapper_status(buf, sizeof(buf), &active_wrapper_status);
        if (0 > ret)
        {
            LOG_ERROR_AND_DIE("libiotrace_struct_print_wrapper_status() returned %d", ret);
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
#endif

/**
 * Sends meta data to influxdb.
 *
 * Sends ip and port of the control socket to influxdb. Must
 * be called for each new thread. So for each thread in each
 * process and host an entry in influxdb is made. This entry
 * can be used to control the wrappers in the thread.
 */
#if defined(IOTRACE_ENABLE_INFLUXDB) && defined(ENABLE_REMOTE_CONTROL)
void write_metadata_into_influxdb(void)
{
    struct influx_meta data;

    // wait until communication_thread has bound control socket
    // TODO: memory fence to allow different memory models
    while (0 > libiotrace_control_port) {
        sleep(1); // TODO: is there a better way (without busy loop)
    }
    data.port = libiotrace_control_port;
    data.ip = local_ip;
    data.wrapper = &active_wrapper_status;

    //buffer for body
    int body_length = libiotrace_struct_push_max_size_influx_meta(0) + 1; /* +1 for trailing null character (function build by macros; gives length of body to send) */
    char body[body_length];
    body_length = libiotrace_struct_push_influx_meta(body, body_length, &data, "");
    if (0 > body_length)
    {
        LOG_ERROR_AND_DIE("libiotrace_struct_push_influx_meta() returned %d", body_length);
    }
    body_length--; /*last comma in ret*/
    body[body_length] = '\0'; /*remove last comma*/

    char short_log_name[50];
    shorten_log_name(short_log_name, sizeof(short_log_name), log_name, log_name_len);

    const char labels[] = "libiotrace_control,jobname=%s,hostname=%s,processid=%d,thread=%d";
    int body_labels_length = strlen(labels)
            + sizeof(short_log_name) /* jobname */
            + HOST_NAME_MAX /* hostname */
            + COUNT_DEC_AS_CHAR(pid) + 1 /* processid with sign */
            + COUNT_DEC_AS_CHAR(tid) + 1; /* thread with sign */
    char body_labels[body_labels_length];
    snprintf(body_labels, sizeof(body_labels), labels, short_log_name, hostname, pid, tid);
    body_labels_length = strlen(body_labels);

    u_int64_t current_time = gettime();
    int timestamp_length = COUNT_DEC_AS_CHAR(current_time);
    char timestamp[timestamp_length];
#ifdef REALTIME
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64, current_time);
#else
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64, system_start_time + current_time);
#endif
    timestamp_length = strlen(timestamp);

    const int content_length = body_labels_length + 1 /*space*/ + body_length + 1 /*space*/ + timestamp_length;

    const char header[] = "POST /api/v2/write?bucket=%s&precision=ns&org=%s HTTP/1.1" LINE_BREAK
            "Host: %s:%s" LINE_BREAK
            "Accept: */*" LINE_BREAK
            "Authorization: Token %s" LINE_BREAK
            "Content-Length: %d" LINE_BREAK
            "Content-Type: application/x-www-form-urlencoded" LINE_BREAK
            LINE_BREAK
            "%s %s %s";
    const int message_length = strlen(header)
            + influx_bucket_len
            + influx_organization_len
            + database_ip_len
            + database_port_len
            + influx_token_len
            + COUNT_DEC_AS_CHAR(content_length) /* Content-Length */
            + body_labels_length
            + body_length
            + timestamp_length;

    //buffer all (header + body)
    char message[message_length + 1];
    snprintf(message, sizeof(message), header, influx_bucket, influx_organization, database_ip, database_port, influx_token,
            content_length, body_labels, body, timestamp);

    send_data(message, socket_peer);
}
#endif

/**
 * Creates additional control socket
 *
 * The additional socket for incoming connections is bound to a
 * PORT in the range from PORT_RANGE_MIN to PORT_RANGE_MAX.
 * Which port is bound is written to a file.
 *
 * @return control socket
 */
#ifdef ENABLE_REMOTE_CONTROL
SOCKET prepare_control_socket(void) {
    SOCKET socket_control;

    // Open Socket to receive control information
    struct sockaddr_in addr;
    socket_control = CALL_REAL_POSIX_SYNC(socket)(PF_INET, SOCK_STREAM, 0);
    if (socket_control < 0)
    {
        LOG_ERROR_AND_DIE("could not open socket, errno %d", errno);
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

#ifdef IOTRACE_ENABLE_LOGFILE
//            if (ioctl(socket_control, SIOCGIFCONF) < 0)
//            {
//                LOG_ERROR_AND_DIE("ioctl returned -1, errno %d", errno);
//            }

            //Write PORT and IP for control commands to file
            struct ifreq ifreqs[20];
            struct ifconf ic;
            struct control_meta meta_data;
            int ret;
            int count;
            char buf[libiotrace_struct_max_size_control_meta() + sizeof(LINE_BREAK)];

            ic.ifc_len = sizeof ifreqs;
            ic.ifc_req = ifreqs;

            if (ioctl(socket_control, SIOCGIFCONF, &ic) < 0)
            {
                LOG_ERROR_AND_DIE("ioctl returned -1, errno %d", errno);
            }

            int fd = CALL_REAL_POSIX_SYNC(open)(control_log_name,
                                                O_WRONLY | O_CREAT | O_APPEND,
                                                S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH);
            if (-1 == fd)
            {
                LOG_ERROR_AND_DIE("open() of file %s returned %d", control_log_name, fd);
            }

            meta_data.pid = pid;
            meta_data.port = i;
            for (size_t l = 0; l < ic.ifc_len / sizeof(struct ifreq); ++l)
            {
                meta_data.interface_name = ifreqs[l].ifr_name;
                meta_data.ip = inet_ntoa(((struct sockaddr_in *)&ifreqs[l].ifr_addr)->sin_addr);

                ret = libiotrace_struct_print_control_meta(buf, sizeof(buf), &meta_data); //Function is present at runtime, built with macros from libiotrace_defines.h
                strcpy(buf + ret, LINE_BREAK);
                count = ret + sizeof(LINE_BREAK) - 1;
                ret = CALL_REAL_POSIX_SYNC(write)(fd, buf, count);
                if (0 > ret) {
                    LOG_ERROR_AND_DIE("write() returned %d", ret);
                }
                if (ret < count) {
                    LOG_ERROR_AND_DIE("incomplete write() occurred");
                }
            }

            CALL_REAL_POSIX_SYNC(close)
            (fd);
#endif

            break;
        }
    }
    if (i > PORT_RANGE_MAX)
    {
        CALL_REAL_POSIX_SYNC(close)
        (socket_control);
        LOG_ERROR_AND_DIE("unable to bind socket");
    }

#ifdef IOTRACE_ENABLE_INFLUXDB
    libiotrace_control_port = i;
#endif

    // Listen to socket
    int ret = listen(socket_control, 10);
    if (0 > ret)
    {
        CALL_REAL_POSIX_SYNC(close)
        (socket_control);
        LOG_ERROR_AND_DIE("unable to listen to socket, errno=%d", errno);
    }

    return socket_control;
}
#endif

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
 * enables controlling which wrapper is active.
 * Incoming http requests are handled via callback function
 * "url_callback".
 *
 * This thread runs and reads/listens the multiple sockets until
 * "event_cleanup_done" is set to "true". This is done during call
 * of "cleanup_process" if the program exits.
 *
 * @param[in] arg Not used.
 * @return Not used (allways NULL)
 */
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
void* communication_thread(ATTRIBUTE_UNUSED void *arg) {
    struct timeval select_timeout;

#ifdef ENABLE_POWER_MEASUREMENT
    if (-1 == socket_peer) {
        prepare_socket();
    }
#endif

    // Read responses from influxdb and read control messages
    while (!event_cleanup_done) {
        fd_set fd_recv_sockets;
        FD_ZERO(&fd_recv_sockets);
        SOCKET socket_max = -1;

#ifdef IOTRACE_ENABLE_INFLUXDB
        // Add one connection to influxdb per open thread to fd_set
        pthread_mutex_lock(&socket_lock);
        for (int i = 0; i < recv_sockets_len; i++) {
            FD_SET(recv_sockets[i]->socket, &fd_recv_sockets);
            if (recv_sockets[i]->socket > socket_max) {
                socket_max = recv_sockets[i]->socket;
            }
        }
        pthread_mutex_unlock(&socket_lock);
#endif

#ifdef ENABLE_REMOTE_CONTROL
        // Add listening socket for establishing control connections to fd_set
        FD_SET(socket_control, &fd_recv_sockets);
        if (socket_control > socket_max)
        {
            socket_max = socket_control;
        }

        // Add active control connections to fd_set
        for (int i = 0; i < open_control_sockets_len; i++)
        {
            FD_SET(open_control_sockets[i]->socket, &fd_recv_sockets);
            if (open_control_sockets[i]->socket > socket_max)
            {
                socket_max = open_control_sockets[i]->socket;
            }
        }
#endif

        select_timeout.tv_sec = SELECT_TIMEOUT_SECONDS;
        select_timeout.tv_usec = 0;
        int ret = CALL_REAL_POSIX_SYNC(select)(socket_max + 1, &fd_recv_sockets,
                NULL, NULL, &select_timeout);
        if (-1 == ret && EINTR != errno) /* ignore interrupts via signal (they are not for us) */
        {
            LOG_WARN("select() returned -1, errno=%d.", errno);
            break;
        } else if (0 < ret) {
            /* Select: At least one socket is ready to be processed */
            /* check which descriptor/socket is ready to read if there was new data before timeout expired */

#ifdef IOTRACE_ENABLE_INFLUXDB
            // receive responses from influxdb
            pthread_mutex_lock(&socket_lock);
            for (int i = 0; i < recv_sockets_len; i++) {
                //Which sockets are ready to read
                if (FD_ISSET(recv_sockets[i]->socket, &fd_recv_sockets)) {
                    char read[4096];
                    ssize_t bytes_received = recv(recv_sockets[i]->socket, read,
                            4096, 0);
                    if (1 > bytes_received) {
                        //Socket is destroyed or closed by peer
                    	LOG_WARN("socket destroyed or closed by peer");
                    	CLOSESOCKET(recv_sockets[i]->socket);
                    	libiotrace_socket *s = recv_sockets[i];
                    	delete_socket(recv_sockets[i]->socket, NULL, &recv_sockets_len, &recv_sockets);
                    	CALL_REAL_ALLOC_SYNC(free)(s);
                        i--;
                    } else {
                        enum llhttp_errno err = llhttp_execute(
                                &(recv_sockets[i]->parser), read,
                                bytes_received);
                        if (err != HPE_OK) {
                            const char *errno_text = llhttp_errno_name(err);
                            LOG_ERROR_AND_DIE(
                                    "error parsing influxdb response: %s: %s",
                                    errno_text, recv_sockets[i]->parser.reason);
                        }
                    }
                }
            }
            pthread_mutex_unlock(&socket_lock);
#endif

#ifdef ENABLE_REMOTE_CONTROL
            // If socket to establish new connections is ready
            if (FD_ISSET(socket_control, &fd_recv_sockets))
            {
                //Accept one new socket but more could be ready at this point
                SOCKET socket = accept(socket_control, NULL, NULL);
                if (0 > socket)
                {
                    LOG_ERROR_AND_DIE("accept returned -1, errno=%d", errno);
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
                        CALL_REAL_ALLOC_SYNC(free)(s);
                        i--;
                    }
                    else
                    {
                        socket_peer = open_control_sockets[i]->socket; // is needed by callback => must be set before llhttp_execute()
                        enum llhttp_errno err = llhttp_execute(&(open_control_sockets[i]->parser), read, bytes_received);
                        if (err != HPE_OK)
                        {
                            const char *errno_text = llhttp_errno_name(err);
                            snprintf(read, sizeof(read), "HTTP/1.1 400 Bad Request" LINE_BREAK "Content-Length: %lu" LINE_BREAK "Content-Type: application/json" LINE_BREAK LINE_BREAK "%s: %s",
                                    strlen(errno_text) + 2 + strlen(open_control_sockets[i]->parser.reason), errno_text, open_control_sockets[i]->parser.reason);
                            send_data(read, socket_peer);
                        }
                    }
                }
            }
#endif
        }

#ifdef ENABLE_POWER_MEASUREMENT
        power_measurement_step();
#endif
    }

#ifdef ENABLE_REMOTE_CONTROL
    CLOSESOCKET(socket_control);
    for (int i = 0; i < open_control_sockets_len; i++)
    {
        CLOSESOCKET(open_control_sockets[i]->socket);
        CALL_REAL_ALLOC_SYNC(free)(open_control_sockets[i]);
    }
    CALL_REAL_ALLOC_SYNC(free)(open_control_sockets);
#endif

    return NULL;
}
#endif

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
int libiotrace_get_env(const char *env_name, char *dst, const int max_len,
        const char error_if_not_exists) {
    char *log;
    int length;

    log = getenv(env_name);
    if (NULL == log) {
        if (error_if_not_exists) {
            LOG_ERROR_AND_DIE("getenv(\"%s\") returned NULL", env_name);
        } else {
            return 0;
        }
    }
    length = strlen(log);
    if (max_len < length) {
        LOG_ERROR_AND_DIE("getenv() returned %s too long (%d bytes) for buffer",
                           env_name, length);
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
void read_whitelist(void) {
    int fd;
    int ret;
    struct stat statbuf;
    char *buffer;
    char *p;
    char *line = NULL;
    char *clean_line = NULL;
    char *end_clean_line = NULL;
    off_t toread;
    off_t file_len;

    fd = CALL_REAL_POSIX_SYNC(open)(whitelist, O_RDONLY);
    if (-1 == fd) {
        LOG_ERROR_AND_DIE("open() failed, errno=%d", errno);
    }

    ret = fstat(fd, &statbuf);
    if (-1 == ret) {
        LOG_ERROR_AND_DIE("fstat() failed, errno=%d", errno);
    }

    if (0 >= statbuf.st_size) {
        // file exists but is empty
        return;
    }
    file_len = statbuf.st_size;

    buffer = (char*) CALL_REAL_ALLOC_SYNC(malloc)(file_len + 1); // +1 for terminating '\0'
    if (NULL == buffer) {
        LOG_ERROR_AND_DIE("malloc() failed");
    }

    // read whole file
    for (p = buffer, toread = file_len; toread > 0; toread -= ret, p += ret) {
        ret = CALL_REAL_POSIX_SYNC(read)(fd, p, toread);
        if (0 > ret) {
            LOG_ERROR_AND_DIE("read() failed, errno=%d", errno);
        } else if (0 == ret) {
            // signal interrupt of file changed?
            if (-1 == fstat(fd, &statbuf)) {
                LOG_ERROR_AND_DIE("fstat() failed, errno=%d", errno);
            }
            if (file_len != statbuf.st_size) {
                LOG_ERROR_AND_DIE("whitelist got changed during read");
            }
        }
    }

    // add terminating '\0'
    buffer[file_len] = '\0';

    p = buffer;
    while (NULL != (line = read_line(buffer, file_len, &p))) {

        clean_line = line;

        // remove leading spaces
        while (isspace((unsigned char )*clean_line)) {
            clean_line++;
        }

        // not a comment and not only spaces
        if (*clean_line != '#' && *clean_line != '\0') {
            end_clean_line = clean_line + strlen(clean_line) - 1;

            // remove trailing spaces
            while (end_clean_line > clean_line
                    && isspace((unsigned char )*end_clean_line)) {
                end_clean_line--;
            }
            end_clean_line[1] = '\0';

            toggle_wrapper(clean_line, 1);
        }
    }

    ret = CALL_REAL_POSIX_SYNC(close)(fd);
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
 * the process itself is running => no synchronization is needed (if a program
 * uses ctor to start threads/processes this will lead to errors).
 */
void init_process(void) {
    int length;

    if (!init_done) {
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_VARIABLE
#include "event_wrapper.h"
#include "event_sim_wrapper.h"

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

#ifdef WITH_ALLOC
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_VARIABLE
#include "alloc_wrapper.h"
#endif

#if !defined(IO_LIB_STATIC)
        init_wrapper(); /* WARNING: glibc calls (CALL_REAL_POSIX_SYNC) will work ONLY AFTER THIS LINE */
#endif

#ifdef FILENAME_RESOLUTION_ENABLED
{
    /* (0) Get & parse env for max # of filenames in fnmap  --> TODO: Remove once fmap supports dynamic resizing */
        long fnres_fnmap_max_fnames = FNRES_DEFAULT_FNMAP_MAX_FNAMES;
        char *fnres_fnmap_max_fnames_env_str = NULL;
        if ((fnres_fnmap_max_fnames_env_str = getenv(FNRES_ENV_FNMAP_MAX_FNAMES))) {
            if (str_to_long(fnres_fnmap_max_fnames_env_str, &fnres_fnmap_max_fnames) ||
                (0 >= fnres_fnmap_max_fnames || FNRES_MAX_FNMAP_MAX_FNAMES < fnres_fnmap_max_fnames)) {
                fnres_fnmap_max_fnames = FNRES_DEFAULT_FNMAP_MAX_FNAMES;
                LOG_WARN("Invalid value for env var `%s`, using default (%ld) instead",
                         FNRES_ENV_FNMAP_MAX_FNAMES, fnres_fnmap_max_fnames);
            }
        }

    /* (1) Init module */
        fnres_init(fnres_fnmap_max_fnames);
}
#endif

#ifdef STRACING_ENABLED
        stracing_init_stracer();
#endif /* STRACING_ENABLED */

#if !defined(HAVE_HOST_NAME_MAX)
#if defined(HAVE__POSIX_HOST_NAME_MAX)
        host_name_max = _POSIX_HOST_NAME_MAX;
#else
        host_name_max = sysconf(_SC_HOST_NAME_MAX);
#endif
#endif

        pid = getpid();
        hostname = CALL_REAL_ALLOC_SYNC(malloc)(HOST_NAME_MAX);
        if (NULL == hostname)
            LOG_ERROR_AND_DIE("malloc failed, errno=%d", errno);

        gethostname(hostname, HOST_NAME_MAX);

#if defined(IOTRACE_ENABLE_LOGFILE) \
    || defined(IOTRACE_ENABLE_INFLUXDB) /* log_name is also used to build short_log_name */
        const char filesystem_postfix[] = "_filesystem_";
        const char filesystem_extension[] = ".log";
        length = libiotrace_get_env(ENV_LOG_NAME, log_name,
                MAXFILENAME - strlen(filesystem_extension)
                        - strlen(filesystem_postfix) - strlen(hostname), 1);
        log_name_len = length;
#endif

#if !defined(IO_LIB_STATIC) && (defined(IOTRACE_ENABLE_LOGFILE) || defined(IOTRACE_ENABLE_INFLUXDB))
        generate_env(log_name_env, ENV_LOG_NAME, length, log_name);
#endif

#ifdef IOTRACE_ENABLE_LOGFILE
        strcpy(filesystem_log_name, log_name);
        strcpy(working_dir_log_name, log_name);
#endif
#if defined(ENABLE_REMOTE_CONTROL) && defined(IOTRACE_ENABLE_LOGFILE)
        strcpy(control_log_name, log_name);
#endif
#ifdef IOTRACE_ENABLE_LOGFILE
        const char log_name_postfix[] = "_iotrace.log";
        strcpy(log_name + length, log_name_postfix);
        log_name_len += sizeof(log_name_postfix);
        strcpy(filesystem_log_name + length, filesystem_postfix);
        strcpy(filesystem_log_name + length + strlen(filesystem_postfix),
                hostname);
        strcpy(
                filesystem_log_name + length + strlen(filesystem_postfix)
                        + strlen(hostname), filesystem_extension);
        strcpy(working_dir_log_name + length, "_working_dir.log");

        io_log_file_buffer_init_process(log_name);
#endif
#if defined(ENABLE_REMOTE_CONTROL) && defined(IOTRACE_ENABLE_LOGFILE)
        strcpy(control_log_name + length, "_control.log");
#endif

#ifdef IOTRACE_ENABLE_INFLUXDB

        // get token from environment
        length = libiotrace_get_env(ENV_INFLUX_TOKEN, influx_token,
                MAX_INFLUX_TOKEN, 1);
        influx_token_len = strlen(influx_token);

#ifndef IO_LIB_STATIC
        generate_env(influx_token_env, ENV_INFLUX_TOKEN, length, influx_token);
#endif

        // get bucket name from environment
        length = libiotrace_get_env(ENV_INFLUX_BUCKET, influx_bucket,
                MAX_INFLUX_BUCKET, 1);
        influx_bucket_len = strlen(influx_bucket);

#ifndef IO_LIB_STATIC
        generate_env(influx_bucket_env, ENV_INFLUX_BUCKET, length,
                influx_bucket);
#endif

        // get organization name from environment
        length = libiotrace_get_env(ENV_INFLUX_ORGANIZATION,
                influx_organization, MAX_INFLUX_ORGANIZATION, 1);
        influx_organization_len = strlen(influx_organization);

#ifndef IO_LIB_STATIC
        generate_env(influx_organization_env, ENV_INFLUX_ORGANIZATION, length,
                influx_organization);
#endif

        // get database ip from environment
        length = libiotrace_get_env(ENV_DATABASE_IP, database_ip,
                MAX_DATABASE_IP, 1);
        database_ip_len = strlen(database_ip);

#ifndef IO_LIB_STATIC
        generate_env(database_ip_env, ENV_DATABASE_IP, length, database_ip);
#endif

        // get database port from environment
        length = libiotrace_get_env(ENV_DATABASE_PORT, database_port,
                MAX_DATABASE_PORT, 1);
        database_port_len = strlen(database_port);

#ifndef IO_LIB_STATIC
        generate_env(database_port_env, ENV_DATABASE_PORT, length,
                database_port);
#endif

#endif

        // Path to wrapper whitelist
        length = libiotrace_get_env(ENV_WRAPPER_WHITELIST, whitelist,
        MAXFILENAME, 0);
        if (0 != length) {
#ifndef IO_LIB_STATIC
            has_whitelist = 1;
            generate_env(whitelist_env, ENV_WRAPPER_WHITELIST, length,
                    whitelist);
#endif

            read_whitelist();
        } else {
#ifndef IO_LIB_STATIC
            has_whitelist = 0;
#endif
        }

#ifndef IO_LIB_STATIC
        length = strlen(ENV_LD_PRELOAD);
        strcpy(ld_preload, ENV_LD_PRELOAD);
        strcpy(ld_preload + length, "=");
        length = libiotrace_get_env(ENV_LD_PRELOAD, ld_preload + length + 1,
        MAXFILENAME, 1);
#endif

#ifndef REALTIME
        system_start_time = iotrace_get_boot_time();
#endif

#ifdef IOTRACE_ENABLE_INFLUXDB
        pthread_mutex_init(&socket_lock, NULL);
#endif
#if defined(IOTRACE_ENABLE_INFLUXDB) && defined(ENABLE_REMOTE_CONTROL)
        pthread_mutex_init(&ip_lock, NULL);
#endif

        /* Initialize user callbacks and settings */
#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
        llhttp_settings_init(&settings);
#endif

        /* Set user callback */
#ifdef ENABLE_REMOTE_CONTROL
        settings.on_url = url_callback_requests;
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
        settings.on_status = url_callback_responses;
#endif

        /* open and configure control socket */
#ifdef ENABLE_REMOTE_CONTROL
        socket_control = prepare_control_socket();
#endif

#ifdef IOTRACE_ENABLE_INFLUXDB
            if (-1 == socket_peer) {
                    prepare_socket();
            }
#endif

#ifdef ENABLE_POWER_MEASUREMENT
        power_measurement_init();
#endif


        /* at this point all preparations necessary for a wrapper call
         * are done: set corresponding flag */
        init_done = 1;

#if defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
        //Create receive thread per process
        pthread_t recv_thread;

        /* pthread_create uses malloc. Call must be done after
         * init_done is set (see pthread_atfork call for details). */
        int ret = CALL_REAL_POSIX_SYNC(pthread_create)(&recv_thread, NULL, communication_thread,
                NULL);
        if (0 != ret) {
            LOG_WARN("pthread_create() failed. (%d)", ret);
            return;
        }
#endif

        /* pthread_atfork uses malloc. malloc could be wrapped (see
         * alloc.h and alloc.c). The call of pthread_atfork must be
         * done after init_done is set to prevent a recursion between
         * pthread_atfork, malloc and init_process.
         * Also communication_thread must be created before
         * pthread_atfork is called, because malloc wrapper waits
         * for socket connection to influxdb to write influx meta
         * data and this connection is created in
         * communication_thread */
        pthread_atfork(NULL, NULL, reset_values_in_forked_process);

#if defined(ENABLE_FILESYSTEM_METADATA) && \
        (defined(IOTRACE_ENABLE_LOGFILE) \
                || defined(IOTRACE_ENABLE_INFLUXDB))
#  ifdef __linux__ // TODO: RAY MacOS; Windows?
        print_filesystem();
#  endif
#endif
#ifdef IOTRACE_ENABLE_LOGFILE
        print_working_directory();
#endif

#ifdef WITH_STD_IO
        /* open_std* functions must be called after communication_thread
         * is created (because they can call init_thread which may call
         * write_metadata_into_influxdb which waits for binding of the
         * control socket in communication_thread). */

        open_std_fd(STDIN_FILENO);
        open_std_fd(STDOUT_FILENO);
        open_std_fd(STDERR_FILENO);

        open_std_file(stdin);
        open_std_file(stdout);
        open_std_file(stderr);
#endif
    }
}

/**
 * Fills stacktrace information to given struct basic.
 *
 * @param[out] data A pointer to a struct basic structure
 */
void get_stacktrace(struct basic *data) {
    int size;
    void *trace = CALL_REAL_ALLOC_SYNC(malloc)(
            sizeof(void*) * (stacktrace_depth + 3));
    char **messages = (char**) NULL;

    if (NULL == trace) {
        LOG_ERROR_AND_DIE("malloc() returned NULL");
    }

    size = backtrace(trace, stacktrace_depth + 3);
    if (0 >= size) {
        LOG_ERROR_AND_DIE("backtrace() returned %d", size);
    }

    if (stacktrace_ptr) {
        LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY((*data), stacktrace_pointer,
                trace, 3, size)
    } else {
        LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL((*data), stacktrace_pointer)
    }

    if (stacktrace_symbol) {
        messages = backtrace_symbols(trace, size);
        if (NULL == messages) {
            LOG_ERROR_AND_DIE("backtrace_symbols() returned NULL with errno=%d",
                               errno);
        }

        LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY((*data), stacktrace_symbols,
                messages, 3, size)
    } else {
        LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL((*data),
                stacktrace_symbols)
    }

    if (!stacktrace_ptr) {
        CALL_REAL_ALLOC_SYNC(free)(trace);
    }
}

/**
 * Initialize libiotrace for current thread.
 *
 * Is called from get_basic() during first call of a wrapper in a thread.
 */
void init_thread(void) {
    tid = iotrace_get_tid();
#ifdef IOTRACE_ENABLE_LOGFILE
    io_log_file_buffer_init_thread();
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
    if (-1 == socket_peer) {
        prepare_socket();
    }
#  ifdef ENABLE_REMOTE_CONTROL
    write_metadata_into_influxdb();
#  endif
#endif

#ifdef STRACING_ENABLED
    stracing_tracee_register_with_stracer();
#endif
}

/**
 * Sets some basic information to the given structure.
 *
 * Fills the "pid", "tid", "hostname" and if
 * needed the stacktrace with the current values.
 * If "get_basic" is called for the first time in a new
 * thread the "init_thread" function is called to
 * initialize libiotrace for the current thread.
 *
 * @param[out] data A pointer to a struct basic structure
 */
void get_basic(struct basic *data) {
    /* tid is thread local storage => no synchronization with
     * other threads is needed */
    if (tid == -1) {
        /* call once per new thread */
        init_thread();
    }

    data->pid = pid;
    data->tid = tid;

    data->hostname = hostname;

    if (0 < stacktrace_depth && (stacktrace_ptr || stacktrace_symbol)) {
        get_stacktrace(data);
    } else {
        LIBIOTRACE_STRUCT_SET_MALLOC_STRING_ARRAY_NULL((*data),
                stacktrace_symbols)
        LIBIOTRACE_STRUCT_SET_MALLOC_PTR_ARRAY_NULL((*data), stacktrace_pointer)
    }
}

/**
 * Sends a struct basic to influxdb.
 *
 * @param[in] data Pointer to struct basic
 */
#ifdef IOTRACE_ENABLE_INFLUXDB
void write_into_influxdb(struct basic *data) {
    if (event_cleanup_done || no_sending) {
        return;
    }

    if (-1 == socket_peer) {
                prepare_socket();
        }

    //buffer for body
    int body_length = libiotrace_struct_push_max_size_basic(0) + 1; /* +1 for trailing null character (function build by macros; gives length of body to send) */
    char body[body_length];
    body_length = libiotrace_struct_push_basic(body, body_length, data, "");
    if (0 > body_length) {
        LOG_ERROR_AND_DIE("libiotrace_struct_push_basic() returned %d",
                body_length);
    }
    body_length--; /*last comma in ret*/
    body[body_length] = '\0'; /*remove last comma*/

    char short_log_name[50];
    shorten_log_name(short_log_name, sizeof(short_log_name), log_name,
            log_name_len);

    const char labels[] =
            "libiotrace,jobname=%s,hostname=%s,processid=%d,thread=%d,functionname=%s";
    int body_labels_length = strlen(labels) + sizeof(short_log_name) /* jobname */
    + HOST_NAME_MAX /* hostname */
    + COUNT_DEC_AS_CHAR(data->pid) + 1 /* processid with sign */
    + COUNT_DEC_AS_CHAR(data->tid) + 1 /* thread with sign */
    + MAX_FUNCTION_NAME; /* functionname */
    char body_labels[body_labels_length];
    snprintf(body_labels, sizeof(body_labels), labels, short_log_name,
            data->hostname, data->pid, data->tid,
            data->function_name);
    body_labels_length = strlen(body_labels);

    int timestamp_length = COUNT_DEC_AS_CHAR(data->time_end);
    char timestamp[timestamp_length];
#ifdef REALTIME
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64, data->time_end);
#else
    snprintf(timestamp, sizeof(timestamp), "%" PRIu64,
            system_start_time + data->time_end);
#endif
    timestamp_length = strlen(timestamp);

    const int content_length = body_labels_length + 1 /*space*/+ body_length + 1 /*space*/
            + timestamp_length;

    const char header[] =
            "POST /api/v2/write?bucket=%s&precision=ns&org=%s HTTP/1.1" LINE_BREAK
            "Host: %s:%s" LINE_BREAK
            "Accept: */*" LINE_BREAK
            "Authorization: Token %s" LINE_BREAK
            "Content-Length: %d" LINE_BREAK
            "Content-Type: application/x-www-form-urlencoded" LINE_BREAK
            LINE_BREAK
            "%s %s %s";
    const int message_length = strlen(header) + influx_bucket_len
            + influx_organization_len + database_ip_len + database_port_len
            + influx_token_len + COUNT_DEC_AS_CHAR(content_length) /* Content-Length */
            + body_labels_length + body_length + timestamp_length;

    //buffer all (header + body)
    char message[message_length + 1];
    snprintf(message, sizeof(message), header, influx_bucket,
            influx_organization, database_ip, database_port, influx_token,
            content_length, body_labels, body, timestamp);

    send_data(message, socket_peer);
}
#endif

/**
 * Frees dynamically allocated memory in struct basic
 *
 * @aram[in] data Pointer to struct basic
 */
void free_memory(struct basic *data) {
    libiotrace_struct_free_basic(data);
}

/**
 * Write buffer and close sockets before process terminates.
 *
 * Is called after main() via "dtor" section or during a call of a
 * wrapper of a "exit*" function. Writes buffer contents to file,
 * closes open connections (sockets) and destroys mutexes.
 */
#if defined(IOTRACE_ENABLE_LOGFILE) || defined(IOTRACE_ENABLE_INFLUXDB) || defined(ENABLE_REMOTE_CONTROL)
void cleanup_process(void) {
    event_cleanup_done = 1;

#ifdef ENABLE_POWER_MEASUREMENT
    power_measurement_cleanup();
#endif

#ifdef IOTRACE_ENABLE_LOGFILE

#ifdef LOG_WRAPPER_TIME
    struct basic data;
    data.time_start = 0;
    data.time_end = 0;
    data.return_state = ok;
    data.return_state_detail = NULL;
#endif

    WRAPPER_TIME_START(data)

    io_log_file_buffer_clear();

#ifdef LOG_WRAPPER_TIME
    get_basic(&data);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
    POSIX_IO_SET_FUNCTION_NAME_NO_WRAPPER(data.function_name);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)
#endif

    WRAPPER_TIME_END(data);

#ifdef LOG_WRAPPER_TIME
#  ifdef FILENAME_RESOLUTION_ENABLED
    fnres_trace_ioevent(&data);
#  endif

    if (active_wrapper_status.cleanup_process && !no_logging) {
        io_log_file_buffer_write(&data);
    }

    io_log_file_buffer_destroy();

    WRAP_FREE(&data)
#endif /* LOG_WRAPPER_TIME */

#endif /* IOTRACE_ENABLE_LOGFILE */

#ifdef FILENAME_RESOLUTION_ENABLED
    //fnres_fin();
#endif

#ifdef IOTRACE_ENABLE_INFLUXDB
    pthread_mutex_lock(&socket_lock);
    for (int i = 0; i < recv_sockets_len; i++) {
        shutdown(recv_sockets[i]->socket, SHUT_WR);
        while (1) {
            fd_set reads;
            FD_ZERO(&reads);
            FD_SET(recv_sockets[i]->socket, &reads);

            int ret = CALL_REAL_POSIX_SYNC(select)(recv_sockets[i]->socket + 1,
                    &reads, NULL, NULL, NULL);
            if (-1 == ret) {
                LOG_WARN("select() returned -1, errno=%d.", errno);
                break;
            }
            if (FD_ISSET(recv_sockets[i]->socket, &reads)) {
                char read[4096];
                int bytes_received = recv(recv_sockets[i]->socket, read, 4096,
                        0);
                if (bytes_received < 1) {
                    // Connection closed by peer
                    break;
                }
            }
        }
        CLOSESOCKET(recv_sockets[i]->socket);
    }
    pthread_mutex_unlock(&socket_lock);

    pthread_mutex_destroy(&socket_lock);
#endif

#if defined(IOTRACE_ENABLE_INFLUXDB) && defined(ENABLE_REMOTE_CONTROL)
    pthread_mutex_destroy(&ip_lock);
#endif
}
#endif

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
#ifndef IO_LIB_STATIC
void check_ld_preload(char *env[], char *const envp[], const char *func) {
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
        LOG_ERROR_AND_DIE(
                "during call of %s envp[] has more elements then buffer (%d)",
                func, MAX_EXEC_ARRAY_LENGTH);
    }

    if (!has_ld_preload) {
        int count_libiotrace_env = 1;
#if defined(IOTRACE_ENABLE_LOGFILE) || defined(IOTRACE_ENABLE_INFLUXDB)
        count_libiotrace_env++;
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
        count_libiotrace_env += 5;
#endif
        if (has_whitelist) {
            count_libiotrace_env++;
        }

        if (MAX_EXEC_ARRAY_LENGTH <= env_element + count_libiotrace_env) {
            LOG_ERROR_AND_DIE(
                    "during call if %s envp[] with added libiotrace-variables has more elements then buffer (%d)",
                    func, MAX_EXEC_ARRAY_LENGTH);
        }
        env[env_element] = &ld_preload[0];
#if defined(IOTRACE_ENABLE_LOGFILE) || defined(IOTRACE_ENABLE_INFLUXDB)
        env[++env_element] = &log_name_env[0];
#endif
#ifdef IOTRACE_ENABLE_INFLUXDB
        env[++env_element] = &database_ip_env[0];
        env[++env_element] = &database_port_env[0];
        env[++env_element] = &influx_token_env[0];
        env[++env_element] = &influx_bucket_env[0];
        env[++env_element] = &influx_organization_env[0];
#endif
        if (has_whitelist) {
            env[++env_element] = &whitelist_env[0];
        }
        env[++env_element] = NULL;
    }
}
#endif

/*******************************************************************************/
/* exec and exit function wrapper                                              */

int WRAP(execve)(const char *filename, char *const argv[], char *const envp[]) {
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

    if (-1 == ret) {
        data.return_state = error;
    } else {
        data.return_state = ok;
    }

    WRAP_END(data, execve)
    return ret;
}

int WRAP(execv)(const char *path, char *const argv[]) {
    int ret;
    struct basic data;
    WRAP_START(data)

    get_basic(&data);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
    POSIX_IO_SET_FUNCTION_NAME(data.function_name);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

    data.return_state = ok;
    CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execv, path, argv)

    if (-1 == ret) {
        data.return_state = error;
    } else {
        data.return_state = ok;
    }

    WRAP_END(data, execv)
    return ret;
}

int WRAP(execl)(const char *path, const char *arg, ... /* (char  *) NULL */) {
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
    element = (char*) ((void*) arg);
    while (NULL != element) {
        if (count >= MAX_EXEC_ARRAY_LENGTH - 1) {
            LOG_ERROR_AND_DIE(
                    "buffer (%d elements) not big enough for argument array",
                    MAX_EXEC_ARRAY_LENGTH);
        }
        argv[count] = element;
        count++;

        element = va_arg(ap, char*);
    }
    argv[count] = element;
    va_end(ap);
    CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execv, path, argv)

    if (-1 == ret) {
        data.return_state = error;
    } else {
        data.return_state = ok;
    }

    WRAP_END(data, execl)
    return ret;
}

int WRAP(execvp)(const char *file, char *const argv[]) {
    int ret;
    struct basic data;
    WRAP_START(data)

    get_basic(&data);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
    POSIX_IO_SET_FUNCTION_NAME(data.function_name);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

    data.return_state = ok;
    CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvp, file, argv)

    if (-1 == ret) {
        data.return_state = error;
    } else {
        data.return_state = ok;
    }

    WRAP_END(data, execvp)
    return ret;
}

int WRAP(execlp)(const char *file, const char *arg, ... /* (char  *) NULL */) {
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
    element = (char*) ((void*) arg);
    while (NULL != element) {
        if (count >= MAX_EXEC_ARRAY_LENGTH - 1) {
            LOG_ERROR_AND_DIE(
                    "buffer (%d elements) not big enough for argument array",
                    MAX_EXEC_ARRAY_LENGTH);
        }
        argv[count] = element;
        count++;

        element = va_arg(ap, char*);
    }
    argv[count] = element;
    va_end(ap);
    CALL_REAL_FUNCTION_RET_NO_RETURN(data, ret, execvp, file, argv)

    if (-1 == ret) {
        data.return_state = error;
    } else {
        data.return_state = ok;
    }

    WRAP_END(data, execlp)
    return ret;
}

#ifdef HAVE_EXECVPE
int WRAP(execvpe)(const char *file, char *const argv[], char *const envp[]) {
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

    if (-1 == ret) {
        data.return_state = error;
    } else {
        data.return_state = ok;
    }

    WRAP_END(data, execvpe)
    return ret;
}
#endif

int WRAP(execle)(const char *path, const char *arg,
        ... /*, (char *) NULL, char * const envp[] */) {
#ifndef HAVE_EXECVPE
    LOG_ERROR_AND_DIE("wrapper needs function execvpe() to work properly");
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
    element = (char*) ((void*) arg);
    while (NULL != element) {
        if (count >= MAX_EXEC_ARRAY_LENGTH - 1) {
            LOG_ERROR_AND_DIE(
                    "buffer (%d elements) not big enough for argument array",
                    MAX_EXEC_ARRAY_LENGTH);
        }
        argv[count] = element;
        count++;

        element = va_arg(ap, char*);
    }
    argv[count] = element;
    envp = va_arg(ap, char**);
    va_end(ap);

#ifndef IO_LIB_STATIC
    char *env[MAX_EXEC_ARRAY_LENGTH];
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

    WRAP_END(data, execle)
    return ret;
}

void WRAP(_exit)(int status) {
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

static void cleanup_thread(void) {
// !!!   TODO: Check whether gets exec'ed when calling `pthread_cancel` (otherwise potential mem-leak) ???   !!!

#ifdef STRACING_ENABLED
    stracing_tracee_fin();
#endif
}

struct pthread_create_data {
    void* (*start_routine)(void*);
    void *restrict arg;
};

void* pthread_create_start_routine(void *arg) {
/* !!!  WARNING: Changing the name of this function requires also a change in stracer's code (tasks/unwind.c: `LIBIOTRACE_PRECLUDED_PTHREAD_FCT`)  !!! */
    struct pthread_create_data *data = (struct pthread_create_data*) arg;
    void *ret;

    if (tid == -1) {
        /* call once per new thread */
        init_thread();
    }

    ret = data->start_routine(data->arg);
    CALL_REAL_ALLOC_SYNC(free)(data);

    cleanup_thread();

    return ret;
}

int WRAP(pthread_create)(pthread_t *restrict thread,
        const pthread_attr_t *restrict attr, void* (*start_routine)(void*),
        void *restrict arg) {
    int ret;
    struct basic data;
    WRAP_START(data)

    get_basic(&data);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, function_data)
    POSIX_IO_SET_FUNCTION_NAME(data.function_name);
    LIBIOTRACE_STRUCT_SET_VOID_P_NULL(data, file_type)

    struct pthread_create_data *pthread_data = CALL_REAL_ALLOC_SYNC(malloc)(
            sizeof(struct pthread_create_data));
    if (NULL == pthread_data) {
        LOG_ERROR_AND_DIE("malloc failed, errno=%d", errno);
    }
    pthread_data->start_routine = start_routine;
    pthread_data->arg = arg;

    CALL_REAL_FUNCTION_RET(data, ret, pthread_create, thread, attr,
            pthread_create_start_routine, pthread_data)

    if (0 == ret) {
        data.return_state = ok;
    } else {
        data.return_state = error;
    }

    WRAP_END(data, pthread_create)
    return ret;
}

/*
 * POWER MEASUREMENT
 */

#ifdef ENABLE_POWER_MEASUREMENT

// cpu-family: 85 = Intel | 23 = AMD
// cpu-model for Intel: 85
// cpu-model for AMD: 113

CpuInfo *cpu_info = NULL;
uint64_t last_time = 0;
char print_buffer[BUFSIZ];

void power_measurement_init(void) {

    last_time = gettime();

    int cpu_family = 0;
    int cpu_model = 0;
    get_cpu_info(&cpu_family, &cpu_model);
    LOG_DEBUG("cpu_family: %d  cpu_model: %d", cpu_family);
    LOG_DEBUG("cpu_model: %d", cpu_model);
    power_measurement_load_cpu_info();

#ifdef  ENABLE_POWER_MEASUREMENT_RAPL
    rapl_init(cpu_family, cpu_model);
#endif


}

void power_measurement_step(void) {
    uint64_t diff = gettime() - last_time;
    if (diff > 1000000000) {

#ifdef  ENABLE_POWER_MEASUREMENT_RAPL
        rapl_measurement();
#endif
        last_time = gettime();
    }
}

void power_measurement_cleanup(void) {

#ifdef  ENABLE_POWER_MEASUREMENT_RAPL
    rapl_free();
#endif
    power_measurement_free_cpu_info();
}
// CPU INFOs

void get_cpu_info(int *family, int *model) {
    FILE *fp = CALL_REAL_POSIX_SYNC(fopen)("/proc/cpuinfo", "r");
    if (fp == NULL) {
        LOG_ERROR_AND_DIE("Error by open /proc/cpuinfo!");
    }

    char line[256];
    while (CALL_REAL_POSIX_SYNC(fgets)(line, sizeof(line), fp) != NULL) {
        int temp_family, temp_model;
        if (sscanf(line, "cpu family\t: %d", &temp_family) == 1) {
            *family = temp_family;
        } else if (sscanf(line, "model\t\t: %d", &temp_model) == 1) {
            *model = temp_model;
        }
    }
    CALL_REAL_POSIX_SYNC(fclose)(fp);
}

int power_measurement_get_number_of_max_cpu_count(void) {
    FILE *file;
    int num_read;
    int cpu_count = 1;

    file = CALL_REAL_POSIX_SYNC(fopen)("/sys/devices/system/cpu/kernel_max", "r");
    if (file == NULL) {
        return 99; //cpu_count;
    }
    num_read = CALL_REAL_POSIX_SYNC(fscanf)(file, "%d", &cpu_count);
    CALL_REAL_POSIX_SYNC(fclose)(file);

    if (num_read == 1) {
        cpu_count++;
    } else {
        cpu_count = 1;
    }
    return cpu_count;
}

void power_measurement_load_cpu_info(void) {
    int package, str_err, num_read;
    char filename[BUFSIZ];
    FILE *cpu_physical_package_id;
    int max_cpu_count = power_measurement_get_number_of_max_cpu_count();


    cpu_info = (CpuInfo *) CALL_REAL_ALLOC_SYNC(calloc)(1, sizeof(CpuInfo));
    cpu_info->cpu_packages = CALL_REAL_ALLOC_SYNC(calloc)(max_cpu_count, sizeof(CpuPackage));

    for (int i = 0; i < max_cpu_count; ++i) {
        cpu_info->cpu_packages[i].id = -1;
        cpu_info->cpu_packages[i].number_cpu_count = 0;
        cpu_info->cpu_packages[i].cpu_ids = NULL;;
    }


    for (int cpu_id = 0; cpu_id < max_cpu_count; ++cpu_id) {
        str_err = snprintf(filename, BUFSIZ, "/sys/devices/system/cpu/cpu%d/topology/physical_package_id", cpu_id);
        filename[BUFSIZ - 1] = 0;
        if (str_err > BUFSIZ) {
            return;
        };

        cpu_physical_package_id = CALL_REAL_POSIX_SYNC(fopen)(filename, "r");
        if (cpu_physical_package_id == NULL) {
            break;
        }
        num_read = CALL_REAL_POSIX_SYNC(fscanf)(cpu_physical_package_id, "%d", &package);
        CALL_REAL_POSIX_SYNC(fclose)(cpu_physical_package_id);

        if (num_read != 1) {
            sprintf(print_buffer, "Error by reading CPU Physical Package Number from CPUId [Read Count: %4d]: %4d\n", num_read, cpu_id);
            CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, strlen(print_buffer));
            CALL_REAL_POSIX_SYNC(fflush)(stdout);
            return;
        }
        sprintf(print_buffer, "INFO CPU Package (CPU %d): %d \n", cpu_id, package);
        CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, strlen(print_buffer));
        CALL_REAL_POSIX_SYNC(fflush)(stdout);

        cpu_info->cpu_count++;

        if (cpu_info->cpu_packages[package].id == -1) {
            cpu_info->cpu_packages[package].cpu_ids = CALL_REAL_ALLOC_SYNC(calloc)(max_cpu_count, sizeof(unsigned int));
            cpu_info->package_count++;
        }
        cpu_info->cpu_packages[package].id = package;
        cpu_info->cpu_packages[package].cpu_ids[cpu_info->cpu_packages[package].number_cpu_count] = cpu_id;
        cpu_info->cpu_packages[package].number_cpu_count += 1;
    }

    sprintf(print_buffer, "[INFO] Total Count CPU: %u | Packages: %u \n", cpu_info->cpu_count, cpu_info->package_count);
    CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, strlen(print_buffer));
    CALL_REAL_POSIX_SYNC(fflush)(stdout);

    for (unsigned int i = 0; i < cpu_info->package_count; ++i) {
        sprintf(print_buffer, "[INFO] Package %d CPUs: %u => ", cpu_info->cpu_packages[i].id, cpu_info->cpu_packages[i].number_cpu_count);
        CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, strlen(print_buffer));

        for (unsigned int pos = 0; pos < cpu_info->cpu_packages[i].number_cpu_count; ++pos) {
            sprintf(print_buffer, "%u, ", cpu_info->cpu_packages[i].cpu_ids[pos]);
            CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, strlen(print_buffer));
        }
        CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, "\n", 1);
        CALL_REAL_POSIX_SYNC(fflush)(stdout);
    }
}

void power_measurement_free_cpu_info(void) {

    for (unsigned int i = 0; i < cpu_info->package_count; ++i) {
        CALL_REAL_ALLOC_SYNC(free)((void*)cpu_info->cpu_packages[i].cpu_ids);
    }
    CALL_REAL_ALLOC_SYNC(free)(cpu_info->cpu_packages);
    CALL_REAL_ALLOC_SYNC(free)(cpu_info);
}

//RAPL

#ifdef  ENABLE_POWER_MEASUREMENT_RAPL

FileState *fd_array = NULL;
CPUMeasurementTask *cpu_measurement_tasks = NULL;
int cpu_measurement_tasks_count = 0;
unsigned int cpu_count = 0;

int power_divisor, time_divisor;
int cpu_energy_divisor, dram_energy_divisor;

int rapl_init(int cpu_family, int cpu_model) {
    switch(cpu_family) {
        case CPU_INTEL:
        case CPU_AMD:
            break;
        default:
            LOG_ERROR_AND_DIE("CPU Family not Supported");
    }

    cpu_count = cpu_info->cpu_count;
    fd_array = CALL_REAL_ALLOC_SYNC(calloc)(cpu_count, sizeof(FileState));
    if (fd_array == NULL) {
        return -1;
    }


    // Setup CPU Values by cpu_family and cpu_model


    unsigned int msr_rapl_power_unit;
    unsigned int msr_pkg_energy_status, msr_pp0_energy_status;

    int package_avail = 0;
    int dram_avail = 0;
    int pp0_avail = 0;
    int pp1_avail = 0;
    int psys_avail = 0;
    int different_units = 0;

    if (cpu_family == CPU_INTEL) {
        if (cpu_model == 85) {
            package_avail = 1;
            pp0_avail = 0;
            pp1_avail = 0;
            dram_avail = 1;
            psys_avail = 0;
            different_units = 1;
        }
    } else if (cpu_family == CPU_AMD) {
        if (cpu_model == 113) {
            package_avail = 1;
            pp0_avail = 1;        // Doesn't work on EPYC?
            pp1_avail = 0;
            dram_avail = 0;
            psys_avail = 0;
            different_units = 0;
        }
    }

    if (package_avail == 0) {
        LOG_ERROR_AND_DIE("ERROR: cpu_model not supported\n");
        return -1;
    }

    //Open all Files

    for (unsigned int i = 0; i < cpu_count; ++i) {
        if (rapl_open_file(i) == -1) {
            LOG_ERROR_AND_DIE("ERROR: Cann't Open CPU File with ID: %3u\n", i);
            return -1;
        }
    }

    // Create Tasks For every CPU (PAPI use only for packet)
    unsigned int task_count = ((package_avail * cpu_count) +
                      (pp0_avail * cpu_count) +
                      (pp1_avail * cpu_count) +
                      (dram_avail * cpu_count) +
                      (psys_avail * cpu_count)) * 2;

    if (cpu_family == CPU_INTEL) {
        task_count += (4 * cpu_count) * 2;
    }

    cpu_measurement_tasks = CALL_REAL_ALLOC_SYNC(calloc)(task_count, sizeof(CPUMeasurementTask));

    //Set cpu_family Values
    if (cpu_family == CPU_INTEL) {
        msr_rapl_power_unit = MSR_INTEL_RAPL_POWER_UNIT;
        msr_pkg_energy_status = MSR_INTEL_PKG_ENERGY_STATUS;
        msr_pp0_energy_status = MSR_INTEL_PP0_ENERGY_STATUS;
    } else if (cpu_family == CPU_AMD) {
        msr_rapl_power_unit = MSR_AMD_RAPL_POWER_UNIT;
        msr_pkg_energy_status = MSR_AMD_PKG_ENERGY_STATUS;
        msr_pp0_energy_status = MSR_AMD_PP0_ENERGY_STATUS;
    }

    sprintf(print_buffer, "TASK COUNT %3u \n", task_count);
    CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, 16);
    CALL_REAL_POSIX_SYNC(fflush)(stdout);

    //CPU Info
    {
        const unsigned int cpu_id = 0;
        int fd = rapl_open_file(cpu_id);
        long long result = rapl_read_msr(fd, msr_rapl_power_unit);

        power_divisor = 1 << ((result >> POWER_UNIT_OFFSET) & POWER_UNIT_MASK);
        cpu_energy_divisor = 1 << ((result >> ENERGY_UNIT_OFFSET) & ENERGY_UNIT_MASK);
        time_divisor = 1 << ((result >> TIME_UNIT_OFFSET) & TIME_UNIT_MASK);

        /* Note! On Haswell-EP DRAM energy is fixed at 15.3uJ	*/
        /* see https://lkml.org/lkml/2015/3/20/582		*/
        /* Knights Landing is the same */
        /* so is Broadwell-EP */
        if (different_units) {
            dram_energy_divisor = 1 << 16;
        } else {
            dram_energy_divisor = cpu_energy_divisor;
        }
    }

    //TODO: create struct send to influx
    LOG_DEBUG("Power units = %.4fW | ", 1.0 / power_divisor);
    LOG_DEBUG("CPU Energy units = %.8fJ | ", 1.0 / cpu_energy_divisor);
    LOG_DEBUG("DRAM Energy units = %.8fJ | ", 1.0 / dram_energy_divisor);
    LOG_DEBUG("Time units = %.8fs ", 1.0 / time_divisor);

    //Create Task
    for (unsigned int cpu_package = 0; cpu_package < cpu_info->package_count; ++cpu_package) {
        const unsigned int cpu_package_id = cpu_info->cpu_packages[cpu_package].id;

# ifdef ENABLE_POWER_MEASUREMENT_PER_KERN
        for (unsigned int cpu_index = 0; cpu_index < cpu_info->cpu_packages[cpu_package].number_cpu_count; ++cpu_index) {
            const unsigned int cpu_id = cpu_info->cpu_packages[cpu_package].cpu_ids[cpu_index];
# else
            const unsigned int cpu_id = 0;
#endif

            if (cpu_family == CPU_INTEL) {
                // "Thermal specification in counts; package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "THERMAL_SPEC_CNT",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_THERMAL_CNT);

                // "Thermal specification for package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "THERMAL_SPEC_CNT",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_MINIMUM_CNT);


                // "Minimum power in counts; package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "MINIMUM_POWER_CNT",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_MINIMUM_CNT);

                // "Minimum power for package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "MINIMUM_POWER",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_MINIMUM);

                // "Maximum power in counts; package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "MAXIMUM_POWER_CNT",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_MAXIMUM_CNT);

                // "Maximum power for package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "MAXIMUM_POWER",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_MAXIMUM);

                // "Maximum time window in counts; package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "MAXIMUM_TIME_WINDOW",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_TIME_WINDOW_CNT);

                // "Maximum time window for package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "MAXIMUM_TIME_WINDOW",
                                 MSR_PKG_POWER_INFO,
                                 PACKAGE_TIME_WINDOW);
            }

            if (package_avail) {
                // "Energy used in counts by chip package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PACKAGE_ENERGY_CNT",
                                 msr_pkg_energy_status,
                                 PACKAGE_ENERGY_CNT);

                // "Energy used by chip package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PACKAGE_ENERGY",
                                 msr_pkg_energy_status,
                                 PACKAGE_ENERGY);
            }
            if (pp1_avail) {
                // "Energy used in counts by Power Plane 1 (Often GPU) on package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PP1_ENERGY_CNT",
                                 MSR_PP1_ENERGY_STATUS,
                                 PACKAGE_ENERGY_CNT);

                // "Energy used by Power Plane 1 (Often GPU) on package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PP1_ENERGY",
                                 MSR_PP1_ENERGY_STATUS,
                                 PACKAGE_ENERGY);


            }
            if (dram_avail) {
                // "Energy used in counts by DRAM on package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PACKAGE_ENERGY_CNT",
                                 MSR_DRAM_ENERGY_STATUS,
                                 PACKAGE_ENERGY_CNT);

                // "Energy used by DRAM on package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "DRAM_ENERGY",
                                 MSR_DRAM_ENERGY_STATUS,
                                 DRAM_ENERGY);
            }
            if (psys_avail) {
                // "Energy used in counts by SoC on package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PSYS_ENERGY_CNT",
                                 MSR_PLATFORM_ENERGY_STATUS,
                                 PACKAGE_ENERGY_CNT);


                // "Energy used by SoC on package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PSYS_ENERGY_CNT",
                                 MSR_PLATFORM_ENERGY_STATUS,
                                 PLATFORM_ENERGY);
            }
            if (pp0_avail) {
                // "Energy used in counts by all cores in package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PP0_ENERGY_CNT",
                                 msr_pp0_energy_status,
                                 PACKAGE_ENERGY_CNT);

                // "Energy used by all cores in package <cpu_package_id>"
                rapl_create_task(&cpu_measurement_tasks[cpu_measurement_tasks_count++],
                                 cpu_id,
                                 cpu_package_id,
                                 "PP0_ENERGY",
                                 msr_pp0_energy_status,
                                 PACKAGE_ENERGY);
            }
# ifdef ENABLE_POWER_MEASUREMENT_PER_KERN
        }
# endif
    }

    //init lastValues
    rapl_measurement();

    return 0;
}


void rapl_create_task(CPUMeasurementTask *cpu_measurement_task, unsigned int cpu_id, unsigned int cpu_package_id, char *name, unsigned int offset_in_file, unsigned int type) {

    strncpy(cpu_measurement_task->name, name, MAX_STR_LEN - 1);

    cpu_measurement_task->offset_in_file = offset_in_file;
    cpu_measurement_task->type = type;

    cpu_measurement_task->cpu_id = cpu_id;
    cpu_measurement_task->cpu_package = cpu_package_id;
    cpu_measurement_task->last_measurement_value = -1;
}

void rapl_free(void) {
    for (unsigned int i = 0; i < cpu_count; ++i) {
        CALL_REAL_POSIX_SYNC(close)(fd_array[i].file_descriptor);
    }
    CALL_REAL_ALLOC_SYNC(free)(fd_array);
}

int rapl_open_file(unsigned int offset) {

    int fd = 0;
    char filename[BUFSIZ];

    if (fd_array[offset].open == 0) {

        sprintf(filename, "/dev/cpu/%u/msr_safe", offset);
        fd = CALL_REAL_POSIX_SYNC(open)(filename, O_RDONLY);

        if (fd < 0) {
            sprintf(filename, "/dev/cpu/%u/msr", offset);
            fd = CALL_REAL_POSIX_SYNC(open)(filename, O_RDONLY);
        }

        if (fd > 0) {
            fd_array[offset].file_descriptor = fd;
            fd_array[offset].open = 1;

            sprintf(print_buffer, "Open File %3d: %3u\n", fd, offset);
            CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, 19);
            CALL_REAL_POSIX_SYNC(fflush)(stdout);
        } else {
            LOG_ERROR_AND_DIE("\n\n\n-------ERROR------\nCant open File for id: %3u\nFile: %s\nFile Descriptor: %d (%d)\nRUN 'chmod 666 /dev/cpu/*/msr'",offset, filename, fd, errno);
        }
    } else {
        fd = fd_array[offset].file_descriptor;
    }

    return fd;
}

void rapl_measurement(void) {
    for (int i = 0; i < cpu_measurement_tasks_count; ++i) {
        int fd = rapl_open_file(cpu_measurement_tasks[i].cpu_id);
        long long measurement_value = rapl_read_msr(fd, cpu_measurement_tasks[i].offset_in_file);

        if (cpu_measurement_tasks[i].last_measurement_value == -1) {
            cpu_measurement_tasks[i].last_measurement_value = measurement_value;
            continue;
        }

        sprintf(print_buffer, "[ %3u | %3u ] ", cpu_measurement_tasks[i].cpu_package, cpu_measurement_tasks[i].cpu_id);
        CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, 14);
        sprintf(print_buffer, "%25s -> %10lld -> %10lld\n", cpu_measurement_tasks[i].name, measurement_value, measurement_value - cpu_measurement_tasks[i].last_measurement_value);
        CALL_REAL_POSIX_SYNC(write)(STDOUT_FILENO, print_buffer, 25 + 3 + 25 + 4 + 10 + 4 + 10 + 1);
        CALL_REAL_POSIX_SYNC(fflush)(stdout);

        CPUMeasurementTask task = cpu_measurement_tasks[i];


        long long difference_to_last_value = 0;

        if (task.type == PACKAGE_ENERGY ||
        task.type == DRAM_ENERGY ||
        task.type == PLATFORM_ENERGY  ||
        task.type == PACKAGE_ENERGY_CNT) {
            if (measurement_value > task.last_measurement_value) {
                difference_to_last_value = measurement_value - task.last_measurement_value;
            } else {
                difference_to_last_value = measurement_value + (0x100000000 - task.last_measurement_value);
            }
        }
        struct power_measurement_data data = {
                gettime(),
                getpid(),
                (int)task.cpu_package,
                (int)task.cpu_id,
                task.name,
                (int)task.type,
                difference_to_last_value,
                rapl_convert_energy(data.type, difference_to_last_value),
                measurement_value,
        };
        write_power_measurement_data_into_influxdb(&data, METHOD_RAPL);

        cpu_measurement_tasks[i].last_measurement_value = measurement_value;
    }
}

long long rapl_convert_energy(int type, long long value) {
    union {
        long long ll;
        double fp;
    } return_val;

    return_val.ll = value;

    if (type==PACKAGE_ENERGY) {
        return_val.ll = (long long)(((double)value/cpu_energy_divisor)*1e9);
    }

    if (type==DRAM_ENERGY) {
        return_val.ll = (long long)(((double)value/dram_energy_divisor)*1e9);
    }

    if (type==PLATFORM_ENERGY) {
        return_val.ll = (long long)(((double)value/cpu_energy_divisor)*1e9);
    }

    if (type==PACKAGE_THERMAL) {
        return_val.fp = (double)((value>>THERMAL_SHIFT)&POWER_INFO_UNIT_MASK)/(double)power_divisor;
    }

    if (type==PACKAGE_MINIMUM) {
        return_val.fp = (double)((value>>MINIMUM_POWER_SHIFT)&POWER_INFO_UNIT_MASK)/(double)power_divisor;
    }

    if (type==PACKAGE_MAXIMUM) {
        return_val.fp = (double)((value>>MAXIMUM_POWER_SHIFT)&POWER_INFO_UNIT_MASK)/(double)power_divisor;
    }

    if (type==PACKAGE_TIME_WINDOW) {
        return_val.fp =  (double)((value>>MAXIMUM_TIME_WINDOW_SHIFT)&POWER_INFO_UNIT_MASK)/(double)time_divisor;
    }

    if (type==PACKAGE_THERMAL_CNT) {
        return_val.ll = ((value>>THERMAL_SHIFT)&POWER_INFO_UNIT_MASK);
    }

    if (type==PACKAGE_MINIMUM_CNT) {
        return_val.ll = ((value>>MINIMUM_POWER_SHIFT)&POWER_INFO_UNIT_MASK);
    }

    if (type==PACKAGE_MAXIMUM_CNT) {
        return_val.ll = ((value>>MAXIMUM_POWER_SHIFT)&POWER_INFO_UNIT_MASK);
    }

    if (type==PACKAGE_TIME_WINDOW_CNT) {
        return_val.ll = ((value>>MAXIMUM_TIME_WINDOW_SHIFT)&POWER_INFO_UNIT_MASK);
    }

    return return_val.ll;
}

long long rapl_read_msr(int file_descriptor, unsigned int offset_in_file) {

    u_int64_t data;

    ssize_t read_size = CALL_REAL_POSIX_SYNC(pread)(file_descriptor, &data, sizeof(data), offset_in_file);

    if (file_descriptor < 0 || read_size != sizeof data) {
        LOG_ERROR_AND_DIE("\n ERROR: Can't read value (%3u) from File: %3d -> %10ld/%10lu \n", offset_in_file, file_descriptor, read_size, sizeof(data));
        return -1;
    }

    return (long long) data & 0xFFFFFFFF;
}

#endif
#endif
