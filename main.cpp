#include <pthread.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#include <string.h> 
#include <stdlib.h>
#include <dirent.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include <sys/stat.h>
#include <vector>
#include <semaphore.h>

#include "raknet_serv.h"
#include "raknet_client.h"
#include "task_queue.h"
#include "test.h"
#include <iostream>
#include <string.h>
#include "common.h"
#include "pure_log.h"
#include "systimer.h"

unsigned int  logdst_ip;
unsigned int  logdst_port;
unsigned int  logdst_index;

Server server;
int m_count=0;
int iUp_NUM=0;


bool media_cut_over;


p_base::CLog logi;
CConfig* conf = NULL;

bool isupload=false;



char * getyesday(void)
{
    time_t now=0;
    struct tm *tm_now;
    static char  datetime[TMP_BUF_SIZE];
 
    time(&now);
	now=now-24*60*60;
    tm_now = localtime(&now);
    strftime(datetime, 200, "%Y%m%d\0\n", tm_now); 
	return datetime;
}


/******************get the time by day with seconds standadly*******************/
time_t GetTimeD(void)
{
	//time_t today_day;
	struct tm *ptr;
	
	time_t lt;
	lt =time(NULL);
	ptr=localtime(&lt);
	return lt-(ptr->tm_hour)*3600-ptr->tm_min*60-ptr->tm_sec;
}

/******************get the time by day with seconds standadly*******************/
char* GetTimeStr(void)
{
    time_t now;
    static char timebuf[TMP_BUF_SIZE]={0};
    struct tm *tm_now;
 
    time(&now);
    tm_now = localtime(&now);
 
    //sprintf(timebuf,"%d-%d-%d %d:%d:%d\n",tm_now->tm_year+1900,tm_now->tm_mon, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
    sprintf(timebuf,"%d:%d:%d\n",tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec);
  
    //printf("%s\n",timebuf);
    
 
    return timebuf;
}


char* getstrtime(time_t secs)
{
	time_t timep=secs;
	static char timebuf[TMP_BUF_SIZE];
	struct tm *p;
	p=localtime(&timep); /*取得当地时间*/
	sprintf (timebuf,"%d-%d-%d %d:%d:%d", p->tm_year+1900,p->tm_mon+1, p->tm_mday,p->tm_hour, p->tm_min, p->tm_sec);
	return timebuf;
}

time_t get_stdday_time(void)
{
	struct tm *ptr;
	time_t now_time,std_time;
	now_time =time(NULL);
	ptr=localtime(&now_time);
	
	ptr->tm_sec=0;
	ptr->tm_min=0;
	ptr->tm_hour=0;

	std_time = mktime(ptr);
	return std_time;
}

time_t StrToSec( char *strtm)
{
	char *str=NULL;
	int  hour=0,min=0,sec=0;
    if((str=strtok(strtm,":"))!=NULL)
	    hour=atoi(str);
	if((str=strtok(NULL,":"))!=NULL)
	    min=atoi(str);
    if((str=strtok(NULL,":"))!=NULL)
	    sec=atoi(str);
	return hour*60*60+min*60+sec;
}


time_t GetTimeSec(void)
{
    time_t now;
    time(&now);
	return  now;
}

