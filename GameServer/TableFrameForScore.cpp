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
#include "GameServerThread.h"
#include "timeFunction.h"

//游戏数据长度
#define	GAME_DATA_LEN	(4096)
extern CNetConnector *pNetDB;
extern CNetConnector *pNetCenter;

CTableFrameForScore::CTableFrameForScore(CGameServerThread*  pGsThread)
	: m_pTableFrameSink(NULL)
	, m_pGsThread(pGsThread)
	, m_pGameKindInfo(NULL)
	, m_pRoomKindInfo(NULL)
	, m_UserList(NULL)
	, m_bGameStart(false)
	, m_GameRoundPhase(en_GameRound_Free)
	, m_pRedis(NULL)
	, m_dwStartTime(0)
	, m_dwAndroidLeaveTime(0)
{
	m_GameRecord.Init();
	memset(&m_TableState, 0, sizeof(m_TableState));
	memset(&m_PrivateTableInfo, 0, sizeof(m_PrivateTableInfo));

	m_benefitReward.dwMainID = MSG_FRAME_MAIN;
	m_benefitReward.dwSubID = SUB_S2C_GET_BENEFIT_REWARD;
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

	if (pRoomKind->wGameID != GAME_FISH)
	{
		m_SysTimer.resize(DW_SYS_TIMER_NUMS);
		m_dwTimerPlus = 1000;
	}
	else
	{
		m_SysTimer.resize(DW_SYS_TIMER_NUMS_FOR_FISH);
        m_dwTimerPlus = 200;
		//m_dwTimerPlus = 100;
	}
	//初始化定时器数据
	for (size_t i = 0; i < m_SysTimer.size(); ++i)
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
CT_BOOL CTableFrameForScore::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf/* = nullptr*/, CT_WORD dwLen/*= 0*/, CT_BOOL bRecord/* = false*/)
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
	
	if (bRecord)
	{
		static CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
		CT_WORD wHeadSize = sizeof(CMD_Command);
		
		if (pBuf != NULL)
		{
			memcpy(chBuf + wHeadSize, pBuf, dwLen);
		}
		CMD_Command*	 pMesHead = (CMD_Command*)chBuf;
		memset(pMesHead, 0, wHeadSize);
		pMesHead->dwDataSize = dwLen;
		pMesHead->dwMainID = MSG_GAME_MAIN;
		pMesHead->dwSubID = wSubCmdID;
		
		m_GameRecord.WriteBytes((CT_CHAR*)chBuf, wHeadSize + dwLen);
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
	CT_WORD iSaveFirstIndex = m_SysTimer.size();
	for (size_t i = 0; i < m_SysTimer.size(); ++i)
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
			m_SysTimer.size() == iSaveFirstIndex)
		{
			iSaveFirstIndex = i;
		}
	}
	if (iSaveFirstIndex < m_SysTimer.size())
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
	for (size_t i = 0; i < m_SysTimer.size(); ++i)
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
	for (size_t i = 0; i < m_SysTimer.size(); ++i)
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
	if (ScoreData.cbCardType != 255 && ScoreData.llRealScore >= m_pRoomKindInfo->iBroadcastScore)
	{
		SendUserWinScoreInfo(m_UserList[dwChairID], ScoreData);
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
		//m_mapReportInfo.clear();
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
	return NULL ;  //m_UserList[dwChairID]->GetUserBaseData().szHeadUrl;
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
	    //捕鱼先调用这个接口
	    if (m_pGameKindInfo->wGameID == GAME_FISH)
        {
            m_pTableFrameSink->OnUserLeft(pUser->GetChairID(), false);
        }

		pUser->SetUserStatus(sOffLine);
		pUser->SetTrustShip(true);
		BroadcastUserState(pUser, bSendStateMyself);
		DelGameUserToProxy(pUser);
		pUser->SetUserCon(NULL);
		pUser->SetClientNetAddr(0);
		if (m_pGameKindInfo->wGameID != GAME_FISH)
        {
            m_pTableFrameSink->OnUserLeft(pUser->GetChairID(), false);
        }
	}
	else
	{
		//玩家退出时先判断是否可以送救济金
		if (!pUser->IsAndroid() && CServerCfg::m_nBenefitStatus != 0)
		{
			tagBenefitReward* pBenefitReward = CServerCfg::GetBenefitReward(pUser->GetUserVip2());
			if (pBenefitReward)
			{
				if (pUser->GetUserScore() + pUser->GetUserBankScore() < pBenefitReward->dwLessScore)
				{
                    CheckBenefitReward(pUser, pBenefitReward);
					/*CT_BOOL bBenefitRward = GetBenefitReward(pUser, pBenefitReward);
					if (bBenefitRward)
					{
						SendUserMsg(pUser, &m_benefitReward, sizeof(MSG_SC_AutoGet_Benefit));
					}*/
				}
			}
		}

		m_pTableFrameSink->OnUserLeft(pUser->GetChairID(), false);
        if (m_TableState.bIsLock)
        {
            m_pGsThread->UpdateGroupPRoomUserInfo(pUser, m_PrivateTableInfo.dwRoomNum, false);
        }
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

		//检测是否
		if (m_pRoomKindInfo->wGameID != GAME_BR && GetPlayerCount() == 0)
        {
            if (m_TableState.bIsLock)
            {
                //m_pGsThread->RemoveGroupPRoom(m_PrivateTableInfo.dwRoomNum);
                m_TableState.bIsLock = false;
                m_PrivateTableInfo.reset();
            }
            CGameTableManager::get_instance().FreeNomalTable(this);
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
		if (m_pGameKindInfo->wGameID == GAME_FISH)
		{
			m_pTableFrameSink->OnUserOffLine(pUser->GetChairID(), false);
		}
		else
		{
			m_pTableFrameSink->OnUserLeft(pUser->GetChairID(), false);
		}
	}
	else
	{
		m_pTableFrameSink->OnUserLeft(pUser->GetChairID(), false);
        if (m_TableState.bIsLock)
        {
            m_pGsThread->UpdateGroupPRoomUserInfo(pUser, m_PrivateTableInfo.dwRoomNum, false);
        }
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

GS_UserBaseData* CTableFrameForScore::GetUserBaseData(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}

	CServerUserItem* pUserItem = m_UserList[dwChairID];
	if (pUserItem == NULL)
	{
		return NULL;
	}

	return &pUserItem->GetUserBaseData();
}

//获取玩家的金币信息
GS_UserScoreData* CTableFrameForScore::GetUserScoreData(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return 0;
	}

	CServerUserItem* pUserItem = m_UserList[dwChairID];
	if (pUserItem == NULL)
	{
		return NULL;
	}

	return &pUserItem->GetUserScoreData();
}

