#include <stdio.h>  
#include <fcntl.h>  
#include <linux/ioctl.h>
#include <stdlib.h> 
#include <semaphore.h>
//#include <iostream>
#include<sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <stdio.h>
#include <sys/time.h>
#include <string.h>
#include <errno.h>
#include <sys/select.h>    
#include "rc522/MFRC522.h"
#include "rc522/ISO7816.h"
#include "socket/changeIp.h"
#include "cJSON/cJSON.h"
#include "txt/readtxt.h"
#include "socket/tcpclienttoQT.h"
#include "socket/tcpclienttoPosServer.h"
#include "debug/debug.h"
#include "byteConversion/byteConversion.h"
#include "ykt_app.h"

//本地回环地址创建TCP_SERVER_SOCKET用于接收QT的tcp链接   
//用户设定的IP地址创建TCP_CLIENT_SOCKET 用于跟服务器连
//创建线程1实时检查qt过来的tcp连接并进行数据交互
//创建线程2实时查询服务器是否有数据过来
//主线程做寻卡操作
//需要交替运行的线程之前用信号量来回通知                                                                                                                                                                                                                                                    
#define DISPLY_BLANK "\r\n"\
//本地回环地址服务器信息
#define SERV_PORT 9000        //服务器端口
#define POLL_SERV_PORT 9001   //QT查询功能服务器端口
#define SERV_IP "127.0.0.1"   //服务器ip
#define CONCURRENT_MAX   8 //最多的客户端连接个数
#define BACKLOG 5     //完成三次握手但没有accept的队列的长度   
#define BUFFER_SIZE 1024    

char input_msg[BUFFER_SIZE];    
char recv_msg[BUFFER_SIZE];
char buf[1024]; //ԃԚׁд˽ߝ
static uint8_t CardType = 0;//定义卡的类型 1为CPU卡，0为M1卡
static uint8_t m_ClientNum = 0;//已经连接的客户端数量  
static pthread_t thread[4];  //两个线程
static pthread_mutex_t mut;//互斥锁
sem_t   tSemRcvOrder;	//主线程接收数据采集线程的信号量
sem_t	tSemtcpserver;//tcpsever运行的信号量
sem_t	tSemtcpclient;//tcpclient posserver运行的信号量运行的信号量
			

static int server_sock_fd,client_fds[CONCURRENT_MAX],client_sock_fd ;   //创建两个文件描述符，servfd为监听套接字，clitfd用于数据传输
static socklen_t clit_size = 0, address_len;  ; //用于accept函数中保存客户端的地址结构体大小
static struct sockaddr_in server_addr,clit_addr,client_address;    //创建地址结构体，分别用来存放服务端和客户端的地址信息
  //fd_set  
fd_set server_fd_set; 
int max_fd = -1;    
struct timeval tv;  //socket连接超时时间设置    
//通过界面设置的设备IP地址
IPMESSAGE posIpMessage;
int servfd;
int qtconsumeClitfd;//qt消费的socket ID
int qtpollClitfd;//qt查询的socket ID


//app ykt  context
typedef struct {
uint8_t  aucCardData[256];
uint8_t	 jsonSenddata[500];//posserver交互数据
uint8_t  posServerCmd;//跟posserver的交互指令
uint8_t	 cardName[21];//卡姓名
uint8_t  cardId[20];//卡号
uint16_t stationCode;//站点号
uint16_t posCode;//设备号
uint32_t CardNum;//唯一序列号
uint32_t CurrentConsumMoney;//消费金额
	
}app_ykt_context;
//context
static app_ykt_context appYktCxt;	
/**@brief 初始化TCPSERVER
 */
