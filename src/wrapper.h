#ifndef WRAPPER_H
#define WRAPPER_H

#define MAXLINE 4096

/* Obtem IP na forma decimal 
 * Recebe um unsigned long s_addr;
 */
#define GETIP(s_addr) \
    ((unsigned char *)&s_addr)[0], \
    ((unsigned char *)&s_addr)[1], \
    ((unsigned char *)&s_addr)[2], \
    ((unsigned char *)&s_addr)[3]

/* Funções "envelopadoras" comuns */

int Socket(int domain, int type, int protocol);

void Bind(int fd, const struct sockaddr_in *addr, socklen_t addr_len);

void Listen(int fd, int queue);

int Accept(int fd, struct sockaddr_in *addr, socklen_t *len);

void Getnameinfo(const struct sockaddr_in *addr, socklen_t addr_len,
                 char *host, size_t hostlen, char *serv, size_t servlen);

int Read(int fd, char *buf, size_t count);

void Write(int fd, const char *buf);

void Connect(int fd, const struct sockaddr_in *addr, socklen_t len);

void Inet_pton(int af, const char *src, void *dst);

void Getsockname(int fd, struct sockaddr_in *addr, socklen_t *len);

void System(const char *command);

#endif // WRAPPER_H
