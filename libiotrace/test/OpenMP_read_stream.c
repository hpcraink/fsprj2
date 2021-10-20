#include <stdio.h>
#include <assert.h>
#include "omp.h"

int main(void) {
	FILE * file;
	char buffer;
	int ret;

#   pragma omp parallel private(file, buffer, ret)
    {
        file = fopen("/etc/passwd", "r");
        assert(NULL != file);

        fread(&buffer, sizeof(char), 1, file);
        ret = fclose(file);
        assert(ret == 0);

    }

    return ret;
}
