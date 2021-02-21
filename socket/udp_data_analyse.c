
#include "../ExternVariableDef.h"
#include <stdio.h>  
#include <stdbool.h>  
#include <sys/un.h>
#include <sys/socket.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>
#include "../txt/readTxt.h"
#include "../sysTime/sysTime.h"
#include "../debug/debug.h"
#include "../socket/tcpclienttoPosServer.h"
#include "../sqlite3/possqlite3.h"
//#include "../crcFiles/crcFiles.h"

 unsigned char Tx_Buffer[2048];  //udp数据缓冲区
 uint16_t udp_len = 0;



 void	Init_Serial(void)
{
	SerialReceiveLen=0;
	bitSerial_ReceiveEnd=0;
	//NetFlag = 1;
	SerialReCase=0;
	SerialReceiveTimeCount=0;
	memset(SerialUnion.S_DatasBuffer,0,100);
} 


static void Chg_BlkNameSub(uint8_t bbit,uint8_t *blknum)//增加黑名单
{
	uint32_t iii=0;
	iii=ChgBCDStringToUlong(blknum,4);
	if(!bbit)//增加黑名单
	{
		sqlite3_blaknumber_insert_db(iii);
	}
	else//从黑名单数据库清除
	{
		sqlite3_blaknumber_del_db(iii);
	}

}
static void CalComdSymblDatas(uint8_t Comd,uint8_t * Datas)
{
	uint8_t  i;
	for (i=0;i<6;i++)
		Datas[i]=Comd+0x11* i;
}