int getoklogs(unsigned int *log_ip,unsigned int *log_port,unsigned int *log_index)
{		
		mysqlop  *mysql=server.getsql();		
		list<MLlist> *mylist=server.getlist();

		int logser_num=0;
		logser_num=server.getlogslist();
		logi.prt(p_base::LOG_TERM,"The nums of loger  is :%d\n",logser_num);
		server.lslogslist();
		
		int medisaser_num=0;
		medisaser_num=server.getmediaslist();
		logi.prt(p_base::LOG_TERM,"The nums of media  is :%d\n",medisaser_num);
		server.lsmediaslist();
				

		int minnum=0;
		int tmp_num=0;

		//find list
		list<MLlist>::iterator iter_new;
		list<MLlist>::iterator iter;
			
		logi.prt(p_base::LOG_INFO,"list size is:%d\n",mylist->size());

		for(iter= mylist->begin(); iter != mylist->end();iter++ )
		{
			if(iter->ServerType==LOG_S)
			{
				iter_new=iter;
				break;
			}
				
		}	
            //get the num which in db by condition
		char datastr[TMP_BUF_SIZE];
		sprintf(datastr,"select * from ticklog where WorkServerIP ='%s' and  Status=1",server.ip2str(iter->fromip));	
		tmp_num=mysql->sqlexec(datastr,SQL_CHENDK_NUM,NULL);

		if(tmp_num==0)
		{
			*log_ip=iter->fromip;	//logdst_ip 代表最终的目的log ip;
			*log_port=iter->fromport;
			*log_index=iter->ServerIndex;
			return 0;
		}else{
			minnum=tmp_num;
			*log_ip=iter->fromip;
			*log_port=iter->fromport;
			*log_index=iter->ServerIndex;	
			
		}			
			
	   for(; iter_new != mylist->end(); )  		
       { 
            list <MLlist> ::iterator t = ++iter_new;  
            if( t->ServerType==1)//如果是1表示是logserver  
		    {  
				char datastr[TMP_BUF_SIZE];
				sprintf(datastr,"select * from ticklog where WorkServerIP=%u and Status=1",iter_new->fromip);
				tmp_num=mysql->sqlexec(datastr,SQL_CHENDK_NUM,NULL);

				if(tmp_num==0)
				{	
					*log_ip=iter_new->fromip;	//logdst_ip 代表最终的目的log ip;						
					*log_port=iter_new->fromport;
					*log_index=iter_new->ServerIndex;
					return 0;
				}else if(tmp_num<minnum)
				{
						
					minnum=tmp_num;
					*log_ip=iter_new->fromip;
					*log_port=iter_new->fromport;
					*log_index=iter_new->ServerIndex;
				}
		      }  
			
	  }

	return 0;
}


/************************mount db******************************/
void nfs_mount(void)
{
	//不恩删
	/*
	char *cmd_mount="mount 192.168.19.223:/home/tv189 /home/tv189";
	system(cmd_mount);

	if(!opendir("/home/tv189"))
	{
		perror("mount error");
		exit(EXIT_FAILURE);
	}
	*/
}

/************************send data to log server*****************/
bool snd2logs(RakPeerInterface* mypeer,  char *sndbuf,int len,unsigned int dig_ip,unsigned int port,unsigned int index)
{
	Client client;
	SystemAddress addr(dig_ip,port);
	addr.systemIndex=index;

	St_Log st_test;
	if (len<1)
	{
		logi.prt(p_base::LOG_INFO,"the length of data ,will send logser   :%d\n",len);
		//exit(-1);
	}

	// for test print 
	for(int i=0;i<len;i++)
	{
		memcpy(&st_test,sndbuf+sizeof(int)*2+i*sizeof(St_Log),sizeof(St_Log));
		logi.prt(p_base::LOG_INFO,"send to log type:%d\n,send to log media ip:%s\n,send to log iWorkID:%d\n,send to log iUploadNumber:%d\n,send to log iSequence:%d\n, send to log spec:%d\n,send to log szLogFilePath:%s\n",st_test.type,st_test.szIP,st_test.iWorkID,st_test.iUploadNumber,st_test.iSequence,st_test.spec,st_test.szLogFilePath);
		cout<<"send to log type---"<<st_test.type<<endl;
		cout<<"send to log media ip---"<<st_test.szIP <<endl;
		cout<<"send to log iWorkID---"<<st_test.iWorkID<<endl;
		cout<<"send to log iUploadNumber---"<<st_test.iUploadNumber<<endl;
		cout<<"send to log iSequence---"<<st_test.iSequence<<endl;
		cout<<"send to log spec---"<<st_test.spec<<endl;
		cout<<"send to log szLogFilePath---"<<st_test.szLogFilePath<<endl;
		cout<<"send to log data is bad "<<st_test.DataIsBad<<endl;
		memset(&st_test,0,sizeof(St_Log));
	}
	
	if(mypeer->IsConnected(addr)==true)
	{
		//send data
		logi.prt(p_base::LOG_TERM,"send ............................:%s\n",sndbuf+sizeof(int));
		client.SendPacket(mypeer,ID_C2L_TASKASSIGN, addr, sndbuf, len*sizeof(St_Log)+2*sizeof(int));
		return true;
	}else{
		logi.prt(p_base::LOG_INFO,"not connet ............................\n");
		return false;
	}
}



