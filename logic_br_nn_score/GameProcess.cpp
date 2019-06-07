
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
#include "GlobalEnum.h"
#include <queue>
#include <set>

//打印日志
#define PRINT_GAME_INFO					1
#define PRINT_GAME_WARNING				1


////////////////////////////////////////////////////////////////////////
//定时器
#define ID_TIME_FREE					100					//空闲定时器
#define ID_TIME_SEND_CARD				101					//发牌定时器
#define ID_TIME_BET_SCORE				102					//下注定时器 
#define ID_TIME_GAME_END				103					//结束定时器 
#define	ID_TIME_PING					200					//ping值定时器

//定时器时间
#define TIME_FREE						2					//空闲时间
#define TIME_SENDCARD					3					//发牌时间
#define TIME_BET						15					//下注时间

#define TIME_GAME_END					10					//结束时间 (有人下注有大赢家)
#define TIME_GAME_END_NO_BET			5					//结束时间(无人下注)
#define TIME_GAME_END_NO_BIGWINER		10					//结束时间(有人下注无大赢家)

#define TIME_PING						5000

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
#define CAN_JETTON_SCORE 5000 //至少需要五十块钱才能下注

CGameProcess::CGameProcess(void) : m_dwCellScore(1), m_pGameDeskPtr(NULL)
{
	//初始化桌子数据
	InitGameTableData();
	//初始化数据
	InitGameData();
	//清理游戏数据
	ClearGameData();
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
	//更新游戏配置
	UpdateGameConfig();

	//检查是否跨天
	CT_DWORD dwTimeNow = (CT_DWORD)time(NULL);
	CT_BOOL bAcrossDay = Utility::isAcrossTheDay(m_dwTodayTime, dwTimeNow);
	if (bAcrossDay)
	{
		m_llTodayStorageControl = 0;
		m_dwTodayTime = dwTimeNow;
	}

	//洗牌
	XiPai();

	CT_LONGLONG llBankerScore = GetBankerScore();
	m_llBankerSourceScore = llBankerScore;

	//控牌操作（目前这个函数要在下发牌数据之前执行）
	StorageControl();
    CalTenPointArea();

	//发送开始消息
	NN_CMD_S_GameStart GameStart;
	memset(&GameStart, 0, sizeof(NN_CMD_S_GameStart));
	GameStart.cbTimeLeave = TIME_SENDCARD;							//剩余时间
	//GameStart.dStorageCount = m_llCaiJinChiCount*TO_DOUBLE;		//库存数值
	GameStart.cbBankerCount = (CT_BYTE)m_wBankerCount;				//当前庄家当庄次数
	GameStart.wBankerUser = m_wBankerUser;							//庄家
	GameStart.dBankerScore = llBankerScore*TO_DOUBLE;				//庄家金币
	//GameStart.dBetScoreLimit = (llBankerScore / MAX_TYPE_MULTIPLE)*TO_DOUBLE;	//下注金额限制
	m_wGameStatus = GAME_STATUS_SENDCARD;

	for (CT_BYTE cbArea = 0; cbArea < AREA_COUNT + 1; cbArea++)
	{
		memcpy(GameStart.cbTableCardArray[cbArea], m_cbTableCardArray[cbArea], FIRST_CARD);
	}

	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
		{
			//玩家不存在

			//清空玩家输赢记录
			//memset(&m_UserHistory[i], 0, sizeof(m_UserHistory[i]));

			continue;
		}

		m_llUserSourceScore[i] = m_pGameDeskPtr->GetUserScore(i);

		//发送数据
		SendTableData(i, NN_SUB_S_GAME_START, &GameStart, sizeof(NN_CMD_S_GameStart), false);

		if (m_pGameDeskPtr->IsAndroidUser(i))
		{
			SendTableData(i, NN_SUB_S_FLASH_AREA, &m_FlashArea, sizeof(NN_CMD_S_Game_FlashArea), false);
		}
	}

	//m_pGameDeskPtr->SetGameTimer(ID_TIME_PING, TIME_PING);
	/*//打印初始牌型
	for (CT_WORD i = 0; i < 5; ++i)
	{
		CT_WORD wtype = m_GameLogic.GetCardType(m_cbTableCardArray[i], 5);
		LOG_IF(INFO,PRINT_GAME_INFO) << " area = " << i << ",type = " << wtype;
	}*/
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
        BlackListControl();
        StoreageControlChangeAreaCard();
		ChangeAllKillAreaCard();
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
	switch (m_wGameStatus)
	{
	case GAME_STATUS_FREE:			//空闲状态
	{
		//发送消息
		NN_CMD_S_StatusFree GameFree;
		//memset(&GameFree,0, sizeof(GameFree));

		CT_DWORD dwTimeLeave = m_dwFreeTime - (CT_DWORD)time(NULL);
		if (dwTimeLeave == 0 || dwTimeLeave > TIME_FREE)
		{
			dwTimeLeave = 1;
		}
		//LOG_IF(INFO,PRINT_GAME_INFO) <<"OnEventGameScene:free dwChairID = "<< dwChairID << ",dwTimeLeave = " << dwTimeLeave;

		GameFree.cbTimeLeave = dwTimeLeave;									//剩余时间
		//GameFree.dStorageCount = m_llCaiJinChiCount*TO_DOUBLE;			//库存数值
		GameFree.wBankerUser = m_wBankerUser;								//当前庄家
		GameFree.dBankerScore = GetBankerScore()*TO_DOUBLE;					//庄家金币
		GameFree.dBankerWinScore = m_llBankerWinScore*TO_DOUBLE;			//庄家成绩
		GameFree.dBankerTotalWinScore = m_llBankerTotalWinScore*TO_DOUBLE;	//庄家总成绩
		GameFree.cbBankCurrCount = (CT_BYTE)m_wBankerCount;					//庄家当前次数
		GameFree.cbBankTotalCount = MAX_BANKER_COUNT;						//最大做庄次数
		GameFree.dApplyBankerCondition = m_llApplyBankerCondition*TO_DOUBLE;//申请庄家条件
		GameFree.dUserSitLimitScore = SIT_SCORE_LIMIT*TO_DOUBLE;			//玩家坐下金币限制

		//总下注限制，不与玩家自身金币比较
		CT_LONGLONG llAreaLimitScore = 0;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			llAreaLimitScore += GetAreaBetScore(wAreaIndex);
		}
		GameFree.dAreaLimitScore = (GetBankerScore() / m_wMaxCardMulTiple - llAreaLimitScore)*TO_DOUBLE;

		//玩家还可下注金币限制
		CT_LONGLONG llJettonCount = GetUserAllAreaBetScore(dwChairID);
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(dwChairID);
		CT_LONGLONG llUserLimitScore = llUserScore / m_wMaxCardMulTiple;
		GameFree.dUserMaxScore = (llUserLimitScore - llJettonCount)*TO_DOUBLE;

		SendTableData(dwChairID, SC_GAMESCENE_FREE, &GameFree, sizeof(NN_CMD_S_StatusFree), false);
		//同步椅子信息
		SendUserSitList(dwChairID);
		//SendCaiJinChiChampionInfo(dwChairID);
		break;
	}
	case GAME_STATUS_SENDCARD:
	{
		NN_CMD_S_StatusSendCard StatusSendCard;
		CT_DWORD dwTime = (CT_DWORD)time(NULL) - m_dwSendCardTime;
		dwTime = dwTime < TIME_SENDCARD ? (TIME_SENDCARD - dwTime) : 0;
		StatusSendCard.cbTimeLeave = (CT_BYTE)dwTime;								//剩余时间
		StatusSendCard.cbTotalTime = TIME_SENDCARD;									//总时间
		StatusSendCard.wBankerUser = m_wBankerUser;									//当前庄家
		StatusSendCard.dBankerScore = GetBankerScore()*TO_DOUBLE;					//庄家金币
		StatusSendCard.dBankerWinScore = m_llBankerWinScore*TO_DOUBLE;				//庄家成绩
		StatusSendCard.dBankerTotalWinScore = m_llBankerTotalWinScore*TO_DOUBLE;	//庄家总成绩
		StatusSendCard.cbBankCurrCount = (CT_BYTE)m_wBankerCount;					//庄家当前次数
		StatusSendCard.cbBankTotalCount = MAX_BANKER_COUNT;							//最大做庄次数
		StatusSendCard.dApplyBankerCondition = m_llApplyBankerCondition*TO_DOUBLE;	//申请庄家条件

		//总下注限制，不与玩家自身金币比较
		CT_LONGLONG llAreaLimitScore = 0;
		for (CT_BYTE wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			llAreaLimitScore += GetAreaBetScore(wAreaIndex);
		}

		for (CT_BYTE cbAreaIndex = 0; cbAreaIndex != AREA_COUNT + 1; ++ cbAreaIndex)
		{
			memcpy(StatusSendCard.cbTableCardArray[cbAreaIndex], m_cbTableCardArray[cbAreaIndex], FIRST_CARD);
		}

		StatusSendCard.dAreaLimitScore = (GetBankerScore() / m_wMaxCardMulTiple - llAreaLimitScore)*TO_DOUBLE;

		//玩家还可下注金币限制
		CT_LONGLONG llJettonCount = GetUserAllAreaBetScore(dwChairID);
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(dwChairID);
		CT_LONGLONG llUserLimitScore = llUserScore / m_wMaxCardMulTiple;
		StatusSendCard.dUserMaxScore = (llUserLimitScore - llJettonCount)*TO_DOUBLE;

		SendTableData(dwChairID, SC_GAMESCENE_SENDCARD, &StatusSendCard, sizeof(NN_CMD_S_StatusSendCard), false);

		//同步椅子信息
		SendUserSitList(dwChairID);
		break;
	}
	case GAME_STATUS_SCORE:		//下注状态
	{
		NN_CMD_S_StatusPlay StatusPlay;
		memset(&StatusPlay, 0, sizeof(NN_CMD_S_StatusPlay));

		//游戏状态
		StatusPlay.wGameStatus = m_wGameStatus;
		//库存数值
		//StatusPlay.dStorageCount = m_llCaiJinChiCount*TO_DOUBLE;								
		//当前庄家
		StatusPlay.wBankerUser = m_wBankerUser;										
		//庄家分数
		StatusPlay.dBankerScore = GetBankerScore()*TO_DOUBLE;	
		//庄家成绩
		StatusPlay.dBankerWinScore = m_llBankerWinScore*TO_DOUBLE;		
		StatusPlay.dBankerTotalWinScore = m_llBankerTotalWinScore*TO_DOUBLE;	//庄家总成绩
		StatusPlay.cbBankCurrCount = (CT_BYTE)m_wBankerCount;					//庄家当前次数
		StatusPlay.cbBankTotalCount = MAX_BANKER_COUNT;							//最大做庄次数
		//申请庄家条件
		StatusPlay.dApplyBankerCondition = m_llApplyBankerCondition*TO_DOUBLE;	
		//玩家是否可以续注
		StatusPlay.cbCanAddLastBet = m_cbCanAddLastJetton[dwChairID];
		//玩家坐下金币限制
		StatusPlay.dUserSitLimitScore = SIT_SCORE_LIMIT*TO_DOUBLE;		
		//税收金额
		//StatusPlay.dRevenueScore = m_llRevenueScore*TO_DOUBLE;								

		//全体总注
		//memcpy(StatusPlay.dAllJettonScore, m_llAllJettonScore, sizeof(StatusPlay.dAllJettonScore));
		for (CT_WORD i = 0; i <= AREA_COUNT; ++i)
		{
			StatusPlay.dAllJettonScore[i] = m_llAllJettonScore[i]*TO_DOUBLE;
			memcpy(StatusPlay.cbTableCardArray[i], m_cbTableCardArray[i], FIRST_CARD);
		}
		
		//桌面扑克
		//memcpy(StatusPlay.cbTableCardArray, m_cbTableCardArray, sizeof(StatusPlay.cbTableCardArray));
		
		for (CT_WORD i = 0; i <= AREA_COUNT; ++i)
		{
			//桌面牌类型
			//StatusPlay.cbTableCarType[i] = m_GameLogic.GetCardType(StatusPlay.cbTableCardArray[i], MAX_COUNT);
			if (i != 0)
			{
				//个人总注
				StatusPlay.dUserJettonScore[i] += m_llUserJettonScore[i][dwChairID]*TO_DOUBLE;
			}	
		}

		//区域输赢分			
		//memcpy(StatusPlay.llAreaLWScore, m_llAreaTotalLWScore, sizeof(StatusPlay.llAreaLWScore));
		for (CT_WORD i = 0; i <= AREA_COUNT; ++i)
		{
			StatusPlay.dAreaLWScore[i] = m_llAreaTotalLWScore[i]*TO_DOUBLE;
		}

		if (StatusPlay.wGameStatus == GAME_STATUS_SCORE)
		{
			CT_DWORD dwTime = (CT_DWORD)time(NULL) - m_dwBetTime;
			dwTime = dwTime < TIME_BET ? (TIME_BET - dwTime - 1) : 0;
			StatusPlay.cbTimeLeave = (CT_BYTE)dwTime;
			//LOG_IF(INFO, PRINT_GAME_INFO) << "OnEventGameScene: score dwChairID = " << dwChairID << ",dwTimeLeave = " << dwTime;
		}
		else if (StatusPlay.wGameStatus == GAME_STATUS_END)
		{//结束时间 + 准备时间
			CT_DWORD dwTime = m_dwEndTime - (CT_DWORD)time(NULL);
			//dwTime = dwTime < TIME_GAME_END ? (TIME_GAME_END - dwTime - 1) : 0;
			dwTime += TIME_FREE;
			StatusPlay.cbTimeLeave = (CT_BYTE)dwTime;
			//LOG_IF(INFO, PRINT_GAME_INFO) << "OnEventGameScene: end dwChairID = " << dwChairID << ",dwTimeLeave = " << dwTime;
		}

		//玩家还可下注金币限制
		CT_LONGLONG llJettonCount = GetUserAllAreaBetScore(dwChairID);
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(dwChairID);
		CT_LONGLONG llUserLimitScore = llUserScore / m_wMaxCardMulTiple;
		StatusPlay.dUserMaxScore = (llUserLimitScore - llJettonCount)*TO_DOUBLE;

		//输赢分(0-5:对应椅子玩家 6：无座玩家（包括自己） 7:自己)
		//椅子
		for (CT_WORD i = 0; i < MAX_SIT_POS; ++i)
		{//0-5:对应椅子玩家
			if (m_wSitList[i] == INVALID_CHAIR) continue;
			StatusPlay.dLWScore[i] = m_llUserAllLWScore[m_wSitList[i]]*TO_DOUBLE;
			for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
			{
				StatusPlay.dUserAreaLWScore[i][wAreaIndex] = m_llUserAreaTotalLWScore[m_wSitList[i]][wAreaIndex]*TO_DOUBLE;
			}
		}

		//无座
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{//6:无座玩家（包括自己）
			if (!m_pGameDeskPtr->IsExistUser(i)) continue;
			if (0 == m_wPlayStatus[i]) continue;
			if (GetUserSitPos(i) != INVALID_CHAIR) continue;

			StatusPlay.dLWScore[6] += m_llUserAllLWScore[i]*TO_DOUBLE;
			for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
			{
				StatusPlay.dUserAreaLWScore[6][wAreaIndex] = m_llUserAreaTotalLWScore[i][wAreaIndex]*TO_DOUBLE;
			}
		}

		//7:自己
		StatusPlay.dLWScore[7] += m_llUserAllLWScore[dwChairID]*TO_DOUBLE;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			StatusPlay.dUserAreaLWScore[7][wAreaIndex] = m_llUserAreaTotalLWScore[dwChairID][wAreaIndex]*TO_DOUBLE;
		}
		//发送数据
		SendTableData(dwChairID, SC_GAMESCENE_PLAY, &StatusPlay, sizeof(NN_CMD_S_StatusPlay),false);
		//同步椅子信息
		SendUserSitList(dwChairID);
		//SendCaiJinChiChampionInfo(dwChairID);
		break;
	}
	case GAME_STATUS_END:
	{
		NN_CMD_S_StatusEnd statusEnd;
		memset(&statusEnd, 0, sizeof(statusEnd));

		statusEnd.cbTimeLeave = (CT_BYTE)(m_dwEndTime - time(NULL));
		statusEnd.cbTotalTime = m_cbEndTimeInterval;

		statusEnd.wBankerUser = m_wBankerUser;
		statusEnd.dBankerWinScore = m_GameEnd.dBankerWinScore;
		statusEnd.dBankerPreScore = m_llBankerSourceScore*TO_DOUBLE;
		statusEnd.dBankerScore = m_GameEnd.dBankerScore;
		statusEnd.dBankerTotalWinScore = m_llBankerTotalWinScore*TO_DOUBLE;
		statusEnd.cbBankCurrCount = (CT_BYTE)m_wBankerCount;					//庄家当前次数
		statusEnd.cbBankTotalCount = MAX_BANKER_COUNT;

		statusEnd.dApplyBankerCondition = m_llApplyBankerCondition*TO_DOUBLE;
		statusEnd.dUserSitLimitScore = SIT_SCORE_LIMIT*TO_DOUBLE;

		for (CT_WORD i = 0; i <= AREA_COUNT; ++i)
		{
			statusEnd.dAllJettonScore[i] = m_llAllJettonScore[i] * TO_DOUBLE;

			if (i != 0)
			{
				//个人总注
				statusEnd.dUserJettonScore[i] += m_llUserJettonScore[i][dwChairID] * TO_DOUBLE;
			}
		}

		memcpy(&statusEnd.cbHintCard, &m_GameEnd.cbHintCard, sizeof(statusEnd.cbHintCard));
		memcpy(&statusEnd.cbTableCardArray, &m_GameEnd.cbTableCardArray, sizeof(statusEnd.cbTableCardArray));
		memcpy(&statusEnd.cbTableCarType, &m_GameEnd.cbTableCarType, sizeof(statusEnd.cbTableCarType));
		memcpy(&statusEnd.iMultiple, &m_GameEnd.iMultiple, sizeof(statusEnd.iMultiple));
		memcpy(&statusEnd.dLWScore, &m_GameEnd.dLWScore, sizeof(statusEnd.dLWScore));
		memcpy(&statusEnd.dAreaLWScore, &m_GameEnd.dAreaLWScore, sizeof(statusEnd.dAreaLWScore));
		statusEnd.dRevenueScore = m_GameEnd.dRevenueScore;
		memcpy(&statusEnd.dUserAreaLWScore, &m_GameEnd.dUserAreaLWScore, sizeof(statusEnd.dUserAreaLWScore));
		memcpy(&statusEnd.dUserTotalScore, &m_GameEnd.dUserTotalScore, sizeof(statusEnd.dUserTotalScore));
		//进入的玩家真实分数不能通过原来的成绩获取,查询玩家的真实分数
		statusEnd.dUserTotalScore[dwChairID] = m_pGameDeskPtr->GetUserScore(dwChairID) * TO_DOUBLE;
		//拷贝玩家ID
		memcpy(&statusEnd.dwUserID, &m_dwAllUserID, sizeof(m_GameEnd.dwUserID));

		CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
		if (m_dwAllUserID[dwChairID] != dwUserID )
		{
			statusEnd.dLWScore[7] = 0;
			//statusEnd.dAreaLWScore[7] = 0;
			//LOG(WARNING) << "not the same user!";
		}
		//else
		//{
		//	LOG(WARNING) << "the same user??????, user id :" << dwUserID;
		//}

		//结算状态
		statusEnd.cbEndType = en_Normal_End;
		if (m_cbBankerWinAreaCount == AREA_COUNT)
		{
			statusEnd.cbEndType = en_All_Kill;
		}
		else if (m_cbBankerWinAreaCount == 0)
		{
			statusEnd.cbEndType = en_All_Compensate;
		}
		if (m_wPlayStatus[dwChairID] == 0)
		{
			statusEnd.cbEndType = en_No_Bet;
		}
		statusEnd.cbEndType = m_GameEnd.cbEndType;
		statusEnd.cbBigWinnerHeadIndex = m_GameEnd.cbBigWinnerHeadIndex;
		statusEnd.cbBigWinnerSex = m_GameEnd.cbBigWinnerSex;
		memcpy(&statusEnd.szBigWinnerNickName, &m_GameEnd.szBigWinnerNickName, sizeof(statusEnd.szBigWinnerNickName));
		statusEnd.dBigWinnerScore = statusEnd.dBigWinnerScore;
		memcpy(&statusEnd.cbHistory, &m_GameEnd.cbHistory, sizeof(statusEnd.cbHistory));

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
	case ID_TIME_FREE:			//空闲时间
	{
		//清理定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_FREE);

		m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Start);
		//开始游戏
		OnEventGameStart();

		//设置时间
		CT_DWORD dwSendCardTime = TIME_SENDCARD;
		m_pGameDeskPtr->SetGameTimer(ID_TIME_SEND_CARD, dwSendCardTime * 1000);

		//设置状态
		m_wGameStatus = GAME_STATUS_SENDCARD;
		m_dwSendCardTime = (CT_DWORD)time(NULL);
		//LOG_IF(INFO, PRINT_GAME_INFO) << "OnTimerMessage ID_TIME_FREE curr_banker = " << m_wBankerUser ;
		return ;
	}
	case ID_TIME_SEND_CARD:			//发牌时间
	{
		//清理定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_SEND_CARD);

		NN_CMD_S_GameBet GameStartBet;
		GameStartBet.cbTimeLeave = TIME_BET;
		GameStartBet.dBetScoreLimit = (m_llBankerSourceScore / m_wMaxCardMulTiple)*TO_DOUBLE;	//下注金额限制

		for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
			{//玩家不存在
				continue;
			}

			//检测玩家是否还可以续注
			if (m_cbCanAddLastJetton[i] == 1)
			{
				CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
				CT_LONGLONG llLastJettonCount = 0;
				for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
				{
					llLastJettonCount += m_llUserLastJettonScore[wAreaIndex][i];
				}

				if (llLastJettonCount > llUserScore/ m_wMaxCardMulTiple || llUserScore < CAN_JETTON_SCORE)
				{
					m_cbCanAddLastJetton[i] = 0;
					for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
					{
						m_llUserLastJettonScore[wAreaIndex][i] = 0;
					}
					//memset(m_llUserLastJettonScore, 0, sizeof(m_llUserLastJettonScore));
				}
			}
			GameStartBet.cbCanAddLastBet = m_cbCanAddLastJetton[i];
			
			//发送数据
			SendTableData(i, NN_SUB_S_GAME_START_BET, &GameStartBet, sizeof(NN_CMD_S_GameBet), false);
		}

		//设置时间
		CT_DWORD dwBetTime = TIME_BET;
		m_pGameDeskPtr->SetGameTimer(ID_TIME_BET_SCORE, dwBetTime * 1000);
		m_wGameStatus = GAME_STATUS_SCORE;
		m_dwBetTime = (CT_DWORD)time(NULL);
		return;
	}
	case ID_TIME_BET_SCORE:			//下注时间到
	{
		//清理定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_BET_SCORE);
		//设置状态
		m_wGameStatus = GAME_STATUS_END;

		//结束游戏
		OnEventGameEnd(INVALID_CHAIR, GER_NORMAL);
		//设置时间 （定时器放到gameend里面设置）
		//CT_DWORD dwEndTime = TIME_GAME_END;
		//m_pGameDeskPtr->SetGameTimer(ID_TIME_GAME_END, dwEndTime * 1000);
		//m_dwEndTime = (CT_DWORD)time(NULL) + TIME_GAME_END;	
		
		//LOG_IF(INFO, PRINT_GAME_INFO) << "OnTimerMessage ID_TIME_BET_SCORE  curr_banker = " << m_wBankerUser;
		return;
	}
	case ID_TIME_GAME_END:			//结束游戏
	{
		//清理定时器
		m_pGameDeskPtr->KillGameTimer(ID_TIME_GAME_END);
		
		//清除桌子离线玩家
		m_pGameDeskPtr->ClearTableUser(INVALID_CHAIR, true);
		//0点重置彩金池
		//ResetCaiJinChi();
		//更新申请列表
		SendBankerApplyList(INVALID_CHAIR);
		//更新庄家
		UpdateBanker();
		//更新座位信息
		//SendUserSitList(INVALID_CHAIR);
		UpdateUserSitInfo();
		//设置时间
		CT_DWORD dwFreeTime = TIME_FREE;
		//设置状态
		m_wGameStatus = GAME_STATUS_FREE;
		//空闲，等待下注
		NN_CMD_S_GameFree GameFree;
		memset(&GameFree, 0, sizeof(GameFree));
		GameFree.cbTimeLeave = dwFreeTime;						//剩余时间
		//GameFree.dStorageCount = m_llCaiJinChiCount*TO_DOUBLE;	//库存数值
		GameFree.wBankerUser = m_wBankerUser;					//庄家
		GameFree.dBankerScore = GetBankerScore()*TO_DOUBLE;		//庄家金币
		//GameFree.cbBankCount = (CT_BYTE)m_wBankerCount;
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i)) 
				continue;
			SendTableData(i, NN_SUB_S_GAME_FREE, &GameFree, sizeof(NN_CMD_S_GameFree), false);
		}
		m_pGameDeskPtr->SetGameTimer(ID_TIME_FREE, dwFreeTime * 1000);
		m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Free);
		m_dwFreeTime = (CT_DWORD)time(NULL) + TIME_FREE;
	
		//LOG_IF(INFO, PRINT_GAME_INFO) << "OnTimerMessage ID_TIME_GAME_END  curr_banker = " << m_wBankerUser;
		return;
	}
	case ID_TIME_PING:
	{
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (m_pGameDeskPtr->IsExistUser(i) && !m_pGameDeskPtr->IsAndroidUser(i))
			{
				SendTableData(i, NN_SUB_S_PING);
				m_dwPingTime = Utility::GetTick();
				break;
			}	
		}
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
	case NN_SUB_C_BET:					//用户下注
	{
		if (GAME_STATUS_SCORE != m_wGameStatus)
		{//游戏状态不对
			SendErrorCode(wChairID, ERROR_STATUS);
			return false;
		}

		if (sizeof(NN_CMD_C_Bet) != dwDataSize)
		{//效验消息大小
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}
		//消息处理
		NN_CMD_C_Bet* pBet = (NN_CMD_C_Bet*)pDataBuffer;
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
	case NN_SUB_C_BANKER_APPLY_OPERATE:					//申请/取消申请庄家
	{
		//效验消息
		if (sizeof(NN_CMD_C_BankerApplyOperate) != dwDataSize)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}
		//消息处理
		NN_CMD_C_BankerApplyOperate* pOperate = (NN_CMD_C_BankerApplyOperate*)pDataBuffer;
		if (NULL == pOperate)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}
		if (pOperate->cbType == 0)
		{
			wRtn = OnUserApplyBanker(wChairID);
			//LOG_IF(INFO, PRINT_GAME_INFO) << "OnUserApplyBanker wChairID=" << wChairID << ",wRtn=" << wRtn;
		}
		else
		{
			wRtn = OnUserCancelApplyBanker(wChairID);
			//LOG_IF(INFO, PRINT_GAME_INFO) << "OnUserCancelApplyBanker wChairID=" << wChairID << ",wRtn=" << wRtn;
		}	
		return wRtn > 0;
	}
	case NN_SUB_C_APPLY_CANCEL_BANKER:				//申请下庄
	{
		/*if (GAME_STATUS_FREE != m_wGameStatus)
		{//游戏状态不对
			SendErrorCode(wChairID, ERROR_STATUS);
			return false;
		}*/

		//效验消息
		if (sizeof(NN_CMD_C_ApplyCancelBanker) != dwDataSize)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}
		//消息处理
		NN_CMD_C_ApplyCancelBanker* pCancelApplyBanker = (NN_CMD_C_ApplyCancelBanker*)pDataBuffer;
		if (NULL == pCancelApplyBanker)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}

		wRtn = OnUserCancelBanker(wChairID);
		
		return wRtn > 0;
	}
	case NN_SUB_C_SIT:
	{
		return true;
		/*if (GAME_STATUS_END == m_wGameStatus)
		{//游戏状态不对
			SendErrorCode(wChairID, ERROR_STATUS);
			return false;
		}

		//效验消息
		if (sizeof(NN_CMD_C_Sit) != dwDataSize)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}
		//消息处理
		NN_CMD_C_Sit* pSit = (NN_CMD_C_Sit*)pDataBuffer;
		if (NULL == pSit)
		{
			SendErrorCode(wChairID, ERROR_MESSAGE);
			return false;
		}

		wRtn = OnUserSit(wChairID, pSit->cbType, pSit->cbSitPos);
		//LOG_IF(INFO, PRINT_GAME_INFO) << "OnUserSit wChairID=" << wChairID << ",wRtn=" << wRtn;
		return wRtn > 0;*/
	}
	case NN_SUB_C_ADD_LAST_BET:
	{
		if (m_wGameStatus != GAME_STATUS_SCORE)
		{
			SendErrorCode(wChairID, ERROR_STATUS);
			return false;
		}

		wRtn = OnUserBetLastScore(wChairID);
		return wRtn > 0;
	}
	case NN_SUB_C_ADD_QUERY_USER_HISTORY:
	{
		return 	OnQueryUserGameHistory(wChairID);
	}
	case NN_SUB_C_PING:
	{
		CT_DWORD dwNow = Utility::GetTick();
		LOG(INFO) << "ping: " << dwNow - m_dwPingTime;
		return true;
		
	}
	}
	return CT_TRUE;
}

