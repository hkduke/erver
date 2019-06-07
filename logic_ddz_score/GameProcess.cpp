
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

//打印日志
#define		PRINT_LOG_INFO						1
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//#define IDI_CHECK_TABLE			100					//检查桌子
//#define IDI_CHECK_TRUSTEE			101					//断线托管

//#define IDI_AUTO_READY				200					//自动准备
#define IDI_CALL_BANKER				300					//叫地主
#define IDI_ADD_DOUBLE				400					//加陪
#define IDI_OUT_CARD				500					//出牌
#define IDI_TICK_USER				600					//剔除用户
#define IDI_CHECK_END				700					//检测游戏结束

//#define	TIME_AUTO_READY				16					//自动准备时间(15)
#define TIME_DEAL_CARD				2					//发牌时间(2)
#define TIME_CALL_BANKER			15					//叫地主时间(15)
#define TIME_ADD_DOUBLE				10					//加陪时间(5)
#define	TIME_OUT_CARD				15					//出牌时间(15)
#define TIME_FIRST_OUT_CARD			25					//第一次出牌时间
#define TIME_TICK_USER				1					//剔除用户时间
#define TIME_CHECK_END				600					//检测桌子结束

//网络补偿时间
#define TIME_NETWORK_COMPENSATE     2					//网络补偿

//分值倍数
#define SCORE_TIME_BOMB				2					//炸弹倍数
#define SCORE_TIME_CHUANTIAN		2					//春天倍数
#define SCORE_TIME_FANCHUAN			2					//反春倍数

#define TIME_OUT_TRUSTEE_COUNT		1					//超时进入托管次数

////////////////////////////////////////////////////////////////////////
std::map<CT_DWORD, tagBlackChannel>CGameProcess::m_mapChannelControl;
CGameProcess::CGameProcess(void) 
	: m_cbNoBankCount(1), m_dwlCellScore(1), m_pGameDeskPtr(NULL), m_pGameRoomKindInfo(NULL)
{
	m_dwFirstUser = INVALID_CHAIR;
	//清理游戏数据
	ClearGameData();
	//初始化数据
	InitGameData();

	//CT_BYTE cbRandCard[FULL_COUNT];
	//std::vector<CT_BYTE> vecCard;
	//for (int i = 0; i < 1000000; ++i)
	//{
	//	m_GameLogic.RandCardList(cbRandCard, CountArray(cbRandCard), 0);


	//	for (int j = 0; j < FULL_COUNT; j++)
	//	{
	//		auto it = std::find(vecCard.begin(), vecCard.end(), cbRandCard[j]);
	//		if (it != vecCard.end())
	//		{
	//			LOG(ERROR) << "fuck, i want to fuck you.";
	//		}
	//		vecCard.push_back(cbRandCard[j]);
	//	}

	//	vecCard.clear();
	//}

	//CT_BYTE cbCardData[20] = { 0x3a, 0x3a, 0x2a, 0x2a,0x1a };

	//CT_BYTE cbType = m_GameLogic.GetCardType(cbCardData, 5);
	//if (cbType == CT_ERROR)
	//{
	//	LOG(WARNING) << "card type error";
	//}

}

CGameProcess::~CGameProcess(void)
{
}

