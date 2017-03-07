#include "mysqlop.h"
#include <string.h>

/*******************stuctrue ************************/ 
mysqlop::mysqlop()
{
	 pthread_mutex_init(&lock,NULL);
	 conn_db=mysql_init(NULL);
	 if(!conn_db)
	 {
		 fprintf(stderr, "mysql_init_failed/n");
	 }
	 conn_db = mysql_real_connect(conn_db,"192.168.19.223","root","123456","test",0,NULL,0);
	 if( conn_db ){
		 printf("Connection success\n");
	 }else{
		printf("Error connecting to database: %s\n", mysql_error(conn_db));
	 }	 

}

/******************* free  stuctrue ************************/ 
mysqlop::~mysqlop()
{
	mysql_close(conn_db);
}

/************************sql exec*****************************/
int mysqlop::sqlexec(char *sqlstr,int type, char *result)
{
	pthread_mutex_lock(&lock);
	int tmp_num=0;
	switch (type)
	{
		case SQL_EXIC_CMD:
			exec_res(sqlstr);
			break;	
		case SQL_REQUIRE_DATA:
			tmp_num=query_res(sqlstr,result);
			break;
		case SQL_CHENDK_NUM:
			tmp_num=query_num(sqlstr);
			break;
	}
	
	pthread_mutex_unlock(&lock);	
	
	return tmp_num;
}

/************************get res*****************************/
void mysqlop::exec_res(char *sqlstr)
{
	int ok;
	ok=mysql_query(conn_db,sqlstr);
	if(ok)
	{
		printf("Error making query:%s\n", mysql_error(conn_db));
		exit(-1);
	}
	//else printf("Query made... \n");
}

/************************query res*****************************/
int mysqlop::query_res(char *sqlstr, char *result)
{
	int tmp_num=0;
	int ok;
	St_Log_RE *st=new St_Log_RE;

	ok=mysql_query(conn_db,sqlstr);
	if(ok)
	{
		printf("Error making query:%s\n", mysql_error(conn_db));
		exit(-1);
	}
	res=mysql_use_result(conn_db);
	
	unsigned int num_fields;
	unsigned int i;

	num_fields = mysql_num_fields(res);

	while ((row = mysql_fetch_row(res)))
	{
	   unsigned long *lengths;
	   lengths = mysql_fetch_lengths(res);
	   for(i = 0; i < num_fields; i++)
	   {
	   		//st->type_r=HLS_VOD;//注意这个
	   		st->type_r=(MediaSvr_Type)atoi(row[1]);//注意这个
	   		strcpy(st->szIP_r,row[2]);
			st->iWorkID_r=atoi(row[0]);
			st->iUploadNumber_r=atoi(row[3]);
			st->iSequence_r=atoi(row[4]);
			st->spec_r=atoi(row[5]);
			strcpy(st->szLogFilePath_r,row[6]);
			st->DataIsBad=0;
#if 0
			st->type_r=HLS_VOD;//注意这个			
			st->iWorkID_r=atoi(row[0]);
			st->iUploadNumber_r=atoi(row[2]);
			st->iSequence_r=atoi(row[3]);
			st->spec_r=atoi(row[4]);
			strcpy(st->szLogFilePath_r,row[5]);
			st->DataIsBad=0;
#endif
	   }	   
	   memcpy(result+tmp_num*sizeof(St_Log_RE),( char *)st,sizeof(St_Log_RE));
	   tmp_num++;
	}
	mysql_free_result(res);
	delete st;
	return tmp_num;
}


/************************get the num*****************************/
int mysqlop::query_num(char *sqlstr)
{
	int ok;
	ok=mysql_query(conn_db,sqlstr);
	if(ok)
	{
		printf("Error making query:%s\n", mysql_error(conn_db));
		exit(-1);
	}

	res=mysql_use_result(conn_db);
	
	unsigned int num_fields;
	unsigned int i;

	num_fields = mysql_num_fields(res);

	int tmp_num=0;
	while ((row = mysql_fetch_row(res)))
	{
		tmp_num++;
	}
	mysql_free_result(res);
	//printf("the num is :%d\n",tmp_num); 
	return tmp_num;
}