void CGameProcess::OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (m_wGameStatus == GAME_STATUS_FREE)
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
			//设置时间
			CT_DWORD dwFreeTime = TIME_FREE;
			m_pGameDeskPtr->SetGameTimer(ID_TIME_FREE, dwFreeTime * 1000);
			m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Start);
			m_dwFreeTime = (CT_DWORD)time(NULL) + TIME_FREE;
			UpdateUserSitInfo();
			//LOG_IF(INFO,PRINT_GAME_INFO) << "OnUserEnter: ID_TIME_FREE m_dwFreeTime = " << m_dwFreeTime;
		}
	}
	//历史记录消息
	NN_CMD_S_HistoryResult HistoryResult;
	memset(&HistoryResult, 0, sizeof(HistoryResult));
	HistoryResult.cbCount = m_cbHistoryCount;
	memcpy(HistoryResult.cbHistory,m_cbHistory,sizeof(HistoryResult.cbHistory));
	memcpy(HistoryResult.cbWinRate, m_cbWinRate, sizeof(HistoryResult.cbWinRate));
	memcpy(HistoryResult.cbHistoryCardType, m_cbHistoryCardType, sizeof(HistoryResult.cbHistoryCardType));
	SendTableData(dwChairID, NN_SUB_S_QUERY_HISTORY, &HistoryResult, sizeof(NN_CMD_S_HistoryResult), false);
    
    SendBankerApplyList(INVALID_CHAIR);
	//如果新进入玩家,清空游戏局数
	if (m_wPlayStatus[dwChairID] == 0)
	{
		memset(&m_UserHistory[dwChairID], 0, sizeof(m_UserHistory[dwChairID]));
		m_UserHistory[dwChairID].wChairID = dwChairID;
	}
}

void CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag)
{
	if (/*dwChairID < 0 ||*/ dwChairID >= GAME_PLAYER)
	{
		return;
	}

	//如果未押注玩家离开，直接踢出去( 非游戏中框架已经清空玩家了)
	CT_BOOL bRealLeave = true;
	if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_Start)
	{
		if (GetUserAllAreaBetScore(dwChairID) == 0 && dwChairID != m_wBankerUser)
		{
			m_pGameDeskPtr->ClearTableUser(dwChairID, true);
		}
		else
		{
			bRealLeave = false;
		}
	}

	if (bRealLeave)
	{
		//座椅操作
		CT_WORD wPos = GetUserSitPos(dwChairID);
		if (wPos != INVALID_CHAIR)
		{
			m_wSitList[wPos] = INVALID_CHAIR; //椅子上玩家金币不足或者退出了，更新椅子信息
		}
		//申请庄家链表操作
		for (CT_WORD i = 0; i < m_ApplyUserArray.size(); ++i)
		{
			if (dwChairID != m_ApplyUserArray[i])
			{
				continue;
			}
			//已申请,删除申请列表数据
			m_ApplyUserArray.erase(m_ApplyUserArray.begin() + i);
			SendBankerApplyList(INVALID_CHAIR);
			break;
		}

		if (dwChairID == m_wBankerUser)
		{
			OnUserCancelBanker(dwChairID);
		}

		//删除玩家ID
		//m_dwAllUserID[dwChairID] = 0;
		memset(&m_UserHistory[dwChairID], 0, sizeof(m_UserHistory[dwChairID]));


		//玩家离开清空续注功能
		if (m_cbCanAddLastJetton[dwChairID] == 1)
		{
			m_cbCanAddLastJetton[dwChairID] = 0;
			for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
			{
				m_llUserLastJettonScore[wAreaIndex][dwChairID] = 0;
			}
		}
	}
}

