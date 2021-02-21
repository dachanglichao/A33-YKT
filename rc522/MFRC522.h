
#include "string.h"
#include <stdint.h>
#include <unistd.h>



//宏定义
#define CARD_NOCARD   0xff
#define CARD_OK   0
#define MEMDEV_IOC_MAGIC  'k'
#define RC522_Reset	_IOR(MEMDEV_IOC_MAGIC, 1,int)
#define RC522_Read	_IOR(MEMDEV_IOC_MAGIC, 2,int)
#define RC522_Write	_IOR(MEMDEV_IOC_MAGIC, 3,int)
#define		_RC531BufSize		66 //模块最大数据流长度(=实际的数据长度+2) //h531
//
/////////////////////////////////////////////////////////////////////
//MF522命令字
/////////////////////////////////////////////////////////////////////
#define PCD_IDLE              0x00               //取消当前命令
#define PCD_AUTHENT           0x0E               //验证密钥
#define PCD_RECEIVE           0x08               //接收数据
#define PCD_TRANSMIT          0x04               //发送数据
#define PCD_TRANSCEIVE        0x0C               //发送并接收数据
#define PCD_RESETPHASE        0x0F               //复位
#define PCD_CALCCRC           0x03               //CRC计算

/////////////////////////////////////////////////////////////////////
//Mifare_One卡片命令字
/////////////////////////////////////////////////////////////////////
#define PICC_REQIDL           0x26               //寻天线区内未进入休眠状态
#define PICC_REQALL           0x52               //寻天线区内全部卡
#define PICC_ANTICOLL1        0x93               //防冲撞
#define PICC_ANTICOLL2        0x95               //防冲撞
#define PICC_AUTHENT1A        0x60               //验证A密钥
#define PICC_AUTHENT1B        0x61               //验证B密钥
#define PICC_READ             0x30               //读块
#define PICC_WRITE            0xA0               //写块
#define PICC_DECREMENT        0xC0               //扣款
#define PICC_INCREMENT        0xC1               //充值
#define PICC_RESTORE          0xC2               //调块数据到缓冲区
#define PICC_TRANSFER         0xB0               //保存缓冲区中数据
#define PICC_HALT             0x50               //休眠

/////////////////////////////////////////////////////////////////////
//MF522 FIFO长度定义
/////////////////////////////////////////////////////////////////////
#define DEF_FIFO_LENGTH       64                 //FIFO size=64byte

/////////////////////////////////////////////////////////////////////
//MF522寄存器定义
/////////////////////////////////////////////////////////////////////
// PAGE 0
#define     RFU00                 0x00    
#define     CommandReg            0x01   //启动停止 
#define     ComIEnReg             0x02    // 使能和禁用中断请求
#define     DivlEnReg             0x03    //使能和禁用中断请求
#define     ComIrqReg             0x04    // 中断请求
#define     DivIrqReg             0x05	  //中断请求
#define     ErrorReg              0x06    //显示上一个指令执行的错误状态
#define     Status1Reg            0x07    //通信状态
#define     Status2Reg            0x08    //接收和发送状态
#define     FIFODataReg           0x09	  // 64字节FIFO缓冲区的输入和输出
#define     FIFOLevelReg          0x0A	  //FIFO缓冲区中已存储字节的数量
#define     WaterLevelReg         0x0B	  //FIFO缓冲区溢出和空警告
#define     ControlReg            0x0C	  //多种控制寄存器
#define     BitFramingReg         0x0D	 //面向位的帧的调整
#define     CollReg               0x0E	 //检查产生位冲突的第一个位的地址
#define     RFU0F                 0x0F	  //
// PAGE 1     
#define     RFU10                 0x10
#define     ModeReg               0x11 //发送和接收的通用模式的设置
#define     TxModeReg             0x12 //数据发送时的传输速率
#define     RxModeReg             0x13 //数据接收时的传输速率 
#define     TxControlReg          0x14 // 天线驱动器引脚TX1和TX2的控制
#define     TxAutoReg             0x15 //发送调制的设置
#define     TxSelReg              0x16	//选择天线驱动的内部信号源
#define     RxSelReg              0x17	 //内部接收器的设置 
#define     RxThresholdReg        0x18	// 位解码器阀值的选择
#define     DemodReg              0x19	// 解调器的设置
#define     RFU1A                 0x1A	 //
#define     RFU1B                 0x1B
#define     RFU1C             	 0x1C
#define     RFU1D                 0x1D
#define     RFU1E                 0x1E
#define     SerialSpeedReg        0x1F //
// PAGE 2    
#define     RFU20                 0x20  
#define     CRCResultRegH         0x21 //CRC计算后MSB位和LSB位的值
#define     CRCResultRegL         0x22
#define     RFU23                 0x23
#define     ModWidthReg           0x24 //调制宽度的设置
#define     RFU25                 0x25 //
#define     RFCfgReg              0x26	//接收增益的配置
#define     GsNReg                0x27	//选择天线驱动器引脚 TX1 和TX2在调制时的电导值
#define     CWGsCfgReg            0x28	//定义 p-driver 无调制的输出电导
#define     ModGsCfgReg           0x29	//定义 p-driver 经过调制的输出电导
#define     TModeReg              0x2A // 内部定时器的设置
#define     TPrescalerReg         0x2B //
#define     TReloadRegH           0x2C	//16位定时器的重载值
#define     TReloadRegL           0x2D
#define     TCounterValueRegH     0x2E	//显示16定时器的当前值
#define     TCounterValueRegL     0x2F
// PAGE 3      
#define     RFU30                 0x30
#define     TestSel1Reg           0x31	  //测试信号的配置
#define     TestSel2Reg           0x32	  //测试信号的配置和 PRBS控制
#define     TestPinEnReg          0x33	  //使能 D1 到 D7的输出驱动器 
#define     TestPinValueReg       0x34	 //定义引脚DI到D7当其用于I/O口时的电平值
#define     TestBusReg            0x35	//显示内部测试总线的状态
#define     AutoTestReg           0x36	//数字自检的控制
#define     VersionReg            0x37
#define     AnalogTestReg         0x38
#define     TestDAC1Reg           0x39  
#define     TestDAC2Reg           0x3A   
#define     TestADCReg            0x3B   
#define     RFU3C                 0x3C   
#define     RFU3D                 0x3D   
#define     RFU3E                 0x3E   
#define     RFU3F		  		       0x3F

/////////////////////////////////////////////////////////////////////
//和MF522通讯时返回的错误代码
/////////////////////////////////////////////////////////////////////
#define MI_OK                     0
#define MI_NOTAGERR               1
#define MI_ERR                    2
//
#define MAX_LEN					  80

void MFRC522_Initializtion(void) ;//RC522初始化

/******************寻卡操作区分M1还是CPU卡*******************************/
uint8_t MFRC522_Auth(uint8_t authMode, uint8_t BlockAddr, uint8_t *Sectorkey, uint8_t *serNum) ;
uint8_t MFRC522_Read(uint8_t blockAddr, uint8_t *recvData) ;
uint8_t MFRC522_Write(uint8_t blockAddr, uint8_t *writeData) ;
uint8_t Request_Card_info(uint8_t Cardtype ,uint8_t RcvLen, void * Rcvdata,uint8_t *CardSerialNumBuf);