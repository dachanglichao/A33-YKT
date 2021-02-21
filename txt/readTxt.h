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

//设备的IP地址
typedef struct ipmessage_t {
    char *localip;
    char *mask;
    char *gateway;
    char *routeip;
    char *port;
    char *maincode;
	char *stationCode;
	char *noNetconsume;
}IPMESSAGE;

typedef struct posmessage_t {
	char *CardSector;
	char *CardRebate;
	char *MatchCode;
	char *CardKeyCode;
	char *CalCardKey;
	char *CardMinBalance;
	char *DayLimetMoney;
	char *CommEncryptKey;
	char *CardBatchEnable;
	char *PurseEnable;
}POSMESSAGE;

typedef struct consumemode_t {
	char *ConsumeMode;
	char *Money;

}CONSUMEMODEMESSAGE;

extern char *ifname ;
extern void readIp(IPMESSAGE);
void readposInformation(POSMESSAGE posInformation);
void writeposInformation(const char *key,char *value);

//读取消费模式
void readPosConsumeMode(CONSUMEMODEMESSAGE consumeModeMessage);

void readtexttest();