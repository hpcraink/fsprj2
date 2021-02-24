/**
 * @file Implementation of Posix-IO functions.
 */
#include "libiotrace_config.h"

#ifdef HAVE_UNISTD_H
#include <unistd.h>
#endif

#include <wchar.h>
#include <stdio.h>
#include <stdio_ext.h>
#include <stddef.h>
#include <stdint.h>
#include <fcntl.h>
#ifdef HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif

//ToDo: test for existance
#include <sys/un.h> //<afunix.h> on Windows?
//#include <netinet/in.h>
//#include <linux/netlink.h>
#include <sys/uio.h>
#include <sys/mman.h>
#ifdef HAVE_SCHED_H
#include <sched.h>
#endif

#include <string.h>
#include <stdarg.h>
#include <time.h>

#include "json_include_struct.h"
#include "event.h"
#include "wrapper_defines.h"
#include "posix_io.h"

#include "wrapper_name.h"

#ifndef IO_LIB_STATIC
#define HAVE_OPEN_ELLIPSES
#ifdef HAVE_OPEN_ELLIPSES
REAL_DEFINITION_TYPE int REAL_DEFINITION(open)(const char *filename, int flags, ...) REAL_DEFINITION_INIT;
#ifdef HAVE_OPEN64
REAL_DEFINITION_TYPE int REAL_DEFINITION(open64)(const char *filename, int flags, ...) REAL_DEFINITION_INIT;
#endif
#else
REAL_DEFINITION_TYPE int REAL_DEFINITION(open)(const char *filename, int flags, mode_t mode) REAL_DEFINITION_INIT;
#ifdef HAVE_OPEN64
REAL_DEFINITION_TYPE int REAL_DEFINITION(open64)(const char *filename, int flags, mode_t mode) REAL_DEFINITION_INIT;
#endif
#endif
#ifdef HAVE_OPENAT
REAL_DEFINITION_TYPE int REAL_DEFINITION(openat)(int dirfd, const char *pathname, int flags, ...) REAL_DEFINITION_INIT;
//ToDo: test HAVE_OPEN_ELLIPSE
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(creat)(const char *filename, mode_t mode) REAL_DEFINITION_INIT;
#ifdef HAVE_CREAT64
REAL_DEFINITION_TYPE int REAL_DEFINITION(creat64)(const char *filename, mode_t mode) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(close)(int filedes) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(read)(int filedes, void *buffer, size_t size) REAL_DEFINITION_INIT;
#ifdef HAVE_PREAD
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pread)(int filedes, void *buffer, size_t size, off_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PREAD64
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pread64)(int filedes, void *buffer, size_t size, off64_t offset) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(write)(int filedes, const void *buffer, size_t size) REAL_DEFINITION_INIT;
#ifdef HAVE_PWRITE
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pwrite)(int filedes, const void *buffer, size_t size, off_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PWRITE64
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pwrite64)(int filedes, const void *buffer, size_t size, off64_t offset) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE off_t REAL_DEFINITION(lseek)(int filedes, off_t offset, int whence) REAL_DEFINITION_INIT;
#ifdef HAVE_LSEEK64
REAL_DEFINITION_TYPE off64_t REAL_DEFINITION(lseek64)(int filedes, off64_t offset, int whence) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_READV
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(readv)(int filedes, const struct iovec *vector, int count) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_WRITEV
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(writev)(int filedes, const struct iovec *vector, int count) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PREADV
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PREADV64
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(preadv64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PWRITEV
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pwritev)(int fd, const struct iovec *iov, int iovcnt, off_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PWRITEV64
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pwritev64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PREADV2
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(preadv2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PREADV64V2
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(preadv64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PWRITEV2
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pwritev2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PWRITEV64V2
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_COPY_FILE_RANGE
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(copy_file_range)(int inputfd, off64_t *inputpos, int outputfd, off64_t *outputpos, size_t length, unsigned int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MMAP
REAL_DEFINITION_TYPE void *REAL_DEFINITION(mmap)(void *address, size_t length, int protect, int flags, int filedes, off_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MMAP64
REAL_DEFINITION_TYPE void *REAL_DEFINITION(mmap64)(void *address, size_t length, int protect, int flags, int filedes, off64_t offset) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MUNMAP
REAL_DEFINITION_TYPE int REAL_DEFINITION(munmap)(void *addr, size_t length) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MSYNC
REAL_DEFINITION_TYPE int REAL_DEFINITION(msync)(void *address, size_t length, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MREMAP
REAL_DEFINITION_TYPE void *REAL_DEFINITION(mremap)(void *old_address, size_t old_length, size_t new_length, int flags, ...) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MADVISE
REAL_DEFINITION_TYPE int REAL_DEFINITION(madvise)(void *addr, size_t length, int advice) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_POSIX_MADVISE
REAL_DEFINITION_TYPE int REAL_DEFINITION(posix_madvise)(void *addr, size_t len, int advice) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout) REAL_DEFINITION_INIT;
#ifdef HAVE_SYNC
REAL_DEFINITION_TYPE void REAL_DEFINITION(sync)(void) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_SYNCFS
REAL_DEFINITION_TYPE int REAL_DEFINITION(syncfs)(int fd) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FSYNC
REAL_DEFINITION_TYPE int REAL_DEFINITION(fsync)(int fd) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FDATASYNC
REAL_DEFINITION_TYPE int REAL_DEFINITION(fdatasync)(int fd) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(dup)(int oldfd) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(dup2)(int oldfd, int newfd) REAL_DEFINITION_INIT;
#ifdef HAVE_DUP3
REAL_DEFINITION_TYPE int REAL_DEFINITION(dup3)(int oldfd, int newfd, int flags) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fcntl)(int fd, int cmd, ...) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(socket)(int domain, int type, int protocol) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(accept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen) REAL_DEFINITION_INIT;
#ifdef HAVE_ACCEPT4
REAL_DEFINITION_TYPE int REAL_DEFINITION(accept4)(int sockfd, struct sockaddr *addr, socklen_t *addrlen, int flags) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(socketpair)(int domain, int type, int protocol, int sv[2]) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(pipe)(int pipefd[2]) REAL_DEFINITION_INIT;
#ifdef HAVE_PIPE2
REAL_DEFINITION_TYPE int REAL_DEFINITION(pipe2)(int pipefd[2], int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MEMFD_CREATE
REAL_DEFINITION_TYPE int REAL_DEFINITION(memfd_create)(const char *name, unsigned int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_EPOLL_CREATE
REAL_DEFINITION_TYPE int REAL_DEFINITION(epoll_create)(int size) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_EPOLL_CREATE1
REAL_DEFINITION_TYPE int REAL_DEFINITION(epoll_create1)(int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MKSTEMP
REAL_DEFINITION_TYPE int REAL_DEFINITION(mkstemp)(char *template) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MKOSTEMP
REAL_DEFINITION_TYPE int REAL_DEFINITION(mkostemp)(char *template, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MKSTEMPS
REAL_DEFINITION_TYPE int REAL_DEFINITION(mkstemps)(char *template, int suffixlen) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_MKOSTEMPS
REAL_DEFINITION_TYPE int REAL_DEFINITION(mkostemps)(char *template, int suffixlen, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_EVENTFD
REAL_DEFINITION_TYPE int REAL_DEFINITION(eventfd)(unsigned int initval, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_INOTIFY_INIT
REAL_DEFINITION_TYPE int REAL_DEFINITION(inotify_init)(void) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_INOTIFY_INIT1
REAL_DEFINITION_TYPE int REAL_DEFINITION(inotify_init1)(int flags) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE struct dirent *REAL_DEFINITION(readdir)(DIR *dirp) REAL_DEFINITION_INIT;
#ifdef HAVE_DIRFD
REAL_DEFINITION_TYPE int REAL_DEFINITION(dirfd)(DIR *dirp) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(sendmsg)(int sockfd, const struct msghdr *msg, int flags) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(recvmsg)(int sockfd, struct msghdr *msg, int flags) REAL_DEFINITION_INIT;
#ifdef HAVE_SENDMMSG
REAL_DEFINITION_TYPE int REAL_DEFINITION(sendmmsg)(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_RECVMMSG
#ifdef HAVE_RECVMMSG_CONST_TIMESPEC
REAL_DEFINITION_TYPE int REAL_DEFINITION(recvmmsg)(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags, const struct timespec *timeout) REAL_DEFINITION_INIT;
#else
REAL_DEFINITION_TYPE int REAL_DEFINITION(recvmmsg)(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags, struct timespec *timeout) REAL_DEFINITION_INIT;
#endif
#endif

/* POSIX and GNU extension stream */
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(fopen)(const char *filename, const char *opentype) REAL_DEFINITION_INIT;
#ifdef HAVE_FOPEN64
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(fopen64)(const char *filename, const char *opentype) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(freopen)(const char *filename, const char *opentype, FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FREOPEN64
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(freopen64)(const char *filename, const char *opentype, FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FDOPEN
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(fdopen)(int fd, const char *opentype) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fclose)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FCLOSEALL
REAL_DEFINITION_TYPE int REAL_DEFINITION(fcloseall)(void) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FLOCKFILE
REAL_DEFINITION_TYPE void REAL_DEFINITION(flockfile)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FTRYLOCKFILE
REAL_DEFINITION_TYPE int REAL_DEFINITION(ftrylockfile)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FUNLOCKFILE
REAL_DEFINITION_TYPE void REAL_DEFINITION(funlockfile)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FWIDE
REAL_DEFINITION_TYPE int REAL_DEFINITION(fwide)(FILE *stream, int mode) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fputc)(int c, FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(fputwc)(wchar_t wc, FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FPUTC_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(fputc_unlocked)(int c, FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FPUTWC_UNLOCKED
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(fputwc_unlocked)(wchar_t wc, FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(putc_MACRO)(int c, FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(putwc_MACRO)(wchar_t wc, FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_PUTC_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(putc_unlocked_MACRO)(int c, FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PUTWC_UNLOCKED
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(putwc_unlocked_MACRO)(wchar_t wc, FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fputs)(const char *s, FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(fputws)(const wchar_t *ws, FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FPUTS_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(fputs_unlocked)(const char *s, FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FPUTWS_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(fputws_unlocked)(const wchar_t *ws, FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_PUTW
REAL_DEFINITION_TYPE int REAL_DEFINITION(putw)(int w, FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fgetc)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(fgetwc)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FGETC_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(fgetc_unlocked)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FGETWC_UNLOCKED
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(fgetwc_unlocked)(FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(getc_MACRO)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(getwc_MACRO)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_GETC_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(getc_unlocked_MACRO)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_GETWC_UNLOCKED
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(getwc_unlocked_MACRO)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_GETW
REAL_DEFINITION_TYPE int REAL_DEFINITION(getw)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_GETLINE
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(getline)(char **lineptr, size_t *n, FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_GETDELIM
REAL_DEFINITION_TYPE ssize_t REAL_DEFINITION(getdelim)(char **lineptr, size_t *n, int delimiter, FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE char *REAL_DEFINITION(fgets)(char *s, int count, FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE wchar_t *REAL_DEFINITION(fgetws)(wchar_t *ws, int count, FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FGETS_UNLOCKED
REAL_DEFINITION_TYPE char *REAL_DEFINITION(fgets_unlocked)(char *s, int count, FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FGETWS_UNLOCKED
REAL_DEFINITION_TYPE wchar_t *REAL_DEFINITION(fgetws_unlocked)(wchar_t *ws, int count, FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(ungetc)(int c, FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE wint_t REAL_DEFINITION(ungetwc)(wint_t wc, FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE size_t REAL_DEFINITION(fread)(void *data, size_t size, size_t count, FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FREAD_UNLOCKED
REAL_DEFINITION_TYPE size_t REAL_DEFINITION(fread_unlocked)(void *data, size_t size, size_t count, FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE size_t REAL_DEFINITION(fwrite)(const void *data, size_t size, size_t count, FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FWRITE_UNLOCKED
REAL_DEFINITION_TYPE size_t REAL_DEFINITION(fwrite_unlocked)(const void *data, size_t size, size_t count, FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fprintf)(FILE *stream, const char *template, ...) REAL_DEFINITION_INIT;
#ifdef HAVE_FWPRINTF
REAL_DEFINITION_TYPE int REAL_DEFINITION(fwprintf)(FILE *stream, const wchar_t *template, ...) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(vfprintf)(FILE *stream, const char *template, va_list ap) REAL_DEFINITION_INIT;
#ifdef HAVE_VFWPRINTF
REAL_DEFINITION_TYPE int REAL_DEFINITION(vfwprintf)(FILE *stream, const wchar_t *template, va_list ap) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fscanf)(FILE *stream, const char *template, ...) REAL_DEFINITION_INIT;
#ifdef HAVE_FWSCANF
REAL_DEFINITION_TYPE int REAL_DEFINITION(fwscanf)(FILE *stream, const wchar_t *template, ...) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_VFSCANF
REAL_DEFINITION_TYPE int REAL_DEFINITION(vfscanf)(FILE *stream, const char *template, va_list ap) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_VFWSCANF
REAL_DEFINITION_TYPE int REAL_DEFINITION(vfwscanf)(FILE *stream, const wchar_t *template, va_list ap) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(feof)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FEOF_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(feof_unlocked)(FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(ferror)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FERROR_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(ferror_unlocked)(FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE void REAL_DEFINITION(clearerr)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_CLEARERR_UNLOCKED
REAL_DEFINITION_TYPE void REAL_DEFINITION(clearerr_unlocked)(FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE long int REAL_DEFINITION(ftell)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FTELLO
REAL_DEFINITION_TYPE off_t REAL_DEFINITION(ftello)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FTELLO64
REAL_DEFINITION_TYPE off64_t REAL_DEFINITION(ftello64)(FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fseek)(FILE *stream, long int offset, int whence) REAL_DEFINITION_INIT;
#ifdef HAVE_FSEEKO
REAL_DEFINITION_TYPE int REAL_DEFINITION(fseeko)(FILE *stream, off_t offset, int whence) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FSEEKO64
REAL_DEFINITION_TYPE int REAL_DEFINITION(fseeko64)(FILE *stream, off64_t offset, int whence) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE void REAL_DEFINITION(rewind)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(fgetpos)(FILE *stream, fpos_t *position) REAL_DEFINITION_INIT;
#ifdef HAVE_FGETPOS64
REAL_DEFINITION_TYPE int REAL_DEFINITION(fgetpos64)(FILE *stream, fpos64_t *position) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fsetpos)(FILE *stream, const fpos_t *position) REAL_DEFINITION_INIT;
#ifdef HAVE_FSETPOS64
REAL_DEFINITION_TYPE int REAL_DEFINITION(fsetpos64)(FILE *stream, const fpos64_t *position) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(fflush)(FILE *stream) REAL_DEFINITION_INIT;
#ifdef HAVE_FFLUSH_UNLOCKED
REAL_DEFINITION_TYPE int REAL_DEFINITION(fflush_unlocked)(FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE int REAL_DEFINITION(setvbuf)(FILE *stream, char *buf, int mode, size_t size) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE void REAL_DEFINITION(setbuf)(FILE *stream, char *buf) REAL_DEFINITION_INIT;
#ifdef HAVE_SETBUFFER
REAL_DEFINITION_TYPE void REAL_DEFINITION(setbuffer)(FILE *stream, char *buf, size_t size) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_SETLINEBUF
REAL_DEFINITION_TYPE void REAL_DEFINITION(setlinebuf)(FILE *stream) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_FILENO
REAL_DEFINITION_TYPE int REAL_DEFINITION(fileno)(FILE *stream) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(tmpfile)(void) REAL_DEFINITION_INIT;
#ifdef HAVE_TMPFILE64
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(tmpfile64)(void) REAL_DEFINITION_INIT;
#endif
REAL_DEFINITION_TYPE FILE *REAL_DEFINITION(popen)(const char *command, const char *type) REAL_DEFINITION_INIT;

/* Solaris extensions for POSIX stream */
REAL_DEFINITION_TYPE int REAL_DEFINITION(__freadable)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(__fwritable)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(__freading)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(__fwriting)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(__fsetlocking)(FILE *stream, int type) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE void REAL_DEFINITION(_flushlbf)(void) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE void REAL_DEFINITION(__fpurge)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE int REAL_DEFINITION(__flbf)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE size_t REAL_DEFINITION(__fbufsize)(FILE *stream) REAL_DEFINITION_INIT;
REAL_DEFINITION_TYPE size_t REAL_DEFINITION(__fpending)(FILE *stream) REAL_DEFINITION_INIT;

/* other functions, needed for tracking file descriptors and memory mappings */
REAL_DEFINITION_TYPE pid_t REAL_DEFINITION(fork)(void) REAL_DEFINITION_INIT;
#ifdef HAVE_VFORK
REAL_DEFINITION_TYPE pid_t REAL_DEFINITION(vfork)(void) REAL_DEFINITION_INIT;
#endif
#ifdef HAVE_CLONE
REAL_DEFINITION_TYPE int REAL_DEFINITION(clone)(int (*fn)(void *), void *child_stack, int flags, void *arg, ... /* pid_t *ptid, void *newtls, pid_t *ctid */) REAL_DEFINITION_INIT;
#endif
#endif

#ifndef IO_LIB_STATIC
char posix_io_init_done = 0;
/* Initialize pointers for glibc functions. */
void posix_io_init()
{
	if (!posix_io_init_done)
	{

#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_DLSYM
#include "posix_io_wrapper.h"

		posix_io_init_done = 1;
	}
}
#endif

char toggle_posix_wrapper(char *line, char toggle)
{
	char ret = 1;

	if (!strcmp(line, "")) {
		ret = 0;
	}
#undef WRAPPER_NAME_TO_SOURCE
#define WRAPPER_NAME_TO_SOURCE WRAPPER_NAME_TO_SET_VARIABLE
#include "posix_io_wrapper.h"
	else
	{
		ret = 0;
	}

	return ret;
}

enum access_mode get_access_mode(int flags)
{
	int access_mode = flags & O_ACCMODE;

	switch (access_mode)
	{
	case O_RDONLY:
		return read_only;
	case O_WRONLY:
		return write_only;
	case O_RDWR:
		return read_and_write;
	default:
		return unknown_access_mode;
	}
}

enum lock_mode get_lock_mode(int type)
{
	switch (type)
	{
	case FSETLOCKING_INTERNAL:
		return internal;
	case FSETLOCKING_BYCALLER:
		return bycaller;
	case FSETLOCKING_QUERY:
		return query_lock_mode;
	default:
		return unknown_lock_mode;
	}
}

enum lock_mode get_orientation_mode(int mode, char param)
{
	if (mode > 0)
	{
		return wide;
	}
	else if (mode < 0)
	{
		return narrow;
	}
	else if (param)
	{
		return query_orientation_mode;
	}
	else
	{
		return not_set;
	}
}

enum buffer_mode get_buffer_mode(int mode)
{
	switch (mode)
	{
	case _IOFBF:
		return fully_buffered;
	case _IOLBF:
		return line_buffered;
	case _IONBF:
		return unbuffered;
	default:
		return unknown_buffer_mode;
	}
}

enum read_write_state get_return_state_c(int ret)
{
	if (ret == EOF)
	{
		return eof;
	}
	else
	{
		return ok;
	}
}

enum read_write_state get_return_state_wc(wint_t ret)
{
	if (ret == WEOF)
	{
		return eof;
	}
	else
	{
		return ok;
	}
}

enum seek_where get_seek_where(int whence)
{
	switch (whence)
	{
	case SEEK_SET:
		return beginning_of_file;
	case SEEK_CUR:
		return current_position;
	case SEEK_END:
		return end_of_file;
#ifdef HAVE_SEEK_DATA
	case SEEK_DATA:
		return next_data;
#endif
#ifdef HAVE_SEEK_HOLE
	case SEEK_HOLE:
		return next_hole;
#endif
	default:
		return unknown_seek_where;
	}
}

enum lock_type get_lock_type(short type)
{
	switch (type)
	{
	case F_RDLCK:
		return read_lock;
	case F_WRLCK:
		return write_lock;
	case F_UNLCK:
		return unlock;
	default:
		return unknown_lock_type;
	}
}

enum owner_type get_owner_type(int type)
{
	switch (type)
	{
	case F_OWNER_TID:
		return owner_thread;
	case F_OWNER_PID:
		return owner_process;
	case F_OWNER_PGRP:
		return owner_process_group;
	default:
		return unknown_owner_type;
	}
}

enum lease_type get_lease_type(int type)
{
	switch (type)
	{
	case F_RDLCK:
		return read_lease;
	case F_WRLCK:
		return write_lease;
	case F_UNLCK:
		return unlock_lease;
	default:
		return unknown_lease_type;
	}
}

enum madvice_advice get_madvice_advice(int advice)
{
	switch (advice)
	{
	case MADV_NORMAL:
		return normal;
	case MADV_RANDOM:
		return madvice_random;
	case MADV_SEQUENTIAL:
		return sequential;
	case MADV_WILLNEED:
		return willneed;
	case MADV_DONTNEED:
		return dontneed;
#ifdef HAVE_MADV_REMOVE
	case MADV_REMOVE:
		return madvice_remove;
#endif
#ifdef HAVE_MADV_DONTFORK
	case MADV_DONTFORK:
		return dontfork;
#endif
#ifdef HAVE_MADV_DOFORK
	case MADV_DOFORK:
		return dofork;
#endif
#ifdef HAVE_MADV_HWPOISON
	case MADV_HWPOISON:
		return hwpoison;
#endif
#ifdef HAVE_MADV_MERGEABLE
	case MADV_MERGEABLE:
		return mergeable;
#endif
#ifdef HAVE_MADV_UNMERGEABLE
	case MADV_UNMERGEABLE:
		return unmergeable;
#endif
#ifdef HAVE_MADV_SOFT_OFFLINE
	case MADV_SOFT_OFFLINE:
		return soft_offline;
#endif
#ifdef HAVE_MADV_HUGEPAGE
	case MADV_HUGEPAGE:
		return hugepage;
#endif
#ifdef HAVE_MADV_NOHUGEPAGE
	case MADV_NOHUGEPAGE:
		return nohugepage;
#endif
#ifdef HAVE_MADV_DONTDUMP
	case MADV_DONTDUMP:
		return dontdump;
#endif
#ifdef HAVE_MADV_DODUMP
	case MADV_DODUMP:
		return dodump;
#endif
#ifdef HAVE_MADV_FREE
	case MADV_FREE:
		return madvice_free;
#endif
#ifdef HAVE_MADV_WIPEONFORK
	case MADV_WIPEONFORK:
		return wipeonfork;
#endif
#ifdef HAVE_MADV_KEEPONFORK
	case MADV_KEEPONFORK:
		return keeponfork;
#endif
	default:
		return unknown_madvice_advice;
	}
}

enum posix_madvice_advice get_posix_madvice_advice(int advice)
{
	switch (advice)
	{
	case POSIX_MADV_NORMAL:
		return posix_normal;
	case POSIX_MADV_RANDOM:
		return posix_madvice_random;
	case POSIX_MADV_SEQUENTIAL:
		return posix_sequential;
	case POSIX_MADV_WILLNEED:
		return posix_willneed;
	case POSIX_MADV_DONTNEED:
		return posix_dontneed;
	default:
		return unknown_posix_madvice_advice;
	}
}

#if defined(F_GET_RW_HINT) || defined(F_SET_RW_HINT) || defined(F_GET_FILE_RW_HINT) || defined(F_SET_FILE_RW_HINT)
enum hint_write_life get_hint_write_life(uint64_t hint)
{
	switch (hint)
	{
	case RWF_WRITE_LIFE_NOT_SET: /* ToDo: RWF instead of RWH ??? */
		return hint_write_life_not_set;
	case RWH_WRITE_LIFE_NONE:
		return hint_write_life_none;
	case RWH_WRITE_LIFE_SHORT:
		return hint_write_life_short;
	case RWH_WRITE_LIFE_MEDIUM:
		return hint_write_life_medium;
	case RWH_WRITE_LIFE_LONG:
		return hint_write_life_long;
	case RWH_WRITE_LIFE_EXTREME:
		return hint_write_life_extreme;
	default:
		return unknown_hint_write_life;
	}
}
#endif

enum fcntl_cmd get_fcntl_cmd(int cmd)
{
	switch (cmd)
	{
	case F_DUPFD:
		return dupfd;
	case F_DUPFD_CLOEXEC:
		return dupfd_cloexec;
	case F_GETFD:
		return getfd;
	case F_SETFD:
		return setfd;
	case F_GETFL:
		return getfl;
	case F_SETFL:
		return setfl;
	case F_SETLK:
		return setlk;
	case F_SETLKW:
		return setlkw;
	case F_GETLK:
		return getlk;
#ifdef F_OFD_SETLK
	case F_OFD_SETLK:
		return ofd_setlk;
#endif
#ifdef F_OFD_SETLKW
	case F_OFD_SETLKW:
		return ofd_setlkw;
#endif
#ifdef F_OFD_GETLK
	case F_OFD_GETLK:
		return ofd_getlk;
#endif
	case F_GETOWN:
		return getown;
	case F_SETOWN:
		return setown;
	case F_GETOWN_EX:
		return getown_ex;
	case F_SETOWN_EX:
		return setown_ex;
	case F_GETSIG:
		return getsig;
	case F_SETSIG:
		return setsig;
	case F_SETLEASE:
		return setlease;
	case F_GETLEASE:
		return getlease;
	case F_NOTIFY:
		return notify;
	case F_SETPIPE_SZ:
		return setpipe_sz;
	case F_GETPIPE_SZ:
		return getpipe_sz;
#ifdef F_ADD_SEALS
	case F_ADD_SEALS:
		return add_seals;
#endif
#ifdef F_GET_SEALS
	case F_GET_SEALS:
		return get_seals;
#endif
#ifdef F_GET_RW_HINT
	case F_GET_RW_HINT:
		return get_rw_hint;
#endif
#ifdef F_SET_RW_HINT
	case F_SET_RW_HINT:
		return set_rw_hint;
#endif
#ifdef F_GET_FILE_RW_HINT
	case F_GET_FILE_RW_HINT:
		return get_file_rw_hint;
#endif
#ifdef F_SET_FILE_RW_HINT
	case F_SET_FILE_RW_HINT:
		return set_file_rw_hint;
#endif
	default:
		return unknown_fcntl_cmd;
	}
}

void get_creation_flags(const int flags, struct creation_flags *cf)
{
#ifdef HAVE_O_CLOEXEC
	cf->cloexec = flags & O_CLOEXEC ? 1 : 0;
#endif
#ifdef HAVE_O_DIRECTORY
	cf->directory = flags & O_DIRECTORY ? 1 : 0;
#endif
#ifdef HAVE_O_NOFOLLOW
	cf->nofollow = flags & O_NOFOLLOW ? 1 : 0;
#endif
#ifdef HAVE_O_TMPFILE
	cf->tmpfile = flags & O_TMPFILE ? 1 : 0;
#endif
	cf->creat = flags & O_CREAT ? 1 : 0;
	cf->excl = flags & O_EXCL ? 1 : 0;
	cf->noctty = flags & O_NOCTTY ? 1 : 0;
	cf->trunc = flags & O_TRUNC ? 1 : 0;
}

void get_file_descriptor_flags(const int flags,
							   struct file_descriptor_flags *fdf)
{
	fdf->cloexec = flags & FD_CLOEXEC ? 1 : 0;
}

void get_status_flags(const int flags, struct status_flags *sf)
{
#ifdef HAVE_O_DIRECT
	sf->direct = flags & O_DIRECT ? 1 : 0;
#endif
#ifdef HAVE_O_NOATIME
	sf->noatime = flags & O_NOATIME ? 1 : 0;
#endif
#ifdef HAVE_O_PATH
	sf->path = flags & O_PATH ? 1 : 0;
#endif
#ifdef HAVE_O_LARGEFILE
	sf->largefile = flags & O_LARGEFILE ? 1 : 0;
#endif
	sf->append = flags & O_APPEND ? 1 : 0;
	sf->async = flags & O_ASYNC ? 1 : 0;
	sf->dsync = flags & O_DSYNC ? 1 : 0;
	sf->nonblock = flags & O_NONBLOCK ? 1 : 0;
	sf->ndelay = flags & O_NDELAY ? 1 : 0;
	sf->sync = flags & O_SYNC ? 1 : 0;
	sf->initial_append = 0;
	sf->delete_on_close = 0;
	sf->unique_open = 0;
	sf->sequential = 0;
}

#if defined(HAVE_PREADV2) || defined(HAVE_PREADV64V2) || defined(HAVE_PWRITEV2) || defined(HAVE_PWRITEV64V2)
void get_rwf_flags(const int flags, struct rwf_flags *rf)
{
	rf->hipri = flags & RWF_HIPRI ? 1 : 0;
	rf->dsync = flags & RWF_DSYNC ? 1 : 0;
	rf->sync = flags & RWF_SYNC ? 1 : 0;
	rf->nowait = flags & RWF_NOWAIT ? 1 : 0;
#ifdef HAVE_RWF_APPEND
	rf->append = flags & RWF_APPEND ? 1 : 0;
#endif
}
#endif

void get_mode_flags(mode_t mode, struct mode_flags *mf)
{
	mf->read_by_owner = mode & S_IRUSR ? 1 : 0;
	mf->write_by_owner = mode & S_IWUSR ? 1 : 0;
	mf->execute_by_owner = mode & S_IXUSR ? 1 : 0;
	mf->read_by_group = mode & S_IRGRP ? 1 : 0;
	mf->write_by_group = mode & S_IWGRP ? 1 : 0;
	mf->execute_by_group = mode & S_IXGRP ? 1 : 0;
	mf->read_by_others = mode & S_IROTH ? 1 : 0;
	mf->write_by_others = mode & S_IWOTH ? 1 : 0;
	mf->execute_by_others = mode & S_IXOTH ? 1 : 0;
}

void get_directory_notify_flags(int flags, struct directory_notify_flags *dnf)
{
	dnf->directory_access = flags & DN_ACCESS ? 1 : 0;
	dnf->directory_modify = flags & DN_MODIFY ? 1 : 0;
	dnf->directory_create = flags & DN_CREATE ? 1 : 0;
	dnf->directory_delete = flags & DN_DELETE ? 1 : 0;
	dnf->directory_rename = flags & DN_RENAME ? 1 : 0;
	dnf->directory_attribute = flags & DN_ATTRIB ? 1 : 0;
	dnf->directory_multishot = flags & DN_MULTISHOT ? 1 : 0;
}

#if defined(F_ADD_SEALS) || defined(F_GET_SEALS)
void get_seal_flags(int flags, struct seal_flags *sf)
{
	sf->seal_seal = flags & F_SEAL_SEAL ? 1 : 0;
	sf->seal_shrink = flags & F_SEAL_SHRINK ? 1 : 0;
	sf->seal_grow = flags & F_SEAL_GROW ? 1 : 0;
	sf->seal_write = flags & F_SEAL_WRITE ? 1 : 0;
}
#endif

void get_memory_protection_flags(int protect,
								 struct memory_protection_flags *mmf)
{
	mmf->executed = protect & PROT_EXEC ? 1 : 0;
	mmf->read = protect & PROT_READ ? 1 : 0;
	mmf->written = protect & PROT_WRITE ? 1 : 0;
}

#ifdef HAVE_MREMAP
void get_memory_remap_flags(int flags, struct memory_remap_flags *mrf)
{
	mrf->maymove = flags & MREMAP_MAYMOVE ? 1 : 0;
	mrf->fixed = flags & MREMAP_FIXED ? 1 : 0;
}
#endif

void get_memory_map_flags(int flags, struct memory_map_flags *mpf)
{
	mpf->shared = flags & MAP_SHARED ? 1 : 0;
	mpf->private = flags & MAP_PRIVATE ? 1 : 0;
	mpf->fixed = flags & MAP_FIXED ? 1 : 0;
#ifdef HAVE_MAP_32BIT
	mpf->bit32 = flags & MAP_32BIT ? 1 : 0;
#endif
#ifdef HAVE_MAP_ANONYMOUS
	mpf->anonymous = flags & MAP_ANONYMOUS ? 1 : 0;
#endif
#ifdef HAVE_MAP_DENYWRITE
	mpf->denywrite = flags & MAP_DENYWRITE ? 1 : 0;
#endif
#ifdef HAVE_MAP_EXECUTABLE
	mpf->executable = flags & MAP_EXECUTABLE ? 1 : 0;
#endif
#ifdef HAVE_MAP_FILE
	mpf->file = flags & MAP_FILE ? 1 : 0;
#endif
#ifdef HAVE_MAP_GROWSDOWN
	mpf->growsdown = flags & MAP_GROWSDOWN ? 1 : 0;
#endif
#ifdef HAVE_MAP_HUGETLB
	mpf->hugetlb = flags & MAP_HUGETLB ? 1 : 0;
#endif
#ifdef HAVE_MAP_HUGE_2MB
	mpf->huge_2mb = flags & MAP_HUGE_2MB ? 1 : 0;
#endif
#ifdef HAVE_MAP_HUGE_1GB
	mpf->huge_1gb = flags & MAP_HUGE_1GB ? 1 : 0;
#endif
#ifdef HAVE_MAP_LOCKED
	mpf->locked = flags & MAP_LOCKED ? 1 : 0;
#endif
#ifdef HAVE_MAP_NONBLOCK
	mpf->nonblock = flags & MAP_NONBLOCK ? 1 : 0;
#endif
#ifdef HAVE_MAP_NORESERVE
	mpf->noreserve = flags & MAP_NORESERVE ? 1 : 0;
#endif
#ifdef HAVE_MAP_POPULATE
	mpf->populate = flags & MAP_POPULATE ? 1 : 0;
#endif
#ifdef HAVE_MAP_STACK
	mpf->stack = flags & MAP_STACK ? 1 : 0;
#endif
#ifdef HAVE_MAP_UNINITIALIZED
	mpf->uninitialized = flags & MAP_UNINITIALIZED ? 1 : 0;
#endif
}

void get_memory_sync_flags(int flags, struct memory_sync_flags *msf)
{
	msf->sync = flags & MS_SYNC ? 1 : 0;
	msf->async = flags & MS_ASYNC ? 1 : 0;
	msf->invalidate = flags & MS_INVALIDATE ? 1 : 0;
}

enum access_mode check_mode(const char *mode, struct creation_flags *cf,
							struct status_flags *sf)
{
#ifdef HAVE_O_DIRECTORY
	cf->directory = 0;
#endif
#ifdef HAVE_O_NOFOLLOW
	cf->nofollow = 0;
#endif
#ifdef HAVE_O_TMPFILE
	cf->tmpfile = 0;
#endif
	cf->noctty = 0;
#ifdef HAVE_O_DIRECT
	sf->direct = 0;
#endif
#ifdef HAVE_O_NOATIME
	sf->noatime = 0;
#endif
#ifdef HAVE_O_PATH
	sf->path = 0;
#endif
#ifdef HAVE_O_LARGEFILE
	sf->largefile = 0;
#endif
	sf->async = 0;
	sf->dsync = 0;
	sf->nonblock = 0;
	sf->ndelay = 0;
	sf->sync = 0;
	sf->delete_on_close = 0;
	sf->unique_open = 0;
	sf->sequential = 0;
	sf->initial_append = 0;

	// ToDo: c
	// ToDo: m
	// ToDo: ,ccs=<string>
	// ToDo: largefile from first write/read?
#ifdef HAVE_O_CLOEXEC
	if (strchr(mode, 'e') != NULL)
	{
		cf->cloexec = 1;
	}
	else
	{
		cf->cloexec = 0;
	}
#endif

	if (strchr(mode, 'x') != NULL)
	{
		cf->excl = 1;
	}
	else
	{
		cf->excl = 0;
	}

	if (strchr(mode, 'r') != NULL)
	{
		cf->creat = 0;
		cf->trunc = 0;
		sf->append = 0;
		if (strchr(mode, '+') == NULL)
		{
			return read_only;
		}
		else
		{
			return read_and_write;
		}
	}
	else if (strchr(mode, 'w') != NULL)
	{
		cf->creat = 1;
		cf->trunc = 1;
		sf->append = 0;
		if (strchr(mode, '+') == NULL)
		{
			return write_only;
		}
		else
		{
			return read_and_write;
		}
	}
	else if (strchr(mode, 'a') != NULL)
	{
		cf->creat = 1;
		cf->trunc = 0;
		sf->append = 1;
		if (strchr(mode, '+') == NULL)
		{
			return write_only;
		}
		else
		{
			return read_and_write;
		}
	}
	else
	{
		cf->creat = 0;
		cf->trunc = 0;
		sf->append = 0;
		return unknown_access_mode;
	}
}

enum boolean is_connection_based(int type)
{
	int tmp = type & ~SOCK_NONBLOCK & ~SOCK_CLOEXEC;

	switch (tmp)
	{
	case SOCK_STREAM:
	case SOCK_SEQPACKET:
		return true;
	default:
		return false;
	}
}

void cstring_to_hex(const char *cstring, char *hexstring, size_t length_cstring)
{
	const char *hex = "0123456789abcdef";

	for (; length_cstring > 0; length_cstring--)
	{
		*hexstring++ = hex[((unsigned char)*cstring >> 4)];
		*hexstring++ = hex[(*cstring) & 0x0f];
		cstring++;
	}

	*hexstring = '\0';
}

void get_sockaddr(const struct sockaddr *addr,
				  struct sockaddr_function *sockaddr_data, const socklen_t addrlen,
				  char *hex_addr)
{
	size_t len;

	sockaddr_data->family = addr->sa_family;

	if (sockaddr_data->family == AF_UNIX)
	{
		if (sizeof(sa_family_t) >= addrlen)
		{
			// unnamed socket
			hex_addr[0] = '\0';
		}
		else
		{
			struct sockaddr_un *addr_un = (struct sockaddr_un *)addr;
			if (addr_un->sun_path[0] == '\0')
			{
				// abstract socket
				cstring_to_hex(addr_un->sun_path, hex_addr,
							   (addrlen - offsetof(struct sockaddr_un, sun_path)));
			}
			else
			{
				// pathname socket
				len = addrlen - offsetof(struct sockaddr_un, sun_path);
				memcpy((void *)hex_addr, (void *)addr_un->sun_path, len);
				hex_addr[len] = '\0';
			}
		}
	}
	else
	{

		//		switch (sockaddr_data->family) {
		//		case AF_INET:
		//			//len = addrlen - offsetof(struct sockaddr, sa_data)
		//			len = sizeof(struct sockaddr_in) - sizeof(sa_family_t);
		//			break;
		//		case AF_INET6:
		//			len = sizeof(struct sockaddr_in6) - sizeof(sa_family_t);
		//			break;
		//		case AF_NETLINK:
		//			len = sizeof(struct sockaddr_nl) - sizeof(sa_family_t);
		//			break;
		//		default:
		len = 0;
		//		}

		cstring_to_hex(addr->sa_data, hex_addr, len);
	}

	sockaddr_data->address = hex_addr;
}

#ifdef HAVE_OPEN_ELLIPSES
int WRAP(open)(const char *filename, int flags, ...)
{
#else
//int WRAP(open)(const char *filename, int flags) {
//	return WRAP(open)(filename, flags, 0); // ToDo: get default mode instead of 0
//}
int WRAP(open)(const char *filename, int flags, mode_t mode)
{
#endif
	int ret;
	char expanded_symlinks[MAXFILENAME];
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

#ifdef HAVE_OPEN_ELLIPSES
	if (__OPEN_NEEDS_MODE(flags))
	{
		va_list ap;
		mode_t mode;
		va_start(ap, flags); //get_mode_flags
		mode = va_arg(ap, mode_t);
		va_end(ap);
		get_mode_flags(mode, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags, mode)
	}
	else
	{
		get_mode_flags(0, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags)
	}
#else
	// ToDo: mode_t mode = os_getmode();
	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags, mode)
#endif

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	WRAP_END(data, open)
	return ret;
}

#ifdef HAVE_OPEN64
#ifdef HAVE_OPEN_ELLIPSES
int WRAP(open64)(const char *filename, int flags, ...)
{
#else
int WRAP(open64)(const char *filename, int flags, mode_t mode)
{
#endif
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

#ifdef HAVE_OPEN_ELLIPSES
	if (__OPEN_NEEDS_MODE(flags))
	{
		va_list ap;
		mode_t mode;
		va_start(ap, flags); //get_mode_flags
		mode = va_arg(ap, mode_t);
		va_end(ap);
		get_mode_flags(mode, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open64, filename, flags, mode)
	}
	else
	{
		get_mode_flags(0, &open_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, open64, filename, flags)
	}
#else
	// ToDo: mode_t mode = os_getmode();
	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, open64, filename, flags, mode)
#endif

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	WRAP_END(data, open64)
	return ret;
}
#endif

#ifdef HAVE_OPENAT
#ifdef HAVE_OPEN_ELLIPSES
int WRAP(openat)(int dirfd, const char *pathname, int flags, ...)
{
#else
int WRAP(openat)(int dirfd, const char *pathname, int flags, mode_t mode)
{
#endif
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct openat_function openat_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, openat_function, openat_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	openat_data.file_name = pathname;
	openat_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &openat_data.creation);
	get_status_flags(flags, &openat_data.status);
	openat_data.file_descriptor = dirfd;
	if (AT_FDCWD == dirfd)
	{
		openat_data.relative_to = current_working_dir;
	}
	else
	{
		openat_data.relative_to = file;
	}

#ifdef HAVE_OPEN_ELLIPSES
	if (__OPEN_NEEDS_MODE(flags))
	{
		va_list ap;
		mode_t mode;
		va_start(ap, flags);
		mode = va_arg(ap, mode_t);
		va_end(ap);
		get_mode_flags(mode, &openat_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, openat, dirfd, pathname, flags, mode)
	}
	else
	{
		get_mode_flags(0, &openat_data.file_mode);
		CALL_REAL_FUNCTION_RET(data, ret, openat, dirfd, pathname, flags)
	}
#else
	// ToDo: mode_t mode = os_getmode();
	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, open, filename, flags, mode)
#endif

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(openat_data.id));
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	WRAP_END(data, openat)
	return ret;
}
#endif

int WRAP(creat)(const char *filename, mode_t mode)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	int flags = O_CREAT | O_WRONLY | O_TRUNC;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, creat, filename, mode)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	WRAP_END(data, creat)
	return ret;
}

#ifdef HAVE_CREAT64
int WRAP(creat64)(const char *filename, mode_t mode)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	int flags = O_CREAT | O_WRONLY | O_TRUNC;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = get_access_mode(flags);
	get_creation_flags(flags, &open_data.creation);
	get_status_flags(flags, &open_data.status);

	get_mode_flags(mode, &open_data.file_mode);
	CALL_REAL_FUNCTION_RET(data, ret, creat64, filename, mode)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	WRAP_END(data, creat64)
	return ret;
}
#endif

int WRAP(close)(int filedes)
{
	int ret;
	struct file_descriptor file_descriptor_data;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, close, filedes)

	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, close)
	return ret;
}

ssize_t WRAP(read)(int filedes, void *buffer, size_t size)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, read, filedes, buffer, size)

	if (ret == -1)
	{
		data.return_state = error;
		read_data.read_bytes = 0;
	}
	else if (ret == 0 && size != 0)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data, read)
	return ret;
}

#ifdef HAVE_PREAD
ssize_t WRAP(pread)(int filedes, void *buffer, size_t size, off_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pread, filedes, buffer, size, offset)

	if (ret == -1)
	{
		data.return_state = error;
		pread_data.read_bytes = 0;
	}
	else if (ret == 0 && size != 0)
	{
		data.return_state = eof;
		pread_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data, pread)
	return ret;
}
#endif

#ifdef HAVE_PREAD64
ssize_t WRAP(pread64)(int filedes, void *buffer, size_t size, off64_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pread64, filedes, buffer, size, offset)

	if (-1 == ret)
	{
		data.return_state = error;
		pread_data.read_bytes = 0;
	}
	else if (0 == ret && 0 != size)
	{
		data.return_state = eof;
		pread_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data, pread64)
	return ret;
}
#endif

ssize_t WRAP(write)(int filedes, const void *buffer, size_t size)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, write, filedes, buffer, size)

	if (-1 == ret)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data, write)
	return ret;
}

#ifdef HAVE_PWRITE
ssize_t WRAP(pwrite)(int filedes, const void *buffer, size_t size, off_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwrite, filedes, buffer, size, offset)

	if (-1 == ret)
	{
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data, pwrite)
	return ret;
}
#endif

#ifdef HAVE_PWRITE64
ssize_t WRAP(pwrite64)(int filedes, const void *buffer, size_t size,
					   off64_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwrite, filedes, buffer, size, offset)

	if (-1 == ret)
	{
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data, pwrite64)
	return ret;
}
#endif

off_t WRAP(lseek)(int filedes, off_t offset, int whence)
{
	off_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct lpositioning_function lpositioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lpositioning_function,
						   lpositioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, lseek, filedes, offset, whence)

	lpositioning_data.offset = offset;
	lpositioning_data.relative_to = get_seek_where(whence);
	lpositioning_data.new_offset_relative_to_beginning_of_file = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, lseek)
	return ret;
}

#ifdef HAVE_LSEEK64
off64_t WRAP(lseek64)(int filedes, off64_t offset, int whence)
{
	off64_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct lpositioning_function lpositioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lpositioning_function,
						   lpositioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, lseek64, filedes, offset, whence)

	lpositioning_data.offset = offset;
	lpositioning_data.relative_to = get_seek_where(whence);
	lpositioning_data.new_offset_relative_to_beginning_of_file = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, lseek64)
	return ret;
}
#endif

#ifdef HAVE_READV
ssize_t WRAP(readv)(int filedes, const struct iovec *vector, int count)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, readv, filedes, vector, count)

	if (ret == -1)
	{
		data.return_state = error;
		read_data.read_bytes = 0;
	}
	else if (ret == 0 && count != 0)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data, readv)
	return ret;
}
#endif

#ifdef HAVE_WRITEV
ssize_t WRAP(writev)(int filedes, const struct iovec *vector, int count)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, writev, filedes, vector, count)

	if (-1 == ret)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data, writev)
	return ret;
}
#endif

#ifdef HAVE_PREADV
ssize_t WRAP(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, preadv, fd, iov, iovcnt, offset)

	if (ret == -1)
	{
		data.return_state = error;
		pread_data.read_bytes = 0;
	}
	else if (ret == 0 && iovcnt != 0)
	{
		data.return_state = eof;
		pread_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data, preadv)
	return ret;
}
#endif

#ifdef HAVE_PREADV64
ssize_t WRAP(preadv64)(int fd, const struct iovec *iov, int iovcnt,
					   off64_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pread_function pread_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread_function, pread_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pread_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, preadv64, fd, iov, iovcnt, offset)

	if (ret == -1)
	{
		data.return_state = error;
		pread_data.read_bytes = 0;
	}
	else if (ret == 0 && iovcnt != 0)
	{
		data.return_state = eof;
		pread_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pread_data.read_bytes = ret;
	}

	WRAP_END(data, preadv64)
	return ret;
}
#endif

#ifdef HAVE_PWRITEV
ssize_t WRAP(pwritev)(int fd, const struct iovec *iov, int iovcnt, off_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwritev, fd, iov, iovcnt, offset)

	if (-1 == ret)
	{
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data, pwritev)
	return ret;
}
#endif

#ifdef HAVE_PWRITEV64
ssize_t WRAP(pwritev64)(int fd, const struct iovec *iov, int iovcnt,
						off64_t offset)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pwrite_function pwrite_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite_function, pwrite_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pwrite_data.position = offset;

	CALL_REAL_FUNCTION_RET(data, ret, pwritev64, fd, iov, iovcnt, offset)

	if (-1 == ret)
	{
		data.return_state = error;
		pwrite_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pwrite_data.written_bytes = ret;
	}

	WRAP_END(data, pwritev64)
	return ret;
}
#endif

#ifdef HAVE_PREADV2
ssize_t WRAP(preadv2)(int fd, const struct iovec *iov, int iovcnt, off_t offset,
					  int flags)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pread2_function pread2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread2_function, pread2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pread2_data.position = offset;
	get_rwf_flags(flags, &pread2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, preadv2, fd, iov, iovcnt, offset, flags)

	if (ret == -1)
	{
		data.return_state = error;
		pread2_data.read_bytes = 0;
	}
	else if (ret == 0 && iovcnt != 0)
	{
		data.return_state = eof;
		pread2_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pread2_data.read_bytes = ret;
	}

	WRAP_END(data, preadv2)
	return ret;
}
#endif

#ifdef HAVE_PREADV64V2
ssize_t WRAP(preadv64v2)(int fd, const struct iovec *iov, int iovcnt,
						 off64_t offset, int flags)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pread2_function pread2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pread2_function, pread2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pread2_data.position = offset;
	get_rwf_flags(flags, &pread2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, preadv64v2, fd, iov, iovcnt, offset,
						   flags)

	if (ret == -1)
	{
		data.return_state = error;
		pread2_data.read_bytes = 0;
	}
	else if (ret == 0 && iovcnt != 0)
	{
		data.return_state = eof;
		pread2_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pread2_data.read_bytes = ret;
	}

	WRAP_END(data, preadv64v2)
	return ret;
}
#endif

#ifdef HAVE_PWRITEV2
ssize_t WRAP(pwritev2)(int fd, const struct iovec *iov, int iovcnt,
					   off_t offset, int flags)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pwrite2_function pwrite2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite2_function, pwrite2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pwrite2_data.position = offset;
	get_rwf_flags(flags, &pwrite2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, pwritev2, fd, iov, iovcnt, offset, flags)

	if (-1 == ret)
	{
		data.return_state = error;
		pwrite2_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pwrite2_data.written_bytes = ret;
	}

	WRAP_END(data, pwritev2)
	return ret;
}
#endif

#ifdef HAVE_PWRITEV64V2
ssize_t WRAP(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt,
						  off64_t offset, int flags)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct pwrite2_function pwrite2_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, pwrite2_function, pwrite2_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	pwrite2_data.position = offset;
	get_rwf_flags(flags, &pwrite2_data.flags);

	CALL_REAL_FUNCTION_RET(data, ret, pwritev64v2, fd, iov, iovcnt, offset,
						   flags)

	if (-1 == ret)
	{
		data.return_state = error;
		pwrite2_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		pwrite2_data.written_bytes = ret;
	}

	WRAP_END(data, pwritev64v2)
	return ret;
}
#endif

#ifdef HAVE_COPY_FILE_RANGE
ssize_t WRAP(copy_file_range)(int inputfd, off64_t *inputpos, int outputfd,
							  off64_t *outputpos, size_t length, unsigned int flags)
{
	ssize_t ret;
	struct basic data;
	struct file_descriptor file_descriptor_read_data;
	struct file_descriptor file_descriptor_write_data;
	struct copy_read_function copy_read_data;
	struct copy_write_function copy_write_data;
	WRAP_START(data)

	get_basic(&data);
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	if (NULL != inputpos)
	{
		copy_read_data.relative_to = beginning_of_file;
		copy_read_data.position = *inputpos;
	}
	else
	{
		copy_read_data.relative_to = current_position;
		copy_read_data.position = 0;
	}
	if (NULL != outputpos)
	{
		copy_write_data.relative_to = beginning_of_file;
		copy_write_data.position = *outputpos;
	}
	else
	{
		copy_write_data.relative_to = current_position;
		copy_write_data.position = 0;
	}
	copy_read_data.to_file_descriptor = outputfd;
	copy_write_data.from_file_descriptor = inputfd;

	CALL_REAL_FUNCTION_RET(data, ret, copy_file_range, inputfd, inputpos,
						   outputfd, outputpos, length, flags)

	if (ret == -1)
	{
		data.return_state = error;
		copy_read_data.read_bytes = 0;
		copy_write_data.written_bytes = 0;
	}
	else if (ret == 0 && length != 0)
	{
		data.return_state = eof;
		copy_read_data.read_bytes = 0;
		copy_write_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		copy_read_data.read_bytes = ret;
		copy_write_data.written_bytes = ret;
	}

	file_descriptor_read_data.descriptor = inputfd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_read_data)
	JSON_STRUCT_SET_VOID_P(data, function_data, copy_read_function,
						   copy_read_data)
	WRAP_END(data, copy_file_range)
	file_descriptor_write_data.descriptor = outputfd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_write_data)
	JSON_STRUCT_SET_VOID_P(data, function_data, copy_write_function,
						   copy_write_data)
	WRAP_END(data, copy_file_range)
	return ret;
}
#endif

