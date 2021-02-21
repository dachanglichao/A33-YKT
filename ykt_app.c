#include <stdio.h>  
#include <stdbool.h>  
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
#include <getopt.h>


#include <sys/select.h>    
#include "rc522/MFRC522.h"
#include "rc522/ISO7816.h"
#include "socket/changeIp.h"
#include "cJSON/cJSON.h"
#include "txt/readtxt.h"
#include "sysTime/sysTime.h"
#include "descode/descode.h"
#include "beep/beep.h"
#include "socket/tcpclienttoQT.h"
#include "socket/tcpclienttoPosServer.h"
#include "debug/debug.h"
#include "byteConversion/byteConversion.h"
#include "sqlite3/sqlite3.h"
#include "sqlite3/possqlite3.h"
#include "pcf8563/pcf8563.h"
#include "battery/battery.h"
#include "socket/udp_data_analyse.h"
#include "ykt_app.h"
#include <sys/ipc.h>
#include <sys/msg.h>
#include "VariableDef.h"
//#include "systime/sysTime.h"



//本地回环地址创建TCP_SERVER_SOCKET用于接收QT的tcp链接   
//用户设定的IP地址创建TCP_CLIENT_SOCKET 用于跟服务器连
//创建线程1实时检查qt过来的tcp连接并进行数据交互
//创建线程2实时查询服务器是否有数据过来
//主线程做寻卡操作
//需要交替运行的线程之前用信号量来回通知                                                                                                                                                                                                                                                    
#define DISPLY_BLANK "\r\n"\
//本地回环地址服务器信息
#define SERV_PORT 5000        //服务器端口
#define SERV_IP "127.0.0.1"   //服务器ip
#define CONCURRENT_MAX   4 //最多的客户端连接个数
#define BACKLOG 		 5 //完成三次握手但没有accept的队列的长度   
#define BUFFER_SIZE 1024    
#define devicetype	1
#define TIMEOUTNUMBER   100//总的超时时间100*10ms
#define	Off_MAXRECORD	    	3000//×î¶àµÄ´æ´¢¼ÇÂ¼Êý

//连续刷卡
//#define CON_SWIPE_CARD

//char send_msg[BUFFER_SIZE];    
char recv_msg[BUFFER_SIZE];
static uint8_t m_ClientNum = 0;//已经连接的客户端数量  
static pthread_t thread[5];  //两个线程
//定义一个全局的互斥变量
pthread_mutex_t mutex ;

sem_t   tSemRcvOrder;	//主线程接收数据采集线程的信号量


static int server_sock_fd,client_fds[CONCURRENT_MAX],client_sock_fd ;   //创建两个文件描述符，servfd为监听套接字，clitfd用于数据传输
static socklen_t clit_size = 0, address_len;  ; //用于accept函数中保存客户端的地址结构体大小
static struct sockaddr_in server_addr,clit_addr,client_address;    //创建地址结构体，分别用来存放服务端和客户端的地址信息
 //fd_set  
fd_set server_fd_set; 
int max_fd = -1;    
struct timeval tv;  //socket连接超时时间设置    
//通过界面设置的设备IP地址
IPMESSAGE posIpMessage;
POSMESSAGE posMessage;
CONSUMEMODEMESSAGE consumeModeMessage;//消费模式


socklen_t qtromte_len;



int servfd;
int qtconsumeClitfd;//qt消费的socket ID
int qtpollClitfd;//qt查询的socket ID


struct sockaddr_in qtaddress;//处理qtudp通讯地址
struct sockaddr_in conremoteAddr;//qt消费界面端口
#define CONSUMEQTPORT 5001

struct sockaddr_in conModeremoteAddr;//qt设置消费模式端口
#define CONSUMEMODEQTPORT 5002

struct sockaddr_in quDayremoteAddr;//qt查询某日消费金额
#define QUDAYQTPORT 5003

struct sockaddr_in quRecordremoteAddr;//qt查询采集记录
#define QURECORDQTPORT 5004

socklen_t qt_len;


//uint8_t ConsumCase =0 ;//消费状态机

union	uTimeUnion		SysTimeDatas;//系统时间
//app ykt  context
typedef struct {
bool     bconsumeWorking;//正在消费过程中
bool 	 bsocketconnectOk;//socket是否创建
bool     breadDataFronSocket;//从posserver读取到数据
uint8_t  batteryStatus;//供电状态
uint8_t  usesectornum;//扇区号
uint8_t  netstatus;//网络连接状态
uint8_t  consumeMode;//消费模式
uint8_t	 cardName[21];//卡姓名
uint8_t  CardPrinterNum[20];//印刷编号卡号
uint8_t  CardSerialNum[8];//唯一序列号
uint8_t  aucCardData[20];
uint16_t posServerCmd;//跟posserver的交互指令
uint32_t CurrentConsumMoney;//消费金额

uint32_t LimtConsumeMoney;//系统脱机限额
uint32_t noNetRecordNum;//脱机消费笔数
uint32_t noNetRecordMoney;//脱机消费总额
uint32_t offline_UpIndex;//脱机记录指针
uint32_t batchNum;//交易流水号标记累加跟交易时间组成不重复的流水号：站点号+设备号+交易时间+batchNum
struct sRecordStruct sRt;//脱机交易记录
struct sRecordStruct sUpRt;//上传脱机交易记录
struct sRecordMoneyStruct RecordStr;//脱机记录总额笔数
	
}app_ykt_context;
//context
static app_ykt_context appYktCxt;	


#define KEY 1000  //语音播放消息队列KEY
int aplaymsqid;//消息队列ID

struct msg
{
	long int mtype;
	char mtext[200];
};


    
//创建消息队列跟aplayMP3进程通讯
static void app_ykt_create_aplay_msg(void)
{
	//创建qt主界面消息队列
	aplaymsqid=msgget((key_t)KEY,IPC_EXCL);  /*检查消息队列是否存在*/
	if(aplaymsqid < 0)
	{
		aplaymsqid = msgget((key_t)KEY,IPC_CREAT|0666);/*创建消息队列*/
		if(aplaymsqid <0)
		{
			debug("failed to create msq \n");
		}
		debug("aplaymsqid == %d\n",aplaymsqid);
	}
	else
		debug("消息队列已经存在id =%d\r\n",aplaymsqid);
}