//发送邮件
CT_VOID CTableFrameForScore::SendUserMail(tagUserMail* pUserMail)
{
    if (pUserMail == NULL)
        return;

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_SEND_MAIL, pUserMail, sizeof(tagUserMail));
}

//获取玩家椅子号(返回INVALID_CHAIR, 说明不在线)
CT_WORD CTableFrameForScore::GetUserChairID(CT_DWORD dwUserID)
{
    CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
    if (pUserItem == NULL)
    {
        return INVALID_CHAIR;
    }

    return pUserItem->GetChairID();
}

CT_VOID CTableFrameForScore::ClearTableUser(CT_DWORD wChairID/* = INVALID_CHAIR*/, CT_BOOL bSendState/* = false*/, CT_BOOL bSendStateMyself/* = false*/, CT_BYTE cbSendErrorCode/* = 0*/)
{
	//踢掉某个玩家
	if (wChairID != INVALID_CHAIR)
	{
		/*
		 * 这个条件判断里面不应该有m_pTableFrameSink->OnUserLeft(i, false);
		 */

		CServerUserItem* pUserItem = m_UserList[wChairID];
		if (pUserItem == NULL)
		{
			return;
		}

		//踢玩家之前先判断是否可以送救济金
		//CT_BOOL bBenefitRward = false;
		//CT_UINT64 uClientSock = pUserItem->GetClientNetAddr();
		//acl::aio_socket_stream* pUserProxySock = pUserItem->GetUserCon();
		if (!pUserItem->IsAndroid() && CServerCfg::m_nBenefitStatus != 0)
		{
			tagBenefitReward* pBenefitReward = CServerCfg::GetBenefitReward(pUserItem->GetUserVip2());
			if (pBenefitReward)
			{
				if (pUserItem->GetUserScore() + pUserItem->GetUserBankScore() < pBenefitReward->dwLessScore)
				{
                    CheckBenefitReward(pUserItem, pBenefitReward);
					//bBenefitRward = GetBenefitReward(pUserItem, pBenefitReward);
				}
			}
		}

		if (cbSendErrorCode != 0)
		{
			MSG_SC_GameErrorCode gameErrorCode;
			gameErrorCode.dwMainID = MSG_FRAME_MAIN;
			gameErrorCode.dwSubID = SUB_S2C_ENTER_ROOM_FAIL;
			gameErrorCode.iErrorCode = cbSendErrorCode;
			SendUserMsg(pUserItem, &gameErrorCode, sizeof(gameErrorCode));
		}

        if (m_TableState.bIsLock)
        {
            m_pGsThread->UpdateGroupPRoomUserInfo(pUserItem, m_PrivateTableInfo.dwRoomNum, false);
        }
		UserStandUp(pUserItem, bSendState, bSendStateMyself);
		if (!pUserItem->IsAndroid())
		{
			DelUserOnlineInfo(pUserItem->GetUserID());
			DelGameUserToProxy(pUserItem);
            CServerUserManager::get_instance().DeleteUserItem(pUserItem);
		}
		else
		{
			CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
		}

		if (GetPlayerCount() == 0 && m_pRoomKindInfo->wGameID != GAME_BR)
		{
			if (m_pRoomKindInfo->wGameID == GAME_NN || m_pRoomKindInfo->wGameID == GAME_ZJH)
			{
				ListTableFrame& listUseTable = CGameTableManager::get_instance().GetUsedTableFrame();
				if (listUseTable.size() > 1)
				{
                    if (m_TableState.bIsLock)
                    {
                        //m_pGsThread->RemoveGroupPRoom(m_PrivateTableInfo.dwRoomNum);
                        m_TableState.bIsLock = false;
                        m_PrivateTableInfo.reset();
                    }
					CGameTableManager::get_instance().FreeNomalTable(this);
				}
			}
			else
			{
                if (m_TableState.bIsLock)
                {
                    //m_pGsThread->RemoveGroupPRoom(m_PrivateTableInfo.dwRoomNum);
                    m_TableState.bIsLock = false;
                    m_PrivateTableInfo.reset();
                }
				CGameTableManager::get_instance().FreeNomalTable(this);
			}
		}

		//如果有救济金奖励,则下通知
		/*
		if (bBenefitRward)
		{
			m_benefitReward.uValue1 = uClientSock;
			m_pGsThread->SendMsg(pUserProxySock, &m_benefitReward, sizeof(MSG_SC_AutoGet_Benefit));
		}
		*/

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
		//CT_BOOL bBenefitRward = false;
		//CT_UINT64 uClientSock = pUserItem->GetClientNetAddr();
		//acl::aio_socket_stream* pUserProxySock = pUserItem->GetUserCon();
		if (!pUserItem->IsAndroid() && CServerCfg::m_nBenefitStatus != 0)
		{
            tagBenefitReward* pBenefitReward = CServerCfg::GetBenefitReward(pUserItem->GetUserVip2());
            if (pBenefitReward)
            {
                if (pUserItem->GetUserScore() + pUserItem->GetUserBankScore() < pBenefitReward->dwLessScore)
                {
					//bBenefitRward = GetBenefitReward(pUserItem, pBenefitReward);
					CheckBenefitReward(pUserItem, pBenefitReward);
                }
            }
		}
		
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
            if (m_TableState.bIsLock)
            {
                m_pGsThread->UpdateGroupPRoomUserInfo(pUserItem, m_PrivateTableInfo.dwRoomNum, false);
            }
			UserStandUp(pUserItem, bSendState, true);
			if (!pUserItem->IsAndroid())
			{
				DelUserOnlineInfo(pUserItem->GetUserID());
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
            //炸金花主动换桌
            if (m_pGameKindInfo->wGameID == GAME_ZJH && IsAndroidUser(i) == false)
            {
                GS_UserScoreData* pUserScoreData = GetUserScoreData(i);
                if (pUserScoreData)
                {
                    if (pUserScoreData->dwPlayCount == pUserScoreData->dwChangeTableCount)
                    {
						pUserScoreData->dwChangeTableCount = (rand() % 5+1) + 5;
						pUserScoreData->dwPlayCount = 0;
                        m_pGsThread->OnUserChangeTable(pUserItem->GetUserID(), true);
						continue;
					}
                }
            }
			pUserItem->SetUserStatus(sSit);
		}

		//如果有救济金奖励,则下通知
		/*if (bBenefitRward)
		{
			m_benefitReward.uValue1 = uClientSock;
			m_pGsThread->SendMsg(pUserProxySock, &m_benefitReward, sizeof(MSG_SC_AutoGet_Benefit));
		}*/
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

			//抢庄牛牛每次只能让一个机器人下桌
			if (m_pRoomKindInfo->wGameID == GAME_NN || m_pRoomKindInfo->wGameID == GAME_ZJH)
			{
				break;
			}
		}
	}
	
	if (m_pRoomKindInfo->wGameID == GAME_NN || m_pRoomKindInfo->wGameID == GAME_ZJH)
	{
		//牛牛时常保留机器人在线玩游戏
		CT_WORD wRealPlayer = GetRealPlayerCount();
		CT_WORD wMaxAnroidCount = CAndroidUserMgr::get_instance().GetActiveAndroidCount();
		CT_WORD wLeftAndroidCount = 0;

		//如果有设定机器人数量，则随机机器人数量
		if (wMaxAnroidCount != 0)
		{
			if (wRealPlayer == 1 || wRealPlayer == 0)
			{
				wLeftAndroidCount = wMaxAnroidCount;
			}
			else if (wRealPlayer == 2)
			{
				wLeftAndroidCount = 1;
			}
			else if (wRealPlayer == 3)
			{
				wLeftAndroidCount = rand() % 2 ? 1 : 0;
			}
			else
			{
				wLeftAndroidCount = 0;
			}
		}
		
		for (CT_WORD i = 0; i < wMaxPlayer; ++i)
		{
			CServerUserItem* pUserItem = m_UserList[i];
			if (pUserItem == NULL)
			{
				continue;
			}

			CT_WORD dwAndroidCount = GetAndroidPlayerCount();
			if (dwAndroidCount <= wLeftAndroidCount)
				break;

			if (pUserItem->IsAndroid())
			{
				//LOG(WARNING) << "decrease android count.";
				UserStandUp(pUserItem, true);
				//DelUserOnlineInfo(pUserItem->GetUserID());
				CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
			}
		}

		//抢庄牛牛保留
		if (GetPlayerCount() == 0)
		{
			ListTableFrame& listUseTable = CGameTableManager::get_instance().GetUsedTableFrame();
			if (listUseTable.size() > 1)
			{
				LOG(WARNING) << "free table, table id: " << GetTableID();
                if (m_TableState.bIsLock)
                {
                    //m_pGsThread->RemoveGroupPRoom(m_PrivateTableInfo.dwRoomNum);
                    m_TableState.bIsLock = false;
                    m_PrivateTableInfo.reset();
                }
				CGameTableManager::get_instance().FreeNomalTable(this);
			}
		}
	}
	else if (m_pRoomKindInfo->wGameID != GAME_BR && GetRealPlayerCount() == 0)
	{
		//检测如果没有真人玩家了，则所有机器人下线
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
				//DelUserOnlineInfo(pUserItem->GetUserID());
				CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
			}
		}

		//释放这张桌子
        if (m_TableState.bIsLock)
        {
            //m_pGsThread->RemoveGroupPRoom(m_PrivateTableInfo.dwRoomNum);
            m_TableState.bIsLock = false;
            m_PrivateTableInfo.reset();
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
	BroadcastUserInfo(pUserItem, bReConnect);
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
	DrawInfo.llTodayStock = llTodayStock;
	DrawInfo.llAndroidStock = llAndroidStock;
	DrawInfo.wChangeCardRatio = wChangeCardRatio;
	DrawInfo.wSystemAllKillRatio = wSystemAllKillRation;
	CT_DWORD dwNow = (CT_DWORD)time(NULL);
	DrawInfo.dwPlayTime =  (m_dwStartTime != 0 ? (dwNow - m_dwStartTime) : 1);		//游戏时长
	DrawInfo.dwRecordTime = dwNow;						//当前时间
	if (pAreaCardType != NULL)
	{
		memcpy(DrawInfo.cbAreaCardType, pAreaCardType, cbAreaCount);
		/*for (int i = 0; i < 5; i++)
		{
			LOG(WARNING) << (int)DrawInfo.cbAreaCardType[i];
		}*/
	}

	static CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;
	dwSendSize += sizeof(MSG_G2DB_Record_Draw_Info);

	static MSG_G2CS_PlayGame playGame;
	playGame.wGameID = m_pRoomKindInfo->wGameID;
	playGame.wKindID = m_pRoomKindInfo->wKindID;
	playGame.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
	playGame.dwRecordTime = DrawInfo.dwRecordTime;

	for (CT_WORD i = 0; i < wUserCount; ++i)
	{
		if (pRecordScoreInfo[i].cbStatus == 0)
			continue;

		//捕鱼的时长另外计算
		if (m_pRoomKindInfo->wGameID == GAME_FISH)
		{
			CServerUserItem* pUserItem = GetTableUserItem(i);
			if (pUserItem != NULL && !pUserItem->IsAndroid())
			{
				GS_UserScoreData& userScoreData = pUserItem->GetUserScoreData();
				DrawInfo.dwPlayTime = dwNow - userScoreData.dwEnterTime;	//游戏时长

				/*
				if (pRecordScoreInfo[i].iScore >= 0)
				{
					userScoreData.llWinScore += pRecordScoreInfo[i].iScore;
					userScoreData.dwWinCount += 1;
				}
				else
				{
					userScoreData.llLostScore += pRecordScoreInfo[i].iScore;
					userScoreData.dwLostCount += 1;
				}

				if (pRecordScoreInfo[i].dwRevenue != 0)
				{
					userScoreData.dwRevenue += pRecordScoreInfo[i].dwRevenue;
				}
				 */
			}
		}
		else if ((m_pRoomKindInfo->wGameID == GAME_DDZ && m_pRoomKindInfo->wRoomKindID == PRIMARY_ROOM) ||
			(m_pRoomKindInfo->wGameID == GAME_PDK && m_pRoomKindInfo->wRoomKindID == FREE_ROOM))
        {
            CServerUserItem* pUserItem = GetTableUserItem(i);
            if (pUserItem != NULL && !pUserItem->IsAndroid())
            {
                GS_UserBaseData &userBaseData = pUserItem->GetUserBaseData();
                userBaseData.dwTodayPlayCount += 1;

                //设置玩家局数
                IncrbyUserPlayCount(userBaseData.dwUserID);

                if (m_pRoomKindInfo->wRoomKindID == GAME_DDZ)
                {
                    userBaseData.llTotalWinScore += pRecordScoreInfo[i].iScore;
                    //设置玩家的分数
					IncrbyUserTotalWinScore(userBaseData.dwUserID, pRecordScoreInfo[i].iScore);
                }
            }
        }

		static MSG_G2DB_Record_Draw_Score DrawScore;
		memset(&DrawScore, 0, sizeof(DrawScore));
		DrawScore.dwUserID = pRecordScoreInfo[i].dwUserID;	//玩家id
		DrawScore.wChairID = i;								//玩家椅子id	
		++DrawInfo.wUserCount;								//游戏参与人数

		//输赢金币
		DrawScore.iScore = pRecordScoreInfo[i].iScore;
		DrawScore.llSourceScore = pRecordScoreInfo[i].llSourceScore;
		playGame.dwJettonScore = 0;
		for (int j = 0; j < 4; j++)
		{
			DrawScore.iAreaJetton[j] = pRecordScoreInfo[i].iAreaJetton[j];
			
			//真人玩家下注额
			if (!DrawScore.cbIsbot && pRecordScoreInfo[i].iAreaJetton[j] != 0)
            {
                playGame.dwJettonScore += pRecordScoreInfo[i].iAreaJetton[j];
            }
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

			//大额度机器人参与财神降临统计
			if (playGame.dwJettonScore >= 200000)
			{
				playGame.dwUserID = DrawScore.dwUserID;
				playGame.iWinScore = (DrawScore.iScore - DrawScore.dwRevenue);
				playGame.dwRevenue = DrawScore.dwRevenue;
				CT_BYTE cbUserSex = GetUserSex(i);
				playGame.cbSex = (cbUserSex == 0 ? GetUserSexInCache(playGame.dwUserID) : cbUserSex);
				CT_BYTE cbHeadID = GetHeadIndex(i);
				playGame.cbHeadID = (cbHeadID == 0 ? GetHeadIndexInCache(playGame.dwUserID) : cbHeadID);
				SendCenterMsg(MSG_GCS_MAIN, SUB_G2CS_USER_PLAY_GAME, &playGame, sizeof(playGame));
			}
		}
		else
		{
			//非机器人玩家发送中心服（中心服统计任务用）
			playGame.dwUserID = DrawScore.dwUserID;
			playGame.iWinScore = (DrawScore.iScore - DrawScore.dwRevenue);
            playGame.dwRevenue = DrawScore.dwRevenue;
			CT_BYTE cbUserSex = GetUserSex(i);
			playGame.cbSex = (cbUserSex == 0 ? GetUserSexInCache(playGame.dwUserID) : cbUserSex);
			CT_BYTE cbHeadID = GetHeadIndex(i);
			playGame.cbHeadID = (cbHeadID == 0 ? GetHeadIndexInCache(playGame.dwUserID) : cbHeadID);
            SendCenterMsg(MSG_GCS_MAIN, SUB_G2CS_USER_PLAY_GAME, &playGame, sizeof(playGame));
		}
		memcpy(szBuffer + dwSendSize, &DrawScore, sizeof(MSG_G2DB_Record_Draw_Score));
		dwSendSize += sizeof(MSG_G2DB_Record_Draw_Score);
	}
	
	CT_DWORD dwLen = (CT_DWORD)m_GameRecord.GetReadSize();
	if (dwLen > 0)
	{
		const CT_CHAR* pBuf = m_GameRecord.GetReadBuffer();
        //CMD_Command*	 pMesHead = (CMD_Command*)pBuf;
        //LOG(WARNING) << "play record main id: " << pMesHead->dwMainID << ", sub id: " << pMesHead->dwSubID << ", data size: " << pMesHead->dwDataSize;
		DrawInfo.dwRecordLen = dwLen;
		memcpy(szBuffer + dwSendSize, pBuf, dwLen);
		dwSendSize += DrawInfo.dwRecordLen;
		//提交读大小
		m_GameRecord.CommitReadBytes(dwLen);
		//重置缓冲
		m_GameRecord.Compact();
	}
	memcpy(szBuffer, &DrawInfo, sizeof(MSG_G2DB_Record_Draw_Info));

	//更新DB
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_RECORD_DRAW_INFO, szBuffer, dwSendSize);
	
	//InsertReportInfoToDB();
	
	return true;
}

