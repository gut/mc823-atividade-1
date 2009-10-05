#ifndef UTIL_H
#define UTIL_H

#define MAXLINE         4096
#define MAXDATASIZE     1024

enum {
    CONNCLOSED = -2,
    READERROR = -1,
    ALLDATARECVD,
    RECEIVING
};

int readall(int fd, char *read, size_t maxlen);

int writeall(int fd, const char *buf, size_t count);

#endif