//创建消息队列发送数据到qt主界面
static void app_ykt_send_msg_to_aplay(uint8_t *data,uint16_t len)
{
	int ret_value;
	struct msg msgtxt;

	/* 发送消息队列 */
	msgtxt.mtype = data[0];
	memcpy(msgtxt.mtext,data,len);
	ret_value = msgsnd(aplaymsqid,&msgtxt,sizeof(msgtxt.mtext),0);
	debug("ret_value ==%d \n",ret_value);
	if ( ret_value < 0 ) {
		printf("msgsnd() write msg failed\n");
	//exit(-1);
	}
}
/**@brief 语音播放
 */
static void app_ykt_player(uint8_t status)
{
	//system("/opt/work/posApp/amixer cset numid=27 1");
	app_ykt_send_msg_to_aplay(&status,1);//
}



/*@brief QT显示函数端口号9000
cmd 显示命令*/
static void app_ykt_qt_disp(uint8_t cmd,uint8_t *data,uint len)
{
	uint8_t dataTemp[255],send_num;
	
	dataTemp[0] = cmd;//
	memcpy(dataTemp+1,data,len);
	qtromte_len = sizeof(struct sockaddr); 
	if(cmd == QT_DISP_BALANCE_CMD ||cmd == QT_SET_CONSUME_MODE_CMD ||cmd==QT_DISP_CONSUME_MODE_CMD ||cmd==QT_DISP_CLEAR_CMD||cmd==QT_DISP_ERROR_CODE_CMD)//消费界面
		send_num = sendto(qtconsumeClitfd, dataTemp, len+1, 0, (struct sockaddr *)&conremoteAddr, qtromte_len); 
	if(cmd== QT_DISP_TOAL_MONEY_CMD)//查询日消费额
		send_num = sendto(qtconsumeClitfd, dataTemp, len+1, 0, (struct sockaddr *)&quDayremoteAddr, qtromte_len);

	if(cmd==QT_DISP_RECORD_MONEY_CMD)
		send_num = sendto(qtconsumeClitfd, dataTemp, len+1, 0, (struct sockaddr *)&quRecordremoteAddr, qtromte_len);
}

/*@brief 解析qt主界面发送的数据
 */
static void app_ykt_response_qt_data(char *data)
{
	uint16_t templen=0;
	uint8_t tempbuf[50],day1[8],day2[8];
	
	switch (data[0])
	{
		case 0xA0://传入消费金额
		CurrentConsumMoney = data[2]*65536+ data[3]*256+ data[4];
		debug("CurrentConsumMoney==%d\n ", CurrentConsumMoney);
		break;

		case QT_DISP_TOAL_MONEY_CMD://查询某日--某日的消费金额

			memcpy(day1,recv_msg+3,6);
			debug("day1 =%s\n",day1);
			memcpy(day2,recv_msg+11,6);
			debug("day2 =%s\n",day2);
			appYktCxt.RecordStr = sqlite3_consume_query_daymoney_db(day1,day2);
			recv_msg[0] = appYktCxt.RecordStr.RecordToalMoney>>24;
			recv_msg[1] = appYktCxt.RecordStr.RecordToalMoney>>16;
			recv_msg[2] = appYktCxt.RecordStr.RecordToalMoney>>8;
			recv_msg[3] = appYktCxt.RecordStr.RecordToalMoney;
			app_ykt_qt_disp(QT_DISP_TOAL_MONEY_CMD,recv_msg,4);//QT显示查询某日的消费记录
			app_ykt_player(QT_CONSUME_QUERY_OK);//查询成功	
			//	sem_post(&tSemtcpclient);
			break;

			case QT_DISP_RECORD_MONEY_CMD://查询脱机消费记录
			appYktCxt.RecordStr = sqlite3_consume_query_consumemoney_db();
			if(appYktCxt.RecordStr.status)//查询成功
			{
				recv_msg[0] = appYktCxt.RecordStr.RecordToalNumber>>24;
				recv_msg[1] = appYktCxt.RecordStr.RecordToalNumber>>16;
				recv_msg[2] = appYktCxt.RecordStr.RecordToalNumber>>8;
				recv_msg[3] = appYktCxt.RecordStr.RecordToalNumber;

				recv_msg[4] = appYktCxt.RecordStr.RecordToalMoney>>24;
				recv_msg[5] = appYktCxt.RecordStr.RecordToalMoney>>16;
				recv_msg[6] = appYktCxt.RecordStr.RecordToalMoney>>8;
				recv_msg[7] = appYktCxt.RecordStr.RecordToalMoney;
				debug("noNetRecordNum ==%d,noNetRecordMoney==%d\n",appYktCxt.RecordStr.RecordToalNumber,appYktCxt.RecordStr.RecordToalMoney);
				app_ykt_qt_disp(QT_DISP_RECORD_MONEY_CMD,recv_msg,8);//QT显示脱机消费记录笔数总额
				app_ykt_player(QT_CONSUME_QUERY_OK);//查询成功	
			}
			else
			{
				app_ykt_player(QT_CONSUME_QUERY_FAIL);//查询失败
			}

			break;
			
			case QT_SET_CONSUME_MODE_CMD://设置消费模式
			appYktCxt.consumeMode = data[1];//消费模式
			debug("消费模式= %d,消费金额=%d\n",data[1],CurrentConsumMoney);	
			app_ykt_qt_disp(QT_DISP_CONSUME_MODE_CMD,&data[1],1);  //回显消费模式到QT
			
			break;
	}
	
}

/*@brief 线程接收qt主界面发送的数据
 */
