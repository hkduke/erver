#include "TableFrame.h"
#include "GameTableManager.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "ServerCfg.h"
#include "Utility.h"
#include "ServerMgr.h"
#include "GroupMgr.h"
#include <fstream>

//游戏数据长度
#define	GAME_DATA_LEN	(10240)
extern CNetConnector *pNetDB;
extern CNetConnector *pNetCenter;

CTableFrame::CTableFrame()
	: m_pTableFrameSink(NULL)
	, m_pGameKindInfo(NULL)
	, m_pRoomKindInfo(NULL)
	, m_UserList(NULL)
	, m_bGameStart(false)
	, m_GameRoundPhase(en_GameRound_Free)
	, m_GameRoundID(0)
	, m_dwPlayTime(0)
	, m_pRedis(NULL)
{
	m_GameMsg.Init();
	memset(&m_TableState, 0, sizeof(m_TableState));
	memset(&m_PrivateTableInfo, 0, sizeof(m_PrivateTableInfo));
}

CTableFrame::~CTableFrame()
{
	Clear();
}

void CTableFrame::Init(ITableFrameSink* pSink, const TableState& state, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis)
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

	InitApplyVipRoomData();
	InitDismissData();
}

void CTableFrame::InitApplyVipRoomData()
{
	m_mapApplyVipRoom.clear();
}

void CTableFrame::InitDismissData()
{
	m_vecDismissPRoom.resize(m_pRoomKindInfo->wStartMaxPlayer);
	for (int i = 0; i < m_pRoomKindInfo->wStartMaxPlayer; ++i)
	{
		m_vecDismissPRoom[i] = en_Dismiss_Max;				//申请解散房间
	}
	m_dwApplyDismissTime = 0;
	m_dwApplyDismissUserID = 0;
	m_bDismissPRoom = false;
}

void CTableFrame::Clear()
{
	delete[]  m_UserList;
	m_UserList = NULL;
}

//发送数据
/*CT_BOOL CTableFrame::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID)
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
CT_BOOL CTableFrame::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf/* = nullptr*/, CT_WORD dwLen/*= 0*/, CT_BOOL bRecord/* = true*/)
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
		CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
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

		m_GameMsg.WriteBytes((CT_CHAR*)chBuf, wHeadSize + dwLen);
	}
	return true;
}

//设置定时器
CT_BOOL CTableFrame::SetGameTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam/* = 0*/)
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
CT_BOOL CTableFrame::KillGameTimer(CT_WORD wTimerID)
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

//获得桌子信息
void CTableFrame::GetTableInfo(TableState& TableInfo)
{
	TableInfo = m_TableState;
}

//获取桌子ID
CT_DWORD CTableFrame::GetTableID()
{
	return m_TableState.dwTableID;
}

//获得游戏配置数据
void CTableFrame::GetGameCfgData(GameCfgData& CfgData)
{

}

//处理输赢积分
CT_BOOL CTableFrame::WriteUserScore(CT_DWORD dwChairID, const ScoreInfo& ScoreData)
{
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	if (dwChairID>=dwMaxPlayer)
	{
		return false;
	}
	if (NULL==m_UserList[dwChairID])
	{
		return false;
	}
	//玩家ID 
	//CT_DWORD dwUserID = m_UserList[dwChairID]->GetUserID();

	//写内存
	GS_UserScoreData& GameScore = m_UserList[dwChairID]->GetUserScoreData();

	//金币
	GameScore.llAddScore += ScoreData.llScore;

	GS_UserBaseData& BaseData = m_UserList[dwChairID]->GetUserBaseData();
	BaseData.llScore += ScoreData.llScore;

	/*if (enScoreKind_Lost == ScoreData.scoreKind)
	{//输了
		++GameScore.dwLostCount;
	}
	else if (enScoreKind_Win == ScoreData.scoreKind)
	{//赢了.
		++GameScore.dwWinCount;
	}
	else if (enScoreKind_Draw == ScoreData.scoreKind)
	{//和
		++GameScore.dwDrawCount;
	}*/

	//保存玩家每小局信息
	SavePlayCountRecord(m_UserList[dwChairID]->GetUserID(), GetNickName(dwChairID),  dwChairID, ScoreData.llScore);

	//广播玩家金币
	BroadcastUserScore(m_UserList[dwChairID]);

	return true;
}

//解散游戏
CT_BOOL CTableFrame::DismissGame(CT_BYTE cbDismissType)
{
	//直接解散房间
	MSG_SC_DismissPRoomResult dismissPRoom;
	dismissPRoom.dwMainID = MSG_FRAME_MAIN;
	dismissPRoom.dwSubID = SUB_S2C_DISMISS_PROOM_RESULT;
	dismissPRoom.cbOperateResult = cbDismissType;//en_Dismiss_PRoom_ByAdmin;
	dismissPRoom.cbOperateCount = 1;
	dismissPRoom.arrUserID[0] = m_PrivateTableInfo.dwOwnerUserID;
	SendAllUserData(&dismissPRoom, sizeof(dismissPRoom));

	if (m_GameRoundPhase == en_GameRound_Free)
	{
		//退还钻石头
		if (m_PrivateTableInfo.dwGroupID != 0)
		{
			if (m_PrivateTableInfo.cbGroupType == GX_GROUP_ROOM)
			{
				CT_WORD wGem = CGameTableManager::get_instance().GetPRoomNeedGem(m_PrivateTableInfo.wTotalPlayCount);
				CT_DWORD dwMasterID = CGSGroupMgr::get_instance().GetGroupMasterID(m_PrivateTableInfo.dwGroupID);
				CGameTableManager::get_instance().SendBackGroupPRoomGem(m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.dwRecordID, dwMasterID, wGem);
			}
			else
			{
				CGameTableManager::get_instance().SendBackWXGroupPRoomGem(m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.dwRecordID);
			}
		}
		CGameTableManager::get_instance().GoCenterUpdatePRoom(m_pRoomKindInfo->wGameID, m_pRoomKindInfo->wKindID, m_PrivateTableInfo.dwRoomNum, m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.cbGroupType, true, PROOM_END);
		TickoutUserAndClearPRoomInfo();
		return true;
	}

	m_bDismissPRoom = true;
	m_pTableFrameSink->OnEventGameEnd(INVALID_CHAIR, GER_DISMISS);
	return true;
}

//结束游戏
CT_BOOL CTableFrame::ConcludeGame(CT_BYTE cbGameStatus, const CT_CHAR* pDrawInfo)
{
	if (IsPrivateRoom())
	{	
		//私人场
		//第一局结束的时候扣房主的消息
		if (m_PrivateTableInfo.wCurrPlayCount == 1)
		{
			if (!m_bDismissPRoom && m_PrivateTableInfo.dwGroupID == 0)
			{
				CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(m_PrivateTableInfo.dwOwnerUserID);
				if (pUserItem != NULL)
				{
					DeductUserGem(pUserItem);
				}
			}
			else if (m_PrivateTableInfo.dwGroupID != 0)
			{
				if (!m_bDismissPRoom)
				{
					RecordGroupRoomInfo();
				}
				else
				{
					if (m_PrivateTableInfo.cbGroupType == GX_GROUP_ROOM)
					{
						CT_WORD wNeedGem = CGameTableManager::get_instance().GetPRoomNeedGem(m_PrivateTableInfo.wTotalPlayCount);
						CT_DWORD dwMasterID = CGSGroupMgr::get_instance().GetGroupMasterID(m_PrivateTableInfo.dwGroupID);
						CGameTableManager::get_instance().SendBackGroupPRoomGem(m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.dwRecordID, dwMasterID, wNeedGem);
					}
					else
					{
						CGameTableManager::get_instance().SendBackWXGroupPRoomGem(m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.dwRecordID);
					}
				}
			}
		}

		//到中心服务器删除房间（因为到此游戏已经开始,所以只需要清空中心的房间信息，不需要清空群组信息）
		CGameTableManager::get_instance().GoCenterUpdatePRoom(m_pRoomKindInfo->wGameID, m_pRoomKindInfo->wKindID, \
			m_PrivateTableInfo.dwRoomNum, m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.cbGroupType, false, PROOM_END);

		//写录像文件
		if (m_bGameStart)
		{
			SaveVideoToDB();
		}
		m_bGameStart = false;

		//是否结算了
		if (m_pTableFrameSink->IsGameRoundClearing(m_bDismissPRoom))
		{
			if (m_PrivateTableInfo.wCurrPlayCount > 1 || (m_PrivateTableInfo.wCurrPlayCount == 1 && !m_bDismissPRoom))
			{
				UpdatePRoomGameRoundReward();
				SaveGameRoundRecord(pDrawInfo);
				SaveWinLostCount(pDrawInfo);

				if (m_PrivateTableInfo.dwGroupID != 0 && m_PrivateTableInfo.cbGroupType == GX_GROUP_ROOM)
				{
					SendGroupPRoomClearing();
				}
			}
			m_GameRoundPhase = en_GameRound_End;
		}
		else
		{
			//等待下一把开始
			CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
			for (CT_BYTE i = 0; i < wMaxPlayer; ++i)
			{
				if (NULL == m_UserList[i]) continue;

				//m_UserList[i]->SetTrustShip(CT_FALSE);
				m_UserList[i]->SetUserStatus(sSit);
				BroadcastUserState(m_UserList[i]);
			}
		}
	}
	else
	{//娱乐场

	}

	return true;
}

//计算税收
CT_LONGLONG CTableFrame::CalculateRevenue(CT_DWORD dwChairID, CT_LONGLONG llScore)
{
	return 0;
}

