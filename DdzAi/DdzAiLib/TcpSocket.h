/* ----------------------------------------------------------------------------------------------------------




-----------------------------------------------------------------------------------------------------------*/
#ifndef __TcpSocket_h__
#define __TcpSocket_h__

#include "PopWorldDefine.h"
#include "PopWorldInclude.h"

#define	RECV_BUF_LENGTH	(1024*1024)
#define	SEND_BUF_LENGTH	(1024*1024)

enum eSocketTypes
{
	St_Listen = 0,
	St_Common,
};

enum eTCPConnectStates
{
	Tcs_Closed = 0,
	Tcs_Opened,
	Tcs_Connected,
	Tcs_Error,
};

enum eRecvError
{
	ERR_RECV_NOSOCK = -1,
	ERR_RECV_NOBUFF = -2,
	ERR_RECV_REMOTE = -3,
	ERR_RECV_FALIED = -4,
};

enum eSendError
{
	ERR_SEND_NOSOCK = -1,
	ERR_SEND_NOBUFF = -2,
	ERR_SEND_FAILED = -3,
};

class TcpSocket
{
public:
	TcpSocket();
	virtual ~TcpSocket();

	SOCKET		GetSocketFD();
	int			GetStatus();

	int			CreateClient( const char* szIPAddr = 0 );
	int			ConnectTo( const char* szIPAddr, unsigned short uPort );
	int			ConnectTo( ULONG uIPAddr, USHORT uPort );

	int			CreateServer( unsigned short unPort, const char* szIPAddr = 0 );
	int			Accept( SOCKET iAcceptFD );

	int			RecvData();
	int			GetOneMsg( short& nMsgLength, char * pMsg );
	int			SendOneMsg( short nMsgLength, char * pMsg );
	int			GetOneMsgWithSmallOrder( short& nMsgLength, char * pMsg );

	int			AddToCheckSet( fd_set* pCheckSet );
	int			IsFDSetted( fd_set* pCheckSet );

	int			HasReserveData();
	int			CleanReserveData();

	void		Shutdown();

private:
	void		Close();
	
	SOCKET		m_iSocketFD;
	int			m_iSocketType;
	int			m_iStatus;

	int			m_iReadBegin;
	int			m_iReadEnd;
	char *		m_vRecvBuffer;

	int			m_iSendBegin;
	int			m_iSendEnd;
	char *		m_vSendBuffer;
};

/*---------------------------------------------------------------------------------------------------------*/
inline SOCKET TcpSocket::GetSocketFD()
{
	return m_iSocketFD;
}

/*---------------------------------------------------------------------------------------------------------*/
inline int TcpSocket::GetStatus()
{
	return m_iStatus;
}

#endif
