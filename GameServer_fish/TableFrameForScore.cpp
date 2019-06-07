#include "TableFrameForScore.h"
#include "GameTableManager.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "ServerCfg.h"
#include "Utility.h"
#include "ServerMgr.h"
#include <fstream>
#include <math.h>
#include "AndroidUserMgr.h"
#include "timeFunction.h"

//游戏数据长度
#define	GAME_DATA_LEN	(4096)
extern CNetConnector *pNetDB;
extern CNetConnector *pNetCenter;

CTableFrameForScore::CTableFrameForScore()
	: m_pTableFrameSink(NULL)
	, m_pGameKindInfo(NULL)
	, m_pRoomKindInfo(NULL)
	, m_UserList(NULL)
	, m_bGameStart(false)
	, m_GameRoundPhase(en_GameRound_Free)
	, m_pRedis(NULL)
{
	memset(&m_TableState, 0, sizeof(m_TableState));
	memset(&m_PrivateTableInfo, 0, sizeof(m_PrivateTableInfo));
}

CTableFrameForScore::~CTableFrameForScore()
{
	Clear();
}

void CTableFrameForScore::Init(ITableFrameSink* pSink, const TableState& state, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis)
{
	m_pTableFrameSink = pSink;

	m_pGameKindInfo = pGameKind;
	m_pRoomKindInfo = pRoomKind;
	m_pRedis = pRedis;

	memcpy(&m_TableState, &state, sizeof(TableState));

	m_UserList = new CServerUserItem*[m_pRoomKindInfo->wStartMaxPlayer];
	memset(m_UserList, 0, sizeof(CServerUserItem*) * m_pRoomKindInfo->wStartMaxPlayer);

	//初始化定时器数据
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		m_SysTimer[i].wTimerID = -1;
		m_SysTimer[i].iTimeInterval = 0;
		m_SysTimer[i].iCountTime = 0;
		m_SysTimer[i].isEnabled = CT_FALSE;
		m_SysTimer[i].dwParam = 0;
	}
}
void CTableFrameForScore::Clear()
{
	delete[]  m_UserList;
	m_UserList = NULL;
}

//发送数据
/*CT_BOOL CTableFrameForScore::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID)
{
	if (dwChairID != INVALID_CHAIR && dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return false;
	}

	if (dwChairID == INVALID_CHAIR)
	{
		CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
		for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
		{
			if (NULL == m_UserList[i]) continue;

			//发送游戏消息
			SendGameMsg(m_UserList[i], wSubCmdID);
		}
	}
	else
	{
		//发送游戏消息
		SendGameMsg(m_UserList[dwChairID], wSubCmdID);
	}
	return true;
}*/

//发送数据
CT_BOOL CTableFrameForScore::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf/* = nullptr*/, CT_WORD dwLen/*= 0*/, CT_BOOL bRecord/* = true*/)
{
	if (dwChairID!=INVALID_CHAIR && dwChairID>= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return false;
	}

	if (dwChairID == INVALID_CHAIR)
	{
		CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
		for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
		{
			if (NULL == m_UserList[i]) continue;

			//发送游戏消息
			SendGameMsg(m_UserList[i], wSubCmdID, pBuf, dwLen);
		}
	}
	else 
	{
		//发送游戏消息
		SendGameMsg(m_UserList[dwChairID], wSubCmdID, pBuf, dwLen);
	}

	return true;
}

//设置定时器
CT_BOOL CTableFrameForScore::SetGameTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam/* = 0*/)
{
	if (dwTime <= 0)
	{
		return CT_FALSE;
	}
	CT_WORD iSaveFirstIndex = DW_SYS_TIMER_NUMS;
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (m_SysTimer[i].wTimerID == wTimerID)
		{
			m_SysTimer[i].iTimeInterval = dwTime;
			m_SysTimer[i].iCountTime = 0;
			m_SysTimer[i].isEnabled = CT_TRUE;
			m_SysTimer[i].dwParam = dwParam;
			return CT_TRUE;
		}
		else if (!m_SysTimer[i].isEnabled && \
			DW_SYS_TIMER_NUMS == iSaveFirstIndex)
		{
			iSaveFirstIndex = i;
		}
	}
	if (iSaveFirstIndex < DW_SYS_TIMER_NUMS)
	{
		m_SysTimer[iSaveFirstIndex].wTimerID = wTimerID;
		m_SysTimer[iSaveFirstIndex].iTimeInterval = dwTime;
		m_SysTimer[iSaveFirstIndex].iCountTime = 0;
		m_SysTimer[iSaveFirstIndex].isEnabled = CT_TRUE;
		m_SysTimer[iSaveFirstIndex].dwParam = dwParam;
		return CT_TRUE;
	}
	return CT_FALSE;
}

//销毁定时器
CT_BOOL CTableFrameForScore::KillGameTimer(CT_WORD wTimerID)
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (m_SysTimer[i].wTimerID == wTimerID)
		{
			m_SysTimer[i].wTimerID = -1;
			m_SysTimer[i].iTimeInterval = 0;
			m_SysTimer[i].iCountTime = 0;
			m_SysTimer[i].isEnabled = CT_FALSE;
			m_SysTimer[i].dwParam = 0;
			return CT_TRUE;
		}
	}
	return CT_FALSE;
}

CT_BOOL CTableFrameForScore::IsHasGameTimer(CT_WORD wTimerID)
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (m_SysTimer[i].wTimerID == wTimerID)
		{
			return CT_TRUE;
		}
	}
	return CT_FALSE;
}

//获得桌子信息
void CTableFrameForScore::GetTableInfo(TableState& TableInfo)
{
	TableInfo = m_TableState;
}

CT_DWORD CTableFrameForScore::GetTableID()
{
	return m_TableState.dwTableID;
}

//获得游戏配置数据
void CTableFrameForScore::GetGameCfgData(GameCfgData& CfgData)
{

}

//处理输赢积分
CT_BOOL CTableFrameForScore::WriteUserScore(CT_DWORD dwChairID, const ScoreInfo& ScoreData)
{
	//SetTableLock(false);
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	if (dwChairID>=dwMaxPlayer)
	{
		return false;
	}
	if (NULL==m_UserList[dwChairID])
	{
		return false;
	}

	//更新玩家金币
	AddUserScore(m_UserList[dwChairID], ScoreData.llScore);

	//更新输赢记录
	//UpdateUserScoreInfo(m_UserList[dwChairID], ScoreData);

	//广播玩家金币
	if (ScoreData.bBroadcast)
	{
		BroadcastUserScore(m_UserList[dwChairID]);
	}

	//赢金榜记录
	if (ScoreData.llRealScore > 0 && (m_UserList[dwChairID]->IsAndroid() == false))
	{
		AddEarnScoreInfo(m_UserList[dwChairID], ScoreData.llRealScore);
	}

	//赢分广播
	if (ScoreData.llRealScore >= m_pRoomKindInfo->iBroadcastScore)
	{
		SendUserWinScoreInfo(m_UserList[dwChairID], ScoreData.llRealScore);
	}

	return true;
}

