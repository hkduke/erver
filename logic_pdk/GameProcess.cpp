
#include "stdafx.h"
#include <stdio.h>
#include "GameProcess.h"
#include "ITableFrame.h"
#include <time.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"

//打印日志
#define		PRINT_LOG_INFO						0
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//定时器
#define ID_TIME_SEND_CARD				100					//发牌定时器
#define ID_TIME_OUT_CARD				101					//出牌定时器
#define ID_TIME_GAME_END				102					//结束定时器
#define ID_TIME_NEXT_OUT_CARD			103					//下一个玩家出牌
#define ID_TIME_NEW_TURN				104					//新一轮定时器

//定时器时间
#define TIME_SEND_CARD					2500				//发牌时间

////////////////////////////////////////////////////////////////////////


CGameProcess::CGameProcess(void)
	: m_dwGameType(0)
	, m_dwServiceCost(0)
	, m_dwlCellScore(1)
	, m_pGameDeskPtr(NULL)
{
	//清理游戏数据
	ClearGameData();
	//初始化数据
	InitGameData();
}

CGameProcess::~CGameProcess(void)
{
}

//游戏开始
void CGameProcess::OnEventGameStart()
{
	assert(0 != m_pGameDeskPtr);
	//初始化数据
	InitGameData();
	//解释私人场数据
	ParsePrivateData(m_PrivateTableInfo.szOtherParam);
	//更新游戏配置
	UpdateGameConfig();
	//清除所有定时器
	ClearAllTimer();
	//设置游戏状态
	m_cbGameStatus = GAME_STATUS_PLAY;
	XiPai();

	//当前玩家
	m_dwCurrentUser = m_dwFirstUser;

	//发送开始消息
	for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		PDK_16_CMD_S_SendCard SendCard;
		memset(&SendCard, 0, sizeof(PDK_16_CMD_S_SendCard));
		SendCard.dwFirstUser = m_dwCurrentUser;
		SendCard.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
		SendCard.cbMustThree = IsMustOutThree(i);
		SendCard.cbCardCount = m_cbCardCount[i];
		SendCard.cbOutCardTime = m_cbOutCardTime;
		memcpy(SendCard.cbCardData, &m_cbCardData[i], MAX_COUNT);

		//发送数据
		SendTableData(i, PDK_16_SUB_S_SEND_CARD, &SendCard, sizeof(PDK_16_CMD_S_SendCard));
	}
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//清除所有定时器
	ClearAllTimer();
	//设置游戏状态-空闲状态
	m_cbGameStatus = GAME_STATUS_FREE;

	switch (GETag)
	{
	case GER_NORMAL:
	{//正常退出
		NormalGameEnd(dwChairID);
		break;
	}
	case GER_USER_LEFT:
	{//玩家强制退出,则把该玩家托管不出。
		break;
	}
	case GER_DISMISS:
	{//游戏解散
		break;
	}
	default:
		break;
	}

	//当前时间
	std::string strTime = Utility::GetTimeNowString();

	//游戏结束
	m_pGameDeskPtr->ConcludeGame(GAME_STATUS_FREE, strTime.c_str());

	//大结算
	if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_End)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnEventGameEnd: SendTotalClearing";

		//发送总结算
		SendTotalClearing(strTime);

		//清理桌子上的用户
		m_pGameDeskPtr->ClearTableUser();
	}
}

//发送场景
void CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID >= GAME_PLAYER)
	{
		return;
	}

	switch (m_cbGameStatus)
	{
	case GAME_STATUS_FREE:	//空闲状态
	{
		FPF_MSG_GS_FREE GameFree;
		memset(&GameFree, 0, sizeof(FPF_MSG_GS_FREE));
		GameFree.dwCellScore = m_dwlCellScore;
		GameFree.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_FREE, &GameFree, sizeof(FPF_MSG_GS_FREE), false);
		break;
	}
	case GAME_STATUS_PLAY:	//游戏中状态
	{
		//场景数据
		FPF_MSG_GS_PLAY GamePlay;
		memset(&GamePlay, 0, sizeof(FPF_MSG_GS_PLAY));
		//基础积分
		GamePlay.dwCellScore = m_dwlCellScore;
		GamePlay.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
		//剩余时间
		CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
		GamePlay.cbTimeLeave = (CT_BYTE)(m_cbOutCardTime - std::min<CT_BYTE>(dwPassTime, m_cbOutCardTime));

		//当前用户(出牌玩家)
		GamePlay.dwCurrentUser = m_dwCurrentUser;

		if (m_dwTurnWiner != m_dwCurrentUser)
		{
			tagSearchCardResult SearchCardResult;
			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
			if (cbResultCount == 0)
			{//不能出
				GamePlay.cbIsPass = 1;
			}
		}

		GamePlay.cbMustThree = IsMustOutThree(dwChairID);
		//手牌数据
		memcpy(GamePlay.cbCardCount, m_cbCardCount, sizeof(m_cbCardCount));
		memcpy(GamePlay.cbCardData, &m_cbCardData[dwChairID], sizeof(GamePlay.cbCardData));
		//胜利玩家
		GamePlay.dwTurnWiner = m_dwTurnWiner;
		//出牌数据
		GamePlay.cbTurnCardCount = m_cbTurnCardCount;
		memcpy(GamePlay.cbTurnCardData, m_cbTurnCardData, sizeof(GamePlay.cbTurnCardData));
		//是否报单
		memcpy(GamePlay.cbSingle, m_cbUserSingle, sizeof(GamePlay.cbSingle));
		//炸弹个数
		memcpy(GamePlay.cbRombCount, m_cbRombCount, sizeof(GamePlay.cbRombCount));

		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_PLAY, &GamePlay, sizeof(FPF_MSG_GS_PLAY), false);

		if (m_dwCurrentUser == dwChairID)
		{//通知出牌
			NotifyOutCard(dwChairID, GamePlay.cbTimeLeave, 1, (m_cbTurnCardCount == 0) ? true : false);
		}
		break;
	}
	case GAME_STATUS_END:
	{
		//场景数据
		FPF_MSG_GS_END EndStatus;
		memset(&EndStatus, 0, sizeof(FPF_MSG_GS_END));
		//基础积分
		EndStatus.dwCellScore = m_dwlCellScore;
		EndStatus.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
		//剩余时间
		CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
		EndStatus.cbTimeLeave = (CT_BYTE)(m_cbOutCardTime - std::min<CT_BYTE>(dwPassTime, m_cbOutCardTime));
		//剩余牌
		memcpy(&EndStatus.cbRemainCard, m_cbCardData, sizeof(m_cbCardData));
		memcpy(&EndStatus.cbRemainCount, m_cbCardCount, sizeof(m_cbCardCount));

		//发送结算状态数据
		SendTableData(dwChairID, SC_GAMESCENE_END, &EndStatus, sizeof(FPF_MSG_GS_END), false);

		//游戏结算消息
		if (EndStatus.cbTimeLeave >= 5)
		{
			SendTableData(dwChairID, PDK_16_SUB_S_GAME_END, &m_GameEnd, sizeof(PDK_16_CMD_S_GameEnd));
		}

		break;
	}
	default:
		break;
	}
	return;
}

