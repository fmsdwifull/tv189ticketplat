#include <stdio.h>
#include <pthread.h>
#include <mysql/mysql.h>
#include <string.h>
#include <stdlib.h>
#include <iostream>

#include "common.h"


typedef enum 
{
	SQL_EXIC_CMD,
	SQL_CHENDK_NUM,
	SQL_REQUIRE_DATA,
	
}Opty;


#ifndef ENUM_H
#define ENUM_H
typedef enum 
{
	HLS_VOD=1,
	HLS_LIVE,
	HTTP_VOD,
	RTSP_LIVE,
	RTSP_VOD,
	RTSP_TIMESHIFT,
	LOG_NULL,
}MediaSvr_Type;
#endif


typedef struct 
{
	MediaSvr_Type type_r;
	char szIP_r[16];
	int iWorkID_r;//key in db
	int iUploadNumber_r;
	int iSequence_r;
	int spec_r;
	char szLogFilePath_r[128];
	char szDate[DATE_SIZE];
	int  DataIsBad;
}St_Log_RE;






class  mysqlop
{
	public:
		mysqlop();
		~mysqlop();
	    int sqlexec(char *sqlstr,int type, char *result);
		
		void exec_res(char *sqlstr);
		int query_res(char *sqlstr, char *result);		
		int query_num(char *sqlstr);
		
	private:
		MYSQL *conn_db;
		MYSQL_RES *res;
		MYSQL_ROW row;
		pthread_mutex_t lock;

};