//游戏开始
void CGameProcess::OnEventGameStart()
{
	//assert(0 != m_pGameDeskPtr);
	//初始化数据
	InitGameData();
	//解释私人场数据
	//ParsePrivateData(m_PrivateTableInfo.szOtherParam);
	//更新游戏配置
	//UpdateGameConfig();
	//清除所有定时器
	ClearAllTimer();
	//设置游戏状态
	m_cbGameStatus = GAME_SCENE_CALL;
	m_pGameDeskPtr->SetGameTimer(IDI_CHECK_END, TIME_CHECK_END*1000);

	//混乱扑克m_cbBombCount
	CT_BYTE cbRandCard[FULL_COUNT];
	if (m_pGameRoomKindInfo->wRoomKindID == PRIMARY_ROOM)//初级房间%70几率出现炸弹
	{
		/*CT_DWORD dwFirstUserID = m_pGameDeskPtr->GetUserID(0);
		int nRand = rand() % 10;
		CT_BYTE cbBomCount = 0;
		if (nRand < 7)
		{
			cbBomCount = 1;
		}*/

		m_GameLogic.RandCardList(cbRandCard, CountArray(cbRandCard), 0);
	
	}
	else
	{
		m_GameLogic.RandCardList(cbRandCard, CountArray(cbRandCard), 0);
	}

	//配好牌功能
    if (PRIMARY_ROOM == m_pGameRoomKindInfo->wRoomKindID)
    {
        MatchGoodCard(cbRandCard, CountArray(cbRandCard));
    }

    /*
	cbRandCard[0] = 0x22;
    cbRandCard[1] = 0x12;
    cbRandCard[2] = 0x31;
    cbRandCard[3] = 0x3d;
    cbRandCard[4] = 0x2d;
    cbRandCard[5] = 0x1d;
    cbRandCard[6] = 0x2c;
    cbRandCard[7] = 0x3a;
    cbRandCard[8] = 0x2a;
    cbRandCard[9] = 0x1a;
    cbRandCard[10] = 0x0a;
    cbRandCard[11] = 0x39;
    cbRandCard[12] = 0x28;
    cbRandCard[13] = 0x37;
    cbRandCard[14] = 0x17;
    cbRandCard[15] = 0x06;
    cbRandCard[16] = 0x13;

    cbRandCard[17] = 0x21;
    cbRandCard[18] = 0x01;
    cbRandCard[19] = 0x3c;
    cbRandCard[20] = 0x1c;
    cbRandCard[21] = 0x2b;
    cbRandCard[22] = 0x0b;
    cbRandCard[23] = 0x1b;
    cbRandCard[24] = 0x09;
    cbRandCard[25] = 0x08;
    cbRandCard[26] = 0x07;
    cbRandCard[27] = 0x36;
    cbRandCard[28] = 0x26;
    cbRandCard[29] = 0x35;
    cbRandCard[30] = 0x25;
    cbRandCard[31] = 0x24;
    cbRandCard[32] = 0x14;
    cbRandCard[33] = 0x04;

    cbRandCard[34] = 0x4e;
    cbRandCard[35] = 0x4f;
    cbRandCard[36] = 0x32;
    cbRandCard[37] = 0x02;
    cbRandCard[38] = 0x0d;
    cbRandCard[39] = 0x0c;
    cbRandCard[40] = 0x3b;
    cbRandCard[41] = 0x19;
    cbRandCard[42] = 0x38;
    cbRandCard[43] = 0x27;
    cbRandCard[44] = 0x16;
    cbRandCard[45] = 0x15;
    cbRandCard[46] = 0x05;
    cbRandCard[47] = 0x34;
    cbRandCard[48] = 0x33;
    cbRandCard[49] = 0x23;
    cbRandCard[50] = 0x03;

    cbRandCard[51] = 0x29;
    cbRandCard[52] = 0x11;
    cbRandCard[53] = 0x18;
	*/
	//if (m_dwFirstUser == INVALID_CHAIR)
	{
		//设置用户
		//m_dwFirstUser = m_pGameDeskPtr->GetPRoomOwnerChairID();
		//if (m_dwFirstUser == INVALID_CHAIR)
		//{
			m_dwFirstUser = rand() % GAME_PLAYER;
		//}	
	}
	m_dwCurrentUser = m_dwFirstUser;

	//设置底牌
	memcpy(m_cbBankerCard, &cbRandCard[DISPATCH_COUNT], sizeof(m_cbBankerCard));

	//用户扑克
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
		//CT_DWORD wUserIndex = (m_dwFirstUser + i) % GAME_PLAYER;
		m_cbHandCardCount[i] = NORMAL_COUNT;
		memcpy(&m_cbHandCardData[i], &cbRandCard[i*m_cbHandCardCount[i]], sizeof(CT_BYTE)*m_cbHandCardCount[i]);
	}

	//构造变量
	CMD_S_GameStart GameStart;
	GameStart.dwCurrentUser = m_dwCurrentUser;
	GameStart.dwStartTime = m_dwStartTime;
	GameStart.cbTimeLeft = TIME_CALL_BANKER;
	//GameStart.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();
	//=================test=begin=================

	
	//0	
	/*m_cbHandCardData[0][0]	=  0x02;
	m_cbHandCardData[0][1]	=  0x12;
	m_cbHandCardData[0][2]	=  0x22;
	m_cbHandCardData[0][3]	=  0x32;
	m_cbHandCardData[0][4]	=  0x01;
	m_cbHandCardData[0][5]	=  0x3b;
	m_cbHandCardData[0][6]	=  0x1a;
	m_cbHandCardData[0][7]	=  0x0a;
	m_cbHandCardData[0][8]	=  0x29;
	m_cbHandCardData[0][9]	=  0x38;
	m_cbHandCardData[0][10]	=  0x08;
	m_cbHandCardData[0][11] =  0x36;
	m_cbHandCardData[0][12] =  0x26;
	m_cbHandCardData[0][13] =  0x06;
	m_cbHandCardData[0][14] =  0x25;
	m_cbHandCardData[0][15] =  0x24;
	m_cbHandCardData[0][16] =  0x33;*/
			

	
	/*1*/
	/*m_cbHandCardData[1][0] = 0x31;
	m_cbHandCardData[1][1] = 0x3d;
	m_cbHandCardData[1][2] = 0x2d;
	m_cbHandCardData[1][3] = 0x0d;
	m_cbHandCardData[1][4] = 0x1b;
	m_cbHandCardData[1][5] = 0x0b;
	m_cbHandCardData[1][6] = 0x39;
	m_cbHandCardData[1][7] = 0x09;
	m_cbHandCardData[1][8] = 0x18;
	m_cbHandCardData[1][9] = 0x17;
	m_cbHandCardData[1][10] = 0x07;
	m_cbHandCardData[1][11] = 0x16;
	m_cbHandCardData[1][12] = 0x15;
	m_cbHandCardData[1][13] = 0x04;
	m_cbHandCardData[1][14] = 0x23;
	m_cbHandCardData[1][15] = 0x13;
	m_cbHandCardData[1][16] = 0x03;*/

	/*2*/
	/*m_cbHandCardData[2][0] = 0x34;
	m_cbHandCardData[2][1] = 0x14;
	m_cbHandCardData[2][2] = 0x35;
	m_cbHandCardData[2][3] = 0x05;
	m_cbHandCardData[2][4] = 0x37;
	m_cbHandCardData[2][5] = 0x27;
	m_cbHandCardData[2][6] = 0x28;
	m_cbHandCardData[2][7] = 0x19;
	m_cbHandCardData[2][8] = 0x3a;
	m_cbHandCardData[2][9] = 0x2a;
	m_cbHandCardData[2][10] = 0x3c;
	m_cbHandCardData[2][11] = 0x2c;
	m_cbHandCardData[2][12] = 0x1c;
	m_cbHandCardData[2][13] = 0x0c;
	m_cbHandCardData[2][14] = 0x1d;
	m_cbHandCardData[2][15] = 0x4e;
	m_cbHandCardData[2][16] = 0x4f;

	//配置底牌
	m_cbBankerCard[0] = 0x21;
	m_cbBankerCard[1] = 0x11;
	m_cbBankerCard[2] = 0x3b;*/
	//=============test=end=====================
	
	//游戏记录的空闲场景构造
    CMD_S_StatusFree StatusFree;
    memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
    StatusFree.dCellScore = m_dwlCellScore*TO_DOUBLE;
    m_pGameDeskPtr->WriteGameSceneToRecord(&StatusFree, sizeof(StatusFree), SC_GAMESCENE_FREE);
    
	bool bHasAndroid = false;
	//发送数据
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
		//构造扑克
	//	assert(CountArray(GameStart.cbCardData) >= m_cbHandCardCount[i]);
        GameStart.dwStartUser = i; //m_dwFirstUser;
        memcpy(GameStart.cbCardData, m_cbHandCardData[i], sizeof(GameStart.cbCardData));
        //发送数据
        SendTableData(i, SUB_S_GAME_START, &GameStart, sizeof(GameStart), true);
        
		if (m_pGameDeskPtr->IsAndroidUser(i))
		{
            if (bHasAndroid == false)
            {
                bHasAndroid = true;
            }
		}
	}

	//排列扑克
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		m_GameLogic.SortCardList(m_cbHandCardData[i], m_cbHandCardCount[i], ST_ORDER);

		//printf("game splayer %d hand card: ", i);
		//for (int j = 0; j < NORMAL_COUNT; ++j)
		//{
		//	printf("%x, ", m_cbHandCardData[i][j]);
		//}
		//printf("\n");
	}



	//给机器人发送开始数据
	if (bHasAndroid)
	{
		CMD_S_GameStartAi GameStartAi;
		GameStartAi.dwStartUser = m_dwFirstUser;
		GameStartAi.dwCurrentUser = m_dwCurrentUser;
		GameStartAi.dwStartTime = m_dwStartTime;
		memcpy(GameStartAi.cbBankerCard, m_cbBankerCard, sizeof(GameStartAi.cbBankerCard));

		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			for (CT_BYTE j = 0; j != NORMAL_COUNT; ++j)
			{
				GameStartAi.cbCardData[i][j] = m_cbHandCardData[i][j];
			}
			//memcpy(GameStartAi.cbCardData[i], m_cbHandCardData[i], sizeof(GameStartAi.cbCardData));
		}

		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			if (m_pGameDeskPtr->IsAndroidUser(i))
			{
				//发送数据
				SendTableData(i, SUB_S_GAME_START, &GameStartAi, sizeof(GameStartAi), false);
			}
		}
	}

	//托管
	IsTrustee();
	return;
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//清除所有定时器
	//ClearAllTimer();
	//设置游戏状态-空闲状态
	m_cbGameStatus = GAME_SCENE_FREE;

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

	//以下代码放到全部踢掉机器人后执行
	//当前时间
	//std::string strTime = Utility::GetTimeNowString();
	//游戏结束
	//m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());

	//大结算
	/*if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_End)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnEventGameEnd: SendTotalClearing";

		//发送总结算
		SendTotalClearing(strTime);

		//清理桌子上的用户
		m_pGameDeskPtr->ClearTableUser();
	}*/
	//托管
	//IsTrustee();
	m_cbGameStatus = GAME_SCENE_FREE;

	//保留一秒时间掉机器人(直接t掉机器人引起androidmgr的机器人的m_AndroidUserItemSitting失效)
	if (m_PrivateTableInfo.dwRoomNum == 0)
    {
        m_pGameDeskPtr->SetGameTimer(IDI_TICK_USER, 1000);

        //结束直接T真人
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            //斗地主游戏剔除用户时不需要广播状态给客户端
            if (!m_pGameDeskPtr->IsAndroidUser(i))
                m_pGameDeskPtr->ClearTableUser(i, false);
        }
    }
    else
    {
        m_pGameDeskPtr->SetGameTimer(IDI_TICK_USER, 16000);
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            m_pGameDeskPtr->SetUserStatus(i, sSit);
        }

        //当前时间
        std::string strTime = Utility::GetTimeNowString();
        //游戏结束
        m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());
    }
	
	return;
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
			CMD_S_StatusFree StatusFree;
			memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
			StatusFree.dCellScore = m_dwlCellScore*0.01f;
			StatusFree.dwCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

			//发送数据
			SendTableData(dwChairID, SC_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree),false);
			break;
		}
		case GAME_SCENE_CALL:	//叫地主状态
		{
			//构造数据
			CMD_S_StatusCall StatusCall;
			memset(&StatusCall,0, sizeof(StatusCall));

			//单元积分
			StatusCall.dCellScore = m_dwlCellScore*0.01f;

			//剩余时间 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusCall.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//游戏信息
			StatusCall.wCurrentUser = m_dwCurrentUser;
			memcpy(StatusCall.cbCallBankerInfo, m_cbCallBankerInfo, sizeof(m_cbCallBankerInfo));

			//手上扑克
			memcpy(StatusCall.cbHandCardData, m_cbHandCardData[dwChairID], sizeof(StatusCall.cbHandCardData));
	
			//托管状态
			memcpy(StatusCall.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//发送场景
			SendTableData(dwChairID, SC_GAMESCENE_CALL, &StatusCall, sizeof(StatusCall), false);
			break;
		}
		case GAME_SCENE_DOUBLE:
		{
			CMD_S_StatusDouble StatusDouble;
			memset(&StatusDouble, 0, sizeof(StatusDouble));

			//单元积分
			StatusDouble.dCellScore = m_dwlCellScore*0.01f;
			StatusDouble.cbCallScore = (CT_BYTE)m_dwStartTime;

			//剩余时间 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusDouble.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//游戏变量			 
			StatusDouble.wBankerUser = m_dwBankerUser;
			StatusDouble.wCurrTime = m_wEachTotalTime[dwChairID];
			memcpy(StatusDouble.cbDoubleInfo, m_cbAddDoubleInfo, sizeof(StatusDouble.cbDoubleInfo));

			//扑克信息
			memcpy(StatusDouble.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));
			//手上扑克
			memcpy(StatusDouble.cbHandCardCount, m_cbHandCardCount, sizeof(StatusDouble.cbHandCardCount));
			memcpy(StatusDouble.cbHandCardData, m_cbHandCardData[dwChairID], m_cbHandCardCount[dwChairID] * sizeof(CT_BYTE));
			//托管状态
			memcpy(StatusDouble.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//发送场景
			SendTableData(dwChairID, SC_GAMESCENE_DOUBLE, &StatusDouble, sizeof(StatusDouble), false);

			break;
		}
		case GAME_SCENE_PLAY:	//游戏状态
		{
			//构造数据
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay, 0, sizeof(StatusPlay));

			//单元积分
			StatusPlay.dCellScore = m_dwlCellScore*0.01f;

			//剩余时间 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusPlay.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//出牌信息
			StatusPlay.wTurnWiner = m_dwTurnWiner;
			StatusPlay.cbTurnCardCount = m_cbTurnCardCount;
			memcpy(StatusPlay.cbTurnCardData, m_cbTurnCardData, m_cbTurnCardCount * sizeof(CT_BYTE));

			//游戏变量			 
			StatusPlay.dwBankerUser = m_dwBankerUser;
			StatusPlay.dwCurrentUser = m_dwCurrentUser;

			StatusPlay.cbCallScore = (CT_BYTE)m_dwStartTime;
			if (dwChairID == m_dwBankerUser)
			{
				StatusPlay.dwBombTime = m_dwBombTime*2;
			}
			else
			{
				StatusPlay.dwBombTime = m_dwBombTime;
			}
		
			memcpy(StatusPlay.cbAddDoubleInfo, m_cbAddDoubleInfo, sizeof(StatusPlay.cbAddDoubleInfo));

			//扑克信息
			memcpy(StatusPlay.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));
			memcpy(StatusPlay.cbHandCardCount, m_cbHandCardCount, sizeof(m_cbHandCardCount));
			memcpy(StatusPlay.cbOutCount, m_cbOutCardCount, sizeof(StatusPlay.cbOutCount));
			memcpy(StatusPlay.cbOutCardListCount, m_cbOutCardListCount, sizeof(StatusPlay.cbOutCardListCount));
			memcpy(StatusPlay.cbOutCardList, m_cbOutCardList, sizeof(StatusPlay.cbOutCardList));

			//手上扑克
			memcpy(StatusPlay.cbHandCardData, m_cbHandCardData[dwChairID], m_cbHandCardCount[dwChairID] * sizeof(CT_BYTE));
			//托管状态
			memcpy(StatusPlay.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//发送场景
			SendTableData(dwChairID, SC_GAMESCENE_PLAY, &StatusPlay, sizeof(StatusPlay),false);
			break;
		}
		case GAME_SCENE_END://目前已经不会出现这个场景
		{
			//场景数据
			CMD_S_StatusEND StatusEnd;
			memset(&StatusEnd, 0, sizeof(CMD_S_StatusEND));
			//基础积分
			StatusEnd.dCellScore = m_dwlCellScore*0.01f;
			StatusEnd.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
			//剩余时间
			CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
			StatusEnd.cbTimeLeave = (CT_BYTE)(m_cbOpTotalTime - std::min<CT_BYTE>(dwPassTime, m_cbOpTotalTime));
			//扑克信息
			memcpy(StatusEnd.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));
			memcpy(StatusEnd.cbHandCardCount, m_cbHandCardCount, sizeof(m_cbHandCardCount));
			//手上扑克
			memcpy(StatusEnd.cbHandCardData, m_cbHandCardData[dwChairID], m_cbHandCardCount[dwChairID] * sizeof(CT_BYTE));

			//发送结算状态数据
			SendTableData(dwChairID, SC_GAMESCENE_END, &StatusEnd, sizeof(CMD_S_StatusEND), false);
			
			//游戏结算消息x
			if (StatusEnd.cbTimeLeave>=5)
			{
				SendTableData(dwChairID, SUB_S_GAME_CONCLUDE, &StatusEnd, sizeof(CMD_S_StatusEND), false);
			}

			break;
		}
		default:
			break;
	}
	return ;
}

