#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


#ifndef __MYDEFINE_H
#define __MYDEFINE_H

#define		ulong	unsigned 	long
#define		uint	unsigned  short
#define		uchar	unsigned  char


struct	sDatasStruct//通讯数据格式
{
	uint16_t	UartSeAddrCode;
	uint16_t	UartReAddrCode;
	uint16_t	UartComd;
	uchar	UartStatus;
	unsigned char	UartAddrH;//地址高
	unsigned char	UartAddrL;//地址低
	unsigned char	UartFrameNum;//帧号
	unsigned char	DatasLen;//数据长度
	unsigned char	Datas[139];//数据包
};
union	sDatasUnion
{
	unsigned char	S_DatasBuffer[150];
	struct	sDatasStruct	S_DatasStruct;
};


#define		MAXCARDPRINTERNUM	262144 //最大卡号
#define MAXRECORD_NUM  655340//最大交易记录

////错误信息定义
#define		SYS_OK					0
#define		CARD_OK					0
#define		SYS_RC500_ERROR			1//读卡模块 
#define		SYS_FLASH_ERROR			2//FLASH
#define		SYS_24C256_ERROR		3//24C256
#define		SYS_24C64_ERROR			4//24C64
#define		CARD_BATCH_ERROR		5//批次
#define		CARD_NUMBER_ERROR		6//卡号不合法(1.BCD 2.超范围)
#define		CARD_LOSTCARDNUM		7//挂失卡
#define		CARD_DATA_ERROR			8//用户数据错误(1.卡数据校验和错2.卡折扣为0)
#define		CARD_LITTLEMONEY		9//金额不足
#define		CARD_OVERDATE			10//1.卡过期2.卡未启用3.黑名单有效月4.钱包有效期5.日期格式
#define		SYS_SAVERECORDFULL		11//记录满
#define   	SYS_PINCHECK_FAIL   	12//系统校验失败
#define		CARD_LIMIT_NUM			13//限次(1.无匹配方案2.限次)
#define		CARD_CONSUM_OVER		14//超出消费限额(1.单笔2.规划范围3.日限4.月限)
#define		SYS_8563_ERROR			15//上电检测时钟模块
#define		NETCARD_ERROR			16//TCP网卡初试化失败
#define		TIME_ERROR				17//消费中检测合法性
#define		CARD_STATUS_ERROR		18 //卡状态错误


//zjx_change_mark
//新添加的错误
#define     MATCHCODE_ERR        19//PSAM初始化错误(1.复位2.PPS失败3.一卡通目录4.触点接触不良)
#define     CPU_SELFILE_FAIL        20//选文件失败(1.主目录2.一卡通应用3.钱包文件)
#define     CPU_READFILE_FAIL       21//读文件失败(1.公共信息2.累计文件3.记录)
#define     CPU_REDABALANCE_FAIL    22//读余额失败
#define			Uncert_State						23//写卡、交易失败
#define     CARD_SAME_ERROR	        24//非同一张卡错误(1.序列号2.卡印刷号)
#define 		GET_PROVE_FAIL		    	25//取交易认证失败
#define     PSAM_MAC1_FAIL		    	26//MAC1
#define     PSAM_CHECK_MAC2		    	27//MAC2
#define	    PSAM_RESET_ERROR          28//匹配字错
#define	    PSAM_FILEPARA_ERR       29//PSAM错误(1.一卡通目录2.参数读取3.站点为0)
#define			NO_PURSE_ERROR					30 //钱包没设置
#define			SYS_CONSUM_ESC					31 //钱包没设置
#define			No_Use					        32 //不允许此身份消费
#define			No_ip					        	33 //IP丢失

#define     Consume_Ok             16
#define     Consume_Err            17
#define     XP_Start               18
#define     Clr_Pos                20
#define     Real_Ok                21

//不显示的状态
#define     PSAM_COM_ERROR          0xf1//PSAM通讯错误(1.长度2.无响应3.响应错误) 
#define     CPU_WRITEPURSE_FAIL     0xf2//写累计失败
#define     CONSUM_PROCE_FAIL	    	0xf3//交易失败
#define			CARD_NOCARD							0xff


/////////////通讯命令定义//////////////////////
#define		RD_ADDR_COMD	   	    0x30//读站点

#define   DOWNLODER_COMD        0x19  //下载

