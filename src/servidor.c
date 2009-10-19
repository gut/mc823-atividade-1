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
#define SLEEP_TIME 10

#define LOG(fmt, rest...)                       \
    do {                                        \
        FILE *log = fopen("server.log", "a");   \
        if (log) {                              \
            fprintf(log, fmt, ## rest);         \
            fclose(log);                        \
        }                                       \
    } while (0)

static void process_request(int, const char*, const char*);

static void sigchld_handler(int);

int
main(int argc, char **argv)
{
    int listenfd, connfd, pid, listenq;
    struct sockaddr_in servaddr;
    struct sockaddr_in clientaddr;
    char error[MAXLINE + 1];
    char host[NI_MAXHOST], hp[NI_MAXSERV];
    socklen_t len;
    time_t thetime;

    if (argc != 3) {
        snprintf(error, MAXLINE, "uso: %s <Port> <Backlog size>\n", argv[0]);
        fprintf(stderr, error);
        exit(EXIT_FAILURE);
    }

    /* Define o tamanho do backlog a ser usado no Listen */
    char *ptr = NULL;
    errno = 0;
    listenq = (int)strtol(argv[2], &ptr, 10);
    if (errno != 0) {
        perror("strtol");
        exit(EXIT_FAILURE);
    } else if (*ptr != '\0') {
        fprintf(stderr, "ERRO: backlog '%s' invalido.\n", argv[2]);
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
    Listen(listenfd, listenq);

    /* Funcao para lidar com filhos-zumbi */
    signal(SIGCHLD, sigchld_handler);

    /* Testando se eh possivel abrir arquivo de log */
    FILE *log = fopen("server.log", "a");
    if (!log)
        fprintf(stderr, "AVISO: Nao foi possivel abrir arquivo de log."
                " Nenhuma informacao serah logada.\n");
    fclose(log);

    /*
     * main loop: espere por um pedido de conexao, devolva saida do
     * comando enviado pelo cliente e feche a conexao
     */
    for ( ; ; ) {
        /* Espera por um pedido de conexao */
        len = sizeof(clientaddr);
        connfd = accept(listenfd, (struct sockaddr *)&clientaddr, &len);
        if (connfd < 0) {
            /* Algum sinal chegou no meio do accept. Ignore */
            if (errno == EINTR)
                continue;
            else
                perror("accept");
        }

        /* Determina quem enviou a mensagem */
        Getnameinfo(&clientaddr, len, host, sizeof(host), hp, sizeof(hp));
        time(&thetime);
        struct tm *t = localtime(&thetime);
        LOG("%s:%s conectado em %s", host, hp, asctime(t));

        pid = fork();
        if (pid == 0) {
            char *h = strdup(host);
            char *p = strdup(hp);

            /* Filho para de escutar conexoes */
            close(listenfd);
            process_request(connfd, h, p);

            fprintf(stdout, "%s:%s desconectado\n", h, p);
            time(&thetime);
            t = localtime(&thetime);
            LOG("%s:%s desconectado em %s", h, p, asctime(t));

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

        if (pclose(pipe) < 0 && errno != ECHILD)
            perror("pclose");
    }
    /* Filho encerra sua conexao */
    sleep(SLEEP_TIME);
    close(connfd);
}


static void sigchld_handler(int signal)
{
    while (waitpid(-1, NULL, WNOHANG) > 0);
}
