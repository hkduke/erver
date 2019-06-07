
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
#include "GlobalEnum.h"
#include "IGameOtherData.h"

//打印日志
#define		PRINT_LOG_INFO						1
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//CT_WORD CGameProcess::m_wLargeJetton[MAX_JETTON_NUM] = { 2000,3000,5000,8000,10000 };	//大筹码
//CT_WORD CGameProcess::m_wMediumJetton[MAX_JETTON_NUM] ={ 1000,1500,2500,4000,5000 };	//中筹码
//CT_WORD CGameProcess::m_wSmallJetton[MAX_JETTON_NUM] = { 200,300,500,800,1000 };		//小筹码

//定时器
#define IDI_GAME_ADD_SCORE				1							//下注定时器
#define IDI_AUTO_FOLLOW					2							//自动加注
#define IDI_CLEAR_OFFLINE_USER			3							//清理离线玩家
#define IDI_GAME_READY					4							//准备定时器

#define TIME_GAME_ADD_SCORE				10							//下注时间
#define TIME_AUTO_FOLLOW				1							//自动加注
#define TIME_CLEAR_USER					3							//清理离线或者金币不足用户
#define TIME_GAME_READY					3+3							//准备时间（真正准备是3秒,有3秒是客户端飘金币动画时间（不包括比牌时间））

#define TIME_NETWORK_COMPENSATE         5                           //网络补偿

#define END_TIME_ALLIN					4							//allin结束
#define END_TIME_RUSH					6							//火拼结束
#define END_TIME_RUSH_GIVEUP			4							//火拼最后一个玩家弃牌结束
#define END_TIME_COMPARE				4							//比牌结束


////////////////////////////////////////////////////////////////////////
//静态变量
const CT_WORD	CGameProcess::m_wPlayerCount = GAME_PLAYER;			//游戏人数
CT_LONGLONG CGameProcess::m_lStockScore = 0;
CT_LONGLONG CGameProcess::m_lStockLowLimit = 0;

CGameProcess::CGameProcess(void)
: m_random_gen(m_random_device())
{
	//游戏变量
	m_wBankerUser = INVALID_CHAIR;
	m_wCurrentUser = INVALID_CHAIR;
	//m_lCompareCount = 0;
	m_bGameEnd = false;
	//memset(m_wFlashUser, 0, sizeof(m_wFlashUser));

	//用户状态
	memset(&m_StGameEnd, 0, sizeof(m_StGameEnd));
	memset(m_cbPlayStatus, 0, sizeof(m_cbPlayStatus));
	memset(m_cbRealPlayer, 0, sizeof(m_cbRealPlayer));
	memset(m_cbAndroidStatus, 0, sizeof(m_cbAndroidStatus));
	for (int i = 0; i < m_wPlayerCount; i++)
	{
		m_bMingZhu[i] = false;
		m_wCompardUser[i].clear();
	}

	//扑克变量
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(m_cbCardDepotRatio, 0, sizeof(m_cbCardDepotRatio));

	//下注信息
	//m_lMaxCellScore = 0L;
	m_lCellScore = 0L;
	m_lCurrentJetton = 0L;
	//m_lMaxScore = 0;
	//memset(m_lTableScore, 0, sizeof(m_lTableScore));
	//memset(m_lUserMaxScore, 0, sizeof(m_lUserMaxScore));
	//m_lStockLowLimit = 0;

	//清理游戏数据
	//ClearGameData();
	//初始化数据
	//InitGameData();

	/*CT_BYTE cbCardData1[MAX_COUNT] = { 0x04, 0x03, 0x02 };
	CT_BYTE cbCardData2[MAX_COUNT] = { 0x14, 0x13, 0x12 };

	CT_BYTE cbResult = m_GameLogic.CompareCard(cbCardData1, cbCardData2, MAX_COUNT, 0);

	LOG(WARNING) << "compare result: " << (int)cbResult;*/
}

CGameProcess::~CGameProcess(void)
{
}

//游戏开始
void CGameProcess::OnEventGameStart()
{
	//初始化数据
	InitGameData();

	//清除所有定时器
	ClearAllTimer();
	//设置游戏状态
	m_cbGameStatus = GAME_SCENE_PLAY;

	m_bGameEnd = false;
    m_lCurrentJetton = m_lCellScore;

    //分发扑克 (洗牌)
    m_GameLogic.RandCardList(m_cbRepertoryCard, sizeof(m_cbRepertoryCard));

	/*tagGameRoomKind* pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
    if (pGameRoomKind->wRoomKindID == PRIMARY_ROOM)
    {
        SendCardControl();
    }
    else*/
    {
        SendCardRand();
    }


	//游戏开始
	AnalyseStartCard();
    AnalyseBlackListCard();

	//用户设置
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{ 
		//获取牌型
		//CT_BYTE FirstData[MAX_COUNT];
		//memcpy(FirstData, m_cbHandCardData[i], sizeof(FirstData));

		//大小排序
		//m_GameLogic.SortCardList(FirstData, MAX_COUNT);
		m_GameLogic.SortCardList(m_cbHandCardData[i], MAX_COUNT);
		 m_cbHandCardType[i] = m_GameLogic.GetCardType(m_cbHandCardData[i], MAX_COUNT);

		if (m_cbPlayStatus[i] == true)
		{
			m_lTableScore[i] = m_lCellScore;
			//扣分
			m_pGameDeskPtr->AddUserScore(i, -m_lCellScore, false);
		}
	}

	//设置庄家
	if (m_wBankerUser == INVALID_CHAIR)
		m_wBankerUser = rand() % m_wPlayerCount;

	//庄家离开
	if (m_wBankerUser < m_wPlayerCount && m_cbPlayStatus[m_wBankerUser] == false)
		m_wBankerUser = rand() % m_wPlayerCount;

	//确定庄家
	while (m_cbPlayStatus[m_wBankerUser] == false)
	{
		m_wBankerUser = (m_wBankerUser + 1) % m_wPlayerCount;
	}

	//当前用户
	m_wCurrentUser = (m_wBankerUser + 1) % m_wPlayerCount;
	while (m_cbPlayStatus[m_wCurrentUser] == false)
	{
		m_wCurrentUser = (m_wCurrentUser + 1) % m_wPlayerCount;
	}

	//构造数据
	CMD_S_GameStart GameStart;
	memset(&GameStart, 0, sizeof(GameStart));
	GameStart.dCellScore = m_lCellScore*TO_DOUBLE;
	GameStart.dCurrentJetton = m_lCurrentJetton*TO_DOUBLE;
	GameStart.wBankerUser = m_wBankerUser;
	GameStart.wCurrentUser = m_wCurrentUser;
	memcpy(&GameStart.cbPlayStatus, m_cbPlayStatus, sizeof(CT_BYTE)*GAME_PLAYER);

	//for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	//{
	//	if (m_cbPlayStatus[i] == true)
	//		GameStart.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
	//}

	CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
	GameStart.wTimeLeft = wTimeLeft;
	m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
	m_wTotalOpTime = wTimeLeft;

	//发送数据
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		if (m_cbPlayStatus[i] == true)
		{
			GameStart.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
			GameStart.dUserScore[i] = m_pGameDeskPtr->GetUserScore(i)*TO_DOUBLE;
		}
	}
	
    CMD_S_StatusFree StatusFree;
    StatusFree.dCellScore = m_lCellScore*TO_DOUBLE;
    StatusFree.cbCanRushCount = CAN_RUSH_JETTON_COUNT;
	m_pGameDeskPtr->WriteGameSceneToRecord(&StatusFree, sizeof(StatusFree), SUB_SC_GAMESCENE_FREE);
	
	CT_BOOL bRecord = false;
	for (int i = 0; i < m_wPlayerCount; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
			continue;
		
		if (!bRecord)
		{
			m_pGameDeskPtr->SendTableData(i, SUB_S_GAME_START, &GameStart, sizeof(GameStart), true);
			bRecord = true;
		}
		else
		{
			m_pGameDeskPtr->SendTableData(i, SUB_S_GAME_START, &GameStart, sizeof(GameStart));
		}

		//如果是真人玩家，检测
		if (m_pGameDeskPtr->IsAndroidUser(i) == false)
		{
			GS_UserScoreData* pUserScoreData = m_pGameDeskPtr->GetUserScoreData(i);
			if ((pUserScoreData->dwPlayCount+1) == pUserScoreData->dwChangeTableCount)
			{
				CMD_S_ChangeTable_Tips tips;
				tips.cbResult = en_ReadyChangeTable;
				m_pGameDeskPtr->SendTableData(i, SUB_S_CHANGE_TABLE_TIPS, &tips, sizeof(tips));
				//LOG(WARNING) << "send change table tips!";
			}
		}
	}


	//给机器人发送所有人的牌数据
	for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	{
		//机器人数据
		CT_BOOL bExistUser = m_pGameDeskPtr->IsExistUser(i);
		if (bExistUser == false) 
			continue;

		if (m_pGameDeskPtr->IsAndroidUser(i) == true)
		{
			//机器人数据
			CMD_S_AndroidCard AndroidCard;
			memset(&AndroidCard, 0, sizeof(AndroidCard));

			//设置变量
			AndroidCard.lStockScore = m_lStockScore;
			memcpy(AndroidCard.cbRealPlayer, m_cbRealPlayer, sizeof(m_cbRealPlayer));
			memcpy(AndroidCard.cbAndroidStatus, m_cbAndroidStatus, sizeof(m_cbAndroidStatus));
			memcpy(AndroidCard.cbAllHandCardData, m_cbHandCardData, sizeof(m_cbHandCardData));

			m_pGameDeskPtr->SendTableData(i, SUB_S_ANDROID_CARD, &AndroidCard, sizeof(AndroidCard));
		}
	}

	//玩家下注定时器
	m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + TIME_NETWORK_COMPENSATE)*1000);
	
	//m_strRecord << "{\"rec\": \"轮数:0<br/>庄位:" << m_wBankerUser << ", 首位: " ;
	
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//结束时清理所有定时器
	ClearAllTimer();

	m_cbGameStatus = GAME_SCENE_FREE;
	//switch (GETag)
	{
	//case GER_COMPARECARD:	//比牌结束
	//case GER_NO_PLAYER:		//没有玩家
	{
		//if (m_bGameEnd)
		//	return;
		//m_bGameEnd = true;

		//定义变量
		CMD_S_GameEnd GameEnd;
		memset(&GameEnd, 0, sizeof(GameEnd));
		RecordScoreInfo	endScoreInfo[GAME_PLAYER];

		//唯一玩家
		CT_WORD wWinner = 0;
		//CT_WORD wUserCount = 0;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (m_cbPlayStatus[i] == true)
			{
				//wUserCount++;
				wWinner = i;

				if (m_pGameDeskPtr->GetUserStatus(i) != sOffLine)
					m_pGameDeskPtr->SetUserStatus(i, sSit);

				if (GER_COMPARECARD == GETag)
					m_wBankerUser = i;
			}
		}
		m_wWinnerUser = wWinner;
		//UpdateGoodCardInfo(wWinner, true);

		//计算总注
		CT_LONGLONG lWinnerScore = 0L;
		CT_LONGLONG lGameScore[GAME_PLAYER];
		memset(lGameScore, 0, sizeof(lGameScore));

		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (i == wWinner)
				continue;

			lGameScore[i] = -m_lTableScore[i];
			lWinnerScore += m_lTableScore[i];

			if (m_bGameStatus[i])
			{
				endScoreInfo[i].dwUserID = m_dwPlayUserID[i];
				endScoreInfo[i].llSourceScore = m_llUserSourceScore[i];
				endScoreInfo[i].iScore = (CT_INT32)lGameScore[i];
				endScoreInfo[i].cbStatus = 1;
				endScoreInfo[i].cbIsAndroid = m_cbAndroidStatus[i];
			}
		}

		//库存统计
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			//赢家的是机器人的话
			if (i == wWinner && m_cbAndroidStatus[wWinner])
			{
				m_lStockScore += lWinnerScore;
				continue;
			}
			
			//库存累计
			if (m_cbAndroidStatus[i])
				m_lStockScore += lGameScore[i];
		}

		//处理税收
		CT_LONGLONG lGameTax = m_pGameDeskPtr->CalculateRevenue(wWinner, lWinnerScore);
		lGameScore[wWinner] = lWinnerScore - lGameTax;
		CT_LONGLONG llWinnerWinScore = lGameScore[wWinner];
		endScoreInfo[wWinner].dwUserID = m_dwPlayUserID[wWinner];
		endScoreInfo[wWinner].llSourceScore = m_llUserSourceScore[wWinner];
		endScoreInfo[wWinner].iScore = (CT_INT32)lWinnerScore;
		endScoreInfo[wWinner].dwRevenue = (CT_INT32)lGameTax;
		endScoreInfo[wWinner].cbStatus = 1;
		endScoreInfo[wWinner].cbIsAndroid = m_cbAndroidStatus[wWinner];

		//玩家输赢
		UpdateUserScoreInfo(wWinner, lWinnerScore, lGameTax);

		//不让别人看到弃牌玩家的牌
		for (int i = 0; i != GAME_PLAYER; ++i)
		{
			//if (1 == m_cbGiveUp[i])	
			//	continue;

			memcpy(&GameEnd.cbCardData[i], m_cbHandCardData[i], sizeof(m_cbHandCardData[i]));
			GameEnd.cbCardType[i] = m_cbHandCardType[i];
		}

		//前端没有用这个字段，先不赋值，但暂时不删除这个字段
		//if (GETag == GER_NO_PLAYER)
		//	GameEnd.cbEndState = 1;
		//else
		//	GameEnd.cbEndState = 0;

		//写赢家积分
		ScoreInfo scoreInfo;
		scoreInfo.dwUserID = m_pGameDeskPtr->GetUserID(wWinner);
		//scoreInfo.scoreKind = (lGameScore[wWinner] > 0) ? enScoreKind_Win : enScoreKind_Lost;
		//scoreInfo.scoreKind = enScoreKind_Win;
		lGameScore[wWinner] += m_lTableScore[wWinner]; //把自己下的注加回来,自己下的注不用扣税
		scoreInfo.llScore = lGameScore[wWinner];
		scoreInfo.llRealScore = llWinnerWinScore;
		scoreInfo.bBroadcast = false;
		if (m_cbHandCardType[wWinner] >= CT_SHUN_JIN)
			scoreInfo.cbCardType = m_cbHandCardType[wWinner];
		m_pGameDeskPtr->WriteUserScore(wWinner, scoreInfo);

		//库存统计
		/*for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			//库存累计
			if (m_pGameDeskPtr->IsAndroidUser(i))
				m_lStockScore += lGameScore[i];
		}*/

		memcpy(GameEnd.cbGameState, m_bGameStatus2, sizeof(GameEnd.cbGameState));

		//设置等待开始
		m_cbGameStatus = GAME_SCENE_END;
		CT_WORD wTimeLeft = 0;
		CT_BYTE nReason = (CT_BYTE)GETag;
		if (nReason == GER_NO_PLAYER)
		{
			wTimeLeft = (TIME_GAME_READY) + 1;
		}
		else if (nReason == GER_COMPARECARD)
		{
			wTimeLeft = (TIME_GAME_READY + END_TIME_COMPARE); 
		}
		else if (nReason == GER_ALLIN)
		{
			wTimeLeft = (TIME_GAME_READY + END_TIME_ALLIN);
		}
		else if (nReason == GER_RUSH)
		{
			wTimeLeft = (TIME_GAME_READY + END_TIME_RUSH);
		}
		else if (nReason == GER_RUSH_GIVEUP)
		{
			wTimeLeft = (TIME_GAME_READY + END_TIME_RUSH_GIVEUP);
		}
		else
		{
			wTimeLeft = (TIME_GAME_READY + END_TIME_COMPARE);
		}

		
		//wTimeLeft += TIME_NETWORK_COMPENSATE; //1秒时间补偿
		m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
		m_wTotalOpTime = wTimeLeft;
		GameEnd.wTimeLeft = TIME_GAME_READY; //客户端不扣比牌时间

		for (CT_WORD i = 0; i < m_wPlayerCount; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
				continue;

			//if (!m_bGameStatus[i])
			//	continue;

			if (m_bGameStatus[i])
			{
				if (i == wWinner)
				{
					GameEnd.dGameScore[i] = llWinnerWinScore * TO_DOUBLE;
				} else
				{
					GameEnd.dGameScore[i] = lGameScore[i] * TO_DOUBLE;
				}
				//LOG(WARNING) << "game end score: " << GameEnd.dGameScore[i];
				GameEnd.dTotalScore[i] = m_pGameDeskPtr->GetUserScore(i)*TO_DOUBLE;
			}
		}

		//扑克数据
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
				continue;

			//if (!m_bGameStatus[i])
			//	continue;
			memset(&GameEnd.wCompareUser, 0, sizeof(GameEnd.wCompareUser));

			if (m_bGameStatus[i])
			{
				//CT_WORD wCount = 0;
				while (m_wCompardUser[i].size() > 0)
				{
					CT_WORD wUser = m_wCompardUser[i][0];
					if (wUser >= GAME_PLAYER)
					{
						LOG(WARNING) << "compare card user err, user chair id: " << wUser;
						break;
					}
					GameEnd.wCompareUser[wUser] = 1;
					m_wCompardUser[i].erase(m_wCompardUser[i].begin());
				}
			}
			
			if (i == wWinner)
				m_pGameDeskPtr->SendTableData(i, SUB_S_GAME_END, &GameEnd, sizeof(GameEnd), true);
			else
				m_pGameDeskPtr->SendTableData(i, SUB_S_GAME_END, &GameEnd, sizeof(GameEnd));
		}

		m_pGameDeskPtr->SetGameTimer(IDI_GAME_READY, wTimeLeft * 1000);

		
		CT_DWORD dwBankerUserID = 0;
		if (m_wBankerUser != INVALID_CHAIR)
            dwBankerUserID = m_dwPlayUserID[m_wBankerUser];
		
		//记录游戏数据
		m_pGameDeskPtr->RecordRawInfo(endScoreInfo, GAME_PLAYER, NULL, 0, dwBankerUserID, m_lStockScore, 0, 0, 0, 0);

		//清空离线玩家（时间比进入准备的时间少5秒）
		m_pGameDeskPtr->SetGameTimer(IDI_CLEAR_OFFLINE_USER, (wTimeLeft - 1) * 1000);
		//LOG(WARNING) << "time left: " <<  (wTimeLeft - 1) * 1000;
	}
	}

	//当前时间
	std::string strTime = Utility::GetTimeNowString();
	//游戏结束
	m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());
}