//解散游戏
CT_BOOL CTableFrameForScore::DismissGame(CT_BYTE cbDismissType)
{
	m_pTableFrameSink->OnEventGameEnd(INVALID_CHAIR, GER_DISMISS);
	return true;
}

//结束游戏
CT_BOOL CTableFrameForScore::ConcludeGame(CT_BYTE cbGameStatus, const CT_CHAR* pDrawInfo)
{
	m_GameRoundPhase = en_GameRound_Free;
	m_bGameStart = false;
	//m_pTableFrameSink->ClearGameData();

	return true;
}

//计算税收
CT_LONGLONG CTableFrameForScore::CalculateRevenue(CT_DWORD dwChairID, CT_LONGLONG llScore)
{
	return (CT_LONGLONG)ceil(llScore * GetRevenue() / 1000);
}

void CTableFrameForScore::GetPrivateTableInfo(PrivateTableInfo& privateTableInfo)
{
	privateTableInfo.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
	privateTableInfo.dwOwnerUserID = m_PrivateTableInfo.dwOwnerUserID;
	privateTableInfo.wCurrPlayCount = m_PrivateTableInfo.wCurrPlayCount;
	privateTableInfo.wTotalPlayCount = m_PrivateTableInfo.wTotalPlayCount;
	privateTableInfo.wUserCount = m_PrivateTableInfo.wUserCount;
	//privateTableInfo.wEndLimit = m_PrivateTableInfo.wEndLimit;
	//privateTableInfo.wWinLimit = m_PrivateTableInfo.wWinLimit;
	privateTableInfo.cbIsVip = m_PrivateTableInfo.cbIsVip;
	_snprintf_info(privateTableInfo.szOtherParam, sizeof(privateTableInfo.szOtherParam), "%s", m_PrivateTableInfo.szOtherParam);
}

//用户是否存在
CT_BOOL CTableFrameForScore::IsExistUser(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return false;
	}
	if (NULL==m_UserList[dwChairID])
	{
		return false;
	}
	return m_UserList[dwChairID]->GetUserID()>0;
}

//是否私人场 
CT_BOOL CTableFrameForScore::IsPrivateRoom()
{
	return (m_pRoomKindInfo->wRoomKindID == PRIVATE_ROOM) ? true : false;
}

CT_VOID CTableFrameForScore::SetGameRoundPhase(enGameRoundPhase gameRoundPhase)
{
	if (gameRoundPhase == en_GameRound_Start)
	{
		m_dwStartTime = (CT_DWORD)time(NULL);
	}
	m_GameRoundPhase = gameRoundPhase;
}

//获取私人房间房主椅子id
CT_WORD CTableFrameForScore::GetPRoomOwnerChairID()
{
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
		{
			continue;
		}

		if (pUserItem->GetUserID() == m_PrivateTableInfo.dwOwnerUserID)
		{
			return i;
		}
	}

	return INVALID_CHAIR;
}

//删除玩家的连接
CT_VOID CTableFrameForScore::CloseUserConnect(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return;
	}

	if (NULL == m_UserList[dwChairID])
	{
		return;
	}


	if (!m_UserList[dwChairID]->IsAndroid())
	{
		MSG_GS2P_CloseConnect closeConnect;
		closeConnect.dwUserID = m_UserList[dwChairID]->GetUserID();
		m_UserList[dwChairID]->SendUserMessage(MSG_PGS_MAIN, SUB_GS2P_CLOSE_USER_CONNECT, &closeConnect, sizeof(closeConnect));
	}	
}


//设置玩家托管状态
CT_VOID CTableFrameForScore::SetUserTrustee(CT_DWORD dwChairID, CT_BOOL bTrustee)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return;
	}
	m_UserList[dwChairID]->SetTrustShip(bTrustee);
}

//获得玩家托管状态
CT_BOOL CTableFrameForScore::GetUserTrustee(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return false;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return false;
	}
	return m_UserList[dwChairID]->GetTrustShip();
}

//获得玩家金币
CT_LONGLONG CTableFrameForScore::GetUserScore(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return 0;
	}
	return m_UserList[dwChairID]->GetUserScore();
}

CT_VOID CTableFrameForScore::AddUserScore(CT_DWORD dwChairID, CT_LONGLONG llAddScore, CT_BOOL bBroadcast/* = true*/)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return;
	}

	//更新玩家金币
	AddUserScore(m_UserList[dwChairID], llAddScore);

	//广播玩家金币
	if (bBroadcast)
		BroadcastUserScore(m_UserList[dwChairID]);
}

//设置玩家状态
CT_VOID CTableFrameForScore::SetUserStatus(CT_DWORD dwChairID, USER_STATUS status)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return;
	}
	m_UserList[dwChairID]->SetUserStatus(status);
}

//玩家状态
CT_BYTE CTableFrameForScore::GetUserStatus(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return 0;
	}
	return m_UserList[dwChairID]->GetUserStatus();
}


char* CTableFrameForScore::GetNickName(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return NULL;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return NULL;
	}
	return m_UserList[dwChairID]->GetUserBaseData().szNickName;
}

char* CTableFrameForScore::GetHeadUrl(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return NULL;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return NULL;
	}
	return NULL; //m_UserList[dwChairID]->GetUserBaseData().szHeadUrl;
}

CT_BYTE CTableFrameForScore::GetHeadIndex(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return 0;
	}
	return m_UserList[dwChairID]->GetUserBaseData().cbHeadIndex;
}

CT_BYTE CTableFrameForScore::GetVipLevel(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return 0;
	}
	return m_UserList[dwChairID]->GetUserBaseData().cbVipLevel;
}

// 获取玩家的性别
CT_BYTE CTableFrameForScore::GetUserSex(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return 0;
	}
	return m_UserList[dwChairID]->GetUserBaseData().cbGender;
}

CT_DWORD CTableFrameForScore::GetUserID(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return 0;
	}
	return m_UserList[dwChairID]->GetUserID();
}

//获得当前局数
CT_WORD CTableFrameForScore::GetCurrPlayCount()
{
	return m_PrivateTableInfo.wCurrPlayCount;
}

//设置玩家准备状态
CT_VOID CTableFrameForScore::SetUserReady(CT_DWORD dwChairID)
{
	CServerUserItem* pUserItem = m_UserList[dwChairID];
	if (pUserItem == NULL)
	{
		return;
	}

	pUserItem->SetUserStatus(sReady);
	m_pTableFrameSink->OnUserReady(dwChairID, false);

	if (CheckGameStart())
	{
		if (GetGameRoundPhase() == en_GameRound_Free)
		{
			GameRoundStartDeploy();
		}
		GameStart();
	}
	else
	{
		BroadcastUserState(pUserItem, true);
	}
}

