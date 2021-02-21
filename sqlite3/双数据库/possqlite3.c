#include <stdio.h>
#include <stdlib.h>
#include "sqlite3.h"
#include "../debug/debug.h"
#include "possqlite3.h"
#include "../ExternVariableDef.h"

static char *zErrMsg =NULL;
static char **azResult=NULL; //二维数组存放结果
static int nrow=0;
static int ncolumn = 0;
sqlite3 *recod_db=NULL;//未采记录数据库句柄
sqlite3 *collectedRecod_db=NULL;//已采记录数据库句柄

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
	
	/* 打开或创建已采记录数据库 */
	len = sqlite3_open("collectedRecod.db",&collectedRecod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(collectedRecod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(collectedRecod_db);
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named collectedRecod successfully!\n");

	/* 创建未采记录表 */
	//recordIdb 一直累加不清零
	 sql = " CREATE TABLE ConsumeData(\
		 recordId INTEGER,\   
		 recordTag INTEGER,\   
		 CurrentConsumMoney INTEGER,\
		 Time VARCHAR(14),\
		 recoedDatas VARCHAR(64)\
	 );" ;

	sqlite3_exec(recod_db,sql,NULL,NULL,&zErrMsg);
	
	/* 创建已采记录表 */
	 char *sql = " CREATE TABLE collecteConsumeData(\
	 	 recordId INTEGER,\   
		 recordTag INTEGER,\   
	 	 CurrentConsumMoney INTEGER,\
		 Time VARCHAR(14),\
	 	 recoedDatas VARCHAR(64)\
		 );" ;

	sqlite3_exec(collectedRecod_db,sql,NULL,NULL,&zErrMsg);
	
	sqlite3_close(recod_db);
	sqlite3_close(collectedRecod_db);
	return len;
}

//插入数据
int sqlite3_consume_insert_db(struct sRecordStruct sRt)
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
	   return len;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named user successfully!\n");

	/*插入数据	*/
	debug("sRt.CurrentConsumMoney==%d\n",sRt.CurrentConsumMoney);
	sprintf(tempdata,"INSERT INTO 'ConsumeData'VALUES(%d,%d,%d,'%s','%s');",sRt.recordId,sRt.recordTag ,sRt.CurrentConsumMoney,sRt.ConsumeTime,sRt.recoedDatas);
	debug("tempdata= %s\n", tempdata);
	sqlite3_exec(recod_db,tempdata,NULL,NULL,&zErrMsg);
	
//	sqlite3_consume_query_record_db(3);
	sqlite3_consume_close_db(recod_db);
}

//查询交易记录
struct sRecordStruct sqlite3_consume_query_record_db( uint8_t number)
{
	int i,len,tagId,recordTag;
	char buffer[100];
	struct sRecordStruct sRt;
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
	char *sql="select *from ConsumeData where recordTag = 0";
	if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
	{	
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{
			// 取出第0列字段的值
			sRt.recordId = sqlite3_column_int(stmt, 0);
			// 取出第1列字段的值
			sRt.recordTag = sqlite3_column_int(stmt, 1);
			// 取出第2列字段的值
			sRt.CurrentConsumMoney = sqlite3_column_int(stmt, 2);
			// 取出第4列字段的值
			sRt.recoedDatas = sqlite3_column_text(stmt, 4);
			debug("recordId = %d; recordTga = %d; CurrentConsumMoney = %d, recordDatas = %s \r\n",sRt.recordId,sRt.recordTag,sRt.CurrentConsumMoney,sRt.recoedDatas);			
			break;
		}
	}
	else
	{
		debug("elect *from ConsumeData where recordTag > 0 err \r\n");
	}
  sqlite3_close_db(recod_db);  
  return sRt;
}

