
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
//#include "timeFunction.h"
#include <set>
#include <random>

//打印日志
#define PRINT_GAME_INFO					1
#define PRINT_GAME_WARNING				1


////////////////////////////////////////////////////////////////////////
//定时器
#define ID_TIME_SEND_BET_RESULT			101					//同步下注信息定时器
#define ID_TIME_BET_SCORE				102					//下注定时器 
#define ID_TIME_GAME_END				103					//结束定时器 

//定时器时间
#define TIME_SEND_BET_RESULT			1					//同步下注消息
#define TIME_BET						12					//下注时间
#define TIME_GAME_END					10					//结束时间 (有人下注有大赢家)

////////////////////////////////////////////////////////////////////////

struct tagChairUserScore
{
	CT_WORD		wChairID;
	CT_LONGLONG llScore;
};

bool comp_by_score(tagChairUserScore& p1, tagChairUserScore& p2)
{
	return p1.llScore > p2.llScore;
}

bool comp_by_score2(CGameProcess::tagAreaWinScore& p1, CGameProcess::tagAreaWinScore &p2)
{
	return p1.llWinScore < p2.llWinScore;
}

bool comp_by_score3(CGameProcess::tagAreaWinScore& p1, CGameProcess::tagAreaWinScore &p2)
{
	return p1.llWinScore > p2.llWinScore;
}

bool comp_by_jetton(CGameProcess::tagUserHistoryEx& p1, CGameProcess::tagUserHistoryEx& p2)
{
	return p1.dwJettonScore > p2.dwJettonScore;
}

CT_LONGLONG CGameProcess::m_llSmallJetton[MAX_JETTON_NUM] = { 100,1000,5000,10000,50000 };

CGameProcess::CGameProcess(void) : m_pGameDeskPtr(NULL)
{
	//初始化桌子数据
	InitGameTableData();
	//初始化数据
	InitGameData();
	//清理游戏数据
	//ClearGameData();
}

CGameProcess::~CGameProcess(void)
{
}

//游戏开始
void CGameProcess::OnEventGameStart()
{
	//检测是否跨天
	CT_DWORD dwTimeNow = (CT_DWORD)time(NULL);
	CT_BOOL bAcrossDay = Utility::isAcrossTheDay(m_dwTodayTime, dwTimeNow);
	if (bAcrossDay)
	{
		m_llTodayStorageControl = 0;
		m_dwTodayTime = dwTimeNow;
	}

	//初始化数据
	InitGameData();

	//开始下注消息
	HHDZ_CMD_S_GameBet GameStartBet;
	GameStartBet.cbTimeLeave = TIME_BET;

	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
		{
			//玩家不存在
			continue;
		}

		//检测玩家是否还可以续注
		if (m_cbCanAddLastJetton[i] == 1)
		{
			CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
			CT_LONGLONG llLastJettonCount = 0;
			for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
			{
				llLastJettonCount += m_llUserLastJettonScore[wAreaIndex][i];
			}

			if (llLastJettonCount > llUserScore || llUserScore < CAN_JETTON_SCORE)
			{
				m_cbCanAddLastJetton[i] = 0;
				for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
				{
					m_llUserLastJettonScore[wAreaIndex][i] = 0;
				}
				//memset(m_llUserLastJettonScore, 0, sizeof(m_llUserLastJettonScore));
			}
		}
		GameStartBet.cbCanAddLastBet = m_cbCanAddLastJetton[i];

		//发送数据
		SendTableData(i, HHDZ_SUB_S_GAME_START_BET, &GameStartBet, sizeof(HHDZ_CMD_S_GameBet), false);
	}


	//所有跟投玩家开始新一轮的跟投
	for (auto& it : m_mapFollowBetInfo)
	{
		it.second.cbFollowThisRound = 0;
	}
	return;
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//m_dwEndTime = (int)time(NULL) + TIME_GAME_END;
	//LOG_IF(INFO, PRINT_GAME_INFO) << "OnEventGameEnd";
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


	//游戏结束
	m_pGameDeskPtr->ConcludeGame(GAME_STATUS_FREE, "");
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
	/*case GAME_STATUS_FREE:			//空闲状态
	{
		//发送消息
		HHDZ_CMD_S_StatusFree GameFree;

		CT_DWORD dwTimeLeave = m_dwFreeTime - (CT_DWORD)time(NULL);
		if (dwTimeLeave == 0 || dwTimeLeave > TIME_FREE)
		{
			dwTimeLeave = 1;
		}
		GameFree.cbTimeLeave = dwTimeLeave;									//剩余时间

		SendTableData(dwChairID, SC_GAMESCENE_FREE, &GameFree, sizeof(HHDZ_CMD_S_StatusFree), false);
		
		//同步椅子信息
		SendUserSitList(dwChairID);
		break;
	}
	case GAME_STATUS_SENDCARD:
	{
		HHDZ_CMD_S_StatusSendCard StatusSendCard;
		CT_DWORD dwTime = (CT_DWORD)time(NULL) - m_dwSendCardTime;
		dwTime = dwTime < TIME_SENDCARD ? (TIME_SENDCARD - dwTime) : 0;
		StatusSendCard.cbTimeLeave = (CT_BYTE)dwTime;								//剩余时间
		StatusSendCard.cbTotalTime = TIME_SENDCARD;									//总时间

		SendTableData(dwChairID, SC_GAMESCENE_SENDCARD, &StatusSendCard, sizeof(HHDZ_CMD_S_StatusSendCard), false);

		//同步椅子信息
		SendUserSitList(dwChairID);
		break;
	}*/
	case GAME_STATUS_SCORE:		//下注状态
	{
		HHDZ_CMD_S_StatusPlay StatusPlay;
		memset(&StatusPlay, 0, sizeof(HHDZ_CMD_S_StatusPlay));

		//玩家是否可以续注
		StatusPlay.cbCanAddLastBet = m_cbCanAddLastJetton[dwChairID];
		//全体总注
		for (CT_WORD i = 0; i < AREA_COUNT; ++i)
		{
			StatusPlay.dAllJettonScore[i] = m_llAllJettonScore[i]*TO_DOUBLE;
		}
		
		for (CT_WORD i = 0; i < AREA_COUNT; ++i)
		{
			//个人总注
			StatusPlay.dUserJettonScore[i] += m_llUserJettonScore[i][dwChairID] * TO_DOUBLE;
		}

		CT_DWORD dwTime = m_dwBetTime - (CT_DWORD)time(NULL);
		//dwTime = dwTime < 0 ? 0 : dwTime;
		StatusPlay.cbTimeLeave = (CT_BYTE)dwTime;

		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_PLAY, &StatusPlay, sizeof(HHDZ_CMD_S_StatusPlay),false);
		//同步椅子信息
		SendUserSitList(dwChairID);
		break;
	}
	case GAME_STATUS_END:
	{
		HHDZ_CMD_S_StatusEnd statusEnd;
		memset(&statusEnd, 0, sizeof(statusEnd));

		CT_BYTE cbTime = (CT_BYTE)(m_dwEndTime - time(NULL));
		cbTime = cbTime < 0 ? 0 : cbTime;
		statusEnd.cbTimeLeave = cbTime;

		statusEnd.cbTotalTime = TIME_GAME_END;

		for (CT_WORD i = 0; i < AREA_COUNT; ++i)
		{
			statusEnd.dAllJettonScore[i] = m_llAllJettonScore[i] * TO_DOUBLE;

			//个人总注
			statusEnd.dUserJettonScore[i] += m_llUserJettonScore[i][dwChairID] * TO_DOUBLE;
		}

		memcpy(&statusEnd.cbTableCardArray, &m_GameEnd.cbTableCardArray, sizeof(statusEnd.cbTableCardArray));
		memcpy(&statusEnd.cbTableCarType, &m_GameEnd.cbTableCardType, sizeof(statusEnd.cbTableCarType));
		memcpy(&statusEnd.dLWScore, &m_GameEnd.dLWScore, sizeof(statusEnd.dLWScore));
		CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
		//玩家自己的输赢分
		auto it = m_mapUserLWScore.find(dwUserID);
		if (it != m_mapUserLWScore.end())
		{
			statusEnd.dLWScore[7] = it->second*TO_DOUBLE;
		}
		else
		{
			statusEnd.dLWScore[7] = 0;
		}

		memcpy(&statusEnd.cbAreaWin, &m_GameEnd.cbAreaWin, sizeof(statusEnd.cbAreaWin));
		memcpy(&statusEnd.dUserAreaLWScore, &m_GameEnd.dUserAreaLWScore, sizeof(statusEnd.dUserAreaLWScore));
		memcpy(&statusEnd.dUserTotalScore, &m_GameEnd.dUserTotalScore, sizeof(statusEnd.dUserTotalScore));
		//进入的玩家真实分数不能通过原来的成绩获取,查询玩家的真实分数
		statusEnd.dUserTotalScore[dwChairID] = m_pGameDeskPtr->GetUserScore(dwChairID) * TO_DOUBLE;
		//拷贝玩家ID
		memcpy(&statusEnd.dwUserID, &m_dwAllUserID, sizeof(m_GameEnd.dwUserID));
		//statusEnd.cbEndType = m_GameEnd.cbEndType;
		statusEnd.cbBigWinnerHeadIndex = m_GameEnd.cbBigWinnerHeadIndex;
		memcpy(&statusEnd.szBigWinnerNickName, &m_GameEnd.szBigWinnerNickName, sizeof(statusEnd.szBigWinnerNickName));
		statusEnd.dBigWinnerScore = statusEnd.dBigWinnerScore;
		//memcpy(&statusEnd.cbHistory, &m_GameEnd.cbHistory, sizeof(statusEnd.cbHistory));

		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_END, &statusEnd, sizeof(statusEnd), false);
		//同步椅子信息
		SendUserSitList(dwChairID);
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
        case ID_TIME_SEND_BET_RESULT:
        {
            HHDZ_CMD_S_Timing_BetResult timedBetResult;
            for (int i = 0; i < AREA_COUNT; ++i)
            {
                timedBetResult.dAreaBetScore[i] = m_llAreaTimedRefreshScore[i]*TO_DOUBLE;
                timedBetResult.dAreaTotalBetScore[i] = GetAreaBetScore(i)*TO_DOUBLE;//各区域总已下注金额

                //清空定时刷新数据
                m_llAreaTimedRefreshScore[i] = 0;
            }
            SendTableData(INVALID_CHAIR, HHDZ_SUB_S_BET_SCORE_RESULT_TIMED_REFRESH, &timedBetResult, sizeof(timedBetResult), false);

            break;
        }
	case ID_TIME_BET_SCORE:			//下注时间到
	{
		//清理定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_BET_SCORE);
        m_pGameDeskPtr->KillGameTimer(ID_TIME_SEND_BET_RESULT);

		//设置状态
		m_cbGameStatus = GAME_STATUS_END;

		//控牌操作
		StorageControl();
		//结束游戏
		OnEventGameEnd(INVALID_CHAIR, GER_NORMAL);

		m_pGameDeskPtr->SetGameTimer(ID_TIME_GAME_END, (TIME_GAME_END + 1) * 1000);
		m_dwEndTime = (CT_DWORD)time(NULL) + TIME_GAME_END;	

		m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Free);

		return;
	}
	case ID_TIME_GAME_END:			//结束游戏
	{
		//清理定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_GAME_END);
		
		//清除桌子离线玩家
		m_pGameDeskPtr->ClearTableUser(INVALID_CHAIR, true);

		//计算玩家历史上记录
		CalUserHistory(true);

		OnEventGameStart();
		
		//设置状态
		m_cbGameStatus = GAME_STATUS_SCORE;

		m_dwBetTime = (CT_DWORD)time(NULL) + TIME_BET;
		m_pGameDeskPtr->SetGameTimer(ID_TIME_BET_SCORE, (TIME_BET) * 1000);
        m_pGameDeskPtr->SetGameTimer(ID_TIME_SEND_BET_RESULT, TIME_SEND_BET_RESULT*1000);
        m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Start);
		return;
	}
	return;
	}
}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	CT_WORD wRtn = 0;
	switch (dwSubCmdID)
	{
	case HHDZ_SUB_C_BET:					//用户下注
	{
		if (GAME_STATUS_SCORE != m_cbGameStatus)
		{//游戏状态不对
			SendErrorCode(wChairID, ERROR_STATUS);
			return false;
		}

		if (sizeof(HHDZ_CMD_C_Bet) != dwDataSize)
		{//效验消息大小
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}
		//消息处理
		HHDZ_CMD_C_Bet* pBet = (HHDZ_CMD_C_Bet*)pDataBuffer;
		if (NULL == pBet)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}

		wRtn = OnUserBetScore(wChairID, pBet->cbJettonArea,pBet->wJettonIndex);

		/*LOG_IF(INFO, PRINT_GAME_INFO) << "OnUserBetScore wChairID=" << wChairID << ",wRtn=" << wRtn 
			<<" ,cbJettonArea="<<(int)pBet->cbJettonArea << " ,wJettonIndex=" << pBet->wJettonIndex;*/
		return wRtn > 0;
	}
	case HHDZ_SUB_C_ADD_LAST_BET:
	{
	    return true;
		if (m_cbGameStatus != GAME_STATUS_SCORE)
		{
			SendErrorCode(wChairID, ERROR_STATUS);
			return false;
		}

		wRtn = OnUserBetLastScore(wChairID);
		return wRtn > 0;
	}
	case HHDZ_SUB_C_QUERY_USER_RANK:
	{
		return 	OnQueryUserGameHistory(wChairID);
	}
	case HHDZ_SUB_C_FOLLOW_BET:
	{
		return true;
		//效验消息大小
		if (sizeof(HHDZ_CMD_C_Follow_Bet) != dwDataSize)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}

		//消息处理
		HHDZ_CMD_C_Follow_Bet* pFollowBet = (HHDZ_CMD_C_Follow_Bet*)pDataBuffer;
		if (NULL == pFollowBet)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}

		return OnUserFollowBet(wChairID, pFollowBet->cbSitID, pFollowBet->cbJettonIndex, pFollowBet->cbCount);
	}
	case HHDZ_SUB_C_CANCEL_FOLLOW:
	{
		return false;
		return OnUserCancelFollowBet(wChairID, en_Cancel_Self, true);
	}
	}
	return CT_TRUE;
}