void CTableFrame::GetPrivateTableInfo(PrivateTableInfo& privateTableInfo)
{
	//privateTableInfo.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
	//privateTableInfo.dwOwnerUserID = m_PrivateTableInfo.dwOwnerUserID;
	//privateTableInfo.wCurrPlayCount = m_PrivateTableInfo.wCurrPlayCount;
	//privateTableInfo.wTotalPlayCount = m_PrivateTableInfo.wTotalPlayCount;
	//privateTableInfo.wUserCount = m_PrivateTableInfo.wUserCount;
	//privateTableInfo.wEndLimit = m_PrivateTableInfo.wEndLimit;
	//privateTableInfo.wWinLimit = m_PrivateTableInfo.wWinLimit;
	//privateTableInfo.cbIsVip = m_PrivateTableInfo.cbIsVip;
	//_snprintf_info(privateTableInfo.szOtherParam, sizeof(privateTableInfo.szOtherParam), "%s", m_PrivateTableInfo.szOtherParam);
	memcpy(&privateTableInfo, &m_PrivateTableInfo, sizeof(PrivateTableInfo));
}

//用户是否存在
CT_BOOL CTableFrame::IsExistUser(CT_DWORD dwChairID)
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
CT_BOOL CTableFrame::IsPrivateRoom()
{
	return (m_pRoomKindInfo->wRoomKindID == PRIVATE_ROOM)?true:false;
}

//获取私人房间房主椅子id
CT_WORD CTableFrame::GetPRoomOwnerChairID()
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
CT_VOID CTableFrame::CloseUserConnect(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return;
	}

	if (NULL == m_UserList[dwChairID])
	{
		return;
	}

	MSG_GS2P_CloseConnect closeConnect;
	closeConnect.dwUserID = m_UserList[dwChairID]->GetUserID();
	m_UserList[dwChairID]->SendUserMessage(MSG_PGS_MAIN, SUB_GS2P_CLOSE_USER_CONNECT, &closeConnect, sizeof(closeConnect));
}


//设置玩家托管状态
CT_VOID CTableFrame::SetUserTrustee(CT_DWORD dwChairID, CT_BOOL bTrustee)
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
CT_BOOL CTableFrame::GetUserTrustee(CT_DWORD dwChairID)
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
CT_LONGLONG CTableFrame::GetUserScore(CT_DWORD dwChairID)
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

CT_VOID CTableFrame::AddUserScore(CT_DWORD dwChairID, CT_LONGLONG llAddScore, CT_BOOL bBroadcast/* = true*/)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return ;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return ;
	}

	//写内存
	GS_UserScoreData& GameScore = m_UserList[dwChairID]->GetUserScoreData();
	//金币
	GameScore.llAddScore += llAddScore;

	GS_UserBaseData& BaseData = m_UserList[dwChairID]->GetUserBaseData();
	//金币
	BaseData.llScore += llAddScore;

	//广播玩家金币
	if (bBroadcast)
		BroadcastUserScore(m_UserList[dwChairID]);
}

//设置玩家状态
CT_VOID CTableFrame::SetUserStatus(CT_DWORD dwChairID, USER_STATUS status)
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
CT_BYTE CTableFrame::GetUserStatus(CT_DWORD dwChairID)
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


char* CTableFrame::GetNickName(CT_DWORD dwChairID)
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

char* CTableFrame::GetHeadUrl(CT_DWORD dwChairID)
{
	if (dwChairID >= m_pRoomKindInfo->wStartMaxPlayer)
	{
		return NULL;
	}
	if (NULL == m_UserList[dwChairID])
	{
		return NULL;
	}
	return NULL ; //m_UserList[dwChairID]->GetUserBaseData().szHeadUrl;
}

//获取玩家头像索引
CT_BYTE CTableFrame::GetHeadIndex(CT_DWORD dwChairID)
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

CT_BYTE CTableFrame::GetVipLevel(CT_DWORD dwChairID)
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
CT_BYTE CTableFrame::GetUserSex(CT_DWORD dwChairID)
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

CT_DWORD CTableFrame::GetUserID(CT_DWORD dwChairID)
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
CT_WORD CTableFrame::GetCurrPlayCount()
{
	return m_PrivateTableInfo.wCurrPlayCount;
}

CT_VOID CTableFrame::ClearTableUser(CT_DWORD wChairID /*= INVALID_CHAIR*/, CT_BOOL bSendState /*= false*/, CT_BOOL bSendStateMyself /*= false*/, CT_BYTE cbSendErrorCode/* = 0*/)
{
	/*if (m_bGameStart == true)
	{
		return;
	}*/

	if (m_GameRoundPhase != en_GameRound_End)
	{
		LOG(WARNING) << "game round phase is not end, can not clear table user!";
		return;
	}

	TickoutUserAndClearPRoomInfo();
	/*CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
		{
			continue;
		}

		DelUserPRoomNum(pUserItem);
		UserStandUp(pUserItem, false);
		DelGameUserToProxy(pUserItem);
		CServerUserManager::get_instance().DeleteUserItem(pUserItem);
	}

	SetTableLock(false);
	DelPRoomInfo();
	InitPHZData();
	memset(&m_PrivateTableInfo, 0, sizeof(m_PrivateTableInfo));
	memset(&m_TableState, 0, sizeof(m_TableState));*/
}

//设置玩家准备状态
CT_VOID CTableFrame::SetUserReady(CT_DWORD dwChairID)
{
	CServerUserItem* pUserItem = m_UserList[dwChairID];
	if (pUserItem == NULL)
	{
		return;
	}

	pUserItem->SetUserStatus(sReady);

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
		BroadcastUserState(pUserItem);
	}
}

//玩家离开
void CTableFrame::OnUserLeft(CServerUserItem* pUser, bool bSendStateMyself, bool bForceLeave)
{
	if (pUser == NULL)
	{
		return;
	}

	//房卡场强制离开需要发送此消息
	if (bForceLeave)
	{
		SendForceLeaveTableMsg(pUser);
	}

	if (m_GameRoundPhase == en_GameRound_Start || IsPRoomOwner(pUser->GetUserID()))
	{
		pUser->SetUserStatus(sOffLine);
		BroadcastUserState(pUser);
		DelGameUserToProxy(pUser);
		pUser->SetUserCon(NULL);
		pUser->SetClientNetAddr(0);
	}
	else
	{
		//不是房主，则删除房间信息和在线信息
		if (IsPRoomOwner(pUser->GetUserID()) == false)
		{
			//pTableFrame->DelUserPRoomNum(pUserItem->GetUserID());
			DelUserOnlineInfo(pUser->GetUserID());
			DecrPRoomUserCount();
		}

		if (IsPrivateRoom())
			RemoveVipPRoomApplyWhenLogout(pUser->GetUserID());

		UserStandUp(pUser, true, bSendStateMyself);
		DelGameUserToProxy(pUser);
		CServerUserManager::get_instance().DeleteUserItem(pUser);

		//如果这张桌子已经空了
		if (GetPlayerCount() == 0)
		{
			SetTableLock(false);
			static PrivateTableInfo tableInfo;
			memset(&tableInfo, 0, sizeof(tableInfo));
			SetPrivateTableInfo(tableInfo);
		}
	}
}

//玩家离线
void CTableFrame::OnUserOffLine(CServerUserItem* pUser)
{
	if (pUser == NULL)
	{
		return;
	}

	pUser->SetUserStatus(sOffLine);
	BroadcastUserState(pUser);
	DelGameUserToProxy(pUser);
	pUser->SetUserCon(NULL);
	pUser->SetClientNetAddr(0);
}

CServerUserItem* CTableFrame::GetTableUserItem(CT_WORD wChairID)
{
	if (wChairID >= m_pRoomKindInfo->wStartMaxPlayer) 
		return NULL;

	//获取用户
	return m_UserList[wChairID];
}

CServerUserItem* CTableFrame::GetTableUserItemByUserID(CT_DWORD dwUserID)
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
void CTableFrame::OnTimePulse()
{
	//游戏定时器计时
	GameTimerMsg();

	CheckTimeoutDismissPRoom();
}

//游戏定时器计时
void CTableFrame::GameTimerMsg()
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (!m_SysTimer[i].isEnabled)
		{//无效
			continue;
		}
		m_SysTimer[i].iCountTime += 500;
		if (m_SysTimer[i].iCountTime >= m_SysTimer[i].iTimeInterval)
		{
			m_SysTimer[i].iCountTime = m_SysTimer[i].iCountTime - m_SysTimer[i].iTimeInterval;
			m_pTableFrameSink->OnTimerMessage(m_SysTimer[i].wTimerID, m_SysTimer[i].dwParam);
		}
	}
}

//发送游戏消息
/*CT_VOID CTableFrame::SendGameMsg(CServerUserItem* pUser, CT_WORD wSubCmdID)
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
CT_VOID CTableFrame::SendGameMsg(CServerUserItem* pUser, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen)
{
	if (NULL==pUser)
	{
		return;
	}
	if (dwLen>GAME_DATA_LEN)
	{
		return;
	}

	CT_DWORD dwDownSize = sizeof(MSG_GameMsgDownHead);
	CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
	MSG_GameMsgDownHead DownData;
	memset(&DownData, 0, dwDownSize);
	DownData.dwMainID = MSG_GAME_MAIN;
	DownData.dwSubID = wSubCmdID;
	memcpy(chBuf, &DownData, dwDownSize);
	if (dwLen != 0)
		memcpy(&chBuf[dwDownSize], pBuf, dwLen);

	SendUserMsg(pUser, (MSG_GameMsgDownHead*)chBuf, dwDownSize + dwLen);
}

CT_VOID CTableFrame::SendUserMsg(CServerUserItem* pUser, MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen)
{
	if (NULL == pUser || NULL == pDownData)
	{
		return;
	}
	pDownData->uValue1 = pUser->GetClientNetAddr();
	pDownData->dwValue2 = 0;
	pUser->SendUserMessage(MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, (CT_CHAR*)pDownData, dwLen);
}

CT_VOID CTableFrame::SendAllUserData(MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen)
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

CT_VOID CTableFrame::BroadcastUserInfo(CServerUserItem* pUser)
{
	if (pUser == NULL)
	{
		return;
	}

	MSG_SC_UserBaseInfo info;
	memset(&info, 0, sizeof(MSG_SC_UserBaseInfo));
	info.dwMainID = MSG_FRAME_MAIN;
	info.dwSubID = SUB_S2C_USER_ENTER;
	pUser->GetUserBaseMsgData(info);

	//先广播自己本人信息
	SendUserMsg(pUser, &info, sizeof(MSG_SC_UserBaseInfo));

	//向其它游戏中的玩家广播自己本人信息
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i] && pUser != m_UserList[i])
		{
			SendUserMsg(m_UserList[i], &info, sizeof(MSG_SC_UserBaseInfo));
		}
	}
}

CT_VOID CTableFrame::BroadcastUserState(CServerUserItem* pUser, CT_BOOL bSendStateMyself/* = false*/)
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

