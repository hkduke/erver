/* ----------------------------------------------------------------------------------------------------------



 in 2009-3-19
-----------------------------------------------------------------------------------------------------------*/
#include "DdzAiServer.h"
#include "AiThread.h"
#include "Log.h"
#include "ConfigIniFile.h"

/*---------------------------------------------------------------------------------------------------------*/
DdzAiServer::DdzAiServer()
: m_oCommunication2(Communication2::CT_Server)
{
	m_eRunFlag = Dbcsrf_Running;

	int i = 0;
	for( ; i < Max_Ai_Thread; ++i )
	{
		m_vAiThreadList[i] = new AiThread;
	}

	time( &m_tLastPrintStatTime );
	time( &m_tLastDisconnectCheckTime );

	m_szCfgFile = 0;
	m_bCommunicationInit = false;
}

/*---------------------------------------------------------------------------------------------------------*/
DdzAiServer::~DdzAiServer()
{

}

/*---------------------------------------------------------------------------------------------------------*/
int DdzAiServer::Initialize( const char* szCfg , bool bRunTestCase )
{
	m_szCfgFile = szCfg;

	// read config file
	if ( ReadConfigFile( m_szCfgFile ) )
	{
		Log_Msg( Log_Failed, "DdzAiServer: read config file failed!\n" );
		return -1;
	}

	int i = 0;
	for( ; i < Max_Ai_Thread; i++ )
	{
		m_vAiThreadList[i]->Initialize( i, &m_oCommunication2 );
	}

	Log_Msg( Log_Debug, " DdzAiServer: thread Initialize end, now create thread.\n " );

	for( i = 0; i < Max_Ai_Thread; i++ )
	{
		if ( m_vAiThreadList[i]->CreateThread() )
		{
			Log_Msg( Log_Failed, "DdzAiServer: Initialize() create db thread %d failed.\n", i );
			return -1;
		}
	}

	Log_Msg( Log_Normal, "DdzAiServer: successfully create %d threads to handle request.\n", i );

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
void DdzAiServer::Run()
{
	while ( 1 )
	{
		if( Dbcsrf_DBCacheServerExit & m_eRunFlag )
		{
			m_eRunFlag &= ~Dbcsrf_DBCacheServerExit;
			Log_Msg( Log_Normal, "DdzAiServer: Stop all threads...\n" );

			int i = 0;
			for( ; i < Max_Ai_Thread; i++ )
			{
				m_vAiThreadList[i]->DBThreadOnEnd();
				m_vAiThreadList[i]->StopThread();
			}

			Log_Msg( Log_Normal, "DdzAiServer: Exit...\n" );
			break;
		}

		if ( Dbcsrf_ReConfig & m_eRunFlag )
		{
			m_eRunFlag &= ~Dbcsrf_ReConfig;

			Log_Msg( Log_Normal, "DdzAiServer: Have got command to reload config, read config ...\n" );

			ReadConfigFile( m_szCfgFile );

			int i = 0;
			for( ; i < Max_Ai_Thread; i++ )
			{
				m_vAiThreadList[i]->DBThreadOnEnd();
			}

			Log_Msg( Log_Normal, "DdzAiServer: Successfully reload config ...\n" );
		}

		InternalCheck();

		ReceiveAndDispatchMsg();
	}
}

/*---------------------------------------------------------------------------------------------------------*/
void DdzAiServer::SetRunFlag( DBCacheServerRunFlag eFlag )
{
}
/*---------------------------------------------------------------------------------------------------------*/
int DdzAiServer::Execute( SwitchMsgHead * header, void* pMsg, short nMsgLength, int iConnection )
{
	if (!header)
	{
		return -1;
	}

	switch( header->m_bTransferType )
	{
	case TT_ByUID:
		{
			// send special db thread
			int iThreadChoice = header->m_stTransferInfo.unUid % Max_Ai_Thread;

			int iRet = m_vAiThreadList[iThreadChoice]->SendOneMsg( nMsgLength, (BYTE*)pMsg );

			if ( iRet >= 0 )
			{
				m_vAiThreadList[iThreadChoice]->WakeUp();
			}
			else
			{
			}
		}
		break;
	case TT_ServerToGroup:
	case TT_ServerToServer:
	case TT_Broadcast:
		{
			for( int iThreadChoice = 0; iThreadChoice < Max_Ai_Thread; iThreadChoice++ )
			{
				int iRet = m_vAiThreadList[iThreadChoice]->SendOneMsg( nMsgLength, (BYTE*)pMsg );
				if ( iRet >= 0 )
				{
					m_vAiThreadList[iThreadChoice]->WakeUp();
				}
				else
				{
				}
			}
		}
		break;

	default:
		{
			Log_Msg( Log_Debug, "DBCacheServer: DispatchOneMsg, invalid transfer type %d.\n", header->m_bTransferType );
		}
	}
	return 0;
}
/*---------------------------------------------------------------------------------------------------------*/
int DdzAiServer::ReadConfigFile( const char* szCfg )
{
	if ( !szCfg )
	{
		return -1;
	}

	ConfigIniFile oIniFile;
	if ( oIniFile.OpenFile( szCfg ) < 0 )
	{
		return -1;
	}

	oIniFile.GetIntValue( "AiServer", "ServerID", m_oServerConfig.m_iServerID, 1 );
	oIniFile.GetStringValue( "AiServer", "ServerIP", m_oServerConfig.m_szServerIP, 30 );

	char szConfigName[256] = {0};
	oIniFile.GetStringValue( "AiServer", "CommunicationConfig", szConfigName, sizeof(szConfigName));

	//communication2
	if (!m_bCommunicationInit)
	{
		int iRet = m_oCommunication2.Initialize( Cst_DBServer, m_oServerConfig.m_iServerID, szConfigName, this );
		if ( iRet < 0 )
		{
			return -2;
		}

		m_bCommunicationInit = true;
	}

	return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
void DdzAiServer::PrintMsgToLog( BYTE* pMsg, int nMsgLength )
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
	Log_Msg( Log_Temp, "Print Hex:%s", tmpBuffer );
#endif
}


/*---------------------------------------------------------------------------------------------------------*/
void DdzAiServer::InternalCheck()
{
	time_t tNow = time( 0 );
	NetMsg oInternalMsg( 0 );

	if ( tNow - m_tLastDisconnectCheckTime >= 30 )
	{
		for( int i = 0; i < Max_Ai_Thread; ++i )
		{
			SendInternalMsgToDBThread( i, &oInternalMsg );
		}

		m_tLastDisconnectCheckTime = tNow;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
int DdzAiServer::ReceiveAndDispatchMsg()
{
	m_oCommunication2.Update(this);
}

/*---------------------------------------------------------------------------------------------------------*/
int DdzAiServer::SendInternalMsgToDBThread( int iDBThreadId, NetMsg* pMsg )
{
 	unsigned char vBody[MSG_BUFFER_LENGTH];
 	short sBodyLength;
 
 	int iTempRet = -1;
 
 	if( iDBThreadId < 0 || iDBThreadId >= Max_Ai_Thread || !pMsg )
 	{
 		Log_Msg( Log_Error, "DBCacheServer: In SendInternalMsgToDBThread[%d], invalid input param.\n", iDBThreadId );
 		return -1;
 	}
 
  	if( pMsg->EncodeMsg( vBody, sBodyLength ) )
  	{
  		Log_Msg( Log_Error, "DBCacheServer: In SendInternalMsgToDBThread, encode message failed.\n" );
  		return -2;
  	}

	iTempRet = m_vAiThreadList[iDBThreadId]->SendOneMsg( sBodyLength, vBody );
	if( iTempRet >= 0 )
	{
		m_vAiThreadList[iDBThreadId]->WakeUp();
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
void DdzAiServer::PrintServerStatistic()
{

}

void DdzAiServer::RecvMsgCallback(SwitchMsgHead * header, short nMsgLength, char * pMsg, short bufLen, char * pBuf, void * cbData)
{
	DdzAiServer * This = (DdzAiServer*)cbData;

	switch( header->m_bTransferType )
	{
	case TT_ByUID:
		{
			// send special db thread
			int iThreadChoice = header->m_stTransferInfo.unUid % Max_Ai_Thread;

			int iRet = This->m_vAiThreadList[iThreadChoice]->SendOneMsg( bufLen, (BYTE*)pBuf );

			if ( iRet >= 0 )
			{
				This->m_vAiThreadList[iThreadChoice]->WakeUp();
			}
			else
			{
			}
		}
		break;
	case TT_ServerToServer:
	case TT_Broadcast:
		{
			for( int iThreadChoice = 0; iThreadChoice < Max_Ai_Thread; iThreadChoice++ )
			{
				int iRet = This->m_vAiThreadList[iThreadChoice]->SendOneMsg( bufLen, (BYTE*)pBuf );
				if ( iRet >= 0 )
				{
					This->m_vAiThreadList[iThreadChoice]->WakeUp();
				}
				else
				{
				}
			}
		}
		break;

	default:
		{
			Log_Msg( Log_Debug, "DBCacheServer: DispatchOneMsg, invalid transfer type %d.\n", header->m_bTransferType );
		}
	}
	return;
}

/* ----------------------------------------------------------------------------------------------------------
End of file */