//定时器事件
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	if (dwTimerID == IDI_CHECK_END)
	{
		OnEventGameEnd(m_dwBankerUser, GER_NORMAL);
		return;
	}
	 
	//LOG(WARNING) << "run timer, timer id: " << dwTimerID;

	m_pGameDeskPtr->KillGameTimer(dwTimerID);
	if (m_cbGameStatus == GAME_SCENE_FREE)
	{
		//if (!m_pGameDeskPtr->IsExistUser(dwParam))
		//{
		//	return;
		//}

		switch (dwTimerID - dwParam)
		{
		//case IDI_AUTO_READY:
		//{
		//	m_pGameDeskPtr->SetUserReady(dwParam);
		//}
		//break;
		case IDI_TICK_USER:
		{
            if (m_PrivateTableInfo.dwRoomNum == 0)
            {
                for (CT_DWORD i= 0; i < GAME_PLAYER; ++i)
                {
                    if (!m_pGameDeskPtr->IsExistUser(i))
                    {
                        continue;
                    }

                    //斗地主游戏剔除用户时不需要广播状态给客户端
                    m_pGameDeskPtr->ClearTableUser(i, false);
                }
                //当前时间
                std::string strTime = Utility::GetTimeNowString();
                //游戏结束
                m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());
            }
            else
            {
                for (CT_DWORD i= 0; i < GAME_PLAYER; ++i)
                {
                    if (!m_pGameDeskPtr->IsExistUser(i))
                    {
                        continue;
                    }

                    //斗地主游戏剔除用户时不需要广播状态给客户端
                    if (m_pGameDeskPtr->GetUserStatus(i) != sReady)
                        m_pGameDeskPtr->ClearTableUser(i, true);
                }
            }
		}
		break;
		default:
			break;
		}
		return;
	}
	else if (m_cbGameStatus == GAME_SCENE_DOUBLE && dwTimerID == IDI_ADD_DOUBLE)
	{
		for (CT_DWORD i = 0; i != GAME_PLAYER; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i) || m_dwBankerUser == i)
			{
				continue;
			}

			if (m_cbAddDoubleInfo[i] == CB_NOT_ADD_DOUBLE)
			{
				OnAutoDouble(i);
			}
		}
		return;
	}

	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return;
	}
    CT_DWORD dwTime = time(NULL);
	//叫地主状态不主动托管，游戏状态超时1次托管
	if (m_cbGameStatus == GAME_SCENE_PLAY &&
		m_cbTrustee[m_dwCurrentUser] != 1 && 
		(m_cbTimeOutCount[m_dwCurrentUser] >= TIME_OUT_TRUSTEE_COUNT || m_cbTurnCardCount == 0)
        && ((int)(m_cbOpTotalTime-1) <= (int)(dwTime-m_dwOpStartTime)))
	{
		SetTrustee(m_dwCurrentUser, 1);
	}

	++m_cbTimeOutCount[m_dwCurrentUser];

	CT_BOOL bSystemSucceed = true;
	switch (dwTimerID)
	{
	case IDI_CALL_BANKER:
	{
		bSystemSucceed = OnAutoCallLand();
	}
	break;
	case IDI_OUT_CARD:
	{
        //LOG(ERROR) << "lTime " << iTime << " m_dwOpStartTime " << (int)m_dwOpStartTime << " m_cbOpTotalTime " << (int)m_cbOpTotalTime;
	    if(((m_cbOpTotalTime-1) <= (int)(dwTime - m_dwOpStartTime)) || m_cbTrustee[m_dwCurrentUser])
        {
            //return;
            if (m_cbTrustee[m_dwCurrentUser])
                bSystemSucceed = OnAutoOutCard();
            else
                bSystemSucceed = OnUserPassCard(m_dwCurrentUser);
        } else{
            bSystemSucceed = false;
            //LOG(ERROR) << "IDI_OUT_CARD33 " << (int)m_cbOpTotalTime <<" " << (int)(iTime-m_dwOpStartTime);
	    }
	}
	break;
	default:
		break;
	}

	if (bSystemSucceed == false)
	{
        m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (((m_cbOpTotalTime-(dwTime-m_dwOpStartTime)) >  0) ? (m_cbOpTotalTime-(dwTime-m_dwOpStartTime)) : 1)*1000);
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "OnTimerMessage err, time id: " << dwTimerID;
	}
}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	switch (dwSubCmdID)
	{
		case SUB_C_CALL_BANKER:	//用户叫地主
		{
			//效验数据
			//assert(dwDataSize == sizeof(CMD_C_CallBanker));
			if (dwDataSize != sizeof(CMD_C_CallBanker)) return false;

			//状态效验
			//assert(m_cbGameStatus != GAME_SCENE_CALL);
			if (m_cbGameStatus != GAME_SCENE_CALL) return true;

			//变量定义
			CMD_C_CallBanker * pCallBanker = (CMD_C_CallBanker *)pDataBuffer;

			//消息处理
			return OnUserCallBanker(wChairID, pCallBanker->cbCallInfo);
		}
		case SUB_C_ADD_DOUBLE:
		{
			//效验数据
			if (dwDataSize != sizeof(CMD_C_Double)) return false;

			//状态效验
			//ASSERT(m_cbGameStatus==GAME_SCENE_DOUBLE);
			if (m_cbGameStatus != GAME_SCENE_DOUBLE) return true;

			//变量定义
			CMD_C_Double * pDouble = (CMD_C_Double *)pDataBuffer;

			//消息处理
			return OnUserDouble(wChairID, pDouble->cbDoubleInfo);
		}
		case SUB_C_OUT_CARD:	//用户出牌
		{
			//变量定义
			CMD_C_OutCard * pOutCard = (CMD_C_OutCard *)pDataBuffer;
			CT_DWORD dwHeadSize = sizeof(CMD_C_OutCard) - sizeof(pOutCard->cbCardData);

			//效验数据
			/*assert((dwDataSize >= dwHeadSize) && (dwDataSize == (dwHeadSize + pOutCard->cbCardCount * sizeof(CT_BYTE))));*/
			if ((dwDataSize < dwHeadSize)/* || (dwDataSize != (dwHeadSize + pOutCard->cbCardCount * sizeof(CT_BYTE)))*/) return false;

			//状态效验
			//assert(m_cbGameStatus!= GAME_SCENE_PLAY);
			if (m_cbGameStatus != GAME_SCENE_PLAY) return true;


			//消息处理
			return OnUserOutCard(wChairID, pOutCard->cbCardData, pOutCard->cbCardCount);
		}
		case SUB_C_PASS_CARD:	//用户放弃
		{
			//状态效验
			//assert(m_cbGameStatus == GAME_SCENE_PLAY);
			if (m_cbGameStatus != GAME_SCENE_PLAY) return true;

			//消息处理
			return OnUserPassCard(wChairID);
		}
		case SUB_C_TRUSTEE: //托管
		{
			if (dwDataSize != sizeof(CMD_C_Trustee)) return false;

			//变量定义
			CMD_C_Trustee * pTrustee = (CMD_C_Trustee *)pDataBuffer;

			if (pTrustee->wChairID == INVALID_CHAIR) return true;
			m_cbTrustee[pTrustee->wChairID] = 1;

			//构造消息
			CMD_S_StatusTrustee StatusTrustee;
			memcpy(StatusTrustee.cbTrustee, m_cbTrustee,sizeof(m_cbTrustee));
			//发送数据
			for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
			    if (i == m_dwBankerUser)
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
                }
                else
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
                }
			}

			if (m_dwCurrentUser == wChairID)
			{
				IsTrustee(false);
			}
			return true;
		}
		case SUB_C_CANCEL_TRUSTEE: //取消托管
		{
			if (dwDataSize != sizeof(CMD_C_CancelTrustee)) return false;

			//变量定义
			CMD_C_CancelTrustee * pCancelTrustee = (CMD_C_CancelTrustee *)pDataBuffer;

			if (pCancelTrustee->wChairID == INVALID_CHAIR) return true;

			m_cbTrustee[pCancelTrustee->wChairID] = 0;
			//构造消息
			CMD_S_StatusTrustee StatusTrustee;
			memcpy(StatusTrustee.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//发送数据
			for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
                if (i == m_dwBankerUser)
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
                }
                else
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
                }
			}
			if(pCancelTrustee->wChairID==m_dwCurrentUser)
			{
				long lTime = time(NULL);
				if((m_cbOpTotalTime - (lTime-m_dwOpStartTime)) > 1)
				{
					m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
					int iTime = m_cbOpTotalTime - (lTime-m_dwOpStartTime) + TIME_NETWORK_COMPENSATE;
					//LOG(ERROR) << "i " << iTime << " " << lTime << " " << m_dwOpStartTime;
					m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (iTime) * 1000);
				}
			}
			//IsTrustee();
			return true;
		}
		case SUB_C_CHEAT_LOOK_CARD:
		{
			if (dwDataSize != sizeof(CMD_C_CheatLookCard)) return false;

			CT_BOOL bAndroid = m_pGameDeskPtr->IsAndroidUser(wChairID);
			if (!bAndroid)
			{
				return false;
			}

			//变量定义
			CMD_C_CheatLookCard * pCheatLookCard = (CMD_C_CheatLookCard *)pDataBuffer;
			if (pCheatLookCard->wBeCheatChairID >= GAME_PLAYER)
			{
				return true;
			}

			CMD_S_CheatLookCard cheatCardData;
			cheatCardData.wCardUser = pCheatLookCard->wBeCheatChairID;
			cheatCardData.cbCardCount = m_cbHandCardCount[pCheatLookCard->wBeCheatChairID];
			memcpy(cheatCardData.cbCardData, m_cbHandCardData[pCheatLookCard->wBeCheatChairID], sizeof(cheatCardData.cbCardData));
			SendTableData(wChairID, SUB_S_CHEAT_LOOK_CARD, &cheatCardData, sizeof(cheatCardData), false);

			return true;
		}
	}
	return CT_TRUE;
}

//用户进入
void CGameProcess::OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID == INVALID_CHAIR)
	{
		return;
	}

	//m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY + dwChairID);
	//m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER + dwChairID);

	//if (m_pGameDeskPtr->GetUserStatus(dwChairID) < sReady)
	//{
	//	m_pGameDeskPtr->SetGameTimer(IDI_AUTO_READY + dwChairID, TIME_AUTO_READY * 1000, dwChairID);
	//}
}

void CGameProcess::OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID == INVALID_CHAIR)
	{
		return;
	}

	//m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY + dwChairID);
	//m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER + dwChairID);
}

//用户离开
void CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag)
{

}

//用户断线
void CGameProcess::OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{

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
	//LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Score=" << m_iTotalLWScore[dwChairID];
	//return m_iTotalLWScore[dwChairID];
	return 0;
}