//const char* pCardTypeName[10] = {"无","单牌类型", "对子类型","顺子类型","金花类型","顺金类型", "豹子类型","大对子"};
//#include <iostream>
void CGameProcess::OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (m_cbGameStatus == GAME_STATUS_FREE)
	{
		if (!m_bIsTableRuning)
		{
			CT_DWORD dwTimeNow = (CT_DWORD)time(NULL);
			CT_BOOL bAcrossDay = Utility::isAcrossTheDay(m_dwTodayTime, dwTimeNow);
			if (bAcrossDay)
			{
				m_llTodayStorageControl = 0;
				m_dwTodayTime = dwTimeNow;
			}

			m_bIsTableRuning = true;

			//游戏开始
			OnEventGameStart();

			//设置状态
			m_cbGameStatus = GAME_STATUS_SCORE;
			//设置时间
			m_pGameDeskPtr->SetGameTimer(ID_TIME_BET_SCORE, (TIME_BET) * 1000);
			m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Start);
			m_dwBetTime = (CT_DWORD)time(NULL) + TIME_BET;
		}
	}

	//历史记录消息
	HHDZ_CMD_S_HistoryResult HistoryResult;
	memset(&HistoryResult, 0, sizeof(HistoryResult));
	HistoryResult.cbCount = m_cbHistoryCount;
	memcpy(HistoryResult.cbHistory,m_cbHistory,sizeof(HistoryResult.cbHistory));
	memcpy(HistoryResult.cbWinRate, m_cbWinRate, sizeof(HistoryResult.cbWinRate));
	if (m_cbHistoryCount >= HISTORY_CARD_TYPE_COUNT)
	{
		CT_BYTE cbWinCardTypeIndex = 0;
		for (CT_BYTE i = m_cbHistoryCount-HISTORY_CARD_TYPE_COUNT; i != m_cbHistoryCount; ++i)
		{
			HistoryResult.cbWinCardType[cbWinCardTypeIndex++] = m_cbWinCardType[i];
		}
	} else
	{
		for (CT_BYTE i = 0; i != HISTORY_CARD_TYPE_COUNT; ++i)
		{
			HistoryResult.cbWinCardType[i] = m_cbWinCardType[i];
		}
	}


	//大路
	for (auto posRow = 0; posRow != (int)m_vecBigHistory.size(); ++posRow)
	{
		std::vector<CT_BYTE>& vecHistoryValue = m_vecBigHistory[posRow];
		for (auto posCol = 0; posCol != (int)vecHistoryValue.size(); ++posCol)
		{
			HistoryResult.cbBigHistory[posRow][posCol] = vecHistoryValue[posCol];
		}
	}
	//大眼仔路
	for (auto posRow = 0; posRow != (int)m_vecBigEyeRoad.size(); ++posRow)
	{
		std::vector<CT_BYTE>& vecHistoryValue = m_vecBigEyeRoad[posRow];
		for (auto posCol = 0; posCol != (int)vecHistoryValue.size(); ++posCol)
		{
			HistoryResult.cbBigEyeHistory[posRow][posCol] = vecHistoryValue[posCol];
		}
	}
	//小路
	for (auto posRow = 0; posRow != (int)m_vecSmallRoad.size(); ++posRow)
	{
		std::vector<CT_BYTE>& vecHistoryValue = m_vecSmallRoad[posRow];
		for (auto posCol = 0; posCol != (int)vecHistoryValue.size(); ++posCol)
		{
			HistoryResult.cbSmallHistory[posRow][posCol] = vecHistoryValue[posCol];
		}
	}
	//蟑螂路
	for (auto posRow = 0; posRow != (int)m_vecCockroachRoad.size(); ++posRow)
	{
		std::vector<CT_BYTE>& vecHistoryValue = m_vecCockroachRoad[posRow];
		for (auto posCol = 0; posCol != (int)vecHistoryValue.size(); ++posCol)
		{
			HistoryResult.cbCockroachHistory[posRow][posCol] = vecHistoryValue[posCol];
		}
	}
	HistoryResult.cbRedPredictBigEye = m_cbRedPredictBigEye;
	HistoryResult.cbRedPredictSmall = m_cbRedPredictSmall;
	HistoryResult.cbRedPredictCockroach = m_cbRedPredictCockroach;

	HistoryResult.cbBlackPredictBigEye = m_cbBlackPredictBigEye;
	HistoryResult.cbBlackPredictSmall = m_cbBlackPredictSmall;
	HistoryResult.cbBlackPredictCockroach = m_cbBlackPredictCockroach;

	SendTableData(dwChairID, HHDZ_SUB_S_QUERY_HISTORY, &HistoryResult, sizeof(HHDZ_CMD_S_HistoryResult), false);

	//玩家跟投信息
	auto it = m_mapFollowBetInfo.find(dwChairID);
	if (it != m_mapFollowBetInfo.end())
	{
		tagUserFollowBet& UserFollowBetInfo = it->second;
		SendUserFollowInfo(dwChairID, UserFollowBetInfo);
	}


	//如果新进入玩家,清空游戏局数
	if (m_cbPlayStatus[dwChairID] == 0)
	{
		memset(&m_UserHistory[dwChairID], 0, sizeof(m_UserHistory[dwChairID]));
		m_UserHistory[dwChairID].wChairID = dwChairID;
	}
}

void CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag)
{
	if (dwChairID >= GAME_PLAYER)
	{
		return;
	}

	CT_BOOL bRealLeave = true;
	//如果未押注玩家离开，直接踢出去( 非游戏中框架已经清空玩家了)
	if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_Start)
	{
		if (GetUserAllAreaBetScore(dwChairID) == 0)
		{
			m_pGameDeskPtr->ClearTableUser(dwChairID, true);
			//memset(&m_UserHistory[dwChairID], 0, sizeof(m_UserHistory[dwChairID]));
		}
		else
		{
			bRealLeave = false;
		}
	}
	
	//真正里开
	if (bRealLeave)
	{
		memset(&m_UserHistory[dwChairID], 0, sizeof(m_UserHistory[dwChairID]));

		//座椅操作
		CT_WORD wPos = GetUserSitPos(dwChairID);
		if (wPos != INVALID_CHAIR)
		{
			m_wSitList[wPos] = INVALID_CHAIR; //更新椅子信息

			SortUserHistory(true);
		}

		//跟投信息
		auto it = m_mapFollowBetInfo.find(dwChairID);
		if (it != m_mapFollowBetInfo.end())
		{
			OnUserCancelFollowBet(dwChairID, en_Cancel_Self, false);
		}

		//删除玩家ID
		m_dwAllUserID[dwChairID] = 0;

		//玩家离开清空续注功能
		if (m_cbCanAddLastJetton[dwChairID] == 1)
		{
			m_cbCanAddLastJetton[dwChairID] = 0;
			for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
			{
				m_llUserLastJettonScore[wAreaIndex][dwChairID] = 0;
				m_llUserJettonScore[wAreaIndex][dwChairID] = 0;
			}
		}
	}
}

void CGameProcess::OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	//空闲状态
	if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_Free)
	{
		//座椅操作
		CT_WORD wPos = GetUserSitPos(dwChairID);
		if (wPos != INVALID_CHAIR)
		{
			m_wSitList[wPos] = INVALID_CHAIR; //更新椅子信息

			memset(&m_UserHistory[dwChairID], 0, sizeof(m_UserHistory[dwChairID]));
			SortUserHistory(true);
		}
	}

	//删除玩家ID
	m_dwAllUserID[dwChairID] = 0;

	//玩家离开清空续注功能
	if (m_cbCanAddLastJetton[dwChairID] == 1)
	{
		m_cbCanAddLastJetton[dwChairID] = 0;
		for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
		{
			m_llUserLastJettonScore[wAreaIndex][dwChairID] = 0;
		}
	}
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

}

//私人房游戏大局是否结束
bool CGameProcess::IsGameRoundClearing(CT_BOOL bDismissGame)
{
	/*if (bDismissGame)
	{//解散
		return true;
	}*/
	return false;
}

//获取大局结算的总分
CT_INT32 CGameProcess::GetGameRoundTotalScore(CT_DWORD dwChairID)
{
	if (dwChairID >= GAME_PLAYER)
	{
		//LOG_IF(INFO, PRINT_GAME_INFO) << "GetGameRoundTotalScore: Err dwChairID=" << dwChairID;
		return 0;
	}
	return (int)m_llUserAllLWScore[dwChairID];
}

//设置游戏黑名单
CT_VOID CGameProcess::SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList)
{
	m_vecBlackList.clear();
	
	for (auto& it : vecBlackList)
	{
		m_vecBlackList.push_back(it);
	}
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
	//是否有玩家下注
	m_bHasUserAddJetton = false;
	//游戏中玩家
	memset(m_cbPlayStatus,0,sizeof(m_cbPlayStatus));
	//全体总注
	memset(m_llAllJettonScore,0,sizeof(m_llAllJettonScore));
	//真实玩家总注
	memset(m_llRealUserJettonScore, 0 , sizeof(m_llRealUserJettonScore));
	//区域个人总注
	memset(m_llUserJettonScore,0,sizeof(m_llUserJettonScore));
	//个人总注
	memset(m_llUserTotalJettonScore, 0, sizeof(m_llUserTotalJettonScore));
	//参与游戏的玩家ID
	memset(m_dwAllUserID, 0, sizeof(m_dwAllUserID));

	//下注时间
	m_dwBetTime = 0;
	//结束时间
	m_dwEndTime = 0;
	//m_cbEndTimeInterval = 0;
	//桌面扑克
	memset(m_cbTableCardArray,0,sizeof(m_cbTableCardArray));
	//区域总输赢分	
	memset(m_llAreaTotalLWScore, 0, sizeof(m_llAreaTotalLWScore));
	//玩家区域总输赢分
	memset(m_llUserAreaTotalLWScore, 0, sizeof(m_llUserAreaTotalLWScore));
	
	//剩余数目
	m_cbLeftCardCount = MAX_CARD_TOTAL;
	//库存扑克
	memset(m_cbRepertoryCard, 0, sizeof(m_cbRepertoryCard));
	//玩家当局总输赢分
	memset(m_llUserAllLWScore, 0, sizeof(m_llUserAllLWScore));
	//玩家原始分
	memset(&m_llUserSourceScore, 0, sizeof(m_llUserSourceScore));
	//定时刷新金币信息
    memset(&m_llAreaTimedRefreshScore, 0, sizeof(m_llAreaTimedRefreshScore));

	//游戏记录
	memset(m_RecordScoreInfo, 0, sizeof(m_RecordScoreInfo));

	//结算数据
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));

	//玩家输赢分(根据userid来索引)
	m_mapUserLWScore.clear();
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
	m_pGameDeskPtr->KillGameTimer(ID_TIME_BET_SCORE);
	m_pGameDeskPtr->KillGameTimer(ID_TIME_GAME_END);
}

