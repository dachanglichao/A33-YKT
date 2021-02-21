
#include <stdint.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "iso7816.h"
#include "delay.h"
#include "CalucationFile.h"



int ReadCardCommonDatas(uint8_t *scidcode, uint8_t *CardSerialNum,uint8_t usesectornum,uint8_t *CardPrinterNum,uint8_t *LimtConsumeData_CPU)
//写入脱机限额
char WriteNoNetLimitMoney(char *buf,char *scidcode, char *CardSerialNum,char usesectornum,char *CardPrinterNum);