//设置游戏黑名单(斗地主游戏设置为渠道ID和渠道对应的发差牌的概率)
CT_VOID CGameProcess::SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList)
{
	for (auto& itVec : vecBlackList)
	{
		auto itMap = m_mapChannelControl.find(itVec.dwUserID);
		if (itMap != m_mapChannelControl.end())
		{
			itMap->second.wBadCardRation = itVec.wUserLostRatio;
		}
		else
		{
            tagBlackChannel channelControl;
			channelControl.dwChannelID = itVec.dwUserID;
			channelControl.wBadCardRation = itVec.wUserLostRatio;
			m_mapChannelControl.insert(std::make_pair(channelControl.dwChannelID, channelControl));
		}
	}

	for (auto& it : m_mapChannelControl)
    {
	    LOG(WARNING) << "channel id: " << it.first << ", ration: " << it.second.wBadCardRation;
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
    //系统时间
	m_dwSysTime = 0;

	//游戏状态
	m_cbGameStatus = GAME_SCENE_FREE;							
	//首叫用户
/*	m_dwFirstUser = INVALID_CHAIR;*/
	//庄家用户
	m_dwBankerUser = INVALID_CHAIR;
	//当前玩家
	m_dwCurrentUser = INVALID_CHAIR;

	//炸弹个数
	m_cbBombCount = 0;
	m_cbMissile = 0;
	m_wMaxBombCount = 10000;
	m_cbOpTotalTime = 0;
	m_dwOpStartTime = 0;
	memset(m_cbEachBombCount, 0, sizeof(m_cbEachBombCount));
	memset(m_wEachTotalTime, 0, sizeof(m_wEachTotalTime));
	//初始倍数
	m_dwStartTime = 1;
	//基础积分
	m_dwlCellScore = 1;
	
	//炸弹倍数
	m_dwBombTime = 1;
	//春天倍数
	m_dwChunTianbTime = 1;
	//出牌数据
	memset(m_cbOutCardCount, 0, sizeof(m_cbOutCardCount));

	//叫地主信息
	memset(m_cbCallBankerInfo, 0, sizeof(m_cbCallBankerInfo));
	//加倍信息
	memset(m_cbAddDoubleInfo, 0, sizeof(m_cbAddDoubleInfo));

	//胜利玩家
	m_dwTurnWiner = INVALID_CHAIR;
	//出牌数目
	m_cbTurnCardCount = 0;
	//出牌数据	
	memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	memset(m_cbOutCardListCount, 0, sizeof(m_cbOutCardListCount));
	memset(m_cbOutCardList, 0, sizeof(m_cbOutCardList));

	//初始化剩余牌数据
	/*for (int i = 0; i < MAX_CARD_VALUE; ++i)
	{
		if (i < 13)
		{
			m_cbRemainCard[i] = 4;
		}
		else
		{
			m_cbRemainCard[i] = 1;
		}
	}*/

	//游戏底牌
	memset(m_cbBankerCard, 0, sizeof(m_cbBankerCard));
	//扑克数目
	memset(m_cbHandCardCount, 0, sizeof(m_cbHandCardCount));
	//手上扑克
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	//是否托管
	memset(m_cbTrustee, 0, sizeof(m_cbTrustee));
	//超时次数
	memset(m_cbTimeOutCount, 0, sizeof(m_cbTimeOutCount));
	
	//游戏结算
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));

	memset(m_RecordScoreInfo, 0, sizeof(m_RecordScoreInfo));

	if (NULL != m_pGameDeskPtr)
	{//获取私人房信息
		m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
		m_dwlCellScore = m_pGameDeskPtr->GetGameCellScore();
		m_pGameRoomKindInfo = m_pGameDeskPtr->GetGameKindInfo();
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
	//开始删除所有定时器
	//m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY);
	m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
	m_pGameDeskPtr->KillGameTimer(IDI_ADD_DOUBLE);
	m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
	m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER);
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwWinChairID)
{
	//删除出牌定时器
	m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
	m_pGameDeskPtr->KillGameTimer(IDI_CHECK_END);

	if (dwWinChairID>=INVALID_CHAIR)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "NormalGameEnd Err!";
		return;
	}

	DDZ_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));

	//炸弹信息
	if (m_cbBombCount == 0)
	{
		GameEnd.cbBombTimes = 0;
	}
	else
	{
		GameEnd.cbBombTimes = 1;
		for (int i = 0; i < m_cbBombCount; i++)
		{
			GameEnd.cbBombTimes *= SCORE_TIME_BOMB;
		}	
	}
	
	GameEnd.cbMissileTimes = m_cbMissile*SCORE_TIME_BOMB;
	GameEnd.cbCallScore = (CT_BYTE)m_dwStartTime;

	//用户扑克和加倍信息
	CT_DWORD dwUserDouble[GAME_PLAYER] = { 1,1,1 };
	CT_BYTE cbDoubleCount = 0;
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		//拷贝扑克
		GameEnd.cbCardCount[i] = m_cbHandCardCount[i];
		memcpy(&GameEnd.cbHandCardData[i], m_cbHandCardData[i], m_cbHandCardCount[i] * sizeof(CT_BYTE));

		GameEnd.cbDoubleInfo[i] = m_cbAddDoubleInfo[i];
		if (i != m_dwBankerUser)
		{
			if (CB_ADD_DOUBLE == m_cbAddDoubleInfo[i])
			{
				++cbDoubleCount;
				dwUserDouble[i] *= 2L;
			}
		}
	}

	//春天判断
	if (dwWinChairID == m_dwBankerUser)
	{
		//用户定义
		CT_WORD wUser1 = GetNextUser(m_dwBankerUser);
		CT_WORD wUser2 = GetNextUser(m_dwBankerUser+1);

		//用户判断
		if ((m_cbOutCardCount[wUser1] == 0) && (m_cbOutCardCount[wUser2] == 0))
		{
			GameEnd.bChunTian = 1;
			m_dwChunTianbTime = SCORE_TIME_CHUANTIAN;
		}
	}
	else
	{
		//反春天判断
		if (m_cbOutCardCount[m_dwBankerUser] == 1)
		{
			GameEnd.bFanChunTian = 1;
			m_dwChunTianbTime = SCORE_TIME_FANCHUAN;
		}
	}

	//积分变量
	tagScoreInfo ScoreInfoArray[GAME_PLAYER];
	memset(&ScoreInfoArray,0, sizeof(ScoreInfoArray));

	CT_INT32 iCellScore = m_dwlCellScore;

	if (m_wMaxBombCount != 1)
	{
		m_dwBombTime = m_dwBombTime > m_wMaxBombCount ? m_wMaxBombCount : m_dwBombTime;
	}
	
	CT_INT32 iBankerScore = iCellScore *m_dwStartTime* m_dwBombTime * m_dwChunTianbTime;

	CT_INT32 iGameScore[GAME_PLAYER] = { 0,0,0 };
	CT_BYTE bWinMaxScore[GAME_PLAYER] = { 0, 0, 0 };
	//庄家赢
	if (m_cbHandCardCount[m_dwBankerUser] == 0)
	{
		//庄家最大赢分
		CT_LONGLONG llBankerMaxWinScore = m_pGameDeskPtr->GetUserScore(m_dwBankerUser);
		CT_INT32 iBankerWinScore = 0;
		for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
		{
			if (i != m_dwBankerUser)
			{
				//玩家分不够的话有多少输多少
				//CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
				CT_LONGLONG llLostScore = iBankerScore*dwUserDouble[i];//llUserScore > iBankerScore*dwUserDouble[i] ? iBankerScore*dwUserDouble[i] : llUserScore;

				ScoreInfoArray[i].iScore -= (CT_INT32)llLostScore;
				ScoreInfoArray[i].cbType = SCORE_TYPE_LOSE;
				GameEnd.bLose[i] = GAME_LOSE;

				//设置积分
				iBankerWinScore += llLostScore;
				//ScoreInfoArray[m_dwBankerUser].iScore += (CT_INT32)llLostScore;
			}
		}

		//判断庄家是否达到赢分最大值,则退还多出的金额
        CT_INT32 llReturnBackPartScore = 0;
		if (iBankerWinScore > llBankerMaxWinScore)
		{
			bWinMaxScore[m_dwBankerUser] = 1;
			CT_INT32 llReturnBackScore = iBankerWinScore - (CT_INT32)llBankerMaxWinScore;
			//ScoreInfoArray[m_dwBankerUser].iScore -= llReturnBackScore;
			if (cbDoubleCount == 1)
			{
				llReturnBackPartScore = llReturnBackScore / 3;
			}
			else
			{
				llReturnBackPartScore = llReturnBackScore / 2;
			}
		}

        //金币校正(退还金额和保证闲家够扣）
        for (CT_BYTE i = 0; i != GAME_PLAYER; ++i)
        {
            if (i != m_dwBankerUser)
            {
                //需要退还金币
                if (llReturnBackPartScore != 0)
                {
                    if (cbDoubleCount == 1)
                    {
                        if (CB_ADD_DOUBLE == m_cbAddDoubleInfo[i])
                        {
                            ScoreInfoArray[i].iScore += (llReturnBackPartScore * 2);
                        }
                        else
                        {
                            ScoreInfoArray[i].iScore += llReturnBackPartScore;
                        }
                    }
                    else
                    {
                        ScoreInfoArray[i].iScore += llReturnBackPartScore;
                    }
                }
                //保证闲家够扣
                CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
                if (ScoreInfoArray[i].iScore + llUserScore < 0)
                {
                    ScoreInfoArray[i].iScore = -llUserScore;
                }

                //ScoreInfoArray[i].iScore里面的值是负数，所以banker user应该是减低去这个值
				ScoreInfoArray[m_dwBankerUser].iScore -= ScoreInfoArray[i].iScore;
            }
        }

		ScoreInfoArray[m_dwBankerUser].cbType = SCORE_TYPE_WIN;
		GameEnd.bLose[m_dwBankerUser] = GAME_WIN;
	}
	else
	{
		//先统计庄家总输分
		CT_INT32 llBankLostScore = 0;
		CT_INT32 llTrueWinScore[GAME_PLAYER] = { 0,0,0 };
		for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
		{
			if (i != m_dwBankerUser)
			{
				llBankLostScore += iBankerScore*dwUserDouble[i];
				llTrueWinScore[i] += llBankLostScore;
			}
		}

		CT_LONGLONG llBankScore = m_pGameDeskPtr->GetUserScore(m_dwBankerUser);
		if (llBankScore > llBankLostScore)
		{
			ScoreInfoArray[m_dwBankerUser].cbType = SCORE_TYPE_LOSE;
			GameEnd.bLose[m_dwBankerUser] = GAME_LOSE;
			ScoreInfoArray[m_dwBankerUser].iScore -= llBankLostScore;

			for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
			{
				if (i != m_dwBankerUser)
				{
					CT_INT32 llWinScore = iBankerScore*dwUserDouble[i];

					//计算是否超过最大金额
					CT_LONGLONG llMaxWinScore = m_pGameDeskPtr->GetUserScore(i);
					if (llWinScore > llMaxWinScore)
					{
						bWinMaxScore[i] = 1;
						CT_INT32 llReturnBackScore = llWinScore - (CT_INT32)llMaxWinScore;
						llWinScore -= llReturnBackScore;
						ScoreInfoArray[m_dwBankerUser].iScore += llReturnBackScore;
					}

					//写入积分
					ScoreInfoArray[i].iScore += llWinScore;
					ScoreInfoArray[i].cbType = SCORE_TYPE_WIN;
					GameEnd.bLose[i] = GAME_WIN;
				}
			}
		}
		else
		{
			//如果只有一个玩家加倍
			CT_INT32 llScaleBankScore = 0;
			if (cbDoubleCount == 1)
			{
				llScaleBankScore = (CT_INT32)llBankScore / 3;
			}
			else
			{
				llScaleBankScore = (CT_INT32)llBankScore / 2;
			}
			
			ScoreInfoArray[m_dwBankerUser].cbType = SCORE_TYPE_LOSE;
			GameEnd.bLose[m_dwBankerUser] = GAME_LOSE;
			ScoreInfoArray[m_dwBankerUser].iScore -= (CT_INT32)llBankScore;

			for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
			{
				if (i != m_dwBankerUser)
				{
					CT_INT32 llWinScore = 0;
					if (m_cbAddDoubleInfo[i] == CB_ADD_DOUBLE)
					{
						if (cbDoubleCount == 1)
						{
							llWinScore += llScaleBankScore * 2;
						}
						else
						{
							llWinScore += llScaleBankScore;
						}
					}
					else
					{
						llWinScore += llScaleBankScore;
					}

					CT_LONGLONG llWinMaxScore = m_pGameDeskPtr->GetUserScore(i);
					if (llWinScore > llWinMaxScore)
					{
						bWinMaxScore[i] = 1;
						CT_INT32 llReturnBackScore = llWinScore - (CT_INT32)llWinMaxScore;
						llWinScore -= llReturnBackScore;
						ScoreInfoArray[m_dwBankerUser].iScore += llReturnBackScore;
					}
					
					ScoreInfoArray[i].iScore = llWinScore;
					ScoreInfoArray[i].cbType = SCORE_TYPE_WIN;
					GameEnd.bLose[i] = GAME_WIN;
				}
			}
			
			//检测农民是否可以再拿到一些钱
			if (ScoreInfoArray[m_dwBankerUser].iScore + llBankScore != 0)
			{
				for (CT_BYTE i = 0; i != GAME_PLAYER; ++i)
				{
					if (i == m_dwBankerUser)
					{
						continue;
					}

					if (bWinMaxScore[i] == 0 && llTrueWinScore[i] > ScoreInfoArray[i].iScore)
					{
						CT_INT32 iNeedMoreScore = llTrueWinScore[i] - ScoreInfoArray[i].iScore;
						CT_INT32 iBankerLeftScore = (CT_INT32)(llBankScore + ScoreInfoArray[m_dwBankerUser].iScore);
						CT_INT32 iCompensationScore = (iBankerLeftScore > iNeedMoreScore) ? iNeedMoreScore : iBankerLeftScore;

						ScoreInfoArray[i].iScore += iCompensationScore;
						ScoreInfoArray[m_dwBankerUser].iScore -= iCompensationScore;
						break;
					}
				}
			}
		}
	}

	//写入积分
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		iGameScore[i] = ScoreInfoArray[i].iScore;

		m_RecordScoreInfo[i].dwUserID = m_pGameDeskPtr->GetUserID(i);
		m_RecordScoreInfo[i].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(i);
		m_RecordScoreInfo[i].cbStatus = 1;

		CT_INT32 iRevenue = 0;
		m_RecordScoreInfo[i].iScore = iGameScore[i];
		m_RecordScoreInfo[i].llSourceScore = m_pGameDeskPtr->GetUserScore(i);
		//计算税收
		if (iGameScore[i] > 0 && (m_RecordScoreInfo[i].cbIsAndroid == 0))
		{
			iRevenue = (CT_INT32)m_pGameDeskPtr->CalculateRevenue(i, iGameScore[i]);
			iGameScore[i] -= iRevenue;
			m_RecordScoreInfo[i].dwRevenue = iRevenue;
		}
		ScoreInfo  ScoreData;
		//memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = i;
		ScoreData.bBroadcast = true;
		ScoreData.llScore = iGameScore[i];
		ScoreData.llRealScore= iGameScore[i];
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);
	}

	//把各个玩家的分数转为double,并将所有玩家取消托管
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		GameEnd.dGameScore[i] = iGameScore[i] * 0.01f;
	}

	//发送数据
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		GameEnd.bWinMaxScore = bWinMaxScore[i];
		
		if (i == m_dwBankerUser)
        {
            SendTableData(i, SUB_S_GAME_CONCLUDE, &GameEnd, sizeof(GameEnd), true);
        }
        else
        {
            SendTableData(i, SUB_S_GAME_CONCLUDE, &GameEnd, sizeof(GameEnd), false);
        }
        
		if (m_cbTrustee[i] == 1)
		{
			SetTrustee(i, 0);
		}
	}
    
    CT_DWORD dwBankerUserID = 0;
    if (m_dwBankerUser != INVALID_CHAIR)
    {
        dwBankerUserID = m_pGameDeskPtr->GetUserID(m_dwBankerUser);
    }
	
	//记录基本信息
	m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, NULL, 0, dwBankerUserID, 0, 0, 0, 0, 0);

	//切换用户
	m_dwFirstUser = dwWinChairID;
	return;
}

