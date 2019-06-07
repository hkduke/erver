
#include "stdafx.h"
#include <stdio.h>
#include "GameProcess.h"
#include "DuoCaiDuoFuCfg.h"
#include "ITableFrame.h"
#include <time.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "GlobalEnum.h"
#include <cmath>
#include <iostream>

#define IDI_SAVE_STOCK				100					//保存血池值
#define TIME_SAVE_STOCK 			30*1000				//保存血池值时间

#define IDI_LET_AI_LEFT				101					//让AI离开桌子
#define TIME_LET_AI_LEFT 			60*1000				//让AI离开桌子时间

#define IDI_SELECT_FREE_GAME		102					//选择免费游戏超时
#define TIME_SELECT_FREE_GAME 		2*1000				//选择免费游戏超时时间
#define TIME_FREE_GAME 		        (10)				//选择免费游戏超时时间
/*CT_DWORD CGameProcess::m_dwJetton[JETTON_COUNT] = { 50, 100, 200, 300, 400, 500, 600, 700, 800, 900,
													1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
													9000, 10000, 50000, 100000, 150000, 200000, 300000};*/

//CT_LONGLONG CGameProcess::m_llStock = 0;
//CT_LONGLONG CGameProcess::m_llTodayStock = 0;
//CT_DWORD    CGameProcess::m_dwTodayTime = (CT_DWORD)time(NULL);

tagStockControlInfo CGameProcess::m_stockControlInfo[ROOM_NUM];
CT_LONGLONG CGameProcess::m_llStockTaxTemp[ROOM_NUM] = {0, 0, 0, 0};
//CGameProcess::en_StockStatus CGameProcess::m_enStockStatus = CGameProcess::en_StockStatus::en_EatScore;
int CGameProcess::m_nStockCheatVal[ROOM_NUM] = {10, 10, 10, 10};
std::map<CT_DWORD, CGameProcess::tagDianKongData> CGameProcess::m_mapUserPointControl;

#define CAN_JETTON_SCORE 2000 //至少需要二十块钱才能下注

CGameProcess::CGameProcess(void)
: m_dwCellScore(1)
, m_uLastSpinTime(0)
, m_llUserWinScore(0)
//, m_wUnLuckyCount(0)
//, m_llLastJetton(0)
, m_pGameDeskPtr(NULL)
, m_gen(m_rd())
{
	//清理游戏数据
	//ClearGameData();
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
	//清理游戏数据
	//ClearGameData();
	//初始化数据
	InitGameData();
}

//发送场景
void CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
    switch (m_wGameStatus)
    {
        case GAME_STATUS_FREE:
        case GAME_STATUS_PLAY:
        {
			MSG_GS_FREE freeGameScene;
			//for (int i = 0; i < JETTON_COUNT; ++i)
			//{
				freeGameScene.dUserLeaseScore = CAN_JETTON_SCORE*TO_DOUBLE;
			//}
            std::vector<CDCDFCfg::tagStakeCfg> & stakeCfg= CDCDFCfg::get_instance().m_vStakeCfg;
            for (CT_DWORD i = 0; i < MAX_STAKE_TYPE; ++i)
            {
                freeGameScene.dStakeJetton[i] = (stakeCfg[i].llBetTotalStake*TO_DOUBLE);
            }
			SendTableData(dwChairID, SC_GAMESCENE_FREE, &freeGameScene, sizeof(MSG_GS_FREE));
			break;
        }
        default:
            break;
    }
}

