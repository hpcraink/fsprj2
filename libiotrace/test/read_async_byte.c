#include <aio.h>
#include <fcntl.h>
#include <assert.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>

#define BUFFER_SIZE 50

int main(void) {
    int fd;
    int ret;
    struct aiocb aiocb1;
    struct aiocb aiocb2;
    struct aiocb aiocb3;
    struct sigevent aio1_sigevent;
    struct aiocb * list[4];
    struct timespec timeout;
    volatile char buffer1[BUFFER_SIZE];
    volatile char buffer2[BUFFER_SIZE / 2];

    fd = open("/etc/passwd", O_RDONLY);
    assert(0 <= fd);

    aiocb1.aio_fildes = fd;
    aiocb1.aio_buf = &buffer1;
    aiocb1.aio_nbytes = BUFFER_SIZE;
    aiocb1.aio_offset = 0;
    aiocb1.aio_lio_opcode = LIO_READ;
    aiocb1.aio_reqprio = 0;
    aio1_sigevent.sigev_notify = SIGEV_NONE;
    aiocb1.aio_sigevent = aio1_sigevent;

    aiocb2.aio_fildes = fd;
    aiocb2.aio_buf = &buffer2;
    aiocb2.aio_nbytes = BUFFER_SIZE / 2;
    aiocb2.aio_offset = 1;
    aiocb2.aio_lio_opcode = LIO_READ;
    aiocb2.aio_reqprio = 0;
    aiocb2.aio_sigevent = aio1_sigevent;

    aiocb3.aio_fildes = fd;
    aiocb3.aio_buf = &buffer1;
    aiocb3.aio_nbytes = BUFFER_SIZE;
    aiocb3.aio_offset = 0;
    aiocb3.aio_lio_opcode = LIO_NOP;
    aiocb3.aio_reqprio = 0;
    aiocb3.aio_sigevent = aio1_sigevent;

    aio_read(&aiocb1);

    while (1) {
        ret = aio_error(&aiocb1);
        if (EINPROGRESS != ret) {
            break;
        }
    }

    ret = aio_return(&aiocb1);

    list[0] = &aiocb1;
    list[1] = NULL;
    list[2] = &aiocb2;
    list[3] = &aiocb3;
    lio_listio(LIO_WAIT, list, 4, &aio1_sigevent);

    timeout.tv_sec = 1;
    timeout.tv_nsec = 0;
    const struct aiocb * const clist[4] = { &aiocb1, NULL, &aiocb2, &aiocb3 };
    aio_suspend(clist, 4, &timeout);

    aio_cancel(fd, NULL);

    close(fd);

    return ret;
}