//写红包的详细信息
CT_BOOL CTableFrameForScore::RecordHongBaoSLInfo(RecordHBSLInfo* pHbInfo, RecordHBSLGrabInfo* pGrabInfo, CT_WORD wUserCount)
{
    static CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE];
    CT_DWORD dwSendSize = 0;

    MSG_G2DB_Record_HBSL_Info hbslInfo;
    hbslInfo.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
    memcpy(&hbslInfo, pHbInfo, sizeof(RecordHBSLInfo));
    dwSendSize += sizeof(MSG_G2DB_Record_HBSL_Info);

    for (CT_WORD i = 0; i < wUserCount; ++i)
    {
    	memcpy(szBuffer + dwSendSize, pGrabInfo++, sizeof(RecordHBSLGrabInfo));
        dwSendSize += sizeof(RecordHBSLGrabInfo);
        ++hbslInfo.wUserCount;
    }

    memcpy(szBuffer, &hbslInfo, sizeof(MSG_G2DB_Record_HBSL_Info));

    //发送DB
    SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_RECORD_HBSL_INFO, szBuffer, dwSendSize);

    return true;
}

//写回放的空闲场景
CT_BOOL CTableFrameForScore::WriteGameSceneToRecord(CT_VOID* pData, CT_DWORD dwDataSize, CT_WORD wSubMsgID)
{
	WriteEnterSuccMsg();
	WriteUserEnterMsg();
	
	static CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
	CT_WORD wHeadSize = sizeof(CMD_Command);
	memcpy(chBuf + wHeadSize, pData, dwDataSize);
	
	CMD_Command* pMesHead = (CMD_Command*)chBuf;
	memset(pMesHead, 0, wHeadSize);
	pMesHead->dwDataSize = dwDataSize;
	pMesHead->dwMainID = MSG_GAME_MAIN;
	pMesHead->dwSubID = wSubMsgID;
	
	m_GameRecord.WriteBytes((CT_CHAR*)chBuf, wHeadSize + dwDataSize);
    
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
		else if ( m_pRoomKindInfo->wGameID == GAME_JZNC)
        {
            pUserItem->SetUserStatus(sSit);
        }
		else if (bDistribute
		|| m_pRoomKindInfo->wGameID == GAME_ZJH
		|| m_pRoomKindInfo->wGameID == GAME_MJ
		|| m_pRoomKindInfo->wGameID == GAME_DDZ
		|| m_pRoomKindInfo->wGameID == GAME_PDK)
		{
			pUserItem->SetUserStatus(sReady);
		}
		else
		{
			pUserItem->SetUserStatus(sSit);
		}

		if (pUserItem->IsAndroid()/* && (m_pRoomKindInfo->wGameID == GAME_DDZ || m_pRoomKindInfo->wGameID == GAME_ZJH || m_pRoomKindInfo->wGameID == GAME_FISH)*/)
		{
			IAndroidUserItemSink* pSink = pUserItem->GetAndroidUserItemSink();
			if (pSink)
			{
				pSink->Initialization(this, pUserItem->GetChairID(), pUserItem);
				//斗地主设置AI线程
				if (m_pRoomKindInfo->wGameID == GAME_DDZ)
                {
                    pSink->SetDdzAiThread(CAndroidUserMgr::get_instance().GetDdzAiThread());
                }
			}
		}
		
		OnUserEnterAction(pUserItem, bDistribute);

		if (m_pGameKindInfo->wGameID == GAME_FISH)
		{
			if(m_pGameKindInfo->wKindID == GAME_FISH_SHCS)
            {
                //进入深海捕鱼房间的玩家，读取点控数据
                MSG_GS2DB_ReadDianKongData data;
                data.dwUserID = pUserItem->GetUserID();
                data.dwGameIndex = CGameTableManager::get_instance().GetGameIndex();
                SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_READ_USER_DIANKONG_DATA, &data, sizeof(data));
            }
		    else if(m_pGameKindInfo->wKindID == GAME_FISH_JS)
            {
		        //进入极速捕鱼房间的玩家，读取miss库
		        JSFish_Player_Miss_Info jpmi;
		        jpmi.dwUserID = pUserItem->GetUserID();
		        jpmi.miss = 0;
                SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_READ_USER_MISS_INFO, &jpmi, sizeof(jpmi));
            }
		}

		//真人玩家设置在线信息
		if (!pUserItem->IsAndroid())
		{
			SetUserOnlineInfo(pUserItem->GetUserID());
		}


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
	for (size_t i = 0; i < m_SysTimer.size(); ++i)
	{
		if (!m_SysTimer[i].isEnabled)
		{//无效
			continue;
		}
		m_SysTimer[i].iCountTime += m_dwTimerPlus;
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

CT_VOID CTableFrameForScore::BroadcastUserInfo(CServerUserItem* pUser, CT_BOOL bReConnect)
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

	if (!bReConnect)
	{
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
	usergameinfo.dScore = pFromUser->GetUserScore()*TO_DOUBLE;
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
	if (m_pGameKindInfo->wGameID == GAME_BR)
	{
	    static CT_BYTE szBuff[SYS_NET_SENDBUF_CLIENT];
	    CT_DWORD dwSendSize = 0;
        MSG_GameMsgDownHead gameMsgDownHead;
        gameMsgDownHead.dwMainID = MSG_FRAME_MAIN;
        gameMsgDownHead.dwSubID = SUB_S2C_USER_ENTER;
        memcpy(szBuff + dwSendSize, &gameMsgDownHead, sizeof(MSG_GameMsgDownHead));
        dwSendSize += sizeof(MSG_GameMsgDownHead);

        CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
        int nCount = 0;
        for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
        {
            if (NULL != m_UserList[i] && pUser != m_UserList[i])
            {
                MSG_SC_UserBaseInfo_ForScoreEx userbaseinfo;
                m_UserList[i]->GetUserBaseMsgDataEx2(userbaseinfo);
                memcpy(szBuff + dwSendSize, &userbaseinfo, sizeof(MSG_SC_UserBaseInfo_ForScoreEx));
                dwSendSize += sizeof(MSG_SC_UserBaseInfo_ForScoreEx);
                ++nCount;

                if (nCount >= 100)
                {
                    SendUserMsg(pUser, (MSG_GameMsgDownHead*)szBuff, dwSendSize);

                    dwSendSize = 0;
                    MSG_GameMsgDownHead gameMsgDownHead;
                    gameMsgDownHead.dwMainID = MSG_FRAME_MAIN;
                    gameMsgDownHead.dwSubID = SUB_S2C_USER_ENTER;
                    memcpy(szBuff + dwSendSize, &gameMsgDownHead, sizeof(MSG_GameMsgDownHead));
                    dwSendSize += sizeof(MSG_GameMsgDownHead);
                }
            }
        }

        if (dwSendSize > sizeof(MSG_GameMsgDownHead))
            SendUserMsg(pUser, (MSG_GameMsgDownHead*)szBuff, dwSendSize);
	}
	else
	{
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
		enterRoomSucc.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
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

CT_VOID CTableFrameForScore::IncrbyUserPlayCount(CT_DWORD dwUserID)
{
    if (m_pRedis == NULL)
    {
        return;
    }

    acl::string key;
    key.format("account_%u", dwUserID);

    CT_DWORD dwGameIndex = CGameTableManager::get_instance().GetGameIndex();
    acl::string attriName;
    attriName.format("%d_c", dwGameIndex);

    m_pRedis->clear();
    if (m_pRedis->hincrby(key.c_str(), attriName.c_str(), 1) == false)
    {
        LOG(WARNING) << "IncrbyUserPlayCount fail, userid:" << dwUserID;
        return;
    }
}

CT_VOID CTableFrameForScore::IncrbyUserTotalWinScore(CT_DWORD dwUserID, CT_INT32 iScore)
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key;
	key.format("account_%u", dwUserID);

	CT_DWORD dwGameIndex = CGameTableManager::get_instance().GetGameIndex();
	acl::string attriName;
	attriName.format("%d_s", dwGameIndex);

	m_pRedis->clear();
	if (m_pRedis->hincrby(key.c_str(), attriName.c_str(), iScore) == false)
	{
		LOG(WARNING) << "IncrbyUserTotalWinScore fail, userid:" << dwUserID;
		return;
	}
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

	if (m_pGameKindInfo->wGameID == GAME_ZJH || m_pRoomKindInfo->wGameID == GAME_NN || m_pRoomKindInfo->wGameID == GAME_JZNC)
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
	
	//清空玩家举报信息
	//m_mapReportInfo.clear();
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
		UpdateUserScoreToDB(pUserItem->GetUserID(), llAddScore, false);
		UpdateUserScoreToCenterServer(pUserItem->GetUserID(), llSourceScore, llAddScore, PLAY_GAME);
	}
	else
	{
		pUserItem->SetUserScore(llScore + pUserItem->GetUserScore());
		//UpdateUserScoreToDB(pUserItem->GetUserID(), llScore, true);
	}

	//广播给自己.
	//BroadcastUserForUser(pUserItem, pUserItem);
}

CT_VOID CTableFrameForScore::UpdateUserScoreToDB(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_BOOL bAndroid)
{
	MSG_UpdateUser_Score updateScoreToDB;
	updateScoreToDB.dwUserID = dwUserID;
	updateScoreToDB.llAddScore = llAddScore;
	updateScoreToDB.bAndroid = bAndroid;
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

//更新举报信息
CT_VOID CTableFrameForScore::InsertReportInfoToDB()
{
	/*if (m_mapReportInfo.empty())
		return;
	
	CT_BYTE szBuffer[2048];
	CT_DWORD dwSendSize = 0;
	
	MSG_G2DB_ReportInfo reportInfo;
	for (auto& it : m_mapReportInfo)
	{
		std::set<CT_DWORD >& setBeReport = it.second;
		for (auto& itSet : setBeReport)
		{
			reportInfo.dwUserID = it.first;
			reportInfo.dwBeReportUserID = itSet;
			memcpy(szBuffer + dwSendSize, &reportInfo, sizeof(reportInfo));
			dwSendSize += sizeof(reportInfo);
		}
	}
	
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_INSERT_REPORT, &szBuffer, dwSendSize);
	m_mapReportInfo.clear();*/
}

CT_VOID CTableFrameForScore::SendUserWinScoreInfo(CServerUserItem* pUserItem, const ScoreInfo& ScoreData)
{
	MSG_G2CS_WinScore_Info winScoreInfo;
	_snprintf_info(winScoreInfo.szNickName, sizeof(winScoreInfo.szNickName), "%s", pUserItem->GetUserNickName());
	_snprintf_info(winScoreInfo.szServerName, sizeof(winScoreInfo.szServerName), "%s", m_pRoomKindInfo->szRoomKindName);
	winScoreInfo.llWinScore = ScoreData.llRealScore;
	winScoreInfo.cbCardType = ScoreData.cbCardType;
	winScoreInfo.cbVip2 = pUserItem->GetUserVip2();
	winScoreInfo.wGameID = m_pGameKindInfo->wGameID * 100 + m_pGameKindInfo->wKindID;

	SendCenterMsg(MSG_GCS_MAIN, SUB_G2CS_USER_WIN_SCORE, &winScoreInfo, sizeof(winScoreInfo));
}

CT_VOID CTableFrameForScore::SavePlayerDianKongData(MSG_G2DB_User_PointControl &data)
{
	data.dwGameIndex = CGameTableManager::get_instance().GetGameIndex();
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_SAVE_USER_DIANKONG_DATA, &data, sizeof(MSG_G2DB_User_PointControl));
}

CT_VOID CTableFrameForScore::DelPlayerDianKongData(MSG_GS2DB_DelUserDianKongData &data)
{
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_DEL_USER_DIANKONG_DATA, &data, sizeof(MSG_GS2DB_DelUserDianKongData));
}