/************************send data to log server ,let it upload *****************/
bool snd2upload(RakPeerInterface* mypeer,  char *sndbuf,int len,unsigned int dig_ip,unsigned int port,unsigned int index)
{
	Client client;
	SystemAddress addr(dig_ip,port);
	addr.systemIndex=index;

	if (len<1)
	{
		logi.prt(p_base::LOG_INFO,"the length of data ,will send logser   :%d\n",len);
		//exit(-1);
	}
	
	if(mypeer->IsConnected(addr)==true)
	{
		//send data
		logi.prt(p_base::LOG_TERM,"send ............................:%s\n",sndbuf+sizeof(int));
		client.SendPacket(mypeer,ID_C2L_UPLOADTICKET, addr, sndbuf, len);
		return true;
	}else{
		logi.prt(p_base::LOG_INFO,"not connet ............................\n");
		return false;
	}
}


/********************updata  db after sending data to log*****************/
void updatadb(unsigned int log_ip,char *buf,int num)
{
	char datastr[256]={0};
	//char* tmptime=NULL;
	St_Log log_tmp;

	mysqlop  *mysql=server.getsql();
    if(log_ip!=0)
		for(int i=0;i<num;i++)
		{
			memcpy(&log_tmp,buf+2*sizeof(int)+i*sizeof(St_Log),sizeof(St_Log));
			cout<<"iWorkID  "<<log_tmp.iWorkID << endl;	
			//cout<<"-------------------------WorkServerIP  "<< log_ip<< endl;		
	
			//sprintf(datastr,"update ticklog set Status=1 where WorkServerIP=%u",log_tmp.iWorkID);
			sprintf(datastr,"update ticklog set status=1,WorkServerIP='%s',AssignTime=%u,AssignTime_2='%s' where Workid=%d",server.ip2str(log_ip),GetTimeSec(),GetTimeStr(),log_tmp.iWorkID);
			logi.prt(p_base::LOG_TERM,"---------%s",datastr);
			
			mysql->sqlexec(datastr,SQL_EXIC_CMD,NULL);
			usleep(100);
		}
	mysql=NULL;
}


/********************send msg  betw pthread*****************/
void sndnet(char *sendbuf ,int len)
{
	int msgid;
	struct msgtype_s 
	{
		long mtype;
		char buffer[DB_BUF_SIZE];
	};
	struct msgtype_s msg;

	strncpy(msg.buffer,sendbuf,len);
	if((msgid=msgget(1022,	 IPC_CREAT|0666))==-1)
	{
		fprintf(stderr,"Creat Message  Error:%s\n", strerror(errno));
		exit(1);
	}
	msg.mtype = 1;
	msgsnd(msgid, &msg, sizeof(msg.buffer), IPC_NOWAIT); 
	memset(&msg, 0, sizeof(struct msgtype_s));

}