static void *app_ykt_get_data_qt_thread(void)
{
	int receivedatalen,i;
	while(1)
	{
	//	bzero(recv_msg, BUFFER_SIZE);    
		receivedatalen = recvfrom(qtconsumeClitfd, recv_msg, BUFFER_SIZE, 0, (struct sockaddr *)&qtaddress, &qt_len);  
		if(receivedatalen)
		{		
			debug("recv_qt_msg == ");
			for(i=0;i<receivedatalen;i++)
			{
				debug("%2X ", recv_msg[i]);
			}
			debug("\n");
			//qt数据解析
			app_ykt_response_qt_data(recv_msg);
		}
		else
		{
		//重新创建连接
			//debug("未接收到qt数据");		
		}
	}	
}



/*@brief TCPSERVER线程实时查询新接进来的客户端连接
 */
static void *app_ykt_tcpserver_accept_thread(void)
{
  
	
}
/********************************************************
//卡余额诊断
********************************************************/
uint8_t static CardBalanceDiag(void)
{
	uint32_t	iii;
	uint32_t	jjj;

	if (CurrentConsumMoney>999999)//单次规划限额9999.99
		return	CARD_LITTLEMONEY;

	iii=ChgBCDStringToUlong(OldBalance,4);
	jjj=ChgBCDStringToUlong(CardMinBalance,3);
	jjj+=CurrentConsumMoney;
	debug("iii==%d,jjj==%d\r\n",iii,jjj);
	if (jjj>iii)                //余额不足
		return	CARD_LITTLEMONEY;


	iii=Max_ConsumMoney;//单笔限额
	if (CurrentConsumMoney>iii)
		return	CARD_CONSUM_OVER;

	jjj=Limit_DayMoney;//日限额
	if (CurrentConsumMoney>jjj)
		return	CARD_CONSUM_OVER;

	iii=CardDayConsumMoney;//日累计
	if(Limit_MoneySign>1)//日限额标志
	{
		if ( !memcmp(CardConsumDate,SysTimeDatas.TimeString,3) ) 
		{			
			iii+=CurrentConsumMoney;
			if (iii>jjj)
				return	CARD_CONSUM_OVER;//超出消费限额
		}
	}
	else if(Limit_MoneySign==1)//月限额标志
	{
		if ( !memcmp(CardConsumDate,SysTimeDatas.TimeString,2) ) 
		{
			iii+=CurrentConsumMoney;
			if (iii>jjj)
				return	CARD_CONSUM_OVER;//超出消费限额
		}
	}
	return	0;
}
/****************************************************
存储消费记录
*****************************/
static uint8_t 	app_ykt_save_record(uint8_t bbit)
{
	uint8_t		aaa[4],buffer[6];
	uint8_t 	*tempbuf, *ptr;
	uint16_t	ii;
	uint32_t	iii;
	uint32_t	SumMoney;
	struct sRecordStruct sRt;

	
	ptr = malloc(sizeof(char)*64);
	if (!bbit)
		SumMoney=SumConsumMoney;
	else
		SumMoney=CurrentConsumMoney;
//	if (!SumMoney && !bitConsumZeroEnable)
//		return	 1;	

	if (!bbit)
		ptr[1]=0;//0==消费记录 1==存款记录 2== 补贴记录
	else
	{
		if(WriteCardSta==1)
			return	1;
		else if(WriteCardSta)
			ptr[1]=0x80;//异常记录
	}	
	//sRt.recordTag = malloc(sizeof(char)*4);
	sRt.recordTag = 0;

	ptr[0]=0xa0;	
	if (ConsumMode==CONSUM_NUM)
	{
		if (MenuSort>70)
			MenuSort=0;	
		ptr[2]=MenuSort;
	}
	else if (ConsumMode==CONSUM_RATION)
		ptr[2]=0;//
	else
		ptr[2]=0xfe;//
	memcpy(ptr+4,CardSerialNumBak,4);//
	memcpy(ptr+8,CardPrinterNum+1,3);//

	if(SumMoney)//
	{
	    ptr[3]=PurseUsingNum;//钱包号

		memcpy(ptr+11,OldBalance,4);//

		ChgUlongToBCDString(SumMoney,aaa,4);
		sRt.CurrentConsumMoney = SumMoney;
		memcpy(ptr+15,aaa+1,3);//消费额

		iii=ChgBCDStringToUlong(OldBalance,4);
		iii-=SumMoney;
		ChgUlongToBCDString(iii,ptr+18,4);//

		ii=GetU16_HiLo(PurseContrlNum)+1;
		ii = DoubleBigToSmall(ii);
		memcpy(ptr+26,(uchar *)&ii,2);//钱包流水号
		PosConsumCount++;
		PosConsumCount = DoubleBigToSmall(PosConsumCount);	//adlc
		memcpy(ptr+28,(uchar *)&PosConsumCount,2);//设备流水号
		PosConsumCount = DoubleBigToSmall(PosConsumCount);	//adlc
	}
	else
	{
	    ptr[3]=0;//限次模式
		memset(ptr+11,0,11);
		memset(ptr+26,0,4);
	}
	//消费时间
	//sRt.ConsumeTime = malloc(sizeof(char)*12);
//	timestruct.S_Time.YearChar   = gdHexToBCD(timestruct.S_Time.YearChar);
//	timestruct.S_Time.MonthChar  = gdHexToBCD(timestruct.S_Time.MonthChar);
//	timestruct.S_Time.DayChar    = gdHexToBCD(timestruct.S_Time.DayChar);
//	timestruct.S_Time.HourChar   = gdHexToBCD(timestruct.S_Time.HourChar);
//	timestruct.S_Time.MinuteChar = gdHexToBCD(timestruct.S_Time.MinuteChar);
//	timestruct.S_Time.SecondChar
	SysTimeDatas = lib_systime_get_systime();
	memcpy(CardConsumTime,SysTimeDatas.TimeString,6);
	
	gdHexGroupToHexString(CardConsumTime,buffer,3);
	sRt.ConsumeTime = atoi(buffer);
	ChgTimeToRecordDatas(CardConsumTime,ptr+22);
	debug("CardConsumTime =%2X %2X %2X %2X",ptr[23],ptr[24],ptr[25],ptr[26]);
	
	ptr[30]=0xff;
	ptr[31]=CalCheckSum(ptr+1,30);
	tempbuf = malloc(sizeof(char)*64);
	//sRt.recoedDatas =malloc(sizeof(char)*80);
	gdHexGroupToHexString(ptr,sRt.recoedDatas,32);
	debug("sRt.recoedDatas=%s\r\n",sRt.recoedDatas);

	sRt.recordId = SaveRecordIndex;//存储记录指针
	SaveRecordIndex++;

	sqlite3_consume_insert_db(sRt);//插入消费记录到数据库
	free(tempbuf);

	free(ptr);
	return	0;
}