#ifdef HAVE_MMAP
void *WRAP(mmap)(void *address, size_t length, int protect, int flags,
				 int filedes, off_t offset)
{
	void *ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct memory_map_function memory_map_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_map_function,
						   memory_map_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	get_memory_protection_flags(protect, &memory_map_data.protection_flags);
	get_memory_map_flags(flags, &memory_map_data.map_flags);
	memory_map_data.offset = offset;
	memory_map_data.length = length;

	CALL_REAL_FUNCTION_RET(data, ret, mmap, address, length, protect, flags,
						   filedes, offset)

	if (MAP_FAILED == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}
	memory_map_data.address = ret;

	WRAP_END(data, mmap)
	return ret;
}
#endif

#ifdef HAVE_MMAP64
void *WRAP(mmap64)(void *address, size_t length, int protect, int flags,
				   int filedes, off64_t offset)
{
	void *ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct memory_map_function memory_map_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_map_function,
						   memory_map_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = filedes;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	get_memory_protection_flags(protect, &memory_map_data.protection_flags);
	get_memory_map_flags(flags, &memory_map_data.map_flags);
	memory_map_data.offset = offset;
	memory_map_data.length = length;

	CALL_REAL_FUNCTION_RET(data, ret, mmap64, address, length, protect, flags,
						   filedes, offset)

	if (MAP_FAILED == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}
	memory_map_data.address = ret;

	WRAP_END(data, mmap64)
	return ret;
}
#endif