//定时器事件
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
    switch (dwTimerID)
    {
        case IDI_SAVE_STOCK:
        {
            //保存4个房间的血池信息
            for (int i = 0; i < ROOM_NUM; i++)
            {
                m_stockControlInfo[i].dwServerID = i;
                m_pGameDeskPtr->SaveStockControlInfo(&m_stockControlInfo[i]);
            }
            break;
        }
        case IDI_LET_AI_LEFT:
        {
            m_pGameDeskPtr->KillGameTimer(IDI_LET_AI_LEFT);
            m_pGameDeskPtr->ClearTableUser(dwParam);
            break;
        }
        case IDI_SELECT_FREE_GAME:
        {
            CT_DWORD dwTime = time(NULL);
            for (auto i = m_mapFreeGameInfo.begin(); i != m_mapFreeGameInfo.end();)
            {
                if((dwTime - i->second.dwGetFreeTime) > TIME_FREE_GAME)
                {
                    CT_WORD wChairID = m_pGameDeskPtr->GetUserChairID(i->first);
                    FreeGameStart(i->first, i->second);
                    if(m_pGameDeskPtr->GetUserStatus(wChairID) == sOffLine)
                    {
                        m_pGameDeskPtr->ClearTableUser(wChairID);
                    }
                    m_mapFreeGameInfo.erase(i++);
                }
                else
                {
                    ++i;
                }
            }

            break;
        }
        default:
            break;
    }
}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	switch (dwSubCmdID)
	{
		case SUB_C_START:
		{
			if (dwDataSize != sizeof(CMD_C_Start))
			{
				return false;
			}

			CMD_C_Start* pStart = (CMD_C_Start*)pDataBuffer;
            GameStart(wChairID, pStart);
			break;
		}
        case SUB_C_FREE_GAME_TYPE:
        {
            if (dwDataSize != sizeof(CMD_C_FreeGame))
            {
                return false;
            }
            CT_BYTE cbFail = 0;
            CMD_C_FreeGame* pStart = (CMD_C_FreeGame*)pDataBuffer;
            auto pUser = m_mapFreeGameInfo.find(m_pGameDeskPtr->GetUserID(wChairID));
            if(pUser == m_mapFreeGameInfo.end())
            {
                cbFail = 1;
            }
            else if(pStart->cbFreeGameType < en_3X5_15 || pStart->cbFreeGameType > en_Sys_Rand)
            {
                cbFail = 2;
            }
            else
            {
                pUser->second.cbFreeGameType = pStart->cbFreeGameType;
                FreeGameStart(pUser->first, pUser->second);
                m_mapFreeGameInfo.erase(pUser);
            }

            if(cbFail)
            {
                CMD_S_FreeGameFail message;
                memset(&message, 0, sizeof(message));
                message.cbResult = cbFail;
                //发送结果
                SendTableData(wChairID, SUB_S_FREE_GAME_FAIL, &message, sizeof(message));
            }
            break;
        }
        case SUB_C_JACK_POT:
        {
            if (dwDataSize != sizeof(CMD_C_GetJackpot))
            {
                return false;
            }
            CMD_C_GetJackpot* pNetInfo = (CMD_C_GetJackpot*)pDataBuffer;
            if(pNetInfo->cbJettonIndex > CDCDFCfg::get_instance().m_vStakeCfg.size())
            {
                LOG(ERROR) << "SUB_C_JACK_POT cbJettonIndex:" << (int)pNetInfo->cbJettonIndex << "m_vStakeCfg.SIZE:" << CDCDFCfg::get_instance().m_vStakeCfg.size();
                return false;
            }
            CMD_S_Jackpot jackPotInfo;
            memset(&jackPotInfo, 0, sizeof(jackPotInfo));
            jackPotInfo.cbJettonIndex=pNetInfo->cbJettonIndex;
            auto pJackpot = m_mBonusJackpot.find(pNetInfo->cbJettonIndex);
            if(pJackpot == m_mBonusJackpot.end())
            {
                LOG(ERROR) << "SUB_C_JACK_POT pJackpot == m_mBonusJackpot.end() cbJettonIndex:" << (int)pNetInfo->cbJettonIndex;
                return false;
            }
            for (int i = 0; i < JACK_POT; ++i)
            {
                jackPotInfo.dJackpot[i] = pJackpot->second[i];
            }
            SendTableData(wChairID, SUB_S_JACK_POT, &jackPotInfo, sizeof(jackPotInfo));
            break;
        }
	}
	return true;
}

void CGameProcess::OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
    m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Start);
}

//用户离开
void CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag /*= 0*/)
{
    if (dwChairID == INVALID_CHAIR)
    {
        return;
    }

    //清空玩家赢分
    m_llUserWinScore = 0;

    //玩家狗屎运重置
    //m_wUnLuckyCount = 0;

    //玩家上一次押注重置
    //m_llLastJetton = 0;
    CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
    auto pUser = m_mapFreeGameInfo.find(dwUserID);
    if(pUser == m_mapFreeGameInfo.end())
    {
        //清空玩家
        m_pGameDeskPtr->ClearTableUser(dwChairID, true, true);
    }

    //设置桌子状态
    m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Free);
    //删除用户下注记录表
    auto userBetIndex = m_mUserLastBetIndex.find(dwUserID);
    if(userBetIndex != m_mUserLastBetIndex.end())
    {
        std::vector<CT_DWORD> & userLastList = m_mOnlineBetInfo[userBetIndex->second];
        for (CT_DWORD i = 0; i < userLastList.size(); ++i)
        {
            if(userLastList[i] == dwUserID)
            {
                userLastList.erase(userLastList.begin()+i);
                break;
            }
        }
        m_mUserLastBetIndex.erase(userBetIndex);
    }
}

//用户换桌
void CGameProcess::OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce)
{
}

//用户断线
void CGameProcess::OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
    OnUserLeft(dwChairID, bIsLookUser);
}

//设置指针
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	assert(NULL != pTableFrame);
	m_pGameDeskPtr = pTableFrame;

	//初始化游戏数据
	InitGameData();

	//初始化定时保存血池值
	if (m_pGameDeskPtr->GetTableID() == 0)
    {
	    m_pGameDeskPtr->SetGameTimer(IDI_SAVE_STOCK, TIME_SAVE_STOCK, 0);
	    CDCDFCfg::get_instance().LoadCfg();
        std::vector<CDCDFCfg::tagStakeCfg> & stalkeCfg = CDCDFCfg::get_instance().m_vStakeCfg;
        CDCDFCfg::tagBonusJackpotCfg  & bonusJackpotCfg = CDCDFCfg::get_instance().m_BonusJackpotCfg;
        for (int i = 0; i < stalkeCfg.size(); ++i)
        {
            std::vector<CT_DOUBLE> vJackPot;
            for (int j = 0; j < bonusJackpotCfg.vBonusInfo.size(); ++j)
            {
                vJackPot.push_back(bonusJackpotCfg.vBonusInfo[j].llBaseValue*stalkeCfg[i].wBetMultiplier);
            }
            m_mBonusJackpot.insert(std::make_pair(stalkeCfg[i].cbBetIndex,vJackPot));
        }
    }
}

