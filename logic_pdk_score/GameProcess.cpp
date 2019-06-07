
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
#include "../logic_ddz_score/MSG_DDZ.h"

//打印日志
#define		PRINT_LOG_INFO						1
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//#define IDI_CHECK_TABLE			100					//检查桌子
//#define IDI_CHECK_TRUSTEE			101					//断线托管

//#define IDI_AUTO_READY				200					//自动准备

#define IDI_OUT_CARD				500					//出牌
#define IDI_TICK_USER				600					//剔除用户
#define IDI_CHECK_END				700					//检测游戏结束

//#define	TIME_AUTO_READY				16					//自动准备时间(15)
#define TIME_TICK_USER				1					//剔除用户时间
#define TIME_CHECK_END				600					//检测桌子结束
#define TIME_TRUSTEE				1					//检测桌子结束
//网络补偿时间
#define TIME_NETWORK_COMPENSATE     1					//网络补偿

//分值倍数
#define SCORE_TIME_BOMB				5					//炸弹倍数
#define SCORE_TIME_CHUANTIAN		2					//春天倍数
#define SCORE_TIME_FANCHUAN			2					//反春倍数

#define TIME_OUT_TRUSTEE_COUNT		0					//超时进入托管次数

////////////////////////////////////////////////////////////////////////
CGameProcess::CGameProcess(void) 
	:m_dwlCellScore(1), m_pGameDeskPtr(NULL), m_pGameRoomKindInfo(NULL)
{
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

	//记录开始游戏每个人的分数
    for (int k = 0; k < GAME_PLAYER; ++k) {
        m_RecordScoreInfo[k].llSourceScore = m_pGameDeskPtr->GetUserScore(k);
    }

    //double dScore = m_pGameDeskPtr->GetUserScore(0)*TO_DOUBLE;
    //LOG(INFO) << "Startmoney_1: "<< m_pGameDeskPtr->GetUserScore(0) <<", 2: "<<m_pGameDeskPtr->GetUserScore(1) << ", 3: " << m_pGameDeskPtr->GetUserScore(2);
    //LOG(INFO) << "Startmoney_1: "<< dScore <<", 2: "<<(m_pGameDeskPtr->GetUserScore(1)*TO_DOUBLE) << ", 3: " << (m_pGameDeskPtr->GetUserScore(2)*TO_DOUBLE);
    //设置游戏状态
	m_cbGameStatus = GAME_SCENE_PLAY;
	m_pGameDeskPtr->SetGameTimer(IDI_CHECK_END, (TIME_CHECK_END)*1000);

	//混乱扑克m_cbBombCount
	CT_BYTE cbRandCard[FULL_COUNT];
	if (m_pGameRoomKindInfo->wRoomKindID == PRIMARY_ROOM)//初级房间%70几率出现炸弹
	{
		/*CT_DWORD dwFirstUserID = m_pGameDeskPtr->GetUserID(0);
		srand((CT_DWORD)time(NULL) + dwFirstUserID);
		int nRand = rand() % 10;
		CT_BYTE cbBomCount = 0;
		if (nRand < 7)
		{
			cbBomCount = 1;
		}*/

		m_GameLogic.RandCardData(cbRandCard, CountArray(cbRandCard));
	
	}
	else
	{
		m_GameLogic.RandCardData(cbRandCard, CountArray(cbRandCard));
	}

	if (FREE_ROOM == m_pGameRoomKindInfo->wRoomKindID)
	{
		MatchGoodCard(cbRandCard, CountArray(cbRandCard));
	}

	/*
	cbRandCard[0] = 0x21;
    cbRandCard[1] = 0x11;
    cbRandCard[2] = 0x01;
    cbRandCard[3] = 0x2d;
    cbRandCard[4] = 0x1c;
    cbRandCard[5] = 0x3b;
    cbRandCard[6] = 0x0b;
    cbRandCard[7] = 0x29;
    cbRandCard[8] = 0x38;
    cbRandCard[9] = 0x28;
    cbRandCard[10] = 0x18;
    cbRandCard[11] = 0x08;
    cbRandCard[12] = 0x27;
    cbRandCard[13] = 0x17;
    cbRandCard[14] = 0x07;
    cbRandCard[15] = 0x04;

    cbRandCard[16] = 0x1d;
    cbRandCard[17] = 0x2c;
    cbRandCard[18] = 0x0c;
    cbRandCard[19] = 0x1b;
    cbRandCard[20] = 0x1a;
    cbRandCard[21] = 0x0a;
    cbRandCard[22] = 0x09;
    cbRandCard[23] = 0x36;
    cbRandCard[24] = 0x26;
    cbRandCard[25] = 0x06;
    cbRandCard[26] = 0x35;
    cbRandCard[27] = 0x25;
    cbRandCard[28] = 0x05;
    cbRandCard[29] = 0x23;
    cbRandCard[30] = 0x13;
    cbRandCard[31] = 0x03;

    cbRandCard[32] = 0x32;
    cbRandCard[33] = 0x3d;
    cbRandCard[34] = 0x0d;
    cbRandCard[35] = 0x3c;
    cbRandCard[36] = 0x2b;
    cbRandCard[37] = 0x3a;
    cbRandCard[38] = 0x2a;
    cbRandCard[39] = 0x39;
    cbRandCard[40] = 0x19;
    cbRandCard[41] = 0x37;
    cbRandCard[42] = 0x16;
    cbRandCard[43] = 0x15;
    cbRandCard[44] = 0x34;
    cbRandCard[45] = 0x24;
    cbRandCard[46] = 0x14;
    cbRandCard[47] = 0x33;
	*/
	//用户扑克
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
		m_cbHandCardCount[i] = NORMAL_COUNT;
		memcpy(&m_cbHandCardData[i], &cbRandCard[i*m_cbHandCardCount[i]], sizeof(CT_BYTE)*m_cbHandCardCount[i]);
        for (int j = 0; (j < NORMAL_COUNT)&&(INVALID_CHAIR == m_dwCurrentUser); ++j)
        {
            if (0x23==m_cbHandCardData[i][j])
            {
                m_dwCurrentUser = i;
            }
        }
	}
	if (m_dwCurrentUser >= GAME_PLAYER)
    {
		LOG(ERROR) <<"deal error";
        return;
    }
	m_dwBankerUser = m_dwCurrentUser;
	//构造变量
	CMD_S_GameStart GameStart;
	memset(&GameStart,0, sizeof(GameStart));
	GameStart.dwCurrentUser = m_dwCurrentUser;
	GameStart.cbTimeLeft = TIME_DEAL_CARD+TIME_FIRST_OUT_CARD;
	
	//游戏记录的空闲场景构造
    CMD_S_StatusFree StatusFree;
    memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
    StatusFree.dCellScore = m_dwlCellScore*TO_DOUBLE;
    m_pGameDeskPtr->WriteGameSceneToRecord(&StatusFree, sizeof(StatusFree), SC_GAMESCENE_FREE);
    
	bool bHasAndroid = false;
	//发送数据
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
        GameStart.dwStartUser = i;
		//构造扑克
	//	assert(CountArray(GameStart.cbCardData) >= m_cbHandCardCount[i]);
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
		m_GameLogic.SortCardList(m_cbHandCardData[i], m_cbHandCardCount[i], ST_DESC);

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

		GameStartAi.dwCurrentUser = m_dwCurrentUser;

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
    m_cbOpTotalTime = GameStart.cbTimeLeft + TIME_NETWORK_COMPENSATE;
    m_dwOpStartTime = (CT_DWORD)time(NULL);
	//托管
	IsTrustee(m_cbOpTotalTime);
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

	//当前时间
	std::string strTime = Utility::GetTimeNowString();

	//游戏结束
	m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());

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
            StatusFree.dCellScore = m_dwlCellScore*TO_DOUBLE;
			StatusFree.dwCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

			//发送数据
			SendTableData(dwChairID, SC_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree),false);
			break;
		}
		case GAME_SCENE_PLAY:	//游戏状态
		{
			//构造数据
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay, 0, sizeof(StatusPlay));
			//单元积分
			StatusPlay.dCellScore = m_dwlCellScore*TO_DOUBLE;
			StatusPlay.wFirstOutChairID = (CT_WORD)m_dwBankerUser;
			//剩余时间 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusPlay.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//出牌信息
			StatusPlay.wTurnWiner = m_dwTurnWiner;
			StatusPlay.cbTurnCardCount = m_cbTurnCardCount;
			memcpy(StatusPlay.cbTurnCardData, m_cbTurnCardData, m_cbTurnCardCount * sizeof(CT_BYTE));

			//游戏变量
			StatusPlay.dwCurrentUser = m_dwCurrentUser;



			//扑克信息
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
			StatusEnd.dCellScore = m_dwlCellScore*TO_DOUBLE;
			//StatusEnd.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
			//剩余时间
			CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
			StatusEnd.cbTimeLeave = (CT_BYTE)(m_cbOpTotalTime - std::min<CT_BYTE>(dwPassTime, m_cbOpTotalTime));
			//扑克信息
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
		LOG(ERROR) << "Timer triggers the end of the game,";
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


	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return;
	}
    CT_DWORD dwTime = time(NULL);
	//叫地主状态不主动托管，游戏状态超时1次托管
	if (m_cbGameStatus == GAME_SCENE_PLAY &&
		m_cbTrustee[m_dwCurrentUser] != 1
		&& (!m_bIsPassCard) && ((m_cbOpTotalTime-1)<= (int)(dwTime-m_dwOpStartTime)))
	{
		SetTrustee(m_dwCurrentUser, 1);
        //LOG(ERROR) << "IDI_OUT_CARD SetTrustee" ;
    }
    //LOG(WARNING) << "IDI_OUT_CARD " << (int)m_cbOpTotalTime << " " << (itime-m_dwOpStartTime)<< " m_cbTrustee[m_dwCurrentUser] " << (int)m_cbTrustee[m_dwCurrentUser] << " m_bIsPassCard " << m_bIsPassCard;
	CT_BOOL bSystemSucceed = true;
	switch (dwTimerID)
	{
	case IDI_OUT_CARD:
	{
        ++m_cbTimeOutCount[m_dwCurrentUser];
		//LOG(WARNING) << "ON IDI_OUT_CARD TIMER";
        if(m_cbTrustee[m_dwCurrentUser] || (m_bIsPassCard))
        {
            bSystemSucceed = OnAutoOutCard();
        } else{
            //LOG(ERROR) << "IDI_OUT_CARD " << (int)m_cbOpTotalTime << " "<< (itime-m_dwOpStartTime);
            bSystemSucceed = false;
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
    bool bRet = true;
	switch (dwSubCmdID)
	{
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
            bRet = OnUserOutCard(wChairID, pOutCard->cbCardData, pOutCard->cbCardCount);
            break;
		}
		case SUB_C_PASS_CARD:	//用户放弃
		{
			//状态效验
			//assert(m_cbGameStatus == GAME_SCENE_PLAY);
			if (m_cbGameStatus != GAME_SCENE_PLAY) return true;

			//消息处理
            bRet = OnUserPassCard(wChairID);
            break;
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
			/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
				SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
			}*/
			SendTableData(INVALID_CHAIR, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
			if (m_dwCurrentUser == wChairID)
			{
				IsTrustee(0);
			}
			//LOG(ERROR) << "SUB_C_TRUSTEE" << " m_cbTrustee["<<(int)pTrustee->wChairID<<"]=" << (int)m_cbTrustee[pTrustee->wChairID];
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
			/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
				SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
			}*/

            SendTableData(INVALID_CHAIR, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
            //LOG(ERROR) << "SUB_C_CANCEL_TRUSTEE" << " m_cbTrustee["<<(int)pCancelTrustee->wChairID<<"]=" << (int)m_cbTrustee[pCancelTrustee->wChairID];
            if(pCancelTrustee->wChairID==m_dwCurrentUser)
            {
                time_t tTime = time(NULL);
                if((m_cbOpTotalTime - (tTime-m_dwOpStartTime)) > 1)
                {
                    m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
                    m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (m_cbOpTotalTime - (tTime-m_dwOpStartTime)) * 1000);
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
	if (!bRet)
    {
        m_pGameDeskPtr->CloseUserConnect(wChairID);
    }
	return bRet;
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
	//随机种子
	srand((unsigned)time(NULL));
	//系统时间
	m_dwSysTime = 0;

	//游戏状态
	m_cbGameStatus = GAME_SCENE_FREE;							
	//首叫用户
/*	m_dwFirstUser = INVALID_CHAIR;*/
	//当前玩家
	m_dwCurrentUser = INVALID_CHAIR;
	m_dwBankerUser = INVALID_CHAIR;
	//炸弹个数
	m_dwBombChairID = INVALID_CHAIR;

	m_cbOpTotalTime = 0;
	m_dwOpStartTime = 0;

	memset(m_cbEachBombCount, 0, sizeof(m_cbEachBombCount));
	memset(m_dEachBombScore, 0, sizeof(m_dEachBombScore));
    memset(m_dEachBombRevenue, 0, sizeof(m_dEachBombRevenue));

	//基础积分
	m_dwlCellScore = 100;
	

	//春天倍数
	//memset(m_dwChunTianbTime, 1, sizeof(m_dwChunTianbTime));
	//出牌数据
	memset(m_cbOutCardCount, 0, sizeof(m_cbOutCardCount));


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
	m_bIsPassCard = false;
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

	PDK_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));

    GameEnd.dCellScore = m_dwlCellScore*TO_DOUBLE;
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		//拷贝扑克
		GameEnd.cbCardCount[i] = m_cbHandCardCount[i];
		memcpy(&GameEnd.cbHandCardData[i], m_cbHandCardData[i], m_cbHandCardCount[i] * sizeof(CT_BYTE));

	}
    memcpy(&GameEnd.cbEachBombCount, m_cbEachBombCount, sizeof(GameEnd.cbEachBombCount));
    memcpy(&GameEnd.dEachBombScore, m_dEachBombScore, sizeof(GameEnd.dEachBombScore));


    CT_LONGLONG llGameScore[GAME_PLAYER] = { 0,0,0 };
	CT_LONGLONG llBankerMaxWinScore = m_pGameDeskPtr->GetUserScore(dwWinChairID);
	for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	{
		//过滤赢家
		if (i == dwWinChairID) continue;

		//输赢分数
		CT_LONGLONG llTempScore = m_cbHandCardCount[i] * (CT_INT32)m_dwlCellScore;

		CT_BYTE cbMaxSelfCard = MAX_COUNT;

		if (cbMaxSelfCard == m_cbHandCardCount[i] && m_cbOutCardCount[i] == 0)
		{	//被关乘2
			//被关
			GameEnd.bChunTian[i] = true;

			//全关分
			llTempScore *= SCORE_TIME_CHUANTIAN;

		}
		else if (i == m_dwBankerUser && 1 == m_cbOutCardCount[i])
        {
            GameEnd.bChunTian[i] = 2;
            //反关
            llTempScore *= SCORE_TIME_CHUANTIAN;
        }
        llGameScore[i] -= llTempScore;
        llGameScore[dwWinChairID] += llTempScore;
	}
	//判断是否达到赢分最大值,则退还多出的金额
	CT_LONGLONG llReturnBackPartScore[GAME_PLAYER] = {0,0,0};
	//赢的总反>入场时的前
	if ((llGameScore[dwWinChairID] + m_dEachBombScore[dwWinChairID]) > (llBankerMaxWinScore - m_dEachBombScore[dwWinChairID]))
	{
        GameEnd.bWinMaxScore = true;
		CT_LONGLONG llTempTotal = llGameScore[dwWinChairID];
        CT_LONGLONG llReturnBackScore = (llGameScore[dwWinChairID]+ m_dEachBombScore[dwWinChairID]) - (llBankerMaxWinScore - m_dEachBombScore[dwWinChairID]);
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            if (i == dwWinChairID) continue;
            if ((llGameScore[i]+m_dEachBombScore[i]) < 0)
            {
                llReturnBackPartScore[i] = ((CT_DOUBLE)((CT_DOUBLE)(llGameScore[i]*(-1))/llTempTotal)*llReturnBackScore);
            }
        }
	}

	//金币校正(退还金额和保证输家够扣）
	for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	{
		if ((i != dwWinChairID) && (llReturnBackPartScore[i] > 0))
		{
			//需要退还金币
            llGameScore[i] += llReturnBackPartScore[i];
            llGameScore[dwWinChairID] -= llReturnBackPartScore[i];
		}
	}

	for (CT_DWORD k = 0; k < GAME_PLAYER; ++k)
	{
		if (k == dwWinChairID) continue;
		//保证输家够扣
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(k);
		if (llGameScore[k] + llUserScore < 0)
		{
            llGameScore[dwWinChairID] += (llGameScore[k] + llUserScore);
            llGameScore[k] = (-llUserScore);
		}
		GameEnd.bLose[k] = GAME_LOSE;
	}
	GameEnd.bLose[dwWinChairID] = GAME_WIN;
    //LOG(INFO) << "GameEnd:beformoney_1: "<< m_pGameDeskPtr->GetUserScore(0) <<", 2: "<<m_pGameDeskPtr->GetUserScore(1) << ", 3: " << m_pGameDeskPtr->GetUserScore(2);
    //LOG(INFO) << "GameEnd:beformoney_1: "<< (m_pGameDeskPtr->GetUserScore(0)*TO_DOUBLE) <<", 2: "<<(m_pGameDeskPtr->GetUserScore(1)*TO_DOUBLE) << ", 3: " << (m_pGameDeskPtr->GetUserScore(2)*TO_DOUBLE);
    //写入积分
	for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	{
		m_RecordScoreInfo[i].dwUserID = m_pGameDeskPtr->GetUserID(i);
		m_RecordScoreInfo[i].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(i);
		m_RecordScoreInfo[i].cbStatus = 1;

		CT_INT32 iRevenue = 0;
		m_RecordScoreInfo[i].iScore = llGameScore[i];
		//计算税收
		if (llGameScore[i] > 0)
		{
			iRevenue = (CT_INT32)m_pGameDeskPtr->CalculateRevenue(i, llGameScore[i]);
            llGameScore[i] -= iRevenue;
			m_RecordScoreInfo[i].dwRevenue = iRevenue;
		}
		ScoreInfo  ScoreData;
		memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = i;
		ScoreData.bBroadcast = true;
		ScoreData.llScore = llGameScore[i];
		ScoreData.llRealScore= llGameScore[i];
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);
        m_RecordScoreInfo[i].iScore += (m_dEachBombScore[i]+m_dEachBombRevenue[i]);
        m_RecordScoreInfo[i].dwRevenue += m_dEachBombRevenue[i];
	}

	//把各个玩家的分数转为double,并将所有玩家取消托管
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		GameEnd.dGameScore[i] = llGameScore[i] * TO_DOUBLE;
        GameEnd.dEachBombScore[i] *= TO_DOUBLE;
	}
	SendTableData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameEnd, sizeof(GameEnd), true);
    //LOG(INFO) << "START****************GameEnd*******************";
    //LOG(INFO) << "GameEnd:Endmoney_1: "<< m_pGameDeskPtr->GetUserScore(0) <<", 2: "<<m_pGameDeskPtr->GetUserScore(1) << ", 3: " << m_pGameDeskPtr->GetUserScore(2);
    //LOG(INFO) << "GameEnd:Endmoney_1: "<< (m_pGameDeskPtr->GetUserScore(0)*TO_DOUBLE) <<", 2: "<<(m_pGameDeskPtr->GetUserScore(1)*TO_DOUBLE) << ", 3: " << (m_pGameDeskPtr->GetUserScore(2)*TO_DOUBLE);
    //LOG(INFO) << "GameEnd:GameScore_1: "<< GameEnd.dGameScore[0] <<", 2: "<<GameEnd.dGameScore[1] << ", 3: " << GameEnd.dGameScore[2];
    //LOG(INFO) << "GameEnd:GameScore_1: "<< llGameScore[0] <<", 2: "<<llGameScore[1] << ", 3: " << llGameScore[2];
    //LOG(INFO) << "GameEnd:BombScore_1: "<< GameEnd.dEachBombScore[0] <<", 2: "<<GameEnd.dEachBombScore[1] << ", 3:" << GameEnd.dEachBombScore[2];
    //LOG(INFO) << "GameEnd:BombScore_1: "<< m_dEachBombScore[0] <<", 2: "<<m_dEachBombScore[1] << ", 3:" << m_dEachBombScore[2];
    //LOG(INFO) << "GameEnd:BombCount_1: "<< (int)GameEnd.cbEachBombCount[0] <<", 2: "<<(int)GameEnd.cbEachBombCount[1] << ", 3: " << (int)GameEnd.cbEachBombCount[2];
    //LOG(INFO) << "GameEnd:TotalScore_1: "<< GameEnd.dGameScore[0] + GameEnd.dEachBombScore[0]<<" ,2: "<<GameEnd.dGameScore[1] + GameEnd.dEachBombScore[1]<< ", 3: " << GameEnd.dGameScore[2]+ GameEnd.dEachBombScore[2];
    //LOG(INFO) << "END****************GameEnd*******************";
	//发送数据
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		if (m_cbTrustee[i] == 1)
		{
			SetTrustee(i, 0);
		}
	}
    
   /* CT_DWORD dwBankerUserID = 0;
    if (dwWinChairID != INVALID_CHAIR)
    {
        dwBankerUserID = m_pGameDeskPtr->GetUserID(dwWinChairID);
    }*/
	
	//记录基本信息
	m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, NULL, 0, 0, 0, 0, 0, 0, 0);

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
	//CT_BYTE cbBombCount = (CT_BYTE) *nodeCond->get_int64();

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
	/*if (m_pGameDeskPtr->IsAndroidUser(wChairID))
	{
		SetTrustee(wChairID, 0);
	}*/
	CMD_S_OutCard OutCard;
	memset(&OutCard, 0, sizeof(OutCard));
	if (cbOutCount <= 0)
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);
		OutCard.cbIsFail = 1;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		return false;
	}

	//判断出牌是否合法
	if (cbOutCount > m_cbHandCardCount[wChairID])
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);

		LOG(WARNING) << "out card count more than hand card count :"   << (int)cbOutCount <<" m_cbHandCardCount:" <<(int)m_cbHandCardCount[wChairID];
		OutCard.cbIsFail = 2;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		return false;
	}

	for (CT_BYTE i = 0; i < cbOutCount; ++i)
	{
		if (cbOutCard[i] == 0)
		{
			if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
				m_pGameDeskPtr->CloseUserConnect(wChairID);

			LOG(WARNING) << "out card data is empty? ";
			OutCard.cbIsFail = 3;//
			SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
			return false;
		}
	}

	/*if (cbOutCard[0] == 0)
	{
		LOG(WARNING) << "card data is empty!";
		return false;
	}*/

	CT_BYTE cbOutCardTemp[MAX_COUNT];
	memcpy(cbOutCardTemp, cbOutCard, cbOutCount);

	m_GameLogic.SortCardList(cbOutCard, cbOutCount, ST_DESC);

	if (wChairID != m_dwCurrentUser) 
	{
		OutCard.cbIsFail = 4;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		m_pGameDeskPtr->CloseUserConnect(wChairID);
		LOG(WARNING) << "OnUserOutCard not this user!";
		return false;
	}

	//获取类型
	CT_BYTE cbCardType = m_GameLogic.GetCardType(cbOutCard, cbOutCount);

	//类型判断
	if (cbCardType == CT_ERROR)
	{
		//assert(FALSE);
		LOG(ERROR) << "user out card, card type error, card data: " ;
		for (CT_BYTE i = 0;  i != cbOutCount; ++i)
		{
			LOG(ERROR) << std::hex << (int)cbOutCard[i];
		}
		OutCard.cbIsFail = 5;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		m_pGameDeskPtr->CloseUserConnect(wChairID);
		return false;
	}

	//出牌判断
	if (m_cbTurnCardCount != 0)
	{
		//对比扑克
		if (m_GameLogic.CompareCard(m_cbTurnCardData, m_cbTurnCardCount, cbOutCard, cbOutCount) == false)
		{
			//assert(FALSE);
			LOG(WARNING) << "compare card error.";
			OutCard.cbIsFail = 6;//
			SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
			return false;
		}
	}

    CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
    if ((1 == cbOutCount) && (m_cbHandCardCount[m_dwCurrentUser] > 1)&& (m_cbHandCardCount[dwNextUser] == 1))
    {
        //找最大单张
        CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
        if (m_GameLogic.GetLogicValue(cbMaxSingle) > m_GameLogic.GetLogicValue(cbOutCard[0]))
        {
            OutCard.cbIsFail = 8;//
            SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
            return false;
        }
    }

	//删除扑克
    if (m_GameLogic.RemoveCardList(cbOutCard, cbOutCount, m_cbHandCardData[wChairID], m_cbHandCardCount[wChairID]) == false)
    {
        //assert(FALSE);
        LOG(WARNING) << "remove card error.";
        OutCard.cbIsFail = 7;//
        SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
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


	OutCard.dwOutCardUser = wChairID;
	OutCard.cbCardCount = cbOutCount;
	OutCard.dwCurrentUser = m_dwCurrentUser;
	OutCard.cbTimeLeft = TIME_OUT_CARD;
	m_bIsPassCard = false;
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

	if (INVALID_CHAIR != m_dwCurrentUser)
    {
        tagSearchCardResult SearchCardResult;
        memset(&SearchCardResult, 0, sizeof(SearchCardResult));
        CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
        if (cbResultCount <= 0)
        {//不可以出牌
            OutCard.cbTimeLeft = TIME_NOT_OUT_CARD;
			m_bIsPassCard = true;
        }
    }

	//发送数据
	CT_DWORD wHeadSize = sizeof(OutCard) - sizeof(OutCard.cbCardData);
	CT_DWORD wSendSize = wHeadSize + OutCard.cbCardCount * sizeof(CT_BYTE);
	//for (CT_BYTE i = 0;i < GAME_PLAYER;i++)
	//{
		SendTableData(INVALID_CHAIR, SUB_S_OUT_CARD, &OutCard, wSendSize, true);
	//}

	//炸弹判断
	if (cbCardType == CT_BOMB_CARD)
	{
		m_dwBombChairID = wChairID;
	}
	//出牌最大
	//if (cbCardType == CT_MISSILE_CARD) m_cbTurnCardCount = 0;

	//结束判断
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
	    if(INVALID_CHAIR != m_dwBombChairID)
        {
            CalculateBombScore();
        }
		OnEventGameEnd(wChairID, GER_NORMAL);
	}
	else
	{
        m_cbOpTotalTime = OutCard.cbTimeLeft + TIME_NETWORK_COMPENSATE;
        m_dwOpStartTime = (CT_DWORD)time(NULL);
		//进入托管流程
		IsTrustee(m_cbOpTotalTime);
	}
	
	return true;
}