void CGameProcess::OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	//删除玩家ID
	//m_dwAllUserID[dwChairID] = 0;

	//玩家离开清空续注功能
	if (m_cbCanAddLastJetton[dwChairID] == 1)
	{
		m_cbCanAddLastJetton[dwChairID] = 0;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			m_llUserLastJettonScore[wAreaIndex][dwChairID] = 0;
		}
	}
}

void CGameProcess::OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore)
{
	if (m_wPlayStatus[dwChairID] == 1)
	{
		m_llUserSourceScore[dwChairID] += llRechargeScore;
	}
}

//设置指针
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	assert(NULL != pTableFrame);
	m_pGameDeskPtr = pTableFrame;

	tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
	if (pGameRoomKind->wRoomKindID == PRIMARY_ROOM)
	{
		m_wMaxCardMulTiple = MAX_TYPE_MULTIPLE;
	}
	else
	{
		m_wMaxCardMulTiple = MAX_TYPE_MULTIPLE2;
	}

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

CT_VOID CGameProcess::SetSystemOprateRatio(std::vector<tagBrGameOperateRatio> &vecChangeCardRatio)
{
	m_vecSystemOperateRatio.clear();
	
	for (auto& it : vecChangeCardRatio)
	{
		m_vecSystemOperateRatio.push_back(it);
	}
	
	CalSystemOperateRation();
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
	//游戏状态
	m_wGameStatus = GAME_STATUS_FREE;
	//发牌个数
	m_cbSendCardCount = 0;
	//是否有玩家下注
	m_bHasUserAddJetton = false;
	//游戏中玩家
	memset(m_wPlayStatus,0,sizeof(m_wPlayStatus));
	//倍数
	//memset(m_iMultiple, 1, sizeof(m_iMultiple));
	for (CT_BYTE i = 0; i < (AREA_COUNT + 1); ++i)
	{
		m_iMultiple[i] = 1;
	}
	//全体总注
	memset(m_llAllJettonScore,0,sizeof(m_llAllJettonScore));
	memset(m_llRealUserJettonScore,0, sizeof(m_llRealUserJettonScore));
	//区域个人总注
	memset(m_llUserJettonScore,0,sizeof(m_llUserJettonScore));
	//个人总注
	memset(m_llUserRoundTotalJettonScore, 0, sizeof(m_llUserRoundTotalJettonScore));
	
	//空闲时间
	m_dwFreeTime = 0;
	//发牌时间
	m_dwSendCardTime = 0;
	//下注时间
	m_dwBetTime = 0;
	//结束时间
	m_dwEndTime = 0;
	m_cbEndTimeInterval = 0;
	//桌面扑克
	memset(m_cbTableCardArray,0,sizeof(m_cbTableCardArray));
	//庄家赢分
	m_llBankerWinScore = 0;	
	//庄家原始分数
	m_llBankerSourceScore = 0;
	//是否申请了下庄
	m_bIsApplyCancelBanker = false;
	//庄家赢区域总数
	m_cbBankerWinAreaCount = 0;
	//区域总输赢分	
	memset(m_llAreaTotalLWScore, 0, sizeof(m_llAreaTotalLWScore));
	//玩家区域总输赢分
	memset(m_llUserAreaTotalLWScore, 0, sizeof(m_llUserAreaTotalLWScore));
	//用户ID
	memset(m_dwAllUserID, 0, sizeof(m_dwAllUserID));
	//用户原始分
	memset(m_llUserSourceScore, 0, sizeof(m_llUserSourceScore));
	
	//税收金额
	//m_llRevenueScore = 0;		
	//下局庄家
	m_wNextBankerUser = INVALID_CHAIR;	
	//剩余数目
	//m_cbSendCardCount = MAX_CARD_TOTAL;
	//库存扑克
	memset(m_cbRepertoryCard, 0, sizeof(m_cbRepertoryCard));
	//玩家当局总输赢分
	memset(m_llUserAllLWScore, 0, sizeof(m_llUserAllLWScore));
	//当局彩金池奖励
	//memset(m_llCaiJinChiRew, 0, sizeof(m_llCaiJinChiRew));
	memset(&m_FlashArea, 0, sizeof(m_FlashArea));

	//游戏记录
	memset(m_RecordScoreInfo, 0, sizeof(m_RecordScoreInfo));
	
	if (m_pGameDeskPtr)
	{
		tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
		if (pGameRoomKind->wRoomKindID == PRIMARY_ROOM)
		{
			//申请条件
			m_llApplyBankerCondition = APPLY_BANKER_SCORE;
		}
		else
		{
			m_llApplyBankerCondition = APPLY_BANKER_SCORE_2;
		}
	}
	
	m_bCheatAllKill = false;
	m_bCheatBlackList = false;
    m_bChangeAreaCard = false;
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
	m_pGameDeskPtr->KillGameTimer(ID_TIME_FREE);
}

//初始化游戏桌子数据
void CGameProcess::InitGameTableData()
{
	//庄家
	m_wBankerUser = INVALID_CHAIR;
	//做庄次数
	m_wBankerCount = 1;
	//下局庄家
	m_wNextBankerUser = INVALID_CHAIR;
	m_llBankerTotalWinScore = 0;

	//系统坐庄次数
	//m_dwSystemBankerCount = 0;
	//系统主动通杀次数
	//m_cbSystemAllKillCount = 0;

	//库存数值
	//m_llCaiJinChiCount = BASE_CAIJINCHI_NUM;//先写死，需要后台配置
	//m_llOpenRewScore = GetNextCaiJinChiOpenRew();
	//椅子信息
	//memset(m_wSitList, INVALID_CHAIR, sizeof(m_wSitList));
	for (int i = 0; i < MAX_SIT_POS; ++i)
	{
		m_wSitList[i] = INVALID_CHAIR;
	}

	//库存相关
	m_llStorageControl = 0;
	m_llStorageLowerLimit = 0;
	m_llTodayStorageControl = 0;
	m_llTodayStorageHighLimit = 0;
	m_wChangeCardRatio = 0;
	m_llAndroidStorage = 0;
	m_llAndroidStorageLowerLimit = 0;
	m_wSystemAllKillRatio = 0;
	m_dwTodayTime = (CT_DWORD)time(NULL);

	//历史记录
	memset(m_cbHistory, 0, sizeof(m_cbHistory));
	memset(m_cbHistoryCardType, 0, sizeof(m_cbHistoryCardType));
	memset(m_cbWinRate, 0, sizeof(m_cbWinRate));
	m_cbHistoryCount = 0;
	m_bIsTableRuning = false;

	//保存在桌子上所有玩家ID
	memset(m_dwAllUserID, 0, sizeof(m_dwAllUserID));

	//总的下注信息
	memset(m_UserHistory, 0, sizeof(m_UserHistory));

	//续注
	memset(m_cbCanAddLastJetton, 0, sizeof(m_cbCanAddLastJetton));
	memset(m_llUserLastJettonScore, 0, sizeof(m_llUserLastJettonScore));

	m_wMaxCardMulTiple = 1;
}

//更新庄家
CT_BOOL CGameProcess::UpdateBanker()
{
	//是否切换庄家
	CT_BYTE cbReason = m_bIsApplyCancelBanker ? 2 : 0;
	CT_BOOL bChangeBanker = m_bIsApplyCancelBanker;
	m_bIsApplyCancelBanker = false;
	if (m_wNextBankerUser != INVALID_CHAIR)
	{
		if (!m_pGameDeskPtr->IsExistUser(m_wNextBankerUser))
		{
			m_wNextBankerUser = INVALID_CHAIR;
		}
		bChangeBanker = true;
	}
	//检查条件
	if (!bChangeBanker && m_wBankerUser != INVALID_CHAIR)
	{
		if (m_pGameDeskPtr->IsExistUser(m_wBankerUser))
		{
			//当达到最大坐庄次数，自动下庄
			if (m_wBankerCount > MAX_BANKER_COUNT)
			{
				bChangeBanker = true;
				cbReason = 1;
			}
			//金币不够，自动下庄
			CT_LONGLONG llBankerScore = m_pGameDeskPtr->GetUserScore(m_wBankerUser);
			if (llBankerScore < m_llApplyBankerCondition)
			{
				bChangeBanker = true;
				cbReason = 3;
			}
		}
		else
		{
			bChangeBanker = true;
			cbReason = 2;
		}
	}

	if (!bChangeBanker)
	{
		if (m_wBankerUser == INVALID_CHAIR && !m_ApplyUserArray.empty())//当前系统当庄，申请链表有人
		{
			bChangeBanker = true;
			cbReason = 0;
		}
	}

	if (bChangeBanker)
	{//切换庄家
		if (m_wNextBankerUser == INVALID_CHAIR)
		{
			if (!m_ApplyUserArray.empty())
			{
				m_wNextBankerUser = m_ApplyUserArray[0];
				m_ApplyUserArray.erase(m_ApplyUserArray.begin());
			}
		}	
	}
	return SendChangeBanker(bChangeBanker, cbReason);
}

