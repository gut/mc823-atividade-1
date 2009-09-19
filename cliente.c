#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

/* Obtem IP na forma decimal */
#define GETIP(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

int main(int argc, char **argv) {
   int    sockfd, n;
   char   recvline[MAXLINE + 1];
   char   error[MAXLINE + 1];
   struct sockaddr_in servaddr;
   struct sockaddr_in local;
   socklen_t len;

   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress> <Port>");
      perror(error);
      exit(1);
   }

   /* socket: cria o socket */
   if ( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
      perror("socket error");
      exit(1);
   }

   /* Constroi o endereco de internet do servidor */
   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(atoi(argv[2]));
   /* Converte string em uma struct de endereco de internet */
   if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) {
      perror("inet_pton error");
      exit(1);
   }

   /* connect: cria a conexao com o servidor */
   if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) < 0) {
      perror("connect error");
      exit(1);
   }

   /* getsockname: obtem IP e porta do socket local */
   len = sizeof(local);
   if (getsockname(sockfd, (struct sockaddr *)&local, &len) < 0) {
       perror("getsockname error");
       exit(1);
   }
   fprintf(stdout, "Local address: %d.%d.%d.%d:%d\n",
           GETIP(local.sin_addr.s_addr), ntohs(local.sin_port));


   /* read: obtem resposta do servidor */
   while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
      recvline[n] = 0;
      /* Imprime resposta obtida */
      if (fputs(recvline, stdout) == EOF) {
         perror("fputs error");
         exit(1);
      }
   }

   if (n < 0) {
      perror("read error");
      exit(1);
   }

   exit(0);
}