//从未采记录数据库中把已采记录移动到已采数据库
int sqlite3_consume_move_db(int index)
{
	char tempdata[200];
	int len;
	struct sRecordStruct sRt;
	sqlite3_stmt *stmt = NULL; // 用来取数据的
	
	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	//根据ID号从数据库中查找到要删除的记录
	sprintf(tempdata,"select *from ConsumeData where recordId = %d;",index);
	if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
	{	
		while(sqlite3_step(stmt) == SQLITE_ROW ) 
		{
			// 取出第0列字段的值
			sRt.recordId = sqlite3_column_int(stmt, 0);
			// 取出第1列字段的值
			sRt.recordTag = sqlite3_column_int(stmt, 1);
			// 取出第2列字段的值
			sRt.CurrentConsumMoney = sqlite3_column_int(stmt, 2);
			// 取出第3列字段的值
			sRt.Time = sqlite3_column_int(stmt, 3);
			// 取出第4列字段的值
			sRt.recoedDatas = sqlite3_column_text(stmt, 4);
			debug("recordId = %d; recordTga = %d; CurrentConsumMoney = %d, Time = %s, recordDatas = %s \r\n",sRt.recordId, sRt.recordTag, sRt.CurrentConsumMoney, sRt.Time, sRt.recoedDatas);			
			break;
		}
	}
	//删除本条记录
	sprintf(tempdata,"delete from ConsumeData where recordId = %d ;",index);
	debug("tempdata= %s\n", tempdata);
	sqlite3_exec( recod_db , tempdata , NULL , NULL , &zErrMsg );
	sqlite3_close(collectedRecod_db);
	
	/*把从未采数据库中已删除的记录添加到已采数据库中*/
	/* 打开数据库 */
	len = sqlite3_open("collectedRecod.db",&collectedRecod_db);
	sprintf(tempdata,"INSERT INTO 'collecteConsumeData'VALUES(%d,%d,%d,'%s','%s');",sRt.recordId, sRt.recordTag, sRt.CurrentConsumMoney, sRt.ConsumeTime, sRt.recoedDatas);
	debug("tempdata= %s\n", tempdata);
	sqlite3_exec(collectedRecod_db,tempdata,NULL,NULL,&zErrMsg);
	sqlite3_close_db(collectedRecod_db);
}