//获得下一个玩家
CT_WORD CGameProcess::GetNextUser()
{
	for (CT_WORD i = 1; i <= GAME_PLAYER; ++i)
	{
		CT_WORD wTempBanker = (m_wBankerUser + i) % GAME_PLAYER;
		if (1 == m_wPlayStatus[wTempBanker])
		{
			return wTempBanker;
		}
	}
	return m_wBankerUser;
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwChairID)
{
	//递增坐庄次数
	if (m_wBankerUser != INVALID_CHAIR)
	{
		m_wBankerCount++;
	}
	
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));
	m_GameEnd.wBankerUser = m_wBankerUser;								//庄家 
	m_GameEnd.dBankerScore = GetBankerScore()*TO_DOUBLE;				//庄家金币 
	//m_GameEnd.wBankerCount = m_wBankerCount;							//当庄次数 
	memcpy(m_GameEnd.cbTableCardArray, m_cbTableCardArray, sizeof(m_cbTableCardArray));//桌面扑克
	
	//计算倍数和牌型
	tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
	for (CT_WORD i = 0; i < AREA_COUNT + 1; i++)
	{
		//倍数
		m_iMultiple[i] = m_GameLogic.GetMultiple(m_GameEnd.cbTableCardArray[i], (CT_BYTE)pGameRoomKind->wRoomKindID);
		m_GameEnd.cbTableCarType[i] = m_GameLogic.GetCardType(m_GameEnd.cbTableCardArray[i], MAX_COUNT);

		if (m_GameEnd.cbTableCarType[i] > 0 && m_GameEnd.cbTableCarType[i] <= 10)
		{
			CT_BYTE cbOxCard[MAX_COUNT] = { 0 };
			memcpy(cbOxCard, &m_GameEnd.cbTableCardArray[i], MAX_COUNT);
			m_GameLogic.GetOxCard(cbOxCard);

			for (CT_BYTE cbCardIndex = 3; cbCardIndex < MAX_COUNT; ++cbCardIndex)
			{
				CT_BYTE cbOxCardData = cbOxCard[cbCardIndex];
				for (CT_BYTE cbCardIndexEx = 0; cbCardIndexEx < MAX_COUNT; ++cbCardIndexEx)
				{
					if (m_GameEnd.cbTableCardArray[i][cbCardIndexEx] == cbOxCardData)
					{
						m_GameEnd.cbHintCard[i][cbCardIndexEx] = 1;
						break;
					}
				}
			}
		}
	}
	memcpy(m_GameEnd.iMultiple, m_iMultiple, sizeof(m_iMultiple));

	CT_WORD wBankerMultiple = m_GameEnd.iMultiple[0];

	//计算输赢
	CT_LONGLONG llLWScore[GAME_PLAYER] = { 0 };								//玩家输赢分
	memset(llLWScore, 0, sizeof(llLWScore));

	CT_LONGLONG llUserRevenue[GAME_PLAYER] = { 0 };							//玩家税收
	memset(llUserRevenue, 0, sizeof(llUserRevenue));

	CT_LONGLONG llUserAreaRevenue[GAME_PLAYER][AREA_COUNT + 1] = {{0}};    //玩家区域税收
	memset(llUserAreaRevenue, 0, sizeof(llUserAreaRevenue));

	CT_LONGLONG llRealLWScore[GAME_PLAYER] = { 0 };							//玩家真正的输赢分(加上玩家下注和扣掉的税)
	memset(llRealLWScore, 0, sizeof(llRealLWScore));

	CT_LONGLONG llUserAreaLWScore[GAME_PLAYER][AREA_COUNT + 1] = {{ 0 }}; 	//玩家各区域输赢分
	memset(llUserAreaLWScore, 0, sizeof(llUserAreaLWScore));

	CT_BYTE  cbUserWinMaxScoreCardType[GAME_PLAYER] = { 0 };
	memset(cbUserWinMaxScoreCardType, 0, sizeof(cbUserWinMaxScoreCardType));

	CT_LONGLONG llBankerRevenue = 0;
	CT_BYTE cbBankerWinCount = 0;
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		CT_LONGLONG llAreaLWScore = 0;
		if (m_GameLogic.CompareCard(m_GameEnd.cbTableCardArray[0], m_GameEnd.cbTableCardArray[wAreaIndex]))
		{
			++cbBankerWinCount;
			//庄赢
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;
				if (m_wBankerUser == i) continue;

				//玩家对应区域下注金额
				CT_LONGLONG llTempScore = GetUserAreaBetScore(i, wAreaIndex);
				if (llTempScore == 0) continue;

				CT_LONGLONG llBankWinScore = llTempScore * wBankerMultiple * m_dwCellScore;
				CT_LONGLONG llRevenue = m_pGameDeskPtr->CalculateRevenue(i, llBankWinScore);//计算税收
				llBankerRevenue += llRevenue;
				if (m_wBankerUser != INVALID_CHAIR)
				{
					llUserAreaRevenue[m_wBankerUser][wAreaIndex] += llRevenue;
				}

				m_llBankerWinScore += llBankWinScore;				//庄赢
				llLWScore[i] -= llBankWinScore;						//闲输

				llRealLWScore[i] -= llBankWinScore;					
				llRealLWScore[i] += llTempScore;					//加上玩家已经的注才是玩家要扣的钱
				llUserAreaLWScore[i][wAreaIndex] -= llBankWinScore;
				llAreaLWScore -= llBankWinScore;

				//统计库存
				if (m_wBankerUser == INVALID_CHAIR)				//如果是系统坐庄，系统赢真人，加库存
				{
					if (!m_pGameDeskPtr->IsAndroidUser(i))
					{
						m_llStorageControl += llBankWinScore;
						m_llTodayStorageControl += llBankWinScore;
					}
				}
				else
				{
					if (m_pGameDeskPtr->IsAndroidUser(i))		//真人坐庄，机器人输钱，减库存
					{
						m_llStorageControl -= llBankWinScore;
						m_llAndroidStorage -= llBankWinScore;

						m_llTodayStorageControl -= llBankWinScore;
					}
				}
			}
			m_GameEnd.cbHistory[wAreaIndex - 1] = 0;
		}
		else
		{	
			//闲赢
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;
				if (m_wBankerUser == i) continue;
				//玩家对应区域下注金额
				CT_LONGLONG llTempScore = GetUserAreaBetScore(i, wAreaIndex);
				if (llTempScore == 0) continue;

				CT_LONGLONG llBankLostScore = llTempScore * m_GameEnd.iMultiple[wAreaIndex] * m_dwCellScore;
				CT_LONGLONG llRevenue = m_pGameDeskPtr->CalculateRevenue(i, llBankLostScore);//计算税收
				llUserRevenue[i] += llRevenue;
				llUserAreaRevenue[i][wAreaIndex] += llRevenue;

				m_llBankerWinScore -= llBankLostScore;		//庄输
				llLWScore[i] += llBankLostScore;			//闲赢
				llRealLWScore[i] += llBankLostScore;
				llRealLWScore[i] += llTempScore;			//加上玩家下的注才是玩家要加的钱
				llUserAreaLWScore[i][wAreaIndex] += llBankLostScore;
				llAreaLWScore += llBankLostScore;
				//保存玩家赢分最大的牌型
				if (wAreaIndex == 1 || (wAreaIndex != 1 && llUserAreaLWScore[i][wAreaIndex] > llUserAreaLWScore[i][wAreaIndex -1]))
					cbUserWinMaxScoreCardType[i] = m_GameEnd.cbTableCarType[wAreaIndex];


				//统计库存
				if (m_wBankerUser == INVALID_CHAIR)						//如果是系统坐庄，真人赢钱，减少库存
				{
					if (!m_pGameDeskPtr->IsAndroidUser(i))
					{
						m_llStorageControl -= llBankLostScore;
						m_llTodayStorageControl -= llBankLostScore;
					}
				}
				else
				{
					if (m_pGameDeskPtr->IsAndroidUser(i))				//如果真人坐庄，机器人赢钱，增加库存
					{
						m_llStorageControl += llBankLostScore;
						m_llAndroidStorage += llBankLostScore;
						m_llTodayStorageControl += llBankLostScore;
					}
				}
			}
			m_GameEnd.cbHistory[wAreaIndex - 1] = 1;
		}

		m_GameEnd.dAreaLWScore[wAreaIndex] += llAreaLWScore*TO_DOUBLE;			//区域输赢分
		//路单数据
		if (m_cbHistoryCount == HISTORY_COUNT)
		{
			for (CT_BYTE cbCount = 0; cbCount < (m_cbHistoryCount-1) ; ++cbCount)
			{
				m_cbHistory[cbCount][wAreaIndex - 1] = m_cbHistory[cbCount + 1][wAreaIndex - 1];
				m_cbHistoryCardType[cbCount][wAreaIndex - 1] = m_cbHistoryCardType[cbCount + 1][wAreaIndex - 1];
			}
			m_cbHistory[m_cbHistoryCount-1][wAreaIndex - 1] = m_GameEnd.cbHistory[wAreaIndex - 1];
			m_cbHistoryCardType[m_cbHistoryCount-1][wAreaIndex - 1] = m_GameEnd.cbTableCarType[wAreaIndex];
		}
		else
		{
			m_cbHistory[m_cbHistoryCount][wAreaIndex - 1] = m_GameEnd.cbHistory[wAreaIndex - 1];
			m_cbHistoryCardType[m_cbHistoryCount][wAreaIndex - 1] = m_GameEnd.cbTableCarType[wAreaIndex];
		}
	}
	if (m_cbHistoryCount < HISTORY_COUNT)
	{
		++m_cbHistoryCount;
	}

	//计算最近10局胜率
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		CT_BYTE cbWinCount = 0;
		for (CT_BYTE cbHistoryCount = 0; cbHistoryCount != m_cbHistoryCount; ++cbHistoryCount)
		{
			if (m_cbHistory[cbHistoryCount][wAreaIndex - 1] == 1)
			{
				++cbWinCount;
			}
		}
		m_cbWinRate[wAreaIndex - 1] = cbWinCount * 100 / m_cbHistoryCount;
		m_GameEnd.cbWinRate[wAreaIndex - 1] = m_cbWinRate[wAreaIndex - 1];
	}

	//非系统坐庄，记录庄家输赢
	if (m_GameEnd.wBankerUser != INVALID_CHAIR)
	{
		llLWScore[m_GameEnd.wBankerUser] = m_llBankerWinScore;
		llRealLWScore[m_GameEnd.wBankerUser] = m_llBankerWinScore;
		llUserRevenue[m_GameEnd.wBankerUser] = llBankerRevenue;
		m_wPlayStatus[m_GameEnd.wBankerUser] = 1;
		m_dwAllUserID[m_GameEnd.wBankerUser] = m_pGameDeskPtr->GetUserID(m_GameEnd.wBankerUser);
		//庄家不能继注
		m_cbCanAddLastJetton[m_GameEnd.wBankerUser] = 0;
		m_llBankerTotalWinScore += (m_llBankerWinScore - llBankerRevenue);
		m_GameEnd.dBankerTotalWinScore = m_llBankerTotalWinScore*TO_DOUBLE;
	}

	//计算玩家的
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_pGameDeskPtr->IsExistUser(i))
		{
			
			CT_BYTE cbIndex = m_UserHistory[i].wPlayCount % USER_HISTORY_COUNT;

			if (m_wPlayStatus[i] == 0)
			{
				m_UserHistory[i].llJettonScore[cbIndex] = 0;
				m_UserHistory[i].cbWinCount[cbIndex] = 0;
			}
			else
			{
				m_llUserAllLWScore[i] = llLWScore[i];

				m_UserHistory[i].llJettonScore[cbIndex] = m_llUserRoundTotalJettonScore[i];
				if (llLWScore[i] > 0)
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

	//计算通杀通赔
	m_GameEnd.cbEndType = en_Normal_End;
	if (cbBankerWinCount == AREA_COUNT)
	{
		m_GameEnd.cbEndType = en_All_Kill;
	}
	else if (cbBankerWinCount == 0)
	{
		m_GameEnd.cbEndType = en_All_Compensate;
	}
	m_cbBankerWinAreaCount = cbBankerWinCount;

	CT_LONGLONG llBigWinnerScore = 0;
	CT_WORD wBigWinnerChairID = INVALID_CHAIR;

	//拷贝玩家ID
	memcpy(&m_GameEnd.dwUserID, &m_dwAllUserID, sizeof(m_GameEnd.dwUserID));
	bool bNeedRecord = false;
	//写入积分
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		//没有下注的玩家
		if (0 == m_wPlayStatus[i])
		{
			//标识下一局不可以续注
			if (m_cbCanAddLastJetton[i] != 0)
				m_cbCanAddLastJetton[i] = 0;

			if (m_pGameDeskPtr->IsExistUser(i))
			{
				m_GameEnd.dUserTotalScore[i] = m_pGameDeskPtr->GetUserScore(i)*TO_DOUBLE;
			}

			continue;
		}

		//if (0 == m_wPlayStatus[i])
		//	continue;

		if (llLWScore[i] > 0)
		{
			CT_LONGLONG llUserWinScore = llLWScore[i] - llUserRevenue[i];
			if (i != m_wBankerUser && llUserWinScore > llBigWinnerScore)
			{
				llBigWinnerScore = llUserWinScore;
				wBigWinnerChairID = i;
			}
		}
		else
		{
			//玩家要扣掉的分数
			CT_LONGLONG llUserRealLost = -(llLWScore[i] - llUserRevenue[i]);
			if (llUserRealLost > m_llUserSourceScore[i])
			{
				CT_LONGLONG llBalance = llUserRealLost - m_llUserSourceScore[i];
				//因为llLWScore[i]是负数，所以得加上差额
				llLWScore[i] += llBalance;
				llRealLWScore[i] += llBalance;
			}
		}

		//写积分
		ScoreInfo  ScoreData;
		memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = m_pGameDeskPtr->GetUserID(i);
		ScoreData.bBroadcast = false; 
		ScoreData.llScore = llRealLWScore[i] - llUserRevenue[i];
		ScoreData.llRealScore = llLWScore[i] - llUserRevenue[i];
		if (i == m_wBankerUser)
			ScoreData.cbCardType = m_GameEnd.cbTableCarType[0] >= 10 ? m_GameEnd.cbTableCarType[0]: 255;
		else
			ScoreData.cbCardType = cbUserWinMaxScoreCardType[i] >= 10 ? cbUserWinMaxScoreCardType[i] : 255;

		m_pGameDeskPtr->WriteUserScore(i, ScoreData);

		//百人牛牛结算金币不广播，给前端发送最后的金额以刷新界面
		m_GameEnd.dUserTotalScore[i] = m_pGameDeskPtr->GetUserScore(i)*TO_DOUBLE;

		//积分记录部分(真人坐庄，全部数据记录，系统坐庄，不记录机器人)
		//if (m_wBankerUser != INVALID_CHAIR || (m_wBankerUser == INVALID_CHAIR && !m_pGameDeskPtr->IsAndroidUser(i)))
		{
			CT_BOOL bIsAndroid = m_pGameDeskPtr->IsAndroidUser(i);
			m_RecordScoreInfo[i].llSourceScore = m_llUserSourceScore[i];
			m_RecordScoreInfo[i].iScore = (CT_INT32)llLWScore[i];
			m_RecordScoreInfo[i].dwUserID = m_pGameDeskPtr->GetUserID(i);
			m_RecordScoreInfo[i].cbIsAndroid = bIsAndroid ? 1 : 0;
			m_RecordScoreInfo[i].dwRevenue = bIsAndroid ? 0: (CT_DWORD)llUserRevenue[i]; //机器人不记录税收
			m_RecordScoreInfo[i].cbStatus = 1;
			for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
			{
				m_RecordScoreInfo[i].iAreaJetton[wAreaIndex - 1] = (CT_INT32)m_llUserJettonScore[wAreaIndex][i]; //(CT_INT32)llUserAreaLWScore[i][wAreaIndex];
				m_llUserLastJettonScore[wAreaIndex][i] = m_llUserJettonScore[wAreaIndex][i];
			}
			
			if (!bNeedRecord)
			{
				bNeedRecord = true;
			}
		}

		//标识下局可以续注
		if (i != m_wBankerUser)
		{
			m_cbCanAddLastJetton[i] = 1;
		}
	}

	if (wBigWinnerChairID != INVALID_CHAIR)
	{
		m_GameEnd.cbBigWinnerHeadIndex = m_pGameDeskPtr->GetHeadIndex(wBigWinnerChairID);
		m_GameEnd.cbBigWinnerSex = m_pGameDeskPtr->GetUserSex(wBigWinnerChairID);
		m_GameEnd.dBigWinnerScore = llBigWinnerScore*TO_DOUBLE;
		_snprintf_info(m_GameEnd.szBigWinnerNickName, sizeof(m_GameEnd.szBigWinnerNickName), "%s", m_pGameDeskPtr->GetNickName(wBigWinnerChairID));
	}

	//控牌库存
	//m_llStorageControl += llSymtemLWScore;

	//庄家输赢分
	m_GameEnd.dBankerWinScore = (m_llBankerWinScore - llBankerRevenue)*TO_DOUBLE;

	//记录区域总输赢分
	m_GameEnd.dAreaLWScore[0] = m_GameEnd.dBankerWinScore;
	memcpy(m_llAreaTotalLWScore, m_GameEnd.dAreaLWScore, sizeof(m_llAreaTotalLWScore));

	//记录税收金额
	//m_llRevenueScore = (CT_LONGLONG)GameEnd.dRevenueScore*100;
	//玩家区域总输赢分
	memcpy(m_llUserAreaTotalLWScore, llUserAreaLWScore, sizeof(m_llUserAreaTotalLWScore));
	//椅子
	for (CT_WORD i = 0; i < MAX_SIT_POS; ++i)
	{
		//0-5:对应椅子玩家
		if (m_wSitList[i] == INVALID_CHAIR) continue; 
		m_GameEnd.dLWScore[i] = (llLWScore[m_wSitList[i]] - llUserRevenue[m_wSitList[i]]) *TO_DOUBLE;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			m_GameEnd.dUserAreaLWScore[i][wAreaIndex] = llUserAreaLWScore[m_wSitList[i]][wAreaIndex]*TO_DOUBLE;
		}
	}

	//无座
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		//6:无座玩家（包括自己）
		if (!m_pGameDeskPtr->IsExistUser(i))  continue;
		if (0 == m_wPlayStatus[i]) continue;
		if (GetUserSitPos(i) != INVALID_CHAIR) continue;

		m_GameEnd.dLWScore[6] += llLWScore[i]*TO_DOUBLE;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			m_GameEnd.dUserAreaLWScore[6][wAreaIndex] += llUserAreaLWScore[i][wAreaIndex]*TO_DOUBLE;
		}
	}

	//设置时间
	m_cbEndTimeInterval = TIME_GAME_END;
	if (!m_bHasUserAddJetton)
	{
		m_cbEndTimeInterval = TIME_GAME_END_NO_BET;
	}
	else
	{
		if (wBigWinnerChairID == INVALID_CHAIR)
		{
			m_cbEndTimeInterval = TIME_GAME_END_NO_BIGWINER;
		}
	}

	m_GameEnd.cbTimeLeave = m_cbEndTimeInterval;	//剩余时间

	//先记录结算类型
	CT_BYTE cbGameEndType = m_GameEnd.cbEndType;

	//发送数据
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
		{
			//玩家不存在
			continue;
		}

		//没有下注，将结算状态改为没有下注
		if (m_wPlayStatus[i] == 0)
		{
			m_GameEnd.cbEndType = en_No_Bet;
		}

		m_GameEnd.dLWScore[7] = (llLWScore[i] - llUserRevenue[i]) *TO_DOUBLE;// 7:自己
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			m_GameEnd.dUserAreaLWScore[7][wAreaIndex] = (llUserAreaLWScore[i][wAreaIndex] - llUserAreaRevenue[i][wAreaIndex])*TO_DOUBLE;
		}
		SendTableData(i, NN_SUB_S_GAME_END, &m_GameEnd, sizeof(NN_CMD_S_GameEnd), false);

		//还原结算状态
		if (m_wPlayStatus[i] == 0)
		{
			m_GameEnd.cbEndType = cbGameEndType;
		}
	}

	m_pGameDeskPtr->SetGameTimer(ID_TIME_GAME_END, m_cbEndTimeInterval * 1000);
	m_dwEndTime = (CT_DWORD)time(NULL) + m_cbEndTimeInterval;

	//写入积分
	//WriteGameScore();
	//记录基本信息
	CT_DWORD dwBankerUserID = 0;
	if (m_wBankerUser != INVALID_CHAIR)
	{
		dwBankerUserID = m_pGameDeskPtr->GetUserID(m_wBankerUser);
	}
	if (bNeedRecord)
	{
		m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, m_GameEnd.cbTableCarType, AREA_COUNT + 1, dwBankerUserID, m_llStorageControl, m_llAndroidStorage, m_llTodayStorageControl, m_wSystemAllKillRatio, m_wChangeCardRatio);
	}
	
	CalSystemOperateRation();
	/*LOG(WARNING) << "bai ren niu niu game: " << pGameRoomKind->wRoomKindID <<", total storage: " << (CT_LONGLONG)(m_llStorageControl*TO_DOUBLE) << ", total storage lower limit: "<< (CT_LONGLONG)(m_llStorageLowerLimit*TO_DOUBLE)
		<< ", today storage: " << (CT_LONGLONG)(m_llTodayStorageControl*TO_DOUBLE) << ", today high limit:" << (CT_LONGLONG)(m_llTodayStorageHighLimit*TO_DOUBLE) << ", all kill ratio: " << m_wSystemAllKillRatio <<", change card area count: " << m_wChangeCardRatio;*/
}

CT_LONGLONG CGameProcess::GetNextCaiJinChiOpenRew()
{
	/*
	CT_WORD wNum = rand() % 9;
	CT_WORD wPercent =(30 + 5 * wNum);
	CT_LONGLONG llOpenRewScore = (m_llCaiJinChiCount + (m_llCaiJinChiCount * wPercent / 100));
	return llOpenRewScore;
	*/
	return 0;
}

