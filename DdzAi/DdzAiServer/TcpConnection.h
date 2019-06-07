/* ----------------------------------------------------------------------------------------------------------



in 2009-3-10
-----------------------------------------------------------------------------------------------------------*/
#ifndef __TcpConnection_h__
#define __TcpConnection_h__

#include "PopWorldDefine.h"
#include "TcpSocket.h"


class FMsgQueue;

enum eBlockStates
{
	BLK_EMPTY    = 0,
	BLK_SAVING   = 1,
	BLK_REDOING  = 2
};

enum eConnectStates
{
	Connect_OFF  = 0,
	Connect_ON   = 1
};

enum eConnectFlags
{
	FLAG_CONNECT_IGNORE = 0,
	FLAG_CONNECT_REDO	= 1,
	FLAG_CONNECT_CTRL	= 2,
};


class TcpConnection
{
public:
	TcpConnection();
	virtual ~TcpConnection();

	enum eConnectParas
	{
#ifdef _LDEBUG_
		BLOCKQUEUESIZE = 0x2000,
#else
		BLOCKQUEUESIZE = 0x1000000,
#endif
	};

	void	Initialize( short nType, short nID, ULONG uIPAddr, USHORT uPort );
	int		ConnectToServer( const char* szLocalAddr = 0 );

	int		GetConnectInfo( short* pType, short* pID, ULONG* puIpAddr );
	ULONG	GetConnectAddr();
	USHORT	GetConnectPort();
	short	GetType();
	short	GetID();

	int		AcceptConnect( int iAcceptFD );
	int		IsConnectCanRecv();

	int		RegToCheckSet( fd_set *pCheckSet );
	int		IsFDSetted( fd_set *pCheckSet );

	int		RecvAllData();
	int		GetOneMsg( short& nMsgLength, char * pMsg  );
	int		SendMsg( short nMsgLength, char * pMsg, int iFlag = FLAG_CONNECT_IGNORE );

	int		CleanBlockQueue( int iQueueLength );

	void	SetConnectState( int iConnectState );

	int		GetSocketStatus();
	SOCKET	GetSocketFD();

private:
	TcpSocket			m_oSocket;

	short				m_sType;
	short				m_sID;
	ULONG				m_ulIPAddr;
	USHORT				m_usPort;

	int					m_iConnectState;

	FMsgQueue*			m_pBlockQueue;
	FMsgQueue*			m_pRedoQueue;
	int					m_iCurrentDumpSeq;
	int					m_iCurrentRedoSeq;
	int					m_iBlockStatus;

#ifdef _POSIX_MT_
	pthread_mutex_t		m_stMutex;
#endif
};

/*---------------------------------------------------------------------------------------------------------*/
inline int TcpConnection::GetConnectInfo( short* pType, short* pID, ULONG* puIpAddr )
{
	if ( !pType || !pID || !puIpAddr )
	{
		return -1;
	}

	*pType		= m_sType;
	*pID		= m_sID;
	*puIpAddr	= m_ulIPAddr;

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
inline ULONG TcpConnection::GetConnectAddr()
{
	return m_ulIPAddr;
}

/*---------------------------------------------------------------------------------------------------------*/
inline USHORT TcpConnection::GetConnectPort()
{
	return m_usPort;
}

/*---------------------------------------------------------------------------------------------------------*/
inline short TcpConnection::GetType()
{
	return m_sType;
}

/*---------------------------------------------------------------------------------------------------------*/
inline short TcpConnection::GetID()
{
	return m_sID;
}


#endif
