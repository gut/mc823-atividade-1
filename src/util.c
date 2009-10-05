#include <stdio.h>
#include <unistd.h>
#include "util.h"

/*
 * Envia todos os dados do buffer 'buf' para socket descriptor sd.
 * Retorna -1 em erro ou numero total de bytes enviados.
 */
int
writeall(int sd, const char *buf, size_t len)
{
    int total = len;
    int n;
    char *end = (char *)buf;

    while (len) {
        n = write(sd, end, len);
        if (n < 0) {
            perror("write");
            return n;
        }
        end += n;
        len -= n;
    }

    return total - len;

}

/*
 * Le todos os dados enviados ateh receber um
 * '\0' ou encher o buffer com 'len' caracteres.
 * Retorna -1 se erro, 0 se encerrou a leitura ou
 * 1 caso tenha lido maxlen caracteres mas nao tenha
 * terminado de ler todos os bytes do buffer
 */
int
readall(int sd, char *buf, size_t maxlen)
{
    int n;
    char *end = buf;

    if (!buf) {
        fprintf(stderr, "Can not write on not malloc'ed buffer\n");
        return -1;
    }

    do {
        n = read(sd, end++, 1);
        if (n < 0) {
            perror("read");
            return -1;
        }
        maxlen--;
    } while(*(end - 1) && maxlen && n);

    /* Quem escrevia encerrou conexao */
    if (!n)
        return -2;

    /* Todos os dados foram recebidos */
    if (!*(end - 1))
        return 0;

    return 1;
}