#ifdef HAVE_MUNMAP
int WRAP(munmap)(void *addr, size_t length)
{
	int ret;
	struct basic data;
	struct file_memory file_memory_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	file_memory_data.length = length;
	file_memory_data.address = addr;

	CALL_REAL_FUNCTION_RET(data, ret, munmap, addr, length)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, munmap)
	return ret;
}
#endif

#ifdef HAVE_MSYNC
int WRAP(msync)(void *address, size_t length, int flags)
{
	int ret;
	struct basic data;
	struct file_memory file_memory_data;
	struct memory_sync_function memory_sync_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_sync_function,
						   memory_sync_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	get_memory_sync_flags(flags, &memory_sync_data.sync_flags);
	file_memory_data.length = length;
	file_memory_data.address = address;

	CALL_REAL_FUNCTION_RET(data, ret, msync, address, length, flags)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, msync)
	return ret;
}
#endif

#ifdef HAVE_MREMAP
void *WRAP(mremap)(void *old_address, size_t old_length, size_t new_length,
				   int flags, ...)
{
	void *ret;
	struct basic data;
	struct file_memory file_memory_data;
	struct memory_remap_function memory_remap_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_remap_function,
						   memory_remap_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	get_memory_remap_flags(flags, &memory_remap_data.remap_flags);
	file_memory_data.length = old_length;
	file_memory_data.address = old_address;
	memory_remap_data.new_length = new_length;

	if (memory_remap_data.remap_flags.maymove && memory_remap_data.remap_flags.fixed)
	{
		va_list ap;
		void *new_address;
		va_start(ap, flags);
		new_address = va_arg(ap, void *);
		va_end(ap);
		CALL_REAL_FUNCTION_RET(data, ret, mremap, old_address, old_length,
							   new_length, flags, new_address)
	}
	else
	{
		CALL_REAL_FUNCTION_RET(data, ret, mremap, old_address, old_length,
							   new_length, flags)
	}

	if (MAP_FAILED == ret)
	{
		data.return_state = error;
	}
	else
	{
		memory_remap_data.new_address = ret;
		data.return_state = ok;
	}

	WRAP_END(data, mremap)
	return ret;
}
#endif

