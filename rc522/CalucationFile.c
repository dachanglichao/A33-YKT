
#include "CalucationFile.h"
#include "../debug/debug.h"


 uint32_t	ChgKeyStringToUlong(uint8_t * ptr ,uint8_t Len)//BCD码字符串转换为整型数
{
	 uint32_t		ii=0;
	uint32_t		jj=1;
	uint8_t		aa;
	do
	{
		aa=(*(ptr+Len-1));
		ii|=aa*jj;
		jj=jj*0x100;
	}while(--Len);
	return	ii;
}

uint32_t	ChgStringsToInt(uint8_t * ptr)
{
	uint8_t	i;
	uint16_t	kk=1;
	uint32_t	ii=0;
	uint8_t	st_data;
	for (i=0;i<5;i++)
	{
		st_data= ptr[4-i];
		ii+=(uint32_t)kk*st_data;	
		kk=kk*10;
	}	
	return	ii;
}

uint8_t BytesComp( uint8_t * CharDptr1,uint8_t * CharDptr2,uint8_t CompNum)
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
uint8_t	BytesCheckSum(uint8_t *ptr,uint8_t Len)
{
	uint8_t	aa=0;
	uint8_t bb;
	Len--;
	do
	{	
		aa+=*ptr++;
	}while (--Len);	 
	bb= *ptr;
	aa=~aa;
	if (aa!=bb) 
		return 1;
	else
		return 0;
}
uint8_t	CalCheckSum(uint8_t  * Ptr,uint8_t charLen)
{
	uint8_t 	st_data=0;
	do
	{
		st_data+= * Ptr++;
	}while (--charLen);
	st_data=~st_data;
	return st_data;
}
uint8_t	BCD_String_Diag(uint8_t * ptr ,uint8_t Len)//BCD码字符串诊断
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
uint32_t	ChgBCDStringToUlong(uint8_t * ptr ,uint8_t Len)//BCD码字符串转换为整型数
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
void	ChgUlongToBCDString(uint32_t iii,uint8_t * ptr,uint8_t Len)
{
	uint8_t 		i;
	uint8_t 		aa;
	uint32_t		jj=1;
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

uint8_t HexToBCD(uint8_t aa )
{
	return((aa/10)*16+aa%10);
}
uint8_t BCDToHex(uint8_t aa )
{
	return((aa/16)*10+aa%16);
}


void	ChgIntToStrings(uint16_t	ii,uint8_t * ptr)
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


uint32_t	ChgInputToUlong(uint8_t * ptr,uint8_t Num)//输入的数字转换为长整形
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
void	FormatBuffer(uint8_t	SLen,uint8_t * ptr ,uint8_t * Len)
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
uint32_tGetU32_HiLo(uint8_t * lbuf)
{
	uint8_t * p_buf;
	uint32_t r_buf;
	
	p_buf = lbuf;
	r_buf = (uint32_t)p_buf[3] + ((uint32_t)p_buf[2]<<8) + ((uint32_t)p_buf[1]<<16) + ((uint32_t)p_buf[0]<<24);
	return r_buf;
} 

//取双字节数据(高位在前)
uint16_t GetU16_HiLo(uint8_t * lbuf)
{
	uint8_t * p_buf;
	uint16_t  r_buf;
	
	p_buf = lbuf;
	r_buf = (uint16_t)p_buf[1] + ((uint16_t)p_buf[0]<<8);
	return r_buf;
} 
 
//设置四字节数据(高位在前)
void PutU32_HiLo(uint8_t * lbuf,uint32_t ldata)
{
	lbuf[0]=(uint8_t)(ldata>>24);
	lbuf[1]=(uint8_t)(ldata>>16);
	lbuf[2]=(uint8_t)(ldata>>8);
	lbuf[3]=(uint8_t)(ldata);
} 

//设置双字节数据(高位在前)
void PutU16_HiLo(uint8_t * lbuf,uint16_t ldata)
{	
	lbuf[0]=(uint8_t)(ldata>>8);
	lbuf[1]=(uint8_t)(ldata);
}

//双字节大小端转换
uint16_t DoubleBigToSmall(uint16_t a)
{
	uint16_t c;
	unsigned char b[2];
	b[0] =(unsigned char) (a);
	b[1] = (unsigned char)(a>>8);
	c = (uint16_t)(b[0]<<8) + b[1];
	return c;
}

//四字节大小端转换
int32_t FourBigToSmall(uint32_t a)
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
void HexGroupToHexString(uint8_t *data,uint8_t *dst,uint8_t len)
{
	uint8_t i;
	uint8_t str[250];
	//uint8_t *dst;
	for(i=0;i<len;i++)
	{
		 str[2*i] = data[i]>>4;
     str[2*i+1] = data[i]&0xf;
	}
	for(i=0;i<len*2;i++)
	{
		sprintf(&dst[i],"%X",str[i]);
	}
}
//16进制字符串转16进制数组
uint16_t HexStringToHexGroup(uint8_t *psrc,uint8_t *buf,uint16_t len)
	{
    int i,n = 0;

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
unsigned char AddQuFan(uint8_t *str,uint8_t len)
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
 