CT_VOID CTableFrame::BroadcastUserScore(CServerUserItem* pUser)
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

CT_VOID CTableFrame::BroadcastUserForUser(CServerUserItem* pFromUser, CServerUserItem* pToUser)
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

CT_VOID CTableFrame::SendOtherUserDataToUser(CServerUserItem* pUser)
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
			MSG_SC_UserBaseInfo userbaseinfo;
			memset(&userbaseinfo, 0, sizeof(MSG_SC_UserBaseInfo));
			userbaseinfo.dwMainID = MSG_FRAME_MAIN;
			userbaseinfo.dwSubID = SUB_S2C_USER_ENTER;
			m_UserList[i]->GetUserBaseMsgData(userbaseinfo);
			SendUserMsg(pUser, &userbaseinfo, sizeof(MSG_SC_UserBaseInfo));

			//BroadcastUserForUser(m_UserList[i], pUser);
		}
	}	
}

//游戏消息
void CTableFrame::OnGameEvent(CT_WORD wChairID, CT_DWORD dwSubID, const CT_VOID* pData, CT_WORD wDataSize)
{
	CT_BOOL bRtn = m_pTableFrameSink->OnGameMessage(wChairID,dwSubID,pData,wDataSize);
	if (!bRtn)
	{
		LOG(WARNING) << "game process message fail? subid: " << dwSubID;
	}
}

void CTableFrame::SendUserSitdownFinish(CServerUserItem* pUser)
{
	MSG_SC_EnterPRoomSucc enterPRoomSucc;
	enterPRoomSucc.dwMainID = MSG_FRAME_MAIN;
	enterPRoomSucc.dwSubID = SUB_S2C_ENTER_PROOM_SUCC;
	enterPRoomSucc.uValue1 = pUser->GetClientNetAddr();
	enterPRoomSucc.wGameID = m_pRoomKindInfo->wGameID;
	enterPRoomSucc.wKindID = m_pRoomKindInfo->wKindID;
	enterPRoomSucc.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
	enterPRoomSucc.dwRoomNum = pUser->GetPRoomNum();
	enterPRoomSucc.dwRoomOwnerID = GetPRoomOwner();
	enterPRoomSucc.dwCellScore = m_pRoomKindInfo->dwCellScore;
	enterPRoomSucc.cbIsVip = m_PrivateTableInfo.cbIsVip;
	enterPRoomSucc.wUserCount = m_PrivateTableInfo.wUserCount;
	enterPRoomSucc.wPlayCount = m_PrivateTableInfo.wTotalPlayCount;
	_snprintf_info(enterPRoomSucc.szOtherParam, sizeof(enterPRoomSucc.szOtherParam), "%s", m_PrivateTableInfo.szOtherParam);

	SendUserMsg(pUser, &enterPRoomSucc, sizeof(MSG_SC_EnterPRoomSucc));

	AddGameUserToProxy(pUser);
}

void CTableFrame::AddGameUserToProxy(CServerUserItem* pUser)
{
	MSG_GS2P_AddGameUserInfo addUserToPS;
	addUserToPS.dwUserID = pUser->GetUserID();
	addUserToPS.dwServerID = CServerCfg::m_nServerID;
	addUserToPS.uClientNetAddr = pUser->GetClientNetAddr();
	pUser->SendUserMessage(MSG_PGS_MAIN, SUB_GS2P_ADD_USER, &addUserToPS, sizeof(addUserToPS));
}

//到proxy删除这个玩家
void CTableFrame::DelGameUserToProxy(CServerUserItem* pUser)
{
	MSG_GS2P_DelGameUser delUserToPS;
	delUserToPS.dwUserID = pUser->GetUserID();
	pUser->SendUserMessage(MSG_PGS_MAIN, SUB_GS2P_DEL_USER, &delUserToPS, sizeof(delUserToPS));
}

CT_WORD CTableFrame::GetPlayerCount()
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


CT_BOOL	CTableFrame::CheckGameStart()
{
	//CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;

	CT_WORD wReadyUserCount = 0;
	CT_WORD wUserCount = 0;
	for (CT_DWORD i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
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
		if (wReadyUserCount == wUserCount)
		{
			if (m_PrivateTableInfo.wUserCount == 5 && wReadyUserCount >= 2)
			{
				return true;
			}
			if (m_PrivateTableInfo.wUserCount == 8 && wReadyUserCount >= 6)
			{
				return true;
			}
		}
	}
	else
	{
		if (m_PrivateTableInfo.wUserCount == wReadyUserCount)
		{
			return true;
		}
	}

	return false;
}

CT_VOID CTableFrame::GameStart()
{
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (m_UserList[i]!=NULL && m_UserList[i]->IsReady())
		{
			m_UserList[i]->SetUserStatus(sPlaying);
			BroadcastUserState(m_UserList[i]);
		}
	}

	m_bGameStart = true;
	if (IsPrivateRoom())
	{
		WriteEnterSuccMsg();
		WriteUserEnterMsg();
		WriteFreeGameScene();
	}

	++m_PrivateTableInfo.wCurrPlayCount;
	m_pTableFrameSink->OnEventGameStart();
}

CT_VOID CTableFrame::ApplyDismissPRoom(CServerUserItem* pUserItem)
{
	if (pUserItem == NULL)
	{
		return;
	}

	if (m_dwApplyDismissUserID != 0)
	{
		return;
	}

	// 如果游戏没有开始，只有房主才能解散房间
	if (m_GameRoundPhase == en_GameRound_Free)
	{
		if (pUserItem->GetUserID() != GetPRoomOwner())
		{
			LOG(WARNING) << "only private room owner can dismiss room when the game not start, group id: " << (CT_WORD)m_PrivateTableInfo.dwGroupID;
			return;
		}

		//直接解散房间
		MSG_SC_DismissPRoomResult dismissPRoom;
		dismissPRoom.dwMainID = MSG_FRAME_MAIN;
		dismissPRoom.dwSubID = SUB_S2C_DISMISS_PROOM_RESULT;
		dismissPRoom.cbOperateResult = en_Dismiss_PRoom;
		dismissPRoom.cbOperateCount = 1;
		dismissPRoom.arrUserID[0] = pUserItem->GetUserID();
		SendAllUserData(&dismissPRoom, sizeof(dismissPRoom));

		CGameTableManager::get_instance().GoCenterUpdatePRoom(m_pRoomKindInfo->wGameID, m_pRoomKindInfo->wKindID, m_PrivateTableInfo.dwRoomNum, m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.cbGroupType, true, PROOM_END);
		TickoutUserAndClearPRoomInfo();
		return;
	}
	else if (m_GameRoundPhase == en_GameRound_Start)
	{
		//游戏已经开始，则记录
		m_vecDismissPRoom[pUserItem->GetChairID()] = en_Dismiss_PRoom;
		m_dwApplyDismissTime = Utility::GetTime();
		m_dwApplyDismissUserID = pUserItem->GetUserID();
		m_bDismissPRoom = false;

		//广播申请信息
		MSG_SC_ApplyDismissPRoom applyDismissPRoom;
		applyDismissPRoom.dwMainID = MSG_FRAME_MAIN;
		applyDismissPRoom.dwSubID = SUB_S2C_APPLY_DISMISS_PROOM;
		applyDismissPRoom.dwUserID = pUserItem->GetUserID();

		applyDismissPRoom.cbWaitOperate = 0;
		CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
		for (CT_WORD i = 0; i < wMaxPlayer; ++i)
		{
			CServerUserItem* pOtherUserItem = m_UserList[i];
			if (pOtherUserItem == NULL || pOtherUserItem == pUserItem)
			{
				continue;
			}

			applyDismissPRoom.arrWaitUserID[applyDismissPRoom.cbWaitOperate++] = pOtherUserItem->GetUserID();
		}

		SendAllUserData(&applyDismissPRoom, sizeof(applyDismissPRoom));
	}
	else
	{
		LOG(WARNING) << "m_GameRoundPhase: " << (int)m_GameRoundPhase << ", can not apply dismiss proom!";
	}
}

