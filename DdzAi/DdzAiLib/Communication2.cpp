/* ----------------------------------------------------------------------------------------------------------
 

 in 2010-10-11
 ----------------------------------------------------------------------------------------------------------*/
#include "Communication2.h"
#include "SwitchMsgHead.h"
#include "Log.h"
#include "Thread.h"
#include "ConfigIniFile.h"

//-------------------------------------------------------------------
static bool CheckIPValue(char * ipstring)
{
	char * trimip = ipstring + strlen(ipstring);
	for (; trimip != ipstring; trimip--)
	{
		if (isdigit(*trimip)) 
			break;
		else 
			*trimip = '\0';
	}
	for (trimip = ipstring; trimip != ipstring + strlen(ipstring); trimip++)
	{
		if (isdigit(*trimip))
			break;
	}
	return (inet_addr(trimip) != INADDR_NONE);
}

static bool CheckPortValue(int portval)
{
	return (portval < 65536 && portval > 1024);
}

/*---------------------------------------------------------------------------------------------------------*/
static void PrintMsgToLog( BYTE* pMsg, int nMsgLength )
{
#ifdef _LDEBUG_

	int i;

	char tmpBuffer[16384];
	char strTemp[32];

	if ( nMsgLength <= 0 || nMsgLength > 4096 || pMsg == NULL )
	{
		return ;
	}

	tmpBuffer[0] = '\0';
	for ( i = 0; i < nMsgLength; i++ )
	{
		if ( !(i % 16) )
		{
			sprintf(strTemp, "\n%04d>    ", i / 16 + 1);
			strcat(tmpBuffer, strTemp);
		}
		sprintf(strTemp, "%02X ", (unsigned char)pMsg[i]);
		strcat(tmpBuffer, strTemp);
	}

	strcat(tmpBuffer, "\n");
	Log_Msg( Log_Debug, "Print Hex:%s", tmpBuffer );
#endif
}


/* ----------------------------------------------------------------------------------------------------------*/
Communication2::Communication2( eCommunicationType eType ) 
: m_eType(eType)
{
	switch ( eType )
	{
	case CT_Server:
		m_pUpdateFunc = &Communication2::ServerUpdate;
		m_pSendMsgFunc = &Communication2::SingleSendMsg;
		break;

	case CT_Client:
		m_pUpdateFunc = &Communication2::ClientUpdate;
		m_pSendMsgFunc = &Communication2::SingleSendMsg;
		break;

	case CT_ThreadClient:
		m_pUpdateFunc = &Communication2::ThreadClientUpdate;
		m_pSendMsgFunc = &Communication2::MultiThreadSendMsg;
		break;

	case CT_ThreadServer:
		m_pUpdateFunc = &Communication2::ThreadServerUpdate;
		m_pSendMsgFunc = &Communication2::MultiThreadSendMsg;
		break;
	}
	
	memset( (void*)&m_oConfig, 0, sizeof( m_oConfig ) );
	time( &m_tLastConnectCheckTime );

	memset( (void*)&m_vLastKeepAliveTime[0], 0, sizeof( m_vLastKeepAliveTime ) );

	m_iCurrUnregisterNum = 0;
	memset( ( void* )m_vUnregisterInfo, 0, sizeof(UnregisterSocketInfo) * MAX_UNREGISTER_NUM );

	memset( ( void* )&m_oLastBlockCheckTime, 0, sizeof( timeval ) );
}

/* ----------------------------------------------------------------------------------------------------------*/
Communication2::~Communication2()
{

}

