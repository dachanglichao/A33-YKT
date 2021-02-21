#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "mydefine.h"



//云ID交互指令
#define YKT_IDLE_CMD 		        0x0000//空闲
#define YKT_CONNRCT_YUN_CMD 		0xF0F0//连接后台
#define YKT_HEART_CMD 			    0x0065//心跳包获取系统时间
#define YKT_USE_SECTOR_CMD 			0x1002//设置扇区号
#define YKT_USE_ID_CMD 				0x1004//设置用户ID
#define YKT_LIMT_MONEY_CMD 			0x1006//设置脱机限额

#define YKT_GET_BALANCE_CMD 		0x2101//查询余额
#define YKT_CONSUNE_CMD 			0x2102//消费指令
#define YKT_POLL_RECORD_CMD 		0x2103//查询消费记录
#define YKT_UP_RECORD_CMD 		    0x2109//上传脱机记录



//QT显示指令
#define QT_DISP_BALANCE_CMD 		0xA0//QT显示余额
#define QT_DISP_TOAL_MONEY_CMD 		0xA1//QT显示消费记录总额
#define QT_DISP_NET_STATUS_CMD 		0xA2//QT显示网络状态
#define QT_DISP_CLEAR_CMD 			0xA3//QT清空显示
#define QT_DISP_RECORD_MONEY_CMD 	0xA4//QT查询脱机记录
#define QT_SET_CONSUME_MODE_CMD 	0xA5//QT设置消费模式
#define QT_DISP_CONSUME_MODE_CMD 	0xA6//回显消费模式到QT显示
#define QT_DISP_ERROR_CODE_CMD 		0xA8//显示错误码


//QT消息队列
#define QT_MSG_MAIN_WIN            0//跟QT主消费界面创建消息队列

//错误码
#define QT_CONSUME_0      			0
#define QT_CONSUME_1      			1
#define QT_CONSUME_2      			2
#define QT_CONSUME_3      			3
#define QT_CONSUME_4      			4
#define QT_CONSUME_5      			5
#define QT_CONSUME_6      			6
#define QT_CONSUME_7      			7
#define QT_CONSUME_8      			8
#define QT_CONSUME_9      			9
#define QT_CONSUME_D      			10

#define QT_CONSUME_OK      			0xE0//消费成功
#define QT_CONSUME_FAIL    			QT_CONSUME_OK+1//消费失败
#define QT_CONSUME_QUERY_OK         QT_CONSUME_OK+2//查询成功


#define QT_CONSUME_NONET_NO_MONEY   0xf9//脱机钱包余额不足
#define QT_CONSUME_QUERY_FAIL       0xfa//查询失败
#define QT_SQLITE3_ERROR        	0xfb//数据库创建失败
#define QT_CONSUME_NET_ERROR        0xfc//网络错误
#define QT_CONSUME_MONEY         	0xfd//计算模式
#define QT_CONSUME_RATION        	0xfe//自动模式




//QT消息队列数据结构体
struct SendMsgQT
{
	char cmd;
	char data[500]	;
};

//消费模式
//typedef enum{
//	CONSUM_MONEY =0,//计算模式
//	CONSUM_RATION//自动定额模式

//};
int app_ykt_main(void);
