#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include "util.h"

/*
 * Envia todos os dados para socket
 *
 * @param sd socket descriptor para o qual enviar
 * @param buf buffer de dados
 * @param len numero de bytes a enviar
 * @return -1 se erro ou len (numero bytes enviados).
 */
ssize_t
writeall(int sd, const void *buf, size_t len)
{
    size_t left;
    ssize_t written;
    const char *ptr = buf;

    left = len;
    while (left > 0) {
        written = write(sd, ptr, left);
        if (written <= 0) {
            if (errno == EINTR)
                written = 0;
            else
                return -1;
        }
        left -= written;
        ptr += written;
    }

    return len;
}

void
Writeall(int sd, const void *ptr, size_t len)
{
    if (writeall(sd, ptr, len) != len) {
        perror("Written error");
        exit(EXIT_FAILURE);
    }
}

/*
 * Le dados do socket ateh numero maximo de caracteres
 * ou receber um EOF.
 *
 * @param sd socket descriptor do qual ler
 * @param buf buffer onde armazenar dados lidos
 * @param len numero maximo de caracteres a ler
 * @return numero de caracteres lidos ou -1, em caso de erro
 */
ssize_t
readall(int sd, void *buf, size_t maxlen)
{
    int left;
    ssize_t nread;
    char *ptr = buf;

    left = maxlen;
    while (left > 0) {
        nread = read(sd, ptr, left);
        if (nread < 0) {
            if (errno == EINTR)
                nread = 0;
            else
                return -1;
        } else if (!nread) // EOF
            break;

        left -= nread;
        ptr += nread;
    }

    return (maxlen - left);
}

ssize_t
Readall(int sd, void *ptr, size_t len)
{
    ssize_t n = readall(sd, ptr, len);
    if (n < 0) {
        perror("readall");
        exit(EXIT_FAILURE);
    }
    return n;
}

ssize_t
readline(int sd, void *buf, size_t maxlen)
{
    ssize_t i, count;
    char c, *ptr;

    ptr = buf;
    for (i = 1; i < maxlen; i++) {
again:
        count = read(sd, &c, 1);
        if (count == 1) {
            *ptr++ = c;
            if (c == '\n')
                break;
        } else if (!count) {
            if (i == 1)
                return 0; // EOF, nada lido
            else
                break; // EOF, algo lido
        } else {
            if (errno == EINTR)
                goto again;
            return -1;
        }
    }

    /* Termina string */
    *ptr = '\0';

    return i;
}

ssize_t
Readline(int sd, void *ptr, size_t maxlen)
{
    ssize_t n = readline(sd, ptr, maxlen);
    if (n < 0) {
        perror("readline");
        exit(EXIT_FAILURE);
    }
    return n;
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
