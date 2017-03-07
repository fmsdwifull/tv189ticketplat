#include "RakPeerInterface.h"
#include "RakNetworkFactory.h"
#include "BitStream.h"
#include <stdlib.h> // For atoi
#include <cstring> // For strlen
#include "Rand.h"
#include "RakNetStatistics.h"
#include "MessageIdentifiers.h"
#include <stdio.h>


#include <string>

#ifndef  _H_Kbhit
#define  _H_Kbhit
	#include "Kbhit.h"
#endif




#include "GetTime.h"
#include "RakAssert.h"
#include "RakSleep.h"
#include "Getche.h"



#include <unistd.h> // usleep
#include <cstdio>

#include <iostream>


#include <list>   
#include <string.h>

using namespace std;  

#define  UNASSIGNED  1
#define  ASSIGNED    2 
#define  FINESHED    3 
  


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
		ID_L2M_JOIN_RECEIVED,
		ID_L2C_LEAVE,
		ID_L2M_LEAVE_RECEIVED,
		ID_M2C_NEWLOGCREATED,
		ID_C2M_NEWLOGCREATED_RECEIVED,
		ID_C2L_TASKASSIGN,
		ID_L2C_TASKASSIGN_RECEIVED,
		ID_L2C_TASKFINISHED,
		ID_C2L_TASKFINISHEDR_RECEIVED,
	};

#endif 




//static const int NUM_CLIENTS=64;

 
#if 1

// Connects, sends data over time, disconnects, repeat
class Client
{
	public:
		Client();
		~Client();
		void Startup(void);
		void Connect(void);
		void Disconnect(void);
		void Update(RakNetTime curTime);

		void SetConnectInfo(const char* szIP, const unsigned short iPort);
		bool IsConnect(void);
		bool SendPacket(RakPeerInterface *mypeer,UserMessageIDTypes msgid, void* data = 0, int length = 0);
		int GetStampTick(void) {m_iStamp++; return m_iStamp;};


		bool SendPacket(RakPeerInterface *mypeer, UserMessageIDTypes msgid, SystemAddress desAddr, void* data = NULL, int length = 0);

			
		bool isConnected;
		RakPeerInterface *peer;
		RakNetTime nextSendTime;
		RakNetTime flipConnectionTime;

		string m_strIP;
        unsigned short m_iPort;
        int m_iStamp;
};
#endif