void ReceiveSub(void)//接收数据处理
{
	uint8_t		status,j;
	uint8_t		DatasBuffer[40];
	uint8_t		aaa[6]={0,0,0,0,0,0};
	uint8_t    	buf[1];
	uint8_t   	Buffer[100];
	uint8_t  	LoadSum =0;
	uint8_t		i,PPage,Yearchar,MonthChar,DayChar,Nums;
	uint16_t 	bigtosmalltemp,downflag;
	uint32_t 	ii,Addr,iii,SumMoney;
	uint8_t		*key;
	struct sMoneyplanStruct stru;
	
	bitSerial_SendRequest=1;
	SerialUnion.S_DatasStruct.UartStatus=0;
	memset(Buffer,0,100);
	switch (SerialUnion.S_DatasStruct.UartComd & 0xfff)
	{
		debug("SerialUnion.S_DatasStruct.UartComd==%d\n",SerialUnion.S_DatasStruct.UartComd);
		case	RD_ADDR_COMD://读站点
			SerialUnion.S_DatasStruct.Datas[0]=MainCode>>8;
			SerialUnion.S_DatasStruct.Datas[1]=(uchar)MainCode;
			SerialUnion.S_DatasStruct.DatasLen=2;
			break;
		case    DOWNLODER_COMD:
			{
				LoadModeFlag = 1;
				LoadModeResponse[0] = STX;
				LoadModeResponse[1]=SerialUnion.S_DatasStruct.UartReAddrCode>>8;
				LoadModeResponse[2]=SerialUnion.S_DatasStruct.UartReAddrCode%256;
				LoadModeResponse[3]=SerialUnion.S_DatasStruct.UartSeAddrCode>>8	;
				LoadModeResponse[4]=SerialUnion.S_DatasStruct.UartSeAddrCode%256;
				LoadModeResponse[5]=SerialUnion.S_DatasStruct.UartComd	;
				LoadModeResponse[6]=SerialUnion.S_DatasStruct.UartStatus;
				LoadModeResponse[7]=SerialUnion.S_DatasStruct.UartAddrH	;
				LoadModeResponse[8]=SerialUnion.S_DatasStruct.UartAddrL	;
				LoadModeResponse[9]=SerialUnion.S_DatasStruct.UartFrameNum;
				LoadModeResponse[10]=2;
				LoadModeResponse[11]='G';
				LoadModeResponse[12]='G';
	
				for(i=1;i<13;i++)
				{
					LoadSum+=LoadModeResponse[i];
				}
				LoadModeResponse[13]=ETX;
				LoadModeResponse[14]=LoadSum;
			}
			break;
		case	RD_USERCODE_COMD://上传匹配字
			memcpy(DatasBuffer,MatchCode,4);
			DatasBuffer[4]=CalCheckSum(DatasBuffer,4);
			CalEncryptDatas(0,DatasBuffer,CommEncryptKey,SerialUnion.S_DatasStruct.Datas,5);//鍔犵爜
			SerialUnion.S_DatasStruct.DatasLen=5;
			SerialUnion.S_DatasStruct.UartComd|=0x8000;	//鏁版嵁鍔犲瘑
			break;
		case	SET_USERCODE_COMD://设置匹配字
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if ((SerialUnion.S_DatasStruct.UartComd & 0x8000) && SerialUnion.S_DatasStruct.DatasLen==5)
			{
				CalEncryptDatas(1,SerialUnion.S_DatasStruct.Datas,CommEncryptKey,DatasBuffer+1,5);//瑙ｅ瘑
				if (!BytesCheckSum(DatasBuffer+1,5))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					DatasBuffer[0]=0xa0;
					DatasBuffer[5]=CalCheckSum(DatasBuffer,5);
					
					key = "MatchCode";
					gdHexGroupToHexString(DatasBuffer+1,Buffer,4);
					if(memcmp(MatchCode,DatasBuffer+1,4))
					{
						memcpy(MatchCode,DatasBuffer+1,4);
						writeposInformation(key,Buffer);
						bitUpdateParameter=1;	
					}
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			SerialUnion.S_DatasStruct.UartComd&=0xfff;	 										
			break;
		case	SET_RDCARDCODE_COMD://下载读卡密码
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if ((SerialUnion.S_DatasStruct.UartComd & 0x8000) && SerialUnion.S_DatasStruct.DatasLen==7)
			{
				CalEncryptDatas(1,SerialUnion.S_DatasStruct.Datas,CommEncryptKey,DatasBuffer+1,7);//瑙ｅ瘑
				if (!BytesCheckSum(DatasBuffer+1,7))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					DatasBuffer[0]=0xa0;
					DatasBuffer[7]=CalCheckSum(DatasBuffer,7);
					
					key = "CardKeyCode";
					gdHexGroupToHexString(DatasBuffer+1,Buffer,6);
					if(memcmp(CardKeyCode,DatasBuffer+1,6))
					{
						memcpy(CardKeyCode,DatasBuffer+1,6);
						writeposInformation(key,Buffer);			
						SerialUnion.S_DatasStruct.DatasLen=0;
						bitUpdateParameter=1;	
					}
					
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			SerialUnion.S_DatasStruct.UartComd&=0xfff;	 										
			break;
		case	RD_RDCARDCODE_COMD://上传读卡密码
			memcpy(DatasBuffer,CardKeyCode,6);
			DatasBuffer[6]=CalCheckSum(DatasBuffer,6);
			CalEncryptDatas(0,DatasBuffer,CommEncryptKey,SerialUnion.S_DatasStruct.Datas,7);//鍔犲瘑
			SerialUnion.S_DatasStruct.DatasLen=7;
			SerialUnion.S_DatasStruct.UartComd|=0x8000;	//鏁版嵁鍔犲瘑			
			break;
		case	RD_COMMSECTOR_COMD://读出公共扇区号
			SerialUnion.S_DatasStruct.Datas[0]=CardSector;
			SerialUnion.S_DatasStruct.DatasLen=1;
			break;			
		case	SET_COMMSECTOR_COMD://设置公共扇区号
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==1)
			{
				SerialUnion.S_DatasStruct.UartStatus=0;
				
				key = "CardSector";
				debug("0001\n");
				if(CardSector !=SerialUnion.S_DatasStruct.Datas[0])
				{
					CardSector = SerialUnion.S_DatasStruct.Datas[0];
					debug("0002\n");
					gdHexGroupToHexString(&SerialUnion.S_DatasStruct.Datas[0],Buffer,1);
					debug("0003\n");
					writeposInformation(key,Buffer);	
					debug("0004\n");
					CardSector = SerialUnion.S_DatasStruct.Datas[0];
					bitUpdateParameter=1;	
				}
			}
			debug("0004\n");
			SerialUnion.S_DatasStruct.DatasLen=0;	
			break;
		case	SET_CALCARDKEY_COMD://下载写卡密码
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if ((SerialUnion.S_DatasStruct.UartComd & 0x8000) && SerialUnion.S_DatasStruct.DatasLen==9)
			{
				CalEncryptDatas(1,SerialUnion.S_DatasStruct.Datas,CommEncryptKey,DatasBuffer+1,9);//瑙ｅ瘑
				if (!BytesCheckSum(DatasBuffer+1,9))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					key = "CalCardKey";
					if(memcmp(CalCardKey,DatasBuffer+1,8))
					{
						memcpy(CalCardKey,DatasBuffer+1,8);
						gdHexGroupToHexString(DatasBuffer+1,Buffer,8);
						writeposInformation(key,Buffer);
						memcpy(CalCardKey,DatasBuffer+1,8);					
						bitUpdateParameter=1;	
					}
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			SerialUnion.S_DatasStruct.UartComd&=0xfff;
			break;
		case	RD_CALCARDKEY_COMD://上传写卡密码
			memcpy(DatasBuffer,CalCardKey,8);
			DatasBuffer[8]=CalCheckSum(DatasBuffer,8);
			CalEncryptDatas(0,DatasBuffer,CommEncryptKey,SerialUnion.S_DatasStruct.Datas,9);//鍔犲瘑
			SerialUnion.S_DatasStruct.DatasLen=9;
			SerialUnion.S_DatasStruct.UartComd|=0x8000;
			break;

		case	POS_RST_COMD:   //POS机复位
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(POS_RST_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					bitNeedRST=1;
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
			
		case	SET_BATCH_COMD://设置批次是否有效
			if (SerialUnion.S_DatasStruct.DatasLen==32)
			{
				memcpy(DatasBuffer,SerialUnion.S_DatasStruct.Datas,32);
//					debug("鍗℃壒娆?==");
//				for(i=0;i<32;i++)
//					debug("%2X",DatasBuffer[i]);

				if(memcmp(CardBatchEnable,DatasBuffer,32))
				{
					memcpy(CardBatchEnable,DatasBuffer,32);
					key = "CardBatchEnable";
					gdHexGroupToHexString(DatasBuffer,Buffer,32);
					writeposInformation(key,Buffer);
					bitUpdateParameter=1;	
				}
			}
			else
				SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	RD_BATCH_COMD://读出批次
			SerialUnion.S_DatasStruct.DatasLen=32;
			memcpy(SerialUnion.S_DatasStruct.Datas,CardBatchEnable,32);
	   		break;
		case	RD_MINMONEY_COMD://上传底金
			SerialUnion.S_DatasStruct.Datas[0]=0;
			memcpy(SerialUnion.S_DatasStruct.Datas+1,CardMinBalance,3);
			SerialUnion.S_DatasStruct.DatasLen=4;			
			break;
		case	SET_MINMONEY_COMD://下载底金
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==4 && !BCD_String_Diag(SerialUnion.S_DatasStruct.Datas,4))
			{
				SerialUnion.S_DatasStruct.UartStatus=0;
				key = "CardMinBalance";
				if(memcmp(CardMinBalance,SerialUnion.S_DatasStruct.Datas+1,3))
				{
					gdHexGroupToHexString(DatasBuffer+1,SerialUnion.S_DatasStruct.Datas+1,3);
					writeposInformation(key,Buffer);
					memcpy(CardMinBalance,DatasBuffer+1,3);
					bitUpdateParameter=1;		
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;			
			break;
		case	RD_DAYLIMET_COMD://读取日限额
			SerialUnion.S_DatasStruct.Datas[0]=0;
			memcpy(SerialUnion.S_DatasStruct.Datas+1,DayLimetMoney,3);
			SerialUnion.S_DatasStruct.DatasLen=4;			
			break;
		case	SET_DAYLIMET_COMD://设置日限额
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==4 && !BCD_String_Diag(SerialUnion.S_DatasStruct.Datas,4))
			{
				SerialUnion.S_DatasStruct.UartStatus=0;
				key = "DayLimetMoney";
				if(DayLimetMoney,SerialUnion.S_DatasStruct.Datas+1,3)
				{
					gdHexGroupToHexString(SerialUnion.S_DatasStruct.Datas+1,Buffer,3);
					writeposInformation(key,Buffer);
					memcpy(DayLimetMoney,DatasBuffer+1,3);
					bitUpdateParameter=1;		
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;			
			break;
		case	RD_BLKNAME_TIME_COMD://读取黑名单有效期
			SerialUnion.S_DatasStruct.Datas[0]=CardEnableMonths;
			SerialUnion.S_DatasStruct.DatasLen=1;
			break;
		case	SET_BLKNAMETIME_COMD://设置黑名单有效期
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==1)
			{
				/*SerialUnion.S_DatasStruct.UartStatus=0;
				DatasBuffer[0]=0xa0;
				DatasBuffer[1]=SerialUnion.S_DatasStruct.Datas[0];
				DatasBuffer[2]=CalCheckSum(DatasBuffer,2);
				WrBytesToAT24C64(CardEnableMonths_Addr,DatasBuffer,3);
				bitUpdateParameter=1;*/	
			}
			SerialUnion.S_DatasStruct.DatasLen=0;		
			break;
		case	CLR_PURSE_COMD://读出钱包
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(CLR_PURSE_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
				SerialUnion.S_DatasStruct.UartStatus=0;
					/*SerialUnion.S_DatasStruct.UartStatus=0;
					Erase_One_Sector(PurseKind_Addr);
					Erase_One_Sector(PurseEnable_Addr);
					memcpy(DatasBuffer,DefaultKind,4);
					Flash_Write_Bytes(PurseKind_Addr,DatasBuffer,4);
					Flag_NotDefault=1;*/
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
		
			break;
		case	SET_PURSE_COMD://设置钱包
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;	
			if (SerialUnion.S_DatasStruct.UartAddrL<100 && !(SerialUnion.S_DatasStruct.DatasLen%13))
			{
				SerialUnion.S_DatasStruct.UartStatus=0;	
				if ((SerialUnion.S_DatasStruct.UartAddrL+(SerialUnion.S_DatasStruct.DatasLen/13) )<101)
					PPage=SerialUnion.S_DatasStruct.DatasLen/13;
				else
					PPage=100-SerialUnion.S_DatasStruct.UartAddrL;	
	
//				for (i=0;i<PPage;i++)
//				{
//				    DatasBuffer[0]=0xa0;
//					DatasBuffer[1]=SerialUnion.S_DatasStruct.UartAddrL+i;
//					memcpy(DatasBuffer+2,SerialUnion.S_DatasStruct.Datas+i*13,13);
//					DatasBuffer[15]=CalCheckSum(DatasBuffer,15);
//					Flash_Write_Bytes(Addr,DatasBuffer,16);
//					Addr+=16;
//				}	
				if(memcmp(PurseEnable,SerialUnion.S_DatasStruct.Datas,10))
				{
					memcpy(PurseEnable,SerialUnion.S_DatasStruct.Datas,10);
					key = "PurseEnable";
					gdHexGroupToHexString(SerialUnion.S_DatasStruct.Datas,Buffer,13);
					writeposInformation(key,Buffer);
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	RD_TIME2_COMD://读取系统时间
			//Read_Sysdate(SysTimeDatas.TimeString);
			memcpy(SerialUnion.S_DatasStruct.Datas,SysTimeDatas.TimeString,7);
			SerialUnion.S_DatasStruct.DatasLen=7;
			break;
		case	SET_TIME2_COMD://设置系统时间
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==7 && !DiagTimeString(0,SerialUnion.S_DatasStruct.Datas) && !DiagTimeString(1,SerialUnion.S_DatasStruct.Datas+3))
			{
				SerialUnion.S_DatasStruct.UartStatus=0;
				hw_pcf8563_set_sysdate(SerialUnion.S_DatasStruct.Datas);
				lib_systime_set_systime(SerialUnion.S_DatasStruct.Datas[0],SerialUnion.S_DatasStruct.Datas[1],SerialUnion.S_DatasStruct.Datas[2],
										SerialUnion.S_DatasStruct.Datas[3],SerialUnion.S_DatasStruct.Datas[4],SerialUnion.S_DatasStruct.Datas[5]);
				//Set_Sysdate(SerialUnion.S_DatasStruct.Datas);
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	CLR_BLKNUM_COMD://清除黑名单
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(CLR_BLKNUM_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					//Clr_PosSub(1,2);
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	ADD_BLKNUM_COMD://下载黑名单
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;	
		
			if (SerialUnion.S_DatasStruct.DatasLen && !(SerialUnion.S_DatasStruct.DatasLen%4))
			{
				status=SerialUnion.S_DatasStruct.DatasLen/4;
				for(i=0;i<status;i++)
				{
					//iii=ChgBCDStringToUlong(SerialUnion.S_DatasStruct.Datas+i*4,4);
					//gdHexGroupToHexString(SerialUnion.S_DatasStruct.Datas +i*4,Buffer,4);
					debug("blknumber = %s\n",Buffer);
					Chg_BlkNameSub(0,SerialUnion.S_DatasStruct.Datas +i*4);
				}
				SerialUnion.S_DatasStruct.UartStatus=0;
			}	
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	DEL_BLKNUM_COMD://删除黑名单
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;	
			if (SerialUnion.S_DatasStruct.DatasLen && !(SerialUnion.S_DatasStruct.DatasLen%4))
			{
				status=SerialUnion.S_DatasStruct.DatasLen/4;
				for(i=0;i<status;i++)
				{
					//iii=ChgBCDStringToUlong(SerialUnion.S_DatasStruct.Datas+i*4,4);
					//gdHexGroupToHexString(SerialUnion.S_DatasStruct.Datas +i*4,Buffer,4);
					Chg_BlkNameSub(1,SerialUnion.S_DatasStruct.Datas +i*4);
				}
				SerialUnion.S_DatasStruct.UartStatus=0;
			}	
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	DEL_ADD_BLKNUM_COMD:
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;	
			if (SerialUnion.S_DatasStruct.DatasLen && !(SerialUnion.S_DatasStruct.DatasLen%5))
			{
				status=SerialUnion.S_DatasStruct.DatasLen/5;
				for(i=0;i<status;i++)
				{	
					//iii=ChgBCDStringToUlong(SerialUnion.S_DatasStruct.Datas+i*5+1,4);
					gdHexGroupToHexString(SerialUnion.S_DatasStruct.Datas +i*4,Buffer,4);
					//if (iii<MAXCARDPRINTERNUM)
					{
						if (SerialUnion.S_DatasStruct.Datas[i*5]==0x55)
							Chg_BlkNameSub(0,Buffer);
						if (SerialUnion.S_DatasStruct.Datas[i*5]==0xaa)
							Chg_BlkNameSub(1,Buffer);
					}	
				}
				SerialUnion.S_DatasStruct.UartStatus=0;
			}	
			SerialUnion.S_DatasStruct.DatasLen=0;						
			break;
		case	CLR_POSDATAS_COMD://清除POS机数据
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==7)
			{
				CalComdSymblDatas(CLR_POSDATAS_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas+1,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					/*bitNeedRST=1;
					Disp_Clr_Ram();
					Clr_PosSub(1,SerialUnion.S_DatasStruct.Datas[0]);
					STX_FLAG =1;*/
					sqlite3_consume_clr_db();
					sqlite3_blaknumber_clr_db();					
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	RD_POSSTATUS_COMD://读取POS机状态
			//InitSaveRecordSub(0);
			SerialUnion.S_DatasStruct.Datas[0]=0;
			bigtosmalltemp = DoubleBigToSmall(RecordSum); //adlc
			memcpy(SerialUnion.S_DatasStruct.Datas+1,(uchar *)&bigtosmalltemp,2);
			bigtosmalltemp = DoubleBigToSmall(NoCollectRecordSum); //adlc
			memcpy(SerialUnion.S_DatasStruct.Datas+3,(uchar *)&bigtosmalltemp,2);
			bigtosmalltemp = DoubleBigToSmall(ReCollectRecordIndex);//adlc
			memcpy(SerialUnion.S_DatasStruct.Datas+5,(uchar *)&bigtosmalltemp,2);
			bigtosmalltemp = DoubleBigToSmall(SaveRecordIndex);//adlc
			memcpy(SerialUnion.S_DatasStruct.Datas+7,(uchar *)&bigtosmalltemp,2);
			SerialUnion.S_DatasStruct.DatasLen=9;
			break;
		case	SET_ENCRYPTKEY_COMD://下载传输密钥
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==8)
			{
				SerialUnion.S_DatasStruct.UartStatus=0;
				
				key = "CommEncryptKey";
				if(memcmp(CommEncryptKey,SerialUnion.S_DatasStruct.Datas,8))
				{
					memcpy(CommEncryptKey,SerialUnion.S_DatasStruct.Datas,8);
					gdHexGroupToHexString(SerialUnion.S_DatasStruct.Datas,Buffer,8);
					debug("浼犺緭绉橀挜== %s\r\n",Buffer);
					writeposInformation(key,Buffer);
					bitUpdateParameter=1;	
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;	
			break;
		case	RD_ENCRYPTKEY_COMD://上传传输密钥
			memcpy(SerialUnion.S_DatasStruct.Datas,CommEncryptKey,8);
			SerialUnion.S_DatasStruct.DatasLen=8;
			break;
		case	RD_CONSUMMONEY_COMD://读出消费额
			/*if (!SerialUnion.S_DatasStruct.UartAddrL)//A值
				SumMoney=Sys_SumConsumMoney;
			else if (SerialUnion.S_DatasStruct.UartAddrL==1)
			{//L值
				SumMoney=0;
				ii=NoCollectRecordIndex;
				i=0;
				while (ii!=SaveRecordIndex)
				{
					Addr=(ulong)ii*RECORD_SIZE;
				//	PPage=*(1+(uchar *)&iii);
				//	memcpy((uchar *)&Addr,(2+(uchar *)&iii),2);
					Flash_Rd_Bytes(Addr,DatasBuffer,32);
					if (DatasBuffer[0]==0xa0 && !BytesCheckSum(DatasBuffer+1,31)&& !(DatasBuffer[1]&0x80))
						SumMoney+=ChgBCDStringToUlong(DatasBuffer+15,3);
					ii=(ii+1)%MAXRECORD_NUM;
					i++;
					if (!(i%64))
						SerialSendChar(STX);
				}
			}
			else if (SerialUnion.S_DatasStruct.UartAddrL==2 && SerialUnion.S_DatasStruct.DatasLen==2)
			{//D值
				InitSaveRecordSub(0);
				SumMoney=0;
				ii=ReCollectRecordIndex;
				i=0;
				while (ii!=SaveRecordIndex)
				{
					Addr=(ulong)ii*RECORD_SIZE;
					//PPage=*(1+(uchar *)&iii);
					//memcpy((uchar *)&Addr,(2+(uchar *)&iii),2);
					Flash_Rd_Bytes(Addr,DatasBuffer,32);
					if (!DatasBuffer[0] || DatasBuffer[0]==0xa0)
					{
						if (!BytesCheckSum(DatasBuffer+1,31)&& !(DatasBuffer[1]&0x80))
						{
							ChgRecordDatasToTime(DatasBuffer+22,aaa);
							if ((!SerialUnion.S_DatasStruct.Datas[0]|| SerialUnion.S_DatasStruct.Datas[0]==aaa[1]) && 
								(!SerialUnion.S_DatasStruct.Datas[1]|| SerialUnion.S_DatasStruct.Datas[1]==aaa[2])	)
								SumMoney+=ChgBCDStringToUlong(DatasBuffer+15,3);
						}
					}
					i++;
					if (!(i%64))
						SerialSendChar(STX);
				}
			}
			else
				SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			SumMoney = FourBigToSmall(SumMoney);
			memcpy(SerialUnion.S_DatasStruct.Datas,(uchar *)&SumMoney,4);
			SumMoney = FourBigToSmall(SumMoney);
			SerialUnion.S_DatasStruct.DatasLen=4;
			break;
			*/
		case	CLR_NENU_COMD://清菜单
			/*SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(CLR_NENU_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					Addr=MenuPrince_Addr;
					for (ii=0;ii<280;ii++)
					{
						if (!(ii%70))
						{
							SerialSendChar(STX); 
//							TCP_Send_STX();
						}						
						j=0xff;
						Clr_WatchDog();
						WrBytesToAT24C64(Addr++,&j,1);
						Clr_WatchDog();
					}
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			*/
			break;
		case	SET_MENU_COMD://设置菜单
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;	
			/*if (SerialUnion.S_DatasStruct.UartAddrL<70 && !(SerialUnion.S_DatasStruct.DatasLen%3))
			{
				SerialUnion.S_DatasStruct.UartStatus=0;	
				if ((SerialUnion.S_DatasStruct.UartAddrL+(SerialUnion.S_DatasStruct.DatasLen/3) )<71)
					PPage=SerialUnion.S_DatasStruct.DatasLen/3;
				else
					PPage=70-SerialUnion.S_DatasStruct.UartAddrL;	
				Addr=MenuPrince_Addr+(uint)SerialUnion.S_DatasStruct.UartAddrL*4;
				for (i=0;i<PPage;i++)
				{
					memcpy(DatasBuffer,SerialUnion.S_DatasStruct.Datas+i*3,3);
					DatasBuffer[3]=CalCheckSum(DatasBuffer,3);
					WrBytesToAT24C64(Addr,DatasBuffer,4);
					memset(DatasBuffer,0,4);
					RdBytesFromAT24C64(Addr,DatasBuffer,4);
					Addr+=4;
					Clr_WatchDog();
				}
			}*/
			
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	RD_MENU_COMD://读取菜单
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;	
			/*if (SerialUnion.S_DatasStruct.UartAddrL<70)
			{
				SerialUnion.S_DatasStruct.UartStatus=0;	
				Addr=MenuPrince_Addr+(uint)SerialUnion.S_DatasStruct.UartAddrL*4;
				if ((71-SerialUnion.S_DatasStruct.UartAddrL)>32)
					status=32;
				else
					status=70-SerialUnion.S_DatasStruct.UartAddrL;
				PPage=0;
				for (i=0;i<status;i++)
				{
					Clr_WatchDog();
					RdBytesFromAT24C64((Addr+i*4),DatasBuffer,4);
					if (!BytesCheckSum(DatasBuffer,4) && !BCD_String_Diag(DatasBuffer,3))
					{
						memcpy(SerialUnion.S_DatasStruct.Datas,DatasBuffer,3);
						PPage+=3;	
					}
					else
						break;
				}
				if (PPage)
					SerialUnion.S_DatasStruct.DatasLen=PPage;
				else
					SerialUnion.S_DatasStruct.UartStatus=NoDatas_Error;
			}
			*/
			break;
		case	CLR_MENUNAME_COMD://清除菜单
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			/*if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(CLR_MENUNAME_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					Erase_One_Sector(MenuName_Addr);
					Clr_WatchDog();
				}
			}*/
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	SET_MENUNAME_COMD://设置菜名
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;	
			/*if (SerialUnion.S_DatasStruct.UartAddrL<70 && !(SerialUnion.S_DatasStruct.DatasLen%16))
			{
				SerialUnion.S_DatasStruct.UartStatus=0;	
				if ((SerialUnion.S_DatasStruct.UartAddrL+(SerialUnion.S_DatasStruct.DatasLen/16) )<71)
					PPage=SerialUnion.S_DatasStruct.DatasLen/16;
				else
					PPage=70-SerialUnion.S_DatasStruct.UartAddrL;	
				Addr=MenuName_Addr+(uint)SerialUnion.S_DatasStruct.UartAddrL*16;
				for (i=0;i<PPage;i++)
				{
					memcpy(DatasBuffer,SerialUnion.S_DatasStruct.Datas+i*16,16);
					Flash_Write_Bytes(Addr,DatasBuffer,16);
					Addr+=16;
					Clr_WatchDog();
				}
			}
			*/
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	CLR_SORTPRINCE_COMD://清除价格方案
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(CLR_SORTPRINCE_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					sqlite3_moneyplan_clr_db();
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	SET_SORTPRINCE_COMD://设置价格方案
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen && !(SerialUnion.S_DatasStruct.DatasLen%13))
			{
				SerialUnion.S_DatasStruct.UartStatus=0;	
				PPage=SerialUnion.S_DatasStruct.DatasLen/13;
				ii=SerialUnion.S_DatasStruct.UartAddrL+PPage;
				if (ii>200)
					PPage=(uint)200-SerialUnion.S_DatasStruct.UartAddrL;
				for (i=0;i<PPage;i++)
				{
					DatasBuffer[0]=0xa0;
					memcpy(DatasBuffer+1,SerialUnion.S_DatasStruct.Datas+i*13,13);
					DatasBuffer[14]=CalCheckSum(DatasBuffer,14);
					stru.serid = MoneyPlanIndex;
					gdHexGroupToHexString(DatasBuffer,Buffer,15);
					memcpy(stru.recoedDatas,Buffer,30);
					sqlite3_moneyplan_insert_db(stru);
					MoneyPlanIndex++;
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	RD_SORTPRINCE_COMD://读出价格方案
			/*Addr=SortPrince_Addr+SerialUnion.S_DatasStruct.UartAddrL*16;
			if (((uint)256-SerialUnion.S_DatasStruct.UartAddrL)<8)
				PPage=(uint)256-SerialUnion.S_DatasStruct.UartAddrL;
			else
				PPage=8;
			SerialUnion.S_DatasStruct.DatasLen=0;
			for (i=0;i<PPage;i++)
			{
				Flash_Rd_Bytes(Addr,DatasBuffer,15);
				if (DatasBuffer[0]==0xa0 && !BytesCheckSum(DatasBuffer,15))
				{
					memcpy(SerialUnion.S_DatasStruct.Datas+i*13,DatasBuffer+1,13);
					SerialUnion.S_DatasStruct.DatasLen+=13;
				}
				else
					break;
			}
			break;*/

		case	RD_RECORD_COMD://采集交易记录
			ii=NoCollectRecordIndex;
			SerialSendNoCollectNum=0;
			debug("NoCollectRecordIndex = %d\n",NoCollectRecordIndex);
			debug("SaveRecordIndex = %d\n",SaveRecordIndex);
			if (SerialUnion.S_DatasStruct.DatasLen==1)
			{
				SerialUnion.S_DatasStruct.UartStatus=0;
				Nums=SerialUnion.S_DatasStruct.Datas[0];
				if (Nums>3)
					Nums=3;
				i=0;
				j=10;
				SerialUnion.S_DatasStruct.DatasLen=0;
				while( ( ii!=SaveRecordIndex ||  bitRecordFull ) && i<Nums && j)
				{
					sqlite3_consume_query_record_db(NoCollectRecordIndex +SerialSendNoCollectNum);//数据库中查找记录
					debug("recordSt.recordId    = %d\n",recordSt.recordId);
					debug("recordSt.recoedDatas = %s\n",recordSt.recoedDatas);
					gdHexStringToHexGroup(recordSt.recoedDatas, DatasBuffer, 32);
					SerialSendNoCollectNum++;
								
					if (DatasBuffer[0]==0xa0 && !BytesCheckSum(DatasBuffer+1,31))
					{
						memcpy(SerialUnion.S_DatasStruct.Datas+SerialUnion.S_DatasStruct.DatasLen,DatasBuffer+1,31);
						SerialUnion.S_DatasStruct.DatasLen+=31;
						i++;
					}
					else
					{
						debug("checksum fail \n");
					}
					
					ii=(ii+1)%MAXRECORD_NUM;
					j--;
				}
				if (!SerialUnion.S_DatasStruct.DatasLen)
				{
					if (ii==SaveRecordIndex)
						SerialUnion.S_DatasStruct.UartStatus=NoDatas_Error;//记录采集完成
					else
						SerialUnion.S_DatasStruct.UartStatus=Running_Status;//正在查找数据		
				}
			}
			break;
		case	DEL_RECORD_COMD://交易记录移指针
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(DEL_RECORD_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					if (SerialSendNoCollectNum)
					{
						while (( NoCollectRecordIndex!=SaveRecordIndex || bitRecordFull) && SerialSendNoCollectNum )
						{
							sqlite3_consume_move_db(NoCollectRecordIndex,SerialSendNoCollectNum);//已采记录数据库操作
		
							NoCollectRecordIndex = (NoCollectRecordIndex+SerialSendNoCollectNum)%MAXRECORD_NUM;
							SerialSendNoCollectNum = 0;
						}
					}
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	INIT_RECORD_PTR_COMD://初始化交易记录指针
//			sqlite3_consume_query_collectedRecord_db(aaa);
//			ReCollectRecordIndex = recordSt.recordId;
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	RD_RERECORD_COMD://复采交易记录
			SerialSendReCollectNum=0;
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==3)
			{
				SerialUnion.S_DatasStruct.UartStatus=0;
				Nums=SerialUnion.S_DatasStruct.Datas[0];//采集的个数
				if (Nums>3)
					Nums=3;
				Yearchar = SysTimeDatas.S_Time.YearChar;//采集的年
				MonthChar=SerialUnion.S_DatasStruct.Datas[1];//采集的月
				DayChar=SerialUnion.S_DatasStruct.Datas[2];//采集的日
				aaa[0] = Yearchar;
				aaa[1] = MonthChar;
				aaa[2] = DayChar;
				iii = gdChgBCDStringTouint32_t(aaa, 3);
				debug("澶嶉噰鏃舵湡= %d\n",iii );
				if(MonthChar !=SerialUnion.S_DatasStruct.Datas[1] ||DayChar !=SerialUnion.S_DatasStruct.Datas[2])
				{
					sqlite3_consume_query_collectedRecord_db(aaa);
					ReCollectRecordIndex = recordSt.recordId;
					ii=ReCollectRecordIndex;
					debug("ReCollectRecordIndex =%d\n",ReCollectRecordIndex);
				}
				i=0;
				SerialUnion.S_DatasStruct.DatasLen=0;
				bitHaveReCollectRecord=0;
				j=10;
				while ((ii!=SaveRecordIndex ||bitRecordFull) && i<Nums && j)
				{
					
					{
						sqlite3_consume_query_record_db(ReCollectRecordIndex +SerialSendNoCollectNum);//数据库中查找记录
						debug("recordSt.recordId    = %d\n",recordSt.recordId);
						debug("recordSt.recoedDatas = %s\n",recordSt.recoedDatas);
						gdHexStringToHexGroup(recordSt.recoedDatas, DatasBuffer, 32);
						SerialSendNoCollectNum++;
						debug("recordSt.ConsumeTime= %d\n",recordSt.ConsumeTime);
									
						if (DatasBuffer[0]==0xa0 && !BytesCheckSum(DatasBuffer+1,31) && iii==recordSt.ConsumeTime)
						{
							memcpy(SerialUnion.S_DatasStruct.Datas+SerialUnion.S_DatasStruct.DatasLen,DatasBuffer+1,31);
							SerialUnion.S_DatasStruct.DatasLen+=31;
							i++;
						}
						else
						{
							debug("checksum fail \n");
						}
					}
				
					ii=(ii+1)%MAXRECORD_NUM;
					//SerialSendReCollectNum++;
					bitHaveReCollectRecord=1;
					j--;
				}
				if (!SerialUnion.S_DatasStruct.DatasLen)
				{
					if (ii==SaveRecordIndex)
						SerialUnion.S_DatasStruct.UartStatus=NoDatas_Error;//记录采集完成
					else
						SerialUnion.S_DatasStruct.UartStatus=Running_Status;//正在查找数据	
				}
			}
			break;
		case	DEL_RERECORD_COMD://复采记录移指针
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(DEL_RERECORD_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
					SerialUnion.S_DatasStruct.UartStatus=0;
					if (bitHaveReCollectRecord)
					{
						bitHaveReCollectRecord=0;
						while (( ReCollectRecordIndex!=SaveRecordIndex || bitRecordFull)  &&  SerialSendReCollectNum )
						{
							ReCollectRecordIndex=(ReCollectRecordIndex+1)%MAXRECORD_NUM;
							SerialSendReCollectNum--;
					 	}
					}	
				}
			}
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
		case	LOAD_PROGAM_COMD://固件更新
			SerialUnion.S_DatasStruct.UartStatus=ReDatas_Error;
			/*if (SerialUnion.S_DatasStruct.DatasLen==6)
			{
				CalComdSymblDatas(LOAD_PROGAM_COMD,DatasBuffer);
				if (!memcmp(SerialUnion.S_DatasStruct.Datas,DatasBuffer,6))
				{
				
					bitNeedDownLoad=1;
					SerialUnion.S_DatasStruct.UartStatus=0;
				}
			}
		
			SerialUnion.S_DatasStruct.DatasLen=0;*/
			break;
		default:
			SerialUnion.S_DatasStruct.UartStatus=ReComd_Error;
			SerialUnion.S_DatasStruct.DatasLen=0;
			break;
	}
}

static void	UdpAscToHex(uint8_t * Sum,uint8_t aa)
{
	uint8_t	bb;
	bb=aa>>4;
	if (bb<10)
		bb+=0x30;
	else
		bb+=0x37;
	Sum[0]+=bb;
	Tx_Buffer[udp_len++] = bb;

	bb=aa&15;
	if (bb<10)
		bb+=0x30;
	else
		bb+=0x37;
	Sum[0]+=bb;
	
	Tx_Buffer[udp_len++] = bb;
		
}

void	udpSendSub(void)//udp 鏁版嵁鍙戦??
{
	uint8_t	i;
	uint16_t	Sum;
	volatile uchar	CheckSum;

	udp_len = 0;
	Tx_Buffer[udp_len++] = STX;
	SerialUnion.S_DatasStruct.UartSeAddrCode=MainCode;
	SerialUnion.S_DatasStruct.UartReAddrCode=0;
	if (SerialUnion.S_DatasStruct.UartStatus)
		SerialUnion.S_DatasStruct.DatasLen=0;

	SerialUnion.S_DatasStruct.UartReAddrCode = DoubleBigToSmall(SerialUnion.S_DatasStruct.UartReAddrCode);
	SerialUnion.S_DatasStruct.UartComd = DoubleBigToSmall(SerialUnion.S_DatasStruct.UartComd);
	SerialUnion.S_DatasStruct.UartSeAddrCode = DoubleBigToSmall(SerialUnion.S_DatasStruct.UartSeAddrCode);
	
	Sum=Cal_CRC_Sub(SerialUnion.S_DatasBuffer,SerialUnion.S_DatasStruct.DatasLen+11);

	for(i=0;i<SerialUnion.S_DatasStruct.DatasLen+11;i++)
	{
		UdpAscToHex(&CheckSum,SerialUnion.S_DatasBuffer[i]);	
	}
	Tx_Buffer[udp_len++] = ETX;
	UdpAscToHex(&CheckSum,Sum/256);
	UdpAscToHex(&CheckSum,Sum%256);

	UdpSocketSendDataToPosServer(Tx_Buffer,udp_len);
	Init_Serial();
	debug("杩斿洖鏁版嵁==");
	debug("%s\r\n",Tx_Buffer);
	for(i=0;i<udp_len;i++)
	{
		debug("%2X ",Tx_Buffer[i]);
	}
	debug("\r\n");
}


 


