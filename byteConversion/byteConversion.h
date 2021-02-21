#ifndef BYTECONVERSION_H
#define BYTECONVERSION_H
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>


//字节流转换为十六进制字符串
void ByteToHexStr( const char *sSrc,  char *sDest, int nSrcLen );
//十六进制字符串转换为字节流
void HexStrToByte(const uint8_t* source, uint8_t* dest, int sourceLen);

uint8_t  	gdBytesCheckSum(uint8_t*,uint8_t);//校验和比较
uint8_t    gdBytesComp( uint8_t* ,uint8_t*,uint8_t);//字符串比较，正确返回0
uint8_t	    gdBCD_String_Diag(uint8_t* ,uint8_t);//BCD码字符串诊断
uint8_t  gdBCDToHex(uint8_t );
uint8_t  gdHexToBCD(uint8_t );
uint32_t	gdChgBCDStringToUlong(uint8_t* ,uint8_t);//BCD码字符串转换为整型数
uint8_t	gdCalCheckSum(uint8_t* ,uint8_t);//计算校验和?
void	gdChgUlongToBCDString( uint32_t ,uint8_t* ,uint8_t);
void	gdChgIntToStrings(uint16_t	ii,uint8_t* ptr);
uint32_t	gdChgInputToUlong(uint8_t* ,uint8_t);//输入的数字转换为长整形
uint32_t 	gdGetU32_HiLo(uint8_t*);
uint16_t 	gdgdGetU16_HiLo(uint8_t*);
void 	gdPutU32_HiLo(uint8_t*,uint32_t);
void 	gdPutU16_HiLo(uint8_t*,uint);
void	gdFormatBuffer(uint8_t,uint8_t* ,uint8_t*);
uint32_t	gdChgKeyStringToUlong(uint8_t* ptr ,uint8_t Len);
uint32_t	gdChgStringsToInt(uint8_t* ptr);
uint16_t gdDoubleBigToSmall(uint16_t a);
void gdHexGroupToHexString(uint8_t*data,uint8_t*dst,uint8_t len);
uint8_t gdHexStringToHexGroup(uint8_t*,uint8_t*,uint16_t);
uint8_t gdAddQuFan(uint8_t*str,uint8_t len);//累加和取反
int gdstringToInt(char *s)  ;
uint8_t	FindMonth(uint8_t * ptr);

void	SerialAscToHex(uint8_t * Sum,uint8_t aa );



#endif
