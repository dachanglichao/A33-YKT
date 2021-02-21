
#include <string.h>
#include <stdio.h>
#include "mydefine.h"




//struct	sDatasStruct//通讯数据格式
//{
//	uint16_t	UartSeAddrCode;
//	uint16_t	UartReAddrCode;
//	uint16_t	UartComd;
//	uchar	UartStatus;
//	unsigned char	UartAddrH;//地址高
//	unsigned char	UartAddrL;//地址低
//	unsigned char	UartFrameNum;//帧号
//	unsigned char	DatasLen;//数据长度
//	unsigned char	Datas[139];//数据包
//};
//union	sDatasUnion
//{
//	unsigned char	S_DatasBuffer[150];
//	struct	sDatasStruct	S_DatasStruct;
//};


uint		MainCode ;//站点号
uint		UdpCode = 9000;//UDP端口号
uchar		OutChar16C550;
uchar		CommModeChar=0;

uchar		bitPinEnable=0;//超限额密码消费使能
uchar		bitNeedRST=0;
uchar		bitNeedDownLoad=0;
uchar		bitPurseEnd=0;

uint		SerialReceiveTimeCount=0;
uchar		TCPDatasComd;
uchar		LedTcpIndex;//显示TCPIP网络参数的索引

uchar		DispCount;
uchar		PinCount=0;
uchar		bitHaveReadBalance=0;
uchar		bitSetDateEnable=0;//时间设置使能 
uchar		bitWrittingFlash=0;
uchar		bitDispFlash=0;
uchar		bitBeepError=0;
uchar		bitHaveCollectRecord=0;
uchar	  	bitHaveReCollectRecord=0;
uchar		bitStatus8563=0;
uchar		bitSysTimeDisable=0;
uchar		bitRecordFull=0;
uchar		bitConsumZeroEnable=0;//限次
uchar		bitHaveSeclectPurse=0;
uchar		bitCommStatus;//读卡状态备份

uchar		StatusBak;//是否有卡
uchar		DelayReadCardTimeCount=0;
uchar		CtrlChar=0xff;//控制字节输出，主要通过573控制蜂鸣器和485通讯的收发转换
uchar		LoopCount=0;//主循环计数
uchar		bitUpdateParameter=1;
uchar		DownLoadPragamSymbl[6];//需要下载程序的标识数据,0xb0--0xb5,需要下载
uchar		LoadProgamReDatas[6] ;
uchar		ConsumCase=0;
uchar		ConsumMode=DEFAULT_CONSUM_MODE;//消费方式

uchar		T3_TimeCount=0;
uchar		bitHaveLedError=0;
uchar		bitNeedDiagRecordFull=1;

uchar		InputCase=0;
uchar		InputBuffer[8];
uchar		InputCount=0;
uchar		bitHaveInputDatas=0;
uchar		InputNum=0;
uchar		bitInPutPoint=0;
uchar		InputMaxNum=0;
uint		MulData=0;
ulong		SingleMoney=0;

ulong		Sys_SumConsumMoney=0;	

uchar		PurseUsingNum;//当前钱包号
uchar		SelectPurseNum;//应用钱包起始号
/////////////////////系统参数///////////////
uchar		CardBatchEnable[32];//批次是否有效
uchar		CommEncryptKey[8];
uchar		PurseEnable[10];//钱包使能
uchar		CardMinBalance[3];//卡底金
uchar		DayLimetMoney[3];//下载的日限额
uchar		CardEnableMonths;//黑名单有效月数

uchar		ConsumModeEnable;//消费方式允许
ulong		SumConsumMoney=0;//总消费额
ulong		SumPrintMoney=0;//打印的总消费额
uchar		bitNeedPrinter=0;//1-需要打印
ulong		CurrentConsumMoney=0;//本次消费额
uchar		MenuSort=0;//菜号
uchar		InputMenuSort=0;
uchar   LoadModeResponse[15];
uchar   LoadModeFlag = 0;//ad by lc

uint		BeepDelayTimeCount=0;//蜂鸣器
uint    BeepTimes=0;
uchar		bitHaveKey=0;
uchar		bitHaveReleaseKey=1;
ulong		KeyValue;
uchar		Receive_Ip_Port[6];

union		sDatasUnion  SerialUnion;
uchar		SerialReceiveLen;
uchar		SerialReCase;
uchar		rChar;
uint		SerialReCheckSum;
uchar		SerialReCheckSumCount;

uchar   bitUARTSendEnd;
uchar		bitSerial_ReceiveEnd;
uchar		bitSerial_SendRequest;
uchar		bitEven;

uchar		bitPF8563Error=0;
union		uTimeUnion		SysTimeDatas;
union		uTimeUnion		SysTimeDatasBak;

uint32_t		RecordSum=0;//记录总数
uint32_t		NoCollectRecordSum=0;//没有采集的记录总数
uint32_t		NoCollectRecordIndex=0;//没有采集的记录指针
uint32_t		ReCollectRecordIndex=0;//已经采集的记录指针
uint32_t		SaveRecordIndex=0;//存储记录指针
uchar		SerialSendNoCollectNum=0;//上次传送的未采记录个数
uchar		SerialSendReCollectNum=0;//上次传送的复采记录个数
uchar		MoneyPlanIndex = 0;//价格方案数据库存储指针

