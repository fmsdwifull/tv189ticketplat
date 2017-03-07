#include <vector>
#include <arpa/inet.h>
#include <sys/socket.h>

#include "raknet_serv.h"

using namespace std;

#define SERVER_PORT 6666
#define RANDOM_DATA_SIZE 200
char randomData[RANDOM_DATA_SIZE];

extern p_base::CLog logi;
extern int m_count;
extern bool media_cut_over;



/************************Server**************************/
Server::Server()
{
	_media_data=new media_data;	
	ML_list=new list<MLlist> ; 
	As_M=new list<As_Media_List>; 
	sql=new mysqlop();
	setpeer();	
	
	pthread_mutex_init(&lock,NULL);
}

/************************~Server**************************/
Server::~Server()
{
	delete _media_data;
	delete sql;
}
/***********************set peer************************/
void Server::setpeer()
{	
	peer = RakNetworkFactory::GetRakPeerInterface();
}

/***********************get peer************************/
RakPeerInterface* Server::getpeer()
{
	return peer;
}

/***********************get mysql db************************/
mysqlop* Server::getsql()
{
	return sql;
}

/***********************get  ml list************************/

list<MLlist>* Server::getlist()
{
	return  ML_list;
}

/***************************get  cut_m_list *********************/
list<As_Media_List>* Server::getcut_m_list()
{
	return As_M;
}

/**************************clear list *****************************/
void Server::clear_m_list()
{
	As_M->clear();
}

/************************Start**************************/
void Server::Start(void)
{
	SocketDescriptor socketDescriptor;
	socketDescriptor.port=(unsigned short) SERVER_PORT;
	peer->Startup((unsigned short) NUM_CLIENTS,0,&socketDescriptor,1);
	peer->SetMaximumIncomingConnections(NUM_CLIENTS);
}

/*****************ConnectionCount**************************/
unsigned Server::ConnectionCount(void) const
{
	unsigned i,count;
	for (i=0,count=0; i < NUM_CLIENTS;i++)
		if (peer->GetSystemAddressFromIndex(i)!=UNASSIGNED_SYSTEM_ADDRESS)
			count++;
	return count;
}

/*****************Update**************************/
//void Server::Update(RakNetTime curTime)
void Server::Update()

{
	Packet *p = peer->Receive();
	while (p)
	{
		//logi.prt(p_base::LOG_INFO,"**************have data****************\n");
		//systemAddress
		switch (p->data[0])
		{
		case ID_CONNECTION_LOST://失去连接
			lostconn(p->systemAddress.binaryAddress,p->systemAddress.port);
			break;
		case ID_DISCONNECTION_NOTIFICATION:  //
		case ID_NEW_INCOMING_CONNECTION://新的连接进来
			logi.prt(p_base::LOG_INFO,"Connections = %i\n", ConnectionCount());
			break;
		case ID_USER_PACKET_ENUM:
			uerenum();
			break;
		case ID_M2C_JOIN:
			m2cjoin(p,MEDIA_S); 
			break;
		case ID_C2M_JOIN_RECEIVED:
		case ID_M2C_LEAVE:
			//m2cleave(p->systemAddress.binaryAddress,p->systemAddress.port);
			lostconn(p->systemAddress.binaryAddress,p->systemAddress.port);
			break;
		case ID_C2M_LEAVE_RECEIVED:
		case ID_L2C_JOIN:
			l2cjoin(p,LOG_S);
			break;
		case ID_C2L_JOIN_RECEIVED:
			break;
		case ID_L2C_LEAVE:
			//l2cleave(p->systemAddress.binaryAddress,p->systemAddress.port);
			lostconn(p->systemAddress.binaryAddress,p->systemAddress.port);
			break;
		case ID_M2C_ENDCUTLOD:
			endcutlod(p);
			break;

		case ID_C2L_LEAVE_RECEIVED:
			break;
		case ID_M2C_NEWLOGCREATED:
			m2cnewlog(p);
			break;	
		case ID_C2M_NEWLOGCREATED_RECEIVED:
		case ID_C2L_TASKASSIGN:
		case ID_L2C_TASKASSIGN_RECEIVED:
		case ID_L2C_TASKFINISHED:
			l2ctask(p);
			break;	
		case ID_C2L_TASKFINISHEDR_RECEIVED:
		break;
		default:
			logi.prt(p_base::LOG_INFO,"default***************************");
		
		
		}
		peer->DeallocatePacket(p);
		p = peer->Receive();
	}
}


