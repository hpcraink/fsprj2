#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>

int main(void) {
	struct stat sb;
	int ret;
	int fd;

	ret = stat("/etc/passwd", &sb);
	assert(-1 != ret);

    fd = open("/etc/passwd", O_RDONLY);
    assert (0 <= fd);

    ret = fstat(fd, &sb);
    assert(-1 != ret);

    ret = lstat("/etc/passwd", &sb);
    assert(-1 != ret);

    ret = fstatat(fd, "", &sb, AT_EMPTY_PATH);
    assert(-1 != ret);
 
    ret = close (fd);

    return ret;
}
