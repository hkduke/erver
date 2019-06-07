

/* ----------------------------------------------------------------------------------------------------------



 in 2009-3-21
-----------------------------------------------------------------------------------------------------------*/
#ifndef __AIThread_h__
#define __AIThread_h__

#include "Communication2.h"
#include "Thread.h"
#include <limits.h>

class FMsgQueue;
class TcpConnection;
class NetMsg;
class BodyZDSetPlayerInfo;

const int MSG_INPUT_QUEUE_LENGTH = 8*1024*1024;

class AiThread : public Thread
{
public:
	AiThread();
	virtual ~AiThread();

	virtual int					ThreadInit();
	virtual void				Run();
	virtual int					CanBlocked();

	void						DBThreadOnEnd();
	int							SendOneMsg( short nMsgLength, BYTE* pMsg );
	int							RecvOneMsg( short& nMsgLength, BYTE* pMsg );

	int							Initialize( int iThreadID, Communication2* pCommunication );

public:

	void						PrintMsgToLog( BYTE* pMsg, int nMsgLength );

	void						ThreadInternalCheck();

	int							SendMsgToMainServer( int nDBSwitchID, short sMainServerID, BYTE* pBuf, short nBufLength );
	int							SendMsgToMainServer(short sMainServerID, NetMsgBody * msgbody );
	int							ConvertBufToMsg( short nBufLength, BYTE* pBuf, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg );
	int							OnEvent( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg );
	int							Process_C2S_PlayCard( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg );
	int							Process_C2S_PassivePlayCard( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg );
	int							Process_C2S_GrabLandlord( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg );

	FMsgQueue*							m_pInputQueue;					
	SwitchMsgHead						m_oCurrSwitchMsgHead;

	int									m_iThreadID;
	time_t								m_tLastCheckTime;

	time_t								m_tCurTime;
	time_t								m_tLastSessionCheckTime;

	Communication2 *					m_pCommunication2;
};

#endif
