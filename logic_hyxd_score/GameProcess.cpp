
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
#include "FuXingGaoZhaoCfg.h"
#include <cmath>


#define IDI_SAVE_STOCK				100					//保存血池值
#define TIME_SAVE_STOCK 			30*1000				//保存血池值时间

#define IDI_LET_AI_LEFT				101					//让AI离开桌子
#define TIME_LET_AI_LEFT 			60*1000				//让AI离开桌子时间
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
			FXGZ_MSG_GS_FREE freeGameScene;
			for (int i = 0; i < JETTON_COUNT; ++i)
			{
				freeGameScene.dUserLeaseScore = CAN_JETTON_SCORE*TO_DOUBLE;
			}

			SendTableData(dwChairID, SC_GAMESCENE_FREE, &freeGameScene, sizeof(FXGZ_MSG_GS_FREE));
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
        default:
            break;
    }
}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	switch (dwSubCmdID)
	{
		case FXGZ_SUB_C_START:
		{
			if (dwDataSize != sizeof(FXGZ_CMD_C_Start))
			{
				return false;
			}

			FXGZ_CMD_C_Start* pStart = (FXGZ_CMD_C_Start*)pDataBuffer;
            GameStart(wChairID, pStart);
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

    //清空玩家
    m_pGameDeskPtr->ClearTableUser(dwChairID, true, true);

    //设置桌子状态
    m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Free);
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
	    CFXGZCfg::get_instance().LoadCfg();
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

#include <iostream>
bool CGameProcess::GameStart(CT_DWORD dwChairID, FXGZ_CMD_C_Start* pStart)
{
    //判断旋转最小时间
    CT_DWORD dwNow = (CT_DWORD)time(NULL);
    if (dwNow - m_uLastSpinTime < CFXGZCfg::get_instance().m_wSpinInterval)
    {
        return false;
    }

	CT_LONGLONG llJettonScore = ceil(pStart->dJettonScore*TO_LL);
	CT_LONGLONG llUserSourceScore = m_pGameDeskPtr->GetUserScore(dwChairID);

	if (llUserSourceScore < CAN_JETTON_SCORE)
	{
		FXGZ_CMD_S_StartFail StartFail;
		StartFail.cbResult = 2;
		SendTableData(dwChairID, SUB_S_MAIN_STRAT_FAIL, &StartFail, sizeof(StartFail));

        if (m_pGameDeskPtr->IsAndroidUser(dwChairID))
            m_pGameDeskPtr->SetGameTimer(IDI_LET_AI_LEFT, TIME_LET_AI_LEFT, dwChairID);
        return false;
	}
	else if (llJettonScore > llUserSourceScore)
	{
		FXGZ_CMD_S_StartFail StartFail;
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

	//根据玩家的下注额选择房间Index，这个不是serverid。是虚拟的房间id
    int nRoomIndex = GetJettonRoomIndex(llJettonScore);

	//生成Icon数据
    bool bActivationFreeGame = false;
	CT_LONGLONG llUserTotalWinScore = 0;
	CT_LONGLONG llUserMainGameWinScore = 0;
    unsigned int nTotalMult = 0;
    {
        CT_BYTE cbIcon[ICON_ROW][ICON_COL];
        std::map<int, std::vector<int>> mapIcon;
        CalcStockStatus(nRoomIndex);
        GenerateMainGameIcon(dwUserID, cbIcon, nRoomIndex, mapIcon, nTotalMult);

        CT_WORD wPayBasicUnit = CFXGZCfg::get_instance().m_wPayBasicUnit;
        FXGZ_CMD_S_Start startResult;
        startResult.dSourceScore = (llUserSourceScore - llJettonScore)*TO_DOUBLE;
        if (nTotalMult != 0)
        {
            llUserMainGameWinScore = llJettonScore * nTotalMult / wPayBasicUnit;
            llUserTotalWinScore += llUserMainGameWinScore;
            startResult.dWinScore = llUserMainGameWinScore * TO_DOUBLE;
            //判断是否大赢家
            if (nTotalMult >= CFXGZCfg::get_instance().m_vecPayNotify[1].dwPayoutMax)
            {
                startResult.cbBigWinner = 1;
            }
        }
        memcpy(startResult.cbResultIcon, cbIcon, sizeof(startResult.cbResultIcon));

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
            if (it.first == 0)
            {
                bActivationFreeGame = true;
            }
            startResult.flashIcon[startResult.cbRewardCount].cbIconId = (CT_BYTE)it.first;
            startResult.flashIcon[startResult.cbRewardCount].cbFlashEndCol = (CT_BYTE)it.second.size();
            ++startResult.cbRewardCount;
            //std::cout << "闪动的头像: " << it.first << ", 闪到第几列：" << it.second.size() << std::endl;
        }
        startResult.cbFreeGame = bActivationFreeGame ? 1 : 0;

        //发送结果
        SendTableData(dwChairID, SUB_S_MAIN_START, &startResult, sizeof(startResult));
    }

    //激活免费游戏
    if (bActivationFreeGame)
    {
        //std::cout << "获得免费游戏！" << std::endl;
        CT_BYTE szBuffer[1024];
        CT_DWORD dwSendSize = 0;

        unsigned int nFGTotalMult = 0;
        for (int i = 0; i < 10; ++i)
        {
            CT_BYTE cbIcon[ICON_ROW][ICON_COL];
            std::map<int, std::vector<int>> mapFreeIcon;
            unsigned int nCurrentTotalMult = 0;
            GenerateFreeGameIcon(cbIcon, mapFreeIcon, nCurrentTotalMult, nFGTotalMult);
            nFGTotalMult += nCurrentTotalMult;

            //计算得分
            CT_WORD wPayBasicUnit = CFXGZCfg::get_instance().m_wPayBasicUnit;
            FXGZ_CMD_S_Start freeResult;
            freeResult.dSourceScore = (llUserSourceScore - llJettonScore + llUserMainGameWinScore)*TO_DOUBLE;
            //当前得分
            if (nCurrentTotalMult != 0)
            {
                CT_LONGLONG llUserWinScore = llJettonScore * nCurrentTotalMult / wPayBasicUnit;
                llUserTotalWinScore += llUserWinScore;
                freeResult.dWinScore = llUserWinScore * TO_DOUBLE;
            }

            //判断是否大赢家(目前免费游戏最后一次一定给大赢家标志)
            if (i == 9 /*&& nFGTotalMult >= CFXGZCfg::get_instance().m_vecPayNotify[1].dwPayoutMax*/)
            {
                freeResult.cbBigWinner = 1;
            }

            memcpy(freeResult.cbResultIcon, cbIcon, sizeof(freeResult.cbResultIcon));
            for (auto& it : mapFreeIcon)
            {
                freeResult.flashIcon[freeResult.cbRewardCount].cbIconId = (CT_BYTE)it.first;
                freeResult.flashIcon[freeResult.cbRewardCount].cbFlashEndCol = (CT_BYTE)it.second.size();
                ++freeResult.cbRewardCount;
            }

            //拷贝数据
            memcpy(szBuffer + dwSendSize, &freeResult, sizeof(freeResult));
            dwSendSize += sizeof(freeResult);
        }

        //发送免费游戏数据
        SendTableData(dwChairID, SUB_S_FREE_START, &szBuffer, dwSendSize);
    }

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
    //recordScoreInfo.dwRevenue = llUserRealWinScore >= llJettonScore ? (CT_DWORD)m_pGameDeskPtr->CalculateRevenue(dwChairID, llUserRealWinScore) : 0;
    CT_WORD& m_wTaxRatio = (nRoomIndex == 0 ? CFXGZCfg::get_instance().m_wTaxRatio2 : CFXGZCfg::get_instance().m_wTaxRatio);
    recordScoreInfo.dwRevenue = (CT_DWORD)ceil((llUserTotalWinScore + llJettonScore) * m_wTaxRatio/ 1000);
    //recordScoreInfo.cbIsAndroid = 0;
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
        m_stockControlInfo[nRoomIndex].llDianKongWinLost -= llUserRealWinScore;

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
            for (int k = 0; k < POINT_CHEAT_CASE_COUNT; ++k)
            {
                CFXGZCfg::tagPointCheatCaseCfg& pointCheatCaseCfg = CFXGZCfg::get_instance().m_pointCheatCaseCfg[k];
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
            }
        }
        m_stockControlInfo[nRoomIndex].llStock -= llUserRealWinScore;
    }

    //系统输赢分
    m_stockControlInfo[nRoomIndex].llTotalWinScore += llJettonScore;
    if (llUserTotalWinScore != 0)
        m_stockControlInfo[nRoomIndex].llTotalLostScore += llUserTotalWinScore;

    //玩家输赢分
    /*if (llUserRealWinScore > 0)
    {
        m_llStockTaxTemp[nRoomIndex] += llUserRealWinScore;
    }
    else
    {
        m_llStockTaxTemp[nRoomIndex] -= llUserRealWinScore;
    }*/
    m_llStockTaxTemp[nRoomIndex] += (llUserTotalWinScore + llJettonScore);

    //判断是否到达税收条件
    CT_DWORD& dwTaxBasicUnit = CFXGZCfg::get_instance().m_dwBasicTaxValue;
    while (m_llStockTaxTemp[nRoomIndex] >= dwTaxBasicUnit)
    {
        //收税
        CT_WORD& m_wTaxRatio = (nRoomIndex == 0 ? CFXGZCfg::get_instance().m_wTaxRatio2 : CFXGZCfg::get_instance().m_wTaxRatio);
        CT_LONGLONG llTax = (CT_LONGLONG)ceil(dwTaxBasicUnit * m_wTaxRatio/ 1000);
        m_stockControlInfo[nRoomIndex].llStock -= llTax;
        m_stockControlInfo[nRoomIndex].llBlackTax += llTax;

        //税收变量调整
        m_llStockTaxTemp[nRoomIndex] -= dwTaxBasicUnit;
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
    m_pGameDeskPtr->SetUserReady(dwChairID);

    return true;
}

