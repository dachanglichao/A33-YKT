#ifndef DESCODE_H
#define DESCODE_H


#define ElemType unsigned char

#define u16 unsigned short int
#define u8 unsigned char

void DES_3_P_Transform(ElemType data_a[32]);
void DES_3_Char8ToBit64(ElemType ch[8], ElemType *bit_a);
void DES_3_Bit64ToChar8(ElemType bit_a[64], ElemType ch[8]);

void encrypt_data(ElemType * bytedatain, int inLength, ElemType * outHEXdata, int * outLength, char * key);
void encrypt_data_XOR(ElemType * bytedatain, int inLength, ElemType * outHEXdata, int * outLength, char * key);
int decrypt_data(ElemType * HEXdatain, int inLength, ElemType * outbytedata, int * outLength, char * key);
int decrypt_data_XOR(ElemType * HEXdatain, int inLength, ElemType * outbytedata, int * outLength, char * key);

void DES_3_Test(void);
#endif