//发送场景
void CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID >= GAME_PLAYER)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "OnEventGameScene: dwChairID = " << (int)dwChairID << "  err";
		return;
	}

	switch (m_cbGameStatus)
	{
		case GAME_SCENE_FREE:	//空闲状态
		{
			//构造数据
			CMD_S_StatusFree StatusFree;
			memset(&StatusFree, 0, sizeof(StatusFree));

			//设置变量
			StatusFree.dCellScore = m_lCellScore*TO_DOUBLE;
			StatusFree.cbCanRushCount = CAN_RUSH_JETTON_COUNT;
			//发送场景
			m_pGameDeskPtr->SendTableData(dwChairID, SUB_SC_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree));
			break;
		}
		case GAME_SCENE_PLAY:	//游戏状态
		{
			//构造数据
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay, 0, sizeof(StatusPlay));

			//加注信息
			StatusPlay.dCellScore = m_lCellScore*TO_DOUBLE;
			
			CT_BYTE cbTimes = m_bMingZhu[m_wCurrentUser] ? 2 : 1;
			StatusPlay.dCurrentJetton = (m_lCurrentJetton*cbTimes)*TO_DOUBLE;

			//设置变量
			StatusPlay.wBankerUser = m_wBankerUser;
			StatusPlay.wCurrentUser = m_wCurrentUser;
			memcpy(StatusPlay.cbPlayStatus, m_cbPlayStatus, sizeof(StatusPlay.cbPlayStatus));
			memcpy(StatusPlay.bMingZhu, m_bMingZhu, sizeof(StatusPlay.bMingZhu));
			memcpy(StatusPlay.bGiveUp, m_cbGiveUp, sizeof(StatusPlay.bGiveUp));
			memcpy(StatusPlay.bAutoFollow, m_bAutoFollow, sizeof(StatusPlay.bAutoFollow));
			for (int i = 0; i != GAME_PLAYER; i++)
			{
				StatusPlay.dTableJetton[i] = m_lTableScore[i] * TO_DOUBLE;
				StatusPlay.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
			}
	
			//当前状态
			//StatusPlay.bCompareState = (m_lCompareCount <= 0) ? false : true;
			StatusPlay.wJettonCount = m_wAddScoreCount[m_wCurrentUser];
			StatusPlay.wTimeLeft = m_dwOpEndTime - (CT_DWORD)time(NULL);
			StatusPlay.wTotalTime = m_wTotalOpTime;
			StatusPlay.cbCanRushCount = CAN_RUSH_JETTON_COUNT;
			StatusPlay.cbRushState = m_bRushState;
			CT_LONGLONG llRushJetton = m_bRushState ? m_lRushJetton : GetLeastUserScore();
			CT_WORD wMod = llRushJetton % 100;
			llRushJetton -= wMod;
			StatusPlay.dRushJetton = llRushJetton*TO_DOUBLE;
			//设置扑克
			if (m_bMingZhu[dwChairID])
			{
				memcpy(&StatusPlay.cbHandCardData, &m_cbHandCardData[dwChairID], MAX_COUNT);
				StatusPlay.cbHandCardType = m_cbHandCardType[dwChairID];
			}

			//发送场景
			m_pGameDeskPtr->SendTableData(dwChairID, SUB_SC_GAMESCENE_PLAY, &StatusPlay, sizeof(StatusPlay));
			break;
		}
		case GAME_SCENE_END:
		{
			CMD_S_StatusEnd statusEnd;
			statusEnd.wWaitTime = m_dwOpEndTime - (CT_DWORD)time(NULL);;
			statusEnd.cbCanRushCount = CAN_RUSH_JETTON_COUNT;
			m_pGameDeskPtr->SendTableData(dwChairID, SUB_SC_GAMESCENE_END, &statusEnd, sizeof(statusEnd));
		}
		break;
		default:
			break;
	}
	return ;
}

//定时器事件
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	//结束定时
	/*if (dwTimerID == IDI_GAME_COMPAREEND || IDI_GAME_OPENEND == dwTimerID)
	{
		//删除时间
		if (dwTimerID == IDI_GAME_COMPAREEND)
			m_pGameDeskPtr->KillGameTimer(IDI_GAME_COMPAREEND);
		else 
			m_pGameDeskPtr->KillGameTimer(IDI_GAME_OPENEND);

		//发送信息
		m_pGameDeskPtr->SendTableData(INVALID_CHAIR, SUB_S_GAME_END, &m_StGameEnd, sizeof(m_StGameEnd));

		//结束游戏
		//OnEventGameEnd()
	}*/

	switch (dwTimerID)
	{
	case IDI_GAME_ADD_SCORE:					//玩家下注定时器
	{
		//删除时间
		m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
		//当前玩家弃牌
		OnUserGiveUp(m_wCurrentUser, true);
	}
	break;
	case IDI_AUTO_FOLLOW:
	{
		//删除时间
		m_pGameDeskPtr->KillGameTimer(IDI_AUTO_FOLLOW);

		//当前玩家自动跟注
		HandleAutoFollow(m_wCurrentUser);
	}
	break;
	case IDI_CLEAR_OFFLINE_USER:
	{
		//删除时间
		m_pGameDeskPtr->KillGameTimer(IDI_CLEAR_OFFLINE_USER);

		//清理系统弃牌的玩家
		for (CT_BYTE i = 0; i != GAME_PLAYER; ++i)
		{
			if (m_cbSystemGiveUp[i] == 1)
			{
				m_pGameDeskPtr->ClearTableUser(i, true, true, ENTER_ROOM_LONG_TIME_NOT_OP);
			}
		}

		//清理离线玩家
		m_pGameDeskPtr->ClearTableUser(INVALID_CHAIR, true);
	}
	break;
	case IDI_GAME_READY:
	{
		//删除时间
		m_pGameDeskPtr->KillGameTimer(IDI_GAME_READY);
		//所有玩家准备
		OnAllUserReady();
	}
	break;
	default:break;
	}
}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	bool bSucc = false;
	switch (dwSubCmdID)
	{
	case SUB_C_GIVE_UP:			//用户放弃
	{
		//状态判断
		if (m_cbPlayStatus[wChairID] == false)
			return false;
		
		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;
		
		//消息处理
		//return OnUserGiveUp(wChairID);
		bSucc = OnUserGiveUp(wChairID);
		break;
	}
	case SUB_C_LOOK_CARD:		//用户看牌
	//case SUB_C_OPEN_CARD:		//用户开牌
	{
		//状态判断
		if (m_cbPlayStatus[wChairID] == false)
			return false;

		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;

		//消息处理
	//	if (dwSubCmdID == SUB_C_LOOK_CARD)
		//return OnUserLookCard(wChairID);
		bSucc = OnUserLookCard(wChairID);
		break;

		//return OnUserOpenCard(wChairID);
	}
	case SUB_C_OPEN_CARD:
	{
		bSucc = OnUserOpenCard(wChairID);
		break;
	}
	case SUB_C_COMPARE_CARD:	//用户比牌
	{
		//效验数据
		if (dwDataSize != sizeof(CMD_C_CompareCard)) return false;

		//变量定义
		CMD_C_CompareCard * pCompareCard = (CMD_C_CompareCard *)pDataBuffer;

		//参数效验
		if (wChairID != m_wCurrentUser)
			return false;
		
		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;

		//状态判断
		if (m_cbPlayStatus[wChairID] == false || m_cbPlayStatus[pCompareCard->wCompareUser] == false)
			return false;

		//消息处理
		bSucc = OnUserCompareCard(wChairID, pCompareCard->wCompareUser);
		break;
	}
	case SUB_C_ADD_SCORE:		//用户加注
	{
		//效验数据
		if (dwDataSize != sizeof(CMD_C_AddScore)) return false;

		//变量定义
		CMD_C_AddScore * pAddScore = (CMD_C_AddScore *)pDataBuffer;

		//状态判断
		if (m_cbPlayStatus[wChairID] == false) return false;
		
		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;

		//当前状态
		//if (pAddScore->wState > 0)m_lCompareCount = pAddScore->lScore;
		CT_LONGLONG llAddScore = (CT_LONGLONG)pAddScore->dScore*100;

		//消息处理
		bSucc = OnUserAddScore(wChairID, llAddScore, false);
		break;
	}
	case SUB_C_ALL_IN:
	{
		//状态判断
		if (m_cbPlayStatus[wChairID] == false)
			return false;
		
		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;

		//消息处理
		bSucc = OnUserAllIn(wChairID);
		break;
	}
	case SUB_C_AUTO_FOLLOW:
	{
		//状态判断
		if (m_cbPlayStatus[wChairID] == false)
			return false;
		
		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;

		//消息处理
		bSucc = OnUserAutoFollow(wChairID);
		break;
	}
	case SUB_C_CANCEL_AUTO_FOLLOW:
	{
		//状态判断
		if (m_cbPlayStatus[wChairID] == false)
			return false;
		
		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;
		
		//消息处理
		bSucc = OnUserCancelAutoFollow(wChairID);
		break;
	}
	case SUB_C_RUSH:
	{
		return false;

		//状态判断
		if (m_cbPlayStatus[wChairID] == false)
			return false;
		
		if (m_cbGameStatus != GAME_SCENE_PLAY)
			return false;

		bSucc = OnUserRush(wChairID);
		break;
	}
	}

	//如果碰到处理消息失败的情况，断掉玩家连接
	if (!bSucc)
	{
		m_pGameDeskPtr->CloseUserConnect(wChairID);
	}
	return bSucc;
}

//用户离开
void CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag)
{
	if (dwChairID == INVALID_CHAIR)
	{
		return;
	}

	//如果玩家原来是在游戏中，则清空玩家状态
	if (m_cbPlayStatus[dwChairID] == 0)
	{

		//这里不能清理玩家游戏的状态
		/*if (m_bGameStatus[dwChairID] == 1)
		{
			m_bGameStatus[dwChairID] = 0;
		}*/
		
		if (m_bGameStatus2[dwChairID] == 1)
		{
			m_bGameStatus2[dwChairID] = 0;
		}
		
		//如果是游戏已经开始了，则清空玩家。如果游戏未开始，框架会自动清空玩家
		if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_Start)
		{
			m_pGameDeskPtr->ClearTableUser(dwChairID, true);
		}

		if (m_cbSystemGiveUp[dwChairID] == 1)
		{
			m_cbSystemGiveUp[dwChairID] = 0;
		}
	}
}

//用户换桌
void CGameProcess::OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce)
{
    if (dwChairID == INVALID_CHAIR)
    {
        return;
    }


    if (!bForce)
	{
		//如果玩家原来是在游戏中，则清空玩家状态
		if (m_cbPlayStatus[dwChairID] == 0)
		{

			if (m_bGameStatus2[dwChairID] == 1)
			{
				m_bGameStatus2[dwChairID] = 0;
			}

			if (m_cbSystemGiveUp[dwChairID] == 1)
			{
				m_cbSystemGiveUp[dwChairID] = 0;
			}
		}
	}
	else
	{
		//强迫换桌回调成功
		CMD_S_ChangeTable_Tips tips;
		tips.cbResult = en_FinishChangeTable;
		m_pGameDeskPtr->SendTableData(dwChairID, SUB_S_CHANGE_TABLE_TIPS, &tips, sizeof(tips));
	}


}

