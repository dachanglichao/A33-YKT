
#include "../ExternVariableDef.h"
#include "../mydefine.h"
#include "../systime/sysTime.h"
#include "../debug/debug.h"




/***********************************************************************/

/******************************************************************************/


uchar	CardAuthKeySub(uchar,uchar); 
uchar	ReadCardCommonDatas_M1 (void);//����������������
uchar	ReadCard_DaySumConsumMoney_M1(void);//�����ۼ������Ѷ�
uchar	ReadCardBalanceDatas_M1(uchar );//������Ӧ�����Ŀ����
uchar	WriteBalanceToCard_M1(uchar,uchar);//д��
void	CalPurseKeyCode(uchar *,uchar *,uchar *,uchar *);//����Ǯ����д������
uchar	ReWriteCardSub_M1(uchar);

//����֤
uchar	CardAuthKeySub(uchar bbit, uchar	BlockNum)
{//bbit=0-KeyA,bbit=1--KeyB
	uchar	status;
	uchar	st_data;
	uchar 	CardKey[6]={0xA0,0xA1,0xA2,0xA3,0xA4,0xA5};
	uchar 	CardKeyBuffer[12];
	uchar	i;
	//debug("CardKeyCode==%2X %2X %2X %2X %2X %2X ",CardKeyCode[0],CardKeyCode[1],CardKeyCode[2],CardKeyCode[3],CardKeyCode[4],CardKeyCode[5]);
	if (!bbit)//����������֤�̶�����
		memcpy(CardKey,CardKeyCode,6);
	else//Ǯ��д������ļ���
		CalPurseKeyCode(CardSerialNum,MatchCode,CalCardKey,CardKey);			
		//status=Mf500HostCodeKey(CardKey,CardKeyBuffer);
	if (bbit)
		st_data=0x61;//KEYB
	else
		st_data=0x60;//KEYA	
	for (i=0;i<5;i++)
	{
	//debug("CardSerialNum==%2X %2X %2X %2X \r\n",CardSerialNum[0],CardSerialNum[1],CardSerialNum[2],CardSerialNum[3]);
		status=MFRC522_Auth(st_data,BlockNum,CardKey,CardSerialNum);
		if (!status)
			break;
	}
	if (status)
		return	CARD_NOCARD;
	else
		return	CARD_OK;
}

