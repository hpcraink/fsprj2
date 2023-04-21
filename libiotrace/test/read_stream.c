#include "libiotrace_config.h"

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
    int ret;

    file = fopen("/etc/passwd", "r");
    assert(NULL != file);

    fwide(file, 0);

    __freadable(file);
    __fwritable(file);
    __fsetlocking(file, FSETLOCKING_QUERY);

    ret = ftell(file);
    assert(-1 != ret);
    ret = ftello(file);
    assert(-1 != ret);

    flockfile(file);
    ret = fread(&buffer, sizeof(char), 1, file);
    assert(sizeof(char) == ret);
    funlockfile(file);

    ret = ftell(file);
    assert(-1 != ret);
    ret = ftello(file);
    assert(-1 != ret);
    fseeko(file, 4, SEEK_CUR);
    ret = ftello(file);
    assert(-1 != ret);
    rewind(file);
    ret = ftello(file);
    assert(-1 != ret);

    fwide(file, 0);

    file = freopen(NULL, "r", file);
    assert(NULL != file);

    fgetc(file);
    c = getc(file);
    ungetc(c, file);

    tmpLine = NULL;
    tmpSize = 0;
    ret = getline(&tmpLine, &tmpSize, file);
    assert(-1 != ret);
    free(tmpLine);

    tmpLine = (char*) malloc(50);
    assert (NULL != tmpLine);
    tmpSize = 50;
    ret = getdelim(&tmpLine, &tmpSize, '\n', file);
    assert(-1 != ret);
    free(tmpLine);

    fclose(file);
#ifdef HAVE_FCLOSEALL
    fcloseall();
#endif

    file = fopen("/etc/passwd", "w");
    //assert(NULL != file);

    return 0;
}