/***********************disconnect ************************/
void Server::lostconn(int tmp_ip,unsigned short tmp_port)
{
	logi.prt(p_base::LOG_INFO,"ip: %u-----port:%d\n",tmp_ip,tmp_port);
	logi.prt(p_base::LOG_INFO,"disconnet  with :\n");

	list<MLlist>::iterator iter;

	for(iter = ML_list->begin(); iter != ML_list->end(); )  
	{	 
		list <MLlist> ::iterator t = iter++;	
		
		logi.prt(p_base::LOG_INFO,"ip: %u-----port:%d\n",t->fromip,t->fromport);
		if( (t->fromip==tmp_ip)&&(t->fromport==tmp_port))	
		{  
			ML_list->erase(t); 
			logi.prt(p_base::LOG_INFO,"ip: %u-----port:%d\n",t->fromip,t->fromport);
		}  
	}
}
void Server::uerenum()
{
	
}


/***********************get the size of media server list *****************/
int Server::getmediaslist()
{
	list<MLlist>::iterator iter;
	int num=0;
	for(iter= ML_list->begin(); iter != ML_list->end(); iter++)
	{
		if(iter->ServerType==MEDIA_S)
		{
			num++;
		}
	}
	return num;
}	

/***********************trave media server list************************/
void Server::lsmediaslist()
{
	sockaddr_in p;
	char *ip;

	list<MLlist>::iterator iter;
	for(iter= ML_list->begin(); iter != ML_list->end(); iter++)
	{
		if(iter->ServerType==0)
		{
			p.sin_addr.s_addr=iter->fromip;
			ip=inet_ntoa(p.sin_addr);
			logi.prt(p_base::LOG_TERM,"mediaser ip sets is :%s\n",ip);
		}
	}
}	


char* Server::ip2str( unsigned int intip)
{
	sockaddr_in p;
	static  char *ip=NULL;

	p.sin_addr.s_addr=intip;
	ip=inet_ntoa(p.sin_addr);
	
	return ip;
}


void Server::endcutlod(Packet *p)
{			
	logi.prt(p_base::LOG_TERM,"cut media list count is ------------:%d\n",m_count);
	list<As_Media_List>::iterator iter;
	for(iter = As_M->begin(); iter != As_M->end();iter++ )  
	{	 
		if((p->systemAddress.binaryAddress==iter->media_ip)&&(p->systemAddress.port==iter->media_port))
		{
			m_count--;
			logi.prt(p_base::LOG_TERM,"m_count-------------------------------------  :%d",m_count);
		}
	}
	if(m_count==0)
	{
		media_cut_over=true;
		clear_xml();
		updata_xml();
	}
	logi.prt(p_base::LOG_TERM,"media_cut_over-------------------------------------  :%d",media_cut_over);	
}
/***********************get the size of log server list *****************/
int Server::getlogslist()
{
	list<MLlist>::iterator iter;
	int num=0;
	for(iter= ML_list->begin(); iter != ML_list->end(); iter++)
	{
		if(iter->ServerType==1)
		{
			num++;
		}
	}
	return num;
}	


