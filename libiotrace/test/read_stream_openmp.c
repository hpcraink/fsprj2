#include <stdio.h>
#include <assert.h>
#include "omp.h"

int main(void) {
#pragma omp parallel default(none) private(file, buffer)
{
    FILE * file;
    char buffer;

    file = fopen("/etc/passwd", "r");
    assert (NULL != file);

    fread(&buffer, sizeof(char), 1, file);

    return fclose(file);
}
}
