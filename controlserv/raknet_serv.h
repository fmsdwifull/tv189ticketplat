
#ifndef _SERVER_RAKNET_H
#define _SERVER_RAKNET_H


#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include "BitStream.h"
#include <stdlib.h> // For atoi
#include <cstring> // For strlen
#include "Rand.h"
#include "RakNetStatistics.h"
#include "MessageIdentifiers.h"
#include <stdio.h>
#include <vector>
#include <pthread.h>


#ifndef  _H_Kbhit
#define  _H_Kbhit
	#include "Kbhit.h"
#endif

#include "GetTime.h"
#include "RakAssert.h"
#include "RakSleep.h"
#include "Getche.h"
#include "dbop.h"


#include "mysqlop.h"

#include "common.h"

#include <unistd.h> // usleep
#include <cstdio>

#include <iostream>


#include <list>   
#include <string.h>
#include "pure_log.h"
#include "config.h"



using namespace std;  
  
static const int NUM_CLIENTS=64;


//#define ID_USRMSG_BEGIN = ID_USER_PACKET_ENUM,

//MEDIAL  

#ifndef  _ENUM_USER_TYPE
#define  _ENUM_USER_TYPE
enum UserMessageIDTypes
{

		ID_USRMSG_BEGIN = ID_USER_PACKET_ENUM,
		ID_M2C_JOIN,
		ID_C2M_JOIN_RECEIVED,
		ID_M2C_LEAVE,
		ID_C2M_LEAVE_RECEIVED,
		ID_L2C_JOIN,
		ID_C2L_JOIN_RECEIVED,
		ID_L2C_LEAVE,
		ID_C2L_LEAVE_RECEIVED,
		ID_C2M_STARTCUTLOG,
		ID_M2C_ENDCUTLOD,
		ID_M2C_NEWLOGCREATED,
		ID_C2M_NEWLOGCREATED_RECEIVED,
		ID_C2L_TASKASSIGN,
		ID_L2C_TASKASSIGN_RECEIVED,
		ID_L2C_TASKFINISHED,
		ID_C2L_TASKFINISHEDR_RECEIVED,
		ID_C2L_UPLOADTICKET,
		ID_L2C_UPLOADTICKET_FINISHED,


};

#endif 

#ifndef __DATABASE_H_
#define __DATABASE_H_ 

typedef struct {
	 int iStamp;
	 int iUploadNumber;
	 int iSequence;
	 int iLogFilePathLength;
	 char szLogFilePath[SEG_PATH_LENGTH];
}media_data;
#endif

#ifndef ENUM_H
#define ENUM_H
typedef enum 
{
	HLS_VOD,
	HLS_LIVE,
	HTTP_VOD,
	RTSP_LIVE,
	RTSP_VOD,
	RTSP_TIMESHIFT,
	LOG_NULL,
}MediaSvr_Type;
#endif

typedef enum
{
	MEDIA_S,
	LOG_S,
}Server_Type;

typedef struct 
{
	int iStamp;
}M2C_Leave;

typedef struct  
{
	unsigned int iStamp;
	unsigned int iUploadNumber;
	unsigned int iSequence;
	unsigned char szDate[DATE_SIZE];
	unsigned char szLogFilePath[128];
}M2C_NewLogCreated;

typedef struct{
	 unsigned int fromip;
	 unsigned short fromport;
	 int fromindex;
	 int ServerIndex;
	 int ServerType;//应该是联合还是枚举，忘了
	 MediaSvr_Type ServerSubType;
}MLlist;

//media server join  
typedef struct {
	int iStamp;
	MediaSvr_Type type;
}M2C_Join;
//
typedef struct{
	unsigned int MediaServerTye;

	unsigned int MediaServerIP;
	unsigned int UploadSeq;
	unsigned int Seq;
	unsigned int Spec;
	unsigned char LogPath[100];
	unsigned int Status;
	unsigned int WorkServerIP;
	unsigned int AssignTime;
	unsigned char szDate[DATE_SIZE];
	
}M2C_dbase;