static void app_ykt_tcpserver_init(void)
{	
	memset(&server_addr,0,sizeof(server_addr));  //初始化
	memset(&clit_addr,0,sizeof(clit_addr));  //初始化
	memset(&client_fds,0,sizeof(client_fds));  //初始化
	if((server_sock_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)  //创建套接字
	{
		debug("socket error");   
		return 1;
	}      
        //给服务端的地址结构体赋值
	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(SERV_PORT); //将主机上的小端字节序转换为网络传输的大端字节序（如果主机本身就是大端字节序就不用转换了）
	server_addr.sin_addr.s_addr = inet_addr(SERV_IP); //将字符串形式的ip地址转换为点分十进制格式的ip地址
        //绑定地址信息到监听套接字上，第二个参数强转是因为形参类型为sockaddr ，而实参类型是sockaddr_in 型的

	if(bind(server_sock_fd,(struct sockaddr *)& server_addr,sizeof(server_addr)) == -1)
	{
		debug("bind error");   
		return 1;
	}  
        //将servfd套接字置为监听状态

	if(listen(server_sock_fd,BACKLOG) == -1)
	{
		debug("listen error");   
		return 1;
	}
}

/**@brief TCPSERVER线程实时查询新接进来的客户端连接
 */
static void *app_ykt_tcpserver_accept_thread(void)
{
	int i; 
	struct sockaddr_in serv_addr,clit_addr;
	memset(&serv_addr,0,sizeof(serv_addr)); 
	memset(&clit_addr,0,sizeof(clit_addr)); 
	if((server_sock_fd = socket(AF_INET,SOCK_STREAM,0)) == -1)  
	{
		return 0;
	}      
       
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(POLL_SERV_PORT); 
	serv_addr.sin_addr.s_addr = inet_addr(SERV_IP);

	if(bind(server_sock_fd,(struct sockaddr *)& serv_addr,sizeof(serv_addr)) == -1)
	{
		debug("bind error\n");
		return 0;
	}         
	if(listen(server_sock_fd,1024) == -1)
	{
		debug("bind error\n");
		return 0;
	}       
	while(1)
	{
		sem_wait(&tSemtcpserver);	//等待信号量

		//设置超时时间
		tv.tv_sec = 1;    
		tv.tv_usec = 0;    
		FD_ZERO(&server_fd_set);    
		FD_SET(STDIN_FILENO, &server_fd_set);    
		if(max_fd <STDIN_FILENO)    
		{    
			max_fd = STDIN_FILENO;    
		}    
		//printf("STDIN_FILENO=%d\n", STDIN_FILENO);    
	//服务器端socket    
		FD_SET(server_sock_fd, &server_fd_set);    
	   // printf("server_sock_fd=%d\n", server_sock_fd);    
		if(max_fd < server_sock_fd)    
		{    
			max_fd = server_sock_fd;    
		}    
	//客户端连接    
		for(i =0; i < CONCURRENT_MAX; i++)    
		{    
			//printf("client_fds[%d]=%d\n", i, client_fds[i]);    
			if(client_fds[i] != 0)    
			{    
				FD_SET(client_fds[i], &server_fd_set);    
				if(max_fd < client_fds[i])    
				{    
					max_fd = client_fds[i];    
				}    
			}    
		}    
		int ret = select(max_fd + 1, &server_fd_set, NULL, NULL, &tv);    
		if(ret < 0)    
		{    
			debug("select 出错\n");    
				
		}    
		else if(ret == 0)    
		{    
			debug("select 超时\n");    
			   
		}    
		else    
		{    
			//ret 为未状态发生变化的文件描述符的个数    
			if(FD_ISSET(STDIN_FILENO, &server_fd_set))    
			{    
				debug("发送消息：\n");    
				bzero(input_msg, BUFFER_SIZE);    
				fgets(input_msg, BUFFER_SIZE, stdin);    
				   
				for(i = 0; i < CONCURRENT_MAX; i++)    
				{    
					if(client_fds[i] != 0)    
					{    
						debug("client_fds[%d]=%d\n", i, client_fds[i]);    
						send(client_fds[i], input_msg, BUFFER_SIZE, 0);    
					}    
				}    
			}    
			if(FD_ISSET(server_sock_fd, &server_fd_set))    
			{    
				//有新的连接请求    			  
				qtpollClitfd = accept(server_sock_fd, (struct sockaddr *)&client_address, &address_len);    
				debug("new connection qtpollClitfd = %d\n", qtpollClitfd);    
				if(qtpollClitfd > 0)    
				{    
					int index = -1;    
					for(i = 0; i < CONCURRENT_MAX; i++)    
					{    
						if(client_fds[i] == 0)    
						{    
							index = i;    
							client_fds[i] = qtpollClitfd;    
							break;    
						}    
					}    
					if(index >= 0)    
					{    
						debug("新客户端(%d)加入成功 %s:%d\n", index, inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));    
					}    
					else    
					{    
						bzero(input_msg, BUFFER_SIZE);    
						strcpy(input_msg, "服务器加入的客户端数达到最大值,无法加入!\n");    
						send(qtpollClitfd, input_msg, BUFFER_SIZE, 0);    
						debug("客户端连接数达到最大值，新客户端加入失败 %s:%d\n", inet_ntoa(client_address.sin_addr), ntohs(client_address.sin_port));    
					}    
				}    
			}    
			for(i =0; i < CONCURRENT_MAX; i++)    
			{    
				if(client_fds[i] !=0)    
				{    
					if(FD_ISSET(client_fds[i], &server_fd_set))    
					{    
						//处理某个客户端过来的消息    
						bzero(recv_msg, BUFFER_SIZE);    
						long byte_num = recv(client_fds[i], recv_msg, BUFFER_SIZE, 0);    
						if (byte_num > 0)    
						{    
							if(byte_num > BUFFER_SIZE)    
							{    
								byte_num = BUFFER_SIZE;    
							}    
							recv_msg[byte_num] = '\0';    
							debug("客户端(%d):%s\n", i, recv_msg); 
							//原路返回接收的信息
							sem_wait(&tSemRcvOrder);	//等待信号量
							send(client_fds[i], recv_msg, BUFFER_SIZE, 0);						
						}    
						else if(byte_num < 0)    
						{    
							debug("从客户端(%d)接受消息出错.\n", i);    
						}    
						else    
						{    
							FD_CLR(client_fds[i], &server_fd_set);    
							client_fds[i] = 0;    
							debug("客户端(%d)退出了\n", i);    
						}    
					}    
				}    
			}    
		} 
	}
}
/**@brief 请求消费
 */
