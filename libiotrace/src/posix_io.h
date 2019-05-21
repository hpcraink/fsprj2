#ifndef LIBIOTRACE_POSIX_IO_H
#define LIBIOTRACE_POSIX_IO_H

#include "libiotrace_config.h"
#ifdef HAVE_STDLIB_H
#  include <stdlib.h>
#endif
#include <wchar.h>
#include <stdio.h>

#define REAL(function_macro) __REAL(function_macro)
#ifdef IO_LIB_STATIC
#  define REAL_TYPE
#  define __REAL(function_name) __real_##function_name
#  define REAL_INIT
#else
#  define REAL_TYPE static
#  define __REAL(function_name) (*__real_##function_name)
#  define REAL_INIT = NULL
#endif

/* Function pointers for glibc functions */
/* POSIX and GNU extension byte */
REAL_TYPE int REAL(open)(const char *pathname, int flags) REAL_INIT;
REAL_TYPE int REAL(close)(int fd) REAL_INIT;
REAL_TYPE ssize_t REAL(read)(int fd, void *buf, size_t count) REAL_INIT;
/* POSIX and GNU extension stream */
REAL_TYPE FILE * REAL(fopen)(const char *filename, const char *opentype) REAL_INIT;
REAL_TYPE FILE * REAL(fopen64)(const char *filename, const char *opentype) REAL_INIT;
REAL_TYPE FILE * REAL(freopen)(const char *filename, const char *opentype, FILE *stream) REAL_INIT;
REAL_TYPE FILE * REAL(freopen64)(const char *filename, const char *opentype, FILE *stream) REAL_INIT;
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
//long int ftell ( FILE *stream )
/* Solaris extensions for POSIX stream */
REAL_TYPE int REAL(__freadable)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__fwritable)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__freading)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__fwriting)(FILE *stream) REAL_INIT;
REAL_TYPE int REAL(__fsetlocking)(FILE *stream, int type) REAL_INIT;

#ifndef IO_LIB_STATIC
static void posix_io_init() ATTRIBUTE_CONSTRUCTOR;
/* Initialize pointers for glibc functions.
 * This has to be in the header file because other files use the "__real_" functions
 * instead of the normal posix functions (e.g. see event.c or json_defines.h). */
static void posix_io_init() {
#ifdef _GNU_SOURCE
#define DLSYM(function_macro) __DLSYM(function_macro)
#define __DLSYM(function) do { __real_##function = dlsym(RTLD_NEXT, #function); \
                               assert(NULL != __real_##function); \
                             } while (0)
	DLSYM(open);
	DLSYM(close);
	DLSYM(read);

	DLSYM(fopen);
	DLSYM(fopen64);
	DLSYM(freopen);
	DLSYM(freopen64);
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

	DLSYM(__freadable);
	DLSYM(__fwritable);
	DLSYM(__freading);
	DLSYM(__fwriting);
	DLSYM(__fsetlocking);
#endif
}
#endif

#endif /* LIBIOTRACE_POSIX_IO_H */
