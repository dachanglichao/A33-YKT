#include "sysTime.h"
#include <time.h>
#include <sys/time.h>  

#include <signal.h>
#include "../debug/debug.h"

union uTimeUnion timestruct;
#define N 100 //设置最大的定时器个数			
static int count = 0;  
static struct itimerval oldtv;  

struct Timer //Timer结构体，用来保存一个定时器的信息
{
    int total_time; //每隔total_time秒
    int left_time; //还剩left_time秒
    int func; //该定时器超时，要执行的代码的标志

}myTimer[N]; //定义Timer类型的数组，用来保存所有的定时器


static int timerNum  =0;//代表定时器的个数
//static int remaing[N] ;//i


//回调函数
hal_timer_timeout_handler cb[N];


uint8_t		DiagTimeString(uint8_t bbit,uint8_t * ptr)
{
	uint8_t	 aa;
	uint8_t 	 bb;
	uint8_t	 Max;
	uint8_t	MaxDay[13]={0,31,28,31,30,31,30,31,31,30,31,30,31};
	if (!bbit)
	{
		if (BCD_String_Diag(ptr,3))
			return	1;	
		aa=BCDToHex(ptr[1]);
		if (!aa || aa>12)
			return	1;
		Max=MaxDay[aa];
		if (aa==2)
		{
			bb=BCDToHex(ptr[0]);
			if (!(bb%4))
				Max=29;	
		}
		aa=BCDToHex(ptr[2]);
		if (!aa || aa>Max)
			return	1;				
	}
	else
	{
		if (BCD_String_Diag(ptr,3))
			return	1;	
		aa=BCDToHex(ptr[0]);
		if ( aa>23)
			return	1;
		aa=BCDToHex(ptr[1]);
		if ( aa>59)
			return	1;
		aa=BCDToHex(ptr[2]);
		if ( aa>59)
			return	1;
	}
	return  0;
}



//获取系统时间
union uTimeUnion  lib_systime_get_systime(void)
{
	char *wday[]={"Sun","Mon","Tue","Wed","Thu","Fri","Sat"};
	int temp;
	time_t timep;
	struct tm *p;
	time(&timep);
	p=gmtime(&timep);
	//debug("%d %d %d ",(1900+p->tm_year),(1+p->tm_mon),p->tm_mday);
//	debug("%d %d %d\n",  p->tm_hour, p->tm_min, p->tm_sec);
	if(p->tm_year>100)
	temp = p->tm_year -100;
	timestruct.S_Time.YearChar 	 = temp;
	//timestruct.S_Time.YearChar 	 = 1900+p->tm_year;
	timestruct.S_Time.MonthChar  = 1+p->tm_mon;
	timestruct.S_Time.DayChar 	 = p->tm_mday;
	timestruct.S_Time.HourChar 	 = p->tm_hour;
	timestruct.S_Time.MinuteChar = p->tm_min;
	timestruct.S_Time.SecondChar = p->tm_sec;

	timestruct.S_Time.YearChar   = gdHexToBCD(timestruct.S_Time.YearChar);
	timestruct.S_Time.MonthChar  = gdHexToBCD(timestruct.S_Time.MonthChar);
	timestruct.S_Time.DayChar    = gdHexToBCD(timestruct.S_Time.DayChar);
	timestruct.S_Time.HourChar   = gdHexToBCD(timestruct.S_Time.HourChar);
	timestruct.S_Time.MinuteChar = gdHexToBCD(timestruct.S_Time.MinuteChar);
	timestruct.S_Time.SecondChar = gdHexToBCD(timestruct.S_Time.SecondChar);

	return timestruct;

}

//设置系统时间
void lib_systime_set_systime(int year,int month,int day,int hour,int min,int sec)
{
    struct tm tptr;
    struct timeval tv;

    tptr.tm_year = year - 1900;
    tptr.tm_mon = month - 1;
    tptr.tm_mday = day;
    tptr.tm_hour = hour;
    tptr.tm_min = min;
    tptr.tm_sec = sec;

    tv.tv_sec = mktime(&tptr);
    tv.tv_usec = 0;
    settimeofday(&tv, NULL);

}


//设置定时器
void lib_systime_creat_timer(int ms,hal_timer_timeout_handler callBackFun) //新建一个计时器
{
    //设定定时器。
	 struct itimerval itv;  
    itv.it_interval.tv_sec = 0;  
    itv.it_interval.tv_usec = 1000;  
    itv.it_value.tv_sec = 0;  
    itv.it_value.tv_usec = 1000;  
    setitimer(ITIMER_REAL, &itv, &oldtv); 

	myTimer[timerNum].total_time = ms;
	myTimer[timerNum].left_time = ms;
	myTimer[timerNum].func = timerNum;
	cb[timerNum++] = callBackFun;
}


static void timeout(int sig_num) //判断定时器是否超时，以及超时时所要执行的动作
{		
	  int j;	
	  for(j=0;j<timerNum;j++)	
	  {	
		 if(myTimer[j].left_time!=0)
			myTimer[j].left_time--;
		 else	
		 {	
			 switch(myTimer[j].func)	
			 {		//通过匹配myTimer[j].func，判断下一步选择哪种操作	
				case 0:	
				//printf("------Timer 1: --Hello Aillo!\n");
				cb[0]();
				break;	
				case 1:
				cb[1]();
				//printf("------Timer 2: --Hello Jackie!\n");
				break;

				case 2:
				cb[2]();
				//printf("------Timer 3: --Hello PiPi!\n");
				break;	
			 }	
			 myTimer[j].left_time=myTimer[j].total_time; //循环计时	
		 }	
	  }	
}

//启动定时器
void lib_systime_start_timer(void)
{
	signal(SIGALRM,timeout); //接到SIGALRM信号，则执行timeout函数
}

//



