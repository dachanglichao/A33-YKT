#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include<sys/un.h>
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <unistd.h> 
#include <arpa/inet.h> 
#include "../debug/debug.h"

#define SERVER_PORT 8888
#define MAXDATASIZE 2048  
#define SERVER_IP "127.0.0.1" 
 int qtsockfd, numbytes; 
char buf[MAXDATASIZE]; 
struct sockaddr_in server_addr; 
//创建TCP——CLIENT跟QTTCP——server通讯；利用本地回环地址的形式
int MakeQTTcpSocket()
{

    debug("\n======================QT client initialization======================\n"); 
    if ((qtsockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) { 
        perror("socket"); 
        exit(1); 
    }
    debug("socket connect ok");

    server_addr.sin_family = AF_INET; 
    server_addr.sin_port = htons(SERVER_PORT); 
    server_addr.sin_addr.s_addr = inet_addr(SERVER_IP); 
    bzero(&(server_addr.sin_zero),sizeof(server_addr.sin_zero)); 

    if (connect(qtsockfd, (struct sockaddr *)&server_addr,sizeof(struct sockaddr_in)) == -1){
         perror("connect error"); 
         exit(1);
     } 
    

}
///tcp socket 发送数据到QT
int TcpSocketSendDataToQt(char *str) 
{ 
    //sent to the server
  send(qtsockfd,str,strlen(str),0);
       // close(sockfd); 
        return 0;
}