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
#if HAVE_OPEN64 //ToDo: HAVE_...
REAL_TYPE int REAL(open64)(const char *filename, int flags, ...) REAL_INIT;
#endif
#else
//REAL_TYPE int REAL(open)(const char *filename, int flags) REAL_INIT; // ToDo: function without mode possible?
REAL_TYPE int REAL(open)(const char *filename, int flags, mode_t mode) REAL_INIT;
#if HAVE_OPEN64
REAL_TYPE int REAL(open64)(const char *filename, int flags, mode_t mode) REAL_INIT;
#endif
#endif
#if HAVE_OPENAT
REAL_TYPE int REAL(openat)(int dirfd, const char *pathname, int flags, ...) REAL_INIT; //ToDo: test HAVE_OPEN_ELLIPSE
#endif
REAL_TYPE int REAL(creat)(const char *filename, mode_t mode) REAL_INIT;
#ifdef HAVE_CREAT64
REAL_TYPE int REAL(creat64)(const char *filename, mode_t mode) REAL_INIT;
#endif
REAL_TYPE int REAL(close)(int filedes) REAL_INIT;
REAL_TYPE ssize_t REAL(read)(int filedes, void *buffer, size_t size) REAL_INIT;
#if HAVE_PREAD
REAL_TYPE ssize_t REAL(pread)(int filedes, void *buffer, size_t size, off_t offset) REAL_INIT;
#endif
#if HAVE_PREAD64
REAL_TYPE ssize_t REAL(pread64)(int filedes, void *buffer, size_t size, off64_t offset) REAL_INIT;
#endif
REAL_TYPE ssize_t REAL(write)(int filedes, const void *buffer, size_t size) REAL_INIT;
#if HAVE_PWRITE
REAL_TYPE ssize_t REAL(pwrite)(int filedes, const void *buffer, size_t size, off_t offset) REAL_INIT;
#endif
#if HAVE_PWRITE64
REAL_TYPE ssize_t REAL(pwrite64)(int filedes, const void *buffer, size_t size, off64_t offset) REAL_INIT;
#endif
REAL_TYPE off_t REAL(lseek)(int filedes, off_t offset, int whence) REAL_INIT;
#if HAVE_LSEEK64
REAL_TYPE off64_t REAL(lseek64)(int filedes, off64_t offset, int whence) REAL_INIT;
#endif
#if HAVE_READV
REAL_TYPE ssize_t REAL(readv)(int filedes, const struct iovec *vector, int count) REAL_INIT;
#endif
#if HAVE_WRITEV
REAL_TYPE ssize_t REAL(writev)(int filedes, const struct iovec *vector, int count) REAL_INIT;
#endif
#if HAVE_PREADV
REAL_TYPE ssize_t REAL(preadv)(int fd, const struct iovec *iov, int iovcnt, off_t offset) REAL_INIT;
#endif
#if HAVE_PREADV64
REAL_TYPE ssize_t REAL(preadv64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset) REAL_INIT;
#endif
#if HAVE_PWRITEV
REAL_TYPE ssize_t REAL(pwritev)(int fd, const struct iovec *iov, int iovcnt, off_t offset) REAL_INIT;
#endif
#if HAVE_PWRITEV64
REAL_TYPE ssize_t REAL(pwritev64)(int fd, const struct iovec *iov, int iovcnt, off64_t offset) REAL_INIT;
#endif
#if HAVE_PREADV2
REAL_TYPE ssize_t REAL(preadv2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags) REAL_INIT;
#endif
#if HAVE_PREADV64V2
REAL_TYPE ssize_t REAL(preadv64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags) REAL_INIT;
#endif
#if HAVE_PWRITEV2
REAL_TYPE ssize_t REAL(pwritev2)(int fd, const struct iovec *iov, int iovcnt, off_t offset, int flags) REAL_INIT;
#endif
#if HAVE_PWRITEV64V2
REAL_TYPE ssize_t REAL(pwritev64v2)(int fd, const struct iovec *iov, int iovcnt, off64_t offset, int flags) REAL_INIT;
#endif
#if HAVE_COPY_FILE_RANGE
REAL_TYPE ssize_t REAL(copy_file_range)(int inputfd, off64_t *inputpos, int outputfd, off64_t *outputpos, size_t length, unsigned int flags) REAL_INIT;
#endif
#if HAVE_MMAP
REAL_TYPE void * REAL(mmap)(void *address, size_t length, int protect, int flags, int filedes, off_t offset) REAL_INIT;
#endif
#if HAVE_MMAP64
REAL_TYPE void * REAL(mmap64)(void *address, size_t length, int protect, int flags, int filedes, off64_t offset) REAL_INIT;
#endif
#if HAVE_MUNMAP
REAL_TYPE int REAL(munmap)(void *addr, size_t length) REAL_INIT;
#endif
#if HAVE_MSYNC
REAL_TYPE int REAL(msync)(void *address, size_t length, int flags) REAL_INIT;
#endif
#if HAVE_MREMAP
REAL_TYPE void * REAL(mremap)(void *old_address, size_t old_length, size_t new_length, int flags, ...) REAL_INIT;
#endif
#if HAVE_MADVISE
REAL_TYPE int REAL(madvise)(void *addr, size_t length, int advice) REAL_INIT;
#endif
//ToDo: posix_madvise()
//ToDo: dprintf(), vdprintf() + feature test
//ToDo: dup
//ToDo: int fcntl(int fd, int cmd, ... /* arg */ );
//ToDo: int fsync(int fd);
//ToDo: sync
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
#if HAVE_FOPEN64
REAL_TYPE FILE * REAL(fopen64)(const char *filename, const char *opentype) REAL_INIT;
#endif
REAL_TYPE FILE * REAL(freopen)(const char *filename, const char *opentype, FILE *stream) REAL_INIT;
#if HAVE_FREOPEN64
REAL_TYPE FILE * REAL(freopen64)(const char *filename, const char *opentype, FILE *stream) REAL_INIT;
#endif
#if HAVE_FDOPEN
REAL_TYPE FILE * REAL(fdopen)(int fd, const char *opentype) REAL_INIT;
#endif
REAL_TYPE int REAL(fclose)(FILE *stream) REAL_INIT;
#if HAVE_FCLOSEALL
REAL_TYPE int REAL(fcloseall)(void) REAL_INIT;
#endif
#if HAVE_FLOCKFILE
REAL_TYPE void REAL(flockfile)(FILE *stream) REAL_INIT;
#endif
#if HAVE_FTRYLOCKFILE
REAL_TYPE int REAL(ftrylockfile)(FILE *stream) REAL_INIT;
#endif
#if HAVE_FUNLOCKFILE
REAL_TYPE void REAL(funlockfile)(FILE *stream) REAL_INIT;
#endif
#if HAVE_FWIDE
REAL_TYPE int REAL(fwide)(FILE *stream, int mode) REAL_INIT;
#endif
REAL_TYPE int REAL(fputc)(int c, FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(fputwc)(wchar_t wc, FILE *stream) REAL_INIT;
#if HAVE_FPUTC_UNLOCKED
REAL_TYPE int REAL(fputc_unlocked)(int c, FILE *stream) REAL_INIT;
#endif
#if HAVE_FPUTWC_UNLOCKED
REAL_TYPE wint_t REAL(fputwc_unlocked)(wchar_t wc, FILE *stream) REAL_INIT;
#endif
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
#if HAVE_PUTC_UNLOCKED
#ifdef putc_unlocked
#   error "Unknown macro for putc_unlocked function!"
#else
#   define putc_unlocked_MACRO putc_unlocked
#endif
REAL_TYPE int REAL(putc_unlocked_MACRO)(int c, FILE *stream) REAL_INIT;
#endif
#if HAVE_PUTWC_UNLOCKED
#ifdef putwc_unlocked
#   error "Unknown macro for putwc_unlocked function!"
#else
#   define putwc_unlocked_MACRO putwc_unlocked
#endif
REAL_TYPE wint_t REAL(putwc_unlocked_MACRO)(wchar_t wc, FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(fputs)(const char *s, FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fputws)(const wchar_t *ws, FILE *stream) REAL_INIT;
#if HAVE_FPUTS_UNLOCKED
REAL_TYPE int REAL(fputs_unlocked)(const char *s, FILE *stream) REAL_INIT;
#endif
#if HAVE_FPUTWS_UNLOCKED
REAL_TYPE int REAL(fputws_unlocked)(const wchar_t *ws, FILE *stream) REAL_INIT;
#endif
#if HAVE_PUTW
REAL_TYPE int REAL(putw)(int w, FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(fgetc)(FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(fgetwc)(FILE *stream) REAL_INIT;
#if HAVE_FGETC_UNLOCKED
REAL_TYPE int REAL(fgetc_unlocked)(FILE *stream) REAL_INIT;
#endif
#if HAVE_FGETWC_UNLOCKED
REAL_TYPE wint_t REAL(fgetwc_unlocked)(FILE *stream) REAL_INIT;
#endif
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
#if HAVE_GETC_UNLOCKED
#ifdef getc_unlocked
#   error "Unknown macro for getc_unlocked function!"
#else
#   define getc_unlocked_MACRO getc_unlocked
#endif
REAL_TYPE int REAL(getc_unlocked_MACRO)(FILE *stream) REAL_INIT;
#endif
#if HAVE_GETWC_UNLOCKED
#ifdef getwc_unlocked
#   error "Unknown macro for getwc_unlocked function!"
#else
#   define getwc_unlocked_MACRO getwc_unlocked
#endif
REAL_TYPE wint_t REAL(getwc_unlocked_MACRO)(FILE *stream) REAL_INIT;
#endif
#if HAVE_GETW
REAL_TYPE int REAL(getw)(FILE *stream) REAL_INIT;
#endif
#if HAVE_GETLINE
REAL_TYPE ssize_t REAL(getline)(char **lineptr, size_t *n, FILE *stream) REAL_INIT;
#endif
#if HAVE_GETDELIM
REAL_TYPE ssize_t REAL(getdelim)(char **lineptr, size_t *n, int delimiter, FILE *stream) REAL_INIT;
#endif
REAL_TYPE char * REAL(fgets)(char *s, int count, FILE *stream) REAL_INIT;
REAL_TYPE wchar_t * REAL(fgetws)(wchar_t *ws, int count, FILE *stream) REAL_INIT;
#if HAVE_FGETS_UNLOCKED
REAL_TYPE char * REAL(fgets_unlocked)(char *s, int count, FILE *stream) REAL_INIT;
#endif
#if HAVE_FGETWS_UNLOCKED
REAL_TYPE wchar_t * REAL(fgetws_unlocked)(wchar_t *ws, int count, FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(ungetc)(int c, FILE *stream) REAL_INIT;
REAL_TYPE wint_t REAL(ungetwc)(wint_t wc, FILE *stream) REAL_INIT;
REAL_TYPE size_t REAL(fread)(void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
#if HAVE_FREAD_UNLOCKED
REAL_TYPE size_t REAL(fread_unlocked)(void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
#endif
REAL_TYPE size_t REAL(fwrite)(const void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
#if HAVE_FWRITE_UNLOCKED
REAL_TYPE size_t REAL(fwrite_unlocked)(const void *data, size_t size, size_t count, FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(fprintf)(FILE *stream, const char *template, ...) REAL_INIT;
#if HAVE_FWPRINTF
REAL_TYPE int REAL(fwprintf)(FILE *stream, const wchar_t *template, ...) REAL_INIT;
#endif
REAL_TYPE int REAL(vfprintf)(FILE *stream, const char *template, va_list ap) REAL_INIT;
#if HAVE_VFWPRINTF
REAL_TYPE int REAL(vfwprintf)(FILE *stream, const wchar_t *template, va_list ap) REAL_INIT;
#endif
REAL_TYPE int REAL(fscanf)(FILE *stream, const char *template, ...) REAL_INIT;
#if HAVE_FWSCANF
REAL_TYPE int REAL(fwscanf)(FILE *stream, const wchar_t *template, ...) REAL_INIT;
#endif
#if HAVE_VFSCANF
REAL_TYPE int REAL(vfscanf)(FILE *stream, const char *template, va_list ap) REAL_INIT;
#endif
#if HAVE_VFWSCANF
REAL_TYPE int REAL(vfwscanf)(FILE *stream, const wchar_t *template, va_list ap) REAL_INIT;
#endif
REAL_TYPE int REAL(feof)(FILE *stream) REAL_INIT;
#if HAVE_FEOF_UNLOCKED
REAL_TYPE int REAL(feof_unlocked)(FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(ferror)(FILE *stream) REAL_INIT;
#if HAVE_FERROR_UNLOCKED
REAL_TYPE int REAL(ferror_unlocked)(FILE *stream) REAL_INIT;
#endif
//Todo: int fileno(FILE *stream);
REAL_TYPE void REAL(clearerr)(FILE *stream) REAL_INIT;
#if HAVE_CLEARERR_UNLOCKED
REAL_TYPE void REAL(clearerr_unlocked)(FILE *stream) REAL_INIT;
#endif
REAL_TYPE long int REAL(ftell)(FILE *stream) REAL_INIT;
#if HAVE_FTELLO
REAL_TYPE off_t REAL(ftello)(FILE *stream) REAL_INIT;
#endif
#if HAVE_FTELLO64
REAL_TYPE off64_t REAL(ftello64)(FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(fseek)(FILE *stream, long int offset, int whence) REAL_INIT;
#if HAVE_FSEEKO
REAL_TYPE int REAL(fseeko)(FILE *stream, off_t offset, int whence) REAL_INIT;
#endif
#if HAVE_FSEEKO64
REAL_TYPE int REAL(fseeko64)(FILE *stream, off64_t offset, int whence) REAL_INIT;
#endif
REAL_TYPE void REAL(rewind)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(fgetpos)(FILE *stream, fpos_t *position) REAL_INIT;
#if HAVE_FGETPOS64
REAL_TYPE int REAL(fgetpos64)(FILE *stream, fpos64_t *position) REAL_INIT;
#endif
REAL_TYPE int REAL(fsetpos)(FILE *stream, const fpos_t *position) REAL_INIT;
#if HAVE_FSETPOS64
REAL_TYPE int REAL(fsetpos64)(FILE *stream, const fpos64_t *position) REAL_INIT;
#endif
REAL_TYPE int REAL(fflush)(FILE *stream) REAL_INIT;
#if HAVE_FFLUSH_UNLOCKED
REAL_TYPE int REAL(fflush_unlocked)(FILE *stream) REAL_INIT;
#endif
REAL_TYPE int REAL(setvbuf)(FILE *stream, char *buf, int mode, size_t size) REAL_INIT;
REAL_TYPE void REAL(setbuf)(FILE *stream, char *buf) REAL_INIT;
#if HAVE_SETBUFFER
REAL_TYPE void REAL(setbuffer)(FILE *stream, char *buf, size_t size) REAL_INIT;
#endif
#if HAVE_SETLINEBUF
REAL_TYPE void REAL(setlinebuf)(FILE *stream) REAL_INIT;
#endif
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
#if HAVE_OPEN64
	DLSYM(open64);
#endif
#if HAVE_OPENAT
	DLSYM(openat);
#endif
	DLSYM(creat);
#ifdef HAVE_CREAT64
	DLSYM(creat64);
#endif
	DLSYM(close);
	DLSYM(read);
#if HAVE_PREAD
	DLSYM(pread);
#endif
#if HAVE_PREAD64
	DLSYM(pread64);
#endif
	DLSYM(write);
#if HAVE_PWRITE
	DLSYM(pwrite);
#endif
#if HAVE_PWRITE64
	DLSYM(pwrite64);
#endif
	DLSYM(lseek);
#if HAVE_LSEEK64
	DLSYM(lseek64);
#endif
#if HAVE_READV
	DLSYM(readv);
#endif
#if HAVE_WRITEV
	DLSYM(writev);
#endif
#if HAVE_PREADV
	DLSYM(preadv);
#endif
#if HAVE_PREADV64
	DLSYM(preadv64);
#endif
#if HAVE_PWRITEV
	DLSYM(pwritev);
#endif
#if HAVE_PWRITEV64
	DLSYM(pwritev64);
#endif
#if HAVE_PREADV2
	DLSYM(preadv2);
#endif
#if HAVE_PREADV64V2
	DLSYM(preadv64v2);
#endif
#if HAVE_PWRITEV2
	DLSYM(pwritev2);
#endif
#if HAVE_PWRITEV64V2
	DLSYM(pwritev64v2);
#endif
#if HAVE_COPY_FILE_RANGE
	DLSYM(copy_file_range);
#endif
#if HAVE_MMAP
	DLSYM(mmap);
#endif
#if HAVE_MMAP64
	DLSYM(mmap64);
#endif
#if HAVE_MUNMAP
	DLSYM(munmap);
#endif
#if HAVE_MSYNC
	DLSYM(msync);
#endif
#if HAVE_MREMAP
	DLSYM(mremap);
#endif
#if HAVE_MADVISE
	DLSYM(madvise);
#endif
	DLSYM(fopen);
#if HAVE_FOPEN64
	DLSYM(fopen64);
#endif
	DLSYM(freopen);
#if HAVE_FREOPEN64
	DLSYM(freopen64);
#endif
#if HAVE_FDOPEN
	DLSYM(fdopen);
#endif
	DLSYM(fclose);
#if HAVE_FCLOSEALL
	DLSYM(fcloseall);
#endif
#if HAVE_FLOCKFILE
	DLSYM(flockfile);
#endif
#if HAVE_FTRYLOCKFILE
	DLSYM(ftrylockfile);
#endif
#if HAVE_FUNLOCKFILE
	DLSYM(funlockfile);
#endif
#if HAVE_FWIDE
	DLSYM(fwide);
#endif
	DLSYM(fputc);
	DLSYM(fputwc);
#if HAVE_FPUTC_UNLOCKED
	DLSYM(fputc_unlocked);
#endif
#if HAVE_FPUTWC_UNLOCKED
	DLSYM(fputwc_unlocked);
#endif
	DLSYM(putc_MACRO);
	DLSYM(putwc_MACRO);
#if HAVE_PUTC_UNLOCKED
	DLSYM(putc_unlocked_MACRO);
#endif
#if HAVE_PUTWC_UNLOCKED
	DLSYM(putwc_unlocked_MACRO);
#endif
	DLSYM(fputs);
	DLSYM(fputws);
#if HAVE_FPUTS_UNLOCKED
	DLSYM(fputs_unlocked);
#endif
#if HAVE_FPUTWS_UNLOCKED
	DLSYM(fputws_unlocked);
#endif
#if HAVE_PUTW
	DLSYM(putw);
#endif
	DLSYM(fgetc);
	DLSYM(fgetwc);
#if HAVE_FGETC_UNLOCKED
	DLSYM(fgetc_unlocked);
#endif
#if HAVE_FGETWC_UNLOCKED
	DLSYM(fgetwc_unlocked);
#endif
	DLSYM(getc_MACRO);
	DLSYM(getwc_MACRO);
#if HAVE_GETC_UNLOCKED
	DLSYM(getc_unlocked_MACRO);
#endif
#if HAVE_GETWC_UNLOCKED
	DLSYM(getwc_unlocked_MACRO);
#endif
#if HAVE_GETW
	DLSYM(getw);
#endif
#if HAVE_GETLINE
	DLSYM(getline);
#endif
#if HAVE_GETDELIM
	DLSYM(getdelim);
#endif
	DLSYM(fgets);
	DLSYM(fgetws);
#if HAVE_FGETS_UNLOCKED
	DLSYM(fgets_unlocked);
#endif
#if HAVE_FGETWS_UNLOCKED
	DLSYM(fgetws_unlocked);
#endif
	DLSYM(ungetc);
	DLSYM(ungetwc);
	DLSYM(fread);
#if HAVE_FREAD_UNLOCKED
	DLSYM(fread_unlocked);
#endif
	DLSYM(fwrite);
#if HAVE_FWRITE_UNLOCKED
	DLSYM(fwrite_unlocked);
#endif
	DLSYM(fprintf);
#if HAVE_FWPRINTF
	DLSYM(fwprintf);
#endif
	DLSYM(vfprintf);
#if HAVE_VFWPRINTF
	DLSYM(vfwprintf);
#endif
	DLSYM(fscanf);
#if HAVE_FWSCANF
	DLSYM(fwscanf);
#endif
#if HAVE_VFSCANF
	DLSYM(vfscanf);
#endif
#if HAVE_VFWSCANF
	DLSYM(vfwscanf);
#endif
	DLSYM(feof);
#if HAVE_FEOF_UNLOCKED
	DLSYM(feof_unlocked);
#endif
	DLSYM(ferror);
#if HAVE_FERROR_UNLOCKED
	DLSYM(ferror_unlocked);
#endif
	DLSYM(clearerr);
#if HAVE_CLEARERR_UNLOCKED
	DLSYM(clearerr_unlocked);
#endif
	DLSYM(ftell);
#if HAVE_FTELLO
	DLSYM(ftello);
#endif
#if HAVE_FTELLO64
	DLSYM(ftello64);
#endif
	DLSYM(fseek);
#if HAVE_FSEEKO
	DLSYM(fseeko);
#endif
#if HAVE_FSEEKO64
	DLSYM(fseeko64);
#endif
	DLSYM(rewind);
	DLSYM(fgetpos);
#if HAVE_FGETPOS64
	DLSYM(fgetpos64);
#endif
	DLSYM(fsetpos);
#if HAVE_FSETPOS64
	DLSYM(fsetpos64);
#endif
	DLSYM(fflush);
#if HAVE_FFLUSH_UNLOCKED
	DLSYM(fflush_unlocked);
#endif
	DLSYM(setvbuf);
	DLSYM(setbuf);
#if HAVE_SETBUFFER
	DLSYM(setbuffer);
#endif
#if HAVE_SETLINEBUF
	DLSYM(setlinebuf);
#endif

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
