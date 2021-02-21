#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * 简单打印调试信息
 */
#define DEBUG        //打印开关
#ifdef DEBUG 
    #define debug(format, args...)  fprintf(stderr, format, ##args)
#else
    #define debug(fmt, args...)
#endif

//error code defines
typedef enum {
		SUCCESS = 0,
		FAIL,	
	
}error_code;