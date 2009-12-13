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

int process_tcp(struct sockaddr_in *);

int process_udp(struct sockaddr_in *);

static void process_request(int, const char*, const char*);

static void sigchld_handler(int);

int
main(int argc, char **argv)
{
    int pid;
    struct sockaddr_in servaddr;
    char error[LINE_MAX + 1];

    if (argc != 3) {
        snprintf(error, LINE_MAX, "uso: %s <Porta TCP> <Porta UDP>\n", argv[0]);
        fprintf(stderr, error);
        exit(EXIT_FAILURE);
    }

    /* Contrucao do endereco Internet do servidor */
    /* servaddr struct zerada */
    bzero(&servaddr, sizeof(servaddr));
    /* Este eh um endereco de Internet */
    servaddr.sin_family      = AF_INET;
    /* Deixe o sistema descobrir nosso IP */
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

    pid = fork();
    if (pid == 0) {  /* tcp */
        /* Esta serah a porta na qual escutaremos tcp */
        servaddr.sin_port = htons(atoi(argv[1]));
        return process_tcp(&servaddr);
    }
    else { /* udp */
        /* Porta para o udp*/
        servaddr.sin_port = htons(atoi(argv[2]));
        return process_udp(&servaddr);
    }
}

int process_tcp(struct sockaddr_in *servaddr) {
    int listenfd, connfd, pid;
    struct sockaddr_in clientaddr;
    char host[NI_MAXHOST], hp[NI_MAXSERV];
    socklen_t len;

    /* Cria socket pai.
     * "listenfd" serah o file descriptor usado
     * para operar com o socket */
    listenfd = Socket(AF_INET, SOCK_STREAM, 0);

    /* Associa o socket pai com uma porta */
    Bind(listenfd, servaddr, sizeof(*servaddr));

    /* Deixa esse socket preparado para aceitar pedidos de conexao */
    Listen(listenfd, LISTENQ);

    /*
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

int process_udp(struct sockaddr_in *servaddr) {
    int localfd, ret;
    struct sockaddr_in clientaddr;
    char host[NI_MAXHOST], hp[NI_MAXSERV];
    char buf[LINE_MAX];
    socklen_t len = sizeof(clientaddr);

    /* Cria socket pai.
     * "localfd" serah o file descriptor usado
     * para operar com o socket */
    localfd = Socket(AF_INET, SOCK_DGRAM, 0);

    /* Associa o socket pai com uma porta */
    Bind(localfd, servaddr, sizeof(*servaddr));

    for ( ; ; ) {
        /* Espera por um pedido de conexao */
        ret = Recvfrom(localfd, buf, LINE_MAX, 0,
                (struct sockaddr *)&clientaddr, &len);

        /* Determina quem enviou a mensagem */
        Getnameinfo(&clientaddr, len, host, sizeof(host), hp, sizeof(hp));

        /* Imprime cliente e sua mensagem */
        fprintf(stdout, "UDP de %s:%s - %s", host, hp, buf);
        /* Mantendo algum \n no final da string */
        if (buf[strlen(buf)-1] != '\n')
            fputc('\n', stdout);

        /* Devolve a mensagem imediatamente */
        ret = Sendto(localfd, buf, strlen(buf), 0,
                (struct sockaddr *)&clientaddr, len);
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
        if (!Readline(connfd, buf, MAXDATASIZE))
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
 * Handler responsavel por finalizar corretamente
 * os servidores-filhos
 */
static void sigchld_handler(int signal)
{
    /* Espera todos os filhos terminarem, sem bloquear */
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
