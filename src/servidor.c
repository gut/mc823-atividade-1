#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include "wrapper.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV 32
#endif
#define LISTENQ 10
#define MAXDATASIZE 1024

static void process_request(int);

int
main(int argc, char **argv)
{
    int listenfd, connfd, pid;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    char error[MAXLINE + 1];
    char host[NI_MAXHOST], hp[NI_MAXSERV];
    socklen_t len;

    if (argc != 2) {
        snprintf(error, MAXLINE, "uso: %s <Port>", argv[0]);
        perror(error);
        exit(EXIT_FAILURE);
    }

    /* Cria socket pai.
     * "listenfd" serah o file descriptor usado
     * para operar com o socket */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* Contrucao do endereco Internet do servidor */
    /* servaddr struct zerada */
    bzero(&servaddr, sizeof(servaddr));
    /* Este eh um endereco de Internet */
    servaddr.sin_family      = AF_INET;
    /* Deixe o sistema descobrir nosso IP */
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    /* Esta serah a porta na qual escutaremos */
    servaddr.sin_port        = htons(atoi(argv[1]));

    /* Associa o socket pai com uma porta */
    Bind(listenfd, &servaddr, sizeof(servaddr));

    /* Deixa esse socket preparado para aceitar pedidos de conexao */
    Listen(listenfd, LISTENQ);

    /*
     * main loop: espere por um pedido de conexao, devolva o comando
     * enviado pelo cliente, execute o comando e feche a conexao
     */
    for ( ; ; ) {
        /* Espera por um pedido de conexao */
        len = sizeof(clientaddr);
        connfd = Accept(listenfd, &clientaddr, &len);

        /* Determina quem enviou a mensagem */
        Getnameinfo(&clientaddr, len, host, sizeof(host), hp, sizeof(hp));
        fprintf(stdout, "conexao estabelecida com %s:%s\n", host, hp);

        pid = fork();
        if (pid == 0) {
            /* Filho para de escutar conexoes */
            close(listenfd);
            process_request(connfd);

            /* Filho encerra sua conexao */
            close(connfd);
            exit(EXIT_SUCCESS);
        }
        /* Pai fecha a conexao */
        close(connfd);
    }

    return 0;
}

static void
process_request(int connfd)
{
    char buf[MAXDATASIZE];

    while (1) {
        bzero(buf, sizeof(buf));
        /* Le comando do cliente */
        if (Read(connfd, buf, sizeof(buf)) == 0)
            break;
        /* Devolve o comando para o cliente */
        Write(connfd, buf);
        /* Executa o comando recebido */
        System(buf);
    }
}