//发送总结算
void CGameProcess::SendTotalClearing(std::string strTime)
{
	/**************************************************
	DDZ_CMD_S_CLEARING ClearingData;
	memset(&ClearingData, 0, sizeof(DDZ_CMD_S_CLEARING));
	ClearingData.wRoomOwner = m_pGameDeskPtr->GetPRoomOwnerChairID();
	ClearingData.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
	//当前时间
	_snprintf_info(ClearingData.szCurTime, sizeof(ClearingData.szCurTime), "%s", strTime.c_str());
	for (CT_WORD i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		ClearingData.llTotalLWScore[i] = m_iTotalLWScore[i];
		ClearingData.cbRombCount[i] = m_cbTotalRomb[i];
		ClearingData.wPlayWinCount[i] = m_wPlayWinCount[i];
		ClearingData.wPlayLoseCount[i] = m_wPlayLoseCount[i];
	}
	for (CT_BYTE i = 0;i < GAME_PLAYER;i++)
	{
		SendTableData(i, SUB_S_CLEARING, &ClearingData, sizeof(ClearingData),(i == 0) ? true : false);
	}
	**************************************************/
}

//解释私人场数据
void CGameProcess::ParsePrivateData(CT_CHAR *pBuf)
{
	if (strlen(pBuf)==0)
	{
		return;
	}
	//获得私人扬数据
	acl::json jsonCond(pBuf);

	const acl::json_node* nodeCond =  jsonCond.getFirstElementByTagName("cbBombCount");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "cbBombCount error!";
		return;
	}
	CT_BYTE cbBombCount = (CT_BYTE) *nodeCond->get_int64();
	for (CT_BYTE i = 0; i < cbBombCount; i++)
	{
		m_wMaxBombCount *= SCORE_TIME_BOMB;
	} 
	
	LOG_IF(INFO, PRINT_LOG_INFO) << "ParsePrivateData: m_cbMaxBombCount = "<<(int)m_wMaxBombCount << ",cbBombCount =" << (int)cbBombCount;
}

//玩家出牌
CT_BOOL CGameProcess::OnUserOutCard(CT_WORD wChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout/*=CT_FALSE*/)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard: wChairID = " << wChairID << ",cbOutCount = " << (int)cbOutCount;
	//效验状态
	//assert(wChairID == m_dwCurrentUser);
	/*printf("player %d out card: ", wChairID);
	for (int j = 0; j < cbOutCount; ++j)
	{
		printf("%x, ", cbOutCard[j]);
	}
	printf("\n");*/

	//机器人先不托管
	if (m_pGameDeskPtr->IsAndroidUser(wChairID) && m_cbTrustee[wChairID] == 1)
	{
		SetTrustee(wChairID, 0);
	}

	if (cbOutCount <= 0)
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);
		return false;
	}

	//判断出牌是否合法
	if (cbOutCount > m_cbHandCardCount[wChairID])
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);

		LOG(WARNING) << "out card count more than hand card count ?";
		return false;
	}

	for (CT_BYTE i = 0; i < cbOutCount; ++i)
	{
		if (cbOutCard[i] == 0)
		{
			if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
				m_pGameDeskPtr->CloseUserConnect(wChairID);

			LOG(WARNING) << "out card data is empty? ";
			return true;
		}
	}

	/*if (cbOutCard[0] == 0)
	{
		LOG(WARNING) << "card data is empty!";
		return false;
	}*/

	CT_BYTE cbOutCardTemp[MAX_COUNT];
	memcpy(cbOutCardTemp, cbOutCard, cbOutCount);
	m_GameLogic.SortCardList(cbOutCard, cbOutCount, ST_ORDER);

	if (wChairID != m_dwCurrentUser) 
	{
        if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
            m_pGameDeskPtr->CloseUserConnect(wChairID);
		LOG(WARNING) << "OnUserOutCard not this user! current user: " << m_dwCurrentUser << ", out card user: " << wChairID;
		return true;
	}

	//获取类型
	CT_BYTE cbCardType = m_GameLogic.GetCardType(cbOutCard, cbOutCount);

	//类型判断
	if (cbCardType == CT_ERROR)
	{
		//assert(FALSE);
		acl::string strCardData;
        for (CT_BYTE i = 0;  i != cbOutCount; ++i)
        {
            //LOG(ERROR) << std::hex << (int)cbOutCard[i];
            strCardData.format_append("%x ", cbOutCard[i]);
        }
		LOG(ERROR) << "user out card, card type error, card data: " << strCardData.c_str() ;

        if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
            m_pGameDeskPtr->CloseUserConnect(wChairID);
		return true;
	}

	//出牌判断
	if (m_cbTurnCardCount != 0)
	{
		//对比扑克
		if (m_GameLogic.CompareCard(m_cbTurnCardData, cbOutCard, m_cbTurnCardCount, cbOutCount) == false)
		{
			//assert(FALSE);
			LOG(WARNING) << "compare card error.";
            m_pGameDeskPtr->CloseUserConnect(wChairID);
			return false;
		}
	}

	//删除扑克
	if (m_GameLogic.RemoveCardList(cbOutCard, cbOutCount, m_cbHandCardData[wChairID], m_cbHandCardCount[wChairID]) == false)
	{
		//assert(FALSE);
		LOG(WARNING) << "remove card error.";
		return false;
	}
	
	//出牌记录
	CT_BYTE cbOutCardCount = m_cbOutCardCount[wChairID];
	m_cbOutCardListCount[wChairID][cbOutCardCount] = cbOutCount;
	memcpy(&m_cbOutCardList[wChairID][cbOutCardCount], cbOutCard, cbOutCount);
	//出牌变量
	m_cbOutCardCount[wChairID]++;

	//设置变量
	m_cbTurnCardCount = cbOutCount;
	m_cbHandCardCount[wChairID] -= cbOutCount;
	memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	memcpy(m_cbTurnCardData, cbOutCard, sizeof(CT_BYTE)*cbOutCount);

	//炸弹判断
	if (cbCardType == CT_BOMB_CARD)
	{
		m_cbBombCount++;
		m_dwBombTime *= SCORE_TIME_BOMB;
		m_cbEachBombCount[wChairID]++;
		
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			m_wEachTotalTime[i] *= SCORE_TIME_BOMB;
		}
	}
	else if (cbCardType == CT_MISSILE_CARD)
	{
		m_cbMissile = 1;
		m_dwBombTime *= SCORE_TIME_BOMB;
		m_cbEachBombCount[wChairID]++;

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			m_wEachTotalTime[i] *= SCORE_TIME_BOMB;
		}
	}

	//切换用户
	m_dwTurnWiner = wChairID;
	if (m_cbHandCardCount[wChairID] != 0)
	{
		//if (cbCardType != CT_MISSILE_CARD)
		//{
			m_dwCurrentUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
		//}
	}
	else m_dwCurrentUser = INVALID_CHAIR;

	//构造数据
	CMD_S_OutCard OutCard;
	OutCard.dwOutCardUser = wChairID;
	OutCard.cbCardCount = cbOutCount;
	OutCard.dwCurrentUser = m_dwCurrentUser;
	OutCard.cbTimeLeft = TIME_OUT_CARD;
	memcpy(OutCard.cbCardData, cbOutCardTemp, m_cbTurnCardCount * sizeof(CT_BYTE));
	
	/*for (CT_BYTE i = 0; i < cbOutCount; ++i)
	{
		CT_BYTE cbCardData = cbOutCard[i];
		CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCardData);
		CT_BYTE cbIndex = cbCardValue - 1;
		if (cbIndex < 0)
		{
			continue;
		}
		--m_cbRemainCard[cbIndex];
	}*/

	//发送数据
	CT_DWORD wHeadSize = sizeof(OutCard) - sizeof(OutCard.cbCardData);
	CT_DWORD wSendSize = wHeadSize + OutCard.cbCardCount * sizeof(CT_BYTE);
	for (CT_BYTE i = 0;i < GAME_PLAYER;i++)
	{
		if (i == m_dwBankerUser)
		{
			OutCard.wUserTimes = m_dwBombTime * 2;
            SendTableData(i, SUB_S_OUT_CARD, &OutCard, wSendSize, true);
		}
		else
		{
			OutCard.wUserTimes = m_dwBombTime;
            SendTableData(i, SUB_S_OUT_CARD, &OutCard, wSendSize, false);
		}
	}

	//出牌最大
	//if (cbCardType == CT_MISSILE_CARD) m_cbTurnCardCount = 0;

	//结束判断
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		OnEventGameEnd(wChairID, GER_NORMAL);
	}
	else
	{
		//进入托管流程
		IsTrustee();
	}
	
	return true;
}