//玩家离开
void CTableFrameForScore::OnUserLeft(CServerUserItem* pUser, bool bSendStateMyself, bool bForceLeave)
{
	if (pUser == NULL)
	{
		return;
	}

	if (m_GameRoundPhase == en_GameRound_Start)
	{
		pUser->SetUserStatus(sOffLine);
		pUser->SetTrustShip(true);
		BroadcastUserState(pUser, bSendStateMyself);
		DelGameUserToProxy(pUser);
		pUser->SetUserCon(NULL);
		pUser->SetClientNetAddr(0);
		m_pTableFrameSink->OnUserLeft(pUser->GetChairID(), false);
	}
	else
	{
		m_pTableFrameSink->OnUserLeft(pUser->GetChairID(), false);
		UserStandUp(pUser, true, bSendStateMyself);
		DelUserOnlineInfo(pUser->GetUserID());
		DelGameUserToProxy(pUser);
		CServerUserManager::get_instance().DeleteUserItem(pUser);

		//人数达到要求，开始游戏
		if (m_pRoomKindInfo->wGameID != GAME_BR && CheckGameStart())
		{
			if (m_GameRoundPhase == en_GameRound_Free)
			{
				GameRoundStartDeploy();
			}
			GameStart();
		}
	}

}

//玩家离线
void CTableFrameForScore::OnUserOffLine(CServerUserItem* pUser)
{
	if (pUser == NULL)
	{
		return;
	}

	if (m_GameRoundPhase == en_GameRound_Start)
	{
		pUser->SetUserStatus(sOffLine);
		pUser->SetTrustShip(true);
		BroadcastUserState(pUser, true);
		DelGameUserToProxy(pUser);
		pUser->SetUserCon(NULL);
		pUser->SetClientNetAddr(0);
	}
	else
	{
		UserStandUp(pUser, true, false);
		DelUserOnlineInfo(pUser->GetUserID());
		DelGameUserToProxy(pUser);
		CServerUserManager::get_instance().DeleteUserItem(pUser);

		//人数达到要求，开始游戏
		if (CheckGameStart())
		{
			if (m_GameRoundPhase == en_GameRound_Free)
			{
				GameRoundStartDeploy();
			}
			GameStart();
		}
	}
}

CT_VOID CTableFrameForScore::ClearTableUser(CT_DWORD wChairID/* = INVALID_CHAIR*/, CT_BOOL bSendState/* = false*/, CT_BOOL bSendStateMyself/* = false*/, CT_BYTE cbSendErrorCode/* = 0*/)
{
	//踢掉某个玩家
	if (wChairID != INVALID_CHAIR)
	{
		CServerUserItem* pUserItem = m_UserList[wChairID];
		if (pUserItem == NULL)
		{
			return;
		}
		if (cbSendErrorCode != 0)
		{
			MSG_SC_GameErrorCode gameErrorCode;
			gameErrorCode.dwMainID = MSG_FRAME_MAIN;
			gameErrorCode.dwSubID = SUB_S2C_ENTER_ROOM_FAIL;
			gameErrorCode.iErrorCode = cbSendErrorCode;
			SendUserMsg(pUserItem, &gameErrorCode, sizeof(gameErrorCode));
		}
		UserStandUp(pUserItem, bSendState, bSendStateMyself);
		DelUserOnlineInfo(pUserItem->GetUserID());
		if (!pUserItem->IsAndroid())
		{
			DelGameUserToProxy(pUserItem);
			CServerUserManager::get_instance().DeleteUserItem(pUserItem);
		}
		else
		{
			CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
		}

		if (GetRealPlayerCount() == 0 && m_pRoomKindInfo->wGameID != GAME_BR)
		{
			CGameTableManager::get_instance().FreeNomalTable(this);
		}
		return;
	}

	//踢掉某个玩家
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
		{
			continue;
		}

		//踢玩家之前先判断是否可以送救济金
		/*if (!pUserItem->IsAndroid())
		{
			if ((pUserItem->GetUserScore() < CServerCfg::m_BenefitRewardCfg.dwLessScore) && CServerCfg::m_nBenefitStatus != 0)
			{
				GetBenefitReward(pUserItem);
			}
		}*/
		
		//清除离线、金币不足等玩家 维服状态
		if (GetUserStatus(i) == sOffLine || (m_pRoomKindInfo->wGameID != GAME_BR && (GetUserScore(i) < m_pRoomKindInfo->dwEnterMinScore))
			|| m_pRoomKindInfo->cbState == SERVER_STOP)
		{
			if (GetUserScore(i) <= m_pRoomKindInfo->dwEnterMinScore)
			{
				MSG_SC_GameErrorCode gameErrorCode;
				gameErrorCode.dwMainID = MSG_FRAME_MAIN;
				gameErrorCode.dwSubID = SUB_S2C_ENTER_ROOM_FAIL;
				gameErrorCode.iErrorCode = ENTER_ROOM_SCORE_NOT_ENOUGH;
				SendUserMsg(pUserItem, &gameErrorCode, sizeof(gameErrorCode));
			}

			if (m_pRoomKindInfo->cbState == SERVER_STOP)
			{
				MSG_SC_GameErrorCode gameErrorCode;
				gameErrorCode.dwMainID = MSG_FRAME_MAIN;
				gameErrorCode.dwSubID = SUB_S2C_ENTER_ROOM_FAIL;
				gameErrorCode.iErrorCode = ENTER_ROOM_SERVER_STOP;
				SendUserMsg(pUserItem, &gameErrorCode, sizeof(gameErrorCode));
			}

			m_pTableFrameSink->OnUserLeft(i, false); //为了使百人牛牛离开时，删除玩家ID
			UserStandUp(pUserItem, bSendState, true);
			DelUserOnlineInfo(pUserItem->GetUserID());
			if (!pUserItem->IsAndroid())
			{
				DelGameUserToProxy(pUserItem);
				CServerUserManager::get_instance().DeleteUserItem(pUserItem);
			}
			else
			{
				CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
			}
		}
		else
		{
			pUserItem->SetUserStatus(sSit);
		}
	}

	//游戏结束记录机器人玩家已玩局数
	for (CT_WORD i = 0; i < m_pRoomKindInfo->wStartMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
		{
			continue;
		}
		if (pUserItem->IsAndroid())
		{
			pUserItem->AddPlayCount();
			//是否让机器人下线
			IsLetAndroidOffline(pUserItem);
		}
	}

	//检测如果没有真人玩家了，则所有机器人下线
	if (m_pRoomKindInfo->wGameID != GAME_BR && GetRealPlayerCount() == 0)
	{
		for (CT_WORD i = 0; i < wMaxPlayer; ++i)
		{
			CServerUserItem* pUserItem = m_UserList[i];
			if (pUserItem == NULL)
			{
				continue;
			}
			if (pUserItem->IsAndroid())
			{
				UserStandUp(pUserItem, true);
				DelUserOnlineInfo(pUserItem->GetUserID());
				CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
			}
		}

		CGameTableManager::get_instance().FreeNomalTable(this);
	}
}

