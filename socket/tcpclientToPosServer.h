
#include <string.h>


//创建TCP——CLIENT跟PosServer通讯；利用本地回环地址的形式
int MakePosServerUdpSocket(char *ip,int port);


//udp socket接收数据PosServer
int UdpSocketSendDataToPosServer(char *send_buf,uint16_t strlen) ;


//tcp socket 发送数据到PosServer并等待数据返回，接收到数据后返回数据长度
int UdpSocketRecDataFromPosServer(uint8_t *Recdata,int recLength) ;


//断开socket 
int UdpSocketclosePosServerSocket(void) ;