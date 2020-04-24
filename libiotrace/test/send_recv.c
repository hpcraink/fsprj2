#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <string.h>
#include <assert.h>
#include <fcntl.h>
#include <unistd.h>
#include <errno.h>
#include <linux/kcmp.h>

#define MY_SOCK_PATH "./libiotrace_test_socket"

static int kcmp(pid_t pid1, pid_t pid2, int type, unsigned long idx1,
		unsigned long idx2) {
	return syscall(SYS_kcmp, pid1, pid2, type, idx1, idx2);
}

int main(void) {
	pid_t pid = getpid();
	int fd;
	int fd2;
	int send_fd;
	int recv_fd;
	int accept_fd;
	int ret;
	socklen_t addr_len;
	struct sockaddr_un addr;
	struct sockaddr_un accept_addr;
	struct msghdr msg = { 0 };
#ifdef _GNU_SOURCE
	struct mmsghdr mmsg[2] = { 0 };
#endif
	struct cmsghdr *cmsg;
	int myfds[1];
	int myfds2[2];
	int *fdptr;
	char iobuf[1];
	struct iovec io = { .iov_base = iobuf, .iov_len = sizeof(iobuf) };
	union {
		char buf[CMSG_SPACE(sizeof(myfds))];
		struct cmsghdr align;
	} u = { .align = 0 };
	union {
		char buf[CMSG_SPACE(sizeof(myfds2))];
		struct cmsghdr align;
	} u2 = { .align = 0 };

	iobuf[0] = '\0';

	fd = open("/etc/passwd", O_RDONLY);
	assert(0 <= fd);

	send_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	assert(0 <= send_fd);
	recv_fd = socket(AF_UNIX, SOCK_STREAM, 0);
	assert(0 <= recv_fd);

	memset(&addr, 0, sizeof(struct sockaddr_un));
	addr.sun_family = AF_UNIX;
	strncpy(addr.sun_path, MY_SOCK_PATH, sizeof(addr.sun_path) - 1);

	ret = bind(recv_fd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un));
	assert(0 <= ret);

	ret = listen(recv_fd, 1);
	assert(0 == ret);

	ret = connect(send_fd, (struct sockaddr*) &addr,
			sizeof(struct sockaddr_un));
	assert(0 <= ret);

	myfds[0] = fd;

	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = u.buf;
	msg.msg_controllen = sizeof(u.buf);
	cmsg = CMSG_FIRSTHDR(&msg);
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(myfds));
	fdptr = (int*) CMSG_DATA(cmsg);
	memcpy(fdptr, myfds, sizeof(myfds));

	ret = sendmsg(send_fd, &msg, 0);
	assert(sizeof(iobuf) == ret);

	addr_len = sizeof(accept_addr);
	accept_fd = accept(recv_fd, (struct sockaddr*) &accept_addr, &addr_len);

	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &io;
	msg.msg_iovlen = 1;
	msg.msg_control = u.buf;
	msg.msg_controllen = sizeof(u.buf);

	ret = recvmsg(accept_fd, &msg, 0);
	assert(sizeof(iobuf) == ret);

	cmsg = CMSG_FIRSTHDR(&msg);
	assert(cmsg->cmsg_level == SOL_SOCKET);
	assert(cmsg->cmsg_type == SCM_RIGHTS);
	assert(cmsg->cmsg_len - CMSG_LEN(0) == sizeof(myfds));
	fd2 = *((int*) CMSG_DATA(cmsg));

	ret = kcmp(pid, pid, KCMP_FILE, fd, fd2);
	assert(0 == ret);

#ifdef _GNU_SOURCE
	myfds[0] = fd;

	mmsg[0].msg_hdr.msg_iov = &io;
	mmsg[0].msg_hdr.msg_iovlen = 1;
	mmsg[0].msg_hdr.msg_control = u.buf;
	mmsg[0].msg_hdr.msg_controllen = sizeof(u.buf);
	cmsg = CMSG_FIRSTHDR(&(mmsg[0].msg_hdr));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(myfds));
	fdptr = (int*) CMSG_DATA(cmsg);
	memcpy(fdptr, myfds, sizeof(myfds));

	myfds2[0] = fd;
	myfds2[1] = fd2;

	mmsg[1].msg_hdr.msg_iov = &io;
	mmsg[1].msg_hdr.msg_iovlen = 1;
	mmsg[1].msg_hdr.msg_control = u2.buf;
	mmsg[1].msg_hdr.msg_controllen = sizeof(u2.buf);
	cmsg = CMSG_FIRSTHDR(&(mmsg[1].msg_hdr));
	cmsg->cmsg_level = SOL_SOCKET;
	cmsg->cmsg_type = SCM_RIGHTS;
	cmsg->cmsg_len = CMSG_LEN(sizeof(myfds2));
	fdptr = (int*) CMSG_DATA(cmsg);
	memcpy(fdptr, myfds2, sizeof(myfds2));

	ret = sendmmsg(send_fd, mmsg, 2, 0);
	assert(2 == ret);

	mmsg[0].msg_hdr.msg_name = NULL;
	mmsg[0].msg_hdr.msg_namelen = 0;
	mmsg[0].msg_hdr.msg_iov = &io;
	mmsg[0].msg_hdr.msg_iovlen = 1;
	mmsg[0].msg_hdr.msg_control = u.buf;
	mmsg[0].msg_hdr.msg_controllen = sizeof(u.buf);

	mmsg[1].msg_hdr.msg_name = NULL;
	mmsg[1].msg_hdr.msg_namelen = 0;
	mmsg[1].msg_hdr.msg_iov = &io;
	mmsg[1].msg_hdr.msg_iovlen = 1;
	mmsg[1].msg_hdr.msg_control = u2.buf;
	mmsg[1].msg_hdr.msg_controllen = sizeof(u2.buf);

	ret = recvmmsg(accept_fd, mmsg, 2, 0, NULL);
	assert(2 == ret);
#endif

	ret = close(fd);
	assert(0 == ret);
	ret = close(fd2);
	assert(0 == ret);

	ret = close(send_fd);
	assert(0 == ret);
	ret = close(recv_fd);
	assert(0 == ret);
	ret = close(accept_fd);
	assert(0 == ret);

	unlink(MY_SOCK_PATH);
}
