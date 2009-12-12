#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "wrapper.h"

int
Socket(int domain, int type, int protocol)
{
    int listenfd;

    listenfd = socket(domain, type, protocol);
    if (listenfd == -1) {
        perror("socket");
        exit(EXIT_FAILURE);
    }
    return listenfd;
}

void
Bind(int listenfd, const struct sockaddr_in *servaddr, socklen_t len)
{
    if (bind(listenfd, (struct sockaddr *)servaddr, len) == -1) {
        perror("bind");
        exit(EXIT_FAILURE);
    }
}

void
Listen(int listenfd, int queue)
{
    if (listen(listenfd, queue) == -1) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

int
Accept(int listenfd, struct sockaddr_in *addr, socklen_t *len)
{
    int connfd;

    connfd = accept(listenfd, (struct sockaddr *)addr, len);
    if (connfd == -1) {
        perror("accept");
        exit(EXIT_FAILURE);
    }
    return connfd;
}

void
Connect(int fd, const struct sockaddr_in *addr, socklen_t len)
{
    if (connect(fd, (struct sockaddr *)addr, len) < 0) {
        perror("connect");
        exit(EXIT_FAILURE);
    }
}

void
Inet_pton(int af, const char *src, void *dst)
{
    if (inet_pton(af, src, dst) <= 0) {
        perror("inet_pton");
        exit(EXIT_FAILURE);
    }
}

void
Getsockname(int fd, struct sockaddr_in *addr, socklen_t *len)
{
    if (getsockname(fd, (struct sockaddr *)addr, len) < 0)
        perror("getsockname");
}

void
Getnameinfo(const struct sockaddr_in *addr, socklen_t len,
            char *host, size_t hlen, char *serv, size_t slen)
{
    int error;

    /* gethostbyaddr eh obsoleta. Usando getnameinfo */
    error = getnameinfo((const struct sockaddr *)addr, len, host, hlen,
                        serv, slen, NI_NUMERICHOST|NI_NUMERICSERV);
    if (error != 0)
        fprintf(stderr, "erro em getnameinfo: %s\n", gai_strerror(error));
}

int
Select(int nfds, fd_set *readfds, fd_set *writefds, fd_set *errorfds, struct timeval *timeout)
{
    int readysocks = select(nfds, readfds, writefds, errorfds, timeout);
    if (readysocks < 0) {
        perror("select");
        exit(EXIT_FAILURE);
    }
    return readysocks;
}

ssize_t
Sendto(int sd, const void *msg, size_t len, int flags,
       const struct sockaddr *dest, socklen_t dest_len)
{
    ssize_t sent = sendto(sd, msg, len, flags, dest, dest_len);
    if (sent < 0) {
        perror("sendto");
        exit(EXIT_FAILURE);
    }

    return sent;
}

ssize_t
Recvfrom(int sd, void *buf, size_t len, int flags, struct sockaddr *addr,
         socklen_t *addr_len)
{
    ssize_t recved = recvfrom(sd, buf, len, flags, addr, addr_len);
    if (recved < 0) {
        perror("recvfrom");
        exit(EXIT_FAILURE);
    }
    return recved;
}