/***********************trave log server list************************/
void Server::lslogslist()
{
	sockaddr_in p;
	char *ip;

	list<MLlist>::iterator iter;
	for(iter= ML_list->begin(); iter != ML_list->end(); iter++)
	{
		if(iter->ServerType==1)
		{	
			char tmp_buf[256]={0};
			int tmp_num=0;
			sprintf(tmp_buf,"select * from ticklog where WorkServerIP=%u and Status=1",iter->fromip);
			tmp_num=sql->sqlexec(tmp_buf,SQL_CHENDK_NUM,NULL);

			p.sin_addr.s_addr=iter->fromip;
			ip=inet_ntoa(p.sin_addr);
			logi.prt(p_base::LOG_TERM,"logser ip sets is :%s,  work num is :%d",ip,tmp_num);
		}
		usleep(1000);
	}
}	


/******************media server join to control server******************/
void Server::m2cjoin(Packet *p,int media) 
{
	MLlist	ML_spec; //space	
	M2C_Join *mj;
	mj=(M2C_Join*)(p->data+1);
	ML_spec.fromip=p->systemAddress.binaryAddress;
	
	ML_spec.fromport=p->systemAddress.port;
	
	ML_spec.fromindex=p->systemAddress.systemIndex;
	ML_spec.ServerIndex=0;
	ML_spec.ServerType=media;//0--media server ;1--logserver
	ML_spec.ServerSubType=mj->type;
	ML_list->push_back(ML_spec);

	list<MLlist>::iterator it; //定义遍历指示器(类似于int i=0)	  

	for(it = ML_list->begin(); it!= ML_list->end() ;it++)  
	{  
		logi.prt(p_base::LOG_INFO," media join +++++:%s\n",ip2str(it->fromip));
	}  
	clear_xml();
	updata_xml();
}


/******************media server disconnect from control server******************/
void Server::m2cleave(int tmp_ip,unsigned short tmp_prot)
{
	list<MLlist>::iterator iter;
	for(iter = ML_list->begin(); iter != ML_list->end(); )  
	{	 
		list <MLlist> ::iterator t = iter++;	 
		if( (t->fromip==tmp_ip)&&(t->fromport==tmp_prot))	
		{  
			ML_list->erase(t);  
		}  
	}
	for(iter = ML_list->begin(); iter!= ML_list->end() ;iter++)  
	{  
		logi.prt(p_base::LOG_INFO," leave +++++:%s\n",ip2str(iter->fromip));
	}  
	clear_xml();
	updata_xml();
}


/******************log server join to control server******************/
void Server::l2cjoin(Packet *p,int logser)
{
	MLlist	ML_spec; //space
	L2C_Join *lj;
	lj=(L2C_Join*)(p->data+1);
	ML_spec.fromip=p->systemAddress.binaryAddress;
	ML_spec.fromport=p->systemAddress.port;
	ML_spec.fromindex=p->systemAddress.systemIndex;
	
	ML_spec.ServerIndex=0;
	ML_spec.ServerType=logser;//0--media server ;1--logserver
	ML_spec.ServerSubType=LOG_NULL;//
	
	ML_list->push_back(ML_spec);

	clear_xml();
	updata_xml();

}


/******************log server disconnect from control server******************/
void Server::l2cleave(int tmp_ip,unsigned short tmp_port)
{	
	list<MLlist>::iterator iter;		
	for(iter = ML_list->begin(); iter != ML_list->end(); )  
	{	 
		list <MLlist> ::iterator t = iter++;	 
		if( (t->fromip==tmp_ip)&&(t->fromport==tmp_port))	
		{  
			ML_list->erase(t);  
		}  
	}
	for(iter = ML_list->begin(); iter!= ML_list->end() ;iter++)  
	{  
		logi.prt(p_base::LOG_INFO,"leave +++++:%s\n",ip2str(iter->fromip));
	}  
	clear_xml();
	updata_xml();
}


