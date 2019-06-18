#include <stdio.h>
#include <stdio_ext.h>
#include <stdlib.h>
#include <wchar.h>
#include <assert.h>

int main(void) {
	FILE *file;
	char buffer;
	char *tmpLine;
	size_t tmpSize;
	int c;

	file = fopen("/etc/passwd", "r");
	assert(NULL != file);

	fwide(file, 0);

	__freadable(file);
	__fwritable(file);
	__fsetlocking(file, FSETLOCKING_QUERY);

	ftell(file);
	ftello(file);

	flockfile(file);
	fread(&buffer, sizeof(char), 1, file);
	funlockfile(file);

	ftell(file);
	ftello(file);
	fseeko(file, 4, SEEK_CUR);
	ftello(file);
	rewind(file);
	ftello(file);

	fwide(file, 0);

	file = freopen(NULL, "r", file);
	assert(NULL != file);

	fgetc(file);
	c = getc(file);
	ungetc(c, file);
	tmpLine = NULL;
	tmpSize = 0;
	getline(&tmpLine, &tmpSize, file);
	free(tmpLine);

	fclose(file);

	fcloseall();

	file = fopen("/etc/passwd", "w");
	//assert(NULL != file);

	return 0;
}