#ifdef HAVE_MADVISE
int WRAP(madvise)(void *addr, size_t length, int advice)
{
	int ret;
	struct basic data;
	struct file_memory file_memory_data;
	struct memory_madvise_function memory_madvise_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_madvise_function,
						   memory_madvise_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	memory_madvise_data.advice = get_madvice_advice(advice);
	file_memory_data.length = length;
	file_memory_data.address = addr;

	CALL_REAL_FUNCTION_RET(data, ret, madvise, addr, length, advice)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, madvise)
	return ret;
}
#endif

#ifdef HAVE_POSIX_MADVISE
int WRAP(posix_madvise)(void *addr, size_t len, int advice)
{
	int ret;
	struct basic data;
	struct file_memory file_memory_data;
	struct memory_posix_madvise_function memory_posix_madvise_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, memory_posix_madvise_function,
						   memory_posix_madvise_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_memory, file_memory_data)
	memory_posix_madvise_function_data.advice = get_posix_madvice_advice(
		advice);
	file_memory_data.length = len;
	file_memory_data.address = addr;

	CALL_REAL_FUNCTION_RET(data, ret, posix_madvise, addr, len, advice)

	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, posix_madvise)
	return ret;
}
#endif

int WRAP(select)(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds,
				 struct timeval *timeout)
{
	int ret;
	struct basic data;
	struct select_function select_function_data;
	fd_set readfds_before;
	fd_set writefds_before;
	fd_set exceptfds_before;
	fd_set readfds_after;
	fd_set writefds_after;
	fd_set exceptfds_after;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, select_function,
						   select_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
	select_function_data.timeout.sec = timeout->tv_sec;
	select_function_data.timeout.micro_sec = timeout->tv_usec;
	if (NULL != readfds)
	{
		readfds_before = *readfds;
		select_function_data.files_waiting_for_read = &readfds_before;
	}
	else
	{
		select_function_data.files_waiting_for_read = NULL;
	}
	if (NULL != writefds)
	{
		writefds_before = *writefds;
		select_function_data.files_waiting_for_write = &writefds_before;
	}
	else
	{
		select_function_data.files_waiting_for_write = NULL;
	}
	if (NULL != exceptfds)
	{
		exceptfds_before = *exceptfds;
		select_function_data.files_waiting_for_except = &exceptfds_before;
	}
	else
	{
		select_function_data.files_waiting_for_except = NULL;
	}

