#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define SOCKET_CLOSE_DELAY 15
#define MAXLINE 4096

/* Obtem IP na forma decimal */
#define GETIP(addr) \
    ((unsigned char *)&addr)[0], \
    ((unsigned char *)&addr)[1], \
    ((unsigned char *)&addr)[2], \
    ((unsigned char *)&addr)[3]

int main (int argc, char **argv) {
   int    listenfd, connfd;
   struct sockaddr_in servaddr;
   struct sockaddr_in clientaddr;
   socklen_t len;
   char   error[MAXLINE + 1];
   char   buf[MAXDATASIZE];
   char   recvline[MAXLINE + 1];
   time_t ticks;

   if (argc != 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }

   /* socket: cria socket pai.
    * "listenfd" serah o file descriptor usado
    * para operar com o socket */
   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
   }

   /* Contrucao do endereco Internet do servidor */
   /* servaddr struct zerada */
   bzero(&servaddr, sizeof(servaddr));
   /* Este eh um endereco de Internet */
   servaddr.sin_family      = AF_INET;
   /* Deixe o sistema descobrir nosso IP */
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   /* Esta serah a porta na qual escutaremos */
   servaddr.sin_port        = htons(atoi(argv[1]));

   /* bind: associa o socket pai com uma porta */
   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   }

   /* listen: deixa esse socket preparado to aceitar pedidos de conexao */
   if (listen(listenfd, LISTENQ) == -1) {
      perror("listen");
      exit(1);
   }

   /*
    * main loop: espere por um pedido de conexao, devolva a hora,
    * e feche a conexao
    */
   for ( ; ; ) {
      /* accept: espera por um pedido de conexao */
      len = sizeof(clientaddr);
      if ((connfd = accept(listenfd, (struct sockaddr *) &clientaddr, &len)) == -1 ) {
         perror("accept");
         exit(1);
      }
      fprintf(stdout, "Connection established with: %d.%d.%d.%d:%d\n",
              GETIP(clientaddr.sin_addr.s_addr), ntohs(clientaddr.sin_port));

      /* read: obtem a mensagem do cliente */
      fprintf(stdout, "Command to run:\n");

      while (fgets(recvline, MAXLINE, (FILE*)connfd)) {
         /* Imprime resposta obtida */
         if (fputs(recvline, stdout) == EOF) {
            perror("fputs error");
            exit(1);
         }
         bzero(&recvline, strlen(recvline));
      }
 
      if (recvline != NULL) {
         perror("read error");
         exit(1);
      }

      ticks = time(NULL);
      snprintf(buf, sizeof(buf), "%.24s\r\n", ctime(&ticks));
      /* write: devolve a data para o cliente */
      write(connfd, buf, strlen(buf));

      /* Apenas para o ex1: sleep(SOCKET_CLOSE_DELAY); */
      /* close: fecha a conexao */
      close(connfd);
   }
   return(0);
}