CT_VOID CTableFrame::OperateDismissPRoom(CServerUserItem* pUserItem, CT_BYTE cbOperateCode)
{
	if (pUserItem == NULL)
	{
		return;
	}

	if (m_GameRoundPhase != en_GameRound_Start)
	{
		LOG(WARNING) << "game not start, can not operate dismiss private room. user id: " << pUserItem->GetUserID() << ", code: " << (CT_WORD)cbOperateCode;
		return;
	}

	if (m_dwApplyDismissUserID == 0)
	{
		LOG(WARNING) << "not one apply dismiss private room or the apply had complete, user id: " << pUserItem->GetUserID() << ", code: " << (CT_WORD)cbOperateCode;
		return;
	}

	//记录操作
	m_vecDismissPRoom[pUserItem->GetChairID()] = (enDismissPRoom)cbOperateCode;

	//如果不同意，直接返回不同意
	if (cbOperateCode == en_UnDismiss_PRoom)
	{
		CT_BOOL bDismiss = CheckCanNotDismissPRoom();
		if (bDismiss)
		{
			MSG_SC_DismissPRoomResult dismissPRoom;
			dismissPRoom.dwMainID = MSG_FRAME_MAIN;
			dismissPRoom.dwSubID = SUB_S2C_DISMISS_PROOM_RESULT;
			dismissPRoom.cbOperateResult = en_UnDismiss_PRoom;
			dismissPRoom.cbOperateCount = 0;
			dismissPRoom.arrUserID[dismissPRoom.cbOperateCount++] = pUserItem->GetUserID();
			SendAllUserData(&dismissPRoom, sizeof(dismissPRoom));

			InitDismissData();
			return;
		}
	}

	//如果同意，则结算这场游戏
	CT_BOOL bCanDismiss = CheckCanDismissPRoom();
	if (bCanDismiss)
	{
		MSG_SC_DismissPRoomResult dismissPRoom;
		dismissPRoom.dwMainID = MSG_FRAME_MAIN;
		dismissPRoom.dwSubID = SUB_S2C_DISMISS_PROOM_RESULT;
		dismissPRoom.cbOperateResult = en_Dismiss_PRoom;

		CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
		dismissPRoom.cbOperateCount = 0;
		for (CT_WORD i = 0; i < wMaxPlayer; ++i)
		{
			CServerUserItem* pFindUserItem = m_UserList[i];
			if (pFindUserItem == NULL)
			{
				continue;
			}

			if (m_vecDismissPRoom[i] == en_Dismiss_PRoom)
			{
				dismissPRoom.arrUserID[dismissPRoom.cbOperateCount++] = pFindUserItem->GetUserID();
			}
		}
		
		SendAllUserData(&dismissPRoom, sizeof(dismissPRoom));
		m_bDismissPRoom = true;
		m_pTableFrameSink->OnEventGameEnd(INVALID_CHAIR, GER_DISMISS);
		return;
	}

	//发送当前解散场景
	MSG_SC_OperateDismissPRoom operateDismiss;
	operateDismiss.dwMainID = MSG_FRAME_MAIN;
	operateDismiss.dwSubID = SUB_S2C_OPERATE_DISMISS_PROOM;
	operateDismiss.dwUserID = pUserItem->GetUserID();
	operateDismiss.cbOperate = cbOperateCode;
	SendAllUserData(&operateDismiss, sizeof(operateDismiss));
}

CT_VOID	CTableFrame::CheckTimeoutDismissPRoom()
{
	//10秒钟的缓冲时间
	if (m_dwApplyDismissUserID != 0)
	{
		if (Utility::GetTime() - m_dwApplyDismissTime > (APPLY_DISMISS_PROOM_TIME + 10))
		{
			m_bDismissPRoom = true;
			m_pTableFrameSink->OnEventGameEnd(INVALID_CHAIR, GER_DISMISS);
		}
	}
}

CT_BOOL CTableFrame::CheckCanNotDismissPRoom()
{
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	CT_WORD wDisagreeDissmissCount = 0;
	CT_WORD wPlayerCount = 0;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
			continue;

		wPlayerCount++;

		if (m_vecDismissPRoom[i] == en_UnDismiss_PRoom)
		{
			++wDisagreeDissmissCount;
		}
	}

	switch (m_pRoomKindInfo->wGameID)
	{
	case GAME_PHZ:
	case GAME_PDK:
	case GAME_NN:
	case GAME_MJ:
	case GAME_DDZ:
		if (wDisagreeDissmissCount  != 0)
		{
			return true;
		}
		break;

	/*case GAME_NN:
		if (m_PrivateTableInfo.wUserCount == 5)
		{
			if ((wPlayerCount <= 3 && wDisagreeDissmissCount != 0) ||
				(wPlayerCount > 3 && wDisagreeDissmissCount >= 2))
			{
				return true;
			}
		}
		if (m_PrivateTableInfo.wUserCount == 8)
		{
			if ((wPlayerCount <= 5 && wDisagreeDissmissCount != 0) ||
				(wPlayerCount > 5 && wDisagreeDissmissCount >= 3))
			{
				return true;
			}
		}*/
	default:
		break;
	}


	return false;

}

CT_BOOL CTableFrame::CheckCanDismissPRoom()
{
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	CT_WORD wAgreeDissmissCount = 0;
	CT_WORD wPlayerCount = 0;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
			continue;

		wPlayerCount++;
		if (m_vecDismissPRoom[i] == en_Dismiss_PRoom)
		{
			++wAgreeDissmissCount;
		}
	}

	switch (m_pRoomKindInfo->wGameID)
	{
	case GAME_PHZ:
	case GAME_DDZ:
	case GAME_PDK:
		if (wAgreeDissmissCount >= 2)
		{
			return true;
		}
		break;
	case GAME_MJ:
		if (wAgreeDissmissCount >= 3)
		{
			return true;
		}
		break;

	case GAME_NN:
		if (m_PrivateTableInfo.wUserCount == 5)
		{
			if ((wPlayerCount <= 3 && wAgreeDissmissCount == wPlayerCount) ||
				(wPlayerCount > 3 && wAgreeDissmissCount >= 3))
			{
				return true;
			}
		}
		if (m_PrivateTableInfo.wUserCount == 8)
		{
			if ((wPlayerCount <= 5 && wAgreeDissmissCount == wPlayerCount) ||
				(wPlayerCount > 5 && wAgreeDissmissCount >= 5))
			{
				return true;
			}
		}
		break;
	default:
		break;
	}

	return false;
}

CT_VOID CTableFrame::TickoutUserAndClearPRoomInfo()
{
	if (IsPRoomVipMode())
	{
		for (auto& it : m_mapApplyVipRoom)
		{
			if (it.second.cbState == en_PRoomApply_Agree)
			{
				//把不在线的玩家在线信息清空
				CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(it.first);
				if (pUserItem == NULL)
				{
					//DelUserPRoomNum(it.first);
					DelUserOnlineInfo(it.first);
				}
			}
		}
	}

	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
		{
			continue;
		}

		//DelUserPRoomNum(pUserItem->GetUserID());
		DelUserOnlineInfo(pUserItem->GetUserID());
		UserStandUp(pUserItem, false);
		DelGameUserToProxy(pUserItem);
		CServerUserManager::get_instance().DeleteUserItem(pUserItem);
	}

	SetTableLock(false);
	DelPRoomInfo();
	InitApplyVipRoomData();
	InitDismissData();
	m_pTableFrameSink->ClearGameData();
	memset(&m_PrivateTableInfo, 0, sizeof(m_PrivateTableInfo));
}

CT_VOID CTableFrame::SendDismissPRoomInfo(CServerUserItem* pUser)
{
	MSG_SC_DismissPRoomScene dismissScene;
	dismissScene.dwMainID = MSG_FRAME_MAIN;
	dismissScene.dwSubID = SUB_S2C_DISMISS_PROOM_SCENE;
	dismissScene.dwApplyUserID = m_dwApplyDismissUserID;
	CT_DWORD dwTimeLeft = APPLY_DISMISS_PROOM_TIME - (Utility::GetTime() - m_dwApplyDismissTime);
	dismissScene.dwDismissTimeLeft = (dwTimeLeft > 0) ? dwTimeLeft : 2;
	dismissScene.cbCount = 0;
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL || pUserItem->GetUserID() == m_dwApplyDismissUserID)
		{
			continue;
		}
		dismissScene.arrOperate[dismissScene.cbCount] = m_vecDismissPRoom[pUserItem->GetChairID()];
		dismissScene.arrUserID[dismissScene.cbCount] = pUserItem->GetUserID();
		++dismissScene.cbCount;
	}

	SendUserMsg(pUser, &dismissScene, sizeof(dismissScene));
}

CT_VOID CTableFrame::SendTableSameIpTip()
{
	std::map<CT_DWORD, std::string> mapUserAllIp;
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
		{
			continue;
		}
		std::string strIp = pUserItem->GetUserIp();
		mapUserAllIp.insert(std::make_pair(pUserItem->GetUserID(), strIp));
	}

	//CT_BOOL bSameIp = false;
	std::map<std::string, CT_DWORD> mapUserNotSameIp;
	std::map<CT_DWORD, std::string> mapUserSameIp;
	for (auto& it : mapUserAllIp)
	{
		auto itIp = mapUserNotSameIp.find(it.second);
		if (itIp != mapUserNotSameIp.end())
		{
	
			auto itIp2 = mapUserAllIp.find(itIp->second);
			if (itIp2 != mapUserAllIp.end())
			{
				mapUserSameIp.insert(std::make_pair(itIp2->first, itIp2->second));
				mapUserSameIp.insert(std::make_pair(it.first, it.second));
			}
			//bSameIp = true;
		}
		else
		{
			mapUserNotSameIp.insert(std::make_pair(it.second, it.first));
		}
	}

	//if (bSameIp)
	{
		MSG_SC_SameIp_Tips sameIp;
		memset(&sameIp, 0, sizeof(sameIp));
		sameIp.dwMainID = MSG_FRAME_MAIN;
		sameIp.dwSubID = SUB_S2C_SAME_IP_TIPS;

		for (auto& it : mapUserSameIp)
		{
			sameIp.arrUserID[sameIp.cbSameCount++] = it.first;
		}
		SendAllUserData(&sameIp, sizeof(MSG_SC_SameIp_Tips));
	//	LOG(INFO) << "send same ip info.";
	}

}

CT_VOID CTableFrame::GameRoundStartDeploy()
{
	m_GameRoundPhase = en_GameRound_Start;
	m_GameRoundID = 0;
	m_dwPlayTime = Utility::GetTime();
	InsertGameRoundRecordEx();
	SendTableSameIpTip();
	CGameTableManager::get_instance().GoCenterUpdatePRoom(m_pRoomKindInfo->wGameID, m_pRoomKindInfo->wKindID, m_PrivateTableInfo.dwRoomNum, m_PrivateTableInfo.dwGroupID, m_PrivateTableInfo.cbGroupType, true, PROOM_START);
}

CT_VOID CTableFrame::UserVoiceChat(CServerUserItem* pUser, CT_CHAR* pChatUrl)
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

CT_VOID CTableFrame::UserTextChat(CServerUserItem* pUser, CT_BYTE cbType, CT_BYTE cbIndex)
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