CT_WORD CGameProcess::OnUserBetScore(CT_WORD wChairID, CT_BYTE cbJettonArea, CT_WORD wJettonIndex, CT_BOOL bIsAndroid)
{
	/*LOG_IF(INFO,PRINT_GAME_INFO) << "OnUserBetScore: wChairID = " << wChairID << ",cbJettonArea = " << (int)cbJettonArea
		<< ",wJettonIndex =" << wJettonIndex <<",bIsAndroid = " << (int)bIsAndroid;*/
	if (m_wBankerUser == wChairID)
	{//庄家不能下注
		SendErrorCode(wChairID, ERROR_BANKE_NOT_BET);
		return 1;
	}

	if (wJettonIndex < 0  || wJettonIndex >= MAX_JETTON_NUM)
	{//下注筹码错误
		SendErrorCode(wChairID, ERROR_BET_SCORE);
		return 2;
	}

	if (cbJettonArea < 1 || cbJettonArea > AREA_COUNT)
	{
		//下注区域错误
		SendErrorCode(wChairID, ERROR_BET_AREA);
		return 3;
	}

	//玩家已下注金币
	CT_LONGLONG llJettonCount = GetUserAllAreaBetScore(wChairID);

	//玩家当前拥有金币
	CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);
	//玩家当前下注金币
	CT_LONGLONG llJettonScore = m_llSmallJetton[wJettonIndex];

	//校验金币是否足够下注
	//if (llUserScore < (llJettonCount + llJettonScore))
	if (llUserScore < llJettonScore || llUserScore < CAN_JETTON_SCORE)
	{
		SendErrorCode(wChairID, ERROR_SCORE_NOT_ENOUGH);
		return 4;
	}
	//闲家可下注金额=自身金额/最大牌型倍数（3）
	CT_LONGLONG llUserLimitScore = (llUserScore+ llJettonCount) / m_wMaxCardMulTiple;
	if ((llJettonCount + llJettonScore) > llUserLimitScore)
	{
		SendErrorCode(wChairID, ERROR_SCORE_LIMIT);
		return 5;
	}

	//当台面下注金额已达到庄家最大可下注金额时，闲家无法下注
	//闲家下注总金额不可超过庄家金币的3/1
	CT_LONGLONG llBankerLimitScore = GetBankerScore() / m_wMaxCardMulTiple;
	CT_LONGLONG llAreaLimitScore = 0;
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		llAreaLimitScore += GetAreaBetScore(wAreaIndex);
	}
	llAreaLimitScore += llJettonScore;
	if (llAreaLimitScore > llBankerLimitScore)
	{
		SendErrorCode(wChairID, ERROR_SCORE_LIMIT);
		return 6;
	}
	if (m_wPlayStatus[wChairID] == 0)
	{
		m_wPlayStatus[wChairID] = 1;
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
	m_pGameDeskPtr->AddUserScore(wChairID, -llJettonScore, true);

	AddAreaBetScore(cbJettonArea, llJettonScore, m_pGameDeskPtr->IsAndroidUser(wChairID));
	AddUserAreaBetScore(wChairID, cbJettonArea, llJettonScore);

	//消息处理
	NN_CMD_S_BetResult BetResult;
	memset(&BetResult, 0, sizeof(NN_CMD_S_BetResult));
	BetResult.wChairID = wChairID;									//用户位置
	BetResult.cbJettonArea = cbJettonArea;							//下注区域
	BetResult.wJettonIndex = wJettonIndex;							//下注索引
	BetResult.dBankerLimitScore = (llBankerLimitScore - llAreaLimitScore)*TO_DOUBLE; //庄家下注金额限制
	
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)	
	{
		BetResult.dAreaBetScore[wAreaIndex] = GetAreaBetScore(wAreaIndex)*TO_DOUBLE;//各区域总已下注金额
	}

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
		{//玩家不存在
			continue;
		}

		BetResult.dUserLimitScore = 0.0f;
		CT_LONGLONG llUserAllAreaBetScore = 0;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			CT_LONGLONG llUserAreaBetScore = GetUserAreaBetScore(i, wAreaIndex);
			BetResult.dUserBetScore[wAreaIndex] = llUserAreaBetScore*TO_DOUBLE;//玩家各区域已下注金额
			llUserAllAreaBetScore += llUserAreaBetScore;
			//BetResult.llUserLimitScore = ((m_pGameDeskPtr->GetUserScore(i)/ MAX_TYPE_MULTIPLE) - GetUserAllAreaBetScore(i)); //玩家下注金额限制
		}
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
		CT_LONGLONG llUserLimit = (llUserScore + llUserAllAreaBetScore) / m_wMaxCardMulTiple;

		if (llUserLimit > llUserAllAreaBetScore)
		{
			BetResult.dUserLimitScore = (llUserLimit - llUserAllAreaBetScore)*TO_DOUBLE;
		}
		SendTableData(i, NN_SUB_S_BET_SCORE_RESULT, &BetResult, sizeof(BetResult),false);
	}
	//LOG_IF(INFO, PRINT_GAME_INFO) << "OnUserBetScore: bet succ! wChairID = " << wChairID ;
	return 7;
}

CT_WORD CGameProcess::OnUserBetLastScore(CT_WORD wChairID)
{
	if (m_wBankerUser == wChairID)
	{
		//庄家不能下注
		SendErrorCode(wChairID, ERROR_BANKE_NOT_BET);
		LOG(WARNING) << "banker user can not bet last score, chair id:" << wChairID << ", user id: " << m_pGameDeskPtr->GetUserID(wChairID);
		return 1;
	}

	if (m_cbCanAddLastJetton[wChairID] == 0)
	{
		SendErrorCode(wChairID, ERROR_OPERATE);
		LOG(WARNING) << "user can not bet last score, chair id:" << wChairID << ", user id: " << m_pGameDeskPtr->GetUserID(wChairID);
		return 2;
	}

	m_cbCanAddLastJetton[wChairID] = 0;

	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		CT_LONGLONG llLastJettonScore = m_llUserLastJettonScore[wAreaIndex][wChairID];

		if (llLastJettonScore == 0)
			continue;

		 //玩家已下注金币
		 CT_LONGLONG llJettonCount = GetUserAllAreaBetScore(wChairID);

		 //玩家当前拥有金币
		 CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);

		 //玩家当前下注金币
		 CT_LONGLONG llJettonScore = llLastJettonScore;

		 //校验金币是否足够下注
		 if (llUserScore < llJettonScore || llUserScore < CAN_JETTON_SCORE)
		 {
			 SendErrorCode(wChairID, ERROR_SCORE_NOT_ENOUGH);
			 return 4;
		 }

		 //闲家可下注金额=自身金额/最大牌型倍数（3）
		 CT_LONGLONG llUserLimitScore = (llUserScore + llJettonCount) / m_wMaxCardMulTiple;
		 if ((llJettonCount + llJettonScore) > llUserLimitScore)
		 {
			 SendErrorCode(wChairID, ERROR_SCORE_LIMIT);
			 return 5;
		 }

		 //当台面下注金额已达到庄家最大可下注金额时，闲家无法下注
		 //闲家下注总金额不可超过庄家金币的3/1
		 CT_LONGLONG llBankerLimitScore = GetBankerScore() / m_wMaxCardMulTiple;
		 CT_LONGLONG llAreaLimitScore = 0;
		 for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		 {
			 llAreaLimitScore += GetAreaBetScore(wAreaIndex);
		 }
		 llAreaLimitScore += llJettonScore;
		 if (llAreaLimitScore > llBankerLimitScore)
		 {
			 SendErrorCode(wChairID, ERROR_SCORE_LIMIT);
			 return 6;
		 }

		 if (m_wPlayStatus[wChairID] == 0)
		 {
			 m_wPlayStatus[wChairID] = 1;
			 m_llUserSourceScore[wChairID] = m_pGameDeskPtr->GetUserScore(wChairID);
			 m_dwAllUserID[wChairID] = m_pGameDeskPtr->GetUserID(wChairID);
		 }

		 if (!m_bHasUserAddJetton)
		 {
			 m_bHasUserAddJetton = true;
		 }

		 //扣除玩家下注数
		 m_pGameDeskPtr->AddUserScore(wChairID, -llJettonScore, true);

		 AddAreaBetScore(wAreaIndex, llJettonScore, m_pGameDeskPtr->IsAndroidUser(wChairID));
		 AddUserAreaBetScore(wChairID, wAreaIndex, llJettonScore);

		 //消息处理
		 NN_CMD_S_LastBetResult BetResult;
		 memset(&BetResult, 0, sizeof(NN_CMD_S_LastBetResult));
		 BetResult.wChairID = wChairID;									//用户位置
		 BetResult.cbJettonArea = (CT_BYTE)wAreaIndex;					//下注区域
		 BetResult.dJettonScore = llJettonScore*TO_DOUBLE;				//下注金额
		 BetResult.dBankerLimitScore = (llBankerLimitScore - llAreaLimitScore)*TO_DOUBLE; //庄家下注金额限制

		 for (CT_WORD wAreaIndex1 = 1; wAreaIndex1 <= AREA_COUNT; ++wAreaIndex1)
		 {
			 BetResult.dAreaBetScore[wAreaIndex1] = GetAreaBetScore(wAreaIndex1)*TO_DOUBLE;//各区域总已下注金额
		 }

		 for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		 {
			 if (!m_pGameDeskPtr->IsExistUser(i))
			 {//玩家不存在
				 continue;
			 }

			 /*BetResult.dUserLimitScore = 0.0f;
			 CT_LONGLONG llUserLimit = m_llUserSourceScore[i] / MAX_TYPE_MULTIPLE;
			 CT_LONGLONG llUserAllAreaBetScore = GetUserAllAreaBetScore(i);
			 if (llUserLimit > llUserAllAreaBetScore)
			 {
				 BetResult.dUserLimitScore = (llUserLimit - llUserAllAreaBetScore)*TO_DOUBLE;
			 }
			 for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
			 {
				 BetResult.dUserBetScore[wAreaIndex] = GetUserAreaBetScore(i, wAreaIndex)*TO_DOUBLE;//玩家各区域已下注金额
			 }*/
			 BetResult.dUserLimitScore = 0.0f;
			 CT_LONGLONG llUserAllAreaBetScore = 0;
			 for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
			 {
				 CT_LONGLONG llUserAreaBetScore = GetUserAreaBetScore(i, wAreaIndex);
				 BetResult.dUserBetScore[wAreaIndex] = llUserAreaBetScore*TO_DOUBLE;//玩家各区域已下注金额
				 llUserAllAreaBetScore += llUserAreaBetScore;
				 //BetResult.llUserLimitScore = ((m_pGameDeskPtr->GetUserScore(i)/ MAX_TYPE_MULTIPLE) - GetUserAllAreaBetScore(i)); //玩家下注金额限制
			 }
			 CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
			 CT_LONGLONG llUserLimit = (llUserScore + llUserAllAreaBetScore) / m_wMaxCardMulTiple;

			 if (llUserLimit > llUserAllAreaBetScore)
			 {
				 BetResult.dUserLimitScore = (llUserLimit - llUserAllAreaBetScore)*TO_DOUBLE;
			 }
			 SendTableData(i, NN_SUB_S_ADD_LAST_BET, &BetResult, sizeof(BetResult), false);
		 }
	}

	return 7;
}

CT_WORD CGameProcess::OnUserApplyBanker(CT_WORD wChairID)
{
	//合法判断
	CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);
	if (llUserScore < m_llApplyBankerCondition)
	{
		SendErrorCode(wChairID, ERROR_APPLY_LIMIT);
		return 1;
	}
	//已做庄
	if (wChairID == m_wBankerUser)
	{
		SendErrorCode(wChairID, ERROR_IS_BANKER);
		return 2;
	}
	
	//存在判断
	CT_INT32 iApplyUserCount = (CT_INT32)m_ApplyUserArray.size();
	for (CT_WORD wIndex = 0; wIndex < iApplyUserCount; ++wIndex)
	{
		CT_WORD wApplyUserChairID = m_ApplyUserArray[wIndex];
		if (wChairID == wApplyUserChairID) //已申请
		{
			SendErrorCode(wChairID, ERROR_IS_APPLY);
			return 3;
		}
	}
	CT_BOOL bChangeBanker = false;
	if (m_wBankerUser == INVALID_CHAIR && iApplyUserCount == 0 && m_wGameStatus == GAME_STATUS_FREE) //是否第一个申请
	{
		//切换庄家
		m_wNextBankerUser = wChairID;
		bChangeBanker = true;
	}
	else
	{
		//保存信息,加入申请列表
		m_ApplyUserArray.push_back(wChairID);
	}

	//构造变量
	NN_CMD_S_BankerApplyOperate operate;
	memset(&operate, 0, sizeof(operate));
	operate.cbType = 0;
	operate.wOperateUser = wChairID;

	//发送消息
	SendTableData(wChairID, NN_SUB_S_BANKER_APPLY_OPERATE, &operate, sizeof(operate),false);
	//发送申请列表
	if (!bChangeBanker)
	{
		SendBankerApplyList(INVALID_CHAIR);
	}
	//发送庄家信息
	if (m_wGameStatus == GAME_STATUS_FREE)
	{
		SendChangeBanker(bChangeBanker, 0);
	}
	return 4;
}

CT_WORD CGameProcess::OnUserCancelApplyBanker(CT_WORD wChairID)
{
	//庄家取消
	if (wChairID == m_wBankerUser)
	{
		SendErrorCode(wChairID, ERROR_OPERATE);
		return 1;
	}

	//是否申请
	CT_BOOL bIsApply = false;

	//非庄家取消
	for (CT_WORD i = 0; i < m_ApplyUserArray.size(); ++i)
	{
		if (wChairID != m_ApplyUserArray[i])
		{
			continue;
		}
		bIsApply = true;
		//已申请,删除申请列表数据
		m_ApplyUserArray.erase(m_ApplyUserArray.begin() + i);
		break;
	}
	
	if (!bIsApply)
	{
		SendErrorCode(wChairID, ERROR_NOT_APPLY);
		return 2;
	}

	//构造变量
	NN_CMD_S_BankerApplyOperate operate;
	memset(&operate, 0, sizeof(operate));
	operate.cbType = 1;
	operate.wOperateUser = wChairID;

	//发送消息
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i)) continue;
		if (m_pGameDeskPtr->IsAndroidUser(i)) continue;
		
		SendTableData(i, NN_SUB_S_BANKER_APPLY_OPERATE, &operate, sizeof(operate), false);
	}	
	return 3;
}

//申请下庄
CT_WORD CGameProcess::OnUserCancelBanker(CT_WORD wChairID)
{
	if (wChairID != m_wBankerUser)
	{
		SendErrorCode(wChairID, ERROR_NOT_BANKER);
		return 1;
	}
	CT_BOOL bIsChangeBanker = false;

	//申请列表是否有人
	if (m_ApplyUserArray.empty())
	{
		//系统当庄
		m_wNextBankerUser = INVALID_CHAIR;
	}
	else
	{
		//切换庄家,删除列表
		m_wNextBankerUser = m_ApplyUserArray[0];
		m_ApplyUserArray.erase(m_ApplyUserArray.begin());
	}

	//游戏中保存信息，游戏结束切换庄家
	if (m_wGameStatus == GAME_STATUS_FREE)
	{
		bIsChangeBanker = true;
	}
	else
	{
		m_bIsApplyCancelBanker = true;
	}

	NN_CMD_S_CancelBanker cancelBanker;
	memset(&cancelBanker, 0, sizeof(cancelBanker));
	cancelBanker.wCancelUser = wChairID;
	//发送消息
	SendTableData(wChairID, NN_SUB_S_APPLY_CANCEL_BANKER, &cancelBanker, sizeof(cancelBanker), false);
	//切换庄家
	SendChangeBanker(bIsChangeBanker, 2);
	return 2;
}

CT_BOOL CGameProcess::SendChangeBanker(CT_BOOL bChangeBanker, CT_BYTE cbReason)
{
	//广播庄家信息
	if (bChangeBanker)
	{
		//LOG_IF(INFO,GAME_PLAYER) << "SendChangeBanker: oldBanker = "<< m_wBankerUser << ",newBanker = "<< m_wNextBankerUser;
		m_wBankerCount = 1;
		CT_WORD wLastBankerUser = m_wBankerUser;
		m_wBankerUser = m_wNextBankerUser;
		m_wNextBankerUser = INVALID_CHAIR; 
		m_llBankerTotalWinScore = 0;

		NN_CMD_S_ChangeBanker changeBanker;
		memset(&changeBanker, 0, sizeof(changeBanker));
		changeBanker.wBankerUser = m_wBankerUser;				//当庄玩家
		changeBanker.dBankerScore = GetBankerScore()*TO_DOUBLE;	//庄家金币
		changeBanker.wLastBankerUser = wLastBankerUser;
		changeBanker.cbReason = cbReason;

		//发送消息
		SendTableData(INVALID_CHAIR, NN_SUB_S_CHANGE_BANKER, &changeBanker, sizeof(changeBanker), false);
		return true;
	}
	return false;
}