//玩家不出
CT_BOOL CGameProcess::OnUserPassCard(CT_WORD wChairID)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard :wChairID = " << wChairID ;
	//效验状态
	//assert((wChairID == m_dwCurrentUser) && (m_cbTurnCardCount != 0));
	if ((wChairID != m_dwCurrentUser) || (m_cbTurnCardCount == 0)) return false;

	//构造消息
	CMD_S_PassCard PassCard;
	memset(&PassCard,0,sizeof(PassCard));
	tagSearchCardResult SearchCardResult;
	memset(&SearchCardResult, 0, sizeof(SearchCardResult));
	CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbHandCardData[wChairID], m_cbHandCardCount[wChairID], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
	if (cbResultCount > 0)
	{//有可以出的牌
		PassCard.cbIsFail = true;
		LOG(ERROR)<<"OnUserPassCard FAIL";
		SendTableData(wChairID, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), false);
		return false;
	}
    PassCard.cbTurnOver = 0;

	//设置变量
	m_dwCurrentUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
	if (m_dwCurrentUser == m_dwTurnWiner)
	{
        memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
        m_cbTurnCardCount = 0;
        PassCard.cbTurnOver = 1;
    }

	PassCard.dwPassCardUser = wChairID;
	PassCard.dwCurrentUser = m_dwCurrentUser;

	PassCard.cbTimeLeft = TIME_OUT_CARD;
	m_bIsPassCard = false;
	//PassCard.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();

	if (!PassCard.cbTurnOver)
    {
        memset(&SearchCardResult, 0, sizeof(SearchCardResult));
        cbResultCount = m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
        if (cbResultCount <= 0)
        {//要不起玩家时间
            PassCard.cbTimeLeft = TIME_NOT_OUT_CARD;
			m_bIsPassCard = true;
        }
    }

	//发送数据
	/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), true);
        }
        else
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), false);
        }
	}*/
    SendTableData(INVALID_CHAIR, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), true);

    CalculateBombScore();
    m_cbOpTotalTime = PassCard.cbTimeLeft + TIME_NETWORK_COMPENSATE;
    m_dwOpStartTime = (CT_DWORD)time(NULL);
	//托管
	IsTrustee(m_cbOpTotalTime);
	return true;
}

