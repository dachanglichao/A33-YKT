
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include "delay.h"
#include  "MFRC522.h"
#include "../debug/debug.h"
#include "../ExternVariableDef.h"


#define PRINT_DBUG

 typedef struct 
{
	uint8_t  cmd;           
	uint8_t  status;        
	uint8_t  nBytesSent;    
	uint8_t  nBytesToSend;  
	uint8_t  irqSource;     
	uint8_t  collPos; 
	uint16_t  nBytesReceived;
	uint16_t nBitsReceived;    
} MfCmdInfo;

MfCmdInfo MInfo;

typedef struct 
{
	uint16_t RcvLen;//读卡返回数据长度
	uint8_t  TypeA_Sort;//卡片类型
	uint8_t  T_CL_Block_number; //T=CL协议的块号
	uint8_t  RcvdataBuf[256];//读卡返回数据
	uint8_t  CardType[2];//卡类型
	uint8_t	 CardSerialNum[4];//卡片唯一序列号
	uint8_t  MRcvBuffer[80];
	uint8_t  MSndBuffer[80];//h531
	uint8_t Maximum_Frame_Size;//CPU卡帧缓存最大数
	int fd;//RC522驱动文件句柄

}MFRC522_context;
MFRC522_context MFRC522_cxt;

const unsigned char FSCI_Code_Tab[16]= {16,24,32,40,48,64,96,128,255,255,255,255,255,255,255,255};//CPU卡帧缓存表

static char TypeA_PiccRATS(void);
//测试是否有卡


//打开RC522驱动文件
void InitRc522Driver()
{
	MFRC522_cxt.fd=open("/dev/rfid_rc522_dev",O_RDWR);  
	if(MFRC522_cxt.fd==-1){  
		perror("error open\n");  
		exit(-1);  
	}  
	printf("open /dev/rfid_rc522_dev successfully    :%8X\n",RC522_Reset);  
	//retval=ioctl(fd,RC522_Reset,data);  PcdReset();
}