/***************deal raknet data from media server and logserve *********************/
void  *send_log_func(void *)
{   
	//list<MLlist> *mylist=server.getlist();
	RakPeerInterface *mypeer=server.getpeer();
	mysqlop  *mysql=server.getsql();
	C2L_TaskAssign c2l_task;
	//int log_count=0;

	char *sqlbuf=(char *)malloc(DB_BUF_SIZE);
	if(sqlbuf==NULL)
	{
		logi.prt(p_base::LOG_INFO,"sqlbuf malloc error \n");
		exit(-1);
	}
	memset(sqlbuf,0,DB_BUF_SIZE);

	char *send_log_buf=(char *)malloc(DB_BUF_SIZE);
	if(send_log_buf==NULL)
	{
		logi.prt(p_base::LOG_INFO,"send_log_buf malloc error \n");
		exit(-1);
	}	
	memset(send_log_buf,0,DB_BUF_SIZE);

    //time deal
	while (1)
	{	
		//-1
		int tmp_num=-1;
		tmp_num=mysql->sqlexec((char *)"select *  from ticklog where Status=0 and Spec=-1",SQL_REQUIRE_DATA,sqlbuf);
		c2l_task.iStamp=0;
		c2l_task.iNewLogCount=1;

		int logser_num=0;
		logser_num=server.getlogslist();
		logi.prt(p_base::LOG_TERM,"The nums of loger  is :%d\n",logser_num);
		//server.lslogslist();
		
		if((logser_num=!0)&&(tmp_num!=0))
		{
			for(int i=0;i<tmp_num;i++)
			{
				
				St_Log st_test;
			
				memcpy(send_log_buf,&c2l_task.iStamp,sizeof(int));
				memcpy(send_log_buf+sizeof(int)*2,&c2l_task.iNewLogCount,sizeof(int));			
				memcpy(send_log_buf+sizeof(int)*3,sqlbuf+i*sizeof(St_Log),sizeof(St_Log));

				
				memcpy(&st_test,send_log_buf+sizeof(int)*3,sizeof(St_Log));
				if((st_test.type >=1)&&(st_test.type<=3))
				{
					c2l_task.iTaskType=0;
					logi.prt(p_base::LOG_TERM,"query  type is hls--------");
					
				}else if((st_test.type >=4)&&(st_test.type<=6))
				{
					c2l_task.iTaskType=1;					
					logi.prt(p_base::LOG_TERM,"query  type is rstp--------");
				}else
				{
					logi.prt(p_base::LOG_TERM,"query error");
				}
				memcpy(send_log_buf+sizeof(int),&c2l_task.iTaskType,sizeof(int));
								
				unsigned int logip=0;
				unsigned int logport=0;
				unsigned int logindex=0;
							
				if(getoklogs(&logip,&logport,&logindex)!=0)
				{
		
					logi.prt(p_base::LOG_TERM,"get	log server ip error \n");
				}
								
				bool ret=false; 		
				logi.prt(p_base::LOG_TERM,"start send--------------------------:%u--%d\n",logip,tmp_num);
				if(logip!=0)
					ret=snd2logs(mypeer,send_log_buf,1,logip,logport,logindex);
				if(ret)
				{
					printf("start updata--------------------------------:%d\n",tmp_num);
					updatadb(logip,send_log_buf,tmp_num);
				}
				memset(send_log_buf,0,DB_BUF_SIZE);
				sleep(1);
			}
		    memset(sqlbuf,0,DB_BUF_SIZE);
		}



		int tmp_count=-1;
		tmp_count=mysql->sqlexec((char *)"select *	from ticklog where	Spec=-1 and (Status=1 or Status=0 ) ",SQL_CHENDK_NUM,NULL);
		logi.prt(p_base::LOG_TERM,"tmp_count*************:%d---%d\n",tmp_count,media_cut_over);
		

		//not -1		
		if((tmp_count==0)&&(media_cut_over==true))
		{
			char sqlcmd[SQLCMD]={0};
			media_cut_over=false;
			for(int i=0;i<10;i++)
			{	
				tmp_num=0;

				memset(sqlbuf,0,DB_BUF_SIZE);
				sprintf(sqlcmd,"select *  from ticklog where Status=0 and Spec=%d",i);
				tmp_num=mysql->sqlexec(sqlcmd,SQL_REQUIRE_DATA,sqlbuf);

				
				if(tmp_num!=0)
				{
					logi.prt(p_base::LOG_TERM,"tmp_num*******************************  %d ",tmp_num);
					if(tmp_num==1)
						continue;
					c2l_task.iStamp=0;
					c2l_task.iTaskType=0;//0:HLS,HTTP //1: RTSP
					c2l_task.iNewLogCount=tmp_num;
		
					memcpy(send_log_buf,&c2l_task.iStamp,sizeof(int));	
					memcpy(send_log_buf+sizeof(int),&c2l_task.iTaskType,sizeof(int));	
					memcpy(send_log_buf+sizeof(int)*2,&c2l_task.iNewLogCount,sizeof(int));
					
					memcpy(send_log_buf+sizeof(int)*3,sqlbuf,sizeof(St_Log)*tmp_num);
					logi.prt(p_base::LOG_INFO,"sqlsend	 is not -1	,the count is  :%d\n",tmp_num);
		
					unsigned int logip=0;
					unsigned int logport=0;
					unsigned int logindex=0;
							
					if(getoklogs(&logip,&logport,&logindex)!=0)
					{
						logi.prt(p_base::LOG_INFO,"get	log server ip error \n");
					}
								
					bool ret=false; 		
					if(logip!=0)
						ret=snd2logs(mypeer,send_log_buf,tmp_num,logip,logport,logindex);
					if(ret)
					{
						updatadb(logip,send_log_buf,tmp_num);
					}
		
					memset(send_log_buf, 0, DB_BUF_SIZE);	
					//memset(msgbuf,0,DB_BUF_SIZE);
					sleep(1);
				}
					
			}
		}

		if(isupload==true)
		{	
			unsigned int logip=0;
			unsigned int logport=0;
			unsigned int logindex=0;

			char *uploadbuf=NULL;
			uploadbuf=getyesday();
			
			isupload=false;
							
			if(getoklogs(&logip,&logport,&logindex)!=0)
			{
				logi.prt(p_base::LOG_TERM,"get	log server ip error \n");
			}
			if(logip!=0)
			{
				bool ret=false;
				ret=snd2upload(mypeer,uploadbuf,DATE_SIZE,logip,logport,logindex);
				if(!ret)
				{
					logi.prt(p_base::LOG_TERM," upload log error \n");					
				}
			}
				
		}


		
		sleep(1);		
	}

	free(sqlbuf);
	free(send_log_buf);
}


