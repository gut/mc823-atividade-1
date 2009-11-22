#ifndef WRAPPER_H
#define WRAPPER_H

#include <sys/select.h>

/* Obtem IP na forma decimal
 * Recebe um unsigned long s_addr;
 */
#define GETIP(s_addr) \
    ((unsigned char *)&s_addr)[0], \
    ((unsigned char *)&s_addr)[1], \
    ((unsigned char *)&s_addr)[2], \
    ((unsigned char *)&s_addr)[3]

/* Funcoes "envelopadoras" comuns para sockets */

int Socket(int domain, int type, int protocol);

void Bind(int fd, const struct sockaddr_in *addr, socklen_t addr_len);

void Listen(int fd, int queue);

int Accept(int fd, struct sockaddr_in *addr, socklen_t *len);

void Getnameinfo(const struct sockaddr_in *addr, socklen_t addr_len,
                 char *host, size_t hostlen, char *serv, size_t servlen);

void Connect(int fd, const struct sockaddr_in *addr, socklen_t len);

void Inet_pton(int af, const char *src, void *dst);

void Getsockname(int fd, struct sockaddr_in *addr, socklen_t *len);

int Select(int nfds, fd_set *, fd_set *, fd_set *, struct timeval *);

#endif // WRAPPER_H
