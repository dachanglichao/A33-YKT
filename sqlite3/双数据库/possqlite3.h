#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"

//脱机交易记录结构体
struct	sRecordStruct
{
	int  recordId;
	int  recordTag;
	int  CurrentConsumMoney;
	unsigned char *recoedDatas;
	char ConsumeTime[20];		
};

//脱机交易记录结构体
struct	sRecordMoneyStruct
{
	char status;
	int  RecordToalNumber;	
	int  RecordToalMoney;		
};

//如果数据库存在打开数据库，不存在就创建数据库
int sqlite3_consume_open_db();
//sqlite3数据库测试
void sqlite3_test();
//插入数据
int sqlite3_consume_insert_db(struct sRecordStruct sRt);
//查询数据
struct sRecordStruct sqlite3_consume_query_record_db(uint8_t number);
//关闭数据库
void sqlite3_consume_close_db();
//查询未采消费总额跟消费笔数
struct	sRecordMoneyStruct sqlite3_consume_query_consumemoney_db(void);
//查询已采消费总额跟消费笔数
struct	sRecordMoneyStruct sqlite3_consume_query_collectedConsumemoney_db(void);
/*查询未采集记录初始记录的id=NoCollectRecordIndex
bit = 0 查找记录初始ID
bit = 1 查找记录最后的ID
********************************************/
uint32_t  sqlite3_consume_query_NoCollectRecordIndex_db(uint8_t bit);
//删除记录
int sqlite3_consume_delete_db(struct sRecordStruct sRt);
//从未采记录数据库中把已采记录移动到已采数据库
int sqlite3_consume_move_db(int index);