//初始化游戏桌子数据
void CGameProcess::InitGameTableData()
{
	//桌子最开始的状态
	m_cbGameStatus = GAME_STATUS_FREE;

	for (int i = 0; i < MAX_SIT_POS; ++i)
	{
		m_wSitList[i] = INVALID_CHAIR;
	}

	memset(&m_LuckyUserHistory, 0, sizeof(m_LuckyUserHistory));

	m_llStorageControl = 0;
	//历史记录
	memset(m_cbHistory, 0, sizeof(m_cbHistory));
	memset(m_cbWinRate, 0, sizeof(m_cbWinRate));
	m_cbHistoryCount = 0;

	m_cbLastWin = 0;
	m_cbBigRoadPosRow = 0;
	m_cbBigRoadPosCol = 0;

	//预测值
    m_cbBlackPredictBigEye = 255;
    m_cbBlackPredictSmall = 255;
    m_cbBlackPredictCockroach = 255;
    m_cbRedPredictBigEye = 255;
    m_cbRedPredictSmall = 255;
    m_cbRedPredictCockroach = 255;

    //桌子是否已经启动
	m_bIsTableRuning = false;
	m_dwTodayTime = (CT_DWORD)time(NULL);

	//保存在桌子上所有玩家ID
	memset(m_dwAllUserID, 0, sizeof(m_dwAllUserID));

	//续注
	memset(m_cbCanAddLastJetton, 0, sizeof(m_cbCanAddLastJetton));
	memset(m_llUserLastJettonScore, 0, sizeof(m_llUserLastJettonScore));
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwChairID)
{
	//洗牌
	XiPai();

	m_GameEnd.cbTimeLeave = TIME_GAME_END;

	//拷贝桌面扑克
	memcpy(m_GameEnd.cbTableCardArray, m_cbTableCardArray, sizeof(m_GameEnd.cbTableCardArray));

	//获取各区域牌型
	CT_BYTE cbCardData[CARD_AREA_COUNT][MAX_COUNT];
	memcpy(cbCardData, &m_cbTableCardArray, sizeof(cbCardData));

	CT_BYTE cbRecordCardType[CARD_AREA_COUNT]; //用于记录
	for (CT_BYTE cbArea = 0; cbArea < CARD_AREA_COUNT; ++cbArea)
	{
		m_GameLogic.SortCardList(cbCardData[cbArea], MAX_COUNT);
		m_GameEnd.cbTableCardType[cbArea] = m_GameLogic.GetCardType(cbCardData[cbArea], MAX_COUNT);
		cbRecordCardType[cbArea] = m_GameEnd.cbTableCardType[cbArea];
	}

	//玩家赢钱
	CT_LONGLONG llUserWinScore[GAME_PLAYER];
	memset(&llUserWinScore, 0, sizeof(llUserWinScore));

	//玩家输钱
	CT_LONGLONG llUserLostScore[GAME_PLAYER];
	memset(&llUserLostScore, 0, sizeof(llUserLostScore));

	//玩家赢钱要返还的本金
	CT_LONGLONG llUserReturnScore[GAME_PLAYER];
	memset(&llUserReturnScore, 0, sizeof(llUserReturnScore));

	//两个区域牌型比较
	CT_BYTE cbGoodLuckMultiple = 0;
	CT_BYTE cbResult = m_GameLogic.CompareCard(cbCardData[0], cbCardData[1], MAX_COUNT);
	CT_BYTE cbWinArea = en_Hei;
	if (cbResult == 1)
	{
		//计算幸运牌型倍数
		cbGoodLuckMultiple = m_GameLogic.GetCardMultiple(m_GameEnd.cbTableCardType[0], cbCardData[0]);

		//区域输赢
		m_GameEnd.cbAreaWin[0] = 1;
		if (cbGoodLuckMultiple != 0)
			m_GameEnd.cbAreaWin[2] = 1;

		//黑方赢
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (m_cbPlayStatus[i] == 0)
				continue;

			//黑区加钱
			llUserWinScore[i] += m_llUserJettonScore[0][i];
			llUserReturnScore[i] += m_llUserJettonScore[0][i];
			//区域输赢
			m_llUserAreaTotalLWScore[0][i] += m_llUserJettonScore[0][i];
	

			//总输钱
			llUserLostScore[i] += m_llUserJettonScore[1][i];
			//区域输赢
			m_llUserAreaTotalLWScore[1][i] -= m_llUserJettonScore[1][i];
			
			//幸运区域有没有下注
			if (m_llUserJettonScore[2][i] != 0)
			{
				if (cbGoodLuckMultiple != 0)
				{
					llUserWinScore[i] += m_llUserJettonScore[2][i]*cbGoodLuckMultiple;
					llUserReturnScore[i] += m_llUserJettonScore[2][i];

					//区域输赢
					m_llUserAreaTotalLWScore[2][i] += m_llUserJettonScore[2][i]*cbGoodLuckMultiple;
				}
				else
				{
					//总输钱
					llUserLostScore[i] += m_llUserJettonScore[2][i];
					//区域输赢
					m_llUserAreaTotalLWScore[2][i] -= m_llUserJettonScore[2][i];
				}
			}
		}

		//路单记录
		m_GameEnd.cbHistory = en_HeiWin;
		if (cbGoodLuckMultiple == 1)
		{
			//暂时用于表示大于9的对子
			m_GameEnd.cbWinCardType = CT_SPECIAL;
			cbRecordCardType[en_Hei] = CT_SPECIAL;
		}
		else
		{
			m_GameEnd.cbWinCardType = m_GameEnd.cbTableCardType[0];
		}
	}
	else
	{
		cbWinArea = en_Hong;
		//计算幸运牌型倍数
		cbGoodLuckMultiple = m_GameLogic.GetCardMultiple(m_GameEnd.cbTableCardType[1], m_GameEnd.cbTableCardArray[1]);

		//区域输赢
		m_GameEnd.cbAreaWin[1] = 1;
		if (cbGoodLuckMultiple != 0)
			m_GameEnd.cbAreaWin[2] = 1;

		//红方赢
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (m_cbPlayStatus[i] == 0)
				continue;

			//黑区输钱
			llUserLostScore[i] += m_llUserJettonScore[0][i];
			//区域输赢
			m_llUserAreaTotalLWScore[0][i] -= m_llUserJettonScore[0][i];

			//红区赢钱
			llUserWinScore[i] += m_llUserJettonScore[1][i];
			llUserReturnScore[i] += m_llUserJettonScore[1][i];
			//区域输赢
			m_llUserAreaTotalLWScore[1][i] += m_llUserJettonScore[1][i];

			//幸运区域有没有下注
			if (m_llUserJettonScore[2][i] != 0)
			{
				if (cbGoodLuckMultiple != 0)
				{
					llUserWinScore[i] += m_llUserJettonScore[2][i]*cbGoodLuckMultiple;
					llUserReturnScore[i] += m_llUserJettonScore[2][i];
					//区域输赢
					m_llUserAreaTotalLWScore[2][i] += m_llUserJettonScore[2][i]*cbGoodLuckMultiple;
				}
				else
				{
					llUserLostScore[i] += m_llUserJettonScore[2][i];
					//区域输赢
					m_llUserAreaTotalLWScore[2][i] -= m_llUserJettonScore[2][i];
				}
			}	
		}

		//路单记录
		m_GameEnd.cbHistory = en_HongWin;
		if (cbGoodLuckMultiple == 1)
		{
			//暂时用于表示大于9的对子
			m_GameEnd.cbWinCardType = CT_SPECIAL;
			cbRecordCardType[en_Hong] = CT_SPECIAL;
		}
		else
		{
			m_GameEnd.cbWinCardType = m_GameEnd.cbTableCardType[1];
		}
	}

	//路单数据
	if (m_cbHistoryCount == HISTORY_COUNT)
	{
		for (CT_BYTE cbCount = 0; cbCount < (m_cbHistoryCount - 1); ++cbCount)
		{
			m_cbHistory[cbCount] = m_cbHistory[cbCount + 1];
			m_cbWinCardType[cbCount] = m_cbWinCardType[cbCount + 1];
		}
		m_cbHistory[m_cbHistoryCount - 1] = m_GameEnd.cbHistory;
		m_cbWinCardType[m_cbHistoryCount - 1] = m_GameEnd.cbWinCardType;
	}
	else
	{
		m_cbHistory[m_cbHistoryCount] = m_GameEnd.cbHistory;
		m_cbWinCardType[m_cbHistoryCount] = m_GameEnd.cbWinCardType;
	}

	if (m_cbHistoryCount < HISTORY_COUNT)
	{
		++m_cbHistoryCount;
	}

	//这几个函数顺序执行
	//CT_DWORD dwTick1 = Utility::GetTick();
	RecordBigRoad();
	RecordBigEyeRoad(m_vecBigEyeRoad, m_vecOriginalBigHistory, false, en_HeiWin);
	RecordSmallRoad(m_vecSmallRoad, m_vecOriginalBigHistory, false, en_HeiWin);
	RecordCockroachRoad(m_vecCockroachRoad, m_vecOriginalBigHistory, false, en_HeiWin);

	//算大眼仔路预测
	auto nOriginalHistorySize = m_vecOriginalBigHistory.size();

	//预测黑方记录
	{
		std::vector<std::vector<CT_BYTE>> vecOriginalBigHistory;
		if (nOriginalHistorySize > 10)
		{
			vecOriginalBigHistory.assign(m_vecOriginalBigHistory.begin() + (nOriginalHistorySize - 5), m_vecOriginalBigHistory.end());
		}
		else
		{
			vecOriginalBigHistory.assign(m_vecOriginalBigHistory.begin(), m_vecOriginalBigHistory.end());
		}
		GenerateOriginalRecord(vecOriginalBigHistory, en_HeiWin);

		std::vector<std::vector<CT_BYTE>> vecBigEyeRoad;
		std::vector<std::vector<CT_BYTE>> vecSmallRoad;
		std::vector<std::vector<CT_BYTE>> vecCockroachRoad;

		RecordBigEyeRoad(vecBigEyeRoad, vecOriginalBigHistory, true, en_HeiWin);
		RecordSmallRoad(vecSmallRoad, vecOriginalBigHistory, true, en_HeiWin);
		RecordCockroachRoad(vecCockroachRoad, vecOriginalBigHistory, true, en_HeiWin);
	}

	//预测红方
	{
		std::vector<std::vector<CT_BYTE>> vecOriginalBigHistory;
		if (nOriginalHistorySize > 10)
		{
			vecOriginalBigHistory.assign(m_vecOriginalBigHistory.begin() + (nOriginalHistorySize - 5), m_vecOriginalBigHistory.end());
		}
		else
		{
			vecOriginalBigHistory.assign(m_vecOriginalBigHistory.begin(), m_vecOriginalBigHistory.end());
		}
		GenerateOriginalRecord(vecOriginalBigHistory, en_HongWin);

		std::vector<std::vector<CT_BYTE>> vecBigEyeRoad;
		std::vector<std::vector<CT_BYTE>> vecSmallRoad;
		std::vector<std::vector<CT_BYTE>> vecCockroachRoad;

		RecordBigEyeRoad(vecBigEyeRoad, vecOriginalBigHistory, true, en_HongWin);
		RecordSmallRoad(vecSmallRoad, vecOriginalBigHistory, true, en_HongWin);
		RecordCockroachRoad(vecCockroachRoad, vecOriginalBigHistory, true, en_HongWin);
	}

	//LOG(WARNING) << "话费时间:" << Utility::GetTick() - dwTick1;

	m_GameEnd.cbPosRow = m_cbBigRoadPosRow;
	m_GameEnd.cbPsoCol = m_cbBigRoadPosCol;

	//计算最近20局胜率
	CT_BYTE cbHeiWinCount = 0;
	CT_BYTE cbHongWinCount = 0;
	for (CT_BYTE cbHistoryCount = 0; cbHistoryCount != m_cbHistoryCount; ++cbHistoryCount)
	{
		if (m_cbHistory[cbHistoryCount] == en_HeiWin)
		{
			++cbHeiWinCount;
		}
		else
		{
			++cbHongWinCount;
		}
	}
	//黑方赢率
	m_cbWinRate[0] = cbHeiWinCount * 100 / m_cbHistoryCount;
	m_GameEnd.cbWinRate[0] = m_cbWinRate[0];
	//红方赢率
	m_cbWinRate[1] = 100 - m_cbWinRate[0]; //cbHongWinCount * 100 / m_cbHistoryCount;
	m_GameEnd.cbWinRate[1] = m_cbWinRate[1];


	CT_LONGLONG llBigWinnerScore = 0;
	CT_WORD wBigWinnerChairID = INVALID_CHAIR;
	//CT_BOOL bNeedRecord = false;
	CT_LONGLONG llRevenue[GAME_PLAYER];
	memset(&llRevenue, 0, sizeof(llRevenue));

	//结算金币
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		//没有下注的玩家
		if (0 == m_cbPlayStatus[i])
		{
			//标识下一局不可以续注
			if (m_cbCanAddLastJetton[i] != 0)
				m_cbCanAddLastJetton[i] = 0;

			if (m_pGameDeskPtr->IsExistUser(i))
			{
				m_GameEnd.dUserTotalScore[i] = m_pGameDeskPtr->GetUserScore(i)*TO_DOUBLE;
			}
		}

		if (0 == m_cbPlayStatus[i])
			continue;

		m_GameEnd.dwUserID[i] = m_pGameDeskPtr->GetUserID(i);

		//计算税收
		llRevenue[i] = m_pGameDeskPtr->CalculateRevenue(i, llUserWinScore[i]);

		//玩家总输赢，不算税收，方便计算历史上输赢记录
		m_llUserAllLWScore[i] = llUserWinScore[i] - llUserLostScore[i];
		m_mapUserLWScore[m_GameEnd.dwUserID[i]] = m_llUserAllLWScore[i];

		CT_BOOL bAndroid = m_pGameDeskPtr->IsAndroidUser(i);
		if (!bAndroid)
		{
			m_llStorageControl -= m_llUserAllLWScore[i];
			m_llTodayStorageControl -= m_llUserAllLWScore[i];
		}

		CT_LONGLONG llUserAddScore = llUserWinScore[i] + llUserReturnScore[i] - llRevenue[i];
		CT_LONGLONG llUserRealWinScore = llUserWinScore[i] - llUserLostScore[i] - llRevenue[i];
		//计算大赢家
		if (llUserWinScore[i] > 0)
		{
			if (llUserRealWinScore > llBigWinnerScore)
			{
				llBigWinnerScore = llUserRealWinScore;
				wBigWinnerChairID = i;
			}
		}

		//游戏记录的数据
		m_RecordScoreInfo[i].cbIsAndroid = bAndroid;
		m_RecordScoreInfo[i].cbStatus = 1;
		m_RecordScoreInfo[i].dwRevenue = (CT_DWORD)llRevenue[i];
		m_RecordScoreInfo[i].dwUserID = m_pGameDeskPtr->GetUserID(i);
		m_RecordScoreInfo[i].llSourceScore = m_llUserSourceScore[i];
		m_RecordScoreInfo[i].iScore = (int)(llUserWinScore[i] - llUserLostScore[i]);

		for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
		{
			m_RecordScoreInfo[i].iAreaJetton[wAreaIndex] = (CT_INT32)m_llUserJettonScore[wAreaIndex][i]; //(CT_INT32)llUserAreaLWScore[i][wAreaIndex];
			m_llUserLastJettonScore[wAreaIndex][i] = m_llUserJettonScore[wAreaIndex][i];
		}

		//if (/*m_RecordScoreInfo[i].cbIsAndroid &&*/ !bNeedRecord)
		//{
		//	bNeedRecord = true;
		//}

		//写积分
		ScoreInfo  ScoreData;
		//memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = m_pGameDeskPtr->GetUserID(i);
		ScoreData.bBroadcast = false;
		ScoreData.llScore = llUserAddScore;
		ScoreData.llRealScore = llUserRealWinScore;
		ScoreData.cbCardType = m_GameEnd.cbTableCardType[cbWinArea] >= CT_SHUN_JIN ? m_GameEnd.cbTableCardType[cbWinArea] : 255;
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);

		//百人牛牛结算金币不广播，给前端发送最后的金额以刷新界面
		m_GameEnd.dUserTotalScore[i] = m_pGameDeskPtr->GetUserScore(i)*TO_DOUBLE;

		m_cbCanAddLastJetton[i] = 1;
	}

	if (wBigWinnerChairID != INVALID_CHAIR)
	{
		m_GameEnd.cbBigWinnerHeadIndex = m_pGameDeskPtr->GetHeadIndex(wBigWinnerChairID);
		m_GameEnd.cbBigWinnerSex = m_pGameDeskPtr->GetUserSex(wBigWinnerChairID);
		m_GameEnd.dBigWinnerScore = llBigWinnerScore*TO_DOUBLE;
		_snprintf_info(m_GameEnd.szBigWinnerNickName, sizeof(m_GameEnd.szBigWinnerNickName), "%s", m_pGameDeskPtr->GetNickName(wBigWinnerChairID));
	}

	//有座玩家输赢
	for (CT_WORD i = 0; i < MAX_SIT_POS; ++i)
	{
		//0-5:对应椅子玩家
		if (m_wSitList[i] == INVALID_CHAIR) continue;

		CT_WORD wChairID = m_wSitList[i];
		m_GameEnd.dLWScore[i] = (llUserWinScore[wChairID] - llUserLostScore[wChairID] - llRevenue[wChairID])*TO_DOUBLE;
		for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
		{
			m_GameEnd.dUserAreaLWScore[i][wAreaIndex] = m_llUserAreaTotalLWScore[wAreaIndex][wChairID] * TO_DOUBLE;
		}
	}

	//无座玩空输赢
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		//6:无座玩家（包括自己）
		if (!m_pGameDeskPtr->IsExistUser(i))  continue;
		if (0 == m_cbPlayStatus[i]) continue;

		if (GetUserSitPos(i) != INVALID_CHAIR) continue;

		m_GameEnd.dLWScore[6] += (llUserWinScore[i] - llUserLostScore[i] - llRevenue[i])*TO_DOUBLE;
		for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
		{
			m_GameEnd.dUserAreaLWScore[6][wAreaIndex] += m_llUserAreaTotalLWScore[wAreaIndex][i] * TO_DOUBLE;
		}
	}

	//发送数据
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
		{
			//玩家不存在
			continue;
		}

		//没有下注，将结算状态改为没有下注
	/*	if (m_cbPlayStatus[i] == 0)
		{
			m_GameEnd.cbEndType = en_No_Bet;
		}*/

		m_GameEnd.dLWScore[7] = (llUserWinScore[i] - llUserLostScore[i] - llRevenue[i]) *TO_DOUBLE;// 7:自己
		for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
		{
			m_GameEnd.dUserAreaLWScore[7][wAreaIndex] = (m_llUserAreaTotalLWScore[wAreaIndex][i] - llRevenue[i])*TO_DOUBLE;
		}
		SendTableData(i, HHDZ_SUB_S_GAME_END, &m_GameEnd, sizeof(HHDZ_CMD_S_GameEnd), false);
	}

	//保存游戏记录
	//if (bNeedRecord)
	{
		//RecordScoreInfo* pRecordScoreInfo, CT_WORD wUserCount, CT_BYTE* pAreaCardType, CT_BYTE cbAreaCount, CT_DWORD dwBankerUserID, CT_LONGLONG llStock, CT_LONGLONG llAndroidStock);

		/*LOG(WARNING) << "hhdz total storage: " << (m_llStorageControl/100) << ", total storage lower limit:" << m_llStorageLowerLimit/100 << ", today storage: "
			<< m_llTodayStorageControl/100 << ", today high limit:" << m_llTodayStorageHighLimit/100 << ",system all kill ratio: " << m_wSystemAllKillRatio;*/
		m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, cbRecordCardType, CARD_AREA_COUNT, 0, m_llStorageControl, 0, m_llTodayStorageControl, 0, 0);
	}
}

