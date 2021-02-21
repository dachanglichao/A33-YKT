
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

//初始化电池检测
 void battery_init(void);

//读取电池状态
//返回0==电源状态；返回1==电池供电状态
int battery_read_satatus(void);

