#include  "MFRC522.h"
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "iso7816.h"
#include "delay.h"
#include "CalucationFile.h"


static uint8_t     ComChallenge[5]={0x00,0x84,0x00,0x00,0x04};//取随机数

#define PRINT_DBUG

//目标卡选择
#define CPUtarget  0
#define PSAMtarget 1

//函数声明


static uint16_t GetU8ToU16(uint8_t * Buf)
{
	uint16_t sw1sw2;
	sw1sw2 = (uint16_t)Buf[1] + ((uint16_t)Buf[0]<<8);
	return sw1sw2;
} 

//----------------------------------------------------------
//通用函数：IC7816中转发送接收
//入口：Sort：0--CPU卡  1--PSAM卡	
//	SendLen：发送长度
//	SendBuf：发送指针
//	 RcvLen：接收长度指针
//	 RcvBuf：接收数据指针	
//出口：结果代码
//----------------------------------------------------------
uint16_t IC7816_relay_station(uint8_t Sort, uint8_t SendLen, void * SendBuf, uint8_t * RcvLen, void * RcvBuf)
{
	char  status;
	uint16_t   ret7816;
	
	if(!Sort)
		status=ISO7816_TRANSCEIVE(SendLen, SendBuf, RcvLen, RcvBuf);
	else
		status=ISO7816_PSAM(SendLen, SendBuf, RcvLen, RcvBuf);

	if(!status)
	{
		if(!Sort)
		*RcvLen-=4;
		else
		*RcvLen-=2;
		ret7816=GetU16_HiLo(&((uint8_t*)RcvBuf)[*RcvLen]);
		return ret7816;
	}
	else
		return (0x5000+status);
	
}

//----------------------------------------------------------
//终端从卡上读取随机数
//命令：00 84 00 00 
//用此命令模拟卡是否存在
//------------------------------------------------------------

uint8_t CPU_Get_Challenge(uint8_t * RcvLen, uint8_t * RcvBuf)
{
    uint16_t 	 sw1sw2=0;
	sw1sw2=IC7816_relay_station(0,5,ComChallenge,RcvLen,RcvBuf);
	if(sw1sw2!=0x9000)
		return	CARD_NOCARD;
	else
		return	CARD_OK;
}

//----------------------------------------------------------
//通过应用标识符来选择文件
//----------------------------------------------------------
uint16_t CPU_Select_File_AID(uint8_t Sort, uint8_t * FileAID,uint8_t * RcvLen, uint8_t * RcvBuf)
{
	uint8_t SendBuf[7];

	SendBuf[0]=0x00;//CLA
	SendBuf[1]=0xA4;//INS
	SendBuf[2]=0x00;//P1
	SendBuf[3]=0x00;//P2
	SendBuf[4]=0x02;//Lc
	memcpy(SendBuf+5,FileAID,2);
	
	return IC7816_relay_station(Sort,7, SendBuf, RcvLen, RcvBuf);
}

//----------------------------------------------------------
//通过应用标识符来选择文件
//----------------------------------------------------------
uint16_t CPU_Select_DKFile_AID(uint8_t Sort, uint8_t * FileAID,uint8_t * RcvLen, uint8_t * RcvBuf)
{
	uint8_t SendBuf[14];

	SendBuf[0]=0x00;//CLA
	SendBuf[1]=0xA4;//INS
	SendBuf[2]=0x04;//P1
	SendBuf[3]=0x00;//P2
	SendBuf[4]=0x02;//Lc
	memcpy(SendBuf+5,FileAID,2);
	
	return IC7816_relay_station(Sort,14, SendBuf, RcvLen, RcvBuf);
}