CT_VOID CTableFrameForScore::UserVoiceChat(CServerUserItem* pUser, CT_CHAR* pChatUrl)
{
	if (pUser == NULL)
	{
		return;
	}

	MSG_SC_Voice_Chat voiceChat;
	voiceChat.dwMainID = MSG_FRAME_MAIN;
	voiceChat.dwSubID = SUB_S2C_VOICE_CHAT;
	voiceChat.dwUserID = pUser->GetUserID();
	_snprintf_info(voiceChat.szChatUrl, sizeof(voiceChat.szChatUrl), "%s", pChatUrl);

	SendAllUserData(&voiceChat, sizeof(MSG_SC_Voice_Chat));
}

CT_VOID CTableFrameForScore::UserTextChat(CServerUserItem* pUser, CT_BYTE cbType, CT_BYTE cbIndex)
{
	if (pUser == NULL)
	{
		return;
	}

	MSG_SC_Text_Chat textChat;
	textChat.dwMainID = MSG_FRAME_MAIN;
	textChat.dwSubID = SUB_S2C_TEXT_CHAT;
	textChat.dwUserID = pUser->GetUserID();
	textChat.cbType = cbType;
	textChat.cbIndex = cbIndex;

	SendAllUserData(&textChat, sizeof(MSG_SC_Voice_Chat));
}

CT_VOID CTableFrameForScore::GameRoundStartDeploy()
{
	m_GameRoundPhase = en_GameRound_Start;
}

CT_VOID CTableFrameForScore::OnUserEnterAction(CServerUserItem* pUserItem, CT_BOOL bDistribute/* = false*/, CT_BOOL bReConnect/* = false*/)
{
	SendUserSitdownFinish(pUserItem, bDistribute);
	//广播自己本人信息
	BroadcastUserInfo(pUserItem);
	//将其它玩家的信息发送至自己
	SendOtherUserDataToUser(pUserItem);
	//广播用户状态
	BroadcastUserState(pUserItem, true);
	//玩家进入
	m_pTableFrameSink->OnUserEnter(pUserItem->GetChairID(), pUserItem->IsLookOn());
	//恢复游戏场景
	m_pTableFrameSink->OnEventGameScene(pUserItem->GetChairID(), pUserItem->IsLookOn());
}

CT_BOOL CTableFrameForScore::UserStandUp(CServerUserItem* pUserItem, CT_BOOL bSendState/* = true*/, CT_BOOL bSendStateMyself/* = false*/)
{
	if (pUserItem == NULL)
	{
		return false;
	}

	CT_WORD wTableMaxPlayerCount = m_pRoomKindInfo->wStartMaxPlayer;
	for (int i = 0; i < wTableMaxPlayerCount; ++i)
	{
		if (m_UserList[i] == pUserItem)
		{
			pUserItem->SetUserStatus(sGetOut);
			if (bSendState)
			{
				BroadcastUserState(pUserItem, bSendStateMyself);
			}
			//金币场更新玩家的输赢数据
			//UpdateUserScoreInfo(pUserItem);
			pUserItem->SetTableID(ERROR_TABLE_ID);
			pUserItem->SetChairID(ERROR_CHAIR_ID);
			m_UserList[i] = NULL;
			return true;
		}
	}
	return false;
}

CT_BOOL CTableFrameForScore::RecordRawInfo(RecordScoreInfo* pRecordScoreInfo, CT_WORD wUserCount, CT_BYTE* pAreaCardType, CT_BYTE cbAreaCount,CT_DWORD dwBankerUserID,\
								  CT_LONGLONG llStock, CT_LONGLONG llAndroidStock, CT_LONGLONG llTodayStock, CT_WORD wSystemAllKillRation, CT_WORD wChangeCardRatio)
{
	MSG_G2DB_Record_Draw_Info DrawInfo;
	memset(&DrawInfo, 0, sizeof(DrawInfo));
	DrawInfo.wGameID = m_pRoomKindInfo->wGameID;
	DrawInfo.wKindID = m_pRoomKindInfo->wKindID;
	DrawInfo.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
	DrawInfo.wTableID = m_TableState.dwTableID;						//桌子ID
	DrawInfo.dwBankerUserID = dwBankerUserID;
	DrawInfo.dwServerID = CServerCfg::m_nServerID;
	DrawInfo.llStock = llStock;
	DrawInfo.llAndroidStock = llAndroidStock;
	DrawInfo.wSystemAllKillRatio = wSystemAllKillRation;
	DrawInfo.wChangeCardRatio = wChangeCardRatio;
	DrawInfo.dwPlayTime = (CT_DWORD)time(NULL) - m_dwStartTime;		//游戏时长
	if (pAreaCardType != NULL)
	{
		memcpy(DrawInfo.cbAreaCardType, pAreaCardType, cbAreaCount);
		for (int i = 0; 0 < 5; i++)
		{
			LOG(WARNING) << DrawInfo.cbAreaCardType[i];
		}
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;
	dwSendSize += sizeof(MSG_G2DB_Record_Draw_Info);

	static MSG_G2CS_PlayGame playGame;
	playGame.wGameID = m_pRoomKindInfo->wGameID;
	playGame.wKindID = m_pRoomKindInfo->wKindID;
	playGame.wRoomKindID = m_pRoomKindInfo->wRoomKindID;

	for (CT_WORD i = 0; i < wUserCount; ++i)
	{
		if (pRecordScoreInfo[i].cbStatus == 0)
			continue;

		MSG_G2DB_Record_Draw_Score DrawScore;
		memset(&DrawScore, 0, sizeof(DrawScore));
		DrawScore.dwUserID = pRecordScoreInfo[i].dwUserID;	//玩家id
		DrawScore.wChairID = i;								//玩家椅子id	
		++DrawInfo.wUserCount;								//游戏参与人数

		//输赢金币
		DrawScore.iScore = pRecordScoreInfo[i].iScore;
		DrawScore.llSourceScore = pRecordScoreInfo[i].llSourceScore;
		for (int j = 0; j < 4; j++)
		{
			DrawScore.iAreaJetton[j] = pRecordScoreInfo[i].iAreaJetton[j];
		}
		//memcpy(DrawScore.iAreaScore, pRecordScoreInfo[i].iAreaScore, sizeof(DrawScore.iAreaScore));
		DrawScore.dwRevenue = pRecordScoreInfo[i].dwRevenue;
		if (NULL != m_UserList[i])
		{
			_snprintf_info(DrawScore.szLocation, sizeof(DrawScore.szLocation), "%s", m_UserList[i]->GetUserLocation());
		}

		//是否是机器人
		DrawScore.cbIsbot = pRecordScoreInfo[i].cbIsAndroid;
		if (DrawScore.cbIsbot)
		{
			++DrawInfo.wAndroidCount;
		}
		else
		{
			//非机器人玩家发送中心服（中心服统计任务用）
			playGame.dwUserID = DrawScore.dwUserID;
			SendCenterMsg(MSG_GCS_MAIN, SUB_G2CS_USER_PLAY_GAME, &playGame, sizeof(playGame));
		}
		memcpy(szBuffer + dwSendSize, &DrawScore, sizeof(MSG_G2DB_Record_Draw_Score));
		dwSendSize += sizeof(MSG_G2DB_Record_Draw_Score);
	}
	memcpy(szBuffer, &DrawInfo, sizeof(MSG_G2DB_Record_Draw_Info));

	//更新DB
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_RECORD_DRAW_INFO, szBuffer, dwSendSize);
	return true;
}