/******************media server send new logs ******************/
void Server::m2cnewlog(Packet *p)
{
	//get  dirname and filename
	char src[TMP_BUF_SIZE];
	char src_new[TMP_BUF_SIZE];
	char filename[TMP_BUF_SIZE];
	
	char *filename_new;
	char dirname[TMP_BUF_SIZE];
	M2C_dbase DB_table;
	
	M2C_NewLogCreated  *dn;		

	//这里应该判断总包的长度，以免发生段错误
	logi.prt(p_base::LOG_INFO,"parket length is :%d\n",p->length);
	if(p->length<sizeof(M2C_NewLogCreated))
	{
		logi.prt(p_base::LOG_INFO,"recv data is too lower,please check \n");
		//exit(-1);
	}
	dn=(M2C_NewLogCreated*)(p->data+1);
#if 0
	strcpy(src,"/mnt/log/");
	strcat(src,(char *)dn->szLogFilePath);
	printf("filename is :%s\n",src);

	if(deal_db.Fileparase(src,filename,dirname)!=0)
	{
		perror("file name error ");
		exit(-1);
	}
	printf("filename:%s,dirname:%s\n",filename,dirname);

	filename_new=deal_db.modifylogfile(dirname,filename);

	char newpath[TMP_BUF_SIZE]={0};
	strcpy(src_new,(char *)dn->szLogFilePath);
	
	if(deal_db.Fileparase(src_new,filename,newpath)!=0)
	{
		perror("file name error ");
		exit(-1);
	}
	strcpy(newpath,filename_new);
#endif 
	//fine media server type 
	list<MLlist>::iterator iter;
	for(iter = ML_list->begin(); iter != ML_list->end(); iter++)  
	{	  
		if( (iter->fromip==p->systemAddress.binaryAddress)&&(iter->fromport==p->systemAddress.port))	
		{  
			DB_table.MediaServerTye=iter->ServerSubType;
			logi.prt(p_base::LOG_TERM,"MediaServerTye************************:%d",DB_table.MediaServerTye);
		}  
	}



	//ready for insert db 
	DB_table.MediaServerIP=p->systemAddress.binaryAddress;
	DB_table.UploadSeq=dn->iUploadNumber;
	DB_table.Seq=dn->iSequence;
	DB_table.Spec=-1;
	//strcpy((char *)DB_table.LogPath,newpath);
	strcpy((char *)DB_table.LogPath,(char *)dn->szLogFilePath);

	DB_table.Status=0;
	DB_table.WorkServerIP=0;
	DB_table.AssignTime=0;

	strcpy((char *)DB_table.szDate,(char *)dn->szDate);

	logi.prt(p_base::LOG_INFO,"mediaS+++++:%d\n",dn->iStamp);
	logi.prt(p_base::LOG_INFO,"mediaS+++++:%d\n",dn->iUploadNumber);
	logi.prt(p_base::LOG_INFO,"mediaS+++++:%d\n",dn->iSequence);
	logi.prt(p_base::LOG_INFO,"mediaS+++++:%d\n",dn->szLogFilePath);
	logi.prt(p_base::LOG_INFO,"mediaS+++++:%d\n",dn->szDate);

	char str_tmp[256];
	sprintf(str_tmp,"insert into ticklog(MediaServerType,MediaServerIP, UploadSeq,Seq,Spec,LogPath,Status,WorkServerIP,AssignTime,TimeDay) values (%d,'%s',%d,%d,%d,'%s',%d,'%s',%d,'%s')",  DB_table.MediaServerTye,ip2str(DB_table.MediaServerIP),DB_table.UploadSeq,DB_table.Seq,DB_table.Spec,DB_table.LogPath,DB_table.Status,ip2str(DB_table.WorkServerIP),DB_table.AssignTime,DB_table.szDate);
	//logi.prt(p_base::LOG_INFO,"str_tmp:%s\n",str_tmp);
	sql->sqlexec(str_tmp,SQL_EXIC_CMD,NULL);

	clear_xml();
	updata_xml();
}


/******************get the time by day with seconds standadly*******************/
char* Server::GetTimeStr(void)
{
    time_t now;
    static char timebuf[TMP_BUF_SIZE]={0};
    struct tm *tm_now;
    time(&now);
    tm_now = localtime(&now);
 
    //sprintf(timebuf,"%d-%d-%d %d:%d:%d\n",tm_now->tm_year+1900,tm_now->tm_mon, tm_now->tm_mday, tm_now->tm_hour, tm_now->tm_min,tm_now->tm_sec);
    sprintf(timebuf,"%d:%d:%d\n",tm_now->tm_hour, tm_now->tm_min, tm_now->tm_sec); 
    return timebuf;
}

