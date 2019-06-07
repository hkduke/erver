/* ----------------------------------------------------------------------------------------------------------

in 2009-3-10
-----------------------------------------------------------------------------------------------------------*/
#include "PopWorldInclude.h"
#include "TcpSocket.h"
#include "Thread.h"
#include "Log.h"

/*---------------------------------------------------------------------------------------------------------*/
TcpSocket::TcpSocket()
{
	m_iSocketFD		= INVALID_SOCKET;
	m_iSocketType	= St_Common;
	m_iStatus		= Tcs_Closed;

	m_iReadBegin	= 0;
	m_iReadEnd		= 0;

	m_iSendBegin	= 0;
	m_iSendEnd		= 0;

	m_vRecvBuffer	= new char[RECV_BUF_LENGTH];
	m_vSendBuffer	= new char[SEND_BUF_LENGTH];
}

/*---------------------------------------------------------------------------------------------------------*/
TcpSocket::~TcpSocket()
{
	if ( m_iStatus == Tcs_Opened && m_iSocketFD > 0 )
	{
		Close();
	}

	if (m_vRecvBuffer != NULL)
	{
		delete[] m_vRecvBuffer;
		m_vRecvBuffer = NULL;
	}

	if (m_vSendBuffer != NULL)
	{
		delete[] m_vSendBuffer;
		m_vSendBuffer = NULL;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::CreateServer( unsigned short unPort, const char* szIPAddr )
{
	if ( m_iStatus == Tcs_Opened && m_iSocketFD > 0 )
	{
		Close();
	}

	m_iSocketType	= St_Listen;

	m_iSocketFD = socket( AF_INET, SOCK_STREAM, 0 );
	if ( m_iSocketFD < 0 )
	{
		m_iStatus = Tcs_Closed;
		m_iSocketFD = INVALID_SOCKET;
		return -1;
	}

	int iReusePortFlag = 1;
	if( setsockopt(m_iSocketFD, SOL_SOCKET, SO_REUSEADDR, (const char *)&iReusePortFlag, sizeof(int)) )
	{
		Log_Msg( Log_Error,  "TcpSocket: Set socket addr reusable failed, %s.\n", strerror( errno ) );
	}

	sockaddr_in stSockAddr;
	memset( ( void* )&stSockAddr, 0, sizeof( sockaddr_in ) );
	
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons( unPort );
	if ( szIPAddr )
	{
		stSockAddr.sin_addr.s_addr = inet_addr( szIPAddr );
	}
	else
	{
		stSockAddr.sin_addr.s_addr = htonl( INADDR_ANY ); 
	}

	if ( bind( m_iSocketFD, ( const struct sockaddr* )&stSockAddr, sizeof( stSockAddr ) ) )
	{
		Close();
		Log_Msg( Log_Failed, "TcpSocket: Bind failed, %s.\n", strerror( errno ) );
		return -1;
	}

	if( listen( m_iSocketFD, 10 ) < 0 )
	{
		Log_Msg( Log_Failed, "TcpSocket: Listen failed, %s.\n", strerror( errno ) );
		Close();
		return -1;
	}

#if defined(_LINUX) 
	// set no block
	int iFlags;
	iFlags = fcntl( m_iSocketFD, F_GETFL, 0 );
	iFlags |= O_NONBLOCK;
	iFlags |= O_NDELAY;
	fcntl( m_iSocketFD, F_SETFL, iFlags );
#elif defined(_WIN32)
	u_long non_block = 1;  
	ioctlsocket(m_iSocketFD, FIONBIO, &non_block);   
#else
#error unsupported platform.
#endif

	m_iStatus = Tcs_Opened;

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::Accept( SOCKET iAcceptFD )
{
	int iTempRet = 0;

	if( iAcceptFD < 0 )
	{
		return -1;
	}
	if( m_iSocketFD > 0 && m_iStatus == Tcs_Connected )
	{
		Log_Msg( Log_Error, "TcpSocket: Warning, another connection request from remote, close the previous(%d).\n", m_iSocketFD );
		Close();
	}

	m_iSocketFD		= iAcceptFD;
	m_iSocketType	= St_Common;
	m_iStatus		= Tcs_Connected;
	m_iReadBegin	= 0;
	m_iReadEnd		= 0;

	m_iSendBegin	= 0;
	m_iSendEnd		= 0;

	// set no block
#if defined(_LINUX)
	int iFlags;
	iFlags = fcntl( m_iSocketFD, F_GETFL, 0 );
	iFlags |= O_NONBLOCK;
	iFlags |= O_NDELAY;
	fcntl( m_iSocketFD, F_SETFL, iFlags );
#elif defined(_WIN32)
	u_long non_block = 1;  
	ioctlsocket(m_iSocketFD, FIONBIO, &non_block);
#else
#error unsupported platform.
#endif
	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::CreateClient( const char* szIPAddr )
{
	if ( m_iStatus == Tcs_Opened && m_iSocketFD > 0 )
	{
		Close();
	}

	m_iSocketType	= St_Common;

	m_iSocketFD = socket( AF_INET, SOCK_STREAM, 0 );
	if ( m_iSocketFD < 0 )
	{
		m_iStatus = Tcs_Closed;
		m_iSocketFD = INVALID_SOCKET;
		return -1;
	}

	if ( szIPAddr )
	{
		sockaddr_in stSockAddr;
		memset( ( void* )&stSockAddr, 0, sizeof( sockaddr_in ) );

		stSockAddr.sin_family = AF_INET;
		stSockAddr.sin_addr.s_addr = inet_addr( szIPAddr );
		bind( m_iSocketFD, ( const struct sockaddr* )&stSockAddr, sizeof( stSockAddr ) );
	}

	m_iStatus = Tcs_Opened;

	m_iReadBegin	= 0;
	m_iReadEnd		= 0;

	m_iSendBegin	= 0;
	m_iSendEnd		= 0;

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::ConnectTo( const char *szIPAddr, USHORT uPort )
{
	if( !szIPAddr )
	{
		return -1;
	}

	if( m_iStatus != Tcs_Opened || m_iSocketFD < 0 )
	{
		return -2;
	}

	sockaddr_in stSockAddr;
	memset( ( void* )&stSockAddr, 0, sizeof( sockaddr_in ) );
	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons( uPort );
	stSockAddr.sin_addr.s_addr = inet_addr( szIPAddr );

	if( connect(m_iSocketFD, ( const struct sockaddr* )&stSockAddr, sizeof( sockaddr_in ) ) )
	{
		Close();
		return -3;
	}

	// set no block
#if defined(_LINUX)
	int iFlags;
	iFlags = fcntl( m_iSocketFD, F_GETFL, 0 );
	iFlags |= O_NONBLOCK;
	iFlags |= O_NDELAY;
	fcntl( m_iSocketFD, F_SETFL, iFlags );
#elif defined(_WIN32)
	u_long non_block = 1;  
	ioctlsocket(m_iSocketFD, FIONBIO, &non_block);
#else
#error unsupported platform.
#endif

	m_iReadBegin	= 0;
	m_iReadEnd		= 0;
	m_iSendBegin	= 0;
	m_iSendEnd		= 0;

	m_iStatus		= Tcs_Connected;

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::ConnectTo( ULONG uIPAddr, USHORT uPort )
{

	if( m_iStatus != Tcs_Opened || m_iSocketFD < 0 )
	{
		return -2;
	}

	sockaddr_in stSockAddr;
	memset( ( void* )&stSockAddr, 0, sizeof( sockaddr_in ) );

	stSockAddr.sin_family = AF_INET;
	stSockAddr.sin_port = htons(uPort);
	stSockAddr.sin_addr.s_addr = uIPAddr;

	if( connect( m_iSocketFD, ( const struct sockaddr* )&stSockAddr, sizeof( sockaddr_in ) ) )
	{
		Close();
		return -3;
	}

	// set no block
#if defined(_LINUX)
	int iFlags;
	iFlags = fcntl( m_iSocketFD, F_GETFL, 0 );
	iFlags |= O_NONBLOCK;
	iFlags |= O_NDELAY;
	fcntl( m_iSocketFD, F_SETFL, iFlags );
#elif defined(_WIN32)
	u_long non_block = 1;  
	ioctlsocket(m_iSocketFD, FIONBIO, &non_block);    
#else
#error unsupported platform.
#endif

	m_iReadBegin	= 0;
	m_iReadEnd		= 0;
	m_iSendBegin	= 0;
	m_iSendEnd		= 0;

	m_iStatus		= Tcs_Connected;

	return 0;

}

/*---------------------------------------------------------------------------------------------------------*/
void TcpSocket::Close()
{
	if( m_iSocketFD > 0 )
	{
		closesocket( m_iSocketFD );
	}

	m_iSocketFD = INVALID_SOCKET;
	m_iStatus = Tcs_Closed;
}

/*---------------------------------------------------------------------------------------------------------*/
void TcpSocket::Shutdown()
{
	if( m_iSocketFD > 0 )
	{
		closesocket( m_iSocketFD );
	}

	m_iSocketFD = INVALID_SOCKET;
	m_iStatus = Tcs_Closed;
}

/*---------------------------------------------------------------------------------------------------------*/
static int SockAddrToString(sockaddr_in *pstSockAddr, char *szResult)
{
	char *pcTempIP = NULL;
	unsigned short nTempPort = 0;

	if( !pstSockAddr || !szResult )
	{
		return -1;
	}

	pcTempIP = inet_ntoa( pstSockAddr->sin_addr );

	if( !pcTempIP )
	{
		return -1;
	}

	nTempPort = ntohs( pstSockAddr->sin_port );

	sprintf( szResult, "%s:%d", pcTempIP, nTempPort );

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::RecvData()
{
	int iRecvedBytes	= 0;
	int iMaxBytes		= 0;
	int iTempRet		= 0;

	if( m_iSocketFD < 0 || m_iStatus != Tcs_Connected )
	{
		return ERR_RECV_NOSOCK;
	}

	if( m_iReadEnd == m_iReadBegin )
	{
		m_iReadBegin = 0;
		m_iReadEnd = 0;
	}

	//char szPeerAddr[32];
	//sockaddr stPeerAddr;
	//socklen_t iPeerAddrLen = sizeof( sockaddr ); 

	do
	{
		if( m_iReadEnd == RECV_BUF_LENGTH )
		{
			Log_Msg( Log_Error, "TcpSocket: The recv buffer is full now(%d, %d), stop recv data, fd = %d.\n", m_iReadBegin, m_iReadEnd, m_iSocketFD );

			iTempRet = ERR_RECV_NOBUFF;
			break;
		}

		iRecvedBytes = recv( m_iSocketFD, &m_vRecvBuffer[m_iReadEnd], RECV_BUF_LENGTH - m_iReadEnd, 0 );
		if( iRecvedBytes > 0 )
		{
			m_iReadEnd += iRecvedBytes;
		}
		else if( iRecvedBytes == 0 )
		{
			//getpeername( m_iSocketFD, &stPeerAddr, &iPeerAddrLen );
			//SockAddrToString( ( sockaddr_in* )&stPeerAddr, szPeerAddr );

			Log_Msg( Log_Error, "TcpSocket: The remote site closed this conn, fd = %d, errno = %d.\n", m_iSocketFD, errno );
			Close();

			iTempRet = ERR_RECV_REMOTE;

			break;
		}
		else if( errno != EAGAIN )
		{
			//getpeername( m_iSocketFD, &stPeerAddr, &iPeerAddrLen );
			//SockAddrToString( ( sockaddr_in* )&stPeerAddr, szPeerAddr );

			Log_Msg( Log_Error, "TcpSocket: Error in read, %s, socket fd = %d.\n", strerror(errno), m_iSocketFD);
			Close();

			iTempRet = ERR_RECV_FALIED;
			break;
		}
	} while( iRecvedBytes > 0 );

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::GetOneMsg( short& nMsgLength, char * pMsg )
{
	if( !pMsg )
	{
		return -1;
	}
	
	int iDataLength = m_iReadEnd - m_iReadBegin;

	if( iDataLength <= 0 )
	{
		return 0;
	}

	if( iDataLength < sizeof( short ) )
	{
		if( m_iReadEnd == RECV_BUF_LENGTH )
		{
			memcpy( ( void* )&m_vRecvBuffer[0], ( const void* )&m_vRecvBuffer[m_iReadBegin], iDataLength );
			m_iReadBegin = 0;
			m_iReadEnd = iDataLength;
		}

		return 0;
	}

	short nTempLength;
	memcpy( ( void* )&nTempLength, ( void* )&m_vRecvBuffer[m_iReadBegin], sizeof( nTempLength ) );
	nTempLength = ntohs( nTempLength );

	if( nTempLength <= 0 || nTempLength + sizeof( short ) > RECV_BUF_LENGTH )
	{
		m_iReadBegin = m_iReadEnd = 0;
		return -1;
	}
	
	if( iDataLength < nTempLength )
	{
		if( m_iReadEnd == RECV_BUF_LENGTH )
		{
			memcpy( ( void* )&m_vRecvBuffer[0], ( const void* )&m_vRecvBuffer[m_iReadBegin], iDataLength );
			m_iReadBegin = 0;
			m_iReadEnd = iDataLength;
		}

		return 0;
	}
	
	nMsgLength = nTempLength;

	memcpy( ( void* )pMsg, ( const void* )&m_vRecvBuffer[m_iReadBegin], nMsgLength );

	m_iReadBegin += nTempLength;
	if( m_iReadBegin == m_iReadEnd )
	{
		m_iReadBegin	= 0;
		m_iReadEnd		= 0;
	}

	return 1;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::GetOneMsgWithSmallOrder( short& nMsgLength, char * pMsg )
{
	if( !pMsg )
	{
		return -1;
	}

	int iDataLength = m_iReadEnd - m_iReadBegin;

	if( iDataLength <= 0 )
	{
		return 0;
	}

	if( iDataLength < sizeof( short ) )
	{
		if( m_iReadEnd == RECV_BUF_LENGTH )
		{
			memcpy( ( void* )&m_vRecvBuffer[0], ( const void* )&m_vRecvBuffer[m_iReadBegin], iDataLength );
			m_iReadBegin = 0;
			m_iReadEnd = iDataLength;
		}

		return 0;
	}

	short nTempLength;
	memcpy( ( void* )&nTempLength, ( void* )&m_vRecvBuffer[m_iReadBegin], sizeof( nTempLength ) );

	if( nTempLength <= 0 || nTempLength + sizeof( short ) > RECV_BUF_LENGTH )
	{
		m_iReadBegin = m_iReadEnd = 0;
		return -1;
	}

	if( iDataLength < nTempLength )
	{
		if( m_iReadEnd == RECV_BUF_LENGTH )
		{
			memcpy( ( void* )&m_vRecvBuffer[0], ( const void* )&m_vRecvBuffer[m_iReadBegin], iDataLength );
			m_iReadBegin = 0;
			m_iReadEnd = iDataLength;
		}

		return 0;
	}

	nMsgLength = nTempLength;

	memcpy( ( void* )pMsg, ( const void* )&m_vRecvBuffer[m_iReadBegin], nMsgLength );

	m_iReadBegin += nTempLength;
	if( m_iReadBegin == m_iReadEnd )
	{
		m_iReadBegin	= 0;
		m_iReadEnd		= 0;
	}

	return 1;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::SendOneMsg( short nMsgLength, char * pMsg )
{
	if( !pMsg || m_iSocketFD < 0 || m_iStatus != Tcs_Connected )
	{
		return ERR_SEND_NOSOCK;
	}

	// if have left send data
	int iBytesLeft = m_iSendEnd - m_iSendBegin;
	char * pTemp = &( m_vSendBuffer[m_iSendBegin] );

	int iByteSend = 0;
	int iTempRet = 0;
	while( iBytesLeft > 0 )
	{
		iByteSend = send(m_iSocketFD, (const char *)pTemp, iBytesLeft, 0);
		if( iByteSend > 0 )
		{
			pTemp			+= iByteSend;
			iBytesLeft		-= iByteSend;
			m_iSendBegin	+= iByteSend;
		}

		if( iByteSend < 0 && errno != EAGAIN )
		{
			m_iStatus	= Tcs_Error;
			iTempRet	= ERR_SEND_FAILED;
			break;
		}
		else if( iByteSend < 0 )
		{
			iTempRet	= ERR_SEND_NOBUFF;
			break;
		}
	}

	if( iBytesLeft == 0 )
	{
		// left data success, continue send data
		m_iSendBegin	= 0;
		m_iSendEnd		= 0;
	}
	else
	{
		// return
		return iTempRet;
	}
	
	// send data
	iBytesLeft = nMsgLength;
	pTemp = pMsg;

	while( iBytesLeft > 0 )
	{
		iByteSend = send( m_iSocketFD, ( const char* )pTemp, iBytesLeft, 0 );

		if( iByteSend > 0 )
		{
			pTemp += iByteSend;
			iBytesLeft -= iByteSend;
		}

		if( iByteSend < 0 && errno != EAGAIN )
		{
			m_iStatus = Tcs_Error;
			iTempRet = ERR_SEND_FAILED;

			memcpy( ( void* )&( m_vSendBuffer[m_iSendEnd] ), (const void *)pTemp, iBytesLeft );
			m_iSendEnd += iBytesLeft;

			break;
		}
		else if( m_iSendEnd < 0 )
		{
			// socket buffer is full, take left data into buffer
			memcpy( ( void* )&( m_vSendBuffer[m_iSendEnd] ), (const void *)pTemp, iBytesLeft );
			m_iSendEnd += iBytesLeft;
			iTempRet = 0;
			break;
		}
	}

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::AddToCheckSet( fd_set *pCheckSet )
{
	int iTempRet = 0;

	if( !pCheckSet )
	{
		return -1;
	}

	if( m_iSocketFD > 0 && m_iStatus == Tcs_Connected )
	{
		FD_SET( m_iSocketFD, pCheckSet );
	}
	else if( m_iSocketFD > 0 )
	{
		Close();
		iTempRet = -2;
	}

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::IsFDSetted( fd_set *pCheckSet )
{
	int iTempRet = 0;

	if( !pCheckSet )
	{
		return 0;
	}

	if( m_iSocketFD > 0 && m_iStatus == Tcs_Connected )
	{
		iTempRet = FD_ISSET( m_iSocketFD, pCheckSet );
	}
	else
	{
		iTempRet = 0;
	}

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::HasReserveData()
{
	if (m_iSendEnd - m_iSendBegin > 0 )
	{
		return 1;
	}
	else
	{
		return 0;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpSocket::CleanReserveData()
{
	if( m_iSocketFD < 0 || m_iStatus != Tcs_Connected )
	{
		return ERR_SEND_NOSOCK;
	}

	int iByteSend		= 0;
	int iByteLeft		= 0;
	int iByteCleaned	= 0;
	int iTempRet		= 0;
	char *pTemp			= 0;
	
	iByteLeft = m_iSendEnd - m_iSendBegin;
	pTemp = &( m_vSendBuffer[m_iSendBegin] );
	while( iByteLeft > 0 )
	{
		iByteSend = send( m_iSocketFD, ( const char* )pTemp, iByteLeft, 0 );
		if( iByteSend > 0 )
		{
			pTemp		+= iByteSend;
			iByteLeft	-= iByteSend;
			m_iSendBegin += iByteSend;
			iByteCleaned += iByteSend;
		}

		if( iByteSend < 0 && errno != EAGAIN )
		{
			m_iStatus	= Tcs_Error;
			iTempRet	= ERR_SEND_FAILED;
			break;
		}
		else if( iByteSend < 0 )
		{
			iTempRet = ERR_SEND_NOBUFF;
			break;
		}
	}

	if( iByteLeft == 0 )
	{
		m_iSendBegin	= 0;
		m_iSendEnd		= 0;
		iTempRet		= 0;
	}

	Log_Msg( Log_Debug, "TcpSocket: %d bytes is cleaned, left %d bytes.\n", iByteCleaned, iByteLeft );
	return iTempRet;
}


/* ----------------------------------------------------------------------------------------------------------
End of file */
