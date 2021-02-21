#include "descode.h"
#include "../debug/debug.h"



void handled()
{

}

/*-???? ------------------------------------------------------------------*/

ElemType des3_S[8][4][16] =
{
	{	//S1  
		{ 14,4,13,1,2,15,11,8,3,10,6,12,5,9,0,7 },
		{ 0,15,7,4,14,2,13,1,10,6,12,11,9,5,3,8 },
		{ 4,1,14,8,13,6,2,11,15,12,9,7,3,10,5,0 },
		{ 15,12,8,2,4,9,1,7,5,11,3,14,10,0,6,13 }
	},

	{ //S2
		{ 15,1,8,14,6,11,3,4,9,7,2,13,12,0,5,10 },
		{ 3,13,4,7,15,2,8,14,12,0,1,10,6,9,11,5 },
		{ 0,14,7,11,10,4,13,1,5,8,12,6,9,3,2,15 },
		{ 13,8,10,1,3,15,4,2,11,6,7,12,0,5,14,9 }
	},
	//S3  
	{
		{ 10,0,9,14,6,3,15,5,1,13,12,7,11,4,2,8 },
		{ 13,7,0,9,3,4,6,10,2,8,5,14,12,11,15,1 },
		{ 13,6,4,9,8,15,3,0,11,1,2,12,5,10,14,7 },
		{ 1,10,13,0,6,9,8,7,4,15,14,3,11,5,2,12 }
	},
	//S4  
	{
		{ 7,13,14,3,0,6,9,10,1,2,8,5,11,12,4,15 },
		{ 13,8,11,5,6,15,0,3,4,7,2,12,1,10,14,9 },
		{ 10,6,9,0,12,11,7,13,15,1,3,14,5,2,8,4 },
		{ 3,15,0,6,10,1,13,8,9,4,5,11,12,7,2,14 }
	},
	//S5	
	{
		{ 2,12,4,1,7,10,11,6,8,5,3,15,13,0,14,9 },
		{ 14,11,2,12,4,7,13,1,5,0,15,10,3,9,8,6 },
		{ 4,2,1,11,10,13,7,8,15,9,12,5,6,3,0,14 },
		{ 11,8,12,7,1,14,2,13,6,15,0,9,10,4,5,3 }
	},
	//S6	 
	{
		{ 12,1,10,15,9,2,6,8,0,13,3,4,14,7,5,11 },
		{ 10,15,4,2,7,12,9,5,6,1,13,14,0,11,3,8 },
		{ 9,14,15,5,2,8,12,3,7,0,4,10,1,13,11,6 },
		{ 4,3,2,12,9,5,15,10,11,14,1,7,6,0,8,13 }
	},
	//S7	 
	{
		{ 4,11,2,14,15,0,8,13,3,12,9,7,5,10,6,1 },
		{ 13,0,11,7,4,9,1,10,14,3,5,12,2,15,8,6 },
		{ 1,4,11,13,12,3,7,14,10,15,6,8,0,5,9,2 },
		{ 6,11,13,8,1,4,10,7,9,5,0,15,14,2,3,12 }
	},
	//S8	 
	{
		{ 13,2,8,4,6,15,11,1,10,9,3,14,5,0,12,7 },
		{ 1,15,13,8,10,3,7,4,12,5,6,11,0,14,9,2 },
		{ 7,11,4,1,9,12,14,2,0,6,10,13,15,3,5,8 },
		{ 2,1,14,7,4,10,8,13,15,12,9,0,3,5,6,11 }
	}
};

ElemType DES3_E_Table[48] =
{
	31,0,1,2,3,4,
	3,4,5,6,7,8,
	7,8,9,10,11,12,
	11,12,13,14,15,16,
	15,16,17,18,19,20,
	19,20,21,22,23,24,
	23,24,25,26,27,28,
	27,28,29,30,31,0
};