/******************log server send new logs *****************/
void Server::l2ctask(Packet *p)
{
	int log_finished_num=0;
	int log_new_num=0;
	int tick_new_num=0;
	int stamp=0;	
	char datastr[TMP_BUF_SIZE]={0};

	
	
	logi.prt(p_base::LOG_TERM,"datastr------------------------------------------:%d",p->length);

	
	M2C_dbase DB_table;
	St_Log sl_tmp;
	
	vector<St_Log> vec_fininsh_tmp;
	vector<St_Log> vec_new_tmp;

	logi.prt(p_base::LOG_INFO,"the lenth of parket from loger server is :%d\n",p->length);
	
	if(p->length<sizeof(St_Log))
	{
		logi.prt(p_base::LOG_INFO,"the lenth of parket from loger server is :%d\n",p->length);
	}

	memcpy(&stamp,(p->data+1),sizeof(int));	
	memcpy(&log_finished_num,(p->data+1)+sizeof(int),sizeof(int));
			
	cout<<"    log_finished_num   " <<log_finished_num<<"   -   "<<sizeof(St_Log)<<endl;
	for(int i=0; i<log_finished_num;i++)
	{
		memcpy(&sl_tmp,p->data+1+sizeof(int)*2+i*sizeof(St_Log),sizeof(St_Log));
		logi.prt(p_base::LOG_INFO,"finished******:%d\n",sl_tmp.type);
		logi.prt(p_base::LOG_INFO,"finished******:%d\n",sl_tmp.iWorkID);
		logi.prt(p_base::LOG_INFO,"finished******:%d\n",sl_tmp.iUploadNumber);
		logi.prt(p_base::LOG_INFO,"finished******:%d\n",sl_tmp.iSequence);
		logi.prt(p_base::LOG_INFO,"finished******:%d\n",sl_tmp.spec);
		logi.prt(p_base::LOG_INFO,"finished******:%s\n",sl_tmp.szLogFilePath);
		//inster data to db      
		if(sl_tmp.DataIsBad==-1)
			sprintf(datastr,"update ticklog set Status=3,FinishdTime='%s' where Workid=%d",GetTimeStr(),sl_tmp.iWorkID);	
		else
			sprintf(datastr,"update ticklog set Status=2,FinishdTime='%s' where Workid=%d",GetTimeStr(),sl_tmp.iWorkID);

		
		logi.prt(p_base::LOG_INFO,"datastr:%s\n",datastr);
		sql->sqlexec(datastr,SQL_EXIC_CMD,NULL);

		
		
		memset(datastr,0,TMP_BUF_SIZE);
		usleep(1000);
	}
	
	//new log from logserver
	memcpy(&log_new_num,(p->data+1)+sizeof(int)*2+sizeof(St_Log)*log_finished_num,sizeof(int));
	
	cout<<"    log_new_num   " <<log_new_num<<endl;
	for(int i=0; i<log_new_num;i++)
	{
		memcpy(&sl_tmp,p->data+1+sizeof(int)*2+sizeof(St_Log)*log_finished_num+i*sizeof(St_Log)+sizeof(int),sizeof(St_Log));
		logi.prt(p_base::LOG_INFO,"new******:%d\n",sl_tmp.type);
		logi.prt(p_base::LOG_INFO,"new******:%d\n",sl_tmp.iWorkID);
		logi.prt(p_base::LOG_INFO,"new******:%d\n",sl_tmp.iUploadNumber);
		logi.prt(p_base::LOG_INFO,"new******:%d\n",sl_tmp.iSequence);
		logi.prt(p_base::LOG_INFO,"new******:%d\n",sl_tmp.spec);
		logi.prt(p_base::LOG_INFO,"new******:%s\n",sl_tmp.szLogFilePath);

		DB_table.MediaServerTye=sl_tmp.type;
		DB_table.MediaServerIP=0;
		DB_table.UploadSeq=sl_tmp.iUploadNumber;
		DB_table.Seq=sl_tmp.iSequence;
		DB_table.Spec=sl_tmp.spec;
		strcpy((char *)DB_table.LogPath,(char *)sl_tmp.szLogFilePath);
		DB_table.Status=0;
		DB_table.WorkServerIP=0;
		DB_table.AssignTime=10;


		//
		sprintf(datastr,"insert into ticklog(MediaServerType,MediaServerIP, UploadSeq,Seq,Spec,LogPath,Status,WorkServerIP,AssignTime,ProduceTime) values (%d,'%s',%d,%d,%d,'%s',%d,'%s',%d,'%s')",DB_table.MediaServerTye,ip2str(DB_table.MediaServerIP),DB_table.UploadSeq,DB_table.Seq,DB_table.Spec,DB_table.LogPath,DB_table.Status,ip2str(DB_table.WorkServerIP),DB_table.AssignTime,GetTimeStr());
		logi.prt(p_base::LOG_INFO,"datastr--:%s\n",datastr);
		sql->sqlexec(datastr,SQL_EXIC_CMD,NULL);		
		usleep(1000);
		
    }	


	memcpy(&tick_new_num,(p->data+1)+sizeof(int)*2+sizeof(St_Log)*log_finished_num+sizeof(St_Log)*log_new_num,sizeof(int));//128
	char tickbuf[128]={0};
	for(int i=0;i<tick_new_num;i++)
	{
		memcpy(tickbuf,(p->data+1)+sizeof(int)*2+sizeof(St_Log)*log_finished_num+sizeof(St_Log)*log_new_num+i*128+sizeof(int),128);			
		logi.prt(p_base::LOG_TERM,"tickbuf-------:%s\n",tickbuf);
	}
	clear_xml();
	updata_xml();

	
	//ready for input db 

}

