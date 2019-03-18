#include <stdio.h>
#include <assert.h>

int main(void) {
    FILE * file;
    char buffer;

    file = fopen("/etc/passwd", "r");
    assert (NULL != file);

    fread(&buffer, sizeof(char), 1, file);

    return fclose(file);
}