CT_VOID CTableFrameForScore::SaveFishStatisticsInfo(FishStatisticsInfo info[], int arrSize)
{
	CFishControl::get_instance().ReportFishStatisticsInfo(info, arrSize);
}

//设置捕鱼的炮数
CT_VOID CTableFrameForScore::AddFishFireCount(CT_WORD wChairID, CT_DWORD dwCount)
{
	static MSG_G2CS_PlayGame playGame;
	playGame.wGameID = m_pRoomKindInfo->wGameID;
	playGame.wKindID = m_pRoomKindInfo->wKindID;
	playGame.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
	playGame.dwJettonScore = dwCount;
	playGame.dwUserID = GetUserID(wChairID);
	SendCenterMsg(MSG_GCS_MAIN, SUB_G2CS_FISH_FIRE_COUNT, &playGame, sizeof(playGame));
}

//设置黑名单
CT_VOID CTableFrameForScore::SetBlackListUser(tagBrGameBlackList* pBlackListUser)
{
    MSG_GS2DB_BlackList_User blackListUser;
    blackListUser.dwServerID = CServerCfg::m_nServerID;
    blackListUser.dwUserID = pBlackListUser->dwUserID;
    blackListUser.dwControlScore = pBlackListUser->dwControlScore;
    blackListUser.wUserLostRatio = pBlackListUser->wUserLostRatio;
    SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_BLACK_LIST_USER, &blackListUser, sizeof(MSG_GS2DB_BlackList_User));
}

