##############################################
#
# 单目录通用Makefile
# 目标文件可自己的设定
# 始须调试程序，修改 CFLAGS 变量为-Wall -g
# By     Lzy
# Time  2011-6-3
#
##############################################
  
# EXECUTABLE为目标的可执行文件名, 可以根据具体的情况对其进行修改。
EXECUTABLE := a33pos
TARGET_LIB := -lsqlite3

TOPDIR := $(PWD)
  
# 修改隐含规则中宏
CC := arm-linux-gnueabi-gcc
#CFLAGS := gcc
#LDFLAGS+= -static
#LDFLAGS+= -lftd2xx 
  
# 列出工作目录下所有以“.c”结尾的文件，以空格分隔，将文件列表赋给变量SOURCE
SOURCE := $(wildcard *.c) 
SOURCE += $(wildcard socket/*.c)
SOURCE += $(wildcard rc522/*.c)
SOURCE += $(wildcard txt/*.c)
SOURCE += $(wildcard byteConversion/*.c)
SOURCE += $(wildcard cJSON/*.c)
#SOURCE += $(wildcard descode/*.c)
SOURCE += $(wildcard ReadWriteCard/*.c)
SOURCE += $(wildcard systime/*.c)
SOURCE += $(wildcard crcFiles/*.c)
SOURCE += $(wildcard sqlite3/*.c)
SOURCE += $(wildcard pcf8563/*.c)
SOURCE += $(wildcard battery/*.c)
  
# 调用patsubst函数，生成与源文件对应的“.o”文件列表
OBJS := $(patsubst %.c, %.o, $(SOURCE) )
  
# 编译所有".o"文件生成可执行文件
all : $(EXECUTABLE)
$(EXECUTABLE) : $(OBJS)
	@$(CC) $(OBJS) -o $(EXECUTABLE) -lm -lpthread -L ./ $(TARGET_LIB) 
  
# 声明伪目标
.PHONY : clean
  
# 删除所有中间文件和目标文件
clean :
	@rm -f $(EXECUTABLE) $(OBJS) *.o 

