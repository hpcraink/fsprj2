#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <dirent.h>

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

    ret = fstatat(AT_FDCWD, "/etc/passwd", &sb, 0);
    assert(-1 != ret);

    DIR *dir = opendir("/etc");
    assert(NULL != dir);

    ret = fstatat(dirfd(dir), "./passwd", &sb, 0);
    assert(-1 != ret);
 
    ret = close (fd);

    ret = stat("/etc/file_does_not_exist_at_all", &sb);
    assert(-1 == ret);

    ret = fstat(fd, &sb);
    assert(-1 == ret);

    ret = lstat("/etc/file_does_not_exist_at_all", &sb);
    assert(-1 == ret);

    ret = fstatat(fd, "", &sb, AT_EMPTY_PATH);
    assert(-1 == ret);

    return ret;
}