/**********************************************************************************
 一卡通写卡模式消费流程
 *********************************************************************************/
static void app_ykt_consume(void)
{
	uint8_t status;
	uint8_t tempbuf[50];
	uint16_t i=0,j;
	uint32_t iii=0;

	switch(ConsumCase)
	{
		case 0://初始化读卡消费变量
		ConsumCase=1;
		SumConsumMoney=0;       //总消费
		//CurrentConsumMoney=0;   //消费额
		break;

		case 1:
		status=RequestCard();//卡复位信息
		if(!status)//有卡
		{
			ConsumCase = 2;
		}
		break;

		case 2://寻卡并读取卡片信息
//		status=RequestCard();//卡复位信息
//		if(!status)//有卡
		{//选择一卡通文件，读公共信息和累计文件			
			status=ReadCardCommonDatas();
			debug("status =%d \r\n",status);
			if(!status)
			{	
				//Card_Rebate = Read_Rebate();//读出折扣值No_Use
				bitCommStatus=0;
				//找消费钱包号，pin校验读出余额
			    status=SearchPurseBalnanceDatas(CardIdentity,0,&PurseUsingNum,&SelectPurseNum);
				
				iii = gdChgBCDStringTouint32_t(OldBalance,4);
				debug("OldBalance= %d\r\n",iii);
				memcpy(CardSerialNumBak,CardSerialNum,4);
				if(!CurrentConsumMoney)//查询余额
				{	
					if(appYktCxt.consumeMode == CONSUM_PLAN)//价格方案
					{
						status = CurrentConsumMoneyDiag(CardIdentity);
						if(status)	
						{	
							ConsumCase=5;//等待拔卡
							debug("价格方案err =%d\n",status);
							tempbuf[0] = status;
							app_ykt_qt_disp(QT_DISP_ERROR_CODE_CMD,tempbuf,1);  //显示错误码
							app_ykt_player(0xA0 +status);//语音播放错误码
							bitCommStatus=1;
							
						}
						else
							ConsumCase = 3;
					}
					else
					{
						tempbuf[0] = 0xff;//查询余额
						tempbuf[1] = iii>>24;
						tempbuf[2] = iii>>16;
						tempbuf[3] = iii>>8;
						tempbuf[4] = iii;
						memset(tempbuf+5,0,4);
						memcpy(tempbuf+9,CardPrinterNum,4);
						memcpy(tempbuf+13,nameBuf,nameBuf[0]+1); 			
						app_ykt_qt_disp(QT_DISP_BALANCE_CMD,tempbuf,29);//QT显示余额
						app_ykt_player(QT_CONSUME_QUERY_OK);//查询成功
						ConsumCase=5;//等待拔卡
					}
	
				}
				else//消费模式
				{
					ConsumCase = 3;
				}
			}
			else
			{
				ConsumCase = 5;//等待取卡
				tempbuf[0] = status;
				app_ykt_qt_disp(QT_DISP_ERROR_CODE_CMD,tempbuf,1);  //QT显示错误码
				app_ykt_player(0xA0 +status);//语音播放错误码
				bitCommStatus=1;
			}
		}
		break;

		case 3://消费额诊断
		status=CardBalanceDiag();
		if(status)//卡的余额诊断
		{
			tempbuf[0] = status;
			app_ykt_qt_disp(QT_DISP_ERROR_CODE_CMD,tempbuf,1);  //QT显示错误码
			app_ykt_player(0xA0 +status);//语音播放错误码
			debug("卡诊断失败=%d",status);
			ConsumCase = 5;//等待取卡
		}
		else//开始扣款操作
		{
			hw_pcf8563_get_sysdate(SysTimeDatas.TimeString);
			debug("SysTimeDatas.TimeString==%2x %2x %2x %2x %2x %2x\n",
				SysTimeDatas.TimeString[0],SysTimeDatas.TimeString[1],SysTimeDatas.TimeString[2],
				SysTimeDatas.TimeString[3],SysTimeDatas.TimeString[4],SysTimeDatas.TimeString[5]);
			status=ConsumPocess();//消费
			if (!status)
			{	
				iii = gdChgBCDStringTouint32_t(NewBalance,4);
				SumConsumMoney=CurrentConsumMoney;
				debug("NewBalance= %d\r\n",iii);
				tempbuf[0] = 0;
				tempbuf[1] = iii>>24;
				tempbuf[2] = iii>>16;
				tempbuf[3] = iii>>8;
				tempbuf[4] = iii;

				iii = CurrentConsumMoney;
				debug("CurrentConsumMoney= %d\r\n",iii);
				tempbuf[5] = iii>>24;
				tempbuf[6] = iii>>16;
				tempbuf[7] = iii>>8;
				tempbuf[8] = iii;
				memcpy(tempbuf+9,CardPrinterNum,4);
				memcpy(tempbuf+13,nameBuf,nameBuf[0]+1); 	
				app_ykt_save_record(0);	//存储消费记录			
				app_ykt_qt_disp(QT_DISP_BALANCE_CMD,tempbuf,29);  //QT显示余额
				app_ykt_player(QT_CONSUME_OK);//消费成功QT_CARD_BATCH_ERROR
				ConsumCase=5;//等待拔卡
			}
			else
			{
				ConsumCase=4;//等待拔卡
				app_ykt_player(QT_CONSUME_FAIL);//扣款失败
				debug("扣款失败重新放卡\r\n");
			}
		}		
		break;

		case 4://扣款失败重新放卡
		status=CheckCardPrinterNum();
		if(!status)
		{
			status=ReWriteCardSub_M1(0);		
		}//
		else
			ConsumCase = 0;//新卡进场重新进入消费流程
		if(!status)
		{
			iii = gdChgBCDStringTouint32_t(NewBalance,4);
			SumConsumMoney=CurrentConsumMoney;
			debug("NewBalance= %d\r\n",iii);
			tempbuf[0] = 0;
			tempbuf[1] = iii>>24;
			tempbuf[2] = iii>>16;
			tempbuf[3] = iii>>8;
			tempbuf[4] = iii;
			memcpy(tempbuf+5,CardPrinterNum,4);
			memcpy(tempbuf+9,nameBuf,nameBuf[0]+1); 	
			app_ykt_save_record(0);	//存储消费记录			
			app_ykt_qt_disp(QT_DISP_BALANCE_CMD,tempbuf,25);  //QT显示余额
			app_ykt_player(QT_CONSUME_OK);//消费成功
			ConsumCase=5;//等待拔卡	
		}
		else
		{	
			ConsumCase = 5;//等待拔卡
		}		
		break;
			
		case 5://等待卡拿开	
		if(!RequestCard())//寻到卡
		{			
			//连续刷卡测试
			#ifdef CON_SWIPE_CARD
			{			
				debug("卡片已离场\n");
				ConsumCase = 0;//
				
				if(appYktCxt.consumeMode == CONSUM_MONEY)//计算模式
					appYktCxt.CurrentConsumMoney = 0;
				recv_msg[0] = 1;//卡片已离场
				app_ykt_qt_disp(QT_DISP_CLEAR_CMD,recv_msg,1);  //QT清空显示
					
			}
			#endif	
		}
		else
		{
			debug("卡片已离场\n");
			ConsumCase = 0;//
			recv_msg[0] = 1;//卡片已离场
			if(!bitCommStatus)//无错误弹窗
				app_ykt_qt_disp(QT_DISP_CLEAR_CMD,recv_msg,1);  //QT清空显示
			appYktCxt.posServerCmd = YKT_IDLE_CMD;
			if(appYktCxt.consumeMode == CONSUM_MONEY ||appYktCxt.consumeMode == CONSUM_PLAN)//计算或者价格方案模式
				CurrentConsumMoney = 0;
		}			
		break;
	}
}

