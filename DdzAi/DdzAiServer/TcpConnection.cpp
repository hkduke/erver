/* ----------------------------------------------------------------------------------------------------------

in 2009-3-10
-----------------------------------------------------------------------------------------------------------*/
#include "PopWorldInclude.h"
#include "TcpConnection.h"
#include "Log.h"
#include "FMsgQueue.h"
#include "Thread.h"
#include "../DdzAiPublic/include/MsgDefine.h"
/*---------------------------------------------------------------------------------------------------------*/
TcpConnection::TcpConnection()
{
	m_ulIPAddr			= 0;
	m_sID				= -1;
	m_sType				= -1;

	m_iConnectState		= Connect_OFF;

	m_iBlockStatus		= BLK_EMPTY;
	m_iCurrentDumpSeq	= 0;
	m_iCurrentRedoSeq	= 0;
	m_pBlockQueue		= 0;
	m_pRedoQueue		= 0;

#ifdef _POSIX_MT_
	pthread_mutex_init( &m_stMutex, NULL );
#endif
}

/*---------------------------------------------------------------------------------------------------------*/
TcpConnection::~TcpConnection()
{
	if( m_pBlockQueue )
	{
		delete m_pBlockQueue;
		m_pBlockQueue = 0;
	}
	if( m_pRedoQueue )
	{
		delete m_pRedoQueue;
		m_pRedoQueue = 0;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
void TcpConnection::Initialize( short nType, short nID, ULONG uIPAddr, USHORT uPort )
{
	m_ulIPAddr	= uIPAddr;
	m_usPort	= uPort;
	m_sID		= nID;
	m_sType		= nType;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::ConnectToServer( const char* szLocalAddr )
{
	if ( m_sID < 0 )
	{
		return -1;
	}

	if ( m_oSocket.CreateClient( szLocalAddr ) )
	{
		Log_Msg( Log_Failed, "TcpConnection: Connect create client %d failed.\n", m_sID );
		return -2;
	}

	return m_oSocket.ConnectTo( m_ulIPAddr, m_usPort );
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::AcceptConnect( int iAcceptFD )
{
	int iTempRet = 0;

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	iTempRet = m_oSocket.Accept( iAcceptFD );

	m_iConnectState = Connect_OFF;

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::IsConnectCanRecv()
{
	int iTempRet = 0;

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif
	if( m_oSocket.GetSocketFD() > 0 && m_oSocket.GetStatus() == Tcs_Connected )
	{
		iTempRet = 1;
	}
	else
	{
		iTempRet = 0;
	}
#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::RegToCheckSet(fd_set *pCheckSet)
{
	int iTempRet = 0;

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	iTempRet = m_oSocket.AddToCheckSet( pCheckSet );

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::IsFDSetted(fd_set *pCheckSet)
{
	int iTempRet = 0;

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	iTempRet = m_oSocket.IsFDSetted( pCheckSet );

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::RecvAllData()
{
	int iTempRet = 0;

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	iTempRet = m_oSocket.RecvData();

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::GetOneMsg( short &nMsgLength, char *pMsg )
{
	return m_oSocket.GetOneMsg( nMsgLength, pMsg );
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::SendMsg( short nMsgLength, char *pMsg, int iFlag )
{
	int iTempRet = 0;
	
#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	if( m_sType < 0 || m_sID < 0 )
	{
#ifdef _POSIX_MT_
		pthread_mutex_unlock( &m_stMutex );
#endif
		return -1;
	}
	
	if( iFlag == FLAG_CONNECT_CTRL || iFlag == FLAG_CONNECT_IGNORE )
	{
		//带外数据和只读操作优先发送,不参与排队
		if( m_oSocket.GetSocketFD() > 0 && m_oSocket.GetStatus() == Tcs_Connected )
		{
			iTempRet = m_oSocket.SendOneMsg( nMsgLength, pMsg );
		}
		else
		{
			iTempRet = -2;
		}

#ifdef _POSIX_MT_
		pthread_mutex_unlock( &m_stMutex );
#endif
		return iTempRet;
	}

	if( m_iBlockStatus == BLK_EMPTY && m_iConnectState == Connect_ON )
	{
		iTempRet = m_oSocket.SendOneMsg( nMsgLength, pMsg );
	}

	if( ( m_iConnectState == Connect_OFF && m_oSocket.GetSocketFD() > 0 && m_oSocket.GetStatus() == Tcs_Connected ) ||
		( ( m_iBlockStatus != BLK_EMPTY || iTempRet != 0 ) && iFlag == FLAG_CONNECT_REDO ) )
	{
		Log_Msg( Log_Debug, "TcpConnection: Now begin to reserve msgs.\n" );

		if( m_pBlockQueue == 0 )
		{
			Log_Msg( Log_Debug, "TcpConnection: Create block queue.\n" );
			m_pBlockQueue = new FMsgQueue( BLOCKQUEUESIZE );
		}

		if( m_pBlockQueue )
		{
			if( m_iBlockStatus == BLK_EMPTY )
			{
				m_iBlockStatus = BLK_SAVING;
				Log_Msg( Log_Debug, "TcpConnection: Now the block status of (%d,%d) is %d.\n", m_sType, m_sID, m_iBlockStatus );
			}
			
			int iRsvRet = m_pBlockQueue->PushTailMsg( (const unsigned char*)pMsg, nMsgLength );
			if( iRsvRet == -2 )
			{
				if( m_pRedoQueue != m_pBlockQueue )
				{
					char szDumpFile[64];
					sprintf(szDumpFile, "MSG%02d_%02d_%02d.tmp", m_sType, m_sID, m_iCurrentDumpSeq);
					Log_Msg( Log_Debug, "TcpConnection: Block queue is full now, begin to dump to file %s.\n", szDumpFile );
					
					m_pBlockQueue->SaveToFile( szDumpFile );
					m_pBlockQueue->CleanQueue();
					
					m_iCurrentDumpSeq++;
				}
				else
				{
					Log_Msg( Log_Debug, "TcpConnection: Block queue is full and redo queue is just the same, so create another block queue.\n" );
					m_pBlockQueue = new FMsgQueue( BLOCKQUEUESIZE );
				}

				if( m_pBlockQueue )
				{
					m_pBlockQueue->PushTailMsg( (const unsigned char*)pMsg, nMsgLength );
				}
			}
			Log_Msg( Log_Debug, "TcpConnection: Successfully reserved Msg.\n" );
		}
	}

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::CleanBlockQueue( int iQueueLength )
{
	int iRedoCount	= 0;
	int i			= 0;
	int iTempRet	= 0;
	short nMsgLength;
	BYTE  pMsgBuf[MSG_BUFFER_LENGTH];

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	if( m_iConnectState == Connect_ON && m_oSocket.GetSocketFD() > 0 && 
		m_oSocket.GetStatus() == Tcs_Connected && m_iBlockStatus != BLK_EMPTY &&
		m_oSocket.CleanReserveData() == 0 )
	{
		Log_Msg( Log_Debug, "TcpConnection: Now begin to redo transfer, will recover no more than %d Msgs.\n", iQueueLength );
		if( m_pRedoQueue == 0 )
		{
			m_pRedoQueue = new FMsgQueue( BLOCKQUEUESIZE );
			Log_Msg( Log_Debug, "TcpConnection: Redo queue created now.\n" );
		}
		
		if( m_pRedoQueue )
		{
			for( i = 0; i < iQueueLength; i++ )
			{
				if( m_oSocket.HasReserveData() )
				{
					Log_Msg( Log_Debug, "TcpConnection: Redo process is stopped because of data block.\n" );
					break;
				}

				nMsgLength = MSG_BUFFER_LENGTH;
				m_pRedoQueue->GetHeadMsg( pMsgBuf, &nMsgLength );
				if( nMsgLength == 0 )
				{
					Log_Msg( Log_Debug, "TcpConnection: The redo queue is empty now.\n" );

					if( m_iCurrentRedoSeq < m_iCurrentDumpSeq )
					{

						char szDumpFile[64];
					
						sprintf( szDumpFile, "MSG%02d_%02d_%02d.tmp", m_sType, m_sID, m_iCurrentRedoSeq );
						Log_Msg( Log_Debug,"TcpConnection: Load redo queue from file %s.\n", szDumpFile );

						m_pRedoQueue->LoadFromFile( szDumpFile );
						m_iCurrentRedoSeq++;
						i--;

						unlink( ( const char* )szDumpFile );
					}
					else if( m_pRedoQueue != m_pBlockQueue )
					{
						delete m_pRedoQueue;
						m_pRedoQueue = m_pBlockQueue;

						i--;

						Log_Msg( Log_Debug, "TcpConnection: Now begin to redo current block queue.\n" );
					}
					else
					{
						delete m_pRedoQueue;
						m_pRedoQueue = 0;
						m_pBlockQueue = NULL;

						m_iBlockStatus = BLK_EMPTY;
						
						m_iCurrentRedoSeq = 0;
						m_iCurrentDumpSeq = 0;

						Log_Msg( Log_Debug, "TcpConnection: Now all block queue is cleaned.\n" );
						break;
					}

				}
				else
				{
					iTempRet = m_oSocket.SendOneMsg( nMsgLength, (char*)pMsgBuf );
					if( iTempRet == 0 )
					{
						iRedoCount++;
						Log_Msg( Log_Debug, "TcpConnection: Redo one msg OK.\n" );
					}
					else
					{
						Log_Msg( Log_Error, "TcpConnection: Resend one msg failed of %d.\n", iTempRet );
						break;
					}
				}

			}	// end for
		}	// m_pRedoQueue
	}

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iRedoCount;
}

/*---------------------------------------------------------------------------------------------------------*/
void TcpConnection::SetConnectState( int iConnectState )
{
#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	m_iConnectState = iConnectState;

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif
}

/*---------------------------------------------------------------------------------------------------------*/
SOCKET TcpConnection::GetSocketFD()
{
	int iRet;

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	iRet = m_oSocket.GetSocketFD();

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int TcpConnection::GetSocketStatus()
{
	int iRet;

#ifdef _POSIX_MT_
	pthread_mutex_lock( &m_stMutex );
#endif

	iRet = m_oSocket.GetStatus();

#ifdef _POSIX_MT_
	pthread_mutex_unlock( &m_stMutex );
#endif

	return iRet;
}


/* ----------------------------------------------------------------------------------------------------------
End of file */