CT_VOID CTableFrame::SetPrivateTableInfo(const PrivateTableInfo& privateInfo)
{
	//m_PrivateTableInfo.dwOwnerUserID = privateInfo.dwOwnerUserID;
	//m_PrivateTableInfo.dwRoomNum = privateInfo.dwRoomNum;
	//m_PrivateTableInfo.wCurrPlayCount = 0;
	//m_PrivateTableInfo.wTotalPlayCount = privateInfo.wTotalPlayCount;
	//m_PrivateTableInfo.wUserCount = privateInfo.wUserCount;
	//m_PrivateTableInfo.wEndLimit = privateInfo.wEndLimit;
	//m_PrivateTableInfo.wWinLimit = privateInfo.wWinLimit;
	//m_PrivateTableInfo.cbIsVip = privateInfo.cbIsVip;
	//m_PrivateTableInfo.cbAuto = privateInfo.cbAuto;
	//m_PrivateTableInfo.dwGroupID = privateInfo.dwGroupID;
	//m_PrivateTableInfo.dwRecordID = privateInfo.dwRecordID;
	//_snprintf_info(m_PrivateTableInfo.szOtherParam, sizeof(m_PrivateTableInfo.szOtherParam), "%s", privateInfo.szOtherParam);
	memcpy(&m_PrivateTableInfo, &privateInfo, sizeof(PrivateTableInfo));
}

CT_BYTE CTableFrame::AddApplyVipPRoom(CT_DWORD dwUserID, const GS_UserBaseData& userBaseData)
{
	auto it = m_mapApplyVipRoom.find(dwUserID);
	if (it != m_mapApplyVipRoom.end())
	{
		return en_Add_Repeat;
	}

	if (m_mapApplyVipRoom.size() >= en_PRoomApplyMax)
	{
		return en_Add_Max;
	}

	stApplyVipPRoomInfo applyInfo;
	applyInfo.cbState = en_PRoomApply_Undisposed;
	applyInfo.cbGender = userBaseData.cbGender;
	_snprintf_info(applyInfo.szNickname, sizeof(applyInfo.szNickname), "%s", userBaseData.szNickName);
	_snprintf_info(applyInfo.szApplyTime, sizeof(applyInfo.szApplyTime), "%s", Utility::GetTimeNowString("%m-%d %H:%M").c_str());
	//_snprintf_info(applyInfo.szHeadUrl, sizeof(applyInfo.szHeadUrl), "%s", userBaseData.szHeadUrl);
	m_mapApplyVipRoom.insert(std::make_pair(dwUserID, applyInfo));
	return en_Add_Succ;
}

CT_BYTE CTableFrame::GetVipPRoomApplyState(CT_DWORD dwUserID)
{
	auto it = m_mapApplyVipRoom.find(dwUserID);
	if (it != m_mapApplyVipRoom.end())
	{
		return it->second.cbState;
	}

	return en_PRoomApply_Unapply;
}


CT_VOID CTableFrame::SendVipPRoomApplyToOwner(CT_DWORD dwUserID)
{
	auto it = m_mapApplyVipRoom.find(dwUserID);
	if (it == m_mapApplyVipRoom.end())
	{
		return;
	}

	MSG_SC_Apply_VipPRoom_ToOwner applyPRoomInfo;
	applyPRoomInfo.dwMainID = MSG_FRAME_MAIN;
	applyPRoomInfo.dwSubID = SUB_S2C_APPLY_PROOM_TO_OWNER;
	applyPRoomInfo.dwUserID = dwUserID;
	applyPRoomInfo.cbGender = it->second.cbGender;
	applyPRoomInfo.cbState = it->second.cbState;
	_snprintf_info(applyPRoomInfo.szNickname, sizeof(applyPRoomInfo.szNickname), "%s", it->second.szNickname);
	_snprintf_info(applyPRoomInfo.szApplyTime, sizeof(applyPRoomInfo.szApplyTime), "%s", it->second.szApplyTime);
	_snprintf_info(applyPRoomInfo.szHeadUrl, sizeof(applyPRoomInfo.szHeadUrl), "%s", it->second.szHeadUrl);

	CT_WORD wChairID = GetPRoomOwnerChairID();
	if (wChairID != INVALID_CHAIR)
	{
		CServerUserItem* pUserItem = GetTableUserItem(wChairID);
		if (pUserItem)
		{
			SendUserMsg(pUserItem, &applyPRoomInfo, sizeof(applyPRoomInfo));
		}
	}
}

CT_VOID CTableFrame::SendAllVipPRoomApplyToOwner()
{
	MSG_SC_Apply_VipPRoom_All_ToOwner applyPRoom;
	memset(&applyPRoom, 0, sizeof(applyPRoom));
	applyPRoom.dwMainID = MSG_FRAME_MAIN;
	applyPRoom.dwSubID = SUB_S2C_APPLY_PROOM_TO_OWNER_ALL;

	for (auto& it : m_mapApplyVipRoom)
	{
		applyPRoom.arrUserID[applyPRoom.cbCount] = it.first;
		applyPRoom.arrState[applyPRoom.cbCount] = it.second.cbState;
		applyPRoom.cbGender[applyPRoom.cbCount] = it.second.cbGender;
		_snprintf_info(applyPRoom.szNickname[applyPRoom.cbCount], sizeof(applyPRoom.szNickname[applyPRoom.cbCount]), "%s", it.second.szNickname);
		_snprintf_info(applyPRoom.szHeadUrl[applyPRoom.cbCount], sizeof(applyPRoom.szHeadUrl[applyPRoom.cbCount]), "%s", it.second.szHeadUrl);
		_snprintf_info(applyPRoom.szApplyTime[applyPRoom.cbCount], sizeof(applyPRoom.szApplyTime[applyPRoom.cbCount]), "%s", it.second.szApplyTime);

		++applyPRoom.cbCount;
		if (applyPRoom.cbCount >= en_PRoomApplyMax)
		{
			break;
		}
	}

	if (applyPRoom.cbCount > 0)
	{
		CT_WORD wChairID = GetPRoomOwnerChairID();
		if (wChairID != INVALID_CHAIR)
		{
			CServerUserItem* pUserItem = GetTableUserItem(wChairID);
			if (pUserItem)
			{
				SendUserMsg(pUserItem, &applyPRoom, sizeof(applyPRoom));
			}
		}
	}
}

CT_VOID CTableFrame::RemoveVipPRoomApplyWhenLogout(CT_DWORD dwUserID)
{
	//退出的玩家当作不同意处理
	SendVipPRoomApplyOperateToOwner(dwUserID, en_PRoomApply_Disagree);
	auto it = m_mapApplyVipRoom.find(dwUserID);
	if (it != m_mapApplyVipRoom.end())
	{
		m_mapApplyVipRoom.erase(it);
	}
}

CT_VOID CTableFrame::OwnerOperateApply(CT_DWORD dwOwnerID, CT_DWORD dwUserID, CT_BYTE cbOperate)
{
	//不是房主, 直接返回
	if (m_PrivateTableInfo.dwOwnerUserID != dwOwnerID)
	{
		return;
	}

	if (cbOperate != en_PRoomApply_Agree && cbOperate != en_PRoomApply_Disagree)
	{
		return;
	}

	auto it = m_mapApplyVipRoom.find(dwUserID);
	if (it == m_mapApplyVipRoom.end())
	{
		return;
	}

	//如果玩家已经在其他房间
	if (CheckUserIsOnline(dwUserID)/* || CheckUserHasRoom(dwUserID)*/)
	{
		SendVipPRoomApplyOperateToOwner(dwUserID, en_PRoomApply_InOtherRoom);
		m_mapApplyVipRoom.erase(it);
		return;
	}

	if (cbOperate == en_PRoomApply_Agree)
	{
		it->second.cbState = en_PRoomApply_Agree;
		//设置玩家在线信息
		//SetUserPRoomNum(dwUserID);
		SetUserOnlineInfo(dwUserID, m_PrivateTableInfo.dwRoomNum);
		IncrPRoomUserCount();

		//向前端发送已同意请求
		stUserProxyInfo proxyInfo;
		CT_BOOL bSucc = GetUserProxyInfo(dwUserID, proxyInfo);
		if (bSucc)
		{
			acl::aio_socket_stream*  pProxySocket =CServerMgr::get_instance().FindProxyServer(proxyInfo.dwProxyServerID);
			if (pProxySocket)
			{
				MSG_SC_OwnerAgreeEnterPRoom agreeEnterPRoom;
				agreeEnterPRoom.dwMainID = MSG_FRAME_MAIN;
				agreeEnterPRoom.dwSubID = SUB_S2C_OWNER_AGREDD_ENTER_PROOM;
				agreeEnterPRoom.uValue1 = proxyInfo.uClinetNet;
				agreeEnterPRoom.wGameID = m_pRoomKindInfo->wGameID;
				agreeEnterPRoom.wKindID = m_pRoomKindInfo->wKindID;
				agreeEnterPRoom.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
				CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, &agreeEnterPRoom, sizeof(agreeEnterPRoom));
			}
		}
	}
	else
	{
		m_mapApplyVipRoom.erase(it);
	}

	SendVipPRoomApplyOperateToOwner(dwUserID, cbOperate);
}

CT_VOID CTableFrame::SendVipPRoomApplyOperateToOwner(CT_DWORD dwUserID, CT_BYTE cbOperate)
{
	MSG_SC_OperateApplyPRoom operateApply;
	operateApply.dwMainID = MSG_FRAME_MAIN;
	operateApply.dwSubID = SUB_S2C_OPERATE_APPLY_PROOM_RESULT;
	operateApply.dwUserID = dwUserID;
	operateApply.cbOperateResult = cbOperate;

	CT_WORD wChairID = GetPRoomOwnerChairID();
	if (wChairID != INVALID_CHAIR)
	{
		CServerUserItem* pUserItem = GetTableUserItem(wChairID);
		if (pUserItem)
		{
			SendUserMsg(pUserItem, &operateApply, sizeof(MSG_SC_OperateApplyPRoom));
		}
	}
}