/* ----------------------------------------------------------------------------------------------------------*/
void Communication2::Update( ICommunicationCommand* pCommand )
{
	( this->*m_pUpdateFunc )( pCommand );
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::Initialize( int iServerType, int iServerID, const char* szFinename, ICommunicationCommand* pCommand )
{
	m_iServerID = iServerID;
	m_iServerType = iServerType;

	int iRet = LoadConfig( szFinename );
	if ( iRet != 0 )
	{
		Log_Msg( Log_Failed, "Communication2: Read config file failed, Error code = %d .\n ", iRet );
		return -1;
	}

	if ( m_eType == CT_Server || m_eType == CT_ThreadServer )
	{
		if( m_stListenSocket.CreateServer( ( USHORT )atoi(  m_oConfig.m_szLocalPort ), m_oConfig.m_szLocalIPAddr ) )
		{
			Log_Msg( Log_Failed, "Communication: [ServerType = %s ] Create listen socket failed.\n", m_eType == CT_Server ? "CT_Server" : "CT_ThreadServer" );
			return -2;
		}
		Log_Msg( Log_Debug, "Communication: [ServerType = %s ] Create listen socket successful.\n", m_eType == CT_Server ? "CT_Server" : "CT_ThreadServer" );

		for( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
		{
			m_vConnection[i].Initialize( m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId, m_vConnectInfo[i].m_ulAddr, m_vConnectInfo[i].m_usPort );
		}
	}

	if ( m_eType == CT_Client || m_eType == CT_ThreadClient )
	{
		m_tLastConnectCheckTime = time( 0 );
		
		for( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
		{
			m_vConnection[i].Initialize( m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId, m_vConnectInfo[i].m_ulAddr, m_vConnectInfo[i].m_usPort );

			if( m_vConnection[i].ConnectToServer( m_oConfig.m_szLocalIPAddr ) )
			{
				in_addr inaddr;
				inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

				Log_Msg( Log_Error, "Communication: error connect to server %d IP %s:%u failed.\n", m_vConnectInfo[i].m_sServerId,
					inet_ntoa( inaddr ), m_vConnectInfo[i].m_usPort );
				continue;
			}
			else
			{
				in_addr inaddr;
				inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

				Log_Msg( Log_Normal, "Communication: connect to server[(TYPE = %d)(ID = %d)](%s:%d) is succeed.\n",
					m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId, inet_ntoa( inaddr ), m_vConnectInfo[i].m_usPort );
			}

			if( RegisterToServer( &( m_vConnection[i] ), (short)m_iServerType, (short)m_iServerID , m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId ))
			{
				in_addr inaddr;
				inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

				Log_Msg( Log_Error, "Communication: error register to server %d IP %s:%u failed.\n", m_vConnectInfo[i].m_sServerId,
					inet_ntoa( inaddr ),  m_vConnectInfo[i].m_usPort );
				continue;
			}
			else
			{
				in_addr inaddr;
				inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

				Log_Msg( Log_Normal, "Communication: register to server[(TYPE = %d)(ID = %d)](%s:%d) is succeed.\n",
					m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId, inet_ntoa( inaddr ), m_vConnectInfo[i].m_usPort );
				m_vConnection[i].SetConnectState( Connect_ON );

 				if (m_vConnectInfo[i].m_sServerType == Cst_UtilServer)
 				{
 					pCommand->Notify();
 				}
			}

			time( &m_vLastKeepAliveTime[i] );
		}
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
void Communication2::CheckConnectStatus(ICommunicationCommand* pCommand)
{
	time_t tNow = time( 0 );

	if( ( tNow - m_tLastConnectCheckTime ) > CONNECT_TIME_CHECK )
	{
		for( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
		{
			if( ( m_vConnection[i].GetSocketStatus() == Tcs_Connected ) && ( m_vConnection[i].GetSocketFD() > 0 ) )
			{
				if ( tNow - m_vLastKeepAliveTime[i] >= KEEP_ALIVE_SECONDS )
				{
					SendKeepAliveToServer( &( m_vConnection[i] ), (short)m_iServerType, (short)m_iServerID, m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId );
					time( &m_vLastKeepAliveTime[i] );
				}
			}
			else
			{
				Log_Msg( Log_Normal, "Communication: connect to server[%d](ID = %d) is not connected, try to reconnect it.\n",
					i, m_vConnection[i].GetID() );

				if( m_vConnection[i].ConnectToServer( m_oConfig.m_szLocalIPAddr ) )
				{
					in_addr inaddr;
					inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

					Log_Msg( Log_Error, "Communication: error connect to server %d IP %s:%u failed.\n", m_vConnectInfo[i].m_sServerId,
						inet_ntoa( inaddr ), m_vConnectInfo[i].m_usPort );
					continue;
				}
				else
				{
					in_addr inaddr;
					inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

					Log_Msg( Log_Normal, "Communication: connect to server[(TYPE = %d)(ID = %d)](%s:%d) is succeed.\n",
						m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId, inet_ntoa( inaddr ), m_vConnectInfo[i].m_usPort );

				}

				if( RegisterToServer( &( m_vConnection[i] ), (short)m_iServerType, (short)m_iServerID , m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId ))
				{
					in_addr inaddr;
					inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

					Log_Msg( Log_Error, "Communication: error register to server %d IP %s:%u failed.\n", m_vConnectInfo[i].m_sServerId,
						inet_ntoa( inaddr ),  m_vConnectInfo[i].m_usPort );
					continue;
				}
				else
				{
					in_addr inaddr;
					inaddr.s_addr = m_vConnectInfo[i].m_ulAddr;

					Log_Msg( Log_Normal, "Communication: register to server[(TYPE = %d)(ID = %d)](%s:%d) is succeed.\n",
						m_vConnectInfo[i].m_sServerType, m_vConnectInfo[i].m_sServerId, inet_ntoa( inaddr ), m_vConnectInfo[i].m_usPort );
					m_vConnection[i].SetConnectState( Connect_ON );

					if (pCommand)
					{
						Log_Msg(Log_Debug, "communication reconnect notify\n");
						pCommand->Notify();
					}
				}

				time( &m_vLastKeepAliveTime[i] );
			}
		}

		m_tLastConnectCheckTime = tNow;
	}
}

/* ----------------------------------------------------------------------------------------------------------*/
void Communication2::ClientUpdate( ICommunicationCommand* pCommand )
{
	CheckConnectStatus(pCommand);

	ReceiveAndDispatchConnectMsg( pCommand );

	CheckBlockMsg();
}

/*---------------------------------------------------------------------------------------------------------*/
void Communication2::GetBackServer( BackServerOnlineStatus* pBackServerList, int& iBackServerCount )
{
	int iMaxBackServerCount = iBackServerCount;

	iBackServerCount = 0;

	for( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; ++i )
	{
		if ( (   m_vConnectInfo[i].m_sServerType == Cst_DBServer || m_vConnectInfo[i].m_sServerType == Cst_UtilServer ) && iBackServerCount < iMaxBackServerCount )
		{
			pBackServerList[iBackServerCount].m_sServerID = m_vConnectInfo[i].m_sServerId;
			pBackServerList[iBackServerCount].m_sServerType = m_vConnectInfo[i].m_sServerType;
			pBackServerList[iBackServerCount].m_ucServerStatus =  ( m_vConnection[i].GetSocketFD() > 0 && m_vConnection[i].GetSocketStatus() == Tcs_Connected ) ? 1 : 0;

			iBackServerCount++;
		}
	}

}

/*---------------------------------------------------------------------------------------------------------*/
int Communication2::ReceiveAndDispatchConnectMsg( ICommunicationCommand* pCommand )
{
	SOCKET iFD = INVALID_SOCKET;
	int iStatus = Tcs_Closed;

	fd_set fds_read;
	FD_ZERO(&fds_read);

	int i = 0;
	for( ; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		iFD = m_vConnection[i].GetSocketFD();
		iStatus = m_vConnection[i].GetSocketStatus();

		if( iFD > 0 && iStatus == Tcs_Connected )
		{
			FD_SET( iFD, &fds_read );
		}
	}

	struct timeval oSelectTime;
	oSelectTime.tv_sec = 0;
	oSelectTime.tv_usec = 1000;

	int iOpenFDNum = select( FD_SETSIZE, &fds_read, NULL, NULL, &oSelectTime );
	if( iOpenFDNum <= 0 )
	{
		return 0;
	}

	int iCount = 0;
	short nMsgLength;
	BYTE  bMsgBuf[MSG_BUFFER_LENGTH];

	for( i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		iFD = m_vConnection[i].GetSocketFD();
		if(iFD < 0)
		{
			continue;
		}

		if( FD_ISSET( iFD, &fds_read ) )
		{
			m_vConnection[i].RecvAllData();
			while( m_vConnection[i].GetOneMsg( nMsgLength, (char*)&bMsgBuf[ 0 ] ) > 0 )
			{
				int iRet = DispatchOneMsgFromConnect( i, nMsgLength, bMsgBuf, pCommand, i );
				if ( iRet )
				{
					Log_Msg( Log_Debug, "Communication: DispatchOneMsgFromDBSwitch a msg error iRet = %d.\n", iRet );
				}

				iCount++;
			}
		}
	}

	return iCount;
}

/*---------------------------------------------------------------------------------------------------------*/
int Communication2::ServerReceiveAndDispatchConnectMsg( ICommunicationCommand* pCommand )
{
	SOCKET iFD = INVALID_SOCKET;
	int iStatus = Tcs_Closed;

	fd_set fds_read;
	FD_ZERO(&fds_read);

	int i = 0;
	for( ; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		iFD = m_vConnection[i].GetSocketFD();
		iStatus = m_vConnection[i].GetSocketStatus();

		if( iFD > 0 && iStatus == Tcs_Connected )
		{
			FD_SET( iFD, &fds_read );
		}
	}

	struct timeval oSelectTime;
	oSelectTime.tv_sec = 0;
	oSelectTime.tv_usec = 1000;

	int iOpenFDNum = select( FD_SETSIZE, &fds_read, NULL, NULL, &oSelectTime );
	if( iOpenFDNum <= 0 )
	{
		return 0;
	}

	int iCount = 0;
	short nMsgLength;
	BYTE  bMsgBuf[MSG_BUFFER_LENGTH];

	for( i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		iFD = m_vConnection[i].GetSocketFD();
		if(iFD < 0)
		{
			continue;
		}

		if( FD_ISSET( iFD, &fds_read ) )
		{
			m_vConnection[i].RecvAllData();
			while( m_vConnection[i].GetOneMsg( nMsgLength, (char*)&bMsgBuf[ 0 ] ) > 0 )
			{
				int iRet = ServerDispatchOneMsgFromConnect( i, nMsgLength, bMsgBuf, pCommand, i );
				if ( iRet )
				{
					Log_Msg( Log_Debug, "Communication: DispatchOneMsgFromDBSwitch a msg error iRet = %d.\n", iRet );
				}

				iCount++;
			}
		}
	}

	return iCount;
}

/*---------------------------------------------------------------------------------------------------------*/
int Communication2::DispatchOneMsgFromConnect( int iDBSwitchServerId, short nMsgLength, BYTE* pMsg, ICommunicationCommand* pCommand, int iConnect )
{
	int iRet = 0;

	if( iDBSwitchServerId < 0 || iDBSwitchServerId >= m_oConfig.m_iConnectCount || nMsgLength <= 0 || !pMsg )
	{
		Log_Msg( Log_Debug, "Communication: Invalid input in Communication2::DispatchOneMsgFromConnect.\n" );
		return -1;
	}

	BYTE* pTempMsg = pMsg;
	SwitchMsgHead oSwitchHead;

	if( oSwitchHead.DecodeMsg( pTempMsg, nMsgLength ) )
	{
		Log_Msg( Log_Debug, "Communication: Error Msg, can't decode SwitchMsgHead.\n" );
		return -2;
	}

	if( Cst_SwitchServer == oSwitchHead.m_nDstFE )
	{
		// process heartbeat
		if( ( sizeof( BYTE ) == oSwitchHead.m_nOptionLength ) && ( COMMAND_KEEPALIVE == oSwitchHead.m_vOption[0]) )
		{
			time( &m_vLastKeepAliveTime[iDBSwitchServerId] );
			return 0;
		}
	}

	if ( oSwitchHead.m_nBodyLength <= 0 )
	{
		return 0;
	}

	short nHeadLength = oSwitchHead.m_nTotalLength - oSwitchHead.m_nBodyLength;
	BYTE* pTemp = pMsg + nHeadLength;

	NetMsg oNetMsg( ( void* )m_szInMsgBuf );

	iRet = oNetMsg.DecodeMsg( pTemp, oSwitchHead.m_nBodyLength );
	if ( iRet )
	{
		Log_Msg( Log_Debug, "Communication: ConvertDBCacheBufToMsg error ret = %d.\n", iRet );
		return -3;
	}

	if ( pCommand )
	{
		iRet = pCommand->Execute( &oSwitchHead, &oNetMsg, 0, iConnect);
	}

	return iRet;
}
/*---------------------------------------------------------------------------------------------------------*/
int Communication2::ServerDispatchOneMsgFromConnect( int iDBSwitchServerId, short nMsgLength, BYTE* pMsg, ICommunicationCommand* pCommand, int iConnect )
{
	int iRet = 0;

	if( iDBSwitchServerId < 0 || iDBSwitchServerId >= m_oConfig.m_iConnectCount || nMsgLength <= 0 || !pMsg )
	{
		Log_Msg( Log_Debug, "Communication: Invalid input in Communication2::ServerDispatchOneMsgFromConnect.\n" );
		return -1;
	}

	BYTE* pTempMsg = pMsg;
	SwitchMsgHead oSwitchHead;

	if( oSwitchHead.DecodeMsg( pTempMsg, nMsgLength ) )
	{
		Log_Msg( Log_Debug, "Communication: Error Msg, can't decode SwitchMsgHead.\n" );
		return -2;
	}

	if( Cst_SwitchServer == oSwitchHead.m_nDstFE )
	{
		// process heartbeat
		if( ( sizeof( BYTE ) == oSwitchHead.m_nOptionLength ) && ( COMMAND_KEEPALIVE == oSwitchHead.m_vOption[0]) )
		{
			time( &m_vLastKeepAliveTime[iDBSwitchServerId] );
			return 0;
		}
	}

	if ( oSwitchHead.m_nBodyLength <= 0 )
	{
		return 0;
	}

	short nHeadLength = oSwitchHead.m_nTotalLength - oSwitchHead.m_nBodyLength;
	BYTE* pTemp = pMsg + nHeadLength;

	if ( pCommand )
	{
		iRet = pCommand->Execute( &oSwitchHead, pMsg/*pTemp*/, nMsgLength, iConnect );
	}

	return iRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int Communication2::RegisterToServer( TcpConnection* pTcpConnect, short sSrcType, short sScrId, short sDstType, short sDstId )
{
	if ( !pTcpConnect )
	{
		return -1;
	}

	SwitchMsgHead oSwitchMsgHead;

	oSwitchMsgHead.m_nSrcFE = sSrcType;
	oSwitchMsgHead.m_nSrcID = sScrId;
	oSwitchMsgHead.m_nDstFE = Cst_SwitchServer;
	oSwitchMsgHead.m_stTransferInfo.nDestID = sDstId;
	oSwitchMsgHead.m_bTransferType = TT_ServerToServer;
	oSwitchMsgHead.m_nOptionLength = sizeof( BYTE );
	oSwitchMsgHead.m_vOption[0] = COMMAND_REGSERVER;
	oSwitchMsgHead.m_nBodyLength = 0;
	oSwitchMsgHead.m_nTotalLength = BASE_SWITCH_HEAD_LENGTH + sizeof( short );
	oSwitchMsgHead.m_nTotalLength += oSwitchMsgHead.m_nOptionLength;
	oSwitchMsgHead.m_nTotalLength += oSwitchMsgHead.m_nBodyLength;

	BYTE szMsgBuf[MSG_BUFFER_LENGTH];
	short nMsgLength = MSG_BUFFER_LENGTH;

	if( oSwitchMsgHead.EncodeMsg( szMsgBuf, nMsgLength ) )
	{
		Log_Msg( Log_Error, "Communication2: error register to server encode failed.\n" );
		return -1;
	}
	Log_Msg( Log_Error, "Communication2: RegisterToServer nMsgLength[%d]\n",
		nMsgLength);

	pTcpConnect->SendMsg( nMsgLength, (char*)szMsgBuf, FLAG_CONNECT_CTRL );

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int Communication2::SendKeepAliveToServer( TcpConnection* pTcpConnect, short sSrcType, short sScrId, short sDstType, short sDstId )
{
	if ( !pTcpConnect )
	{
		return -1;
	}

	SwitchMsgHead oSwitchMsgHead;

	oSwitchMsgHead.m_nSrcFE = sSrcType;
	oSwitchMsgHead.m_nSrcID = sScrId;
	oSwitchMsgHead.m_nDstFE = Cst_SwitchServer;
	oSwitchMsgHead.m_stTransferInfo.nDestID = sDstId;
	oSwitchMsgHead.m_bTransferType = TT_ServerToServer;
	oSwitchMsgHead.m_nOptionLength = sizeof( BYTE );
	oSwitchMsgHead.m_vOption[0] = COMMAND_KEEPALIVE;
	oSwitchMsgHead.m_nBodyLength = 0;
	oSwitchMsgHead.m_nTotalLength = BASE_SWITCH_HEAD_LENGTH + sizeof( short );
	oSwitchMsgHead.m_nTotalLength += oSwitchMsgHead.m_nOptionLength;
	oSwitchMsgHead.m_nTotalLength += oSwitchMsgHead.m_nBodyLength;

	BYTE szMsgBuf[MSG_BUFFER_LENGTH];
	short nMsgLength = MSG_BUFFER_LENGTH;

	if( oSwitchMsgHead.EncodeMsg( szMsgBuf, nMsgLength ) )
	{
		Log_Msg( Log_Error, "Communication2: error send alive to server encode failed.\n" );
		return -1;
	}

	pTcpConnect->SendMsg( nMsgLength, (char*)szMsgBuf, FLAG_CONNECT_CTRL );

	return 0;
}


/* ----------------------------------------------------------------------------------------------------------*/
void Communication2::ServerUpdate( ICommunicationCommand* pCommand )
{
	CheckConnectRequest();

	HandleConnectRequest();

	ServerReceiveAndDispatchConnectMsg( pCommand );

	CheckBlockMsg();
}

/*---------------------------------------------------------------------------------------------------------*/
static void TimeValMinus( timeval& tvA, timeval& tvB, timeval& tvResult )
{
	timeval tvTemp;

	if( tvA.tv_usec < tvB.tv_usec )
	{
		tvTemp.tv_usec = ( 1000000 + tvA.tv_usec ) - tvB.tv_usec;
		tvTemp.tv_sec = tvA.tv_sec - tvB.tv_sec - 1;
	}
	else
	{
		tvTemp.tv_usec = tvA.tv_usec - tvB.tv_usec;
		tvTemp.tv_sec  = tvA.tv_sec - tvB.tv_sec;
	}

	tvResult.tv_sec = tvTemp.tv_sec;
	tvResult.tv_usec = tvTemp.tv_usec;
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::CheckBlockMsg()
{
	timeval tvNow, tvGap;

	gettimeofday( &tvNow, NULL );
	TimeValMinus( tvNow, m_oLastBlockCheckTime, tvGap );
	if( tvGap.tv_sec < 1 )
	{
		return 0;
	}

	m_oLastBlockCheckTime.tv_sec = tvNow.tv_sec;
	m_oLastBlockCheckTime.tv_usec = tvNow.tv_usec;

	int i = 0;
	int iCleanCount = 0;

	for( ; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		if( m_vConnection[i].IsConnectCanRecv() )
		{
			iCleanCount += m_vConnection[i].CleanBlockQueue( MAX_CLEAN_BLOCK_MSGS );
		}
	}

	if( iCleanCount > 0 )
	{
		ThreadLogOut( TLL_Normal, "---- Clean block codes ended, %d block msgs is cleaned ----\n", iCleanCount );
	}

	return iCleanCount;
}

/*---------------------------------------------------------------------------------------------------------*/
static int SockaddrToString( sockaddr_in *pstSockAddr, char *szResult )
{
	char *pcTempIP = NULL;
	unsigned short nTempPort = 0;

	if( !pstSockAddr || !szResult )
	{
		return -1;
	}

	pcTempIP = inet_ntoa(pstSockAddr->sin_addr);

	if( !pcTempIP )
	{
		return -1;
	}

	nTempPort = ntohs(pstSockAddr->sin_port);

	sprintf(szResult, "%s:%d", pcTempIP, nTempPort);

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
void Communication2::HandleConnectRequest()
{
	time_t tCurrentTime = 0;
	time( &tCurrentTime );

	if ( tCurrentTime - m_tLastConnectCheckTime >= REQUEST_CONNECT_TIME_CHECK )
	{
		m_tLastConnectCheckTime = tCurrentTime;

		int i = m_iCurrUnregisterNum - 1;
		for( ; i >= 0; --i )
		{
			if(tCurrentTime - m_vUnregisterInfo[i].m_tAcceptTime > ACCEPT_TIME_CHECK )
			{
				int iSocketFD = m_vUnregisterInfo[i].m_iSocketFD;

				Log_Msg( Log_Warning, "Communication: Wait register timeout so close it, fd = %d.\n", iSocketFD );

				DeleteOneUnregisterConn( i );
				closesocket( iSocketFD );
			}
		}

	}
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::CheckConnectRequest()
{
	timeval tvListenTime;
	tvListenTime.tv_sec = 0;
	tvListenTime.tv_usec = 1000;

	fd_set fds_read;
	FD_ZERO( &fds_read );

	int iListenSocketFD = -1;
	int iMaxSocketFD = -1;

	iListenSocketFD = m_stListenSocket.GetSocketFD();
	FD_SET( iListenSocketFD, &fds_read );
	iMaxSocketFD = iListenSocketFD;

	int i = 0;
	for( ; i < m_iCurrUnregisterNum; ++i )
	{
		FD_SET( m_vUnregisterInfo[i].m_iSocketFD, &fds_read );
		if( m_vUnregisterInfo[i].m_iSocketFD > iMaxSocketFD )
		{
			iMaxSocketFD = m_vUnregisterInfo[i].m_iSocketFD;
		}
	}

	int iTmp = select( iMaxSocketFD + 1, &fds_read, 0, 0, &tvListenTime );
	if( iTmp <= 0 )
	{
		if( iTmp < 0 )
		{
			Log_Msg( Log_Error, "Communication: CheckConnectRequest select error, %s.\n", strerror( errno ) );
		}
		return iTmp;
	}
	
	int iNewSocketFD = -1;
	struct sockaddr_in stConnectAddr;
	socklen_t iAddrLength = sizeof( stConnectAddr );
	char szConnectAddr[32];

	// check listen socket
	if ( FD_ISSET( iListenSocketFD, &fds_read ) )
	{
		iNewSocketFD = accept( iListenSocketFD, ( struct sockaddr* )&stConnectAddr, &iAddrLength );

		SockaddrToString( &stConnectAddr, (char *)szConnectAddr );

		if( m_iCurrUnregisterNum >= MAX_UNREGISTER_NUM )
		{
			Log_Msg( Log_Error, "Communication: error there is no empty space(cur num: %d) to record.\n", m_iCurrUnregisterNum );
			closesocket( iNewSocketFD );
		}
		else
		{
			m_vUnregisterInfo[m_iCurrUnregisterNum].m_iSocketFD = iNewSocketFD;
			m_vUnregisterInfo[m_iCurrUnregisterNum].m_uIP = stConnectAddr.sin_addr.s_addr;
			m_vUnregisterInfo[m_iCurrUnregisterNum].m_tAcceptTime = time( 0 );
			m_iCurrUnregisterNum++;
		}
	}

	// check unregister connect
	for(i = m_iCurrUnregisterNum-1; i >= 0; --i)
	{
		if ( FD_ISSET( m_vUnregisterInfo[i].m_iSocketFD, &fds_read ) )
		{
			ReceiveAndRegister( i );
		}
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int Communication2::ReceiveAndRegister( int iUnregisterIndex )
{
	if( iUnregisterIndex < 0 || iUnregisterIndex >= m_iCurrUnregisterNum )
	{
		Log_Msg( Log_Error, "Communication: Error in ReceiveAndRegister, Unregister index(%d) is invalid\n", iUnregisterIndex );
		return -1;
	}

	int iSocketFD = m_vUnregisterInfo[iUnregisterIndex].m_iSocketFD;
	ULONG ulIPAddr = m_vUnregisterInfo[iUnregisterIndex].m_uIP;

	BYTE szTmpBuf[17] = { 0 };
	int iRecvedBytes = recv( iSocketFD, (char*)szTmpBuf, sizeof( szTmpBuf ), 0 );
	if( iRecvedBytes == 0 )
	{
		Log_Msg( Log_Error, "Communication: The remote site may closed this connect, fd = %d, errno = %d.\n", iSocketFD, errno );
		DeleteOneUnregisterConn( iUnregisterIndex );
		closesocket( iSocketFD );
		return -1;
	}

	if( iRecvedBytes < 0 )
	{
		if( errno != EAGAIN )
		{
			Log_Msg( Log_Error, "Communication: Error in read conn, fd = %d, errno = %d.\n", iSocketFD, errno );
			DeleteOneUnregisterConn( iUnregisterIndex );
			closesocket( iSocketFD );
		}
		return -1;
	}

	DeleteOneUnregisterConn( iUnregisterIndex );

	if( iRecvedBytes < BASE_SWITCH_HEAD_LENGTH )
	{
		Log_Msg( Log_Error, "Communication: Error not enough len, fd = %d, recv len = %d.\n", iSocketFD, iRecvedBytes );
		closesocket( iSocketFD );
		return -1;
	}

	SwitchMsgHead stTmpSwitchHead;
	if( stTmpSwitchHead.DecodeMsg( szTmpBuf, iRecvedBytes ) )
	{
		Log_Msg( Log_Error, "Communication: Error decode switch head failed, fd = %d, recv len = %d.\n", iSocketFD, iRecvedBytes );
		closesocket( iSocketFD );
		return -1;
	}

	if( ( stTmpSwitchHead.m_nDstFE != Cst_SwitchServer ) ||( stTmpSwitchHead.m_nOptionLength <= 0 )
		|| ( stTmpSwitchHead.m_vOption[0] != COMMAND_REGSERVER ) )
	{
		Log_Msg( Log_Error, "Communication: Error switch head para is invalid, fd = %d, dstFE = %d, OptionLength = %d.\n",
			iSocketFD, stTmpSwitchHead.m_nDstFE, stTmpSwitchHead.m_nOptionLength );
		closesocket( iSocketFD );
		return -1;
	}

	TcpConnection* pAcceptConnect = GetConnectByAddrAndID( stTmpSwitchHead.m_nSrcFE, stTmpSwitchHead.m_nSrcID, ulIPAddr );
	if( pAcceptConnect == 0 )
	{
		in_addr inaddr;
		inaddr.s_addr = ulIPAddr;

		Log_Msg( Log_Error, "Communication: Source FE(%d) , ID(%d) or addr(%s) is not valid of socket fd(%d) .\n",
			stTmpSwitchHead.m_nSrcFE, stTmpSwitchHead.m_nSrcID, inet_ntoa(inaddr), iSocketFD );
		closesocket( iSocketFD );
		return -1;
	}

	int iAcceptRequest = pAcceptConnect->AcceptConnect( iSocketFD );
	if( iAcceptRequest < 0 )
	{
		if( iAcceptRequest == -2 )
		{
			Log_Msg( Log_Error, "Communication: The server had connected, ignore the conn request (fd: %d).\n", iSocketFD );
		}
		closesocket( iSocketFD );
		return -1;
	}

	{
		in_addr inaddr;
		inaddr.s_addr = pAcceptConnect->GetConnectAddr();

		Log_Msg( Log_Normal, "Communication: Connect(%d, %d)(IP=%s) connected OK, the FD is %d.\n",
			pAcceptConnect->GetType(), pAcceptConnect->GetID(), inet_ntoa( inaddr ), iSocketFD );
	}

	pAcceptConnect->SetConnectState( Connect_ON );

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::DeleteOneUnregisterConn( int iUnRegisterIndex )
{
	if( iUnRegisterIndex < 0 || iUnRegisterIndex >= m_iCurrUnregisterNum )
	{
		Log_Msg( Log_Error, "DBSwitchServer: Error in DeleteOneUnregisterConn, Unregister index(%d) is invalid\n", iUnRegisterIndex );
		return -1;
	}

	m_iCurrUnregisterNum--;
	if((m_iCurrUnregisterNum > 0) && (iUnRegisterIndex < m_iCurrUnregisterNum))
	{
		m_vUnregisterInfo[ iUnRegisterIndex ] = m_vUnregisterInfo[ m_iCurrUnregisterNum ];
	}

	return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
TcpConnection* Communication2::GetConnectByAddrAndID( short shSrcFE, short shSrcID, ULONG ulSrcAddr )
{
	TcpConnection *pTcpConnect = 0;
	int iMaxServerNumber = 0;

	int i = 0;
	for( ; i < MAX_CONNECTIONS && i < m_oConfig.m_iConnectCount ; i++ )
	{
		if ( m_vConnectInfo[i].m_ulAddr == ulSrcAddr && m_vConnectInfo[i].m_sServerId ==  shSrcID )
		{
			return &m_vConnection[i];
		}
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::FindRepeatConfig( short sServerType, short sServerId )
{
	for ( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		if ( m_vConnectInfo[i].m_sServerType == sServerType && m_vConnectInfo[i].m_sServerId == sServerId )
		{
			return 1;
		}
	}
	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::LoadConfig( const char * filename )
{
	if ( filename == NULL )
	{
		return -1;
	}

	ConfigIniFile oConfigReader;
	if (oConfigReader.OpenFile( filename ) != 0)
	{
		Log_Msg( Log_Failed, "Communication2: open file %s failed.\n", filename );
		return -2;
	}

	oConfigReader.GetStringValue("Communication2", "LocalIP", m_oConfig.m_szLocalIPAddr, Max_IPAndPort_Str_Len );
	if ( CheckIPValue( m_oConfig.m_szLocalIPAddr ) == false )
	{
		Log_Msg( Log_Failed, "Communication2: invalid element(%s/%s=%s) in config file %s.\n", "Communication", "BindIP", m_oConfig.m_szLocalIPAddr, filename );
		oConfigReader.CloseFile();
		return -3;
	}

	oConfigReader.GetStringValue( "Communication2", "LocalPort", m_oConfig.m_szLocalPort, Max_IPAndPort_Str_Len );
	if ( CheckPortValue( atoi( m_oConfig.m_szLocalPort ) ) == false )
	{
		Log_Msg( Log_Failed, "Communication2: invalid element(%s/%s=%u) in config file %s.\n", "Communication", "BindPort", atoi( m_oConfig.m_szLocalPort ), filename );
		oConfigReader.CloseFile();
		return -4;
	}

	int sectionCount = 0;
	oConfigReader.GetIntValue("Communication2", "ConnectCount", m_oConfig.m_iConnectCount );
	for ( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		char secname[32] = {0};
		snprintf(secname, sizeof(secname), "Connect_%d", i + 1);

		char ipstr[Max_IPAndPort_Str_Len] = {0};
		oConfigReader.GetStringValue( secname, "ConnectIP", ipstr, Max_IPAndPort_Str_Len );
		if ( CheckIPValue(ipstr) == false )
		{
			Log_Msg( Log_Failed, "Communication: invalid element(%s/%s=%s) in config file %s.\n", secname, "ConnectIP", ipstr, filename );
			oConfigReader.CloseFile();
			return -5;
		}
		
		int port = 0;
		oConfigReader.GetIntValue( secname, "ConnectPort", port );
		if ( CheckPortValue(port) == false )
		{
			Log_Msg( Log_Failed, "Communication: invalid element(%s/%s=%u) in config file %s.\n", secname, "ConnectPort", port, filename );
			oConfigReader.CloseFile();
			return -6;
		}

		int iServerType = 0;
		oConfigReader.GetIntValue( secname, "ConnectServerType", iServerType );
		if ( iServerType < Cst_MainServer || iServerType >= Cst_ServerMax )
		{
			Log_Msg( Log_Failed, "Communication: invalid element(%s/%s=%u) in config file %s.\n", secname, "ConnectServerType", iServerType, filename );
			oConfigReader.CloseFile();
			return -7;
		}
		
		int iServerId = 0;
		oConfigReader.GetIntValue( secname, "ConnectServerId", iServerId );
		if ( iServerId <= 0 || iServerId >= 32000 )
		{
			Log_Msg( Log_Failed, "Communication: invalid element(%s/%s=%u) in config file %s.\n", secname, "ConnectServerType", iServerId, filename );
			oConfigReader.CloseFile();
			return -8;
		}

		int iRepeat = FindRepeatConfig( (short)iServerType, (short)iServerId );
		if ( iRepeat )
		{
			Log_Msg( Log_Failed, "Communication: repeat server config(type=%d id=%d) in config file %s.\n",  iServerType, iServerId, filename );
			oConfigReader.CloseFile();
			return -8;
		}

		m_vConnectInfo[i].m_iTimestamp	= 0;
		m_vConnectInfo[i].m_usPort		= ( unsigned short )( port );
		m_vConnectInfo[i].m_ulAddr		= inet_addr( ipstr );
		m_vConnectInfo[i].m_sServerType	= ( short )iServerType;
		m_vConnectInfo[i].m_sServerId	= ( short )iServerId;
	}
	
	oConfigReader.CloseFile();

	return 0;
}


/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::SingleSendMsg( unsigned char* pBuffer, int iSize, void* pSender, int iFlag )
{
	TcpConnection* pConnect = ( TcpConnection* )pSender;
	if ( !pConnect )
	{
		return -1;
	}

	int ret = -1;
	ret = pConnect->SendMsg( (short)iSize, (char*)pBuffer, iFlag );

	return ret;
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::SendMsgToGroup( NetMsgBody * pMsg, int iSrvType, unsigned char bCommand /* = 0 */ )
{
	unsigned char szMsgBuf[MSG_BUFFER_LENGTH];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	NetMsg oNetMsg( pMsg );
	oNetMsg.m_oHead.m_sMsgID = ( short )( pMsg->GetType() );
	if (oNetMsg.EncodeMsg( szMsgBuf, sMsgBufLength ) != 0)
	{
		return -1;
	}
	
	return this->SendMsgToGroup( szMsgBuf, sMsgBufLength, iSrvType, bCommand );
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::SendMsgToGroup( unsigned char * pMsg, short sMsgLen, int iSrvType, unsigned char bCommand /* = 0 */ )
{
	Log_Msg(Log_Debug, "Communication: Send to Group: type:%d\n", iSrvType);

	if ( !pMsg || sMsgLen <= 0 )
	{
		return -2;
	}

	unsigned char szMsgBuf[ MSG_BUFFER_LENGTH+100 ];
	short sMsgBufLength = MSG_BUFFER_LENGTH+100;

	SwitchMsgHead oSwitchMsgHead;

	oSwitchMsgHead.m_nSrcFE					= ( short )( m_iServerType );
	oSwitchMsgHead.m_nSrcID					= ( short )( m_iServerID );
	oSwitchMsgHead.m_nDstFE					= ( short )( iSrvType );
	oSwitchMsgHead.m_bTransferType			= TT_ServerToGroup;
	if ( bCommand != 0 )
	{
		oSwitchMsgHead.m_nOptionLength		= 0;
	}
	else
	{
		oSwitchMsgHead.m_nOptionLength		= sizeof( unsigned char );
		oSwitchMsgHead.m_bOptionFlag		= (BYTE)0x01;
	}
	oSwitchMsgHead.m_nBodyLength			= sMsgLen;

	if ( oSwitchMsgHead.EncodeMsg( szMsgBuf, sMsgBufLength ) )
	{
		return -3;
	}

	memcpy( szMsgBuf + sMsgBufLength, pMsg, sMsgLen );
	int iTotalLen = sMsgBufLength + sMsgLen;

	for ( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		if (m_vConnectInfo[i].m_sServerType == ( short )iSrvType )
		{
			int iRet = ( this->*m_pSendMsgFunc )( szMsgBuf, iTotalLen, ( void* )( &m_vConnection[i] ), bCommand != 0 ? FLAG_CONNECT_IGNORE : FLAG_CONNECT_REDO );
			if ( iRet != 0 )
			{
				Log_Msg( Log_Normal, "Communication: SendMsgToServer id=%d  fail iRet=%d\n",
					m_vConnectInfo[i].m_sServerId,
					iRet);
			}
		}
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::SendMsgToServer( NetMsgBody * pMsg, int iSrvType, int iSrvId, unsigned char bCommand /* = 0 */ )
{
	unsigned char szMsgBuf[MSG_BUFFER_LENGTH];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	NetMsg oNetMsg( pMsg );
	oNetMsg.m_oHead.m_sMsgID = ( short )( pMsg->GetType() );
	if (oNetMsg.EncodeMsg( szMsgBuf, sMsgBufLength ) != 0)
	{
		return -1;
	}

	return SendMsgToServer( szMsgBuf, sMsgBufLength, iSrvType, iSrvId, bCommand );
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::SendMsgToServer( unsigned char * pMsg, short sMsgLen, int iSrvType, int iSrvId, unsigned char bCommand /* = 0 */ )
{
	//Log_Msg( Log_Debug, "Communication: Send to Server: type:%d id:%d\n", iSrvType, iSrvId );

	if ( !pMsg || sMsgLen <= 0 )
	{
		return -2;
	}

	unsigned char szMsgBuf[ MSG_BUFFER_LENGTH+100 ];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	SwitchMsgHead oSwitchMsgHead;

	oSwitchMsgHead.m_nSrcFE					= ( short )( m_iServerType );
	oSwitchMsgHead.m_nSrcID					= ( short )( m_iServerID );
	oSwitchMsgHead.m_nDstFE					= ( short )( iSrvType );
	oSwitchMsgHead.m_bTransferType			= TT_ServerToServer;
	oSwitchMsgHead.m_stTransferInfo.nDestID = (short)iSrvId;
	if ( bCommand != 0 )
	{
		oSwitchMsgHead.m_nOptionLength		= 0;
	}
	else
	{
		oSwitchMsgHead.m_nOptionLength		= sizeof( unsigned char );
		oSwitchMsgHead.m_bOptionFlag		= (BYTE)0x01;
	}
	oSwitchMsgHead.m_nBodyLength			= sMsgLen;

	if ( oSwitchMsgHead.EncodeMsg( szMsgBuf, sMsgBufLength ) )
	{
		return -3;
	}

	memcpy( szMsgBuf + sMsgBufLength, pMsg, sMsgLen );
	int iTotalLen = sMsgBufLength + sMsgLen;

	for ( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		if (m_vConnectInfo[i].m_sServerType == ( short )iSrvType
			&& m_vConnectInfo[i].m_sServerId == ( short )iSrvId)
		{
			int iRet = ( this->*m_pSendMsgFunc )( szMsgBuf, iTotalLen, ( void* )( &m_vConnection[i] ), bCommand != 0 ? FLAG_CONNECT_IGNORE : FLAG_CONNECT_REDO );
			if ( iRet != 0 )
			{
				return iRet - 4;
			}
			else
			{
				return 0;
			}
		}
	}

	return -5;
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::SendMsgByUID( NetMsgBody * pMsg, int iSrvType, int iSrvId, unsigned int unUid, unsigned char bCommand /* = 0 */ )
{
	unsigned char szMsgBuf[MSG_BUFFER_LENGTH];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	NetMsg oNetMsg( pMsg );
	oNetMsg.m_oHead.m_sMsgID = ( short )( pMsg->GetType() );
	if (oNetMsg.EncodeMsg( szMsgBuf, sMsgBufLength ) != 0)
	{
		return -1;
	}

	return SendMsgByUID( szMsgBuf, sMsgBufLength, iSrvType, iSrvId, unUid, bCommand );
}

/* ----------------------------------------------------------------------------------------------------------*/
int Communication2::SendMsgByUID( unsigned char * pMsg, short sMsgLen, int iSrvType, int iSrvId, unsigned int unUid, unsigned char bCommand /* = 0 */ )
{
	//Log_Msg(Log_Debug, "Communication: Send to Server: type:%d\n", iSrvType);

	if ( !pMsg || sMsgLen <= 0 )
	{
		return -2;
	}

	unsigned char szMsgBuf[ MSG_BUFFER_LENGTH+100 ];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	SwitchMsgHead oSwitchMsgHead;

	oSwitchMsgHead.m_nSrcFE					= ( short )( m_iServerType );
	oSwitchMsgHead.m_nSrcID					= ( short )( m_iServerID );
	oSwitchMsgHead.m_nDstFE					= ( short )( iSrvType );
	oSwitchMsgHead.m_bTransferType			= TT_ByUID;
	oSwitchMsgHead.m_stTransferInfo.unUid	= unUid;
	if ( bCommand != 0 )
	{
		oSwitchMsgHead.m_nOptionLength		= 0;
	}
	else
	{
		oSwitchMsgHead.m_nOptionLength		= sizeof( unsigned char );
		oSwitchMsgHead.m_bOptionFlag		= (BYTE)0x01;
	}
	oSwitchMsgHead.m_nBodyLength			= sMsgLen;

	if ( oSwitchMsgHead.EncodeMsg( szMsgBuf, sMsgBufLength ) )
	{
		return -3;
	}

	memcpy( szMsgBuf + sMsgBufLength, pMsg, sMsgLen );
	int iTotalLen = sMsgBufLength + sMsgLen;

	for ( int i = 0; i < m_oConfig.m_iConnectCount && i < MAX_CONNECTIONS; i++ )
	{
		if (m_vConnectInfo[i].m_sServerType == ( short )iSrvType )
		{
			int iRet = ( this->*m_pSendMsgFunc )( szMsgBuf, iTotalLen, ( void* )( &m_vConnection[i] ), bCommand != 0 ? FLAG_CONNECT_IGNORE : FLAG_CONNECT_REDO );
			if ( iRet != 0 )
			{
				return iRet - 3;
			}
		}
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------
   End of file */