//定时器事件
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch (dwTimerID)
	{
	case ID_TIME_SEND_CARD:			//发牌动画时间
	{
		//销毁定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_SEND_CARD);
		//通知玩家出牌
		NotifyOutCard(m_dwCurrentUser, m_cbOutCardTime, 1, 0);
		return;
	}
	case ID_TIME_OUT_CARD:			//等待玩家出牌时间 
	{
		//游戏状态判断
		if (m_cbGameStatus != GAME_STATUS_PLAY) return;
		//销毁定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_OUT_CARD);

		CT_BYTE cbCardCount = 0;														//出牌数目
		CT_BYTE cbCardData[MAX_COUNT] = { 0 };								//扑克列表
		memset(cbCardData, 0, sizeof(cbCardData));

		CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % m_PrivateTableInfo.wUserCount;
		if (1 == m_cbTurnCardCount && m_cbUserSingle[dwNextUser])
		{
			//找最大单张
			CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
			if (cbMaxSingle>0)
			{
				cbCardCount = 1;
				cbCardData[0] = cbMaxSingle;
			}
		}
		else
		{
			tagSearchCardResult SearchCardResult;
			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
			if (cbResultCount>0)
			{
				cbCardCount = SearchCardResult.cbCardCount[0];
				memcpy(cbCardData, &SearchCardResult.cbResultCard[0], sizeof(CT_BYTE)*cbCardCount);
			}
		}

		//找到合适的牌
		if (cbCardCount>0)
		{
			OnUserOutCard(m_dwCurrentUser, cbCardData, cbCardCount);
		}
		else
		{//提取异常
			OnEventGameEnd(INVALID_CHAIR, GER_DISMISS);
		}

		return;
	}
	case ID_TIME_NEXT_OUT_CARD:		//下一个玩家出牌
	{
		//销毁定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_NEXT_OUT_CARD);
		//通知下一个玩家出牌
		CT_INT32 iRtn = NotifyNextOutCard(m_dwResumeUser);
		LOG_IF(INFO, PRINT_LOG_INFO) << "ID_TIME_NEXT_OUT_CARD: NotifyNextOutCard iRtn=" << iRtn;
		return;
	}
	case ID_TIME_GAME_END:			//游戏结束(显示结算框时间)
	{
		//游戏状态判断
		if (m_cbGameStatus != GAME_STATUS_END) return;
		//销毁定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_GAME_END);
		//调用框架游戏结束
		m_pGameDeskPtr->ConcludeGame(GAME_STATUS_FREE, NULL);
		return;
	}
	}
}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	CT_INT32 iRtn = false;
	switch (dwSubCmdID)
	{
	case PDK_16_SUB_C_OUT_CARD:					//出牌消息
	{
		//效验消息
		if (sizeof(PDK_16_CMD_C_OutCard) != dwDataSize)
		{
			return false;
		}
		//消息处理
		PDK_16_CMD_C_OutCard* pOutCard = (PDK_16_CMD_C_OutCard*)pDataBuffer;
		if (NULL == pOutCard)
		{
			return false;
		}
		iRtn = OnUserOutCard(wChairID, pOutCard->cbCardData, pOutCard->cbCardCount);
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard iRtn=" << iRtn;
		return iRtn > 0;
	}
	case PKD_16_SUB_C_PASS_CARD:
	{//玩家不出
		if (wChairID >= m_PrivateTableInfo.wUserCount)
		{
			LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard wChairID Err!";
			return false;
		}
		iRtn = OnUserPassCard(wChairID);
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard iRtn=" << iRtn;
		return iRtn > 0;
	}
	}
	return CT_TRUE;
}

//设置指针
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	assert(NULL != pTableFrame);
	m_pGameDeskPtr = pTableFrame;

	//初始化游戏数据
	InitGameData();
	//更新游戏配置
	UpdateGameConfig();
}

//清理游戏数据
void CGameProcess::ClearGameData()
{
	//各玩家总输赢分
	memset(m_iTotalLWScore, 0, sizeof(m_iTotalLWScore));
	//各玩家全关次数
	memset(m_cbTotalQuanGuan, 0, sizeof(m_cbTotalQuanGuan));
	//各玩家被关次数
	memset(m_cbTotalBeiGuan, 0, sizeof(m_cbTotalBeiGuan));
	//各玩家炸弹个数
	memset(m_cbTotalRomb, 0, sizeof(m_cbTotalRomb));
	//各玩家通赔个数
	memset(m_cbTotalTongPei, 0, sizeof(m_cbTotalTongPei));
	//首出牌牌玩家
	m_dwFirstUser = INVALID_CHAIR;
	//是否要出黑桃3
	m_cbIsOutThree = 1;
}