//设置指针
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	//assert(NULL != pTableFrame);
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
	//memset(m_iTotalLWScore, 0, sizeof(m_iTotalLWScore));
	//各玩家炸弹个数
	//memset(m_cbTotalRomb, 0, sizeof(m_cbTotalRomb));
	//输赢次数
	//memset(m_wPlayWinCount, 0, sizeof(m_wPlayWinCount));
	//memset(m_wPlayLoseCount, 0, sizeof(m_wPlayLoseCount));
	//首叫用户
	/*m_dwFirstUser = INVALID_CHAIR;*/

}


//私人房游戏大局是否结束
bool CGameProcess::IsGameRoundClearing(CT_BOOL bDismissGame)
{
	return false;
}

//获取大局结算的总分
CT_INT32 CGameProcess::GetGameRoundTotalScore(CT_DWORD dwChairID)
{
	return 0;
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

//设置炸金花的牌库概率
CT_VOID CGameProcess::SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio)
{
	if (pZjhCardDepotRatio == NULL)
		return;

	if (pZjhCardDepotRatio->cbDepotID == 1)
	{
		m_cbCardDepotRatio[0][0] = pZjhCardDepotRatio->cbCard1Ration;
		m_cbCardDepotRatio[0][1] = pZjhCardDepotRatio->cbCard2Ration;
		m_cbCardDepotRatio[0][2] = pZjhCardDepotRatio->cbCard3Ration;
		m_cbCardDepotRatio[0][3] = pZjhCardDepotRatio->cbCard4Ration;
		m_cbCardDepotRatio[0][4] = pZjhCardDepotRatio->cbCard5Ration;
		m_cbCardDepotRatio[0][5] = pZjhCardDepotRatio->cbCard6Ration;
	}
	else if (pZjhCardDepotRatio->cbDepotID == 2)
	{
		m_cbCardDepotRatio[1][0] = pZjhCardDepotRatio->cbCard1Ration;
		m_cbCardDepotRatio[1][1] = pZjhCardDepotRatio->cbCard2Ration;
		m_cbCardDepotRatio[1][2] = pZjhCardDepotRatio->cbCard3Ration;
		m_cbCardDepotRatio[1][3] = pZjhCardDepotRatio->cbCard4Ration;
		m_cbCardDepotRatio[1][4] = pZjhCardDepotRatio->cbCard5Ration;
		m_cbCardDepotRatio[1][5] = pZjhCardDepotRatio->cbCard6Ration;
	}
}


//发送数据
CT_BOOL CGameProcess::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BOOL bRecord)
{
	if (NULL == m_pGameDeskPtr)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "SendTableData: dwChairID = " << (int)dwChairID << ",wSubCmdID ="<< (int)wSubCmdID << " err";
		return false;
	}
	return m_pGameDeskPtr->SendTableData(dwChairID, wSubCmdID, pBuf, dwLen, bRecord);
}

//初始化游戏数据
void CGameProcess::InitGameData()
{
	m_wBankerUser = INVALID_CHAIR;
	m_wCurrentUser = INVALID_CHAIR;
	m_wWinnerUser = INVALID_CHAIR;
	//m_lCompareCount = 0;
	m_bGameEnd = false;
	//memset(m_wFlashUser, 0, sizeof(m_wFlashUser));

	//用户状态
	memset(&m_StGameEnd, 0, sizeof(m_StGameEnd));
	memset(&m_cbGiveUp, 0, sizeof(m_cbGiveUp));
	memset(&m_cbSystemGiveUp, 0, sizeof(m_cbSystemGiveUp));
	memset(m_cbPlayStatus, 0, sizeof(m_cbPlayStatus));
	memset(m_bGameStatus, 0, sizeof(m_bGameStatus));
	memset(m_bGameStatus2, 0, sizeof(m_bGameStatus2));
	memset(m_bAutoFollow, 0, sizeof(m_bAutoFollow));
	memset(m_dwPlayUserID, 0, sizeof(m_dwPlayUserID));
	memset(m_cbRealPlayer, 0, sizeof(m_cbRealPlayer));
	memset(m_cbAndroidStatus, 0, sizeof(m_cbAndroidStatus));
	//memset(m_wAddScoreCount, 0, sizeof(m_wAddScoreCount));
	m_cbGameStatus = GAME_SCENE_FREE;
	for (int i = 0; i < m_wPlayerCount; i++)
	{
		m_bMingZhu[i] = false;
		m_wCompardUser[i].clear();
		m_wAddScoreCount[i] = 1;
	}

	//扑克变量
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(m_cbHandCardType, 0, sizeof(m_cbHandCardType));
	m_cbSendCardCount = 0;
	memset(m_cbRepertoryCard, 0, sizeof(m_cbRepertoryCard));
	//m_strRecord.clear();

	//下注信息
	//m_lMaxCellScore = 0L;
	m_lCellScore = 0L;
	m_lCurrentJetton = 0L;
	//m_lMaxScore = 0;
	memset(m_llUserSourceScore, 0, sizeof(m_llUserSourceScore));
	memset(m_lTableScore, 0, sizeof(m_lTableScore));
	//memset(m_lUserMaxScore, 0, sizeof(m_lUserMaxScore));
	m_bRushState = false;
	m_wStartRushUser = INVALID_CHAIR;
	m_lRushJetton = 0;

	if (NULL != m_pGameDeskPtr)
	{
		//m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
		m_lCellScore = m_pGameDeskPtr->GetGameCellScore();
		m_llMaxJettonScore = MAX_JETTON_MULTIPLE*m_lCellScore;
		/*tagGameRoomKind* pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
		if (pGameRoomKind)
		{
			if (pGameRoomKind->wRoomKindID == PRIMARY_ROOM)
			{
				memcpy(m_wCurJetton, m_wSmallJetton, sizeof(m_wCurJetton));
			}
			else if (pGameRoomKind->wRoomKindID == MIDDLE_ROOM)
			{
				memcpy(m_wCurJetton, m_wMediumJetton, sizeof(m_wCurJetton));
			}
			else if (pGameRoomKind->wRoomKindID == SENIOR_ROOM)
			{
				memcpy(m_wCurJetton, m_wLargeJetton, sizeof(m_wCurJetton));
			}
			else
			{
				memcpy(m_wCurJetton, m_wSmallJetton, sizeof(m_wCurJetton));
			}
		}*/
	}
}

//更新游戏配置
void CGameProcess::UpdateGameConfig()
{
	if (NULL== m_pGameDeskPtr)
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
	//等待玩家出牌定时器和自动出牌定时器
	m_pGameDeskPtr->KillGameTimer(IDI_AUTO_FOLLOW);
	m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
	m_pGameDeskPtr->KillGameTimer(IDI_GAME_READY);
	m_pGameDeskPtr->KillGameTimer(IDI_CLEAR_OFFLINE_USER);
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwWinChairID)
{
	if (dwWinChairID>=INVALID_CHAIR)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "NormalGameEnd Err!";
		return;
	}


}

//发送总结算
void CGameProcess::SendTotalClearing(std::string strTime)
{	
}

//解释私人场数据
void CGameProcess::ParsePrivateData(CT_CHAR *pBuf)
{

}

CT_WORD CGameProcess::GetNextUser(CT_WORD wChairID)
{
	CT_WORD wNextPlayer = INVALID_CHAIR;
	for (CT_WORD i = 1; i < m_wPlayerCount; i++)
	{
		//设置变量
		wNextPlayer = (wChairID + i) % m_wPlayerCount;

		//继续判断
		if (m_cbPlayStatus[wNextPlayer] == true)
			break;
	}

	return wNextPlayer;
}

//获取至少金币数
CT_LONGLONG CGameProcess::GetLeastUserScore()
{
    if (m_wCurrentUser == INVALID_CHAIR)
    {
        return  0;
    }

    if (m_wAddScoreCount[m_wCurrentUser] < CAN_RUSH_JETTON_COUNT)
    {
        return 0;
    }

	CT_LONGLONG llLeastScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser);
    CT_WORD wLeastUser = m_wCurrentUser;
	//找到金币最少的玩家
	for (CT_WORD i = 0; i != GAME_PLAYER; ++i)
	{
		if (m_cbPlayStatus[i] == false || i == m_wCurrentUser)
			continue;

		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
		if (llUserScore < llLeastScore)
        {
            llLeastScore = llUserScore;
            wLeastUser = i;
        }
	}

	//判断最后一个人是否
    CT_BYTE cbMul = m_bMingZhu[wLeastUser] ? 2 : 1;
    if (llLeastScore < cbMul*m_lCurrentJetton)
    {
        return  0;
    }

	return llLeastScore;
}

CT_BOOL CGameProcess::CheckCanRush()
{
	for (CT_BYTE i = 0; i != GAME_PLAYER; ++i)
	{
		if (m_cbPlayStatus[i] == false)
			continue;

		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
		CT_BYTE cbMul = m_bMingZhu[i] ? 2 : 1;
		if (llUserScore < cbMul*m_lCurrentJetton)
		{
			return  false;
		}
	}

	return true;
}

//随机发牌
CT_VOID CGameProcess::SendCardRand()
{
    for (CT_WORD i = 0; i < m_wPlayerCount; i++)
    {
        //获取用户
        if (!m_pGameDeskPtr->IsExistUser(i))
            continue;

        if (m_pGameDeskPtr->IsAndroidUser(i) != true)
            m_cbRealPlayer[i] = true;
        if (m_pGameDeskPtr->IsAndroidUser(i) == true)
            m_cbAndroidStatus[i] = true;

        m_llUserSourceScore[i] = m_pGameDeskPtr->GetUserScore(i);
        m_dwPlayUserID[i] = m_pGameDeskPtr->GetUserID(i);
        m_pGameDeskPtr->SetUserStatus(i, sPlaying);
        //LOG(ERROR) << "set user playing, user id: " << m_pGameDeskPtr->GetUserID(i);
        //CT_LONGLONG lUserScore = m_pGameDeskPtr->GetUserScore(i);
        //设置变量
        m_cbPlayStatus[i] = true;
        m_bGameStatus[i] = true;
        m_bGameStatus2[i] = true;
    }

    //m_lMaxCellScore = m_wCurJetton[MAX_JETTON_NUM - 1];

    for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
    {
        if (0 == m_cbPlayStatus[i])
            continue;

        //手牌
        memcpy(&m_cbHandCardData[i], &m_cbRepertoryCard[m_cbSendCardCount], sizeof(CT_BYTE)*MAX_COUNT);
        m_cbSendCardCount += MAX_COUNT;
    }
}
//控制发牌
CT_VOID CGameProcess::SendCardControl()
{
	/*
    std::vector<std::unique_ptr<tagGoodCardParam>> vecGoodParam;
    for (CT_WORD i = 0; i < m_wPlayerCount; i++)
    {
        //获取用户
        if (!m_pGameDeskPtr->IsExistUser(i))
            continue;

        if (m_pGameDeskPtr->IsAndroidUser(i) != true)
            m_cbRealPlayer[i] = true;
        if (m_pGameDeskPtr->IsAndroidUser(i) == true)
            m_cbAndroidStatus[i] = true;

        m_llUserSourceScore[i] = m_pGameDeskPtr->GetUserScore(i);
        m_dwPlayUserID[i] = m_pGameDeskPtr->GetUserID(i);
        m_pGameDeskPtr->SetUserStatus(i, sPlaying);
        //LOG(ERROR) << "set user playing, user id: " << m_pGameDeskPtr->GetUserID(i);
        //CT_LONGLONG lUserScore = m_pGameDeskPtr->GetUserScore(i);
        //设置变量
        m_cbPlayStatus[i] = true;
        m_bGameStatus[i] = true;
        m_bGameStatus2[i] = true;

        std::unique_ptr<tagGoodCardParam> ptrGoodParam(new tagGoodCardParam);
        ptrGoodParam->wChairID = i;
        ptrGoodParam->cbParam = m_pGameDeskPtr->GetThisGameGoodCardParam(i);
        vecGoodParam.push_back(std::move(ptrGoodParam));
        //m_lUserMaxScore[i] = lUserScore;
    }

    std::sort(vecGoodParam.begin(), vecGoodParam.end(), [](std::unique_ptr<tagGoodCardParam>& a, std::unique_ptr<tagGoodCardParam>& b)
    {
        return a->cbParam > b->cbParam;
    });

    //m_lMaxCellScore = m_wCurJetton[MAX_JETTON_NUM - 1];

    for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
    {
        if (0 == m_cbPlayStatus[i])
            continue;

        //手牌
        //memcpy(&m_cbHandCardData[i], &m_cbRepertoryCard[m_cbSendCardCount], sizeof(CT_BYTE)*MAX_COUNT);
        //m_cbSendCardCount += MAX_COUNT;

        //设置玩家游戏局数
        CT_DWORD dwPlayCount = m_pGameDeskPtr->GetUserPlayThisGameCount(i);
        m_pGameDeskPtr->SetUserPlayThisGameCount(i, ++dwPlayCount);
        CT_BOOL bSucc = false;
        if (!vecGoodParam.empty())
        {
            if (vecGoodParam[0]->wChairID == i)
            {
                std::uniform_int_distribution<> disRandNum(1, 100);
                CT_BYTE cbRandNum = disRandNum(m_random_gen);
                if (cbRandNum <= vecGoodParam[0]->cbParam)
                {
                    bSucc = GetCardFromDepot2(m_cbHandCardData[i]);
                }
                else
                {
                    bSucc = GetCardFromDepot1(m_cbHandCardData[i]);
                }
                if (!bSucc)
                {
                    LOG(WARNING) << "get card from depot fail, user id: " << m_pGameDeskPtr->GetUserID(i);
                    bSucc = GetRandCard(m_cbHandCardData[i]);
                }
            }
            else
            {
                bSucc = GetRandCard(m_cbHandCardData[i]);
            }
        }
        else
        {
            bSucc = GetRandCard(m_cbHandCardData[i]);
        }
    }
    */
}

//获得下一个玩家
//CT_DWORD CGameProcess::GetNextUser(CT_DWORD dwChairID)
//{
//	return (dwChairID + 1) % GAME_PLAYER;
//}

void CGameProcess::OnAllUserReady()
{
	m_cbGameStatus = GAME_SCENE_FREE;
	//InitGameData();
	for (CT_WORD i = 0; i != GAME_PLAYER; ++i)
	{
		//注意这里，如果有一个玩家进来时已经是准备状态，就不能再设置准备
		//（因为都准备状态后，游戏会开始，然后这里再设置成准备状态，会引起玩家状态不对）
		if (m_pGameDeskPtr->IsExistUser(i) && !(m_pGameDeskPtr->GetUserStatus(i) >= sReady))
		{
			//LOG(ERROR) << "set user ready, user id: " << m_pGameDeskPtr->GetUserID(i);
			m_pGameDeskPtr->SetUserReady(i);
		}
	}
}

