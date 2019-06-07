/* ----------------------------------------------------------------------------------------------------------



2009-3-21
-----------------------------------------------------------------------------------------------------------*/
#include "AiThread.h"
#include "Log.h"
#include "FMsgQueue.h"
#include "../DdzAiPublic/include/MsgBodyLogic.h"
#include "./ai/ddz_state.h"
#include "./ai/mcts_node.h"

/*---------------------------------------------------------------------------------------------------------*/
AiThread::AiThread()
{
	m_pInputQueue = new FMsgQueue( MSG_INPUT_QUEUE_LENGTH );

	memset( ( void* )&m_oCurrSwitchMsgHead, 0, sizeof( m_oCurrSwitchMsgHead ) );

	m_tLastCheckTime = 0;
	m_tCurTime = 0;
	m_iThreadID = -1;
}

/*---------------------------------------------------------------------------------------------------------*/
AiThread::~AiThread()
{
	if ( m_pInputQueue )
	{
		delete m_pInputQueue;
		m_pInputQueue = 0;
	}
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::CanBlocked()
{
	if ( !m_pInputQueue )
	{
		return 1;
	}

	return m_pInputQueue->IsQueueEmpty();
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::ThreadInit()
{
	
	return 0;
}

int AiThread::Initialize( int iThreadID, Communication2* pCommunication )
{
	if ( !pCommunication )
	{
		return -1;
	}

	m_iThreadID = iThreadID;

	m_pCommunication2 = pCommunication;
		
	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
void AiThread::Run()
{
	int		iRet;

	BYTE	vMsgBody[MSG_BUFFER_LENGTH];
	NetMsg	oNetMsg( ( void* )vMsgBody );

 	BYTE	vMsgBuf[MSG_BUFFER_LENGTH];
 	short	nMsgLength;

	time( &m_tLastCheckTime );
	time( &m_tLastSessionCheckTime );

	ThreadLogOut( TLL_Normal, "Thread id = %d Init to run.\n", m_iThreadID );

	while( 1 )
	{
		DetermineBlock();

		nMsgLength = MSG_BUFFER_LENGTH;
		iRet = RecvOneMsg( nMsgLength, vMsgBuf );
		if( iRet < 0 )
		{
			//如果没有收到消息，让线程挂起2ms
			struct timeval tv;
			tv.tv_sec = 2 / 1000;
			tv.tv_usec = (2 % 1000) * 1000;
			select(0, NULL, NULL, NULL, &tv);
			continue;
		}

		BYTE* pTempMsg = vMsgBuf;
		
		if( m_oCurrSwitchMsgHead.DecodeMsg( pTempMsg, nMsgLength ) )
		{
			continue;
		}

		if ( m_oCurrSwitchMsgHead.m_nBodyLength <= 0 )
		{
			continue;
		}

		short nHeadLength = m_oCurrSwitchMsgHead.m_nTotalLength - m_oCurrSwitchMsgHead.m_nBodyLength;
		BYTE* pTemp = pTempMsg + nHeadLength;
		
		if ( nMsgLength <= 0 )
		{
			ThreadLogOut( TLL_Debug, "DBThread: Got one empty msg from input queue, ignore it.\n" );
			continue;
		}
	 
	 	iRet = oNetMsg.DecodeMsg( pTempMsg + nHeadLength, m_oCurrSwitchMsgHead.m_nBodyLength );
	 	if ( iRet )
	 	{
	 		Log_Msg( Log_Debug, "DBThread: ConvertDBCacheBufToMsg error ret = %d.\n", iRet );
	 		continue ;
	 	}

//		if( oNetMsg.m_oHead.m_sMsgID != MC_DB_INTERNAL_TIMEOUT )//do not generate much more logs
//		{
//			ThreadLogOut( TLL_Debug, "DBThread: Msg(%d) from (%d,%d).\n", oNetMsg.m_oHead.m_sMsgID, m_oCurrSwitchMsgHead.m_nSrcFE, m_oCurrSwitchMsgHead.m_nSrcID);
//		}
		
		iRet = OnEvent( 0, &m_oCurrSwitchMsgHead, &oNetMsg );
		if ( iRet )
		{
			ThreadLogOut( TLL_Debug, "DBThread: Process Msg(%d): Ret:%d.\n", oNetMsg.m_oHead.m_sMsgID, iRet );
		}
	}
}

/*---------------------------------------------------------------------------------------------------------*/
void AiThread::DBThreadOnEnd()
{
	ThreadLogOut( TLL_Debug, "DBThread: ThreadInternalCheck, start...\n" );

	ThreadInternalCheck();
}

/*---------------------------------------------------------------------------------------------------------*/
void AiThread::ThreadInternalCheck()
{
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::SendOneMsg( short nMsgLength, BYTE* pMsg )
{
	if(!pMsg || nMsgLength <= 0)
	{
		return -1;
	}
	
	int iRet;

	pthread_mutex_lock( &m_stMutex );

	iRet = m_pInputQueue->PushTailMsg( ( const BYTE * )pMsg, nMsgLength );

	pthread_mutex_unlock( &m_stMutex );

	return iRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::RecvOneMsg( short& nMsgLength, BYTE* pMsg )
{
	if ( !pMsg )
	{
		return -1;
	}

	int iRet;

	pthread_mutex_lock( &m_stMutex );

	iRet = m_pInputQueue->GetHeadMsg( pMsg, &nMsgLength );

	pthread_mutex_unlock( &m_stMutex );
	
	return iRet;
}

/*---------------------------------------------------------------------------------------------------------*/
void AiThread::PrintMsgToLog( BYTE* pMsg, int nMsgLength )
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
	ThreadLogOut( TLL_Debug, "Print Hex:%s", tmpBuffer );
#endif
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::ConvertBufToMsg( short nBufLength, BYTE* pBuf, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg )
{
	if(nBufLength <= 0 || !pBuf || !pSwitchMsgHead || !pMsg )
	{
		ThreadLogOut( TLL_Debug, " DBThread: In ConvertBufToMsg, invalid input para.\n " );
		return -1;
	}
	
	BYTE* pTemp = pBuf;
	if( pSwitchMsgHead->DecodeMsg( pTemp, nBufLength ) )
	{
		ThreadLogOut( TLL_Debug, " DBThread: In ConvertBufToMsg, decode DBSwitchMsgHead failed.\n " );
		return -2;
	}
	
	if(pSwitchMsgHead->m_nTotalLength > nBufLength )
	{
		ThreadLogOut( TLL_Debug, " DBThread: In ConvertBufToMsg, too short buf(%d), total must be %d.\n", nBufLength, pSwitchMsgHead->m_nTotalLength );
		return -3;
	}

	short nHeadLength = pSwitchMsgHead->m_nTotalLength - pSwitchMsgHead->m_nBodyLength;
	pTemp += nHeadLength;

	int ret = 0;
	if ( (ret = pMsg->DecodeMsg( pTemp, pSwitchMsgHead->m_nBodyLength )) != 0 )
	{
		ThreadLogOut( TLL_Debug, "DBThread: In ConvertBufToMsg, decode NetMsg failed. ret = %d\n", ret );
		return -4;
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::OnEvent( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg )
{
	int iTempRet = 0;
	timeval tvTempBegin, tvTempEnd, tvTempHandle;

	if ( !pMsg )
	{
		return -1;
	}

	gettimeofday( &tvTempBegin, 0 );

	switch( pMsg->m_oHead.m_sMsgID )
	{
	case MC_C2S_Play_Card:
		iTempRet = Process_C2S_PlayCard(nReqDBSwitchId, pSwitchMsgHead, pMsg);
		break;
	case MC_C2S_Passive_Play_Card:
		iTempRet = Process_C2S_PassivePlayCard(nReqDBSwitchId, pSwitchMsgHead, pMsg);
		break;

	case MC_C2S_Grab_Landlord:
		iTempRet = Process_C2S_GrabLandlord(nReqDBSwitchId, pSwitchMsgHead, pMsg);
		break;
	}


	return iTempRet;
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::Process_C2S_GrabLandlord( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg )
{
	if ( !pMsg )
	{
		return -3;
	}

	if ( pMsg->m_oHead.m_sMsgID != MC_C2S_Grab_Landlord )
	{
		return -4;
	}

	C2S_Grab_Landlord* pRequest = ( C2S_Grab_Landlord* )pMsg->m_pMsgBody;
	if ( !pRequest )
	{
		return -5;
	}

	S2C_Grab_Landlord_Result grabLandlordResult;
	grabLandlordResult.m_uiAiHandle			= pRequest->m_uiAiHandle;
	grabLandlordResult.m_uiScore			= 0;

	for(int i = 0; i < 3; ++i)
	{
		if (pRequest->m_vthreePaiList[i] >= pai_type_3 && pRequest->m_vthreePaiList[i] <= pai_type_blossom)
		{
			pRequest->m_vPaiList1[pRequest->m_vthreePaiList[i]]++;
		}
	}

	bool onceShow = false;
	struct timeval t_start,t_end;
	gettimeofday(&t_start, NULL);

	for(int i = 0; i < 1; ++i)
	{
		ddz_state ddzState;
		int playerid[3] = {1,2,3};
		ddzState.setPlayerList(playerid, 3);
		ddzState.setLandlord(1);

		ddzState.setPaiList(1, pRequest->m_vPaiList1);
		ddzState.setPaiList(2, pRequest->m_vPaiList2);
		ddzState.setPaiList(3, pRequest->m_vPaiList3);

		if (!onceShow)
		{
			ThreadLogOut(TLL_Error, "Process_C2S_GrabLandlord tid=%d ---------------------------->%d [%u %u, %u]\nPlayer1 ---------------------------->%s\nPlayer2 ---------------------------->%s\nPlayer3 ---------------------------->%s\n", m_iThreadID, pRequest->m_uiAiHandle, 
				pRequest->m_vthreePaiList[0], pRequest->m_vthreePaiList[1], pRequest->m_vthreePaiList[2], ddzState.get_pai_list(1).c_str(), ddzState.get_pai_list(2).c_str(), ddzState.get_pai_list(3).c_str());
			onceShow = true;
		}

		compute_options player1_options, player2_options;
		player1_options.max_iterations = 120;
		player1_options.number_of_threads = 1;
		player1_options.verbose = false;

		player2_options.max_iterations = 120;
		player2_options.number_of_threads = 1;
		player2_options.verbose = false;

		int count = 0;
		while (ddzState.has_moves())
		{
			mcts_move* move = compute_move(&ddzState, player1_options);
			if (move)
			{
				ddzState.do_move(move);
				delete move;
			}

			move = compute_move(&ddzState, player1_options);
			if (move)
			{
				ddzState.do_move(move);
				delete move;
			}

			move = compute_move(&ddzState, player1_options);
			if (move)
			{
				ddzState.do_move(move);
				delete move;
			}

			count++;
			if (count > 80)
				break;
		}

		if (ddzState.get_win(0))
			grabLandlordResult.m_uiScore++;
	}

	gettimeofday(&t_end, NULL);

	ThreadLogOut(TLL_Error, "End GrabLandlord tid=%d time=%u win_count=%d ------------------------------>%u\n", m_iThreadID, ((t_end.tv_sec-t_start.tv_sec)*1000000+t_end.tv_usec-t_start.tv_usec) / 1000, grabLandlordResult.m_uiScore, pRequest->m_uiAiHandle);

	unsigned char szMsgBuf[ MSG_BUFFER_LENGTH ];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	NetMsg oNetMsg( ( void* )&grabLandlordResult );
	oNetMsg.m_oHead.m_sMsgID = MC_S2C_Grab_Landlord_Result;

	sMsgBufLength = MSG_BUFFER_LENGTH;
	if ( oNetMsg.EncodeMsg( szMsgBuf, sMsgBufLength ) == 0 )
	{
		SendMsgToMainServer( nReqDBSwitchId, pSwitchMsgHead->m_nSrcID, szMsgBuf, sMsgBufLength );
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::Process_C2S_PassivePlayCard( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg )
{
	if ( !pMsg )
	{
		return -3;
	}

	if ( pMsg->m_oHead.m_sMsgID != MC_C2S_Passive_Play_Card )
	{
		return -4;
	}

	C2S_Passive_Play_Card* pRequest = ( C2S_Passive_Play_Card* )pMsg->m_pMsgBody;
	if ( !pRequest )
	{
		return -5;
	}

	S2C_Passive_Play_Card_Result playCardResult;
	playCardResult.m_uiAiHandle			= pRequest->m_uiAiHandle;
	playCardResult.m_oDdzMove._type     = ddz_type_no_move;
	playCardResult.m_oDdzMove._alone_1  = 0;
	playCardResult.m_oDdzMove._alone_2  = 0;
	playCardResult.m_oDdzMove._alone_3  = 0;
	playCardResult.m_oDdzMove._alone_4  = 0;
	playCardResult.m_oDdzMove._airplane_pairs = 0;
	playCardResult.m_oDdzMove._combo_count = 0;

	ddz_move* limit_move = new ddz_move(pRequest->m_oLimitPai._type, pRequest->m_oLimitPai._alone_1, pRequest->m_oLimitPai._alone_2, pRequest->m_oLimitPai._alone_3, pRequest->m_oLimitPai._alone_4);
	limit_move->_airplane_pairs = pRequest->m_oLimitPai._airplane_pairs == 1;
	limit_move->_combo_count = pRequest->m_oLimitPai._combo_count;
	memcpy(limit_move->_combo_list, pRequest->m_oLimitPai._combo_list, 20*sizeof(unsigned char));

	ddz_state ddzState(limit_move, (int)pRequest->m_ucOutPaiIndex);

	int playerid[3] = {1,2,3};
	ddzState.setPlayerList(playerid, 3);
	ddzState.setLandlord(playerid[pRequest->m_ucLandlordIndex]);

	ddzState.setPaiList(1, pRequest->m_vPaiList1);
	ddzState.setPaiList(2, pRequest->m_vPaiList2);
	ddzState.setPaiList(3, pRequest->m_vPaiList3);

	char szbuf[128]= {0};
	for(int i = 0; i < pRequest->m_oLimitPai._combo_count; ++i)
	{
		if (ddzState.get_pai_value(pRequest->m_oLimitPai._combo_list[i]))
			strcat(szbuf, ddzState.get_pai_value(pRequest->m_oLimitPai._combo_list[i]));
	}

	ThreadLogOut(TLL_Error, "Process_C2S_PassivePlayCard tid=%d ---------------------------->%d outindex=%d landlord=%d type=%d alone1=%d alone2=%d alone3=%d alone4=%d airpairs=%d count=%d [%s]\nPlayer1 ---------------------------->%s\nPlayer2 ---------------------------->%s\nPlayer3 ---------------------------->%s\n", m_iThreadID, pRequest->m_uiAiHandle, 
		pRequest->m_ucOutPaiIndex, pRequest->m_ucLandlordIndex, pRequest->m_oLimitPai._type, pRequest->m_oLimitPai._alone_1, pRequest->m_oLimitPai._alone_2, pRequest->m_oLimitPai._alone_3, pRequest->m_oLimitPai._alone_4, pRequest->m_oLimitPai._airplane_pairs ?1:0, pRequest->m_oLimitPai._combo_count, szbuf, ddzState.get_pai_list(1).c_str(), ddzState.get_pai_list(2).c_str(), ddzState.get_pai_list(3).c_str());

	compute_options player1_options, player2_options;
	player1_options.max_iterations = 300;
	player1_options.number_of_threads = 1;
	player1_options.verbose = false;

	player2_options.max_iterations = 300;
	player2_options.number_of_threads = 1;
	player2_options.verbose = false;

	struct timeval t_start,t_end;
	gettimeofday(&t_start, NULL);

	mcts_move* move = compute_move(&ddzState, player1_options);
	if (move)
	{
		ddz_move* dz_move = dynamic_cast<ddz_move*>(move);
		if (dz_move)
		{
			playCardResult.m_oDdzMove._type = dz_move->_type;
			playCardResult.m_oDdzMove._alone_1 = dz_move->_alone_1;
			playCardResult.m_oDdzMove._alone_2 = dz_move->_alone_2;
			playCardResult.m_oDdzMove._alone_3 = dz_move->_alone_3;
			playCardResult.m_oDdzMove._alone_4 = dz_move->_alone_4;
			playCardResult.m_oDdzMove._airplane_pairs = dz_move->_airplane_pairs ? 1 : 0;
			playCardResult.m_oDdzMove._combo_count = dz_move->_combo_count;
			memcpy(playCardResult.m_oDdzMove._combo_list, dz_move->_combo_list, 20*sizeof(unsigned char));
		}
	}
	delete move;

	gettimeofday(&t_end, NULL);

	ThreadLogOut(TLL_Error, "End Passive tid=%d time=%u ------------------------------>type=%d alone1=%d alone2=%d alone3=%d\n", m_iThreadID, ((t_end.tv_sec-t_start.tv_sec)*1000000+t_end.tv_usec-t_start.tv_usec) / 1000, playCardResult.m_oDdzMove._type, 
		playCardResult.m_oDdzMove._alone_1, playCardResult.m_oDdzMove._alone_2, playCardResult.m_oDdzMove._alone_3 );


	unsigned char szMsgBuf[ MSG_BUFFER_LENGTH ];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	NetMsg oNetMsg( ( void* )&playCardResult );
	oNetMsg.m_oHead.m_sMsgID = MC_S2C_Passive_Play_Card_Result;

	sMsgBufLength = MSG_BUFFER_LENGTH;
	if ( oNetMsg.EncodeMsg( szMsgBuf, sMsgBufLength ) == 0 )
	{
		SendMsgToMainServer( nReqDBSwitchId, pSwitchMsgHead->m_nSrcID, szMsgBuf, sMsgBufLength );
	}

	return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
int AiThread::Process_C2S_PlayCard( int nReqDBSwitchId, SwitchMsgHead* pSwitchMsgHead, NetMsg* pMsg )
{
	if ( !pMsg )
	{
		return -3;
	}

	if ( pMsg->m_oHead.m_sMsgID != MC_C2S_Play_Card )
	{
		return -4;
	}

	C2S_Play_Card* pRequest = ( C2S_Play_Card* )pMsg->m_pMsgBody;
	if ( !pRequest )
	{
		return -5;
	}

	S2C_Play_Card_Result playCardResult;
	playCardResult.m_uiAiHandle			= pRequest->m_uiAiHandle;
	playCardResult.m_oDdzMove._type     = ddz_type_no_move;
	playCardResult.m_oDdzMove._alone_1  = 0;
	playCardResult.m_oDdzMove._alone_2  = 0;
	playCardResult.m_oDdzMove._alone_3  = 0;
	playCardResult.m_oDdzMove._alone_4  = 0;
	playCardResult.m_oDdzMove._airplane_pairs = 0;
	playCardResult.m_oDdzMove._combo_count = 0;

	ddz_state ddzState;
	int playerid[3] = {1,2,3};
	ddzState.setPlayerList(playerid, 3);
	ddzState.setLandlord(playerid[pRequest->m_ucLandlordIndex]);
	
	ddzState.setPaiList(1, pRequest->m_vPaiList1);
	ddzState.setPaiList(2, pRequest->m_vPaiList2);
	ddzState.setPaiList(3, pRequest->m_vPaiList3);

	ThreadLogOut(TLL_Error, "Process_C2S_PlayCard tid=%d ---------------------------->%d outindex=%d\nPlayer1 ---------------------------->%s\nPlayer2 ---------------------------->%s\nPlayer3 ---------------------------->%s\n", m_iThreadID, pRequest->m_uiAiHandle, pRequest->m_ucLandlordIndex,
		ddzState.get_pai_list(1).c_str(), ddzState.get_pai_list(2).c_str(), ddzState.get_pai_list(3).c_str());

	compute_options player1_options, player2_options;
	player1_options.max_iterations = 300;
	player1_options.number_of_threads = 1;
	player1_options.verbose = false;

	player2_options.max_iterations = 300;
	player2_options.number_of_threads = 1;
	player2_options.verbose = false;

	struct timeval t_start,t_end;
	gettimeofday(&t_start, NULL);

	mcts_move* move = compute_move(&ddzState, player1_options);
	if (move)
	{
		ddz_move* dz_move = dynamic_cast<ddz_move*>(move);
		if (dz_move)
		{
			playCardResult.m_oDdzMove._type = dz_move->_type;
			playCardResult.m_oDdzMove._alone_1 = dz_move->_alone_1;
			playCardResult.m_oDdzMove._alone_2 = dz_move->_alone_2;
			playCardResult.m_oDdzMove._alone_3 = dz_move->_alone_3;
			playCardResult.m_oDdzMove._alone_4 = dz_move->_alone_4;
			playCardResult.m_oDdzMove._airplane_pairs = dz_move->_airplane_pairs ? 1 : 0;
			playCardResult.m_oDdzMove._combo_count = dz_move->_combo_count;
			memcpy(playCardResult.m_oDdzMove._combo_list, dz_move->_combo_list, 20*sizeof(unsigned char));
		}
	}
	delete move;

	gettimeofday(&t_end, NULL);

	ThreadLogOut(TLL_Error, "End PlayCard tid=%d time=%u ---------------------------->type=%d alone1=%d alone2=%d alone3=%d\n", m_iThreadID, ((t_end.tv_sec-t_start.tv_sec)*1000000+t_end.tv_usec-t_start.tv_usec) / 1000, playCardResult.m_oDdzMove._type, 
		playCardResult.m_oDdzMove._alone_1, playCardResult.m_oDdzMove._alone_2, playCardResult.m_oDdzMove._alone_3);

	unsigned char szMsgBuf[ MSG_BUFFER_LENGTH ];
	short sMsgBufLength = MSG_BUFFER_LENGTH;

	NetMsg oNetMsg( ( void* )&playCardResult );
	oNetMsg.m_oHead.m_sMsgID = MC_S2C_Play_Card_Result;

	sMsgBufLength = MSG_BUFFER_LENGTH;
	if ( oNetMsg.EncodeMsg( szMsgBuf, sMsgBufLength ) == 0 )
	{
		SendMsgToMainServer( nReqDBSwitchId, pSwitchMsgHead->m_nSrcID, szMsgBuf, sMsgBufLength );
	}

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::SendMsgToMainServer( int nDBSwitchID, short sMainServerID, BYTE* pBuf, short nBufLength )
{
	if (!m_pCommunication2)
	{
		return -1;
	}

	if (m_pCommunication2->SendMsgToServer((unsigned char*)pBuf, nBufLength, Cst_MainServer, sMainServerID) != 0)
	{
		return -2;
	}
	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int AiThread::SendMsgToMainServer(short sMainServerID, NetMsgBody * msgbody )
{
	if (!m_pCommunication2)
	{
		return -1;
	}

	if (m_pCommunication2->SendMsgToServer(msgbody, Cst_MainServer, sMainServerID) != 0)
	{
		return -2;
	}
	return 0;
}

/* ----------------------------------------------------------------------------------------------------------
End of file */
