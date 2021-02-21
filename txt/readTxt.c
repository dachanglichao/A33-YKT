#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h> 
#include "readTxt.h"


typedef struct item_t {
    char *key;
    char *value;
}ITEM;

IPMESSAGE posIpMessage;
POSMESSAGE posMessage;

/*
 *去除字符串右端空格
 */
char *strtrimr(char *pstr)
{
    int i;
    i = strlen(pstr) - 1;
    while (isspace(pstr[i]) && (i >= 0))
        pstr[i--] = '\0';
    return pstr;
}
/*
 *去除字符串左端空格
 */
char *strtriml(char *pstr)
{
    int i = 0,j;
    j = strlen(pstr) - 1;
    while (isspace(pstr[i]) && (i <= j))
        i++;
    if (0<i)
        strcpy(pstr, &pstr[i]);
    return pstr;
}
/*
 *去除字符串两端空格
 */
char *strtrim(char *pstr)
{
    char *p;
    p = strtrimr(pstr);
    return strtriml(p);
}


/*
 *从配置文件的一行读出key或value,返回item指针
 *line--从配置文件读出的一行
 */
int get_item_from_line(char *line, struct item_t *item)
{
 
  char *p = (char *)malloc(sizeof(char)*1000);
  char *p2 = (char *)malloc(sizeof(char)*1000);
  p = line;
   // p = strtrim(line);//删除空格的函数
        int len = sizeof(line);

    if(len <= 0){
        return 1;//空行
    }
    else if(p[0]=='#'){
        return 2;
    }else{
        p2 = strchr(p, '=');
        *p2++ = '\0';
        item->key = (char *)malloc(strlen(p) + 1);
        item->value = (char *)malloc(strlen(p2) + 1);
        strcpy(item->key,p);
        strcpy(item->value,p2);

        }
    return 0;//查询成功
}

int file_to_items(const char *file, struct item_t *items, int *num)
{
    char line[1024];
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL)
        return 1;
    int i = 0;
    while(fgets(line, 1023, fp))
    {
        char *p = strtrim(line);
        int len = strlen(p);
        if(len <= 0)
        {
            continue;
        }
        else if(p[0]=='#')
        {
            continue;
        }
        else
        {
            char *p2 = strchr(p, '=');
            /*这里认为只有key没什么意义*/
            if(p2 == NULL)
                continue;
            *p2++ = '\0';
            items[i].key = (char *)malloc(strlen(p) + 1);
            items[i].value = (char *)malloc(strlen(p2) + 1);
            strcpy(items[i].key,p);
            strcpy(items[i].value,p2);

            i++;
        }
    }
    (*num) = i;
    fclose(fp);
    return 0;
}

/*
 *读取value
 */
int read_conf_value(const char *key,char *value1,const char *file)
{
    char line[2048];
    char *key1,*key3,*key2,codenum;
    FILE *fp;
    fp = fopen(file,"r");
    if(fp == NULL)
    {
        printf("open file error\n");
          return 1;//文件打开错误
    }
      
    while (fgets(line, 2047, fp)){
        ITEM item;
        get_item_from_line(line,&item);
        
        if(!strcmp(item.key,key)){

            strcpy(value1,item.value);
            fclose(fp);
            free(item.key);
            free(item.value);
            break;
        }
        
    }
    return 0;//成功

}
int write_conf_value(const char *key,char *value,const char *file)
{
    ITEM items[20];// 假定配置项最多有20个
    int num;//存储从文件读取的有效数目
    file_to_items(file, items, &num);

    int i=0;
    //查找要修改的项
    for(i=0;i<num;i++){
        if(!strcmp(items[i].key, key)){
            items[i].value = value;
            break;
        }
    }

    // 更新配置文件,应该有备份，下面的操作会将文件内容清除
    FILE *fp;
    fp = fopen(file, "w");
    if(fp == NULL)
        return 1;

    i=0;
    for(i=0;i<num;i++){
        fprintf(fp,"%s=%s\n",items[i].key, items[i].value);
        //printf("%s=%s\n",items[i].key, items[i].value);
    }
    fclose(fp);
    //清除工作
 /* i=0;
    for(i=0;i<num;i++){
        free(items[i].key);
        free(items[i].value);
    }*/

    return 0;

}