CT_VOID CTableFrame::UpdatePRoomGameRoundReward()
{
	CT_WORD wTableMaxPlayerCount = m_pRoomKindInfo->wStartMaxPlayer;
	for (int i = 0; i < wTableMaxPlayerCount; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem == NULL)
		{
			continue;
		}

		CT_DWORD dwGameRoundCount = pUserItem->GetGameRoundCount();
		pUserItem->SetGameRoundCount(dwGameRoundCount + 1);

		CT_DWORD dwNewGameRoundCount = pUserItem->GetGameRoundCount();

		//更新玩家大局局数
		UpdateUserPRoomGameRoundCount(pUserItem->GetUserID(), dwNewGameRoundCount);
		SaveGameRoundCount(pUserItem->GetUserID(), dwNewGameRoundCount);

		//如果有奖励，则奖励
		CT_DWORD dwRewardGem = 0;
		CT_BOOL bReward = CGameTableManager::get_instance().CheckGameRoundReward(dwNewGameRoundCount, dwRewardGem);
		if (bReward)
		{
			//CT_DWORD dwGem = pUserItem->GetUserGem();
			AddUserGem(pUserItem, dwRewardGem);

			//下发奖励信息给客户端
			SendGameRoundReward(pUserItem, dwNewGameRoundCount, dwRewardGem);
		}
	}
}

CT_VOID CTableFrame::SendGameRoundReward(CServerUserItem* pUserItem, CT_DWORD dwGameRoundCount, CT_DWORD dwRewardGem)
{
	if (pUserItem == NULL)
	{
		return;
	}

	MSG_SC_GameRoundReward gameRoundReward;
	gameRoundReward.dwMainID = MSG_FRAME_MAIN;
	gameRoundReward.dwSubID = SUB_S2C_PROOM_GAMEROUND_REWARD;
	gameRoundReward.dwGameRoundCount = dwGameRoundCount;
	gameRoundReward.dwRewardGem = dwRewardGem;
	SendUserMsg(pUserItem, &gameRoundReward, sizeof(gameRoundReward));
}

CT_BOOL CTableFrame::PrivateRoomSitChair(CServerUserItem* pUserItem)
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
		m_UserList[i]->SetPRoomNum(m_PrivateTableInfo.dwRoomNum);
		//设置玩家为准备状态
		if (m_pRoomKindInfo->wGameID != GAME_NN)
		{
			pUserItem->SetUserStatus(sReady);
		}
		else
		{
			pUserItem->SetUserStatus(sSit);
		}

		OnUserEnterAction(pUserItem);

		//vip房间，房主直接设置在线信息
		/*if (IsPRoomOwner(pUserItem->GetUserID()) && IsPRoomVipMode())
		{
			SetUserOnlineInfo(pUserItem->GetUserID(), pUserItem->GetPRoomNum());
		}

		//普通房间，设置在线信息，非房主设置房间信息
		if (!IsPRoomVipMode())
		{
			SetUserOnlineInfo(pUserItem->GetUserID(), pUserItem->GetPRoomNum());
			//if (!IsPRoomOwner(pUserItem->GetUserID()))
			//{
			//	SetUserPRoomNum(pUserItem->GetUserID());
			//}
		}*/
		if (!IsPRoomOwner(pUserItem->GetUserID()))
		{
			SetUserOnlineInfo(pUserItem->GetUserID(), m_PrivateTableInfo.dwRoomNum);
		}
		IncrPRoomUserCount();

		//检查是否可以开始
		if (CheckGameStart() == true)
		{
			GameRoundStartDeploy();
			GameStart();
		}

		return true;
	}

	return false;
}

CT_BOOL CTableFrame::UserStandUp(CServerUserItem* pUserItem, CT_BOOL bSendState/* = true*/, CT_BOOL bSendStateMyself/* = false*/)
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
			pUserItem->SetTableID(ERROR_TABLE_ID);
			pUserItem->SetChairID(ERROR_CHAIR_ID);
			m_UserList[i] = NULL;
			return true;
		}
	}
	return false;
}

CT_VOID CTableFrame::OnUserEnterAction(CServerUserItem* pUserItem, CT_BOOL bDistribute/* = false*/, CT_BOOL bReConnect/* = false*/)
{
	//CT_DWORD dwUserID = pUserItem->GetUserID();
	CT_DWORD dwChairID = pUserItem->GetChairID();

	//发送玩家坐下完成的消息
	SendUserSitdownFinish(pUserItem);

	//广播自己本人信息
	BroadcastUserInfo(pUserItem);
	//将其它玩家的信息发送至自己
	SendOtherUserDataToUser(pUserItem);
	//广播用户状态
	BroadcastUserState(pUserItem);
	m_pTableFrameSink->OnUserEnter(dwChairID, pUserItem->IsLookOn());
	//恢复游戏场景
	m_pTableFrameSink->OnEventGameScene(dwChairID, pUserItem->IsLookOn());

	//发送用户申请解散的信息
	if (m_dwApplyDismissUserID!=0 /*&& m_GameRoundPhase == en_GameRound_Free*/)
	{
		SendDismissPRoomInfo(pUserItem);
	}

	//发送VIP申请列表
	if (m_GameRoundPhase == en_GameRound_Free && IsPRoomVipMode() && IsPRoomOwner(pUserItem->GetUserID()))
	{
		SendAllVipPRoomApplyToOwner();
	}
}

CT_VOID CTableFrame::SendForceLeaveTableMsg(CServerUserItem* pUserItem)
{
	MSG_SC_DismissPRoomResult dismissPRoom;
	dismissPRoom.dwMainID = MSG_FRAME_MAIN;
	dismissPRoom.dwSubID = SUB_S2C_DISMISS_PROOM_RESULT;
	dismissPRoom.cbOperateResult = en_Force_LeavePRoom_ByAdmin;
	dismissPRoom.cbOperateCount = 1;
	dismissPRoom.arrUserID[0] = m_PrivateTableInfo.dwOwnerUserID;
	SendUserMsg(pUserItem, &dismissPRoom, sizeof(dismissPRoom));
}

CT_VOID CTableFrame::SetAllUserOnlineInfo()
{
	CT_WORD wMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem != NULL)
		{
			SetUserOnlineInfo(pUserItem->GetUserID(), pUserItem->GetPRoomNum());
		}
	}
}

CT_VOID CTableFrame::SetUserOnlineInfo(CT_DWORD dwUserID, CT_DWORD dwRoomNum)
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
	val_roomnum.format("%u", dwRoomNum);

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

CT_VOID CTableFrame::SetOtherUserPRoomNum()
{
	CT_WORD wMaxPlayer =  m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_WORD i = 0; i < wMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (pUserItem != NULL && pUserItem->GetUserID() != m_PrivateTableInfo.dwOwnerUserID)
		{
			SetUserPRoomNum(pUserItem->GetUserID());
		}
	}
}

CT_VOID CTableFrame::SetUserPRoomNum(CT_DWORD dwUserID)
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key, val;
	key.format("proom_%u", dwUserID);
	val.format("%u", m_PrivateTableInfo.dwRoomNum);

	m_pRedis->clear();
	if (m_pRedis->set(key, val) == false)
	{
		LOG(WARNING) << "set user proom num fail, userid:" << dwUserID;
		return;
	}
	return;
}

CT_VOID CTableFrame::DelUserPRoomNum(CT_DWORD dwUserID)
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key;
	key.format("proom_%u", dwUserID);

	m_pRedis->clear();
	if (m_pRedis->del_one(key) <= 0)
	{
		LOG(WARNING) << "del user proom num fail, userid:" << dwUserID;
		return;
	}
	return;
}

CT_VOID CTableFrame::DelUserOnlineInfo(CT_DWORD dwUserID)
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

CT_VOID CTableFrame::IncrPRoomUserCount()
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key;
	key.format("proom_info_%u", m_PrivateTableInfo.dwRoomNum);

	m_pRedis->clear();
	if (m_pRedis->hincrby(key, "curruser", 1) == false)
	{
		LOG(WARNING) << "increase proom user count fail, room num:" << m_PrivateTableInfo.dwRoomNum;
		return;
	}
	return;
}

CT_VOID CTableFrame::DecrPRoomUserCount()
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key;
	key.format("proom_info_%u", m_PrivateTableInfo.dwRoomNum);

	m_pRedis->clear();
	if (m_pRedis->hincrby(key, "curruser", -1) == false)
	{
		LOG(WARNING) << "decrease proom user count fail, room num:" << m_PrivateTableInfo.dwRoomNum;
		return;
	}
	return;
}

CT_VOID CTableFrame::DelPRoomInfo()
{
	if (m_pRedis == NULL)
	{
		return;
	}

	acl::string key;
	key.format("proom_info_%u", m_PrivateTableInfo.dwRoomNum);

	m_pRedis->clear();
	if (m_pRedis->del_one(key) <= 0)
	{
		LOG(WARNING) << "delete proom info fail, proom num:" << m_PrivateTableInfo.dwRoomNum;
		return;
	}
	return;
}

CT_VOID CTableFrame::DeductUserGem(CServerUserItem* pUserItem)
{
	if (pUserItem == NULL || m_pRedis == NULL)
	{
		return;
	}

	CT_DWORD dwGem = pUserItem->GetUserGem();	
	CT_WORD wNeedGem = CGameTableManager::get_instance().GetPRoomNeedGem(m_PrivateTableInfo.wTotalPlayCount);
	if (wNeedGem != 0)
	{
		CT_INT32 dwNewGem = dwGem - wNeedGem;
		if (dwNewGem < 0)
		{
			LOG(WARNING) << "user gem not enough to deduct? fuck";
		}

		CT_INT32 iReallyGem = ((dwNewGem >= 0) ? wNeedGem : dwGem);

		dwNewGem = (dwNewGem > 0) ? dwNewGem : 0;

		
		AddUserGem(pUserItem, -iReallyGem);
		InsertGemConsumeRecord(wNeedGem, dwNewGem);
	}
}

CT_VOID CTableFrame::AddUserGem(CServerUserItem* pUserItem, CT_INT32 nAddGem)
{
	if (pUserItem == NULL)
	{
		return;
	}

	acl::string key;
	key.format("account_%u", pUserItem->GetUserID());
	//acl::string val;
	//val.format("%d", nAddGem);

	long long llNewGem = 0;
	m_pRedis->clear();
	if (m_pRedis->hincrby(key, "gem", nAddGem, &llNewGem) == false)
	{
		LOG(WARNING) << "update redis gem fail, user id: " << pUserItem->GetUserID();
		return;
	}

	pUserItem->SetUserGem((CT_DWORD)llNewGem);
	UpdateUserGemToDB(pUserItem->GetUserID(), nAddGem);
	UpdateUserGemToCenterServer(pUserItem->GetUserID(), nAddGem);
	UpdateUserGemToClient(pUserItem, (CT_DWORD)llNewGem);
}

