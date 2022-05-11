#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <netinet/tcp.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

#include <stdio.h>
#include <string.h>

int main(int argc, char *argv[])
{

    if (argc < 3)
    {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    //Configure remote address
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    struct addrinfo *peer_address;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address))
    {
        fprintf(stderr, "getaddrinfo() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    //Print remote address
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer),
                NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

    //Create
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family,
                         peer_address->ai_socktype, peer_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    //Set socket option TCP_NODELAY
    int option = 0;
    if (setsockopt(socket_peer, IPPROTO_TCP, TCP_NODELAY, (void *)&option, sizeof(option)))
    {
        fprintf(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    //Set socket option REUSEADDR
    int option2 = 0;
    if (setsockopt(socket_peer, SOL_SOCKET, SO_REUSEADDR, (void *)&option2, sizeof(option2)))
    {
        fprintf(stderr, "setsockopt() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }

    //Connect
    if (connect(socket_peer,
                peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "connect() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(peer_address);

    printf("Connected.\n");
    printf("Enter text to send\n");

    char message[250];
    char body[] = "some_metric 6.10\nsome_other_metric 7.14\n";
    
    snprintf(message, sizeof(message), "POST /metrics/job/some_job HTTP/1.1\nHost: localhost:9091\nAccept: */*\n"
    "Content-Length: %ld\nContent-Type: application/x-www-form-urlencoded\n\n%s", strlen(body), body);
    
    
    int bytes_sent = send(socket_peer, message, strlen(message), 0);
    printf("Sent %d bytes.\n", bytes_sent);


    CLOSESOCKET(socket_peer);

    printf("Finished.\n");
    return 0;
}
