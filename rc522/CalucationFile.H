#include "string.h"
#include <stdint.h>
#include <unistd.h>

		 
uint8_t   	BytesCheckSum(uint8_t *,uint8_t);//Ğ£ÑéºÍ±È½Ï
uint8_t     BytesComp( uint8_t * ,uint8_t *,uint8_t);//×Ö·û´®±È½Ï£¬ÕıÈ··µ»Ø0
uint8_t	    BCD_String_Diag(uint8_t * ,uint8_t);//BCDÂë×Ö·û´®Õï¶Ï
uint8_t     BCDToHex(uint8_t  );
uint8_t     HexToBCD(uint8_t  );
uint32_t	ChgBCDStringToUlong(uint8_t * ,uint8_t);//BCDÂë×Ö·û´®×ª»»ÎªÕûĞÍÊı
uint8_t	    CalCheckSum(uint8_t * ,uint8_t);//¼ÆËãĞ£ÑéºÍ£
void	    ChgUlongToBCDString( uint32_t ,uint8_t * ,uint8_t );
void	    ChgIntToStrings(uint16_t	ii,uint8_t * ptr);
uint32_t	ChgInputToUlong(uint8_t * ,uint8_t);//ÊäÈëµÄÊı×Ö×ª»»Îª³¤ÕûĞÎ
uint32_t 	GetU32_HiLo(uint8_t *);
uint16_t 	GetU16_HiLo(uint8_t *);
void 	    PutU32_HiLo(uint8_t *,uint32_t);
void 	    PutU16_HiLo(uint8_t *,uint16_t);
void	    FormatBuffer(uint8_t,uint8_t * ,uint8_t *);
uint32_t	ChgKeyStringToUlong(uint8_t * ptr ,uint8_t Len);
uint32_t	ChgStringsToInt(uint8_t * ptr);
uint16_t    DoubleBigToSmall(uint16_t a);
void        HexGroupToHexString(uint8_t *data,uint8_t *dst,uint8_t len);
uint16_t    HexStringToHexGroup(uint8_t *,uint8_t *,uint16_t);
uint8_t     AddQuFan(uint8_t *str,uint8_t len);//ÀÛ¼ÓºÍÈ¡·´
 