//获得下一个玩家
CT_DWORD CGameProcess::GetNextUser(CT_DWORD dwChairID)
{
	return (dwChairID + 1) % GAME_PLAYER;
}

CT_BOOL CGameProcess::IsTrustee(CT_DWORD dwTime)
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
	case GAME_SCENE_PLAY:
	{
	    if (dwTime)
		{
			m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
		}

		if ((m_cbOpTotalTime-1)<=(time(NULL)-m_dwOpStartTime))
        {
            m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
            m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, ((TIME_NETWORK_COMPENSATE) * 1000));
        }
		else if (m_cbTrustee[m_dwCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, ((TIME_TRUSTEE > dwTime && dwTime != 0) ? dwTime: TIME_TRUSTEE)*1000 + 500);
		}
		else
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (dwTime * 1000));
			//LOG(WARNING) << "SET IDI_OUT_CARD TIMER m_dwCurrentUser: "<< m_dwCurrentUser;
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
	/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
        }
        else
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
        }
	}*/
    SendTableData(INVALID_CHAIR, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
    //LOG(ERROR) << "SendTableData";
}
//配好牌
CT_BOOL CGameProcess::MatchGoodCard(CT_BYTE cbRandCard[], CT_BYTE cbRandCount)
{
	CT_BOOL bConfigCard[GAME_PLAYER] = {false};
	CT_BYTE cbConfigCount = 0;

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if ((!m_pGameDeskPtr->IsAndroidUser(i)) && (m_pGameDeskPtr->GetUserBaseData(i)) && (m_pGameDeskPtr->GetUserBaseData(i)->dwTodayPlayCount < 3))
		{
			bConfigCard[i] = true;
			++cbConfigCount;
		}
	}

	if(cbConfigCount > 3)
    {
	    LOG(ERROR) << "cbConfigCount > 3 error!";
        return false;
    }

	//只有单元积分等于1的场次才会配好牌
	if (cbConfigCount > 0)
	{
		CT_BYTE cbTempCard[FULL_COUNT]={0};
		memcpy(cbTempCard, cbRandCard,CountArray(cbTempCard));
		memset(cbRandCard,0,cbRandCount);
		CT_BYTE cbGoodCard[GOOD_CARD_COUTN];
		memset(cbGoodCard,0,CountArray(cbGoodCard));
		m_GameLogic.GetGoodCardData(cbGoodCard, GOOD_CARD_COUTN);
		//抽取好牌
		m_GameLogic.RemoveGoodCardData(cbGoodCard, GOOD_CARD_COUTN, cbTempCard, FULL_COUNT);
		int iGoodIndex = 0;
		int iBadIndex = 0;
		CT_BYTE cbGoodCount = (cbConfigCount==2) ? (GOOD_CARD_MAX-1): GOOD_CARD_MAX;

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			int iNeedGoodCount = 0;
			//三个都配牌
			if(3 == cbConfigCount)
			{
                iNeedGoodCount = cbGoodCount-2;
			}
			else if (bConfigCard[i])
			{
				//分发扑克
				iNeedGoodCount = cbGoodCount;

			}
			else if (2 == cbConfigCount)
			{
			    //一个不配牌
				iNeedGoodCount = (GOOD_CARD_COUTN-2*cbGoodCount);
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
	}

	return true;
}