CT_VOID CTableFrameForScore::SaveStockControlInfo(tagStockControlInfo* pStockControlInfo)
{
	CT_DWORD dwServerID = (m_pRoomKindInfo->wGameID == GAME_FXGZ ? CServerCfg::m_nServerID*100+pStockControlInfo->dwServerID : CServerCfg::m_nServerID);
	MSG_GS2DB_SaveFishControlData data;
	data.llBloodPoolDianKongWinLose = pStockControlInfo->llDianKongWinLost;
	data.llTotalKuCun = pStockControlInfo->llStock;
	data.llTotalLoseScore = pStockControlInfo->llTotalLostScore;
	data.llTotalTax = pStockControlInfo->llBlackTax;
	data.llTotalWinScore = pStockControlInfo->llTotalWinScore;
	data.nServerID = dwServerID;
	data.nKunCunStatus = pStockControlInfo->iStockStatus;
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_CONTROL_DATA, &data, sizeof(MSG_GS2DB_SaveFishControlData));

	if (m_pRoomKindInfo->wGameID == GAME_FXGZ)
    {
        MSG_GS2DB_SaveFishKuCun saveFishKuCun;
        saveFishKuCun.serverID = dwServerID;
        saveFishKuCun.llCurKuCun = pStockControlInfo->llStock;
        saveFishKuCun.llCurTime = time(NULL);
        SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_CURRENT_KUCUN, &saveFishKuCun, sizeof(MSG_GS2DB_SaveFishKuCun));
        //LOG(INFO) << "save current stock.";
    }
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