void CGameProcess::CalUserHistory(CT_BOOL bUpdateToClient)
{
	//计算玩家的历史记录
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_pGameDeskPtr->IsExistUser(i))
		{
			CT_BYTE cbIndex = m_UserHistory[i].wPlayCount % USER_HISTORY_COUNT;

			if (m_cbPlayStatus[i] == 0)
			{
				m_UserHistory[i].llJettonScore[cbIndex] = 0;
				m_UserHistory[i].cbWinCount[cbIndex] = 0;
			}
			else
			{
				m_UserHistory[i].llJettonScore[cbIndex] = m_llUserTotalJettonScore[i];
				if (m_llUserAllLWScore[i] > 0)
				{
					m_UserHistory[i].cbWinCount[cbIndex] = 1;
				}
				else
				{
					m_UserHistory[i].cbWinCount[cbIndex] = 0;
				}
			}
			++m_UserHistory[i].wPlayCount;
		}
	}

	//重新计算排名
	SortUserHistory(bUpdateToClient);
}

//玩家历史记录排序
void CGameProcess::SortUserHistory(CT_BOOL bUpdateToClient)
{
	m_vecUserHistory.clear();
	tagUserHistoryEx historySum;
	CT_BYTE	cbMaxWinCount = 0;

	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_pGameDeskPtr->IsExistUser(i))
		{
			CT_BYTE  cbWinCount = 0;
			CT_DWORD dwJettonScore = 0;
			for (CT_BYTE cbHistoryCount = 0; cbHistoryCount < USER_HISTORY_COUNT; ++cbHistoryCount)
			{
				cbWinCount += m_UserHistory[i].cbWinCount[cbHistoryCount];
				dwJettonScore += (CT_DWORD)m_UserHistory[i].llJettonScore[cbHistoryCount];
			}
			dwJettonScore = (CT_DWORD)(dwJettonScore * TO_DOUBLE);

			historySum.cbWinCount = cbWinCount;
			historySum.dwJettonScore = dwJettonScore;
			historySum.wChairID = i;
			if (cbWinCount > cbMaxWinCount)
			{
				cbMaxWinCount = cbWinCount;
				m_LuckyUserHistory.wChairID = i;
				m_LuckyUserHistory.cbWinCount = cbWinCount;
				m_LuckyUserHistory.dwJettonScore = dwJettonScore;
			}

			m_vecUserHistory.push_back(historySum);
		}
	}

	std::sort(m_vecUserHistory.begin(), m_vecUserHistory.end(), comp_by_jetton);

	//更新座椅信息
	UpdateUserSitInfo(bUpdateToClient);
}

//处理玩家跟投消息
void CGameProcess::HandleUserFollow(CT_DWORD dwChairID, CT_BYTE cbJettonArea)
{
	//查找玩家是否在座椅上
	CT_BYTE cbFindSitId = MAX_SIT_POS;
	for (CT_BYTE cbSitId = 0;  cbSitId < MAX_SIT_POS; ++cbSitId)
	{
		if (m_wSitList[cbSitId] == dwChairID)
		{
			cbFindSitId = cbSitId;
			break;
		}
	}

	//不在座椅上
	if (cbFindSitId == MAX_SIT_POS)
	{
		return;
	}

	//查找跟投信息
	auto it = m_mapBeFollowBetInfo.find(cbFindSitId);
	if (it == m_mapBeFollowBetInfo.end())
	{
		return;
	}

	//钱不够或者跟投次数达到的玩家
	std::map<CT_WORD, CT_BYTE> mapCancelFollow;

	std::set<CT_WORD>& setFollowChairId = it->second;
	for (auto& itSet : setFollowChairId)
	{
		CT_WORD wFollowChairId = itSet;

		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wFollowChairId);
		if (llUserScore < CAN_JETTON_SCORE)
		{
			mapCancelFollow.insert(std::make_pair(wFollowChairId, en_Cancel_NoMoney));
			continue;
		}

		//找到玩家的跟投信息
		auto it = m_mapFollowBetInfo.find(wFollowChairId);
		if (it != m_mapFollowBetInfo.end())
		{
			tagUserFollowBet& userFollowInfo = it->second;
			CT_LONGLONG llFollowBet = m_llSmallJetton[userFollowInfo.cbJettonIndex];
			if (llUserScore < llFollowBet)
			{
				mapCancelFollow.insert(std::make_pair(wFollowChairId, en_Cancel_NoMoney));
				continue;
			}

			if (userFollowInfo.cbFollowThisRound == 1)
			{
				continue;
			}

			//跟注
			++userFollowInfo.cbCurrCount;
			userFollowInfo.cbFollowThisRound = 1;
			OnUserBetScore(wFollowChairId, cbJettonArea, userFollowInfo.cbJettonIndex, false);

			//更新跟投玩家的跟投次数信息
			SendUserFollowInfo(wFollowChairId, userFollowInfo);

			if (userFollowInfo.cbCurrCount == userFollowInfo.cbTotalCount)
			{
				mapCancelFollow.insert(std::make_pair(wFollowChairId, en_Cancel_AtTimes));
				continue;
			}
		}
	}

	//取消跟注的玩家
	if (!mapCancelFollow.empty())
	{
		for (auto& it : mapCancelFollow)
		{
			OnUserCancelFollowBet(it.first, it.second, true);
		}

		//更新玩家被更投信息
		UpdateUserSitUserFollowInfo(cbFindSitId);
	}
}

//跟投玩家的跟投信息
void CGameProcess::SendUserFollowInfo(CT_DWORD dwChairID, const tagUserFollowBet& userFollowInfo)
{
	HHDZ_CMD_S_FollowBetInfo followBetInfo;
	followBetInfo.cbBeFollowSitID = userFollowInfo.cbFollowSitID;
	followBetInfo.cbFollowTimes = userFollowInfo.cbTotalCount;
	followBetInfo.cbCurrentTimes = userFollowInfo.cbCurrCount;
	followBetInfo.cbFollowJettonIndex = userFollowInfo.cbJettonIndex;
	SendTableData(dwChairID, HHDZ_SUB_S_FOLLOW_INFO, &followBetInfo, sizeof(HHDZ_CMD_S_FollowBetInfo), false);
}
 
CT_WORD CGameProcess::OnUserBetScore(CT_WORD wChairID, CT_BYTE cbJettonArea, CT_WORD wJettonIndex, CT_BOOL bIsAndroid)
{
	/*LOG_IF(INFO,PRINT_GAME_INFO) << "OnUserBetScore: wChairID = " << wChairID << ",cbJettonArea = " << (int)cbJettonArea
		<< ",wJettonIndex =" << wJettonIndex <<",bIsAndroid = " << (int)bIsAndroid;*/
	if (wJettonIndex < 0  || wJettonIndex >= MAX_JETTON_NUM)
	{
		//下注筹码错误
		SendErrorCode(wChairID, ERROR_BET_SCORE);
		return 1;
	}

	if (cbJettonArea < 0 || cbJettonArea > AREA_COUNT)
	{
		//下注区域错误
		SendErrorCode(wChairID, ERROR_BET_AREA);
		return 2;
	}

	//玩家当前拥有金币
	CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);
	//玩家当前下注金币
	CT_LONGLONG llJettonScore = m_llSmallJetton[wJettonIndex];

	//校验金币是否足够下注
	if (llUserScore < llJettonScore || llUserScore < CAN_JETTON_SCORE)
	{
		SendErrorCode(wChairID, ERROR_SCORE_NOT_ENOUGH);
		return 3;
	}

	//判断是否红黑 只能下一方
	//屏蔽只能下注一方 2018-12-26
	/*if (cbJettonArea == 0)
	{
		//检测红区是否下注
		if (GetUserAreaBetScore(wChairID, 1) != 0)
		{
			SendErrorCode(wChairID, ERROR_BET_TWO_AREA);
			return 4;
		}
	}
	else if (cbJettonArea == 1)
	{
		//检测黑区是否下注
		if (GetUserAreaBetScore(wChairID, 0) != 0)
		{
			SendErrorCode(wChairID, ERROR_BET_TWO_AREA);
			return 4;
		}
	}*/

	if (m_cbPlayStatus[wChairID] == 0)
	{
		m_cbPlayStatus[wChairID] = 1;
		m_llUserSourceScore[wChairID] = m_pGameDeskPtr->GetUserScore(wChairID);
		m_dwAllUserID[wChairID] = m_pGameDeskPtr->GetUserID(wChairID);
	}

	if (!m_bHasUserAddJetton)
	{
		m_bHasUserAddJetton = true;
	}

	if (m_cbCanAddLastJetton[wChairID] == 1)
	{
		m_cbCanAddLastJetton[wChairID] = 0;
	}

	//扣除玩家下注数
	m_pGameDeskPtr->AddUserScore(wChairID, -llJettonScore, false);

	AddAreaBetScore(cbJettonArea, llJettonScore, m_pGameDeskPtr->IsAndroidUser(wChairID));
	AddUserAreaBetScore(wChairID, cbJettonArea, llJettonScore);

	//消息处理
	HHDZ_CMD_S_BetResult BetResult;
	memset(&BetResult, 0, sizeof(HHDZ_CMD_S_BetResult));
	BetResult.wChairID = wChairID;									//用户位置
	BetResult.cbJettonArea = cbJettonArea;							//下注区域
	BetResult.wJettonIndex = wJettonIndex;							//下注索引

	for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)	
	{
		BetResult.dAreaBetScore[wAreaIndex] = GetAreaBetScore(wAreaIndex)*TO_DOUBLE;//各区域总已下注金额
	}

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
		{
			//玩家不存在
			continue;
		}

		//只实时更新6个座位上的玩家和下注本人的信息
		if (i != wChairID && cbJettonArea != en_GoogLuck)
		{
			bool bFindSit = false;
			for (CT_WORD pos = 0; pos < MAX_SIT_POS; pos++)
			{
				if (wChairID == m_wSitList[pos])
				{
					bFindSit = true;
					break;
				}
			}

			if (!bFindSit)
				continue;
		}

		for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
		{
			CT_LONGLONG llUserAreaBetScore = GetUserAreaBetScore(i, wAreaIndex);
			BetResult.dUserBetScore[wAreaIndex] = llUserAreaBetScore*TO_DOUBLE;	//玩家各区域已下注金额
		}

		SendTableData(i, HHDZ_SUB_S_BET_SCORE_RESULT, &BetResult, sizeof(BetResult),false);
	}

	//HandleUserFollow(wChairID, cbJettonArea);
	return 6;
}

CT_WORD CGameProcess::OnUserBetLastScore(CT_WORD wChairID)
{
	if (m_cbCanAddLastJetton[wChairID] == 0)
	{
		SendErrorCode(wChairID, ERROR_OPERATE);
		LOG(WARNING) << "user can not bet last score, chair id:" << wChairID << ", user id: " << m_pGameDeskPtr->GetUserID(wChairID);
		return 1;
	}

	m_cbCanAddLastJetton[wChairID] = 0;

	for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
	{
		CT_LONGLONG llLastJettonScore = m_llUserLastJettonScore[wAreaIndex][wChairID];

		if (llLastJettonScore == 0)
			continue;

		 //玩家已下注金币
		// CT_LONGLONG llJettonCount = GetUserAllAreaBetScore(wChairID);

		 //玩家当前拥有金币
		 CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);

		 //玩家当前下注金币
		 CT_LONGLONG llJettonScore = llLastJettonScore;

		 //校验金币是否足够下注
		 if (llUserScore < llJettonScore || llUserScore < CAN_JETTON_SCORE)
		 {
			 SendErrorCode(wChairID, ERROR_SCORE_NOT_ENOUGH);
			 return 2;
		 }

		 if (m_cbPlayStatus[wChairID] == 0)
		 {
			 m_cbPlayStatus[wChairID] = 1;
			 m_llUserSourceScore[wChairID] = m_pGameDeskPtr->GetUserScore(wChairID);
			 m_dwAllUserID[wChairID] = m_pGameDeskPtr->GetUserID(wChairID);
		 }

		 if (!m_bHasUserAddJetton)
		 {
			 m_bHasUserAddJetton = true;
		 }

		 //扣除玩家下注数
		 m_pGameDeskPtr->AddUserScore(wChairID, -llJettonScore, false);

		 AddAreaBetScore(wAreaIndex, llJettonScore, m_pGameDeskPtr->IsAndroidUser(wChairID));
		 AddUserAreaBetScore(wChairID, wAreaIndex, llJettonScore);

		 //消息处理
		 HHDZ_CMD_S_LastBetResult BetResult;
		 memset(&BetResult, 0, sizeof(HHDZ_CMD_S_LastBetResult));
		 BetResult.wChairID = wChairID;									//用户位置
		 BetResult.cbJettonArea = (CT_BYTE)wAreaIndex;					//下注区域
		 BetResult.dJettonScore = llJettonScore*TO_DOUBLE;				//下注金额

		 for (CT_WORD wAreaIndex1 = 0; wAreaIndex1 < AREA_COUNT; ++wAreaIndex1)
		 {
			 BetResult.dAreaBetScore[wAreaIndex1] = GetAreaBetScore(wAreaIndex1)*TO_DOUBLE;//各区域总已下注金额
		 }

		 for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		 {
			 if (!m_pGameDeskPtr->IsExistUser(i))
			 {//玩家不存在
				 continue;
			 }

			 //只实时更新6个座位上的玩家和下注本人的信息
			 if (i != wChairID)
			 {
				 bool bFindSit = false;
				 for (CT_WORD pos = 0; pos < MAX_SIT_POS; pos++)
				 {
					 if (wChairID == m_wSitList[pos])
					 {
						 bFindSit = true;
						 break;
					 }
				 }

				 if (!bFindSit)
					 continue;
			 }

			 for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
			 {
				 BetResult.dUserBetScore[wAreaIndex] = GetUserAreaBetScore(i, wAreaIndex)*TO_DOUBLE;	//玩家各区域已下注金额
			 }
			 SendTableData(i, HHDZ_SUB_S_ADD_LAST_BET, &BetResult, sizeof(BetResult), false);
		 }

		 //HandleUserFollow(wChairID, (CT_BYTE)wAreaIndex);
	}

	return 3;
}

//查询玩家的近20局游戏数据
CT_BOOL CGameProcess::OnQueryUserGameHistory(CT_WORD wChairID)
{
	CT_BYTE  szBuffer[2500];
	CT_DWORD dwSendSize = 0;
	//
	HHDZ_CMD_S_UserRank history;
	history.dwUserID = m_pGameDeskPtr->GetUserID(m_LuckyUserHistory.wChairID);
	history.cbgender = m_pGameDeskPtr->GetUserSex(m_LuckyUserHistory.wChairID);
	history.cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(m_LuckyUserHistory.wChairID);
	history.cbWinCount = m_LuckyUserHistory.cbWinCount;
	history.dwTotalJetton = m_LuckyUserHistory.dwJettonScore;
	memcpy(szBuffer, &history, sizeof(HHDZ_CMD_S_UserRank));
	dwSendSize += sizeof(HHDZ_CMD_S_UserRank);

	int nCount = 0;
	for (auto& it : m_vecUserHistory)
	{
		if (it.wChairID == m_LuckyUserHistory.wChairID)
			continue;

		//如果位置上没有玩家，则不下发这个玩家的数据，因为中途不在6个宝座上的玩家退出并没有更新这个列表
		if (!m_pGameDeskPtr->IsExistUser(it.wChairID))
			continue;

		history.dwUserID = m_pGameDeskPtr->GetUserID(it.wChairID);
		history.cbgender = m_pGameDeskPtr->GetUserSex(it.wChairID);
		history.cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(it.wChairID);
		history.cbWinCount = it.cbWinCount;
		history.dwTotalJetton = it.dwJettonScore;
		memcpy(szBuffer + dwSendSize, &history, sizeof(HHDZ_CMD_S_UserRank));
		dwSendSize += sizeof(HHDZ_CMD_S_UserRank);
        ++nCount;

        if (nCount >= 100)
		{
			break;
		}
	}
	SendTableData(wChairID, HHDZ_SUB_S_QUERY_USER_RANK, szBuffer, dwSendSize);

	return true;
}