//ID_C2M_STARTCUTLOG
typedef struct 
{
	int iStamp;
	char szDate[DATE_SIZE];
	int iUploadNumber;
}M2C_StartCutLog;

//ID_C2M_ENDCUTLOD
typedef struct 
{
	int iStamp;
	char szDate[DATE_SIZE];
	int iUploadNumber;
}M2C_EndCutLog;

typedef struct 
{
	int iStamp;
	char szDate[DATE_SIZE];
	int iUploadNumber;
}C2M_StartCutLog;



//L2C
typedef struct 
{
	int iStamp;
}L2C_Join;

typedef struct 
{
	int iStamp;
}L2C_Leave;

typedef struct 
{
	int iStamp;
}L2C_JoinReceived;

//msg struct 
#ifndef  _MSG_S_
#define  _MSG_S_
typedef struct	
{
	long mtype;
	unsigned char buffer[DB_BUF_SIZE];
	//unsigned char *buffer;

}msgtype_s;
#endif

typedef struct  
{
	long mtype;
	char buffer[DB_BUF_SIZE];
	//char *buffer;
}msgtype_r;


//snddata struct
#ifndef _ST_H_
#define _ST_H_
typedef struct 
{
	MediaSvr_Type type;
	char szIP[16];
	int iWorkID;
	int iUploadNumber;
	int iSequence;
	int spec;
	char szLogFilePath[128];
	char szDate[DATE_SIZE];
	int  DataIsBad;
}St_Log;

#endif


//send log server 
typedef struct 
{
	int iStamp;
	int iTaskType; //0:HLS,HTTP //1: RTSP
	int iNewLogCount;
	vector<St_Log>  vecNewLog;
}C2L_TaskAssign;

struct msgtype_s 
{
	long mtype;
	char buffer[TMP_BUF_SIZE];
};



//list for store data of media infomation
typedef struct
{
	unsigned long media_ip;
	unsigned short media_port;
}As_Media_List;
 
// Just listens for ID_USER_PACKET_ENUM and validates its integrity
class Server
{
	public:
		Server();
		~Server();
		void Start(void);
		void setpeer();
		RakPeerInterface* getpeer();
		list<MLlist>* getlist();
		list<As_Media_List> * getcut_m_list();

		
		mysqlop* getsql();
		unsigned ConnectionCount(void) const;
		void Update();
		int  Fileparase(char *src,char *filename,char *dirname);
		void lostconn(int tmp_ip,unsigned short tmp_port);
		void uerenum();
		
		void clear_m_list();
		
		void l2cjoin(Packet *p,int logser);		
		void l2cleave(int tmp_ip,unsigned short tmp_port);
		void m2cjoin(Packet *p,int media) ;	
		void m2cleave(int tmp_ip,unsigned short tmp_port);
		void m2cnewlog(Packet *p);
		void l2ctask(Packet *p);
		
		void endcutlod(Packet *p);
		
		int  getlogslist();
		void lslogslist();
		int  getmediaslist();
		void lsmediaslist();
		char* GetTimeStr();

		char* ip2str( unsigned int intip);
		
		void clear_xml();
		
		void updata_xml();
		
		void insertmedia_nd(TiXmlElement* fromnode,char *popnodename,char *vip,char *vstatus);
		
		void insertlog_nd(TiXmlElement* fromnode,char *popnodename,char *vip,char *vstatus,char *vworkqueu);
		
		void insertsig_nd(TiXmlElement* fromnode,char *popnodename,char *value);
		//int travl_list();

		
		
	private:
		media_data *_media_data;
		Dboperatong deal_db;//file operator object		
		mysqlop  *sql;
		RakPeerInterface *peer;
		list<MLlist>  *ML_list; 	
		list<As_Media_List> *As_M;
		pthread_mutex_t lock;


};

#endif