//监听posserver发过来的数据线程
static void app_ykt_get_serverdata_thread(void)
{
	//uint8_t Recdata[512];
	uint16_t recLength,i;
	
	
	while(1)
	{
		//加锁
		pthread_mutex_lock(&mutex);

	//	debug("app_ykt_get_serverdata_thread \r\n");
		recLength = UdpSocketRecDataFromPosServer(SerialUnion.S_DatasBuffer, 512) ;		

		SerialUnion.S_DatasStruct.UartReAddrCode = DoubleBigToSmall(SerialUnion.S_DatasStruct.UartReAddrCode);
		SerialUnion.S_DatasStruct.UartComd = DoubleBigToSmall(SerialUnion.S_DatasStruct.UartComd);
		SerialUnion.S_DatasStruct.UartSeAddrCode = DoubleBigToSmall(SerialUnion.S_DatasStruct.UartSeAddrCode);

		debug("SerialUnion.S_DatasStruct.UartSeAddrCode == %d\r\n",SerialUnion.S_DatasStruct.UartSeAddrCode);
		debug("SerialUnion.S_DatasStruct.UartReAddrCode == %d\r\n",SerialUnion.S_DatasStruct.UartReAddrCode);
		debug("SerialUnion.S_DatasStruct.UartComd == 	   %d\r\n",SerialUnion.S_DatasStruct.UartComd);
		debug("SerialUnion.S_DatasStruct.UartStatus ==    %2X\r\n",SerialUnion.S_DatasStruct.UartStatus);
		debug("SerialUnion.S_DatasStruct.UartAddrH ==     %2X\r\n",SerialUnion.S_DatasStruct.UartAddrH);
		debug("SerialUnion.S_DatasStruct.UartAddrL ==     %2X\r\n",SerialUnion.S_DatasStruct.UartAddrL);
		debug("SerialUnion.S_DatasStruct.UartFrameNum ==  %2X\r\n",SerialUnion.S_DatasStruct.UartFrameNum);
		debug("SerialUnion.S_DatasStruct.DatasLen ==      %2X\r\n",SerialUnion.S_DatasStruct.DatasLen);
		debug("udp rec data ==");
		for(i=0;i<SerialUnion.S_DatasStruct.DatasLen;i++)
		{
			debug("%2X ",SerialUnion.S_DatasStruct.Datas[i]);
		}
		debug("\r\n");

		if(MainCode ==SerialUnion.S_DatasStruct.UartReAddrCode |SerialUnion.S_DatasStruct.UartComd==RD_ADDR_COMD)
		{
			//while(ConsumCase >1);
			appYktCxt.breadDataFronSocket = true;
//			//命令处理
//			ReceiveSub();
//			//数据返回
//			udpSendSub();
		}
		else
			Init_Serial();
		
		//解锁
		pthread_mutex_unlock(&mutex);

	}
	
}




/**@brief 创建线程
 */
static void app_ykt_create_thread(void)
{
	int temp;
	memset(&thread, 0, sizeof(thread));          //comment1

//	if((temp = pthread_create(&thread[1], NULL, app_ykt_tcpserver_accept_thread, NULL)) != 0)//监听QT过来的socket链接
//			debug("线程1创建失败!\n");
//	else
//			debug("线程1被创建\n");
//		
	if((temp = pthread_create(&thread[2], NULL, app_ykt_get_data_qt_thread, NULL)) != 0)//监听QT过来的数据
			debug("线程2创建失败");
	else
			debug("线程2被创建\n");


	if((temp = pthread_create(&thread[3], NULL, app_ykt_get_serverdata_thread, NULL)) != 0)//监听posserver数据
			debug("线程3创建失败");
	else
			debug("线程3被创建\n");	
}