CT_BOOL	CTableFrameForScore::GetBenefitReward(CServerUserItem* pUserItem, tagBenefitReward* pBenefitReward)
{
	GS_UserBaseData& userBaseData = pUserItem->GetUserBaseData();

	/*acl::string key;
	key.format("benefit_%u", pUserItem->GetUserID());

	std::map<acl::string, acl::string> result;
	m_pRedis->clear();
	if (m_pRedis->hgetall(key, result) == false)
	{
		return;
	}

	benefitData.cbCurrCount = (CT_BYTE)atoi(result["count"].c_str());
	benefitData.dwLastTime =  (CT_DWORD)atoi(result["lastTime"].c_str());*/

	//检查是否同一天
	//std::string now = Utility::GetTimeNowString();
	//std::string totay = now.substr(0, 10);
	//std::string lastDay = benefitData.strLastTime.substr(0, 10);

	CT_DWORD dwNow = time(NULL);
	bool bAcrossDay = isAcrossTheDay(userBaseData.dwBenefitLastTime, dwNow);

	if (bAcrossDay)
	{
		userBaseData.cbBenefitCount = 0;
	}

	if (userBaseData.cbBenefitCount < pBenefitReward->cbRewardCount)
	{
		++userBaseData.cbBenefitCount;
		userBaseData.dwBenefitLastTime = dwNow;

		acl::string key;
		key.format("account_%u", pUserItem->GetUserID());

		//更新玩家缓存结果
		std::map<acl::string, acl::string> benefitInfo;
		acl::string attCount, attTime;
		acl::string valCount, valTime;

		attCount.format("%s", "almsc");
		valCount.format("%d", userBaseData.cbBenefitCount);
		attTime.format("%s", "almstime");
		valTime.format("%u", dwNow);

		benefitInfo[attCount] = valCount;
		benefitInfo[attTime] = valTime;

		m_pRedis->clear();
		if (m_pRedis->hmset(key.c_str(), benefitInfo) == false)
		{
			LOG(WARNING) << "set benefit failed, userid: " << pUserItem->GetUserID();
			return false;
		}

		//下发玩家领取结果
		//MSG_SC_AutoGet_Benefit benefitReward;
		//benefitReward.dwMainID = MSG_FRAME_MAIN;
		//benefitReward.dwSubID = SUB_S2C_GET_BENEFIT_REWARD;
		m_benefitReward.cbCurrBenefitCount = userBaseData.cbBenefitCount;
		m_benefitReward.cbRemainBenefitCount = pBenefitReward->cbRewardCount - userBaseData.cbBenefitCount;
		m_benefitReward.dRewardScore = pBenefitReward->dwRewardScore*TO_DOUBLE;
		//SendUserMsg(pUserItem, &benefitReward, sizeof(benefitReward));
		
		//CT_DWORD dwSourceGem = pUserItem->GetUserGem();
		CT_LONGLONG llSourceScore = pUserItem->GetUserScore();

		//更新金币
		AddUserScore(pUserItem, pBenefitReward->dwRewardScore);
		//广播玩家金币
		BroadcastUserScore(pUserItem);

		//到DB更新数据
		MSG_G2GB_Update_BenefitInfo benefitInfoToDB;
		benefitInfoToDB.dwUserID = pUserItem->GetUserID();
		benefitInfoToDB.cbRewardCount = userBaseData.cbBenefitCount;
		//benefitInfoToDB.dwSourceGem = dwSourceGem;
		benefitInfoToDB.llSourceScore = llSourceScore;
		benefitInfoToDB.dwRewardScore = pBenefitReward->dwRewardScore;
		benefitInfoToDB.dwLastTime = dwNow;
		//_snprintf_info(benefitInfoToDB.szLastTime, sizeof(benefitInfoToDB.szLastTime), "%s", now.c_str());

		SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_UPDATE_BENEFIT_INFO, &benefitInfoToDB, sizeof(benefitInfoToDB));

		return true;
	}

	return false;
}