//私人房游戏大局是否结束
bool CGameProcess::IsGameRoundClearing(CT_BOOL bDismissGame)
{
	if (bDismissGame)
	{//解散
		return true;
	}

	CT_WORD wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
	if (wCurrPlayCount >= m_PrivateTableInfo.wTotalPlayCount)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "IsGameRoundClearing: true";
		return true;
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "IsGameRoundClearing: false";
	return false;
}

//获取大局结算的总分
CT_INT32 CGameProcess::GetGameRoundTotalScore(CT_DWORD dwChairID)
{
	if (dwChairID >= m_PrivateTableInfo.wUserCount)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Err dwChairID=" << dwChairID;
		return 0;
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Score=" << m_iTotalLWScore[dwChairID];
	return m_iTotalLWScore[dwChairID];
}

//发送数据
CT_BOOL CGameProcess::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BOOL bRecord)
{
	if (NULL == m_pGameDeskPtr)
	{
		return false;
	}
	return m_pGameDeskPtr->SendTableData(dwChairID, wSubCmdID, pBuf, dwLen, bRecord);
}

//初始化游戏数据
void CGameProcess::InitGameData()
{
	//随机种子
	//srand((unsigned)time(NULL));
	srand(Utility::GetTick());
	//系统时间
	m_dwSysTime = 0;
	//剩余数目
	m_cbLeftCardCount = MAX_CARD_TOTAL;
	//库存扑克
	memset(m_cbRepertoryCard, 0, sizeof(m_cbRepertoryCard));
	//游戏状态
	m_cbGameStatus = GAME_STATUS_FREE;
	//当前玩家
	m_dwCurrentUser = INVALID_CHAIR;
	//还原玩家
	m_dwResumeUser = INVALID_CHAIR;
	//胜利玩家
	m_dwTurnWiner = INVALID_CHAIR;
	//上家牌型
	m_cbTurnCardType = CT_ERROR;
	//出牌数目
	m_cbTurnCardCount = 0;
	//出牌数据
	memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	//炸弹个数
	memset(m_cbRombCount, 0, sizeof(m_cbRombCount));
	//玩家牌数量
	memset(m_cbCardCount, 0, sizeof(m_cbCardCount));
	//玩家牌数据
	memset(m_cbCardData, 0, sizeof(m_cbCardData));
	//丢弃数目
	memset(m_cbDiscardCount, 0, sizeof(m_cbDiscardCount));
	//丢弃记录
	memset(m_cbDiscardCard, 0, sizeof(m_cbDiscardCard));
	//出牌次数
	memset(m_cbOutCardCount, 0, sizeof(m_cbOutCardCount));
	//出牌时间
	m_cbOutCardTime = 8;
	//玩家报单
	memset(m_cbUserSingle, 0, sizeof(m_cbUserSingle));

	m_bMustThree = false;
	m_cbSelfCardCount = MAX_COUNT;

	//游戏结算
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));

	if (NULL != m_pGameDeskPtr)
	{//获取私人房信息
		m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
	}
	//是否显示牌数
	m_cbIsLookCardCount = 0;
}

//更新游戏配置
void CGameProcess::UpdateGameConfig()
{
	if (NULL == m_pGameDeskPtr)
	{
		return;
	}
	GameCfgData Cfg;
	memset(&Cfg, 0, sizeof(Cfg));
	m_pGameDeskPtr->GetGameCfgData(Cfg);
}