//处理all消息
//2017年12月4日 21:21:40 修改allIn 功能
/*void CGameProcess::HandleAllIn(bool bGiveUp)
{
	//2017年12月4日 21:21:40 修改allIn 功能
	if (!m_bAllInState)
		return;

	CT_LONGLONG llUserAddJetton = m_lAllInJetton;
	if (!bGiveUp)
	{
		//判断当前用户的钱是否足够
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser);
		if (llUserScore < m_lAllInJetton)
		{
			LOG(WARNING) << "all in action, but user not enough score, all in score: " << m_lAllInJetton
				<< ", user score: " << llUserScore << ", userid: " << m_pGameDeskPtr->GetUserID(m_wCurrentUser);
			llUserAddJetton = llUserScore;
		}

		//压注
		m_pGameDeskPtr->AddUserScore(m_wCurrentUser, -llUserAddJetton, false);
		m_lTableScore[m_wCurrentUser] += llUserAddJetton;
		m_wAddScoreCount[m_wCurrentUser] += 1;
	}

	CT_WORD wNextPlayer = INVALID_CHAIR;
	for (CT_WORD i = 1; i < m_wPlayerCount; i++)
	{
		//设置变量
		wNextPlayer = (m_wCurrentUser + i) % m_wPlayerCount;

		//继续判断
		if (m_cbPlayStatus[wNextPlayer] == true) break;
	}

	if (wNextPlayer == m_wStartAllInUser)
	{
		CMD_S_AllIn_Result allInResult;
		memset(&allInResult, 0, sizeof(allInResult));

		//参与比牌的用户
		allInResult.wStartAllInUser = m_wStartAllInUser;
		allInResult.cbStartAllInUserWin = 1;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (m_cbPlayStatus[i])
			{
				allInResult.wCompareUser[i] = i;
			}
			else
			{
				allInResult.wCompareUser[i] = INVALID_CHAIR;
			}
		}

		//比牌
		//还在游戏中的玩家,互相比牌
		for (CT_WORD i = 0; i < m_wPlayerCount; ++i)
		{
			if (m_cbPlayStatus[i] == false)
				continue;

			for (CT_WORD j = 0; j < m_wPlayerCount; ++j)
			{
				if (m_cbPlayStatus[j] == false || i == j)
					continue;

				m_wCompardUser[i].push_back(j);
			}
		}

		bool bStartAllInUserWin = true;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (m_cbPlayStatus[i] == false || i == m_wStartAllInUser)
				continue;

			//比较大小
			CT_WORD wChairID = m_GameLogic.CompareCard(m_cbHandCardData[m_wStartAllInUser], m_cbHandCardData[i], MAX_COUNT);
			if (wChairID != 1)
			{
				//比牌输后，将首先发起All in的玩家设为非游戏状态
				m_cbPlayStatus[m_wStartAllInUser] = false;
				bStartAllInUserWin = false;
				allInResult.cbStartAllInUserWin = 0;
				break;
			}
		}
	
		allInResult.wAllInUser = m_wCurrentUser;
		if (bGiveUp)
		{
			allInResult.cbAllInUserGiveUp = 1;
		}
		else
		{
			allInResult.cbAllInUserGiveUp = 0;
			allInResult.dAllInJettonCount = llUserAddJetton*TO_DOUBLE;
			allInResult.dAllInUserTotalJetton = m_lTableScore[m_wCurrentUser] * TO_DOUBLE;
			for (CT_WORD i = 0; i < GAME_PLAYER; i++)
			{
				allInResult.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
			}
			allInResult.dAllInUserCurrentScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser)*TO_DOUBLE;
		}

		//first all in user win
		if (bStartAllInUserWin)
		{
			//其他玩家设为无效状态
			for (CT_WORD i = 0; i < m_wPlayerCount; i++)
			{
				if (m_cbPlayStatus[i] == false || i == m_wStartAllInUser)
					continue;

				m_cbPlayStatus[i] = false;

				if (m_pGameDeskPtr->GetUserStatus(i) == sPlaying)
				{
					m_pGameDeskPtr->SetUserStatus(i, sSit);
				}
			}

			allInResult.wCurrentUser = INVALID_CHAIR;
			//CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
			//allInResult.wTimeLeft = wTimeLeft;
			//m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
			//m_wTotalOpTime = wTimeLeft;
			m_pGameDeskPtr->SendTableData(INVALID_CHAIR, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult));

			//玩家下注定时器
			//m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + 2) * 1000);
			//m_wCurrentUser = wNextPlayer;

			//比牌结束后，先将allIn状态重置
			m_bAllInState = false;
			m_wStartAllInUser = INVALID_CHAIR;
			m_lAllInJetton = 0;
			m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
			OnEventGameEnd(m_wStartAllInUser , (GameEndTag)GER_COMPARECARD);
		}
		else //no first all in user win 
		{
			//人数统计
			CT_WORD wPlayerCount = 0;
			for (CT_WORD i = 0; i < m_wPlayerCount; i++)
			{
				if (m_cbPlayStatus[i] == true)
				{
					wPlayerCount++;
				}
			}

			for (CT_WORD i = 1; i < m_wPlayerCount; i++)
			{
				//设置变量
				wNextPlayer = (m_wCurrentUser + i) % m_wPlayerCount;
				//继续判断
				if (m_cbPlayStatus[wNextPlayer] == true) break;
			}

			//比牌结束后，先将allIn状态重置
			m_bAllInState = false;
			m_wStartAllInUser = INVALID_CHAIR;
			m_lAllInJetton = 0;

			if (wPlayerCount < 2)
			{
				allInResult.wCurrentUser = INVALID_CHAIR;
				//CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
				//allInResult.wTimeLeft = wTimeLeft;
				//m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
				//m_wTotalOpTime = wTimeLeft;
				m_pGameDeskPtr->SendTableData(INVALID_CHAIR, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult));
				m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
				OnEventGameEnd(m_wStartAllInUser, (GameEndTag)GER_COMPARECARD);
			}
			else
			{

				allInResult.wCurrentUser = wNextPlayer;
				allInResult.wJettonCount = m_wAddScoreCount[wNextPlayer];

				CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
				allInResult.wTimeLeft = wTimeLeft;
				m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
				m_wTotalOpTime = wTimeLeft;
				//发送数据
				for (CT_WORD i = 0; i < GAME_PLAYER; i++)
				{
					if (!m_pGameDeskPtr->IsExistUser(i))
						continue;

					CT_BYTE cbTimes = m_bMingZhu[i] ? 2 : 1;
					allInResult.dCurrentJetton = (m_lCurrentJetton*cbTimes)*TO_DOUBLE;
					m_pGameDeskPtr->SendTableData(i, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult));
				}
				//玩家下注定时器
				m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft+2) * 1000);
				m_wCurrentUser = wNextPlayer;
				if (m_bAutoFollow[m_wCurrentUser])
				{
					m_pGameDeskPtr->SetGameTimer(IDI_AUTO_FOLLOW, TIME_AUTO_FOLLOW * 1000);
				}
			}
		}
	}
	else
	{
		//构造数据
		CMD_S_AllIn allIn;
		allIn.wCurrentUser = wNextPlayer;
		allIn.wJettonCount = m_wAddScoreCount[wNextPlayer];
		allIn.dCurrentJetton = m_lAllInJetton*TO_DOUBLE;

		allIn.wAllInUser = m_wCurrentUser;
		if (bGiveUp)
		{
			allIn.cbAllInUserGiveUp = 1;
		}
		else
		{
			allIn.cbAllInUserGiveUp = 0;
			allIn.dAllInJettonCount = llUserAddJetton*TO_DOUBLE;
			allIn.dAllInUserTotalJetton = m_lTableScore[m_wCurrentUser] * TO_DOUBLE;
			for (CT_WORD i = 0; i < GAME_PLAYER; i++)
			{
				allIn.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
			}
			allIn.dAllInUserCurrentScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser)*TO_DOUBLE;
		}

		CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
		allIn.wTimeLeft = wTimeLeft;
		m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
		m_wTotalOpTime = wTimeLeft;

		m_pGameDeskPtr->SendTableData(INVALID_CHAIR, SUB_S_ALL_IN, &allIn, sizeof(allIn));

		//玩家下注定时器
		m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + 2) * 1000);
		m_wCurrentUser = wNextPlayer;
		if (m_bAutoFollow[m_wCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_AUTO_FOLLOW, TIME_AUTO_FOLLOW * 1000);
		}
	}
}
*/

void CGameProcess::HandleAllIn()
{
	CT_LONGLONG llUserAddJetton = m_pGameDeskPtr->GetUserScore(m_wCurrentUser);
	//压注
	m_pGameDeskPtr->AddUserScore(m_wCurrentUser, -llUserAddJetton, false);
	m_lTableScore[m_wCurrentUser] += llUserAddJetton;
	m_wAddScoreCount[m_wCurrentUser] += 1;


	CMD_S_AllIn_Result allInResult;
	memset(&allInResult, 0, sizeof(allInResult));

	//参与比牌的用户
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		if (m_cbPlayStatus[i])
		{
			allInResult.wCompareUser[i] = 1;
		}
		else
		{
			allInResult.wCompareUser[i] = INVALID_CHAIR;
		}
	}

	//比牌
	//还在游戏中的玩家,互相比牌
	for (CT_WORD i = 0; i < m_wPlayerCount; ++i)
	{
		if (m_cbPlayStatus[i] == false)
			continue;

		for (CT_WORD j = 0; j < m_wPlayerCount; ++j)
		{
			if (m_cbPlayStatus[j] == false || i == j)
				continue;

			m_wCompardUser[i].push_back(j);
		}
	}

	//判断孤注一掷的用户是否能赢出
	bool bStartAllInUserWin = true;
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		if (m_cbPlayStatus[i] == false || i == m_wCurrentUser)
			continue;

		//比较大小
		CT_WORD wChairID = m_GameLogic.CompareCard(m_cbHandCardData[m_wCurrentUser], m_cbHandCardData[i], MAX_COUNT);
		if (wChairID != 1)
		{
			//比牌输后，将首先发起All in的玩家设为非游戏状态
			m_cbPlayStatus[m_wCurrentUser] = false;
			bStartAllInUserWin = false;
			break;
		}
	}

	allInResult.wAllInUser = m_wCurrentUser;
	allInResult.dAllInJettonCount = llUserAddJetton*TO_DOUBLE;
	allInResult.dAllInUserTotalJetton = m_lTableScore[m_wCurrentUser] * TO_DOUBLE;
	for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	{
		allInResult.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
	}
	allInResult.dAllInUserCurrentScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser)*TO_DOUBLE;
	allInResult.cbStartAllInUserWin = bStartAllInUserWin ? 1 : 0;
	//first all in user win
	if (bStartAllInUserWin)
	{
		//其他玩家设为无效状态
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (m_cbPlayStatus[i] == false || i == m_wCurrentUser)
				continue;

			m_cbPlayStatus[i] = false;
			//UpdateGoodCardInfo(i, false);
			UpdateUserScoreInfo(i, -m_lTableScore[i], 0);

			if (m_pGameDeskPtr->GetUserStatus(i) != sOffLine)
				m_pGameDeskPtr->SetUserStatus(i, sSit);
		}

		allInResult.wCurrentUser = INVALID_CHAIR;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
				continue;
			if (i == m_wCurrentUser)
				m_pGameDeskPtr->SendTableData(i, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult), true);
			else
				m_pGameDeskPtr->SendTableData(i, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult));
		}

		//比牌结束后，先将allIn状态重置
		m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
		OnEventGameEnd(INVALID_CHAIR, (GameEndTag)GER_ALLIN);
	}
	else //no all in user win 
	{
		//人数统计
		CT_WORD wPlayerCount = 0;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (m_cbPlayStatus[i] == true)
			{
				wPlayerCount++;
			}
		}

		if (wPlayerCount < 2)
		{
			allInResult.wCurrentUser = INVALID_CHAIR;
			allInResult.wJettonCount = m_wAddScoreCount[m_wCurrentUser];
			for (CT_WORD i = 0; i < m_wPlayerCount; i++)
			{
				if (!m_pGameDeskPtr->IsExistUser(i))
					continue;
				if (i == m_wCurrentUser)
					m_pGameDeskPtr->SendTableData(i, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult), true);
				else
					m_pGameDeskPtr->SendTableData(i, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult));
			}
			m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
			OnEventGameEnd(INVALID_CHAIR, (GameEndTag)GER_ALLIN);
		}
		else
		{
			CT_WORD wNextPlayer = INVALID_CHAIR;
			for (CT_WORD i = 1; i < m_wPlayerCount; i++)
			{
				//设置变量
				wNextPlayer = (m_wCurrentUser + i) % m_wPlayerCount;
				//继续判断
				if (m_cbPlayStatus[wNextPlayer] == true) break;
			}

			allInResult.wCurrentUser = wNextPlayer;
			allInResult.wJettonCount = m_wAddScoreCount[wNextPlayer];
			allInResult.dRustJetton = floor(GetLeastUserScore() * TO_DOUBLE);

			CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
			allInResult.wTimeLeft = wTimeLeft;
			m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
			m_wTotalOpTime = wTimeLeft;
			//发送数据
			for (CT_WORD i = 0; i < GAME_PLAYER; i++)
			{
				if (!m_pGameDeskPtr->IsExistUser(i))
					continue;

				CT_BYTE cbTimes = m_bMingZhu[i] ? 2 : 1;
				allInResult.dCurrentJetton = (m_lCurrentJetton*cbTimes)*TO_DOUBLE;
				if (i == m_wCurrentUser)
					m_pGameDeskPtr->SendTableData(i, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult), true);
				else
					m_pGameDeskPtr->SendTableData(i, SUB_S_ALL_IN_RESULT, &allInResult, sizeof(allInResult));
			}

			//玩家下注定时器
			m_wCurrentUser = wNextPlayer;
			if (!HandleMorethanRoundEnd())
			{
				m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + TIME_NETWORK_COMPENSATE) * 1000);
				if (m_bAutoFollow[m_wCurrentUser])
				{
					m_pGameDeskPtr->SetGameTimer(IDI_AUTO_FOLLOW, TIME_AUTO_FOLLOW * 1000);
				}
			}
		}
	}
}

