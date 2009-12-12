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
    int sockfd, maxfd, use_udp = 0;
    fd_set sread;
    char recvline[LINE_MAX];
    char sendline[LINE_MAX];
    char error[LINE_MAX];
    struct sockaddr_in servaddr;
    struct sockaddr_in local;
    struct timeval timeout;
    socklen_t slen;

    if (argc != 4) {
        snprintf(error, sizeof(error) - 1,
                 "uso: %s <IPaddress> <Port> <Protocol>", argv[0]);
        perror(error);
        exit(EXIT_FAILURE);
    }

    use_udp = strcmp(argv[3], "udp") == 0;

    /* Cria o socket */
    sockfd = use_udp ? Socket(AF_INET, SOCK_DGRAM, 0) :
                       Socket(AF_INET, SOCK_STREAM, 0);

    /* Constroi o endereco de internet do servidor */
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(atoi(argv[2]));

    /* Converte string em uma struct de endereco de internet */
    Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

    if (use_udp) {
        bzero(&local, sizeof(local));
        local.sin_family = AF_INET;
        local.sin_port = htons(0);
        local.sin_addr.s_addr = htonl(INADDR_ANY);
        Bind(sockfd, (struct sockaddr *)&local, sizeof(local));
    } else {
        /* Cria a conexao tcp com o servidor */
        Connect(sockfd, &servaddr, sizeof(servaddr));
    }

    /* Inicializa os fd_sets */
    FD_ZERO(&sread);

    /* Diz para stdin bufferizar de linha em linha */
    setvbuf(stdin, NULL, _IOLBF, 0);//LINE_MAX);

    int stdineof = 0;
    int activity = 0;
    timeout.tv_sec = 30; // 10s
    timeout.tv_usec = 0;
    slen = sizeof(local);

    while (1) {
        /* Inserindo fd's nos fd_sets apropriados */
        activity = 0;
        if (!stdineof)
            FD_SET(fileno(stdin), &sread);
        FD_SET(sockfd, &sread);

        maxfd = MAX(sockfd, fileno(stdin)) + 1;
        Select(maxfd, &sread, NULL, NULL, &timeout);

        /* Obtem resposta do servidor */
        if (FD_ISSET(sockfd, &sread)) {
            activity = 1;
            int ret;
            if (use_udp) {
                ret = Recvfrom(sockfd, recvline, LINE_MAX, 0,
                               (struct sockaddr *)&local, &slen);
                if (local.sin_addr.s_addr != servaddr.sin_addr.s_addr ||
                    local.sin_port != servaddr.sin_port) {
                    printf("Not for me this message\n");
                    goto read;
                }
            }
            else
                ret = Readline(sockfd, recvline, LINE_MAX);
            if (!ret) {
                if (stdineof)
                    break;
                else {
                    fprintf(stderr, "server terminated prematurely\n");
                    exit(EXIT_FAILURE);
                }
            }
            fputs(recvline, stdout);
        }

read:
        /* Le dados da entrada padrao */
        if (FD_ISSET(fileno(stdin), &sread)) {
            activity = 1;
            if (fgets(sendline, LINE_MAX, stdin) == NULL) {
                stdineof = 1;
                if (!use_udp)
                    shutdown(sockfd, SHUT_WR);
                FD_CLR(fileno(stdin), &sread);
                continue;
            }
            use_udp ?
                Sendto(sockfd, sendline, strlen(sendline), 0,
                       (struct sockaddr *)&servaddr, sizeof(servaddr)) :
                Writeall(sockfd, sendline, strlen(sendline));
        }

        /* Timeout ? */
        if (!activity) {
            printf("Ooops, select timed out\n");
            break;
        }

    }

    /* Fecha conexao */
    close(sockfd);

    exit(EXIT_SUCCESS);
}
