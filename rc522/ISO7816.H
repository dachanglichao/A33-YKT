

#include "string.h"
#include <stdint.h>
#include <unistd.h>

#define     MATCHCODE_ERR           19//PSAM初始化错误(1.复位2.PPS失败3.一卡通目录4.触点接触不良)
#define     CPU_SELFILE_FAIL        20//选文件失败(1.主目录2.一卡通应用3.钱包文件)
#define     CPU_READFILE_FAIL       21//读文件失败(1.公共信息2.累计文件3.记录)
#define     CPU_REDABALANCE_FAIL    22//读余额失败
#define		Uncert_State						23//写卡、交易失败
#define     CARD_SAME_ERROR	        24//非同一张卡错误(1.序列号2.卡印刷号)
#define 	GET_PROVE_FAIL		    25//取交易认证失败
#define     PSAM_MAC1_FAIL		    26//MAC1
#define     PSAM_CHECK_MAC2		    27//MAC2
#define	    PSAM_RESET_ERROR        28//匹配字错
#define	    PSAM_FILEPARA_ERR       29//PSAM错误(1.一卡通目录2.参数读取3.站点为0)
#define		NO_PURSE_ERROR			30 //钱包没设置
#define		SYS_CONSUM_ESC			31 //钱包没设置
#define		No_Use					32 //不允许此身份消费
#define		No_ip					33 //IP丢失#define		CARD_NOCARD				0xff
  
#define     PSAM_COM_ERROR          0xf1//PSAM通讯错误(1.长度2.无响应3.响应错误) 
#define     CPU_WRITEPURSE_FAIL     0xf2//写累计失败
#define     CONSUM_PROCE_FAIL	    0xf3//交易失败



char ISO7816_PSAM(unsigned char SendLen, unsigned char * SendBuf, unsigned char * RcvLen, unsigned char * Rcvdata);

uint8_t ReadCardCommonDatas_CPU(void);//根据卡规划读取CPU卡数据

