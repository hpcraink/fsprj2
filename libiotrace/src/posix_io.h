#ifndef LIBIOTRACE_POSIX_IO_H
#define LIBIOTRACE_POSIX_IO_H

#include "libiotrace_config.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#include <wchar.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/uio.h>
#include <sys/mman.h>
#include "wrapper_defines.h"

/* Function pointers for glibc functions */

/* POSIX and GNU extension byte */
// ToDo: set HAVE_OPEN_ELLIPSES or check in wrapper if optional argument is needed/provided (like in mremap): test it!!!!
#define HAVE_OPEN_ELLIPSES
#ifdef HAVE_OPEN_ELLIPSES
REAL_TYPE int REAL(open)(const char *filename, int flags, ...) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE //ToDo: HAVE_...
REAL_TYPE int REAL(open64)(const char *filename, int flags, ...) REAL_INIT;
#endif
#else
//REAL_TYPE int REAL(open)(const char *filename, int flags) REAL_INIT; // ToDo: function without mode possible?
REAL_TYPE int REAL(open)(const char *filename, int flags, mode_t mode) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE int REAL(open64)(const char *filename, int flags, mode_t mode) REAL_INIT;
#endif
#endif
//ToDo: ifdef _ATFILE_SOURCE
#if _POSIX_C_SOURCE >= 200809L || _ATFILE_SOURCE
REAL_TYPE int REAL(openat)(int dirfd, const char *pathname, int flags, ...) REAL_INIT; //ToDo: test HAVE_OPEN_ELLIPSE
#endif
REAL_TYPE int REAL(creat)(const char *filename, mode_t mode) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE int REAL(creat64)(const char *filename, mode_t mode) REAL_INIT;
#endif
REAL_TYPE int REAL(close)(int filedes) REAL_INIT;
REAL_TYPE ssize_t REAL(read)(int filedes, void *buffer, size_t size) REAL_INIT;
#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 500
REAL_TYPE ssize_t REAL(pread)(int filedes, void *buffer, size_t size, off_t offset) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE ssize_t REAL(pread64)(int filedes, void *buffer, size_t size, off64_t offset) REAL_INIT;
#endif
#endif
REAL_TYPE ssize_t REAL(write)(int filedes, const void *buffer, size_t size) REAL_INIT;
#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 500
REAL_TYPE ssize_t REAL(pwrite)(int filedes, const void *buffer, size_t size, off_t offset) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE ssize_t REAL(pwrite64)(int filedes, const void *buffer, size_t size, off64_t offset) REAL_INIT;
#endif
#endif
REAL_TYPE off_t REAL(lseek)(int filedes, off_t offset, int whence) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE off64_t REAL(lseek64)(int filedes, off64_t offset, int whence) REAL_INIT;
#endif
#ifdef _DEFAULT_SOURCE
REAL_TYPE ssize_t REAL(readv)(int filedes, const struct iovec *vector, int count) REAL_INIT;
REAL_TYPE ssize_t REAL(writev)(int filedes, const struct iovec *vector, int count) REAL_INIT;
REAL_TYPE ssize_t REAL(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE ssize_t REAL(preadv64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset) REAL_INIT;
#endif
REAL_TYPE ssize_t REAL(pwritev)(int fd, const struct iovec *iov, int iovcnt, off_t offset) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE ssize_t REAL(pwritev64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset) REAL_INIT;
#endif
REAL_TYPE ssize_t REAL(preadv2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE ssize_t REAL(preadv64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags) REAL_INIT;
#endif
REAL_TYPE ssize_t REAL(pwritev2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE ssize_t REAL(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags) REAL_INIT;
#endif
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(_GNU_SOURCE)
REAL_TYPE ssize_t REAL(copy_file_range)(int inputfd, off64_t *inputpos, int outputfd, off64_t *outputpos, size_t length, unsigned int flags) REAL_INIT;
#endif
REAL_TYPE void * REAL(mmap)(void *address, size_t length, int protect, int flags, int filedes, off_t offset) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE void * REAL(mmap64)(void *address, size_t length, int protect, int flags, int filedes, off64_t offset) REAL_INIT;
#endif
REAL_TYPE int REAL(munmap)(void *addr, size_t length) REAL_INIT;
REAL_TYPE int REAL(msync)(void *address, size_t length, int flags) REAL_INIT;
#ifdef _GNU_SOURCE
REAL_TYPE void * REAL(mremap)(void *old_address, size_t old_length, size_t new_length, int flags, ...) REAL_INIT;
#endif
//ToDo: dup
//ToDo: int fcntl(int fd, int cmd, ... /* arg */ );
//ToDo: int fsync(int fd);
//ToDo: int fdatasync(int fd);
//ToDo: int unlink(const char *pathname); and int unlinkat(int dirfd, const char *pathname, int flags); ????
//ToDo: struct dirent *readdir(DIR *dirp); ???? Dir-functions?
//ToDo: int select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, struct timeval *timeout);
//ToDo: int pselect(int nfds, fd_set *readfds, fd_set *writefds, fd_set *exceptfds, const struct timespec *timeout, const sigset_t *sigmask);
//ToDo: int fallocate(int fd, int mode, off_t offset, off_t len); and posix_fallocate
//ToDo: ssize_t sendfile(int out_fd, int in_fd, off_t *offset, size_t count);
//ToDo: ssize_t splice(int fd_in, loff_t *off_in, int fd_out, loff_t *off_out, size_t len, unsigned int flags);

/* POSIX and GNU extension stream */
REAL_TYPE FILE * REAL(fopen)(const char *filename, const char *opentype) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE FILE * REAL(fopen64)(const char *filename, const char *opentype) REAL_INIT;
#endif
REAL_TYPE FILE * REAL(freopen)(const char *filename, const char *opentype, FILE *stream) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE FILE * REAL(freopen64)(const char *filename, const char *opentype, FILE *stream) REAL_INIT;
#endif
REAL_TYPE FILE * REAL(fdopen)(int fd, const char *opentype) REAL_INIT;
REAL_TYPE int REAL(fclose)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fcloseall)(void) REAL_INIT;
REAL_TYPE void REAL(flockfile)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(ftrylockfile)(FILE *stream) REAL_INIT;
REAL_TYPE void REAL(funlockfile)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fwide)(FILE *stream, int mode) REAL_INIT;
REAL_TYPE int REAL(fputc)(int c, FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(fputwc)(wchar_t wc, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fputc_unlocked)(int c, FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(fputwc_unlocked)(wchar_t wc, FILE *stream) REAL_INIT;
#ifdef putc
#   if putc == _IO_putc
#       define putc_MACRO _IO_putc
#   else
#       error "Unknown macro for putc function!"
#   endif
#else
#   define putc_MACRO putc
#endif
REAL_TYPE int REAL(putc_MACRO)(int c, FILE *stream) REAL_INIT;
#ifdef putwc
#   error "Unknown macro for putwc function!"
#else
#   define putwc_MACRO putwc
#endif
REAL_TYPE wint_t REAL(putwc_MACRO)(wchar_t wc, FILE *stream) REAL_INIT;
#ifdef putc_unlocked
#   error "Unknown macro for putc_unlocked function!"
#else
#   define putc_unlocked_MACRO putc_unlocked
#endif
REAL_TYPE int REAL(putc_unlocked_MACRO)(int c, FILE *stream) REAL_INIT;
#ifdef putwc_unlocked
#   error "Unknown macro for putwc_unlocked function!"
#else
#   define putwc_unlocked_MACRO putwc_unlocked
#endif
REAL_TYPE wint_t REAL(putwc_unlocked_MACRO)(wchar_t wc, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fputs)(const char *s, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fputws)(const wchar_t *ws, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fputs_unlocked)(const char *s, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fputws_unlocked)(const wchar_t *ws, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(putw)(int w, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fgetc)(FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(fgetwc)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fgetc_unlocked)(FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(fgetwc_unlocked)(FILE *stream) REAL_INIT;
#ifdef getc
#   if getc == _IO_getc
#       define getc_MACRO _IO_getc
#   else
#       error "Unknown macro for getc function!"
#   endif
#else
#   define getc_MACRO getc
#endif
REAL_TYPE int REAL(getc_MACRO)(FILE *stream) REAL_INIT;
#ifdef getwc
#   error "Unknown macro for getwc function!"
#else
#   define getwc_MACRO getwc
#endif
REAL_TYPE wint_t REAL(getwc_MACRO)(FILE *stream) REAL_INIT;
#ifdef getc_unlocked
#   error "Unknown macro for getc_unlocked function!"
#else
#   define getc_unlocked_MACRO getc_unlocked
#endif
REAL_TYPE int REAL(getc_unlocked_MACRO)(FILE *stream) REAL_INIT;
#ifdef getwc_unlocked
#   error "Unknown macro for getwc_unlocked function!"
#else
#   define getwc_unlocked_MACRO getwc_unlocked
#endif
REAL_TYPE wint_t REAL(getwc_unlocked_MACRO)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(getw)(FILE *stream) REAL_INIT;
REAL_TYPE ssize_t REAL(getline)(char **lineptr, size_t *n, FILE *stream) REAL_INIT;
REAL_TYPE ssize_t REAL(getdelim)(char **lineptr, size_t *n, int delimiter, FILE *stream) REAL_INIT;
REAL_TYPE char * REAL(fgets)(char *s, int count, FILE *stream) REAL_INIT;
REAL_TYPE wchar_t * REAL(fgetws)(wchar_t *ws, int count, FILE *stream) REAL_INIT;
REAL_TYPE char * REAL(fgets_unlocked)(char *s, int count, FILE *stream) REAL_INIT;
REAL_TYPE wchar_t * REAL(fgetws_unlocked)(wchar_t *ws, int count, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(ungetc)(int c, FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(ungetwc)(wint_t wc, FILE *stream) REAL_INIT;
REAL_TYPE size_t REAL(fread)(void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
REAL_TYPE size_t REAL(fread_unlocked)(void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
REAL_TYPE size_t REAL(fwrite)(const void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
REAL_TYPE size_t REAL(fwrite_unlocked)(const void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fprintf)(FILE *stream, const char *template, ...) REAL_INIT;
REAL_TYPE int REAL(fwprintf)(FILE *stream, const wchar_t *template, ...) REAL_INIT;
REAL_TYPE int REAL(vfprintf)(FILE *stream, const char *template, va_list ap) REAL_INIT;
REAL_TYPE int REAL(vfwprintf)(FILE *stream, const wchar_t *template, va_list ap) REAL_INIT;
REAL_TYPE int REAL(fscanf)(FILE *stream, const char *template, ...) REAL_INIT;
REAL_TYPE int REAL(fwscanf)(FILE *stream, const wchar_t *template, ...) REAL_INIT;
REAL_TYPE int REAL(vfscanf)(FILE *stream, const char *template, va_list ap) REAL_INIT;
REAL_TYPE int REAL(vfwscanf)(FILE *stream, const wchar_t *template, va_list ap) REAL_INIT;
REAL_TYPE int REAL(feof)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(feof_unlocked)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(ferror)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(ferror_unlocked)(FILE *stream) REAL_INIT;
REAL_TYPE void REAL(clearerr)(FILE *stream) REAL_INIT;
REAL_TYPE void REAL(clearerr_unlocked)(FILE *stream) REAL_INIT;
REAL_TYPE long int REAL(ftell)(FILE *stream) REAL_INIT;
REAL_TYPE off_t REAL(ftello)(FILE *stream) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE off64_t REAL(ftello64)(FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(fseek)(FILE *stream, long int offset, int whence) REAL_INIT;
REAL_TYPE int REAL(fseeko)(FILE *stream, off_t offset, int whence) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE int REAL(fseeko64)(FILE *stream, off64_t offset, int whence) REAL_INIT;
#endif
REAL_TYPE void REAL(rewind)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fgetpos)(FILE *stream, fpos_t *position) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE int REAL(fgetpos64)(FILE *stream, fpos64_t *position) REAL_INIT;
#endif
REAL_TYPE int REAL(fsetpos)(FILE *stream, const fpos_t *position) REAL_INIT;
#ifdef _LARGEFILE64_SOURCE
REAL_TYPE int REAL(fsetpos64)(FILE *stream, const fpos64_t *position) REAL_INIT;
#endif
REAL_TYPE int REAL(fflush)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fflush_unlocked)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(setvbuf)(FILE *stream, char *buf, int mode, size_t size) REAL_INIT;
REAL_TYPE void REAL(setbuf)(FILE *stream, char *buf) REAL_INIT;
REAL_TYPE void REAL(setbuffer)(FILE *stream, char *buf, size_t size) REAL_INIT;
REAL_TYPE void REAL(setlinebuf)(FILE *stream) REAL_INIT;
//ToDo: purge !!!

/* Solaris extensions for POSIX stream */
REAL_TYPE int REAL(__freadable)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__fwritable)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__freading)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__fwriting)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__fsetlocking)(FILE *stream, int type) REAL_INIT;
REAL_TYPE void REAL(_flushlbf)(void) REAL_INIT;
REAL_TYPE void REAL(__fpurge)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__flbf)(FILE *stream) REAL_INIT;
REAL_TYPE size_t REAL(__fbufsize)(FILE *stream) REAL_INIT;
REAL_TYPE size_t REAL(__fpending)(FILE *stream) REAL_INIT;

#ifndef IO_LIB_STATIC
static void posix_io_init() ATTRIBUTE_CONSTRUCTOR;
/* Initialize pointers for glibc functions.
 * This has to be in the header file because other files use the "__real_" functions
 * instead of the normal posix functions (e.g. see event.c or json_defines.h). */
static void posix_io_init() {
	DLSYM(open);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(open64);
#endif
	DLSYM(openat);
	DLSYM(creat);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(creat64);
#endif
	DLSYM(close);
	DLSYM(read);
#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 500
	DLSYM(pread);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(pread64);
#endif
#endif
	DLSYM(write);
#if _POSIX_C_SOURCE >= 200809L || _XOPEN_SOURCE >= 500
	DLSYM(pwrite);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(pwrite64);
#endif
#endif
	DLSYM(lseek);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(lseek64);
#endif
#ifdef _DEFAULT_SOURCE
	DLSYM(readv);
	DLSYM(writev);
	DLSYM(preadv);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(preadv64);
#endif
	DLSYM(pwritev);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(pwritev64);
#endif
	DLSYM(preadv2);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(preadv64v2);
#endif
	DLSYM(pwritev2);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(pwritev64v2);
#endif
#endif
#if defined(_LARGEFILE64_SOURCE) && defined(_GNU_SOURCE)
	DLSYM(copy_file_range);
#endif
	DLSYM(mmap);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(mmap64);
#endif
	DLSYM(munmap);
	DLSYM(msync);
#ifdef _GNU_SOURCE
	DLSYM(mremap);
#endif

	DLSYM(fopen);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(fopen64);
#endif
	DLSYM(freopen);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(freopen64);
#endif
	DLSYM(fdopen);
	DLSYM(fclose);
	DLSYM(fcloseall);
	DLSYM(flockfile);
	DLSYM(ftrylockfile);
	DLSYM(funlockfile);
	DLSYM(fwide);
	DLSYM(fputc);
	DLSYM(fputwc);
	DLSYM(fputc_unlocked);
	DLSYM(fputwc_unlocked);
	DLSYM(putc_MACRO);
	DLSYM(putwc_MACRO);
	DLSYM(putc_unlocked_MACRO);
	DLSYM(putwc_unlocked_MACRO);
	DLSYM(fputs);
	DLSYM(fputws);
	DLSYM(fputs_unlocked);
	DLSYM(fputws_unlocked);
	DLSYM(putw);
	DLSYM(fgetc);
	DLSYM(fgetwc);
	DLSYM(fgetc_unlocked);
	DLSYM(fgetwc_unlocked);
	DLSYM(getc_MACRO);
	DLSYM(getwc_MACRO);
	DLSYM(getc_unlocked_MACRO);
	DLSYM(getwc_unlocked_MACRO);
	DLSYM(getw);
	DLSYM(getline);
	DLSYM(getdelim);
	DLSYM(fgets);
	DLSYM(fgetws);
	DLSYM(fgets_unlocked);
	DLSYM(fgetws_unlocked);
	DLSYM(ungetc);
	DLSYM(ungetwc);
	DLSYM(fread);
	DLSYM(fread_unlocked);
	DLSYM(fwrite);
	DLSYM(fwrite_unlocked);
	DLSYM(fprintf);
	DLSYM(fwprintf);
	DLSYM(vfprintf);
	DLSYM(vfwprintf);
	DLSYM(fscanf);
	DLSYM(fwscanf);
	DLSYM(vfscanf);
	DLSYM(vfwscanf);
	DLSYM(feof);
	DLSYM(feof_unlocked);
	DLSYM(ferror);
	DLSYM(ferror_unlocked);
	DLSYM(clearerr);
	DLSYM(clearerr_unlocked);
	DLSYM(ftell);
	DLSYM(ftello);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(ftello64);
#endif
	DLSYM(fseek);
	DLSYM(fseeko);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(fseeko64);
#endif
	DLSYM(rewind);
	DLSYM(fgetpos);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(fgetpos64);
#endif
	DLSYM(fsetpos);
#ifdef _LARGEFILE64_SOURCE
	DLSYM(fsetpos64);
#endif
	DLSYM(fflush);
	DLSYM(fflush_unlocked);
	DLSYM(setvbuf);
	DLSYM(setbuf);
	DLSYM(setbuffer);
	DLSYM(setlinebuf);

	DLSYM(__freadable);
	DLSYM(__fwritable);
	DLSYM(__freading);
	DLSYM(__fwriting);
	DLSYM(__fsetlocking);
	DLSYM(_flushlbf);
	DLSYM(__fpurge);
	DLSYM(__flbf);
	DLSYM(__fbufsize);
	DLSYM(__fpending);
}
#endif

#endif /* LIBIOTRACE_POSIX_IO_H */