//玩家跟投
CT_BOOL CGameProcess::OnUserFollowBet(CT_WORD wChairID, CT_BYTE cbSitID, CT_BYTE cbJettonIndex, CT_BYTE cbFollowCount)
{
	//判断位置是否有玩家
	if (m_pGameDeskPtr->IsExistUser(wChairID) == false)
	{
		LOG(WARNING) << "follow bet fail, is not exist user? chair id : " << wChairID;
		return false;
	}

	//判断座椅上是否有玩家
	HHDZ_CMD_S_FollowBet followBet;
	if (cbSitID < 0 || cbSitID >= MAX_SIT_POS)
	{
		followBet.cbResult = 2;
		SendTableData(wChairID, HHDZ_SUB_S_FOLLOW_RESULT, &followBet, sizeof(followBet), false);
		return false;
	}
	if (m_wSitList[cbSitID] == INVALID_CHAIR)
	{
		followBet.cbResult = 2;
		SendTableData(wChairID, HHDZ_SUB_S_FOLLOW_RESULT, &followBet, sizeof(followBet), false);
		return false;
	}
	if (cbJettonIndex < 0 || cbJettonIndex >= MAX_JETTON_NUM)
	{
		followBet.cbResult = 2;
		SendTableData(wChairID, HHDZ_SUB_S_FOLLOW_RESULT, &followBet, sizeof(followBet), false);
		return false;
	}

	//不能跟自己
	if (m_wSitList[cbSitID] == wChairID)
	{
		followBet.cbResult = 4;
		SendTableData(wChairID, HHDZ_SUB_S_FOLLOW_RESULT, &followBet, sizeof(followBet), false);
		return false;
	}

	//判断玩家是否跟投其他座椅
	auto it = m_mapFollowBetInfo.find(wChairID);
	if (it != m_mapFollowBetInfo.end())
	{
		followBet.cbResult = 3;
		SendTableData(wChairID, HHDZ_SUB_S_FOLLOW_RESULT, &followBet, sizeof(followBet), false);
		return false;
	}

	//判断玩家是否有钱跟一把以上
	CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);
	CT_LONGLONG llFollowBet = m_llSmallJetton[cbJettonIndex];
	if (llUserScore < CAN_JETTON_SCORE || llFollowBet > llUserScore)
	{
		followBet.cbResult = 1;
		SendTableData(wChairID, HHDZ_SUB_S_FOLLOW_RESULT, &followBet, sizeof(followBet), false);
		return false;
	}

	//插入玩家跟投信息
	std::set<CT_WORD>& setFollowUser =  m_mapBeFollowBetInfo[cbSitID];
	setFollowUser.insert(wChairID);

	tagUserFollowBet followBetInfo;
	followBetInfo.cbFollowSitID = cbSitID;
	followBetInfo.cbJettonIndex = cbJettonIndex;
	followBetInfo.cbTotalCount = cbFollowCount;
	followBetInfo.cbCurrCount = 0;
	followBetInfo.cbFollowThisRound = 0;
	m_mapFollowBetInfo.insert(std::make_pair(wChairID, followBetInfo));

	//反馈给前端已经跟投成功
	followBet.cbResult = 0;
	SendTableData(wChairID, HHDZ_SUB_S_FOLLOW_RESULT, &followBet, sizeof(followBet), false);

	//更新玩家被更投信息
	UpdateUserSitUserFollowInfo(cbSitID);
	
	return true;
}

//玩家取消跟投
CT_BOOL CGameProcess::OnUserCancelFollowBet(CT_WORD wChairID, CT_BYTE cbReason, CT_BOOL bUpdateToClient /*= true*/)
{
	auto itFollow = m_mapFollowBetInfo.find(wChairID);
	if (itFollow == m_mapFollowBetInfo.end())
	{
		if (bUpdateToClient)
		{
			HHDZ_CMD_S_CancelFollowBet cancelFollow;
			cancelFollow.cbReason = cbReason;
			SendTableData(wChairID, HHDZ_SUB_S_CANCEL_FOLLOW, &cancelFollow, sizeof(cancelFollow), false);
		}
		return true;
	}

	//删除座椅被跟投信息
	tagUserFollowBet& userFollowBetInfo = itFollow->second;
	auto itBeFollow =  m_mapBeFollowBetInfo.find(userFollowBetInfo.cbFollowSitID);
	if (itBeFollow != m_mapBeFollowBetInfo.end())
	{
		std::set<CT_WORD>& setFollowUser = itBeFollow->second;
		auto itFollowUser = setFollowUser.find(wChairID);
		if (itFollowUser != setFollowUser.end())
		{
			setFollowUser.erase(wChairID);
		}
	}

	//更新玩家被更投信息
	UpdateUserSitUserFollowInfo(itFollow->second.cbFollowSitID);

	//删除玩家跟投信息
	m_mapFollowBetInfo.erase(itFollow);

	if (bUpdateToClient)
	{
		HHDZ_CMD_S_CancelFollowBet cancelFollow;
		cancelFollow.cbReason = cbReason;
		SendTableData(wChairID, HHDZ_SUB_S_CANCEL_FOLLOW, &cancelFollow, sizeof(cancelFollow), false);
	}



	return true;
}

CT_BOOL CGameProcess::SendUserSitList(CT_WORD wChairID)
{
	HHDZ_CMD_S_UserSitList userSitList;
	memset(&userSitList, 0, sizeof(userSitList));

	//座椅信息	
	for (CT_BYTE pos = 0; pos < MAX_SIT_POS; ++pos)
	{
		userSitList.wSitList[pos] = m_wSitList[pos];

		std::set<CT_WORD>& setFollowUser = m_mapBeFollowBetInfo[pos];
		userSitList.wBeFollowCount[pos] = (CT_WORD)setFollowUser.size();
	}

	//发送消息
	SendTableData(wChairID, HHDZ_SUB_S_USER_SIT_LIST, &userSitList, sizeof(userSitList), false);
	return true;
}

CT_BOOL CGameProcess::SendErrorCode(CT_WORD wChairID, enHHDZ_ErrorCode enCodeNum)
{
	HHDZ_CMD_S_ErrorCode errorCode;
	memset(&errorCode, 0, sizeof(errorCode));
	errorCode.cbLoseReason = enCodeNum;

	//发送消息
	if (wChairID == INVALID_CHAIR)
	{
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
			{//玩家不存在
				continue;
			}
			SendTableData(i, HHDZ_SUB_S_OPERATE_FAIL, &errorCode, sizeof(errorCode),false);
		}
	}
	else
	{
		SendTableData(wChairID, HHDZ_SUB_S_OPERATE_FAIL, &errorCode, sizeof(errorCode), false);
	}
	
	return true;
}

//更新座椅信息
CT_VOID CGameProcess::UpdateUserSitInfo(CT_BOOL bUpdateToClient)
{
	m_wSitList[0] = m_LuckyUserHistory.wChairID;

	CT_BYTE cbSitList = 1;
	for (auto& it : m_vecUserHistory)
	{
		if (it.wChairID == m_LuckyUserHistory.wChairID)
			continue;

		m_wSitList[cbSitList++] = it.wChairID;

		if (cbSitList > MAX_SIT_POS)
			break;
	}

	if (bUpdateToClient)
	{
		SendUserSitList(INVALID_CHAIR);
	}

	//检测座椅上是否有玩家
	for (CT_BYTE cbSitId = 0; cbSitId < MAX_SIT_POS; ++cbSitId)
	{
		if (m_wSitList[cbSitId] == INVALID_CHAIR)
		{
			std::set<CT_WORD>& setFollowUser = m_mapBeFollowBetInfo[cbSitId];
			for (auto& it : setFollowUser)
			{
				OnUserCancelFollowBet(it, en_Cancel_SitNoPlayer, true);
			}
		}
	}
}

//更新座椅的跟投信息
CT_VOID CGameProcess::UpdateUserSitUserFollowInfo(CT_BYTE cbSitId)
{
	HHDZ_CMD_S_BeFollowBetInfo beFollowInfo;
	beFollowInfo.cbSitID = cbSitId;

	std::set<CT_WORD>& setFollowUser = m_mapBeFollowBetInfo[cbSitId];
	beFollowInfo.cbBeFollowTimes = (CT_BYTE)setFollowUser.size();
	SendTableData(INVALID_CHAIR, HHDZ_SUB_S_SIT_BEFOLLOW_INFO, &beFollowInfo, sizeof(beFollowInfo));
}

CT_LONGLONG CGameProcess::GetUserAreaBetScore(CT_WORD wChairID, CT_WORD wAreaIndex)
{
	if (wAreaIndex < 0 && wAreaIndex > AREA_COUNT)
	{
		return 0;
	}
	return m_llUserJettonScore[wAreaIndex][wChairID];
}

CT_VOID CGameProcess::AddUserAreaBetScore(CT_WORD wChairID, CT_WORD wAreaIndex,CT_LONGLONG llAddScore)
{
	if (wAreaIndex < 0 && wAreaIndex > AREA_COUNT)
	{
		return;
	}
	m_llUserJettonScore[wAreaIndex][wChairID] += llAddScore;
	m_llUserTotalJettonScore[wChairID] += llAddScore;

    //增加统计定时刷新数据
    if (wAreaIndex != en_GoogLuck)
        m_llAreaTimedRefreshScore[wAreaIndex] += llAddScore;
}

CT_LONGLONG CGameProcess::GetUserAllAreaBetScore(CT_WORD wChairID)
{
	CT_LONGLONG llJettonCount = 0;
	for (CT_WORD wAreaIndex = 0; wAreaIndex < AREA_COUNT; ++wAreaIndex)
	{
		llJettonCount += GetUserAreaBetScore(wChairID, wAreaIndex);
	}
	return llJettonCount;
}

CT_LONGLONG CGameProcess::GetAreaBetScore(CT_WORD wAreaIndex)
{
	return m_llAllJettonScore[wAreaIndex];
}

CT_LONGLONG CGameProcess::GetRealUserAreaBetScore(CT_WORD wAreaIndex)
{
	return m_llRealUserJettonScore[wAreaIndex];
}

void CGameProcess::AddAreaBetScore(CT_WORD wAreaIndex,CT_LONGLONG llAddScore, CT_BOOL bAndroid)
{
	m_llAllJettonScore[wAreaIndex] += llAddScore;

	if (!bAndroid)
	{
		m_llRealUserJettonScore[wAreaIndex] += llAddScore;
	}
}

CT_WORD CGameProcess::GetUserSitPos(CT_WORD wChairID)
{
	CT_WORD wPos = INVALID_CHAIR;
	for (CT_WORD pos = 0; pos < MAX_SIT_POS; pos++)
	{
		if (wChairID != m_wSitList[pos])
		{
			continue;
		}
		wPos = pos;
		break;
	}
	return wPos;
}

