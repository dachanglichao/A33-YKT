#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <error.h>
#include <net/route.h>



extern char *ifname ;

 /******************************************************************
描述：设置IP，网关，子网掩码
参数：ifname设备号：例如eth0；Ipaddr IP地址；mask 子网掩码；gateway 网关

 *******************************************************************/
int SetIfAddr(char *ifname, char *Ipaddr, char *mask,char *gateway);