void CGameProcess::HandleRush(bool bGiveUp)
{
	if (!m_bRushState)
		return;

	CT_LONGLONG llUserAddJetton = m_lRushJetton;
	if (!bGiveUp)
	{
		//判断当前用户的钱是否足够
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser);
		if (llUserScore < m_lRushJetton)
		{
			LOG(WARNING) << "rush action, but user not enough score, all in score: " << m_lRushJetton
				<< ", user score: " << llUserScore << ", user id: " << m_pGameDeskPtr->GetUserID(m_wCurrentUser);
			llUserAddJetton = llUserScore;
		}

		//压注
		m_pGameDeskPtr->AddUserScore(m_wCurrentUser, -llUserAddJetton, false);
		m_lTableScore[m_wCurrentUser] += llUserAddJetton;
		m_wAddScoreCount[m_wCurrentUser] += 1;
	}

	CT_WORD wNextPlayer = INVALID_CHAIR;
	for (CT_WORD i = 1; i < m_wPlayerCount; i++)
	{
		//设置变量
		wNextPlayer = (m_wCurrentUser + i) % m_wPlayerCount;

		//继续判断
		if (m_cbPlayStatus[wNextPlayer] == true) break;
	}

	if (wNextPlayer == m_wStartRushUser)
	{
		CMD_S_Rush_Result rushResult;
		memset(&rushResult, 0, sizeof(rushResult));

		//参与比牌的用户
		//rushResult.wStartRushUser = m_wStartRushUser;
		//rushResult.cbStartRushUserWin = 1;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (m_cbPlayStatus[i])
			{
				rushResult.wCompareUser[i] = i;
			}
			else
			{
				rushResult.wCompareUser[i] = INVALID_CHAIR;
			}
		}

		//bool bStartRushUserWin = true;
		CT_WORD wRushWinUser = m_wStartRushUser;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (m_cbPlayStatus[i] == false || wRushWinUser == i || m_wStartRushUser == i)
				continue;
		
			//比较大小
			CT_WORD wChairID = m_GameLogic.CompareCard(m_cbHandCardData[wRushWinUser], m_cbHandCardData[i], MAX_COUNT, 1);

			CT_WORD wLostUser, wWinUser;
			if (wChairID == 1)
			{
				wWinUser = wRushWinUser;
				wLostUser = i;
			}
			else
			{
				wWinUser = i;
				wLostUser = wRushWinUser;
				wRushWinUser = i;
			}

			//设置数据
			m_wCompardUser[wLostUser].push_back(wWinUser);
			m_wCompardUser[wWinUser].push_back(wLostUser);
			m_cbPlayStatus[wLostUser] = false;
			//UpdateGoodCardInfo(wLostUser, false);
			UpdateUserScoreInfo(wLostUser, -m_lTableScore[wLostUser], 0);
			if (m_pGameDeskPtr->GetUserStatus(wLostUser) != sOffLine)
				m_pGameDeskPtr->SetUserStatus(wLostUser, sSit);
		}

		//if (wWinUser != m_wStartRushUser)
		//{
			//bStartRushUserWin = false;
			//rushResult.cbStartRushUserWin = 0;
		//}
		rushResult.wRushWinUser = wRushWinUser;

		rushResult.wRushUser = m_wCurrentUser;
		if (bGiveUp)
		{
			rushResult.cbRushUserGiveUp = 1;
		}
		else
		{
			rushResult.cbRushUserGiveUp = 0;
			rushResult.dRushJettonCount = llUserAddJetton*TO_DOUBLE;
			rushResult.dRushUserTotalJetton = m_lTableScore[m_wCurrentUser] * TO_DOUBLE;
			for (CT_WORD i = 0; i < GAME_PLAYER; i++)
			{
				rushResult.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
			}
			rushResult.dRushUserCurrentScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser)*TO_DOUBLE;
		}
		
		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
				continue;
			
			if (i == wRushWinUser)
				m_pGameDeskPtr->SendTableData(i, SUB_S_RUSH_RESULT, &rushResult, sizeof(rushResult), true);
			else
				m_pGameDeskPtr->SendTableData(i, SUB_S_RUSH_RESULT, &rushResult, sizeof(rushResult));
		}
		

		//比牌结束后，先将allIn状态重置
		m_bRushState = false;
		m_wStartRushUser = INVALID_CHAIR;
		m_lRushJetton = 0;
		m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
		if (bGiveUp)
		{
			OnEventGameEnd(INVALID_CHAIR, (GameEndTag)GER_RUSH_GIVEUP);
		}
		else
		{
			OnEventGameEnd(INVALID_CHAIR, (GameEndTag)GER_RUSH);
		}
	}
	else
	{
		//构造数据
		CMD_S_Rush rush;
		rush.wCurrentUser = wNextPlayer;
		rush.wJettonCount = m_wAddScoreCount[wNextPlayer];
		rush.dCurrentJetton = m_lRushJetton*TO_DOUBLE;

		rush.wRushUser = m_wCurrentUser;
		if (bGiveUp)
		{
			rush.cbRushUserGiveUp = 1;
		}
		else
		{
			rush.cbRushUserGiveUp = 0;
			rush.dRushJettonCount = llUserAddJetton*TO_DOUBLE;
			rush.dRushUserTotalJetton = m_lTableScore[m_wCurrentUser] * TO_DOUBLE;
			for (CT_WORD i = 0; i < GAME_PLAYER; i++)
			{
				rush.dTotalJetton += m_lTableScore[i] * TO_DOUBLE;
			}
			rush.dRushUserCurrentScore = m_pGameDeskPtr->GetUserScore(m_wCurrentUser)*TO_DOUBLE;
		}

		CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
		rush.wTimeLeft = wTimeLeft;
		m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
		m_wTotalOpTime = wTimeLeft;
		
		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
				continue;
			
			if (i == wNextPlayer)
				m_pGameDeskPtr->SendTableData(i, SUB_S_RUSH, &rush, sizeof(rush), true);
			else
				m_pGameDeskPtr->SendTableData(i, SUB_S_RUSH, &rush, sizeof(rush));
		}

		//玩家下注定时器
		m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + TIME_NETWORK_COMPENSATE) * 1000);
		m_wCurrentUser = wNextPlayer;
		if (m_bAutoFollow[m_wCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_AUTO_FOLLOW, TIME_AUTO_FOLLOW * 1000);
		}
	}
}

void CGameProcess::HandleAutoFollow(CT_WORD wChairID)
{
	if (m_wCurrentUser != wChairID || m_bAutoFollow[wChairID] == false || m_cbPlayStatus[wChairID] == 0)
		return;

	CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);
	if (llUserScore <= 0)
	{
		OnUserGiveUp(wChairID);
		return;
	}

	if (m_bRushState)
	{
		OnUserRush(wChairID);
		return;
	}

	CT_LONGLONG lTimes = 1;
	if (m_bMingZhu[wChairID])
		lTimes = 2;

	CT_LONGLONG llNeedScore = m_lCurrentJetton*lTimes;
	if (llNeedScore >= llUserScore)
		OnUserAllIn(wChairID);
	else
		OnUserAddScore(wChairID, llNeedScore, false);
}

bool CGameProcess::HandleMorethanRoundEnd()
{
	for (int i = 0; i < m_wPlayerCount; ++i)
	{
		if (m_cbPlayStatus[i] == false)
			continue;

		CT_WORD wJettonCount = m_wAddScoreCount[i];

		if (wJettonCount < (MAX_JETTON_ROUND+1))
		{
			return false;
		}
	}

	//所有玩家都超过20局则结束游戏
	/*//现在所有玩家之间互相比牌
	for (CT_WORD i = 0; i < m_wPlayerCount; ++i)
	{
		if (m_cbPlayStatus[i] == false)
			continue;

		for (CT_WORD j = 0; j < m_wPlayerCount; ++j)
		{
			if (m_cbPlayStatus[j] == false || i == j)
				continue;

			m_wCompardUser[i].push_back(j);
		}
	}*/

	CMD_S_OutRound_Result outRoundResult;
	outRoundResult.wCompareUser[m_wCurrentUser] = 1;
	//找到最游戏
	CT_WORD wRushWinUser = m_wCurrentUser;
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		if (m_cbPlayStatus[i] == false || wRushWinUser == i)
			continue;

		outRoundResult.wCompareUser[i] = 1;

		//比较大小
		CT_WORD wChairID = m_GameLogic.CompareCard(m_cbHandCardData[wRushWinUser], m_cbHandCardData[i], MAX_COUNT, 1);
		CT_WORD wLostUser, wWinUser;
		if (wChairID == 1)
		{
			wWinUser = wRushWinUser;
			wLostUser = i;
		}
		else
		{
			wWinUser = i;
			wLostUser = wRushWinUser;
			wRushWinUser = i;
		}

		//设置数据
		m_wCompardUser[wLostUser].push_back(wWinUser);
		m_wCompardUser[wWinUser].push_back(wLostUser);
		m_cbPlayStatus[wLostUser] = false;
		//UpdateGoodCardInfo(wLostUser, false);
		UpdateUserScoreInfo(wLostUser, -m_lTableScore[wLostUser], 0);
		if (m_pGameDeskPtr->GetUserStatus(wLostUser) != sOffLine)
		{
			m_pGameDeskPtr->SetUserStatus(wLostUser, sSit);
		}
	}

	outRoundResult.wRushWinUser = wRushWinUser;
	
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
			continue;
		
		if (i == m_wCurrentUser)
			SendTableData(i, SUB_S_OUT_ROUND_END, &outRoundResult, sizeof(outRoundResult), true);
		else
			SendTableData(i, SUB_S_OUT_ROUND_END, &outRoundResult, sizeof(outRoundResult));
	}
	

	OnEventGameEnd(INVALID_CHAIR, (GameEndTag)GER_RUSH);
	return true;
}

//放弃事件
bool CGameProcess::OnUserGiveUp(CT_WORD wChairID, bool bExit)
{
	/*if (m_wCurrentUser != wChairID)
	{
		return false;
	}*/

	//设置数据
	m_cbPlayStatus[wChairID] = false;
	//UpdateGoodCardInfo(wChairID, false);
	UpdateUserScoreInfo(wChairID, -m_lTableScore[wChairID], 0);


	m_wCompardUser[wChairID].clear();

	//如果是自动加注状态弃牌，则设为非自动加注状态
	if (m_bAutoFollow[wChairID])
	{
		m_bAutoFollow[wChairID] = false;
	}

	//谁弃牌，谁设为坐下状态 
	if (m_pGameDeskPtr->GetUserStatus(wChairID) == sPlaying)
	{
		m_pGameDeskPtr->SetUserStatus(wChairID, sSit);
	}

	if (bExit)
	{
		//m_bGameStatus[wChairID] = 0;
		//m_pGameDeskPtr->ClearTableUser(wChairID, true, true);
		//m_pGameDeskPtr->SetUserStatus(wChairID, sOffLine);
		//记录弃牌
		m_cbSystemGiveUp[wChairID] = 1;
	}

	if (m_wCurrentUser == wChairID && m_bRushState)
	{
		HandleRush(true);
		return true;
	}

	//人数统计
	CT_WORD wPlayerCount = 0;
	for (CT_WORD i = 0; i<m_wPlayerCount; i++)
	{
		if (m_cbPlayStatus[i] == true)
		{
			wPlayerCount++;
		}
	}

	//记录弃牌
	m_cbGiveUp[wChairID] = true;

	//判断结束
	if (wPlayerCount >= 2)
	{
		//发送消息
		CMD_S_GiveUp GiveUp;
		GiveUp.wGiveUpUser = wChairID;

		//如果不是当前玩家弃牌，则不用切换玩家
		if (m_wCurrentUser != wChairID)
		{
			GiveUp.cbSwitchUser = 0;
			CT_BOOL bRecord = false;
			for (CT_WORD i = 0; i < m_wPlayerCount; i++)
			{
				if (!m_pGameDeskPtr->IsExistUser(i))
					continue;

				if (!bRecord)
				{
					bRecord = true;
					m_pGameDeskPtr->SendTableData(i, SUB_S_GIVE_UP, &GiveUp, sizeof(GiveUp), true);
				}
				else
				{
					m_pGameDeskPtr->SendTableData(i, SUB_S_GIVE_UP, &GiveUp, sizeof(GiveUp));
				}
			}
			return true;
		}
		GiveUp.cbSwitchUser = 1;
		//用户切换
		CT_WORD wNextPlayer = INVALID_CHAIR;
		for (CT_WORD i = 1; i < m_wPlayerCount; i++)
		{
			//设置变量
			wNextPlayer = (m_wCurrentUser + i) % m_wPlayerCount;

			//继续判断
			if (m_cbPlayStatus[wNextPlayer] == true) break;
		}
		m_wCurrentUser = wNextPlayer;
		GiveUp.wCurrentUser = m_wCurrentUser;
		GiveUp.wJettonCount = m_wAddScoreCount[m_wCurrentUser];
		GiveUp.dRustJetton = floor(GetLeastUserScore() * TO_DOUBLE);

		CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
		GiveUp.wTimeLeft = wTimeLeft;
		m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
		m_wTotalOpTime = wTimeLeft;
		

		if (!HandleMorethanRoundEnd())
		{
			//玩家下注定时器
			m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + TIME_NETWORK_COMPENSATE) * 1000);

			CT_WORD wSubCmdID = bExit ? SUB_S_GIVE_UP_TIME_OUT : SUB_S_GIVE_UP;
			
			for (CT_WORD i = 0; i < m_wPlayerCount; i++)
			{
				if (!m_pGameDeskPtr->IsExistUser(i))
					continue;
				
				if (i == m_wCurrentUser)
					m_pGameDeskPtr->SendTableData(i, wSubCmdID, &GiveUp, sizeof(GiveUp), true);
				else
					m_pGameDeskPtr->SendTableData(i, wSubCmdID, &GiveUp, sizeof(GiveUp));
			}
			
			if (m_bAutoFollow[m_wCurrentUser])
			{
				m_pGameDeskPtr->SetGameTimer(IDI_AUTO_FOLLOW, TIME_AUTO_FOLLOW * 1000);
			}
			//OnUserAddScore(wChairID, 0L, true, false);
		}
	}
	else
	{
		CMD_S_GiveUp GiveUp;
		GiveUp.cbSwitchUser = 0;
		GiveUp.wGiveUpUser = wChairID;
		GiveUp.wCurrentUser = INVALID_CHAIR;
		CT_BOOL bRecord = false;
        CT_WORD wSubCmdID = bExit ? SUB_S_GIVE_UP_TIME_OUT : SUB_S_GIVE_UP;
		for (CT_WORD i = 0; i < m_wPlayerCount; i++)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
				continue;
			
			if (!bRecord)
			{
				bRecord = true;
				m_pGameDeskPtr->SendTableData(i, wSubCmdID, &GiveUp, sizeof(GiveUp), true);
			}
			else
			{
				m_pGameDeskPtr->SendTableData(i, wSubCmdID, &GiveUp, sizeof(GiveUp));
			}
		}

		m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
		OnEventGameEnd(INVALID_CHAIR, (GameEndTag)GER_NO_PLAYER);
	}

	return true;
}

