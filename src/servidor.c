#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <netdb.h>
#include <unistd.h>
#include <time.h>
#include <sys/wait.h>
#include <signal.h>
#include <errno.h>
#include "wrapper.h"
#include "util.h"

#ifndef NI_MAXHOST
#define NI_MAXHOST 1025
#endif

#ifndef NI_MAXSERV
#define NI_MAXSERV 32
#endif

#ifndef LISTENQ
#define LISTENQ 10
#endif

static void process_request(int, const char*, const char*);

static void sigchld_handler(int);

int
main(int argc, char **argv)
{
    int listenfd, connfd, pid;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    char error[LINE_MAX + 1];
    char host[NI_MAXHOST], hp[NI_MAXSERV];
    socklen_t len;

    if (argc != 2) {
        snprintf(error, LINE_MAX, "uso: %s <Port>\n", argv[0]);
        fprintf(stderr, error);
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
     * ## Modificado da atividade anterior ##
     * Funcao para lidar corretamente com processos-filho para
     * que nao se tornem processos-zumbi
     */
    signal(SIGCHLD, sigchld_handler);

    /*
     * main loop: espere por um pedido de conexao, devolva saida do
     * comando enviado pelo cliente e feche a conexao
     */
    for ( ; ; ) {
        /* Espera por um pedido de conexao */
        len = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
        if (connfd < 0) {

            /*
             * ## Modificado da atividade anterior ##
             * Algum sinal chegou no meio do accept. Ignore
             */
            if (errno == EINTR)
                continue;
            else
                perror("accept");
        }

        /* Determina quem enviou a mensagem */
        Getnameinfo(&clientaddr, len, host, sizeof(host), hp, sizeof(hp));

        pid = fork();
        if (pid == 0) {
            char *h = strdup(host);
            char *p = strdup(hp);

            /* Filho para de escutar conexoes */
            close(listenfd);
            process_request(connfd, h, p);

            fprintf(stdout, "%s:%s desconectado\n", h, p);

            free(h);
            free(p);

            exit(EXIT_SUCCESS);
        }
        /* Pai fecha a conexao */
        close(connfd);
    }

    return 0;
}

static void
process_request(int connfd, const char *host, const char *port)
{
    char buf[MAXDATASIZE];
    int len;

    while (1) {
        /* Le comando do cliente */
        if (Readline(connfd, buf, MAXDATASIZE) < 0)
            break;

        /* Imprime cliente e seu comando a ser executado */
        fprintf(stdout, "%s:%s - %s", host, port, buf);
		/* Mantendo algum \n no final da string */
		if (buf[strlen(buf)-1] != '\n')
			fputc('\n', stdout);

        /* Devolve saida do comando para o cliente */
        len = writeall(connfd, buf, strlen(buf));
        if (len != strlen(buf)) {
            perror("writeall");
            break;
        }
    }
    /* Filho encerra sua conexao */
    close(connfd);
}

/*
 * ## Modificado da atividade anterior ##
 * Handler responsavel por finalizar corretamente
 * os servidores-filhos
 */
static void sigchld_handler(int signal)
{
    /* Espera todos os filhos terminarem, sem bloquear */
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
