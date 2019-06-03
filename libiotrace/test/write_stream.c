#include <stdio.h>
#include <stdio_ext.h>
#include <wchar.h>
#include <assert.h>

int main(void) {
	fprintf(stderr, "test error\n");
	fprintf(stdout, "test output\n");

	return 0;
}