void Server::clear_xml()
{
	const char* filepath = "report3.xml";
	TiXmlDocument doc(filepath);

	bool loadOk = doc.LoadFile();	
	if (!loadOk) {			
		logi.prt(p_base::LOG_TERM,"Could not load test file");
		exit(-1);
	}
	//TiXmlHandle docHandle(&doc);
	TiXmlElement* root = doc.RootElement();
	

	logi.prt(p_base::LOG_TERM,"++++++++++++++++++++++++++++++++++++++++++++:%s\n",root->Value());
	// TiXmlNode*  node = root->FirstChild();
	//root->RemoveChild(node);

	//delete mediaserver node
	for( TiXmlNode*  node = root->FirstChild("mediaserver");node;node = node->NextSibling("mediaserver")) // nextsibling 下一个兄弟
	{
		
		logi.prt(p_base::LOG_TERM,"1111111:%s\n",node->Value());
		root->RemoveChild(node);
	}
    //delete logserver node
	for( TiXmlNode*  node = root->FirstChild("logserver");node;node = node->NextSibling("logserver")) // nextsibling 下一个兄弟
	{
		
		logi.prt(p_base::LOG_TERM,"22222:%s\n",node->Value());
		root->RemoveChild(node);
	}

	//delete lastuploadtime node 
	TiXmlNode* node1 = root->FirstChild("lastuploadtime");
	root->RemoveChild(node1);

	//delete nextuploadtime node
	TiXmlNode* node2 = root->FirstChild("nextuploadtime");
	root->RemoveChild(node2);

	
	//delete lastprocesstime node
	TiXmlNode* node3 = root->FirstChild("lastprocesstime");
	root->RemoveChild(node3);
	
	
	doc.SaveFile();	
}