//功能描述?向MFRC522的某一寄存器写一个字节数据
//输入参数?addr--寄存器地址?val--要写入的值
static void Write_MFRC522(uint8_t Address, uint8_t value) 
{
	int ret;
	unsigned char temp[2];
	temp[0] = Address;
	temp[1] = value;
	ret=ioctl(MFRC522_cxt.fd,RC522_Write,temp);  

}
//功能描述?从MFRC522的某一寄存器读一个字节数据
//输入参数?addr--寄存器地址
//返 回 值?返回读取到的一个字节数据 
static uint8_t Read_MFRC522(uint8_t addr) 
{  
	int ret;
	unsigned char temp[2];
	temp[0] = addr;
	ret=ioctl(MFRC522_cxt.fd,RC522_Read,temp);  
	return temp[0];

}
//下面两个函数只对能读写位有效
//功能描述?置RC522寄存器位
//输入参数?reg--寄存器地址;mask--置位值
static void SetBitMask(uint8_t reg, uint8_t mask)   
{     
	uint8_t tmp=0;  
	tmp=Read_MFRC522(reg);     
	Write_MFRC522(reg,tmp|mask);  // set bit mask 
}
//功能描述?清RC522寄存器位
//输入参数?reg--寄存器地址;mask--清位值
static void ClearBitMask(uint8_t reg, uint8_t mask)   
{     
	uint8_t tmp=0;
	//     
	tmp=Read_MFRC522(reg);     
	Write_MFRC522(reg,tmp&(~mask));  //clear bit mask 
}
//功能描述?开启天线,每次启动或关闭天线发射之间应至少有1ms的间隔
static void AntennaOn(void) 
{  
	uint8_t temp;
	//   
	temp=Read_MFRC522(TxControlReg);  
	if ((temp&0x03)==0)  
	{   
		SetBitMask(TxControlReg,0x03);  
	}
}
//功能描述?关闭天线,每次启动或关闭天线发射之间应至少有1ms的间隔
void AntennaOff(void) 
{  
	ClearBitMask(TxControlReg,0x03);
}
void ResetInfo(void)
{
	MInfo.cmd            = 0;
	MInfo.status         = MI_OK;
	MInfo.irqSource      = 0;
	MInfo.nBytesSent     = 0;
	MInfo.nBytesToSend   = 0;
	MInfo.nBytesReceived = 0;
	MInfo.nBitsReceived  = 0;
	MInfo.collPos        = 0;
}
//功能描述?复位MFRC522
void MFRC522_Reset(void) 
{ 	
	//外复位可以不用
	int ret;
	unsigned char temp[2];
	ret=ioctl(MFRC522_cxt.fd,RC522_Reset,temp);  

	Write_MFRC522(0x01,0x0F); //soft reset
 // while(Read_MFRC522(0x27) != 0x88); //wait chip start ok	
	//内复位   
	Write_MFRC522(CommandReg, PCD_RESETPHASE); 
	Write_MFRC522(TModeReg,0x8D);

}
//
void MFRC522_Initializtion(void) 
{	
	 
	MFRC522_Reset();         
	//Timer: TPrescaler*TreloadVal/6.78MHz = 0xD3E*0x32/6.78=25ms     
	Write_MFRC522(TModeReg,0x8D);				//TAuto=1为自动计数模式，受通信协议影向。低4位为预分频值的高4位
	//aa = Read_MFRC522(TModeReg);
	//Write_MFRC522(TModeReg,0x1D);				//TAutoRestart=1为自动重载计时，0x0D3E是0.5ms的定时初值//test    
	Write_MFRC522(TPrescalerReg,0x3E); 	//预分频值的低8位     
	Write_MFRC522(TReloadRegL,0x52);		//计数器的低8位                
	Write_MFRC522(TReloadRegH,0x00);		//计数器的高8位       
	Write_MFRC522(TxAutoReg,0x40); 			//100%ASK     
	Write_MFRC522(ModeReg,0x3D); 				//CRC初始值0x6363
	Write_MFRC522(CommandReg,0x00);			//启动MFRC522  
	Write_MFRC522(RFCfgReg, 0x4F);    //RxGain = 48dB调节卡感应距离      
	AntennaOn();          							//打开天线        							//打开天线 
}
//功能描述?RC522和ISO14443卡通讯
//输入参数?command--MF522命令字
//					sendData--通过RC522发送到卡片的数据
//					sendLen--发送的数据长度
//					BackData--接收到的卡片返回数据
//					BackLen--返回数据的位长度
//返 回 值?成功返回MI_O
static uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen) 
{
	uint8_t  status=MI_ERR;
	uint8_t  irqEn=0x00;
	uint8_t  waitIRq=0x00;
	uint8_t  lastBits;
	uint8_t  n;
	uint16_t i;
	//根据命预设中断参数
	switch (command)     
	{         
		case PCD_AUTHENT:  		//认证卡密   
			irqEn 	= 0x12;			//    
			waitIRq = 0x10;			//    
			break;
		case PCD_TRANSCEIVE: 	//发送FIFO中数据      
			irqEn 	= 0x77;			//    
			waitIRq = 0x30;			//    
			break;      
		default:    
			break;     
	}
	//
	Write_MFRC522(ComIEnReg, irqEn|0x80);		//允许中断请求     
	ClearBitMask(ComIrqReg, 0x80);  				//清除所有中断请求位               	
	SetBitMask(FIFOLevelReg, 0x80);  				//FlushBuffer=1, FIFO初始化
	Write_MFRC522(CommandReg, PCD_IDLE); 		//使MFRC522空闲   
	//向FIFO中写入数据     
	for (i=0; i<sendLen; i++)
		Write_MFRC522(FIFODataReg, sendData[i]);
	//执行命令
	Write_MFRC522(CommandReg, command);
	//天线发送数据     
	if (command == PCD_TRANSCEIVE)					//如果是卡片通信命令，MFRC522开始向天线发送数据      
		SetBitMask(BitFramingReg, 0x80);  		//StartSend=1,transmission of data starts    
	//等待接收数据完成     
	i = 25; //i根据时钟频率调整?操作M1卡最大等待时间25ms     
	do      
	{  	
		delay_ms(1);
		n = Read_MFRC522(ComIrqReg);       
		i--;	     
	}while ((i!=0) && !(n&0x01) && !(n&waitIRq));	//接收完就退出n=0x64
	//停止发送
	if(n>0x64)
	{
	ClearBitMask(BitFramingReg, 0x80);   		//StartSend=0	
	}
	ClearBitMask(BitFramingReg, 0x80);   		//StartSend=0
	//如果在25ms内读到卡
	if (i != 0)     
	{            
		if(!(Read_MFRC522(ErrorReg) & 0x1B)) //BufferOvfl Collerr CRCErr ProtecolErr         
		{   
			status = MI_OK;         
			if (n & irqEn & 0x01)			//                  
				status = MI_NOTAGERR;		//
			//
			if (command == PCD_TRANSCEIVE)             
			{                 
				n = Read_MFRC522(FIFOLevelReg);		//n=0x02                
				lastBits = Read_MFRC522(ControlReg) & 0x07;	//lastBits=0               
				if (lastBits!=0)                         
					*backLen = (n-1)*8 + lastBits; 
				else
					*backLen = n*8;	//backLen=0x10=16
				//
				if (n == 0)                         
				 	n = 1;                        
				if (n > MAX_LEN)         
				 	n = MAX_LEN;
				//
				for (i=0; i<n; i++)                 
					backData[i] = Read_MFRC522(FIFODataReg); 
			}
			//
			status = MI_OK;		
		}
		else
			status = MI_ERR;
	}	
	//
	Write_MFRC522(ControlReg,0x80);			//timer stops     
	Write_MFRC522(CommandReg, PCD_IDLE);	//
	return status;
}
//功能描述?寻卡?读取卡类型号
//输入参数?reqMode--寻卡方式
//					TagType--返回卡片类型
//					0x4400 = Mifare_UltraLight
//					0x0400 = Mifare_One(S50)
//					0x0200 = Mifare_One(S70)
//					0x0800 = Mifare_Pro(X)
//					0x4403 = Mifare_DESFire
//返 回 值?成功返回MI_OK	
static uint8_t Mf500PiccRequest(uint8_t reqMode, uint8_t *TagType)
{  
	uint8_t  status;    
	uint16_t backBits;   //接收到的数据位数
	//   
	Write_MFRC522(BitFramingReg, 0x07);  //TxLastBists = BitFramingReg[2..0]   
	TagType[0] = reqMode;  
	status = MFRC522_ToCard(PCD_TRANSCEIVE, TagType, 1, TagType, &backBits); 
	// 
	if ((status != MI_OK) || (backBits != 0x10))  
	{       
		status = MI_ERR;
	}
	//  
	return status; 
}
//功能描述?防冲突检测?读取选中卡片的卡序列号
//输入参数?serNum--返回4字节卡序列号,第5字节为校验字节
//返 回 值?成功返回MI_OK
static uint8_t MFRC522_Anticoll(uint8_t *serNum) 
{     
	uint8_t  status;     
	uint8_t  i;     
	uint8_t  serNumCheck=0;     
	uint16_t unLen;
	uint8_t  CardUID[8];
	//           
	ClearBitMask(Status2Reg, 0x08);  			//TempSensclear     
	ClearBitMask(CollReg,0x80);   				//ValuesAfterColl  
	Write_MFRC522(BitFramingReg, 0x00);  	//TxLastBists = BitFramingReg[2..0]
	serNum[0] = PICC_ANTICOLL1;     
	serNum[1] = 0x20;     
	status = MFRC522_ToCard(PCD_TRANSCEIVE, serNum, 2, CardUID, &unLen);
	//      
	if (status == MI_OK)
	{   
		//校验卡序列号   
		for(i=0;i<4;i++)   
			serNumCheck^=CardUID[i];
		//
		if(serNumCheck!=CardUID[i])        
			status=MI_ERR;
		memcpy(serNum,CardUID,4);
	}
	SetBitMask(CollReg,0x80);  //ValuesAfterColl=1
	//      
	return status;
}
//功能描述?用MF522计算CRC
//输入参数?pIndata--要读数CRC的数据?len--数据长度?pOutData--计算的CRC结果
static void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData) 
{     
	uint16_t i;
	uint8_t  n;
	//      
	ClearBitMask(DivIrqReg, 0x04);   			//CRCIrq = 0     
	SetBitMask(FIFOLevelReg, 0x80);   		//清FIFO指针     
	Write_MFRC522(CommandReg, PCD_IDLE);   
	//向FIFO中写入数据      
	for (i=0; i<len; i++)
		Write_MFRC522(FIFODataReg, *(pIndata+i));
	//开始RCR计算
	Write_MFRC522(CommandReg, PCD_CALCCRC);
	//等待CRC计算完成     
	i = 1000;     
	do      
	{         
		n = Read_MFRC522(DivIrqReg);         
		i--;   
	}while ((i!=0) && !(n&0x04));   //CRCIrq = 1
	//读取CRC计算结果     
	pOutData[0] = Read_MFRC522(CRCResultRegL);     
	pOutData[1] = Read_MFRC522(CRCResultRegH);
	Write_MFRC522(CommandReg, PCD_IDLE);
}
//功能描述?选卡?读取卡存储器容量
//输入参数?serNum--传入卡序列号
//返 回 值?成功返回卡容量
static uint8_t MFRC522_SelectTag(uint8_t *serNum) 
{     
	uint8_t  i;     
	uint8_t  status;     
	volatile uint8_t  size;     
	uint16_t recvBits;     
	uint8_t  buffer[9];
	//     
	buffer[0] = PICC_ANTICOLL1;	//防撞码1     
	buffer[1] = 0x70;
	buffer[6] = 0x00;						     
	for (i=0; i<4; i++)					
	{
		buffer[i+2] = *(serNum+i);	//buffer[2]-buffer[5]为卡序列号
		buffer[6]  ^=	*(serNum+i);	//卡校验码
	}
	//
	CalulateCRC(buffer, 7, &buffer[7]);	//buffer[7]-buffer[8]为RCR校验码
	ClearBitMask(Status2Reg,0x08);
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buffer, 9, buffer, &recvBits);
	//
	if ((status == MI_OK) && (recvBits == 0x18))    
		size = buffer[0];     
	else    
		size = 0;
	//	     
	return status; 
}
//功能描述?验证卡片密码
//输入参数?authMode--密码验证模式
//					0x60 = 验证A密钥
//					0x61 = 验证B密钥
//					BlockAddr--块地址
//					Sectorkey--扇区密码
//					serNum--卡片序列号?4字节
//返 回 值?成功返回MI_OK
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum) 
{     
	uint8_t  status;     
	uint16_t recvBits;     
	uint8_t  i;  
	uint8_t  buff[12];    
	//验证模式+块地址+扇区密码+卡序列号     
	buff[0] = authMode;		//验证模式     
	buff[1] = BlockAddr;	//块地址     
	for (i=0; i<6; i++)
		buff[i+2] = *(Sectorkey+i);	//扇区密码
	//
	for (i=0; i<4; i++)
		buff[i+8] = *(serNum+i);		//卡序列号
	//
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	//      
	if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
		status = MI_ERR;
	//
	return status;
}
//功能描述?读块数据
//输入参数?blockAddr--块地址;recvData--读出的块数据
//返 回 值?成功返回MI_OK
uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData) 
{     
	uint8_t  status;     
	uint16_t unLen;
	//      
	recvData[0] = PICC_READ;     
	recvData[1] = blockAddr;     
	CalulateCRC(recvData,2, &recvData[2]);     
	status = MFRC522_ToCard(PCD_TRANSCEIVE, recvData, 4, recvData, &unLen);
	//
	if ((status != MI_OK) || (unLen != 0x90))
		status = MI_ERR;
	//
	return status;
}
//功能描述?写块数据
//输入参数?blockAddr--块地址;writeData--向块写16字节数据
//返 回 值?成功返回MI_OK
uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData) 
{     
	uint8_t  status;     
	uint16_t recvBits;     
	uint8_t  i;  
	uint8_t  buff[18];
	//           
	buff[0] = PICC_WRITE;     
	buff[1] = blockAddr;     
	CalulateCRC(buff, 2, &buff[2]);     
	status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff, &recvBits);
	//
	if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))
		status = MI_ERR;
	//
	if (status == MI_OK)     
	{         
		for (i=0; i<16; i++)  //向FIFO写16Byte数据                     
			buff[i] = *(writeData+i);
		//                     
		CalulateCRC(buff, 16, &buff[16]);         
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);           
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))               
			status = MI_ERR;         
	}          
	return status;
}
//功能描述?命令卡片进入休眠状态
void MFRC522_Halt(void) 
{    
	uint16_t unLen;     
	uint8_t  buff[4];
	//       
	buff[0] = PICC_HALT;     
	buff[1] = 0;     
	CalulateCRC(buff, 2, &buff[2]);       
	MFRC522_ToCard(PCD_TRANSCEIVE, buff, 4, buff,&unLen);
}
//