//看牌事件
bool CGameProcess::OnUserLookCard(CT_WORD wChairID)
{
	//状态效验
	//if (m_wCurrentUser != wChairID)
	//	return false;

	//检测玩家能否比牌(庄家任何时候可以看牌)
	if (wChairID != m_wBankerUser)
	{
		if (!(m_wAddScoreCount[wChairID] >= CAN_LOOKCARD_JETTON_COUNT))
		{
			LOG(WARNING) << "look card, but add jetton count is :" << m_wAddScoreCount[wChairID];
			return false;
		}
	}

	//参数效验
	if (m_bMingZhu[wChairID])
		return true;

	//设置参数
	m_bMingZhu[wChairID] = true;

	//构造数据
	CMD_S_LookCard LookCard;
	memset(&LookCard, 0, sizeof(LookCard));
	LookCard.wLookCardUser = wChairID;
	LookCard.wJettonCount = m_wAddScoreCount[m_wCurrentUser];
	LookCard.dRustJetton = m_bRushState ? m_lRushJetton*TO_DOUBLE : floor(GetLeastUserScore() * TO_DOUBLE);
	//LookCard.cbAllInState = m_bAllInState;
	LookCard.wCurrentUser = m_wCurrentUser;

	//当前玩家看牌后重置注定时器
	if (m_wCurrentUser == wChairID)
	{
		CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
		LookCard.cbTimeLeft = (CT_BYTE)wTimeLeft;
		LookCard.cbTotalTime = (CT_BYTE)wTimeLeft;
		m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
		m_wTotalOpTime = wTimeLeft;
		m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + TIME_NETWORK_COMPENSATE) * 1000);
	}
	else
	{
		CT_BYTE cbTimeLeft = (CT_BYTE)(m_dwOpEndTime - (CT_DWORD)time(NULL));
		LookCard.cbTimeLeft = cbTimeLeft;
		LookCard.cbTotalTime = (CT_BYTE)m_wTotalOpTime;
	}

	for (CT_WORD i = 0; i< GAME_PLAYER; i++)
	{
		CT_BOOL bExist = m_pGameDeskPtr->IsExistUser(i);
		if (!bExist)
		{
			continue;
		}

		if (i == wChairID)
		{
			memcpy(LookCard.cbCardData, m_cbHandCardData[wChairID], sizeof(m_cbHandCardData[0]));
			LookCard.cbCardType = m_cbHandCardType[wChairID];
		}
		else
		{
			memset(LookCard.cbCardData, 0, sizeof(LookCard.cbCardData));
		}
		CT_BYTE cbTimes = m_bMingZhu[i] ? 2 : 1;
		LookCard.dCurrentJetton = m_lCurrentJetton * cbTimes * TO_DOUBLE;
		
		if (i == wChairID)
		{
			m_pGameDeskPtr->SendTableData(i, SUB_S_LOOK_CARD, &LookCard, sizeof(LookCard), true);
		}
		else
		{
			m_pGameDeskPtr->SendTableData(i, SUB_S_LOOK_CARD, &LookCard, sizeof(LookCard), false);
		}
	}

	return true;
}

//比牌事件
bool CGameProcess::OnUserCompareCard(CT_WORD wFirstChairID, CT_WORD wNextChairID)
{
	if (m_bRushState)
	{
		return false;
	}

	//检测玩家能否比牌
	if (!(m_wAddScoreCount[wFirstChairID] > CAN_LOOKCARD_JETTON_COUNT))
	{
		LOG(WARNING) << "compare card fail, add jetton count is :" << m_wAddScoreCount[wFirstChairID] << ", user id: " << m_pGameDeskPtr->GetUserID(wFirstChairID);
		return false;
	}

	//检测玩家的钱是否足够比牌
	CT_LONGLONG lTimes = 2;
	if (m_bMingZhu[wFirstChairID])
		lTimes = 4;

	CT_LONGLONG llNeedScore = m_lCurrentJetton*lTimes;
	if (llNeedScore > m_pGameDeskPtr->GetUserScore(wFirstChairID))
	{
		LOG(WARNING) << "compare card fail, user score :" << m_pGameDeskPtr->GetUserScore(wFirstChairID) << ", need score: " << llNeedScore << ", user id: " << m_pGameDeskPtr->GetUserID(wFirstChairID);
		return false;
	}

	//下注
	OnUserAddScore(wFirstChairID, llNeedScore, true);

	//比较大小
	CT_WORD wChairID = m_GameLogic.CompareCard(m_cbHandCardData[wFirstChairID], m_cbHandCardData[wNextChairID], MAX_COUNT);

	//状态设置
	//m_lCompareCount = 0;

	//胜利用户
	CT_WORD wLostUser, wWinUser;
	if (wChairID == 1)
	{
		wWinUser = wFirstChairID;
		wLostUser = wNextChairID;
	}
	else
	{
		wWinUser = wNextChairID;
		wLostUser = wFirstChairID;
	}

	//设置数据
	m_wCompardUser[wLostUser].push_back(wWinUser);
	m_wCompardUser[wWinUser].push_back(wLostUser);
	m_cbPlayStatus[wLostUser] = false;
	//UpdateGoodCardInfo(wLostUser, false);
	UpdateUserScoreInfo(wLostUser, -m_lTableScore[wLostUser], 0);

	if (m_pGameDeskPtr->GetUserStatus(wLostUser) != sOffLine)
	{
		m_pGameDeskPtr->SetUserStatus(wLostUser, sSit);
	}

	//人数统计
	CT_WORD wPlayerCount = 0;
	for (CT_WORD i = 0; i<m_wPlayerCount; i++)
	{
		if (m_cbPlayStatus[i] == true)
			wPlayerCount++;
	}

	//构造数据
	CMD_S_CompareCard CompareCard;

	if (m_wCurrentUser != INVALID_CHAIR)
	{
		CompareCard.wJettonCount = m_wAddScoreCount[m_wCurrentUser];
		CompareCard.dUserCurrentScore = m_pGameDeskPtr->GetUserScore(wFirstChairID)*TO_DOUBLE;
	}

	//继续游戏
	CT_WORD wTimeLeft = 0;
	if (wPlayerCount >= 2)
	{
		//用户切换
		CT_WORD wNextPlayer = INVALID_CHAIR;
		for (CT_WORD i = 1; i<m_wPlayerCount; i++)
		{
			//设置变量
			wNextPlayer = (m_wCurrentUser + i) % m_wPlayerCount;

			//继续判断
			if (m_cbPlayStatus[wNextPlayer] == true)
				break;
		}
		//设置用户
		m_wCurrentUser = wNextPlayer;

		wTimeLeft = (TIME_GAME_ADD_SCORE + 5);
		CompareCard.wTimeLeft = wTimeLeft;
		m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
		m_wTotalOpTime = wTimeLeft;
	}
	else 
		m_wCurrentUser = INVALID_CHAIR;

	CompareCard.wCurrentUser = m_wCurrentUser;
	CompareCard.wLostUser = wLostUser;
	CompareCard.wCompareUser[0] = wFirstChairID;
	CompareCard.wCompareUser[1] = wNextChairID;

	CompareCard.wAddJettonUser = wFirstChairID;
	CompareCard.dAddJettonCount = llNeedScore*TO_DOUBLE;
	CompareCard.dUserTotalJetton = m_lTableScore[wFirstChairID] * TO_DOUBLE;
	CompareCard.dRustJetton = floor(GetLeastUserScore() * TO_DOUBLE);
	for (CT_WORD i = 0; i != GAME_PLAYER; ++i)
	{
		CompareCard.dTotalJetton += m_lTableScore[i]*TO_DOUBLE;
	}

	CT_BOOL bRecord = false;
	for (CT_WORD i = 0; i != GAME_PLAYER; ++i)
	{
		if (!m_pGameDeskPtr->IsExistUser(i))
			continue;

		CT_BYTE cbTime = m_bMingZhu[i] ? 2 : 1;
		CompareCard.dCurrentJetton = m_lCurrentJetton*cbTime*TO_DOUBLE;
		
		if (m_wCurrentUser != INVALID_CHAIR)
		{
			if (m_wCurrentUser == i)
			{
				m_pGameDeskPtr->SendTableData(i, SUB_S_COMPARE_CARD, &CompareCard, sizeof(CompareCard), true);
			}
			else
			{
				m_pGameDeskPtr->SendTableData(i, SUB_S_COMPARE_CARD, &CompareCard, sizeof(CompareCard), false);
			}
		}
		else
		{
			if (!bRecord)
			{
				bRecord = true;
				m_pGameDeskPtr->SendTableData(i, SUB_S_COMPARE_CARD, &CompareCard, sizeof(CompareCard), true);
			}
			else
			{
				m_pGameDeskPtr->SendTableData(i, SUB_S_COMPARE_CARD, &CompareCard, sizeof(CompareCard), false);
			}
		}
	}

	//结束游戏
	if (wPlayerCount<2)
	{
		m_wBankerUser = wWinUser;
		//m_wFlashUser[wNextChairID] = true;
		//m_wFlashUser[wFirstChairID] = true;
		m_pGameDeskPtr->KillGameTimer(IDI_GAME_ADD_SCORE);
		OnEventGameEnd(wWinUser, (GameEndTag)GER_COMPARECARD);
	}
	else
	{
		if (!HandleMorethanRoundEnd())
		{
			//玩家下注定时器
			m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + TIME_NETWORK_COMPENSATE) * 1000);
			if (m_bAutoFollow[m_wCurrentUser])
			{
				m_pGameDeskPtr->SetGameTimer(IDI_AUTO_FOLLOW, TIME_AUTO_FOLLOW * 1000);
			}
		}	
	}

	return true;
}

//开牌事件
bool CGameProcess::OnUserOpenCard(CT_WORD wUserID)
{
	if (m_wWinnerUser == INVALID_CHAIR)
		return false;

	if (wUserID != m_wWinnerUser)
		return false;

	//清理数据
	m_wCurrentUser = INVALID_CHAIR;
	//m_lCompareCount = 0;

	//保存扑克
	//CT_BYTE cbUserCardData[GAME_PLAYER][MAX_COUNT] = { 0 };
	//memcpy(cbUserCardData, m_cbHandCardData, sizeof(cbUserCardData));

	//比牌玩家
	//CT_WORD wWinner = wUserID;

	//查找最大玩家
	/*for (CT_WORD i = 1; i<m_wPlayerCount; i++)
	{
		CT_WORD w = (wUserID + i) % m_wPlayerCount;

		//用户过滤
		if (m_cbPlayStatus[w] == false) continue;

		//对比扑克
		if (m_GameLogic.CompareCard(cbUserCardData[w], cbUserCardData[wWinner], MAX_COUNT) >= 1)
		{
			wWinner = w;
		}
	}
	assert(m_cbPlayStatus[wWinner] == true);
	if (m_cbPlayStatus[wWinner] == false)return false;

	//胜利玩家
	m_wBankerUser = wWinner;*/

	//构造数据
	CMD_S_OpenCard OpenCard;
	OpenCard.wWinner = m_wWinnerUser;
	memcpy(OpenCard.cbCardData, m_cbHandCardData[m_wWinnerUser], sizeof(OpenCard.cbCardData));
	OpenCard.cbCardType = m_cbHandCardType[m_wWinnerUser];

	//发送数据
	m_pGameDeskPtr->SendTableData(INVALID_CHAIR, SUB_S_OPEN_CARD, &OpenCard, sizeof(OpenCard));

	//结束游戏
	//for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	//{
	//	if (m_cbPlayStatus[i] == true)
	//		m_wFlashUser[i] = true;
	//}

	//OnEventGameEnd(wWinner, GER_NORMAL);

	return true;
}

//加注事件
bool CGameProcess::OnUserAddScore(CT_WORD wChairID, CT_LONGLONG lScore, bool bCompareCard)
{
	if (m_bRushState)
	{
		return false;
	}

	//状态效验
	if (m_wCurrentUser != wChairID) return false;

	//当前倍数bGiveUp过滤了lScore为0
	CT_LONGLONG lTimes = (m_bMingZhu[wChairID]) ? 2 : 1;
	if (bCompareCard)
	{
		lTimes *= 2;
	}

	CT_LONGLONG llCurrentJetton = lScore / lTimes;
	CT_BOOL bInJetton = false;

	if (llCurrentJetton == m_lCurrentJetton)							//跟注或者比牌
	{
		bInJetton = true;
	}
	else if (m_bMingZhu[wChairID] && lScore <= m_llMaxJettonScore)		//看牌加注
	{
		if (llCurrentJetton == m_lCurrentJetton*JETTON_MULTIPLE_1
			|| llCurrentJetton == m_lCurrentJetton*JETTON_MULTIPLE_2
			|| lScore == m_llMaxJettonScore)
		{
			bInJetton = true;
		}
	}
	else if (!m_bMingZhu[wChairID] && lScore <= m_llMaxJettonScore / 2)	//不看牌加注
	{
		if (llCurrentJetton == m_lCurrentJetton*JETTON_MULTIPLE_1
			|| llCurrentJetton == m_lCurrentJetton*JETTON_MULTIPLE_2
			|| lScore == m_llMaxJettonScore/2)
		{
			bInJetton = true;
		}
	}

	//加注金币效验
	if (lScore < 0 || !bInJetton)
	{
		LOG(WARNING) << "OnUserAddScore " << m_pGameDeskPtr->GetUserID(wChairID) << " add score: " << lScore << ", user score: " << m_pGameDeskPtr->GetUserScore(wChairID)
		<< ", m_llMaxJettonScore : "<< m_llMaxJettonScore << ", current jetton: " << m_lCurrentJetton << ", is ming zhu: " << (int)m_bMingZhu[wChairID];
		return false;
	}

	//if ((lScore + m_lTableScore[wChairID]) > m_lUserMaxScore[wChairID])
	//	return false;

	if (m_pGameDeskPtr->GetUserScore(wChairID) < lScore)
	{ 
		LOG(WARNING) << "OnUserAddScore " << m_pGameDeskPtr->GetUserID(wChairID) << " add score: " << lScore << ", but user score : " << m_pGameDeskPtr->GetUserScore(wChairID)
			<< ", is not enough.";
		return false;
	}

	if (m_lCurrentJetton > llCurrentJetton)
		return false;

	CT_BYTE cbState = 1;
	if (llCurrentJetton > m_lCurrentJetton)
	{
		m_lCurrentJetton = llCurrentJetton;
		cbState = 2;
	}

	//用户注金
	m_lTableScore[wChairID] += lScore;

	//下注次数
	m_wAddScoreCount[wChairID] += 1;

	//扣用户分
	m_pGameDeskPtr->AddUserScore(wChairID, -lScore, false);

	//设置用户
	if (!bCompareCard)
	{
		//用户切换
		CT_WORD wNextPlayer = INVALID_CHAIR;
		for (CT_WORD i = 1; i<m_wPlayerCount; i++)
		{
			//设置变量
			wNextPlayer = (m_wCurrentUser + i) % m_wPlayerCount;

			//继续判断
			if (m_cbPlayStatus[wNextPlayer] == true) break;
		}
		m_wCurrentUser = wNextPlayer;

		//构造数据
		CMD_S_AddScore AddScore;
		AddScore.cbState = cbState;
		AddScore.wCurrentUser = m_wCurrentUser;
		AddScore.wJettonCount = m_wAddScoreCount[m_wCurrentUser] > MAX_JETTON_ROUND ? MAX_JETTON_ROUND : m_wAddScoreCount[m_wCurrentUser];
		
		AddScore.wAddJettonUser = wChairID;
		AddScore.dAddJettonCount = lScore*TO_DOUBLE;
		AddScore.dUserTotalJetton = m_lTableScore[wChairID] * TO_DOUBLE;
		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			AddScore.dTotalJetton += m_lTableScore[i]*TO_DOUBLE;
		}
		AddScore.dUserCurrentScore = m_pGameDeskPtr->GetUserScore(wChairID)*TO_DOUBLE;

		CT_WORD wTimeLeft = TIME_GAME_ADD_SCORE;
		AddScore.wTimeLeft = wTimeLeft;
		m_dwOpEndTime = (CT_DWORD)time(NULL) + wTimeLeft;
		m_wTotalOpTime = wTimeLeft;
		AddScore.dRustJetton = floor(GetLeastUserScore() * TO_DOUBLE);
		
		//发送数据
		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			if (!m_pGameDeskPtr->IsExistUser(i))
				continue;

			CT_BYTE cbTimes = m_bMingZhu[i] ? 2 : 1;
			AddScore.dCurrentJetton = (m_lCurrentJetton*cbTimes)*TO_DOUBLE;
			
			if (i == m_wCurrentUser)
			{
				m_pGameDeskPtr->SendTableData(i, SUB_S_ADD_SCORE, &AddScore, sizeof(AddScore), true);
			}
			else
			{
				m_pGameDeskPtr->SendTableData(i, SUB_S_ADD_SCORE, &AddScore, sizeof(AddScore));
			}
		}

		if (!HandleMorethanRoundEnd())
		{
			//玩家下注定时器
			m_pGameDeskPtr->SetGameTimer(IDI_GAME_ADD_SCORE, (wTimeLeft + TIME_NETWORK_COMPENSATE) * 1000);

			if (m_bAutoFollow[m_wCurrentUser])
			{
				m_pGameDeskPtr->SetGameTimer(IDI_AUTO_FOLLOW, TIME_AUTO_FOLLOW * 1000);
			}
		}
	}

	return true;
}

