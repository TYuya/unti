/* 概要:シリアルポートをオープンし、通信を行うプログラム。サーバとEV3で通信を行う、EV3から送信されたカラーセンサと超音波センサの値を受信し、制御情報を生成し送信してEV3のライントレース及び衝突回避を制御する 
 */

/* ファイル名:server_active.c */
/* 日付:2017/10/5 */
/* 最後の更新:2018/1/19 */ 
/* 作成者:國弘勇人 */

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h> /* uint, int~系 */
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/ioctl.h>
#include <errno.h>
#include <sys/time.h>

/* マクロ定義 */
#define EV3 "/dev/rfcomm0" /* デバイスファイル名 */
#define BAUDRATE B115200 /* ボーレート */
#define BUFFSIZE 3 /* バッファサイズ */

/* 制御情報 */
#define STOP 1
#define RUN 0

/* 色の識別 */
#define STRAIGHT 0
#define RIGHT 1
#define LEFT 2
#define BACK 3

/* 閾値 */
#define ULTRA_THRESHOLD 20
#define COLOR_THRESHOLD 20


/* 関数定義 */
void main(); /* main */

/* main関数 */
void main() {
  int EV3_fd; /* ファイルディスクリプタ */
  struct termios oldtio, newtio; /* シリアルポート構造体定義 */
  int a,b;
  int val = 1;

  /* IP アドレス、ポート番号、ソケット */
  unsigned short port = 9875;
  unsigned short port1 = 9876;
  int sockOfLiten;
  int dstSocket;
  int srcSocket;

  /* sockaddr_in 構造体 */
  struct sockaddr_in srcAddr;
  struct sockaddr_in dstAddr;
  struct sockaddr_in addrOfSv;
  struct sockaddr_in addrOfCl;
  int dstAddrSize = sizeof(dstAddr);
  

  /* 各種パラメータ */
  int status;
  int numrcv;

  /* sockaddr_in 構造体のセット */
  memset(&srcAddr,0,sizeof(srcAddr));
  srcAddr.sin_port = htons(port);
  srcAddr.sin_family = AF_INET;
  srcAddr.sin_addr.s_addr = htonl(INADDR_ANY);
 
  /* ソケット生成 中継機 */
  srcSocket = socket(AF_INET, SOCK_STREAM, 0);

  bind(srcSocket,(struct sockaddr *)&srcAddr,sizeof(srcAddr));

  listen(srcSocket,1);

  printf("Waiting for connenction ...\n");
  dstSocket = accept(srcSocket,(struct sockaddr *)&dstAddr,&dstAddrSize);
  printf("Connected from %s\n",inet_ntoa(dstAddr.sin_addr));

  /* ソケット生成　GUI */
  sockOfLiten = socket(AF_INET,SOCK_STREAM,0);

  addrOfSv.sin_port = htons(port1);
  addrOfSv.sin_family = AF_INET;
  addrOfSv.sin_addr.s_addr = INADDR_ANY;

  bind(sockOfLiten,(struct sockaddr *) &addrOfSv,sizeof(addrOfSv));

  listen(sockOfLiten,SOMAXCONN);

  int AddrCl = sizeof(addrOfCl);
  int sockOfCon = accept(sockOfLiten,(struct sockaddr *) &addrOfCl,&AddrCl);

  int i; /* ループの変数 */
  int len; /* 通信データバイト数 */
  
  unsigned char rec_buf[BUFFSIZE]; /* データ受信バッファ */
  unsigned char send_buf[BUFFSIZE - 1]; /* データ送信バッファ */
  unsigned char buffer[256];  
 
  int16_t ultra; /* 送られてきた超音波センサの値を格納する変数 */
  unsigned char *ultra_pointer; /* 受信バッファを指すポインタ */
  uint8_t color1; /* 送られてきたカラーセンサの値を格納する変数 */
  unsigned char *color_pointer1; /* 受信バッファを指すポインタ */
  uint8_t color2; /* 送られてきたカラーセンサの値を格納する変数 */
  unsigned char *color_pointer2; /* 受信バッファを指すポインタ */
  
  int8_t ul = 48;
  unsigned char *ul_po;

  ioctl(sockOfCon,FIONBIO,&val);

  char flag1, flag2; /* 白黒定義フラグ */

  sleep(10);

  /* 通信処理開始 */
  while(1) {
    /* --------------------受信モジュール開始--------------------- */
    /* ここで受信待ち、データがEV3から送られてくるまで待つ */
    len = read(dstSocket,rec_buf,BUFFSIZE);

    memset(buffer,0,sizeof(buffer));
    a = recv(sockOfCon,buffer,sizeof(buffer),0);

    if (a < 1) {
      if (errno != EAGAIN) {
        perror("read");
      }
    } else {
      ul_po = &buffer[0];
      ul = *ul_po;
    }
    
    if (ul == 48) {
    /* 受信したデータを 10進数形式で表示、その後改行する */    
    printf("%d %d %d\n", rec_buf[0], rec_buf[1], rec_buf[2]);
      
    /* --------------------受信モジュール終了--------------------- */
    
    /* 受信距離情報選択開始 */
    ultra_pointer = &rec_buf[0]; /* 受信バッファの先頭要素を指す */
    ultra = *ultra_pointer; /* 受信したカラーセンサの値を変数に格納 */
    /* 受信距離情報選択終了 */ 
    
    /* 受信カラー情報選択開始 */
    color_pointer1 = &rec_buf[1]; /* 受信バッファの先頭要素を指す */
    color1 = *color_pointer1; /* 受信したカラーセンサの値を変数に格納 */
    /* 受信カラー情報選択終了 */
    
    /* 受信カラー情報選択開始 */ 
    color_pointer2 = &rec_buf[2]; /* 受信バッファの先頭要素を指す */
    color2 = *color_pointer2; /* 受信したカラーセンサの値を変数に格納 */
    /* 受信カラー情報選択終了 */
    
    /* --------------------送信モジュール開始 --------------------- */
 
    /* 距離情報判断 */
    if (ultra > ULTRA_THRESHOLD) {
      /*障害物なしと判断された場合 */
      send_buf[0] = RUN;
    } else {
      /* 障害物ありと判断された場合 */
      send_buf[0] = STOP;
    }

    /* カラー情報判断 */
    flag1 = color1 < COLOR_THRESHOLD;
    flag2 = color2 < COLOR_THRESHOLD;
    if (flag1 && flag2) send_buf[1] = STRAIGHT;
    else if (flag1) send_buf[1] = RIGHT;
    else if (flag2) send_buf[1] = LEFT;
    else send_buf[1] = STRAIGHT;
    
    } else {
      printf("%d %d %d ",rec_buf[0],rec_buf[1],rec_buf[2]);
      switch(ul) {
        case 49:
          send_buf[0] = RUN;
          send_buf[1] = STRAIGHT;
          printf("上\n");
	  break;
        case 50:
  	  send_buf[0] = RUN;
          send_buf[1] = RIGHT;
          printf("右\n");
	  break;
        case 51:
	  send_buf[0] = RUN;
          send_buf[1] = LEFT;
          printf("左\n");
          break;
        case 52:
	  send_buf[0] = RUN;
          send_buf[1] = BACK;
          printf("下\n");
   	  break;
        case 53:
          send_buf[0] = STOP;
          printf("止\n");
	  break;
      }
    }
 
    /* writeによって送信する */
    len = write(dstSocket, send_buf, BUFFSIZE-1);
    
    if (len < 0) {
      printf("write error\n"); /* read()が負を返したら何らかのI/Oエラー */
      exit(1);
    } 
    /* --------------------送信モジュール終了--------------------- */	
  }
  /* 通信処理終了 */
  
  tcsetattr(EV3_fd, TCSANOW, &oldtio); /* シリアルポートの設定を戻す */
  close(EV3_fd); /* デバイスクローズ */
  close(sockOfLiten);
  close(sockOfCon);
  close(dstSocket);
  close(srcSocket);
  printf("Communication finished\n");
  return;
}