CT_BOOL CGameProcess::SendUserSitList(CT_WORD wChairID)
{
	NN_CMD_S_UserSitList userSitList;
	/*memset(&userSitList, 0, sizeof(userSitList));
	for (CT_WORD pos = 0; pos < MAX_SIT_POS; ++pos)
	{
		if (m_wSitList[pos] == INVALID_CHAIR) continue;
		if (m_pGameDeskPtr->IsExistUser(m_wSitList[pos]))
		{
			CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(m_wSitList[pos]);
			if (llUserScore >= SIT_SCORE_LIMIT) continue;
		}
		m_wSitList[pos] = INVALID_CHAIR; //椅子上玩家金币不足或者退出了，更新椅子信息
	}*/

	//座椅信息	
	for (CT_WORD pos = 0; pos < MAX_SIT_POS; ++pos)
	{
		userSitList.wSitList[pos] = m_wSitList[pos];
		//LOG(WARNING) << "pos user: " << userSitList.wSitList[pos];
	}

	//发送消息
	SendTableData(wChairID, NN_SUB_S_USER_SIT_LIST, &userSitList, sizeof(userSitList), false);
	return true;
}

CT_BOOL CGameProcess::SendBankerApplyList(CT_WORD wChairID)
{
	for (std::vector<CT_WORD>::iterator it = m_ApplyUserArray.begin(); it != m_ApplyUserArray.end();)
	{
		CT_WORD wUserId = *it;
		if (!m_pGameDeskPtr->IsExistUser(wUserId))
		{
			it = m_ApplyUserArray.erase(it);
		}
		else
		{
			++it;
		}
	}
	NN_CMD_S_Banker_Apply_List bankerApplyList;
	memset(&bankerApplyList, 0, sizeof(bankerApplyList));
	for (CT_WORD i = 0; i < GAME_PLAYER;  ++i)
	{
		bankerApplyList.wBankerApplyList[i] = INVALID_CHAIR;
	}

	for (CT_WORD i = 0; i < m_ApplyUserArray.size(); ++i)
	{
		++bankerApplyList.dwBankerApplyCount;
		bankerApplyList.wBankerApplyList[i] = m_ApplyUserArray[i];				//庄家申请列表
	}

	//发送消息
	if (wChairID == INVALID_CHAIR)
	{
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
			{//玩家不存在
				continue;
			}
			SendTableData(i, NN_SUB_S_BANKER_APPLY_LIST, &bankerApplyList, sizeof(bankerApplyList), false);
		}
	}
	else
	{
		SendTableData(wChairID, NN_SUB_S_BANKER_APPLY_LIST, &bankerApplyList, sizeof(bankerApplyList), false);
	}
	return true;
}

CT_WORD CGameProcess::OnUserSit(CT_WORD wChairID, CT_BYTE cbType, CT_BYTE cbPos)
{
	/*CT_LONGLONG dwScore = m_pGameDeskPtr->GetUserScore(wChairID);
	//条件不满足
	if (dwScore < SIT_SCORE_LIMIT)
	{
		SendErrorCode(wChairID, ERROR_SIT_LIMIT);
		return 1;
	}*/

	//位置上是否有人
	/*if (m_wSitList[cbPos] != INVALID_CHAIR)
	{
		SendErrorCode(wChairID, ERROR_SIT_NOT_EMPTY);
		return 2;
	}*/

	//位置上是否有人
	if (cbType == 0)//坐下
	{
		//如果原位置有人,则将位置的人先下桌
		if (m_wSitList[cbPos] != INVALID_CHAIR)
		{
			//SendErrorCode(wChairID, ERROR_SIT_NOT_EMPTY);
			//return 2;

			m_wSitList[cbPos] = INVALID_CHAIR;
		}
	}
	else if (cbType == 1)//离开,起来
	{
		if (m_wSitList[cbPos] != wChairID)
		{
			SendErrorCode(wChairID, ERROR_OPERATE);
			return 21;
		}
	}

	CT_WORD wUserPos = GetUserSitPos(wChairID);
	if (cbType == 0) //坐下
	{
		if (wUserPos == INVALID_CHAIR)
		{
			m_wSitList[cbPos] = wChairID;
		}
		else
		{
			//交换位置
			m_wSitList[wUserPos] = INVALID_CHAIR;
			m_wSitList[cbPos] = wChairID;
		}
	}
	else//起来
	{
		m_wSitList[cbPos] = INVALID_CHAIR;
	}


	/*NN_CMD_S_SitResult sitResult;
	memset(&sitResult, 0, sizeof(sitResult));

	sitResult.cbType = cbType;										//0:坐下，1：起来
	sitResult.wOperateUser = wChairID;								//坐/起玩家
	sitResult.cbSitPos = cbPos;										//位置

	//发送消息
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
		{//玩家不存在
			continue;
		}
		SendTableData(i, NN_SUB_S_SIT, &sitResult, sizeof(sitResult), false);
	}*/
	
	return 7;
}

//查询玩家的近20局游戏数据
CT_BOOL CGameProcess::OnQueryUserGameHistory(CT_WORD wChairID)
{
	CT_WORD		wLukyUser = 0;
	CT_BYTE		cbMaxWinCount = 0;
	CT_DWORD	dwLukyJettonSocre = 0;

	std::vector<tagUserHistoryEx> vecUserHistory;
	tagUserHistoryEx historySum;
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
			dwJettonScore = (CT_DWORD)(dwJettonScore/100);

			historySum.cbWinCount = cbWinCount;
			historySum.dwJettonScore = dwJettonScore;
			historySum.wChairID = i;
			if (cbWinCount > cbMaxWinCount)
			{
				wLukyUser = i;
				cbMaxWinCount = cbWinCount;
				dwLukyJettonSocre = dwJettonScore;
			}

			vecUserHistory.push_back(historySum);
		}
	}

	std::sort(vecUserHistory.begin(), vecUserHistory.end(), comp_by_jetton);

	CT_BYTE  szBuffer[2048];
	CT_DWORD dwSendSize = 0;
	//
	NN_CMD_S_UserPlayHistory history;
	history.dwUserID = m_pGameDeskPtr->GetUserID(wLukyUser);
	history.cbgender = m_pGameDeskPtr->GetUserSex(wLukyUser);
	history.cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(wLukyUser);
	history.cbWinCount = cbMaxWinCount;
	history.dwTotalJetton = dwLukyJettonSocre;
	memcpy(szBuffer, &history, sizeof(NN_CMD_S_UserPlayHistory));
	dwSendSize += sizeof(NN_CMD_S_UserPlayHistory);

	for (auto& it : vecUserHistory)
	{
		if (it.wChairID == wLukyUser)
			continue;
	
		history.dwUserID = m_pGameDeskPtr->GetUserID(it.wChairID);
		history.cbgender = m_pGameDeskPtr->GetUserSex(it.wChairID);
		history.cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(it.wChairID);
		history.cbWinCount = it.cbWinCount;
		history.dwTotalJetton = it.dwJettonScore;
		memcpy(szBuffer + dwSendSize, &history, sizeof(NN_CMD_S_UserPlayHistory));
		dwSendSize += sizeof(NN_CMD_S_UserPlayHistory);
	}
	SendTableData(wChairID, NN_SUB_S_QUERY_USER_HISTORY, szBuffer, dwSendSize);

	return true;
}

CT_BOOL CGameProcess::SendErrorCode(CT_WORD wChairID, enNNErrorCode enCodeNum)
{
	NN_CMD_S_ErrorCode errorCode;
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
			SendTableData(i, NN_SUB_S_OPERATE_FAIL, &errorCode, sizeof(errorCode),false);
		}
	}
	else
	{
		SendTableData(wChairID, NN_SUB_S_OPERATE_FAIL, &errorCode, sizeof(errorCode), false);
	}
	
	return true;
}

CT_BOOL CGameProcess::SendCaiJinChiChampionInfo(CT_WORD wChairID)
{
	/*
	NN_CMD_S_CaiJinChiResult CaiJinChiResult;
	memset(&CaiJinChiResult, 0, sizeof(CaiJinChiResult));
	CaiJinChiResult.cbVipLevel = m_cbChampionVipLevel;
	CaiJinChiResult.dCaiJinChiRew = m_llChampionRew*TO_DOUBLE;
	CaiJinChiResult.dwUserID = m_dwUserID;
	memcpy(CaiJinChiResult.szHeadUrl, m_szChampionHeadUrl,sizeof(CaiJinChiResult.szHeadUrl));
	memcpy(CaiJinChiResult.szNickName, m_szChampionNickName, sizeof(CaiJinChiResult.szNickName));
	//发送消息
	if (wChairID == INVALID_CHAIR)
	{
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
			{//玩家不存在
				continue;
			}
			SendTableData(i, NN_SUB_S_QUERY_CAIJINCHI, &CaiJinChiResult, sizeof(CaiJinChiResult), false);
		}
	}
	else
	{
		SendTableData(wChairID, NN_SUB_S_QUERY_CAIJINCHI, &CaiJinChiResult, sizeof(CaiJinChiResult), false);
	}
	*/
	return true;
}

//更新座椅信息
CT_VOID CGameProcess::UpdateUserSitInfo()
{
	//首先让所有无座玩家起立
	for (auto i = 0; i != MAX_SIT_POS; ++i)
	{
		CT_WORD wChairID = m_wSitList[i];
		if (wChairID != INVALID_CHAIR)
		{
			OnUserSit(wChairID, 1, i);
		}
	}

	std::vector<tagChairUserScore> vecUserScore;
	tagChairUserScore userScore;
	//把当前玩家的分数进行排序
	for (auto i = 0; i != GAME_PLAYER; ++i)
	{
		if (m_pGameDeskPtr->IsExistUser(i))
		{
			userScore.wChairID = i;
			userScore.llScore = m_pGameDeskPtr->GetUserScore(i);
			vecUserScore.push_back(userScore);
		}
	}

	//根据玩家的积分进行排序
	std::sort(vecUserScore.begin(), vecUserScore.end(), comp_by_score);

	//按高到低上桌
	int nSitPos = 0;
	for (auto& it : vecUserScore)
	{
		OnUserSit(it.wChairID, 0, nSitPos++);

		if (nSitPos >= MAX_SIT_POS)
		{
			break;
		}
	}

	SendUserSitList(INVALID_CHAIR);
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
	m_llUserRoundTotalJettonScore[wChairID] += llAddScore;
}

CT_LONGLONG CGameProcess::GetUserAllAreaBetScore(CT_WORD wChairID)
{
	CT_LONGLONG llJettonCount = 0;
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		llJettonCount += GetUserAreaBetScore(wChairID, wAreaIndex);
	}
	return llJettonCount;
}

CT_LONGLONG CGameProcess::GetAreaBetScore(CT_WORD wAreaIndex)
{
	return m_llAllJettonScore[wAreaIndex];
}

void CGameProcess::AddAreaBetScore(CT_WORD wAreaIndex,CT_LONGLONG llAddScore, CT_BOOL bAndroid)
{
	m_llAllJettonScore[wAreaIndex] += llAddScore;
	
	if (!bAndroid)
	{
		m_llRealUserJettonScore[wAreaIndex] += llAddScore;
	}
}

CT_LONGLONG CGameProcess::GetBankerScore()
{
	CT_LONGLONG llBankerScore = SYSTEM_BANKER_SCORE;
	if (INVALID_CHAIR != m_wBankerUser)
	{
		llBankerScore = m_pGameDeskPtr->GetUserScore(m_wBankerUser);
	}

	return llBankerScore;
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
	//随机扑克发牌
	//CT_BYTE cbTempArray[MAX_CARD_TOTAL] = { 0 };
	m_GameLogic.RandCardData(m_cbRepertoryCard, sizeof(m_cbRepertoryCard));
	//m_cbRepertoryCard[0] = 0x02;
    //m_cbRepertoryCard[1] = 0x03;
    //m_cbRepertoryCard[2] = 0x04;
    //m_cbRepertoryCard[3] = 0x05;
    //m_cbRepertoryCard[4] = 0x16;
	
	for (CT_WORD wAreaIndex = 0; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		//CT_BYTE cbTempCard[MAX_COUNT] = { 0 };
		//桌面牌数据
		memcpy(&m_cbTableCardArray[wAreaIndex], &m_cbRepertoryCard[wAreaIndex*MAX_COUNT], sizeof(CT_BYTE)*MAX_COUNT);
		m_cbSendCardCount += MAX_COUNT;
		//桌面牌数据
		//memcpy(&m_cbTableCardArray[wAreaIndex], cbTempCard, sizeof(CT_BYTE)*MAX_COUNT);
	}
}

CT_VOID CGameProcess::GetBombCard(CT_BYTE cbBombCard[])
{
	//炸弹牌型
	CT_BYTE cbBombCardArray[13][5] = {
		{ 0x01,0x11,0x21,0x31,0x05 },
		{ 0x02,0x12,0x22,0x32,0x0B },
		{ 0x03,0x13,0x23,0x33,0x26 },
		{ 0x04,0x14,0x24,0x34,0x1C },
		{ 0x05,0x15,0x25,0x35,0x07 },
		{ 0x06,0x16,0x26,0x36,0x05 },
		{ 0x07,0x17,0x27,0x37,0x33 },
		{ 0x08,0x18,0x28,0x38,0x05 },
		{ 0x09,0x19,0x29,0x39,0x24 },
		{ 0x0A,0x1A,0x2A,0x3A,0x15 },
		{ 0x0B,0x1B,0x2B,0x3B,0x05 },
		{ 0x0C,0x1C,0x2C,0x3C,0x28 },
		{ 0x0D,0x1D,0x2D,0x3D,0x05 },
	};
	CT_BYTE cbIndex = rand() % 13;
	memcpy(cbBombCard, cbBombCardArray[cbIndex],sizeof(CT_BYTE)*MAX_COUNT);
	return;
}

CT_VOID CGameProcess::GetFiveSmallCard(CT_BYTE cbBombCard[])
{
	//五小牛牌型
	CT_BYTE cbBombCardArray[5][5] = {
		{ 0x01,0x11,0x21,0x03,0x04 },
		{ 0x11,0x02,0x12,0x31,0x04 },
		{ 0x01,0x02,0x23,0x11,0x03 },
		{ 0x21,0x12,0x01,0x22,0x34 },
		{ 0x01,0x11,0x03,0x21,0x14 },
	};
	CT_BYTE cbIndex = rand() % 5;
	memcpy(cbBombCard, cbBombCardArray[cbIndex], sizeof(CT_BYTE)*MAX_COUNT);
	return ;
}

CT_VOID CGameProcess::GetGoldenBullCard(CT_BYTE cbBombCard[])
{
	//五花牛牌型
	CT_BYTE cbBombCardArray[10][5] = {
		{ 0x0B,0x0C,0x0D,0x1B,0x2B },
		{ 0x0B,0x0C,0x0D,0x1C,0x2C },
		{ 0x0B,0x0C,0x0D,0x1D,0x2D },
		{ 0x1B,0x0C,0x0D,0x3B,0x2B },
		{ 0x0B,0x1C,0x0D,0x1D,0x2B },
		{ 0x3B,0x0C,0x2D,0x1B,0x2B },
		{ 0x3B,0x0C,0x3D,0x1B,0x2B },
		{ 0x0B,0x0C,0x0D,0x1D,0x2C },
		{ 0x0B,0x0C,0x1C,0x2C,0x3C },
		{ 0x0D,0x0C,0x1D,0x2D,0x2B },
	};
	CT_BYTE cbIndex = rand() % 10;
	memcpy(cbBombCard, cbBombCardArray[cbIndex], sizeof(CT_BYTE)*MAX_COUNT);
	return ;
}

CT_BOOL CGameProcess::ResetCaiJinChi()
{
	/*
	tm	tm_now;
	time_t dwNow = time(NULL);
	
#ifdef _WIN32
	::localtime_s(&tm_now, &dwNow);
#else//WIN32
	struct tm* tmp = ::localtime(&dwNow);
	memcpy(&tm_now, tmp, sizeof(struct tm));
#endif

	if (!m_bResetCaiJinChi)
	{//今天还没衰减过，且到0点了
		if (tm_now.tm_hour == 0 && m_llCaiJinChiCount > BASE_CAIJINCHI_NUM)
		{
			CT_LONGLONG llSubVal = m_llCaiJinChiCount * m_wCaiJinChiMaxPercent / 1000;
			if ((m_llCaiJinChiCount - llSubVal) > BASE_CAIJINCHI_NUM)
			{
				m_llCaiJinChiCount -= llSubVal;
			}
			m_bResetCaiJinChi = true;
		}
	}
	else
	{
		if (tm_now.tm_hour != 0)
		{
			m_bResetCaiJinChi = false;
		}
	}
	*/
	return true;
}