static void app_ykt_consume(void)
{
	char	*out;
	cJSON *root;
	uint8_t buf[100];
	uint8_t buf1[100],Tx_Buffer[200],i,j;
	uint16_t totalsize = 0,request_len = 0,header_len = 0;
	uint32_t aa;
	
	root=cJSON_CreateObject();	

	memset(buf,0,20);
	//memcpy(buf,appYktCxt.cardName,appYktCxt.cardName[20]);
		//名字
	//cJSON_AddItemToObject(root,"username",cJSON_CreateString(buf));
	//cJSON_AddItemToObject(root,"username","张三");
	
	//卡序列号
	memset(appYktCxt.cardId,1,20);
	cJSON_AddItemToObject(root,"cardid",cJSON_CreateString(appYktCxt.cardId));
	
	//消费金额
	memset(buf,0,20);
	appYktCxt.CurrentConsumMoney= 120;
	ChgUlongToBCDString(appYktCxt.CurrentConsumMoney,buf1,4);
	HexGroupToHexString(buf1,buf,4);
	memset(buf1,0,20);
	j=0;
	for(i=0;i<8;i++)
	{
		if(buf[i]==0)
			break;
	 if(buf[i] != 0x30)
	 {
		buf1[j++] = buf[i];
	 }
	 else
	 {
		if(j>0)
			buf1[j++] = buf[i];
	 }
	}
	cJSON_AddItemToObject(root,"txamt",cJSON_CreateString(buf1));
	
	//站点号
	ChgUlongToBCDString(appYktCxt.stationCode,buf1,2);
	HexGroupToHexString(buf1,buf,2);
	cJSON_AddItemToObject(root,"appYktCxt.stationcode",cJSON_CreateString(buf));
	
	//设备号
	ChgUlongToBCDString(appYktCxt.posCode,buf1,2);
	HexGroupToHexString(buf1,buf,2);
	cJSON_AddItemToObject(root,"poscode",cJSON_CreateString(buf));
	
	//联机标志
	cJSON_AddStringToObject(root,"online","online");
	
	//流水号
	ChgUlongToBCDString(appYktCxt.posCode,Tx_Buffer,2);
	
	HexGroupToHexString(Tx_Buffer,buf1,12);
	memcpy(Tx_Buffer,buf1,4);
	Tx_Buffer[4] = '-';
	memcpy(Tx_Buffer+5,buf,14);
	memcpy(Tx_Buffer+19,buf1+4,8);
	
	cJSON_AddItemToObject(root,"posjourno",cJSON_CreateString(Tx_Buffer));
	
	//消费模式
	cJSON_AddStringToObject(root,"consumemode","1");
	
	//	交易密码
//	cJSON_AddStringToObject(root,"password","123456");
	
	out=cJSON_PrintUnformatted(root);	
	//out=cJSON_Print(root);	//¤???3ǌ??DD
	request_len = strlen(out);
	debug("request_len==%d",request_len);
	TcpSocketSendDataToPosServer(out);

}


