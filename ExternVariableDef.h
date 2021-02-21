
#include <string.h>
#include <stdio.h>
#include "mydefine.h"


extern	uint		MainCode;//站点号
extern  uint		UdpCode;//UDP端口号
extern	unsigned char		bitPinEnable;
extern	unsigned char		PinCheckSta;
extern	unsigned char		PinCount;
extern	unsigned char		bitNeedRST;
extern	unsigned char		bitNeedDownLoad;
extern	unsigned char		bitPurseEnd;

extern	uint				SerialReceiveTimeCount;
extern	unsigned char		TCPDatasComd;
extern	unsigned char		OutChar16C550;
extern	unsigned char		CommModeChar;

extern	unsigned char		Receive_Ip_Port[6];

extern	unsigned char		CtrlChar;//控制字节输出，主要通过573控制蜂鸣器和485通讯的收发转换
extern	unsigned char		LoopCount;
extern	unsigned char		bitUpdateParameter;
extern	unsigned char		bitHaveReadBalance; 
extern	unsigned char		bitSetDateEnable; 
extern	unsigned char		bitWrittingFlash;
extern	unsigned char		bitDispFlash;
extern	unsigned char		bitBeepError;
extern	unsigned char		bitHaveCollectRecord;
extern	unsigned char		bitHaveReCollectRecord;
extern	unsigned char		bitStatus8563;
extern	unsigned char		bitSysTimeDisable;
extern	unsigned char		bitRecordFull;
extern	unsigned char		bitConsumZeroEnable;
extern	unsigned char		bitHaveSeclectPurse;
extern	unsigned char		bitCommStatus;


extern	unsigned char		StatusBak;
extern	unsigned char		DelayReadCardTimeCount;

extern	unsigned char		DownLoadPragamSymbl[6];
extern	unsigned char		LoadProgamReDatas[6];
extern	unsigned char		T3_TimeCount;
extern	unsigned char		bitHaveLedError;
extern	unsigned char		bitNeedDiagRecordFull;

extern	unsigned char		ConsumCase;
extern	unsigned char		ConsumMode;//消费方式
extern	const uchar			DefaultNetDatas[22];

extern	unsigned long		Sys_SumConsumMoney;	
extern	unsigned char		CardDayConsumCount;
extern	unsigned char		PurseContrlNum[2];
extern	unsigned char		PurseUsingNum;
extern	unsigned char		SelectPurseNum;
extern	unsigned char		CommEncryptKey[8];
extern	unsigned char		PurseEnable[10];
extern	unsigned char		CardBatchEnable[32];//批次是否有效
extern	unsigned char		CardMinBalance[3];
extern	unsigned long		Limit_DayMoney;//日限额
extern	unsigned long		Max_ConsumMoney;
extern	unsigned char		PursesSector[10];
extern	unsigned char		ConsumModeEnable;//消费方式允许
extern	unsigned long		SumConsumMoney;//总消费额
extern	unsigned long		SumPrintMoney;//打印的总消费额
extern	unsigned char		bitNeedPrinter;//1-需要打印

extern	unsigned long		CurrentConsumMoney;//本次消费额
extern	unsigned char		MenuSort;//菜号
extern	unsigned char		InputMenuSort;
extern	unsigned char		Mac2Bak[4];
extern	unsigned char		ConsumSndInfo[60];
extern	unsigned char		ConsumCountDateTime[4];//计次消费日期时间
extern	unsigned char		CardEnableMonths;//黑名单有效月数
extern	unsigned char		DispCount;
extern	uint				BeepDelayTimeCount;//蜂鸣器
extern  uint        		BeepTimes;

extern	unsigned char		bitHaveKey;
extern	unsigned char		bitHaveReleaseKey;
extern	unsigned long		KeyValue;
extern	unsigned char		InputCase;
extern	unsigned char		InputBuffer[8];
extern	unsigned char		InputCount;
extern	unsigned char		bitHaveInputDatas;
extern	unsigned char		InputNum;
extern	unsigned char		bitInPutPoint;
extern	unsigned char		InputMaxNum;
extern	unsigned long		SingleMoney;
extern	uint						MulData;
extern	unsigned char		CardConsumTime[6];

extern	union	sDatasUnion   SerialUnion;

extern		unsigned char		SerialReceiveLen;
extern		unsigned char		SerialReCase;
extern		unsigned char		rChar;
extern		uint						SerialReCheckSum;
extern		unsigned char		SerialReCheckSumCount;

extern	unsigned char		  bitEven;
extern	unsigned char			bitSerial_ReceiveEnd;
extern	unsigned char			bitSerial_SendRequest;
extern	unsigned char     bitUARTSendEnd;

extern	unsigned char		  bitPF8563Error;
extern	union	uTimeUnion	SysTimeDatas;
extern	union	uTimeUnion	SysTimeDatasBak;

extern	uint32_t		RecordSum;//记录总数
extern	uint32_t		NoCollectRecordSum;//没有采集的记录总数
extern	uint32_t		NoCollectRecordIndex;//没有采集的记录指针
extern	uint32_t		ReCollectRecordIndex;//已经采集的记录指针
extern	uint32_t		SaveRecordIndex;//存储记录指针
extern  uchar		    MoneyPlanIndex ;//价格方案数据库存储指针
extern	unsigned char		SerialSendNoCollectNum;//上次传送的未采记录个数
extern	unsigned char		SerialSendReCollectNum;//上次传送的复采记录个数