void readIp(IPMESSAGE posIpMessage)
{
    char *key;
    char *value=NULL;
    char *file;
	char buf[200];
	int i,len=0;
    file="/opt/work/posApp/ip.txt";

    key="LOCALIP";
    value=(char *)malloc(sizeof(char)*100); 
    read_conf_value(key,value,file);
	for(i=0;i<strlen(value);i++)
	{
		if(value[i] =='\r'||value[i] =='\n')
			break;
	}	
    memcpy(posIpMessage.localip,value,i);
    printf("LOCALIP = %s",posIpMessage.localip);

    key="MASK";
    read_conf_value(key,value,file);
	for(i=0;i<strlen(value);i++)
	{
		if(value[i] =='\r'||value[i] =='\n')
			break;
	}
    memcpy(posIpMessage.mask,value,i);
    printf("MASK = %s",posIpMessage.mask);

    key="GATEWAY";
    read_conf_value(key,value,file);
	for(i=0;i<strlen(value);i++)
	{
		if(value[i] =='\r'||value[i] =='\n')
			break;
	}
    memcpy(posIpMessage.gateway,value,i);
    printf("GATEWAY = %s",posIpMessage.gateway);

     key="ROUTEIP";
    read_conf_value(key,value,file);
	for(i=0;i<strlen(value);i++)
	{
		if(value[i] =='\r'||value[i] =='\n')
			break;
	}
    memcpy(posIpMessage.routeip,value,i);
    printf("ROUTEIP = %s",posIpMessage.routeip);

     key="PORT";
    read_conf_value(key,value,file);
	for(i=0;i<strlen(value);i++)
	{
		if(value[i] =='\r'||value[i] =='\n')
			break;
	}
    memcpy(posIpMessage.port,value,i);
    printf("PORT = %s",posIpMessage.port);

	 key="STATIONCODE";
    read_conf_value(key,value,file);
	for(i=0;i<strlen(value);i++)
	{
		if(value[i] =='\r'||value[i] =='\n')
			break;
	}
    memcpy(posIpMessage.stationCode,value,i);
    printf("STATIONCODE = %s",posIpMessage.stationCode);

     key="MAINCODE";
    read_conf_value(key,value,file);
	for(i=0;i<strlen(value);i++)
	{
		if(value[i] =='\r'||value[i] =='\n')
			break;
	}
    memcpy(posIpMessage.maincode,value,i);
    printf("MAINCODE = %s",posIpMessage.maincode);

	 key="NONET";
    read_conf_value(key,value,file);
    strcpy(posIpMessage.noNetconsume,value);
    printf("noNetconsume = %s",posIpMessage.noNetconsume);

    free(value);
    value  =NULL;

}

void readposInformation(POSMESSAGE posInformation)
{
    char *key;
    char *value=NULL;
    char *file;
    file="/opt/work/posApp/posInformation.txt";


    //公共扇区号
    key="CardSector";
    value=(char *)malloc(sizeof(char)*2048); 
    read_conf_value(key,value,file);
    strcpy(posInformation.CardSector,value);
    printf("CardSector = %s\n",posInformation.CardSector);
    //卡折扣
    key="CardRebate";
    read_conf_value(key,value,file);
    strcpy(posInformation.CardRebate,value);
    printf("CardRebate = %s\n",posInformation.CardRebate);
    //卡匹配字
    key="MatchCode";
    read_conf_value(key,value,file);
    strcpy(posInformation.MatchCode,value);
    printf("MatchCode = %s\n",posInformation.MatchCode);
    //读卡密码
    key="CardKeyCode";
    read_conf_value(key,value,file);
    strcpy(posInformation.CardKeyCode,value);
    printf("CardKeyCode = %s\n",posInformation.CardKeyCode);
    //卡密钥
    key="CalCardKey";
    read_conf_value(key,value,file);
    strcpy(posInformation.CalCardKey,value);
    printf("CalCardKey = %s\n",posInformation.CalCardKey);
    //卡底金
    key="CardMinBalance";
    read_conf_value(key,value,file);
    strcpy(posInformation.CardMinBalance,value);
    printf("CardMinBalance = %s\n",posInformation.CardMinBalance);
    //日消费限额
    key="DayLimetMoney";
    read_conf_value(key,value,file);
    strcpy(posInformation.DayLimetMoney,value);
    printf("DayLimetMoney = %s\n",posInformation.DayLimetMoney);
    //传输密钥
    key="CommEncryptKey";
    read_conf_value(key,value,file);
    strcpy(posInformation.CommEncryptKey,value);
    printf("CommEncryptKey = %s\n",posInformation.CommEncryptKey);
    //卡批次
    key="CardBatchEnable";
    read_conf_value(key,value,file);
    strcpy(posInformation.CardBatchEnable,value);
    printf("CardBatchEnable = %s\n",posInformation.CardBatchEnable);
    //钱包号
    key="PurseEnable";
    read_conf_value(key,value,file);
    strcpy(posInformation.PurseEnable,value);
    printf("PurseEnable = %s\n",posInformation.PurseEnable);


    //free(value);
    value  =NULL;
}

//读取消费模式
void readPosConsumeMode(CONSUMEMODEMESSAGE consumeModeMessage)
{
    char *key;
    char *value=NULL;
    char *file;
    file="/opt/work/posApp/consumemode.txt";


    //消费模式
    key="ConsumeMode";
    value=(char *)malloc(sizeof(char)*2048); 
    read_conf_value(key,value,file);
    strcpy(consumeModeMessage.ConsumeMode,value);
    printf("ConsumeMode = %s\n",consumeModeMessage.ConsumeMode);


    //free(value);
    value  =NULL;
}



void readtexttest()
{

FILE *fp;
	if ((fp = fopen("/opt/work/posApp/test.txt", "w")) == NULL);
	{
		printf("文件开始写入\n");
	}
	int i;


	fp = fopen("/opt/work/posApp/test.txt", "w");
	for ( i = 0; i < 65535; i++)
	{
		fprintf(fp, "%d ", i);
	}
	fclose(fp);
	printf("文件排序完毕结果请看文件\n");
}

void writeposInformation(const char *key,char *value)
{
	char *file;
    file="/opt/work/posApp/posInformation.txt";

	write_conf_value(key,value,file);
}
