#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//
#define YKT_CONSUNE_CMD 			0xA0//消费指令
#define YKT_POLL_RECORD_CMD 		0xA1//查询卡的消费记录
#define YKT_POLL_CURDAY_RECORD 		0XA2//查询设备当日的消费记录
#define YKT_POLL_CURMON_RECORD 		0XA3//查询设备当月的消费记录
//
int app_ykt_main(void);
