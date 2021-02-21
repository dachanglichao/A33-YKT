#include <stdio.h>  
#include <stdbool.h>  
#include <fcntl.h>  
#include <linux/ioctl.h>
#include <stdlib.h> 

#include "../debug/debug.h"
#include "possqlite3.h"
#include "../ExternVariableDef.h"

static char *zErrMsg =NULL;
static char **azResult=NULL; //二维数组存放结果
static int nrow=0;
static int ncolumn = 0;
sqlite3 *recod_db=NULL;//未采记录数据库句柄
sqlite3 *blknumber_db=NULL;//黑名单数据库句柄
sqlite3 *moneyplan_db=NULL;//价格方案数据库句柄


static bool recordMux =false;//数据库操作加锁

//上传的交易记录
struct sRecordStruct recordSt;
uint32_t blknumberId = 0;


//创建价格方案数据库
int sqlite3_moneyplan_open_db(void)
{
     int len;
	 
	/* 打开或创建未采记录数据库 */
	len = sqlite3_open("moneyplan.db",&moneyplan_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(moneyplan_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(moneyplan_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named moneyplan_db successfully!\n");
	
	/* 创建未采记录表 */
	//recordIdb 一直累加不清零
	//消费时间年/月/日
	char *sql = "create table moneyplan (id int,datas char);" ;

	sqlite3_exec(moneyplan_db,sql,NULL,NULL,&zErrMsg);
	
	//sqlite3_close(recod_db);
	return len;
}


//插入价格方案到数据库
int sqlite3_moneyplan_insert_db(struct sMoneyplanStruct stru)
{
	char tempdata[200];
	int len;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的


	/* 打开数据库 */
	len = sqlite3_open("moneyplan.db",&moneyplan_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(moneyplan_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(moneyplan_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	sprintf(tempdata,"insert into moneyplan values(%d,'%s');",stru.serid,stru.recoedDatas);
	debug("blknumber tempdata= %s\n", tempdata);
	sqlite3_exec(moneyplan_db,tempdata,NULL,NULL,&zErrMsg);
	

	sqlite3_close(moneyplan_db);
}

//查找符合身份的价格方案
struct sMoneyplanStruct sqlite3_moneyplan_query_db(uint32_t serid )
{
	char tempdata[100],bufer[40];
	int len,numberbak =-1;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的
	memset(bufer,10,0);
	memset(tempdata,100,0);
	struct sMoneyplanStruct stru;

	int ret =-1;

	/* 打开数据库 */
	len = sqlite3_open("moneyplan.db",&moneyplan_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(moneyplan_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(moneyplan_db);
	   return stru;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	
	sprintf(tempdata,"select *from moneyplan where id = %d;",serid);
	debug("moneyplan tempdata= %s\n", tempdata);

	if (sqlite3_prepare(moneyplan_db, tempdata, -1, &stmt, NULL) == SQLITE_OK) {
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			buf = sqlite3_column_text(stmt, 1);
			memcpy(stru.recoedDatas,buf,30);
			debug("data =%s\n",buf);
			return stru;
		}
	}

	return stru;//没有找到符合身份的价格方案
	
}
//查找价格方案存储记录指针
uint32_t  sqlite3_query_moneyplan_db(void)
{
	int index =0,indexbak=0,len;
	sqlite3_stmt *stmt = NULL; // 用来取数据的
	bool start =false;

	/* 打开数据库 */
	len = sqlite3_open("moneyplan.db",&moneyplan_db);
	if( len )
	{
		/*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(moneyplan_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
		sqlite3_close(moneyplan_db);
		return 0;
	}
	else 
		printf("You have opened a sqlite3 database named record successfully!\n");
 
	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	char *sql="select *from moneyplan where id ";
	if (sqlite3_prepare_v2(moneyplan_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
	{   
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{	
			// 取出第0列字段的值ID号
			index= sqlite3_column_int(stmt, 0);	
			index++;
		}

		sqlite3_finalize(stmt);
		sqlite3_close(moneyplan_db);
		return index;	 
	}
	
   	return index;
}

//清空价格方案数据库
int sqlite3_moneyplan_clr_db(void)
{
	char tempdata[200];
	int len;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	
	/* 打开数据库 */
	len = sqlite3_open("moneyplan.db",&moneyplan_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(moneyplan_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(moneyplan_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	
	sprintf(tempdata,"delete from moneyplan_db;");
	debug("blknumber tempdata= %s\n", tempdata);
	sqlite3_exec(moneyplan_db,tempdata,NULL,NULL,&moneyplan_db);
	
	sqlite3_close(moneyplan_db);
}


//如果黑名单数据库存在打开数据库，不存在就创建数据库
int sqlite3_blaknumber_open_db(void)
{
     int len;
	 
	/* 打开或创建未采记录数据库 */
	len = sqlite3_open("blknum.db",&blknumber_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(blknumber_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(blknumber_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named recod_db successfully!\n");
	
	/* 创建未采记录表 */
	//recordIdb 一直累加不清零
	//消费时间年/月/日
	char *sql = "create table blknumber (id int,number int);" ;

	sqlite3_exec(blknumber_db,sql,NULL,NULL,&zErrMsg);
	
	//sqlite3_close(recod_db);
	return len;
}

//查询是否在黑名单数据库
int sqlite3_blaknumber_query_db(uint32_t number)
{
	char tempdata[100],bufer[10];
	int len,numberbak =-1;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的
	memset(bufer,10,0);
	memset(tempdata,100,0);

	int ret =-1;

	/* 打开数据库 */
	len = sqlite3_open("blknum.db",&blknumber_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(blknumber_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(blknumber_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	
	sprintf(tempdata,"select *from blknumber where number = %d;",number);
	debug("blknumber tempdata= %s\n", tempdata);

	ret = -1;
	if (sqlite3_prepare(blknumber_db, tempdata, -1, &stmt, NULL) == SQLITE_OK) {
		//sqlite3_bind_int(stmt, 0, CardNum);
		while (sqlite3_step(stmt) == SQLITE_ROW) {
			numberbak = sqlite3_column_int(stmt, 1);
			if (numberbak == number) {
				break;
			}
		}
	}
	if(numberbak == number)
	{
		debug("黑名单卡\n");
		sqlite3_finalize(stmt);
		sqlite3_close(blknumber_db);
		return 1;

	}
	else
	{
		debug("正常卡卡\n");
		sqlite3_finalize(stmt);
		sqlite3_close(blknumber_db);
		return 0;
	}

	
}


//插入黑名单数据库
int sqlite3_blaknumber_insert_db(uint32_t number)
{
	char tempdata[200];
	int len;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	if(sqlite3_blaknumber_query_db(number))//数据库中已经存在
		return;

	
	/* 打开数据库 */
	len = sqlite3_open("blknum.db",&blknumber_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(blknumber_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(blknumber_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");


	sprintf(tempdata,"insert into blknumber values(%d,%d);",blknumberId++,number);
	debug("blknumber tempdata= %s\n", tempdata);
	sqlite3_exec(blknumber_db,tempdata,NULL,NULL,&zErrMsg);
	

	sqlite3_close(blknumber_db);
}
//删除黑名单从数据库
int sqlite3_blaknumber_del_db(uint32_t number)
{
	char tempdata[200];
	int len;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	
	/* 打开数据库 */
	len = sqlite3_open("blknum.db",&blknumber_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(blknumber_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(blknumber_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	
	sprintf(tempdata,"delete from blknumber where number = %d;",number);
	debug("blknumber tempdata= %s\n", tempdata);
	sqlite3_exec(blknumber_db,tempdata,NULL,NULL,&zErrMsg);
	

	sqlite3_close(blknumber_db);
}

//清空黑名单数据库
int sqlite3_blaknumber_clr_db(void)
{
	char tempdata[200];
	int len;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	
	/* 打开数据库 */
	len = sqlite3_open("blknum.db",&blknumber_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(blknumber_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(blknumber_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	
	sprintf(tempdata,"delete from blknumber;");
	debug("blknumber tempdata= %s\n", tempdata);
	sqlite3_exec(blknumber_db,tempdata,NULL,NULL,&zErrMsg);
	
	sqlite3_close(blknumber_db);
}


//如果数据库存在打开数据库，不存在就创建数据库
int sqlite3_consume_open_db(void)
{
     int len;
	 
	/* 打开或创建未采记录数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named recod_db successfully!\n");
	
	/* 创建未采记录表 */
	//recordIdb 一直累加不清零
	//消费时间年/月/日
	char *sql = "create table consume (id int primary key,tag int,money int,time char,datas char);" ;

	sqlite3_exec(recod_db,sql,NULL,NULL,&zErrMsg);
	
	//sqlite3_close(recod_db);
	return len;
}

//插入数据
int sqlite3_consume_insert_db(struct sRecordStruct sRt)
{
	char tempdata[200];
	int len;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	
	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	/*插入数据	*/
	debug("sRt.CurrentConsumMoney==%d\n",sRt.CurrentConsumMoney);

	sprintf(tempdata,"insert into consume values(%d,%d,%d,%d,'%s');",sRt.recordId,sRt.recordTag, sRt.CurrentConsumMoney ,sRt.ConsumeTime,sRt.recoedDatas);
	debug("tempdata= %s\n", tempdata);
	sqlite3_exec(recod_db,tempdata,NULL,NULL,&zErrMsg);
	

	sqlite3_close(recod_db);
}

//查询交易记录
int sqlite3_consume_query_record_db( int number)
{
	int len;
	char tempdata[200];
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	}
	
	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	sprintf(tempdata,"select *from consume where id = %d;",number);
	debug("tempdata = %s\r\n",tempdata);

	if (sqlite3_prepare_v2(recod_db, tempdata, -1, &stmt, NULL) == SQLITE_OK) 
	{	
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{
			// 取出第0列字段的值
			recordSt.recordId = sqlite3_column_int(stmt, 0);
			// 取出第1列字段的值
			recordSt.recordTag = sqlite3_column_int(stmt, 1);
			// 取出第2列字段的值
			recordSt.CurrentConsumMoney = sqlite3_column_int(stmt, 2);
			// 取出第2列字段的值
			recordSt.ConsumeTime = sqlite3_column_int(stmt, 3);
			// 取出第4列字段的值
			buf = sqlite3_column_text(stmt, 4);
			memcpy(recordSt.recoedDatas,buf,64);
			debug("recordId = %d; recordTga = %d; CurrentConsumMoney = %d, ConsumeTime = %d,recordDatas = %s \r\n",recordSt.recordId,recordSt.recordTag,recordSt.CurrentConsumMoney,recordSt.ConsumeTime,recordSt.recoedDatas);			
			break;
		}
	}
	else
	{
		debug("select *from ConsumeData where recordTag > 0 err \r\n");
	}
	sqlite3_finalize(stmt);	
	sqlite3_close(recod_db);
}

//把已采记录的recordTag由0改写成100
int sqlite3_consume_move_db(int index,int number)
{
	char i=0,tempdata[200];
	int len;
	struct sRecordStruct sRt;
	sqlite3_stmt *stmt = NULL; // 用来取数据的
	sqlite3_stmt *stmt1 = NULL; // 用来取数据的
	
	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	//根据ID号从数据库中查找到要删除的记录
	
	//修改本条记录的recordTag
	for(i=0;i<number;i++)
	{
		sprintf(tempdata,"update consume set tag = 1 where id = %d ",index+i);
		//sprintf(tempdata,"select *from ConsumeData where recordId = %d;",index+i);
		debug("tempdata= %s\n", tempdata);
		//sqlite3_exec(recod_db,tempdata,NULL,NULL,&zErrMsg);
		if(sqlite3_exec(recod_db, tempdata, NULL, NULL, &zErrMsg ) != SQLITE_OK)
		{
			debug("zErrMsg=%s\n", zErrMsg);
		}
		else
		{
			debug("Update done.\n");	
		}
	}
	sqlite3_close(recod_db);
}

/****************************************************
bit = 0 查找未采记录指针ID =NoCollectRecordIndex
bit = 1 查找记录最后的ID = SaveRecordIndex
*************************************************/
uint32_t  sqlite3_consume_query_RecordIndex_db(uint8_t bit)
{
	int index =0,indexbak=0,len;
	sqlite3_stmt *stmt = NULL; // 用来取数据的
	bool start =false;

	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
		/*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
		sqlite3_close(recod_db);
		return 0;
	}
	else 
		printf("You have opened a sqlite3 database named record successfully!\n");
 
	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	if(!bit)//查找未采集记录指针
	{
		char *sql="select *from consume where tag = 0";
		if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
		{   
		  while(sqlite3_step(stmt) == SQLITE_ROW ) 
		  {	
			// 取出第0列字段的值ID号
			index= sqlite3_column_int(stmt, 0);
			sqlite3_finalize(stmt);
			sqlite3_close(recod_db);
			return index ;
		
		  }
			char *sql="select *from consume where tag =0 || tag =1";
			if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
			{   
				while(sqlite3_step(stmt) == SQLITE_ROW ) 
				{	
				// 取出第0列字段的值ID号
				index= sqlite3_column_int(stmt, 0);	
				}
			}
			sqlite3_finalize(stmt);
			sqlite3_close(recod_db);
			index++;
			return index;
		 
		}

	}
	else//查找存储记录指针
	{
		char *sql="select *from consume where tag =0 || tag =1";
		if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
		{   
		  while(sqlite3_step(stmt) == SQLITE_ROW ) 
		  {	
			// 取出第0列字段的值ID号
			index= sqlite3_column_int(stmt, 0);	
		  }
		}
		sqlite3_finalize(stmt);
		sqlite3_close(recod_db);
		index++;
		return index;
	}
	
			
   	return index;
}

//复采交易记录
struct	sRecordStruct sqlite3_consume_query_collectedRecord_db(uint32_t daytime)
{
	char tempdata[200];
	char *dTime;
	char *buf;
	int i,len;
	//struct sRecordStruct sRt ;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*fprintf函数格式化输出错误信息到指定的stderr文件流中*/
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	   return recordSt;
	}
	else 
		printf("You have opened a sqlite3 database named record successfully!\n");
	
	recordSt.recordId = 0;
	recordSt.recordTag =0;
	recordSt.CurrentConsumMoney = 0;
	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	sprintf(tempdata,"select *from consume where time = %d;",daytime);
	if (sqlite3_prepare_v2(recod_db, tempdata, -1, &stmt, NULL) == SQLITE_OK) 
	{   
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{	
			// 取出第0列字段的值
			recordSt.recordId = sqlite3_column_int(stmt, 0);
			// 取出第1列字段的值
			recordSt.recordTag = sqlite3_column_text(stmt, 1);
			// 取出第2列字段的值
			recordSt.CurrentConsumMoney = sqlite3_column_int(stmt, 2);
			// 取出第4列字段的值
			buf = sqlite3_column_text(stmt, 4);
			memcpy(recordSt.recoedDatas,buf,64);
			debug("recordId = %d; recordTga = %s; CurrentConsumMoney = %d; recordDatas = %s \r\n",recordSt.recordId,recordSt.recordTag,recordSt.CurrentConsumMoney,recordSt.recoedDatas);			
			break;
		}
		sqlite3_finalize(stmt);
	}
	//sqlite3_close(recod_db);
   	return recordSt;
}

//查询某日--某日的消费记录总额跟笔数
struct	sRecordMoneyStruct sqlite3_consume_query_daymoney_db(char *daytime1,char *daytime2)
{
	char tempdata[200];
	int i,len;
	uint32_t dTime, day1,day2;
	
	struct	sRecordMoneyStruct RecordStr;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	   RecordStr.RecordToalMoney =0;
	   RecordStr.RecordToalNumber =0;
	   RecordStr.status = 0;//查询失败
	   return RecordStr;
	}
	else 
		printf("You have opened a sqlite3 database named record successfully!\n");
    
	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	day1 = atoi(daytime1);
	day2 = atoi(daytime2);
	//debug("daytim1 = %d day2time = %d \n",day1,day2);
	sprintf(tempdata,"select *from consume where time >= %d ",day1);
	debug("tempdata = %s\n",tempdata);
	RecordStr.RecordToalMoney = 0;
	RecordStr.RecordToalNumber = 0;
	if (sqlite3_prepare_v2(recod_db, tempdata, -1, &stmt, NULL) == SQLITE_OK) 
	{   
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{	
			// 取出第3列字段的值
			dTime = sqlite3_column_int(stmt, 3);
		//	debug("dTime = %d\n",dTime);
			RecordStr.RecordToalNumber++;
			// 取出第2列字段的值
			RecordStr.RecordToalMoney += sqlite3_column_int(stmt, 2);
		}
		sqlite3_finalize(stmt);
		RecordStr.status = 1;//查询成功
		debug("RecordStr.RecordToalNumber =%d, RecordStr.RecordToalMoney =%d\n",RecordStr.RecordToalNumber,RecordStr.RecordToalMoney);
	}
	//sqlite3_close(recod_db);
   	return RecordStr;
}

//查询未采消费总额跟消费笔数
struct	sRecordMoneyStruct sqlite3_consume_query_consumemoney_db(void)
{
	int i,len;
	struct	sRecordMoneyStruct RecordStr;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	   RecordStr.RecordToalMoney =0;
	   RecordStr.RecordToalNumber =0;
	   RecordStr.status = 0;//查询失败
	   return RecordStr;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named record successfully!\n");
    
	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	char *sql="select *from consume where tag = 0 ";
	RecordStr.RecordToalMoney = 0;
	RecordStr.RecordToalNumber = 0;
	if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
	{   
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{	
		RecordStr.RecordToalNumber++;
		// 取出第2列字段的值
		RecordStr.RecordToalMoney += sqlite3_column_int(stmt, 2);
		}
		RecordStr.status = 1;//查询成功
		debug("RecordStr.RecordToalNumber =%d, RecordStr.RecordToalMoney =%d\n",RecordStr.RecordToalNumber,RecordStr.RecordToalMoney);
	}
	sqlite3_finalize(stmt);
	//sqlite3_close(recod_db);
   	return RecordStr;
}

//查询已采消费总额跟消费笔数
struct	sRecordMoneyStruct sqlite3_consume_query_collectedConsumemoney_db(void)
{
	int i,len;
	struct	sRecordMoneyStruct RecordStr;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	   RecordStr.RecordToalMoney =0;
	   RecordStr.RecordToalNumber =0;
	   RecordStr.status = 0;//查询失败
	   return RecordStr;
	}
	else 
		printf("You have opened a sqlite3 database named recod_db successfully!\n");

	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	char *sql="select *from collecte consume where tag = 1 ";
	RecordStr.RecordToalMoney = 0;
	RecordStr.RecordToalNumber = 0;
	if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
	{   
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{	
			RecordStr.RecordToalNumber++;
			// 取出第2列字段的值
			RecordStr.RecordToalMoney += sqlite3_column_int(stmt, 2);
		}
		RecordStr.status = 1;//查询成功
		debug("RecordStr.RecordToalNumber =%d, RecordStr.RecordToalMoney =%d\n",RecordStr.RecordToalNumber,RecordStr.RecordToalMoney);
	}
	//sqlite3_close(recod_db);
   	return RecordStr;
}

//删除记录
int sqlite3_consume_delete_db(struct sRecordStruct sRt)
{
	char tempdata[200];
	int len;

	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	}

	/* 删除某个特定的数据 */
	sprintf(tempdata,"delete from consume where money = %d and time = %s;",sRt.CurrentConsumMoney,sRt.ConsumeTime);

	debug("tempdata= %s\n", tempdata);
	sqlite3_exec( recod_db , tempdata , NULL , NULL , &zErrMsg );
	//sqlite3_close(recod_db);
}

//清空存储记录数据库
void sqlite3_consume_clr_db(void)
{
	char tempdata[200];
	int len;
	char *buf;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	
	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(recod_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	
	sprintf(tempdata,"delete from consume;");
	debug("record tempdata= %s\n", tempdata);
	sqlite3_exec(recod_db,tempdata,NULL,NULL,&zErrMsg);
	
	sqlite3_close(recod_db);
}


void sqlite3_test_gzf()
{
	sqlite3 *db = NULL;
	char *zErrMsg = 0;
	char sql[1024];
	int rc,ID, updata;
	uint8_t i, data[32];
	sqlite3_stmt *stmt;

	rc = sqlite3_open("test.db", &db);
	if (rc) {
		debug("Can't open database: %s\n", sqlite3_errmsg(db));
		sqlite3_close(db);
		exit(1);
	}

	sprintf(sql,
			"CREATE TABLE IF NOT EXISTS RecordFile(ID INTEGER,isUpdate INTEGER,data BLOB)");
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc)
		debug("create data %s\n", zErrMsg);

	sprintf(sql, "create index ID on RecordFile(ID);");
	rc = sqlite3_exec(db, sql, 0, 0, &zErrMsg);

	if (rc)
		debug("create data %s\n", zErrMsg);

	for(i=0;i<100;i++)
	{
		ID =i;
		updata =100;
		memset(data,11,32);	
		sprintf(sql, "INSERT INTO RecordFile VALUES(%d,%d,? );", ID, updata);

		rc = sqlite3_prepare(db, sql, strlen(sql), &stmt, NULL);
		debug("sql =%s\n",sql);

		if (rc != SQLITE_OK) {
			fprintf(stderr, "sql error:%s\n", sqlite3_errmsg(db));
		}
		sqlite3_bind_blob(stmt, 1, data, 32, NULL);
		sqlite3_step(stmt);
		sqlite3_finalize(stmt);
	}
	sqlite3_close(db);
}

//sqlite3数据库测试
void sqlite3_test(void)
{
	int len,rc;
	int i=0;
	int nrow=0;
	int ncolumn = 0;
	char *zErrMsg =NULL;
	char **azResult=NULL; //二维数组存放结果
	char tempdata[200];
	char *buf;
	char sql[1024];

	struct sRecordStruct sRt;

	sqlite3_stmt *stmt = NULL; // 用来取数据的
	
	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
		/*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
		sqlite3_close(recod_db);
		//exit(1);
	}
	else printf("You have opened a sqlite3 database named user successfully!\n");
		/* 创建未采记录表 */
	//recordIdb 一直累加不清零
	//消费时间年/月/日
	sprintf(sql,"create table consume (id INTEGER,tag INTEGER,money INTEGER,time BLOB,datas BLOB);") ;
	//char *sql = "create table student (id int primary key,tag int,money int,time char);" ;

	debug("sql =%s\n",sql);
	sqlite3_exec(recod_db,sql,NULL,NULL,&zErrMsg);
	
	for(i=0;i<1000;i++)
	{
		len = sqlite3_open("record.db",&recod_db);
		//插入数据
		sRt.recordId =i;
		sRt.CurrentConsumMoney =100;
		sRt.recordTag = 0;
		memset(sRt.ConsumeTime,0x30,6);
		memset(sRt.recoedDatas,0x30,62);

		/*sprintf(tempdata,"insert into consume values (%d,%d,%d,'201202','%s');",sRt.recordId,sRt.recordTag, sRt.CurrentConsumMoney,sRt.recoedDatas);
		debug("%s\n", tempdata);
		sqlite3_exec(recod_db,tempdata,NULL,NULL,&zErrMsg);*/

		sprintf(sql, "INSERT INTO consume VALUES(%d,%d,%d,? ,?);", sRt.recordId,sRt.recordTag, sRt.CurrentConsumMoney);
		debug("sql= %s\n", sql);
		rc = sqlite3_prepare(recod_db, sql, strlen(sql), &stmt, NULL);
		debug("sqlite3_prepare\n");
		if (rc != SQLITE_OK) {
			fprintf(stderr, "sql error:%s\n", sqlite3_errmsg(recod_db));
		}
		sqlite3_bind_blob(stmt, 1, sRt.ConsumeTime, 6, NULL);
		debug("sqlite3_bind_blob\n");
	//	sqlite3_bind_blob(stmt, 2, sRt.recoedDatas, 62, NULL);
		sqlite3_step(stmt);
		debug("sqlite3_step\n");
		sqlite3_finalize(stmt);
		
		sqlite3_close(recod_db);
	}

	

	//消费时间年/月/日
//	 char *sql = "create table student; (id int primary key,name char,age int,sex char);";

//	debug("sql =%s\n",sql);
//	sqlite3_exec(recod_db,sql,NULL,NULL,&zErrMsg);
//	
//	for(i=0;i<1000;i++)
//	{
//		//插入数据
//		sRt.recordId =i;
//		sRt.CurrentConsumMoney =100;
//		sRt.recordTag = 0;
//		memset(sRt.ConsumeTime,0x30,6);
//		memset(sRt.recoedDatas,0x30,64);

//		sprintf(tempdata,"insert into student values (%d,'zhang0',20,'m'); ",i);
//		debug("%s\n", tempdata);
//		sqlite3_exec(recod_db,tempdata,NULL,NULL,&zErrMsg);
//	}



//	/* 查询数据 */
//	char *sql11="select *from ConsumeData";
//	sqlite3_get_table( recod_db , sql11 , &azResult , &nrow , &ncolumn , &zErrMsg );
//	printf("nrow=%d ncolumn=%d\n",nrow,ncolumn);
//	printf("the result is:\n");
//	for(i=0;i<(nrow+1)*ncolumn;i++)
//	{
//		printf("azResult[%d]=%s\n",i,azResult[i]);
//	}

//	debug("删除记录\n");
//	/* 删除某个特定的数据 */
//	char *sql111="delete from SensorData where SensorID = 1 ;";
//	sqlite3_exec( recod_db , sql111 , NULL , NULL , &zErrMsg );
//	#ifdef _DEBUG_
//	printf("zErrMsg = %s \n", zErrMsg);
//	sqlite3_free(zErrMsg);
//	#endif

	/* 查询删除后的数据 */
	char *sq2 = "SELECT * FROM SensorData ";
	sqlite3_get_table( recod_db , sq2 , &azResult , &nrow , &ncolumn , &zErrMsg );
	printf( "row:%d column=%d\n " , nrow , ncolumn );
	printf( "After deleting , the result is : \n" );
	for( i=0 ; i<( nrow + 1 ) * ncolumn ; i++ )
	{
		printf( "azResult[%d] = %s\n", i , azResult[i] );
	}
	sqlite3_free_table(azResult);
	#ifdef _DEBUG_
	printf("zErrMsg = %s \n", zErrMsg);
	sqlite3_free(zErrMsg);
	#endif

	sqlite3_close(recod_db);
	return 0;
}