ElemType DES3_P_Table[32] =
{
	15,6,19,20,28,11,27,16,
	0,14,22,25,4,17,30,9,
	1,7,23,13,31,26,2,8,
	18,12,29,5,21,10,3,24
};


ElemType des3_PC_1[56] =
{
	56,48,40,32,24,16,8,
	0,57,49,41,33,25,17,
	9,1,58,50,42,34,26,
	18,10,2,59,51,43,35,
	62,54,46,38,30,22,14,
	6,61,53,45,37,29,21,
	13,5,60,52,44,36,28,
	20,12,4,27,19,11,3
};


ElemType des3_PC_2[48] =
{
	13,16,10,23,0,4,2,27,
	14,5,20,9,22,18,11,3,
	25,7,15,6,26,19,12,1,
	40,51,30,36,46,54,29,39,
	50,44,32,47,43,48,38,55,
	33,52,45,41,49,35,28,31
};

ElemType DES3_IP_Table[64] =
{
	57,49,41,33,25,17,9,1,
	59,51,43,35,27,19,11,3,
	61,53,45,37,29,21,13,5,
	63,55,47,39,31,23,15,7,
	56,48,40,32,24,16,8,0,
	58,50,42,34,26,18,10,2,
	60,52,44,36,28,20,12,4,
	62,54,46,38,30,22,14,6
};

ElemType DES3_IP_1_Table[64] =
{
	39,7,47,15,55,23,63,31,
	38,6,46,14,54,22,62,30,
	37,5,45,13,53,21,61,29,
	36,4,44,12,52,20,60,28,
	35,3,43,11,51,19,59,27,
	34,2,42,10,50,18,58,26,
	33,1,41,9,49,17,57,25,
	32,0,40,8,48,16,56,24
};




/*???? ------------------------------------------------------------------*/
void DES_3_Encrypt(ElemType *plainFile, ElemType *keyStr, ElemType *cipherFile);	//????
void DES_3_Decrypt(ElemType *cipherFile, ElemType *keyStr, ElemType *plainFile);
//????	 
void DES_3_test(void);

/*-???? ------------------------------------------------------------------*/



/*-----------------------------------------------------------------------------
* ?	 ?:????
* ?	 ?:
* ???:
*----------------------------------------------------------------------------*/
void memcpy(u8 *des, u8 *sor, u16 len)
{
	u8 i;

	for (i = 0; i<len; i++)
	{
		des[i] = sor[i];
	}
}

//????8???????????	
void DES_3_Char8ToBit64(ElemType ch[8], ElemType *bit_a)
{
	ElemType cnt, cnt1;

	for (cnt = 0; cnt < 8; cnt++)
	{
		for (cnt1 = 0; cnt1 < 8; cnt1++)
		{
			if ((ch[cnt] & 0x80) == 0x80)
				*bit_a = 0x01;
			else
				*bit_a = 0x00;
			bit_a++;
			ch[cnt] = ch[cnt] << 1;
		}
	}
}

//???????????8????	
void DES_3_Bit64ToChar8(ElemType bit_a[64], ElemType ch[8])
{
	char cnt, cnt1;
	memset(ch, 0, 8);

	for (cnt = 0; cnt < 8; cnt++)
	{

		for (cnt1 = 0; cnt1 < 8; cnt1++)
		{
			ch[cnt] = ch[cnt] << 1;
			ch[cnt] |= *bit_a;
			bit_a++;
		}
	}

}

//??PC1??
void DES_3_PC1_Transform(ElemType key[64], ElemType tempbts[56])
{
	ElemType cnt;

	for (cnt = 0; cnt < 56; cnt++)
	{
		tempbts[cnt] = key[des3_PC_1[cnt]];
	}
}

//??PC2??  
void DES_3_PC2_Transform(ElemType key[56], ElemType tempbts[48])
{
	ElemType cnt;

	for (cnt = 0; cnt < 48; cnt++)
	{
		tempbts[cnt] = key[des3_PC_2[cnt]];
	}

}