//清除所有定时器
void CGameProcess::ClearAllTimer()
{
	//等待玩家出牌定时器
	m_pGameDeskPtr->KillGameTimer(ID_TIME_OUT_CARD);
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwWinChairID)
{
	if (dwWinChairID >= INVALID_CHAIR)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "NormalGameEnd Err!";
		return;
	}
	//首出牌牌玩家
	m_dwFirstUser = dwWinChairID;

	PDK_16_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));
	GameEnd.wWinUser = dwWinChairID;

	CT_BYTE	cbQuanGuan[GAME_PLAYER] = { 0 };							//全关标志
	for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		//过滤赢家
		if (i == dwWinChairID) continue;

		//玩家输赢张数
		CT_INT32 iLWCardCount = (1 == m_cbCardCount[i]) ? 0 : m_cbCardCount[i];
		GameEnd.iLWNum[i] -= iLWCardCount;
		GameEnd.iLWNum[dwWinChairID] += iLWCardCount;
		//输赢分数
		CT_LONGLONG llTempScore = iLWCardCount * (CT_INT32)m_dwlCellScore;

		CT_BYTE cbMaxSelfCard = 0;
		if (m_cbSelfCardCount == 16)
		{
			cbMaxSelfCard = MAX_COUNT;
		}
		else
		{
			cbMaxSelfCard = MAX_COUNT - 1;
		}
		if (cbMaxSelfCard == iLWCardCount)
		{	//被关乘2
			//cbCardCount *= 2;
			//被关
			GameEnd.cbBeiGuan[i] = 1;
			//全关
			++cbQuanGuan[dwWinChairID];
			//全关分
			llTempScore *= 2;
			GameEnd.llLWScore[i] -= llTempScore;
			GameEnd.llLWScore[dwWinChairID] += llTempScore;
		}
		else
		{//没被关
			GameEnd.llLWScore[i] -= llTempScore;
			GameEnd.llLWScore[dwWinChairID] += llTempScore;
		}
	}

	//计算炸弹分
	CT_LONGLONG llRombScore[GAME_PLAYER] = { 0 };
	for (CT_BYTE i = 0; i<m_PrivateTableInfo.wUserCount; ++i)
	{
		if (0 == m_cbRombCount[i]) continue;
		//有炸弹玩家
		CT_LONGLONG llTempScore = m_cbRombCount[i] * (CT_INT32)m_dwlCellScore * 10;
		llRombScore[i] += llTempScore * (m_PrivateTableInfo.wUserCount - 1);

		for (CT_BYTE j = 0; j<m_PrivateTableInfo.wUserCount; ++j)
		{//被炸的玩家
			if (j == i)continue;
			llRombScore[j] -= llTempScore;
		}
	}

	//输赢积分
	for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		GameEnd.llLWScore[i] += llRombScore[i];
	}

	//剩余牌
	memcpy(&GameEnd.cbRemainCard, m_cbCardData, sizeof(m_cbCardData));
	memcpy(&GameEnd.cbRemainCount, m_cbCardCount, sizeof(m_cbCardCount));
	//炸弹个数
	memcpy(&GameEnd.cbRombCount, m_cbRombCount, sizeof(m_cbRombCount));
	//炸弹分数
	memcpy(&GameEnd.llRombScore, llRombScore, sizeof(llRombScore));

	//发送数据
	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_GAME_END, &GameEnd, sizeof(PDK_16_CMD_S_GameEnd));

	//写入积分
	for (CT_BYTE i = 0; i<m_PrivateTableInfo.wUserCount; ++i)
	{
		ScoreInfo  ScoreData;
		memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = i;
		ScoreData.llScore = GameEnd.llLWScore[i];
		ScoreData.scoreKind = enScoreKind_Draw;
		if (GameEnd.iLWNum[i] != 0)
		{
			ScoreData.scoreKind = GameEnd.iLWNum[i]>0 ? enScoreKind_Win : enScoreKind_Lost;
		}
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: dwWinChairID=" << dwWinChairID << ", name=" << m_pGameDeskPtr->GetNickName(dwWinChairID);

	//记录总数据
	for (CT_WORD i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		m_iTotalLWScore[i] += (CT_INT32)GameEnd.llLWScore[i];
		m_cbTotalQuanGuan[i] += cbQuanGuan[i];
		m_cbTotalBeiGuan[i] += GameEnd.cbBeiGuan[i];
		m_cbTotalRomb[i] += GameEnd.cbRombCount[i];

		LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: LWScore=" << GameEnd.llLWScore[i] << ", QG=" << (CT_WORD)cbQuanGuan[i] << ",BG=" << \
			(CT_WORD)GameEnd.cbBeiGuan[i] << ",Romb=" << (CT_WORD)GameEnd.cbRombCount[i];
	}

	//记录游戏结算数据
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));
	memcpy(&m_GameEnd, &GameEnd, sizeof(PDK_16_CMD_S_GameEnd));
}

//发送总结算
void CGameProcess::SendTotalClearing(std::string strTime)
{
	PDK_16_CMD_S_CLEARING ClearingData;
	memset(&ClearingData, 0, sizeof(PDK_16_CMD_S_CLEARING));
	ClearingData.wRoomOwner = m_pGameDeskPtr->GetPRoomOwnerChairID();
	//当前时间
	_snprintf_info(ClearingData.szCurTime, sizeof(ClearingData.szCurTime), "%s", strTime.c_str());
	for (CT_WORD i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		ClearingData.llTotalLWScore[i] = m_iTotalLWScore[i];
		ClearingData.cbQuanGuanCount[i] = m_cbTotalQuanGuan[i];
		ClearingData.cbBeiGuanCount[i] = m_cbTotalBeiGuan[i];
		ClearingData.cbRombCount[i] = m_cbTotalRomb[i];

		LOG_IF(INFO, PRINT_LOG_INFO) << "GetTotalClearing: LW:" << ClearingData.llTotalLWScore[i] << ",QG:" << (CT_WORD)ClearingData.cbQuanGuanCount[i] << \
			",BG:" << (CT_WORD)ClearingData.cbBeiGuanCount[i] << ",Romb:" << (CT_WORD)ClearingData.cbRombCount[i];
	}

	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_CLEARING, &ClearingData, sizeof(PDK_16_CMD_S_CLEARING));
}

//解释私人场数据
void CGameProcess::ParsePrivateData(CT_CHAR *pBuf)
{
	if (strlen(pBuf) == 0)
	{
		return;
	}
	//获得私人扬数据
	acl::json jsonCond(pBuf);

	//是否必黑桃3
	const acl::json_node* nodeCond = jsonCond.getFirstElementByTagName("isOut3");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "isOut3 error!";
		return;
	}
	//必出黑桃3
	m_bMustThree = *nodeCond->get_bool();
	//2人玩法不用黑桃3必出
	if (m_PrivateTableInfo.wUserCount == 2)
	{
		m_bMustThree = false;
	}

	//牌张数-16张还是15张玩法
	nodeCond = jsonCond.getFirstElementByTagName("cardCount");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "card count error!";
		return;
	}
	m_cbSelfCardCount = (CT_BYTE)*nodeCond->get_int64();

	//炸弹不可拆  
	nodeCond = jsonCond.getFirstElementByTagName("notRomb");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is split romb error!";
		return;
	}
	m_GameLogic.m_cbNotSplitRomb = (CT_BYTE)*nodeCond->get_int64();

	//允许4带2  
	nodeCond = jsonCond.getFirstElementByTagName("allow4d2");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is four line two error!";
		return;
	}
	m_GameLogic.m_cbCanFourTakeTwo = (CT_BYTE)*nodeCond->get_int64();

	//允许4带3 
	nodeCond = jsonCond.getFirstElementByTagName("allow4d3");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is four line three error!";
		return;
	}
	m_GameLogic.m_cbCanFourTakeThree = (CT_BYTE)*nodeCond->get_int64();

	//三张可少带出完
	nodeCond = jsonCond.getFirstElementByTagName("outEnd3");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is out end three error!";
		return;
	}
	m_GameLogic.m_cbIsOutThreeEnd = (CT_BYTE)*nodeCond->get_int64();

	//三张可少带接完
	nodeCond = jsonCond.getFirstElementByTagName("put3End");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is end three error!";
		return;
	}
	m_GameLogic.m_cbIsThreeEnd = (CT_BYTE)*nodeCond->get_int64();

	//飞机可少带出完
	nodeCond = jsonCond.getFirstElementByTagName("outFlyEnd");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is out fly end error!";
		return;
	}
	m_GameLogic.m_cbIsOutFlyEnd = (CT_BYTE)*nodeCond->get_int64();

	//飞机可少带接完
	nodeCond = jsonCond.getFirstElementByTagName("putFlyEnd");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is fly end error!";
		return;
	}
	m_GameLogic.m_cbIsFlyEnd = (CT_BYTE)*nodeCond->get_int64();

	//是否显示牌数
	nodeCond = jsonCond.getFirstElementByTagName("showCardCount");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is fly end error!";
		return;
	}
	m_cbIsLookCardCount = (CT_BYTE)*nodeCond->get_int64();

	LOG_IF(INFO, PRINT_LOG_INFO) << "ParsePrivateData: m_bMustThree=" << m_bMustThree;
}

