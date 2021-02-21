
#include "delay.h"

//延时函数
void Msleep(int ms)  
{  
    struct timeval delay;  
    delay.tv_sec = 0;  
    delay.tv_usec = ms * 100; //  
    select(0, NULL, NULL, NULL, &delay);  
} 

void delay_ms(int ms)  
{  
    struct timeval delay;  
    delay.tv_sec = 0;  
    delay.tv_usec = ms * 1000; //  ms  
    select(0, NULL, NULL, NULL, &delay);  
} 