/**@brief 线程poserver  pos跟服务器数据交互一个发送对应一个应答发送接收数据格式 jison
 */
static void *app_ykt_posserver_thread(void)
{

	while(1)
	{
		//等待信号量
		sem_wait(&tSemtcpclient);
		debug ("thread2 : app_ykt_posserver_thread\n");	
		switch(appYktCxt.posServerCmd)
		{		
			case YKT_CONSUNE_CMD://消费指令处理
				app_ykt_consume();
			break;		
		}
	}	
}
/**@brief 线程接收posserver发送的数据
 */
static void *app_ykt_get_data_posserver_thread(void)
{
	int receivedatalen;
	while(1)
	{
		//等待信号量
		//sem_wait(&tSemtcpclient);
		receivedatalen = TcpSocketReceiveDataFromPosServer(appYktCxt.jsonSenddata);
		//接收到数据
		if(receivedatalen)
		{
			debug("appYktCxt.jsonSenddata=%s \r\n",appYktCxt.jsonSenddata);
		}
		else
		{
		//重新创建连接
			debug("tcp socket out 未结束到数据 \n");
			TcpSocketclosePosServerSocket();
			//创建跟PosServer通讯的TCP链接
			if(!MakePosServerTcpSocket(posIpMessage.routeip,gdstringToInt(posIpMessage.port)))
				debug("Make PosServer Tcp Socket ok\n");
			else
				debug("Make PosServer Tcp Socket fail\n");
		}
	}	
}

/**@brief 线程接收posserver发送的数据
 */
static void *app_ykt_get_data_qt_thread(void)
{
	int receivedatalen;
	while(1)
	{

	//	bzero(recv_msg, BUFFER_SIZE);    
		receivedatalen = recv(qtconsumeClitfd, recv_msg, BUFFER_SIZE, 0);    
	
		if(receivedatalen)
		{
			debug("qtconsumeClitfd recv_msg=%s \r\n",recv_msg);
		}
		else
		{
		//重新创建连接
		debug("未接收到qt数据");
		}
	}	
}


/**@brief 创建线程
 */
static void app_ykt_create_thread(void)
{
	int temp;
	memset(&thread, 0, sizeof(thread));          //comment1
	//创建线程
	if((temp = pthread_create(&thread[0], NULL, app_ykt_tcpserver_accept_thread, NULL)) != 0)  //comment2     
			debug("线程1创建失败!/n");
	else
			debug("线程1被创建/n");

	if((temp = pthread_create(&thread[1], NULL, app_ykt_posserver_thread, NULL)) != 0)  //comment3
			debug("线程2创建失败");
	else
			debug("线程2被创建/n");
//实时接收posserver发送的数据
	if((temp = pthread_create(&thread[2], NULL, app_ykt_get_data_posserver_thread, NULL)) != 0)  //comment3
			debug("线程3创建失败");
	else
			debug("线程3被创建/n");
	//实时接收qt发送的数据
	if((temp = pthread_create(&thread[3], NULL, app_ykt_get_data_qt_thread, NULL)) != 0)  //comment3
			debug("线程4创建失败");
	else
			debug("线程4被创建/n");
	
}
/**@brief 创建线程等待
 */
static void app_ykt_thread_wait(void)
{
	//等待线程结束
	if(thread[0] !=0)
	{             //comment4    
		pthread_join(thread[0],NULL);
		debug("线程1已经结束/n");
	}
	if(thread[1] !=0) 
	{  
		//comment5
		pthread_join(thread[1],NULL);
		debug("线程2已经结束/n");
	}
	if(thread[2] !=0) 
	{  
		//comment5
		pthread_join(thread[2],NULL);
		debug("线程3已经结束/n");
	}
	if(thread[3] !=0) 
	{  
		//comment5
		pthread_join(thread[3],NULL);
		debug("线程4已经结束/n");
	}
}
/**@利用回环地址跟qt创建一个socket用于与qt之间的通讯
 */