extern	unsigned char		CardType[2];
extern	unsigned char		CardSerialNum[4];
extern	unsigned char		CardSerialNumBak[4];
extern	unsigned char		CardBatch;//卡批次
extern	unsigned char		CardIdentity;
extern 	unsigned char       nameBuf[16];//名字

extern	unsigned char		CardPrinterNum[4];//印刷编号
extern	unsigned char		OldBalance[4];
extern	unsigned char		NewBalance[4];

extern	unsigned long		CardDayConsumMoney;//卡上的日消费额
extern	unsigned char		CardConsumDate[3];//卡上的日消费日期

extern	 	unsigned char	DispModeChar[6];
extern	 	unsigned char	Disp0_9String[10];
extern	 	unsigned char	PaultRate9600Symbl[4];
extern	 	unsigned char	PaultRate4800Symbl[4];

extern	 	unsigned char	DownLoadDatas[6];
extern	 	unsigned char	AllFF[32];
extern	 	const unsigned char	DispErrorString[34*16];

extern  unsigned char 	CPU_CommBuf[50];
extern  unsigned char 	*CPU_RcLen;
extern	unsigned char   PsamRcvBufCount;
extern	unsigned char   PsamRcvBuf[50];//psam


extern	unsigned char  	Card_Rebate;
extern	unsigned char  	Sys_PinCode[3];
extern	uint   	   		  PosConsumCount;
extern	unsigned char  	WriteCardSta;//写卡状态
extern	unsigned char		MatchCode[4];//匹配字
extern	unsigned char   ComChallenge[5];
extern  unsigned char   BeepCotrl;

extern	unsigned char		PosVerString[10];
extern	unsigned char		LedWattingChar;
extern 	unsigned char		BatModeFlag;

//zjx_change_20110905
extern	unsigned char 	keybuff[4];
extern	unsigned long  	Falsh_TimeCount;
extern	unsigned char		DispBuffer[11];//显示数据

extern	unsigned char		bitAddStatus;
extern	unsigned char	  Forbidden;
extern	unsigned char   ReadKey_Pause; 
extern	unsigned char		bitHaveAuto;
extern	unsigned char		bitLookSysDatas;
extern	unsigned char		bitLookD;

extern	unsigned char 	NetCardDatas[22];
extern	unsigned char 	TypeA_Sort;//M1=0/CPU=1
extern	unsigned char 	Bak_Sort;

extern	unsigned char		CardKeyCode[6];//读卡密码
extern	unsigned char		CalCardKey[8];//卡密钥
extern	unsigned char		CardSector;//公共区的扇区号

extern	unsigned char	 	PinCode[3];
extern	unsigned char		Limit_MoneySign;
extern	unsigned char		DaySumMoneyDatasBak[28];

extern	unsigned char		PurseWrBufferBak[25];
extern	unsigned char		MainWrBufferBak[29];

extern	unsigned char		PurseConsumDateTime[4];
extern	unsigned long		PurseSumConsumMoney;
extern	unsigned char		PurseBT_Num[2];//补贴控制字
extern	unsigned char   DefaultKind[4];

extern	unsigned char		Flag_NotDefault;
extern	unsigned char		bitUseMoneyBlock;
extern	unsigned char   Flag_BakRecode;

extern	unsigned char		PurseDatas_Info[24];//钱包数据备份

extern	unsigned char  	rebufff[70];
extern	unsigned char  	ser0_BytesReceived;
extern	unsigned char  	ser0_ReceiveLen;

//add by lc
extern uint  	RecDelay;  
extern uchar 	Udp_Send_Flag; 
extern uint32_t 		Led_Open_time ;
extern uchar 	STX_FLAG;
extern uchar 	wifi_mode_flag ;
extern uchar	LedTcpIndex;//显示TCPIP网络参数的索引
extern uchar 	LoadModeResponse[15];
extern uchar  LoadModeFlag ;//ad by lc
extern uchar  CpuID[3];
extern uint 	ClientPort;//客户端端口号
extern uint 	POSServerPort;
extern uchar 	RemoteIP[4];
extern uchar	ReWriteCardDatasBak[16];//写卡前将要覆盖的数据

extern uint 	ClientPort;//客户端端口号
extern uchar 	RemoteIP[4];
extern uchar 	RemoteIPFlag;
extern uchar 	EscComsumeSaveRecordFlag ;
extern uchar 	BeepFlag ;
extern uchar	NeedResartFlag;
extern ulong  LocalTime;

extern uchar     FirstSendStx;
extern uchar  		Rebate_YesOrNo_Flag ;
extern uchar DayLimetMoney[3];//下载的日限额
extern uchar DayLimetFlag;
extern ulong ResetEnc28j60Num ;
extern uchar FirstUdpConnect;//第一次建立连接
extern uchar Reset_Wifi_Flag ;
extern uchar ShuRuMoney_Err ;//金额输入不完全


