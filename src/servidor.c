#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include "wrapper.h"
#include "util.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif
#ifndef NI_MAXSERV
#define NI_MAXSERV 32
#endif
#define LISTENQ 10

static void process_request(int, const char*, const char*);

int
main(int argc, char **argv)
{
    int listenfd, connfd, pid;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    char error[MAXLINE + 1];
    char host[NI_MAXHOST], hp[NI_MAXSERV];
    socklen_t len;
    time_t thetime;
    FILE *log;

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

    log = fopen("server.log", "w+");
    if (!log) {
        fprintf(stderr, "Could not open log file\n");
        exit(EXIT_FAILURE);
    }

    /*
     * main loop: espere por um pedido de conexao, devolva saida do
     * comando enviado pelo cliente e feche a conexao
     */
    for ( ; ; ) {
        /* Espera por um pedido de conexao */
        len = sizeof(clientaddr);
        connfd = Accept(listenfd, &clientaddr, &len);

        /* Determina quem enviou a mensagem */
        Getnameinfo(&clientaddr, len, host, sizeof(host), hp, sizeof(hp));
        time(&thetime);
        struct tm *t = localtime(&thetime);
        fprintf(log, "%s:%s conectado em %s", host, hp, asctime(t));

        pid = fork();
        if (pid == 0) {
            char *h = strdup(host);
            char *p = strdup(hp);

            /* Filho para de escutar conexoes */
            close(listenfd);
            process_request(connfd, host, hp);

            /* Filho encerra sua conexao */
            close(connfd);

            fprintf(stdout, "%s:%s desconectado\n", h, p);
            time(&thetime);
            t = localtime(&thetime);
            fprintf(log, "%s:%s desconectado em %s", h, p, asctime(t));

            exit(EXIT_SUCCESS);
            free(h);
            free(p);
        }
        /* Pai fecha a conexao */
        close(connfd);
    }
    fclose(log);

    return 0;
}

static void
process_request(int connfd, const char *host, const char *port)
{
    char buf[MAXDATASIZE], out[MAXDATASIZE];
    FILE *pipe = NULL;
    int len;

    while (1) {
        /* Le comando do cliente */
        if (readall(connfd, buf, MAXDATASIZE) < 0)
            break;

        /* Imprime cliente e seu comando a ser executado */
        fprintf(stdout, "%s:%s - %s\n", host, port, buf);

        /* Executa o comando recebido */
        pipe = popen(buf, "r");
        if (pipe == NULL) {
            perror("popen");
            continue;
        }

        /* Devolve saida do comando para o cliente */
        while (!feof(pipe)) {
            if (fgets(out, sizeof(out), pipe) != NULL) {
                len = writeall(connfd, out, strlen(out));
                if (len != strlen(out)) {
                    perror("writeall");
                    break;
                }
            }
        }
        /* Diz para cliente que nao ha mais dados */
        len = write(connfd, "\0", 1);
        if (len != 1)
            perror("write");

        if (pclose(pipe) < 0)
            perror("pclose");
    }
}