	CALL_REAL_FUNCTION_RET(data, ret, select, nfds, readfds, writefds,
						   exceptfds, timeout)

	if (-1 == ret)
	{
		data.return_state = error;
		select_function_data.files_ready_for_read = NULL;
		select_function_data.files_ready_for_write = NULL;
		select_function_data.files_ready_for_except = NULL;
	}
	else
	{
		data.return_state = ok;
		if (NULL != readfds)
		{
			readfds_after = *readfds;
			select_function_data.files_ready_for_read = &readfds_after;
		}
		else
		{
			select_function_data.files_ready_for_read = NULL;
		}
		if (NULL != writefds)
		{
			writefds_after = *writefds;
			select_function_data.files_ready_for_write = &writefds_after;
		}
		else
		{
			select_function_data.files_ready_for_write = NULL;
		}
		if (NULL != exceptfds)
		{
			exceptfds_after = *exceptfds;
			select_function_data.files_ready_for_except = &exceptfds_after;
		}
		else
		{
			select_function_data.files_ready_for_except = NULL;
		}
	}

	WRAP_END(data, select)
	return ret;
}

#ifdef HAVE_SYNC
void WRAP(sync)(void)
{
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_FUNCTION(data, sync)

	data.return_state = ok;

	WRAP_END(data, sync)
	return;
}
#endif

#ifdef HAVE_SYNCFS
int WRAP(syncfs)(int fd)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, syncfs, fd)

	if (-1 == file)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, syncfs)
	return ret;
}
#endif

#ifdef HAVE_FSYNC
int WRAP(fsync)(int fd)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, fsync, fd)

	if (-1 == file)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, fsync)
	return ret;
}
#endif

#ifdef HAVE_FDATASYNC
int WRAP(fdatasync)(int fd)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, fdatasync, fd)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, fdatasync)
	return ret;
}
#endif

int WRAP(dup)(int oldfd)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct dup_function dup_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, dup_function, dup_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = oldfd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, dup, oldfd)

	dup_data.new_descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, dup)
	return ret;
}

int WRAP(dup2)(int oldfd, int newfd)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct dup_function dup_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, dup_function, dup_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = oldfd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, dup2, oldfd, newfd)

	dup_data.new_descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, dup2)
	return ret;
}

#ifdef HAVE_DUP3
int WRAP(dup3)(int oldfd, int newfd, int flags)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct dup3_function dup3_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, dup3_function, dup3_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = oldfd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	get_creation_flags(flags, &dup3_data.creation);

	CALL_REAL_FUNCTION_RET(data, ret, dup3, oldfd, newfd, flags)

	dup3_data.new_descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, dup3)
	return ret;
}
#endif

int WRAP(fcntl)(int fd, int cmd, ...)
{
	int ret;
	va_list ap;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct fcntl_function fcntl_function_data;
	struct dup_function dup_function_data;
	struct fcntl_fd_function fcntl_fd_function_data;
	struct fcntl_fl_function fcntl_fl_function_data;
	struct fcntl_flock fcntl_flock_data;
	struct fcntl_own fcntl_own_data;
	struct fcntl_sig fcntl_sig_data;
	struct fcntl_lease fcntl_lease_data;
	struct fcntl_dnotify fcntl_dnotify_data;
	struct fcntl_pipe_size fcntl_pipe_size_data;
	struct fcntl_seal fcntl_seal_data;
	struct fcntl_hint fcntl_hint_data;
	int arg_int;
	struct flock *arg_flock;
	struct f_owner_ex *arg_f_owner_ex;
	uint64_t *arg_uint64_t;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fcntl_function,
						   fcntl_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_descriptor_data.descriptor = fd;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	fcntl_function_data.cmd = get_fcntl_cmd(cmd);

	va_start(ap, cmd);
	switch (fcntl_function_data.cmd)
	{
	case dupfd:
	case dupfd_cloexec:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		dup_function_data.new_descriptor = ret;
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, dup_function,
							   dup_function_data)
		break;
	case setfd:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		get_file_descriptor_flags(arg_int, &fcntl_fd_function_data.fd_flags);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_fd_function,
							   fcntl_fd_function_data)
		break;
	case setfl:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		fcntl_fl_function_data.mode = get_access_mode(arg_int);
		get_creation_flags(arg_int, &fcntl_fl_function_data.creation);
		get_status_flags(arg_int, &fcntl_fl_function_data.status);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_fl_function,
							   fcntl_fl_function_data)
		break;
	case setown:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		if (arg_int >= 0)
		{
			fcntl_own_data.type = owner_process;
			fcntl_own_data.id = arg_int;
		}
		else
		{
			fcntl_own_data.type = owner_process_group;
			fcntl_own_data.id = arg_int * -1;
		}
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_own,
							   fcntl_own_data)
		break;
	case setsig:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		fcntl_sig_data.signal = ret;
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_sig,
							   fcntl_sig_data)
		break;
	case setlease:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		fcntl_lease_data.type = get_lease_type(arg_int);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_lease,
							   fcntl_lease_data)
		break;
	case notify:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		get_directory_notify_flags(arg_int, &fcntl_dnotify_data.flags);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_dnotify,
							   fcntl_dnotify_data)
		break;
	case setpipe_sz:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		fcntl_pipe_size_data.bytes = ret;
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_pipe_size,
							   fcntl_pipe_size_data)
		break;
#ifdef F_ADD_SEALS
	case add_seals:
		arg_int = va_arg(ap, int);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_int)
		get_seal_flags(arg_int, &fcntl_seal_data.flags);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_seal,
							   fcntl_seal_data)
		break;
#endif
	case getfd:
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd)
		get_file_descriptor_flags(ret, &fcntl_fd_function_data.fd_flags);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_fd_function,
							   fcntl_fd_function_data)
		break;
	case getfl:
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd)
		fcntl_fl_function_data.mode = get_access_mode(ret);
		get_creation_flags(ret, &fcntl_fl_function_data.creation);
		get_status_flags(ret, &fcntl_fl_function_data.status);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_fl_function,
							   fcntl_fl_function_data)
		break;
	case getown:
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd)
		if (ret >= 0)
		{
			fcntl_own_data.type = owner_process;
			fcntl_own_data.id = ret;
		}
		else
		{
			fcntl_own_data.type = owner_process_group;
			fcntl_own_data.id = ret * -1;
		}
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_own,
							   fcntl_own_data)
		break;
	case getsig:
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd)
		fcntl_sig_data.signal = ret;
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_sig,
							   fcntl_sig_data)
		break;
	case getlease:
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd)
		fcntl_lease_data.type = get_lease_type(ret);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_lease,
							   fcntl_lease_data)
		break;
	case getpipe_sz:
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd)
		fcntl_pipe_size_data.bytes = ret;
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_pipe_size,
							   fcntl_pipe_size_data)
		break;
#ifdef F_GET_SEALS
	case get_seals:
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd)
		get_seal_flags(ret, &fcntl_seal_data.flags);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_seal,
							   fcntl_seal_data)
		break;
#endif
	case setlk:
	case setlkw:
	case getlk:
	case ofd_setlk:
	case ofd_setlkw:
	case ofd_getlk:
		arg_flock = va_arg(ap, struct flock *);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_flock)
		fcntl_flock_data.type = get_lock_type(arg_flock->l_type);
		fcntl_flock_data.relative_to = get_seek_where(arg_flock->l_whence);
		fcntl_flock_data.start = arg_flock->l_start;
		fcntl_flock_data.len = arg_flock->l_len;
		if (fcntl_function_data.cmd == getlk || fcntl_function_data.cmd == ofd_getlk)
		{
			fcntl_flock_data.pid = arg_flock->l_pid;
		}
		else
		{
			fcntl_flock_data.pid = 0;
		}
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_flock,
							   fcntl_flock_data)
		break;
	case getown_ex:
	case setown_ex:
		arg_f_owner_ex = va_arg(ap, struct f_owner_ex *);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_f_owner_ex)
		fcntl_own_data.type = get_owner_type(arg_f_owner_ex->type);
		fcntl_own_data.id = arg_f_owner_ex->pid;
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_own,
							   fcntl_own_data)
		break;
	case get_rw_hint:
	case set_rw_hint:
	case get_file_rw_hint:
	case set_file_rw_hint:
#if defined(F_GET_RW_HINT) || defined(F_SET_RW_HINT) || defined(F_GET_FILE_RW_HINT) || defined(F_SET_FILE_RW_HINT)
		arg_uint64_t = va_arg(ap, uint64_t *);
		CALL_REAL_FUNCTION_RET(data, ret, fcntl, fd, cmd, arg_uint64_t)
		fcntl_hint_data.hint = get_hint_write_life(*arg_uint64_t);
		JSON_STRUCT_SET_VOID_P(fcntl_function_data, cmd_data, fcntl_hint,
							   fcntl_hint_data)
#endif
		break;
	}
	va_end(ap);

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, fcntl)
	return ret;
}

int WRAP(socket)(int domain, int type, int protocol)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct socket_function socket_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, socket_function,
						   socket_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	socket_function_data.connection_based = is_connection_based(type);

	CALL_REAL_FUNCTION_RET(data, ret, socket, domain, type, protocol)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, socket)
	return ret;
}

int WRAP(accept)(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct accept_function accept_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, accept_function,
						   accept_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, accept, sockfd, addr, addrlen)

	accept_function_data.new_descriptor = ret;
	file_descriptor_data.descriptor = sockfd;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, accept)
	return ret;
}

#ifdef HAVE_ACCEPT4
int WRAP(accept4)(int sockfd, struct sockaddr *addr, socklen_t *addrlen,
				  int flags)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct accept_function accept_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, accept_function,
						   accept_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, accept4, sockfd, addr, addrlen, flags)

	accept_function_data.new_descriptor = ret;
	file_descriptor_data.descriptor = sockfd;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, accept4)
	return ret;
}
#endif

int WRAP(socketpair)(int domain, int type, int protocol, int sv[2])
{
	int ret;
	struct basic data;
	struct socketpair_function socketpair_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, socketpair_function,
						   socketpair_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)
	socketpair_function_data.connection_based = is_connection_based(type);

	CALL_REAL_FUNCTION_RET(data, ret, socketpair, domain, type, protocol, sv)

	if (-1 == ret)
	{
		data.return_state = error;
		socketpair_function_data.descriptor1 = -1;
		socketpair_function_data.descriptor2 = -1;
	}
	else
	{
		data.return_state = ok;
		socketpair_function_data.descriptor1 = sv[0];
		socketpair_function_data.descriptor2 = sv[1];
	}

	WRAP_END(data, socketpair)
	return ret;
}

int WRAP(connect)(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret;
	char hex_addr[MAX_SOCKADDR_LENGTH * 2 + 1]; // see struct sockaddr_function.address
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct sockaddr_function sockaddr_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, sockaddr_function,
						   sockaddr_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	file_descriptor_data.descriptor = sockfd;
	get_sockaddr(addr, &sockaddr_data, addrlen, hex_addr);

	CALL_REAL_FUNCTION_RET(data, ret, connect, sockfd, addr, addrlen)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, connect)
	return ret;
}

int WRAP(bind)(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
	int ret;
	char hex_addr[MAX_SOCKADDR_LENGTH * 2 + 1]; // see struct sockaddr_function.address
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct sockaddr_function sockaddr_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, sockaddr_function,
						   sockaddr_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	file_descriptor_data.descriptor = sockfd;
	get_sockaddr(addr, &sockaddr_data, addrlen, hex_addr);

	CALL_REAL_FUNCTION_RET(data, ret, bind, sockfd, addr, addrlen)

	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, bind)
	return ret;
}

int WRAP(pipe)(int pipefd[2])
{
	int ret;
	struct basic data;
	struct file_pair file_pair_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, file_pair, file_pair_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_FUNCTION_RET(data, ret, pipe, pipefd)

	file_pair_data.descriptor1 = pipefd[0];
	file_pair_data.descriptor2 = pipefd[1];
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, pipe)
	return ret;
}

#ifdef HAVE_PIPE2
int WRAP(pipe2)(int pipefd[2], int flags)
{
	int ret;
	struct basic data;
	struct file_pair file_pair_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, file_pair, file_pair_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_FUNCTION_RET(data, ret, pipe2, pipefd, flags)

	file_pair_data.descriptor1 = pipefd[0];
	file_pair_data.descriptor2 = pipefd[1];
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, pipe2)
	return ret;
}
#endif

