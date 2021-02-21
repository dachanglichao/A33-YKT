
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "pcf8563.h"
#include "i2c-dev.h"

//PCF8563 i2c address
#define PCF8563_Read_Add	0xA3
#define PCF8563_Write_Add	0xA2 
static uint8_t		bitSysTimeDisable = 0;
static int fd;//句柄
/* i2c_test r addr
 * i2c_test w addr val
 */

void print_usage(char *file)
{
	printf("%s r addr\n", file);
	printf("%s w addr val\n", file);
}
//init PCF8563
int hw_pcf8563_init()
{
	fd = open("/dev/i2c-0", O_RDWR);
	if (fd < 0)
	{
		printf("can't open /dev/pcf8563\n");
		return -1;
	}

	if (ioctl(fd, I2C_SLAVE, 0x51) < 0)
	{    
		/* ERROR HANDLING; you can check errno to see what went wrong */    
		printf("set addr error!\n");
		return -1;
	}
	return 0;
}
//从指定地址读取一个字节数据
static uint8_t hw_pcf8563_read_byte(uint8_t DevReaAddr,uint16_t ReadAddr)
{
	unsigned char buf[2],temp,i;
	unsigned char addr, data;

	addr = ReadAddr%256;
	data = i2c_smbus_read_word_data(fd, addr);
	return data;
}
//向指定地址写入数据
static uint8_t hw_pcf8563_write_byte(uint16_t WriteAddr,uint8_t DataToWrite)
{	
	unsigned char buf[3],i;
	unsigned char addr, data;
	
	addr = WriteAddr%256;
	data = DataToWrite;
	
	i2c_smbus_write_byte_data(fd, addr, data);	

}

//DevWriAddr :write addr
//DevReaAddr :read addr 
//StarAddr 	 :start addr 0~255
//pBuffer    :
//NumToRead  :
static uint8_t hw_pcf8563_read_buf(uint8_t DevReaAddr,uint16_t StarAddr,uint8_t *pBuffer,uint16_t NumToRead)
{
	while(NumToRead)
	{
		*pBuffer++=hw_pcf8563_read_byte(DevReaAddr,StarAddr++);	
		NumToRead--;
	}
	return 0;
}

//DeviceAddr :
//StarAddr 	 :
//pBuffer    :
//NumToWrite :
static uint8_t hw_pcf8563_write_buf(uint8_t DeviceAddr,uint16_t StarAddr,uint8_t *pBuffer,uint16_t NumToWrite)
{
	while(NumToWrite--)
	{
		hw_pcf8563_write_byte(StarAddr,*pBuffer);
		StarAddr++;
		pBuffer++;
	}		
	return 0;
}

/********************************************************************/ 
//PCF8563 stop clock
static uint8_t PCF8563_stop(void)
{
 	uint8_t stopcode=0x20;
 	if(0==hw_pcf8563_write_buf(PCF8563_Write_Add,0,&stopcode,1))
		return 0;
 	else return 1;
}
//PCF8563 start clock
static uint8_t PCF8563_start(void)
{
 	uint8_t startcode=0x00;
	if(0==hw_pcf8563_write_buf(PCF8563_Write_Add,0,&startcode,1))
		return 0;
 	else return 1;
}
//pcf8563 clear intter
static uint8_t PCF8563_ClearINT(void)
{
 	uint8_t temp=0x12;
	if(0==hw_pcf8563_write_buf(PCF8563_Write_Add,1,&temp,1))
		return 0;
 	else return 1;
}
/************************************************************************
pcf8563 set RTC
参数说明：BCD码
yy:  年
mm:	 月
dd:	 日
hh:	 时
mi:	 分
ss:	 秒
da:	 星期
************************************************************************/
void hw_pcf8563_set_sysdate(uint8_t * ptr)
{
	 uint8_t time[7];
	 time[6]=ptr[0];//年
	 time[5]=ptr[1];//月
	 //time[4]=ptr[2];//星期
	 time[3]=ptr[2];//日
	 time[2]=ptr[3];//时
	 time[1]=ptr[4];//分
	 time[0]=ptr[5];//秒
	 PCF8563_stop();
	 hw_pcf8563_write_buf(PCF8563_Write_Add,2,time,7);

	 PCF8563_start();	    
}
/************************************************************************
pcf8563 read RTC
参数说明：BCD码
yy:  年
mm:	 月
dd:	 日
hh:	 时
mi:	 分
ss:	 秒
da:	 星期
************************************************************************/
void hw_pcf8563_get_sysdate(uint8_t *ptr)
{
	uint8_t time[7];

	hw_pcf8563_read_buf(PCF8563_Read_Add,2,time,7);

	ptr[0]=time[6]; //年
	ptr[1]=time[5]&0x1f; //月
	ptr[2]=time[3]&0x3f; //日
	ptr[3]=time[2]&0x3f; //时
	ptr[4]=time[1]&0x7f; //分
	if (time[0]<0x80)
		bitSysTimeDisable=0;
	else
		bitSysTimeDisable=1;
	ptr[5]=time[0]&0x7f; //秒
	ptr[6]=time[4]&7;	  
}