//zz+e
//---------------------------------------------------------------------------------
//通过短文件标识，0为读出所有文件内容
//FileSFI:短文件标识
//---------------------------------------------------------------------------------
uint16_t CPU_Read_Binary(uint8_t Sort,uint8_t FileSFI,uint8_t p2,uint8_t len,uint8_t * RcvBuf)
{
	uint8_t  SendBuf[5];
	uint8_t  RcvLen;
	uint16_t    ret;
	
	SendBuf[0]=0x00;//CLA
	SendBuf[1]=0xB0;//INS
	SendBuf[2]=(FileSFI&0x1f)|0x80 ;//P1
	SendBuf[3]=0x00;//P2
	SendBuf[4]=len;
	ret=IC7816_relay_station(Sort,5,SendBuf, &RcvLen, RcvBuf);
	if((ret&0xff00)==0x6c00)
	{
		SendBuf[4]=(uint8_t)ret;
		return	IC7816_relay_station(Sort,5,SendBuf, &RcvLen, RcvBuf);
	}
	else
		return ret;	
}


uint16_t CPU_Read_Binary_ID(uint8_t Sort,uint8_t FileSFI,uint8_t p2,uint8_t len,uint8_t * RcvBuf)
{
	uint8_t  SendBuf[5];
	uint8_t  RcvLen;
	uint16_t    ret;
	
	SendBuf[0]=0x00;//CLA
	SendBuf[1]=0xB0;//INS
	SendBuf[2]=(FileSFI&0x1f)|0x80 ;//P1
	SendBuf[3]=0x1D;//P2
	SendBuf[4]=len;
	ret=IC7816_relay_station(Sort,5,SendBuf, &RcvLen, RcvBuf);
	if((ret&0xff00)==0x6c00)
	{
		SendBuf[4]=(uint8_t)ret;
		return	IC7816_relay_station(Sort,5,SendBuf, &RcvLen, RcvBuf);
	}
	else
		return ret;	
}

//-----------------------------------------------------------------
//下面的4个基本函数是PSAM专有的,DES初始化、DES计算在读写器上有应用
//包括：DES初始化、DES计算、MAC1、MAC2
//pSAM初始化金融目录
//------------------------------------------------------------------
uint8_t PSAM_Init_Dir(void)
{

	uint16_t 	sw1sw2=0;  
	uint8_t	FileAID[20];
	uint8_t CPU_RcLen;
	uint8_t CPU_CommBuf[256];

	FileAID[0]=0x3f;
	FileAID[1]=0x00;
	sw1sw2=CPU_Select_File_AID(1,FileAID,&CPU_RcLen,CPU_CommBuf); //选择主文件 
	if(sw1sw2!=0x9000)
		return	PSAM_FILEPARA_ERR;
	delay_ms(100);
	memcpy(FileAID,"\xa0\x00\x00\x00\x03\x86\x98\x07\x01",9);
	sw1sw2=CPU_Select_DKFile_AID(1,FileAID,CPU_RcLen,CPU_CommBuf);	

	if(sw1sw2!=0x9000)
		return	CARD_NOCARD;
	
	return	CARD_OK;
}

/*******************************************************************************
//终端从卡上读取随机数
//命令：00 84 00 00 
//用此命令模拟卡是否存在
*******************************************************************************/
uint16_t Psam_GetChallenge(uint8_t sel,uint8_t *pBuffer,uint16_t Len)
{
	// uint8_t 	Sendbuf[32];
	// uint8_t  Revbuf[32];
	// uint16_t RevLen;
	// uint16_t sw1sw2;
	// memset(Revbuf,0,32) ;
	// Sendbuf[0] = 0x00;
	// Sendbuf[1] = 0x84;
	// Sendbuf[2] = 0x00;
	// Sendbuf[3] = 0x00;
	// Sendbuf[4] = Len;
	// PsamSend(1,Sendbuf,5);
	// RevLen=PsamReceive(1,Revbuf,Len+3,8000); //50Ms
	// sw1sw2=GetU8ToU16(Revbuf+RevLen-2);
	// if(sw1sw2==0x9000)
	// {
	// 	memcpy(pBuffer,Revbuf+1,Len);
	// }
	// return sw1sw2;
}
//----------------------------------------------------------------
//MAC1计算
//SndLen：指明SndInfo的数据长度=14H+8*n(n=1,2,3),这里用N=1.
//SndInfo：发送参数指针，结构如下(以字节顺序)
//用户卡随机数(4)+用户卡交易序号(2)+交易金额(4)+交易类型(1,05/06)+
//  交易日期(4)+交易时间(3)+消费密钥版本号(1)+消费密钥算法标识(1)+
//  用户卡应用序列号(8,要最右边的)+成员银行标识(8)+试点城市标识(8)
//RcvInfo：接收参数指针,终端交易序号(4)+MAC1(4)
//-----------------------------------------------------------------