//是否出黑桃3
CT_BYTE CGameProcess::IsMustOutThree(CT_DWORD dwChairID)
{
	if (0 == m_bMustThree)
	{
		return 0;
	}
	return (dwChairID == m_dwFirstUser && m_cbIsOutThree == 1) ? 1 : 0;
}

//玩家出牌
CT_INT32 CGameProcess::OnUserOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout/*=CT_FALSE*/)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard: dwChairID=" << dwChairID << ",Name=" << m_pGameDeskPtr->GetNickName(dwChairID) << ",cbOutCount=" << (CT_WORD)cbOutCount << \
		",m_cbTurnCardCount=" << (CT_WORD)m_cbTurnCardCount;

	/*std::string strOutCard("OnUserOutCard: cbOutCard[");
	for (CT_WORD i=0; i<cbOutCount; ++i)
	{
	CT_CHAR	szOut[8] = {0};
	_snprintf_info(szOut, sizeof(szOut), "%x,", cbOutCard[i]);
	strOutCard.append(szOut);
	}
	strOutCard.append("]");
	LOG_IF(INFO, PRINT_LOG_INFO) << strOutCard.c_str();

	std::string strTurnCard("OnUserOutCard: m_cbTurnCardData[");
	for (CT_WORD i = 0; i < m_cbTurnCardCount; ++i)
	{
	CT_CHAR	szOut[8] = { 0 };
	_snprintf_info(szOut, sizeof(szOut), "%x,", m_cbTurnCardData[i]);
	strTurnCard.append(szOut);
	}
	strTurnCard.append("]");
	LOG_IF(INFO, PRINT_LOG_INFO) << strTurnCard.c_str();
	LOG_IF(INFO, PRINT_LOG_INFO) << "================================";*/

	if (m_bMustThree)
	{//必出黑桃3
		if (1 == m_cbIsOutThree && !IsHaveThree(cbOutCard, cbOutCount))
		{//是否要出黑桃3
			return 11;
		}
	}

	//效验状态
	if (dwChairID != m_dwCurrentUser)
	{
		return 1;
	}

	if (cbOutCount>0 && cbOutCount>m_cbCardCount[dwChairID])
	{//出牌数量不对
		return 2;
	}

	//飞机连牌数
	CT_BYTE cbAirLineCount[1] = { 0 };

	//获取类型
	CT_BYTE cbCardType = m_GameLogic.GetCardType(cbOutCard, cbOutCount, (/*0 == m_cbTurnCardCount && */cbOutCount == m_cbCardCount[dwChairID]), cbAirLineCount);

	//类型判断
	if (cbCardType == CT_ERROR)
	{
		return 3;
	}

	/*if (cbCardType!=CT_BOMB_CARD && IsHaveRomb(cbOutCard, cbOutCount))
	{
	return 4;
	}*/

	//出牌判断
	if (m_cbTurnCardCount != 0)
	{
		//对比扑克
		if (m_GameLogic.CompareCard(m_cbTurnCardData, m_cbTurnCardCount, cbOutCard, cbOutCount, cbOutCount == m_cbCardCount[dwChairID]) == false)
		{
			return 6;
		}
	}
	else
	{
		//出最后一首牌
		if (cbOutCount == m_cbCardCount[dwChairID])
		{
			//三张是否可少带出完
			if ((cbCardType == CT_THREE_TAKE_TWO) && m_GameLogic.m_cbIsOutThreeEnd != 1 && (cbOutCount != 5))
			{
				return 61;
			}
			//飞机是否可少带出完 
			if ((cbCardType == CT_AIRPLANE) && m_GameLogic.m_cbIsOutFlyEnd != 1)
			{
				if (cbAirLineCount != NULL && cbOutCount != (cbAirLineCount[0] * 5))
				{
					return 62;
				}
			}
		}
	}

	if (1 == cbOutCount)
	{
		CT_DWORD  dwTempUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;
		if (1 == m_cbUserSingle[dwTempUser])
		{//下家报单必须出最大的
			CT_BYTE cbResultCount = m_GameLogic.SearchMaxOneCard(m_cbCardData[dwChairID], m_cbCardCount[dwChairID], cbOutCard);
			//还有最大的
			if (cbResultCount > 0) return 7;
		}
	}

	//删除扑克
	if (m_GameLogic.RemoveCard(cbOutCard, cbOutCount, m_cbCardData[dwChairID], m_cbCardCount[dwChairID]) == false)
	{
		/*std::string strHandCard("OnUserOutCard: HandCard[");
		for (CT_WORD i = 0; i < m_cbCardCount[dwChairID]; ++i)
		{
		CT_CHAR	szOut[8] = { 0 };
		_snprintf_info(szOut, sizeof(szOut), "%x,", m_cbCardData[dwChairID][i]);
		strHandCard.append(szOut);
		}
		strHandCard.append("]");
		LOG_IF(INFO, PRINT_LOG_INFO) << strHandCard.c_str();*/

		return 8;
	}

	//出牌变量
	m_cbOutCardCount[dwChairID]++;
	//减去数量
	m_cbCardCount[dwChairID] -= cbOutCount;

	//设置变量
	m_dwTurnWiner = dwChairID;
	m_cbTurnCardType = cbCardType;
	m_cbTurnCardCount = cbOutCount;
	memcpy(m_cbTurnCardData, cbOutCard, sizeof(CT_BYTE)*cbOutCount);
	//必出黑桃3
	m_cbIsOutThree = 0;

	//玩家报单
	if (1 == m_cbCardCount[dwChairID]) m_cbUserSingle[dwChairID] = 1;

	//当前玩家
	m_dwCurrentUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;

	CT_BYTE cbIsPass = 0;				//是否不出
	tagSearchCardResult SearchCardResult;
	memset(&SearchCardResult, 0, sizeof(SearchCardResult));
	CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
	if (cbResultCount == 0)
	{//不能出
		cbIsPass = 1;
	}

	//构造数据
	PDK_16_CMD_S_OutCardResult OutCardResult;
	memset(&OutCardResult, 0, sizeof(OutCardResult));
	OutCardResult.dwOutCardUser = dwChairID;
	OutCardResult.cbCardCount = cbOutCount;
	OutCardResult.cbSingle = m_cbUserSingle[dwChairID];
	memcpy(OutCardResult.cbCardData, m_cbTurnCardData, m_cbTurnCardCount * sizeof(CT_BYTE));

	OutCardResult.dwCurrentUser = INVALID_CHAIR;
	if (m_cbCardCount[dwChairID]>0)
	{//没有结束
		OutCardResult.dwCurrentUser = m_dwCurrentUser;
		OutCardResult.cbOutCardTime = m_cbOutCardTime;
		OutCardResult.cbNewTurn = 0;
		OutCardResult.cbIsPass = cbIsPass;
	}

	//发送数据
	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_OUT_CARD_RESULT, &OutCardResult, sizeof(PDK_16_CMD_S_OutCardResult));

	if (0 == m_cbCardCount[dwChairID])
	{//结束
		if (cbCardType == CT_BOMB_CARD)
		{
			++m_cbRombCount[dwChairID];
		}
		OnEventGameEnd(dwChairID, GER_NORMAL);
		return 9;
	}
	return 11;
}

