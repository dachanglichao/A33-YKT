#include <stdio.h> 
#include <stdlib.h> 
#include <errno.h> 
#include <string.h>
#include <sys/types.h>
#include <netinet/in.h>
#include<sys/un.h>
#include <sys/socket.h> 
#include <sys/wait.h> 
#include <sys/time.h>
#include <unistd.h> 
#include <arpa/inet.h> 
#include <sys/select.h>
#include "../debug/debug.h"

 
int sockfd, numbytes,sockfdLen; 

struct sockaddr_in local; 
socklen_t local_len;
struct sockaddr_in remoteAddr;



//创建TCP——CLIENT跟PosServer通讯；利用本地回环地址的形式
int MakePosServerUdpSocket(char *ip,int port)
{

    debug("posserverip = %s",ip);
    debug("posserverport = %d\n",port);
	
   // close(sockfd);
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) == -1) { 
		debug("posserver socket socket fail\n");
    }
  	memset(&local, 0, sizeof(struct sockaddr_in));  
    local.sin_family = AF_INET; 
    local.sin_port = htons(port); 
	local.sin_addr.s_addr = htonl(INADDR_ANY);                //设置目的ip
    local_len = sizeof(struct sockaddr); 
	if(bind(sockfd, (struct sockaddr *)&local, sizeof(local)))//绑定端口
    {
        printf("####L(%d) client bind port failed!\n", __LINE__);
        close(sockfd);//关闭socket
    }

//	int optval = 1;
//	setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST | SO_REUSEADDR, &optval, sizeof(int));
	memset(&remoteAddr, 0, sizeof(struct sockaddr_in));
	remoteAddr.sin_family = AF_INET;
	remoteAddr.sin_addr.s_addr = inet_addr(ip);
	remoteAddr.sin_port = htons(port);//UDP 广播包 远端端口

    return SUCCESS;
}

//udp socket接收数据PosServer
int UdpSocketSendDataToPosServer(char *send_buf,uint16_t strlen) 
{ 	
	int send_num;

	send_num = sendto(sockfd, send_buf, strlen, 0, (struct sockaddr *)&remoteAddr, local_len);  
	 
	if(send_num < 0)  
	{  
		debug("udp send data fail\n"); 
	}  
	return send_num;//
}

//tcp socket 发送数据到PosServer并等待数据返回，接收到数据后返回数据长度
int UdpSocketRecDataFromPosServer(uint8_t *Recdata,int recLength) 
{ 
	int recv_num;
	uint8_t tempbuf[512];
	int recvbytes,nRet;
	fd_set fds;
	struct timeval tv;
	
//	FD_ZERO( &fds );
//	FD_SET( sockfd, &fds );
//	//设置超时时间
//	tv.tv_sec = 0;
//	tv.tv_usec = 1000;
//	nRet = select( sockfd+1, &fds, NULL, NULL, &tv );
//	//if(nRet < 0 || nRet== 0)
//	if(!(FD_ISSET(sockfd,&fds)))
//	{		
//		debug("Possocket 接收数据超时 nRet ==%d\r\n",nRet);
//		return 0;//超时或连接错误
//	}
//	
	recv_num = recvfrom(sockfd, tempbuf, recLength, 0, (struct sockaddr *)&local, &local_len);  
	if(recv_num < 0)  
	{
		//debug("recvfrom error:");  
	}  
	else
	{
		Recdata[recv_num] = '\0';  
		gdHexStringToHexGroup(tempbuf+1, Recdata, recv_num-4);
		//debug("client receive %d bytes: %s\n", recv_num, Recdata);  
	}
	recv_num = recv_num/2;
	return recv_num-4;
}

//断开socket 
int UdpSocketclosePosServerSocket(void) 
{
	close(sockfd); 
}