CT_VOID CGameProcess::CalSystemBankerAreaWinUserScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT], std::vector<tagAreaWinScore>& vecAreaWinScore, CT_LONGLONG& llSystemWinScore)
{
	tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
	CT_INT32 iMultiple[AREA_COUNT + 1] = { 0 };//倍数
	for (CT_WORD i = 0; i < AREA_COUNT + 1; i++)
	{
		iMultiple[i] = m_GameLogic.GetMultiple(cbTempCardArray[i], (CT_BYTE)pGameRoomKind->wRoomKindID);
	}
	
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		//庄赢
		if (m_GameLogic.CompareCard(cbTempCardArray[0], cbTempCardArray[wAreaIndex]))
		{
			tagAreaWinScore areaWinScore;
			areaWinScore.cbAreaIndex = (CT_BYTE)wAreaIndex;
			
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (m_pGameDeskPtr->IsAndroidUser(i)) continue;

				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					CT_LONGLONG llUserLostScore = m_llUserJettonScore[wAreaIndex][i] * iMultiple[0] * m_dwCellScore;
					areaWinScore.llWinScore += llUserLostScore;
					llSystemWinScore += llUserLostScore;
				}
			}
			vecAreaWinScore.push_back(areaWinScore);
		}
		else //庄输
		{
			tagAreaWinScore areaWinScore;
			areaWinScore.cbAreaIndex = (CT_BYTE)wAreaIndex;
			
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (m_pGameDeskPtr->IsAndroidUser(i)) continue;

				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					CT_LONGLONG llUserWinScore = m_llUserJettonScore[wAreaIndex][i] * iMultiple[wAreaIndex] * m_dwCellScore;
					areaWinScore.llWinScore -= llUserWinScore;
					llSystemWinScore -= llUserWinScore;
				}
			}
			vecAreaWinScore.push_back(areaWinScore);
		}
	}
}

CT_LONGLONG CGameProcess::CalSystemBankerWinUserScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT])
{
	tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
	CT_LONGLONG	llTotalWinScore = 0;
	CT_INT32 iMultiple[AREA_COUNT + 1] = { 0 };//倍数
	for (CT_WORD i = 0; i < AREA_COUNT + 1; i++)
	{
		iMultiple[i] = m_GameLogic.GetMultiple(cbTempCardArray[i], (CT_BYTE)pGameRoomKind->wRoomKindID);
	}
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		//庄赢
		if (m_GameLogic.CompareCard(cbTempCardArray[0], cbTempCardArray[wAreaIndex]))
		{
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (m_pGameDeskPtr->IsAndroidUser(i)) continue;

				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					llTotalWinScore += m_llUserJettonScore[wAreaIndex][i] * iMultiple[0] * m_dwCellScore;
				}
			}
		}
		else //庄输
		{
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (m_pGameDeskPtr->IsAndroidUser(i)) continue;

				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					llTotalWinScore -= m_llUserJettonScore[wAreaIndex][i] * iMultiple[wAreaIndex] * m_dwCellScore;
				}
			}
		}
	}

	return llTotalWinScore;
}

CT_VOID CGameProcess::CalUserBankerAreaWinSystemScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT], std::vector<tagAreaWinScore>& vecAreaWinScore)
{
	tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
	CT_INT32 iMultiple[AREA_COUNT + 1] = { 0 };//倍数
	for (CT_WORD i = 0; i < AREA_COUNT + 1; i++)
	{
		iMultiple[i] = m_GameLogic.GetMultiple(cbTempCardArray[i], (CT_BYTE)pGameRoomKind->wRoomKindID);
	}
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		//庄赢
		if (m_GameLogic.CompareCard(cbTempCardArray[0], cbTempCardArray[wAreaIndex]))
		{
            tagAreaWinScore areaWinScore;
            areaWinScore.cbAreaIndex = (CT_BYTE)wAreaIndex;
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (!m_pGameDeskPtr->IsAndroidUser(i)) continue;

				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					areaWinScore.llWinScore += m_llUserJettonScore[wAreaIndex][i] * iMultiple[0] * m_dwCellScore;
				}
			}
            vecAreaWinScore.push_back(areaWinScore);
		}
		else //庄输
		{
            tagAreaWinScore areaWinScore;
            areaWinScore.cbAreaIndex = (CT_BYTE)wAreaIndex;
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (!m_pGameDeskPtr->IsAndroidUser(i)) continue;
    
    
				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					areaWinScore.llWinScore -= (m_llUserJettonScore[wAreaIndex][i] * iMultiple[wAreaIndex] * m_dwCellScore);
				}
			}
            vecAreaWinScore.push_back(areaWinScore);
		}
	}
}

CT_LONGLONG CGameProcess::CalUserBankerWinSystemScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT])
{
	tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
	CT_LONGLONG	llTotalLWScore = 0; 
	CT_INT32 iMultiple[AREA_COUNT + 1] = { 0 };//倍数
	for (CT_WORD i = 0; i < AREA_COUNT + 1; i++)
	{
		iMultiple[i] = m_GameLogic.GetMultiple(cbTempCardArray[i], (CT_BYTE)pGameRoomKind->wRoomKindID);
	}
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		//庄赢
		if (m_GameLogic.CompareCard(cbTempCardArray[0], cbTempCardArray[wAreaIndex]))
		{
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (!m_pGameDeskPtr->IsAndroidUser(i)) continue;

				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					llTotalLWScore += m_llUserJettonScore[wAreaIndex][i] * iMultiple[0] * m_dwCellScore;
				}
			}
		}
		else //庄输
		{
			for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
			{
				if (0 == m_wPlayStatus[i]) continue;

				if (m_wBankerUser == i) continue;

				if (!m_pGameDeskPtr->IsAndroidUser(i)) continue;

				if (m_llUserJettonScore[wAreaIndex][i] > 0)
				{
					llTotalLWScore -= m_llUserJettonScore[wAreaIndex][i] * iMultiple[wAreaIndex] * m_dwCellScore;
				}
			}
		}
	}

	return llTotalLWScore;
}

//计算换牌概率
CT_VOID CGameProcess::CalSystemOperateRation(CT_LONGLONG llSystemWinScore /*= 0*/)
{
	/* e.g.
	 * -1000 200
	 * 1000  190
	 * 2000  180
	 * 小于等于-1000的换牌概率是200
	 * 大于-1000小于等于2000的换牌概率是190
	 * 大于2000的换牌概率是190
	 */
	
	for (size_t i = 0; i != m_vecSystemOperateRatio.size(); ++i)
	{
		if (i == m_vecSystemOperateRatio.size()-1)
		{
			m_wChangeCardRatio = m_vecSystemOperateRatio[i].wChangeCardRatio;
			m_wSystemAllKillRatio = m_vecSystemOperateRatio[i].wSystemAllKillRatio;
			return;
		}
		
		if ((m_llTodayStorageControl + llSystemWinScore) <=  m_vecSystemOperateRatio[0].llTotayStock)
		{
            m_wChangeCardRatio = m_vecSystemOperateRatio[0].wChangeCardRatio;
			m_wSystemAllKillRatio = m_vecSystemOperateRatio[0].wSystemAllKillRatio;
			return;
		}
		
		if ((m_llTodayStorageControl + llSystemWinScore) > m_vecSystemOperateRatio[i].llTotayStock && (m_llTodayStorageControl + llSystemWinScore) <= m_vecSystemOperateRatio[i+1].llTotayStock)
		{
            m_wChangeCardRatio = m_vecSystemOperateRatio[i+1].wChangeCardRatio;
			m_wSystemAllKillRatio = m_vecSystemOperateRatio[i+1].wSystemAllKillRatio;
			return;
		}
	}
}

CT_VOID CGameProcess::CalTenPointArea()
{
    //计算前面两张牌的点数
    for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
    {
        CT_BYTE cbTotalPoint = 0;
        for (int j = 0; j < FIRST_CARD; j++)
        {
            CT_BYTE cbCard = m_cbTableCardArray[wAreaIndex][j];
            CT_BYTE cbPoint = m_GameLogic.GetLogicValue(cbCard);
            cbTotalPoint += cbPoint;
        }
        
        if (cbTotalPoint % 10 == 0)
        {
            m_FlashArea.cbFlashArea[wAreaIndex] = 1;
        }
    }
}

CT_BOOL CGameProcess::StorageControl()
{
	if (m_wBankerUser == INVALID_CHAIR)
	{
		//CT_BYTE  cbFlag = 0;
		int nRandNum = rand() % 1000;
		if (nRandNum > m_wSystemAllKillRatio)
		{
			return true;
		}
		
		CT_WORD wMaxTypeArea = 0;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			if (!m_GameLogic.CompareCard(m_cbTableCardArray[wMaxTypeArea], m_cbTableCardArray[wAreaIndex]))
			{
				wMaxTypeArea = wAreaIndex;//找出最大牌跟庄家调换
			}
		}
		
		//如果本来就是庄家通杀, 则不需要换牌
		if (wMaxTypeArea == 0)
		{
			return true;
		}
		
		LOG(WARNING) << "change system banker to all kill!!!";
		m_bCheatAllKill = true;
		//最大牌换给系统庄家
		CT_BYTE cbTempCardArray[MAX_COUNT] = { 0 };
		memcpy(cbTempCardArray, m_cbTableCardArray[0], sizeof(cbTempCardArray));
		memcpy(m_cbTableCardArray[0], m_cbTableCardArray[wMaxTypeArea], sizeof(m_cbTableCardArray[0]));
		memcpy(m_cbTableCardArray[wMaxTypeArea], cbTempCardArray, sizeof(m_cbTableCardArray[wMaxTypeArea]));
	}
	//真人做庄
	else 
	{
		CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT];
		memcpy(cbTempCardArray, m_cbTableCardArray, sizeof(cbTempCardArray));
		CT_LONGLONG	llTotalLWScore = CalUserBankerWinSystemScore(cbTempCardArray);

		//如果系统输分大于库存值
		if ((-llTotalLWScore + m_llAndroidStorage <= m_llAndroidStorageLowerLimit) 
			|| (-llTotalLWScore + m_llStorageControl <= m_llStorageLowerLimit))
		{
			std::vector<tagAreaWinScore> vecAreaWinScore;
			CalUserBankerAreaWinSystemScore(cbTempCardArray, vecAreaWinScore);
			std::sort(vecAreaWinScore.begin(), vecAreaWinScore.end(), comp_by_score3);

			for (auto& it : vecAreaWinScore)
			{
				//真人输钱不用换牌
				if (it.llWinScore <= 0) continue;

				//如果真人庄家赢，则考虑换牌
				CT_BYTE cbAreaIndex = it.cbAreaIndex;
				if (m_GameLogic.CompareCard(cbTempCardArray[0], cbTempCardArray[cbAreaIndex]))
				{
					CT_BYTE cbTempAreaCard[MAX_COUNT];
					memcpy(cbTempAreaCard, cbTempCardArray[0], MAX_COUNT);
					memcpy(cbTempCardArray[0], cbTempCardArray[cbAreaIndex], MAX_COUNT);
					memcpy(cbTempCardArray[cbAreaIndex], cbTempAreaCard, MAX_COUNT);
					LOG(WARNING) << "change banker user poker!";
					llTotalLWScore = CalUserBankerWinSystemScore(cbTempCardArray);
					memcpy(m_cbTableCardArray, cbTempCardArray, sizeof(m_cbTableCardArray));
					if ((-llTotalLWScore + m_llAndroidStorage >= m_llAndroidStorageLowerLimit) 
						&& (-llTotalLWScore + m_llStorageControl >= m_llStorageLowerLimit))
					{
						LOG(WARNING) << "change banker user poker succ! banker user: " << m_wBankerUser;
						break;
					}
				}
			}
		}
		
		return true;
	}
	return false;
}

CT_BOOL CGameProcess::StoreageControlForAllKill()
{
	//CT_INT32 iMultiple[AREA_COUNT + 1] = { 0 };//倍数
	if (m_wBankerUser == INVALID_CHAIR)
	{
		CT_BYTE cbFlag = 0;					//0:不控牌，1：通杀

		//随机是否需要通杀或者库存低于库存
		int nRandNum = rand() % 1000;
		if (nRandNum < m_wSystemAllKillRatio /*|| m_llStorageControl < m_llStorageLowerLimit*/)
		{
			cbFlag = 1;
		}
		
		//不通杀是否考虑换牌
		if (cbFlag == 0)
		{
		    //当今日库存到达上限，则不考虑再换牌
		    if (m_llTodayStorageControl < m_llTodayStorageHighLimit)
            {
                //没有通杀则要考虑是否降低前两张牌是10的倍数赢的几率
                for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
                {
                    //系统输
                    if (!m_GameLogic.CompareCard(m_cbTableCardArray[0], m_cbTableCardArray[wAreaIndex]))
                    {
                        //降低出现10的倍数的区域
                        //前面两张牌的点数
                        CT_BYTE cbTotalPoint = 0;
                        for (int j = 0; j < FIRST_CARD; j++)
                        {
                            CT_BYTE cbCard = m_cbTableCardArray[wAreaIndex][j];
                            CT_BYTE cbPoint = m_GameLogic.GetLogicValue(cbCard);
                            cbTotalPoint += cbPoint;
                        }
            
                        if (cbTotalPoint % 10 == 0)
                        {
                            m_FlashArea.cbFlashArea[wAreaIndex] = 1;
                            //百分之十的几率降低这个区域的胜率
                            int nRandomNum = rand() % 1000;
                            if (nRandomNum < m_wChangeCardRatio)
                            {
								LOG(WARNING) << "area " << wAreaIndex << " is ten point.";
                                bool bChangeSucc = false;
                                for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
                                {
                                    if (m_cbRepertoryCard[n] == 0)
                                    {
                                        continue;
                                    }
                                    
                                    for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
                                    {
										//拷贝这个区域的牌
										CT_BYTE cbTempCardArray[MAX_COUNT] = { 0 };
										memcpy(cbTempCardArray, m_cbTableCardArray[wAreaIndex], MAX_COUNT);
										
                                    	CT_BYTE cbTempCardData = cbTempCardArray[nCardIndex];
										cbTempCardArray[nCardIndex] = m_cbRepertoryCard[n];
                                        if (m_GameLogic.CompareCard(m_cbTableCardArray[0], cbTempCardArray))
                                        {
                                            m_cbRepertoryCard[n] = cbTempCardData;
                                            memcpy(m_cbTableCardArray[wAreaIndex], cbTempCardArray, MAX_COUNT);
                                            LOG(WARNING) << "change ten point card. << area index: " << wAreaIndex;
                                            bChangeSucc = true;
                                            break;
                                        }
                                    }
                        
                                    if (bChangeSucc)
                                        break;
                                }
    
                                //如果换闲家的牌没有换成功,考虑把庄家的牌换大
                                if (!bChangeSucc)
                                {
                                    for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
                                    {
                                        if (m_cbRepertoryCard[n] == 0)
                                        {
                                            continue;
                                        }
            
                                        for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
                                        {
                                            //拷贝这个区域的牌
                                            CT_BYTE  cbTempCardAreaArray[MAX_COUNT] = {0};
                                            memcpy(cbTempCardAreaArray, m_cbTableCardArray[0], MAX_COUNT);
                
                                            CT_BYTE cbTempCardData = cbTempCardAreaArray[nCardIndex];
                                            cbTempCardAreaArray[nCardIndex] = m_cbRepertoryCard[n];
                                            if (m_GameLogic.CompareCard(cbTempCardAreaArray, m_cbTableCardArray[wAreaIndex]))
                                            {
                                                m_cbRepertoryCard[n] = cbTempCardData;
                                                memcpy(m_cbTableCardArray[0], cbTempCardAreaArray, MAX_COUNT);
                                                LOG(WARNING) << "change banker card more than ten point area card.";
                                                bChangeSucc = true;
                                                break;
                                            }
                                        }
            
                                        if (bChangeSucc)
                                            break;
                                    }
                                }
                            }
                        }
                    }
                }
            }
			return true;
		}

		//++m_cbSystemAllKillCount;
		CT_WORD wMaxTypeArea = 0;
		for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
		{
			if (!m_GameLogic.CompareCard(m_cbTableCardArray[wMaxTypeArea], m_cbTableCardArray[wAreaIndex]))
			{
				wMaxTypeArea = wAreaIndex;//找出最大牌跟庄家调换
			}
		}

		//if (nBankerCount == 0)
		//{
		//	m_cbSystemAllKillCount = 0;
		//}

		//如果本来就是庄家通杀, 则不需要换牌
		if (wMaxTypeArea == 0)
		{
			return false;
		}

		LOG(WARNING) << "system banker all kill!!!";

		//最大牌换给系统庄家
		CT_BYTE cbTempCardArray[MAX_COUNT] = { 0 };
		memcpy(cbTempCardArray, m_cbTableCardArray[0], sizeof(cbTempCardArray));
		memcpy(m_cbTableCardArray[0], m_cbTableCardArray[wMaxTypeArea], sizeof(m_cbTableCardArray[0]));
		memcpy(m_cbTableCardArray[wMaxTypeArea], cbTempCardArray, sizeof(m_cbTableCardArray[wMaxTypeArea]));
	}
	
	return true;
}