/**@brief 创建线程等待
 */
static void app_ykt_thread_wait(void)
{
	//等待线程结束
	if(thread[0] !=0)
	{             //comment4    
		pthread_join(thread[0],NULL);
		debug("线程1已经结束\n");
	}
	if(thread[1] !=0) 
	{  
		//comment5
		pthread_join(thread[1],NULL);
		debug("线程2已经结束\n");
	}
	if(thread[2] !=0) 
	{  
		//comment5
		pthread_join(thread[2],NULL);
		debug("线程3已经结束\n");
	}
	if(thread[3] !=0) 
	{  
		//comment5
		pthread_join(thread[3],NULL);
		debug("线程4已经结束\n");
	}
	if(thread[4] !=0) 
	{  
		//comment5
		pthread_join(thread[4],NULL);
		debug("线程5已经结束\n");
	}
}

/**@利用回环地址跟qt创建一个socket用于与qt之间的通讯
 */
static void app_ykt_create_qt_socket(void)
{
	int i,reuse = 1;
	int socket_descriptor; //套接口描述字
	int iter=0;

	//本地端口
	bzero(&qtaddress,sizeof(qtaddress));
	qtaddress.sin_family=AF_INET;
	qtaddress.sin_addr.s_addr=inet_addr(SERV_IP);//这里不一样
	qtaddress.sin_port=htons(SERV_PORT);

	//创建一个 UDP socket
	qtconsumeClitfd=socket(AF_INET,SOCK_DGRAM,0);//IPV4  SOCK_DGRAM 数据报套接字（UDP协议）
	if(bind(qtconsumeClitfd, (struct sockaddr *)&qtaddress, sizeof(qtaddress)))//绑定端口
    {
        printf("qtconsumeClitfdclient bind port failed!\n", __LINE__);
        close(qtconsumeClitfd);//关闭socket
    }

	//qt消费窗口远端端口
	memset(&conremoteAddr, 0, sizeof(struct sockaddr_in));
	conremoteAddr.sin_family = AF_INET;
	conremoteAddr.sin_addr.s_addr = inet_addr(SERV_IP);
	conremoteAddr.sin_port = htons(CONSUMEQTPORT);//UDP 广播包 远端端口

	//qt设置消费模式窗口远端端口
	memset(&conModeremoteAddr, 0, sizeof(struct sockaddr_in));
	conModeremoteAddr.sin_family = AF_INET;
	conModeremoteAddr.sin_addr.s_addr = inet_addr(SERV_IP);
	conModeremoteAddr.sin_port = htons(CONSUMEMODEQTPORT);//UDP 广播包 远端端口quDayremoteAddr

	//qt查询日消费额窗口远端端口
	memset(&quDayremoteAddr, 0, sizeof(struct sockaddr_in));
	quDayremoteAddr.sin_family = AF_INET;
	quDayremoteAddr.sin_addr.s_addr = inet_addr(SERV_IP);
	quDayremoteAddr.sin_port = htons(QUDAYQTPORT);//UDP 广播包 远端端口

	//qt查询未采集记录窗口远端端口
	memset(&quRecordremoteAddr, 0, sizeof(struct sockaddr_in));
	quRecordremoteAddr.sin_family = AF_INET;
	quRecordremoteAddr.sin_addr.s_addr = inet_addr(SERV_IP);
	quRecordremoteAddr.sin_port = htons(QURECORDQTPORT);//UDP 广播包 远端端口
	

	debug("qtconsumeClitfd bind ok\n");	
}

//定时发送心跳函数
static void app_ykt_heart(void )
{	
	//读取消费模式
//	readPosConsumeMode(consumeModeMessage);
//	appYktCxt.consumeMode = atoi(consumeModeMessage.ConsumeMode);
//	if(appYktCxt.consumeMode ==CONSUM_RATION)
//		appYktCxt.CurrentConsumMoney = atoi(consumeModeMessage.Money);
//	if(appYktCxt.consumeMode ==CONSUM_MONEY)
//		appYktCxt.CurrentConsumMoney = 0;
//	debug("消费模式=%d,消费金额 =%d\n",appYktCxt.consumeMode,appYktCxt.CurrentConsumMoney);
//	recv_msg[0] = appYktCxt.consumeMode;
//	recv_msg[1] = appYktCxt.CurrentConsumMoney>>24;
//	recv_msg[2] = appYktCxt.CurrentConsumMoney>>16;
//	recv_msg[3] = appYktCxt.CurrentConsumMoney>>8;
//	recv_msg[4] = appYktCxt.CurrentConsumMoney;
//	app_ykt_qt_disp(QT_DISP_CONSUME_MODE_CMD,recv_msg,5);  //回显消费模式到QT
}

//通讯超时计数器累计定时器
static void app_ykt_tcp_out_timer(void )
{
	uint8_t status;

	//检测电池状态
	appYktCxt.batteryStatus = battery_read_satatus();
	//debug("电池状态==%d\n",appYktCxt.batteryStatus);

	if(appYktCxt.batteryStatus)
		recv_msg[0] = 2;//电源供电状态
	else
		recv_msg[0] = 3;//电池供电状态

//	if(appYktCxt.batteryStatus != status)
//		app_ykt_qt_disp(QT_DISP_CLEAR_CMD,recv_msg,1);  //QT清空显示
	appYktCxt.batteryStatus= status ; 

}

/**@brief Function for initializing ccb dc-ep applicaiton.
 */