void CGameProcess::GenerateMainGameIcon(CT_DWORD dwUserID, CT_BYTE cbIcon[ICON_ROW][ICON_COL], int nRoomIndex, std::map<int, std::vector<int>> &mapIcon, unsigned int &nTotalMult)
{
	int nMainGameCase = CHEAT_CASE_COUNT - 1;

	//判断玩家是否在点控范围中
    auto itPointControl = m_mapUserPointControl.find(dwUserID);
    if (itPointControl != m_mapUserPointControl.end())
    {
        tagDianKongData& diankongData = itPointControl->second;
        for (int i = 0; i < POINT_CHEAT_CASE_COUNT; ++i)
        {
            CFXGZCfg::tagPointCheatCaseCfg& pointCheatCaseCfg = CFXGZCfg::get_instance().m_pointCheatCaseCfg[i];
            if (pointCheatCaseCfg.iCheatVal == diankongData.iDianKongZhi)
            {
                nMainGameCase = pointCheatCaseCfg.cbMainGameCaseId - 1;
                //LOG(WARNING) << "GenerateMainGameIcon in point control. main game case id: " << (int)pointCheatCaseCfg.cbMainGameCaseId;
                break;
            }
        }
    }
    else
    {
        static int nGenerateCount = 1;
        for (int i = 0; i < CHEAT_CASE_COUNT; ++i)
        {
            if (m_nStockCheatVal[nRoomIndex] == CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][i].iCheatVal)
            {
                nMainGameCase = CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][i].cbMainGameCaseId - 1;
                if (nGenerateCount % 100 == 0)
                {
                    nGenerateCount = 1;
                    LOG(WARNING) << "GenerateMainGameIcon in stock control. room index: " << nRoomIndex <<  ", current stock: " << m_stockControlInfo[nRoomIndex].llStock/100
                    <<  ", case stock: " <<  CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][i].llStock/100
                    << ", cheat val: " << CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][i].iCheatVal
                    << ", status: " << m_stockControlInfo[nRoomIndex].iStockStatus
                    << ", main game case id : " << (int)CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][i].cbMainGameCaseId;
                }
                else
                {
                    ++nGenerateCount;
                }
                break;
            }
        }
    }

    if (nMainGameCase >= CHEAT_CASE_COUNT)
    {
        LOG(ERROR) << "Get error main game case: " << nMainGameCase;
        nMainGameCase = 0;
    }

    //nMainGameCase = 2;
    //LOG(WARNING) << "nMainGameCase: " << nMainGameCase;

    //找到mainGameCase
	CFXGZCfg::tagMainGameCaseCfg& mainGameCaseCfg = CFXGZCfg::get_instance().m_mainCaseCfg[nMainGameCase];

    /*for (int k = 0; k < PAY_CASE_COUNT; ++k)
    {
        for (int j = 0; j < ICON_COL; ++j)
        {
            CFXGZCfg::tagCaseStripInfo& caseStripInfo = CFXGZCfg::get_instance().m_mainCaseCfg[k].caseStripInfo[j];
            for (int i = 0; i < ICON_COUNT; ++i)
            {
                std::cout << caseStripInfo.wIconRation[i] << ",";
            }
            std::cout << std::endl;
        }
        std::cout << std::endl;
    }*/

	//std::random_device rd;
	//std::mt19937 gen(rd());

	bool bSucc = false;
	//int nRandCount = 0;
    CT_BYTE cbRandIcon[ICON_ROW][ICON_COL];
    do
    {
        memset(cbRandIcon, 0, sizeof(cbRandIcon));
        for (int col = 0; col < ICON_COL; ++col)
        {
            CFXGZCfg::tagCaseStripInfo& caseStripInfo = mainGameCaseCfg.caseStripInfo[col];
            //生成除金锣以外的图标
            for (int row = 0; row < ICON_ROW; ++row)
            {
                //std::uniform_int_distribution<> disValue(1, 10000);
                //int randValue = disValue(gen);
                int randValue = rand() % 10000 + 1;

                int nIconRatio = 0;
                for (CT_BYTE iconId = 1; iconId < ICON_COUNT; ++iconId)
                {
                    nIconRatio += caseStripInfo.wIconRation[iconId];
                    if (randValue <= nIconRatio)
                    {
                        cbRandIcon[row][col] = iconId;
                        break;
                    }
                }
            }

            //生成金锣
            for (CT_BYTE cbScatCount = 0; cbScatCount < caseStripInfo.cbScatCount; ++cbScatCount)
            {
               // std::uniform_int_distribution<> disValue(1, 10000);
               // int randValue = disValue(gen);
                int randValue = rand() % 10000 + 1;

                //如果命中金锣则替换图标
                if (randValue <= caseStripInfo.wIconRation[0])
                {
                    int nRandIcon = rand() % ICON_ROW;
                    cbRandIcon[nRandIcon][col] = 0;
                }
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
                if (cbIcon == en_CaiShenDao)
                {
                    for (int nIconCount = en_CaiShenDao; nIconCount < ICON_COUNT; ++nIconCount)
                    {
                        std::vector<int>& vecIconCount = mapIcon[nIconCount];
                        if (vecIconCount.empty())
                        {
                            vecIconCount.resize(ICON_COL, 0);
                        }
                        //记录第一行出现的图标的个数
                        vecIconCount[col] += 1;
                    }
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
                    if (nIconId != 0)
                    {
                        nMul *= vecIconCount[col];
                        validCol += 1;
                    }
                }
                else
                {
                    vecIconCount.erase(vecIconCount.begin() + col, vecIconCount.end());
                    break;
                }
            }

            //排除金锣，金锣不能得分
            if (nIconId != 0)
            {
                CFXGZCfg::tagPayTable& payTable = CFXGZCfg::get_instance().m_payTable[nIconId - 1];

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
            /*else if (nIconId == 0)
            {
                bFreeGame = true;
            }*/
        }

        if (nTotalMult < mainGameCaseCfg.dwLimitPayMult)
        {
            bSucc = true;
        }

    } while (!bSucc /*&& ++nRandCount < 100*/);

    //拷贝头像
    memcpy(cbIcon, cbRandIcon, sizeof(cbRandIcon));
}

