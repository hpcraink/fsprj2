#define _GNU_SOURCE
#include <dlfcn.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdarg.h>
#include <pthread.h>

static void init()__attribute__((constructor));
static void cleanup()__attribute__((destructor));

/* Function pointers for glibc functions */
static ssize_t (*real_write)(int fd, const void *buf, size_t count) = NULL;
static int (*real_puts)(const char* str) = NULL;
static int (*real_printf)(const char *__restrict format, ...) = NULL;
static int (*real_vprintf)(const char *__restrict format, _G_va_list arg) = NULL;

/* Buffer */
#define BUFFER_SIZE_BYTES 400
static char data_buffer[BUFFER_SIZE_BYTES];
static char* endpos;
static char* pos;

/* Mutex */
static pthread_mutex_t lock;

void printData() {
	real_printf(data_buffer);
	pos = data_buffer;
	*pos = '\0';
}

void writeData(char *data) {
	int tmp_pos;
	char print[100];
	sprintf(print, "pid:%lu;pts:%lu;", getpid(), pthread_self());
	strcat(print, data);
	strcat(print, "\n");

	/* write (synchronized) */
	pthread_mutex_lock(&lock);
	if (pos + strlen(print) > endpos) {
		printData();
	}
	strcpy(pos, print);
	pos += strlen(print);

	pthread_mutex_unlock(&lock);
}

void cleanup() {
	printData();

	pthread_mutex_destroy(&lock);
}

static void init() {
	real_write = dlsym(RTLD_NEXT, "write");
	real_puts = dlsym(RTLD_NEXT, "puts");
	real_printf = dlsym(RTLD_NEXT, "printf");
	real_vprintf = dlsym(RTLD_NEXT, "vprintf");

	endpos = data_buffer + BUFFER_SIZE_BYTES - 1;
	pos = data_buffer;

	pthread_mutex_init(&lock, NULL);
}

ssize_t write(int fd, const void *buf, size_t count) {
	char print[40];
	sprintf(print, "write:chars#:%lu", count);
	writeData(print);

	return real_write(fd, buf, count);
}

int puts(const char* str) {
	char print[40];
	sprintf(print, "puts:chars#:%lu", strlen(str));
	writeData(print);

	return real_puts(str);
}

int printf(const char *__restrict format, ...) {
	va_list args;
	int retval;

	char print[40];
	sprintf(print, "printf:chars#:%lu", strlen(format));
	writeData(print);

	va_start(args, format);
	/* use vprintf instead of printf because of the variable parameter-list */
	retval = real_vprintf(format, args);
	va_end(args);
	return retval;
}