CT_BOOL CGameProcess::OnAutoOutCard()
{
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return true;
	}

    CT_BYTE cbCardData[MAX_COUNT] = { 0 };
    CT_BYTE cbCardCount = 0;

    CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
    if (1 == m_cbTurnCardCount && m_cbHandCardCount[dwNextUser] == 1)
    {
        //找最大单张
        CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
        if (cbMaxSingle>0)
        {
            cbCardData[0] = cbMaxSingle;
            cbCardCount = 1;
            return OnUserOutCard(m_dwCurrentUser, cbCardData, cbCardCount);
        }
    }
    else
	/*if (0 == m_cbTurnCardCount)
	{//第一个出牌
		CT_BYTE cbCardCount = m_cbHandCardCount[m_dwCurrentUser];
		m_GameLogic.SearchHeadOutCard();
		CT_BYTE cbCard[MAX_COUNT] = { 0 };
		cbCard[0] = m_cbHandCardData[m_dwCurrentUser][cbCardCount - 1];
		return OnUserOutCard(m_dwCurrentUser, cbCard, 1) ;
	}
	else*/
	{
		//跟牌
		//return OnUserPassCard(m_dwCurrentUser) ;

		//检测炸弹直接过牌
		/*if (m_cbTurnCardCount == 2)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			if (cbCardType == CT_MISSILE_CARD)
			{
				return OnUserPassCard(m_dwCurrentUser);
			}
		}*/

		//搜索提示
		tagSearchCardResult	SearchCardResult;					//搜索结果
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		try
		{
			m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount,
				SearchCardResult);
		}
		catch (...)
		{
			SearchCardResult.cbSearchCount = 0;
		}

		if (SearchCardResult.cbSearchCount > 0)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			CT_BYTE cbBestCardIndex = 0;
            CT_BYTE cbBombCount = 0; //拆炸弹牌的数量
            CT_BYTE cbBombDismantCount = 0; //拆炸弹牌的数量
            for (int k = 0; k < SearchCardResult.cbSearchCount; ++k)
            {
                CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[k], SearchCardResult.cbCardCount[k]);
                if (cbSearchCardType == CT_BOMB_CARD) ++cbBombCount;
            }

			for (CT_BYTE i = 0 ; i != SearchCardResult.cbSearchCount; ++i)
			{
				CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[i], SearchCardResult.cbCardCount[i]);
				if (cbCardType != CT_ERROR)
				{

					//相同牌型，直接出牌
					if (cbSearchCardType == cbCardType || ((cbSearchCardType == CT_BOMB_CARD) && (SearchCardResult.cbSearchCount != 1)&&((SearchCardResult.cbSearchCount - cbBombCount) == cbBombDismantCount)))
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
                                            ++cbBombDismantCount;
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
			cbCardCount = SearchCardResult.cbCardCount[cbBestCardIndex];
			memcpy(cbCardData, &SearchCardResult.cbResultCard[cbBestCardIndex], sizeof(CT_BYTE)*cbCardCount);
            if (m_cbHandCardCount[dwNextUser] == 1 && (1 == cbCardCount))
            {
                //找最大单张
                CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
                if (cbMaxSingle>0)
                {
                    cbCardData[0] = cbMaxSingle;
                    cbCardCount = 1;
                }
            }
            //当手牌数量2且当前出的是单牌，判断自己手里的最大的牌是否是所有牌中最大的，如果是先出此牌
            else if((2==m_cbHandCardCount[m_dwCurrentUser]) && (1 == cbCardCount))
            {
                //找最大单张
                CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
                if (cbMaxSingle>0)
                {
                    CT_BYTE cbTempCardData[MAX_COUNT] = { 0 };
                    CT_BYTE cbCount = 0;
                    cbTempCardData[0] = cbMaxSingle;
                    for (CT_DWORD k = 0; k < GAME_PLAYER; ++k)
                    {
                        if (m_dwCurrentUser == k) continue;
                        cbCount += m_GameLogic.SearchMaxOneCard(m_cbHandCardData[k], m_cbHandCardCount[k],cbTempCardData);
                    }
                    if (0 == cbCount)
                    {
                        cbCardData[0] = cbMaxSingle;
                        cbCardCount = 1;
                    }
                }

            }
            else if ((CT_DOUBLE_Ex == m_GameLogic.GetCardType(cbCardData,cbCardCount))
                     && (m_cbHandCardCount[dwNextUser] == 2)
                     && (CT_DOUBLE_Ex == m_GameLogic.GetCardType(m_cbHandCardData[dwNextUser],m_cbHandCardCount[dwNextUser])))
            {
                CT_BYTE cbResultCard[MAX_COUNT] = { 0 };
                //找最大对子
                CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxDouble(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0], cbResultCard);
                if ((cbMaxSingle>0) && (m_GameLogic.GetLogicValue(cbCardData[0]) != m_GameLogic.GetLogicValue(cbResultCard[0])))
                {

                    memcpy(cbCardData,cbResultCard,2* sizeof(CT_BYTE));
                    cbCardCount = 2;
                }
            }
			//排列扑克
			m_GameLogic.SortCardList(cbCardData, cbCardCount, ST_DESC);
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
    return OnUserPassCard(m_dwCurrentUser);
}
CT_VOID CGameProcess::CalculateBombScore()
{
    if (((m_dwBombChairID != INVALID_CHAIR) && (m_dwTurnWiner == m_dwBombChairID) && (m_dwCurrentUser == m_dwBombChairID)) || ((m_dwBombChairID != INVALID_CHAIR) && (0 == m_cbHandCardCount[m_dwBombChairID])))
    {
        //炸弹结算
        m_cbEachBombCount[m_dwBombChairID]++;
        CMD_S_BombScore stBombScore;
        memset(&stBombScore,0, sizeof(stBombScore));

        CT_LONGLONG llTempScore[GAME_PLAYER] = {0,0,0};
        //计算炸弹得分
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            if (i != m_dwBombChairID)
            {
                CT_LONGLONG llTempScore1 = (SCORE_TIME_BOMB*m_dwlCellScore);
                llTempScore[i] -= llTempScore1;
                llTempScore[m_dwBombChairID] += llTempScore1;
            }
        }

        CT_LONGLONG llBankerMaxWinScore = m_pGameDeskPtr->GetUserScore(m_dwBombChairID);
        //判断是否达到赢分最大值,则退还多出的金额
        CT_INT32 llReturnBackPartScore[GAME_PLAYER] = {0,0,0};
        //赢的总额>入场时的钱
        if (llTempScore[m_dwBombChairID] > (llBankerMaxWinScore - m_dEachBombScore[m_dwBombChairID]))
        {
            //计算发现金额
            CT_LONGLONG llTempTotal = llTempScore[m_dwBombChairID]+ m_dEachBombScore[m_dwBombChairID]+m_dEachBombRevenue[m_dwBombChairID];
            CT_LONGLONG llReturnBackScore = (llTempScore[m_dwBombChairID]+ m_dEachBombScore[m_dwBombChairID]) - (llBankerMaxWinScore - m_dEachBombScore[m_dwBombChairID]);
            for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
            {
                if (i == m_dwBombChairID) continue;
                if ((llTempScore[i]+m_dEachBombScore[i]) < 0)
                {
                    llReturnBackPartScore[i] = ((CT_DOUBLE)((CT_DOUBLE)((llTempScore[i]+m_dEachBombScore[i])*(-1))/llTempTotal)*llReturnBackScore);
                }
            }

        }
        //金币校正(退还金额）
        for (CT_DWORD i = 0; (i < GAME_PLAYER) ; ++i)
        {
            if ((i != m_dwBombChairID) && (llReturnBackPartScore[i] > 0))
            {
                //需要退还金币
                llTempScore[i] += llReturnBackPartScore[i];
                llTempScore[m_dwBombChairID] -= llReturnBackPartScore[i];
            }
        }

        //判断输家是否够扣（保证输家够扣）
        for (CT_DWORD k = 0; k < GAME_PLAYER; ++k)
        {
            if (k == m_dwBombChairID) continue;
            //保证闲家够扣
            CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(k);
            if (llTempScore[k] + llUserScore < 0)
            {
                llTempScore[m_dwBombChairID] += (llTempScore[k] + llUserScore);
                llTempScore[k] = (-llUserScore);
            }
            stBombScore.dBombScore[k] = llTempScore[k]*TO_DOUBLE;
            m_pGameDeskPtr->AddUserScore(k,llTempScore[k], true);
            m_dEachBombScore[k] += llTempScore[k];
        }

        CT_INT32 iRevenue = (CT_INT32)m_pGameDeskPtr->CalculateRevenue(m_dwBombChairID, llTempScore[m_dwBombChairID]);
        m_dEachBombRevenue[m_dwBombChairID] += iRevenue;
        m_dEachBombScore[m_dwBombChairID] += (llTempScore[m_dwBombChairID]-iRevenue);

        stBombScore.dBombScore[m_dwBombChairID] = (llTempScore[m_dwBombChairID]-iRevenue)*TO_DOUBLE;

        m_pGameDeskPtr->AddUserScore(m_dwBombChairID,(llTempScore[m_dwBombChairID]-iRevenue),true);
        //LOG(INFO) << "BombScore: " << (llTempScore[m_dwBombChairID]-iRevenue);
        m_dwBombChairID = INVALID_CHAIR;
        SendTableData(INVALID_CHAIR, SUB_S_BOMB_SCORE, &stBombScore, sizeof(stBombScore), true);
    }
}