//????  
void DES_3_ROL(ElemType *data_a, ElemType time)
{
	ElemType temp[56], i;

	//?????????????  
	for (i = 0; i<time; i++)
	{
		temp[i] = *(data_a + i);
		temp[time + i] = *(data_a + 28 + i);
	}

	for (i = 0; i <= 28 - time; i++)
	{
		*(data_a + i) = *(data_a + time + i);
		*(data_a + 28 + i) = *(data_a + 28 + time + i);
	}

	for (i = 0; i<time; i++)
	{
		*(data_a + 28 - time + i) = temp[i];
		*(data_a + 56 - time + i) = temp[i + time];
	}

}

//???? 
void DES_3_ROR(ElemType *data_a, ElemType time)
{
	ElemType temp[56], i;

	//?????????????  
	for (i = 0; i<time; i++)
	{
		temp[time - 1 - i] = *(data_a + 28 - 1 - i);
		temp[time + 1 - i] = *(data_a + 56 - 1 - i);
	}

	for (i = 0; i <= 28 - time; i++)
	{
		*(data_a + 27 - i) = *(data_a + 27 - time - i);
		*(data_a + 55 - i) = *(data_a + 55 - time - i);
	}

	for (i = 0; i<time; i++)
	{
		*(data_a + i) = temp[i];
		*(data_a + 28 + i) = temp[i + 2];
	}
}

//??IP?? 
void DES_3_IP_Transform(ElemType data_a[64])
{
	ElemType cnt;
	ElemType temp[64];

	for (cnt = 0; cnt < 64; cnt++)
	{
		temp[cnt] = data_a[DES3_IP_Table[cnt]];
	}
	memcpy(data_a, temp, 64);

}

//IP???  
void DES_3_IP_1_Transform(ElemType data_a[64])
{
	ElemType cnt;
	ElemType temp[64];

	for (cnt = 0; cnt < 64; cnt++)
	{
		temp[cnt] = data_a[DES3_IP_1_Table[cnt]];
	}

	memcpy(data_a, temp, 64);
}

//????  E??,32???48?
void DES_3_E_Transform(ElemType data_a[32])
{
	ElemType cnt;
	ElemType temp[48];

	for (cnt = 0; cnt < 48; cnt++)
	{
		temp[cnt] = data_a[DES3_E_Table[cnt]];
	}
	memcpy(data_a, temp, 48);
}

//??P??  
void DES_3_P_Transform(ElemType data_a[32])
{
	ElemType cnt;
	ElemType temp[32];

	for (cnt = 0; cnt < 32; cnt++) {
		temp[cnt] = data_a[DES3_P_Table[cnt]];
	}
	memcpy(data_a, temp, 32);

}

//??  
void DES_3_XOR(ElemType R[48], ElemType L[48], ElemType count)
{
	ElemType cnt;
	for (cnt = 0; cnt < count; cnt++)
	{
		R[cnt] ^= L[cnt];
	}

}

//S???48??32?	
void DES_3_SBOX(ElemType data_a[48])
{

	char cnt;
	char lines, row;
	char cur1, cur2;
	char output;
	//??????IP^-1  

	for (cnt = 0; cnt < 8; cnt++)
	{
		cur1 = cnt * 6;
		cur2 = cnt << 2;

		//???S??????	
		lines = (data_a[cur1] << 1) + data_a[cur1 + 5];
		row = (data_a[cur1 + 1] << 3) + (data_a[cur1 + 2] << 2)
			+ (data_a[cur1 + 3] << 1) + data_a[cur1 + 4];
		output = des3_S[cnt][lines][row];

		//??2??  
		data_a[cur2] = (output & 0X08) >> 3;
		data_a[cur2 + 1] = (output & 0X04) >> 2;
		data_a[cur2 + 2] = (output & 0X02) >> 1;
		data_a[cur2 + 3] = output & 0x01;
	}
}

//??  
void DES_3_Swap(ElemType left[32], ElemType right[32])
{
	ElemType temp[32];
	memcpy(temp, left, 32);
	memcpy(left, right, 32);
	memcpy(right, temp, 32);

}