static void app_ykt_create_qt_socket(void)
{
	int i;   
	struct sockaddr_in serv_addr,clit_addr;
	memset(&serv_addr,0,sizeof(serv_addr)); 
	memset(&clit_addr,0,sizeof(clit_addr)); 
	if((servfd = socket(AF_INET,SOCK_STREAM,0)) == -1)  
	{
		return 0;
	}      
       
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(SERV_PORT); 
	serv_addr.sin_addr.s_addr = inet_addr(SERV_IP);

	if(bind(servfd,(struct sockaddr *)& serv_addr,sizeof(serv_addr)) == -1)
	{
		debug("bind error\n");
		return 0;
	}  
       

	if(listen(servfd,1024) == -1)
	{
		debug("bind error\n");
		return 0;
	}
        
	if((qtconsumeClitfd = accept(servfd,(struct sockaddr *)& clit_addr,&clit_size)) == -1)
	{
		debug("bind error\n");
		return 0;
	}

}

/**@brief qtsocket 发送数据到qt显示
 */
static void app_ykt_socket_sendata_qt(int socketid,uint8_t *data,int len)
{
	write(socketid,data,len); 	
}

/**@brief Function for initializing ccb dc-ep applicaiton.
 */
static void app_ykt_init(void)
{	
	
	//初始化RC522
	InitRc522Driver();
	MFRC522_Initializtion();
	
	
	//设备配置信息分配地址空间
	posIpMessage.localip=(char *)malloc(sizeof(char)*50);
	posIpMessage.gateway=(char *)malloc(sizeof(char)*50);
	posIpMessage.mask=(char *)malloc(sizeof(char)*50);
	posIpMessage.routeip=(char *)malloc(sizeof(char)*50);
	posIpMessage.port=(char *)malloc(sizeof(char)*50);
	posIpMessage.maincode=(char *)malloc(sizeof(char)*50);

	//读取设备信息
	readIp(posIpMessage);
	//设置设备IP地址信息
	SetIfAddr(ifname,posIpMessage.localip,posIpMessage.mask,posIpMessage.gateway);
	//创建tcpsocket跟qt通讯
	app_ykt_create_qt_socket();
	//创建跟PosServer通讯的TCP链接
	if(!MakePosServerTcpSocket(posIpMessage.routeip,gdstringToInt(posIpMessage.port)))
		debug("Make PosServer Tcp Socket ok\n");
	else
		debug("Make PosServer Tcp Socket fail\n");
	//tcpserver 初始化
	//app_ykt_tcpserver_init();
	//线程创建
	app_ykt_create_thread();	

}

int app_ykt_main(void)
{  	
	app_ykt_init();
	while(1)
	{
		int rdstate;

		if(Request_Card_info(CardType,1,appYktCxt.aucCardData,&appYktCxt.CardNum) == 0)
		{
			debug("get card:%02X %02X %02X %02X\n",(appYktCxt.CardNum >> 24) & 0xFF,(appYktCxt.CardNum >> 16) & 0xFF,(appYktCxt.CardNum >> 8) & 0xFF,appYktCxt.CardNum & 0xFF);
			
			sprintf(appYktCxt.aucCardData,"%2X %2X %2X %2X",(uint8_t)(appYktCxt.CardNum >> 24),(uint8_t)(appYktCxt.CardNum >> 16),(uint8_t)(appYktCxt.CardNum >> 8),(uint8_t)appYktCxt.CardNum);

			//请求消费
			appYktCxt.posServerCmd = YKT_CONSUNE_CMD;
			app_ykt_socket_sendata_qt(qtconsumeClitfd,appYktCxt.aucCardData,2); 
			//释放posserver线程信号量
			sem_post(&tSemtcpclient);
	
		//ReadCardCommonDatas_CPU();
		//TcpSocketSendDataToQt(buf);
		//TcpSocketSendDataToPosServer(buf);
		}else
		{
			debug("get carderror\n");
			sem_post(&tSemtcpserver);
		}
	}
	//线程创建等待
	app_ykt_thread_wait();
	return 0;  
}  
