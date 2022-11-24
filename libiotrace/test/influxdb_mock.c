#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <pthread.h>
#include <sys/select.h>
#include <assert.h>
#include <string.h>
#include <errno.h>

#define PORT 8086
#define BACKLOG_SIZE 200
#define READ_BUFFER 16 * 4096
#define LINE_BREAK "\r\n"
#define POST "POST /api/v2/write?bucket=hsebucket&precision=ns&org=hse HTTP/1.1"
#define RESPONSE "HTTP/1.1 204 No Content" LINE_BREAK LINE_BREAK LINE_BREAK

void send_data(const char *message, int socket) {
	size_t bytes_to_send = strlen(message);
	const char *message_to_send = message;

	while (bytes_to_send > 0) {
		int bytes_sent = send(socket, message_to_send, bytes_to_send, 0);

		if (-1 == bytes_sent) {
			if (errno == EWOULDBLOCK || errno == EAGAIN) {
				printf("Send buffer is full. Please increase your limit.");
			} else {
				printf("send() returned %d, errno: %d", bytes_sent, errno);
				exit(EXIT_FAILURE);
			}
		} else {
			if ((size_t) bytes_sent < bytes_to_send) {
				bytes_to_send -= bytes_sent;
				message_to_send += bytes_sent;
			} else {
				bytes_to_send = 0;
			}
		}
	}
}

void* process_connection(void *arg) {
	int client = *((int*) arg);
	fd_set readfds;
	size_t pos = 0;

	free(arg);

	printf("thread started, waiting for data on socket %d\n", client);
	fflush(stdout);

	while (1) {
		FD_ZERO(&readfds);
		FD_SET(client, &readfds);

		select(client + 1, &readfds, NULL, NULL, NULL);

		if (FD_ISSET(client, &readfds)) {
			char buffer[READ_BUFFER];
			ssize_t bytes_received = recv(client, buffer, sizeof(buffer), 0);
			if (1 > bytes_received) {
				close(client);
				break;
			} else {
				for (ssize_t i = 0; i < bytes_received; i++) {
					if (buffer[i] == POST[pos]) {
						pos++;
						if (pos == sizeof(POST) - 1) {
							send_data(RESPONSE, client);
//							printf("send response\n");
//							fflush(stdout);
							pos = 0;
						}
					} else {
						pos = 0;
					}
				}
			}

//			printf("read some data on socket %d\n", client);
//			fflush(stdout);
//			printf("%s\n", buffer);
//			fflush(stdout);
		}
	}

	printf("thread stopped for socket %d\n", client);
	fflush(stdout);

	return NULL;
}

int main(int argc __attribute__((unused)),
		char const *argv[] __attribute__((unused))) {
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_port = htons(PORT);
	addr.sin_addr.s_addr = INADDR_ANY;

//	printf("InfluxDB mock creating socket\n");
//	fflush(stdout);

	int sock = socket(AF_INET, SOCK_STREAM, 0);

	bind(sock, (struct sockaddr*) &addr, sizeof(addr));
	listen(sock, BACKLOG_SIZE);

	printf("InfluxDB mock listening on port %d\n", PORT);
	fflush(stdout);

	while (1) {
		int *client = malloc(sizeof(int));
		assert(NULL != client);
		*client = accept4(sock, NULL, NULL, SOCK_NONBLOCK);
		printf("connection accepted, starting thread for socket %d\n", *client);
		fflush(stdout);
		pthread_t thread;
		pthread_create(&thread, NULL, process_connection, client);
	}
}