//孤注一掷
bool CGameProcess::OnUserAllIn(CT_WORD wChairID)
{
	if (m_wCurrentUser != wChairID)
	{
		LOG(WARNING) << "OnUserAllIn not current user.";
		return false;
	}

	//是否还有游戏状态
	if (!m_cbPlayStatus[wChairID])
	{
		LOG(WARNING) << "OnUserAllIn not in play status.";
		return false;
	}

	//判断发起allIn的人是否钱够单注
	CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(wChairID);
	CT_LONGLONG lTimes = (m_bMingZhu[wChairID]) ? 2 : 1;

	//够钱下注不能allIn
	CT_LONGLONG llCurrentJetton = m_lCurrentJetton * lTimes;
	if (llUserScore > llCurrentJetton)
	{
		LOG(WARNING) << "OnUserAllIn " << m_pGameDeskPtr->GetUserID(wChairID) << " score too much. user score: " << llUserScore << ", jetton: " << llCurrentJetton;
		return false;
	}

	HandleAllIn();

	//原版all，先发起allIn后面的要跟注
	//看是否首先发起allIn
	/*if (!m_bAllInState)
	{
		CT_LONGLONG llLeastScore = m_pGameDeskPtr->GetUserScore(wChairID);
		//找到金币最少的玩家
		for (CT_WORD i = 0; i != GAME_PLAYER; ++i)
		{
			if (m_cbPlayStatus[i] == false || i == wChairID)
				continue;

			CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
			if (llUserScore < llLeastScore)
				llLeastScore = llUserScore;
		}

		m_lAllInJetton = llLeastScore;
		m_wStartAllInUser = wChairID;
		m_bAllInState = true;
		
	}
	HandleAllIn(false);*/

	return true;
}

//处理跟到底
bool CGameProcess::OnUserAutoFollow(CT_WORD wChairID)
{
	if (m_bAutoFollow[wChairID] != false)
		return false;

	m_bAutoFollow[wChairID] = true;

	CMD_S_Auto_Follow autoFollow;
	autoFollow.wAutoFollowUser = wChairID;
	if (m_wCurrentUser != INVALID_CHAIR)
	{
		autoFollow.wCurrentUser = m_wCurrentUser;
		autoFollow.wJettonCount = m_wAddScoreCount[m_wCurrentUser];
		autoFollow.cbRushState = m_bRushState;
	}
	autoFollow.cbState = 1;
	SendTableData(INVALID_CHAIR, SUB_S_AUTO_FOLLOW, &autoFollow, sizeof(autoFollow));

	if (m_wCurrentUser == wChairID)
	{
		HandleAutoFollow(wChairID);
	}

	return true;
}

//取消跟到底
bool CGameProcess::OnUserCancelAutoFollow(CT_WORD wChairID)
{
	if (m_bAutoFollow[wChairID] != true)
		return false;

	m_bAutoFollow[wChairID] = false;

	CMD_S_Auto_Follow autoFollow;
	autoFollow.wAutoFollowUser = wChairID;
	if (m_wCurrentUser != INVALID_CHAIR)
	{
		autoFollow.wCurrentUser = m_wCurrentUser;
		autoFollow.wJettonCount = m_wAddScoreCount[m_wCurrentUser];
		autoFollow.cbRushState = m_bRushState;
	}
	autoFollow.cbState = 0;
	SendTableData(INVALID_CHAIR, SUB_S_AUTO_FOLLOW, &autoFollow, sizeof(autoFollow));

	return true;
}

//火拼
bool CGameProcess::OnUserRush(CT_WORD wChairID)
{
	if (m_pGameDeskPtr->IsExistUser(wChairID) == false)
		return false;

	if (m_wAddScoreCount[wChairID] < CAN_RUSH_JETTON_COUNT)
	{
		LOG(WARNING) << "rush fail, add score count: " << m_wAddScoreCount[wChairID];
		return false;
	}

	//看是否首先发起allIn
	if (!m_bRushState)
	{
		CT_LONGLONG llLeastScore = m_pGameDeskPtr->GetUserScore(wChairID);
		//找到金币最少的玩家
		for (CT_WORD i = 0; i != GAME_PLAYER; ++i)
		{
			if (m_cbPlayStatus[i] == false || i == wChairID)
				continue;
			
			CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
			if (llUserScore < llLeastScore)
				llLeastScore = llUserScore;
		}
		
		m_lRushJetton = llLeastScore;
		//去掉后面两位
		CT_WORD wMod = m_lRushJetton % 100;
		m_lRushJetton -= wMod;
		m_wStartRushUser = wChairID;
		m_bRushState = true;
	}

	HandleRush(false);

	return true;
}

//扑克分析
void CGameProcess::AnalyseStartCard()
{
	//机器人数
	CT_WORD wAiCount = 0;
	CT_WORD wPlayerCount = 0;
	for (CT_WORD i = 0; i<m_wPlayerCount; i++)
	{
		//获取用户
		CT_BOOL bExistUser = m_pGameDeskPtr->IsExistUser(i);
		if (bExistUser)
		{
			if (m_cbPlayStatus[i] == false)continue;
			if (m_pGameDeskPtr->IsAndroidUser(i))
			{
				wAiCount++;
			}
			wPlayerCount++;
		}
	}

	//全部机器
	if (wPlayerCount == wAiCount || wAiCount == 0) return;

	//扑克变量
	CT_BYTE cbUserCardData[GAME_PLAYER][MAX_COUNT];
	memcpy(cbUserCardData, m_cbHandCardData, sizeof(m_cbHandCardData));

	//排列扑克
	for (CT_WORD i = 0; i<m_wPlayerCount; i++)
	{
		m_GameLogic.SortCardList(cbUserCardData[i], MAX_COUNT);
	}

	//变量定义
	CT_WORD wWinUser = INVALID_CHAIR;

	//查找数据
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		//用户过滤
		if (m_cbPlayStatus[i] == false) continue;

		//设置用户
		if (wWinUser == INVALID_CHAIR)
		{
			wWinUser = i;
			continue;
		}

		//对比扑克
		if (m_GameLogic.CompareCard(cbUserCardData[i], cbUserCardData[wWinUser], MAX_COUNT, 1) >= 1)
		{
			wWinUser = i;
		}
	}

	//库存判断
	if (m_lStockScore  < m_lStockLowLimit)
	{
		//随机赢家
		CT_WORD wBeWinAndroid = INVALID_CHAIR;

		do
		{
			wBeWinAndroid = rand() % GAME_PLAYER;
		} while (m_pGameDeskPtr->IsExistUser(wBeWinAndroid) == false || !m_pGameDeskPtr->IsAndroidUser(wBeWinAndroid));

		//交换数据
		CT_BYTE cbTempData[MAX_COUNT];
		memcpy(cbTempData, m_cbHandCardData[wBeWinAndroid], sizeof(CT_BYTE)*MAX_COUNT);
		memcpy(m_cbHandCardData[wBeWinAndroid], m_cbHandCardData[wWinUser], sizeof(CT_BYTE)*MAX_COUNT);
		memcpy(m_cbHandCardData[wWinUser], cbTempData, sizeof(CT_BYTE)*MAX_COUNT);
	}
}

void CGameProcess::AnalyseBlackListCard()
{
	//扑克变量
	CT_BYTE cbUserCardData[GAME_PLAYER][MAX_COUNT];
	memcpy(cbUserCardData, m_cbHandCardData, sizeof(m_cbHandCardData));
	
	//排列扑克
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		m_GameLogic.SortCardList(cbUserCardData[i], MAX_COUNT);
	}
	
	//变量定义
	CT_WORD wWinUser = INVALID_CHAIR;
	std::vector<CT_WORD> vecNormalUser;
	std::vector<CT_WORD> vecBlackUser;
	//查找数据
	for (CT_WORD i = 0; i < m_wPlayerCount; i++)
	{
		//用户过滤
		if (m_cbPlayStatus[i]==false)
			continue;
		
		CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(i);
		bool bInBlackList = false;
		for (auto &it : m_vecBlackList)
		{
			if (it.dwUserID == dwUserID)
			{
				bInBlackList = true;
				vecBlackUser.push_back(i);
			}
		}
		
		if (!bInBlackList)
		{
			vecNormalUser.push_back(i);
		}
		
		if (wWinUser == INVALID_CHAIR)
		{
			wWinUser = i;
			continue;
		}
		
		//对比扑克
		if (m_GameLogic.CompareCard(cbUserCardData[i], cbUserCardData[wWinUser], MAX_COUNT, 1) >= 1)
		{
			wWinUser = i;
		}
	}
	
	if (wWinUser==INVALID_CHAIR)
		return;

	if (vecBlackUser.empty())
		return;
	
	if (vecNormalUser.empty())
		return;
	
	auto it = std::find(vecBlackUser.begin(), vecBlackUser.end(), wWinUser);
	if (it != vecBlackUser.end())
	{
		//随机一个白名单玩家换牌
		//auto nRandUser = rand()%vecNormalUser.size();
        //CT_WORD wBeWinUser = vecNormalUser[nRandUser];
		//if (!m_pGameDeskPtr->IsAndroidUser(wBeWinUser))
		//	return;

		//找到白名单最大牌玩家
		CT_WORD wBeWinUser = INVALID_CHAIR;
        for (CT_WORD i = 0; i < m_wPlayerCount; i++)
        {
            //用户过滤
            if (m_cbPlayStatus[i]==false)
                continue;


            auto itNormalWin = std::find(vecBlackUser.begin(), vecBlackUser.end(), i);
            if (itNormalWin != vecBlackUser.end())
            {
                continue;
            }

            if (wBeWinUser == INVALID_CHAIR)
            {
                wBeWinUser = i;
                continue;
            }

            //对比扑克
            if (m_GameLogic.CompareCard(cbUserCardData[i], cbUserCardData[wBeWinUser], MAX_COUNT, 1) >= 1)
            {
                wBeWinUser = i;
            }
        }

        if (wBeWinUser == INVALID_CHAIR)
        {
            auto nRandUser = rand()%vecNormalUser.size();
            wBeWinUser = vecNormalUser[nRandUser];
        }

		//交换数据
		CT_BYTE cbTempData[MAX_COUNT];
		memcpy(cbTempData, m_cbHandCardData[wBeWinUser], sizeof(CT_BYTE)*MAX_COUNT);
		memcpy(m_cbHandCardData[wBeWinUser], m_cbHandCardData[wWinUser], sizeof(CT_BYTE)*MAX_COUNT);
		memcpy(m_cbHandCardData[wWinUser], cbTempData, sizeof(CT_BYTE)*MAX_COUNT);

		CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbHandCardData[wBeWinUser], MAX_COUNT);
		if (cbCardType < CT_SHUN_ZI)
        {
		    CT_BYTE cbRandData[MAX_COUNT];
		    int nRandom = rand() % 100;
		    CT_BOOL bRandomSucc = false;

		    int nRandomCount = 0;
		    while (!bRandomSucc && ++nRandomCount < 50)
			{
				if (nRandom < 50)
				{
					bRandomSucc = GetShunZiPai(cbRandData);
				}
				else if (nRandom < 85)
				{
					bRandomSucc = GetJinHuaPai(cbRandData);
				}
				else
				{
					bRandomSucc = GetShunJinPai(cbRandData);
				}
			}

            if (bRandomSucc)
                memcpy(m_cbHandCardData[wBeWinUser], cbRandData, sizeof(CT_BYTE)*MAX_COUNT);
        }

		//LOG(WARNING) << "change black list winner card.";
	}
    else
    {
        CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbHandCardData[wWinUser], MAX_COUNT);
        if (cbCardType < CT_SHUN_ZI)
        {
            CT_BYTE cbRandData[MAX_COUNT];
            int nRandom = rand() % 100;
            CT_BOOL bRandomSucc = false;

            int nRandomCount = 0;
            while (!bRandomSucc && ++nRandomCount < 50)
            {
                if (nRandom < 50)
                {
                    bRandomSucc = GetShunZiPai(cbRandData);
                }
                else if (nRandom < 85)
                {
                    bRandomSucc = GetJinHuaPai(cbRandData);
                }
                else
                {
                    bRandomSucc = GetShunJinPai(cbRandData);
                }
            }

            if (bRandomSucc)
                memcpy(m_cbHandCardData[wWinUser], cbRandData, sizeof(CT_BYTE)*MAX_COUNT);
        }
    }
}