CT_BOOL CTableFrameForScore::RoomSitChair(CServerUserItem* pUserItem, CT_BOOL bDistribute/* = false*/)
{
	if (pUserItem == NULL)
	{
		return false;
	}

	CT_WORD wTableMaxPlayerCount = m_pRoomKindInfo->wStartMaxPlayer;
	for (int i = 0; i < wTableMaxPlayerCount; ++i)
	{
		if (m_UserList[i] != NULL)
		{
			continue;
		}

		m_UserList[i] = pUserItem;
		m_UserList[i]->SetTableID(m_TableState.dwTableID);
		m_UserList[i]->SetChairID(i);
	//	m_UserList[i]->SetPRoomNum(m_PrivateTableInfo.dwRoomNum);
		//设置玩家为准备状态
		if (m_pRoomKindInfo->wGameID == GAME_BR)
		{
			pUserItem->SetUserStatus(sPlaying);
		}
		else if (bDistribute || m_pRoomKindInfo->wGameID == GAME_ZJH)
		{
			pUserItem->SetUserStatus(sReady);
		}
		else
		{
			pUserItem->SetUserStatus(sSit);
		}
		
		OnUserEnterAction(pUserItem, bDistribute);

		SetUserOnlineInfo(pUserItem->GetUserID());

		if (!m_bGameStart && m_pRoomKindInfo->wGameID != GAME_BR)
		{
			//检查是否可以开始
			if (CheckGameStart() == true)
			{
				GameRoundStartDeploy();
				GameStart();
			}
		}
		return true;
	}

	return false;
}

CServerUserItem* CTableFrameForScore::GetTableUserItem(CT_WORD wChairID)
{
	if (wChairID >= m_pRoomKindInfo->wStartMaxPlayer) 
		return NULL;

	//获取用户
	return m_UserList[wChairID];
}

CServerUserItem* CTableFrameForScore::GetTableUserItemByUserID(CT_DWORD dwUserID)
{
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem != NULL)
		{
			if (pUserItem->GetUserID() == dwUserID)
				return pUserItem;
		}
	}

	return NULL;
}

//时间脉冲
void CTableFrameForScore::OnTimePulse()
{
	//游戏定时器计时
	GameTimerMsg();

}

//游戏定时器计时
void CTableFrameForScore::GameTimerMsg()
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (!m_SysTimer[i].isEnabled)
		{//无效
			continue;
		}
		m_SysTimer[i].iCountTime += 1000;
		if (m_SysTimer[i].iCountTime >= m_SysTimer[i].iTimeInterval)
		{
			m_SysTimer[i].iCountTime = m_SysTimer[i].iCountTime - m_SysTimer[i].iTimeInterval;
			m_pTableFrameSink->OnTimerMessage(m_SysTimer[i].wTimerID,m_SysTimer[i].dwParam);
		}
	}
}

//发送游戏消息
/*CT_VOID CTableFrameForScore::SendGameMsg(CServerUserItem* pUser, CT_WORD wSubCmdID)
{
	if (NULL == pUser)
	{
		return;
	}

	MSG_GameMsgDownHead DownData;
	memset(&DownData, 0, sizeof(MSG_GameMsgDownHead));
	DownData.dwMainID = MSG_FRAME_MAIN;
	DownData.dwSubID = wSubCmdID;
	SendUserMsg(pUser, &DownData, sizeof(MSG_GameMsgDownHead));
}*/

//发送游戏消息
CT_VOID CTableFrameForScore::SendGameMsg(CServerUserItem* pUser, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen)
{
	if (NULL==pUser)
	{
		return;
	}
	if (dwLen>GAME_DATA_LEN)
	{
		return;
	}

	if (!pUser->IsAndroid())
	{
		CT_DWORD dwDownSize = sizeof(MSG_GameMsgDownHead);
		static CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
		MSG_GameMsgDownHead DownData;
		memset(&DownData, 0, dwDownSize);
		DownData.dwMainID = MSG_GAME_MAIN;
		DownData.dwSubID = wSubCmdID;
		memcpy(chBuf, &DownData, dwDownSize);
		if (dwLen != 0)
			memcpy(&chBuf[dwDownSize], pBuf, dwLen);

		SendUserMsg(pUser, (MSG_GameMsgDownHead*)chBuf, dwDownSize + dwLen);
	}
	else
	{
		pUser->SendUserMessage(MSG_GAME_MAIN, wSubCmdID, (CT_CHAR*)pBuf, dwLen);
	}

}

CT_VOID CTableFrameForScore::SendUserMsg(CServerUserItem* pUser, MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen)
{
	if (NULL == pUser || NULL == pDownData)
	{
		return;
	}
	pDownData->uValue1 = pUser->GetClientNetAddr();
	pDownData->dwValue2 = 0;
	pUser->SendUserMessage(MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, (CT_CHAR*)pDownData, dwLen);
}

CT_VOID CTableFrameForScore::SendAllUserData(MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen)
{
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (NULL == m_UserList[i])
		{
			continue;
		}

		SendUserMsg(m_UserList[i], pDownData, dwLen);
	}
}

CT_VOID CTableFrameForScore::BroadcastUserInfo(CServerUserItem* pUser)
{
	if (pUser == NULL)
	{
		return;
	}

	MSG_SC_UserBaseInfo_ForScore info;
	info.dwMainID = MSG_FRAME_MAIN;
	info.dwSubID = SUB_S2C_USER_ENTER;
	pUser->GetUserBaseMsgDataEx(info);

	//先发本人信息给自己
	SendUserMsg(pUser, &info, sizeof(MSG_SC_UserBaseInfo_ForScore));

	//向其它游戏中的玩家广播自己本人信息
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i] && pUser != m_UserList[i])
		{
			SendUserMsg(m_UserList[i], &info, sizeof(MSG_SC_UserBaseInfo_ForScore));
		}
	}
}

