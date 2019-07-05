#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

#define BUFFER_SIZE 256

int main()
{
  unsigned short port = 9875;
  int srcSocket;
  int dstSocket;

  struct sockaddr_in srcAddr;
  struct sockaddr_in dstAddr;
  int dstAddrSize = sizeof(dstAddr);

  int numrcv;
  char buffer[BUFFER_SIZE];
  char *toSendText;
  int i=0,sum=0;
  char tax[BUFFER_SIZE];

  memset(&srcAddr,0,sizeof(srcAddr));
  srcAddr.sin_port = htons(port);
  srcAddr.sin_family = AF_INET;
  srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);

  srcSocket = socket(AF_INET,SOCK_STREAM,0);

  bind(srcSocket,(struct sockaddr *)&srcAddr,sizeof(srcAddr));

  listen(srcSocket,1);

  printf("Waiting for connenction ...\n");
  dstSocket = accept(srcSocket,(struct sockaddr *)&dstAddr,&dstAddrSize);
  printf("Connected from %s\n",inet_ntoa(dstAddr.sin_addr));

  while(1) {
    numrcv = recv(dstSocket,buffer,BUFFER_SIZE,0);
    if (numrcv == 0 || numrcv == -1) {
      close(dstSocket);
      break;
    }
    i = atoi(buffer);
    sum = i + 1;
    printf("received: %d\n",sum);
    snprintf(tax,BUFFER_SIZE,"%d",sum);
    toSendText = tax;
    printf("%c\n",*toSendText);
    write(dstSocket,toSendText,BUFFER_SIZE);
    //send(dstSocket,toSendText,strlen(toSendText)+1,0);
  }
  return 0;
}