CT_VOID CTableFrameForScore::CheckBenefitReward(CServerUserItem* pUserItem, tagBenefitReward* pBenefitReward)
{
	GS_UserBaseData& userBaseData = pUserItem->GetUserBaseData();
	if (userBaseData.cbNotifyBenefit == 1)
    {
        return;
    }

	CT_DWORD dwNow = time(NULL);
	bool bAcrossDay = isAcrossTheDay(userBaseData.dwBenefitLastTime, dwNow);

	if (bAcrossDay)
	{
		userBaseData.cbBenefitCount = 0;
	}

	if (userBaseData.cbBenefitCount < pBenefitReward->cbRewardCount)
	{
		userBaseData.cbNotifyBenefit = 1;

        /*acl::string key;
        key.format("account_%u", pUserItem->GetUserID());

        m_pRedis->clear();
        if (m_pRedis->hset(key.c_str(), "almsf", "1") == -1)
        {
            LOG(WARNING) << "set benefit flag failed, userid: " << pUserItem->GetUserID();
            return;
        }*/

		//下发玩家领取结果
		MSG_SC_HasBenefit hasBenefit;
		hasBenefit.dwMainID = MSG_FRAME_MAIN;
		hasBenefit.dwSubID = SUB_S2C_HAS_BENEFIT;
		hasBenefit.cbState = 1;
		hasBenefit.dwValue2 = userBaseData.dwUserID;
		SendUserMsg(pUserItem, &hasBenefit, sizeof(hasBenefit));
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

	CT_WORD wLimitCount = 0;
	if (m_pRoomKindInfo->wGameID == GAME_BR)
	{
        wLimitCount = 20 + rand() % 50;

        //百人游戏，让少于50块钱的玩家提前离开
        CT_LONGLONG llUserScore = pUserItem->GetUserScore();
	    if (llUserScore <= 10000)
        {
	        int nRandLeave = rand() % 2;
	        if (nRandLeave == 0)
            {
	            wLimitCount = 0;
	            //LOG(WARNING) << "let br game user leave table.";
            }
        }
	}
	else
	{
		wLimitCount = 8 + rand() % 15;
	}

	if (pUserItem->GetPlayCount() >= wLimitCount)
	{
		//LOG(WARNING) << "let android offline.";
		UserStandUp(pUserItem, true);
		CAndroidUserMgr::get_instance().DeleteUserItem(pUserItem);
		m_dwAndroidLeaveTime = Utility::GetTime();
		return true;
	}
	return false;
}

CT_VOID CTableFrameForScore::LetAndroidLeaveNoRealPlayerTabel()
{
	CT_DWORD dwNow = Utility::GetTime();

	//距离上次开桌时间已经超过10分钟,则让所有机器人下桌
	if (dwNow - m_dwStartTime < 60 * 10)
		return;

	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
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
			m_dwAndroidLeaveTime = Utility::GetTime();
		}
	}
}

CT_VOID CTableFrameForScore::InsertUserReportInfo(CServerUserItem* pUser, CT_DWORD dwUserID, CT_DWORD dwBeReportUserID)
{
	/*CT_BOOL bReport = false;
	auto it = m_mapReportInfo.find(dwUserID);
	if (it != m_mapReportInfo.end())
	{
		std::set<CT_DWORD>& setBeReportUserList = it->second;
		if (setBeReportUserList.find(dwBeReportUserID) != setBeReportUserList.end())
		{
			bReport = true;
		}
	}
	
	
	MSG_SC_Report_User result;
	result.dwMainID = MSG_FRAME_MAIN;
	result.dwSubID = SUB_S2C_REPORT_USER;
	if (!bReport)
	{
		m_mapReportInfo[dwUserID].insert(dwBeReportUserID);
		result.cbResult = 0;
	}
	else
	{
		result.cbResult = 1;
	}
	
	SendUserMsg(pUser, &result, sizeof(result));*/
}

