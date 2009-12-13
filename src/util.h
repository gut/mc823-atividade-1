#ifndef UTIL_H
#define UTIL_H

#include <limits.h>
#include <sys/socket.h>

#define MAXDATASIZE     1024
#define MIN(A, B) ((A <= B) ? A : B)
#define MAX(A, B) ((A >= B) ? A : B)


ssize_t readall(int sd, void *read, size_t maxcount);

ssize_t Readall(int sd, void *read, size_t maxcount);

ssize_t writeall(int sd, const void *buf, size_t count);

void Writeall(int sd, const void *buf, size_t count);

ssize_t readline(int sd, void *buf, size_t maxlen);

ssize_t Readline(int sd, void *buf, size_t maxlen);

#endif