//�������ż�������
uchar	ReadCardCommonDatas_M1 (void)
{
	uchar	status,i=0;
	ulong	iii =0;
	uchar	RdCardDatasBuffer[80];

	status=CardAuthKeySub(0,CardSector*4);
	if (status)
		{debug("CardAuth fail\n");
		return	CARD_NOCARD;
		}
	for (i=0;i<5;i++)
	{
		status=MFRC522_Read(CardSector*4,RdCardDatasBuffer);				
		if (!status)
		{	
			status=	BytesCheckSum(RdCardDatasBuffer,16);
			if (!status)
				break;
		}
	}
		debug("RdCardDatasBuffer==");
		for(i=0;i<16;i++)
			debug("%2X ",RdCardDatasBuffer[i]);
		debug("\r\n");
	if (status)
		return	CARD_NOCARD;
	if (memcmp(RdCardDatasBuffer,MatchCode,4))
		{
		debug("MatchCode err\r\n");
		return	MATCHCODE_ERR;//ϵͳ�û������
		}
	if (RdCardDatasBuffer[12])
		return	CARD_STATUS_ERROR;//��״̬����
	CardBatch= RdCardDatasBuffer[4] ;//������
	i=CardBatch/8;
	status=1<<(CardBatch%8);
	if (!(status&CardBatchEnable[i]))
		{debug("CardBatch err\r\n");
		return	CARD_BATCH_ERROR;
		}
 	if (DiagTimeString(0,RdCardDatasBuffer+9))
 		{debug("DiagTimeString err\r\n");
		return	CARD_OVERDATE;//����Ч�ڸ�ʽ���Ϸ�	
 		}
	status=memcmp(RdCardDatasBuffer+9,SysTimeDatas.TimeString,3);
	if (status==0xff)
		{debug("CARD_OVERDATE err\r\n");
		return	CARD_OVERDATE;//������Ч��
		}
	CardIdentity=BCDToHex( RdCardDatasBuffer[13] );//������
	memcpy(CardPrinterNum,RdCardDatasBuffer+5,4);//����	
	if (BCD_String_Diag(CardPrinterNum,4))
	{debug("CARD_NUMBER_ERROR err\r\n");
		return	CARD_NUMBER_ERROR;//���Ų��Ϸ�
	}
	iii=ChgBCDStringToUlong(CardPrinterNum,4);
	if(sqlite3_blaknumber_query_db(iii))
		return	CARD_LOSTCARDNUM;//��ʧ��*/
		
	iii=ChgBCDStringToUlong(CardPrinterNum,4);
	if (iii>MAXCARDPRINTERNUM)
		return	CARD_NUMBER_ERROR;//���ų�����Χ


	for (i=0;i<2;i++)
	{
		status=MFRC522_Read(CardSector*4+1,RdCardDatasBuffer);				
		if (!status)
		{
			status=BytesCheckSum(RdCardDatasBuffer,16);
			if (!status)
				break;
		}
	}
	if (status)
		return	CARD_NOCARD;

	for (i=0;i<3;i++)
	{//��������
		PinCode[i]=RdCardDatasBuffer[i]^MatchCode[i];
		status=MatchCode[3-i]&0x0f;
		status<<=4;
		status+=(MatchCode[3-i]>>4);
 		PinCode[i]^=status;
	}

	if(BCD_String_Diag(RdCardDatasBuffer+3,4))
		Max_ConsumMoney=0;
	else
		Max_ConsumMoney=ChgBCDStringToUlong(RdCardDatasBuffer+3,4);//�����޶�
	
	
	if(DayLimetFlag)//�������ص���
	{
			Limit_DayMoney = ChgBCDStringToUlong(DayLimetMoney,3);//���޶�
	}
	else//���տ��������
	{
		if(BCD_String_Diag(RdCardDatasBuffer+7,4))
			Limit_DayMoney=0;
		else
			Limit_DayMoney=ChgBCDStringToUlong(RdCardDatasBuffer+7,4);//���޶�
	}
	Limit_MoneySign = RdCardDatasBuffer[11];//���������޶�


//	Card_Rebate=RdCardDatasBuffer[12]; //���ۿ� 
//	if( !Card_Rebate || Card_Rebate==0xff )	 Card_Rebate=100;

	for (i=0;i<5;i++)
	{
		status=MFRC522_Read(CardSector*4+2,RdCardDatasBuffer);				
		if (!status)
		{
			status=BytesCheckSum(RdCardDatasBuffer,16);
			if (!status)
				break;
		}
	}
	if (status)
		return	CARD_NOCARD;
	for (i=0;i<10;i++)
	{
		if (RdCardDatasBuffer[i]<64)
			PursesSector[i]=RdCardDatasBuffer[i];
		else
			PursesSector[i]=0xff;//����Ǯ��Ӧ�õ�����
	}
	/*if(Rebate_YesOrNo_Flag)
	{
		Card_Rebate = Read_Rebate();//�����ۿ�ֵNo_Use
		if(Card_Rebate==0xff)
			return No_Use;
	}
	else*/
	{
		Card_Rebate = 100;
	}
	//��չ����
	status=CardAuthKeySub(0,15*4+1);
	if (status)
		{debug("CardAuth fail\n");
		return	CARD_NOCARD;
		}
	for (i=0;i<5;i++)
	{
		status=MFRC522_Read(15*4+1,RdCardDatasBuffer);				
		if (!status)
		{	
			status=	BytesCheckSum(RdCardDatasBuffer,16);
			if (!status)
				break;
		}
	}
	//memset(namebuf,0,20);
	for(i=0;i<16;i++)
	{	
	//	debug("%2X ",RdCardDatasBuffer[i]);
		if(RdCardDatasBuffer[i]==0x46)
		{	//debug("i=%d\r\n",i);
			memset(nameBuf,0,16);
			//gdHexGroupToHexString(RdCardDatasBuffer,nameBuf,i);
			nameBuf[0] = i;
			memcpy(nameBuf+1,RdCardDatasBuffer,i);
			
			break;
		}	
	}

	return	CARD_OK;
}

