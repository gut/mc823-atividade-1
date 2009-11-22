#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include "wrapper.h"
#include "util.h"

int
main(int argc, char **argv)
{
    int sockfd, maxfd;
    fd_set sread;
    char recvline[LINE_MAX];
    char sendline[LINE_MAX];
    char error[LINE_MAX];
    struct sockaddr_in servaddr;
    struct sockaddr_in local;
    socklen_t len;

    if (argc != 3) {
        snprintf(error, sizeof(error) - 1,
                 "uso: %s <IPaddress> <Port>", argv[0]);
        perror(error);
        exit(EXIT_FAILURE);
    }

    /* Cria o socket */
    sockfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* Constroi o endereco de internet do servidor */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    /* Converte string em uma struct de endereco de internet */
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    /* Cria a conexao com o servidor */
    Connect(sockfd, &servaddr, sizeof(servaddr));

    /* Obtem IP e porta do socket local */
    len = sizeof(local);
    Getsockname(sockfd, &local, &len);

    /* Inicializa os fd_sets */
    FD_ZERO(&sread);

    /* Diz para stdin bufferizar de linha em linha */
    setvbuf(stdin, NULL, _IOLBF, LINE_MAX);

    int stdineof = 0;
    while (1) {
        /* Inserindo fd's nos fd_sets apropriados */
        if (!stdineof)
            FD_SET(fileno(stdin), &sread);
        FD_SET(sockfd, &sread);

        maxfd = MAX(sockfd, fileno(stdin)) + 1;
        Select(maxfd, &sread, NULL, NULL, NULL);

        /* Obtem resposta do servidor */
        if (FD_ISSET(sockfd, &sread)) {
            fprintf(stderr, "Server has response for me: \n");
            if (!Readline(sockfd, recvline, LINE_MAX)) {
                if (stdineof) {
                    fprintf(stderr, "Zero read\n");
                    break;
                }
                else {
                    fprintf(stderr, "server terminated prematurely\n");
                    exit(EXIT_FAILURE);
                }
            }
            fprintf(stderr, recvline);
            fputs(recvline, stdout);
        }

        /* Le dados da entrada padrao */
        if (FD_ISSET(fileno(stdin), &sread)) {
            fprintf(stderr, "Stdin data\n");
            if (fgets(sendline, LINE_MAX, stdin) == NULL) {
                fprintf(stderr, "EOF from stdin\n");
                stdineof = 1;
                shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(stdin), &sread);
                continue;
            }
            Writeall(sockfd, sendline, strlen(sendline));
        }
    }

    /* Fecha conexao */
    close(sockfd);

    exit(EXIT_SUCCESS);
}
