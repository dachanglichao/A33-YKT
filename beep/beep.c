#include <stdio.h>
#include <unistd.h>
#include <getopt.h>
#include <fcntl.h>
#include <linux/ioctl.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h> 
#include <time.h>
#include <signal.h>
#include "../debug/debug.h"


static int fd;
static int beeptimerNum=0;//i代表定时器的个数；t表示时间，逐秒递增
static int beepremaing ;

static int BeepTimes =0;
static int BeepDelayTimeCount=0;


//设置定时器
static void lib_beep_creat_timer(int f) //新建一个计时器
{
    //设定定时器。
    beeptimerNum =f;
    beepremaing = alarm(beeptimerNum);
}

//定时器中断函数
static void lib_beep_work_state(void)
{
	BeepDelayTimeCount++;
	if(BeepTimes)
	{
		
		if(BeepDelayTimeCount>10)
		{
			write(fd,1,1);
		}
		else
		{
			write(fd,0,1);
		}
		if(BeepDelayTimeCount>=20)
		{
			BeepDelayTimeCount=0;
			BeepTimes--;
		}
		
	}
	
}

static void lib_beep_timeout(int sig_num) //判断定时器是否超时，以及超时时所要执行的动作
{	
	if(sig_num = SIGALRM)
	{
		beepremaing = alarm(beeptimerNum);
		lib_beep_work_state();
	}
}
//启动定时器
static void lib_systime_start_timer(void)
{
	signal(SIGALRM,lib_beep_timeout); //接到SIGALRM信号，则执行timeout函数
}


//蜂鸣器初始化
void lib_beep_init()
{
	fd = open("/sys/class/gpio_sw/PF3/data",O_RDWR);
	debug("beep_fd==%d\n",fd);
	lib_beep_creat_timer(1);
}

//蜂鸣器蜂鸣
void lib_beep_on(int num)
{
	write(fd,1,1);

	if(!BeepTimes)
	{
		BeepTimes=num;
		BeepDelayTimeCount=0;
	}
	
}