static void app_ykt_init(void)
{	
	int i,status;
	char data[30],Buffer[20];
	char blknumber[] = "11223344";
	uint32_t temp;
	struct	sRecordMoneyStruct RecordStr;
	
	//初始化RC522
	InitRc522Driver();
	MFRC522_Initializtion();
	//初始化消费变量
	appYktCxt.consumeMode = CONSUM_MONEY;//默认计算模式
	appYktCxt.netstatus =0;//网络未连接
	appYktCxt.bsocketconnectOk =false;//初始化socket状态
	appYktCxt.offline_UpIndex = 0;//脱机记录指针
	appYktCxt.breadDataFronSocket =false;
	
	//设备配置信息分配地址空间
	posIpMessage.localip		=(char *)malloc(sizeof(char)*50);
	posIpMessage.gateway		=(char *)malloc(sizeof(char)*50);
	posIpMessage.mask			=(char *)malloc(sizeof(char)*50);
	posIpMessage.routeip		=(char *)malloc(sizeof(char)*50);
	posIpMessage.port			=(char *)malloc(sizeof(char)*50);
	posIpMessage.maincode		=(char *)malloc(sizeof(char)*50);
	posIpMessage.stationCode	=(char *)malloc(sizeof(char)*50);
	posIpMessage.noNetconsume	=(char *)malloc(sizeof(char)*5);

	posMessage.CardSector 		= (char *)malloc(sizeof(char)*50);
	posMessage.CardRebate 		= (char *)malloc(sizeof(char)*10);
	posMessage.MatchCode 		= (char *)malloc(sizeof(char)*50);
	posMessage.CardKeyCode 		= (char *)malloc(sizeof(char)*50);
	posMessage.CalCardKey 		= (char *)malloc(sizeof(char)*50);
	posMessage.CardMinBalance 	= (char *)malloc(sizeof(char)*50);
	posMessage.DayLimetMoney 	= (char *)malloc(sizeof(char)*50);
	posMessage.CommEncryptKey 	= (char *)malloc(sizeof(char)*50);
	posMessage.CardBatchEnable 	= (char *)malloc(sizeof(char)*50);
	posMessage.PurseEnable 		= (char *)malloc(sizeof(char)*50);
	
	consumeModeMessage.ConsumeMode = (char *)malloc(sizeof(char)*50);
	consumeModeMessage.Money       = (char *)malloc(sizeof(char)*50);
	//读取设备信息
	readIp(posIpMessage);

	//设置设备IP地址信息
	SetIfAddr(ifname,posIpMessage.localip,posIpMessage.mask,posIpMessage.gateway);
	gdHexStringToHexGroup(posIpMessage.maincode, Buffer, 4);
	MainCode = atoi(posIpMessage.maincode);
	debug("站点号:%s\r\n",posIpMessage.stationCode);
	debug("设备编号:%s %d\r\n",posIpMessage.maincode,MainCode);
	
	//读取配置信息
	readposInformation(posMessage);	
	debug("readposInformation 01\r\n");
	gdHexStringToHexGroup(posMessage.CardSector, Buffer, 1);
	CardSector = Buffer[0];//公共扇区号
	debug("CardSector = %d\r\n",CardSector);
	
//	/*gdHexStringToHexGroup(posMessage.CardRebate, Buffer, 3);
//	memcpy(CardRebate,Buffer,3);//卡折扣
//	debug("CardRebate[0]==%2X %2X %2X\r\n",CardRebate[0],CardRebate[1],CardRebate[2]);*/
	
	gdHexStringToHexGroup(posMessage.MatchCode, Buffer, 4);
	memcpy(MatchCode,Buffer,4);//卡匹配字
	debug("MatchCode[0]==%2X %2X %2X %2X\r\n",MatchCode[0],MatchCode[1],MatchCode[2],MatchCode[3]);
	
	gdHexStringToHexGroup(posMessage.CardKeyCode, Buffer, 6);
	memcpy(CardKeyCode,Buffer,6);//读卡密码
	debug("CardKeyCode[0]==%2X %2X %2X %2X %2X %2X\r\n",CardKeyCode[0],CardKeyCode[1],CardKeyCode[2],
														CardKeyCode[3],CardKeyCode[4],CardKeyCode[5]);
	
	gdHexStringToHexGroup(posMessage.CalCardKey, Buffer, 8);
	memcpy(CalCardKey,Buffer,8);//写卡密钥
	debug("CalCardKey[0]==%2X %2X %2X %2X %2X %2X %2X %2X\r\n",CalCardKey[0],CalCardKey[1],CalCardKey[2],CalCardKey[3],
															   CalCardKey[4],CalCardKey[5],CalCardKey[6],CalCardKey[7]);
	
	gdHexStringToHexGroup(posMessage.CardMinBalance, Buffer, 3);
	memcpy(CardMinBalance,Buffer,3);//卡底金
	debug("CardMinBalance[0]==%2X %2X %2X\r\n",CardMinBalance[0],CardMinBalance[1],CardMinBalance[2]);
	
	gdHexStringToHexGroup(posMessage.DayLimetMoney, Buffer, 3);
	memcpy(DayLimetMoney,Buffer,3);//日消费限额
	debug("DayLimetMoney[0]==%2X %2X %2X\r\n",DayLimetMoney[0],DayLimetMoney[1],DayLimetMoney[2]);
	
	gdHexStringToHexGroup(posMessage.CommEncryptKey, Buffer, 8);
	memcpy(CommEncryptKey,Buffer,8);//传输密钥
	debug("CommEncryptKey[0]==%2X %2X %2X %2X %2X %2X %2X %2X\r\n",CommEncryptKey[0],CommEncryptKey[1],CommEncryptKey[2],CommEncryptKey[3],
																   CommEncryptKey[4],CommEncryptKey[5],CommEncryptKey[6],CommEncryptKey[7]);
	
	gdHexStringToHexGroup(posMessage.CardBatchEnable, Buffer, 32);
	memcpy(CardBatchEnable,Buffer,32);//卡批次
	debug("卡批次==");
	for(i=0;i<32;i++)
	{
		debug("%2X ",CardBatchEnable[i]);
	}
	debug("\r\n");
	gdHexStringToHexGroup(posMessage.PurseEnable, Buffer, 10);
	memcpy(PurseEnable,Buffer,10);//钱包号
	debug("钱包号==");
	for(i=0;i<32;i++)
	{
		debug("%2X ",PurseEnable[i]);
	}
	debug("\r\n");
	
	/*
	CardSector = 2;//公共扇区号第二扇区
	memcpy(CardKeyCode,"\xA0\xA1\xA2\xA3\xA4\xA5",6);//读卡密码
	memcpy(CalCardKey,"\x87\x65\x43\x21\x87\x65\x43\x21",8);//读卡秘钥
	for(i=0;i<32;i++)
		CardBatchEnable[i] =1;//卡批次都有效
	PursesSector[0] =3;//钱包0扇区号
	PursesSector[1] =4;//钱包1扇区号*/

	//创建语音播放消息队列
	app_ykt_create_aplay_msg();	
	//创建tcpsocket跟qt通讯
	app_ykt_create_qt_socket();
	//创建跟PosServer通讯的TCP链接
	MakePosServerUdpSocket(posIpMessage.routeip,gdstringToInt(posIpMessage.port));

	//读取消费模式
	readPosConsumeMode(consumeModeMessage);
	appYktCxt.consumeMode = atoi(consumeModeMessage.ConsumeMode);
//	if(appYktCxt.consumeMode ==CONSUM_RATION)
//		CurrentConsumMoney = atoi(consumeModeMessage.Money);
//	if(appYktCxt.consumeMode ==CONSUM_MONEY)
//		CurrentConsumMoney = 0;
	recv_msg[0] = appYktCxt.consumeMode;
	app_ykt_qt_disp(QT_DISP_CONSUME_MODE_CMD,recv_msg,1);  //回显消费模式到QT
	
	/* 对互斥锁进行初始化*/
	pthread_mutex_init(&mutex, NULL);
	//线程创建
	app_ykt_create_thread();	
	//初始化电池检测
	battery_init();
	//设置定时器发送心跳函数超时时间2S
	lib_systime_creat_timer(1000,app_ykt_heart);
	//创建通讯超时定时器10ms
	lib_systime_creat_timer(10,app_ykt_tcp_out_timer);
	lib_systime_start_timer();

	//创建数据库	
	status = sqlite3_consume_open_db();
	status = sqlite3_blaknumber_open_db();
	status = sqlite3_moneyplan_open_db();
	//readtexttest();
	//sqlite3_test();
	//sqlite3_test_gzf();
	//未采记录指针
	NoCollectRecordIndex = sqlite3_consume_query_RecordIndex_db(0);
	debug("未采记录指针=%d \r\n",NoCollectRecordIndex);
	//存储记录指针
	SaveRecordIndex = sqlite3_consume_query_RecordIndex_db(1);
	debug("存储记录指针=%d \r\n",SaveRecordIndex);
	//价格方案记录指针
	MoneyPlanIndex = sqlite3_query_moneyplan_db();
	debug("价格方案记录指针=%d \r\n",MoneyPlanIndex);

	//app_ykt_save_record(0);	//存储消费记录	
	//sqlite3_test(recod_db);

	//init pcf8563
	hw_pcf8563_init();
	//memcpy(data,"\x20\x09\x20\x12\x36\x15",6);
	//hw_pcf8563_set_sysdate(data);
	hw_pcf8563_get_sysdate(Buffer);
	debug("pcf8563time==%2x %2x %2x %2x %2x %2x\n",Buffer[0],Buffer[1],Buffer[2],Buffer[3],Buffer[4],Buffer[5]);
	debug("版本号：002\n");


//	system("echo 0 > /sys/class/gpio_sw/PF3/data");
//	sleep(1);
//	system("echo 1 > /sys/class/gpio_sw/PF3/data");
//	sleep(1);
}