CT_BOOL CGameProcess::StoreageControlChangeAreaCard()
{
    if (m_wBankerUser != INVALID_CHAIR)
        return true;
    
	//如果是系统必杀,不再进行各区域杀牌
	if (m_bCheatAllKill || m_bCheatBlackList)
		return true;
	
	//计算系统输赢
	CT_LONGLONG llSystemWinScore = CalSystemBankerWinUserScore(m_cbTableCardArray);
	
	//计算必杀区域数
	CalSystemOperateRation(llSystemWinScore);
	
	//m_wChangeCardRatio暂时用作必杀区域数
	if (m_wChangeCardRatio == 0)
	{
		return true;
	}
	
	std::vector<tagAreaRealBet> vecRealBet;
	tagAreaRealBet realBet;
	for (CT_BYTE cbAreaID = 1; cbAreaID < AREA_COUNT + 1; ++cbAreaID)
	{
		if ( m_llRealUserJettonScore[cbAreaID] != 0)
		{
			realBet.cbAreaID = cbAreaID;
			realBet.llRealBetScore = m_llRealUserJettonScore[cbAreaID];
			vecRealBet.push_back(realBet);
		}
	}
	
	std::sort(vecRealBet.begin(), vecRealBet.end(), [](tagAreaRealBet& a, tagAreaRealBet& b) {
	  return a.llRealBetScore > b.llRealBetScore;
	});

	LOG(WARNING) << "kill area count: " << m_wChangeCardRatio << ", real bet size: " << vecRealBet.size();
	//杀大
	for (CT_WORD i = 0; i < m_wChangeCardRatio; ++i)
	{
		if (i >= vecRealBet.size())
			break;
		
		tagAreaRealBet& refRealBet = vecRealBet[i];
		if (!m_GameLogic.CompareCard(m_cbTableCardArray[0], m_cbTableCardArray[refRealBet.cbAreaID]))
		{
			bool bChangeSucc = false;
			for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
			{
				if (m_cbRepertoryCard[n] == 0)
				{
					continue;
				}
				
				for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
				{
					//拷贝这个区域的牌
					CT_BYTE cbTempCardArray[MAX_COUNT] = { 0 };
					memcpy(cbTempCardArray, m_cbTableCardArray[refRealBet.cbAreaID], MAX_COUNT);
					
					CT_BYTE cbTempCardData = cbTempCardArray[nCardIndex];
					cbTempCardArray[nCardIndex] = m_cbRepertoryCard[n];
					if (m_GameLogic.CompareCard(m_cbTableCardArray[0], cbTempCardArray))
					{
						m_cbRepertoryCard[n] = cbTempCardData;
						memcpy(m_cbTableCardArray[refRealBet.cbAreaID], cbTempCardArray, MAX_COUNT);
						LOG(WARNING) << "change banker card more than bet much area card. area id: " << (int)refRealBet.cbAreaID;
						bChangeSucc = true;
						break;
					}
				}
				
				if (bChangeSucc)
					break;
			}
			
			//如果换闲家的牌没有换成功,考虑把庄家的牌换大
			if (!bChangeSucc)
			{
				for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
				{
					if (m_cbRepertoryCard[n] == 0)
					{
						continue;
					}
					
					for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
					{
						//拷贝这个区域的牌
						CT_BYTE  cbTempCardAreaArray[MAX_COUNT] = {0};
						memcpy(cbTempCardAreaArray, m_cbTableCardArray[0], MAX_COUNT);
						
						CT_BYTE cbTempCardData = cbTempCardAreaArray[nCardIndex];
						cbTempCardAreaArray[nCardIndex] = m_cbRepertoryCard[n];
						if (m_GameLogic.CompareCard(cbTempCardAreaArray, m_cbTableCardArray[realBet.cbAreaID]))
						{
							m_cbRepertoryCard[n] = cbTempCardData;
							memcpy(m_cbTableCardArray[0], cbTempCardAreaArray, MAX_COUNT);
							LOG(WARNING) << "change banker card more than bet much area card. area id: " << (int)refRealBet.cbAreaID;
							bChangeSucc = true;
							break;
						}
					}
					
					if (bChangeSucc)
						break;
				}
			}

			if (!bChangeSucc)
			{
				LOG(WARNING) << "can not change area card? area id: " << (int)refRealBet.cbAreaID;
			}
            else
            {
                m_bChangeAreaCard = true;
            }
		}
	}
	
	//保证有一个区域是赢的
	if (m_wChangeCardRatio >= 3)
	{
		for (CT_WORD i = 3; i < vecRealBet.size() ; ++i)
		{
			tagAreaRealBet& refRealBet = vecRealBet[i];
			if (m_GameLogic.CompareCard(m_cbTableCardArray[0], m_cbTableCardArray[refRealBet.cbAreaID]))
			{
				bool bChangeSucc = false;
				for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
				{
					if (m_cbRepertoryCard[n] == 0)
					{
						continue;
					}
					
					for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
					{
						//拷贝这个区域的牌
						CT_BYTE cbTempCardArray[MAX_COUNT] = { 0 };
						memcpy(cbTempCardArray, m_cbTableCardArray[refRealBet.cbAreaID], MAX_COUNT);
						
						CT_BYTE cbTempCardData = cbTempCardArray[nCardIndex];
						cbTempCardArray[nCardIndex] = m_cbRepertoryCard[n];
						if (!m_GameLogic.CompareCard(m_cbTableCardArray[0], cbTempCardArray))
						{
							m_cbRepertoryCard[n] = cbTempCardData;
							memcpy(m_cbTableCardArray[refRealBet.cbAreaID], cbTempCardArray, MAX_COUNT);
							LOG(WARNING) << "change banker card less than bet less area card. area id: " << (int)refRealBet.cbAreaID;
							bChangeSucc = true;
							break;
						}
					}
					
					if (bChangeSucc)
						break;
				}
			}
		}
	}
	
	return true;
}

CT_VOID CGameProcess::BlackListControl()
{
	if (m_wBankerUser != INVALID_CHAIR)
		return;

	if (m_bCheatAllKill)
		return;
	
	std::vector<tagBlackListWinScore> vecBlackListWinScore;
	for (auto& it : m_vecBlackList)
	{
		for (int i = 0; i < GAME_PLAYER; ++i)
		{
			if (m_dwAllUserID[i] != 0 && m_dwAllUserID[i] == it.dwUserID)
			{
				tagBlackListWinScore blackListWinScore;
				blackListWinScore.wChairID = i;
				blackListWinScore.llWinScore = 0;
				blackListWinScore.wLostRatio = it.wUserLostRatio;
				vecBlackListWinScore.push_back(blackListWinScore);
				//break;
			}
		}
	}
	
	CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT];
	memcpy(cbTempCardArray, m_cbTableCardArray, sizeof(cbTempCardArray));
	
	tagGameRoomKind*  pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
	CT_INT32 iMultiple[AREA_COUNT + 1] = { 0 };//倍数
	for (CT_WORD i = 0; i < AREA_COUNT + 1; i++)
	{
		iMultiple[i] = m_GameLogic.GetMultiple(cbTempCardArray[i], (CT_BYTE)pGameRoomKind->wRoomKindID);
	}
	for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
	{
		//庄输
		if (m_GameLogic.CompareCard(cbTempCardArray[0], cbTempCardArray[wAreaIndex]) == false)
		{
            for (auto& it : vecBlackListWinScore)
            {
                if (0 == m_wPlayStatus[it.wChairID]) continue;
                
                if (m_wBankerUser == it.wChairID) continue;
                
                if (m_llUserJettonScore[wAreaIndex][it.wChairID] > 0)
                {
                    CT_LONGLONG llWinScore = m_llUserJettonScore[wAreaIndex][it.wChairID] * iMultiple[wAreaIndex] * m_dwCellScore;
                    if (llWinScore > it.llWinScore)
                    {
                        it.llWinScore = llWinScore;
                        it.cbMaxJettonArea = wAreaIndex;
                    }
                }
            }
		}
	}
	
	//排序
	std::sort(vecBlackListWinScore.begin(), vecBlackListWinScore.end(), [](tagBlackListWinScore& a, tagBlackListWinScore& b) {
      return a.llWinScore > b.llWinScore;
    });
	
	//修改黑名单下注区域的玩家
    if (!vecBlackListWinScore.empty() && vecBlackListWinScore[0].llWinScore != 0)
    {
        tagBlackListWinScore& blackListWinScore = vecBlackListWinScore[0];
        //系统输
        if (!m_GameLogic.CompareCard(cbTempCardArray[0], cbTempCardArray[blackListWinScore.cbMaxJettonArea]))
        {
            int nRandomNum = rand() % 1000;
            LOG(WARNING) << "random num: " << nRandomNum << ", black lost ratio: " << blackListWinScore.wLostRatio;
            if (nRandomNum <= blackListWinScore.wLostRatio)
            {
				bool bChangeSucc = false;
	
				for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
				{
					if (m_cbRepertoryCard[n] == 0)
					{
						continue;
					}
					
					for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
					{
						//拷贝这个区域的牌
						CT_BYTE  cbTempCardAreaArray[MAX_COUNT] = {0};
						memcpy(cbTempCardAreaArray, cbTempCardArray[blackListWinScore.cbMaxJettonArea], MAX_COUNT);
						
						CT_BYTE cbTempCardData = cbTempCardAreaArray[nCardIndex];
						cbTempCardAreaArray[nCardIndex] = m_cbRepertoryCard[n];
						if (m_GameLogic.CompareCard(m_cbTableCardArray[0], cbTempCardAreaArray))
						{
							m_cbRepertoryCard[n] = cbTempCardData;
							memcpy(m_cbTableCardArray[blackListWinScore.cbMaxJettonArea], cbTempCardAreaArray, MAX_COUNT);
                            LOG(WARNING) << "change black list max jetton card. area index: " << (int)blackListWinScore.cbMaxJettonArea << ", user id: " << m_dwAllUserID[blackListWinScore.wChairID];
							bChangeSucc = true;
							break;
						}
					}
		
					if (bChangeSucc)
						break;
				}
                
                //如果换闲家的牌没有换成功,考虑把庄家的牌换大
                if (!bChangeSucc)
				{
					for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
					{
						if (m_cbRepertoryCard[n] == 0)
						{
							continue;
						}
						
						for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
						{
							//拷贝这个区域的牌
							CT_BYTE  cbTempCardAreaArray[MAX_COUNT] = {0};
							memcpy(cbTempCardAreaArray, cbTempCardArray[0], MAX_COUNT);
							
							CT_BYTE cbTempCardData = cbTempCardAreaArray[nCardIndex];
							cbTempCardAreaArray[nCardIndex] = m_cbRepertoryCard[n];
							if (m_GameLogic.CompareCard(cbTempCardAreaArray, cbTempCardArray[blackListWinScore.cbMaxJettonArea]))
							{
								m_cbRepertoryCard[n] = cbTempCardData;
								memcpy(m_cbTableCardArray[0], cbTempCardAreaArray, MAX_COUNT);
								LOG(WARNING) << "change banker card more than black list user card. area index: " << (int)blackListWinScore.cbMaxJettonArea << ", user id: " << m_dwAllUserID[blackListWinScore.wChairID];
								bChangeSucc = true;
								break;
							}
						}
						
						if (bChangeSucc)
							break;
					}
				}

				m_bCheatBlackList = bChangeSucc;
            }
        }
    }
}

CT_VOID CGameProcess::ChangeAllKillAreaCard()
{
    if (m_wBankerUser != INVALID_CHAIR)
        return;

    if (m_bCheatAllKill || m_bCheatBlackList || m_bChangeAreaCard)
        return;


    CT_BYTE cbBankerWinCount = 0;
    for (CT_WORD wAreaIndex = 1; wAreaIndex <= AREA_COUNT; ++wAreaIndex)
    {
        if (m_GameLogic.CompareCard(m_cbTableCardArray[0], m_cbTableCardArray[wAreaIndex]))
        {
            ++cbBankerWinCount;
        }
    }

    //如果是通杀
    if(cbBankerWinCount == AREA_COUNT)
	{
		bool bChangeSucc = false;
    	int nRandNum = rand() % 100;
    	if(nRandNum <= 80)
		{
    		//随机一个区域赢庄家
    		CT_BYTE cbRandAreaID = rand() % AREA_COUNT + 1;
			if (m_GameLogic.CompareCard(m_cbTableCardArray[0], m_cbTableCardArray[cbRandAreaID]))
			{
				for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
				{
					if (m_cbRepertoryCard[n] == 0)
					{
						continue;
					}

					for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
					{
						//拷贝这个区域的牌
						CT_BYTE cbTempCardArray[MAX_COUNT] = {0};
						memcpy(cbTempCardArray, m_cbTableCardArray[cbRandAreaID], MAX_COUNT);

						CT_BYTE cbTempCardData = cbTempCardArray[nCardIndex];
						cbTempCardArray[nCardIndex] = m_cbRepertoryCard[n];
						if (!m_GameLogic.CompareCard(m_cbTableCardArray[0], cbTempCardArray))
						{
							m_cbRepertoryCard[n] = cbTempCardData;
							memcpy(m_cbTableCardArray[cbRandAreaID], cbTempCardArray, MAX_COUNT);
							bChangeSucc = true;
							LOG(WARNING) << "ChangeAllKillAreaCard: change area: " <<  (int)cbRandAreaID << " for not all kill.";
							break;
						}
					}

					if (bChangeSucc)
						break;
				}
			}
		}

		if (!bChangeSucc)
		{
			LOG(WARNING) << "ChangeAllKillAreaCard: change area for not all kill fail.";
		}
	}

	//如果是通赔
	if(cbBankerWinCount == 0)
	{
		bool bChangeSucc = false;
		int nRandNum = rand() % 100;
		if(nRandNum <= 80)
		{
			//随机一个区域庄家赢
			CT_BYTE cbRandAreaID = rand() % AREA_COUNT + 1;
			if (!m_GameLogic.CompareCard(m_cbTableCardArray[0], m_cbTableCardArray[cbRandAreaID]))
			{
				for (int n = m_cbSendCardCount; n < MAX_CARD_TOTAL; ++n)
				{
					if (m_cbRepertoryCard[n] == 0)
					{
						continue;
					}

					for (int nCardIndex = FIRST_CARD; nCardIndex < MAX_COUNT; ++nCardIndex)
					{
						//拷贝这个区域的牌
						CT_BYTE cbTempCardArray[MAX_COUNT] = {0};
						memcpy(cbTempCardArray, m_cbTableCardArray[cbRandAreaID], MAX_COUNT);

						CT_BYTE cbTempCardData = cbTempCardArray[nCardIndex];
						cbTempCardArray[nCardIndex] = m_cbRepertoryCard[n];
						if (m_GameLogic.CompareCard(m_cbTableCardArray[0], cbTempCardArray))
						{
							m_cbRepertoryCard[n] = cbTempCardData;
							memcpy(m_cbTableCardArray[cbRandAreaID], cbTempCardArray, MAX_COUNT);
							bChangeSucc = true;
							LOG(WARNING) << "ChangeAllKillAreaCard: change area: " <<  (int)cbRandAreaID << " for not all Compensate.";
							break;
						}
					}

					if (bChangeSucc)
						break;
				}
			}
		}

		if (!bChangeSucc)
		{
			LOG(WARNING) << "ChangeAllKillAreaCard: change area for not all Compensate fail.";
		}
	}
}