/****************deal raknet data from media server and logserve ****************/
void  *raknet_serv_func(void *)
{   
	//Server server;
	
	//logi.prt(p_base::LOG_INFO,"This is raknet pthread.\n");
	//logi.prt(p_base::LOG_INFO,"Connects many clients to a single server.\n");


   	server.Start();
	logi.prt(p_base::LOG_INFO,"Started server\n");

	//RakNetTime time = RakNet::GetTime();
	while(1)
	{
		server.Update();
		sleep(1);
	}
}


/***********************system timer ***************************/
void *system_timer_func(void *)
{
	int msgid;
	struct msgtype_s 
	{
    	long mtype;
    	char buffer[TMP_BUF_SIZE];
	};

	time_t uploadtime=0;

	struct msgtype_s msg;
	msg.mtype = 2;
	if((msgid=msgget(1234, IPC_EXCL))==-1)
    {
        fprintf(stderr,"Creat Message  Error:%s/n", strerror(errno));
    }
	

	list<time_elems>* _list=new list<time_elems>;
	list<time_elems>::iterator iter;
	list<time_elems>::iterator iter_ok;
		
	time_elems _elems;		
	bool  _okflag=false;


     //24*(60/(conf->GetValueInt(XML_MERGETIME)  the nums 
     //60*(conf->GetValueInt(XML_MERGETIME)   the time at once
 	logi.prt(p_base::LOG_TERM,"PER_ONCE:%d,PER_HOUR_SEC:%d---------------",24*60/(conf->GetValueInt(XML_MERGETIME)),60*conf->GetValueInt(XML_MERGETIME));
	for(int i=0;i<24*(60/(conf->GetValueInt(XML_MERGETIME)));i++)
	{
		_elems.secs=get_stdday_time()+i*60*(conf->GetValueInt(XML_MERGETIME));
		_elems.flag=false;	
		_list->push_back(_elems);
	}


	//logi.prt(p_base::LOG_TERM,"upload time :%s---------------",conf->GetValueString(XML_UPLOADTIME));
	uploadtime=get_stdday_time()+StrToSec((char *)conf->GetValueString(XML_UPLOADTIME));
	//logi.prt(p_base::LOG_TERM,"upload time :%s-------xxx--------",getstrtime(uploadtime));
	
	

	while(1)
	{
        for(iter = _list->begin(); iter != _list->end(); ++iter)  
		{  
			if( (GetTimeSec() >= iter->secs)&&(iter->flag==false))
			{
				iter->flag=true;
				_okflag=true;	
				iter_ok=iter;				
			}
			continue;
		}  

		if(_okflag==true)
		{
			_okflag=false;
			strcpy(msg.buffer,"cut\0");
			if(msgsnd(msgid, &msg, sizeof(msg.buffer), IPC_NOWAIT)==-1)
			{
				logi.prt(p_base::LOG_TERM,"msgsed error");
			}else
			{
				memset(msg.buffer,0,TMP_BUF_SIZE);
				logi.prt(p_base::LOG_TERM,"  find the time ,now time is******** :%s\n",getstrtime(iter_ok->secs));
			}
		}
		
		int count=0;
		for(iter = _list->begin(); iter != _list->end(); ++iter)  
		{  
			if( (iter->flag==false))
			{
				count++;
			}
		}  
		if((count==0)&&(get_stdday_time()>=60*(conf->GetValueInt(XML_MERGETIME))+iter_ok->secs))
		{
			time_t tmp_time=get_stdday_time();
			logi.prt(p_base::LOG_TERM,"update ***************,the std time is  :%s\n",getstrtime(tmp_time));
			_list->clear();
			for(int i=0;i<24*(60/(conf->GetValueInt(XML_MERGETIME)));i++)
			{
				_elems.secs=get_stdday_time()+i*60*(conf->GetValueInt(XML_MERGETIME));
				_elems.flag=false;
				_list->push_back(_elems);
			}
			strcpy(msg.buffer,"day\0");
			msgsnd(msgid, &msg, sizeof(msg.buffer), IPC_NOWAIT); 			
			memset(msg.buffer,0,TMP_BUF_SIZE);

			uploadtime=get_stdday_time()+StrToSec((char *)conf->GetValueString(XML_UPLOADTIME));
			
		}

		if((GetTimeSec()>=uploadtime)&&(uploadtime!=0))
		{
			uploadtime=0;
			strcpy(msg.buffer,"upload\0");
			msgsnd(msgid, &msg, sizeof(msg.buffer), IPC_NOWAIT); 			
			memset(msg.buffer,0,TMP_BUF_SIZE);
		}
		sleep(1);
	}
}