uchar		PaultRate9600Symbl[4]={0x46,0x57,0x68,0x79};
uchar		PaultRate4800Symbl[4]={0x0a,0x1b,0x2c,0x3d};
uchar		DownLoadDatas[6]={0xb0,0xb1,0xb2,0xb3,0xb4,0xb5};


//---------------------------------------------------------------
uchar 	ConsumSndInfo[60];//CPU卡发送信息
uchar   CPU_CommBuf[50];
uchar  *CPU_RcLen;
uchar   PsamRcvBufCount;
uchar   PsamRcvBuf[50];//psam

//卡UID
uchar			CardType[2];
uchar		CardSerialNum[4];
uchar		CardSerialNumBak[4];
uchar		CardPrinterNum[4];//印刷编号

//一卡通公用信息
uchar		MatchCode[4]={0x12,0x34,0x56,0x78}; //UserCode[4];//用户代码
uchar		CardIdentity;//身份
uchar		CardBatch;//卡批次
ulong		Limit_DayMoney;//日限额
ulong		Max_ConsumMoney;//单笔限额
uchar   	Card_Rebate;//折扣
uchar		PursesSector[10];//钱包文件标识索引
uchar       nameBuf[16];//名字

//一卡通累计信息
ulong		CardDayConsumMoney;//卡上的日消费额
uchar		CardConsumDate[3];//卡上的日消费日期,时间
uchar		ConsumCountDateTime[4];//计次消费日期时间
uchar		CardDayConsumCount=0;//累计次数    //CardPurseDayCount

//一卡通交易信息
uchar		PinCheckSta; //PIN校验状态
uchar   	Sys_PinCode[3]={0x12,0x34,0x56}; //系统PIN码
uchar		PurseContrlNum[2];//卡交易号
uint		PosConsumCount=0;//POS的消费流水号
uchar		CardConsumTime[6];//消费时间，防止MAC校验失败
uchar		Mac2Bak[4];//MAC2
uchar		WriteCardSta=0;//写卡错误状态
uchar		OldBalance[4];
uchar		NewBalance[4];

uchar   ComChallenge[5]={0x00,0x84,0x00,0x00,0x04};//取随机数


//zjx_change_20110905
uchar 	keybuff[4];
ulong   Falsh_TimeCount=0;
uchar   Forbidden=0;
uchar   ReadKey_Pause=0; 
uchar	DispBuffer[11];//显示数据
uchar	LedWattingChar=0;
uchar	bitAddStatus=0;

uchar	bitHaveAuto=0;
uchar	bitLookSysDatas=0;
uchar	bitLookD=0;
uchar	bitUseMoneyBlock;

uchar 	NetCardDatas[22];
uchar 	TypeA_Sort;//M1=0/CPU=1
uchar 	Bak_Sort;

uchar	CardKeyCode[6];//读卡密码
uchar	CalCardKey[8];//卡密钥
uchar	CardSector;//公共区的扇区号 
uchar	PinCode[3];
uchar	Limit_MoneySign = 0xff;
uchar	DaySumMoneyDatasBak[28];//欲覆盖的日累计消费额的数据备份

uchar	PurseWrBufferBak[25];
uchar	MainWrBufferBak[29];

uchar	PurseConsumDateTime[4];
ulong	PurseSumConsumMoney;
uchar	PurseBT_Num[2];

uchar  	DefaultKind[4]={0xab,0xcd,0xef,0xfe};
uchar	Flag_NotDefault;
uchar   Flag_BakRecode;

uchar	PurseDatas_Info[24];//钱包数据备份


uchar  	rebufff[70];//串口接收到的数据 
uchar  	ser0_BytesReceived=0;//已接收到的数据个数
uchar  	ser0_ReceiveLen=0;//期望接收到的个数
 
uint   	RecDelay = 0;   //接受数据延时等待	
//ad by lc
uchar 	Udp_Send_Flag = 0;
uint32_t 	Led_Open_time = 0;
uchar 	STX_FLAG = 0;
uchar 	wifi_mode_flag = 0;
uchar 	BatModeFlag;
uchar  	CpuID[3];
uint 	POSServerPort = 9000;//客户端端口号

uchar	ReWriteCardDatasBak[16];//写卡前将要覆盖的数据

uint 	ClientPort = 9001;//客户端端口号
uchar 	RemoteIP[4]={192,168,10,188};
uchar 	RemoteIPFlag = 1;

uchar 	EscComsumeSaveRecordFlag = 0;
uchar 	BeepFlag = 0;
uchar	NeedResartFlag = 0;
ulong  LocalTime = 0;
uchar     FirstSendStx = 0;
uchar     Rebate_YesOrNo_Flag = 0;
uchar DayLimetFlag = 0;//0时日限额按卡里面的 1按下载的
ulong ResetEnc28j60Num = 0;

uchar FirstUdpConnect=0;//第一次建立连接


uchar 	VoiceVal = 30;
uchar Reset_Wifi_Flag = 0;
uchar ShuRuMoney_Err = 0;//金额输入不完全