//玩家不出
CT_INT32 CGameProcess::OnUserPassCard(CT_DWORD dwChairID)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard: dwChairID=" << dwChairID << ",Name=" << m_pGameDeskPtr->GetNickName(dwChairID) << ",m_cbTurnCardCount=" << (CT_WORD)m_cbTurnCardCount;

	/*std::string strTurnCard("OnUserPassCard: m_cbTurnCardData[");
	for (CT_WORD i = 0; i < m_cbTurnCardCount; ++i)
	{
	CT_CHAR	szOut[8] = { 0 };
	_snprintf_info(szOut, sizeof(szOut), "%x,", m_cbTurnCardData[i]);
	strTurnCard.append(szOut);
	}
	strTurnCard.append("]");
	LOG_IF(INFO, PRINT_LOG_INFO) << strTurnCard.c_str();
	LOG_IF(INFO, PRINT_LOG_INFO) << "================================";*/

	//效验状态
	if (dwChairID != m_dwCurrentUser)
	{//不是当前玩家
		return 1;
	}
	tagSearchCardResult SearchCardResult;
	memset(&SearchCardResult, 0, sizeof(SearchCardResult));
	CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[dwChairID], m_cbCardCount[dwChairID], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
	if (cbResultCount > 0)
	{//有可以出的牌
		return 2;
	}

	//当前玩家
	m_dwCurrentUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;

	CT_BYTE cbNewTurn = 0;		//是否新一轮
	CT_BYTE cbIsPass = 0;				//是否不出
	if (m_dwTurnWiner == m_dwCurrentUser)
	{//新一轮
		cbNewTurn = 1;
		if (m_cbTurnCardType == CT_BOMB_CARD)
		{//记录炸弹
			++m_cbRombCount[m_dwTurnWiner];
		}
		//新一轮
		m_cbTurnCardType = CT_ERROR;
		m_cbTurnCardCount = 0;
		memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	}
	else
	{
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		CT_BYTE cbTempResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
		if (cbTempResultCount == 0)
		{//不能出
			cbIsPass = 1;
		}
	}

	//构造数据
	PDK_16_CMD_S_OutCardResult OutCardResult;
	memset(&OutCardResult, 0, sizeof(OutCardResult));
	OutCardResult.dwOutCardUser = dwChairID;
	OutCardResult.cbCardCount = 0;
	OutCardResult.cbSingle = m_cbUserSingle[dwChairID];

	OutCardResult.dwCurrentUser = m_dwCurrentUser;
	OutCardResult.cbOutCardTime = m_cbOutCardTime;
	OutCardResult.cbNewTurn = cbNewTurn;
	OutCardResult.cbIsPass = cbIsPass;

	//发送数据
	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_OUT_CARD_RESULT, &OutCardResult, sizeof(PDK_16_CMD_S_OutCardResult));

	return 3;
}

