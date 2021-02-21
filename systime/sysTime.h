
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

//时间结构体
struct	sTimeStruct
{
	unsigned char	YearChar;
	unsigned char	MonthChar;
	unsigned char	DayChar;
	unsigned char	HourChar;
	unsigned char	MinuteChar;
	unsigned char	SecondChar;
};
union	uTimeUnion
{
	unsigned char	TimeString[6];
	struct	sTimeStruct	S_Time;
};

//callback
typedef void (*hal_timer_timeout_handler)(void);

union uTimeUnion  lib_systime_get_systime(void);
//设置系统时间
void lib_systime_set_systime(int year,int month,int day,int hour,int min,int sec);
//设置定时器
void lib_systime_creat_timer(int ms,hal_timer_timeout_handler callBackFun); //新建一个计时器; //新建一个计时器
//启动定时器
void lib_systime_start_timer(void);

uint8_t		DiagTimeString(uint8_t bbit,uint8_t * ptr);