//写脱机限额
//uint8_t WriteNoNetLimitMoney(uint8_t *buf)
//{
//	uint 	sw1sw2=0;
//	uint8_t  WriteBuf[80];
//	uint8_t  temp[4];
//	uint8_t status,i;

//	
//	sw1sw2=CPU_Read_Binary(0,21,41,WriteBuf);//发行信息区，即公共信息区
//	if(sw1sw2!=0x9000)
//	{	
//		return	CPU_WRITEPURSE_FAIL;
//	}
//	memcpy(WriteBuf+5,buf,4);
//	//向RF取随机数
//	status=CPU_Get_Challenge(CPU_RcLen,CPU_CommBuf);
//	if(status)
//		return	status;
//	memcpy(temp,CPU_CommBuf,4);

//	memcpy(ConsumSndInfo,"\x80\x1A\x45\x00\x10\x00\x00\x00\x03\x86\x98\x10\x01",13);
//	ConsumSndInfo[13]=CardSerialNum[3];
//	ConsumSndInfo[14]=CardSerialNum[2];
//	ConsumSndInfo[15]=CardSerialNum[1];
//	ConsumSndInfo[16]=CardSerialNum[0];
//	ConsumSndInfo[17]=CardSerialNum[3];
//	ConsumSndInfo[18]=CardSerialNum[2];
//	ConsumSndInfo[19]=CardSerialNum[1];
//	ConsumSndInfo[20]=CardSerialNum[0];