int app_ykt_main(void)
{  	
	char keyvalue;
	char sendbuf[10];
	int i,recLength,timebuf[6];
	
	app_ykt_init();
	
	//获取系统时间
	hw_pcf8563_get_sysdate(SysTimeDatas.TimeString);
	timebuf[0] = 2000+gdBCDToHex(SysTimeDatas.S_Time.YearChar);
	timebuf[1] = gdBCDToHex(SysTimeDatas.S_Time.MonthChar);
	timebuf[2] = gdBCDToHex(SysTimeDatas.S_Time.DayChar);
	timebuf[3] = gdBCDToHex(SysTimeDatas.S_Time.HourChar);
	timebuf[4] = gdBCDToHex(SysTimeDatas.S_Time.MinuteChar);
	timebuf[5] = gdBCDToHex(SysTimeDatas.S_Time.SecondChar);

	debug("timebuf== %d %d %d %d %d %d",timebuf[0],timebuf[1],timebuf[2],timebuf[3],timebuf[4],timebuf[5]);	
	lib_systime_set_systime(timebuf[0],timebuf[1],timebuf[2],timebuf[3],timebuf[4],timebuf[5]);
	//SysTimeDatas = lib_systime_get_systime();
	debug("systime == %2X %2X %2X %2X %2X %2X \n",
		   SysTimeDatas.S_Time.YearChar,SysTimeDatas.S_Time.MonthChar,SysTimeDatas.S_Time.DayChar,
		   SysTimeDatas.S_Time.HourChar,SysTimeDatas.S_Time.MinuteChar,SysTimeDatas.S_Time.SecondChar);

	while(1)
	{
		//加锁
		//pthread_mutex_lock(&mutex);
		//卡消费操作
		app_ykt_consume();	
		if(appYktCxt.breadDataFronSocket ==true)
		{
			appYktCxt.breadDataFronSocket =false;
			//命令处理
			ReceiveSub();
			//数据返回
			udpSendSub();
		}
		//加锁
		//pthread_mutex_unlock(&mutex);
	
	}
	//线程创建等待
	app_ykt_thread_wait();
	return 0;  
}  
