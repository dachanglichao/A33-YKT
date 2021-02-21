
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
	uint16_t RcvLen;//�����������ݳ���
	uint8_t  TypeA_Sort;//��Ƭ����
	uint8_t  T_CL_Block_number; //T=CLЭ��Ŀ��
	uint8_t  RcvdataBuf[256];//������������
	uint8_t  CardType[2];//������
	uint8_t	 CardSerialNum[4];//��ƬΨһ���к�
	uint8_t  MRcvBuffer[80];
	uint8_t  MSndBuffer[80];//h531
	uint8_t Maximum_Frame_Size;//CPU��֡���������
	int fd;//RC522�����ļ����

}MFRC522_context;
MFRC522_context MFRC522_cxt;

const unsigned char FSCI_Code_Tab[16]= {16,24,32,40,48,64,96,128,255,255,255,255,255,255,255,255};//CPU��֡�����

static char TypeA_PiccRATS(void);
//�����Ƿ��п�


//��RC522�����ļ�
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


//��������?��MFRC522��ĳһ�Ĵ���дһ���ֽ�����
//�������?addr--�Ĵ�����ַ?val--Ҫд���ֵ
static void Write_MFRC522(uint8_t Address, uint8_t value) 
{
	int ret;
	unsigned char temp[2];
	temp[0] = Address;
	temp[1] = value;
	ret=ioctl(MFRC522_cxt.fd,RC522_Write,temp);  

}
//��������?��MFRC522��ĳһ�Ĵ�����һ���ֽ�����
//�������?addr--�Ĵ�����ַ
//�� �� ֵ?���ض�ȡ����һ���ֽ����� 
static uint8_t Read_MFRC522(uint8_t addr) 
{  
	int ret;
	unsigned char temp[2];
	temp[0] = addr;
	ret=ioctl(MFRC522_cxt.fd,RC522_Read,temp);  
	return temp[0];

}
//������������ֻ���ܶ�дλ��Ч
//��������?��RC522�Ĵ���λ
//�������?reg--�Ĵ�����ַ;mask--��λֵ
static void SetBitMask(uint8_t reg, uint8_t mask)   
{     
	uint8_t tmp=0;  
	tmp=Read_MFRC522(reg);     
	Write_MFRC522(reg,tmp|mask);  // set bit mask 
}
//��������?��RC522�Ĵ���λ
//�������?reg--�Ĵ�����ַ;mask--��λֵ
static void ClearBitMask(uint8_t reg, uint8_t mask)   
{     
	uint8_t tmp=0;
	//     
	tmp=Read_MFRC522(reg);     
	Write_MFRC522(reg,tmp&(~mask));  //clear bit mask 
}
//��������?��������,ÿ��������ر����߷���֮��Ӧ������1ms�ļ��
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
//��������?�ر�����,ÿ��������ر����߷���֮��Ӧ������1ms�ļ��
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
//��������?��λMFRC522
void MFRC522_Reset(void) 
{ 	
	//�⸴λ���Բ���
	int ret;
	unsigned char temp[2];
	ret=ioctl(MFRC522_cxt.fd,RC522_Reset,temp);  

	Write_MFRC522(0x01,0x0F); //soft reset
 // while(Read_MFRC522(0x27) != 0x88); //wait chip start ok	
	//�ڸ�λ   
	Write_MFRC522(CommandReg, PCD_RESETPHASE); 
	Write_MFRC522(TModeReg,0x8D);

}
//
void MFRC522_Initializtion(void) 
{	
	 
	MFRC522_Reset();         
	//Timer: TPrescaler*TreloadVal/6.78MHz = 0xD3E*0x32/6.78=25ms     
	Write_MFRC522(TModeReg,0x8D);				//TAuto=1Ϊ�Զ�����ģʽ����ͨ��Э��Ӱ�򡣵�4λΪԤ��Ƶֵ�ĸ�4λ
	//aa = Read_MFRC522(TModeReg);
	//Write_MFRC522(TModeReg,0x1D);				//TAutoRestart=1Ϊ�Զ����ؼ�ʱ��0x0D3E��0.5ms�Ķ�ʱ��ֵ//test    
	Write_MFRC522(TPrescalerReg,0x3E); 	//Ԥ��Ƶֵ�ĵ�8λ     
	Write_MFRC522(TReloadRegL,0x52);		//�������ĵ�8λ                
	Write_MFRC522(TReloadRegH,0x00);		//�������ĸ�8λ       
	Write_MFRC522(TxAutoReg,0x40); 			//100%ASK     
	Write_MFRC522(ModeReg,0x3D); 				//CRC��ʼֵ0x6363
	Write_MFRC522(CommandReg,0x00);			//����MFRC522  
	Write_MFRC522(RFCfgReg, 0x4F);    //RxGain = 48dB���ڿ���Ӧ����      
	AntennaOn();          							//������        							//������ 
}
//��������?RC522��ISO14443��ͨѶ
//�������?command--MF522������
//					sendData--ͨ��RC522���͵���Ƭ������
//					sendLen--���͵����ݳ���
//					BackData--���յ��Ŀ�Ƭ��������
//					BackLen--�������ݵ�λ����
//�� �� ֵ?�ɹ�����MI_O
static uint8_t MFRC522_ToCard(uint8_t command, uint8_t *sendData, uint8_t sendLen, uint8_t *backData, uint16_t *backLen) 
{
	uint8_t  status=MI_ERR;
	uint8_t  irqEn=0x00;
	uint8_t  waitIRq=0x00;
	uint8_t  lastBits;
	uint8_t  n;
	uint16_t i;
	//������Ԥ���жϲ���
	switch (command)     
	{         
		case PCD_AUTHENT:  		//��֤����   
			irqEn 	= 0x12;			//    
			waitIRq = 0x10;			//    
			break;
		case PCD_TRANSCEIVE: 	//����FIFO������      
			irqEn 	= 0x77;			//    
			waitIRq = 0x30;			//    
			break;      
		default:    
			break;     
	}
	//
	Write_MFRC522(ComIEnReg, irqEn|0x80);		//�����ж�����     
	ClearBitMask(ComIrqReg, 0x80);  				//��������ж�����λ               	
	SetBitMask(FIFOLevelReg, 0x80);  				//FlushBuffer=1, FIFO��ʼ��
	Write_MFRC522(CommandReg, PCD_IDLE); 		//ʹMFRC522����   
	//��FIFO��д������     
	for (i=0; i<sendLen; i++)
		Write_MFRC522(FIFODataReg, sendData[i]);
	//ִ������
	Write_MFRC522(CommandReg, command);
	//���߷�������     
	if (command == PCD_TRANSCEIVE)					//����ǿ�Ƭͨ�����MFRC522��ʼ�����߷�������      
		SetBitMask(BitFramingReg, 0x80);  		//StartSend=1,transmission of data starts    
	//�ȴ������������     
	i = 25; //i����ʱ��Ƶ�ʵ���?����M1�����ȴ�ʱ��25ms     
	do      
	{  	
		delay_ms(1);
		n = Read_MFRC522(ComIrqReg);       
		i--;	     
	}while ((i!=0) && !(n&0x01) && !(n&waitIRq));	//��������˳�n=0x64
	//ֹͣ����
	if(n>0x64)
	{
	ClearBitMask(BitFramingReg, 0x80);   		//StartSend=0	
	}
	ClearBitMask(BitFramingReg, 0x80);   		//StartSend=0
	//�����25ms�ڶ�����
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
//��������?Ѱ��?��ȡ�����ͺ�
//�������?reqMode--Ѱ����ʽ
//					TagType--���ؿ�Ƭ����
//					0x4400 = Mifare_UltraLight
//					0x0400 = Mifare_One(S50)
//					0x0200 = Mifare_One(S70)
//					0x0800 = Mifare_Pro(X)
//					0x4403 = Mifare_DESFire
//�� �� ֵ?�ɹ�����MI_OK	
static uint8_t Mf500PiccRequest(uint8_t reqMode, uint8_t *TagType)
{  
	uint8_t  status;    
	uint16_t backBits;   //���յ�������λ��
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
//��������?����ͻ���?��ȡѡ�п�Ƭ�Ŀ����к�
//�������?serNum--����4�ֽڿ����к�,��5�ֽ�ΪУ���ֽ�
//�� �� ֵ?�ɹ�����MI_OK
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
		//У�鿨���к�   
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
//��������?��MF522����CRC
//�������?pIndata--Ҫ����CRC������?len--���ݳ���?pOutData--�����CRC���
static void CalulateCRC(uint8_t *pIndata, uint8_t len, uint8_t *pOutData) 
{     
	uint16_t i;
	uint8_t  n;
	//      
	ClearBitMask(DivIrqReg, 0x04);   			//CRCIrq = 0     
	SetBitMask(FIFOLevelReg, 0x80);   		//��FIFOָ��     
	Write_MFRC522(CommandReg, PCD_IDLE);   
	//��FIFO��д������      
	for (i=0; i<len; i++)
		Write_MFRC522(FIFODataReg, *(pIndata+i));
	//��ʼRCR����
	Write_MFRC522(CommandReg, PCD_CALCCRC);
	//�ȴ�CRC�������     
	i = 1000;     
	do      
	{         
		n = Read_MFRC522(DivIrqReg);         
		i--;   
	}while ((i!=0) && !(n&0x04));   //CRCIrq = 1
	//��ȡCRC������     
	pOutData[0] = Read_MFRC522(CRCResultRegL);     
	pOutData[1] = Read_MFRC522(CRCResultRegH);
	Write_MFRC522(CommandReg, PCD_IDLE);
}
//��������?ѡ��?��ȡ���洢������
//�������?serNum--���뿨���к�
//�� �� ֵ?�ɹ����ؿ�����
static uint8_t MFRC522_SelectTag(uint8_t *serNum) 
{     
	uint8_t  i;     
	uint8_t  status;     
	volatile uint8_t  size;     
	uint16_t recvBits;     
	uint8_t  buffer[9];
	//     
	buffer[0] = PICC_ANTICOLL1;	//��ײ��1     
	buffer[1] = 0x70;
	buffer[6] = 0x00;						     
	for (i=0; i<4; i++)					
	{
		buffer[i+2] = *(serNum+i);	//buffer[2]-buffer[5]Ϊ�����к�
		buffer[6]  ^=	*(serNum+i);	//��У����
	}
	//
	CalulateCRC(buffer, 7, &buffer[7]);	//buffer[7]-buffer[8]ΪRCRУ����
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
//��������?��֤��Ƭ����
//�������?authMode--������֤ģʽ
//					0x60 = ��֤A��Կ
//					0x61 = ��֤B��Կ
//					BlockAddr--���ַ
//					Sectorkey--��������
//					serNum--��Ƭ���к�?4�ֽ�
//�� �� ֵ?�ɹ�����MI_OK
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum) 
{     
	uint8_t  status;     
	uint16_t recvBits;     
	uint8_t  i;  
	uint8_t  buff[12];    
	//��֤ģʽ+���ַ+��������+�����к�     
	buff[0] = authMode;		//��֤ģʽ     
	buff[1] = BlockAddr;	//���ַ     
	for (i=0; i<6; i++)
		buff[i+2] = *(Sectorkey+i);	//��������
	//
	for (i=0; i<4; i++)
		buff[i+8] = *(serNum+i);		//�����к�
	//
	status = MFRC522_ToCard(PCD_AUTHENT, buff, 12, buff, &recvBits);
	//      
	if ((status != MI_OK) || (!(Read_MFRC522(Status2Reg) & 0x08)))
		status = MI_ERR;
	//
	return status;
}
//��������?��������
//�������?blockAddr--���ַ;recvData--�����Ŀ�����
//�� �� ֵ?�ɹ�����MI_OK
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
//��������?д������
//�������?blockAddr--���ַ;writeData--���д16�ֽ�����
//�� �� ֵ?�ɹ�����MI_OK
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
		for (i=0; i<16; i++)  //��FIFOд16Byte����                     
			buff[i] = *(writeData+i);
		//                     
		CalulateCRC(buff, 16, &buff[16]);         
		status = MFRC522_ToCard(PCD_TRANSCEIVE, buff, 18, buff, &recvBits);           
		if ((status != MI_OK) || (recvBits != 4) || ((buff[0] & 0x0F) != 0x0A))               
			status = MI_ERR;         
	}          
	return status;
}
//��������?���Ƭ��������״̬
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
���RF CPU����λ��Ϣ
type_A Ѱ��
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
				MFRC522_cxt.TypeA_Sort=0;//M1��
			else if (MFRC522_cxt.CardType[0]==0x08 && !MFRC522_cxt.CardType[1])//CPU
				MFRC522_cxt.TypeA_Sort=0;//CPU��
			else
				return CARD_NOCARD;

			status=MFRC522_Anticoll(CardSerialNum);	//��ײ����	
			if (!status)
			{
				//debug("CardSerialNum = %2X %2X %2X %2X\r\n",MFRC522_cxt.CardSerialNum[0],MFRC522_cxt.CardSerialNum[1],MFRC522_cxt.CardSerialNum[2],MFRC522_cxt.CardSerialNum[3]);
				status=MFRC522_SelectTag(CardSerialNum);	//ѡ��
				//status =0;
				if (!status)
					break;
			}	
		}
		if(status)
		{
//			MFRC522_Initializtion();
//			OSTimeDlyHMSM(0,0,0,10); //���������1S
		}
	}

	if (status)//�޿�
		return CARD_NOCARD; 
	//�п�
	if (!MFRC522_cxt.TypeA_Sort)//04-s50;02-s70
		return	CARD_OK;
	else //CPU
	{
		status=TypeA_PiccRATS();//�˴�������
		if (!status)
		{		    
			memcpy(Rcvdata,MFRC522_cxt.MRcvBuffer,MInfo.nBytesReceived);
			RcvLen=MInfo.nBytesReceived;
			i=MFRC522_cxt.MRcvBuffer[1]&0x0f;
			do//����TYPE_A CPU��֡���������
			{
				MFRC522_cxt.Maximum_Frame_Size=FSCI_Code_Tab[i];
				i--;
			}while (MFRC522_cxt.Maximum_Frame_Size>_RC531BufSize);
		}

		if (status)//�޿�
			return CARD_NOCARD;
	    else
		{
			memcpy(CardSerialNumBuf,CardSerialNum,4);
		
			return CARD_OK;
		}
			
	}
}