///////////////////////////////////////////////////////////////////////
//                      C O D E   K E Y S
///////////////////////////////////////////////////////////////////////
static uint8_t Mf500HostCodeKey(  unsigned char * uncoded, unsigned char *	coded)   
{
//	uint8_t   status = MI_OK;
	uint8_t   cnt = 0;
	uint8_t   ln  = 0;     // low nibble
	uint8_t   hn  = 0;     // high nibble
   	for (cnt = 0; cnt < 6; cnt++)
   	{
		ln = uncoded[cnt] & 0x0F;
		hn = uncoded[cnt] >> 4;
		coded[cnt * 2 + 1]     =  (~ln << 4) | ln;
		coded[cnt * 2 ] =  (~hn << 4) | hn;
  	}
   	return MI_OK;
}

/*------------------------------------------------
检查RF CPU卡复位信息
type_A 寻卡
------------------------------------------------*/
unsigned char check_RFCPU_ResetInfo(unsigned char RcvLen, void * Rcvdata ,uint8_t *CardSerialNumBuf)
{
	unsigned char i;
	unsigned char status;
	

	MFRC522_Initializtion();
	//DeSelect();
	//debug("check_RFCPU_ResetInfo\r\n");
	
	for (i=0;i<3;i++)
	{	
		MFRC522_cxt.TypeA_Sort = 0xff;
		
		status=Mf500PiccRequest(0x52,MFRC522_cxt.CardType);

		if (!status)
		{
			if ((MFRC522_cxt.CardType[0]==0x04 || MFRC522_cxt.CardType[0]==0x02)&& !MFRC522_cxt.CardType[1])//04-s50;02-s70
				MFRC522_cxt.TypeA_Sort=0;//M1卡
			else if (MFRC522_cxt.CardType[0]==0x08 && !MFRC522_cxt.CardType[1])//CPU
				MFRC522_cxt.TypeA_Sort=0;//CPU卡
			else
				return CARD_NOCARD;

			status=MFRC522_Anticoll(CardSerialNum);	//防撞处理	
			if (!status)
			{
				//debug("CardSerialNum = %2X %2X %2X %2X\r\n",MFRC522_cxt.CardSerialNum[0],MFRC522_cxt.CardSerialNum[1],MFRC522_cxt.CardSerialNum[2],MFRC522_cxt.CardSerialNum[3]);
				status=MFRC522_SelectTag(CardSerialNum);	//选卡
				//status =0;
				if (!status)
					break;
			}	
		}
		if(status)
		{
//			MFRC522_Initializtion();
//			OSTimeDlyHMSM(0,0,0,10); //将任务挂起1S
		}
	}

	if (status)//无卡
		return CARD_NOCARD; 
	//有卡
	if (!MFRC522_cxt.TypeA_Sort)//04-s50;02-s70
		return	CARD_OK;
	else //CPU
	{
		status=TypeA_PiccRATS();//此处置类型
		if (!status)
		{		    
			memcpy(Rcvdata,MFRC522_cxt.MRcvBuffer,MInfo.nBytesReceived);
			RcvLen=MInfo.nBytesReceived;
			i=MFRC522_cxt.MRcvBuffer[1]&0x0f;
			do//设置TYPE_A CPU卡帧缓存最大数
			{
				MFRC522_cxt.Maximum_Frame_Size=FSCI_Code_Tab[i];
				i--;
			}while (MFRC522_cxt.Maximum_Frame_Size>_RC531BufSize);
		}

		if (status)//无卡
			return CARD_NOCARD;
	    else
		{
			memcpy(CardSerialNumBuf,CardSerialNum,4);
		
			return CARD_OK;
		}
			
	}
}