CT_VOID CTableFrameForScore::BroadcastUserState(CServerUserItem* pUser, CT_BOOL bSendStateMyself/* = false*/)
{
	if (NULL == pUser)
	{
		assert(NULL != pUser);
		return;
	}

	MSG_SC_GameUserStatus gus;
	memset(&gus, 0, sizeof(MSG_SC_GameUserStatus));
	gus.dwMainID = MSG_FRAME_MAIN;
	gus.dwSubID = SUB_S2C_USER_STATUS;
	gus.wChairID = pUser->GetChairID();
	gus.wTableID = pUser->GetTableID();
	gus.dwUserID = pUser->GetUserID();
	gus.usStatus = pUser->GetUserStatus();
	if (bSendStateMyself == true)
	{
		SendAllUserData(&gus, sizeof(MSG_SC_GameUserStatus));
	}
	else
	{
		CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
		for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
		{
			if (NULL == m_UserList[i] || m_UserList[i] == pUser)
			{
				continue;
			}
			SendUserMsg(m_UserList[i], &gus, sizeof(MSG_SC_GameUserStatus));
		}
	}
}

CT_VOID CTableFrameForScore::BroadcastUserScore(CServerUserItem* pUser)
{
	if (NULL == pUser)
	{
		assert(NULL != pUser);
		return;
	}

	//广播给自己.
	BroadcastUserForUser(pUser, pUser);

	//游戏玩家
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i] && pUser != m_UserList[i])
		{
			BroadcastUserForUser(pUser, m_UserList[i]);
		}
	}
}

CT_VOID CTableFrameForScore::BroadcastUserForUser(CServerUserItem* pFromUser, CServerUserItem* pToUser)
{
	if (NULL == pFromUser || NULL == pToUser)
	{
		assert(CT_FALSE);
		return;
	}

	CT_DWORD dwUserID = pFromUser->GetUserID();
	MSG_SC_UserScoreInfo usergameinfo;
	memset(&usergameinfo, 0, sizeof(MSG_SC_UserScoreInfo));
	usergameinfo.dwMainID = MSG_FRAME_MAIN;
	usergameinfo.dwSubID = SUB_S2C_USER_SCORE;
	usergameinfo.dwUserID = dwUserID;
	usergameinfo.wTableID = pFromUser->GetTableID();
	usergameinfo.wChairID = pFromUser->GetChairID();
	usergameinfo.dScore = pFromUser->GetUserScore()*0.01f;
	SendUserMsg(pToUser, &usergameinfo, sizeof(MSG_SC_UserScoreInfo));
}

CT_VOID CTableFrameForScore::SendOtherUserDataToUser(CServerUserItem* pUser)
{
	if (NULL == pUser)
	{
		assert(NULL != pUser);
		return;
	}
	//游戏玩家
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i] && pUser != m_UserList[i])
		{
			MSG_SC_UserBaseInfo_ForScore userbaseinfo;
			userbaseinfo.dwMainID = MSG_FRAME_MAIN;
			userbaseinfo.dwSubID = SUB_S2C_USER_ENTER;
			m_UserList[i]->GetUserBaseMsgDataEx(userbaseinfo);
			SendUserMsg(pUser, &userbaseinfo, sizeof(MSG_SC_UserBaseInfo_ForScore));

			//BroadcastUserForUser(m_UserList[i], pUser);
		}
	}	
}

//游戏消息
void CTableFrameForScore::OnGameEvent(CT_WORD wChairID, CT_DWORD dwSubID, const CT_VOID* pData, CT_WORD wDataSize)
{
	CT_BOOL bRtn = m_pTableFrameSink->OnGameMessage(wChairID,dwSubID,pData,wDataSize);
	if (!bRtn)
	{
		LOG(WARNING) << "game process message fail? subid: " << dwSubID;
	}
}

void CTableFrameForScore::SendUserSitdownFinish(CServerUserItem* pUser, CT_BOOL bDistribute/* = false*/)
{
	if (!bDistribute)
	{
		MSG_SC_EnterRoomSucc enterRoomSucc;
		enterRoomSucc.dwMainID = MSG_FRAME_MAIN;
		enterRoomSucc.dwSubID = SUB_S2C_ENTER_ROOM_SUCC;
		enterRoomSucc.uValue1 = pUser->GetClientNetAddr();
		enterRoomSucc.wGameID = m_pRoomKindInfo->wGameID;
		enterRoomSucc.wKindID = m_pRoomKindInfo->wKindID;
		enterRoomSucc.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
		enterRoomSucc.dCellScore = m_pRoomKindInfo->dwCellScore*0.01f;

		SendUserMsg(pUser, &enterRoomSucc, sizeof(MSG_SC_EnterRoomSucc));
	}

	AddGameUserToProxy(pUser);
}

void CTableFrameForScore::AddGameUserToProxy(CServerUserItem* pUser)
{
	MSG_GS2P_AddGameUserInfo addUserToPS;
	addUserToPS.dwUserID = pUser->GetUserID();
	addUserToPS.dwServerID = CServerCfg::m_nServerID;
	addUserToPS.uClientNetAddr = pUser->GetClientNetAddr();
	pUser->SendUserMessage(MSG_PGS_MAIN, SUB_GS2P_ADD_USER, &addUserToPS, sizeof(addUserToPS));
}

//到proxy删除这个玩家
void CTableFrameForScore::DelGameUserToProxy(CServerUserItem* pUser)
{
	MSG_GS2P_DelGameUser delUserToPS;
	delUserToPS.dwUserID = pUser->GetUserID();
	pUser->SendUserMessage(MSG_PGS_MAIN, SUB_GS2P_DEL_USER, &delUserToPS, sizeof(delUserToPS));
}

CT_VOID CTableFrameForScore::SetUserOnlineInfo(CT_DWORD dwUserID)
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key;
	acl::string att_gameid, att_kindid, att_roomkind, att_roomnum;
	acl::string val_gameid, val_kindid, val_roomkind, val_roomnum;
	std::map<acl::string, acl::string> onlineInfo;

	key.format("online_%u", dwUserID);
	att_gameid.format("%s", "gameid");
	att_kindid.format("%s", "kindid");
	att_roomkind.format("%s", "roomkind");
	att_roomnum.format("%s", "roomnum");

	val_gameid.format("%u", m_pRoomKindInfo->wGameID);
	val_kindid.format("%u", m_pRoomKindInfo->wKindID);
	val_roomkind.format("%u", m_pRoomKindInfo->wRoomKindID);
	val_roomnum.format("%u", CServerCfg::m_nServerID);

	onlineInfo[att_gameid] = val_gameid;
	onlineInfo[att_kindid] = val_kindid;
	onlineInfo[att_roomkind] = val_roomkind;
	onlineInfo[att_roomnum] = val_roomnum;

	m_pRedis->clear();
	if (m_pRedis->hmset(key, onlineInfo) == false)
	{
		LOG(WARNING) << "set user online info fail, userid:" << dwUserID;

		m_pRedis->clear();
		m_pRedis->del_one(key);
		return;
	}
}