//获取一手散牌
CT_BOOL CGameProcess::GetSanPai(CT_BYTE cbCardData[])
{
	std::uniform_int_distribution<> dis(2, 14);
	CT_BYTE cbRandCardLogicValue = dis(m_random_gen);

	CT_BYTE cbLastCardColor = 0;
	CT_BYTE cbLastCardLogicValue = 0;
	CT_BYTE cbCardCount = 0;

	CT_BOOL bSucc = false;

	//找到第一张牌
	for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
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
	for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
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
					bSucc = true;
					break;
				}
			}
		}
	}

	return bSucc;
}

//获取对子牌
CT_BOOL CGameProcess::GetDuiZiPai(CT_BYTE cbCardData[])
{
	for (CT_BYTE cbRandCount = 0; cbRandCount < 10; ++cbRandCount)
	{
		//随机产生一个
		std::uniform_int_distribution<> dis(2, 14);
		CT_BYTE cbRandCardLogicValue = dis(m_random_gen);

		CT_BYTE cbCardCount = 0;
		CT_BOOL bFindSanPai = false;
		CT_BYTE cbDuiZiCardCount = 0;
		CT_BYTE cbChooseIndex[MAX_COUNT] = {0,0,0};

		for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
		{
			if (m_cbRepertoryCard[cbCardIndex] != 0)
			{
				CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
				CT_BYTE cbCardLogicValue = m_GameLogic.GetCardLogicValue(cbCard);

				if (cbRandCardLogicValue == cbCardLogicValue)
				{
					if (cbDuiZiCardCount < 2)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						++cbDuiZiCardCount;
					}
				}
				else
				{
					if (bFindSanPai == false)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						bFindSanPai = true;
					}
				}

				if (cbCardCount >= MAX_COUNT)
				{
					for (CT_BYTE i = 0; i < MAX_COUNT; ++i)
					{
						CT_BYTE cbIndex = cbChooseIndex[i];
						cbCardData[i] =  m_cbRepertoryCard[cbIndex];
						m_cbRepertoryCard[cbIndex] = 0;
					}
					return true;
				}
			}
		}
	}

	return false;
}

//获取顺子牌
CT_BOOL CGameProcess::GetShunZiPai(CT_BYTE cbCardData[])
{
	//std::random_device rd;
	//std::mt19937 gen(rd());
	for (CT_BYTE cbRandCount = 0; cbRandCount < 10; ++cbRandCount)
	{
		std::uniform_int_distribution<> disCardValue(1, 13);
		CT_BYTE cbRandCardValue = disCardValue(m_random_gen);
		CT_BYTE cbCardCount = 0;
		CT_BYTE cbSecondCardColor = 4;
		CT_BYTE cbCardFlag[MAX_COUNT] = { 0, 0, 0 };
		CT_BYTE cbChooseIndex[MAX_COUNT] = {0, 0, 0};

		for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
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
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[0] = 1;

						if (cbCardCount == 1 && cbSecondCardColor == 4)
						{
							cbSecondCardColor = cbCardColor;
						}
					}
					else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[1] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[1] = 1;

						if (cbCardCount == 1 && cbSecondCardColor == 4)
						{
							cbSecondCardColor = cbCardColor;
						}
					}
					else if (cbCardValue == (cbRandCardValue + 2) && cbCardFlag[2] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[2] = 1;

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
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[0] = 1;

						if (cbCardCount == 1 && cbSecondCardColor == 4)
						{
							cbSecondCardColor = cbCardColor;
						}
					}
					else if (cbCardValue == (cbRandCardValue - 1) && cbCardFlag[1] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[1] = 1;

						if (cbCardCount == 1 && cbSecondCardColor == 4)
						{
							cbSecondCardColor = cbCardColor;
						}
					}
					else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[2] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[2] = 1;

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
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[0] = 1;

						if (cbCardCount == 1 && cbSecondCardColor == 4)
						{
							cbSecondCardColor = cbCardColor;
						}
					}
					else if (cbCardValue == (cbRandCardValue - 1) && cbCardFlag[1] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[1] = 1;

						if (cbCardCount == 1 && cbSecondCardColor == 4)
						{
							cbSecondCardColor = cbCardColor;
						}
					}
					else if (cbCardValue == 1 && cbCardFlag[2] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[2] = 1;

						if (cbCardCount == 1 && cbSecondCardColor == 4)
						{
							cbSecondCardColor = cbCardColor;
						}
					}
				}

				if (cbCardCount >= MAX_COUNT)
				{
					for (CT_BYTE i = 0; i < MAX_COUNT; ++i)
					{
						CT_BYTE cbIndex = cbChooseIndex[i];
						cbCardData[i] = m_cbRepertoryCard[cbIndex];
						m_cbRepertoryCard[cbIndex] = 0;
					}
					return true;
				}
			}
		}
	}

	return false;
}

//获取金花
CT_BOOL CGameProcess::GetJinHuaPai(CT_BYTE cbCardData[])
{
	CT_BYTE cbCardCount = 0;
	CT_BOOL bSucc = false;
	CT_BYTE cbFirstCardColor = 0;

	for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);
			CT_BYTE cbCardColor = m_GameLogic.GetCardColor(cbCard);

			if (cbCardCount == 0)
			{
				cbCardData[cbCardCount++] = cbCard;
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
				bSucc = true;
				break;
			}
		}
	}

	return bSucc;
}

//获取顺金
CT_BOOL CGameProcess::GetShunJinPai(CT_BYTE cbCardData[])
{
	//std::random_device rd;
	//std::mt19937 gen(rd());
	for (CT_BYTE cbRandCount = 0; cbRandCount < 20; ++cbRandCount)
	{
		std::uniform_int_distribution<> disCardValue(1, 13);
		CT_BYTE cbRandCardValue = disCardValue(m_random_gen);

		CT_BYTE cbColorArray[4] = { 0x00, 0x10, 0x20, 0x30 };
		std::uniform_int_distribution<> disCardColor(0, 3);
		CT_BYTE cbRandCardColor = cbColorArray[disCardColor(m_random_gen)];

		//CT_BOOL bSucc = false;
		CT_BYTE cbCardCount = 0;
		CT_BYTE cbCardFlag[MAX_COUNT] = { 0, 0, 0 };
		CT_BYTE cbChooseIndex[MAX_COUNT] = {0, 0, 0};
		for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
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
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[0] = 1;
					}
					else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[1] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[1] = 1;
					}
					else if (cbCardValue == (cbRandCardValue + 2) && cbCardFlag[2] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[2] = 1;
					}
				}
				else if (cbRandCardValue == 12)
				{
					if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[0] = 1;
					}
					else if (cbCardValue == 1 && cbCardFlag[1] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[1] = 1;
					}
					else if (cbCardValue == (cbRandCardValue + 1) && cbCardFlag[2] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[2] = 1;
					}
				}
				else if (cbRandCardValue == 13)
				{
					if (cbCardValue == cbRandCardValue && cbCardFlag[0] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[0] = 1;
					}
					else if (cbCardValue == (cbRandCardValue - 1) && cbCardFlag[1] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[1] = 1;
					}
					else if (cbCardValue == 1 && cbCardFlag[2] == 0)
					{
						cbChooseIndex[cbCardCount++] = cbCardIndex;
						cbCardFlag[2] = 1;
					}
				}

				if (cbCardCount >= MAX_COUNT)
				{
					for (int i = 0; i < MAX_COUNT; ++i)
					{
						CT_BYTE cbIndexEx = cbChooseIndex[i];
						cbCardData[i] = m_cbRepertoryCard[cbIndexEx];
						m_cbRepertoryCard[cbIndexEx] = 0;
					}
					return true;
				}
			}
		}
	}

	return false;
}

//获取豹子
CT_BOOL CGameProcess::GetBaoZiPai(CT_BYTE cbCardData[])
{
	CT_BYTE cbCardValueCount[13] = {0};
	memset(cbCardValueCount, 0, sizeof(cbCardValueCount));
	for (CT_BYTE i = m_cbSendCardCount; i < MAX_CARD_TOTAL; ++i)
	{
		if (m_cbRepertoryCard[i] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[i];
			CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);

			cbCardValueCount[cbCardValue-1] += 1;
		}
	}

	CT_BYTE cbChooseCardValue = 0;
	for (CT_BYTE j = 0; j < 13; ++j)
	{
		if (cbCardValueCount[j] >= 3)
		{
			cbChooseCardValue = j + 1; //value值为j+1
		}
	}

	CT_BYTE cbCardCount = 0;
	CT_BYTE cbChooseIndex[MAX_COUNT] = {0,0,0};
	for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);

			if (cbCardValue == cbChooseCardValue)
			{
				cbChooseIndex[cbCardCount++] = cbCardIndex;
			}

			if (cbCardCount >= MAX_COUNT)
			{
				for (int j = 0; j < MAX_COUNT; ++j)
				{
					CT_BYTE cbIndexEx = cbChooseIndex[j];
					cbCardData[j] = m_cbRepertoryCard[cbIndexEx];
					m_cbRepertoryCard[cbIndexEx] = 0;
				}
				return true;
			}
		}
	}
	return false;
}

//获取随机牌
CT_BOOL CGameProcess::GetRandCard(CT_BYTE cbCardData[])
{
	CT_BOOL bSucc = false;
	CT_BYTE cbCardCount = 0;
	for (CT_BYTE cbCardIndex = m_cbSendCardCount; cbCardIndex < MAX_CARD_TOTAL; ++cbCardIndex)
	{
		if (m_cbRepertoryCard[cbCardIndex] != 0)
		{
			CT_BYTE cbCard = m_cbRepertoryCard[cbCardIndex];
			//CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCard);

			cbCardData[cbCardCount++] = cbCard;
			m_cbRepertoryCard[cbCardIndex] = 0;

			if (cbCardCount >= MAX_COUNT)
			{
				bSucc = true;
				break;
			}
		}
	}

	return bSucc;
}

//1号牌库
CT_BOOL CGameProcess::GetCardFromDepot1(CT_BYTE cbCardData[])
{
	std::uniform_int_distribution<> disRandNum(1, 100);
	CT_BYTE cbRandNum = disRandNum(m_random_gen);

	CT_BOOL bSucc = false;
	if (cbRandNum <= m_cbCardDepotRatio[0][0])		//35%单牌
	{
		bSucc = GetSanPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[0][0] + m_cbCardDepotRatio[0][1]))	//25%对子
	{
		bSucc = GetDuiZiPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[0][0] + m_cbCardDepotRatio[0][1] + m_cbCardDepotRatio[0][2]))	//20%是顺子
	{
		bSucc = GetShunZiPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[0][0] + m_cbCardDepotRatio[0][1] + m_cbCardDepotRatio[0][2] + m_cbCardDepotRatio[0][3]))	//12%是同花
	{
		bSucc = GetJinHuaPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[0][0] + m_cbCardDepotRatio[0][1] + m_cbCardDepotRatio[0][2] + m_cbCardDepotRatio[0][3] + m_cbCardDepotRatio[0][4]))	//6%是同花顺
	{
		bSucc = GetShunJinPai(cbCardData);
	}
	else						//2%是豹子
	{
		bSucc = GetBaoZiPai(cbCardData);
	}

	return bSucc;
}

//2号牌库
CT_BOOL CGameProcess::GetCardFromDepot2(CT_BYTE cbCardData[])
{
	std::uniform_int_distribution<> disRandNum(1, 100);
	CT_BYTE cbRandNum = disRandNum(m_random_gen);

	CT_BOOL bSucc = false;

	if (cbRandNum <= m_cbCardDepotRatio[1][0])		//0%单牌
	{
		bSucc = GetSanPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[1][0] + m_cbCardDepotRatio[1][1]))		//40%对子
	{
		bSucc = GetDuiZiPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[1][0] + m_cbCardDepotRatio[1][1] + m_cbCardDepotRatio[1][2]))	//28%顺子
	{
		bSucc = GetShunZiPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[1][0] + m_cbCardDepotRatio[1][1] + m_cbCardDepotRatio[1][2] + m_cbCardDepotRatio[1][3]))	//22%是同花
	{
		bSucc = GetJinHuaPai(cbCardData);
	}
	else if (cbRandNum <= (m_cbCardDepotRatio[1][0] + m_cbCardDepotRatio[1][1] + m_cbCardDepotRatio[1][2] + m_cbCardDepotRatio[1][3] + m_cbCardDepotRatio[1][4]))	//5%是同花顺
	{
		bSucc = GetShunJinPai(cbCardData);
	}
	else						//5%是豹子
	{
		bSucc = GetBaoZiPai(cbCardData);
	}

	return bSucc;
}

CT_VOID CGameProcess::UpdateGoodCardInfo(CT_WORD wChairID, CT_BOOL bWin)
{
	return;
    /*tagGameRoomKind* pGameRoomKind = m_pGameDeskPtr->GetGameKindInfo();
    if (pGameRoomKind->wRoomKindID != PRIMARY_ROOM)
    {
        return;
    }

	CT_DWORD dwPlayCount = m_pGameDeskPtr->GetUserPlayThisGameCount(wChairID);
	if (dwPlayCount >= 3)
	{
		CT_BYTE cbGoodCardParam = m_pGameDeskPtr->GetThisGameGoodCardParam(wChairID);
		if (bWin)
		{
			cbGoodCardParam -= 20;
		}
		else
		{
			cbGoodCardParam += 10;
		}

		if (cbGoodCardParam > 100)
		{
			cbGoodCardParam = 100;
		}
		if (cbGoodCardParam < 20)
		{
			cbGoodCardParam = 20;
		}

		m_pGameDeskPtr->SetThisGameGoodCardParam(wChairID, cbGoodCardParam);
	}*/
}

CT_VOID CGameProcess::UpdateUserScoreInfo(CT_WORD wChairID, CT_LONGLONG llWinScore, CT_DWORD dwRevenue)
{
	GS_UserScoreData* pUserScoreData = m_pGameDeskPtr->GetUserScoreData(wChairID);
	if (pUserScoreData)
	{
		/*if (llWinScore >=0)
        {
		    pUserScoreData->dwWinCount += 1;
		    pUserScoreData->llWinScore += llWinScore;
		    pUserScoreData->dwRevenue += dwRevenue;
        }
        else
        {
            pUserScoreData->dwLostCount += 1;
            pUserScoreData->llLostScore += llWinScore;
        }*/

        pUserScoreData->dwPlayCount += 1;
	}
}