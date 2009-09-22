#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <unistd.h>
#include "wrapper.h"

int
received_quit_cmd(const char *cmd)
{
    if (!strncmp(cmd, "quit", sizeof("quit") - 1) &&
        cmd[sizeof("quit")] == '\0')
        return 1;
    else if (!strncmp(cmd, "exit", sizeof("exit") - 1) &&
        cmd[sizeof("exit")] == '\0')
        return 1;
    else if (!strncmp(cmd, "bye", sizeof("bye") - 1) &&
        cmd[sizeof("bye")] == '\0')
        return 1;
    else if (!strncmp(cmd, "sair", sizeof("sair") - 1) &&
        cmd[sizeof("sair")] == '\0')
        return 1;
    else if (!strncmp(cmd, "adios", sizeof("adios") - 1) &&
        cmd[sizeof("adios")] == '\0')
        return 1;
    else if (!strncmp(cmd, "salir", sizeof("salir") - 1) &&
        cmd[sizeof("salir")] == '\0')
        return 1;
    else if (!strncmp(cmd, "tchau", sizeof("tchau") - 1) &&
        cmd[sizeof("tchau")] == '\0')
        return 1;
    else
        return 0;
}

int
main(int argc, char **argv)
{
    int sockfd;
    char sendline[MAXLINE];
    char recvline[MAXLINE];
    char error[MAXLINE];
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
    fprintf(stdout, "Conexão local: %d.%d.%d.%d:%d\nConexão remota: %d.%d.%d.%d:%d\n",
            GETIP(local.sin_addr.s_addr), ntohs(local.sin_port),
			GETIP(servaddr.sin_addr.s_addr), ntohs(servaddr.sin_port));

    /* Obtem comando do usuario */
    while (1) {
        fputs("Digite comando: ", stdout);
        fgets(sendline, sizeof(sendline) - 1, stdin);

        if (received_quit_cmd(sendline))
            break;

        /* Envia o comando */
        Write(sockfd, sendline);

        /* 0btem resposta do servidor */
        bzero(&recvline, sizeof(recvline));
        Read(sockfd, recvline, sizeof(recvline) - 1);
        fprintf(stdout, "resposta servidor: %s", recvline);
    }
    /* Fecha conexao */
    close(sockfd);

    exit(EXIT_SUCCESS);
}