//玩家不出
CT_BOOL CGameProcess::OnUserPassCard(CT_WORD wChairID)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard :wChairID = " << wChairID ;
	//效验状态
	//assert((wChairID == m_dwCurrentUser) && (m_cbTurnCardCount != 0));
	if ((wChairID != m_dwCurrentUser) || (m_cbTurnCardCount == 0))
	    return false;

	//设置变量
	m_dwCurrentUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
	if (m_dwCurrentUser == m_dwTurnWiner) m_cbTurnCardCount = 0;

	//构造消息
	CMD_S_PassCard PassCard;
	PassCard.dwPassCardUser = wChairID;
	PassCard.dwCurrentUser = m_dwCurrentUser;
	PassCard.cbTurnOver = (m_cbTurnCardCount == 0) ? 1 : 0;
	PassCard.cbTimeLeft = TIME_OUT_CARD;
	//PassCard.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();
	//发送数据
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), true);
        }
        else
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), false);
        }
	}
	//托管
	IsTrustee();
	return true;
}

CT_BOOL CGameProcess::OnBankerInfo()
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << " OnBankerInfo: m_dwBankerUser = " << m_dwBankerUser;
	//设置状态
	//m_cbGameStatus = GAME_SCENE_PLAY;
	m_cbGameStatus = GAME_SCENE_DOUBLE;

	//发送底牌
	m_cbHandCardCount[m_dwBankerUser] += CountArray(m_cbBankerCard);
	memcpy(&m_cbHandCardData[m_dwBankerUser][NORMAL_COUNT], m_cbBankerCard, sizeof(m_cbBankerCard));

	//排列扑克
	m_GameLogic.SortCardList(m_cbHandCardData[m_dwBankerUser], m_cbHandCardCount[m_dwBankerUser], ST_ORDER);

	//发送消息
	CMD_S_BankerInfo BankerInfo;
	BankerInfo.dwBankerUser = m_dwBankerUser;
	BankerInfo.dwCurrentUser = m_dwCurrentUser;
	BankerInfo.cbTimeLeft = TIME_ADD_DOUBLE;
	memcpy(BankerInfo.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));

	//发送消息并确定每个玩家当前倍数
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_dwBankerUser)
		{
			m_wEachTotalTime[i] = 2;
            SendTableData(i, SUB_S_BANKER_INFO, &BankerInfo, sizeof(BankerInfo), true);
		}
		else
		{
			m_wEachTotalTime[i] = 1;
            SendTableData(i, SUB_S_BANKER_INFO, &BankerInfo, sizeof(BankerInfo), false);
		}
	}
	return true;
}

CT_BOOL CGameProcess::OnUserCallBanker(CT_WORD wChairID, CT_WORD cbCallInfo)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserCallBanker: wChairID = " << wChairID << ",cbCallInfo = " << cbCallInfo;
	//效验状态
	//assert(wChairID == m_dwCurrentUser);
	if (wChairID != m_dwCurrentUser) 
	{ 
		LOG(WARNING) << "OnUserCallBanker, is not current user. chair id: " << wChairID << ", current user: " << m_dwCurrentUser;
		return false;
	}
		

	//效验状态
	if (CB_NOT_CALL < m_cbCallBankerInfo[wChairID])
	{
		LOG(WARNING) << "OnUserCallBanker, has call banker? banker info: " << (CT_WORD)m_cbCallBankerInfo[wChairID];
		return false;
	}

	//效验参数
	//assert(cbCallInfo >= CB_NOT_CALL && cbCallInfo <= CB_NO_CALL_BENKER);
	if (cbCallInfo< CB_NOT_CALL || cbCallInfo > CB_NO_CALL_BENKER)
		cbCallInfo = CB_NO_CALL_BENKER;

	CT_DWORD dwMaxCallBankerUser = 0; //当前已叫地主最大分数的玩家
	CT_WORD cbMaxBankerScore = 0;//当前已叫地主的最大分数
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_cbCallBankerInfo[i] >= CB_CALL_BENKER_1 && m_cbCallBankerInfo[i] <= CB_CALL_BENKER_3)
		{
			if (cbMaxBankerScore < m_cbCallBankerInfo[i])
			{
				cbMaxBankerScore = m_cbCallBankerInfo[i];
				dwMaxCallBankerUser = i;
			}
		}
	}

	//判断是否已经第三轮最后一个人没有叫地主,这个玩家默认叫1分
	if (m_cbNoBankCount >= MAX_NO_CALL_BANKER && cbMaxBankerScore == 0 && cbCallInfo == CB_NO_CALL_BENKER && m_dwFirstUser == GetNextUser(wChairID))
	{
		cbCallInfo = CB_CALL_BENKER_1;
	}

	//检查叫分
	if (cbCallInfo != CB_NO_CALL_BENKER && cbCallInfo <= cbMaxBankerScore)
	{
		LOG(WARNING) << "call banker bigger than mac banker score! call info: " << (CT_WORD)cbCallInfo << ", max banker info: " << (CT_WORD)cbMaxBankerScore;
		return false;
	}

	if (cbCallInfo != CB_NO_CALL_BENKER)
	{
		cbMaxBankerScore = cbCallInfo;
		dwMaxCallBankerUser = wChairID;
	}
	m_cbCallBankerInfo[wChairID] = cbCallInfo;

	//设置用户
	if ((cbCallInfo == CB_CALL_BENKER_3) || (m_dwFirstUser == GetNextUser(wChairID)))
	{
		m_dwCurrentUser = INVALID_CHAIR;
	}
	else
	{
		m_dwCurrentUser = GetNextUser(wChairID);
	}
	m_dwStartTime = cbMaxBankerScore;
	//构造变量
	CMD_S_CallBanker CallBanker;
	
	CallBanker.cbCallInfo = cbCallInfo;
	//CallBanker.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();
	CallBanker.dwLastUser = wChairID;
	CallBanker.dwBankerUser = INVALID_CHAIR;
	CallBanker.dwCurrentUser = m_dwCurrentUser;
	CallBanker.cbTimeLeft = TIME_CALL_BANKER;

	if ((cbCallInfo == CB_CALL_BENKER_3) || (m_dwFirstUser == GetNextUser(wChairID)))
	{
		//三家都不抢地主,重新发牌，并由下一家优先叫地主
		if ((cbMaxBankerScore == 0) && (m_dwFirstUser ==  GetNextUser(wChairID)))
		{
			for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
			{
			    if (m_dwCurrentUser == i)
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), true);
                }
                else
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), false);
			    }
			}

			m_dwFirstUser = GetNextUser(m_dwFirstUser);
			++m_cbNoBankCount;
			OnEventGameStart();	
			return true;
		}
		else
		{
			//设置状态
			m_cbGameStatus = GAME_SCENE_DOUBLE;
			m_dwBankerUser = dwMaxCallBankerUser;
			m_dwCurrentUser = m_dwBankerUser;

			CallBanker.dwBankerUser = m_dwBankerUser;
			CallBanker.dwCurrentUser = m_dwCurrentUser;
			//发送消息
			for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
			{
                if (m_dwCurrentUser == i)
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), true);
                }
                else
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), false);
                }
			}

			//地主确定
			//LOG(WARNING) << "call isTrustee2!";
			//重置叫庄次数
			m_cbNoBankCount = 1;
			m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
			IsTrustee();
			return OnBankerInfo();
		}
	}

	//发送消息
	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
        if (m_dwCurrentUser == i)
        {
            SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), true);
        }
        else
        {
            SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), false);
        }
	}
	//托管
	//LOG(WARNING) << "call isTrustee!";
	m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
	IsTrustee();
	return true;
}

CT_BOOL CGameProcess::OnUserDouble(CT_WORD wChairID, CT_BYTE cbDoubleInfo)
{
	//效验状态
	if (CB_NOT_ADD_DOUBLE != m_cbAddDoubleInfo[wChairID])
	{
		return false;
	}

	if (CB_NOT_ADD_DOUBLE == cbDoubleInfo)
		cbDoubleInfo = CB_NO_ADD_DOUBLE;

	m_cbAddDoubleInfo[wChairID] = cbDoubleInfo;

	//确定倍数（地主不能主动加倍）
	/*if (m_cbAddDoubleInfo[wChairID] == CB_ADD_DOUBLE)
	{
		m_wEachTotalTime[wChairID] += 1;
		m_wEachTotalTime[m_dwBankerUser] += 1;
	}*/

	CMD_S_Double DoubleInfo;
	DoubleInfo.dwCurrentUser = wChairID;
	DoubleInfo.cbDouble = cbDoubleInfo;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		//这个倍数是指除加倍以外的倍数
		if (i == m_dwBankerUser)
		{
			DoubleInfo.wUserTimes = 2;
            SendTableData(i, SUB_S_ADD_DOUBLE, &DoubleInfo, sizeof(DoubleInfo), true);
		}
		else
		{
			DoubleInfo.wUserTimes = 1;
            SendTableData(i, SUB_S_ADD_DOUBLE, &DoubleInfo, sizeof(DoubleInfo), false);
		}
		//DoubleInfo.wUserTimes = m_wEachTotalTime[i];
		//发送消息
	}

	//是否开始
	bool bAddEnd = true;
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_dwBankerUser)
		{
			continue;
		}

		if (CB_NOT_ADD_DOUBLE == m_cbAddDoubleInfo[i])
		{
			bAddEnd = false;
			break;
		}
	}

	if (bAddEnd)
	{
		OnSubOutCardStart();
	}

	return true;
}