uchar	ReadCard_DaySumConsumMoney_M1(void)//�����ۼ������Ѷ�
{
	uchar	status,i;
	uchar	RdCardDatasBuffer[80];
	uint	ii,jj;
	status=CardAuthKeySub(0,PursesSector[0]*4);	
	if (status)
		return	CARD_NOCARD;
	for (i=0;i<5;i++)
	{//���������������ۼ����޶�
		status=MFRC522_Read(PursesSector[0]*4,RdCardDatasBuffer);				
		if (!status)
		{
			status=BytesCheckSum(RdCardDatasBuffer,16);
			if (!status)
				break;
		}
	}
	if (status)
		return	CARD_NOCARD;
	CardDayConsumMoney=0;
	memset(CardConsumDate,0,3);//������ݲ��Ϸ�����0
	memcpy(DaySumMoneyDatasBak,RdCardDatasBuffer,16);//���ݶ��������ۼ����Ѷ�
	memset(DaySumMoneyDatasBak+16,0,12);
	if (!BCD_String_Diag(RdCardDatasBuffer,4))
	{
		CardDayConsumMoney=ChgBCDStringToUlong(RdCardDatasBuffer,4);
		memcpy(CardConsumDate,RdCardDatasBuffer+4,3);//�ϴ����ѵ���Ч��
	}
//	if (!DiagTimeString(0,CardConsumDate))
//	{
//		ii=(uint)12 * BCDToHex(CardConsumDate[0])+BCDToHex(CardConsumDate[1]);//�ϴ����ѵ���
//		jj=(uint)12 * BCDToHex(SysTimeDatas.S_Time.YearChar)+BCDToHex(SysTimeDatas.S_Time.MonthChar);//��ǰ��
//		if (jj>ii)
//		{
//			jj-=ii;
//			if (jj>CardEnableMonths)
//				return	CARD_OVERDATE;
//		}
//	}
	if (!BCD_String_Diag(RdCardDatasBuffer+7,4))
	{
		memcpy(ConsumCountDateTime,RdCardDatasBuffer+7,4);
		CardDayConsumCount=RdCardDatasBuffer[11];
	}
	else
	{
		memset(ConsumCountDateTime,0,4);
		CardDayConsumCount=0;
	}
		
	return	CARD_OK;
}
uchar	ReadCardBalanceDatas_M1( uchar RdSector)//�����������Ƚ�
{
	uchar	status;
	ulong	Money1,Money2;
	uint	CardConsum1,CardConsum2;
	uchar	RdCardDatasBuffer[80];
	uchar	Date1[4],Date2[4];
	uchar		bitUseBlock1=0;
	uchar	i;
	status=CardAuthKeySub(1,RdSector*4);
	if (status)
		return	CARD_NOCARD;
	if (RdSector!=PursesSector[0])
	{//��Ǯ������Ч��
		for (i=0;i<5;i++)
		{
			status=MFRC522_Read(RdSector*4,RdCardDatasBuffer);
			if (!status)
			{
				status=BytesCheckSum(RdCardDatasBuffer,16);
				if (!status)
					break;
			}
		}
		if (status)
			return	CARD_NOCARD;//������
		if (!DiagTimeString(0,RdCardDatasBuffer+12))
		{
			status=memcmp(RdCardDatasBuffer+12,SysTimeDatas.TimeString,3);
			if (status==0xff)
				return	CARD_OVERDATE;//������Ч��
		}		
	}
	for (i=0;i<5;i++)
	{
		status=MFRC522_Read(RdSector*4+1,RdCardDatasBuffer);//���1
		if (!status)
		{
			status=BytesCheckSum(RdCardDatasBuffer,16);
			if (!status)
				break;
		}
	}
	if (status)
		return	CARD_NOCARD;//������
	if ( BCD_String_Diag(RdCardDatasBuffer,4))//����ʽ���
		return	CARD_DATA_ERROR;
	for (i=0;i<5;i++)
	{
		status=MFRC522_Read(RdSector*4+2,RdCardDatasBuffer+16);//���2
		if (!status)
		{
			status=BytesCheckSum(RdCardDatasBuffer+16,16);
			if (!status)
				break;
		}
	}
	if (status)
		return	CARD_NOCARD;//������
	if ( BCD_String_Diag(RdCardDatasBuffer+16,4))//����ʽ���
		return	CARD_DATA_ERROR;
	Money1=ChgBCDStringToUlong(RdCardDatasBuffer,4);
	Money2=ChgBCDStringToUlong(RdCardDatasBuffer+16,4);
	memcpy((uchar *)&CardConsum1,RdCardDatasBuffer+11,2);
	CardConsum1 = DoubleBigToSmall(CardConsum1 );  //adlc
	memcpy((uchar *)&CardConsum2,RdCardDatasBuffer+27,2);
	CardConsum2 = DoubleBigToSmall(CardConsum2 );//adlc
	if (Money1<Money2)
	{
		if (!CardConsum1)
		{
			if (CardConsum2!=65535)
				return	CARD_DATA_ERROR;
		}
		else
		{
			if (CardConsum1<=CardConsum2 || CardConsum1!=(CardConsum2+1))
				return	CARD_DATA_ERROR;				
		}
		bitUseBlock1=1;
	}
	else if (Money1>Money2)
	{
		if (!CardConsum2)
		{
			if (CardConsum1!=65535)
				return	CARD_DATA_ERROR;
		}
		else
		{
			if (CardConsum2<=CardConsum1 || CardConsum2!=(CardConsum1+1))
				return	CARD_DATA_ERROR;				
		}
		bitUseBlock1=0;
	}
	else
	{
		if (CardConsum1!=CardConsum2)
				return	CARD_DATA_ERROR;
		/////////��������ͬ���Ƚ���������///////////
		Date1[0]=RdCardDatasBuffer[4]&0x0f;
		Date1[1]=RdCardDatasBuffer[5]>>3;
		Date1[3]=RdCardDatasBuffer[6]&0x3f;
		i=RdCardDatasBuffer[5]&0x07;
		i<<=2;
		i+=(RdCardDatasBuffer[6]>>6);
		Date1[2]=i;

		Date2[0]=RdCardDatasBuffer[20]&0x0f;
		Date2[1]=RdCardDatasBuffer[21]>>3;
		Date2[3]=RdCardDatasBuffer[22]&0x3f;
		i=RdCardDatasBuffer[21]&0x07;
		i<<=2;
		i+=(RdCardDatasBuffer[22]>>6);
		Date2[2]=i;
		status=memcmp(Date1,Date2,4);//���ڱȽ�
		if (status==1)
			bitUseBlock1=1;	
		else
			bitUseBlock1=0;						
	}
	if (bitUseBlock1)
	{//��һ��
		bitUseMoneyBlock=1;
		status=0;
	}
	else
	{
		bitUseMoneyBlock=0;
		status=16;
	}
	memcpy(OldBalance,RdCardDatasBuffer+status,4);//���
	i=RdCardDatasBuffer[status+4];
	PurseConsumDateTime[0]=HexToBCD(i&0x0f);//��
	i=RdCardDatasBuffer[status+5];
	PurseConsumDateTime[1]=HexToBCD(i>>3);//��
	i=i&0x07;
	PurseConsumDateTime[2]=i<<2;
	i=RdCardDatasBuffer[status+6];
	PurseConsumDateTime[3]=HexToBCD(i&0x3f);//��
	i>>=6;
	i+=	PurseConsumDateTime[2];//ʱ
	PurseConsumDateTime[2]=HexToBCD(i);

	if (!BCD_String_Diag(RdCardDatasBuffer+status+7,4))
		PurseSumConsumMoney=ChgBCDStringToUlong(RdCardDatasBuffer+status+7,4);
	else
		PurseSumConsumMoney=0;

	memcpy(PurseContrlNum,RdCardDatasBuffer+status+11,2);//Ǯ����ˮ��
	memcpy(PurseBT_Num,RdCardDatasBuffer+status+13,2);//�������к�
	//------------------------------------------------------------------
	//����Ǯ��ԭʼ����
	if (bitUseBlock1)//1--�õ�1������Ϊ�������ǵ�2������
		memcpy(PurseDatas_Info,RdCardDatasBuffer+16,16);//�������ݱ���
	else
		memcpy(PurseDatas_Info,RdCardDatasBuffer,16);
	memset(PurseDatas_Info+16,0,8);
	//------------------------------------------------------------------
	if (RdSector==PursesSector[0])
	{//��Ǯ���жϵ׽�
		Money1=ChgBCDStringToUlong(OldBalance,4);
		Money2=ChgBCDStringToUlong(CardMinBalance,3);//�׽�Ƚ�
		if (Money1<Money2)
			return	CARD_LITTLEMONEY;//�����Ӧ���ڵ��ڿ��׽�һ�����Ѷ�
		else
			return	CARD_OK;	
	}
	else
		return	CARD_OK;
}
uchar	WriteBalanceToCard_M1(uchar bbit,uchar WrSector)//д��
{//bit == 0 ����״̬;bit == 1ȡ�����ѻ�дԭ�����
	uchar	status;
	ulong	iii;
	uchar	Buffer[80];
	uchar	BufferBak[80];
	uchar	st_data;
	uchar	i;
	uint16_t   	ii;

	if (bitUseMoneyBlock)//1--�õ�1������Ϊ�������ǵ�2������
		st_data=WrSector*4+2;
	else
		st_data=WrSector*4+1;

	memcpy(ReWriteCardDatasBak,PurseDatas_Info,16);	//ad by lc

	if (!bbit)//����д��
	{
	    //Ǯ���������
	   // memcpy(OldBalance,"\x99\x99\x99\x99",4);
		iii=ChgBCDStringToUlong(OldBalance,4);
		iii=iii-CurrentConsumMoney;	
		ChgUlongToBCDString(iii,NewBalance,4);
		memcpy(Buffer,NewBalance,4);//�����
		if(Limit_MoneySign==1)//���ۼ����Ѷ�
		{
			if (memcmp(PurseConsumDateTime,SysTimeDatas.TimeString+1,1))//����ͬһ��
				ChgUlongToBCDString(CurrentConsumMoney,Buffer+7,4);//Ǯ�������ۼ����Ѷ�
			else
			{
				iii=PurseSumConsumMoney+CurrentConsumMoney;
				ChgUlongToBCDString(iii,Buffer+7,4);
			}
		}
		else//�ۼ����Ѷ�
		{
			if (memcmp(PurseConsumDateTime,SysTimeDatas.TimeString+1,2))//����ͬһ��
				ChgUlongToBCDString(CurrentConsumMoney,Buffer+7,4);//Ǯ�������ۼ����Ѷ�
			else
			{
				iii=PurseSumConsumMoney+CurrentConsumMoney;
				ChgUlongToBCDString(iii,Buffer+7,4);
			}
		}
		//�ÿ�����
		Buffer[4]=BCDToHex(SysTimeDatas.S_Time.MonthChar);
		i=BCDToHex(SysTimeDatas.S_Time.DayChar);
		i<<=3;
		status=BCDToHex(SysTimeDatas.S_Time.HourChar);//ʱ
		i|=(status>>2);
		Buffer[5]=i;
		i=BCDToHex(SysTimeDatas.S_Time.HourChar)<<6;
		i|=BCDToHex(SysTimeDatas.S_Time.MinuteChar);
		Buffer[6]=i;//��
		if (CurrentConsumMoney)
		{
			ii=GetU16_HiLo(PurseContrlNum)+1;
			PutU16_HiLo(Buffer+11,ii);//����ˮ��
		}
		else
		{
			memcpy(Buffer+11,PurseContrlNum,2);//����ˮ��		
		}
		memcpy(Buffer+13,PurseBT_Num,2);//�������к�
		Buffer[15]=CalCheckSum(Buffer,15);
		PurseWrBufferBak[0]=st_data;
		memcpy(PurseWrBufferBak+1,Buffer,16);
		memset(PurseWrBufferBak+17,0,7);
	
	}
	else
	{
		memcpy(PurseWrBufferBak+1,PurseDatas_Info,16);
	}
	
    //��ʼ������
	status=Detect_Card();//Ѱ��,�ȽϿ���
	if (status)
	{
		debug("卡片复位失败\r\n");
		if (!bitBeepError)
		{
			bitBeepError=1;
		}
		return	CARD_NOCARD;
	}		
	bitBeepError=0;

	status=CardAuthKeySub(1,WrSector*4);//��֤
	if (!status)
	{	
		for (i=0;i<5;i++)
		{
			status=MFRC522_Write(PurseWrBufferBak[0],PurseWrBufferBak+1);
			if (!status)
			{
				status=MFRC522_Read(PurseWrBufferBak[0],BufferBak);
				if (!status)
				{
					status=memcmp(PurseWrBufferBak+1,BufferBak,16);
					if (!status)
						break;
				}
			}
		}
	}
	else
		status=CARD_NOCARD;


	if (status)//д������
		 return	CONSUM_PROCE_FAIL;
	
	
	if(!bbit)	
	{	//		//���ۼ����Ѷ�
		memcpy(Buffer,DaySumMoneyDatasBak,16);
		if(Limit_MoneySign==1)
		{
			if (memcmp(CardConsumDate,SysTimeDatas.TimeString,2))
				iii=CurrentConsumMoney;
			else
				iii=CardDayConsumMoney+CurrentConsumMoney;
		}
		else
		{
			if (memcmp(CardConsumDate,SysTimeDatas.TimeString,3))
				iii=CurrentConsumMoney;
			else
				iii=CardDayConsumMoney+CurrentConsumMoney;
		}
		ChgUlongToBCDString(iii,Buffer,4);
		memcpy(Buffer+4,SysTimeDatas.TimeString,3);
		memcpy(Buffer+7,ConsumCountDateTime,4);//�ƴ�����ʱ��ʱ��
		Buffer[11]=CardDayConsumCount;//�۸񷽰�ʱ�ļƴ�
		Buffer[15]=CalCheckSum(Buffer,15);
		memcpy(MainWrBufferBak+1,Buffer,16);

	}
	else
	   memcpy(MainWrBufferBak+1,DaySumMoneyDatasBak,16);
	MainWrBufferBak[0]=PursesSector[0]*4;
	memset(MainWrBufferBak+17,0,12);

	status=CardAuthKeySub(1,MainWrBufferBak[0]);
	if (!status)
	{
		for (i=0;i<5;i++)
		{
			status=MFRC522_Write(MainWrBufferBak[0],MainWrBufferBak+1);
			if (!status)
			{
				status=MFRC522_Read(MainWrBufferBak[0],BufferBak);
				if (!status)
				{
					status=memcmp(MainWrBufferBak+1,BufferBak,16);
					if (!status)
							break;
				}
			}
		}
	}
	if (status)
		return	CPU_WRITEPURSE_FAIL;
	else
		return	CARD_OK;
	
}
uchar	ReWriteCardSub_M1(uchar bbit)
{
	uchar status;
	uchar Buffer[16];
	uchar Buffer1[16];
	uchar temp;

	status=Detect_Card();//Ѱ��,�ȽϿ���
	if (!status)
	{
		bitBeepError=0;
		if(!bbit)
		{
			status=CardAuthKeySub(1,PurseWrBufferBak[0]);
			if (!status)
			{	
				status=MFRC522_Read(PurseWrBufferBak[0],Buffer);
				if(!status)
				{
					status=memcmp(Buffer,PurseDatas_Info,16);
					if(status)
					{
						status=memcmp(Buffer,PurseWrBufferBak+1,16);
						if(status)//�����Ѿ������仯
							return CARD_SAME_ERROR;
						else
						{
							status=CardAuthKeySub(1,MainWrBufferBak[0]);
							if(!status)
								status=MFRC522_Read(MainWrBufferBak[0],Buffer);
							if(!status)
							{
								status=memcmp(Buffer,DaySumMoneyDatasBak,16);
								if(status)
									return CARD_SAME_ERROR;
								status=CardAuthKeySub(1,PurseWrBufferBak[0]);
							}
							if(status)
								return	CONSUM_PROCE_FAIL;
						}
					}
					status=MFRC522_Write(PurseWrBufferBak[0],PurseWrBufferBak+1);
				}
			}
			if(status)
				return	CONSUM_PROCE_FAIL;
		}//д������
		else
		{//ֻд�ۼ�ʱ��֤����
			status=CardAuthKeySub(1,PurseWrBufferBak[0]);
			if (!status)	
				status=MFRC522_Read(PurseWrBufferBak[0],Buffer);
			if (!status)
			{		
				status=memcmp(Buffer,PurseWrBufferBak+1,16);
				if(status)//�����Ѿ������仯
					return CARD_SAME_ERROR;
				else
				{	
					if((PurseWrBufferBak[0]%4)==1)
						temp=PurseWrBufferBak[0]+1;
					else
						temp=PurseWrBufferBak[0]-1;
					status=MFRC522_Read(temp,Buffer1);
					if(!status)
					{
//						status=memcmp(Buffer,Buffer1,4);
//						if(status==1)
//							return CARD_SAME_ERROR;
//						else
//							status=0;
					}
					
				}		
			}
			if(status)
				return	CPU_WRITEPURSE_FAIL;				
		}
		//д���ۼƲ���
		status=CardAuthKeySub(1,MainWrBufferBak[0]);
		if (!status)
			status=MFRC522_Write(MainWrBufferBak[0],MainWrBufferBak+1);
		if (!status)
			return	CARD_OK;
		else
			return	CPU_WRITEPURSE_FAIL;
	}
	else
		return	CARD_NOCARD;
}