//通知下一个玩家出牌
CT_INT32 CGameProcess::NotifyNextOutCard(CT_DWORD dwChairID)
{
	m_dwCurrentUser = INVALID_CHAIR;
	m_dwResumeUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;

	if (m_dwResumeUser == m_dwTurnWiner)
	{
		//当前出牌玩家
		m_dwCurrentUser = m_dwTurnWiner;
		if (m_cbTurnCardType == CT_BOMB_CARD)
		{//记录炸弹
			++m_cbRombCount[m_dwTurnWiner];
		}

		//新一轮
		m_cbTurnCardType = CT_ERROR;
		m_cbTurnCardCount = 0;
		memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));

		//通知出牌
		NotifyOutCard(m_dwCurrentUser, m_cbOutCardTime, 1, 1);
		return 1;
	}
	else
	{
		tagSearchCardResult SearchCardResult;
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwResumeUser], m_cbCardCount[m_dwResumeUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
		//没有可大过的牌
		if (0 == cbResultCount)
		{//通知不能出牌
			NotifyOutCard(m_dwResumeUser, 1, 0, 0);
			m_pGameDeskPtr->SetGameTimer(ID_TIME_NEXT_OUT_CARD, 1000);
		}
		else if (cbResultCount > 0)
		{//找到下家
		 //通知出牌
			NotifyOutCard(m_dwResumeUser, m_cbOutCardTime, 1, 0);
			//当前出牌玩家
			m_dwCurrentUser = m_dwResumeUser;
			return 2;
		}
	}
	return 3;
}

