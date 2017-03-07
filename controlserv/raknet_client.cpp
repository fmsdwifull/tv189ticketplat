#include "raknet_client.h"
#include "raknet_serv.h"


#define SERVER_PORT 7777
#define RANDOM_DATA_SIZE 200

extern 	Server server;


static char randomData[RANDOM_DATA_SIZE];

/*****************Client**************************/
Client::Client()
{
	peer = RakNetworkFactory::GetRakPeerInterface();
}

/*****************~Client**************************/
Client::~Client()
{
	RakNetworkFactory::DestroyRakPeerInterface(peer);
}

/*****************Startup**************************/
void Client::Startup(void)
{
	SocketDescriptor socketDescriptor;
	socketDescriptor.port=0;
	nextSendTime=0;
	peer->Startup(1,30,&socketDescriptor,1);
	flipConnectionTime=RakNet::GetTime()+(randomMT()%30000);
	isConnected=false;
}

/*****************Connect**************************/
void Client::Connect(void)
{
	bool b;
	b=peer->Connect("192.168.19.223", (unsigned short) SERVER_PORT, 0, 0, 0);
	if (b==false)
	{
		printf("Client connect call failed!\n");
	}
}

/*****************Disconnect**************************/
void Client::Disconnect(void)
{
	peer->CloseConnection(peer->GetSystemAddressFromIndex(0),true,0);
	isConnected=false;
}

/*****************IsConnect**************************/
bool Client::IsConnect(void)
{
        //return GetConnectionstatus();
}

/*****************SendPacket**************************/
bool Client::SendPacket(RakPeerInterface *mypeer,UserMessageIDTypes msgid, void* data, int length)
{
        SystemAddress desAddr(m_strIP.c_str(), m_iPort);
        return SendPacket(mypeer,msgid, desAddr, data, length);
}


/*****************Update**************************/
void Client::Update(RakNetTime curTime)
{
	Packet *p = peer->Receive();
	while (p)
	{
		switch (p->data[0])
		{
		case ID_CONNECTION_REQUEST_ACCEPTED:
			isConnected=true;
			break;
			// print out errors
		case ID_CONNECTION_ATTEMPT_FAILED:
			printf("Client Error: ID_CONNECTION_ATTEMPT_FAILED\n");
			isConnected=false;
			break;
		case ID_ALREADY_CONNECTED:
			printf("Client Error: ID_ALREADY_CONNECTED\n");
			break;
		case ID_NO_FREE_INCOMING_CONNECTIONS:
			printf("Client Error: ID_NO_FREE_INCOMING_CONNECTIONS\n");
			isConnected=false;
			break;
		case ID_DISCONNECTION_NOTIFICATION:
			//printf("ID_DISCONNECTION_NOTIFICATION\n");
			isConnected=false;
			break;
		case ID_CONNECTION_LOST:
			printf("Client Error: ID_CONNECTION_LOST\n");
			isConnected=false;
			break;
		case ID_MODIFIED_PACKET:
			printf("Client Error: ID_MODIFIED_PACKET\n");
			break;
		}
		peer->DeallocatePacket(p);
		p = peer->Receive();
	
	}

	if (curTime>flipConnectionTime)
	{
		if (isConnected)
		{
			Disconnect();
			flipConnectionTime=curTime+5000+(randomMT()%10000);
		}
		else
		{
			Connect();
			flipConnectionTime=curTime+5000+(randomMT()%15000);
		}
	}
	if (curTime>nextSendTime && isConnected)
	{
	    strcpy(randomData,"12345");
		//char randomData[RANDOM_DATA_SIZE];
		//peer->Send((const char*)&randomData,RANDOM_DATA_SIZE,HIGH_PRIORITY,RELIABLE_ORDERED,0,UNASSIGNED_SYSTEM_ADDRESS,true);
		peer->Send((const char*)&randomData,RANDOM_DATA_SIZE,HIGH_PRIORITY,RELIABLE_ORDERED,0,UNASSIGNED_SYSTEM_ADDRESS,true);
		nextSendTime=curTime+500+(randomMT()%1000);
	}
}


/************************SendPacket***************************/
bool Client::SendPacket(RakPeerInterface *mypeer,UserMessageIDTypes msgid, SystemAddress desAddr, void* data, int length)
{
        bool ret = false;
        char* szSend = (char*)malloc(sizeof(char) + length);
		

		szSend[0] = (char)msgid;
        if (data != NULL && length !=0)
        {
                memcpy(szSend + 1, data, length);

                ret =mypeer->Send((const char*)szSend,sizeof(char) + length,HIGH_PRIORITY,RELIABLE_ORDERED,0,desAddr,false);
				//Send(const char * data,const int length,PacketPriority priority,PacketReliability reliability,char orderingChannel,const AddressOrGUID systemIdentifier,bool broadcast,uint32_t forceReceipt = 0)
		}
        free(szSend);
        return ret;
}