/*-???? ------------------------------------------------------------------*/

//DES??
void DES_3_Encrypt(ElemType PlainTxt[8], ElemType *keyStr, ElemType CipherTxt[8])
{
	ElemType plainBits[64];
	ElemType copyRight[48];
	ElemType keyBlock[8], PlainTxt1[8];
	ElemType bKey[64]; //??????? 
	ElemType subKeys[48];//????	
	ElemType temp[56]; //?PCI???56???
	char cnt;
	unsigned char des3_MOVE_TIMES[16] = { 1,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

	memcpy(keyBlock, keyStr, 8);
	//??????????  
	DES_3_Char8ToBit64(keyBlock, bKey);
	DES_3_Bit64ToChar8(bKey, keyBlock);

	DES_3_PC1_Transform(bKey, temp);//PC1??

	memcpy(PlainTxt1, PlainTxt, 8); ///////
	DES_3_Char8ToBit64(PlainTxt1, plainBits);
	//??????(IP??)  
	DES_3_IP_Transform(plainBits);

	for (cnt = 0; cnt < 16; cnt++)
	{
		DES_3_ROL(temp, des3_MOVE_TIMES[cnt]); 	 //??
		DES_3_PC2_Transform(temp, subKeys);	 //?????  

		memcpy(copyRight, plainBits + 32, 32);
		//?????????????,?32????48?  
		DES_3_E_Transform(copyRight);

		//?????????????????  
		DES_3_XOR(copyRight, subKeys, 48);

		//??????S?,??32???  
		DES_3_SBOX(copyRight);

		//P??  
		DES_3_P_Transform(copyRight);

		//????????????????  
		DES_3_XOR(plainBits, copyRight, 32);

		if (cnt != 15)
		{
			//??????????  
			DES_3_Swap(plainBits, plainBits + 32);

		}
	}
	//?????(IP^1??)  
	DES_3_IP_1_Transform(plainBits);
	DES_3_Bit64ToChar8(plainBits, CipherTxt);
}


//DES??
void DES_3_Decrypt(ElemType PlainTxt[8], ElemType *keyStr, ElemType CipherTxt[8])
{
	ElemType cipherBits[64];
	ElemType copyRight[48];
	ElemType keyBlock[8];
	ElemType bKey[64];
	ElemType temp[56], PlainTxt1[8];
	ElemType subKeys[48];
	ElemType cnt;
	ElemType des3_MOVE_TIMES[16] = { 0,1,2,2,2,2,2,2,1,2,2,2,2,2,2,1 };

	memcpy(keyBlock, keyStr, 8);
	//??????????  
	DES_3_Char8ToBit64(keyBlock, bKey);
	DES_3_PC1_Transform(bKey, temp);	  //PC1??

	memcpy(PlainTxt1, PlainTxt, 8); /////////  
	DES_3_Char8ToBit64(PlainTxt1, cipherBits);
	//????(IP??)  
	DES_3_IP_Transform(cipherBits);

	//16???  
	for (cnt = 0; cnt <16; cnt++)
	{
		if (cnt != 0)
		{
			DES_3_ROR(temp, des3_MOVE_TIMES[cnt]);   //??
		}
		DES_3_PC2_Transform(temp, subKeys);	  //????? 

		memcpy(copyRight, cipherBits + 32, 32);
		//???????????,?32????48?  
		DES_3_E_Transform(copyRight);
		//???????????????  
		DES_3_XOR(copyRight, subKeys, 48);
		//??????S?,??32???  
		DES_3_SBOX(copyRight);
		//P??  
		DES_3_P_Transform(copyRight);
		//????????????????  
		DES_3_XOR(cipherBits, copyRight, 32);
		if (cnt != 15)//???????
		{
			//??????????  
			DES_3_Swap(cipherBits, cipherBits + 32);
		}
	}
	//?????(IP^1??)  
	DES_3_IP_1_Transform(cipherBits);
	DES_3_Bit64ToChar8(cipherBits, CipherTxt);
}

#if 0
/**********3DES_3??*************/
void En3des(unsigned char m[8], unsigned char k[16], unsigned char e[8])
{
	unsigned char uszetmp[8];
	unsigned char uszmtmp[8];
	unsigned char uszkeytmp[8];


	//16??key????DES_3??
	memcpy(uszkeytmp, k, 8);
	DES_3_Encrypt(m, uszkeytmp, uszetmp);

	//16??key????DES_3??
	memcpy(uszkeytmp, k + 8, 8);
	DES_3_Decrypt(uszetmp, uszkeytmp, uszmtmp);

	//16??key????DES_3??
	memcpy(uszkeytmp, k, 8);
	DES_3_Encrypt(uszmtmp, uszkeytmp, uszetmp);

	memcpy(e, uszetmp, 8);
	return;
}

/**********3DES_3??*******************/
void De3des(unsigned char e[8], unsigned char k[16], unsigned char m[8])
{
	unsigned char uszkeytmp[8];
	unsigned char uszetmp[8];
	unsigned char uszmtmp[8];

	//16??????DES_3??
	memcpy(uszkeytmp, k, 8);
	DES_3_Decrypt(e, uszkeytmp, uszmtmp);

	//16??????des??
	memcpy(uszkeytmp, k + 8, 8);
	DES_3_Encrypt(uszmtmp, uszkeytmp, uszetmp);

	//16??????des??
	memcpy(uszkeytmp, k, 8);
	DES_3_Decrypt(uszetmp, uszkeytmp, uszmtmp);
	memcpy(m, uszmtmp, 8);

	return;
}
#endif 

//8?????
unsigned char key_tab_2[] = { 0x00,0x01,0x02,0x03,0x04,0x05,0x06,0x07 };
//			    0x33,0x33,0x33,0x33,0x33,0x33,0x33,0x33};
unsigned char plain_tab[8] = { 0x01,0x01,0x01,0x01,0x01,0x01,0x01,0x01 };//?????			
unsigned char cipher[8];//??
unsigned char plain_temp[8];//?????

char data2HEX(char data)
{
	if (data >= 0 && data <= 9)
	{
		return data + '0';
	}
	else if (data >= 10 && data <= 15)
	{
		return data - 10 + 'A';
	}
	return 0;
}


#include "descode.h"

ElemType tempData[1024];
ElemType tempOutData[1024];

void encrypt_data(ElemType * bytedatain, int inLength, ElemType * outHEXdata, int * outLength,char * key)
{
	u8 err;
	
	int i;
	int datalength = 0;
	int addlength = 0;


	
//	debug("3DES加密秘钥==");
//	for(i=0;i<8;i++)
//		{
//	debug("%2X ",key[i]);
//	}
//	debug("\n");
//	debug("3DES加密数据==");
//	for(i=0;i<inLength;i++)
//		{
//	debug("%2X ",bytedatain[i]);
//	}
//	debug("\n");

//	debug("加密数据长度= %d\n",inLength);
	
	datalength = inLength - 2;
	addlength = datalength % 8;
	if (addlength)
	{
		addlength = 8;
	}
	datalength = (datalength / 8) * 8 + addlength;

	memcpy(tempData, bytedatain+1, datalength);
	for (i = 0; i < datalength / 8; i++)
	{
		DES_3_Encrypt(tempData+i*8, (unsigned char *)key, tempOutData+i*8);
	}

	outHEXdata[0] = 's';

	for (i = 0; i < datalength; i++)
	{
		outHEXdata[1 + i * 2] = data2HEX(tempOutData[i] >> 4);
		outHEXdata[2 + i * 2] = data2HEX(tempOutData[i] & 0x0F);
	}
	outHEXdata[datalength * 2 + 1] = 'o';
	*outLength = datalength * 2 + 2;

		
//	debug("3DES加密后数据==");
//	for(i=0;i<*outLength;i++)
//		{
//	debug("%2X ",outHEXdata[i]);
//	}
//	debug("\n");
	
	
}

char HEX2data(char data)
{
	if (data >= '0' && data <= '9')
	{
		return data - '0';
	}
	else if (data >= 'A' && data <= 'F')
	{
		return data - 'A' + 10;
	}
	return 0;
}

int decrypt_data(ElemType * HEXdatain, int inLength, ElemType * outbytedata, int * outLength,char * key)
{
	int i;
	int datalength = 0;
	u8 err;




	for (i = 0; i < inLength / 2; i++)
	{
		tempData[i] = HEX2data(HEXdatain[i * 2]);
		tempData[i] <<= 4;
		tempData[i] += HEX2data(HEXdatain[i * 2 + 1]);
	}
	datalength = inLength / 2;

	if ((datalength % 8) != 0)
	{
	
		return -1;
	}
	for (i = 0; i < datalength / 8; i++)
	{
		DES_3_Decrypt(tempData + i * 8, (unsigned char *)key, outbytedata + i * 8);
	}

	*outLength = datalength;


	return 0;
}

void encrypt_data_XOR(ElemType * bytedatain, int inLength, ElemType * outHEXdata, int * outLength, char * key)
{
	int datalength = 0;
	int addlength = 0;
	int i,j;

//	debug("异或加密秘钥==");
//	for(i=0;i<8;i++)
//		{
//	debug("%2X ",key[i]);
//	}
//	debug("\n");
//	debug("异或加密数据==");
//	for(i=0;i<inLength;i++)
//		{
//	debug("%2X ",bytedatain[i]);
//	}
//	debug("\n");

//	debug("加密数据长度= %d\n",inLength);
	
	datalength = inLength - 2;
	addlength = datalength % 8;
	if (addlength)
	{
		addlength = 8;
	}
	datalength = (datalength / 8) * 8 + addlength;

	memcpy(tempData, bytedatain + 1, datalength);
	for (i = 0; i < datalength / 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			tempOutData[i * 8 + j] = tempData[i * 8 + j] ^ key[j];
		}
		//DES_3_Encrypt(tempData + i * 8, (unsigned char *)key, tempOutData + i * 8);
	}

	outHEXdata[0] = 's';

	for (i = 0; i < datalength; i++)
	{
		outHEXdata[1 + i * 2] = data2HEX(tempOutData[i] >> 4);
		outHEXdata[2 + i * 2] = data2HEX(tempOutData[i] & 0x0F);
	}
	outHEXdata[datalength * 2 + 1] = 'o';
	*outLength = datalength * 2 + 2;

//	debug("异或加密后数据==");
//	for(i=0;i< *outLength;i++)
//		{
//	debug("%2X ",outHEXdata[i]);
//	}
//	debug("\n");
}

int decrypt_data_XOR(ElemType * HEXdatain, int inLength, ElemType * outbytedata, int * outLength, char * key)
{
	int datalength = 0;
	int i,j;

	for (i = 0; i < inLength / 2; i++)
	{
		tempData[i] = HEX2data(HEXdatain[i * 2]);
		tempData[i] <<= 4;
		tempData[i] += HEX2data(HEXdatain[i * 2 + 1]);
	}
	datalength = inLength / 2;

	if ((datalength % 8) != 0)
	{
		return -1;
	}
	for (i = 0; i < datalength / 8; i++)
	{
		for (j = 0; j < 8; j++)
		{
			outbytedata[i * 8 + j] = tempData[i * 8 + j] ^ key[j];
		}
		//DES_3_Decrypt(tempData + i * 8, (unsigned char *)key, outbytedata + i * 8);
	}

	*outLength = datalength;
	return 0;
}




void DES_3_Test(void)
{
	DES_3_Encrypt(plain_tab, key_tab_2, cipher);

	DES_3_Decrypt(cipher, key_tab_2, plain_temp);
}