CT_VOID CTableFrameForScore::DelUserOnlineInfo(CT_DWORD dwUserID)
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key;
	key.format("online_%u", dwUserID);

	m_pRedis->clear();
	if (m_pRedis->del_one(key) <= 0)
	{
		LOG(WARNING) << "delete user online info fail, userid:" << dwUserID;
		return;
	}
	return;
}

CT_WORD	CTableFrameForScore::GetRealPlayerCount()
{
	CT_WORD  wCount = 0;
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < wMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i] && !m_UserList[i]->IsAndroid())
		{
			++wCount;
		}
	}
	return wCount;
}

CT_WORD	CTableFrameForScore::GetAndroidPlayerCount()
{
	CT_WORD  wCount = 0;
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < wMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i] && m_UserList[i]->IsAndroid())
		{
			++wCount;
		}
	}
	return wCount;
}

CT_WORD CTableFrameForScore::GetPlayerCount()
{
	CT_WORD  wCount = 0;
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < wMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i])
		{
			++wCount;
		}
	}
	return wCount;
}

CT_WORD CTableFrameForScore::GetMaxPlayerCount()
{
	return m_pRoomKindInfo->wStartMaxPlayer;
}

CT_DWORD CTableFrameForScore::GetGameCellScore()
{
	return m_pRoomKindInfo->dwCellScore;
}

tagGameRoomKind* CTableFrameForScore::GetGameKindInfo()
{
	return m_pRoomKindInfo;
}

CT_LONGLONG CTableFrameForScore::GetRevenue()
{
	return m_pRoomKindInfo->dwRevenue;
}

CT_BOOL	CTableFrameForScore::CheckGameStart()
{
	CT_DWORD dwMaxPlayer = GetMaxPlayerCount();

	CT_WORD wReadyUserCount = 0;
	CT_WORD wUserCount = 0;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		//获取用户
		CServerUserItem * pITableUserItem = GetTableUserItem(i);
		if (pITableUserItem == NULL) continue;

		++wUserCount;
		if (pITableUserItem->IsReady() == false)
		{
			return false;
		}

		++wReadyUserCount;
	}

	if (m_pRoomKindInfo->wGameID == GAME_NN)
	{
		if (wReadyUserCount >= 2)
		{
			return true;
		}
	}
	else if (m_pGameKindInfo->wGameID == GAME_ZJH)
	{
		 if (wReadyUserCount >= m_pRoomKindInfo->wStartMinPlayer)
		 {
			 return true;
		 }
	}
	else
	{
		if (dwMaxPlayer == wReadyUserCount)
		{
			return true;
		}
	}

	return false;
}

CT_VOID CTableFrameForScore::GameStart()
{
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (m_UserList[i]!=NULL && m_UserList[i]->IsReady())
		{
			m_UserList[i]->SetUserStatus(sPlaying);
			BroadcastUserState(m_UserList[i], true);
		}
	}
	//将桌子锁定
	/*if (GetTableisLock() == false)
	{
		SetTableLock(true);
	}*/
	m_bGameStart = true;
	m_dwStartTime = (CT_DWORD)time(NULL);
	//++m_PrivateTableInfo.wCurrPlayCount;
	m_pTableFrameSink->OnEventGameStart();
}

CT_BOOL CTableFrameForScore::IsAndroidUser(CT_DWORD dwChairID)
{
	if (dwChairID >= GetMaxPlayerCount())
	{
		return false;
	}


	if (m_UserList[dwChairID])
	{
		return m_UserList[dwChairID]->IsAndroid();
	}

	return false;
}

CT_LONGLONG CTableFrameForScore::GetUserEnterMinScore()
{
	return m_pRoomKindInfo->dwEnterMinScore;
}

CT_LONGLONG CTableFrameForScore::GetUserEnterMaxScore()
{
	return m_pRoomKindInfo->dwEnterMaxScore;
}

CT_VOID CTableFrameForScore::AddUserScore(CServerUserItem* pUserItem, CT_LONGLONG llScore)
{
	if (pUserItem == NULL)
	{
		return;
	}

	if (!pUserItem->IsAndroid())
	{
		acl::string key;
		key.format("account_%u", pUserItem->GetUserID());

		CT_LONGLONG llNewScore = 0;
		m_pRedis->clear();
		if (m_pRedis->hincrby(key, "score", llScore, &llNewScore) == false)
		{
			LOG(WARNING) << "update redis score fail, user id: " << pUserItem->GetUserID();
			return;
		}

		CT_LONGLONG llAddScore = llScore;
		if (llNewScore < 0)
		{
			acl::string strZeroValue("0");
			if (m_pRedis->hset(key, "score", strZeroValue.c_str()) == -1)
			{
				LOG(WARNING) << "update redis score fail, user id: " << pUserItem->GetUserID();
				return;
			}

			//llNewScore = 0;
			llAddScore = llScore - llNewScore;
		}

		CT_LONGLONG llSourceScore = pUserItem->GetUserScore();
		pUserItem->SetUserScore(llNewScore);
		UpdateUserScoreToDB(pUserItem->GetUserID(), llAddScore);
		UpdateUserScoreToCenterServer(pUserItem->GetUserID(), llSourceScore, llAddScore, PLAY_GAME);
	}
	else
	{
		pUserItem->SetUserScore(llScore + pUserItem->GetUserScore());
		UpdateUserScoreToDB(pUserItem->GetUserID(), llScore);
	}

	//广播给自己.
	BroadcastUserForUser(pUserItem, pUserItem);
}

CT_VOID CTableFrameForScore::UpdateUserScoreToDB(CT_DWORD dwUserID, CT_LONGLONG llAddScore)
{
	MSG_UpdateUser_Score updateScoreToDB;
	updateScoreToDB.dwUserID = dwUserID;
	updateScoreToDB.llAddScore = llAddScore;
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_UPDATE_USER_SCORE, &updateScoreToDB, sizeof(updateScoreToDB));
}

CT_VOID CTableFrameForScore::UpdateUserScoreToCenterServer(CT_DWORD dwUserID, CT_LONGLONG llSourceScore, CT_LONGLONG llAddScore, enScoreChangeType enType)
{
	MSG_UpdateUser_Score updateScoreToCenter;
	updateScoreToCenter.dwUserID = dwUserID;
	updateScoreToCenter.llAddScore = llAddScore;
	updateScoreToCenter.wRoomKind = m_pRoomKindInfo->wRoomKindID;
	updateScoreToCenter.enType = enType;
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_UPDATE_USER_SCORE, &updateScoreToCenter, sizeof(updateScoreToCenter));
}