//清理游戏数据
void CGameProcess::ClearGameData()
{
	m_wGameStatus = GAME_STATUS_FREE;
	m_dwCellScore = 1;
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

//设置血控系统数据(新)
CT_VOID CGameProcess::SetStockControlInfo(std::vector<tagStockControlInfo>& vecStockControlInfo)
{
    for (auto& it : vecStockControlInfo)
    {
        int roomIndex = it.dwServerID % 100;
        if (roomIndex > ROOM_NUM)
            continue;

        m_stockControlInfo[roomIndex].dwServerID = it.dwServerID;
        m_stockControlInfo[roomIndex].llTotalWinScore = it.llTotalWinScore;
        m_stockControlInfo[roomIndex].llTotalLostScore = it.llTotalLostScore;
        m_stockControlInfo[roomIndex].llStock = it.llStock;
        m_stockControlInfo[roomIndex].llBlackTax = it.llBlackTax;
        m_stockControlInfo[roomIndex].llDianKongWinLost = it.llDianKongWinLost;
        m_stockControlInfo[roomIndex].iStockStatus = it.iStockStatus;
        //m_enStockStatus = (en_StockStatus)m_stockControlInfo.iStockStatus;
    }

    //CalcStockStatus();
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

	if (m_pGameDeskPtr != NULL)
	{
		m_dwCellScore = m_pGameDeskPtr->GetGameCellScore();
	}

    m_uLastSpinTime = 0;
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{

}


bool CGameProcess::GameStart(CT_DWORD dwChairID, CMD_C_Start* pStart)
{
    //判断旋转最小时间
    CT_DWORD dwNow = (CT_DWORD)time(NULL);
    if (dwNow - m_uLastSpinTime < CDCDFCfg::get_instance().m_wSpinInterval)
    {
        LOG(ERROR)<<"GameStart error dwNow:"<<dwNow<<" m_uLastSpinTime:"<<m_uLastSpinTime<<" m_wSpinInterval:"<< CDCDFCfg::get_instance().m_wSpinInterval;
        return false;
    }
    if (pStart->cbJettonIndex > MAX_STAKE_TYPE)
    {
        LOG(ERROR)<<"GameStart error cbJettonIndex:"<< (int)pStart->cbJettonIndex;
        return false;
    }
    if(pStart->cbJettonIndex > CDCDFCfg::get_instance().m_vStakeCfg.size())
    {
        LOG(ERROR)<<"GameStart error cbJettonIndex:"<<(int)pStart->cbJettonIndex << "m_vStakeCfg.size():" << CDCDFCfg::get_instance().m_vStakeCfg.size();
        return false;
    }

	CT_LONGLONG llJettonScore = CDCDFCfg::get_instance().m_vStakeCfg[pStart->cbJettonIndex].llBetTotalStake;
	CT_LONGLONG llUserSourceScore = m_pGameDeskPtr->GetUserScore(dwChairID);

	if (llUserSourceScore < CAN_JETTON_SCORE)
	{
		CMD_S_StartFail StartFail;
		StartFail.cbResult = 2;
		SendTableData(dwChairID, SUB_S_MAIN_STRAT_FAIL, &StartFail, sizeof(StartFail));

        if (m_pGameDeskPtr->IsAndroidUser(dwChairID))
            m_pGameDeskPtr->SetGameTimer(IDI_LET_AI_LEFT, TIME_LET_AI_LEFT, dwChairID);
        return false;
	}
	else if (llJettonScore > llUserSourceScore)
	{
		CMD_S_StartFail StartFail;
        StartFail.cbResult = 1;
        SendTableData(dwChairID, SUB_S_MAIN_STRAT_FAIL, &StartFail, sizeof(StartFail));

        if (m_pGameDeskPtr->IsAndroidUser(dwChairID))
            m_pGameDeskPtr->SetGameTimer(IDI_LET_AI_LEFT, TIME_LET_AI_LEFT, dwChairID);
        return false;
	}


	//设置游戏状态
	if (m_wGameStatus == GAME_STATUS_FREE)
    {
	    m_wGameStatus = GAME_STATUS_PLAY;
    }

	//切换下注时，重置
	/*if (m_llLastJetton != llJettonScore)
    {
	    m_wUnLuckyCount = 0;
	    m_llLastJetton = llJettonScore;
    }*/

    CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);

    CDCDFCfg::tagStakeCfg & stakeCfg= CDCDFCfg::get_instance().m_vStakeCfg[pStart->cbJettonIndex];
    std::vector<CDCDFCfg::tagPayNotify> & vPayNotify = CDCDFCfg::get_instance().m_vecPayNotify;
	//生成Icon数据
    bool bActivationFreeGame = false;
	CT_LONGLONG llUserTotalWinScore = 0;
	CT_LONGLONG llUserMainGameWinScore = 0;
    unsigned int nTotalMult = 0;

    CMD_S_Start startResult;
    CT_BYTE cbIcon[ICON_ROW][ICON_COL];
    std::map<int, std::vector<int>> mapIcon;
    GenerateMainGameIcon(dwUserID, cbIcon, startResult.cbWildCount, mapIcon, nTotalMult);
    startResult.dSourceScore = (llUserSourceScore - llJettonScore)*TO_DOUBLE;
    if (nTotalMult != 0)
    {
        llUserMainGameWinScore = stakeCfg.wBetMultiplier * nTotalMult;
        llUserTotalWinScore += llUserMainGameWinScore;
        startResult.dWinScore = llUserMainGameWinScore * TO_DOUBLE;
    }
    memcpy(startResult.cbResultIcon, cbIcon, sizeof(startResult.cbResultIcon));
    int iMulti = llUserMainGameWinScore/stakeCfg.llBetTotalStake;
    for (int i = 0; i < vPayNotify.size(); ++i)
    {
        if(iMulti > vPayNotify[i].dwPayoutMin && iMulti <= vPayNotify[i].dwPayoutMax)
        {
            startResult.cbBigWinner = 1;
            break;
        }
    }

    //CT_BYTE cbRandIcon[ICON_ROW][ICON_COL];
    /*std::cout << "数据2：" << std::endl;
    for (int i = 0; i < ICON_ROW; ++i)
    {
        for (int j = 0; j < ICON_COL; ++j)
        {
             std::cout << (int)cbIcon[i][j] << " ";
        }

        std::cout << std::endl;
    }*/

    for (auto& it : mapIcon)
    {
        if (it.first == en_Scatter)
        {
            bActivationFreeGame = true;
        }
        startResult.flashIcon[startResult.cbRewardCount].cbIconId = (CT_BYTE)it.first;
        startResult.flashIcon[startResult.cbRewardCount].cbFlashEndCol = (CT_BYTE)it.second.size();
        ++startResult.cbRewardCount;
        //std::cout << "闪动的头像: " << it.first << ", 闪到第几列：" << it.second.size() << std::endl;
    }
    //游戏添加将池
    CDCDFCfg::tagBonusJackpotCfg & jackpotCfg = CDCDFCfg::get_instance().m_BonusJackpotCfg;
    startResult.cbFreeGame = bActivationFreeGame ? (rand()%3+1) : 0;
    auto jackpot = m_mBonusJackpot.find(pStart->cbJettonIndex);
    for (int j = 0; (jackpot != m_mBonusJackpot.end()) && (j < jackpot->second.size()); ++j)
    {
        jackpot->second[j] += (jackpotCfg.vBonusInfo[j].fPlusValue*stakeCfg.wBetMultiplier);
        startResult.stJackPotInfo.dJackpot[j] = jackpot->second[j];
    }
    startResult.stJackPotInfo.cbJettonIndex = pStart->cbJettonIndex;
    //发送结果
    SendTableData(dwChairID, SUB_S_MAIN_START, &startResult, sizeof(startResult));

    //记录下注档位
    CT_BYTE cbLastBetIndex = 0;
    auto lastBet = m_mUserLastBetIndex.find(dwUserID);
    if(lastBet != m_mUserLastBetIndex.end())
    {
        cbLastBetIndex = lastBet->second;
        if(lastBet->second != pStart->cbJettonIndex)
        {
            lastBet->second = pStart->cbJettonIndex;
        }
        std::vector<CT_DWORD> & userLastList = m_mOnlineBetInfo[cbLastBetIndex];
        for (CT_DWORD i = 0; i < userLastList.size(); ++i)
        {
            if(userLastList[i] == dwUserID)
            {
                userLastList.erase(userLastList.begin()+i);
                break;
            }
        }
    }
    else
    {
        m_mUserLastBetIndex.insert(std::make_pair(dwUserID, pStart->cbJettonIndex));
    }

    CT_BOOL bHasSelf = false;
    std::vector<CT_DWORD> & userList = m_mOnlineBetInfo[pStart->cbJettonIndex];
    for (CT_DWORD i = 0; i < userList.size(); ++i)
    {
        if(userList[i] == dwUserID)
        {
            bHasSelf = true;
            continue;
        }
        CT_WORD wSendChairID = m_pGameDeskPtr->GetUserChairID(userList[i]);
        if(INVALID_CHAIR != wSendChairID)
        {
            SendTableData(wSendChairID, SUB_S_JACK_POT, &(startResult.stJackPotInfo), sizeof(startResult.stJackPotInfo));
        }
    }
    //如果自己不再该列表内就添加
    if(!bHasSelf)
    {
        userList.push_back(dwUserID);
    }


    //激活免费游戏
    if (bActivationFreeGame)
    {
        m_mapFreeGameInfo[dwUserID].cbJettonIndex = pStart->cbJettonIndex;
        m_mapFreeGameInfo[dwUserID].dwGetFreeTime = time(NULL);
        m_mapFreeGameInfo[dwUserID].cbFreeGameType = startResult.cbFreeGame;
    }

    CalcAndRecordAndWriteScore(dwChairID, dwUserID, llUserTotalWinScore, llJettonScore, llUserSourceScore, startResult.cbFreeGame);

    return true;
}
bool CGameProcess::FreeGameStart(CT_DWORD dwUserID, tagFreeGameInfo & freeGameUser)
{
    CT_DWORD dwChairID = m_pGameDeskPtr->GetUserChairID(dwUserID);
    //判断旋转最小时间
    CT_DWORD dwNow = (CT_DWORD)time(NULL);
    if (dwNow - m_uLastSpinTime < CDCDFCfg::get_instance().m_wSpinInterval)
    {
        return false;
    }

    CT_BYTE cbJettonIndex = freeGameUser.cbJettonIndex;


    CT_BYTE cbType = freeGameUser.cbFreeGameType;
    if(cbType == en_Sys_Rand)
    {
        cbType = rand()%4+1;
    }


    CT_LONGLONG llUserSourceScore = m_pGameDeskPtr->GetUserScore(dwChairID);

    //设置游戏状态
    if (m_wGameStatus == GAME_STATUS_FREE)
    {
        m_wGameStatus = GAME_STATUS_PLAY;
    }

    CDCDFCfg::tagStakeCfg & stakeCfg= CDCDFCfg::get_instance().m_vStakeCfg[cbJettonIndex];
    //std::vector<CDCDFCfg::tagPayNotify> & vPayNotify = CDCDFCfg::get_instance().m_vecPayNotify;

    CMD_S_FreeGameType freeGameType;
    memset(&freeGameType, 0, sizeof(freeGameType));
    freeGameType.cbCount = CDCDFCfg::get_instance().m_mFreeTypeCfg[cbType].dwFreeGameTimes;
    freeGameType.cbRows = CDCDFCfg::get_instance().m_mFreeTypeCfg[cbType].cbRows;
    SendTableData(dwChairID, SUB_S_FREE_GAME_TYPE, &freeGameType, sizeof(freeGameType));

    //生成Icon数据
    CT_LONGLONG llUserTotalWinScore = 0;
    //CT_LONGLONG llUserMainGameWinScore = 0;
    CMD_S_Jackpot  jackPotInfo;
    memset(&jackPotInfo, 0, sizeof(jackPotInfo));

    //std::cout << "获得免费游戏！" << std::endl;
    CT_BYTE szBuffer[1024];
    CT_DWORD dwSendSize = 0;

    unsigned int nFGTotalMult = 0;

    for (int i = 0; i < freeGameType.cbCount; ++i)
    {
        CMD_S_FreeGameStart freeResult;
        CT_BYTE cbIcon[ICON_MAX_ROW][ICON_COL];
        std::map<int, std::vector<int>> mapFreeIcon;
        unsigned int nCurrentTotalMult = 0;
        GenerateFreeGameIcon(cbIcon, mapFreeIcon, nCurrentTotalMult, nFGTotalMult,freeGameType.cbRows ,freeResult.cbWildCount);
        nFGTotalMult += nCurrentTotalMult;

        //计算得分
        freeResult.dSourceScore = (llUserSourceScore*TO_DOUBLE);
        //当前得分
        if (nCurrentTotalMult != 0)
        {
            CT_LONGLONG llUserWinScore = stakeCfg.wBetMultiplier * nCurrentTotalMult;
            llUserTotalWinScore += llUserWinScore;
            freeResult.dWinScore = llUserWinScore * TO_DOUBLE;
        }

        memcpy(freeResult.cbResultIcon, cbIcon, sizeof(freeResult.cbResultIcon));
        for (auto& it : mapFreeIcon)
        {
            if(en_Scatter == it.first)
            {
                freeGameType.cbCount += 3;
                freeResult.cbFreeGame = true;
            }
            freeResult.flashIcon[freeResult.cbRewardCount].cbIconId = (CT_BYTE)it.first;
            freeResult.flashIcon[freeResult.cbRewardCount].cbFlashEndCol = (CT_BYTE)it.second.size();
            ++freeResult.cbRewardCount;
        }
        //游戏添加将池
        CDCDFCfg::tagBonusJackpotCfg & jackpotCfg= CDCDFCfg::get_instance().m_BonusJackpotCfg;

        auto jackpot = m_mBonusJackpot.find(cbJettonIndex);
        for (int j = 0; (jackpot != m_mBonusJackpot.end()) && (j < jackpot->second.size()); ++j)
        {
            jackpot->second[j] += (jackpotCfg.vBonusInfo[j].fPlusValue*stakeCfg.wBetMultiplier);
            freeResult.stJackPotInfo.dJackpot[j] = jackpot->second[j];
        }
        freeResult.stJackPotInfo.cbJettonIndex = cbJettonIndex;
        if(freeGameType.cbCount -1 == i)
        {
            jackPotInfo = freeResult.stJackPotInfo;
        }
        //拷贝数据
        memcpy(szBuffer + dwSendSize, &freeResult, sizeof(freeResult));
        dwSendSize += sizeof(freeResult);
    }
    //发送免费游戏数据
    SendTableData(dwChairID, SUB_S_FREE_START, &szBuffer, dwSendSize);


    std::vector<CT_DWORD> & userList = m_mOnlineBetInfo[cbJettonIndex];
    for (CT_DWORD i = 0; i < userList.size(); ++i)
    {
        if(userList[i] == dwUserID)
        {
            continue;
        }
        CT_WORD wSendChairID = m_pGameDeskPtr->GetUserChairID(userList[i]);
        if(INVALID_CHAIR != wSendChairID)
        {
            SendTableData(wSendChairID, SUB_S_JACK_POT, &(jackPotInfo), sizeof(jackPotInfo));
        }
    }


    CalcAndRecordAndWriteScore(dwChairID, dwUserID, llUserTotalWinScore, 0, llUserSourceScore);

    return true;
}

void CGameProcess::GenerateMainGameIcon(CT_DWORD dwUserID, CT_BYTE cbIcon[ICON_ROW][ICON_COL], CT_BYTE & cbWildCount, std::map<int, std::vector<int>> &mapIcon, unsigned int &nTotalMult)
{
	//判断玩家是否在点控范围中
    auto itPointControl = m_mapUserPointControl.find(dwUserID);
    if (itPointControl != m_mapUserPointControl.end())
    {
        tagDianKongData & diankongData = itPointControl->second;

    }
    else
    {

    }

    auto gameIcon = CDCDFCfg::get_instance().m_mGameIconCfg.find(0);
    if(gameIcon == CDCDFCfg::get_instance().m_mGameIconCfg.end())
    {
        LOG(ERROR) << "m_mGameIconCfg cfg error";
        return;
    }
	bool bSucc = false;
	//int nRandCount = 0;
    CT_BYTE cbRandIcon[ICON_ROW][ICON_COL];
    do
    {
        memset(cbRandIcon, 0, sizeof(cbRandIcon));
        for (int col = 0; col < ICON_COL; ++col)
        {
            std::uniform_int_distribution<unsigned int> disValue(0, gameIcon->second[col].wIndexCounts);
            int randValue = disValue(m_gen);
            for (int row = 0; row < ICON_ROW; ++row)
            {
                cbRandIcon[row][col] = gameIcon->second[col].vIconIDInfo[randValue+row];
            }
        }

        //计算生成的图标的倍数
        mapIcon.clear();
        for (int col = 0; col < ICON_COL; ++col)
        {
            for (int row = 0; row < ICON_ROW; ++row)
            {
                CT_BYTE cbIcon = cbRandIcon[row][col];
                //如果是万能图标，则2号以上图标插入一次
                if (cbIcon == en_Wild)
                {
                    for (int nIconCount = 0; nIconCount < ICON_COUNT; ++nIconCount)
                    {
                        std::vector<int>& vecIconCount = mapIcon[nIconCount];
                        if (vecIconCount.empty())
                        {
                            vecIconCount.resize(ICON_COL, 0);
                        }
                        //记录第一行出现的图标的个数
                        vecIconCount[col] += 1;
                    }
                    cbWildCount++;
                }
                else
                {
                    std::vector<int>& vecIconCount = mapIcon[cbIcon];
                    if (vecIconCount.empty())
                    {
                        vecIconCount.resize(ICON_COL, 0);
                    }
                    //记录第一行出现的图标的个数
                    vecIconCount[col] += 1;
                }
            }
        }

        //遍历整个列表,把列数少于3的删除
        for (auto it = mapIcon.begin(); it != mapIcon.end(); )
        {
            std::vector<int>& vecIconCount = it->second;
            if (vecIconCount[0] == 0 || vecIconCount[1] == 0 || vecIconCount[2] == 0)
            {
                it = mapIcon.erase(it);
            }
            else
            {
                ++it;
            }
        }

        //计算倍数
        //int nTotalMult = 0;
        nTotalMult = 0;
        //bool bFreeGame = false;
        for (auto it = mapIcon.begin(); it != mapIcon.end(); ++it)
        {
            std::vector<int>& vecIconCount = it->second;
            int nIconId = it->first;
            int nMul = 1;
            int validCol = 0;
            for (int col = 0; col < ICON_COL; ++col)
            {
                if (vecIconCount[col] != 0)
                {
                    //组合个数
                    nMul *= vecIconCount[col];
                    validCol += 1;
                }
                else
                {
                    vecIconCount.erase(vecIconCount.begin() + col, vecIconCount.end());
                    break;
                }
            }

            CDCDFCfg::tagPayTable& payTable = CDCDFCfg::get_instance().m_payTable[nIconId];

            if (validCol == 3)
            {
                nMul *= payTable.wThreePayMult;
            }
            else if (validCol == 4)
            {
                nMul *= payTable.wFourPayMult;
            }
            else if (validCol == 5)
            {
                nMul *= payTable.wFivePayMult;
            }

            nTotalMult += nMul;

        }

        bSucc = true;

    } while (!bSucc /*&& ++nRandCount < 100*/);

    //拷贝头像
    memcpy(cbIcon, cbRandIcon, sizeof(cbRandIcon));
}

void CGameProcess::GenerateFreeGameIcon(CT_BYTE cbIcon[ICON_MAX_ROW][ICON_COL], std::map<int, std::vector<int>> &mapIcon, unsigned int &nTotalMult,
        unsigned int &nFreeGameTotalMult, CT_BYTE cbRows, CT_BYTE & cbWildCount)
{
    auto gameIcon = CDCDFCfg::get_instance().m_mGameIconCfg.find(cbRows);
    if(gameIcon == CDCDFCfg::get_instance().m_mGameIconCfg.end())
    {
        LOG(ERROR) << "m_mGameIconCfg cfg error";
        return;
    }
    if(cbRows > ICON_MAX_ROW)
    {
        LOG(ERROR) << "GenerateFreeGameIcon FreeGameInfo.cbRows:" << cbRows;
        cbRows = ICON_MAX_ROW;
    }
    std::random_device rd;
    std::mt19937 gen(rd());

    bool bSucc = false;
    //int nRandCount = 0;
    CT_BYTE cbRandIcon[ICON_MAX_ROW][ICON_COL];
//    do
//    {
        memset(cbRandIcon, 0, sizeof(cbRandIcon));
        for (int col = 0; col < ICON_COL; ++col)
        {
            std::uniform_int_distribution<unsigned int> disValue(0, gameIcon->second[col].wIndexCounts);
            int randValue = disValue(gen);
            for (int row = 0; row < cbRows; ++row)
            {
                cbRandIcon[row][col] = gameIcon->second[col].vIconIDInfo[randValue+row];
            }
        }

        //计算生成的图标的倍数
        mapIcon.clear();
        for (int col = 0; col < ICON_COL; ++col)
        {
            for (int row = 0; row < cbRows; ++row)
            {
                CT_BYTE cbIcon = cbRandIcon[row][col];
                //如果是万能图标，则2号以上图标插入一次
                if (cbIcon == en_Wild)
                {
                    for (int nIconCount = 0; nIconCount < ICON_COUNT; ++nIconCount)
                    {
                        std::vector<int>& vecIconCount = mapIcon[nIconCount];
                        if (vecIconCount.empty())
                        {
                            vecIconCount.resize(ICON_COL, 0);
                        }
                        //记录第一行出现的图标的个数
                        vecIconCount[col] += 1;
                    }
                    cbWildCount++;
                }
                else
                {
                    std::vector<int>& vecIconCount = mapIcon[cbIcon];
                    if (vecIconCount.empty())
                    {
                        vecIconCount.resize(ICON_COL, 0);
                    }
                    //记录第一行出现的图标的个数
                    vecIconCount[col] += 1;
                }
            }
        }

        //遍历整个列表,把列数少于3的删除
        for (auto it = mapIcon.begin(); it != mapIcon.end(); )
        {
            std::vector<int>& vecIconCount = it->second;
            if (vecIconCount[0] == 0 || vecIconCount[1] == 0 || vecIconCount[2] == 0)
            {
                it = mapIcon.erase(it);
            }
            else
            {
                ++it;
            }
        }

        //计算倍数
        //int nTotalMult = 0;
        nTotalMult = 0;
        //bool bFreeGame = false;
        for (auto it = mapIcon.begin(); it != mapIcon.end(); ++it)
        {
            std::vector<int>& vecIconCount = it->second;
            int nIconId = it->first;
            int nMul = 1;
            int validCol = 0;
            for (int col = 0; col < ICON_COL; ++col)
            {
                if (vecIconCount[col] != 0)
                {
                    //组合个数
                    nMul *= vecIconCount[col];
                    validCol += 1;
                }
                else
                {
                    vecIconCount.erase(vecIconCount.begin() + col, vecIconCount.end());
                    break;
                }
            }

            CDCDFCfg::tagPayTable& payTable = CDCDFCfg::get_instance().m_payTable[nIconId];

            if (validCol == 3)
            {
                nMul *= payTable.wThreePayMult;
            }
            else if (validCol == 4)
            {
                nMul *= payTable.wFourPayMult;
            }
            else if (validCol == 5)
            {
                nMul *= payTable.wFivePayMult;
            }

            nTotalMult += nMul;

        }

        bSucc = true;

//    } while (!bSucc /*&& ++nRandCount < 100*/);

    //拷贝头像
    memcpy(cbIcon, cbRandIcon, sizeof(cbRandIcon));
}

//设置捕鱼点控数据
CT_BOOL CGameProcess::SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData)
{
    auto it = m_mapUserPointControl.find(pDianKongData->dwUserID);
    if (it != m_mapUserPointControl.end())
    {
        it->second.iDianKongZhi = pDianKongData->iDianKongZhi;
        it->second.llDianKongFen = pDianKongData->llDianKongFen;
        it->second.llCurDianKongFen = pDianKongData->llCurrDianKongFen;
    }
    else
    {
        tagDianKongData dianKongData;
        dianKongData.dwUserID = pDianKongData->dwUserID;
        dianKongData.iDianKongZhi = pDianKongData->iDianKongZhi;
        dianKongData.llDianKongFen = pDianKongData->llDianKongFen;
        dianKongData.llCurDianKongFen = pDianKongData->llCurrDianKongFen;
        m_mapUserPointControl.insert(std::make_pair(pDianKongData->dwUserID, dianKongData));
    }

    return true;
}