/*------------------------------------------------
���RF M1����λ��Ϣ
type_A Ѱ��
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
				status=MFRC522_SelectTag(CardSerialNum);	//ѡ��
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

	if (status)//�޿�
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


/******************Ѱ����������M1����CPU��*******************************/
uint8_t Request_Card_info(uint8_t Cardtype ,uint8_t RcvLen, void * Rcvdata,uint8_t *CardSerialNumBuf)
{
	uint8_t status;
	if (Cardtype) //CPU��
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
TYPE A��B ����MFRC522_cxt.MSndBuffer����������(��ڣ����ͳ���)(����0 is ok)
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
����TYPE A RATS(��ڣ���)(����0 is ok)
----------------------------------------------------------*/
static char TypeA_PiccRATS(void)
{
	unsigned char  status;

	MFRC522_cxt.MSndBuffer[0] = 0xe0; //select_code;
	status=9;
//	srMode=2;
	do//����TYPE_A CPU��֡���������
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
���Ͳ�����7816�����ݴ�(��ڣ�����ָ�룬���ͳ���)(����0 is ok)
----------------------------------------------------------*/
char ISO7816_TRANSCEIVE(unsigned char SendLen, unsigned char * SendBuf, unsigned char * RcvLen, void * Rcvdata)
{
	char  status;
	char  i;
	unsigned char TotalSendLen; //�ۺϷ��ͳ���
	unsigned char OnceSendLen; //���η��ͳ���
	if (SendLen>=178)//������󻺴���
 	{
		return 0x50; //Ҫ���͵����ݹ���
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
		
		MFRC522_cxt.MSndBuffer[0] |=0x08;  //����CID
		MFRC522_cxt.MSndBuffer[++i] = 0x00;  //CID  ...Ϊ����TF COS����

		TotalSendLen+=OnceSendLen;
		memcpy(&MFRC522_cxt.MSndBuffer[++i],SendBuf,OnceSendLen);
		SendBuf+=OnceSendLen;
		status = TypeAB_PiccTRANSCEIVE(OnceSendLen+i);
		MInfo.nBytesReceived =MInfo.nBitsReceived>>3;
		MFRC522_cxt.T_CL_Block_number ^= 0x01;
		if ((MFRC522_cxt.MSndBuffer[0]&0x10)==0x00) break;//�ֶη��ͽ���
		if (!status)
		{
			if ((MFRC522_cxt.MRcvBuffer[0]&0xf0)!=0xa0)//���ֶ�ȷ��(R-block)
				return 0x51;//�ֶη���δ�õ�ȷ��
		}
		else
			return status;//���ʹ���
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
				status = TypeAB_PiccTRANSCEIVE(3);//����ͨѶʱ����չ
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
			else if(MFRC522_cxt.MRcvBuffer[0] && (MFRC522_cxt.MRcvBuffer[0]&0xc0)==0)//��Ϣ���� 
			{
				if ((*RcvLen+MInfo.nBytesReceived-1)>_RC531BufSize)
				{
					#ifdef PRINT_DBUG//
						printf("status3=:%02X \n",0x52);
					#endif
					return (0x52);//�ֶν��յ����ݹ���
				}
				if(MInfo.nBytesReceived>=2)
				{
					memcpy(*RcvLen+(unsigned char*)Rcvdata,&MFRC522_cxt.MRcvBuffer[2],MInfo.nBytesReceived-2);
					*RcvLen+=MInfo.nBytesReceived-2;
				}

				if ((MFRC522_cxt.MRcvBuffer[0]&0xf0)==0) //���һ֡��Ϣ
					break;
				else if ((MFRC522_cxt.MRcvBuffer[0]&0xf0)==0x10)//�к�����Ϣ
				{
					MFRC522_cxt.MSndBuffer[0]=((MFRC522_cxt.MRcvBuffer[0]&0x0f)|0xa0)^0x01;//���ͷֶν���ȷ��
					MFRC522_cxt.MSndBuffer[0] |=0x08;  //����CID
					MFRC522_cxt.MSndBuffer[1] = 0x00;  //CID  ...Ϊ����TF COS����
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

//522�Լ�ĺ����ɹ�����0��ʧ�ܷ���1
char CV522Pcd_ResetState(void)
{
	char aa;
	  
	MFRC522_Reset();         
	//Timer: TPrescaler*TreloadVal/6.78MHz = 0xD3E*0x32/6.78=25ms     
	Write_MFRC522(TModeReg,0x8D);				//TAuto=1Ϊ�Զ�����ģʽ����ͨ��Э��Ӱ�򡣵�4λΪԤ��Ƶֵ�ĸ�4λ
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