//		//ConsumSndInfo[3]=i++;
//	sw1sw2=IC7816_relay_station(1,21,ConsumSndInfo,CPU_RcLen,CPU_CommBuf);
//	if(sw1sw2!=0x9000)
//			return	CPU_WRITEPURSE_FAIL;	
//	delay_ms(100);
//	//数据组织
//	memcpy(ConsumSndInfo,"\x80\xfa\x05\x00\x18",5);//40个数据=8+32
//	memcpy(ConsumSndInfo+5,temp,4);
//	memset(ConsumSndInfo+9,0,4);
//	memcpy(ConsumSndInfo+13,"\x04\xd6\x95\x00\x0d",5);    //18+6+4
//	memcpy(ConsumSndInfo+18,WriteBuf,9);    //18+6+4
//	memcpy(ConsumSndInfo+27,"\x80\x00",2);//补充数据=32个待发数据
//   for(i=0;i<5;i++)
//	{
//		sw1sw2=IC7816_relay_station(1,29,ConsumSndInfo,CPU_RcLen,CPU_CommBuf);
//		if(sw1sw2==0x9000)
//			break;
//	}
//	if(sw1sw2!=0x9000)
//		return	CPU_WRITEPURSE_FAIL;
//   	memcpy(ConsumSndInfo,"\x04\xd6\x95\x00\x0d",5);//32个数据
//	memcpy(ConsumSndInfo+5,WriteBuf,9);
//	memcpy(ConsumSndInfo+14,CPU_CommBuf,4);//37个待发数据
//	sw1sw2=IC7816_relay_station(0,18,ConsumSndInfo,CPU_RcLen,CPU_CommBuf);
//	if(sw1sw2!=0x9000)
//		return	CPU_WRITEPURSE_FAIL;
//	return 0;
//}
//----------------------------------------------------------
//PSAM发送并接收7816的数据串(入口：发送指针，发送长度)(返回0 is ok)
//命令頭:CLA+INS+P1+P2+P3
//过程字节;
//zjx_change_20120712
//----------------------------------------------------------
char ISO7816_PSAM(unsigned char SendLen, unsigned char * SendBuf, unsigned char * RcvLen, unsigned char * Rcvdata)
{
	// uint8_t PsamSndBuf[5];
	// uint8_t ulen=0;

	// if (SendLen < 5)
	// 	return PSAM_COM_ERROR; //长度错	

	// //=============================================
	// //计算ulen个数
	// if(SendLen==5)
	// {
	// 	if(!SendBuf[4])//情形1及情形2最大数量情况
	// 		ulen=2;
	// 	else//情形2,有期望返回
	// 		ulen=SendBuf[4]+3;	
	// }
	// else
	// {
	// 	if((SendBuf[4]+5)==SendLen || (SendBuf[4]+6)==SendLen )	
	// 		ulen=1;	
	//     else
	//     	return PSAM_COM_ERROR; //长度错	

	// }
	// //=============================================	

	// PsamSend(1,SendBuf,5);//发送命令头

	// PsamReceive(1,PsamRcvBuf,ulen,2000);//接收过程字节INC/INC补码/60/6x、9x

	// if (PsamRcvBufCount==0)	
	// 	return PSAM_COM_ERROR; //无应答
	// else if(PsamRcvBufCount==1)
	// {
	// 	if (ulen==1 && PsamRcvBuf[0]==SendBuf[1]) //过程字节等于INS
	// 	{
	// 		//发送以后的数据
	// 		//均按get
	// 		if((SendBuf[4]+5)==SendLen)
	// 			ulen=2;
	// 		else
	// 		    ulen=SendBuf[4]+3;
	// 		delay_ms(2);
	// 		PsamSend(1,SendBuf+5,SendLen-5);

	// 		//接收数据
	// 		PsamReceive(1,PsamRcvBuf,ulen,2000);
	// 		__nop();
	// 	}
	// 	else //非过程字节
	// 	{
	// 		//接收数据
	// 		Rcvdata[0]=PsamRcvBuf[0];
	// 		PsamReceive(1,PsamRcvBuf,1,2000);			
	// 		if(PsamRcvBufCount==1)//错误状态
	// 		{
	// 			Rcvdata[1]=PsamRcvBuf[0];
	// 			*RcvLen=2;				
	// 			return 0;
	// 		}
	// 		else
	// 			return PSAM_COM_ERROR; //应答错误
	// 	}
	// }

	// if( (PsamRcvBuf[0]==0x61) && (PsamRcvBufCount==2) )
	// {
	// 	//取响应数据
	// 	PsamSndBuf[0]=0x00;
	// 	PsamSndBuf[1]=0xc0;
	// 	PsamSndBuf[2]=0x00;
	// 	PsamSndBuf[3]=0x00;
	// 	PsamSndBuf[4]=PsamRcvBuf[1];
	// 	//发送以后的数据
	// 	delay_ms(2);
	// 	PsamSend(1,PsamSndBuf,5);
	// 	//接收数据
	// 	PsamReceive(1,PsamRcvBuf,PsamSndBuf[4]+3,2000);
	// }
    // //检查接收到的数据
	// if(PsamRcvBufCount<2)
	// 	return PSAM_COM_ERROR; //应答错误
	// else if(PsamRcvBufCount==2) 
	// {
	// 	memcpy(Rcvdata,PsamRcvBuf,PsamRcvBufCount);
	// 	*RcvLen=PsamRcvBufCount;
	// 	return 0;
	// }
	// else//转移接收到的数据
	// {
	// 	memcpy(Rcvdata,&PsamRcvBuf[1],PsamRcvBufCount-1); //第一个字节为INS
	// 	*RcvLen=PsamRcvBufCount-1;
	// 	return 0;
	// }
}

uint16_t GetPsamResetATR(uint8_t Sel ,uint8_t *pBufer)
{ 
	// uint8_t Revbuf[32];
	// uint16_t RevLen=0;
	// PsamReset();
	// RevLen=PsamReceive(1,Revbuf,30,2000);//50Ms
	// memcpy(pBufer,Revbuf,RevLen);
	// return RevLen;
}
//PSAM卡的测试函数
void ISO7816Test(void)
{
	// uint8_t ATR[32];
	// uint8_t buf[8];


	// char pBufer[32];
	// uint16_t RevLen;

	// RevLen=GetPsamResetATR(1,ATR);
	// if(RevLen)
	// {
	// 	RevLen=Psam_GetChallenge(1,buf,8);
	// 	if(RevLen==0x9000)
	// 	{
	// 		delay_ms(10);
	// 	}
	// 	else
	// 	{
	// 			DISP_HARDEErrorSub(PASM_ERROR);
	// 			while(1);
	// 	}
	// }
	// else
	// {
	// 		DISP_HARDEErrorSub(PASM_ERROR);
	// 		while(1);
	// }		
}