//查找下一个出牌玩家
CT_DWORD CGameProcess::FindNextOutCardUser(CT_DWORD dwChairID)
{
	for (CT_BYTE i = 1; i <= m_PrivateTableInfo.wUserCount; ++i)
	{
		CT_DWORD dwTempUser = (dwChairID + i) % m_PrivateTableInfo.wUserCount;
		if (dwTempUser == dwChairID)
		{
			if (m_cbTurnCardType == CT_BOMB_CARD)
			{//记录炸弹
				++m_cbRombCount[m_dwTurnWiner];
			}

			//新一轮
			m_cbTurnCardType = CT_ERROR;
			m_cbTurnCardCount = 0;
			memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
			return dwTempUser;
		}

		tagSearchCardResult SearchCardResult;
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[dwTempUser], m_cbCardCount[dwTempUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
		if (cbResultCount>0)
		{//找到有打得过的玩家
			return dwTempUser;
		}
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "Find User Err!";
	return INVALID_CHAIR;
}

//通知出牌
CT_BOOL CGameProcess::NotifyOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCardTime, CT_BYTE cbOutCardFlag, CT_BYTE cbNewTurn)
{
	PDK_16_CMD_S_NotifyOutCard OutCard;
	memset(&OutCard, 0, sizeof(OutCard));
	OutCard.dwOutCardUser = dwChairID;
	OutCard.cbOutCardTime = cbOutCardTime;
	OutCard.cbOutCardFlag = cbOutCardFlag;
	OutCard.cbNewTurn = cbNewTurn;
	OutCard.cbMustThree = 0;
	if (m_bMustThree)
	{//必出黑桃3
		OutCard.cbMustThree = IsMustOutThree(dwChairID);
	}

	//发送数据
	return SendTableData(INVALID_CHAIR, PDK_16_SUB_S_NOTIFY_OUT_CARD, &OutCard, sizeof(PDK_16_CMD_S_NotifyOutCard));
}

//是否有炸弹
CT_BOOL CGameProcess::IsHaveRomb(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	tagAnalyseResult AnalyseResult = { 0 };
	m_GameLogic.AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

	return (AnalyseResult.cbBlockCount[3] > 0);
}

//是否有黑桃3
CT_BOOL CGameProcess::IsHaveThree(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	for (CT_WORD i = 0; i < cbCardCount&&i < MAX_COUNT; ++i)
	{
		if (cbCardData[i] == 0x33)
		{//黑桃3
			return true;
		}
	}
	return false;
}

void CGameProcess::XiPai()
{
	//分发扑克
	CT_DWORD	dwFirstUser = m_dwFirstUser;
	for (CT_BYTE i = 0; i<100; ++i)
	{
		m_dwFirstUser = dwFirstUser;
		memset(m_cbCardData, 0, sizeof(m_cbCardData));

		CT_BYTE cbMaxCard = 0;
		CT_BYTE cbMaxSelfCard = 0;
		if (m_cbSelfCardCount == 16)
		{
			cbMaxCard = MAX_CARD_TOTAL;
			cbMaxSelfCard = MAX_COUNT;
		}
		else
		{
			cbMaxCard = MAX_CARD_TOTAL - 3;
			cbMaxSelfCard = MAX_COUNT - 1;
		}

		m_cbLeftCardCount = cbMaxCard;
		m_GameLogic.RandCardData(m_cbRepertoryCard, cbMaxCard);

		///////////////////////////////////////////////////////	
		//最后一手三带能打起
		/*m_cbRepertoryCard[47] = 0x03;
		m_cbRepertoryCard[46] = 0x13;
		m_cbRepertoryCard[45] = 0x33;
		m_cbRepertoryCard[44] = 0x04;
		m_cbRepertoryCard[43] = 0x14;
		m_cbRepertoryCard[42] = 0x24;
		m_cbRepertoryCard[41] = 0x34;
		m_cbRepertoryCard[40] = 0x23;
		m_cbRepertoryCard[39] = 0x08;
		m_cbRepertoryCard[38] = 0x2C;
		m_cbRepertoryCard[37] = 0x3C;
		m_cbRepertoryCard[36] = 0x0D;
		m_cbRepertoryCard[35] = 0x1D;
		m_cbRepertoryCard[34] = 0x01;
		m_cbRepertoryCard[33] = 0x11;
		m_cbRepertoryCard[32] = 0x21;

		m_cbRepertoryCard[31] = 0x05;
		m_cbRepertoryCard[30] = 0x15;
		m_cbRepertoryCard[29] = 0x25;
		m_cbRepertoryCard[28] = 0x35;
		m_cbRepertoryCard[27] = 0x06;
		m_cbRepertoryCard[26] = 0x16;
		m_cbRepertoryCard[25] = 0x26;
		m_cbRepertoryCard[24] = 0x36;
		m_cbRepertoryCard[23] = 0x07;
		m_cbRepertoryCard[22] = 0x17;
		m_cbRepertoryCard[21] = 0x27;
		m_cbRepertoryCard[20] = 0x37;
		m_cbRepertoryCard[19] = 0x18;
		m_cbRepertoryCard[18] = 0x1C;
		m_cbRepertoryCard[17] = 0x2D;
		m_cbRepertoryCard[16] = 0x3D;

		m_cbRepertoryCard[15] = 0x09;
		m_cbRepertoryCard[14] = 0x19;
		m_cbRepertoryCard[13] = 0x29;
		m_cbRepertoryCard[12] = 0x39;
		m_cbRepertoryCard[11] = 0x0A;
		m_cbRepertoryCard[10] = 0x1A;
		m_cbRepertoryCard[9]  = 0x2A;
		m_cbRepertoryCard[8]  = 0x3A;
		m_cbRepertoryCard[7]  = 0x0B;
		m_cbRepertoryCard[6]  = 0x1B;
		m_cbRepertoryCard[5]  = 0x2B;
		m_cbRepertoryCard[4]  = 0x3B;
		m_cbRepertoryCard[3]  = 0x28;
		m_cbRepertoryCard[2]  = 0x38;
		m_cbRepertoryCard[1]  = 0x32;
		m_cbRepertoryCard[0]  = 0x0C;
		m_dwFirstUser = 0;
		m_dwCurrentUser = 0;*/

		/*m_cbRepertoryCard[47] = 0x33;
		m_cbRepertoryCard[46] = 0x23;
		m_cbRepertoryCard[45] = 0x13;
		m_cbRepertoryCard[44] = 0x34;
		m_cbRepertoryCard[43] = 0x24;
		m_cbRepertoryCard[42] = 0x14;
		m_cbRepertoryCard[41] = 0x35;
		m_cbRepertoryCard[40] = 0x36;
		m_cbRepertoryCard[39] = 0x3C;
		m_cbRepertoryCard[38] = 0x3D;
		m_cbRepertoryCard[37] = 0x3A;
		m_cbRepertoryCard[36] = 0x2A;
		m_cbRepertoryCard[35] = 0x1A;
		m_cbRepertoryCard[34] = 0x3B;
		m_cbRepertoryCard[33] = 0x2B;
		m_cbRepertoryCard[32] = 0x1B;

		m_cbRepertoryCard[31] = 0x38;
		m_cbRepertoryCard[30] = 0x28;
		m_cbRepertoryCard[29] = 0x18;
		m_cbRepertoryCard[28] = 0x39;
		m_cbRepertoryCard[27] = 0x29;
		m_cbRepertoryCard[26] = 0x19;
		m_cbRepertoryCard[25] = 0x2C;
		m_cbRepertoryCard[24] = 0x1C;
		m_cbRepertoryCard[23] = 0x2D;
		m_cbRepertoryCard[22] = 0x21;
		m_cbRepertoryCard[21] = 0x11;
		m_cbRepertoryCard[20] = 0x32;
		m_cbRepertoryCard[19] = 0x25;
		m_cbRepertoryCard[18] = 0x26;
		m_cbRepertoryCard[17] = 0x37;
		m_cbRepertoryCard[16] = 0x27;

		m_cbRepertoryCard[15] = 0x03;
		m_cbRepertoryCard[14] = 0x04;
		m_cbRepertoryCard[13] = 0x08;
		m_cbRepertoryCard[12] = 0x09;
		m_cbRepertoryCard[11] = 0x05;
		m_cbRepertoryCard[10] = 0x15;
		m_cbRepertoryCard[9] = 0x06;
		m_cbRepertoryCard[8] = 0x16;
		m_cbRepertoryCard[7] = 0x07;
		m_cbRepertoryCard[6] = 0x17;
		m_cbRepertoryCard[5] = 0x0A;
		m_cbRepertoryCard[4] = 0x0B;
		m_cbRepertoryCard[3] = 0x0C;
		m_cbRepertoryCard[2] = 0x1D;
		m_cbRepertoryCard[1] = 0x0D;
		m_cbRepertoryCard[0] = 0x01;*/
		///////////////////////////////////////////////////////
		CT_BYTE cbSingle = 0;
		CT_BYTE cbDouble = 0;
		for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
		{
			m_cbLeftCardCount -= cbMaxSelfCard;
			m_cbCardCount[i] = cbMaxSelfCard;
			memcpy(m_cbCardData[i], &m_cbRepertoryCard[m_cbLeftCardCount], sizeof(CT_BYTE)*cbMaxSelfCard);

			if (m_dwFirstUser != INVALID_CHAIR) continue;

			if (m_PrivateTableInfo.wUserCount == GAME_PLAYER)
			{
				for (CT_BYTE n = 0; n < cbMaxSelfCard; ++n)
				{
					if (0x33 == m_cbCardData[i][n])
					{//记录黑桃3玩家
						m_dwFirstUser = i;
					}
				}
			}
			else
			{
				m_dwFirstUser = rand() % m_PrivateTableInfo.wUserCount;
			}

			tagSearchCardResult SearchCardResult;
			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			if (m_GameLogic.SearchLineCardType(m_cbCardData[i], m_cbCardCount[i], 0, 1, 0, SearchCardResult))
			{//单顺
				for (CT_BYTE i = 0; i < SearchCardResult.cbSearchCount; ++i)
				{
					if (SearchCardResult.cbCardCount[i] >= 12)
					{
						cbSingle = 1;
						break;
					}
				}
			}

			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			if (m_GameLogic.SearchLineCardType(m_cbCardData[i], m_cbCardCount[i], 0, 2, 0, SearchCardResult))
			{//双顺
				for (CT_BYTE i = 0; i < SearchCardResult.cbSearchCount; ++i)
				{
					if (SearchCardResult.cbCardCount[i] >= 7 * 2)
					{
						cbDouble = 1;
						break;
					}
				}
			}
		}
		if (0 == cbSingle && 0 == cbDouble)
		{//没有单顺或双顺
			break;
		}
	}
}