#ifdef HAVE_MEMFD_CREATE
int WRAP(memfd_create)(const char *name, unsigned int flags)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, memfd_create, name, flags)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, memfd_create)
	return ret;
}
#endif

#ifdef HAVE_EPOLL_CREATE
int WRAP(epoll_create)(int size)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, epoll_create, size)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, epoll_create)
	return ret;
}
#endif

#ifdef HAVE_EPOLL_CREATE1
int WRAP(epoll_create1)(int flags)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, epoll_create1, flags)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, epoll_create1)
	return ret;
}
#endif

#ifdef HAVE_MKSTEMP
int WRAP(mkstemp)(char *template)
{
	int tmpflags = O_RDWR | O_CREAT | O_EXCL;
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	open_data.mode = get_access_mode(tmpflags);
	get_creation_flags(tmpflags, &open_data.creation);
	get_status_flags(tmpflags, &open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR, &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, ret, mkstemp, template)

	open_data.file_name = template;

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, mkstemp)
	return ret;
}
#endif

#ifdef HAVE_MKOSTEMP
int WRAP(mkostemp)(char *template, int flags)
{
	int tmpflags = flags | O_RDWR | O_CREAT | O_EXCL;
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	open_data.mode = get_access_mode(tmpflags);
	get_creation_flags(tmpflags, &open_data.creation);
	get_status_flags(tmpflags, &open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR, &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, ret, mkostemp, template, flags)

	open_data.file_name = template;

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, mkostemp)
	return ret;
}
#endif

#ifdef HAVE_MKSTEMPS
int WRAP(mkstemps)(char *template, int suffixlen)
{
	int tmpflags = O_RDWR | O_CREAT | O_EXCL;
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	open_data.mode = get_access_mode(tmpflags);
	get_creation_flags(tmpflags, &open_data.creation);
	get_status_flags(tmpflags, &open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR, &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, ret, mkstemps, template, suffixlen)

	open_data.file_name = template;

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, mkstemps)
	return ret;
}
#endif

#ifdef HAVE_MKOSTEMPS
int WRAP(mkostemps)(char *template, int suffixlen, int flags)
{
	int tmpflags = flags | O_RDWR | O_CREAT | O_EXCL;
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)
	open_data.mode = get_access_mode(tmpflags);
	get_creation_flags(tmpflags, &open_data.creation);
	get_status_flags(tmpflags, &open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR, &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, ret, mkostemps, template, suffixlen, flags)

	open_data.file_name = template;

	file_descriptor_data.descriptor = ret;
	get_file_id(ret, &(open_data.id));
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, mkostemps)
	return ret;
}
#endif

#ifdef HAVE_EVENTFD
int WRAP(eventfd)(unsigned int initval, int flags)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, eventfd, initval, flags)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, eventfd)
	return ret;
}
#endif

#ifdef HAVE_INOTIFY_INIT
int WRAP(inotify_init)(void)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, inotify_init)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, inotify_init)
	return ret;
}
#endif

#ifdef HAVE_INOTIFY_INIT1
int WRAP(inotify_init1)(int flags)
{
	int ret;
	struct basic data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
						   file_descriptor_data)

	CALL_REAL_FUNCTION_RET(data, ret, inotify_init1, flags)

	file_descriptor_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, inotify_init1)
	return ret;
}
#endif

struct dirent *WRAP(readdir)(DIR *dirp)
{
	struct dirent *ret;
	struct basic data;
	struct file_dir file_dir_data;
	struct readdir_function readdir_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, readdir_function,
						   readdir_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_dir, file_dir_data)
	file_dir_data.directory_stream = dirp;

	CALL_REAL_FUNCTION_RET(data, ret, readdir, dirp)

	if (NULL == ret)
	{
		data.return_state = error;
		readdir_function_data.file_name = "";
	}
	else
	{
		data.return_state = ok;
		readdir_function_data.file_name = ret->d_name;
	}

	WRAP_END(data, readdir)
	return ret;
}

#ifdef HAVE_DIRFD
int WRAP(dirfd)(DIR *dirp)
{
	int ret;
	struct basic data;
	struct file_dir file_dir_data;
	struct dirfd_function dirfd_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, dirfd_function,
						   dirfd_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P(data, file_type, file_dir, file_dir_data)
	file_dir_data.directory_stream = dirp;

	CALL_REAL_FUNCTION_RET(data, ret, dirfd, dirp)

	dirfd_function_data.descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, dirfd)
	return ret;
}
#endif

ssize_t WRAP(sendmsg)(int sockfd, const struct msghdr *msg, int flags)
{
	ssize_t ret;
	char hex_addr[MAX_SOCKADDR_LENGTH * 2 + 1]; // see struct sockaddr_function.address
	int fd_count;
	int fd;
	struct cmsghdr *cmsg = NULL;
	struct basic data;
	struct msg_function msg_function_data;
	struct sockaddr_function sockaddr_function_data;
	struct file_descriptor file_descriptor_data;
	struct msghdr *n_msg = (struct msghdr *)((void *)msg);
	WRAP_START(data)

	CALL_REAL_FUNCTION_RET(data, ret, sendmsg, sockfd, msg, flags)

	get_basic(&data);

	if (-1 != ret)
	{
		JSON_STRUCT_SET_VOID_P(data, function_data, msg_function,
							   msg_function_data)
		POSIX_IO_SET_FUNCTION_NAME(data.function_name);
		JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
							   file_descriptor_data)
		file_descriptor_data.descriptor = sockfd;
		data.return_state = ok;

		for (cmsg = CMSG_FIRSTHDR(n_msg); cmsg != NULL;
			 cmsg = CMSG_NXTHDR(n_msg, cmsg))
		{
			if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SCM_RIGHTS))
			{
				if (0 < n_msg->msg_namelen && MAX_SOCKADDR_LENGTH <= n_msg->msg_namelen)
				{
					get_sockaddr(n_msg->msg_name, &sockaddr_function_data,
								 n_msg->msg_namelen, hex_addr);
					msg_function_data.sockaddr = &sockaddr_function_data;
				}
				else
				{
					msg_function_data.sockaddr = NULL;
				}

				fd_count = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
				JSON_STRUCT_SET_INT_ARRAY(msg_function_data, descriptors,
										  (int *)CMSG_DATA(cmsg), fd_count)

				WRAP_END(data, sendmsg)
				return ret;
			}
		}
	}

	WRAP_END_WITHOUT_WRITE(data)
	return ret;
}

ssize_t WRAP(recvmsg)(int sockfd, struct msghdr *msg, int flags)
{
	ssize_t ret;
	char hex_addr[MAX_SOCKADDR_LENGTH * 2 + 1]; // see struct sockaddr_function.address
	int fd_count;
	int fd;
	struct cmsghdr *cmsg = NULL;
	struct basic data;
	struct msg_function msg_function_data;
	struct sockaddr_function sockaddr_function_data;
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	CALL_REAL_FUNCTION_RET(data, ret, recvmsg, sockfd, msg, flags)

	get_basic(&data);

	if (-1 != ret)
	{
		JSON_STRUCT_SET_VOID_P(data, function_data, msg_function,
							   msg_function_data)
		POSIX_IO_SET_FUNCTION_NAME(data.function_name);
		JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
							   file_descriptor_data)
		file_descriptor_data.descriptor = sockfd;
		data.return_state = ok;

		for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
			 cmsg = CMSG_NXTHDR(msg, cmsg))
		{
			if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SCM_RIGHTS))
			{
				if (0 < msg->msg_namelen && MAX_SOCKADDR_LENGTH <= msg->msg_namelen)
				{
					get_sockaddr(msg->msg_name, &sockaddr_function_data,
								 msg->msg_namelen, hex_addr);
					msg_function_data.sockaddr = &sockaddr_function_data;
				}
				else
				{
					msg_function_data.sockaddr = NULL;
				}

				fd_count = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
				JSON_STRUCT_SET_INT_ARRAY(msg_function_data, descriptors,
										  (int *)CMSG_DATA(cmsg), fd_count)

				WRAP_END(data, recvmsg)
				return ret;
			}
		}
	}

	WRAP_END_WITHOUT_WRITE(data)
	return ret;
}

#ifdef HAVE_SENDMMSG
int WRAP(sendmmsg)(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
				   int flags)
{
	int ret;
	int fd_count;
	int sockaddr_count = 0;
	char hex_addr[MAX_MMSG_MESSAGES][MAX_SOCKADDR_LENGTH * 2 + 1]; // see struct sockaddr_function.address
	struct cmsghdr *cmsg = NULL;
	struct msghdr *msg;
	struct basic data;
	struct msg_function msg_function_data[MAX_MMSG_MESSAGES];
	struct msg_function *messages[MAX_MMSG_MESSAGES];
	struct mmsg_function mmsg_function_data;
	struct sockaddr_function sockaddr_function_data[MAX_MMSG_MESSAGES];
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	CALL_REAL_FUNCTION_RET(data, ret, sendmmsg, sockfd, msgvec, vlen, flags)

	get_basic(&data);

	for (int i = 0; i < vlen && i < ret && i < MAX_MMSG_MESSAGES; i++)
	{
		msg = &((msgvec + i)->msg_hdr);

		for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
			 cmsg = CMSG_NXTHDR(msg, cmsg))
		{

			if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SCM_RIGHTS))
			{
				if (0 < msg->msg_namelen && MAX_SOCKADDR_LENGTH <= msg->msg_namelen)
				{
					get_sockaddr(msg->msg_name,
								 &sockaddr_function_data[sockaddr_count],
								 msg->msg_namelen, hex_addr[sockaddr_count]);
					msg_function_data[sockaddr_count].sockaddr =
						&sockaddr_function_data[sockaddr_count];
				}
				else
				{
					msg_function_data[sockaddr_count].sockaddr = NULL;
				}
				messages[sockaddr_count] = &(msg_function_data[sockaddr_count]);

				fd_count = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
				JSON_STRUCT_SET_INT_ARRAY(msg_function_data[sockaddr_count],
										  descriptors, (int *)CMSG_DATA(cmsg), fd_count)

				sockaddr_count++;
				break;
			}
		}
	}

	if (sockaddr_count > 0)
	{
		JSON_STRUCT_SET_VOID_P(data, function_data, mmsg_function,
							   mmsg_function_data)
		JSON_STRUCT_SET_STRUCT_ARRAY(mmsg_function_data, messages, messages,
									 sockaddr_count)
		POSIX_IO_SET_FUNCTION_NAME(data.function_name);
		JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
							   file_descriptor_data)
		file_descriptor_data.descriptor = sockfd;
		data.return_state = ok;

		WRAP_END(data, sendmmsg)
		return ret;
	}

	WRAP_END_WITHOUT_WRITE(data)
	return ret;
}
#endif

#ifdef HAVE_RECVMMSG
#ifdef HAVE_RECVMMSG_CONST_TIMESPEC
int WRAP(recvmmsg)(int sockfd, struct mmsghdr *msgvec, unsigned int vlen, int flags, const struct timespec *timeout)
{
#else
int WRAP(recvmmsg)(int sockfd, struct mmsghdr *msgvec, unsigned int vlen,
				   int flags, struct timespec *timeout)
{
#endif
	int ret;
	int fd_count;
	int sockaddr_count = 0;
	char hex_addr[MAX_MMSG_MESSAGES][MAX_SOCKADDR_LENGTH * 2 + 1]; // see struct sockaddr_function.address
	struct cmsghdr *cmsg = NULL;
	struct msghdr *msg;
	struct basic data;
	struct msg_function msg_function_data[MAX_MMSG_MESSAGES];
	struct msg_function *messages[MAX_MMSG_MESSAGES];
	struct mmsg_function mmsg_function_data;
	struct sockaddr_function sockaddr_function_data[MAX_MMSG_MESSAGES];
	struct file_descriptor file_descriptor_data;
	WRAP_START(data)

	CALL_REAL_FUNCTION_RET(data, ret, recvmmsg, sockfd, msgvec, vlen, flags,
						   timeout)

	get_basic(&data);

	for (int i = 0; i < vlen && i < ret && i < MAX_MMSG_MESSAGES; i++)
	{
		msg = &((msgvec + i)->msg_hdr);

		for (cmsg = CMSG_FIRSTHDR(msg); cmsg != NULL;
			 cmsg = CMSG_NXTHDR(msg, cmsg))
		{

			if ((cmsg->cmsg_level == SOL_SOCKET) && (cmsg->cmsg_type == SCM_RIGHTS))
			{
				if (0 < msg->msg_namelen && MAX_SOCKADDR_LENGTH <= msg->msg_namelen)
				{
					get_sockaddr(msg->msg_name,
								 &sockaddr_function_data[sockaddr_count],
								 msg->msg_namelen, hex_addr[sockaddr_count]);
					msg_function_data[sockaddr_count].sockaddr =
						&sockaddr_function_data[sockaddr_count];
				}
				else
				{
					msg_function_data[sockaddr_count].sockaddr = NULL;
				}
				messages[sockaddr_count] = &(msg_function_data[sockaddr_count]);

				fd_count = (cmsg->cmsg_len - CMSG_LEN(0)) / sizeof(int);
				JSON_STRUCT_SET_INT_ARRAY(msg_function_data[sockaddr_count],
										  descriptors, (int *)CMSG_DATA(cmsg), fd_count)

				sockaddr_count++;
				break;
			}
		}
	}

	if (sockaddr_count > 0)
	{
		JSON_STRUCT_SET_VOID_P(data, function_data, mmsg_function,
							   mmsg_function_data)
		JSON_STRUCT_SET_STRUCT_ARRAY(mmsg_function_data, messages, messages,
									 sockaddr_count)
		POSIX_IO_SET_FUNCTION_NAME(data.function_name);
		JSON_STRUCT_SET_VOID_P(data, file_type, file_descriptor,
							   file_descriptor_data)
		file_descriptor_data.descriptor = sockfd;
		data.return_state = ok;

		WRAP_END(data, recvmmsg)
		return ret;
	}

	WRAP_END_WITHOUT_WRITE(data)
	return ret;
}
#endif

FILE *WRAP(fopen)(const char *filename, const char *opentype)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
								&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
				   &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, fopen, filename, opentype)

	if (NULL == file)
	{
		data.return_state = error;
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id(CALL_REAL_POSIX_SYNC(fileno)(file), &(open_data.id));
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, fopen)
	return file;
}