void CGameProcess::GenerateFreeGameIcon(CT_BYTE cbIcon[ICON_ROW][ICON_COL], std::map<int, std::vector<int>> &mapIcon, unsigned int &nTotalMult, unsigned int &nFreeGameTotalMult)
{
    CFXGZCfg::tagFreeGameCaseCfg& freeGameCaseCfg = CFXGZCfg::get_instance().m_freeCaseCfg;

    //std::random_device rd;
    //std::mt19937 gen(rd());

    bool bSucc = false;
    //int nRandCount = 0;
    CT_BYTE cbRandIcon[ICON_ROW][ICON_COL];
    do
    {
        memset(cbRandIcon, 0, sizeof(cbRandIcon));
        for (int col = 0; col < ICON_COL; ++col)
        {
            CFXGZCfg::tagCaseStripInfo& caseStripInfo = freeGameCaseCfg.caseStripInfo[col];
            //生成除金锣以外的图标
            for (int row = 0; row < ICON_ROW; ++row)
            {
                //std::uniform_int_distribution<> disValue(1, 10000);
                //int randValue = disValue(gen);
                int randValue = rand() % 10000 + 1;

                int nIconRatio = 0;
                for (CT_BYTE iconId = 1; iconId < ICON_COUNT; ++iconId)
                {
                    nIconRatio += caseStripInfo.wIconRation[iconId];
                    if (randValue <= nIconRatio)
                    {
                        cbRandIcon[row][col] = iconId;
                        break;
                    }
                }
            }

            //生成金锣(免费游戏不生成金锣)
            /*
            for (CT_BYTE cbScatCount = 0; cbScatCount < caseStripInfo.cbScatCount; ++cbScatCount)
            {
                std::uniform_int_distribution<> disValue(1, 10000);
                int randValue = disValue(gen);

                //如果命中金锣则替换图标
                if (randValue <= caseStripInfo.wIconRation[0])
                {
                    int nRandIcon = rand() % ICON_ROW;
                    cbRandIcon[nRandIcon][col] = 0;
                }
            }
            */
        }

        //计算生成的图标的倍数
        mapIcon.clear();
        for (int col = 0; col < ICON_COL; ++col)
        {
            for (int row = 0; row < ICON_ROW; ++row)
            {
                CT_BYTE cbIcon = cbRandIcon[row][col];
                //如果是万能图标，则2号以上图标插入一次
                if (cbIcon == en_CaiShenDao)
                {
                    for (int nIconCount = en_CaiShenDao; nIconCount < ICON_COUNT; ++nIconCount)
                    {
                        std::vector<int>& vecIconCount = mapIcon[nIconCount];
                        if (vecIconCount.empty())
                        {
                            vecIconCount.resize(ICON_COL, 0);
                        }
                        //记录第一行出现的图标的个数
                        vecIconCount[col] += 1;
                    }
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
                    if (nIconId != 0)
                    {
                        nMul *= vecIconCount[col];
                        validCol += 1;
                    }
                }
                else
                {
                    vecIconCount.erase(vecIconCount.begin() + col, vecIconCount.end());
                    break;
                }
            }

            if (nIconId != 0)
            {
                CFXGZCfg::tagPayTable& payTable = CFXGZCfg::get_instance().m_payTable[nIconId - 1];

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
        }

        unsigned int nMaxTotalmult = 0;
        if (nFreeGameTotalMult >= freeGameCaseCfg.dwTotalLimitPay)
        {
            nMaxTotalmult = freeGameCaseCfg.dwHighlimitPay;
        }
        else
        {
            nMaxTotalmult = freeGameCaseCfg.dwBasicLimitPay;
        }

        if (nTotalMult <= nMaxTotalmult)
        {
            bSucc = true;
            //nFreeGameTotalMult += nTotalMult;
        }
    } while (!bSucc /*&& ++nRandCount < 200*/);

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

void CGameProcess::CalcStockStatus(int nRoomIndex)
{
    CFXGZCfg::tagCheatCaseCfg& outCheatCaseCfg2 = CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][3];
    CFXGZCfg::tagCheatCaseCfg& outCheatCaseCfg1 = CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][2];
    CFXGZCfg::tagCheatCaseCfg& eatCheatCaseCfg2 = CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][0];
    CFXGZCfg::tagCheatCaseCfg& eatCheatCaseCfg1 = CFXGZCfg::get_instance().m_cheatCaseCfg[nRoomIndex][1];

    if (m_stockControlInfo[nRoomIndex].iStockStatus == en_OutScore)
    {
        if (m_stockControlInfo[nRoomIndex].llStock >= outCheatCaseCfg2.llStock)
        {
            m_nStockCheatVal[nRoomIndex] = outCheatCaseCfg2.iCheatVal;
        }
        else if (m_stockControlInfo[nRoomIndex].llStock >= outCheatCaseCfg1.llStock)
        {
            m_nStockCheatVal[nRoomIndex] = outCheatCaseCfg1.iCheatVal;
        }
        else if (m_stockControlInfo[nRoomIndex].llStock < eatCheatCaseCfg2.llStock)
        {
            //m_enStockStatus = en_EatScore;
            m_stockControlInfo[nRoomIndex].iStockStatus = en_EatScore;
            m_nStockCheatVal[nRoomIndex] = eatCheatCaseCfg2.iCheatVal;
        }
        else if (m_stockControlInfo[nRoomIndex].llStock < eatCheatCaseCfg1.llStock)
        {
           // m_enStockStatus = en_EatScore;
            m_stockControlInfo[nRoomIndex].iStockStatus = en_EatScore;
            m_nStockCheatVal[nRoomIndex] = eatCheatCaseCfg1.iCheatVal;
        }
        else
        {
            m_nStockCheatVal[nRoomIndex] = outCheatCaseCfg1.iCheatVal;
        }
    }
    else
    {
        if (m_stockControlInfo[nRoomIndex].llStock <= eatCheatCaseCfg2.llStock)
        {
            m_nStockCheatVal[nRoomIndex] = eatCheatCaseCfg2.iCheatVal;
        }
        else if (m_stockControlInfo[nRoomIndex].llStock <= eatCheatCaseCfg1.llStock)
        {
            m_nStockCheatVal[nRoomIndex] = eatCheatCaseCfg1.iCheatVal;
        }
        else if (m_stockControlInfo[nRoomIndex].llStock >= outCheatCaseCfg2.llStock)
        {
            //m_enStockStatus = en_OutScore;
            m_stockControlInfo[nRoomIndex].iStockStatus = en_OutScore;
            m_nStockCheatVal[nRoomIndex] = outCheatCaseCfg2.iCheatVal;
        }
        else if (m_stockControlInfo[nRoomIndex].llStock >= outCheatCaseCfg1.llStock)
        {
            //m_enStockStatus = en_OutScore;
            m_stockControlInfo[nRoomIndex].iStockStatus = en_OutScore;
            m_nStockCheatVal[nRoomIndex] = outCheatCaseCfg1.iCheatVal;
        }
        else
        {
            m_nStockCheatVal[nRoomIndex] = eatCheatCaseCfg1.iCheatVal;
        }
    }
}

//计算游戏是属于哪一个场次
int CGameProcess::GetJettonRoomIndex(CT_LONGLONG llJettonScore)
{
    if (llJettonScore >= 50000)
    {
        return 3;
    }
    else if (llJettonScore >= 5000)
    {
        return 2;
    }
    else if (llJettonScore >= 700)
    {
        return 1;
    }
    else
    {
        return 0;
    }
}