uchar	Posi1[16]={7,3,11,8,13,2,10,5,1,14,0,6,15,4,9,12};
uchar	Posi2[16]={10,8,5,1,13,7,11,0,3,14,6,2,15,4,9,12};
uchar	Posi3[16]={0,2,9,13,5,15,3,8,6,11,10,14,7,12,1,4};
uchar	Posi4[16]={0,14,1,6,15,4,8,12,7,2,10,9,13,3,11,5};

void	ChgBytesSequence(uchar * Posi, uchar *ptr1 , uchar * ptr2,uchar Len)
{
	uchar	i;
	uchar	aa,bb,st_data;
	for (i=0;i<Len;i++)
	{
		aa=Posi[2*i];
		bb=aa%2;
		aa=aa/2;
		if (!bb)
			st_data=ptr1[aa]>>4;
		else
			st_data=ptr1[aa]&0x0f;
		st_data<<=4;
		aa=Posi[2*i+1];
		bb=aa%2;
		aa=aa/2;
		if (!bb)
			st_data+=(ptr1[aa]>>4);
		else
			st_data+=(ptr1[aa]&0x0f);	
		ptr2[i]=st_data;
	}
}

void	CalXOR(uchar bbit ,uchar * ptr1,uchar *	keyptr, uchar * ptr2)
{                    
	uchar	i,j;
	uchar	aaa[8];
	uchar	bbb[8];
	if (!bbit)
		ChgBytesSequence(Posi1,ptr1,aaa,8);
	else
		memcpy(aaa,ptr1,8);
	for (i=0;i<8;i++)
	{
		for(j=0;j<8;j++)
		{
			bbb[j]=aaa[(i+j)%8] ^ keyptr[(i+j+7)%8];
		}
		memcpy(aaa,bbb,8);			
	}
	if (bbit)
 		ChgBytesSequence(Posi2,bbb,ptr2,8);
	else
		memcpy(ptr2,bbb,8);
}


