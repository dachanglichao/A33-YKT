
#include "byteConversion.h"

//字节流转换为十六进制字符串
void ByteToHexStr( const char *sSrc,  char *sDest, int nSrcLen )
{
    int  i;
    char szTmp[3];


    for( i = 0; i < nSrcLen; i++ )
    {
        sprintf( szTmp, "%02X", (unsigned char) sSrc[i] );
        memcpy( &sDest[i * 2], szTmp, 2 );
    }
    return ;
}



//十六进制字符串转换为字节流
void HexStrToByte(const uint8_t* source, uint8_t* dest, int sourceLen)
{
    short i;
    unsigned char highByte, lowByte;
    
    for (i = 0; i < sourceLen; i += 2)
    {
        highByte = toupper(source[i]);
        lowByte  = toupper(source[i + 1]);


        if (highByte > 0x39)
            highByte -= 0x37;
        else
            highByte -= 0x30;


        if (lowByte > 0x39)
            lowByte -= 0x37;
        else
            lowByte -= 0x30;


        dest[i / 2] = (highByte << 4) | lowByte;
    }
    return ;
}

uint32_t ChgKeyStringTouint32_t(uint8_t* ptr ,uint8_t Len)//BCD码字符串转换为整型数
{
	uint32_t ii=0;
	uint32_t jj=1;
	uint8_t	aa;
	do
	{
		aa=(*(ptr+Len-1));
		ii|=aa*jj;
		jj=jj*0x100;
	}while(--Len);
	return	ii;
}

uint32_t gdChgStringsToInt(uint8_t* ptr)
{
    uint8_t	i;
    uint16_t kk=1;
    uint32_t ii=0;
    uint8_t	st_data;
    for (i=0;i<5;i++)
    {
        st_data= ptr[4-i];
        ii+=(uint32_t)kk*st_data;	
        kk=kk*10;
    }	
    return	ii;
}

uint8_t gdBytesComp( uint8_t * CharDptr1,uint8_t * CharDptr2,uint8_t CompNum)
{
	uint8_t 			aa;
	uint8_t			bb;
	do
	{
		aa=* CharDptr1++;
		bb=* CharDptr2++;
		if (aa!=bb)	return 1; 
	}while (--CompNum);
	return 0;
}
uint8_t	gdBytesCheckSum(uint8_t * ptr,uint8_t Len)
{
	uint8_t			aa=0;
	uint8_t 			bb;
	Len--;
	do
	{
		aa+=* ptr++;
	}while (--Len);	 
	bb= * ptr;
	aa=~aa;
	if (aa!=bb) 
		return 1;
	else
		return 0;
}
uint8_t	gdCalCheckSum(uint8_t  * Ptr,uint8_t charLen)
{
	uint8_t 	st_data=0;
	do
	{
		st_data+= * Ptr++;
	}while (--charLen);
	st_data=~st_data;
	return st_data;
}
uint8_t	gdBCD_String_Diag(uint8_t * ptr ,uint8_t Len)//BCD码字符串诊断
{
	uint8_t			aa,bb;
	if (!Len)
		return	0;
	do
	{
		aa=* ptr++;
		bb=aa>>4;
		if (bb>9)
			return	1;
		bb=aa & 15;
		if (bb>9)
			return	1;
	}while (--Len);
	return	0;
}
uint32_t	gdChgBCDStringTouint32_t(uint8_t * ptr ,uint8_t Len)//BCD码字符串转换为整型数
{
	uint32_t		ii=0;
	uint32_t		jj=1;
	uint8_t		aa;
	do
	{
		aa=BCDToHex(* (ptr+Len-1));
		ii+=aa*jj;
		jj=jj*100;		
	}while (--Len);
	return	ii;
}
void	gdChguint32_tToBCDString( uint32_t iii,uint8_t * ptr,uint8_t Len)
{
	uint8_t 		i;
	uint8_t 		aa;
	uint32_t 		jj=1;
	for (i=0;i<Len-1;i++)
		jj=jj*100;
 	for (i=0;i<Len;i++)
 	{
 		aa=iii/jj;
		* ptr++=HexToBCD(aa);
		iii=iii%jj;
		jj=jj/100;
 	}	
}