/****************************ini database**************************/
void init_database()
{
    
}

char * getdaytime(void)
{
    time_t now;
    struct tm *tm_now;
    static char  datetime[TMP_BUF_SIZE];
 
    time(&now);
    tm_now = localtime(&now);
    strftime(datetime, 200, "%Y%m%d\0\n", tm_now); 
	return datetime;
}





void assignwork(list<MLlist> *mylist,RakPeerInterface *mypeer,list<As_Media_List> *mycutlist)
{
	Client client;
	St_Log  recycle_log;

	
	char snd2media_buf[TMP_BUF_SIZE]={0};
	//list<MLlist> *mylist=server.getlist();		
	//RakPeerInterface *mypeer=server.getpeer();	
	//list<As_Media_List> *mycutlist=server.getcut_m_list();
	
	list<MLlist>::iterator iter;
	if(mycutlist ==NULL)
	{
			logi.prt(p_base::LOG_TERM, "mycut list errors**************** .");
	}
	
	mysqlop  *mysql=server.getsql();
	list<As_Media_List>::iterator iter_cut;
	C2M_StartCutLog *c2m=new C2M_StartCutLog;

	char *tmp_time=NULL;

	logi.prt(p_base::LOG_TERM, "1 min***************************************");
	char tmpsql[256]={0};
	time_t tmp_time_sec=0;
	tmp_time_sec=GetTimeSec()-60;

	int tmp_num=0;
	char tmp_sql_buf[DB_BUF_SIZE]={0};
	
	sprintf(tmpsql,"select * from ticklog where Status=1 and AssignTime<%u",tmp_time_sec);
	logi.prt(p_base::LOG_TERM, "%s",tmpsql);
	
	tmp_num=mysql->sqlexec(tmpsql,SQL_REQUIRE_DATA,tmp_sql_buf);
	
	memset(tmpsql,0,256);
	
	if(tmp_num!=0)
	{
		
		for(int i=0;i<tmp_num;i++)
		{
			logi.prt(p_base::LOG_TERM, "there have the data that not dealed.");
			memcpy(&recycle_log,tmp_sql_buf+i*sizeof(St_Log),sizeof(St_Log));
			sprintf(tmpsql,"update ticklog set status=0,WorkServerIP=0 where Workid=%d",recycle_log.iWorkID);
			logi.prt(p_base::LOG_TERM,"%s",tmpsql);
			mysql->sqlexec(tmpsql,SQL_EXIC_CMD,NULL);
		}
	}
	media_cut_over=true;
	mycutlist->clear();
	if(mylist->size()!=0)
	{				
		iUp_NUM++;
		if(iUp_NUM>999)
			iUp_NUM=0;
		for(iter= mylist->begin(); iter != mylist->end(); iter++)
		{
			if(iter->ServerType==MEDIA_S)
			{			
				SystemAddress addr(iter->fromip,iter->fromport);
				addr.systemIndex=iter->fromindex;
				tmp_time=getdaytime();
				c2m->iStamp=0;
				c2m->iUploadNumber=iUp_NUM;
				strcpy(c2m->szDate,tmp_time);

				memcpy(snd2media_buf,(char *)c2m,sizeof(C2M_StartCutLog));
				client.SendPacket(mypeer,ID_C2M_STARTCUTLOG, addr, snd2media_buf, TMP_BUF_SIZE);

				As_Media_List  as_m;
				as_m.media_ip=iter->fromip;
				as_m.media_port=iter->fromport;
				logi.prt(p_base::LOG_TERM,"%u----%u  ",as_m.media_ip,as_m.media_port);
	
				mycutlist->push_back(as_m);

				for(iter_cut=mycutlist->begin();iter_cut!=mycutlist->end();iter_cut++)
				{
					logi.prt(p_base::LOG_TERM,	"%u",iter_cut->media_ip);
				}
				logi.prt(p_base::LOG_TERM, "find media server******************:%u",iter->fromip);
			}
		}
		m_count=mycutlist->size();//获取总数
	}
	delete c2m;
	

}
 