#define		POS_RST_COMD			0x10//POS复位
#define		SET_BATCH_COMD		0xb1//设置批次是否有效
#define		RD_BATCH_COMD			0xb2//读出批次是否有效
//=============================================================
#define		RD_USERCODE_COMD			0x32//上传匹配字
#define		SET_USERCODE_COMD			0x22//匹配字下载
#define		SET_CALCARDKEY_COMD		0xb3 //下载写卡的加密密钥
#define		RD_CALCARDKEY_COMD		0xb4 //读出写卡的加密密钥
#define		SET_RDCARDCODE_COMD		0x6c//下载读卡密码
#define		RD_RDCARDCODE_COMD		0x5c//读出读卡密码
#define		RD_COMMSECTOR_COMD		0x7c//读出公共区的扇区号			
#define		SET_COMMSECTOR_COMD		0x7d//设置公共区的山区号
//=============================================================
#define		RD_MINMONEY_COMD   			0x33//上传底金
#define		SET_MINMONEY_COMD   		0x23//底金下载
#define		RD_BLKNAME_TIME_COMD    0x5D//读出黑名单有效期
#define		SET_BLKNAMETIME_COMD   	0x6D//下载黑名单有效期
#define		CLR_PURSE_COMD					0x9c
#define		SET_PURSE_COMD					0x9b//设置钱包
#define		RD_TIME2_COMD   				0x3F//查询时间，
#define		SET_TIME2_COMD   				0x2F//时间下载，
#define		ADD_BLKNUM_COMD					0x2c//增加黑名单
#define		DEL_BLKNUM_COMD					0x4c//删除黑名单
#define		CLR_BLKNUM_COMD					0x44//清除黑名单
#define		DEL_ADD_BLKNUM_COMD			0x4d//加或减黑名单
#define		CLR_POSDATAS_COMD				0x4a//清除POS数据
#define		RD_POSSTATUS_COMD				0x71//读出POS状态
#define		RD_RECORD_COMD   				0x37//上传记录
#define		DEL_RECORD_COMD   			0x47//删除已上传的记录
#define		INIT_RECORD_PTR_COMD   	0x5B//初始化复采指针
#define		RD_RERECORD_COMD	    	0x3C//复采记录
#define		DEL_RERECORD_COMD  			0x48//复采移指针
#define		RD_CONSUMMONEY_COMD			0x101//读出消费额
#define		SET_ENCRYPTKEY_COMD			0x102//下载传输密钥
#define		RD_ENCRYPTKEY_COMD			0x103//读出传输密钥
#define		LOAD_PROGAM_COMD  			0xf0//下载程序
#define		SET_MENU_COMD						0x126 //设置菜单
#define		CLR_NENU_COMD						0x146 //清菜单
#define		RD_MENU_COMD						0x136 //读出菜单
#define		SET_MENUNAME_COMD				0xdd//设置菜名
#define		CLR_MENUNAME_COMD				0xde//清除菜名
#define		SET_SORTPRINCE_COMD		0x95 //下载价格方案
#define		RD_SORTPRINCE_COMD		0x96 //读出价格方案
#define		CLR_SORTPRINCE_COMD		0x97 //清除价格方案
///AD by lc增加折扣费率命令
#define		UP_REBATE_COMD		0xA5 //上传折扣费率
#define		SET_REBATE_COMD		0xA4 //下载折扣费率
#define		CLR_REBATE_COMD		0xA3 //清除折扣费率
#define		SET_DAYLIMET_COMD 	0x2e //设置日限额
#define		RD_DAYLIMET_COMD	0x3e //读取日限额

#define		OK					0
#define		ReDatas_Error		1 //接收的数据错
#define		ReComd_Error		2 //接收的命令错
#define		RdDatas_Error   	3//读数据错
#define		WrDatas_Error   	4//写数据错
#define		NoDatas_Error		5//无可读数据
#define		Running_Status		6//正在执行
#define		Running_Error		7//无法执行


#define		CONSUM_SIMPLE		0 //简易
#define		CONSUM_NUM			1 //菜号
#define		CONSUM_RATION		2 //定额
#define		CONSUM_MONEY		3 //金额
#define		CONSUM_MENU			4 //菜单设置方式
#define     CONSUM_PLAN         5//价格方案

#define		DEFAULT_CONSUM_MODE	CONSUM_MONEY //默认的消费方式

#define		STX		2
#define		ETX		3

#endif