CT_VOID CTableFrame::UpdateUserGemToClient(CServerUserItem* pUserItem, CT_DWORD dwGem)
{
	if (pUserItem == NULL)
	{
		return;
	}

	MSG_SC_GameUserGem updateGem;
	updateGem.dwMainID = MSG_FRAME_MAIN;
	updateGem.dwSubID = SUB_S2C_USER_GEM;
	updateGem.dwUserID = pUserItem->GetUserID();
	updateGem.dwGem = dwGem;
	SendUserMsg(pUserItem, &updateGem, sizeof(MSG_SC_GameUserGem));
}

CT_BOOL CTableFrame::CheckUserIsOnline(CT_DWORD dwUserID)
{
	if (m_pRedis == NULL)
	{
		return false;
	}

	acl::string key;
	key.format("online_%u", dwUserID);

	m_pRedis->clear();
	if (m_pRedis->exists(key) == true)
	{
		return true;
	}

	return false;
}

CT_BOOL CTableFrame::CheckUserHasRoom(CT_DWORD dwUserID)
{
	if (m_pRedis == NULL)
	{
		return false;
	}

	acl::string key;
	key.format("proom_%u", dwUserID);

	m_pRedis->clear();
	if (m_pRedis->exists(key) == true)
	{
		return true;
	}

	return false;
}

CT_BOOL CTableFrame::GetUserProxyInfo(CT_DWORD dwUserID, stUserProxyInfo& proxyInfo)
{
	if (m_pRedis == NULL)
	{
		return false;
	}

	acl::string key;
	acl::string psid;
	acl::string clientNet;
	std::map<acl::string, acl::string> psInfo;

	key.format("psinfo_%u", dwUserID);
	psid.format("psid");
	clientNet.format("netaddr");

	m_pRedis->clear();
	
	if (m_pRedis->hgetall(key, psInfo) == true)
	{
		if (!psInfo.empty())
		{
			proxyInfo.dwProxyServerID = atoi(psInfo[psid]);
			proxyInfo.uClinetNet = Utility::stingToUint64(psInfo["netaddr"].c_str());
			return true;
		}
	}

	return false;
}

CT_VOID CTableFrame::UpdateUserPRoomGameRoundCount(CT_DWORD dwUserID, CT_DWORD dwGameRoundCount)
{
	acl::string key;
	key.format("gameround_%u", dwUserID);

	acl::string att;
	att.format("%u:%u", m_pRoomKindInfo->wGameID, m_pRoomKindInfo->wKindID);

	acl::string val;
	val.format("%u", dwGameRoundCount);

	m_pRedis->clear();
	if (m_pRedis->hset(key, att.c_str(), val.c_str()) == -1)
	{
		LOG(WARNING) << "update redis game round count fail, user id: " << dwUserID;
		return;
	}
}

CT_VOID CTableFrame::WriteVideoCommonPackage(MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen)
{
	CT_CHAR cbMsg[1024] = { 0 };
	CT_CHAR *pMsg = (CT_CHAR*)pDownData;
	CT_WORD wDownHeadSize = sizeof(MSG_GameMsgDownHead);
	CT_WORD wRealSize = dwLen - wDownHeadSize;
	memcpy(cbMsg, pMsg + wDownHeadSize, wRealSize);

	CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
	CT_WORD wHeadSize = sizeof(CMD_Command);
	memcpy(chBuf + wHeadSize, cbMsg, wRealSize);

	CMD_Command*	 pMesHead = (CMD_Command*)chBuf;
	memset(pMesHead, 0, wHeadSize);
	pMesHead->dwDataSize = wRealSize;
	pMesHead->dwMainID = pDownData->dwMainID;
	pMesHead->dwSubID = pDownData->dwSubID;

	m_GameMsg.WriteBytes((CT_CHAR*)chBuf, wRealSize + wHeadSize);
	return;
}

CT_VOID CTableFrame::WriteEnterSuccMsg()
{
	MSG_SC_EnterPRoomSucc enterPRoomSucc;
	enterPRoomSucc.dwMainID = MSG_FRAME_MAIN;
	enterPRoomSucc.dwSubID = SUB_S2C_ENTER_PROOM_SUCC;
	enterPRoomSucc.uValue1 = 0;
	enterPRoomSucc.wGameID = m_pRoomKindInfo->wGameID;
	enterPRoomSucc.wKindID = m_pRoomKindInfo->wKindID;
	enterPRoomSucc.wRoomKindID = m_pRoomKindInfo->wRoomKindID;
	enterPRoomSucc.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
	enterPRoomSucc.dwRoomOwnerID = m_PrivateTableInfo.dwOwnerUserID;
	enterPRoomSucc.dwCellScore = m_pRoomKindInfo->dwCellScore;
	enterPRoomSucc.cbIsVip = m_PrivateTableInfo.cbIsVip;
	enterPRoomSucc.wUserCount = m_PrivateTableInfo.wUserCount;
	enterPRoomSucc.wPlayCount = m_PrivateTableInfo.wTotalPlayCount;
	_snprintf_info(enterPRoomSucc.szOtherParam, sizeof(enterPRoomSucc.szOtherParam), "%s", m_PrivateTableInfo.szOtherParam);

	WriteVideoCommonPackage(&enterPRoomSucc, sizeof(enterPRoomSucc));
}

CT_VOID CTableFrame::WriteUserEnterMsg()
{
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		if (NULL != m_UserList[i])
		{
			MSG_SC_UserBaseInfo info;
			memset(&info, 0, sizeof(MSG_SC_UserBaseInfo));
			info.dwMainID = MSG_FRAME_MAIN;
			info.dwSubID = SUB_S2C_USER_ENTER;
			m_UserList[i]->GetUserBaseMsgData(info);

			WriteVideoCommonPackage(&info, sizeof(info));
		}
	}
}

CT_VOID CTableFrame::WriteFreeGameScene()
{
	MSG_GS_Free_GameScene freeGameScene;
	freeGameScene.dwCellScore = 1;
	freeGameScene.wCurrPlayCount = 1;

	CT_CHAR chBuf[GAME_DATA_LEN] = { 0 };
	CT_WORD wHeadSize = sizeof(CMD_Command);
	CT_WORD wLen = sizeof(freeGameScene);
	memcpy(chBuf + wHeadSize, &freeGameScene, wLen);

	CMD_Command* pMesHead = (CMD_Command*)chBuf;
	memset(pMesHead, 0, wHeadSize);
	pMesHead->dwDataSize = sizeof(freeGameScene);
	pMesHead->dwMainID = MSG_GAME_MAIN;
	pMesHead->dwSubID = 2001;

	m_GameMsg.WriteBytes((CT_CHAR*)chBuf, wHeadSize + wLen);
}

//发送DB信息
CT_VOID CTableFrame::SendDBMsg(CT_WORD mainId, CT_WORD subId, CT_VOID* pMsg, CT_WORD wLen)
{
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), mainId, subId, pMsg, wLen);
}

CT_VOID CTableFrame::UpdateUserGemToDB(CT_DWORD dwUserID, CT_INT32 nAddGem)
{
	MSG_UpdateUser_Gem updateGemToDB;
	updateGemToDB.dwUserID = dwUserID;
	updateGemToDB.nAddGem = nAddGem;
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_UPDATE_USER_GEM, &updateGemToDB, sizeof(updateGemToDB));
}

CT_VOID CTableFrame::UpdateUserGemToCenterServer(CT_DWORD dwUserID, CT_INT32 nAddGem)
{
	MSG_UpdateUser_Gem updateGemToDB;
	updateGemToDB.dwUserID = dwUserID;
	updateGemToDB.nAddGem = nAddGem;
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_UPDATE_USER_GEM, &updateGemToDB, sizeof(updateGemToDB));
}

//玩家钻石消耗记录
CT_VOID CTableFrame::InsertGemConsumeRecord(CT_DWORD dwConsumeGem, CT_DWORD dwNewGem)
{
	MSG_G2DB_Gem_Consume consumeGem;
	consumeGem.wGameID = m_pRoomKindInfo->wGameID;
	consumeGem.wKindID = m_pRoomKindInfo->wKindID;
	consumeGem.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
	consumeGem.dwUserID = m_PrivateTableInfo.dwOwnerUserID;
	consumeGem.dwConsumeGem = dwConsumeGem;
	consumeGem.dwRemainGem = dwNewGem;
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_GEM_CONSUME, &consumeGem, sizeof(consumeGem));
}

CT_VOID CTableFrame::SaveVideoToDB()
{
	/*std::ofstream video("video.txt", std::ios::out | std::ios::trunc | std::ios::binary);
	if (video.is_open())
	{
		video.write(pBuf, dwLen);

		video.close();
	}*/

	const CT_CHAR* pBuf = m_GameMsg.GetReadBuffer();
	CT_INT32 dwLen = (CT_INT32)m_GameMsg.GetReadSize();
	if (dwLen <= 0)
	{
		return;
	}

	CT_INT32 iMaxLen = dwLen;
	if (dwLen >= SYS_NET_SENDBUF_SIZE /*|| m_bDismissPRoom*/)
	{
		iMaxLen = SYS_NET_SENDBUF_SIZE;
		//提交读大小
		m_GameMsg.CommitReadBytes(dwLen);
		//重置缓冲
		m_GameMsg.Compact();
		return;
	}

	MSG_G2DB_PRoom_PlaybackRecord playback;
	playback.uGameRoundID = m_GameRoundID;
	playback.wPlayCount = m_PrivateTableInfo.wCurrPlayCount;
	playback.uBinaryLen = iMaxLen;
	_snprintf_info(playback.szEndTime, sizeof(playback.szEndTime), "%s", m_strPlayCountTime.c_str());
	memcpy(playback.szBinary, pBuf, iMaxLen);
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_PROOM_PLAYBACK_RECORD, &playback, sizeof(playback));

	//提交读大小
	m_GameMsg.CommitReadBytes(dwLen);
	//重置缓冲
	m_GameMsg.Compact();
}