void	CalEncryptDatas(uchar bbit,uchar * S_Datas ,uchar * KeyDatas ,uchar *  T_Datas ,uchar Len )
{
	uchar 	ddd[7]={0x11,0x22,0x33,0x44,0x55,0x66,0x77};
	uchar	aaa[8];
	uchar	bbb[8];
	uchar 	i;
	uchar	st_data;
	memset(bbb,0,8);
	st_data=Len/8;
	for (i=0;i<st_data;i++)
	{
		CalXOR(bbit,S_Datas+i*8,KeyDatas,T_Datas+i*8);
	}
	st_data=Len%8;
	if (st_data)
	{
		memcpy(aaa,S_Datas+(Len/8)*8,st_data);
		memcpy(aaa+st_data,ddd,(8-st_data));
		if (bbit)
			ChgBytesSequence(Posi4,S_Datas+(Len/8)*8,aaa,8);					
		CalXOR(bbit,aaa,KeyDatas,bbb);
		if (!bbit)
			ChgBytesSequence(Posi3,bbb,T_Datas+(Len/8)*8,st_data);
		else
			memcpy(T_Datas+(Len/8)*8,bbb,st_data);	
	}
}	



//����Ǯ����д������
void	CalPurseKeyCode(uchar * SerialNum,uchar * UseCode ,uchar * CalKey,uchar * Key)//����Ǯ����д������
{//�ֱ�Ϊ������Ψһ���кţ�ƥ���֣�������Կ�������������
	uchar aaa[8];
	uchar bbb[8];
	aaa[0]=SerialNum[0];
	aaa[1]=UseCode[0];
	aaa[2]=SerialNum[1];
	aaa[3]=UseCode[1];
	aaa[4]=SerialNum[2];
	aaa[5]=UseCode[2];
	aaa[6]=SerialNum[3];
	aaa[7]=UseCode[3];
	CalEncryptDatas(0,aaa,CalKey,bbb,8);//����
	Key[0]=bbb[0]+bbb[7];
	Key[1]=bbb[1]+bbb[6];
	Key[2]=bbb[2]+bbb[5];
	Key[3]=bbb[3]+bbb[4];
	Key[4]=CalCheckSum(SerialNum,4);
	Key[4]=~Key[4];
	Key[5]=CalCheckSum(UseCode,4);
	Key[5]=~Key[5];
}