//出牌开始
CT_BOOL CGameProcess::OnSubOutCardStart()
{
	m_cbGameStatus = GAME_SCENE_PLAY;

	//发送消息
	CMD_S_StartOutCard startOutCard;
	startOutCard.dwBankerUser = m_dwBankerUser;
	startOutCard.dwCurrentUser = m_dwCurrentUser;
	startOutCard.cbLeftTime = TIME_FIRST_OUT_CARD;
	
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_dwBankerUser)
			SendTableData(i, SUB_S_OUT_START_START, &startOutCard, sizeof(startOutCard), true);
		else
			SendTableData(i, SUB_S_OUT_START_START, &startOutCard, sizeof(startOutCard), false);
	}

	//删除自动加倍定时器
	m_pGameDeskPtr->KillGameTimer(IDI_ADD_DOUBLE);

	//设定一个4S后出牌的定时器（因为客户端要显示确定庄家信息）
	if (m_cbTrustee[m_dwCurrentUser])
	{
		m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, 2000);
	}
	else
	{
		m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (TIME_FIRST_OUT_CARD + TIME_NETWORK_COMPENSATE) * 1000);
	}

	return true;
}

CT_WORD CGameProcess::GetUserCallBanker()
{
	CT_WORD cbMaxBenkerScore = 0;
	CT_WORD wBenker = 0;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_cbCallBankerInfo[i] >= CB_CALL_BENKER_1 && m_cbCallBankerInfo[i] <= CB_CALL_BENKER_3)
		{
			if (cbMaxBenkerScore < m_cbCallBankerInfo[i])
			{
				cbMaxBenkerScore = m_cbCallBankerInfo[i];
				wBenker = i + 1;
			}
		}
	}
	return wBenker;
}

//获得下一个玩家
CT_DWORD CGameProcess::GetNextUser(CT_DWORD dwChairID)
{
	return (dwChairID + 1) % GAME_PLAYER;
}

CT_BOOL CGameProcess::IsTrustee(bool bIsNewTurn/* = true*/)
{
	//非常规座位
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		LOG(WARNING) << "set trustee, but current user is invalid!";
		return false;
	}

	//不允许代打
	//if (m_cbTrustee[m_dwCurrentUser] != 1) return false;


	//删除超时定时器
	//m_pGameDeskPtr->KillGameTimer(IDI_CHECK_TRUSTEE);

	//CT_DWORD dwTimerTime = 1;
	//m_pGameDeskPtr->SetGameTimer(IDI_CHECK_TRUSTEE, dwTimerTime * 1000);

	//根据不同状态设置不同的定时器
	switch (m_cbGameStatus)
	{
	case GAME_SCENE_DOUBLE:
	{
		if (!m_pGameDeskPtr->IsHasGameTimer(IDI_ADD_DOUBLE))
		{
			m_pGameDeskPtr->SetGameTimer(IDI_ADD_DOUBLE, (TIME_ADD_DOUBLE + TIME_NETWORK_COMPENSATE) * 1000);
			m_cbOpTotalTime = TIME_ADD_DOUBLE;
			m_dwOpStartTime = (CT_DWORD)time(NULL);
		}
	}
	break;
	case GAME_SCENE_CALL:
	{
		//LOG(WARNING) << "set IDI_CALL_BANKER timer!";
		m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
		if (m_cbTrustee[m_dwCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_CALL_BANKER, 1000);
		}
		else
		{
			if (m_dwCurrentUser == m_dwFirstUser)
			{
				m_pGameDeskPtr->SetGameTimer(IDI_CALL_BANKER, (TIME_CALL_BANKER + TIME_DEAL_CARD + TIME_NETWORK_COMPENSATE) * 1000);
			}
			else
			{
				m_pGameDeskPtr->SetGameTimer(IDI_CALL_BANKER, (TIME_CALL_BANKER + TIME_NETWORK_COMPENSATE) * 1000);
			}
		}
		m_cbOpTotalTime = TIME_CALL_BANKER;
		m_dwOpStartTime = (CT_DWORD)time(NULL);
	}
	break;
	case GAME_SCENE_PLAY:
	{
		m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
		if (m_cbTrustee[m_dwCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, 2000);
		}
		else
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (TIME_OUT_CARD + TIME_NETWORK_COMPENSATE) * 1000);
		}
		if(bIsNewTurn)
        {
            m_cbOpTotalTime = TIME_OUT_CARD;
            m_dwOpStartTime = (CT_DWORD)time(NULL);
        }
	}
	break;
	default:
		break;
	}

	return true;
}

CT_VOID CGameProcess::SetTrustee(CT_DWORD dwChairID, CT_BYTE cbTrustee)
{
	if (dwChairID == INVALID_CHAIR)
	{
		return;
	}

	//构造消息
	m_cbTrustee[dwChairID] = cbTrustee;
	/*//临时屏蔽机器人的托管下发
	if (m_pGameDeskPtr->IsAndroidUser(dwChairID))
	{
		return;
	}*/

	CMD_S_StatusTrustee StatusTrustee;
	memcpy(StatusTrustee.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
	//发送数据
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
        }
        else
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
        }
	}
}

CT_BOOL CGameProcess::OnAutoCallLand()
{
	//当前已叫地主的最大分数
	/*CT_WORD cbMaxBenkerScore = 0;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_cbCallBankerInfo[i] >= CB_CALL_BENKER_1 && m_cbCallBankerInfo[i] <= CB_CALL_BENKER_3)
		{
			if (cbMaxBenkerScore < m_cbCallBankerInfo[i])
			{
				cbMaxBenkerScore = m_cbCallBankerInfo[i];
			}
		}
	}
	//判断是否叫地主
	int bCallBanker = rand() % 2;
	if (bCallBanker)
	{
		return OnUserCallBanker(m_dwCurrentUser, cbMaxBenkerScore + 1);
	}
	else
	{
		return OnUserCallBanker(m_dwCurrentUser, CB_NO_CALL_BENKER);
	}*/

	//if (m_cbNoBankCount >= 2 && m_dwCurrentUser == m_dwFirstUser)
		//return OnUserCallBanker(m_dwCurrentUser, CB_CALL_BENKER_1);
	//else
	return OnUserCallBanker(m_dwCurrentUser, CB_NO_CALL_BENKER);
}

CT_BOOL CGameProcess::OnAutoDouble(CT_DWORD dwChairID)
{
	//默认不叫
	return OnUserDouble(dwChairID, CB_NO_ADD_DOUBLE);
}

CT_BOOL CGameProcess::OnAutoOutCard()
{
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return true;
	}

	//if (0 == m_cbTurnCardCount)
	//{//第一个出牌
	//	CT_BYTE cbCardCount = m_cbHandCardCount[m_dwCurrentUser];
	//	CT_BYTE cbCard[MAX_COUNT] = { 0 };
	//	cbCard[0] = m_cbHandCardData[m_dwCurrentUser][cbCardCount - 1];
	//	return OnUserOutCard(m_dwCurrentUser, cbCard, 1) ;
	//}
	//else
	{
		//跟牌
		//return OnUserPassCard(m_dwCurrentUser) ;

		//检测炸弹直接过牌
		if (m_cbTurnCardCount == 2)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			if (cbCardType == CT_MISSILE_CARD)
			{
				return OnUserPassCard(m_dwCurrentUser);
			}
		}

		//搜索提示
		tagSearchCardResult	SearchCardResult;					//搜索结果
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		try
		{
			m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount,
				&SearchCardResult);
		}
		catch (...)
		{
			SearchCardResult.cbSearchCount = 0;
		}

		if (SearchCardResult.cbSearchCount > 0)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			CT_BYTE cbBestCardIndex = 0;
			for (CT_BYTE i = 0 ; i != SearchCardResult.cbSearchCount; ++i)
			{
				CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[i], SearchCardResult.cbCardCount[i]);
				if (cbCardType != CT_ERROR)
				{
					//相同牌型，直接出牌
					if (cbSearchCardType == cbCardType)
					{
						CT_BYTE cbLastBestCardIndex = cbBestCardIndex;
						cbBestCardIndex = i;
						CT_BOOL bFindBestIndex = true;

						//尽量不要拆炸弹牌型
						if (cbSearchCardType != CT_BOMB_CARD && cbCardType != CT_BOMB_CARD)
						{
							CT_BOOL bBreak = false;
							//找出炸弹
							for (CT_BYTE j = 0; j < SearchCardResult.cbSearchCount; ++j)
							{
								if (j == i || SearchCardResult.cbCardCount[j] != 4)
								{
									continue;
								}

								CT_BYTE cbCardTypeEx = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[j], SearchCardResult.cbCardCount[j]);
								if (cbCardTypeEx == CT_BOMB_CARD)
								{
									CT_BYTE cbCardValue = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[j][0]);
									for (CT_BYTE m = 0; m < SearchCardResult.cbCardCount[i]; ++m)
									{
										CT_BYTE cbCardValueEx = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[i][m]);
										if (cbCardValue == cbCardValueEx)
										{
											cbBestCardIndex = cbLastBestCardIndex;
											bBreak = true;
											bFindBestIndex = false;
											break;
										}
									}
								}

								if (bBreak)
								{
									break;
								}
							}
						}

						//已经找到最好的牌
						if (bFindBestIndex)
						{
							break;
						}
						
					}
				}
				else
				{
					//找出最长牌型
					if (SearchCardResult.cbCardCount[i] > SearchCardResult.cbCardCount[cbBestCardIndex])
					{
						//自己不主动出弹炸
						if (m_cbTurnCardCount == 0 && cbSearchCardType == CT_BOMB_CARD && m_cbHandCardCount[m_dwCurrentUser] != 4)
						{
							continue;
						}

						CT_BYTE cbLastBestCardIndex = cbBestCardIndex;
						cbBestCardIndex = i;

						//尽量不要拆炸弹牌型
						if (cbSearchCardType != CT_BOMB_CARD && cbCardType != CT_BOMB_CARD)
						{
							CT_BOOL bBreak = false;
							//找出炸弹
							for (CT_BYTE j = 0; j < SearchCardResult.cbSearchCount; ++j)
							{
								if (j == i || SearchCardResult.cbCardCount[j] != 4)
								{
									continue;
								}

								CT_BYTE cbCardTypeEx = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[j], SearchCardResult.cbCardCount[j]);
								if (cbCardTypeEx == CT_BOMB_CARD)
								{
									CT_BYTE cbCardValue = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[j][0]);
									for (CT_BYTE m = 0; m < SearchCardResult.cbCardCount[i]; ++m)
									{
										CT_BYTE cbCardValueEx = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[i][m]);
										if (cbCardValue == cbCardValueEx)
										{
											cbBestCardIndex = cbLastBestCardIndex;
											bBreak = true;
											break;
										}
									}
								}

								if (bBreak)
								{
									break;
								}
							}
						}
					}	
				}
			}
			CT_BYTE cbCardData[MAX_COUNT] = { 0 };
			CT_BYTE cbCardCount = SearchCardResult.cbCardCount[cbBestCardIndex];
			memcpy(cbCardData, &SearchCardResult.cbResultCard[cbBestCardIndex], sizeof(CT_BYTE)*cbCardCount);
			//排列扑克
			m_GameLogic.SortCardList(cbCardData, cbCardCount, ST_ORDER);
			return OnUserOutCard(m_dwCurrentUser, cbCardData, cbCardCount);
		}
		else
		{
			if (m_cbTurnCardCount == 0)
			{
				LOG(WARNING) << "user auto pass, but last user is him?  fuck!!!";
			}
			return OnUserPassCard(m_dwCurrentUser);
		}
	}
}

