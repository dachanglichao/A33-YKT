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

#include "changeIp.h"

char *ifname  = "eth0";

 /***************************************************************
描述：设置IP，网关，子网掩码
参数：ifname设备号：例如eth0；Ipaddr IP地址；mask 子网掩码；gateway 网关

 *******************************************************************/
int SetIfAddr(char *ifname, char *Ipaddr, char *mask,char *gateway)
{
    int fd;
    int rc;
    struct ifreq ifr; 
    struct sockaddr_in *sin;
    struct rtentry  rt;

    printf("SetIp=%s\n",Ipaddr);
    printf("SetMask=%s\n",mask);
    printf("SetGw=%s\n",gateway);
    
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if(fd < 0)
    {
         perror("socket   error");     
        return -1;     
    }
    memset(&ifr,0,sizeof(ifr)); 
    strcpy(ifr.ifr_name,ifname); 
    sin = (struct sockaddr_in*)&ifr.ifr_addr;     
    sin->sin_family = AF_INET;  
   
    //ipaddr
    if(inet_aton(Ipaddr,&(sin->sin_addr)) < 0)   
    {     
        perror("inet_aton   error");     
        return -2;     
    }    
    
    if(ioctl(fd,SIOCSIFADDR,&ifr) < 0)   
    {     
        perror("ioctl   SIOCSIFADDR   error");     
        return -3;     
    }

    //netmask
    if(inet_aton(mask,&(sin->sin_addr)) < 0)   
    {     
        perror("netMask set   error");     
        return -4;     
    }    
    if(ioctl(fd, SIOCSIFNETMASK, &ifr) < 0)
    {
        perror("ioctl");
        return -5;
    }

    //gateway
    memset(&rt, 0, sizeof(struct rtentry));
    memset(sin, 0, sizeof(struct sockaddr_in));
    sin->sin_family = AF_INET;
    sin->sin_port = 0;
    if(inet_aton(gateway, &sin->sin_addr)<0)
    {
       printf ( "inet_aton error\n" );
    }
    memcpy ( &rt.rt_gateway, sin, sizeof(struct sockaddr_in));
    ((struct sockaddr_in *)&rt.rt_dst)->sin_family=AF_INET;
    ((struct sockaddr_in *)&rt.rt_genmask)->sin_family=AF_INET;
    rt.rt_flags = RTF_GATEWAY;
    if (ioctl(fd, SIOCADDRT, &rt)<0)
    {
        printf( "gw set error \n");
        close(fd);
        return -1;
    }
    close(fd);
    return rc;
}

// 获取IP地址
char* getip(char* ip_buf,char *ip_mask)
{
    struct ifreq temp;
    struct sockaddr_in *myaddr ,*mymask;
    int fd = 0;
    int ret = -1;
    strcpy(temp.ifr_name, ifname);
    if((fd=socket(AF_INET, SOCK_STREAM, 0))<0)
    {
        perror("socket   error");  
        return NULL;
    }
    //读取IP
    ret = ioctl(fd, SIOCGIFADDR, &temp);
    
    if(ret < 0)
    {
        return NULL;
        close(fd);
    }
        
    myaddr = (struct sockaddr_in *)&(temp.ifr_addr);
    strcpy(ip_buf, inet_ntoa(myaddr->sin_addr));

    printf("getIp=%s\n",ip_buf);
    //读取子网掩码
     ret = ioctl(fd, SIOCGIFNETMASK, &temp);  
     
    if(ret < 0)
    {
        close(fd);
        printf("get mask error");
        return NULL;
    }
        
    mymask = (struct sockaddr_in *)&(temp.ifr_netmask);
    strcpy(ip_mask, inet_ntoa(mymask->sin_addr));

    printf("getMask=%s\n",ip_mask);
    close(fd);
    printf("get ip message ok\n");
    return 0;
}