/*------------------------------------------------
检查RF M1卡复位信息
type_A 寻卡
------------------------------------------------*/
unsigned char check_RFM1_ResetInfo(uint8_t RcvLen, void * Rcvdata,uint8_t *CardSerialNumBuf)
{
	unsigned char i;
	unsigned char status;
	

	 MFRC522_Initializtion();
	// Msleep(20);
	//DeSelect();
	
	for (i=0;i<3;i++)
	{		
		status=Mf500PiccRequest(0x52,MFRC522_cxt.CardType);
		if (!status)
		{
			if ((MFRC522_cxt.CardType[0]==0x04 || MFRC522_cxt.CardType[0]==0x02)&& !MFRC522_cxt.CardType[1])//04-s50;02-s70
				MFRC522_cxt.TypeA_Sort=0;//M1
			else if (MFRC522_cxt.CardType[0]==0x08 && !MFRC522_cxt.CardType[1])//CPU
				MFRC522_cxt.TypeA_Sort=0;//CPU
			else
				return CARD_NOCARD;

			status=MFRC522_Anticoll(CardSerialNum);	//
			if (!status)
			{
				status=MFRC522_SelectTag(CardSerialNum);	//选卡
				//status =0;
				if (!status)
					break;
			}	
		}
		if(status)
		{
			
//			MFRC522_Initializtion();
//			delay_ms(10);
		}
	}

	if (status)//无卡
	{
		return CARD_NOCARD; 
	}
	//
	if (!MFRC522_cxt.TypeA_Sort)//04-s50;02-s70
	{
		memcpy(CardSerialNumBuf,CardSerialNum,4);
		return	CARD_OK;
	}

}