//更新玩家输赢数据
CT_VOID CTableFrameForScore::UpdateUserScoreInfo(CServerUserItem* pUserItem, const ScoreInfo& ScoreData)
{
	/*MSG_Update_ScoreInfo scoreInfo;
	scoreInfo.dwUserID = ScoreData.dwUserID;
	scoreInfo.bIsAndroid = pUserItem->IsAndroid();
	if (enScoreKind_Lost == ScoreData.scoreKind)
	{
		//输了
		++scoreInfo.dwAddLostCount;
	}
	else if (enScoreKind_Win == ScoreData.scoreKind)
	{
		//赢了
		++scoreInfo.dwAddWinCount;
	}
	else if (enScoreKind_Draw == ScoreData.scoreKind)
	{
		//和
		++scoreInfo.dwAddDrawCount;
	}
	else if (enScoreKind_Flee == ScoreData.scoreKind)
	{
		//逃跑
		++scoreInfo.dwAddFleeCount;
	}

	//更新DB
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_UPDATE_USER_SCORE_INFO, &scoreInfo, sizeof(scoreInfo));*/
	//更新中心服
	//SendCenterMsg(MSG_GCS_MAIN, SUB_G2CS_UPDATE_USER_SCORE_INFO,&scoreInfo, sizeof(scoreInfo));
}

CT_VOID CTableFrameForScore::SendUserWinScoreInfo(CServerUserItem* pUserItem, CT_LONGLONG llWinScore)
{
	MSG_G2CS_WinScore_Info winScoreInfo;
	_snprintf_info(winScoreInfo.szNickName, sizeof(winScoreInfo.szNickName), "%s", pUserItem->GetUserNickName());
	_snprintf_info(winScoreInfo.szServerName, sizeof(winScoreInfo.szServerName), "%s", m_pRoomKindInfo->szRoomKindName);
	winScoreInfo.llWinScore = llWinScore;

	SendCenterMsg(MSG_GCS_MAIN, SUB_G2CS_USER_WIN_SCORE, &winScoreInfo, sizeof(winScoreInfo));
}

//发送DB信息
CT_VOID CTableFrameForScore::SendDBMsg(CT_WORD mainId, CT_WORD subId, CT_VOID* pMsg, CT_WORD wLen)
{
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), mainId, subId, pMsg, wLen);
}

CT_VOID CTableFrameForScore::SendCenterMsg(CT_WORD mainId, CT_WORD subId, CT_VOID* pMsg, CT_WORD wLen)
{
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), mainId, subId, pMsg, wLen);
}

CT_VOID	CTableFrameForScore::GetBenefitReward(CServerUserItem* pUserItem)
{
	tagBenefitData benefitData;

	acl::string key;
	key.format("benefit_%u", pUserItem->GetUserID());

	std::map<acl::string, acl::string> result;
	m_pRedis->clear();
	if (m_pRedis->hgetall(key, result) == false)
	{
		return;
	}

	benefitData.cbCurrCount = (CT_BYTE)atoi(result["count"].c_str());
	benefitData.dwLastTime = (CT_DWORD)atoi(result["lastTime"].c_str());

	//检查是否同一天
	//std::string now = Utility::GetTimeNowString();
	//std::string totay = now.substr(0, 10);
	//std::string lastDay = benefitData.strLastTime.substr(0, 10);
	CT_DWORD dwNow = time(NULL);
	bool bAcrossDay = isAcrossTheDay(benefitData.dwLastTime, dwNow);

	if (bAcrossDay)
	{
		benefitData.cbCurrCount = 0;
	}

	if (benefitData.cbCurrCount < CServerCfg::m_BenefitRewardCfg.cbRewardCount)
	{
		++benefitData.cbCurrCount;
		//更新玩家缓存结果
		std::map<acl::string, acl::string> benefitInfo;
		acl::string attCount, attTime;
		acl::string valCount, valTime;

		attCount.format("%s", "count");
		valCount.format("%d", benefitData.cbCurrCount);
		attTime.format("%s", "lastTime");
		valTime.format("%u", dwNow);

		benefitInfo[attCount] = valCount;
		benefitInfo[attTime] = valTime;

		m_pRedis->clear();
		if (m_pRedis->hmset(key.c_str(), benefitInfo) == false)
		{
			LOG(WARNING) << "set benefit failed, userid: " << pUserItem->GetUserID();
			return;
		}

		//下发玩家领取结果
		MSG_SC_AutoGet_Benefit benefitReward;
		benefitReward.dwMainID = MSG_FRAME_MAIN;
		benefitReward.dwSubID = SUB_S2C_GET_BENEFIT_REWARD;
		benefitReward.cbCurrBenefitCount = benefitData.cbCurrCount;
		benefitReward.dRewardScore = CServerCfg::m_BenefitRewardCfg.dwRewardScore*TO_DOUBLE;
		SendUserMsg(pUserItem, &benefitReward, sizeof(benefitReward));
		
		//CT_DWORD dwSourceGem = pUserItem->GetUserGem();
		CT_LONGLONG llSourceScore = pUserItem->GetUserScore();

		//更新金币
		AddUserScore(pUserItem, CServerCfg::m_BenefitRewardCfg.dwRewardScore);
		//广播玩家金币
		BroadcastUserScore(pUserItem);

		//到DB更新数据
		MSG_G2GB_Update_BenefitInfo benefitInfoToDB;
		benefitInfoToDB.dwUserID = pUserItem->GetUserID();
		benefitInfoToDB.cbRewardCount = benefitData.cbCurrCount;
		//benefitInfoToDB.dwSourceGem = dwSourceGem;
		benefitInfoToDB.llSourceScore = llSourceScore;
		benefitInfoToDB.dwRewardScore = CServerCfg::m_BenefitRewardCfg.dwRewardScore;
        benefitInfoToDB.dwLastTime = dwNow;
		//_snprintf_info(benefitInfoToDB.dwLastTime, sizeof(benefitInfoToDB.szLastTime), "%s", now.c_str());

		SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_UPDATE_BENEFIT_INFO, &benefitInfoToDB, sizeof(benefitInfoToDB));
	}
}

CT_VOID CTableFrameForScore::AddEarnScoreInfo(CServerUserItem* pUserItem, CT_LONGLONG llAddEarnScore)
{
	acl::string key;
	key.format("earnscore_%u", pUserItem->GetUserID());

	m_pRedis->clear();
	if (m_pRedis->incrby(key, llAddEarnScore) == false)
	{
		LOG(WARNING) << "incrby earn score fail, user id: " << pUserItem->GetUserID();
	}
}

CT_BOOL CTableFrameForScore::IsLetAndroidOffline(CServerUserItem* pUserItem)
{
	if (!pUserItem->IsAndroid())
	{
		return false;
	}
	CT_WORD wLimitCount = 5 + rand() % 10;
	if (pUserItem->GetPlayCount() >= wLimitCount)
	{
		UserStandUp(pUserItem, true);
		CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
		return true;
	}
	return false;
}