/***************************send_media_func***********************/
void *send_media_func(void *)
{	
	list<MLlist> *mylist=server.getlist();	
	if(mylist ==NULL)
	{
			logi.prt(p_base::LOG_TERM, "mylist list errors**************** .");
	}
	RakPeerInterface *mypeer=server.getpeer();
	if(mypeer ==NULL)
	{
			logi.prt(p_base::LOG_TERM, "mypeer  errors**************** .");
	}
	list<As_Media_List> *mycutlist=server.getcut_m_list();
	if(mycutlist ==NULL)
	{
			logi.prt(p_base::LOG_TERM, "mycut list errors**************** .");
	}
	
	list<As_Media_List>::iterator iter_cut;
	//C2M_StartCutLog *c2m=new C2M_StartCutLog;

	//char *tmp_time=NULL;

	int msgid;
	//int i=0;
	typedef struct  
	{
    	long mtype;
    	char buffer[TMP_BUF_SIZE];
	}msgtype_r;

	msgtype_r msg;
	
	if((msgid=msgget(1234, IPC_CREAT|0666))==-1)
    {
        fprintf(stderr,"Creat Message  Error:%s\n", strerror(errno));
        exit(1);
    }
	//char snd2media_buf[TMP_BUF_SIZE]={0};
	while(1)
	{
		int ret=msgrcv(msgid, &msg, sizeof(msg.buffer), 2, IPC_NOWAIT|MSG_NOERROR);
		if(ret!=-1)
		{
			if(strcmp(msg.buffer,"day")==0)
			{
				iUp_NUM=0;
			}else if(strcmp(msg.buffer,"cut")==0)
			{
				logi.prt(p_base::LOG_TERM,"msg.buffer***************:%s\n",msg.buffer);

				server.clear_xml();
				server.updata_xml();

				//--------------------------------------------------
				assignwork(mylist,mypeer,mycutlist);
			}else if(strcmp(msg.buffer,"upload")==0)
			{
				logi.prt(p_base::LOG_TERM,"msg.buffer******xx*********:%s\n",msg.buffer);
				isupload=true;
			}
		}
		memset(msg.buffer,0,sizeof(msg.buffer));
		sleep(1);
	}

}