#ifdef HAVE_FOPEN64
FILE *WRAP(fopen64)(const char *filename, const char *opentype)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
								&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
				   &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, fopen64, filename, opentype)

	if (NULL == file)
	{
		data.return_state = error;
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id(CALL_REAL_POSIX_SYNC(fileno)(file), &(open_data.id));
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, fopen64)
	return file;
}
#endif

FILE *WRAP(freopen)(const char *filename, const char *opentype, FILE *stream)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
								&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
				   &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, freopen, filename, opentype, stream)

	if (NULL == file)
	{
		data.return_state = error;
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id(CALL_REAL_POSIX_SYNC(fileno)(file), &(open_data.id));
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, freopen)
	return file;
}

#ifdef HAVE_FREOPEN64
FILE *WRAP(freopen64)(const char *filename, const char *opentype, FILE *stream)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = filename;
	open_data.mode = check_mode(opentype, &open_data.creation,
								&open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
				   &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, freopen64, filename, opentype, stream)

	if (NULL == file)
	{
		data.return_state = error;
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id(CALL_REAL_POSIX_SYNC(fileno)(file), &(open_data.id));
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, freopen64)
	return file;
}
#endif

#ifdef HAVE_FDOPEN
FILE *WRAP(fdopen)(int fd, const char *opentype)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	struct fdopen_function fdopen_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fdopen_function, fdopen_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	fdopen_data.descriptor = fd;
	fdopen_data.mode = check_mode(opentype, &fdopen_data.creation,
								  &fdopen_data.status);

	CALL_REAL_FUNCTION_RET(data, file, fdopen, fd, opentype)

	if (NULL == file)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, fdopen)
	return file;
}
#endif

int WRAP(fclose)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fclose, stream)

	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, fclose)
	return ret;
}

#ifdef HAVE_FCLOSEALL
int WRAP(fcloseall)(void)
{
	int ret;
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_FUNCTION_RET(data, ret, fcloseall)

	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, fcloseall)
	return ret;
}
#endif

#ifdef HAVE_FLOCKFILE
void WRAP(flockfile)(FILE *stream)
{
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, flockfile, stream)

	data.return_state = ok;

	WRAP_END(data, flockfile)
	return;
}
#endif

#ifdef HAVE_FTRYLOCKFILE
int WRAP(ftrylockfile)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ftrylockfile, stream)

	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, ftrylockfile)
	return ret;
}
#endif

#ifdef HAVE_FUNLOCKFILE
void WRAP(funlockfile)(FILE *stream)
{
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, funlockfile, stream)

	data.return_state = ok;

	WRAP_END(data, funlockfile)
	return;
}
#endif

#ifdef HAVE_FWIDE
int WRAP(fwide)(FILE *stream, int mode)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct orientation_mode_function orientation_mode_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, orientation_mode_function,
						   orientation_mode_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	orientation_mode_data.set_mode = get_orientation_mode(mode, 1);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fwide, stream, mode)

	data.return_state = ok;
	orientation_mode_data.return_mode = get_orientation_mode(ret, 0);

	WRAP_END(data, fwide)
	return ret;
}
#endif

int WRAP(fputc)(int c, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputc, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = 1;
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputc)
	return ret;
}

wint_t WRAP(fputwc)(wchar_t wc, FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputwc, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = sizeof(wchar_t);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputwc)
	return ret;
}

#ifdef HAVE_FPUTC_UNLOCKED
int WRAP(fputc_unlocked)(int c, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputc_unlocked, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = 1;
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputc_unlocked)
	return ret;
}
#endif

#ifdef HAVE_FPUTWC_UNLOCKED
wint_t WRAP(fputwc_unlocked)(wchar_t wc, FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputwc_unlocked, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = sizeof(wchar_t);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputwc_unlocked)
	return ret;
}
#endif

int WRAP(putc_MACRO)(int c, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "putc");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, putc_MACRO, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = 1;
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, putc_MACRO)
	return ret;
}

wint_t WRAP(putwc_MACRO)(wchar_t wc, FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "putwc");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, putwc_MACRO, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = sizeof(wchar_t);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, putwc_MACRO)
	return ret;
}

#ifdef HAVE_PUTC_UNLOCKED
int WRAP(putc_unlocked_MACRO)(int c, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "putc_unlocked");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, putc_unlocked_MACRO, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = 1;
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, putc_unlocked_MACRO)
	return ret;
}
#endif

#ifdef HAVE_PUTWC_UNLOCKED
wint_t WRAP(putwc_unlocked_MACRO)(wchar_t wc, FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "putwc_unlocked");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, putwc_unlocked_MACRO, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = sizeof(wchar_t);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, putwc_unlocked_MACRO)
	return ret;
}
#endif

int WRAP(fputs)(const char *s, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputs, s, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = strlen(s);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputs)
	return ret;
}

int WRAP(fputws)(const wchar_t *ws, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputws, ws, stream)

	// ToDo: wchar.h says WEOF is error, man pages says -1 is error, what if WEOF isn't -1 ??? ???
	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = wcslen(ws) * sizeof(wchar_t);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputws)
	return ret;
}

#ifdef HAVE_FPUTS_UNLOCKED
int WRAP(fputs_unlocked)(const char *s, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputs_unlocked, s, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = strlen(s);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputs_unlocked)
	return ret;
}
#endif

#ifdef HAVE_FPUTWS_UNLOCKED
int WRAP(fputws_unlocked)(const wchar_t *ws, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fputws_unlocked, ws, stream)

	// ToDo: wchar.h says WEOF is error, man pages says -1 is error, what if WEOF isn't -1 ???
	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = wcslen(ws) * sizeof(wchar_t);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, fputws_unlocked)
	return ret;
}
#endif

#ifdef HAVE_PUTW
int WRAP(putw)(int w, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, putw, w, stream)

	// ToDo: behavior as described in man pages because header file says nothing about errors
	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		write_data.written_bytes = sizeof(int);
	}
	else
	{
		write_data.written_bytes = 0;
	}

	WRAP_END(data, putw)
	return ret;
}
#endif

int WRAP(fgetc)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetc, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = 1;
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, fgetc)
	return ret;
}

wint_t WRAP(fgetwc)(FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetwc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = sizeof(wchar_t);
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, fgetwc)
	return ret;
}

#ifdef HAVE_FGETC_UNLOCKED
int WRAP(fgetc_unlocked)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetc_unlocked, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = 1;
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, fgetc_unlocked)
	return ret;
}
#endif

#ifdef HAVE_FGETWC_UNLOCKED
wint_t WRAP(fgetwc_unlocked)(FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetwc_unlocked, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = sizeof(wchar_t);
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, fgetwc_unlocked)
	return ret;
}
#endif

int WRAP(getc_MACRO)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "getc");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, getc_MACRO, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = 1;
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, getc_MACRO)
	return ret;
}

wint_t WRAP(getwc_MACRO)(FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "getwc");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, getwc_MACRO, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = sizeof(wchar_t);
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, getwc_MACRO)
	return ret;
}

#ifdef HAVE_GETC_UNLOCKED
int WRAP(getc_unlocked_MACRO)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "getc_unlocked");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, getc_unlocked_MACRO, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = 1;
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, getc_unlocked_MACRO)
	return ret;
}
#endif

#ifdef HAVE_GETWC_UNLOCKED
wint_t WRAP(getwc_unlocked_MACRO)(FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME_STRING(data.function_name, "getwc_unlocked");
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, getwc_unlocked_MACRO, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = sizeof(wchar_t);
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, getwc_unlocked_MACRO)
	return ret;
}
#endif

#ifdef HAVE_GETW
int WRAP(getw)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, getw, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		read_data.read_bytes = sizeof(int);
	}
	else
	{
		read_data.read_bytes = 0;
	}

	WRAP_END(data, getw)
	return ret;
}
#endif

#ifdef HAVE_GETLINE
ssize_t WRAP(getline)(char **lineptr, size_t *n, FILE *stream)
{
	ssize_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, getline, lineptr, n, stream)

	if (ret == -1)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data, getline)
	return ret;
}
#endif

#ifdef HAVE_GETDELIM
ssize_t WRAP(getdelim)(char **lineptr, size_t *n, int delimiter, FILE *stream)
{
	ssize_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, getdelim, lineptr, n, delimiter, stream)

	if (ret == -1)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = ret;
	}

	WRAP_END(data, getdelim)
	return ret;
}
#endif

char *WRAP(fgets)(char *s, int count, FILE *stream)
{
	char *ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgets, s, count, stream)

	if (NULL == ret)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = strlen(s);
	}

	WRAP_END(data, fgets)
	return ret;
}

wchar_t *WRAP(fgetws)(wchar_t *ws, int count, FILE *stream)
{
	wchar_t *ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetws, ws, count, stream)

	if (NULL == ret)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = wcslen(ws) * sizeof(wchar_t);
	}

	WRAP_END(data, fgetws)
	return ret;
}

#ifdef HAVE_FGETS_UNLOCKED
char *WRAP(fgets_unlocked)(char *s, int count, FILE *stream)
{
	char *ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgets_unlocked, s, count, stream)

	if (NULL == ret)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = strlen(s);
	}

	WRAP_END(data, fgets_unlocked)
	return ret;
}
#endif

#ifdef HAVE_FGETWS_UNLOCKED
wchar_t *WRAP(fgetws_unlocked)(wchar_t *ws, int count, FILE *stream)
{
	wchar_t *ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetws_unlocked, ws, count, stream)

	if (NULL == ret)
	{
		data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		read_data.read_bytes = wcslen(ws) * sizeof(wchar_t);
	}

	WRAP_END(data, fgetws_unlocked)
	return ret;
}
#endif

int WRAP(ungetc)(int c, FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct unget_function unget_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, unget_function, unget_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ungetc, c, stream)

	data.return_state = get_return_state_c(ret);
	if (data.return_state == ok)
	{
		unget_data.buffer_bytes = -1;
	}
	else
	{
		unget_data.buffer_bytes = 0;
	}

	WRAP_END(data, ungetc)
	return ret;
}

wint_t WRAP(ungetwc)(wint_t wc, FILE *stream)
{
	wint_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct unget_function unget_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, unget_function, unget_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ungetwc, wc, stream)

	data.return_state = get_return_state_wc(ret);
	if (data.return_state == ok)
	{
		unget_data.buffer_bytes = -1 * (int)sizeof(wchar_t);
	}
	else
	{
		unget_data.buffer_bytes = 0;
	}

	WRAP_END(data, ungetwc)
	return ret;
}

size_t WRAP(fread)(void *data, size_t size, size_t count, FILE *stream)
{
	size_t ret;
	struct basic _data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(_data, ret, fread, data, size, count, stream)

	if (0 == ret)
	{
		_data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		_data.return_state = ok;
		read_data.read_bytes = ret * size;
	}

	WRAP_END(_data, fread)
	return ret;
}

#ifdef HAVE_FREAD_UNLOCKED
size_t WRAP(fread_unlocked)(void *data, size_t size, size_t count, FILE *stream)
{
	size_t ret;
	struct basic _data;
	struct file_stream file_stream_data;
	struct read_function read_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, read_function, read_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(_data, ret, fread_unlocked, data, size, count,
						   stream)

	if (0 == ret)
	{
		_data.return_state = eof;
		read_data.read_bytes = 0;
	}
	else
	{
		_data.return_state = ok;
		read_data.read_bytes = ret * size;
	}

	WRAP_END(_data, fread_unlocked)
	return ret;
}
#endif

size_t WRAP(fwrite)(const void *data, size_t size, size_t count, FILE *stream)
{
	size_t ret;
	struct basic _data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(_data, ret, fwrite, data, size, count, stream)

	if (ret != count)
	{
		_data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		_data.return_state = ok;
		write_data.written_bytes = ret * size;
	}

	WRAP_END(_data, fwrite)
	return ret;
}

#ifdef HAVE_FWRITE_UNLOCKED
size_t WRAP(fwrite_unlocked)(const void *data, size_t size, size_t count,
							 FILE *stream)
{
	size_t ret;
	struct basic _data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(_data)

	get_basic(&_data);
	JSON_STRUCT_SET_VOID_P(_data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(_data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(_data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(_data, ret, fwrite_unlocked, data, size, count,
						   stream)

	if (ret != count)
	{
		_data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		_data.return_state = ok;
		write_data.written_bytes = ret * size;
	}

	WRAP_END(_data, fwrite_unlocked)
	return ret;
}
#endif

int WRAP(fprintf)(FILE *stream, const char *template, ...)
{
	int ret;
	va_list ap;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfprintf, stream, template, ap)
	va_end(ap);

	if (ret < 0)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data, fprintf)
	return ret;
}

#ifdef HAVE_FWPRINTF
int WRAP(fwprintf)(FILE *stream, const wchar_t *template, ...)
{
	int ret;
	va_list ap;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfwprintf, stream, template, ap)
	va_end(ap);

	if (ret < 0)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		write_data.written_bytes = ret * sizeof(wchar_t);
	}

	WRAP_END(data, fwprintf)
	return ret;
}
#endif

int WRAP(vfprintf)(FILE *stream, const char *template, va_list ap)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, vfprintf, stream, template, ap)

	if (ret < 0)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		write_data.written_bytes = ret;
	}

	WRAP_END(data, vfprintf)
	return ret;
}

#ifdef HAVE_VFWPRINTF
int WRAP(vfwprintf)(FILE *stream, const wchar_t *template, va_list ap)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct write_function write_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, write_function, write_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, vfwprintf, stream, template, ap)

	if (ret < 0)
	{
		data.return_state = error;
		write_data.written_bytes = 0;
	}
	else
	{
		data.return_state = ok;
		write_data.written_bytes = ret * sizeof(wchar_t);
	}

	WRAP_END(data, vfwprintf)
	return ret;
}
#endif

int WRAP(fscanf)(FILE *stream, const char *template, ...)
{
	int ret;
	va_list ap;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfscanf, stream, template, ap)
	va_end(ap);

	data.return_state = get_return_state_c(ret);

	WRAP_END(data, fscanf)
	return ret;
}

#ifdef HAVE_FWSCANF
int WRAP(fwscanf)(FILE *stream, const wchar_t *template, ...)
{
	int ret;
	va_list ap;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	va_start(ap, template);
	CALL_REAL_FUNCTION_RET(data, ret, vfwscanf, stream, template, ap)
	va_end(ap);

	data.return_state = get_return_state_wc(ret);

	WRAP_END(data, fwscanf)
	return ret;
}
#endif

