#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>

#define PORT 9876

int main() {
  int sockOfLiten = socket(AF_INET,SOCK_STREAM,0);
  char buffer[256];
  int a;
  
  struct sockaddr_in addrOfSv;
  memset(&addrOfSv,0,sizeof(addrOfSv));
  addrOfSv.sin_family = AF_INET;
  addrOfSv.sin_port = htons(PORT);
  addrOfSv.sin_addr.s_addr = htonl(INADDR_ANY);
  bind(sockOfLiten,(struct sockaddr *) &addrOfSv,sizeof(addrOfSv));

  listen(sockOfLiten,SOMAXCONN);

  struct sockaddr_in addrOfCl;
  int len = sizeof(addrOfCl);
  int sockOfCon = accept(sockOfLiten,(struct sockaddr *)&addrOfCl,&len);
  
  int val = 1;
  ioctl(sockOfCon,FIONBIO,&val);
  //close(sockOfLiten);

  int16_t ultra;
  unsigned char *ultra_pointer;

  while (1) {
    memset(buffer,0,sizeof(buffer));
    a = recv(sockOfCon,buffer,sizeof(buffer),0);
    if (a < 1) {
      if (errno == EAGAIN) {
        printf("0\n");
      } else {
        perror("recv");
        //break;
      }
    } else {
      ultra_pointer = &buffer[0];
      ultra = *ultra_pointer;
     
      printf("%d\n",ultra);
      //break;
    }

    printf("0\n");
  }

  close(sockOfLiten);
  close(sockOfCon);

  return 0;
}