/*************************main func ****************************/
int main(int argc,char **argv)
{ 
   media_cut_over=false;

   //char *para=NULL; 
   logi.init("/home/doudou/test/log/","test");
   logi.set_log_level(p_base::LOG_INFO);

   conf = new CConfig((char *)"/home/doudou/ticketplatform/controlserv/setting.conf");
   if (!conf->Load())
   {
   			logi.prt(p_base::LOG_TERM,"xml config load error");		
   }
   //logi.prt(p_base::LOG_TERM,"xxxxxxxxxxxxxxxxxxxxx:%d\n",conf->GetValueInt(XML_MERGETIME));   
   //logi.prt(p_base::LOG_TERM,"kkkkkkkkkkkkkkkkkkkkk:%d\n",conf->GetValueInt(XML_UPLOADTIME));
   
   //clear_xml();
   //updata_xml();

   
   //conf->SetValueInt("mediacuttime",128);   
   //conf->SetValueInt("once",129);
   /* 	 	  
     if(daemon(1, 1) < 0)  
    {  
  	 logi.prt(p_base::LOG_INFO, "error.");
    }
    */
    
  //create database
  
  init_database();
  pthread_t sockservid;
  pthread_create(&sockservid,NULL,raknet_serv_func,NULL);
  pthread_t sockclientid;

  pthread_create(&sockclientid,NULL,send_log_func,NULL);
  pthread_t sendmediaid;
  pthread_create(&sendmediaid,NULL,send_media_func,NULL);
  pthread_t systimerid;
  pthread_create(&systimerid,NULL,system_timer_func,NULL);
  /*
   while(1)
   {	
  		char cmd[128];
		para=strstr(argv[1],"-");
		scanf("%s",cmd);
		if(strcmp("llist",cmd)==0)
			server.lslogslist();
		sleep(1);
   }
   */
  pthread_join(sockservid,NULL);
  pthread_join(sockclientid,NULL);
  pthread_join(sendmediaid,NULL);
  pthread_join(systimerid,NULL);
  return 0;
} 




