/* ----------------------------------------------------------------------------------------------------------



in in 2010-10-11
----------------------------------------------------------------------------------------------------------*/

#ifndef __COMMUNICATION2_H__
#define __COMMUNICATION2_H__

#include "PopWorldInclude.h"
#include "PopWorldDefine.h"
#include "SwitchMsgHead.h"
#include "ICommand.h"
#include "TcpConnection.h"
#include "../DdzAiPublic/include/MsgDefine.h"

enum{ Max_IPAndPort_Str_Len = 32, MAX_UNREGISTER_NUM = 60 };

struct CommunicationConfig
{
	char m_szLocalIPAddr[Max_IPAndPort_Str_Len];
	char m_szLocalPort[Max_IPAndPort_Str_Len];
	
	int  m_iConnectCount;
};

struct UnregisterSocketInfo
{
	int		m_iSocketFD;									// connect socket
	ULONG   m_uIP;											// ip address
	time_t	m_tAcceptTime;             
};

struct ConnectionProfile2
{
	unsigned long	m_ulAddr;
	unsigned short	m_usPort;
	time_t			m_iTimestamp;
	
	short			m_sServerId;
	short			m_sServerType;

	ConnectionProfile2()
	{
		Clean();
	}

	void Clean()
	{
		m_sServerId	= 0;
		m_ulAddr	= INADDR_NONE;
		m_usPort	= 0;
		m_iTimestamp= 0;
		m_sServerType = 0;
	}

	const char * GetIPStr()
	{
		in_addr addr;
		addr.s_addr = m_ulAddr;
		return inet_ntoa(addr);
	}
};

class Communication2
{
public:
	enum eCommunicationType
	{
		CT_Server = 0,
		CT_Client,
		CT_ThreadServer,
		CT_ThreadClient,
	};

	Communication2( eCommunicationType eType );
	~Communication2();

	int  Initialize( int iServerType, int iServerID, const char* szFinename, ICommunicationCommand* pCommand );

	void Update( ICommunicationCommand* pCommand );

	int SendMsgToGroup( unsigned char * pMsg, short sMsgLen, int iSrvType, unsigned char bCommand = 1 );
	int SendMsgToGroup( NetMsgBody * pMsg, int iSrvType, unsigned char bCommand = 1 );

	int SendMsgToServer( unsigned char * pMsg, short sMsgLen, int iSrvType, int iSrvId, unsigned char bCommand = 1 );
	int SendMsgToServer( NetMsgBody * pMsg, int iSrvType, int iSrvId, unsigned char bCommand = 1 );

	int SendMsgByUID( unsigned char * pMsg, short sMsgLen, int iSrvType, int iSrvId, unsigned int unUid, unsigned char bCommand = 1 );
	int SendMsgByUID( NetMsgBody * pMsg, int iSrvType, int iSrvId, unsigned int unUid, unsigned char bCommand = 1 );

	struct BackServerOnlineStatus
	{
		short	m_sServerID;
		short	m_sServerType;
		unsigned char m_ucServerStatus;
	};
	void GetBackServer( BackServerOnlineStatus* pBackServerList, int& iBackServerCount );

private:
	enum { 
		MAX_CONNECTIONS = 32,
		CONNECT_TIME_CHECK = 5,
		KEEP_ALIVE_SECONDS = 3*CONNECT_TIME_CHECK,
		REQUEST_CONNECT_TIME_CHECK = 10,
		ACCEPT_TIME_CHECK = 10,
		MAX_CLEAN_BLOCK_MSGS = 512,
	};

	Communication2() {}

	int LoadConfig( const char * filename );

	int CheckBlockMsg();
	int FindRepeatConfig( short sServerType, short sServerId );

	void ServerUpdate( ICommunicationCommand* pCommand );
	void ClientUpdate( ICommunicationCommand* pCommand );
	void ThreadClientUpdate( ICommunicationCommand* pCommand );
	void ThreadServerUpdate( ICommunicationCommand* pCommand );

	int RegisterToServer( TcpConnection* pTcpConnect, short sSrcType, short sScrId, short sDstType, short sDstId );
	int SendKeepAliveToServer( TcpConnection* pTcpConnect, short sSrcType, short sScrId, short sDstType, short sDstId );
	int ReceiveAndDispatchConnectMsg( ICommunicationCommand* pCommand );
	int ServerReceiveAndDispatchConnectMsg( ICommunicationCommand* pCommand );
	int DispatchOneMsgFromConnect( int iDBSwitchServerId, short nMsgLength, BYTE* pMsg, ICommunicationCommand* pCommand, int iConnect );
	int ServerDispatchOneMsgFromConnect( int iDBSwitchServerId, short nMsgLength, BYTE* pMsg, ICommunicationCommand* pCommand, int iConnect );
	void CheckConnectStatus(ICommunicationCommand* pCommand);

	int  CheckConnectRequest();
	int  DeleteOneUnregisterConn( int iUnRegisterIndex );
	void HandleConnectRequest();
	int  ReceiveAndRegister( int iUnregisterIndex );
	TcpConnection* GetConnectByAddrAndID( short shSrcFE, short shSrcID, ULONG ulSrcAddr );

	int SingleSendMsg( unsigned char* pBuffer, int iSize, void* pSender, int iFlag );
	int MultiThreadSendMsg( unsigned char* pBuffer, int iSize, void* pSender, int iFlag );

	typedef void (Communication2::*UpdateFunc)( ICommunicationCommand* pCommand );
	typedef int (Communication2::*SendMsgFunc)( unsigned char* pBuffer, int iSize, void* pSender, int iFlag );

	UpdateFunc			m_pUpdateFunc;
	SendMsgFunc			m_pSendMsgFunc;

	TcpConnection		m_vConnection[MAX_CONNECTIONS];
	
	time_t				m_vLastKeepAliveTime[MAX_CONNECTIONS];

	time_t				m_tLastConnectCheckTime;

	unsigned char		m_szInMsgBuf[16 * 128 * 1024];

	ConnectionProfile2	m_vConnectInfo[MAX_CONNECTIONS];
	
	eCommunicationType	m_eType;

	TcpSocket			m_stListenSocket;

	int					m_iServerID;
	int					m_iServerType;

	int					m_iCurrUnregisterNum;
	UnregisterSocketInfo m_vUnregisterInfo[MAX_UNREGISTER_NUM];

	CommunicationConfig	m_oConfig;

	timeval				m_oLastBlockCheckTime;
};


#endif