uint8_t gdHexToBCD(uint8_t aa )
{
	return((aa/10)*16+aa%10);
}
uint8_t gdBCDToHex(uint8_t aa )
{
	return((aa/16)*10+aa%16);
}


void	gdChgIntToStrings(uint16_t	ii,uint8_t * ptr)
{
	uint16_t	kk=10000;
	uint8_t	st_data;
	uint8_t	i;
	for (i=0;i<5;i++)
	{
		st_data=ii/kk;
	//	ptr[i]=0x30+st_data;
		ptr[i]=st_data;
		ii=ii%kk;
		kk=kk/10;
	}	
}


uint32_t	gdChgInputTouint32_t(uint8_t * ptr,uint8_t Num)//输入的数字转换为长整形
{
	uint8_t	i,st_data,j;
	uint32_t	iii=0;
	uint32_t	jjj=100;
	uint8_t	SumNum=0;
//	uint8_t	PointX=0xff;
	uint8_t		bbit=0;
	for (i=0;i<Num;i++)
	{
		st_data=ptr[i];
		if (st_data!=0xff )
		{
			SumNum++;
			if ((st_data&0x80))
			{
				bbit=1;
				for (j=0;j<i;j++)
					jjj=jjj*10;	
			}
		}
		else
			break;
	}
	if (!bbit)
	{
		for (i=0;i<SumNum-1;i++)
			jjj=jjj*10;
	}
	for (i=0;i<SumNum;i++)
	{
		iii+=(ptr[i]&0x0f)*jjj;
		jjj=jjj/10;
		if (!jjj)
			break;
	}
	return	iii;
}
void	gdFormatBuffer(uint8_t	SLen,uint8_t * ptr ,uint8_t * Len)
{
	uint8_t i;
	uint8_t	j=0;
	uint8_t	aaa[10];
	uint8_t		bbit=0;
	memset(aaa,0xff,SLen);
	for (i=0;i<SLen;i++)
	{
		if (ptr[i] || bbit)
		{
			bbit=1;
			aaa[j++]=ptr[i];
		}		
	}
	memcpy(ptr,aaa,SLen);
	Len[0]=j;		
}

//取四字节数据(高位在前)
uint32_t gdGetU32_HiLo(uint8_t * lbuf)
{
	uint8_t * p_buf;
	uint32_t  r_buf;
	
	p_buf = lbuf;
	r_buf = (uint32_t)p_buf[3] + ((uint32_t)p_buf[2]<<8) + ((uint32_t)p_buf[1]<<16) + ((uint32_t)p_buf[0]<<24);
	return r_buf;
} 

//取双字节数据(高位在前)
uint16_t gdGetU16_HiLo(uint8_t * lbuf)
{
	uint8_t * p_buf;
	uint16_t  r_buf;
	
	p_buf = lbuf;
	r_buf = (uint16_t)p_buf[1] + ((uint16_t)p_buf[0]<<8);
	return r_buf;
} 
 
//设置四字节数据(高位在前)
void gdPutU32_HiLo(uint8_t * lbuf,uint32_t ldata)
{
	lbuf[0]=(uint8_t)(ldata>>24);
	lbuf[1]=(uint8_t)(ldata>>16);
	lbuf[2]=(uint8_t)(ldata>>8);
	lbuf[3]=(uint8_t)(ldata);
} 

//设置双字节数据(高位在前)
void gdPutU16_HiLo(uint8_t * lbuf,uint ldata)
{	
	lbuf[0]=(uint8_t)(ldata>>8);
	lbuf[1]=(uint8_t)(ldata);
}

//双字节大小端转换
uint16_t gdDoubleBigToSmall(uint16_t a)
{
	uint16_t c;
	unsigned char b[2];
	b[0] =(unsigned char) (a);
	b[1] = (unsigned char)(a>>8);
	c = (uint16_t)(b[0]<<8) + b[1];
	return c;
}