void CGameProcess::CalcAndRecordAndWriteScore(CT_DWORD dwChairID, CT_DWORD dwUserID, CT_LONGLONG llUserTotalWinScore, CT_LONGLONG llJettonScore, CT_LONGLONG llUserSourceScore, CT_BOOL bHasFreeGame/* = false*/)
{
    //结算玩家分数
    CT_LONGLONG llUserRealWinScore = llUserTotalWinScore-llJettonScore;

    ScoreInfo  ScoreData;
    ScoreData.dwUserID = dwUserID;
    ScoreData.bBroadcast = true;
    ScoreData.llScore = llUserRealWinScore;
    ScoreData.llRealScore= llUserRealWinScore;
    m_pGameDeskPtr->WriteUserScore(dwChairID, ScoreData);

    //计算玩家得分
    RecordScoreInfo	recordScoreInfo;
    recordScoreInfo.dwUserID = dwUserID;
    recordScoreInfo.llSourceScore = llUserSourceScore;
    recordScoreInfo.iScore = (CT_INT32)llUserRealWinScore;
    recordScoreInfo.iAreaJetton[0] = (CT_INT32)llJettonScore;
    recordScoreInfo.dwRevenue = 0;
    recordScoreInfo.cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(dwChairID);
    recordScoreInfo.cbStatus = 1;

    //记录玩家本次总成绩
    m_llUserWinScore += llUserRealWinScore;

    //检测玩家点控数据
    auto itPointControl = m_mapUserPointControl.find(dwUserID);
    if (itPointControl != m_mapUserPointControl.end())
    {
        //计算玩家当前点控分
        tagDianKongData& diankongData = itPointControl->second;
        //累计玩家当前点控分, 赢-负+
        diankongData.llCurDianKongFen -= llUserRealWinScore;
        //库存点控的数值
        m_stockControlInfo[0].llDianKongWinLost -= llUserRealWinScore;

        //判断玩家是否点控结束
        if (diankongData.llCurDianKongFen >= diankongData.llDianKongFen)
        {
            diankongData.iDianKongZhi = 0;
        }

        MSG_G2DB_User_PointControl updateDiankong;
        updateDiankong.dwUserID = dwUserID;
        updateDiankong.iDianKongZhi = diankongData.iDianKongZhi;
        updateDiankong.llDianKongFen = diankongData.llDianKongFen;
        updateDiankong.llCurrDianKongFen = diankongData.llCurDianKongFen;
        m_pGameDeskPtr->SavePlayerDianKongData(updateDiankong);

        //如果点控结束,则把玩家清除出点控列表
        if (diankongData.iDianKongZhi == 0)
        {
            m_mapUserPointControl.erase(itPointControl);
        }
    }
    else
    {
        if (m_llUserWinScore > 0)
        {
            //判断玩家是否达到点控条件，如果达到条件则点控玩家
            /*for (int k = 0; k < POINT_CHEAT_CASE_COUNT; ++k)
            {
                CDCDFCfg::tagPointCheatCaseCfg& pointCheatCaseCfg = CDCDFCfg::get_instance().m_pointCheatCaseCfg[k];
                if (m_llUserWinScore > pointCheatCaseCfg.llUserWinScore)
                {
                    MSG_G2DB_User_PointControl updateDiankong;
                    updateDiankong.dwUserID = dwUserID;
                    updateDiankong.iDianKongZhi = pointCheatCaseCfg.iCheatVal;
                    updateDiankong.llDianKongFen = pointCheatCaseCfg.llCheatScore;
                    updateDiankong.llCurrDianKongFen = 0;
                    m_pGameDeskPtr->SavePlayerDianKongData(updateDiankong);

                    tagDianKongData dianKongData;
                    dianKongData.dwUserID = dwUserID;
                    dianKongData.iDianKongZhi = pointCheatCaseCfg.iCheatVal;
                    dianKongData.llDianKongFen = pointCheatCaseCfg.llCheatScore;
                    dianKongData.llCurDianKongFen = 0;
                    m_mapUserPointControl.insert(std::make_pair(dwUserID, dianKongData));
                    break;
                }
            }*/
        }
        m_stockControlInfo[0].llStock -= llUserRealWinScore;
    }

    //系统输赢分
    m_stockControlInfo[0].llTotalWinScore += llJettonScore;
    if (llUserTotalWinScore != 0)
        m_stockControlInfo[0].llTotalLostScore += llUserTotalWinScore;

    //玩家输赢分
    if (llUserRealWinScore > 0)
    {
        m_llStockTaxTemp[0] += llUserRealWinScore;
    }
    else
    {
        m_llStockTaxTemp[0] -= llUserRealWinScore;
    }

    m_pGameDeskPtr->RecordRawInfo(&recordScoreInfo, GAME_PLAYER, NULL, 0, 0, 0, 0, 0, 0, 0);

    //如果玩家有奖励，重置不中奖次数
    /*if (nTotalMult != 0 || bActivationFreeGame)
    {
        m_wUnLuckyCount = 0;
    }
    else
    {
        ++m_wUnLuckyCount;
    }*/

    //为了统计在线时长,每一局结束时，则标识为下一局的开始
    if(!bHasFreeGame)
    {
        m_pGameDeskPtr->SetUserReady(dwChairID);
    }
}