void Server::insertmedia_nd(TiXmlElement* fromnode,char *popnodename,char *vip,char *vstatus)
{
	TiXmlNode*	  newNode = new TiXmlElement(popnodename);
	const TiXmlNode* name4NewNode = new TiXmlElement("ip");
	newNode->InsertEndChild(*name4NewNode)->InsertEndChild(TiXmlText(vip));

	const TiXmlNode* addr4NewNode = new TiXmlElement("status");
	newNode->InsertEndChild(*addr4NewNode)->InsertEndChild(TiXmlText(vstatus));

	fromnode->InsertEndChild(*newNode);
}

void Server::insertlog_nd(TiXmlElement* fromnode,char *popnodename,char *vip,char *vstatus,char *vworkqueu)
{
	TiXmlNode*	  newNode = new TiXmlElement(popnodename);
	const TiXmlNode* name4NewNode = new TiXmlElement("ip");
	newNode->InsertEndChild(*name4NewNode)->InsertEndChild(TiXmlText(vip));

	const TiXmlNode* addr4NewNode = new TiXmlElement("status");
	newNode->InsertEndChild(*addr4NewNode)->InsertEndChild(TiXmlText(vstatus));

	const TiXmlNode* tel4NewNode = new TiXmlElement("workqueue");
	newNode->InsertEndChild(*tel4NewNode)->InsertEndChild(TiXmlText(vworkqueu));
	fromnode->InsertEndChild(*newNode);
}

void Server::insertsig_nd(TiXmlElement* fromnode,char *popnodename,char *value)
{	
	const TiXmlNode* newNode = new TiXmlElement(popnodename);
	fromnode->InsertEndChild(*newNode)->InsertEndChild(TiXmlText(value));	
}
void Server::updata_xml()
{
	sockaddr_in p;
	char *ip;

	list<MLlist>::iterator iter;
	
	const char* filepath = "report3.xml";
	
	TiXmlDocument doc(filepath);

	bool loadOk = doc.LoadFile();	
	if (!loadOk) {			
		logi.prt(p_base::LOG_TERM,"Could not load test file");
		exit(-1);
	}
	TiXmlElement* root = doc.RootElement();

	//TiXmlHandle docHandle(&doc);
	for(iter= ML_list->begin(); iter != ML_list->end(); iter++)
	{
		if(iter->ServerType==0)
		{
			p.sin_addr.s_addr=iter->fromip;
			ip=inet_ntoa(p.sin_addr);
			logi.prt(p_base::LOG_TERM,"mediaser ip sets is :%s\n",ip);
			if(!media_cut_over)
				insertmedia_nd(root,"mediaserver",ip,(char *)"1");
			else
				insertmedia_nd(root,"mediaserver",ip,(char *)"0");
		}
	}

	for(iter= ML_list->begin(); iter != ML_list->end(); iter++)
	{
		if(iter->ServerType==1)
		{
			p.sin_addr.s_addr=iter->fromip;
			ip=inet_ntoa(p.sin_addr);
			logi.prt(p_base::LOG_TERM,"logser ip sets is :%s\n",ip);

            //get the num of works 
			char tmp_buf[128]={0};
			int tmp_num=0;
			sprintf(tmp_buf,"select * from ticklog where WorkServerIP=%u and Status=1",iter->fromip);
			tmp_num=sql->sqlexec(tmp_buf,SQL_CHENDK_NUM,NULL);
			sprintf(tmp_buf,"%d\0",tmp_num);

			//get the status  of logs
			int tmp_num_s=0;
			tmp_num_s=sql->sqlexec("select * from ticklog where  Status=1",SQL_CHENDK_NUM,NULL);

			//insert
			if(tmp_num_s!=0)
				insertlog_nd(root,"logserver",ip,"1",tmp_buf);
			else
				insertlog_nd(root,"logserver",ip,"0",tmp_buf);

		}
	}

	insertsig_nd(root,(char *)"lastuploadtime",(char *)"19:10");
	insertsig_nd(root,(char *)"lastprocesstime",(char *)"19:10");
	insertsig_nd(root,(char *)"nextuploadtime",(char *)"19:10");
	

	doc.SaveFile();
}



