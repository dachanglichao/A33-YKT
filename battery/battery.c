
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include "battery.h"

static FILE *fd;
//初始化电池检测
 void battery_init(void)
{	
	fd = open("/dev/readgpio", O_RDWR);
	if (fd < 0)
	{
		printf("can't open /dev/readgpio\n");
		return -1;
	}
	printf("电池状态打开\n");

}
//读取电池状态
//返回0==电源状态；返回1==电池供电状态
int battery_read_satatus(void)
{
	char buf;

	read(fd, &buf, 1);
	return buf;

}