CT_VOID CGameProcess::XiPai()
{	
	//m_GameLogic.RandCardList(m_cbRepertoryCard, sizeof(m_cbRepertoryCard));
	//for (CT_WORD wAreaIndex = 0; wAreaIndex < CARD_AREA_COUNT; ++wAreaIndex)
	//{
	//	//桌面牌数据
	//	memcpy(&m_cbTableCardArray[wAreaIndex], &m_cbRepertoryCard[wAreaIndex*MAX_COUNT], sizeof(CT_BYTE)*MAX_COUNT);
	//	m_cbLeftCardCount -= MAX_COUNT;
	//}

	//检测是否足够的钱赔
	CT_LONGLONG llHeiAreaTotalBet = GetRealUserAreaBetScore(en_Hei);
	CT_LONGLONG llHongAreaTotalBet = GetRealUserAreaBetScore(en_Hong);

	CT_LONGLONG llInterval = abs(llHeiAreaTotalBet - llHongAreaTotalBet);
	//计算今日库存最大变化
	CT_LONGLONG llNewTodayStorageControl = m_llTodayStorageControl - llInterval;

	//是否进入控制阶段
    bool bControl = false;
	if (m_llStorageControl < m_llStorageLowerLimit || llNewTodayStorageControl < m_llStorageLowerLimit)
	{
		bControl = true;
	}
	
	//LOG(WARNING) << "system all kill: " << m_wSystemAllKillRatio;
	//bool  bAllKill = false;
	//配牌规则
	m_GameLogic.RandCardList(m_cbRepertoryCard, sizeof(m_cbRepertoryCard));
	int nRandom = rand() % 1000;
	if (nRandom < m_wSystemAllKillRatio || bControl)
	{
		//必杀
		CT_BYTE cbCardData1[MAX_COUNT] = { 0,0,0 };
		CT_BYTE cbCardData2[MAX_COUNT] = { 0,0,0 };
		GetSanPai(cbCardData1, m_cbRepertoryCard);
		GetSanPai(cbCardData2, m_cbRepertoryCard);

		m_GameLogic.SortCardList(cbCardData1, MAX_COUNT);
		m_GameLogic.SortCardList(cbCardData2, MAX_COUNT);

		CT_BYTE cbResult = m_GameLogic.CompareCard(cbCardData1, cbCardData2, MAX_COUNT);
		if (cbResult == 1)
		{
			if (llHeiAreaTotalBet > llHongAreaTotalBet)
			{
				//杀黑
				memcpy(&m_cbTableCardArray[0], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
				memcpy(&m_cbTableCardArray[1], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
			}
			else
			{
				//杀红
				memcpy(&m_cbTableCardArray[0], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
				memcpy(&m_cbTableCardArray[1], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
			}
		}
		else
		{
			if (llHeiAreaTotalBet > llHongAreaTotalBet)
			{
				//杀黑
				memcpy(&m_cbTableCardArray[0], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
				memcpy(&m_cbTableCardArray[1], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
			}
			else
			{
				//杀红
				memcpy(&m_cbTableCardArray[0], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
				memcpy(&m_cbTableCardArray[1], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
			}
		}
		
		
	}
	else if (nRandom < (m_wSystemAllKillRatio + 150))
	{
		//特殊牌
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_int_distribution<> disCardValue(0, 9999);
		int nRandSpecial = disCardValue(gen);

		//int nRandSpecial = rand() % 10000;
		int nRandArea = rand() % CARD_AREA_COUNT;
		CT_BYTE cbCardData1[MAX_COUNT] = { 0,0,0 };
		CT_BYTE cbCardData2[MAX_COUNT] = { 0,0,0 };
		if (nRandSpecial < 668)
		{
			GetBaoZiPai(cbCardData1, m_cbRepertoryCard);
		}
		else if (nRandSpecial < (677 + 1000))
		{
			GetShunJinPai(cbCardData1, m_cbRepertoryCard);
		}
		else if (nRandSpecial < (667 + 1000 + 2500))
		{
			GetJinHuaPai(cbCardData1, m_cbRepertoryCard);
		}
		else if (nRandSpecial < (667 + 1000 + 2500 + 3333))
		{
			GetShunZiPai(cbCardData1, m_cbRepertoryCard);
		}
		else
		{
			GetDuiZiPaiMoreThan8(cbCardData1, m_cbRepertoryCard);
		}
		GetSanPai(cbCardData2, m_cbRepertoryCard);

		if (nRandArea == 0)
		{
			memcpy(&m_cbTableCardArray[0], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
			memcpy(&m_cbTableCardArray[1], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
		}
		else
		{
			memcpy(&m_cbTableCardArray[0], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
			memcpy(&m_cbTableCardArray[1], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
		}
	}
	else if (nRandom < (m_wSystemAllKillRatio + 150 + 350))
	{
		//对子
		int nRandArea = rand() % CARD_AREA_COUNT;
		CT_BYTE cbCardData1[MAX_COUNT] = { 0,0,0 };
		CT_BYTE cbCardData2[MAX_COUNT] = { 0,0,0 };
		GetDuiZiPaiLessThan9(cbCardData1, m_cbRepertoryCard);
		GetSanPai(cbCardData2, m_cbRepertoryCard);

		if (nRandArea == 0)
		{
			memcpy(&m_cbTableCardArray[0], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
			memcpy(&m_cbTableCardArray[1], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
		}
		else
		{
			memcpy(&m_cbTableCardArray[0], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
			memcpy(&m_cbTableCardArray[1], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
		}
	}
	else
	{
		//散牌
		CT_BYTE cbCardData1[MAX_COUNT] = { 0,0,0 };
		CT_BYTE cbCardData2[MAX_COUNT] = { 0,0,0 };
		GetSanPai(cbCardData1, m_cbRepertoryCard);
		GetSanPai(cbCardData2, m_cbRepertoryCard);
		memcpy(&m_cbTableCardArray[0], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
		memcpy(&m_cbTableCardArray[1], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
	}
	
	if (bControl)
		return;

	//计算黑名单人员下注金额
	std::vector<tagBlackListWinScore> vecBlackListWinScore;
	for (auto& it : m_vecBlackList)
	{
		for (int i = 0; i < GAME_PLAYER; ++i)
		{
			if (m_dwAllUserID[i] != 0 && m_dwAllUserID[i] == it.dwUserID)
			{
				//如果玩家下注额度不够，则不控制
				CT_LONGLONG llUserBetScore = GetUserAllAreaBetScore(i);
				if (llUserBetScore < it.dwControlScore)
				{
					break;
				}

				tagBlackListWinScore blackListWinScore;
				memset(&blackListWinScore, 0, sizeof(blackListWinScore));
				blackListWinScore.wChairID = i;
				
				if (m_llUserJettonScore[en_Hei][i] != 0)
				{
					blackListWinScore.llWinScore = m_llUserJettonScore[en_Hei][i];
					blackListWinScore.cbMaxJettonArea = en_Hei;
				}
				else if (m_llUserJettonScore[en_Hong][i] != 0)
				{
					blackListWinScore.llWinScore = m_llUserJettonScore[en_Hong][i];
					blackListWinScore.cbMaxJettonArea = en_Hong;
				}
				blackListWinScore.wLostRatio = it.wUserLostRatio;
				vecBlackListWinScore.push_back(blackListWinScore);
				break;
			}
		}
	}
	
	CT_BOOL bChangeBlacklistCard = false;
	if (!vecBlackListWinScore.empty())
	{
		CT_BYTE cbCardData1[MAX_COUNT] = { 0,0,0 };
		CT_BYTE cbCardData2[MAX_COUNT] = { 0,0,0 };
		memcpy(cbCardData1, &m_cbTableCardArray[0] , sizeof(CT_BYTE)*MAX_COUNT);
		memcpy(cbCardData2, &m_cbTableCardArray[1], sizeof(CT_BYTE)*MAX_COUNT);
		
		m_GameLogic.SortCardList(cbCardData1, MAX_COUNT);
		m_GameLogic.SortCardList(cbCardData2, MAX_COUNT);
		
		//CT_BYTE cbCardType1 = m_GameLogic.GetCardType(cbCardData1, MAX_COUNT);
		//CT_BYTE cbCardType2 = m_GameLogic.GetCardType(cbCardData2, MAX_COUNT);
		
		
        
        //两个区域牌型比较
        //CT_BYTE cbGoodLuckMultiple = 0;
        CT_BYTE cbWinArea = en_Hei;
        CT_BYTE cbResult = m_GameLogic.CompareCard(cbCardData1, cbCardData2, MAX_COUNT);
        if (cbResult == 1)
        {
			cbWinArea = en_Hei;
            //计算幸运牌型倍数
            /*cbGoodLuckMultiple = m_GameLogic.GetCardMultiple(cbCardType1, cbCardData1);
            
            //黑名单中黑方赢钱
            for (auto& it : vecBlackListWinScore)
            {
                if (m_cbPlayStatus[it.wChairID] == 0)
                    continue;
                
                it.llWinScore += m_llUserJettonScore[0][it.wChairID];
    
                if (cbGoodLuckMultiple != 0 && m_llUserJettonScore[2][it.wChairID] != 0)
                {
                    it.llWinScore += m_llUserJettonScore[2][it.wChairID]*cbGoodLuckMultiple;
                }
                
                it.cbMaxJettonArea = en_Hei;
            }*/
        }
        else
        {
			cbWinArea = en_Hong;
            //计算幸运牌型倍数
            /*cbGoodLuckMultiple = m_GameLogic.GetCardMultiple(cbCardType2, cbCardData2);
    
            //黑名单中黑方赢钱
            for (auto& it : vecBlackListWinScore)
            {
                if (m_cbPlayStatus[it.wChairID] == 0)
                    continue;
        
                it.llWinScore += m_llUserJettonScore[1][it.wChairID];
        
                if (cbGoodLuckMultiple != 0 && m_llUserJettonScore[2][it.wChairID] != 0)
                {
                    it.llWinScore += m_llUserJettonScore[2][it.wChairID]*cbGoodLuckMultiple;
                }
        
                it.cbMaxJettonArea = en_Hong;
            }*/
        }
        
        //按黑名单赢分排序
        std::sort(vecBlackListWinScore.begin(), vecBlackListWinScore.end(), [](tagBlackListWinScore& a, tagBlackListWinScore& b) {
          return a.llWinScore > b.llWinScore;
        });
        
        //黑名单
        if (vecBlackListWinScore[0].llWinScore > 0 && vecBlackListWinScore[0].cbMaxJettonArea == cbWinArea)
        {
            int nRandomNum = rand() % 1000;
            LOG(WARNING) << "random num: " << nRandomNum << ", lost ratio: " << vecBlackListWinScore[0].wLostRatio;
            if (nRandomNum <= vecBlackListWinScore[0].wLostRatio)
            {
                //杀黑名单人中赢分最多玩家(杀黑名单中赢钱多的人，只需要换牌就行)
                memcpy(&m_cbTableCardArray[0], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
                memcpy(&m_cbTableCardArray[1], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
            }
        }

		bChangeBlacklistCard = true;
	}
	
	//今日库存控制,放水
	if (!bChangeBlacklistCard && m_llTodayStorageControl > m_llTodayStorageHighLimit)
	{
		LOG(WARNING) << "system be kill....";
		//必杀
		//CT_LONGLONG llHeiAreaTotalBet = GetRealUserAreaBetScore(en_Hei);
		//CT_LONGLONG llHongAreaTotalBet = GetRealUserAreaBetScore(en_Hong);

		CT_BYTE cbCardData1[MAX_COUNT] = { 0,0,0 };
		CT_BYTE cbCardData2[MAX_COUNT] = { 0,0,0 };
		memcpy(cbCardData1, &m_cbTableCardArray[0] , sizeof(CT_BYTE)*MAX_COUNT);
		memcpy(cbCardData2, &m_cbTableCardArray[1], sizeof(CT_BYTE)*MAX_COUNT);

		m_GameLogic.SortCardList(cbCardData1, MAX_COUNT);
		m_GameLogic.SortCardList(cbCardData2, MAX_COUNT);

		CT_BYTE cbResult = m_GameLogic.CompareCard(cbCardData1, cbCardData2, MAX_COUNT);
		if (cbResult == 1)
		{
			if (llHeiAreaTotalBet < llHongAreaTotalBet)
			{
				//自杀黑
				memcpy(&m_cbTableCardArray[0], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
				memcpy(&m_cbTableCardArray[1], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
			}
		}
		else
		{
			if (llHeiAreaTotalBet > llHongAreaTotalBet)
			{
				//自杀红
				memcpy(&m_cbTableCardArray[0], cbCardData2, sizeof(CT_BYTE)*MAX_COUNT);
				memcpy(&m_cbTableCardArray[1], cbCardData1, sizeof(CT_BYTE)*MAX_COUNT);
			}
		}
	}

	/*static int nCount = 0;

	if (nCount < 9)
	{
		m_cbTableCardArray[0][0] = 19;
		m_cbTableCardArray[0][1] = 4;
		m_cbTableCardArray[0][2] = 5;
		m_cbTableCardArray[1][0] = 8;
		m_cbTableCardArray[1][1] = 21;
		m_cbTableCardArray[1][2] = 33;
	}
	else if (nCount < 18)
	{
		m_cbTableCardArray[0][0] = 8;
		m_cbTableCardArray[0][1] = 21;
		m_cbTableCardArray[0][2] = 33;
		m_cbTableCardArray[1][0] = 19;
		m_cbTableCardArray[1][1] = 4;
		m_cbTableCardArray[1][2] = 5;
	}
	else
	{
		m_cbTableCardArray[0][0] = 19;
		m_cbTableCardArray[0][1] = 4;
		m_cbTableCardArray[0][2] = 5;
		m_cbTableCardArray[1][0] = 8;
		m_cbTableCardArray[1][1] = 21;
		m_cbTableCardArray[1][2] = 33;
	}
	nCount++;*/
	//
	/*m_cbTableCardArray[0][0] = 19;
	m_cbTableCardArray[0][1] = 4;
	m_cbTableCardArray[0][2] = 5;
	m_cbTableCardArray[1][0] = 8;
	m_cbTableCardArray[1][1] = 21;
	m_cbTableCardArray[1][2] = 33;*/
}

//获取一手散牌
CT_VOID CGameProcess::GetSanPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(2, 14);

	CT_BYTE cbRandCardLogicValue = dis(gen);

	CT_BYTE cbLastCardColor = 0;
	CT_BYTE cbLastCardLogicValue = 0;
	CT_BYTE cbCardCount = 0;

	//找到第一张牌
	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardLogicValue = m_GameLogic.GetCardLogicValue(cbCard);
			CT_BYTE cbCardColor = m_GameLogic.GetCardColor(cbCard);

			if (cbCardLogicValue == cbRandCardLogicValue)
			{
				cbLastCardColor = cbCardColor;
				cbLastCardLogicValue = cbCardLogicValue;
				cbCardData[cbCardCount++] = cbCard;
				m_cbRepertoryCard[cbCardIndex] = 0;
				break;
			}
		}
	}
	
	//查找剩余的两张牌
	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardLogicValue = m_GameLogic.GetCardLogicValue(cbCard);

			//排除A23顺子
			if (cbLastCardLogicValue == 14 && (cbCardLogicValue == 2 || cbCardLogicValue == 3))
			{
				continue;
			}

			//排除对子
			if (cbCardLogicValue == cbLastCardLogicValue || cbCardLogicValue == cbRandCardLogicValue)
			{
				continue;
			}

			CT_BYTE cbCardColor = m_GameLogic.GetCardColor(cbCard);
			if (abs(cbCardLogicValue - cbLastCardLogicValue) > 1 && cbCardColor != cbLastCardColor)
			{
				cbLastCardColor = cbCardColor;
				cbLastCardLogicValue = cbCardLogicValue;
				cbCardData[cbCardCount++] = cbCard;
				m_cbRepertoryCard[cbCardIndex] = 0;
				if (cbCardCount >= MAX_COUNT)
				{
					break;
				}
			}
		}
	}
}

//获取对子牌(小于9)
CT_VOID CGameProcess::GetDuiZiPaiLessThan9(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	//随机产生一个
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(2, 8);

	CT_BYTE cbRandCardLogicValue = dis(gen);

	CT_BYTE cbCardCount = 0;
	CT_BOOL bFindSanPai = false;
	CT_BYTE cbDuiZiCardCount = 0;
	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardLogicValue = m_GameLogic.GetCardLogicValue(cbCard);

			if (cbRandCardLogicValue == cbCardLogicValue)
			{
				if (cbDuiZiCardCount < 2)
				{
					cbCardData[cbCardCount++] = cbCard;
					++cbDuiZiCardCount;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
			}
			else
			{
				if (bFindSanPai == false)
				{
					cbCardData[cbCardCount++] = cbCard;
					m_cbRepertoryCard[cbCardIndex] = 0;
					bFindSanPai = true;
				}
			}

			if (cbCardCount >= MAX_COUNT)
			{
				break;
			}
		}
	}
}

//获取对子牌(大于等于9)
CT_VOID CGameProcess::GetDuiZiPaiMoreThan8(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	//随机产生一个
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> dis(9, 14);

	CT_BYTE cbRandCardLogicValue = dis(gen);

	CT_BYTE cbCardCount = 0;
	CT_BOOL bFindSanPai = false;
	CT_BYTE cbDuiZiCardCount = 0;
	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardLogicValue = m_GameLogic.GetCardLogicValue(cbCard);

			if (cbRandCardLogicValue == cbCardLogicValue)
			{
				if (cbDuiZiCardCount < 2)
				{
					cbCardData[cbCardCount++] = cbCard;
					++cbDuiZiCardCount;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
			}
			else
			{
				if (bFindSanPai == false)
				{
					cbCardData[cbCardCount++] = cbCard;
					m_cbRepertoryCard[cbCardIndex] = 0;
					bFindSanPai = true;
				}
			}

			if (cbCardCount >= MAX_COUNT)
			{
				break;
			}
		}
	}
}

//获取顺子牌
CT_VOID CGameProcess::GetShunZiPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> disCardValue(1, 13);

	CT_BYTE cbRandCardValue = disCardValue(gen);
	CT_BYTE cbCardCount = 0;
	CT_BYTE cbSecondCardColor = 4;

	CT_BYTE cbCardFlag[MAX_COUNT] = { 0, 0, 0 };

	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);
			CT_BYTE cbCardColor = m_GameLogic.GetCardColor(cbCard);

			if (cbSecondCardColor != 4)
			{
				if (cbCardColor == cbSecondCardColor)
				{
					continue;
				}
			}

			if (cbRandCardValue != 13 && cbRandCardValue != 12) //Q和K 特别处理
			{
				if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[0] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
				else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[1] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[1] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
				else if (cbCardValue == (cbRandCardValue + 2) && cbCardFlag[2] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[2] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
			}
			else if (cbRandCardValue == 12)
			{
				if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[0] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
				else if (cbCardValue == (cbRandCardValue - 1) && cbCardFlag[1] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[1] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
				else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[2] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[2] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
			}
			else if (cbRandCardValue == 13)
			{
				if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[0] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
				else if (cbCardValue == (cbRandCardValue - 1) && cbCardFlag[1] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[1] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
				else if (cbCardValue == 1 && cbCardFlag[2] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[2] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;

					if (cbCardCount == 1 && cbSecondCardColor == 4)
					{
						cbSecondCardColor = cbCardColor;
					}
				}
			}

			if (cbCardCount >= MAX_COUNT)
			{
				break;
			}
		}
	}
}

//获取金花
CT_VOID CGameProcess::GetJinHuaPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	CT_BYTE cbCardCount = 0;
	//CT_BYTE cbFirstCardValue = 0;
	CT_BYTE cbFirstCardColor = 0;

	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);
			CT_BYTE cbCardColor = m_GameLogic.GetCardColor(cbCard);

			if (cbCardCount == 0)
			{
				cbCardData[cbCardCount++] = cbCard;
				//cbFirstCardValue = cbCardValue;
				cbFirstCardColor = cbCardColor;

				m_cbRepertoryCard[cbCardIndex] = 0;
			}
			else if (cbCardCount == 1)
			{

				if (cbFirstCardColor != cbCardColor)
				{
					continue;
				}

				CT_BYTE cbCardValue1 = m_GameLogic.GetCardValue(cbCardData[0]);
				if (cbCardValue != (cbCardValue1 + 1) && cbCardValue != (cbCardValue1 - 1))
				{
					cbCardData[cbCardCount++] = cbCard;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
			}
			else
			{
				if (cbFirstCardColor != cbCardColor)
				{
					continue;
				}

				//CT_BYTE cbCardValue1 = m_GameLogic.GetCardValue(cbCardData[0]);
				CT_BYTE cbCardValue2 = m_GameLogic.GetCardValue(cbCardData[1]);

				if (cbCardValue != (cbCardValue2 + 1) && cbCardValue != (cbCardValue2 - 1))
				{
					CT_BYTE cbCardDataEx[MAX_COUNT];
					memcpy(cbCardDataEx, cbCardData, MAX_COUNT);
					cbCardDataEx[cbCardCount] = cbCard;
					m_GameLogic.SortCardList(cbCardDataEx, MAX_COUNT);

					//排除QKQ顺金
					if (m_GameLogic.GetCardType(cbCardDataEx, MAX_COUNT) == CT_SHUN_JIN)
					{
						continue;
					}

					cbCardData[cbCardCount++] = cbCard;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
			}

			if (cbCardCount >= MAX_COUNT)
			{
				break;
			}
		}
	}
}

//获取顺金
CT_VOID CGameProcess::GetShunJinPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	std::random_device rd;
	std::mt19937 gen(rd());
	std::uniform_int_distribution<> disCardValue(1, 13);
	CT_BYTE cbRandCardValue = disCardValue(gen);

	CT_BYTE cbColorArray[4] = { 0x00, 0x10, 0x20, 0x30 };
	std::uniform_int_distribution<> disCardColor(0, 3);
	CT_BYTE cbRandCardColor = cbColorArray[disCardColor(gen)];

	CT_BYTE cbCardCount = 0;
	CT_BYTE cbCardFlag[MAX_COUNT] = { 0, 0, 0 };
	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);
			CT_BYTE cbCardColor = m_GameLogic.GetCardColor(cbCard);

			if (cbCardColor != cbRandCardColor)
			{
				continue;
			}

			if (cbRandCardValue != 13 && cbRandCardValue != 12) //Q和K 特别处理
			{
				if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[0] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
				else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[1] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[1] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
				else if (cbCardValue == (cbRandCardValue + 2) && cbCardFlag[2] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[2] = 1;
					m_cbRepertoryCard[cbCardIndex] = 4;
				}
			}
			else if (cbRandCardValue == 12)
			{
				if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[0] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
				else if (cbCardValue == 1 && cbCardFlag[1] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[1] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
				else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[2] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[2] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
			}
			else if (cbRandCardValue == 13)
			{
				if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[0] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
				else if (cbCardValue == (cbRandCardValue - 1) && cbCardFlag[1] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[1] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
				else if (cbCardValue == 1 && cbCardFlag[2] == 0)
				{
					cbCardData[cbCardCount++] = cbCard;
					cbCardFlag[2] = 1;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
			}

			if (cbCardCount >= MAX_COUNT)
			{
				break;
			}
		}
	}
}

//获取豹子
CT_VOID CGameProcess::GetBaoZiPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	CT_BYTE cbCardCount = 0;
	CT_BYTE cbFirstCardValue = 0;

	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);

			if (cbCardCount == 0)
			{
				cbCardData[cbCardCount++] = cbCard;
				cbFirstCardValue = cbCardValue;

				m_cbRepertoryCard[cbCardIndex] = 0;
			}
			else
			{
				if (cbCardValue == cbFirstCardValue)
				{
					cbCardData[cbCardCount++] = cbCard;
					m_cbRepertoryCard[cbCardIndex] = 0;
				}
			}

			if (cbCardCount >= MAX_COUNT)
			{
				break;
			}
		}
	}
}

//获取随机牌
CT_VOID CGameProcess::GetRandCard(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[])
{
	CT_BYTE cbCardCount = 0;
	for (CT_BYTE cbCardIndex = 0; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			//CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);

			cbCardData[cbCardCount++] = cbCard;
			m_cbRepertoryCard[cbCardIndex] = 0;

			if (cbCardCount >= MAX_COUNT)
			{
				break;
			}
		}
	}
}

CT_BOOL CGameProcess::StorageControl()
{
	return false;
}

//大路单数据
CT_VOID CGameProcess::RecordBigRoad()
{
	//大眼仔路单
	//如果和上一次的记录不一样
	if (m_cbLastWin != m_GameEnd.cbHistory)
	{
		CT_BOOL bFindVacancy = false;
		//查找是否有首位的空位留着
		for (auto pos = 0; pos != (int)m_vecBigHistory.size(); ++pos)
		{
			std::vector<CT_BYTE>& vecHistoryValue = m_vecBigHistory[pos];
			CT_BYTE cbHistoryValue = vecHistoryValue[0];
			if (cbHistoryValue == 0)
			{
				vecHistoryValue[0] = m_GameEnd.cbHistory;
				bFindVacancy = true;
				//记录位置
				m_cbBigRoadPosRow = pos;
				m_cbBigRoadPosCol = 0;

				break;
			}
		}

		//如果没有找到首位空位，则新建一行数据
		if (!bFindVacancy)
		{
			std::vector<CT_BYTE> vecRecord;
			vecRecord.resize(HISTORY_COLS_COUNT, 0);
			vecRecord[0] = m_GameEnd.cbHistory;
			m_vecBigHistory.push_back(vecRecord);

			//记录位置
			m_cbBigRoadPosRow = (CT_BYTE)(m_vecBigHistory.size() - 1);
			m_cbBigRoadPosCol = 0;
		}

		m_cbLastWin = m_GameEnd.cbHistory;

		//原始记录
		std::vector<CT_BYTE> vecOriginalRecord;
		vecOriginalRecord.push_back(m_GameEnd.cbHistory);
		m_vecOriginalBigHistory.push_back(vecOriginalRecord);
	}
	else
	{
		//原始记录
		std::vector<CT_BYTE>& vecOriginalRecord = m_vecOriginalBigHistory.back();
		vecOriginalRecord.push_back(m_GameEnd.cbHistory);

		//如果记录和上一把一样的话，则需要找到最后一个相同颜色的数值
		CT_BOOL bFindPosEndCol = false;
		CT_BOOL bFindPosEndRow = false;
		//CT_BOOL bInsert = false;

		std::vector<CT_BYTE>& vecValue = m_vecBigHistory[m_cbBigRoadPosRow];
		//CT_BYTE cbHistoryValue = vecValue[m_cbBigRoadPosCol];

		if (m_cbBigRoadPosRow == (m_vecBigHistory.size() - 1))
		{
			bFindPosEndRow = true;
		}

		CT_BYTE cbPosEndCol = 0; //用于下一行的时候，能连接上上一行的相同位置
		//如果是最后一个元素
		if (m_cbBigRoadPosCol == (vecValue.size() - 1))
		{
			bFindPosEndCol = true;
			cbPosEndCol = (CT_BYTE)m_cbBigRoadPosCol;
		}
		else
		{
			if (vecValue[m_cbBigRoadPosCol + 1] != 0)
			{
				bFindPosEndCol = true;
				cbPosEndCol = (CT_BYTE)m_cbBigRoadPosCol;
			}
		}

		//后面还有空位，则插入到后面一个位置
		if (bFindPosEndCol == false)
		{
			vecValue[m_cbBigRoadPosCol + 1] = m_GameEnd.cbHistory;

			//记录位置
			m_cbBigRoadPosCol = (CT_BYTE)m_cbBigRoadPosCol + 1;
		}
		else
		{
			//如果是最后一行，则新建一行数据
			if (bFindPosEndRow == true)
			{
				std::vector<CT_BYTE> vecRecord;
				vecRecord.resize(HISTORY_COLS_COUNT, 0);
				vecRecord[cbPosEndCol] = m_GameEnd.cbHistory;
				m_vecBigHistory.push_back(vecRecord);

				//记录位置
				m_cbBigRoadPosRow = (CT_BYTE)(m_vecBigHistory.size() - 1);
				m_cbBigRoadPosCol = cbPosEndCol;
			}
			else
			{
				//如果不是最后一行,则需要从下一行到找到最后一个空的位置插入
				auto posInsert = m_cbBigRoadPosRow + 1;
				std::vector<CT_BYTE>& vecInserValue = m_vecBigHistory[posInsert];
				for (auto itInsert = vecInserValue.rbegin(); itInsert != vecInserValue.rend(); ++itInsert)
				{
					if (*itInsert == 0)
					{
						*itInsert = m_GameEnd.cbHistory;

						//记录位置
						m_cbBigRoadPosRow = (CT_BYTE)posInsert;
						m_cbBigRoadPosCol = (CT_BYTE)(std::distance(itInsert, vecInserValue.rend()) - 1);
						break;
					}
				}
			}
		}
	}

	//大眼仔路单行数量如果超过20行，则删除第一行
	if (m_vecBigHistory.size() > HISTORY_ROW_COUNT)
	{
		m_vecBigHistory.erase(m_vecBigHistory.begin());
		m_GameEnd.cbRemoveRow = 1;
		//行号减1
		m_cbBigRoadPosRow -= 1;
	}

	//原始数据大于32行的时候去清空数据
	if (m_vecOriginalBigHistory.size() > HISTORY_ROW_COUNT*HISTORY_COLS_COUNT)
	{
		m_vecOriginalBigHistory.erase(m_vecOriginalBigHistory.begin());
	}

}

//大眼仔数据
CT_VOID CGameProcess::RecordBigEyeRoad(std::vector<std::vector<CT_BYTE>>& vecBigEyeRoad, std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, bool bCalPredict, CT_BYTE cbWinArea)
{
	/**************************************************************
	以第二行第二列起头，或以第3列第1行起头
	齐脚：每列第一行比较前列、前前列是否齐脚，齐脚划红，不齐脚划蓝
	碰点：每列第二行后比较前列是否有点，有点划红，没点划蓝
	重复：没碰点后的下一行是否与前一行相同，相同为红
	**************************************************************/

	vecBigEyeRoad.clear();

	CT_BYTE cbCurRow = 0;
	CT_BYTE cbCurCol = 0;
	CT_BYTE cbLastRecord = 0;
	CT_BYTE cbCurrRecord = 0;
	for (auto i = 1; i < (int)vecOriginalBigHistory.size(); ++i)
	{
		std::vector<CT_BYTE>& vecRecord = vecOriginalBigHistory[i];
		std::vector<CT_BYTE>& vecPreColRecord = vecOriginalBigHistory[i-1];

		if (i == 1)
		{
			//第二行第二列开始
			for (auto j = 1; j < (int)vecRecord.size(); ++j)
			{
				/*
				碰点：每列第二行后比较前列是否有点，有点划红，没点划蓝
				重复：没碰点后的下一行是否与前一行相同，相同为红
				*/

				//这里只有当前一列第一个没有点的地方是蓝，其他都为红
				if (j == (int)vecPreColRecord.size())
				{
					cbCurrRecord = en_BluePoint;
				}
				else
				{
					cbCurrRecord = en_HongPoint;
				}
				GenerateRoadData(vecBigEyeRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
			}
		}
		else
		{
			std::vector<CT_BYTE>& vecPrePreColRecord = vecOriginalBigHistory[i - 2];
			for (auto j = 0; j < (int)vecRecord.size(); ++j)
			{
				//第一行判断前列和前前列是否齐头
				if (j == 0)
				{
					if (vecPrePreColRecord.size() == vecPreColRecord.size())
					{
						cbCurrRecord = en_HongPoint;
					}
					else
					{
						cbCurrRecord = en_BluePoint;
					}
					GenerateRoadData(vecBigEyeRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
					continue;
				}

				/*
				碰点：每列第二行后比较前列是否有点，有点划红，没点划蓝
				重复：没碰点后的下一行是否与前一行相同，相同为红
				*/
				if (j == (int)vecPreColRecord.size())
				{
					cbCurrRecord = en_BluePoint;
				}
				else
				{
					cbCurrRecord = en_HongPoint;
				}
				GenerateRoadData(vecBigEyeRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
			}
		}
	}

	if (!bCalPredict)
	{
		//大眼仔路单行数量如果超过HISTORY_ROW_COUNT行，则删除多余行
		if (vecBigEyeRoad.size() > HISTORY_ROW_COUNT)
		{
			if (cbCurCol == 0 && vecBigEyeRoad[cbCurRow][HISTORY_COLS_COUNT -1] == 0)
				m_GameEnd.cbRemoveBigEyeRow = 1;
			else if (cbCurCol == (HISTORY_COLS_COUNT-1) && vecBigEyeRoad[cbCurRow][0] == 0)
				m_GameEnd.cbRemoveBigEyeRow = 1;

			auto nEraseCount = vecBigEyeRoad.size() - HISTORY_ROW_COUNT;
			vecBigEyeRoad.erase(vecBigEyeRoad.begin(), vecBigEyeRoad.begin() + nEraseCount);

			cbCurRow -= (CT_BYTE)nEraseCount;
		}

		if (!vecBigEyeRoad.empty())
		{
			m_GameEnd.cbBigEyePosCol = cbCurCol;
			m_GameEnd.cbBigEyePosRow = cbCurRow;
			m_GameEnd.cbBigEyeColor = vecBigEyeRoad[cbCurRow][cbCurCol];
		} else
		{
			m_GameEnd.cbBigEyePosCol = 255;
			m_GameEnd.cbBigEyePosRow = 255;
		}
	}
	else
	{
		if (!vecBigEyeRoad.empty())
		{
			if (cbWinArea == en_HeiWin)
			{
				m_cbBlackPredictBigEye = vecBigEyeRoad[cbCurRow][cbCurCol];
				m_GameEnd.cbBlackPredictBigEye = m_cbBlackPredictBigEye;
			}
			else
			{
				m_cbRedPredictBigEye = vecBigEyeRoad[cbCurRow][cbCurCol];
				m_GameEnd.cbRedPredictBigEye = m_cbRedPredictBigEye;

			}
		}
		else
		{
			if (cbWinArea == en_HeiWin)
			{
				m_GameEnd.cbBlackPredictBigEye = 255;
			}
			else
			{
				m_GameEnd.cbRedPredictBigEye = 255;
			}
		}
	}
}

//小路数据
CT_VOID CGameProcess::RecordSmallRoad(std::vector<std::vector<CT_BYTE>>& vecSmallRoad, std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, bool bCalPredict, CT_BYTE cbWinArea)
{
	/**************************************************************
	以3列2行起头，或以4列1行起头
	齐脚：每列第1行比较前列、前前前列是否齐脚，齐脚划红，不齐脚划蓝
	碰点：每列第2行后比较前前列是否有点，有点划红，没点划蓝
	重复：没碰点后的下一行是否与前一行相同，相同为红
	**************************************************************/

	vecSmallRoad.clear();

	CT_BYTE cbCurRow = 0;
	CT_BYTE cbCurCol = 0;
	CT_BYTE cbLastRecord = 0;
	CT_BYTE cbCurrRecord = 0;
	for (auto i = 2; i < (int)vecOriginalBigHistory.size(); ++i)
	{
		std::vector<CT_BYTE>& vecRecord = vecOriginalBigHistory[i];
		std::vector<CT_BYTE>& vecPrePreColRecord = vecOriginalBigHistory[i - 2];

		if (i == 2)
		{
			//第三行第二列开始
			for (auto j = 1; j < (int)vecRecord.size(); ++j)
			{
				/*
				碰点：每列第2行后比较前前列是否有点，有点划红，没点划蓝
				重复：没碰点后的下一行是否与前一行相同，相同为红
				*/

				//这里只有当前一列第一个没有点的地方是蓝，其他都为红
				if (j == (int)vecPrePreColRecord.size())
				{
					cbCurrRecord = en_BluePoint;
				}
				else
				{
					cbCurrRecord = en_HongPoint;
				}
				GenerateRoadData(vecSmallRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
			}
		}
		else
		{
			std::vector<CT_BYTE>& vecPreColRecord = vecOriginalBigHistory[i - 1];
			std::vector<CT_BYTE>& vecPrePrePreColRecord = vecOriginalBigHistory[i - 3];
			for (auto j = 0; j < (int)vecRecord.size(); ++j)
			{
				//第一行判断前列和前前前列是否齐头
				if (j == 0)
				{
					if (vecPreColRecord.size() == vecPrePrePreColRecord.size())
					{
						cbCurrRecord = en_HongPoint;
					}
					else
					{
						cbCurrRecord = en_BluePoint;
					}
					GenerateRoadData(vecSmallRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
					continue;
				}

				/*
				碰点：每列第二行后比较前前列是否有点，有点划红，没点划蓝
				重复：没碰点后的下一行是否与前一行相同，相同为红
				*/
				if (j == (int)vecPrePreColRecord.size())
				{
					cbCurrRecord = en_BluePoint;
				}
				else
				{
					cbCurrRecord = en_HongPoint;
				}
				GenerateRoadData(vecSmallRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
			}
		}
	}

	if (!bCalPredict)
	{
		//小路路单行数量如果超过HISTORY_ROW_COUNT行，则删除多余行
		if (vecSmallRoad.size() > HISTORY_ROW_COUNT)
		{
			if (cbCurCol == 0 && vecSmallRoad[cbCurRow][HISTORY_COLS_COUNT -1] == 0)
				m_GameEnd.cbRemoveSmallRow = 1;
			else if (cbCurCol == (HISTORY_COLS_COUNT-1) && vecSmallRoad[cbCurRow][0] == 0)
				m_GameEnd.cbRemoveSmallRow = 1;

			auto nEraseCount = vecSmallRoad.size() - HISTORY_ROW_COUNT;
			vecSmallRoad.erase(vecSmallRoad.begin(), vecSmallRoad.begin() + nEraseCount);

			cbCurRow -= (CT_BYTE)nEraseCount;
		}

		if (!vecSmallRoad.empty())
		{
			m_GameEnd.cbSmallPosCol = cbCurCol;
			m_GameEnd.cbSmallPosRow = cbCurRow;
			m_GameEnd.cbSmallColor = vecSmallRoad[cbCurRow][cbCurCol];
		} else
		{
			m_GameEnd.cbSmallPosCol = 255;
			m_GameEnd.cbSmallPosRow = 255;
		}
	}
	else
	{
		if (!vecSmallRoad.empty())
		{
			if (cbWinArea == en_HeiWin)
			{
				m_cbBlackPredictSmall = vecSmallRoad[cbCurRow][cbCurCol];
				m_GameEnd.cbBlackPredictSmall = m_cbBlackPredictSmall;
			}
			else
			{
				m_cbRedPredictSmall = vecSmallRoad[cbCurRow][cbCurCol];
				m_GameEnd.cbRedPredictSmall = m_cbRedPredictSmall;

			}
		}
		else
		{
			if (cbWinArea == en_HeiWin)
			{
				m_GameEnd.cbBlackPredictSmall = 255;
			}
			else
			{
				m_GameEnd.cbRedPredictSmall = 255;
			}
		}
	}
}

//曱甴路数据
CT_VOID CGameProcess::RecordCockroachRoad(std::vector<std::vector<CT_BYTE>>& vecCockroachRoad,  std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, bool bCalPredict, CT_BYTE cbWinArea)
{
	/**************************************************************
	以4列2行起头，或以5列1行起头
	齐脚：每列第1行比较前列、前前前前列是否齐脚，齐脚划红，不齐脚划蓝
	碰点：每列第2行后比较前前前列是否有点，有点划红，没点划蓝
	重复：没碰点后的下一行是否与前一行相同，相同为红
	**************************************************************/

    vecCockroachRoad.clear();

	CT_BYTE cbCurRow = 0;
	CT_BYTE cbCurCol = 0;
	CT_BYTE cbLastRecord = 0;
	CT_BYTE cbCurrRecord = 0;
	for (auto i = 3; i < (int)vecOriginalBigHistory.size(); ++i)
	{
		std::vector<CT_BYTE>& vecRecord = vecOriginalBigHistory[i];
      std::vector<CT_BYTE>& vecPrePrePreColRecord = vecOriginalBigHistory[i - 3];

		if (i == 3)
		{
			//第四行第二列开始
			for (auto j = 1; j < (int)vecRecord.size(); ++j)
			{
				/*
				碰点：每列第2行后比较前前前列是否有点，有点划红，没点划蓝
				重复：没碰点后的下一行是否与前一行相同，相同为红
				*/

				//这里只有当前前前列第一个没有点的地方是蓝，其他都为红
				if (j == (int)vecPrePrePreColRecord.size())
				{
					cbCurrRecord = en_BluePoint;
				}
				else
				{
					cbCurrRecord = en_HongPoint;
				}
				GenerateRoadData(vecCockroachRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
			}
		}
		else
		{
            std::vector<CT_BYTE>& vecPreColRecord = vecOriginalBigHistory[i - 1];
			std::vector<CT_BYTE>& vecPrePrePrePreColRecord = vecOriginalBigHistory[i - 4];
			for (auto j = 0; j < (int)vecRecord.size(); ++j)
			{
				//第一行判断前列和前前前前列是否齐头
				if (j == 0)
				{
					if (vecPrePrePrePreColRecord.size() == vecPreColRecord.size())
					{
						cbCurrRecord = en_HongPoint;
					}
					else
					{
						cbCurrRecord = en_BluePoint;
					}
					GenerateRoadData(vecCockroachRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
					continue;
				}

				/*
				碰点：每列第2行后比较前前前列是否有点，有点划红，没点划蓝
				重复：没碰点后的下一行是否与前一行相同，相同为红
				*/
				if (j == (int)vecPrePrePreColRecord.size())
				{
					cbCurrRecord = en_BluePoint;
				}
				else
				{
					cbCurrRecord = en_HongPoint;
				}
				GenerateRoadData(vecCockroachRoad, cbLastRecord, cbCurrRecord, cbCurRow, cbCurCol);
			}
		}
	}

	if (!bCalPredict)
	{
		//蟑螂路单行数量如果超过20行，则删除多余行
		if (vecCockroachRoad.size() > HISTORY_ROW_COUNT)
		{
			if (cbCurCol == 0 && vecCockroachRoad[cbCurRow][HISTORY_COLS_COUNT -1] == 0)
				m_GameEnd.cbRemoveCockroachRow = 1;
			else if (cbCurCol == (HISTORY_COLS_COUNT-1) && vecCockroachRoad[cbCurRow][0] == 0)
				m_GameEnd.cbRemoveCockroachRow = 1;

			auto nEraseCount = vecCockroachRoad.size() - HISTORY_ROW_COUNT;
			vecCockroachRoad.erase(vecCockroachRoad.begin(), vecCockroachRoad.begin() + nEraseCount);

			cbCurRow -= (CT_BYTE)nEraseCount;
		}


		if (!vecCockroachRoad.empty())
		{
			m_GameEnd.cbCockroachPosCol = cbCurCol;
			m_GameEnd.cbCockroachPosRow = cbCurRow;
			m_GameEnd.cbCockroachColor = vecCockroachRoad[cbCurRow][cbCurCol];
		} else
		{
			m_GameEnd.cbCockroachPosCol = 255;
			m_GameEnd.cbCockroachPosRow = 255;
		}
	}
	else
	{
		if (!vecCockroachRoad.empty())
		{
			if (cbWinArea == en_HeiWin)
			{
				m_cbBlackPredictCockroach = vecCockroachRoad[cbCurRow][cbCurCol];
				m_GameEnd.cbBlackPredictCockroach = m_cbBlackPredictCockroach;
			}
			else
			{
				m_cbRedPredictCockroach = vecCockroachRoad[cbCurRow][cbCurCol];
				m_GameEnd.cbRedPredictCockroach = m_cbRedPredictCockroach;

			}
		}
		else
		{
			if (cbWinArea == en_HeiWin)
			{
				m_GameEnd.cbBlackPredictCockroach = 255;
			}
			else
			{
				m_GameEnd.cbRedPredictCockroach = 255;
			}
		}
	}
}

CT_VOID CGameProcess::GenerateRoadData(std::vector<std::vector<CT_BYTE>>& vecRoad, CT_BYTE& cbLastRecord, CT_BYTE cbCurrentRecord, CT_BYTE& cbCurrRow, CT_BYTE& cbCurrCol)
{
	//如果和上一次的记录不一样
	if (cbLastRecord != cbCurrentRecord)
	{
		CT_BOOL bFindVacancy = false;
		//查找是否有首位的空位留着
		for (auto pos = 0; pos != (int)vecRoad.size(); ++pos)
		{
			std::vector<CT_BYTE>& vecHistoryValue = vecRoad[pos];
			CT_BYTE cbHistoryValue = vecHistoryValue[0];
			if (cbHistoryValue == 0)
			{
				vecHistoryValue[0] = cbCurrentRecord;
				bFindVacancy = true;
				//记录位置
				cbCurrRow = pos;
				cbCurrCol = 0;

				break;
			}
		}

		//如果没有找到首位空位，则新建一行数据
		if (!bFindVacancy)
		{
			std::vector<CT_BYTE> vecRecord;
			vecRecord.resize(HISTORY_COLS_COUNT, 0);
			vecRecord[0] = cbCurrentRecord;
			vecRoad.push_back(vecRecord);

			//记录位置
			cbCurrRow = (CT_BYTE)(vecRoad.size() - 1);
			cbCurrCol = 0;
		}

		cbLastRecord = cbCurrentRecord;
	}
	else
	{
		//如果记录和上一把一样的话，则需要找到最后一个相同颜色的数值
		CT_BOOL bFindPosEndCol = false;
		CT_BOOL bFindPosEndRow = false;

		std::vector<CT_BYTE>& vecValue = vecRoad[cbCurrRow];

		if (cbCurrRow == (vecRoad.size() - 1))
		{
			bFindPosEndRow = true;
		}

		CT_BYTE cbPosEndCol = 0; //用于下一行的时候，能连接上上一行的相同位置
		//如果是最后一个元素
		if (cbCurrCol == (vecValue.size() - 1))
		{
			bFindPosEndCol = true;
			cbPosEndCol = cbCurrCol;
		}
		else
		{
			if (vecValue[cbCurrCol + 1] != 0)
			{
				bFindPosEndCol = true;
				cbPosEndCol = cbCurrCol;
			}
		}

		//后面还有空位，则插入到后面一个位置
		if (bFindPosEndCol == false)
		{
			vecValue[cbCurrCol + 1] = cbCurrentRecord;

			//记录位置
			cbCurrCol = cbCurrCol + 1;
		}
		else
		{
			//如果是最后一行，则新建一行数据
			if (bFindPosEndRow == true)
			{
				std::vector<CT_BYTE> vecRecord;
				vecRecord.resize(HISTORY_COLS_COUNT, 0);
				vecRecord[cbPosEndCol] = cbCurrentRecord;
				vecRoad.push_back(vecRecord);

				//记录位置
				cbCurrRow = (CT_BYTE)(vecRoad.size() - 1);
				cbCurrCol = cbPosEndCol;
			}
			else
			{
				//如果不是最后一行,则需要从下一行到找到最后一个空的位置插入
				auto posInsert = cbCurrRow + 1;
				std::vector<CT_BYTE>& vecInserValue = vecRoad[posInsert];
				for (auto itInsert = vecInserValue.rbegin(); itInsert != vecInserValue.rend(); ++itInsert)
				{
					if (*itInsert == 0)
					{
						*itInsert = cbCurrentRecord;

						//记录位置
						cbCurrRow = posInsert;
						cbCurrCol = (CT_BYTE)(std::distance(itInsert, vecInserValue.rend()) - 1);
						break;
					}
				}
			}
		}
	}
}

CT_VOID CGameProcess::GenerateOriginalRecord(std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, CT_BYTE winArea)
{
	//原始记录
	if (m_cbLastWin != winArea)
	{
		std::vector<CT_BYTE> vecOriginalRecord;
		vecOriginalRecord.push_back(winArea);
		vecOriginalBigHistory.push_back(vecOriginalRecord);
	}
	else
	{
		std::vector<CT_BYTE>& vecOriginalRecord = vecOriginalBigHistory.back();
		vecOriginalRecord.push_back(m_GameEnd.cbHistory);
	}
}