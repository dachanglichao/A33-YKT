#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include <stdio.h>  
#include <stdbool.h>  
#include <fcntl.h>  
#include <linux/ioctl.h>
#include <stdlib.h> 


//脱机交易记录结构体
struct	sRecordStruct
{
	int  recordId;
	int  recordTag;
	int  CurrentConsumMoney;
	uint32_t  ConsumeTime;
	char recoedDatas[80];		
};


//价格方案结构体
struct	sMoneyplanStruct
{
	int  serid;//卡身份
	char recoedDatas[30];//价格方案		
};


//脱机交易记录结构体
struct	sRecordMoneyStruct
{
	char status;
	int  RecordToalNumber;	
	int  RecordToalMoney;		
};

extern struct sRecordStruct recordSt;


//如果数据库存在打开数据库，不存在就创建数据库
int sqlite3_consume_open_db();
//sqlite3数据库测试
void sqlite3_test();
//插入数据
int sqlite3_consume_insert_db(struct sRecordStruct sRt);
//上传交易记录
int sqlite3_consume_query_record_db(int number);
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
int  sqlite3_consume_query_tRecordIndex_db(int bit);
//删除记录
int sqlite3_consume_delete_db(struct sRecordStruct sRt);
//从未采记录数据库中把已采记录移动到已采数据库
int sqlite3_consume_move_db(int index,int number);
//查询某日--某日的消费记录总额跟笔数
struct	sRecordMoneyStruct sqlite3_consume_query_daymoney_db(char *daytime1,char *datatime2);

//复采交易记录
struct	sRecordStruct sqlite3_consume_query_collectedRecord_db(uint32_t daytime);
//清空存储记录数据库
void sqlite3_consume_clr_db(void);


//如果黑名单数据库存在打开数据库，不存在就创建数据库
int sqlite3_blaknumber_open_db(void);
//插入黑名单数据库
int sqlite3_blaknumber_insert_db(uin32_t);
//删除黑名单从数据库
int sqlite3_blaknumber_del_db(uin32_t);
//清空黑名单数据库
int sqlite3_blaknumber_clr_db(void);
//查询是否在黑名单数据库
int sqlite3_blaknumber_query_db(uin32_t);



//创建价格方案数据库
int sqlite3_moneyplan_open_db(void);
//插入价格方案到数据库
int sqlite3_moneyplan_insert_db(struct sMoneyplanStruct stru);
//查找符合身份的价格方案
struct sMoneyplanStruct sqlite3_moneyplan_query_db(uint32_t serid );
//查找价格方案存储记录指针
uint32_t  sqlite3_query_moneyplan_db(void);
//清空价格方案数据库
int sqlite3_moneyplan_clr_db(void);





void sqlite3_test_gzf();




