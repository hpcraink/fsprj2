#include <stdio.h>
#include <stdio_ext.h>
#include <wchar.h>
#include <assert.h>

int main(void) {
	fprintf(stderr, "test stderr\n");
	fprintf(stdout, "test stdout\n");

	return 0;
}