/******************寻卡操作区分M1还是CPU卡*******************************/
uint8_t Request_Card_info(uint8_t Cardtype ,uint8_t RcvLen, void * Rcvdata,uint8_t *CardSerialNumBuf)
{
	uint8_t status;
	if (Cardtype) //CPU卡
	{
		status = check_RFCPU_ResetInfo(RcvLen, Rcvdata,CardSerialNumBuf);
		if(status==CARD_OK)
		{
			debug("CPUCardSerialNumBuf==:%02X %02X %02X %02X\n",CardSerialNumBuf[0],CardSerialNumBuf[1],CardSerialNumBuf[2],CardSerialNumBuf[3]);		
			return CARD_OK;
		}
		else
		{
			debug("poll card fail\n");
			return 0xff;
		}
		//debug("CpuCardSerialNumBuf==:%02X %02X %02X %02X\n",CardSerialNumBuf[0],CardSerialNumBuf[1],CardSerialNumBuf[2],CardSerialNumBuf[3]);
	}
	else
	{
		status = check_RFM1_ResetInfo( RcvLen, Rcvdata,CardSerialNumBuf);
		if(status==CARD_OK)
		{
			debug("M1CardSerialNumBuf==:%02X %02X %02X %02X\n",CardSerialNumBuf[0],CardSerialNumBuf[1],CardSerialNumBuf[2],CardSerialNumBuf[3]);		
			return CARD_OK;
		}
		else
		{
			//debug("poll card fail\n");
			return 0xff;
			
		}
	}	
}
/*----------------------------------------------------------
TYPE A或B 发送MFRC522_cxt.MSndBuffer并接收数据(入口：发送长度)(返回0 is ok)
----------------------------------------------------------*/
static char TypeAB_PiccTRANSCEIVE(unsigned char SendLen)
{
	uint8_t status = MI_OK;
	ResetInfo();
	CalulateCRC(MFRC522_cxt.MSndBuffer, SendLen, &MFRC522_cxt.MSndBuffer[SendLen]);  
	MInfo.nBytesToSend  = SendLen+2;	
	status = MFRC522_ToCard(PCD_TRANSCEIVE, MFRC522_cxt.MSndBuffer, MInfo.nBytesToSend, MFRC522_cxt.MRcvBuffer, &MInfo.nBitsReceived); 

	//return status;
}
/*----------------------------------------------------------
发送TYPE A RATS(入口：无)(返回0 is ok)
----------------------------------------------------------*/
static char TypeA_PiccRATS(void)
{
	unsigned char  status;

	MFRC522_cxt.MSndBuffer[0] = 0xe0; //select_code;
	status=9;
//	srMode=2;
	do//声明TYPE_A CPU卡帧缓存最大数
	{
		status--;
		MFRC522_cxt.MSndBuffer[1]=FSCI_Code_Tab[status];
	}while (MFRC522_cxt.MSndBuffer[1]>=_RC531BufSize);
	MFRC522_cxt.MSndBuffer[1]=status<<4; 
	status = TypeAB_PiccTRANSCEIVE(2);
	MFRC522_cxt.T_CL_Block_number=0;
	return status;
}
/*----------------------------------------------------------
发送并接收7816的数据串(入口：发送指针，发送长度)(返回0 is ok)
----------------------------------------------------------*/
char ISO7816_TRANSCEIVE(unsigned char SendLen, unsigned char * SendBuf, unsigned char * RcvLen, void * Rcvdata)
{
	char  status;
	char  i;
	unsigned char TotalSendLen; //综合发送长度
	unsigned char OnceSendLen; //单次发送长度
	if (SendLen>=178)//握奇最大缓存数
 	{
		return 0x50; //要发送的数据过多
	}
	TotalSendLen=0;
	do
	{
		i=0;
		//-------------------------------------------------
		//zjx_add_20111130

		if(!SendLen)//Deselect
		{
			MFRC522_cxt.MSndBuffer[0]=0xc2;
		}
		else//I-block
		{
			MFRC522_cxt.MSndBuffer[0]=0x02;	
		}
		MFRC522_cxt.MSndBuffer[0]|=MFRC522_cxt.T_CL_Block_number;
		//-------------------------------------------------
		if ((SendLen-TotalSendLen)>(MFRC522_cxt.Maximum_Frame_Size-5))
		{
			MFRC522_cxt.MSndBuffer[0]|=0x10;
			OnceSendLen=MFRC522_cxt.Maximum_Frame_Size-5;
		}
		else
		{
			OnceSendLen=SendLen-TotalSendLen;
		}
		
		MFRC522_cxt.MSndBuffer[0] |=0x08;  //包含CID
		MFRC522_cxt.MSndBuffer[++i] = 0x00;  //CID  ...为了与TF COS兼容

		TotalSendLen+=OnceSendLen;
		memcpy(&MFRC522_cxt.MSndBuffer[++i],SendBuf,OnceSendLen);
		SendBuf+=OnceSendLen;
		status = TypeAB_PiccTRANSCEIVE(OnceSendLen+i);
		MInfo.nBytesReceived =MInfo.nBitsReceived>>3;
		MFRC522_cxt.T_CL_Block_number ^= 0x01;
		if ((MFRC522_cxt.MSndBuffer[0]&0x10)==0x00) break;//分段发送结束
		if (!status)
		{
			if ((MFRC522_cxt.MRcvBuffer[0]&0xf0)!=0xa0)//检查分段确认(R-block)
				return 0x51;//分段发送未得到确认
		}
		else
			return status;//发送错误
	}while(1);

	*RcvLen=0;
	
	do
	{
		if (!status)
		{
			//====================================================================
		
			#ifdef PRINT_DBUG//
				printf("status1=:%02X \n",status);
			#endif

			if((MFRC522_cxt.MRcvBuffer[0]&0xf0)==0xf0) //WTX
			{
				if(!i)	
				{

					#ifdef PRINT_DBUG//
						printf("status2=:%02X \n",0x53);
					#endif
					return	0x53;
				}
					
				i--;   
				memcpy(MFRC522_cxt.MSndBuffer,MFRC522_cxt.MRcvBuffer,3);
				MFRC522_cxt.MRcvBuffer[0]=0;
				status = TypeAB_PiccTRANSCEIVE(3);//卡机通讯时间扩展
				#ifdef PRINT_DBUG//
						printf("status7=:%02X \n",status);
				#endif
				MInfo.nBytesReceived =MInfo.nBitsReceived>>3;			
			}
			else if((MFRC522_cxt.MRcvBuffer[0]&0xf0)==0xc0)//Deselect
			{
				* RcvLen=0;
				return	0;
			}
			else if(MFRC522_cxt.MRcvBuffer[0] && (MFRC522_cxt.MRcvBuffer[0]&0xc0)==0)//信息分组 
			{
				if ((*RcvLen+MInfo.nBytesReceived-1)>_RC531BufSize)
				{
					#ifdef PRINT_DBUG//
						printf("status3=:%02X \n",0x52);
					#endif
					return (0x52);//分段接收的数据过多
				}
				if(MInfo.nBytesReceived>=2)
				{
					memcpy(*RcvLen+(unsigned char*)Rcvdata,&MFRC522_cxt.MRcvBuffer[2],MInfo.nBytesReceived-2);
					*RcvLen+=MInfo.nBytesReceived-2;
				}

				if ((MFRC522_cxt.MRcvBuffer[0]&0xf0)==0) //最后一帧信息
					break;
				else if ((MFRC522_cxt.MRcvBuffer[0]&0xf0)==0x10)//有后续信息
				{
					MFRC522_cxt.MSndBuffer[0]=((MFRC522_cxt.MRcvBuffer[0]&0x0f)|0xa0)^0x01;//发送分段接收确认
					MFRC522_cxt.MSndBuffer[0] |=0x08;  //包含CID
					MFRC522_cxt.MSndBuffer[1] = 0x00;  //CID  ...为了与TF COS兼容
					status = TypeAB_PiccTRANSCEIVE(2);
					MInfo.nBytesReceived =MInfo.nBitsReceived>>3;

					#ifdef PRINT_DBUG//
							printf("status4=:%02X \n",status);
					#endif
					
				}
			}
			else 
			{
				#ifdef PRINT_DBUG//
					printf("status5=:%02X \n",0xff);
				#endif
				return 0xff;
			}	
			//===================================================================
		}
		else
		{
			#ifdef PRINT_DBUG//
					printf("status6=:%02X \n",status);
			#endif
			return status;
		}
			
	}while(1);

	

	return status;
}

//522自检的函数成功返回0，失败返回1
char CV522Pcd_ResetState(void)
{
	char aa;
	  
	MFRC522_Reset();         
	//Timer: TPrescaler*TreloadVal/6.78MHz = 0xD3E*0x32/6.78=25ms     
	Write_MFRC522(TModeReg,0x8D);				//TAuto=1为自动计数模式，受通信协议影向。低4位为预分频值的高4位
	aa = Read_MFRC522(TModeReg);
	if(aa == 0x8d)
	{
		return 0;
	}
	else
	{
		return 1;
	}
}