CT_VOID CTableFrame::InsertGameRoundRecord()
{
	acl::db_pool* pool = CGameTableManager::get_instance().GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}
	InsertGameRoundRecord(db);
	pool->put(db);
}

CT_VOID CTableFrame::InsertGameRoundRecord(acl::db_handle* db)
{
	//if (db->begin_transaction() == false)
	//{
	//	LOG(WARNING) << "begin transaction false: " << db->get_error();
	//	return;
	//}

	acl::query query;
	query.create_sql("insert into record_big_score(gameid, kindid,roomNum, startTime,endTime) values (:gameidval, :kindidval, :roomNumval, :startTimeval, :endTimeval)")
		.set_format("gameidval", "%d", m_pRoomKindInfo->wGameID)
		.set_format("kindidval", "%d", m_pRoomKindInfo->wKindID)
		.set_format("roomNumval", "%u", m_PrivateTableInfo.dwRoomNum)
		.set_date("startTimeval", time(NULL))
		.set_date("endTimeval", time(NULL));


	if (db->exec_update(query) == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "insert game round record fail, error = " << nError;
		return;
	}

	db->free_result();
	if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "select game round id record fail, error = " << nError;
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_GameRoundID = atoi((*row)["LAST_INSERT_ID()"]);
		LOG(INFO) << "insert game round id: " << m_GameRoundID;
	}

	if (db->length() == 0)
	{
		LOG(ERROR) << "get game round id fail! ";
	}

	/*const acl::db_rows* result = db->get_result();
	if (result)
	{
		const std::vector<acl::db_row*>& rows = result->get_rows();
		for (size_t i = 0; i < rows.size(); i++)
		{
			const acl::db_row* row = rows[i];
			const char* pFieldName = row->field_name(0);
			m_GameRoundID = row->field_int64(pFieldName);
			break;
		}
	}*/
	//if (db->commit() == false)
	//{
	//	LOG(WARNING) << "commit error :" << db->get_errno();
	//	return;
	//}

	db->free_result();
}

CT_VOID CTableFrame::InsertGameRoundRecordEx()
{
	MSG_G2DB_PRoom_InsertGameRound insertGameRound;
	insertGameRound.wGameID = m_pRoomKindInfo->wGameID;
	insertGameRound.wKindID = m_pRoomKindInfo->wKindID;
	insertGameRound.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
	insertGameRound.dwUserID = m_PrivateTableInfo.dwOwnerUserID;
	insertGameRound.dwTableID = m_TableState.dwTableID;
	_snprintf_info(insertGameRound.szParam, sizeof(insertGameRound.szParam), "%s", m_PrivateTableInfo.szOtherParam);
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_PROOM_INSERT_GAMEROUND, &insertGameRound, sizeof(insertGameRound));
}

CT_VOID CTableFrame::SavePlayCountRecord(CT_DWORD dwUserID, CT_CHAR* pNickName, CT_WORD wChairID, CT_LONGLONG llScore)
{
	m_strPlayCountTime = Utility::GetTimeNowString();

	MSG_G2DB_PRoom_PlayCountRecord playCountRecord;
	playCountRecord.uGameRoundID = m_GameRoundID;
	playCountRecord.wCurrPlayCount = m_PrivateTableInfo.wCurrPlayCount;
	playCountRecord.dwUserID = dwUserID;
	playCountRecord.wChairID = wChairID;
	playCountRecord.llScore = llScore;
	_snprintf_info(playCountRecord.szNickName, sizeof(playCountRecord.szNickName), "%s", Utility::Utf82Ansi(pNickName).c_str());
	_snprintf_info(playCountRecord.szEndTime, sizeof(playCountRecord.szEndTime), "%s", m_strPlayCountTime.c_str());
	SendDBMsg(MSG_GDB_MAIN, SUB_G2DB_PROOM_PALYCOUNT_RECORD, &playCountRecord, sizeof(playCountRecord));
}

//保存私人场的结算信息
CT_VOID CTableFrame::SaveGameRoundRecord(const CT_CHAR* pEndTime)
{
	MSG_G2DB_PRoom_GameRoundRecord bigRecord;
	bigRecord.uCount = 0;
	bigRecord.uGameRoundID = m_GameRoundID;
	if (pEndTime)
	{
		_snprintf_info(bigRecord.szEndTime, sizeof(bigRecord.szEndTime), "%s", pEndTime);
	}
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (NULL == pUserItem)
		{
			continue;
		}

		bigRecord.dwArrUserID[bigRecord.uCount] = pUserItem->GetUserID();
		CT_WORD wChairID = pUserItem->GetChairID();
		bigRecord.llArrScore[bigRecord.uCount] = m_pTableFrameSink->GetGameRoundTotalScore(wChairID);
		const CT_CHAR* pNickName = GetNickName(wChairID);
		_snprintf_info(bigRecord.szNickname[i], sizeof(bigRecord.szNickname[i]), "%s", Utility::Utf82Ansi(pNickName).c_str());
		++bigRecord.uCount;
	}

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_PROOM_GAMEROUND_RECORD, &bigRecord, sizeof(bigRecord));
}

//保存输赢记录
CT_VOID CTableFrame::SaveWinLostCount(const CT_CHAR* pEndTime)
{
	MSG_G2DB_WinLostRecord winlostRecord;
	memset(&winlostRecord, 0, sizeof(winlostRecord));
	winlostRecord.wGameID = m_pRoomKindInfo->wGameID;
	if (pEndTime)
	{
		_snprintf_info(winlostRecord.szEndTime, sizeof(winlostRecord.szEndTime), "%s", pEndTime);
	}
	winlostRecord.dwPlayTime = Utility::GetTime() - m_dwPlayTime;
	
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (NULL == pUserItem)
		{
			continue;
		}

		CT_WORD wChairID = pUserItem->GetChairID();
		CT_INT32 nScore = m_pTableFrameSink->GetGameRoundTotalScore(wChairID);
		winlostRecord.dwArrUserID[winlostRecord.uCount] = pUserItem->GetUserID();
		if (nScore > 0)
		{
			winlostRecord.cbWinCount[winlostRecord.uCount] = 1;
		}
		else if (nScore < 0)
		{
			winlostRecord.cbLostCount[winlostRecord.uCount] = 1;
		}
		else
		{
			winlostRecord.cbDrawCount[winlostRecord.uCount] = 1;
		}
		++winlostRecord.uCount;
	}

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_PROOM_GAME_WINLOST, &winlostRecord, sizeof(winlostRecord));
}

CT_VOID CTableFrame::SaveGameRoundCount(CT_DWORD dwUserID, CT_DWORD dwGameRoundCount)
{
	MSG_G2DB_PRoom_GameRoundCount gameRoundCount;
	gameRoundCount.dwUserID = dwUserID;
	gameRoundCount.wGameID = m_pRoomKindInfo->wGameID;
	gameRoundCount.wKindID = m_pRoomKindInfo->wKindID;
	gameRoundCount.dwGameRoundCount = dwGameRoundCount;

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_PROOM_GAMEROUND_COUNT, &gameRoundCount, sizeof(gameRoundCount));
}

//记录自动开房信息
CT_VOID CTableFrame::RecordGroupRoomInfo()
{
	if (m_PrivateTableInfo.cbGroupType == GX_GROUP_ROOM)
	{
		MSG_G2DB_Record_GroupPRoom autoRoomInfo;
		autoRoomInfo.dwGroupID = m_PrivateTableInfo.dwGroupID;
		autoRoomInfo.dwRecordID = m_PrivateTableInfo.dwRecordID;
		autoRoomInfo.uGameRoundID = m_GameRoundID;

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_GROUP_PROOM_RECORD, &autoRoomInfo, sizeof(autoRoomInfo));
	}
	else
	{
		MSG_G2DB_Record_WXGroupPRoom autoRoomInfo;
		autoRoomInfo.dwGroupID = m_PrivateTableInfo.dwGroupID;
		autoRoomInfo.dwRecordID = m_PrivateTableInfo.dwRecordID;
		autoRoomInfo.uGameRoundID = m_GameRoundID;
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_WXGROUP_PROOM_RECORD, &autoRoomInfo, sizeof(autoRoomInfo));
	}
}


CT_VOID CTableFrame::InsertGameRoundResult(CT_UINT64 uGameRoundID)
{
	m_GameRoundID = uGameRoundID;
}

CT_VOID	CTableFrame::SendGroupPRoomClearing()
{
	MSG_G2CS_GroupPRoom_Clearing clearingInfo;
	memset(&clearingInfo, 0, sizeof(clearingInfo));

	clearingInfo.cbCount = 0;
	clearingInfo.dwGroupID = m_PrivateTableInfo.dwGroupID;
	clearingInfo.dwRoomNum = m_PrivateTableInfo.dwRoomNum;
	clearingInfo.wGameID = m_pRoomKindInfo->wGameID;
	clearingInfo.wKindID = m_pRoomKindInfo->wKindID;
	CT_DWORD dwMaxPlayer = m_pRoomKindInfo->wStartMaxPlayer;
	for (CT_DWORD i = 0; i < dwMaxPlayer; ++i)
	{
		CServerUserItem* pUserItem = m_UserList[i];
		if (NULL == pUserItem)
		{
			continue;
		}

		CT_WORD wChairID = pUserItem->GetChairID();
		clearingInfo.iScore[clearingInfo.cbCount] = m_pTableFrameSink->GetGameRoundTotalScore(wChairID);
		const CT_CHAR* pNickName = GetNickName(wChairID);
		_snprintf_info(clearingInfo.szNickName[i], sizeof(clearingInfo.szNickName[i]), "%s", pNickName);
		++clearingInfo.cbCount;
	}

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_GROUPPROOM_CLEARING, &clearingInfo, sizeof(clearingInfo));
}