//------------------------------------------------------------------
//下列函数均为"卡操作应用"相关的
//------------------------------------------------------------------

uint8_t ReadCardCommonDatas_CPU(void)
{
	uint16_t 	 sw1sw2=0,len,i,CPU_RcLen;
	uint8_t	 FileAID[10];
	uint8_t buf[20];
	uint8_t CPU_CommBuf[256];
	
	//默认主文
	//选择一卡通文件
	FileAID[0]=0x3f;
	FileAID[1]=0x00;//zjx_change_20130606
	sw1sw2=CPU_Select_File_AID(0,FileAID,&CPU_RcLen,CPU_CommBuf);

	#ifdef PRINT_DBUG//
			printf("sw1sw2=:%02X \n",sw1sw2);
	#endif
	if(sw1sw2!=0x9000)
	{
		if((sw1sw2&0xf000)==0x9000||(sw1sw2&0xf000)==0x6000)
			return	CPU_SELFILE_FAIL;
		else
			return	CARD_NOCARD;
	}
	// FileAID[0]=0x00;
	// FileAID[1]=0x10;//zjx_change_20130606
	// sw1sw2=CPU_Select_File_AID(0,FileAID,CPU_RcLen,CPU_CommBuf);	
	// if(sw1sw2!=0x9000)
	// {
	// 	if((sw1sw2&0xf000)==0x9000||(sw1sw2&0xf000)==0x6000)
	// 		return	CPU_SELFILE_FAIL;
	// 	else
	// 		return	CARD_NOCARD;
	// }
	
	// sw1sw2=CPU_Read_Binary(0,0x15,0x00,0x04,CPU_CommBuf);//读取卡编号
	// /**************************************************************/	
	// if(sw1sw2==0x9000)//
	// {
	// 	memcpy(CardPrinterNum,CPU_CommBuf,4);
	// }
	// else
	// 	return	CARD_NOCARD;
	// sw1sw2=CPU_Read_Binary(0,0x16,0x00,0x37,CPU_CommBuf);//读取姓名证件号
	// /**************************************************************/	
	// if(sw1sw2==0x9000)//
	// {//获取卡姓名
	// 	for(i=2;i<22;i++)
	// 	{
	// 		if(CPU_CommBuf[i]==0&CPU_CommBuf[i+1]==0)
	// 		{
	// 			memcpy(CardName,CPU_CommBuf+2,i-2);
	// 			//最后一个字节存储有用姓名字节的长度
	// 			CardName[20] = i-2;
	// 			break;
	// 		}
	// 	}
		
	// 	//获取证件号
	// 	HexStringToHexGroup(CPU_CommBuf+44,buf,2);
	// 	memcpy(IDNumber,buf,1);
	// 	len = CPU_CommBuf[46];
	// 	HexStringToHexGroup(CPU_CommBuf+47,buf,len);
	// 	memcpy(IDNumber+1,buf,len/2);
	// 	HexStringToHexGroup(CPU_CommBuf+47+len,buf,CPU_CommBuf[47+len]);
	// 	memcpy(IDNumber+len/2,buf,CPU_CommBuf[47+len]/2);

	// }
	// else
	// 	return	CARD_NOCARD;
	// sw1sw2=CPU_Read_Binary_ID(0,0x15,0x1D,0x01,CPU_CommBuf);//读取卡身份
	// /**************************************************************/	
	// if(sw1sw2==0x9000)//
	// {
	// 	CardIdentity = CPU_CommBuf[0];
		
	// //	RequestConsume();
	// 	return	CARD_OK;
	// }
	// else
	// 	return	CARD_NOCARD;

}










