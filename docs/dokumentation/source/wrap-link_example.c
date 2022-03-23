#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* Function pointers for glibc functions */
ssize_t __real_write(int fd, const void *buf, size_t count);
int __real_puts(const char* str);

ssize_t __wrap_write (int fd, const void *buf, size_t count)
{
	char print[40];
	sprintf(print, "write:chars#:%lu", count);
	writeData(print);

    return __real_write(fd, buf, count);
}

int __wrap_puts (const char* str)
{
	char print[40];
	sprintf(print, "puts:chars#:%lu", strlen(str));
	writeData(print);

    return __real_puts(str);
}