/*查询未采集记录初始记录的id=NoCollectRecordIndex
bit = 0 查找记录初始ID
bit = 1 查找记录最后的ID
*************************************************/
uint32_t  sqlite3_consume_query_NoCollectRecordIndex_db(uint8_t bit)
{
	int index,len;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

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
	char *sql="select *from ConsumeData where recordId ";
	if (sqlite3_prepare_v2(recod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
	{   
	  while(sqlite3_step(stmt) == SQLITE_ROW ) 
	  {	
		// 取出第0列字段的值ID号
		index= sqlite3_column_int(stmt, 0);
		if(!bit)
			break;
	  }
	}
	sqlite3_close_db(recod_db);
   	return index;
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
//   /* 查询数据 */
//	  char *sql="select *from ConsumeData where CurrentConsumMoney ";
//	  sqlite3_get_table( recod_db , sql , &azResult , &nrow , &ncolumn , &zErrMsg );
//	  printf("nrow=%d ncolumn=%d\n",nrow,ncolumn);
//	  printf("the result is:\n");
//    
	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	char *sql="select *from ConsumeData where CurrentConsumMoney ";
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
	sqlite3_close_db(recod_db);
   	return RecordStr;
}

//查询已采消费总额跟消费笔数
struct	sRecordMoneyStruct sqlite3_consume_query_collectedConsumemoney_db(void)
{
	int i,len;
	struct	sRecordMoneyStruct RecordStr;
	sqlite3_stmt *stmt = NULL; // 用来取数据的

	/* 打开数据库 */
	len = sqlite3_open("collectedRecod.db",&collectedRecod_db);
	if( len )
	{
	   /*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
	   fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(collectedRecod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
	   sqlite3_close(collectedRecod_db);
	   RecordStr.RecordToalMoney =0;
	   RecordStr.RecordToalNumber =0;
	   RecordStr.status = 0;//查询失败
	   return RecordStr;
	  // exit(1);
	}
	else 
		printf("You have opened a sqlite3 database named collectedRecod successfully!\n");

	// 每调一次sqlite3_step()函数，stmt就会指向下一条记录
	// -1代表系统会自动计算SQL语句的长度
	char *sql="select *from collecteConsumeData where CurrentConsumMoney ";
	RecordStr.RecordToalMoney = 0;
	RecordStr.RecordToalNumber = 0;
	if (sqlite3_prepare_v2(collectedRecod_db, sql, -1, &stmt, NULL) == SQLITE_OK) 
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
	sqlite3_close_db(collectedRecod_db);
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
	sprintf(tempdata,"delete from ConsumeData where CONSUMEMONEY = %d and Time = %s;",sRt.CurrentConsumMoney,sRt.ConsumeTime);

	debug("tempdata= %s\n", tempdata);
	sqlite3_exec( recod_db , tempdata , NULL , NULL , &zErrMsg );
	sqlite3_close(recod_db);
}

//关闭数据库
void sqlite3_consume_close_db(void)
{
//	sqlite3_free_table(azResult);
#ifdef _DEBUG_
//	printf("zErrMsg = %s \n", zErrMsg);
//	sqlite3_free(zErrMsg);
#endif
	 
	sqlite3_close(recod_db);
}

//sqlite3数据库测试
void sqlite3_test(void)
{
	int len;
	int i=0;
	int nrow=0;
	int ncolumn = 0;
	char *zErrMsg =NULL;
	char **azResult=NULL; //二维数组存放结果
	/* 打开数据库 */
	len = sqlite3_open("record.db",&recod_db);
	if( len )
	{
		/*  fprintf函数格式化输出错误信息到指定的stderr文件流中  */
		fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(recod_db));//sqlite3_errmsg(db)用以获得数据库打开错误码的英文描述。
		sqlite3_close(recod_db);
		exit(1);
	}
	else printf("You have opened a sqlite3 database named user successfully!\n");

	//	 /* 创建表 */
	//	 char *sql = " CREATE TABLE ConsumeData(\
	//	 	 recordTag INTEGER,\   
	//	 	 CurrentConsumMoney INTEGER,\
	//		 Time VARCHAR(14),\
	//		 recoedDatas VARCHAR(64)\
	//		 );" ;
	// //	 	 
	//      sqlite3_exec(db,sql,NULL,NULL,&zErrMsg);
	//#ifdef _DEBUG_
	//      printf("%s\n",zErrMsg);
	//      sqlite3_free(zErrMsg);
	//#endif
	//      /*插入数据  */
	//      char*sql1 ="INSERT INTO 'ConsumeData'VALUES(100,1000,'20151105063021','2015110506302112345678901234567820151105063021123456789012345678');";
	//      sqlite3_exec(db,sql1,NULL,NULL,&zErrMsg);
	//     /* char*sql2 ="INSERT INTO 'SensorData'VALUES(NULL,3,4,201530506302,14.5);";
	//      sqlite3_exec(db,sql2,NULL,NULL,&zErrMsg);
	//      char*sql3 ="INSERT INTO 'SensorData'VALUES(NULL,5,6,201630506413,18.6);";
	//      sqlite3_exec(db,sql3,NULL,NULL,&zErrMsg);*/

	/* 查询数据 */
	char *sql11="select *from ConsumeData";
	sqlite3_get_table( recod_db , sql11 , &azResult , &nrow , &ncolumn , &zErrMsg );
	printf("nrow=%d ncolumn=%d\n",nrow,ncolumn);
	printf("the result is:\n");
	for(i=0;i<(nrow+1)*ncolumn;i++)
	{
		printf("azResult[%d]=%s\n",i,azResult[i]);
	}

	debug("删除记录\n");
	/* 删除某个特定的数据 */
	char *sql111="delete from SensorData where SensorID = 1 ;";
	sqlite3_exec( recod_db , sql111 , NULL , NULL , &zErrMsg );
	#ifdef _DEBUG_
	printf("zErrMsg = %s \n", zErrMsg);
	sqlite3_free(zErrMsg);
	#endif

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

