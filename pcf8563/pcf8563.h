
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//init PCF8563
int hw_pcf8563_init();

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
void hw_pcf8563_set_sysdate(uint8_t * ptr);

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
void hw_pcf8563_get_sysdate(uint8_t *ptr);