//四字节大小端转换
int32_t gdFourBigToSmall(uint32_t a)
{
	uint32_t c;
	unsigned char b[4];
	b[0] =(unsigned char) (a);
	b[1] = (unsigned char)(a>>8);
	b[2] = (unsigned char)(a>>16);
	b[3] = (unsigned char)(a>>24);
	c = (uint32_t)(b[0]<<24) +(uint32_t)(b[1]<<16)+(uint32_t)(b[2]<<8)+ b[3];
	return c;		
}
//16进制数组转字符串
void gdHexGroupToHexString(uint8_t *Buff,uint8_t *OutputStr,uint8_t BuffLen)
{
//	char	ddl,ddh;
//	int i;

//	for (i=0; i<nLen; i++)
//	{
//	ddh = 48 + pbSrc[i] / 16;
//	ddl = 48 + pbSrc[i] % 16;
//	if (ddh > 57) ddh = ddh + 7;
//	if (ddl > 57) ddl = ddl + 7;
//	pbDest[i*2] = ddh;
//	pbDest[i*2+1] = ddl;
//	}

//	pbDest[nLen*2] = '\0';

int i = 0;
	char TempBuff[128] = {0};
	char strBuff[256] = {0};
	
	for(i = 0; i<BuffLen;i++)
	{
		sprintf(TempBuff,"%02x",Buff[i]);//以十六进制格式输出到TempBuff，宽度为2
		strncat(strBuff,TempBuff,BuffLen*2);//将TempBuff追加到strBuff结尾
	}
	strncpy(OutputStr, strBuff, BuffLen*2); //将strBuff复制到OutputStr
	return BuffLen*2;

}
/*将字符串s转换成相应的整数*/  
int gdstringToInt(char *s)  
{  
    int i;  
    int n = 0;  
    for (i = 0; s[i] >= '0' && s[i] <= '9'; ++i)  
    {  
        n = 10 * n + (s[i] - '0');  
    }  
    return n;  
} 

//16进制字符串转16进制数组
uint8_t gdHexStringToHexGroup(uint8_t *psrc,uint8_t *buf,uint16_t len)
{
    uint16_t i,n = 0;

		uint8_t dst[300];
		
		for(i=0;i<len;i++)
		{
			strcpy(dst,"0X");
			strncat(dst,psrc,2);
			buf[i]= strtol(dst,NULL,16);
			psrc+=2;
			//printf("%#X ",buf[i]);
		}
		
    return n;
}
//累计额和取反	
unsigned char gdAddQuFan(uint8_t *str,uint8_t len)
{
	int i;
	uint8_t sum  ;
  for(i=0;i<len;i++)
	{
		 sum += str[i];
	}
	sum = ~sum;
	return sum;
}

uint8_t	FindMonth(uint8_t * ptr)
{
	 uint8_t	MString[37]={'J','a','n','F','e','b','M','a','r',
							 'A','p','r','M','a','y','J','u','n',
							 'J','u','l','A','u','g','S','e','p',
							 'O','c','t','N','o','v','D','e','c'};
	uint8_t	i=0;
	uint8_t	j;
	do
	{
		j=BytesComp(ptr,MString+3*i,3);
		if (!j) 
			return (i+1);
 		i++;
	}while (i<12);
    return (0);
}

void	ChgTimeToRecordDatas(uint8_t * Timeptr,uint8_t * ptr)
{
	uint8_t	aa,bb;
	aa=BCDToHex(Timeptr[0]);//年
	aa<<=2;
	bb=BCDToHex(Timeptr[1]);//月
	bb&=0x0f;
	ptr[0]=aa+(bb>>2);
	aa=BCDToHex(Timeptr[2]);//日
	ptr[1]=bb<<6;
	aa<<=1;
	ptr[1]+=aa;
	aa=BCDToHex(Timeptr[3]);//时
	if (aa>=16)
		ptr[1]++;
	aa&=0x0f;
	ptr[2]=aa<<4;
	aa=BCDToHex(Timeptr[4]);//分
	ptr[2]+=(aa>>2);
	bb=BCDToHex(Timeptr[5]);//秒
	ptr[3]=aa<<6;
	ptr[3]+=bb;		
}

void	SerialAscToHex(uint8_t * Sum,uint8_t aa )
{
	uint8_t	bb;
	bb=aa>>4;
	if (bb<10)
		bb+=0x30;
	else
		bb+=0x37;
	Sum[0]=bb;
	
	bb=aa&15;
	if (bb<10)
		bb+=0x30;
	else
		bb+=0x37;
	Sum[1]=bb;
		
}