#ifdef HAVE_VFSCANF
int WRAP(vfscanf)(FILE *stream, const char *template, va_list ap)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, vfscanf, stream, template, ap)

	data.return_state = get_return_state_c(ret);

	WRAP_END(data, vfscanf)
	return ret;
}
#endif

#ifdef HAVE_VFWSCANF
int WRAP(vfwscanf)(FILE *stream, const wchar_t *template, va_list ap)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, vfwscanf, stream, template, ap)

	data.return_state = get_return_state_wc(ret);

	WRAP_END(data, vfwscanf)
	return ret;
}
#endif

int WRAP(feof)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, feof, stream)

	data.return_state = ok;
	if (ret == 0)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, feof)
	return ret;
}

#ifdef HAVE_FEOF_UNLOCKED
int WRAP(feof_unlocked)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, feof_unlocked, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, feof_unlocked)
	return ret;
}
#endif

int WRAP(ferror)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ferror, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, ferror)
	return ret;
}

#ifdef HAVE_FERROR_UNLOCKED
int WRAP(ferror_unlocked)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ferror_unlocked, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, ferror_unlocked)
	return ret;
}
#endif

void WRAP(clearerr)(FILE *stream)
{
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, clearerr, stream)

	data.return_state = ok;

	WRAP_END(data, clearerr)
	return;
}

#ifdef HAVE_CLEARERR_UNLOCKED
void WRAP(clearerr_unlocked)(FILE *stream)
{
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, clearerr_unlocked, stream)

	data.return_state = ok;

	WRAP_END(data, clearerr_unlocked)
	return;
}
#endif

long int WRAP(ftell)(FILE *stream)
{
	long int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct position_function position_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, position_function,
						   position_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ftell, stream)

	if (-1 == ret)
	{
		data.return_state = error;
		position_data.position = 0;
	}
	else
	{
		data.return_state = ok;
		position_data.position = ret;
	}

	WRAP_END(data, ftell)
	return ret;
}

#ifdef HAVE_FTELLO
off_t WRAP(ftello)(FILE *stream)
{
	off_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct position_function position_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, position_function,
						   position_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ftello, stream)

	if (-1 == ret)
	{
		data.return_state = error;
		position_data.position = 0;
	}
	else
	{
		data.return_state = ok;
		position_data.position = ret;
	}

	WRAP_END(data, ftello)
	return ret;
}
#endif

#ifdef HAVE_FTELLO64
off64_t WRAP(ftello64)(FILE *stream)
{
	off64_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct position_function position_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, position_function,
						   position_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, ftello64, stream)

	if (-1 == ret)
	{
		data.return_state = error;
		position_data.position = 0;
	}
	else
	{
		data.return_state = ok;
		position_data.position = ret;
	}

	WRAP_END(data, ftello64)
	return ret;
}
#endif

int WRAP(fseek)(FILE *stream, long int offset, int whence)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
						   positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fseek, stream, offset, whence)

	positioning_data.offset = offset;
	positioning_data.relative_to = get_seek_where(whence);
	if (0 == ret)
	{
		data.return_state = ok;
		if (end_of_file == positioning_data.relative_to)
		{
			// ToDo: file lock over fseek and ftell (to ensure that ftell returns the with fseek set position)
			positioning_data.offset = CALL_REAL(ftell)(stream);
		}
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, fseek)
	return ret;
}

#ifdef HAVE_FSEEKO
int WRAP(fseeko)(FILE *stream, off_t offset, int whence)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
						   positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fseeko, stream, offset, whence)

	positioning_data.offset = offset;
	positioning_data.relative_to = get_seek_where(whence);
	if (0 == ret)
	{
		data.return_state = ok;
		if (end_of_file == positioning_data.relative_to)
		{
			// ToDo: file lock over fseeko and ftello (to ensure that ftello returns the with fseeko set position)
			positioning_data.offset = CALL_REAL(ftello)(stream);
		}
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, fseeko)
	return ret;
}
#endif

#ifdef HAVE_FSEEKO64
int WRAP(fseeko64)(FILE *stream, off64_t offset, int whence)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
						   positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fseeko64, stream, offset, whence)

	positioning_data.offset = offset;
	positioning_data.relative_to = get_seek_where(whence);
	if (0 == ret)
	{
		data.return_state = ok;
		if (end_of_file == positioning_data.relative_to)
		{
			// ToDo: file lock over fseeko64 and ftello64 (to ensure that ftello64 returns the with fseeko64 set position)
			positioning_data.offset = CALL_REAL(ftello64)(stream);
		}
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, fseeko64)
	return ret;
}
#endif

void WRAP(rewind)(FILE *stream)
{
	struct basic data;
	struct file_stream file_stream_data;
	struct positioning_function positioning_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, positioning_function,
						   positioning_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, rewind, stream)

	data.return_state = ok;
	positioning_data.offset = 0;
	positioning_data.relative_to = get_seek_where(SEEK_SET);

	WRAP_END(data, rewind)
	return;
}

int WRAP(fgetpos)(FILE *stream, fpos_t *position)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetpos, stream, position)

	if (0 != ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, fgetpos)
	return ret;
}

#ifdef HAVE_FGETPOS64
int WRAP(fgetpos64)(FILE *stream, fpos64_t *position)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fgetpos64, stream, position)

	if (0 != ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, fgetpos64)
	return ret;
}
#endif

int WRAP(fsetpos)(FILE *stream, const fpos_t *position)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fsetpos, stream, position)

	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, fsetpos)
	return ret;
}

#ifdef HAVE_FSETPOS64
int WRAP(fsetpos64)(FILE *stream, const fpos64_t *position)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fsetpos64, stream, position)

	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, fsetpos64)
	return ret;
}
#endif

int WRAP(fflush)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fflush, stream)

	data.return_state = get_return_state_c(ret);

	WRAP_END(data, fflush)
	return ret;
}

#ifdef HAVE_FFLUSH_UNLOCKED
int WRAP(fflush_unlocked)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fflush_unlocked, stream)

	data.return_state = get_return_state_c(ret);

	WRAP_END(data, fflush_unlocked)
	return ret;
}
#endif

int WRAP(setvbuf)(FILE *stream, char *buf, int mode, size_t size)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, setvbuf, stream, buf, mode, size)

	buffer_data.buffer_mode = get_buffer_mode(mode);
	buffer_data.buffer_size = size;
	if (0 == ret)
	{
		data.return_state = ok;
	}
	else
	{
		data.return_state = error;
	}

	WRAP_END(data, setvbuf)
	return ret;
}

void WRAP(setbuf)(FILE *stream, char *buf)
{
	struct basic data;
	struct file_stream file_stream_data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, setbuf, stream, buf)

	if (NULL == buf)
	{
		data.return_state = ok;
		buffer_data.buffer_mode = unbuffered;
		buffer_data.buffer_size = 0;
	}
	else
	{
		data.return_state = ok;
		buffer_data.buffer_mode = fully_buffered;
		buffer_data.buffer_size = BUFSIZ;
	}

	WRAP_END(data, setbuf)
	return;
}

#ifdef HAVE_SETBUFFER
void WRAP(setbuffer)(FILE *stream, char *buf, size_t size)
{
	struct basic data;
	struct file_stream file_stream_data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, setbuffer, stream, buf, size)

	if (NULL == buf)
	{
		data.return_state = ok;
		buffer_data.buffer_mode = unbuffered;
		buffer_data.buffer_size = 0;
	}
	else
	{
		data.return_state = ok;
		buffer_data.buffer_mode = fully_buffered;
		buffer_data.buffer_size = size;
	}

	WRAP_END(data, setbuffer)
	return;
}
#endif

#ifdef HAVE_SETLINEBUF
void WRAP(setlinebuf)(FILE *stream)
{
	struct basic data;
	struct file_stream file_stream_data;
	struct buffer_function buffer_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, buffer_function, buffer_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, setlinebuf, stream)

	data.return_state = ok;
	buffer_data.buffer_mode = line_buffered;
	buffer_data.buffer_size = 0;

	WRAP_END(data, setlinebuf)
	return;
}
#endif

#ifdef HAVE_FILENO
int WRAP(fileno)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct fileno_function fileno_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fileno_function, fileno_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, fileno, stream)

	fileno_data.file_descriptor = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, fileno)
	return ret;
}
#endif

FILE *WRAP(tmpfile)(void)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = "";
	open_data.mode = check_mode("w+b", &open_data.creation, &open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
				   &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, tmpfile)

	if (NULL == file)
	{
		data.return_state = error;
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id(CALL_REAL_POSIX_SYNC(fileno)(file), &(open_data.id));
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, tmpfile)
	return file;
}

#ifdef HAVE_TMPFILE64
FILE *WRAP(tmpfile64)(void)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	struct open_function open_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, open_function, open_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	open_data.file_name = "";
	open_data.mode = check_mode("w+b", &open_data.creation, &open_data.status);
	get_mode_flags(S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH,
				   &open_data.file_mode);

	CALL_REAL_FUNCTION_RET(data, file, tmpfile64)

	if (NULL == file)
	{
		data.return_state = error;
		open_data.id.device_id = 0;
		open_data.id.inode_nr = 0;
	}
	else
	{
		data.return_state = ok;
		get_file_id(CALL_REAL_POSIX_SYNC(fileno)(file), &(open_data.id));
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, tmpfile64)
	return file;
}
#endif

FILE *WRAP(popen)(const char *command, const char *type)
{
	FILE *file;
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);

	CALL_REAL_FUNCTION_RET(data, file, popen, command, type)

	if (NULL == file)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	file_stream_data.stream = file;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	WRAP_END(data, popen)
	return file;
}

int WRAP(__freadable)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __freadable, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, __freadable)
	return ret;
}

int WRAP(__fwritable)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __fwritable, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, __fwritable)
	return ret;
}

int WRAP(__freading)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __freading, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, __freading)
	return ret;
}

int WRAP(__fwriting)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __fwriting, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, __fwriting)
	return ret;
}

int WRAP(__fsetlocking)(FILE *stream, int type)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct lock_mode_function lock_mode_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, lock_mode_function,
						   lock_mode_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	lock_mode_data.set_mode = get_lock_mode(type);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __fsetlocking, stream, type)

	data.return_state = ok;
	lock_mode_data.return_mode = get_lock_mode(ret);

	WRAP_END(data, __fsetlocking)
	return ret;
}

void WRAP(_flushlbf)(void)
{
	struct basic data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_FUNCTION(data, _flushlbf)

	data.return_state = ok;

	WRAP_END(data, _flushlbf)
	return;
}

void WRAP(__fpurge)(FILE *stream)
{
	struct basic data;
	struct file_stream file_stream_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P_NULL(data, function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION(data, __fpurge, stream)

	data.return_state = ok;

	WRAP_END(data, __fpurge)
	return;
}

int WRAP(__flbf)(FILE *stream)
{
	int ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct information_function information_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, information_function,
						   information_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __flbf, stream)

	data.return_state = ok;
	if (0 == ret)
	{
		information_data.return_bool = false;
	}
	else
	{
		information_data.return_bool = true;
	}

	WRAP_END(data, __flbf)
	return ret;
}

size_t WRAP(__fbufsize)(FILE *stream)
{
	size_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct bufsize_function bufsize_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, bufsize_function, bufsize_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __fbufsize, stream)

	data.return_state = ok;
	bufsize_data.buffer_size = ret;

	WRAP_END(data, __fbufsize)
	return ret;
}

size_t WRAP(__fpending)(FILE *stream)
{
	size_t ret;
	struct basic data;
	struct file_stream file_stream_data;
	struct bufsize_function bufsize_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, bufsize_function, bufsize_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	file_stream_data.stream = stream;
	JSON_STRUCT_SET_VOID_P(data, file_type, file_stream, file_stream_data)

	CALL_REAL_FUNCTION_RET(data, ret, __fpending, stream)

	data.return_state = ok;
	bufsize_data.buffer_size = ret;

	WRAP_END(data, __fpending)
	return ret;
}

pid_t WRAP(fork)(void)
{
	pid_t ret;
	struct basic data;
	struct fork_function fork_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fork_function,
						   fork_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_FUNCTION_RET(data, ret, fork)

	fork_function_data.pid = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, fork)
	return ret;
}

#ifdef HAVE_VFORK
pid_t WRAP(vfork)(void)
{
	pid_t ret;
	struct basic data;
	struct fork_function fork_function_data;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fork_function,
						   fork_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	CALL_REAL_FUNCTION_RET(data, ret, vfork)

	fork_function_data.pid = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, vfork)
	return ret;
}
#endif

#ifdef HAVE_CLONE
int WRAP(clone)(int (*fn)(void *), void *child_stack, int flags, void *arg, ... /* pid_t *ptid, void *newtls, pid_t *ctid */)
{
	int ret;
	struct basic data;
	struct fork_function fork_function_data;
	va_list ap;
	pid_t *ptid;
	void *newtls;
	pid_t *ctid;
	WRAP_START(data)

	get_basic(&data);
	JSON_STRUCT_SET_VOID_P(data, function_data, fork_function,
						   fork_function_data)
	POSIX_IO_SET_FUNCTION_NAME(data.function_name);
	JSON_STRUCT_SET_VOID_P_NULL(data, file_type)

	va_start(ap, arg);
	if (CLONE_PARENT_SETTID & flags || CLONE_SETTLS & flags || CLONE_CHILD_CLEARTID & flags || CLONE_CHILD_SETTID & flags)
	{
		ptid = va_arg(ap, pid_t *);
		newtls = va_arg(ap, void *);
		ctid = va_arg(ap, pid_t *);
		CALL_REAL_FUNCTION_RET(data, ret, clone, fn, child_stack, flags, arg, ptid, newtls, ctid)
	}
	else
	{
		CALL_REAL_FUNCTION_RET(data, ret, clone, fn, child_stack, flags, arg)
	}
	va_end(ap);

	fork_function_data.pid = ret;
	if (-1 == ret)
	{
		data.return_state = error;
	}
	else
	{
		data.return_state = ok;
	}

	WRAP_END(data, clone)
	return ret;
}
#endif