//配好牌
CT_BOOL CGameProcess::MatchGoodCard(CT_BYTE cbRandCard[], CT_BYTE cbRandCount)
{
    //CT_BOOL bConfigCard[GAME_PLAYER] = {false};
    //CT_BYTE cbConfigCount = 0;
    CT_WORD wChairID = INVALID_CHAIR;
    //给初级场玩家赢钱大于7快的玩家配差牌
	CT_INT32 iRand = rand()%100;
	for (int j = 0; j < GAME_PLAYER; ++j)
	{
		if(m_pGameDeskPtr->IsAndroidUser(j)) continue;
		GS_UserBaseData* pUserBaseData = m_pGameDeskPtr->GetUserBaseData(j);
		if(NULL == pUserBaseData) continue;
		auto it = m_mapChannelControl.find(pUserBaseData->dwchannelID);
		if((it != m_mapChannelControl.end()) && (iRand < it->second.wBadCardRation))
		{
			wChairID = j;
		}
		else if(((pUserBaseData->llTotalWinScore >= 700) && (iRand < 30))|| (pUserBaseData->llTotalWinScore >= 900))
		{
			wChairID = j;
		}
	}
    CT_BYTE cbTempCard[FULL_COUNT]={0};
    memcpy(cbTempCard, cbRandCard,CountArray(cbTempCard));

	/*if(INVALID_CHAIR != wChairID)
    {
        for (CT_WORD j = 0; j < GAME_PLAYER; ++j)
        {
            if(wChairID != j)
            {
                bConfigCard[j] = true;
                cbConfigCount++;
            }
        }
    }
    //只有单元积分等于1的场次才会配好牌
    if (cbConfigCount > 0)
    {
		memset(cbRandCard,0,cbRandCount);
		CT_BYTE cbGoodCard[GOOD_CARD_COUTN];
		memset(cbGoodCard,0,CountArray(cbGoodCard));
		m_GameLogic.GetGoodCardData(cbGoodCard, GOOD_CARD_COUTN);
		//抽取好牌
		m_GameLogic.RemoveGoodCardData(cbGoodCard, GOOD_CARD_COUTN, cbTempCard, FULL_COUNT);
		int iGoodIndex = 0;
		int iBadIndex = 0;

		CT_BYTE cbGoodCount = ((2 == cbConfigCount) ? (GOOD_CARD_MAX-1) : GOOD_CARD_MAX);

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			int iNeedGoodCount = 0;
			//三个都配牌
			if(3 == cbConfigCount)
			{
				iNeedGoodCount = (GOOD_CARD_MAX-1);
			}
			else if (bConfigCard[i])
			{
				//分发扑克
				iNeedGoodCount = cbGoodCount;

			}
			else if (2 == cbConfigCount)
			{
				//一个不配牌
				iNeedGoodCount = (GOOD_CARD_COUTN-2*cbGoodCount)-1;
			}
			else if(1 == cbConfigCount)
			{
				//2个不配牌
				//分发扑克
				iNeedGoodCount = (GOOD_CARD_COUTN-cbGoodCount)/2;
			}
			//分发扑克
			memcpy(&cbRandCard[i*NORMAL_COUNT], &cbGoodCard[iGoodIndex], iNeedGoodCount);
			iGoodIndex += iNeedGoodCount;
			memcpy(&cbRandCard[i*NORMAL_COUNT + iNeedGoodCount], &cbTempCard[iBadIndex], NORMAL_COUNT-iNeedGoodCount);
			iBadIndex += (NORMAL_COUNT-iNeedGoodCount);
		}
        //底牌
        if((GOOD_CARD_COUTN-iGoodIndex) > 0)
        {
            memcpy(&cbRandCard[3*NORMAL_COUNT], &cbGoodCard[iGoodIndex], GOOD_CARD_COUTN-iGoodIndex);
        }

        memcpy(&cbRandCard[3*NORMAL_COUNT+(GOOD_CARD_COUTN-iGoodIndex)], &cbTempCard[iBadIndex], FULL_COUNT-GOOD_CARD_COUTN-iBadIndex);

    }*/
    if(INVALID_CHAIR != wChairID)
    {
        memcpy(cbTempCard, &cbRandCard[wChairID*NORMAL_COUNT], NORMAL_COUNT);
        tagAnalyseResult AnalyseResult;
        memset(&AnalyseResult, 0, sizeof(AnalyseResult));
        m_GameLogic.SortCardList(cbTempCard, NORMAL_COUNT, ST_ORDER);
        m_GameLogic.AnalysebCardData(cbTempCard,NORMAL_COUNT,AnalyseResult);
		CT_BYTE cbIndex = 0;
        if(AnalyseResult.cbBlockCount[3] > 0)
        {

            int iCount = 0;
            for (int i = 0; i < NORMAL_COUNT && iCount < 2; ++i)
            {
                CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
                if(m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[3][0]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && (cbIndex < NORMAL_COUNT))
                {

                    CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                    cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex];
                    cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex] = cbTemp;
                    iCount++;
					cbIndex++;
                }
            }
        }
        for (int k = 0; k < AnalyseResult.cbBlockCount[2]; ++k)
        {
            CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
            for (int i = 0; i < NORMAL_COUNT; ++i)
            {
                if((m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) > 7) && m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i])&& (cbIndex < NORMAL_COUNT))
                {
                    CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                    cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex];
                    cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex] = cbTemp;
					cbIndex++;
                }
            }
        }
        int i2Count = 0;
        int iWangCount = 0;
        int iACount = 0;
        for (int j = 0; j < NORMAL_COUNT; ++j)
        {
            if(m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+j]) == m_GameLogic.GetCardLogicValue(0x02))
            {
                i2Count++;
            }
            if(m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+j]) == m_GameLogic.GetCardLogicValue(0x01))
            {
                iACount++;
            }
            //0x4E,0x4F,
            if(cbRandCard[wChairID*NORMAL_COUNT+j] == 0x4E || cbRandCard[wChairID*NORMAL_COUNT+j]==0x4F)
            {
                iWangCount++;
            }
        }

		for (int i = 0; (i < NORMAL_COUNT) && (iACount > 2); ++i)
		{
			CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
			if(iACount > 2 && (m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) == m_GameLogic.GetCardLogicValue(0x01))&& (cbIndex < NORMAL_COUNT))
			{
				CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
				cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex];
				cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex] = cbTemp;
				iACount--;
				cbIndex++;
			}
		}
		for (int i = 0; (i < NORMAL_COUNT) && (i2Count > 2); ++i)
		{
			CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
			if(i2Count > 2 && (m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) == m_GameLogic.GetCardLogicValue(0x02))&& (cbIndex < NORMAL_COUNT))
			{
                for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT && (i2Count > 2); ++j)
                {
                    if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                    {
                        CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                        cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                        cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                        i2Count--;
                        cbIndex++;
                        break;
                    }
                }
			}

		}
		for (int i = 0; (i < NORMAL_COUNT) && (iWangCount > 0); ++i)
		{
			CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
			if(iWangCount > 0 && (cbRandCard[wChairID*NORMAL_COUNT+i]==0x4F||cbRandCard[wChairID*NORMAL_COUNT+i] == 0x4E)&& (cbIndex < NORMAL_COUNT))
			{
                for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT && (iWangCount > 0); ++j)
                {
                    if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                    {
                        CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                        cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                        cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                        iWangCount--;
                        cbIndex++;
                        break;
                    }
                }

			}
		}
        //单连
        tagSearchCardResult tmpSearchCardResult;
        CT_BYTE cbTmpCount = m_GameLogic.SearchLineCardType(cbTempCard, NORMAL_COUNT, 0, 1, 0, &tmpSearchCardResult);
        if (cbTmpCount > 0)
        {
            CT_BYTE cbCard = tmpSearchCardResult.cbResultCard[0][2];
            for (int i = 0; i < NORMAL_COUNT; ++i)
            {
                CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
                if(m_GameLogic.GetCardLogicValue(cbCard) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && (cbIndex < NORMAL_COUNT))
                {
                    for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT; ++j)
                    {
                        if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                        {
                            CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                            cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                            cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                            cbIndex++;
                            break;
                        }
                    }
                }
            }
        }

        memcpy(cbTempCard, &cbRandCard[wChairID*NORMAL_COUNT], NORMAL_COUNT);
        memset(&AnalyseResult, 0, sizeof(AnalyseResult));
        m_GameLogic.SortCardList(cbTempCard, NORMAL_COUNT, ST_ORDER);
        m_GameLogic.AnalysebCardData(cbTempCard,NORMAL_COUNT,AnalyseResult);
        if(AnalyseResult.cbBlockCount[3] > 0)
        {

            int iCount = 0;
            for (int i = 0; i < NORMAL_COUNT && iCount < 2; ++i)
            {
                CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
                if(m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[3][0]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && (cbIndex < NORMAL_COUNT))
                {
                    for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT; ++j)
                    {
                        if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                        {
                            CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                            cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                            cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                            cbIndex++;
                            break;
                        }
                    }
                }
            }
        }
        for (int k = 0; k < AnalyseResult.cbBlockCount[2]-1; ++k)
        {
            CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
            for (int i = 0; i < NORMAL_COUNT; ++i)
            {
                if((m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) > 7) && m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i])&& (cbIndex < NORMAL_COUNT))
                {
                    for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT; ++j)
                    {
                        if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                        {
                            CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                            cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                            cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                            cbIndex++;
                            break;
                        }
                    }
                }
            }
        }
    }
    return true;
}