//获取玩家头像索引
CT_BYTE CTableFrameForScore::GetHeadIndexInCache(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);
	acl::string strHeadIndex;

	m_pRedis->clear();
	if (m_pRedis->hget(key.c_str(), "headid", strHeadIndex) == false)
	{
		return 1;
	}

	return (CT_BYTE)atoi(strHeadIndex.c_str());
}
// 获取玩家的性别
CT_BYTE CTableFrameForScore::GetUserSexInCache(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("account_%u", dwUserID);
	acl::string strSex;

	m_pRedis->clear();
	if (m_pRedis->hget(key.c_str(), "gender", strSex) == false)
	{
		return 1;
	}

	return (CT_BYTE)atoi(strSex.c_str());
}

CT_VOID CTableFrameForScore::WriteVideoCommonPackage(MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen)
{
	static CT_CHAR cbMsg[GAME_DATA_LEN] = { 0 };
	CT_CHAR *pMsg = (CT_CHAR*)pDownData;
	CT_WORD wDownHeadSize = sizeof(MSG_GameMsgDownHead);
	CT_WORD wRealSize = dwLen - wDownHeadSize;
	memcpy(cbMsg, pMsg + wDownHeadSize, wRealSize);
	
	static CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
	CT_WORD wHeadSize = sizeof(CMD_Command);
	memcpy(chBuf + wHeadSize, cbMsg, wRealSize);
	
	CMD_Command*	 pMesHead = (CMD_Command*)chBuf;
	memset(pMesHead, 0, wHeadSize);
	pMesHead->dwDataSize = wRealSize;
	pMesHead->dwMainID = pDownData->dwMainID;
	pMesHead->dwSubID = pDownData->dwSubID;
	
	//LOG(WARNING) << "WriteVideoCommonPackage, main id: " <<  pDownData->dwMainID << ", sub id: " << pDownData->dwSubID;
	m_GameRecord.WriteBytes((CT_CHAR*)chBuf, wRealSize + wHeadSize);
	return;
}

CT_VOID CTableFrameForScore::WriteEnterSuccMsg()
{
    MSG_SC_EnterRoomSucc enterRoomSucc;
    enterRoomSucc.dwMainID = MSG_FRAME_MAIN;
    enterRoomSucc.dwSubID = SUB_S2C_ENTER_ROOM_SUCC;
    enterRoomSucc.uValue1 = 0;
    enterRoomSucc.wGameID = m_pRoomKindInfo->wGameID;
    enterRoomSucc.wKindID = m_pRoomKindInfo->wKindID;
    enterRoomSucc.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
    enterRoomSucc.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
    enterRoomSucc.dCellScore = m_pRoomKindInfo->dwCellScore*TO_DOUBLE;
    
    WriteVideoCommonPackage(&enterRoomSucc, sizeof(enterRoomSucc));
}

CT_VOID CTableFrameForScore::WriteUserEnterMsg()
{
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i])
		{
            MSG_SC_UserBaseInfo_ForScore info;
            info.dwMainID = MSG_FRAME_MAIN;
            info.dwSubID = SUB_S2C_USER_ENTER;
            m_UserList[i]->GetUserBaseMsgDataEx(info);
			
			WriteVideoCommonPackage(&info, sizeof(info));
		}
	}
}

CT_VOID CTableFrameForScore::SaveJSFishInfo(js_fish_info_type info_type, js_fish_save_info &info)
{
	if(info_type == stock_info)
	{
		MSG_GS2DB_SaveFishKuCun mgs;
		mgs.serverID = CServerCfg::m_nServerID;
		mgs.llCurKuCun = info.save_stock.llCurKuCun;
		mgs.llCurTime = info.save_stock.llCurTime;
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_CURRENT_KUCUN, &mgs, sizeof(MSG_GS2DB_SaveFishKuCun));
	}
	else if(info_type == sys_info)
    {
        MSG_GS2DB_SaveFishControlData data;
        data.llBloodPoolDianKongWinLose = info.sys_info.llSystemDiankongValue;
        data.llTotalKuCun = info.sys_info.llSystemStock;
        data.llTotalLoseScore = info.sys_info.llSystemLoseScore;
        data.llTotalTax = info.sys_info.llSystemTax;
        data.llTotalWinScore = info.sys_info.llSystemWinScore;
        data.nServerID = CServerCfg::m_nServerID;
        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_CONTROL_DATA, &data, sizeof(MSG_GS2DB_SaveFishControlData));
    }
    else if(info_type == today_info)
	{
		MSG_GS2DB_FishTotalWinLoseScore data;
		data.llTotalLoseScore = info.todayinfo.llTotalLoseScore;
		data.llTotalWinScore = info.todayinfo.llTotalWinScore;
		data.llTodayTotalKuCun = info.todayinfo.llTodayTotalKuCun;
		data.llTodayTotalTax = info.todayinfo.llTodayTotalTax;
		data.llTodayDianKongWinLoseTotalScore = info.todayinfo.llTodayDianKongWinLoseTotalScore;
		data.serverID = CServerCfg::m_nServerID;
		tm times;
		Utility::getLocalTime(&times, time(NULL));
		data.nYear = times.tm_year + 1900;
		data.nMonth = times.tm_mon + 1;
		data.nDay = times.tm_mday;
		data.dwTotalEnterCount = info.todayinfo.dwTotalEnterCount;
		data.dwTotalWinCount = info.todayinfo.dwTotalWinCount;
		data.dwTotalPoChanCount = info.todayinfo.dwTotalPoChanCount;
		data.dwTotalRewardCount = info.todayinfo.dwTotalRewardCount;
		data.dwTotalXiFenCount = info.todayinfo.dwTotalXiFenCount;
		memset(data.bfInfo, 0, sizeof(data.bfInfo));
		memset(data.proInfo, 0, sizeof(data.proInfo));
		data.cbbfInfoCount = 0;
		data.cbproInfoCount = 0;

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_FISH_TOTALWINLOSESCORE, &data, sizeof(MSG_GS2DB_FishTotalWinLoseScore));
	}
	else if(info_type == miss_info)
	{
		JSFish_Player_Miss_Info jpmi;
		jpmi.miss = info.missInfo.miss;
		jpmi.dwUserID = info.missInfo.dwUserID;
        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SAVE_MISS_INFO, &jpmi, sizeof(JSFish_Player_Miss_Info));
	}
    else
	{
    	LOG(ERROR) << "SaveJSFishInfo 无效的类型: " << (int)info_type;
	}
}