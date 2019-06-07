
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
#include <cmath>
#include <iostream>

//打印日志
#define		PRINT_LOG_INFO						1
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//#define IDI_CHECK_TABLE			100					//检查桌子
//#define IDI_CHECK_TRUSTEE			101					//断线托管

//#define IDI_AUTO_READY				200					//自动准备
#define IDI_CHECK_START				100					//检测游戏开始
#define IDI_CHECK_END				101					//检测游戏结束
#define IDI_FA_HB			        102					//机器人发红包定时器
#define IDI_GAME_END_REAL			103					//结束游戏后处理
#define IDI_DELETE_USER			    104					//删除用户信息
#define IDI_CHECK_GAME_END			105					//检测游戏结束(乱斗场)
#define IDI_HB_DELETE_END   		106					//删除红包定时器

#define TIME_CHECK_START				1				//检测游戏开始
#define TIME_CHECK_END				6					//检测桌子结束
#define TIME_CHECK_HB_SMAIL_END			(30)					//显示桌面红包数小于MAX_DISPLAY_HB_COUNT时，的结束时间(大乱斗)
#define TIME_CHECK_HB_BIG_END			(12)					//显示桌面红包数大于MAX_DISPLAY_HB_COUNT时，的结束时间(大乱斗)
#define TIME_HB_DELETE_END_SMAIL		(7)					//显示桌面红包数小于MAX_DISPLAY_HB_COUNT时，红包结算了，多长时间后把红包从桌面删除(大乱斗)
#define TIME_CHECK_GAME_END				(TIME_CHECK_HB_BIG_END/2)					//检测桌子结束(乱斗场)
#define TIME_HB_DELETE_END              (TIME_HB_DELETE_END_SMAIL/2)                    //删除红包时间
#define TIME_GAME_END_REAL			5					//结束后处理(普通场时间)
#define TIME_FA_HB				    10					//检查发红包
#define TIME_DELETE_USER				    (20*60)			//检查发红包(20分钟删除一次在线超过24小时的玩家信息)
//网络补偿时间
#define TIME_NETWORK_COMPENSATE     1					//网络补偿

//分值倍数
#define SCORE_TIME_BOMB				5					//炸弹倍数
#define SCORE_TIME_CHUANTIAN		2					//春天倍数
#define SCORE_TIME_FANCHUAN			2					//反春倍数

#define TIME_OUT_TRUSTEE_COUNT		0					//超时进入托管次数

////////////////////////////////////////////////////////////////////////
CT_LONGLONG CGameProcess::m_sllStockScore = 0;              //库存
CT_LONGLONG CGameProcess::m_sllGrabWinScore = 0;										//抢红包玩家赢的抢总和
CT_LONGLONG CGameProcess::m_sllFaWinScore = 0;										//发红包玩家赢的抢总和
CT_LONGLONG CGameProcess::m_sllTotalRealRevenue = 0;
std::map<CT_DWORD, MSG_G2DB_User_PointControl> CGameProcess::m_mapUserPointControl;
std::map<CT_DWORD, tagHBGameBlackList> CGameProcess::m_mapBlackList;
///////////////////////////////////////////////////////////////////////

bool CompareWinLoseScore(const GS_OnlinePlayerExploits & t1, const GS_OnlinePlayerExploits & t2)
{
    return t1.dWinLoseScore > t2.dWinLoseScore;
}
bool ComparePlayCount(const GS_OnlinePlayerExploits & t1, const GS_OnlinePlayerExploits & t2)
{
    return t1.wPlayCount > t2.wPlayCount;
}

///////////////////////////////////////////////////////////////////////
CGameProcess::CGameProcess(void) 
	:m_dwlCellScore(1), m_pGameDeskPtr(NULL), m_pGameRoomKindInfo(NULL), m_gen(m_rd()), m_iProba(0,99)
{
    m_pHBRoomConfig = NULL;
	m_dwHBIDIndex = 1;
	//清理游戏数据
	ClearGameData();
	//初始化数据
	InitGameData();

    m_cbLastRealPlayerCount = 0;
	m_ApplyList.clear();
	m_vHBRoomConfig.clear();					//红包房间配置
	m_vRobotGrabHBCfg.clear();					//机器人抢红包配置
	m_vRobotApplyFaHBCfg.clear();			//机器人申请发红包配置
	m_RobotQuitCfg.vQuitTable.clear();					//机器人退出配置
	m_StockKillScoreCfg.vKillScoreTable.clear();			//机器人库存杀分配置
    m_vFaHBCountVIPCfg.clear();
    m_vRobotFaHBAllotCfg.clear();
    m_dwRandIndex = 0;
    m_bCtrlMasterSwitch = false;
	m_iGrabLastOneHBProba = 0;
    m_dwOldTime = 0;
}

CGameProcess::~CGameProcess(void)
{
	m_ApplyList.clear();
}

//游戏开始
void CGameProcess::OnEventGameStart()
{

	//初始化数据
	InitGameData();
	//解释私人场数据
	//ParsePrivateData(m_PrivateTableInfo.szOtherParam);
	//更新游戏配置
	//UpdateGameConfig();
	//清除所有定时器
	ClearAllTimer();

    //设置游戏状态
	if(m_ApplyList.size() <= 0)
	{
        m_pGameDeskPtr->KillGameTimer(IDI_FA_HB);
        m_pGameDeskPtr->SetGameTimer(IDI_FA_HB, 5*1000);
		LOG(ERROR) << "OnEventGameStart m_ApplyList.size() <= 0";
		return;
	}

	bool bIsHasUserFaHB = false;
	CT_DWORD dwHBID = 0;
	CT_DWORD dwUserID = 0;
	do
	{
		dwHBID = m_ApplyList.begin()->stFaHBInfo.dwHBID;
		m_FaingHB[dwHBID] = (*(m_ApplyList.begin()));
		m_ApplyList.pop_front();

		CT_DWORD dwUserID1 = m_pGameDeskPtr->GetUserID(m_FaingHB[dwHBID].stFaHBInfo.wChairID);
		dwUserID = m_FaingHB[dwHBID].stFaHBInfo.dwUserID;
        if(m_FaingHB[dwHBID].stFaHBInfo.cbThunderNO == RAND_THUNDER_NO)
        {
            std::uniform_int_distribution<unsigned int> iRand(0,9);
			//m_FaingHB[dwHBID].stFaHBInfo.cbThunderNO = (RAND_NUM(m_dwRandIndex++)%10);
            m_FaingHB[dwHBID].stFaHBInfo.cbThunderNO = iRand(m_gen);
        }
        bIsHasUserFaHB = true;
        if(dwUserID1 != dwUserID && 0 == dwUserID)
        {
            bIsHasUserFaHB = false;
            LOG(ERROR) << "wChairID: " << (int)m_FaingHB[dwHBID].stFaHBInfo.wChairID << " dwUserID: " << dwUserID << " m_ApplyFaHB.dwUserID: " << m_FaingHB[dwHBID].stFaHBInfo.dwUserID \
            <<" needScore: "<<(int )m_FaingHB[dwHBID].stFaHBInfo.iHBAmount*TO_SCORE;
			std::map<CT_DWORD, tagHBInfo>::iterator it = m_FaingHB.find(dwHBID);
			m_FaingHB.erase(it);
        }
		//在线且存在且用户ID确认且金币大于发红包金额
		else if(dwUserID1 != dwUserID && (!m_mUserBaseData[dwUserID].bRobot))
		{
			m_FaingHB[dwHBID].stFaHBInfo.wChairID = m_pGameDeskPtr->GetUserChairID(dwUserID);
		}
	}while (m_ApplyList.size() > 0 && (!bIsHasUserFaHB));

    auto ithb = m_FaingHB.find(dwHBID);
	if((!bIsHasUserFaHB) || (ithb == m_FaingHB.end()))
	{
	    m_pGameDeskPtr->KillGameTimer(IDI_FA_HB);
	    m_pGameDeskPtr->SetGameTimer(IDI_FA_HB, 5*1000);
		//设置定时器，启动机器人发红包
		return;
	}

	if(SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID)
	{
        m_cbGameStatus = GAME_SCENE_PLAY;
        m_dwHBID = dwHBID;
	}

	//判断是否需要杀分
	//CT_INT32 iProbaKill = (RAND_NUM(m_dwRandIndex++)%100);
    for (CT_DWORD l = 0; l < m_StockKillScoreCfg.vKillScoreTable.size(); ++l)
    {
        if((m_sllStockScore < m_StockKillScoreCfg.vKillScoreTable[l].llTriggerStockUpperLimit && m_sllStockScore >= m_StockKillScoreCfg.vKillScoreTable[l].llTriggerStockLowerLimit)
        && (int)m_iProba(m_gen) <  m_StockKillScoreCfg.vKillScoreTable[l].iKillScoreProba)
        {
			ithb->second.stAssignHBInfo.iStockCtrlType = m_StockKillScoreCfg.vKillScoreTable[l].iCtrlType;
        }
    }
    //防止配置出错不杀分
    if((en_EatScore != ithb->second.stAssignHBInfo.iStockCtrlType) && m_sllStockScore < -1000000)
    {
		ithb->second.stAssignHBInfo.iStockCtrlType =  en_EatScore;
    }
	//判断参与抢红包的机器人数量
    CT_BYTE cbGrabHBRobotCount = 0;
    CT_DWORD i = 0;
    CT_DWORD j = 0;

	for(i = 0; i < m_vRobotGrabHBCfg.size(); i++)
	{
		if(m_vRobotGrabHBCfg[i].iHBAllotCount == ithb->second.stFaHBInfo.iHBAllotCount)
		{
			for(j = 0; j < m_vRobotGrabHBCfg[i].vRobotGrabHBTable.size(); ++j)
			{
                if(0 == m_vRobotGrabHBCfg[i].vRobotGrabHBTable[j].iRobotUpperLimit)
                {
                    continue;
                }
				if((m_cbLastRealPlayerCount >= m_vRobotGrabHBCfg[i].vRobotGrabHBTable[j].iRealLowerLimit)
				    &&(m_cbLastRealPlayerCount<=m_vRobotGrabHBCfg[i].vRobotGrabHBTable[j].iRealUpperLimit))
				{
                    cbGrabHBRobotCount = RAND(m_vRobotGrabHBCfg[i].vRobotGrabHBTable[j].iRobotLowerLimit,
					        m_vRobotGrabHBCfg[i].vRobotGrabHBTable[j].iRobotUpperLimit,m_dwRandIndex++);
                    //LOG(ERROR) << "RAND cbGrabHBRobotCount: " << (int)cbGrabHBRobotCount<< " j:"<<j;
                    break;
				}
			}
		}
		if(0 != cbGrabHBRobotCount)
        {
            break;
        }
	}
	//判断该玩家是否在点控列表内
	//int iPointCtrlProba = RAND_NUM(m_dwRandIndex++)%100;
    int iPointCtrlProba = (int)m_iProba(m_gen);
	auto it = m_mapUserPointControl.find(ithb->second.stFaHBInfo.dwUserID);
	if(it != m_mapUserPointControl.end() && (it->second.iDianKongZhi != en_NoCtrl) &&(iPointCtrlProba <  m_PointCtrlCfg.iExeProba))
	{
		ithb->second.stAssignHBInfo.iFaPointCtrlType = it->second.iDianKongZhi;
	}
	else if(it != m_mapUserPointControl.end() && (it->second.iDianKongZhi == en_PlayerOutScore) && (en_OutScore == ithb->second.stAssignHBInfo.iStockCtrlType))
    {
        ithb->second.stAssignHBInfo.iStockCtrlType = en_NoCtrl;
    }
	else if(it != m_mapUserPointControl.end() && (it->second.iDianKongZhi == en_PlayerEatScore) && (en_EatScore == ithb->second.stAssignHBInfo.iStockCtrlType))
    {
        ithb->second.stAssignHBInfo.iStockCtrlType = en_NoCtrl;
    }

    ithb->second.stAssignHBInfo.llTotalAmount = ithb->second.stFaHBInfo.iHBAmount*TO_SCORE;
	m_GameLogic.RandAssignHB(m_FaingHB[dwHBID]);
	m_mRealPlayerCount[dwHBID] = 0;

    if(m_mUserBaseData[dwUserID].bRobot)
    {
        m_sllStockScore -= (ithb->second.stFaHBInfo.iHBAmount*TO_SCORE);
    }

	CMD_S_StartGameGrabHB StartGameGrabHB;
	memset(&StartGameGrabHB, 0, sizeof(StartGameGrabHB));
	StartGameGrabHB.stPlayerApply.dwHBID = dwHBID;
	StartGameGrabHB.stPlayerApply.dwUserID = dwUserID;
	StartGameGrabHB.stPlayerApply.wChairID = ithb->second.stFaHBInfo.wChairID;
	StartGameGrabHB.stPlayerApply.cbVipLevel = m_mUserBaseData[dwUserID].cbVipLevel;
	StartGameGrabHB.stPlayerApply.cbHeadIndex = m_mUserBaseData[dwUserID].cbHeadIndex;
	StartGameGrabHB.stPlayerApply.cbGender = m_mUserBaseData[dwUserID].cbGender;
	StartGameGrabHB.stPlayerApply.cbThunderNO = ithb->second.stFaHBInfo.cbThunderNO;
	StartGameGrabHB.stPlayerApply.iFaHBTotalAmount = ithb->second.stFaHBInfo.iHBAmount;
	StartGameGrabHB.stPlayerApply.iHBAllotCount = ithb->second.stFaHBInfo.iHBAllotCount;                      //发的红包可以分配成多少个子包（包数
	StartGameGrabHB.stPlayerApply.fMultiple = ithb->second.stFaHBInfo.dMultiple;				            //倍数
	ithb->second.dwGrabTime = time(NULL);

	if(m_pGameRoomKindInfo->wRoomKindID != SENIOR_ROOM)
	{
		StartGameGrabHB.wTimeLeft = TIME_CHECK_END;
        m_pGameDeskPtr->SetGameTimer(IDI_CHECK_END, (TIME_CHECK_END + TIME_NETWORK_COMPENSATE)*1000);
	}
	else
	{
	    //乱斗场不在此处设置游戏结算定时器
		StartGameGrabHB.wTimeLeft = ((m_FaingHB.size() > MAX_DISPLAY_HB_COUNT) ? TIME_CHECK_HB_BIG_END: TIME_CHECK_HB_SMAIL_END);
	}
	SendTableData(INVALID_CHAIR, SUB_S_GRAB_HB, &StartGameGrabHB, sizeof(StartGameGrabHB),false);
	m_mOpStartTime[dwHBID] = time(NULL);
	m_mOpTotalTime[dwHBID] = StartGameGrabHB.wTimeLeft + TIME_NETWORK_COMPENSATE;
	if((m_pGameRoomKindInfo->wRoomKindID != SENIOR_ROOM) && m_ApplyList.size()>0)
	{
		CMD_S_PlayerApply nextFaHBPlayer;
		nextFaHBPlayer.dwUserID = m_ApplyList.begin()->stFaHBInfo.dwUserID;
		nextFaHBPlayer.dwHBID = m_ApplyList.begin()->stFaHBInfo.dwHBID;
		nextFaHBPlayer.wChairID = m_ApplyList.begin()->stFaHBInfo.wChairID;							//发红包玩家id
		nextFaHBPlayer.cbGender = m_mUserBaseData[nextFaHBPlayer.dwUserID].cbGender;							//性别
		nextFaHBPlayer.cbVipLevel = m_mUserBaseData[nextFaHBPlayer.dwUserID].cbVipLevel;							//VIP等级
		nextFaHBPlayer.cbHeadIndex = m_mUserBaseData[nextFaHBPlayer.dwUserID].cbHeadIndex;						//头像索引
		nextFaHBPlayer.cbThunderNO = m_ApplyList.begin()->stFaHBInfo.cbThunderNO;						//中雷号
		nextFaHBPlayer.iFaHBTotalAmount = m_ApplyList.begin()->stFaHBInfo.iHBAmount;					//红包总金额
		nextFaHBPlayer.iHBAllotCount = m_ApplyList.begin()->stFaHBInfo.iHBAllotCount;                      //发的红包可以分配成多少个子包（包数
		nextFaHBPlayer.fMultiple = m_ApplyList.begin()->stFaHBInfo.dMultiple;				            //倍数
		SendTableData(INVALID_CHAIR, SUB_S_NEXT_FA_HB_PLAYER, &nextFaHBPlayer, sizeof(nextFaHBPlayer),false);
	}

	//通知几个机器人抢红包
	if(cbGrabHBRobotCount > 0)
    {
        CMD_S_NoticeRobotGrabHB NoticeRobotGrabHB;
        memset(&NoticeRobotGrabHB, 0, sizeof(NoticeRobotGrabHB));
        NoticeRobotGrabHB.wTimeLeft = StartGameGrabHB.wTimeLeft;
        NoticeRobotGrabHB.dwHBID = dwHBID;
        std::vector<CT_WORD> vGrabBRobot;
        CT_DWORD iRobotSize = m_vRobotChairID.size();
        if(cbGrabHBRobotCount > iRobotSize)
        {
            //防止下面死循环
            cbGrabHBRobotCount = iRobotSize;
            //LOG(ERROR) << "iRobotSize cbGrabHBRobotCount: " << (int)cbGrabHBRobotCount;
        }
        for(CT_DWORD z = 0; z < (CT_DWORD)cbGrabHBRobotCount && z < iRobotSize && i < m_vRobotGrabHBCfg.size() && j < m_vRobotGrabHBCfg[i].vRobotGrabHBTable.size(); z++)
        {
        	CT_WORD wUpperLimit = m_vRobotGrabHBCfg[i].vRobotGrabHBTable[j].iRobotTimeUpperLimit;
            CT_WORD wLowerLimit = m_vRobotGrabHBCfg[i].vRobotGrabHBTable[j].iRobotTimeLowerLimit;
            if(0 == wUpperLimit)
            {
                continue;
            }

            NoticeRobotGrabHB.wGrabTime = RAND(wLowerLimit, wUpperLimit, m_dwRandIndex++);
            if(NoticeRobotGrabHB.wGrabTime <= 0)
            {
                NoticeRobotGrabHB.wGrabTime = 1;
            }
            //随机一个机器人抢红包，防止重复
            for (int l = 0; l < GAME_PLAYER; ++l)
            {
                CT_DWORD k = 0;
                NoticeRobotGrabHB.wChairID = m_vRobotChairID[RAND_NUM(m_dwRandIndex++)%iRobotSize];
                if(!m_pGameDeskPtr->IsExistUser(NoticeRobotGrabHB.wChairID))
                {
                    continue;
                }
                for (k = 0; k < vGrabBRobot.size(); ++k)
                {
                    if(NoticeRobotGrabHB.wChairID == vGrabBRobot[k] || (m_pGameDeskPtr->GetUserScore(NoticeRobotGrabHB.wChairID)*TO_DOUBLE < (StartGameGrabHB.stPlayerApply.iFaHBTotalAmount*StartGameGrabHB.stPlayerApply.fMultiple)))
                    {
                        break;
                    }
                }
                if(k == vGrabBRobot.size())
                {
                    vGrabBRobot.push_back(NoticeRobotGrabHB.wChairID);
                    break;
                }
                if(vGrabBRobot.size() >= iRobotSize)
                {
                    break;
                }
            }
            SendTableData(NoticeRobotGrabHB.wChairID, SUB_S_NOTICE_ROBOT_GRAB_HB, &NoticeRobotGrabHB, sizeof(NoticeRobotGrabHB), false);
        }
    }
	return;
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//清除所有定时器
	//ClearAllTimer();
    if(SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID)
    {
        m_cbGameStatus = GAME_SCENE_END;
    }

	m_pGameDeskPtr->KillGameTimer(IDI_CHECK_END);
	switch (GETag)
	{
	case GER_NORMAL:
	{//正常退出
		//红包ID
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

	//结束直接T人
	//for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	//{
	//	//斗地主游戏剔除用户时不需要广播状态给客户端
	//	m_pGameDeskPtr->ClearTableUser(i, false);
	//}
	
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
        case GAME_SCENE_END:
		{
			CMD_S_StatusFree StatusFree;
			memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
			StatusFree.stHBRoomBaseCfg = *m_pHBRoomConfig;
			StatusFree.dwCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
			int iVipLevel2 = 0;
			if(m_pGameDeskPtr->GetUserBaseData(dwChairID))
            {
                iVipLevel2 = m_pGameDeskPtr->GetUserBaseData(dwChairID)->cbVipLevel2;
            }
			if(iVipLevel2 < MAX_VIP_LEVEL && iVipLevel2 >= 0)
			{
				StatusFree.stVIPFaHBcfg = m_vFaHBCountVIPCfg[iVipLevel2];
			}
			memcpy(&(StatusFree.stHBButtonCfg), &m_HBButtonCfg, sizeof(StatusFree.stHBButtonCfg));
			//发送数据
			SendTableData(dwChairID, SC_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree),false);
			break;
		}
		case GAME_SCENE_PLAY:	//游戏状态
		{
            auto ithb = m_FaingHB.find(m_dwHBID);
            if(ithb == m_FaingHB.end())
            {
                LOG(ERROR) << "GAME_SCENE_PLAY error" << m_dwHBID;
                return;
            }

			//构造数据
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay, 0, sizeof(StatusPlay));
			//单元积分
			StatusPlay.stHBRoomBaseCfg = *m_pHBRoomConfig;
			//剩余时间 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_mOpTotalTime[m_dwHBID] - (dwNow - m_mOpStartTime[m_dwHBID]);
			StatusPlay.wLeftTime = sLeftTime > 0 ? sLeftTime : 0;
			StatusPlay.cbThunderNO = ithb->second.stFaHBInfo.cbThunderNO;
			StatusPlay.wCurrFaHBChairID =  ithb->second.stFaHBInfo.wChairID;
			StatusPlay.iFaHBTotalAmount = ithb->second.stFaHBInfo.iHBAmount;
			StatusPlay.cbSurplusHBCount = ithb->second.stAssignHBInfo.wRemainHBCount;	//剩余未被抢红包数量
			int index = INVALID_CHAIR;

			for(CT_DWORD i = 0; i < ithb->second.vOpenHBList.size()&& (i < MAX_HB_COUNT); ++i)
			{
				if(ithb->second.vOpenHBList[i].wChairID == ithb->second.stFaHBInfo.wChairID)
				{
					index = i;
				}
				CT_DWORD dwUserID = ithb->second.vOpenHBList[i].dwUserID;
                auto pUserInfo = m_mUserBaseData.find(dwUserID);
                if(pUserInfo == m_mUserBaseData.end())
                {
                    LOG(ERROR) << "GAME_SCENE_PLAY Grab User Info Null uid:" << dwUserID;
                    continue;
                }
				StatusPlay.stAlreadyGrabHBPlayer[i].dwUserID = dwUserID;
				StatusPlay.stAlreadyGrabHBPlayer[i].dWinningAmount = ithb->second.vOpenHBList[i].dWinningAmount;
				StatusPlay.stAlreadyGrabHBPlayer[i].dWinLoseScore = ithb->second.vOpenHBList[i].dWinLoseScore;
				StatusPlay.stAlreadyGrabHBPlayer[i].dScore = pUserInfo->second.llScore*TO_DOUBLE;
				StatusPlay.stAlreadyGrabHBPlayer[i].cbGender = pUserInfo->second.cbGender;
				StatusPlay.stAlreadyGrabHBPlayer[i].cbHeadIndex = pUserInfo->second.cbHeadIndex;
				StatusPlay.stAlreadyGrabHBPlayer[i].cbVipLevel = pUserInfo->second.cbVipLevel;
                StatusPlay.stAlreadyGrabHBPlayer[i].cbIsThunder = ithb->second.vOpenHBList[i].cbIsThunder;
			}

			StatusPlay.stFaHBPlayer.dwUserID = ithb->second.stFaHBInfo.dwUserID;
			if(index != INVALID_CHAIR)
			{
				StatusPlay.stFaHBPlayer.dWinningAmount = ithb->second.vOpenHBList[index].dWinningAmount;
				StatusPlay.stFaHBPlayer.dWinLoseScore = ithb->second.vOpenHBList[index].dWinLoseScore;
			}
            auto pFaUserInfo = m_mUserBaseData.find(StatusPlay.stFaHBPlayer.dwUserID);
            if(pFaUserInfo == m_mUserBaseData.end())
            {
                LOG(ERROR) << "GAME_SCENE_PLAY fa  User Info Null uid:" << StatusPlay.stFaHBPlayer.dwUserID;
                SendTableData(dwChairID, SC_GAMESCENE_PLAY, &StatusPlay, sizeof(StatusPlay),false);
                break;
            }
			StatusPlay.stFaHBPlayer.dScore = pFaUserInfo->second.llScore;
			StatusPlay.stFaHBPlayer.cbGender = pFaUserInfo->second.cbGender;
			StatusPlay.stFaHBPlayer.cbHeadIndex = pFaUserInfo->second.cbHeadIndex;
			StatusPlay.stFaHBPlayer.cbVipLevel = pFaUserInfo->second.cbVipLevel;
            int iVipLevel2 = 0;
            if(m_pGameDeskPtr->GetUserBaseData(dwChairID))
            {
                iVipLevel2 = m_pGameDeskPtr->GetUserBaseData(dwChairID)->cbVipLevel2;
            }
			if(iVipLevel2 < MAX_VIP_LEVEL && iVipLevel2 >= 0)
			{
				StatusPlay.stVIPFaHBcfg = m_vFaHBCountVIPCfg[iVipLevel2];
			}
			StatusPlay.dwHBID = m_dwHBID;
			memcpy(&(StatusPlay.stHBButtonCfg), &m_HBButtonCfg, sizeof(StatusPlay.stHBButtonCfg));
			//发送场景
			SendTableData(dwChairID, SC_GAMESCENE_PLAY, &StatusPlay, sizeof(StatusPlay),false);
			break;
		}
		/*case GAME_SCENE_END://这个场景只有几秒时间
		{
			//场景数据
			CMD_S_StatusEND StatusEnd;
			memset(&StatusEnd, 0, sizeof(CMD_S_StatusEND));
			//剩余时间
			CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
			StatusEnd.cbTimeLeave = (CT_BYTE)(m_cbOpTotalTime - std::min<CT_BYTE>(dwPassTime, m_cbOpTotalTime));
			StatusEnd.wCurrFaHBChairID = m_FaingHB[m_dwHBID].stFaHBInfo.wChairID;
			StatusEnd.iFaHBTotalAmount = m_FaingHB[m_dwHBID].stFaHBInfo.iHBAmount;
			StatusEnd.stHBRoomBaseCfg = *m_pHBRoomConfig;
			StatusEnd.cbNoGrabHBCount = m_mGameEnd[m_dwHBID].cbNoGrabHBCount;
			StatusEnd.dBackTotalAmount =  m_mGameEnd[m_dwHBID].dBackTotalAmount;
			StatusEnd.dGrabedTotalAmount = m_mGameEnd[m_dwHBID].dGrabedTotalAmount;
			StatusEnd.cbGrabedHBCount = m_mGameEnd[m_dwHBID].cbGrabedHBCount;
			StatusEnd.dThunderBackTotalAmount = m_mGameEnd[m_dwHBID].dThunderBackTotalAmount;
			for(int i = 0; i < m_FaingHB[m_dwHBID].vOpenHBList.size()&& (i < MAX_HB_COUNT); ++i)
			{
				CT_DWORD dwUserID = m_FaingHB[m_dwHBID].vOpenHBList[i].dwUserID;
				StatusEnd.stGrabHBPlayer[i].dwUserID = dwUserID;
				StatusEnd.stGrabHBPlayer[i].dWinningAmount = m_FaingHB[m_dwHBID].vOpenHBList[i].dWinningAmount;
				StatusEnd.stGrabHBPlayer[i].dThunderBackAmount = m_FaingHB[m_dwHBID].vOpenHBList[i].dThunderBackAmount;
				StatusEnd.stGrabHBPlayer[i].dScore = m_mUserBaseData[dwUserID].llScore*TO_DOUBLE;
				StatusEnd.stGrabHBPlayer[i].cbGender = m_mUserBaseData[dwUserID].cbGender;
				StatusEnd.stGrabHBPlayer[i].cbHeadIndex = m_mUserBaseData[dwUserID].cbHeadIndex;
				StatusEnd.stGrabHBPlayer[i].cbVipLevel = m_mUserBaseData[dwUserID].cbVipLevel;
			}
			int ivipLevel2 = 0;
            if(m_pGameDeskPtr->GetUserBaseData(dwChairID))
            {
                ivipLevel2 = m_pGameDeskPtr->GetUserBaseData(dwChairID)->cbVipLevel2;
            }
			if(ivipLevel2 < MAX_VIP_LEVEL && ivipLevel2 >= 0)
			{
				StatusEnd.stVIPFaHBcfg = m_vFaHBCountVIPCfg[ivipLevel2];
			}
			//发送结算状态数据
			SendTableData(dwChairID, SC_GAMESCENE_END, &StatusEnd, sizeof(CMD_S_StatusEND), false);
			break;
		}*/
		case GAME_SCENE_GAME:
		{
			CMD_S_GamePlay GamePlay;
			memset(&GamePlay, 0, sizeof(GamePlay));
            int iVipLevel2 = 0;
            if(m_pGameDeskPtr->GetUserBaseData(dwChairID))
            {
                iVipLevel2 = m_pGameDeskPtr->GetUserBaseData(dwChairID)->cbVipLevel2;
            }
            if(iVipLevel2 < MAX_VIP_LEVEL && iVipLevel2 >= 0)
            {
                GamePlay.stVIPFaHBcfg = m_vFaHBCountVIPCfg[iVipLevel2];
            }
            memcpy(&(GamePlay.stHBButtonCfg), &m_HBButtonCfg, sizeof(GamePlay.stHBButtonCfg));
			for (CT_DWORD k = 0; k < MAX_CONFIG && k < m_vHBRoomConfig.size(); ++k)
			{
				GamePlay.stHBRoomBaseCfg[k].iHBAllotCount = m_vHBRoomConfig[k].iHBAllotCount;
				GamePlay.stHBRoomBaseCfg[k].fMultiple = m_vHBRoomConfig[k].fMultiple;
				GamePlay.stHBRoomBaseCfg[k].dwHBUpperLimit = m_vHBRoomConfig[k].dwHBUpperLimit;
				GamePlay.stHBRoomBaseCfg[k].dwHBLowerLimit = m_vHBRoomConfig[k].dwHBLowerLimit;
			}
			GamePlay.dTotalColorPotAmount=m_GameLogic.GetColorPool()*TO_DOUBLE;
			SendTableData(dwChairID, SC_GAMESCENE_GAME, &GamePlay, sizeof(CMD_S_GamePlay), false);

			CT_BYTE  szBuffer[4096];
			memset(szBuffer,0, sizeof(szBuffer));
			CT_DWORD dwSendSize = 0;
			//SendRankings(dwChairID);

			CT_WORD wTime = 0;
			if(m_FaingHB.size() > MAX_DISPLAY_HB_COUNT)
			{
				wTime = TIME_CHECK_HB_BIG_END;
			}
			else
			{
				wTime = TIME_CHECK_HB_SMAIL_END;
			}

            auto it = m_FaingHB.rbegin();
			while(it != m_FaingHB.rend())
			{
				memset(szBuffer,0, sizeof(szBuffer));
				dwSendSize = 0;
				for (CT_DWORD z = 0; it != m_FaingHB.rend() && z < 5; ++it, ++z)
				{
					CMD_S_SendDisplayHB SendDisplayHB;
					memset(&SendDisplayHB, 0, sizeof(SendDisplayHB));
					SendDisplayHB.stFaHBPlayer.dwUserID = it->second.stFaHBInfo.dwUserID;							//用户id
                    auto pFaUserInfo = m_mUserBaseData.find(SendDisplayHB.stFaHBPlayer.dwUserID);
                    if(pFaUserInfo == m_mUserBaseData.end())
                    {
                        LOG(ERROR) << "GAME_SCENE_GAME User Info Null uid:" << SendDisplayHB.stFaHBPlayer.dwUserID;
                        continue;
                    }
					SendDisplayHB.stFaHBPlayer.cbGender = pFaUserInfo->second.cbGender;							//性别
					SendDisplayHB.stFaHBPlayer.cbVipLevel = pFaUserInfo->second.cbVipLevel;							//VIP等级
					SendDisplayHB.stFaHBPlayer.cbHeadIndex = pFaUserInfo->second.cbHeadIndex;						//头像索引
					SendDisplayHB.stFaHBPlayer.dScore = pFaUserInfo->second.llScore;							    //金币

					SendDisplayHB.FaingHB = it->second.stFaHBInfo;

					SendDisplayHB.cbSurplusHBCount = it->second.stAssignHBInfo.wRemainHBCount;

					SendDisplayHB.wLeftTime = (wTime - (time(NULL) - it->second.dwGrabTime));

					for (CT_DWORD j = 0; j < it->second.vOpenHBList.size(); ++j)
					{
						if(it->second.vOpenHBList[j].dwUserID == m_pGameDeskPtr->GetUserID(dwChairID))
						{
							SendDisplayHB.cbIsSelfGrab = true;
                            break;
						}
					}
                    //LOG(ERROR) << SendDisplayHB.stFaHBPlayer.dwUserID << " " <<SendDisplayHB.stFaHBPlayer.dScore << " " << time(NULL) <<" " << vHBInfo[i].dwGrabTime << " " << SendDisplayHB.wLeftTime;
                    memcpy(szBuffer+dwSendSize, &SendDisplayHB, sizeof(SendDisplayHB));
                    dwSendSize += sizeof(CMD_S_SendDisplayHB);
				}
                //LOG(ERROR) << "vHBInfo.size() " << vHBInfo.size()<< " m_FaingHB.size() " << m_FaingHB.size() << " i " << i;
				SendTableData(dwChairID, SUB_S_SEND_DISPLAY_HB, szBuffer, dwSendSize, false);
			}
		}
		break;
		default:
			break;
	}
	return ;
}
//游戏结束后处理
void CGameProcess::GameEndReal()
{
	CT_BOOL bIsDelete = false;
    //机器人退出
    for (CT_DWORD i = 0; i < m_vRobotChairID.size(); ++i)
    {
        //正在发红包的不退出
        if(IsInApplyListByUserID(m_pGameDeskPtr->GetUserID(m_vRobotChairID[i])))
        {
            //LOG(ERROR)<< "GameEndReal ChairID: " << (int)m_vRobotChairID[i];
            continue;
        }
        if(m_pGameDeskPtr->IsAndroidUser(m_vRobotChairID[i]) && (m_pGameDeskPtr->GetUserScore(m_vRobotChairID[i]) < m_RobotQuitCfg.llRemainScore))
        {
			DeleteUserByChairID(m_vRobotChairID[i]);
			bIsDelete = true;
            continue;
        }
        if(!m_pGameDeskPtr->GetUserScoreData(m_vRobotChairID[i]))
        {
            continue;
        }

        CT_DWORD dwTime = (time(NULL) - m_pGameDeskPtr->GetUserScoreData(m_vRobotChairID[i])->dwEnterTime);
        for (CT_DWORD j = m_RobotQuitCfg.vQuitTable.size()-1; (j < m_RobotQuitCfg.vQuitTable.size()) && j >= 0; --j)
        {
			//CT_INT32 iRand = RAND_NUM(m_dwRandIndex++)%100;
            if((dwTime > (CT_DWORD)(m_RobotQuitCfg.vQuitTable[j].iOnlineTime*60)) && (int)m_iProba(m_gen) < m_RobotQuitCfg.vQuitTable[j].iQuitProba)
            {
				DeleteUserByChairID(m_vRobotChairID[i]);
				bIsDelete = true;
            }
        }
        if(m_RobotQuitCfg.vQuitTable.size() <= 0)
        {
            LOG(ERROR) << " m_RobotQuitCfg.vQuitTable.size() <= 0";
        }
    }
    if(SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID)
	{
		m_cbGameStatus = GAME_SCENE_FREE;
	}

	if(bIsDelete)
	{
		StatisticalRobotLocationInfo();
	}
	RobotFaHB();

    //开始游戏
    if(m_ApplyList.size() > 0)
    {
        OnEventGameStart();
    }
    return;
}
//定时器事件
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	//LOG(WARNING) << "run timer, timer id: " << dwTimerID;

	m_pGameDeskPtr->KillGameTimer(dwTimerID);

	switch (dwTimerID)
	{
    case IDI_CHECK_START:
    {
        //开始游戏
        if(m_ApplyList.size() > 0 && m_FaingHB.size() < MAX_DISPLAY_HB_COUNT)
        {
            OnEventGameStart();
        }

       if(SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID && m_ApplyList.size() > 0)
        {
            m_pGameDeskPtr->SetGameTimer(IDI_CHECK_START, TIME_CHECK_START*1000);
        }
    }
    break;
	case IDI_CHECK_END:
    case IDI_CHECK_GAME_END://此定时器全程运行，不Kill
	{
	    CT_WORD wTimes = (CT_WORD)((m_FaingHB.size() > MAX_DISPLAY_HB_COUNT) ? TIME_CHECK_HB_BIG_END: TIME_CHECK_HB_SMAIL_END);
		CT_DWORD dwTime = time(NULL);
		for (auto it = m_FaingHB.begin(); it != m_FaingHB.end();)
		{
		    if(0 == it->second.stFaHBInfo.dwUserID || 0 == it->second.stFaHBInfo.dwHBID || 0 == it->first)
            {
		        LOG(ERROR) << "IDI_CHECK_GAME_END dwUserID: " << it->second.stFaHBInfo.dwUserID << " dwHBID: " << it->second.stFaHBInfo.dwHBID << " first: " << it->first;
                it = m_FaingHB.erase(it);
                continue;
            }

			if((m_pGameRoomKindInfo->wRoomKindID == SENIOR_ROOM) && (0 == it->second.dwEndTime) && ((dwTime - it->second.dwGrabTime) >= wTimes))
			{
			    auto its = it;
			    it++;
				OnEventGameEnd(its->first, GER_NORMAL);
			}
			else if((m_pGameRoomKindInfo->wRoomKindID != SENIOR_ROOM) && ((dwTime - it->second.dwGrabTime) >= TIME_CHECK_END) && (0 == it->second.dwEndTime) && (GAME_SCENE_PLAY == m_cbGameStatus))
			{
                auto its = it;
                it++;
				OnEventGameEnd(its->first, GER_NORMAL);
			}
			else if(m_pGameRoomKindInfo->wRoomKindID != SENIOR_ROOM)
            {
                m_pGameDeskPtr->SetGameTimer(IDI_CHECK_END, 1*1000);
                LOG(ERROR) << "dwTime - it->second.dwGrabTime: " << dwTime - it->second.dwGrabTime << " TIME_CHECK_END: " << (int)TIME_CHECK_END << " dwEndTime: " << it->second.dwEndTime << " m_cbGameStatus:" << (int)m_cbGameStatus;
				it++;
            }
			else
            {
                it++;
            }
		}

		if(IDI_CHECK_GAME_END == dwTimerID)
        {
            m_pGameDeskPtr->SetGameTimer(IDI_CHECK_GAME_END, TIME_CHECK_GAME_END*1000);
        }
	}
	break;
	case IDI_GAME_END_REAL:
    {
		GameEndReal();
    }
    break;
    case IDI_HB_DELETE_END:
    {
        //删除待在桌面的已经结算的红包
        if(SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID)
        {
            DeleteHBByHBID();
        }

        m_pGameDeskPtr->SetGameTimer(IDI_HB_DELETE_END, TIME_HB_DELETE_END*1000);
    }
    break;
    case IDI_FA_HB://此定时器全程运行，不Kill
    {
        if(m_vRobotChairID.size() <= 7)
        {
			StatisticalRobotLocationInfo();
        }

		RobotFaHB();
        m_pGameDeskPtr->SetGameTimer(IDI_FA_HB, TIME_FA_HB*1000);

    }
    break;
    case IDI_DELETE_USER://此定时器全程运行，不Kill
    {
        //LOG(INFO)<< "OnTimer IDI_DELETE_USER: time";
        //删除真人信息
        CT_DWORD dwTime = time(NULL);
        for (auto it3 = m_mUserBaseData.begin(); it3 != m_mUserBaseData.end();)
        {
            if((!it3->second.bRobot) && ((dwTime - it3->second.dwFirstTime) > (60*60*24)) && (INVALID_CHAIR == m_pGameDeskPtr->GetUserChairID(it3->second.dwUserID)))
            {
                it3 = m_mUserBaseData.erase(it3);
            }
            else if(0 == it3->first)
            {
                LOG(ERROR)<<"IDI_DELETE_USER uid: " << it3->first;
                it3 = m_mUserBaseData.erase(it3);
            }
            else
            {
                it3++;
            }
        }
        m_pGameDeskPtr->SetGameTimer(IDI_DELETE_USER, TIME_DELETE_USER*1000);
    }
    break;
	default:
		break;
	}

}
//游戏消息
CT_BOOL CGameProcess::OpenHB(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	//状态效验
	//assert(m_cbGameStatus == GAME_SCENE_PLAY);
	if (SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID  && m_cbGameStatus != GAME_SCENE_PLAY) return true;
	CMD_C_OpenHB * pNetInfo = (CMD_C_OpenHB *)pDataBuffer;
	if(dwDataSize != sizeof(CMD_C_OpenHB))
	{
		LOG(ERROR) << "OpenHB  dwDataSize:" << (int)dwDataSize <<" != " << "sizeof(CMD_C_OpenHB):"<<(int)sizeof(CMD_C_OpenHB);
		return false;
	}
	if((!m_pGameDeskPtr->IsAndroidUser(pNetInfo->wChairID)) && (pNetInfo->dwUserID != m_pGameDeskPtr->GetUserID(pNetInfo->wChairID)))
	{
		LOG(ERROR) << "OpenHB wChairID: " <<  pNetInfo->wChairID << " dwUserID: " << pNetInfo->dwUserID;
		return false;
	}
	CT_DWORD dwHBID = pNetInfo->dwHBID;
	CMD_S_OpenHB OpenHBResponse;
    OpenHBResponse.cbIsFail = 0;
    OpenHBResponse.dwUserID = m_pGameDeskPtr->GetUserID(wChairID);
    OpenHBResponse.wChairID = wChairID;
    OpenHBResponse.dwHBID = dwHBID;
    auto pGrabUserInfo = m_mUserBaseData.find(OpenHBResponse.dwUserID);
    if(pGrabUserInfo == m_mUserBaseData.end())
    {
        LOG(ERROR) << "OpenHB error userInfo";
        return false;
    }
    auto ithb = m_FaingHB.find(dwHBID);
    if(ithb == m_FaingHB.end())
    {
        //LOG(ERROR) << "OpenHB uid:" << m_pGameDeskPtr->GetUserID(wChairID) << " hid: " << dwHBID;
        OpenHBResponse.cbIsFail = 4;//已经结算了
    }
	else if((OpenHBResponse.dwUserID != ithb->second.stFaHBInfo.dwUserID)&&(ithb->second.stFaHBInfo.iHBAmount * (ithb->second.stFaHBInfo.dMultiple) * TO_SCORE) > m_pGameDeskPtr->GetUserScore(wChairID))
	{
		OpenHBResponse.cbIsFail = 3;    //钱不足
	}
	else if	(ithb->second.stAssignHBInfo.wRemainHBCount <= 0)
	{
		OpenHBResponse.cbIsFail = 2;
	}
	else if(0 != ithb->second.dwEndTime)
	{
		OpenHBResponse.cbIsFail = 4;//已经结算了
	}

    //int iRand = disRand(m_gen);
	if(!OpenHBResponse.cbIsFail)
    {
        for (std::vector<CMD_S_OpenHB>::iterator it = ithb->second.vOpenHBList.begin(); it != ithb->second.vOpenHBList.end() && (!OpenHBResponse.cbIsFail); ++it)
        {
            if(it->dwUserID == OpenHBResponse.dwUserID)
            {
                OpenHBResponse.cbIsFail = 1;//已经抢过红包了
            }
        }

        static int s_iGrabHBCount = 0;
        if(s_iGrabHBCount > 4)
		{
			//判断该玩家是否在点控列表内
			//int iPointCtrlProba = RAND_NUM(m_dwRandIndex++)%100;
			auto it = m_mapUserPointControl.find(ithb->second.stFaHBInfo.dwUserID);
			if(it != m_mapUserPointControl.end() && (it->second.iDianKongZhi != en_NoCtrl) &&((int)m_iProba(m_gen) <  m_PointCtrlCfg.iExeProba))
			{
				ithb->second.stAssignHBInfo.iFaPointCtrlType = it->second.iDianKongZhi;
			}
			s_iGrabHBCount = 0;
		}
        else
        {
            s_iGrabHBCount++;
        }
    }

	auto pBlackUser = m_mapBlackList.find(OpenHBResponse.dwUserID);
	int index = MAX_HB_COUNT;
    CT_LONGLONG llTotalWin = 0;
	auto userCtrl = m_mapUserPointControl.find(OpenHBResponse.dwUserID);
	if((0 == OpenHBResponse.cbIsFail) && (ithb->second.stAssignHBInfo.wRemainHBCount > 0))
	{
		index = m_GameLogic.RandAssignHB2(ithb->second, ((userCtrl != m_mapUserPointControl.end()) ? userCtrl->second.iDianKongZhi : en_NoCtrl),
		        m_pGameDeskPtr->IsAndroidUser(wChairID), m_mUserBaseData[ithb->second.stFaHBInfo.dwUserID].bRobot,
		        (OpenHBResponse.dwUserID == ithb->second.stFaHBInfo.dwUserID), m_sllGrabWinScore, pGrabUserInfo->second);
		if (MAX_HB_COUNT != index)
		{
			//抢最后一个红包处理黑名单用户处理
			if((!pGrabUserInfo->second.bRobot) && (pBlackUser != m_mapBlackList.end()) && (ithb->second.stAssignHBInfo.iRobotGrabCount > 0)
			&& (0 == ithb->second.stAssignHBInfo.llRemainAmount) && (0 == ithb->second.stAssignHBInfo.wRemainHBCount)
			&& (!ithb->second.stAssignHBInfo.bIsThunder[index]) && (m_iProba(m_gen) < pBlackUser->second.wUserLostRatio))
			{
				int iRobotIndex = INVALID_CHAIR;
				for (size_t m = 0; m < ithb->second.vOpenHBList.size(); ++m)
				{
					auto pUser = m_mUserBaseData.find(ithb->second.vOpenHBList[m].dwUserID);
					if(pUser != m_mUserBaseData.end() && pUser->second.bRobot)
					{
						iRobotIndex = m;
						break;
					}
				}
				if(INVALID_CHAIR != iRobotIndex)
				{
					GrabLastOneHBBlackListUserDear(ithb->second, index, iRobotIndex, (OpenHBResponse.dwUserID == ithb->second.stFaHBInfo.dwUserID));
				}
			}
		    llTotalWin = ithb->second.stAssignHBInfo.llOutHB[index] - ithb->second.stAssignHBInfo.llThunderBackAmount[index] + ithb->second.stAssignHBInfo.llGrabWinColorPoolAmount[index];
			if (llTotalWin > 0 && OpenHBResponse.dwUserID != ithb->second.stFaHBInfo.dwUserID)
			{
			    ithb->second.stAssignHBInfo.iRevenue[index] = (CT_INT32) m_pGameDeskPtr->CalculateRevenue(wChairID, llTotalWin);
			}
			else if(OpenHBResponse.dwUserID == ithb->second.stFaHBInfo.dwUserID &&  ithb->second.stAssignHBInfo.llGrabWinColorPoolAmount[index] > 0)
            {
                ithb->second.stAssignHBInfo.iRevenue[index] = (CT_INT32) m_pGameDeskPtr->CalculateRevenue(wChairID, ithb->second.stAssignHBInfo.llGrabWinColorPoolAmount[index]);
            }

			OpenHBResponse.dWinningAmount = ithb->second.stAssignHBInfo.llOutHB[index] * TO_DOUBLE;
			OpenHBResponse.cbIsThunder = ithb->second.stAssignHBInfo.bIsThunder[index];
			OpenHBResponse.dwHBID = dwHBID;
			if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			{
				m_mRealPlayerCount[dwHBID]++;
			}
			else
			{
				m_sllStockScore += (ithb->second.stAssignHBInfo.llOutHB[index] - ithb->second.stAssignHBInfo.llThunderBackAmount[index]);
			}
			ithb->second.stAssignHBInfo.llSourceScore[index] = m_pGameDeskPtr->GetUserScore(wChairID);
            ithb->second.stAssignHBInfo.wChairID[index] = wChairID;
            ithb->second.stAssignHBInfo.dwUserID[index] = OpenHBResponse.dwUserID;
		}
	}

	if(index != MAX_HB_COUNT)
	{
	    m_sllGrabWinScore += (ithb->second.stAssignHBInfo.llOutHB[index]-ithb->second.stAssignHBInfo.llThunderBackAmount[index]);
		//扣分
		//CT_LONGLONG llScoreBefore = m_pGameDeskPtr->GetUserScore(wChairID);
        CT_LONGLONG llScore = (ithb->second.stAssignHBInfo.llOutHB[index]-ithb->second.stAssignHBInfo.llThunderBackAmount[index] - ithb->second.stAssignHBInfo.iRevenue[index]+ ithb->second.stAssignHBInfo.llGrabWinColorPoolAmount[index]);
		OpenHBResponse.dWinLoseScore = llScore*TO_DOUBLE;
		OpenHBResponse.dWinColorPotAmount = ithb->second.stAssignHBInfo.llGrabWinColorPoolAmount[index]*TO_DOUBLE;
		OpenHBResponse.dTotalColorPotAmount = m_GameLogic.GetColorPool()*TO_DOUBLE;
		//战况统计
        std::map<CT_DWORD, GS_OnlinePlayerExploits>::iterator it = m_mOnlinePlayerExploits.find(OpenHBResponse.dwUserID);
        if(it != m_mOnlinePlayerExploits.end())
        {
            it->second.dWinLoseScore += (ithb->second.stAssignHBInfo.llOutHB[index]*TO_DOUBLE);
            it->second.wPlayCount++;
            if(OpenHBResponse.dWinLoseScore > 0)
            {
                it->second.wWinCount++;
                pGrabUserInfo->second.llGrabContinueWinScore+=llScore;
				pGrabUserInfo->second.llGrabContinueLoseScore = 0;
            }
            else
			{
				pGrabUserInfo->second.llGrabContinueLoseScore+=llScore;
				pGrabUserInfo->second.llGrabContinueWinScore = 0;
			}
        }
        else
        {
            m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].dwUserID = OpenHBResponse.dwUserID;
            m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].dWinLoseScore = (ithb->second.stAssignHBInfo.llOutHB[index]*TO_DOUBLE);
            m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].wPlayCount = 1;
            m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].cbGender = m_pGameDeskPtr->GetUserSex(wChairID);
            m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(wChairID);
            m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].cbVipLevel = pGrabUserInfo->second.cbVipLevel;
            if(OpenHBResponse.dWinLoseScore > 0)
            {
                m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].wWinCount = 1;
            }
            else
            {
                m_mOnlinePlayerExploits[OpenHBResponse.dwUserID].wWinCount = 0;
            }
        }

        //统计玩家连续抢最后一个红包的次数
        if((0 == ithb->second.stAssignHBInfo.wRemainHBCount) && (0 == ithb->second.stAssignHBInfo.llRemainAmount) /*&& (!ithb->second.stAssignHBInfo.bIsThunder[index])*/)
        {
            pGrabUserInfo->second.dwContinueGrabLastOneHB++;
        }
        else
        {
            pGrabUserInfo->second.dwContinueGrabLastOneHB = 0;
        }

        auto its = m_mOnlinePlayerTotalExploits.find(OpenHBResponse.dwUserID);
        if(its != m_mOnlinePlayerTotalExploits.end())
		{
        	if(OpenHBResponse.dwUserID != ithb->second.stFaHBInfo.dwUserID)
			{
				its->second += (llScore*TO_DOUBLE);
                pGrabUserInfo->second.llWinLoseScore += llScore;
			}
			else
			{
                pGrabUserInfo->second.llWinLoseScore += (llScore-ithb->second.stAssignHBInfo.llOutHB[index]);
				its->second += ((llScore-ithb->second.stAssignHBInfo.llOutHB[index])*TO_DOUBLE);
			}
		}
        else
		{
			if(OpenHBResponse.dwUserID != ithb->second.stFaHBInfo.dwUserID)
			{
                pGrabUserInfo->second.llWinLoseScore += llScore;
				m_mOnlinePlayerTotalExploits[OpenHBResponse.dwUserID] = (llScore*TO_DOUBLE);
			}
			else
			{
                pGrabUserInfo->second.llWinLoseScore += (llScore-ithb->second.stAssignHBInfo.llOutHB[index]);
				m_mOnlinePlayerTotalExploits[OpenHBResponse.dwUserID] = ((llScore-ithb->second.stAssignHBInfo.llOutHB[index])*TO_DOUBLE);
			}
		}

        OpenHBResponse.dTotalWinLose = m_mOnlinePlayerTotalExploits[OpenHBResponse.dwUserID];
        SendTableData(wChairID, SUB_S_OPEN_HB, &OpenHBResponse, sizeof(OpenHBResponse), false);

        ithb->second.vOpenHBList.push_back(OpenHBResponse);

        m_pGameDeskPtr->AddUserScore(wChairID,llScore ,true);

        ChangeFaSource(OpenHBResponse.dwUserID,llScore);
        if(pBlackUser != m_mapBlackList.end())
        {
            pBlackUser->second.llCurrControlScore += llScore;
        }
        pGrabUserInfo->second.llScore = m_pGameDeskPtr->GetUserScore(wChairID);
        if(userCtrl != m_mapUserPointControl.end())
        {
            userCtrl->second.llCurrDianKongFen += llScore;
			MSG_G2DB_User_PointControl data;
			memset(&data, 0, sizeof(data));
			data = userCtrl->second;
            if(en_PlayerOutScore == data.iDianKongZhi)
            {
                data.llDianKongFen *= -1;
            }
			m_pGameDeskPtr->SavePlayerDianKongData(data);
        }
        //LOG(ERROR)< <dwHBID << " dwUserID:"<<OpenHBResponse.dwUserID <<" OpenHB llScoreBefore: " << llScoreBefore << " llScore: " << llScore << " llScoreAfter: " << pGrabUserInfo->second.llScore;

        RecordScoreInfo recordScoreInfo;
        memset(&recordScoreInfo, 0, sizeof(recordScoreInfo));
        recordScoreInfo.dwUserID = m_pGameDeskPtr->GetUserID(wChairID);
        recordScoreInfo.llSourceScore = ithb->second.stAssignHBInfo.llSourceScore[index];
        recordScoreInfo.cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(wChairID);
        recordScoreInfo.cbStatus = 1;
        recordScoreInfo.iScore = llTotalWin;
        recordScoreInfo.dwRevenue = ithb->second.stAssignHBInfo.iRevenue[index];
        m_mRecordScoreInfo[dwHBID].push_back(recordScoreInfo);

        //只有抢红包玩家知道自己是否中雷，其他玩家不能知道
        CT_LONGLONG llHBAmount = ithb->second.stAssignHBInfo.llOutHB[index];
        llHBAmount = (llHBAmount-llHBAmount%10);
        OpenHBResponse.dWinLoseScore = 0;
        OpenHBResponse.cbIsThunder = 0;
        OpenHBResponse.dTotalWinLose = 0;
        OpenHBResponse.dWinningAmount = llHBAmount*TO_DOUBLE;
        for (int i = 0; i < GAME_PLAYER; ++i)
        {
            if(i != wChairID && m_pGameDeskPtr->IsExistUser(i))
            {
                SendTableData(i, SUB_S_OPEN_HB, &OpenHBResponse, sizeof(OpenHBResponse), false);
            }
        }
	}
	else
    {
	    //LOG(ERROR) <<"OpenHB cbIsFail: "<<(int)OpenHBResponse.cbIsFail;
        SendTableData(wChairID, SUB_S_OPEN_HB, &OpenHBResponse, sizeof(OpenHBResponse), false);
        return true;
	}



    if((ithb != m_FaingHB.end()) && (SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID) && (0 >= ithb->second.stAssignHBInfo.wRemainHBCount)
    && (!OpenHBResponse.cbIsFail) && (0 == ithb->second.dwEndTime))
    {
        OnEventGameEnd(dwHBID, GER_NORMAL);
    }
    else if((ithb != m_FaingHB.end()) && (SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID) && (0 >= ithb->second.stAssignHBInfo.wRemainHBCount) && (m_cbGameStatus == GAME_SCENE_PLAY))
    {
        OnEventGameEnd(dwHBID, GER_NORMAL);
    }

	ithb = m_FaingHB.find(dwHBID);
    if((ithb != m_FaingHB.end()) && IsBlackListUserOnline() && (0 == ithb->second.stAssignHBInfo.iRobotGrabCount))
	{
		//黑名单用户在线，且没有机器人抢此红包，则启动一个机器人立即抢此红包
		RobotGrabHB(dwHBID);
	}
	//iRand = RAND_NUM(m_dwRandIndex++)%100;
	if((ithb != m_FaingHB.end()) && (1 == ithb->second.stAssignHBInfo.wRemainHBCount) && ((int)m_iProba(m_gen) < m_iGrabLastOneHBProba))
	{
		//机器人抢最后一个红包
		RobotGrabHB(dwHBID);
	}
	return true;
}
CT_BOOL CGameProcess::ApplyBatchFaHB(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	//变量定义
	CMD_C_ApplyBatchFaHB * pNetInfo = (CMD_C_ApplyBatchFaHB *)pDataBuffer;
	if(dwDataSize != sizeof(CMD_C_ApplyBatchFaHB))
	{
		LOG(ERROR) << "ApplyBatchFaHB  dwDataSize:" << (int)dwDataSize <<" != " << "sizeof(CMD_C_Apply):"<<(int)sizeof(CMD_C_ApplyBatchFaHB);
		return false;
	}

	CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(wChairID);

	CMD_S_ApplyBatchFaHB ApplyReply;
	memset(&ApplyReply, 0, sizeof(ApplyReply));
    ApplyReply.dwUserID = dwUserID;
	int iAlreadyHBCount = GetHBCountInApplyListByUserID(dwUserID);
	if((!m_pGameDeskPtr->IsAndroidUser(pNetInfo->wChairID)) && (pNetInfo->dwUserID != m_pGameDeskPtr->GetUserID(pNetInfo->wChairID)))
	{
		LOG(ERROR) << "ApplyBatchFaHB wChairID: " <<  pNetInfo->wChairID << " dwUserID: " << pNetInfo->dwUserID;
		return false;
	}
	int iMaxFaHBCount = 0;
    for (CT_DWORD k = 0; k < m_vFaHBCountVIPCfg.size(); ++k)
    {
        if(m_vFaHBCountVIPCfg[k].iVIPLevel == m_mUserBaseData[dwUserID].cbVipLevel)
        {
            iMaxFaHBCount = m_vFaHBCountVIPCfg[k].iMaxFaHB;
            break;
        }
    }

    if (m_pGameRoomKindInfo->cbState == SERVER_STOP)
    {
        ApplyReply.cbIsFail = 8;
    }
	else if(pNetInfo->iHBAmount < 3)
    {
        ApplyReply.cbIsFail = 2;
    }
	else if(pNetInfo->iHBAmount > 10000)
    {
        ApplyReply.cbIsFail = 3;
    }
	else if(pNetInfo->iFaHBCount <= 0 || pNetInfo->iFaHBCount > iMaxFaHBCount || iAlreadyHBCount >= iMaxFaHBCount)
	{
		ApplyReply.cbIsFail = 5;
	}
	else if(((pNetInfo->iHBAmount*TO_SCORE)*pNetInfo->iFaHBCount) > m_pGameDeskPtr->GetUserScore(wChairID))
	{
		ApplyReply.cbIsFail = 1;
	}
	else if(pNetInfo->cbThunderNO > RAND_THUNDER_NO)
	{
		ApplyReply.cbIsFail = 4;
	}
	else if(m_ApplyList.size() > MAX_APPLY_LIST_LENTH)
	{
		ApplyReply.cbIsFail = 6;
	}
	else if(pNetInfo->iHBAllotCount > 100 || pNetInfo->iHBAllotCount < 2)
    {
        ApplyReply.cbIsFail = 7;
    }

    tagHBInfo ApplyFaHB;
	memset(&ApplyFaHB, 0, sizeof(ApplyFaHB));
    ApplyFaHB.stAssignHBInfo.init();
	if(!ApplyReply.cbIsFail) {

		ApplyFaHB.stFaHBInfo.dMultiple = -1;
		ApplyFaHB.stFaHBInfo.wChairID = wChairID;
		ApplyFaHB.stFaHBInfo.dwUserID = m_pGameDeskPtr->GetUserID(wChairID);
		ApplyFaHB.stFaHBInfo.cbThunderNO = pNetInfo->cbThunderNO;
		ApplyFaHB.stFaHBInfo.iHBAmount = pNetInfo->iHBAmount;
		if (SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID)
		{
			for (CT_DWORD i = 0; i < m_vHBRoomConfig.size(); ++i)
			{
				if (m_vHBRoomConfig[i].iHBAllotCount == pNetInfo->iHBAllotCount)
				{
					ApplyFaHB.stFaHBInfo.dMultiple = m_vHBRoomConfig[i].fMultiple;
					ApplyFaHB.stFaHBInfo.iHBAllotCount = pNetInfo->iHBAllotCount;
					break;
				}
			}
		}
		else if (m_pHBRoomConfig)
		{
			ApplyFaHB.stFaHBInfo.dMultiple = m_pHBRoomConfig->fMultiple;
			ApplyFaHB.stFaHBInfo.iHBAllotCount = m_pHBRoomConfig->iHBAllotCount;
		}
		if (ApplyFaHB.stFaHBInfo.dMultiple <= 0)
		{
			ApplyReply.cbIsFail = 7;
		}
	}

	if(!ApplyReply.cbIsFail)
	{
		for (int j = 0; j < (iMaxFaHBCount-iAlreadyHBCount) && (j < pNetInfo->iFaHBCount); ++j)
		{
			ApplyFaHB.stFaHBInfo.dwHBID = ((0 >= m_dwHBIDIndex || m_dwHBIDIndex > (999999)) ? (m_dwHBIDIndex=1): (m_dwHBIDIndex++));
			ApplyReply.iFaHBCount++;
            ApplyFaHB.llFaSourceScore = m_pGameDeskPtr->GetUserScore(wChairID);
            CT_LONGLONG llScore = ((pNetInfo->iHBAmount*TO_SCORE) * (-1));
            m_pGameDeskPtr->AddUserScore(wChairID, llScore, true);
			m_ApplyList.push_back(ApplyFaHB);
			if((m_pGameRoomKindInfo->wRoomKindID != SENIOR_ROOM) && (m_ApplyList.size() == 1))
			{
				CMD_S_PlayerApply nextFaHBPlayer;
				nextFaHBPlayer.dwUserID = m_ApplyList.begin()->stFaHBInfo.dwUserID;
				nextFaHBPlayer.dwHBID = m_ApplyList.begin()->stFaHBInfo.dwHBID;
				nextFaHBPlayer.wChairID = m_ApplyList.begin()->stFaHBInfo.wChairID;							//发红包玩家id
				nextFaHBPlayer.cbGender = m_mUserBaseData[nextFaHBPlayer.dwUserID].cbGender;							//性别
				nextFaHBPlayer.cbVipLevel = m_mUserBaseData[nextFaHBPlayer.dwUserID].cbVipLevel;							//VIP等级
				nextFaHBPlayer.cbHeadIndex = m_mUserBaseData[nextFaHBPlayer.dwUserID].cbHeadIndex;						//头像索引
				nextFaHBPlayer.cbThunderNO = m_ApplyList.begin()->stFaHBInfo.cbThunderNO;						//中雷号
				nextFaHBPlayer.iFaHBTotalAmount = m_ApplyList.begin()->stFaHBInfo.iHBAmount;					//红包总金额
				nextFaHBPlayer.iHBAllotCount = m_ApplyList.begin()->stFaHBInfo.iHBAllotCount;                      //发的红包可以分配成多少个子包（包数
				nextFaHBPlayer.fMultiple = m_ApplyList.begin()->stFaHBInfo.dMultiple;				            //倍数
				SendTableData(INVALID_CHAIR, SUB_S_NEXT_FA_HB_PLAYER, &nextFaHBPlayer, sizeof(nextFaHBPlayer),false);
			}
        };
        m_mUserBaseData[dwUserID].llScore = m_pGameDeskPtr->GetUserScore(wChairID);
		SendTableData(INVALID_CHAIR, SUB_S_APPLY_BATCH_FA_HB, &ApplyReply, sizeof(ApplyReply), false);

        if (SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID)
        {
            //for (CT_DWORD i = 0; (i < (MAX_DISPLAY_HB_COUNT-m_FaingHB.size())) && (m_ApplyList.size() > 0); ++i)
            if(m_ApplyList.size() > 0)
            {
                OnEventGameStart();
                if(m_ApplyList.size() > 0)
				{
					m_pGameDeskPtr->SetGameTimer(IDI_CHECK_START, TIME_CHECK_START*1000);
				}
            }
        }
        else if(m_ApplyList.size() > 0 && GAME_SCENE_FREE == m_cbGameStatus)
        {
            OnEventGameStart();
        }
	}
	else
	{
		SendTableData(wChairID, SUB_S_APPLY_BATCH_FA_HB, &ApplyReply, sizeof(ApplyReply), false);
	}
	return true;
}
CT_BOOL CGameProcess::RevokeFaHB(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	//变量定义
	CMD_C_RevokeHB * pNetInfo = (CMD_C_RevokeHB *)pDataBuffer;
	if(dwDataSize != sizeof(CMD_C_RevokeHB))
	{
		LOG(ERROR) << "RevokeFaHB  dwDataSize:" << (int)dwDataSize <<" != " << "sizeof(CMD_C_Apply):"<<(int)sizeof(CMD_C_RevokeHB);
		return false;
	}
	CMD_S_RevokeHB RevokeHB;
	memset(&RevokeHB, 0, sizeof(RevokeHB));
	RevokeHB.wRevokeCount = pNetInfo->wRevokeCount;
	RevokeHB.wChairID = wChairID;
	RevokeHB.dwUserID = m_pGameDeskPtr->GetUserID(wChairID);

	for (CT_DWORD i = 0; i < pNetInfo->wRevokeCount; ++i)
	{
		for (auto it = m_ApplyList.begin(); it !=  m_ApplyList.end(); ++it)
		{
			if(pNetInfo->dwHBID[i] == it->stFaHBInfo.dwHBID && pNetInfo->dwUserID == it->stFaHBInfo.dwUserID)
			{
			    //CT_LONGLONG llScoreBefore = m_pGameDeskPtr->GetUserScore(wChairID);
                CT_LONGLONG llScore = it->stFaHBInfo.iHBAmount*TO_SCORE;
			    m_pGameDeskPtr->AddUserScore(wChairID,llScore);
			    m_mUserBaseData[RevokeHB.dwUserID].llScore = m_pGameDeskPtr->GetUserScore(wChairID);

                //LOG(ERROR)<<RevokeHB.dwUserID<<" RevokeFaHB llScoreBefore: " << llScoreBefore << " llScore: " << llScore << " llScoreAfter: " << m_mUserBaseData[RevokeHB.dwUserID].llScore;
				m_ApplyList.erase(it++);
				it--;
				RevokeHB.wRevokeSuccessCount++;
			}
		}
	}
	//通知客户端有玩家撤销发红包
	SendTableData(INVALID_CHAIR, SUB_S_REVOKE_FA_HB, &RevokeHB, sizeof(RevokeHB), false);
	return true;
}
CT_BOOL CGameProcess::GetHBGrabedInfo(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
    //变量定义
    CMD_C_GrabedHBInfo *pNetInfo = (CMD_C_GrabedHBInfo *) pDataBuffer;
    if (dwDataSize != sizeof(CMD_C_GrabedHBInfo))
    {
        LOG(ERROR) << "GetHBGrabedInfo  dwDataSize:" << (int) dwDataSize << " != " << "sizeof(CMD_C_Apply):"
                   << (int) sizeof(CMD_C_RevokeHB);
        return false;
    }

    CT_DWORD dwUserID = 0;
    if(INVALID_CHAIR != wChairID)
    {
        dwUserID = m_pGameDeskPtr->GetUserID(wChairID);
    }

    CMD_S_GrabedHBInfo GrabedHBInfo;
    memset(&GrabedHBInfo, 0, sizeof(GrabedHBInfo));
    GrabedHBInfo.dwHBID = pNetInfo->dwHBID;
    FillGrabHBInfo(pNetInfo->dwHBID, dwUserID, GrabedHBInfo.stGrabHBPlayer);

    SendTableData(wChairID, SUB_S_GET_HB_GRABED_INFO, &GrabedHBInfo, sizeof(GrabedHBInfo), false);
    return true;
}
//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
    bool bRet = true;
	switch (dwSubCmdID)
	{
		case SUB_C_OPEN_HB:	//用户开红包
		{
			bRet = OpenHB(wChairID, pDataBuffer, dwDataSize);
            break;
		}
		case SUB_C_APPLY_BATCH_FA_HB: //用户批量fa红包
		{
			bRet = ApplyBatchFaHB(wChairID, pDataBuffer, dwDataSize);
			break;
		}
		case SUB_C_REVOKE_FA_HB: //用户批量fa红包
		{
			bRet = RevokeFaHB(wChairID, pDataBuffer, dwDataSize);
			break;
		}
		case SUB_C_GET_HB_GRABED_INFO:
        {
            bRet = GetHBGrabedInfo(wChairID, pDataBuffer, dwDataSize);
            break;
        }
		case SUB_C_GET_APPLY_LIST:	//用户get发红包
		{
			CT_BYTE  szBuffer[4096];
			memset(szBuffer,0, sizeof(szBuffer));
			CT_DWORD dwSendSize = 0;
			for (auto it = m_ApplyList.begin(); it !=  m_ApplyList.end() && (dwSendSize + sizeof(CMD_S_PlayerApply)) <= sizeof(szBuffer); ++it)
			{
				CMD_S_PlayerApply PlayerApply;
				memset(&PlayerApply,0, sizeof(PlayerApply));
				PlayerApply.wChairID = it->stFaHBInfo.wChairID;
				PlayerApply.iFaHBTotalAmount = it->stFaHBInfo.iHBAmount;
				PlayerApply.dwHBID = it->stFaHBInfo.dwHBID;
				PlayerApply.cbThunderNO = it->stFaHBInfo.cbThunderNO;
				PlayerApply.dwUserID = it->stFaHBInfo.dwUserID;
				PlayerApply.fMultiple = it->stFaHBInfo.dMultiple;//用户id
				PlayerApply.iHBAllotCount = it->stFaHBInfo.iHBAllotCount;
				PlayerApply.cbGender = m_mUserBaseData[PlayerApply.dwUserID].cbGender;							//性别
				PlayerApply.cbVipLevel = m_mUserBaseData[PlayerApply.dwUserID].cbVipLevel;							//VIP等级
				PlayerApply.cbHeadIndex = m_mUserBaseData[PlayerApply.dwUserID].cbHeadIndex;						//头像索引
				memcpy(szBuffer+dwSendSize, &PlayerApply, sizeof(CMD_S_PlayerApply));
				dwSendSize += sizeof(CMD_S_PlayerApply);
			}

			SendTableData(wChairID, SUB_S_APPLY_LIST, szBuffer, dwSendSize, false);
		}
		break;
		case SUB_C_GET_RANKING_LIST:
		{
            CalculateRankings(false);
			CT_BYTE  szBuffer[4096];
			memset(szBuffer,0, sizeof(szBuffer));
			CT_DWORD dwSendSize = 0;
			for (auto it = m_PlayerCountsRanking.begin();
				 (it !=  m_PlayerCountsRanking.end() && (dwSendSize + sizeof(GS_OnlinePlayerExploits)) <= sizeof(szBuffer)); ++it)
			{
				memcpy(szBuffer+dwSendSize, &(*it), sizeof(GS_OnlinePlayerExploits));
				dwSendSize += sizeof(GS_OnlinePlayerExploits);
			}

			SendTableData(wChairID, SUB_S_RANKING_LIST, szBuffer, dwSendSize, false);
			//LOG(ERROR) << "SUB_C_GET_RANKING_LIST: " << (int)dwSendSize/sizeof(GS_OnlinePlayerExploits);
		}
		break;
        default:
            break;
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
	CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
    int iVipLevel2 = 0;
    if(m_pGameDeskPtr->GetUserBaseData(dwChairID))
    {
        iVipLevel2 = m_pGameDeskPtr->GetUserBaseData(dwChairID)->cbVipLevel2;
    }
    std::map<CT_DWORD, GS_SimpleUserInfo>::iterator itr = m_mUserBaseData.find(dwUserID);
    if(itr == m_mUserBaseData.end())
    {

        GS_SimpleUserInfo UserInfo;
        UserInfo.dwUserID = dwUserID;
        UserInfo.cbGender = m_pGameDeskPtr->GetUserSex(dwChairID);
        UserInfo.cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(dwChairID);
        UserInfo.cbVipLevel = iVipLevel2;
        UserInfo.bRobot = m_pGameDeskPtr->IsAndroidUser(dwChairID);
        UserInfo.llScore = m_pGameDeskPtr->GetUserScore(dwChairID);
        UserInfo.llSourceScore = m_pGameDeskPtr->GetUserScore(dwChairID);
        UserInfo.dwFirstTime = time(NULL);
        m_mUserBaseData.insert(std::make_pair(dwUserID, UserInfo));
    }
    else
    {
        itr->second.llScore = m_pGameDeskPtr->GetUserScore(dwChairID);
        itr->second.llSourceScore = itr->second.llScore;
        itr->second.cbVipLevel = iVipLevel2;
        itr->second.bRobot = m_pGameDeskPtr->IsAndroidUser(dwChairID);
    }

    //战况统计
    std::map<CT_DWORD, GS_OnlinePlayerExploits>::iterator it = m_mOnlinePlayerExploits.find(dwUserID);
    if(it == m_mOnlinePlayerExploits.end())
    {
        m_mOnlinePlayerExploits[dwUserID].dwUserID = dwUserID;
        m_mOnlinePlayerExploits[dwUserID].dWinLoseScore = 0;
        m_mOnlinePlayerExploits[dwUserID].wPlayCount = 0;
        m_mOnlinePlayerExploits[dwUserID].cbGender = m_pGameDeskPtr->GetUserSex(dwChairID);
        m_mOnlinePlayerExploits[dwUserID].cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(dwChairID);
        m_mOnlinePlayerExploits[dwUserID].cbVipLevel = m_mUserBaseData[dwUserID].cbVipLevel;
        m_mOnlinePlayerExploits[dwUserID].wWinCount = 0;
    }
    //战况统计
	m_mOnlinePlayerTotalExploits[dwUserID] = 0;
	CalculateRankings();
	if(m_mUserBaseData[dwUserID].bRobot)
	{
		m_vRobotChairID.push_back((CT_WORD)dwChairID);
	}

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
    /*CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
    if((!IsInApplyListByUserID(dwUserID)) && (!IsInOpenHBListByUserID(dwUserID)))
    {
        DeleteUserByChairID(dwChairID);
    }*/
    DeleteUserByChairID(dwChairID);
}

//用户断线
void CGameProcess::OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
    /*CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
    if((!IsInApplyListByUserID(dwUserID)) && (!IsInOpenHBListByUserID(dwUserID)))
    {
        DeleteUserByChairID(dwChairID);
    }*/
    DeleteUserByChairID(dwChairID);
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
    m_GameLogic.SetGameRoomKindInfo(m_pGameRoomKindInfo);

    m_pGameDeskPtr->SetGameTimer(IDI_FA_HB, TIME_FA_HB*1000);
    m_pGameDeskPtr->SetGameTimer(IDI_DELETE_USER, TIME_DELETE_USER*1000);
    if(SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID)
    {
        m_pGameDeskPtr->SetGameTimer(IDI_CHECK_GAME_END, TIME_CHECK_GAME_END*1000);
        m_pGameDeskPtr->SetGameTimer(IDI_HB_DELETE_END, TIME_HB_DELETE_END*1000);
    }
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

	//基础积分
	m_dwlCellScore = 100;

	m_dwHBID = 0;

	if (NULL != m_pGameDeskPtr)
	{
        CT_DWORD dwNewTime = time(NULL);
        if((dwNewTime-m_dwOldTime) > 5*60)
        {
            ReadJsonCfg();
            m_GameLogic.SetKillScoreCfg(m_StockKillScoreCfg);
            m_GameLogic.SetPointCtrlCfg(m_PointCtrlCfg);
            m_GameLogic.SetColorPoolCfg(m_vColorPoolCfg);
            m_dwOldTime = dwNewTime;
        }
        //获取私人房信息
		//m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
		m_dwlCellScore = m_pGameDeskPtr->GetGameCellScore();
		m_pGameRoomKindInfo = m_pGameDeskPtr->GetGameKindInfo();

		if(m_vRobotChairID.size() < 9)
		{
			StatisticalRobotLocationInfo();
		}

        if(SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID)
        {
            for (CT_DWORD i = 0; i < m_vHBRoomConfig.size(); ++i) {
                if((CT_DWORD)(m_vHBRoomConfig[i].fMultiple*TO_SCORE) == m_pGameRoomKindInfo->dwCellScore)
                {
                    m_pHBRoomConfig = &(m_vHBRoomConfig[i]);
                }
            }

        }

        if(SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID)
        {
            m_cbGameStatus = GAME_SCENE_GAME;
        }
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
bool CGameProcess::ReadFile(std::string & file, std::string & str)
{
    std::ifstream in;
    std::string line;

    in.open(file, std::ifstream::in);
    if (!in.is_open())
        return false;
    while (getline(in, line)) {
        str.append(line + "\n");
    }
    in.close();
    return true;
}
void CGameProcess::ReadJsonCfg()
{
    std::string file;
    if(SENIOR_ROOM==m_pGameDeskPtr->GetGameKindInfo()->wRoomKindID)
    {
        file="./script/gameserver_HBSL_wxhb.json";
    }
    else
    {
        file="./script/gameserver_HBSL.json";
    }
	std::string data;
	ReadFile(file,data);
	acl::json json;
	json.update(data.c_str());
	if(!json.getElementsByTagName("CtrlMasterSwitch").empty())
	{
        m_bCtrlMasterSwitch = (CT_BOOL)(*(json.getElementsByTagName("CtrlMasterSwitch")[0])->get_int64());
		m_GameLogic.SetCtrlMasterSwitch(m_bCtrlMasterSwitch);
	}
    if(!json.getElementsByTagName("iGrabLastOneHBProba").empty())
    {
        m_iGrabLastOneHBProba = (CT_INT32)(*(json.getElementsByTagName("iGrabLastOneHBProba")[0])->get_int64());
        if(m_iGrabLastOneHBProba > 100 || m_iGrabLastOneHBProba < 0)
        {
            m_iGrabLastOneHBProba = 0;
        }
    }
	//基础配置
	if(!json.getElementsByTagName("HBRoomBaseCfg").empty())
	{
		m_vHBRoomConfig.clear();
        acl::json_node *node1 = json.getElementsByTagName("HBRoomBaseCfg")[0]->first_child();
		acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
        {
            GS_HBRoomBaseCfg HBRoomBaseCfg;
            memset(&HBRoomBaseCfg, 0, sizeof(HBRoomBaseCfg));
            acl::json jsonTmp(*node2);
            if(jsonTmp.getElementsByTagName("iHBAllotCount").size() > 0)
            {
                HBRoomBaseCfg.iHBAllotCount = (CT_INT32)(*(jsonTmp.getElementsByTagName("iHBAllotCount")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("fMultiple").size() > 0)
            {
                HBRoomBaseCfg.fMultiple = (CT_FLOAT)(*(jsonTmp.getElementsByTagName("fMultiple")[0])->get_double());
            }
			HBRoomBaseCfg.dwHBLowerLimit = m_pGameDeskPtr->GetGameKindInfo()->dwEnterMinScore*TO_DOUBLE;
			HBRoomBaseCfg.dwHBUpperLimit = m_pGameDeskPtr->GetGameKindInfo()->dwEnterMaxScore*TO_DOUBLE;
            node2 = node1->next_child();
            m_vHBRoomConfig.push_back(HBRoomBaseCfg);
        }
	}
	//抢红包配置
    if(!json.getElementsByTagName("RobotGrabHBCfg").empty())
	{
		m_vRobotGrabHBCfg.clear();
        acl::json_node *node1 = json.getElementsByTagName("RobotGrabHBCfg")[0]->first_child();
       /* acl::string out;
        node1->to_string(&out);
        LOG(WARNING) << out.c_str();*/
        acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
		{
			GS_RobotGrabHBCfg RobotGrabHBCfg;
            RobotGrabHBCfg.iHBAllotCount = 0;
			acl::json jsonTmp(*node2);
            if(jsonTmp.getElementsByTagName("iHBAllotCount").size() > 0)
            {
                RobotGrabHBCfg.iHBAllotCount = (CT_INT32)(*(jsonTmp.getElementsByTagName("iHBAllotCount")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("vRobotGrabHBTable").size() > 0)
            {
                acl::json_node *node3 = jsonTmp.getElementsByTagName("vRobotGrabHBTable")[0]->first_child();
                acl::json_node *node4 = node3->first_child();
                while(node3 != NULL && node4 != NULL)
                {
                    GS_RobotGrabHBTable RobotGrabHBTable;
                    memset(&RobotGrabHBTable, 0, sizeof(RobotGrabHBTable));
                    acl::json jsonTmp2(*node4);
                    if(jsonTmp2.getElementsByTagName("iRealLowerLimit").size() > 0)
                    {
                        RobotGrabHBTable.iRealLowerLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iRealLowerLimit")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iRealUpperLimit").size() > 0)
                    {
                        RobotGrabHBTable.iRealUpperLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iRealUpperLimit")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iRobotLowerLimit").size() > 0)
                    {
                        RobotGrabHBTable.iRobotLowerLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iRobotLowerLimit")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iRobotUpperLimit").size() > 0)
                    {
                        RobotGrabHBTable.iRobotUpperLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iRobotUpperLimit")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iRobotTimeLowerLimit").size() > 0)
                    {
                        RobotGrabHBTable.iRobotTimeLowerLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iRobotTimeLowerLimit")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iRobotTimeUpperLimit").size() > 0)
                    {
                        RobotGrabHBTable.iRobotTimeUpperLimit = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iRobotTimeUpperLimit")[0])->get_int64());
                    }
                    if(0 == RobotGrabHBTable.iRobotUpperLimit)
                    {
                        RobotGrabHBTable.iRobotUpperLimit = 65535;
                    }
                    if(0 == RobotGrabHBTable.iRobotTimeUpperLimit)
                    {
                        RobotGrabHBTable.iRobotUpperLimit = 5;
                    }
                    if(RobotGrabHBTable.iRobotUpperLimit < RobotGrabHBTable.iRobotLowerLimit)
                    {
                        LOG(ERROR)<< "iRobotUpperLimit: " << RobotGrabHBTable.iRobotUpperLimit << " iRobotLowerLimit: " << RobotGrabHBTable.iRobotLowerLimit;
                        CT_INT32 iTmp = RobotGrabHBTable.iRobotUpperLimit;
                        RobotGrabHBTable.iRobotUpperLimit = RobotGrabHBTable.iRobotLowerLimit;
                        RobotGrabHBTable.iRobotLowerLimit = iTmp;
                    }
                    if(RobotGrabHBTable.iRobotTimeUpperLimit < RobotGrabHBTable.iRobotTimeLowerLimit)
                    {
                        LOG(ERROR)<< "iRobotTimeUpperLimit: " << RobotGrabHBTable.iRobotTimeUpperLimit << " iRobotTimeLowerLimit: " << RobotGrabHBTable.iRobotTimeLowerLimit;
                        CT_INT32 iTmp = RobotGrabHBTable.iRobotTimeUpperLimit;
                        RobotGrabHBTable.iRobotTimeUpperLimit = RobotGrabHBTable.iRobotTimeLowerLimit;
                        RobotGrabHBTable.iRobotTimeLowerLimit = iTmp;
                    }
                    node4 = node3->next_child();
                    RobotGrabHBCfg.vRobotGrabHBTable.push_back(RobotGrabHBTable);
                }
            }
            node2 = node1->next_child();
			m_vRobotGrabHBCfg.push_back(RobotGrabHBCfg);
		}
	}

	//机器人申请发红包配置
    if(!json.getElementsByTagName("RobotApplyFaHBCfg").empty())
    {
        m_vRobotApplyFaHBCfg.clear();
        acl::json_node *node1 = json.getElementsByTagName("RobotApplyFaHBCfg")[0]->first_child();
        acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
        {
            GS_RobotApplyFaHBCfg RobotApplyFaHBCfg;
            memset(&RobotApplyFaHBCfg, 0, sizeof(RobotApplyFaHBCfg));
            acl::json jsonTmp(*node2);
            if(jsonTmp.getElementsByTagName("iApplyListLenLowerLimit").size() > 0)
            {
                RobotApplyFaHBCfg.iApplyListLenLowerLimit = (CT_INT32)(*(jsonTmp.getElementsByTagName("iApplyListLenLowerLimit")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("iApplyListLenUpperLimit").size() > 0)
            {
                RobotApplyFaHBCfg.iApplyListLenUpperLimit = (CT_INT32)(*(jsonTmp.getElementsByTagName("iApplyListLenUpperLimit")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("iProba").size() > 0)
            {
                RobotApplyFaHBCfg.iProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iProba")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("iFaHBRobotCount").size() > 0)
            {
                RobotApplyFaHBCfg.iFaHBRobotCount = (CT_DWORD)(*(jsonTmp.getElementsByTagName("iFaHBRobotCount")[0])->get_int64());
            }
            if(RobotApplyFaHBCfg.iApplyListLenUpperLimit < RobotApplyFaHBCfg.iApplyListLenLowerLimit)
            {
                LOG(ERROR)<< "iApplyListLenUpperLimit: " << RobotApplyFaHBCfg.iApplyListLenUpperLimit << " iApplyListLenLowerLimit: " << RobotApplyFaHBCfg.iApplyListLenLowerLimit;
                CT_INT32 iTmp = RobotApplyFaHBCfg.iApplyListLenUpperLimit;
                RobotApplyFaHBCfg.iApplyListLenUpperLimit = RobotApplyFaHBCfg.iApplyListLenLowerLimit;
                RobotApplyFaHBCfg.iApplyListLenLowerLimit = iTmp;
            }

            node2 = node1->next_child();
            m_vRobotApplyFaHBCfg.push_back(RobotApplyFaHBCfg);
        }
    }
	//机器人退出配置
    if(!json.getElementsByTagName("RobotQuitCfg").empty())
    {
        m_RobotQuitCfg.vQuitTable.clear();
        acl::json jsonTmp(*(json.getElementsByTagName("RobotQuitCfg")[0]));
        if(jsonTmp.getElementsByTagName("iRemainScore").size() > 0)
        {
            m_RobotQuitCfg.llRemainScore = (CT_INT32)(*(jsonTmp.getElementsByTagName("iRemainScore")[0])->get_int64());
			m_RobotQuitCfg.llRemainScore *= TO_SCORE;
        }

        if(!jsonTmp.getElementsByTagName("RobotQuitTable").empty())
        {
            acl::json_node *node1 = jsonTmp.getElementsByTagName("RobotQuitTable")[0]->first_child();
            acl::json_node *node2 = node1->first_child();
            while(node1 != NULL && node2 != NULL) {
                GS_RobotQuitTable RobotQuitTable;
                memset(&RobotQuitTable, 0, sizeof(RobotQuitTable));
                acl::json jsonTmp2(*node2);
                if (jsonTmp2.getElementsByTagName("iOnlineTime").size() > 0) {
                    RobotQuitTable.iOnlineTime = (CT_INT32) (*(jsonTmp2.getElementsByTagName("iOnlineTime")[0])->get_int64());
                }
                if (jsonTmp2.getElementsByTagName("iQuitProba").size() > 0) {
                    RobotQuitTable.iQuitProba = (CT_INT32) (*(jsonTmp2.getElementsByTagName("iQuitProba")[0])->get_int64());
                }
                node2 = node1->next_child();
                if(RobotQuitTable.iOnlineTime <= 0)
                {
                    RobotQuitTable.iOnlineTime = 6;
                }
                if(RobotQuitTable.iQuitProba > 100)
                {
                    RobotQuitTable.iQuitProba = 100;
                }
                if(RobotQuitTable.iQuitProba <= 0)
                {
                    RobotQuitTable.iQuitProba = 50;
                }
                m_RobotQuitCfg.vQuitTable.push_back(RobotQuitTable);
            }
        }
    }
	//库存配置
	if(!json.getElementsByTagName("StockKillScoreCfg").empty())
	{
		m_StockKillScoreCfg.vKillScoreTable.clear();
		acl::json jsonTmp(*(json.getElementsByTagName("StockKillScoreCfg")[0]));
		if(jsonTmp.getElementsByTagName("iRealFaHBRobotThunderProba").size() > 0)
		{
			m_StockKillScoreCfg.iRealFaHBRobotThunderProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iRealFaHBRobotThunderProba")[0])->get_int64());
		}
        if(m_StockKillScoreCfg.iRealFaHBRobotThunderProba < 0 || m_StockKillScoreCfg.iRealFaHBRobotThunderProba > 50)
        {
            m_StockKillScoreCfg.iRealFaHBRobotThunderProba = 30;
        }
		if(jsonTmp.getElementsByTagName("iSysOutScorRealFaHBRobotThunderProba").size() > 0)
		{
			m_StockKillScoreCfg.iSysOutScorRealFaHBRobotThunderProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iSysOutScorRealFaHBRobotThunderProba")[0])->get_int64());
		}
		if(m_StockKillScoreCfg.iSysOutScorRealFaHBRobotThunderProba < 50 || m_StockKillScoreCfg.iSysOutScorRealFaHBRobotThunderProba > 100)
		{
			m_StockKillScoreCfg.iSysOutScorRealFaHBRobotThunderProba = 70;
		}
		if(jsonTmp.getElementsByTagName("i5HBLevelRobotFaRealThunderProba").size() > 0)
		{
			m_StockKillScoreCfg.i5HBLevelRobotFaRealThunderProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("i5HBLevelRobotFaRealThunderProba")[0])->get_int64());
		}
		if(m_StockKillScoreCfg.i5HBLevelRobotFaRealThunderProba < 0 || m_StockKillScoreCfg.i5HBLevelRobotFaRealThunderProba > 100)
		{
			m_StockKillScoreCfg.i5HBLevelRobotFaRealThunderProba = 20;
		}
		if(jsonTmp.getElementsByTagName("i7HBLevelRobotFaRealThunderProba").size() > 0)
		{
			m_StockKillScoreCfg.i7HBLevelRobotFaRealThunderProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("i7HBLevelRobotFaRealThunderProba")[0])->get_int64());
		}
        if(m_StockKillScoreCfg.i7HBLevelRobotFaRealThunderProba < 0 || m_StockKillScoreCfg.i7HBLevelRobotFaRealThunderProba > 100)
        {
            m_StockKillScoreCfg.i7HBLevelRobotFaRealThunderProba = 30;
        }
		if(jsonTmp.getElementsByTagName("i10HBLevelRobotFaRealThunderProba").size() > 0)
		{
			m_StockKillScoreCfg.i10HBLevelRobotFaRealThunderProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("i10HBLevelRobotFaRealThunderProba")[0])->get_int64());
		}
        if(m_StockKillScoreCfg.i10HBLevelRobotFaRealThunderProba < 0 || m_StockKillScoreCfg.i10HBLevelRobotFaRealThunderProba > 50)
        {
            m_StockKillScoreCfg.i10HBLevelRobotFaRealThunderProba = 40;
        }
		if(!jsonTmp.getElementsByTagName("vKillScoreTable").empty())
		{
			acl::json_node *node1 = jsonTmp.getElementsByTagName("vKillScoreTable")[0]->first_child();
			acl::json_node *node2 = node1->first_child();
			while(node1 != NULL && node2 != NULL) {
				GS_TriggerKillScoreTable TriggerKillScoreTable;
				memset(&TriggerKillScoreTable, 0, sizeof(TriggerKillScoreTable));
				acl::json jsonTmp2(*node2);
				if (jsonTmp2.getElementsByTagName("iTriggerStockLowerLimit").size() > 0)
				{
					TriggerKillScoreTable.llTriggerStockLowerLimit = (CT_INT32) (*(jsonTmp2.getElementsByTagName("iTriggerStockLowerLimit")[0])->get_int64());
                    TriggerKillScoreTable.llTriggerStockLowerLimit *= TO_SCORE;
				}
				if (jsonTmp2.getElementsByTagName("iTriggerStockUpperLimit").size() > 0)
				{
					TriggerKillScoreTable.llTriggerStockUpperLimit = (CT_INT32) (*(jsonTmp2.getElementsByTagName("iTriggerStockUpperLimit")[0])->get_int64());
                    TriggerKillScoreTable.llTriggerStockUpperLimit *= TO_SCORE;
				}
				if (jsonTmp2.getElementsByTagName("iKillScoreProba").size() > 0) {
					TriggerKillScoreTable.iKillScoreProba = (CT_INT32) (*(jsonTmp2.getElementsByTagName("iKillScoreProba")[0])->get_int64());
				}
				if (jsonTmp2.getElementsByTagName("iCtrlType").size() > 0) {
					TriggerKillScoreTable.iCtrlType = (CT_INT32) (*(jsonTmp2.getElementsByTagName("iCtrlType")[0])->get_int64());
				}
				node2 = node1->next_child();
				if(TriggerKillScoreTable.iKillScoreProba > 100)
				{
					TriggerKillScoreTable.iKillScoreProba=100;
				}
				if(TriggerKillScoreTable.iKillScoreProba <= 0)
				{
					TriggerKillScoreTable.iKillScoreProba = 50;
				}
				if(0 == TriggerKillScoreTable.llTriggerStockLowerLimit && TriggerKillScoreTable.llTriggerStockUpperLimit < TriggerKillScoreTable.llTriggerStockLowerLimit)
				{
					TriggerKillScoreTable.llTriggerStockLowerLimit = -0x7fffffffffffff;
				}
				if(0 == TriggerKillScoreTable.llTriggerStockUpperLimit && TriggerKillScoreTable.llTriggerStockUpperLimit < TriggerKillScoreTable.llTriggerStockLowerLimit)
				{
					TriggerKillScoreTable.llTriggerStockUpperLimit = 0x7fffffffffffff;
				}
				if(0 != TriggerKillScoreTable.llTriggerStockUpperLimit && 0 != TriggerKillScoreTable.llTriggerStockLowerLimit
				&& TriggerKillScoreTable.llTriggerStockUpperLimit < TriggerKillScoreTable.llTriggerStockLowerLimit)
				{
					LOG(ERROR)<<"cfg error:StockKillScoreCfg.iTriggerStockUpperLimit: "<< TriggerKillScoreTable.llTriggerStockUpperLimit
					<<" StockKillScoreCfg.iTriggerStockLowerLimit: " << TriggerKillScoreTable.llTriggerStockLowerLimit;
                    TriggerKillScoreTable.llTriggerStockUpperLimit = 100;
                    TriggerKillScoreTable.llTriggerStockLowerLimit = -0x7fffffffffffff;
				}
				if(TriggerKillScoreTable.iCtrlType != -1 && TriggerKillScoreTable.iCtrlType != 1)
				{
					if(TriggerKillScoreTable.llTriggerStockLowerLimit > 0 )
						TriggerKillScoreTable.iCtrlType = en_OutScore;
					else
						TriggerKillScoreTable.iCtrlType = en_EatScore;
					LOG(ERROR)<<"StockKillScoreCfg error iCtrlType";
				}
				m_StockKillScoreCfg.vKillScoreTable.push_back(TriggerKillScoreTable);
			}
		}
	}
	//VIP与发红包数量配置
	if(!json.getElementsByTagName("BatchHBCountVIPCfg").empty())
	{
		m_vFaHBCountVIPCfg.clear();
		acl::json_node *node1 = json.getElementsByTagName("BatchHBCountVIPCfg")[0]->first_child();
		acl::json_node *node2 = node1->first_child();
		while(node1 != NULL && node2 != NULL)
		{
			GS_BatchHBCountVIPCfg batchHBCountVIPCfg;
			memset(&batchHBCountVIPCfg, 0, sizeof(batchHBCountVIPCfg));
			acl::json jsonTmp(*node2);
			if(jsonTmp.getElementsByTagName("iVIPLevel").size() > 0)
			{
				batchHBCountVIPCfg.iVIPLevel = (CT_INT32)(*(jsonTmp.getElementsByTagName("iVIPLevel")[0])->get_int64());
			}
			if(jsonTmp.getElementsByTagName("iMaxFaHB").size() > 0)
			{
				batchHBCountVIPCfg.iMaxFaHB = (CT_INT32)(*(jsonTmp.getElementsByTagName("iMaxFaHB")[0])->get_int64());
			}
			node2 = node1->next_child();
			m_vFaHBCountVIPCfg.push_back(batchHBCountVIPCfg);
		}
	}
    //点控配置
    if(!json.getElementsByTagName("PointCtrlCfg").empty())
    {
        m_PointCtrlCfg.vPointCtrlTable.clear();
        acl::json jsonTmp(*(json.getElementsByTagName("PointCtrlCfg")[0]));
        if(jsonTmp.getElementsByTagName("iExeProba").size() > 0)
        {
            m_PointCtrlCfg.iExeProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iExeProba")[0])->get_int64());
            if(m_PointCtrlCfg.iExeProba < 0 || m_PointCtrlCfg.iExeProba > 100)
            {
                m_PointCtrlCfg.iExeProba = 50;
            }
        }
		if(jsonTmp.getElementsByTagName("iTimeInterval").size() > 0)
		{
			m_PointCtrlCfg.iTimeInterval = (CT_INT32)(*(jsonTmp.getElementsByTagName("iTimeInterval")[0])->get_int64());
			if(m_PointCtrlCfg.iTimeInterval < 600 || m_PointCtrlCfg.iTimeInterval > 5*3600)
			{
				m_PointCtrlCfg.iTimeInterval = 2*3600;
			}
		}
        if(jsonTmp.getElementsByTagName("iGrabBaseProbaMultiple").size() > 0)
        {
            m_PointCtrlCfg.iGrabBaseProbaMultiple = (CT_INT32)(*(jsonTmp.getElementsByTagName("iGrabBaseProbaMultiple")[0])->get_int64());
            if(m_PointCtrlCfg.iGrabBaseProbaMultiple < 1 || m_PointCtrlCfg.iGrabBaseProbaMultiple > 10)
            {
                m_PointCtrlCfg.iGrabBaseProbaMultiple = 1;
            }
        }
        acl::json_node *node1 = jsonTmp.getElementsByTagName("vPointCtrlTable")[0]->first_child();
        acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
        {
            GS_PointCtrlTable PointCtrlTable;
            memset(&PointCtrlTable, 0, sizeof(PointCtrlTable));
            acl::json jsonTmp(*node2);
            if(jsonTmp.getElementsByTagName("llChgScoreLowerLimit").size() > 0)
            {
                PointCtrlTable.llChgScoreLowerLimit = (CT_INT32)(*(jsonTmp.getElementsByTagName("llChgScoreLowerLimit")[0])->get_int64());
                PointCtrlTable.llChgScoreLowerLimit *= TO_SCORE;
            }
            if(jsonTmp.getElementsByTagName("llChgScoreUpperLimit").size() > 0)
            {
                PointCtrlTable.llChgScoreUpperLimit = (CT_INT32)(*(jsonTmp.getElementsByTagName("llChgScoreUpperLimit")[0])->get_int64());
                PointCtrlTable.llChgScoreUpperLimit *= TO_SCORE;
            }
			if(jsonTmp.getElementsByTagName("iProba").size() > 0)
			{
                PointCtrlTable.iProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iProba")[0])->get_int64());
			}
			if(jsonTmp.getElementsByTagName("iCtlChgPstLowerLimit").size() > 0)
			{
                PointCtrlTable.iCtlChgPstLowerLimit = (CT_INT32)(*(jsonTmp.getElementsByTagName("iCtlChgPstLowerLimit")[0])->get_int64());
			}
			if(jsonTmp.getElementsByTagName("iCtlChgPstUpperLimit").size() > 0)
			{
                PointCtrlTable.iCtlChgPstUpperLimit = (CT_INT32)(*(jsonTmp.getElementsByTagName("iCtlChgPstUpperLimit")[0])->get_int64());
			}
			if(jsonTmp.getElementsByTagName("iCtrlType").size() > 0)
			{
                PointCtrlTable.iCtrlType = (CT_INT32)(*(jsonTmp.getElementsByTagName("iCtrlType")[0])->get_int64());
			}

			if(PointCtrlTable.llChgScoreLowerLimit == 0)
			{
                PointCtrlTable.llChgScoreLowerLimit = -0x7fffffffffffff;
			}
			if(PointCtrlTable.llChgScoreUpperLimit == 0)
			{
                PointCtrlTable.llChgScoreUpperLimit = 0x7fffffffffffff;
			}
			if(PointCtrlTable.llChgScoreLowerLimit > PointCtrlTable.llChgScoreUpperLimit)
			{
				CT_LONGLONG tmp = PointCtrlTable.llChgScoreLowerLimit;
                PointCtrlTable.llChgScoreLowerLimit = PointCtrlTable.llChgScoreUpperLimit;
                PointCtrlTable.llChgScoreUpperLimit = tmp;
				LOG(ERROR)<<"PointCtrlCfg error llChgScoreLowerLimit llChgScoreUpperLimit";
			}
			if(PointCtrlTable.iProba > 100 || PointCtrlTable.iProba < 0)
			{
                PointCtrlTable.iProba = 50;
				LOG(ERROR)<<"PointCtrlCfg error iProba";
			}
			if(PointCtrlTable.iCtlChgPstLowerLimit > PointCtrlTable.iCtlChgPstUpperLimit)
			{
				CT_LONGLONG tmp = PointCtrlTable.iCtlChgPstLowerLimit;
                PointCtrlTable.iCtlChgPstLowerLimit = PointCtrlTable.iCtlChgPstUpperLimit;
                PointCtrlTable.iCtlChgPstUpperLimit = tmp;
				LOG(ERROR)<<"PointCtrlCfg error iCtlChgPstLowerLimit iCtlChgPstUpperLimit";
			}
			if(PointCtrlTable.iCtlChgPstLowerLimit > 100 || PointCtrlTable.iCtlChgPstLowerLimit < 0)
			{
                PointCtrlTable.iCtlChgPstLowerLimit = 30;
				LOG(ERROR)<<"PointCtrlCfg error iCtlChgPstLowerLimit";
			}
			if(PointCtrlTable.iCtlChgPstUpperLimit > 100 || PointCtrlTable.iCtlChgPstUpperLimit < 0)
			{
                PointCtrlTable.iCtlChgPstUpperLimit = 70;
				LOG(ERROR)<<"PointCtrlCfg error iCtlChgPstUpperLimit";
			}
			if(PointCtrlTable.iCtrlType != -1 && PointCtrlTable.iCtrlType != 1)
			{
				if(PointCtrlTable.llChgScoreLowerLimit > 0 )
                    PointCtrlTable.iCtrlType = -1;
				else
                    PointCtrlTable.iCtrlType = 1;
				LOG(ERROR)<<"PointCtrlCfg error iCtrlType";
			}
            node2 = node1->next_child();
			m_PointCtrlCfg.vPointCtrlTable.push_back(PointCtrlTable);
        }
    }
    //机器人发红包种类配置
    if(!json.getElementsByTagName("RobotFaHBAllotCfg").empty())
    {
        m_vRobotFaHBAllotCfg.clear();
        acl::json_node *node1 = json.getElementsByTagName("RobotFaHBAllotCfg")[0]->first_child();
        acl::json_node *node2 = node1->first_child();
        int iTotalProba = 0;
        while(node1 != NULL && node2 != NULL)
        {
            GS_RobotFaHBAllotCfg RobotFaHBAllotCfg;
            memset(&RobotFaHBAllotCfg, 0, sizeof(RobotFaHBAllotCfg));
            acl::json jsonTmp(*node2);
            if(jsonTmp.getElementsByTagName("iHBAllotCount").size() > 0)
            {
                RobotFaHBAllotCfg.iHBAllotCount = (CT_INT32)(*(jsonTmp.getElementsByTagName("iHBAllotCount")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("iProba").size() > 0)
            {
                RobotFaHBAllotCfg.iProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iProba")[0])->get_int64());
            }
            iTotalProba += RobotFaHBAllotCfg.iProba;
            node2 = node1->next_child();
            m_vRobotFaHBAllotCfg.push_back(RobotFaHBAllotCfg);
        }
        if(iTotalProba != 100)
        {
            LOG(ERROR)<<"RobotFaHBAllotCfg error iProba";
        }
    }
    //彩池配置
    if(!json.getElementsByTagName("ColorPoolCfg").empty())
    {
        m_vColorPoolCfg.clear();
        acl::json_node *node1 = json.getElementsByTagName("ColorPoolCfg")[0]->first_child();
        acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
        {
			GS_ColorPoolCfg ColorPoolCfg;
            memset(&ColorPoolCfg, 0, sizeof(ColorPoolCfg));
            acl::json jsonTmp(*node2);
            if(jsonTmp.getElementsByTagName("iGrabWinPrizeRatio").size() > 0)
            {
				ColorPoolCfg.iGrabWinPrizeRatio = (CT_INT32)(*(jsonTmp.getElementsByTagName("iGrabWinPrizeRatio")[0])->get_int64());
				if(ColorPoolCfg.iGrabWinPrizeRatio < 0  || ColorPoolCfg.iGrabWinPrizeRatio > 100)
				{
					ColorPoolCfg.iGrabWinPrizeRatio = 10;
				}
            }
			if(jsonTmp.getElementsByTagName("iFaWinPrizeRatio").size() > 0)
			{
				ColorPoolCfg.iFaWinPrizeRatio = (CT_INT32)(*(jsonTmp.getElementsByTagName("iFaWinPrizeRatio")[0])->get_int64());
				if(ColorPoolCfg.iFaWinPrizeRatio < 0  || ColorPoolCfg.iFaWinPrizeRatio > 100)
				{
					ColorPoolCfg.iFaWinPrizeRatio = 10;
				}
			}
			if(jsonTmp.getElementsByTagName("llMinWinPrizeAmount").size() > 0)
			{
				ColorPoolCfg.llMinWinPrizeAmount = (CT_INT32)(*(jsonTmp.getElementsByTagName("llMinWinPrizeAmount")[0])->get_int64());
			}
			if(jsonTmp.getElementsByTagName("iPointCtlLoseWinProba").size() > 0)
			{
				ColorPoolCfg.iPointCtlLoseWinProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iPointCtlLoseWinProba")[0])->get_int64());
				if(ColorPoolCfg.iPointCtlLoseWinProba < 0  || ColorPoolCfg.iPointCtlLoseWinProba > 100)
				{
					ColorPoolCfg.iPointCtlLoseWinProba = 10;
				}
			}
			if(jsonTmp.getElementsByTagName("iRobotWinPrizeProba").size() > 0)
			{
				ColorPoolCfg.iRobotWinPrizeProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iRobotWinPrizeProba")[0])->get_int64());
				if(ColorPoolCfg.iRobotWinPrizeProba < 0  || ColorPoolCfg.iRobotWinPrizeProba > 100)
				{
					ColorPoolCfg.iRobotWinPrizeProba = 10;
				}
			}
			if(jsonTmp.getElementsByTagName("iWinPrizeProba").size() > 0)
			{
				ColorPoolCfg.iWinPrizeProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iWinPrizeProba")[0])->get_int64());
				if(ColorPoolCfg.iWinPrizeProba < 0  || ColorPoolCfg.iWinPrizeProba > 100)
				{
					ColorPoolCfg.iWinPrizeProba = 100;
				}
			}
			acl::json_node *node3 = jsonTmp.getElementsByTagName("vWinPrizeNumber")[0]->first_child();
			acl::json_node *node4 = node3->first_child();
			while(node3 != NULL && node4 != NULL)
            {
                ColorPoolCfg.vWinPrizeNumber.push_back((CT_INT32)(*(node4->get_int64())));
                node4 = node3->next_child();
            }
            node2 = node1->next_child();
			m_vColorPoolCfg.push_back(ColorPoolCfg);
        }

    }
    //黑名单配置
    if(!json.getElementsByTagName("BlackListCfg").empty())
    {
        m_BlackListCfg.iContinueGrabLastOneHB = 15;
        acl::json jsonTmp(*(json.getElementsByTagName("BlackListCfg")[0]));
        if(jsonTmp.getElementsByTagName("iContinueGrabLastOneHB").size() > 0)
        {
            m_BlackListCfg.iContinueGrabLastOneHB = (CT_INT32)(*(jsonTmp.getElementsByTagName("iContinueGrabLastOneHB")[0])->get_int64());
            if(m_BlackListCfg.iContinueGrabLastOneHB < 4 || m_BlackListCfg.iContinueGrabLastOneHB > 100)
            {
                m_BlackListCfg.iContinueGrabLastOneHB = 15;
            }
        }
        if(jsonTmp.getElementsByTagName("iCtrlLoseProba").size() > 0)
        {
            m_BlackListCfg.iCtrlLoseProba = (CT_INT32)(*(jsonTmp.getElementsByTagName("iCtrlLoseProba")[0])->get_int64());
            if(m_BlackListCfg.iCtrlLoseProba < 0 || m_BlackListCfg.iCtrlLoseProba > 100)
            {
                m_BlackListCfg.iCtrlLoseProba = 50;
            }
        }
        if(jsonTmp.getElementsByTagName("fCtrlLoseScoreRatio").size() > 0)
        {
            m_BlackListCfg.fCtrlLoseScoreRatio = (CT_FLOAT)(*(jsonTmp.getElementsByTagName("fCtrlLoseScoreRatio")[0])->get_double());
            if(m_BlackListCfg.fCtrlLoseScoreRatio < 0 || m_BlackListCfg.fCtrlLoseScoreRatio > 1)
            {
                m_BlackListCfg.fCtrlLoseScoreRatio = 0.5;
            }
        }
    }
	//黑名单配置
	if(!json.getElementsByTagName("HBButtonCfg").empty())
	{
        memset(&m_HBButtonCfg, 0, sizeof(m_HBButtonCfg));
		acl::json jsonTmp(*(json.getElementsByTagName("HBButtonCfg")[0]));
		acl::json_node *node3 = jsonTmp.getElementsByTagName("iCommonlyUsedAmount")[0]->first_child();
		acl::json_node *node4 = node3->first_child();
		int i = 0;
		while(node3 != NULL && node4 != NULL && i < MAX_CONFIG)
		{
			m_HBButtonCfg.iCommonlyUsedAmount[i]=((CT_INT32)(*(node4->get_int64())));
			if((CT_DWORD)(m_HBButtonCfg.iCommonlyUsedAmount[i]*TO_SCORE) > m_pGameDeskPtr->GetGameKindInfo()->dwEnterMaxScore)
            {
                m_HBButtonCfg.iCommonlyUsedAmount[i] = (i+1)*(m_pGameDeskPtr->GetGameKindInfo()->dwEnterMinScore*TO_DOUBLE);
            }
			i++;
			node4 = node3->next_child();
		}
		acl::json_node *node1 = jsonTmp.getElementsByTagName("iCommonlyUsedCount")[0]->first_child();
		acl::json_node *node2 = node1->first_child();
		i = 0;
		while(node1 != NULL && node2 != NULL && i < MAX_CONFIG)
		{
			m_HBButtonCfg.iCommonlyUsedCount[i]=((CT_INT32)(*(node2->get_int64())));
            if(m_HBButtonCfg.iCommonlyUsedCount[i] > MAX_FA_HB_COUNT)
            {
                m_HBButtonCfg.iCommonlyUsedAmount[i] = (i+1)*(MAX_FA_HB_COUNT/MAX_CONFIG);
            }
			i++;
			node2 = node1->next_child();
		}
	}
	else
    {
        memset(&m_HBButtonCfg, 0, sizeof(m_HBButtonCfg));
        for (int i = 0; i < MAX_CONFIG; ++i)
        {
            m_HBButtonCfg.iCommonlyUsedAmount[i] = (i+1)*(m_pGameDeskPtr->GetGameKindInfo()->dwEnterMinScore*TO_DOUBLE);
            m_HBButtonCfg.iCommonlyUsedAmount[i] = (i+1)*(MAX_FA_HB_COUNT/MAX_CONFIG);
        }
    }
	return;
}
//清除所有定时器
void CGameProcess::ClearAllTimer()
{
	//开始删除所有定时器
	m_pGameDeskPtr->KillGameTimer(IDI_CHECK_END);
    m_pGameDeskPtr->KillGameTimer(IDI_GAME_END_REAL);
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwHBID)
{
    if(0 == dwHBID)
    {
        LOG(ERROR) << "NormalGameEnd error hid: " << dwHBID;
        return;
    }
    auto ithb = m_FaingHB.find(dwHBID);
    if (ithb == m_FaingHB.end())
    {
        LOG(ERROR) << "NormalGameEnd error hid: " << dwHBID;
        return;
    }
    if(0 != ithb->second.dwEndTime)
    {
        LOG(ERROR) << "NormalGameEnd error hid: " << dwHBID <<" Repeated settlement";
        return;
    }
    AssignHBInfo & stAssignHBInfo = ithb->second.stAssignHBInfo;
	CT_DWORD dwFaUserID = ithb->second.stFaHBInfo.dwUserID;
    auto pFaUserInfo = m_mUserBaseData.find(dwFaUserID);
    if (pFaUserInfo == m_mUserBaseData.end())
    {
        LOG(ERROR) << "NormalGameEnd error dwFaUserID: " << dwFaUserID << " hid: " << dwHBID;
        return;
    }
    m_mGameEnd[dwHBID].dwHBID = dwHBID;
	m_mGameEnd[dwHBID].dwFaHBUserID = dwFaUserID;
    m_mGameEnd[dwHBID].wCurrFaHBChairID = m_pGameDeskPtr->GetUserChairID(m_mGameEnd[dwHBID].dwFaHBUserID);
    m_mGameEnd[dwHBID].iFaHBTotalAmount = ithb->second.stFaHBInfo.iHBAmount;
    m_mGameEnd[dwHBID].cbNoGrabHBCount = (CT_BYTE)stAssignHBInfo.wRemainHBCount;
	CT_LONGLONG llBackTotalAmount = stAssignHBInfo.llRemainAmount;
	CT_LONGLONG llGrabedTotalAmount = stAssignHBInfo.llTotalAmount - stAssignHBInfo.llRemainAmount;
	CT_LONGLONG llThunderBackTotalAmount = 0;
	CT_LONGLONG llThunderAmount = 0;
	CT_INT32 iThunderCount = 0;
    CT_INT32 iTotalThunderCount = 0;
    CT_LONGLONG llSelfGrabAmount = 0;
	for (int j = 0; j < ithb->second.stFaHBInfo.iHBAllotCount; ++j)
	{
		if(INVALID_CHAIR != stAssignHBInfo.wChairID[j] && stAssignHBInfo.bIsThunder[j])
		{
			llThunderBackTotalAmount += stAssignHBInfo.llThunderBackAmount[j];
			if(0 != stAssignHBInfo.llThunderBackAmount[j])
            {
			    //等于0 表示自己中雷不算
                iThunderCount++;
                llThunderAmount += stAssignHBInfo.llOutHB[j];
            }
            iTotalThunderCount++;
		}
		if(stAssignHBInfo.dwUserID[j] == dwFaUserID)
        {
            llSelfGrabAmount = stAssignHBInfo.llOutHB[j];
        }
	}
    m_mGameEnd[dwHBID].dBackTotalAmount = llBackTotalAmount*TO_DOUBLE;
    m_mGameEnd[dwHBID].dGrabedTotalAmount = llGrabedTotalAmount*TO_DOUBLE;
    m_mGameEnd[dwHBID].cbGrabedHBCount = (CT_BYTE)(ithb->second.stFaHBInfo.iHBAllotCount-stAssignHBInfo.wRemainHBCount);
    m_mGameEnd[dwHBID].dThunderBackTotalAmount = llThunderBackTotalAmount*TO_DOUBLE;
	CT_LONGLONG llTotalWkin = 0;
	CT_LONGLONG llRevenue = 0;
    llTotalWkin = llThunderBackTotalAmount-(llGrabedTotalAmount-llSelfGrabAmount)+stAssignHBInfo.llFaWinColorPoolAmount;
	if(llTotalWkin > 0)
	{
		llRevenue = m_pGameDeskPtr->CalculateRevenue(ithb->second.stFaHBInfo.wChairID, llTotalWkin);
		m_mGameEnd[dwHBID].dRevenue = llRevenue*TO_DOUBLE;
	}
	m_mGameEnd[dwHBID].dWinOrLose = (llTotalWkin - llRevenue)*TO_DOUBLE;

    CT_WORD wNewChairID = INVALID_CHAIR;
	if(!pFaUserInfo->second.bRobot)
    {
        wNewChairID = m_pGameDeskPtr->GetUserChairID(dwFaUserID);
        ithb->second.stFaHBInfo.wChairID = wNewChairID;
    }
	else
    {
	    //机器人红包没有发完不会离开，第二不能通过此接口获取机器人的座位号
        wNewChairID = ithb->second.stFaHBInfo.wChairID;
    }
    CT_LONGLONG llScore = (llBackTotalAmount+llThunderBackTotalAmount-llRevenue+stAssignHBInfo.llFaWinColorPoolAmount);
    m_mGameEnd[dwHBID].dWinColorPotAmount = stAssignHBInfo.llFaWinColorPoolAmount*TO_DOUBLE;
	m_mOnlinePlayerTotalExploits[dwFaUserID] += ((llTotalWkin-llRevenue)*TO_DOUBLE);
    m_mGameEnd[dwHBID].dTotalWinLose = m_mOnlinePlayerTotalExploits[dwFaUserID];
	pFaUserInfo->second.llWinLoseScore += (llTotalWkin-llRevenue);
    pFaUserInfo->second.llScore += llScore;
	if(wNewChairID != INVALID_CHAIR)
	{
	    //CT_LONGLONG llScoreBefore = m_pGameDeskPtr->GetUserScore(wNewChairID);
		m_pGameDeskPtr->AddUserScore(wNewChairID, llScore, true);
        //LOG(ERROR)<< dwHBID << " dwUserID:" <<m_mGameEnd[dwHBID].dwFaHBUserID <<" NormalGameEnd llScoreBefore: " << llScoreBefore << " llScore: " << llScore << " llScoreAfter: " << m_pGameDeskPtr->GetUserScore(wNewChairID);
	}
	else
	{
	    if(llScore > 0)
        {
            CT_CHAR cbTmp[1000] = {0};
	        if(SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID)
            {
                _snprintf_info(cbTmp, sizeof(cbTmp)-1, "您在红包扫雷-乱斗模式中发起的%.2lf元（%d包，%.2lf倍，雷点为%d）的红包已结算。", (CT_DOUBLE)ithb->second.stFaHBInfo.iHBAmount, ithb->second.stFaHBInfo.iHBAllotCount, ithb->second.stFaHBInfo.dMultiple, ithb->second.stFaHBInfo.cbThunderNO);
            }
            else
            {
                _snprintf_info(cbTmp, sizeof(cbTmp)-1, "您在红包扫雷-计时模式中发起的%.2lf元（%d包，%.2lf倍，雷点为%d）的红包已结算。", (CT_DOUBLE)ithb->second.stFaHBInfo.iHBAmount, m_pHBRoomConfig->iHBAllotCount, m_pHBRoomConfig->fMultiple, ithb->second.stFaHBInfo.cbThunderNO);
            }

            if(stAssignHBInfo.wRemainHBCount == ithb->second.stFaHBInfo.iHBAllotCount)
            {
                _snprintf_info(cbTmp+strlen(cbTmp), sizeof(cbTmp)-1, "其中，抢夺人数%d人，" ,ithb->second.stFaHBInfo.iHBAllotCount - stAssignHBInfo.wRemainHBCount);
            }
            else if(stAssignHBInfo.wRemainHBCount > 0)
            {
                _snprintf_info(cbTmp+strlen(cbTmp), sizeof(cbTmp)-1, "其中，抢夺人数%d人，总中雷%d人，其他玩家中雷%d人，其他玩家中雷总额为%.2lf元，红包剩余金额为%.2lf元。\n"
                        ,ithb->second.stFaHBInfo.iHBAllotCount-stAssignHBInfo.wRemainHBCount, iTotalThunderCount, iThunderCount, (CT_DOUBLE)(llThunderAmount*TO_DOUBLE), (CT_DOUBLE)(llBackTotalAmount*TO_DOUBLE));
            }
            else if(0 == stAssignHBInfo.wRemainHBCount)
            {
                _snprintf_info(cbTmp+strlen(cbTmp), sizeof(cbTmp)-1, "其中，抢夺人数%d人，总中雷%d人，其他玩家中雷%d人，其他玩家中雷总额为%.2lf元。\n"
                        ,ithb->second.stFaHBInfo.iHBAllotCount-stAssignHBInfo.wRemainHBCount, iTotalThunderCount, iThunderCount, (CT_DOUBLE)(llThunderAmount*TO_DOUBLE));
            }
			if(stAssignHBInfo.llFaWinColorPoolAmount > 0)
			{
				_snprintf_info(cbTmp+strlen(cbTmp), sizeof(cbTmp)-1, "获得彩金大奖%.2lf元。\n" ,stAssignHBInfo.llFaWinColorPoolAmount*TO_DOUBLE);
			}
			_snprintf_info(cbTmp+strlen(cbTmp), sizeof(cbTmp)-1, "您最终结算总额为：%.2lf元。", (CT_DOUBLE)(llScore*TO_DOUBLE));
            tagUserMail userMail;
            userMail.cbMailType = 1;
            userMail.cbState = 0;
            userMail.dwSendTime = Utility::GetTime();
            userMail.dwExpiresTime = userMail.dwSendTime + 86400 * 30; //默认一个月的有效期
            userMail.dwUserID = m_mGameEnd[dwHBID].dwFaHBUserID;
            userMail.llScore = llScore;
            userMail.cbScoreChangeType = HBSL_MAIL;
            _snprintf_info(userMail.szTitle, sizeof(userMail.szTitle)-1, "%s", "红包扫雷游戏战况");
            memcpy(userMail.szContent, cbTmp, sizeof(userMail.szContent));
            m_pGameDeskPtr->SendUserMail(&userMail);
            //LOG(ERROR)<< "SendUserMail uid: " << userMail.dwUserID<< " " << userMail.szContent;
        }
	}

	auto userCtrl = m_mapUserPointControl.find(dwFaUserID);
	if(userCtrl != m_mapUserPointControl.end())
	{
		userCtrl->second.llCurrDianKongFen += (llTotalWkin - llRevenue);
		MSG_G2DB_User_PointControl data;
		memset(&data, 0, sizeof(data));
		data = userCtrl->second;
        if(en_PlayerOutScore == data.iDianKongZhi)
        {
            data.llDianKongFen *= -1;
        }
        m_pGameDeskPtr->SavePlayerDianKongData(data);
	}

	//记录库存
	if(pFaUserInfo->second.bRobot)
    {
        m_sllStockScore += (llThunderBackTotalAmount + llBackTotalAmount);
    }

	m_sllFaWinScore += (llThunderBackTotalAmount-llGrabedTotalAmount);

    CT_DWORD index = INVALID_CHAIR;
	CT_LONGLONG llRobotTotalRevenue = 0;
    CT_LONGLONG llRealTotalRevenue = 0;
	for (CT_DWORD i = 0; i < m_mRecordScoreInfo[dwHBID].size(); ++i)
	{
		if(m_mRecordScoreInfo[dwHBID][i].cbIsAndroid)
        {
            llRobotTotalRevenue += m_mRecordScoreInfo[dwHBID][i].dwRevenue;
        }
		else
        {
            llRealTotalRevenue += m_mRecordScoreInfo[dwHBID][i].dwRevenue;
        }
	}
	index = m_mRecordScoreInfo[dwHBID].size();
	{
		RecordScoreInfo recordScoreInfo;
		memset(&recordScoreInfo, 0, sizeof(recordScoreInfo));
		recordScoreInfo.dwUserID = dwFaUserID;
        recordScoreInfo.llSourceScore = ithb->second.llFaSourceScore;
        recordScoreInfo.cbIsAndroid = pFaUserInfo->second.bRobot;
        recordScoreInfo.cbStatus = 1;
        if(wNewChairID != INVALID_CHAIR)
        {
            recordScoreInfo.iScore = llTotalWkin;
        }
        else
        {
            recordScoreInfo.iScore = ((ithb->second.stFaHBInfo.iHBAmount*TO_SCORE*-1)+llRevenue);
        }
        recordScoreInfo.dwRevenue = llRevenue;
		m_mRecordScoreInfo[dwHBID].push_back(recordScoreInfo);
        if(m_mRecordScoreInfo[dwHBID][index].cbIsAndroid)
        {
            llRobotTotalRevenue += llRevenue;
        }
        else
        {
            llRealTotalRevenue += llRevenue;
        }
        ChangeFaSource(dwFaUserID,llTotalWkin-llRevenue);
		//LOG(ERROR) << "Record " << dwFaUserID<< " llSourceScore::" << recordScoreInfo.llSourceScore << " iScore:" << recordScoreInfo.iScore << " dwRevenue:" << recordScoreInfo.dwRevenue;
	}

	//记录基本信息
	RecordScoreInfo stRecordScoreInfo[MAX_HB_COUNT+1];
	for (CT_DWORD i = 0; i < (MAX_HB_COUNT+1) && i < m_mRecordScoreInfo[dwHBID].size(); ++i)
	{
		stRecordScoreInfo[i] = m_mRecordScoreInfo[dwHBID][i];
	}


    CT_BOOL bIsAllRobot = true;
    RecordHBSLInfo HbInfo;
    memset(&HbInfo,0, sizeof(HbInfo));
    HbInfo.dwSendUserID = ithb->second.stFaHBInfo.dwUserID;
    HbInfo.dwHongbaoID = dwHBID;        //红包ID
    HbInfo.dwAmount = ithb->second.stFaHBInfo.iHBAmount*TO_SCORE;           //红包金额(单位:分)
    HbInfo.wAllotCount = ithb->second.stFaHBInfo.iHBAllotCount;		//分包个数
    HbInfo.dMultiple = ithb->second.stFaHBInfo.dMultiple;         //红包赔率
    HbInfo.cbThunderNO = ithb->second.stFaHBInfo.cbThunderNO;        //雷号
    HbInfo.dwSendTime = ithb->second.dwGrabTime;         //发送红包的时间戳
    HbInfo.dwWinColorPrize = (CT_DWORD)stAssignHBInfo.llFaWinColorPoolAmount;

	m_GameLogic.SetColorPool((llRobotTotalRevenue + llRealTotalRevenue) * COLOR_POOL_RATIO);
    m_GameLogic.SetRealColorPool(llRealTotalRevenue * COLOR_POOL_RATIO);
    m_GameLogic.SetRobotColorPool(llRobotTotalRevenue * COLOR_POOL_RATIO);
	m_sllTotalRealRevenue += llRealTotalRevenue;
    if(!pFaUserInfo->second.bRobot)
    {
        bIsAllRobot = false;
    }
    RecordHBSLGrabInfo GrabInfo[MAX_HB_COUNT];
    memset(GrabInfo, 0, sizeof(GrabInfo));
    int iGrabIndex = 0;

	for (auto ait = ithb->second.vOpenHBList.begin(); ait != ithb->second.vOpenHBList.end(); ++ait)
	{
	    if((bIsAllRobot)&&(!m_mUserBaseData[ait->dwUserID].bRobot))
        {
            bIsAllRobot = false;
        }
		//下面是确保数据库内的记录顺序与玩家真实操作的顺序一致
        CT_DWORD k = 0;
		for (; k < MAX_HB_COUNT; ++k) {
			if (stAssignHBInfo.wChairID[k] == INVALID_CHAIR) {
				continue;
			}
			if (stAssignHBInfo.dwUserID[k] == ait->dwUserID)
			{
				break;
			}
		}
		if(k == MAX_HB_COUNT)
		{
			LOG(ERROR)<<"record error!";
			continue;
		}
		GrabInfo[iGrabIndex].dwUserID = stAssignHBInfo.dwUserID[k];           //用户ID
		GrabInfo[iGrabIndex].iScore = (stAssignHBInfo.llOutHB[k]-stAssignHBInfo.llThunderBackAmount[k]);            //抢到的红包金额(单位:分)
		GrabInfo[iGrabIndex].cbIsThunder = stAssignHBInfo.bIsThunder[k];        //是否中雷
        GrabInfo[iGrabIndex].dwWinColorPrize = stAssignHBInfo.llGrabWinColorPoolAmount[k];
		++iGrabIndex;
	}
	if(m_GameLogic.GetColorPool() < COLOR_POOL_INIT_AMOUNR)
	{
		m_GameLogic.SetColorPool(COLOR_POOL_INIT_AMOUNR);
	}
	m_mGameEnd[dwHBID].dTotalColorPotAmount = m_GameLogic.GetColorPool()*TO_DOUBLE;
    ithb->second.dwEndTime = time(NULL);

    FillGrabHBInfo(dwHBID, 0, m_mGameEnd[dwHBID].stGrabHBPlayer);

	SendTableData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &(m_mGameEnd[dwHBID]), sizeof(m_mGameEnd[dwHBID]), false);

	if(!bIsAllRobot)
    {
		if(m_vColorPoolCfg.size() <= 0 && SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID)
		{
			m_GameLogic.SetRealColorPool(m_GameLogic.GetRealColorPool()*-1);
		}
        m_pGameDeskPtr->RecordRawInfo(stRecordScoreInfo, (CT_WORD)m_mRecordScoreInfo[dwHBID].size(), NULL, 0, dwFaUserID, m_sllStockScore, m_GameLogic.GetRealColorPool()/*按伟哥要求将机器人库存当作真人彩池*/, 0, 0, 0);
	    //如果全是机器人就不写记录
        m_pGameDeskPtr->RecordHongBaoSLInfo(&HbInfo, GrabInfo, ithb->second.vOpenHBList.size());
    }
	/*LOG(ERROR)<<dwHBID << " RealColorPool:" << m_GameLogic.GetRealColorPool() << " llRealTotalRevenue: " << llRealTotalRevenue
			  << " ColorPool:" << m_GameLogic.GetColorPool() << " TotalRevenue:" << llRobotTotalRevenue + llRealTotalRevenue << " RobotColorPool:" << m_GameLogic.GetRobotColorPool()
			  << " llRobotTotalRevenue:" << llRobotTotalRevenue<< " llFaWinColorPoolAmount:"<<stAssignHBInfo.llFaWinColorPoolAmount;*/
    m_cbLastRealPlayerCount = m_mRealPlayerCount[dwHBID];

    //必须在删除之前调用点控函数
	JudgeIsPointCtrl(dwHBID);

   if(((SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID) && ((m_FaingHB.size() >= MAX_DISPLAY_HB_COUNT)||((ithb->second.dwEndTime-ithb->second.dwGrabTime)>=TIME_CHECK_HB_SMAIL_END)))
   || (SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID))
   {
	   DeleteHBByHBID(dwHBID);
   }

	CalculateRankings();

    if(SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID)
    {
        m_pGameDeskPtr->SetGameTimer(IDI_GAME_END_REAL, TIME_GAME_END_REAL*1000);
    }
    else
    {
        m_pGameDeskPtr->SetGameTimer(IDI_GAME_END_REAL, 500);
    }

	//LOG(ERROR) <<dwHBID <<" NormalGameEnd Stock: "<< m_sllStockScore << " FaingMapSize: " << m_FaingHB.size();

	if(0 == dwHBID%50)
	{
		/*LOG(INFO) << dwHBID <<" NormalGameEnd Stock:"<< m_sllStockScore << " m_sllFaWinScore:" << m_sllFaWinScore
		<< " m_sllGrabWinScore:" << m_sllGrabWinScore<< " ColorPool:" << m_GameLogic.GetColorPool()
		<< " RealColorPool:" << m_GameLogic.GetRealColorPool() << " RobotColorPool:" << m_GameLogic.GetRobotColorPool()
		<< " m_sllRealColorPoolInitValue:" << m_sllRealColorPoolInitValue
		<< " m_sllTotalRealRevenue:" << m_sllTotalRealRevenue << " RealOutColorPool:" << m_GameLogic.GetRealOutColorPool();*/
	}
    //查看100人输赢比例
    /*if(m_dwHBIDIndex > 10100)
    {
        int iWinCount = 0;
        int iTotalPlayerCount = 0;
        for (auto it = m_mUserBaseData.begin(); it != m_mUserBaseData.end(); ++it)
        {
            CT_CHAR szTmp[1024] = {0};
            iTotalPlayerCount++;
            _snprintf_info(szTmp, sizeof(szTmp), "总合 uid: %d, WinLoseScore: %.2lf", it->first, (CT_DOUBLE)(it->second.llWinLoseScore*TO_DOUBLE));
            LOG(INFO) << szTmp;
            if(it->second.llWinLoseScore > 0)
            {
                iWinCount++;
            }
        }
        CT_CHAR szTmp[1024] = {0};
        _snprintf_info(szTmp, sizeof(szTmp), "总人数：%d,赢的人数：%d, 占比：%.2lf", iTotalPlayerCount, iWinCount, ((CT_DOUBLE)iWinCount/(CT_DOUBLE)iTotalPlayerCount)*100);
        LOG(INFO) << szTmp<< "%";

        iWinCount = 0;
        iTotalPlayerCount = 0;
        for (auto it = m_mOnlinePlayerExploits.begin(); it != m_mOnlinePlayerExploits.end(); ++it)
        {
            if(0 == it->second.wPlayCount)
            {
                continue;
            }
            CT_CHAR szTmp[1024] = {0};
            iTotalPlayerCount++;
            _snprintf_info(szTmp, sizeof(szTmp), "抢 uid: %d, WinLoseScore: %.2lf", it->first, it->second.dWinLoseScore);
            LOG(INFO) << szTmp;
            if(it->second.dWinLoseScore > 0)
            {
                iWinCount++;
            }
        }
        memset(szTmp, 0, sizeof(szTmp));
        _snprintf_info(szTmp, sizeof(szTmp), "抢总人数：%d,赢的人数：%d, 占比：%.2lf", iTotalPlayerCount, iWinCount, ((CT_DOUBLE)iWinCount/(CT_DOUBLE)iTotalPlayerCount)*100);
        LOG(INFO) << szTmp<< "%";
    }
    if(m_dwHBIDIndex > 10100)
    {
        LOG(INFO) << "sss" << "%";
    }*/
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

//设置捕鱼点控数据
CT_BOOL CGameProcess::SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData)
{
    auto it = m_mapUserPointControl.find(pDianKongData->dwUserID);
	if(pDianKongData->iDianKongZhi < 0)
	{
		pDianKongData->iDianKongZhi = -1;
	}
	else if(pDianKongData->iDianKongZhi > 0)
	{
		pDianKongData->iDianKongZhi = 1;
	}
	else
    {
	    if(it != m_mapUserPointControl.end())
        {
            m_mapUserPointControl.erase(it);
        }
	    else
        {
	        LOG(ERROR) << "SetFishDianKongData uid: " << pDianKongData->dwUserID << " iDianKongZhi: " << pDianKongData->iDianKongZhi << " llCurrDianKongFen:" << pDianKongData->llCurrDianKongFen;
        }
        return true;
    }

    if (it != m_mapUserPointControl.end())
    {
		it->second.dwUserID = pDianKongData->dwUserID;
		it->second.dwGameIndex = pDianKongData->dwGameIndex;
		it->second.iDianKongZhi = pDianKongData->iDianKongZhi;
        it->second.llDianKongFen = pDianKongData->llDianKongFen;
        if(en_PlayerOutScore == it->second.iDianKongZhi)
        {
            it->second.llDianKongFen *= -1;
        }
        LOG(INFO) << "WEB modify DianKong " << pDianKongData->dwUserID << " iDianKongZhi:"<<pDianKongData->iDianKongZhi<< " llDianKongFen:"<<pDianKongData->llDianKongFen;
    }
    else if(en_NoCtrl != pDianKongData->iDianKongZhi)
    {
		MSG_G2DB_User_PointControl User_PointControl;
		memset(&User_PointControl,0,sizeof(User_PointControl));
		User_PointControl.dwUserID = pDianKongData->dwUserID;
		User_PointControl.dwGameIndex = pDianKongData->dwGameIndex;
		User_PointControl.iDianKongZhi = pDianKongData->iDianKongZhi;
        User_PointControl.llDianKongFen = pDianKongData->llDianKongFen;
        if(en_PlayerOutScore == User_PointControl.iDianKongZhi)
        {
            User_PointControl.llDianKongFen *= -1;
        }
        User_PointControl.llCurrDianKongFen = pDianKongData->llCurrDianKongFen;
        m_mapUserPointControl.insert(std::make_pair(pDianKongData->dwUserID, User_PointControl));
        LOG(INFO) << "WEB join DianKong " << pDianKongData->dwUserID << " iDianKongZhi:"<<pDianKongData->iDianKongZhi<< " llDianKongFen:"<<pDianKongData->llDianKongFen;
    }
    return true;
}

//是否在抢红包列表内
CT_BOOL CGameProcess::IsInOpenHBListByUserID(CT_DWORD dwUserID)
{
    CT_BOOL bIsHas = false;
	for (auto it = m_FaingHB.begin(); it != m_FaingHB.end(); ++it) {
		for (CT_DWORD i = 0; i < it->second.vOpenHBList.size(); ++i)
		{
			if(dwUserID == it->second.vOpenHBList[i].dwUserID)
			{
				bIsHas=true;
				break;
			}
		}
	}

    return bIsHas;
}
//是否在申请列表内
CT_BOOL CGameProcess::IsInApplyListByUserID(CT_DWORD dwUserID)
{
    CT_BOOL bIsHas = false;
    for (auto it = m_ApplyList.begin(); it !=  m_ApplyList.end(); ++it)
    {
        if(dwUserID == it->stFaHBInfo.dwUserID)
        {
            bIsHas = true;
            break;
        }
    }
	for (auto it = m_FaingHB.begin(); it != m_FaingHB.end() && (!bIsHas); ++it)
	{
		if(dwUserID == it->second.stFaHBInfo.dwUserID)
		{
			bIsHas = true;
			break;
		}
	}
    return bIsHas;
}
//获取某人在申请发红包列表内的未的发红包个数
CT_WORD CGameProcess::GetHBCountInApplyListByUserID(CT_DWORD dwUserID)
{
	CT_WORD wHBCount = 0;
	for (auto it = m_ApplyList.begin(); it !=  m_ApplyList.end(); ++it)
	{
		if(dwUserID == it->stFaHBInfo.dwUserID)
		{
			++wHBCount;
		}
	}
	for (auto it = m_FaingHB.begin(); it != m_FaingHB.end(); ++it)
	{
		if(dwUserID == it->second.stFaHBInfo.dwUserID && it->second.stAssignHBInfo.wRemainHBCount > 0)
		{
			++wHBCount;
		}
	}
	return wHBCount;
}
//删除用户战绩及排行榜
CT_VOID CGameProcess::DeleteUserByChairID(CT_WORD wChairID)
{
	CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(wChairID);

    //删除机器人信息
    if(m_mUserBaseData[dwUserID].bRobot)
    {
        m_mUserBaseData.erase(dwUserID);
        for (auto i = m_vRobotChairID.begin(); i != m_vRobotChairID.end() ; ++i)
        {
            if(wChairID == *i)
            {
                m_vRobotChairID.erase(i);
                break;
            }
        }
    }

    //删除在线玩家战绩
    auto it = m_mOnlinePlayerExploits.find(dwUserID);
    if(it != m_mOnlinePlayerExploits.end())
    {
        m_mOnlinePlayerExploits.erase(it);
    }

	//删除在线玩家战绩
	auto its = m_mOnlinePlayerTotalExploits.find(dwUserID);
	if(its != m_mOnlinePlayerTotalExploits.end())
	{
		m_mOnlinePlayerTotalExploits.erase(its);
	}

    //某人的排行榜信息
    for (auto its = m_PlayerCountsRanking.begin(); its != m_PlayerCountsRanking.end(); ++its)
    {
        if(its->dwUserID == dwUserID)
        {
            m_PlayerCountsRanking.erase(its);
            break;
        }
    }
    //计算排行榜
    /*for (auto its = m_PlayerWinScoresRanking.begin(); its != m_PlayerWinScoresRanking.end(); ++its)
    {
        if(its->dwUserID == dwUserID)
        {
            m_PlayerWinScoresRanking.erase(its);
            break;
        }
    }*/


	if(m_pGameDeskPtr->IsAndroidUser(wChairID))
	{
		m_pGameDeskPtr->ClearTableUser(wChairID, true);
	}

    return;
}
CT_VOID CGameProcess::SendRankings(CT_DWORD dwChairID)
{
    CT_BYTE  szBuffer[4096];
    memset(szBuffer,0, sizeof(szBuffer));
    CT_DWORD dwSendSize = 0;
    for (auto it = m_PlayerCountsRanking.begin();
         (it !=  m_PlayerCountsRanking.end() && (dwSendSize + sizeof(GS_OnlinePlayerExploits)) <= sizeof(szBuffer)
          && (dwSendSize/sizeof(GS_OnlinePlayerExploits) < 200)); ++it)
    {
        memcpy(szBuffer+dwSendSize, &(*it), sizeof(GS_OnlinePlayerExploits));
        dwSendSize += sizeof(GS_OnlinePlayerExploits);
    }
    SendTableData(dwChairID, SUB_S_RANKING_LIST_BY_COUNT, szBuffer, dwSendSize, false);

    /*memset(szBuffer,0, sizeof(szBuffer));
    dwSendSize = 0;
    for (auto it = m_PlayerWinScoresRanking.begin();
         (it !=  m_PlayerWinScoresRanking.end() && (dwSendSize + sizeof(GS_OnlinePlayerExploits)) <= sizeof(szBuffer)
          && (dwSendSize/sizeof(GS_OnlinePlayerExploits) < 5)); ++it)
    {
        memcpy(szBuffer+dwSendSize, &(*it), sizeof(GS_OnlinePlayerExploits));
        dwSendSize += sizeof(GS_OnlinePlayerExploits);
    }
    SendTableData(dwChairID, SUB_S_RANKING_LIST_BY_WINLOSE, szBuffer, dwSendSize, false);*/
    return;
}
//计算排行榜
CT_VOID CGameProcess::CalculateRankings(CT_BOOL bIsCalculate/* = true*/)
{
    CT_DWORD newTime = time(NULL);
    static CT_DWORD oldTime = 0;
    if((!bIsCalculate) && (newTime - oldTime < 8))
    {
        return;
    }
    oldTime = newTime;
    //计算排行榜
    m_PlayerCountsRanking.clear();
    for (auto it = m_mOnlinePlayerExploits.begin(); it != m_mOnlinePlayerExploits.end(); ++it)
    {
        m_PlayerCountsRanking.push_back(it->second);
    }

    std::sort(m_PlayerCountsRanking.begin(),m_PlayerCountsRanking.end(),CompareWinLoseScore);
    std::sort(m_PlayerCountsRanking.begin(),m_PlayerCountsRanking.end(),ComparePlayCount);

    /*if(SENIOR_ROOM == m_pGameRoomKindInfo->wRoomKindID)
    {
        //计算排行榜
        m_PlayerWinScoresRanking.clear();
        for (auto it = m_mOnlinePlayerExploits.begin(); it != m_mOnlinePlayerExploits.end(); ++it)
        {
            m_PlayerWinScoresRanking.push_back(it->second);
        }

        std::sort(m_PlayerWinScoresRanking.begin(),m_PlayerWinScoresRanking.end(),ComparePlayCount);
        std::sort(m_PlayerWinScoresRanking.begin(),m_PlayerWinScoresRanking.end(),CompareWinLoseScore);

        //SendRankings();
    }*/
}
//判断是否需要点控
CT_VOID CGameProcess::JudgeIsPointCtrl(CT_DWORD dwHBID)
{
    auto ithb = m_FaingHB.find(dwHBID);
    if(ithb == m_FaingHB.end())
    {
        LOG(ERROR) << "JudgeIsPointCtrl dwHBID:" << dwHBID;
        return;
    }
    //加入黑名单
    for (auto k = ithb->second.vOpenHBList.begin(); k != ithb->second.vOpenHBList.end(); ++k)
    {
        auto pGrabUser = m_mUserBaseData.find(k->dwUserID);
        auto pBlackUser = m_mapBlackList.find(k->dwUserID);
        if(pGrabUser == m_mUserBaseData.end())
        {
            continue;
        }
        if(pGrabUser->second.bRobot)
        {
            continue;
        }
        GS_UserBaseData* pUserBaseData = m_pGameDeskPtr->GetUserBaseData(m_pGameDeskPtr->GetUserID(k->dwUserID));

        if(pUserBaseData && (pGrabUser->second.dwContinueGrabLastOneHB > (CT_DWORD)m_BlackListCfg.iContinueGrabLastOneHB) && (pBlackUser == m_mapBlackList.end()))
        {
            tagHBGameBlackList GameBlack;
            memset(&GameBlack, 0, sizeof(GameBlack));
            GameBlack.dwUserID = k->dwUserID;
            GameBlack.wUserLostRatio = m_BlackListCfg.iCtrlLoseProba;
            GameBlack.llControlScore = ((pUserBaseData->llTotalWinScore > 0) ? (pUserBaseData->llTotalWinScore*m_BlackListCfg.fCtrlLoseScoreRatio) : pGrabUser->second.llWinLoseScore);
            if(GameBlack.llControlScore < 0)
            {
                continue;
            }
            tagBrGameBlackList brGameBlackList;
            memset(&brGameBlackList, 0, sizeof(brGameBlackList));
            brGameBlackList.dwUserID = GameBlack.dwUserID;
            brGameBlackList.wUserLostRatio = GameBlack.wUserLostRatio;
            brGameBlackList.dwControlScore = (CT_DWORD)GameBlack.llControlScore;
            GameBlack.llControlScore *= -1;
            LOG(INFO) << "join black list UserID:" << GameBlack.dwUserID << " UserLostRatio:"<<GameBlack.wUserLostRatio << " llControlScore:" << GameBlack.llControlScore;
            m_mapBlackList.insert(std::make_pair(k->dwUserID, GameBlack));
            m_pGameDeskPtr->SetBlackListUser(&brGameBlackList);
        }
    }
    //删除黑名单
    for (auto i = m_mapBlackList.begin(); i != m_mapBlackList.end();)
    {
        if(i->second.llCurrControlScore <= i->second.llControlScore)
        {
            tagBrGameBlackList brGameBlackList;
            memset(&brGameBlackList, 0, sizeof(brGameBlackList));
            brGameBlackList.dwUserID = i->second.dwUserID;
            brGameBlackList.wUserLostRatio = 0;
            brGameBlackList.dwControlScore = (CT_DWORD)((i->second.llControlScore < 0)?(i->second.llControlScore*-1):i->second.llControlScore);
            m_pGameDeskPtr->SetBlackListUser(&brGameBlackList);
            LOG(INFO) << "delete black list UserID:" << i->first << " UserLostRatio:"<<i->second.wUserLostRatio << " llControlScore:" << i->second.llControlScore << " llCurrControlScore:" << i->second.llCurrControlScore;
            i = m_mapBlackList.erase(i);
        }
        else
        {
            ++i;
        }
    }

    CT_DWORD dwTime = time(NULL);
	//是判断是否点控结束
	for (auto it = m_mapUserPointControl.begin(); it != m_mapUserPointControl.end();)
	{
		if((en_PlayerEatScore == it->second.iDianKongZhi) && (it->second.llCurrDianKongFen >= it->second.llDianKongFen))
		{
			it->second.iDianKongZhi = en_NoCtrl;
			m_pGameDeskPtr->SavePlayerDianKongData(it->second);
			m_mUserBaseData[it->first].dwPointCtlEndTime = dwTime;
            LOG(INFO)<<"DianKong Delete " << it->first <<" iDianKongZhi:"<< it->second.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(it->second.llDianKongFen*TO_DOUBLE) << " CurrDianKongFen:" << (CT_DOUBLE)(it->second.llCurrDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(m_mUserBaseData[it->first].llWinLoseScore*TO_DOUBLE);
			it = m_mapUserPointControl.erase(it);
		}
		else if((en_PlayerOutScore == it->second.iDianKongZhi) && (it->second.llCurrDianKongFen <= it->second.llDianKongFen))
		{
			it->second.iDianKongZhi = en_NoCtrl;
			MSG_G2DB_User_PointControl data;
			memset(&data, 0, sizeof(data));
			data = it->second;
			data.llDianKongFen *= -1;
			m_pGameDeskPtr->SavePlayerDianKongData(data);
            m_mUserBaseData[it->first].dwPointCtlEndTime = dwTime;
            LOG(INFO)<<"DianKong Delete " << it->first <<" iDianKongZhi:"<< it->second.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(it->second.llDianKongFen*TO_DOUBLE) << " CurrDianKongFen:" << (CT_DOUBLE)(it->second.llCurrDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(m_mUserBaseData[it->first].llWinLoseScore*TO_DOUBLE);
			it = m_mapUserPointControl.erase(it);
		}
		else
		{
			++it;
		}
	}

    if(!m_bCtrlMasterSwitch)
    {
        return;
    }
	//判断抢红包的玩家要不要进行点控
	for (auto k = ithb->second.vOpenHBList.begin(); k != ithb->second.vOpenHBList.end(); ++k)
	{
		CT_DWORD dwUserID = k->dwUserID;
		//CT_INT32 iRand = (RAND_NUM(m_dwRandIndex++)%100);
		auto ctrl = m_mapUserPointControl.find(dwUserID);
		auto userBase = m_mUserBaseData.find(dwUserID);
		if(userBase == m_mUserBaseData.end())
        {
            //LOG(ERROR) << "userBase == m_mUserBaseData.end() dwUserID:" << dwUserID;
            continue;
        }
		if(userBase->second.bRobot)
        {
            continue;
        }
		for (auto zs = m_PointCtrlCfg.vPointCtrlTable.begin(); (zs != m_PointCtrlCfg.vPointCtrlTable.end() && (ctrl == m_mapUserPointControl.end())); ++zs)
		{
			if ((zs->llChgScoreLowerLimit <= m_mUserBaseData[dwUserID].llWinLoseScore) && (m_mUserBaseData[dwUserID].llWinLoseScore < zs->llChgScoreUpperLimit)
			&& ((int)m_iProba(m_gen) < zs->iProba) && ((dwTime - m_mUserBaseData[dwUserID].dwPointCtlEndTime) > (CT_DWORD)m_PointCtrlCfg.iTimeInterval))
			{
				MSG_G2DB_User_PointControl PointControl;
				memset(&PointControl, 0, sizeof(PointControl));
				PointControl.dwUserID = dwUserID;
				PointControl.iDianKongZhi =zs->iCtrlType;
				PointControl.llDianKongFen = ((RAND(zs->iCtlChgPstLowerLimit,zs->iCtlChgPstUpperLimit,m_dwRandIndex++))*TO_DOUBLE*(userBase->second.llWinLoseScore)*(-1));
				m_mapUserPointControl.insert(std::make_pair(dwUserID,PointControl));
				if(en_PlayerOutScore == PointControl.iDianKongZhi)
                {
                    PointControl.llDianKongFen *= -1;
                }
                m_pGameDeskPtr->SavePlayerDianKongData(PointControl);
                LOG(INFO)<<"DianKong Join Grab " << dwUserID <<" iDianKongZhi:"<< PointControl.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(PointControl.llDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(m_mUserBaseData[dwUserID].llWinLoseScore*TO_DOUBLE);
				break;
			}
		}
	}
    //CT_INT32 iRand = (RAND_NUM(m_dwRandIndex++)%100);
	CT_DWORD dwFaUserID = ithb->second.stFaHBInfo.dwUserID;
	auto ctrl = m_mapUserPointControl.find(dwFaUserID);
	auto userBase = m_mUserBaseData.find(dwFaUserID);
    if(userBase == m_mUserBaseData.end())
    {
        LOG(ERROR) << "userBase == m_mUserBaseData.end() dwFaUserID:" << dwFaUserID;
        return;
    }

	if((ctrl == m_mapUserPointControl.end())&& (userBase != m_mUserBaseData.end()) && (!userBase->second.bRobot))
	{
		for (auto zs = m_PointCtrlCfg.vPointCtrlTable.begin(); (zs != m_PointCtrlCfg.vPointCtrlTable.end() && (ctrl == m_mapUserPointControl.end())); ++zs)
		{
			if ((zs->llChgScoreLowerLimit <= userBase->second.llWinLoseScore) && (userBase->second.llWinLoseScore < zs->llChgScoreUpperLimit)
			&& ((int)m_iProba(m_gen) < zs->iProba) && ((dwTime - userBase->second.dwPointCtlEndTime) > (CT_DWORD)m_PointCtrlCfg.iTimeInterval))
			{
				MSG_G2DB_User_PointControl PointControl;
				memset(&PointControl, 0, sizeof(PointControl));
				PointControl.dwUserID = dwFaUserID;
				PointControl.iDianKongZhi =zs->iCtrlType;
				PointControl.llDianKongFen = ((RAND(zs->iCtlChgPstLowerLimit, zs->iCtlChgPstUpperLimit, m_dwRandIndex++))*TO_DOUBLE*(userBase->second.llWinLoseScore)*(-1));
				m_mapUserPointControl.insert(std::make_pair(dwFaUserID,PointControl));
                if(en_PlayerOutScore == PointControl.iDianKongZhi)
                {
                    PointControl.llDianKongFen *= -1;
                }
                m_pGameDeskPtr->SavePlayerDianKongData(PointControl);
                LOG(INFO)<<"DianKong Join Fa " << userBase->first <<" iDianKongZhi:"<< PointControl.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(PointControl.llDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(m_mUserBaseData[userBase->first].llWinLoseScore*TO_DOUBLE);
				break;
			}
		}
	}
	//判断点控合理性--防止有人乱添加点控
    for (auto it = m_mapUserPointControl.begin(); it != m_mapUserPointControl.end();)
    {
        if((en_PlayerEatScore == it->second.iDianKongZhi) && (m_mUserBaseData[it->first].llWinLoseScore > 100000))
		{
			LOG(ERROR)<<"DianKong Error " << it->first <<" iDianKongZhi:"<< it->second.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(it->second.llDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(m_mUserBaseData[it->first].llWinLoseScore*TO_DOUBLE);
			it->second.iDianKongZhi = en_NoCtrl;
			m_pGameDeskPtr->SavePlayerDianKongData(it->second);
			it = m_mapUserPointControl.erase(it);
		}
		else if((en_PlayerOutScore == it->second.iDianKongZhi) && (m_mUserBaseData[it->first].llWinLoseScore < -300000))
		{
			LOG(ERROR)<<"DianKong Error " << it->first <<" iDianKongZhi:"<< it->second.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(it->second.llDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(m_mUserBaseData[it->first].llWinLoseScore*TO_DOUBLE);
			it->second.iDianKongZhi = en_NoCtrl;
			MSG_G2DB_User_PointControl data;
			memset(&data, 0, sizeof(data));
			data = it->second;
			data.llDianKongFen *= -1;
			m_pGameDeskPtr->SavePlayerDianKongData(data);
			it = m_mapUserPointControl.erase(it);
		}
		else
		{
			//LOG(ERROR)<< it->first <<" iDianKongZhi:"<< it->second.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(it->second.llDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(m_mUserBaseData[it->first].llWinLoseScore*TO_DOUBLE);
			++it;
		}
    }
	return;
}
//删除正在发的红包
CT_VOID CGameProcess::DeleteHBByHBID(CT_DWORD dwHBID)
{
    CMD_S_DeleteDisplayHB DeleteDisplayHB;
    memset(&DeleteDisplayHB, 0, sizeof(DeleteDisplayHB));
	CT_DWORD dwTime = time(NULL);
	if(0 != dwHBID)
	{
        DeleteDisplayHB.dwHBID = dwHBID;
        SendTableData(INVALID_CHAIR, SUB_S_DELETE_DISPLAY_HB, &DeleteDisplayHB, sizeof(DeleteDisplayHB), false);
        //LOG(INFO)<< "DeleteHBByHBID  0 dwHBID " << dwHBID;
		m_mRecordScoreInfo.erase(dwHBID);
		m_mGameEnd.erase(dwHBID);
        m_mOpStartTime.erase(dwHBID);
        m_mOpTotalTime.erase(dwHBID);
        m_mRealPlayerCount.erase(dwHBID);
        m_FaingHB.erase(dwHBID);
	}
	else
	{
		for (auto l = m_FaingHB.begin(); l != m_FaingHB.end();)
		{
			if(m_FaingHB.size() >= MAX_DISPLAY_HB_COUNT && m_ApplyList.size() > 0 && (0 != l->second.dwEndTime))
			{
				DeleteDisplayHB.dwHBID = l->second.stFaHBInfo.dwHBID;
				SendTableData(INVALID_CHAIR, SUB_S_DELETE_DISPLAY_HB, &DeleteDisplayHB, sizeof(DeleteDisplayHB), false);
                //LOG(INFO)<<"DeleteHBByHBID 1 dwHBID:" << l->first;
				m_mRecordScoreInfo.erase(l->first);
				m_mGameEnd.erase(l->first);
				m_mOpStartTime.erase(l->first);
				m_mOpTotalTime.erase(l->first);
                m_mRealPlayerCount.erase(l->first);
				l = m_FaingHB.erase(l);
			}
			else if ((0 != l->second.dwEndTime) && ((dwTime - l->second.dwEndTime) > TIME_HB_DELETE_END_SMAIL))
			{
				DeleteDisplayHB.dwHBID = l->second.stFaHBInfo.dwHBID;
				SendTableData(INVALID_CHAIR, SUB_S_DELETE_DISPLAY_HB, &DeleteDisplayHB, sizeof(DeleteDisplayHB), false);
                //LOG(INFO)<<"DeleteHBByHBID 2 dwHBID:" << l->first;
				m_mRecordScoreInfo.erase(l->first);
				m_mGameEnd.erase(l->first);
                m_mOpStartTime.erase(l->first);
                m_mOpTotalTime.erase(l->first);
                m_mRealPlayerCount.erase(l->first);
				l = m_FaingHB.erase(l);
			}
			else if(0 == l->second.stFaHBInfo.dwUserID || 0 == l->second.stFaHBInfo.dwHBID)
            {
                LOG(ERROR)<<"DeleteHBByHBID 3 dwHBID:" << l->first;
                m_mRecordScoreInfo.erase(l->first);
                m_mGameEnd.erase(l->first);
                m_mOpStartTime.erase(l->first);
                m_mOpTotalTime.erase(l->first);
                m_mRealPlayerCount.erase(l->first);
                l = m_FaingHB.erase(l);
            }
			else
			{
				l++;
			}
		}
	}

    if (m_pGameRoomKindInfo->cbState == SERVER_STOP)
    {
        if(m_FaingHB.size() <= 0 && m_ApplyList.size() <= 0)
        {
            for (int i = 0; i < GAME_PLAYER; ++i)
            {
                if(m_pGameDeskPtr->IsExistUser(i))
                {
                    m_pGameDeskPtr->ClearTableUser(i, true, true);
                }
            }
        }
    }

}
//机器人发红包
CT_VOID CGameProcess::RobotFaHB()
{
    if (m_pGameRoomKindInfo->cbState == SERVER_STOP)
    {
        return;
    }
	//机器人发红包
	if(m_vRobotChairID.size() > 0)
	{
		//int iRand = RAND_NUM(m_dwRandIndex++)%100;
		CT_INT32 iCount = 0;
		CT_WORD wListLenth = 0;
		if(m_pGameRoomKindInfo->wRoomKindID != SENIOR_ROOM)
		{
			wListLenth = m_ApplyList.size();
		}
		else
		{
			for (auto it = m_FaingHB.begin(); it != m_FaingHB.end(); ++it)
			{
				if(it->second.stAssignHBInfo.wRemainHBCount > 0)
				{
					wListLenth++;
				}
			}

		}

		for (CT_DWORD i = 0; i < m_vRobotApplyFaHBCfg.size(); ++i)
		{
			if(wListLenth >= m_vRobotApplyFaHBCfg[i].iApplyListLenLowerLimit && wListLenth < m_vRobotApplyFaHBCfg[i].iApplyListLenUpperLimit)
			{
				if((int)m_iProba(m_gen) < m_vRobotApplyFaHBCfg[i].iProba)
				{
                    iCount = m_vRobotApplyFaHBCfg[i].iFaHBRobotCount;
					break;
				}
			}
		}
		/*if(m_mOpStartTime.size() > 0)
        {
            LOG(INFO) << "RobotFaHB "<< m_mOpStartTime.rbegin()->first << " "<< m_mOpStartTime.rbegin()->second << " " << m_mOpStartTime.begin()->first << " " <<m_mOpStartTime.begin()->second;
            LOG(INFO) << "RobotFaHB "<< (time(NULL) - m_mOpStartTime.rbegin()->second);
        }*/

        if(0 == iCount && m_mOpStartTime.size() > 0 && (time(NULL) - m_mOpStartTime.rbegin()->second) >= 10)
        {
            iCount = 1;
        }
        else if(iCount > 0 && m_mOpStartTime.size() > 0 && (time(NULL) - m_mOpStartTime.rbegin()->second) < 4)
        {
            iCount = 0;
        }

		if(iCount > 0 && m_vRobotChairID.size() > 0)
		{
			CMD_S_NoticeRobotFaHB NoticeRobotFaHB;
            for (int j = 0; j < iCount; ++j)
            {
                int index = 0;
                for (CT_DWORD i = 0; i < m_vRobotChairID.size(); ++i)
                {
                    index = RAND_NUM(m_dwRandIndex++)%m_vRobotChairID.size();
                    if(IsInApplyListByUserID(m_pGameDeskPtr->GetUserID(m_vRobotChairID[index])))
                    {
                        continue;
                    }
                    else
                    {
                        break;
                    }
                }

                NoticeRobotFaHB.wChairID = m_vRobotChairID[index%m_vRobotChairID.size()];
                NoticeRobotFaHB.iHBAllotCount = 10;
                if(m_pGameRoomKindInfo->wRoomKindID != SENIOR_ROOM)
                {
                    NoticeRobotFaHB.iHBAllotCount = m_pHBRoomConfig->iHBAllotCount;
                }
                else
                {
                    int iTemp = 0;
                    //int iProb = RAND_NUM(m_dwRandIndex++)%100;
                    for (CT_DWORD i = 0; i < m_vRobotFaHBAllotCfg.size(); ++i)
                    {
                        iTemp += m_vRobotFaHBAllotCfg[i].iProba;
                        if((int)m_iProba(m_gen) < iTemp)
                        {
                            NoticeRobotFaHB.iHBAllotCount = m_vRobotFaHBAllotCfg[i].iHBAllotCount;
                            break;
                        }
                    }
                }

                SendTableData(NoticeRobotFaHB.wChairID, SUB_S_NOTICE_ROBOT_FA_HB, &NoticeRobotFaHB, sizeof(NoticeRobotFaHB), false);
            }

		}

	}
	else
	{
		//LOG(ERROR) << "RobotFaHB m_vRobotChairID.size()==0";
	}
	return;
}
CT_VOID CGameProcess::StatisticalRobotLocationInfo()
{
	m_vRobotChairID.clear();
	//统计机器人的座位信息及个数
	for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	{
		if(m_pGameDeskPtr->IsExistUser(i) && m_pGameDeskPtr->IsAndroidUser(i))
		{
			m_vRobotChairID.push_back(i);
		}
	}
}
//由于金币变化记录写进去的值最终和加出来的对不上，特用此方法--乱斗场同时多个红包在运行
CT_VOID CGameProcess::ChangeFaSource(CT_DWORD dwUserID, CT_LONGLONG llChangeScore)
{
    if(SENIOR_ROOM != m_pGameRoomKindInfo->wRoomKindID)
    {
        return;
    }
    for (auto i = m_FaingHB.begin(); i != m_FaingHB.end(); ++i)
    {
        if((i->second.stFaHBInfo.dwUserID == dwUserID) && (0 == i->second.dwEndTime))
        {
            i->second.llFaSourceScore += llChangeScore;
        }
    }
}

CT_VOID CGameProcess::FillGrabHBInfo(CT_DWORD dwHBID, CT_DWORD dwUserID, GS_PlayerBaseData stGrabHBPlayer[])
{
    auto it = m_FaingHB.find(dwHBID);
    if(it != m_FaingHB.end())
    {
        int index = 0;
        for (auto iter = it->second.vOpenHBList.begin(); iter != it->second.vOpenHBList.end() && index < MAX_HB_COUNT; ++iter)
        {
            stGrabHBPlayer[index].dwUserID = iter->dwUserID;                            //用户id
            stGrabHBPlayer[index].cbIsThunder = iter->cbIsThunder;
            stGrabHBPlayer[index].cbGender = m_mUserBaseData[iter->dwUserID].cbGender;                            //性别
            stGrabHBPlayer[index].cbVipLevel = m_mUserBaseData[iter->dwUserID].cbVipLevel;                            //VIP等级
            stGrabHBPlayer[index].cbHeadIndex = m_mUserBaseData[iter->dwUserID].cbHeadIndex;                        //头像索引
            stGrabHBPlayer[index].dScore = m_mUserBaseData[iter->dwUserID].llScore * TO_DOUBLE;                                //金
            stGrabHBPlayer[index].dWinningAmount = iter->dWinningAmount;                        //抢的红包金额
            stGrabHBPlayer[index].dWinLoseScore = iter->dWinLoseScore;                    //中雷返现金额
            if ((0 == it->second.dwEndTime) && (dwUserID != iter->dwUserID))
            {
                CT_LONGLONG llAmount = iter->dWinningAmount * TO_SCORE;
                llAmount = (llAmount - (llAmount % 10));
                stGrabHBPlayer[index].cbIsThunder = 0;
                stGrabHBPlayer[index].dWinningAmount = llAmount * TO_DOUBLE;                        //抢的红包金额
                stGrabHBPlayer[index].dWinLoseScore = 0;                    //中雷返现金额
            }
            if (0 == iter->dwUserID)
            {
                LOG(ERROR) << "FillGrabHBInfo dwHBID " << it->second.stFaHBInfo.dwHBID << " dwUserID "<< iter->dwUserID;
            }
            index++;
        }
    }
    else
    {
        LOG(ERROR) << "FillGrabHBInfo error find dwHBID: " << dwHBID;
    }
    return;
}

CT_VOID CGameProcess::RobotGrabHB(CT_DWORD dwHBID)
{
	auto ithb = m_FaingHB.find(dwHBID);
	if(ithb == m_FaingHB.end())
	{
		return;
	}
	int indexs =  INVALID_CHAIR;
	for (size_t i = 0; i < m_vRobotChairID.size() + 5; ++i)
	{
		indexs = RAND_NUM(m_dwRandIndex++)%m_vRobotChairID.size();
		if((!m_pGameDeskPtr->IsAndroidUser(indexs)) ||
		   ((ithb->second.stFaHBInfo.iHBAmount * (ithb->second.stFaHBInfo.dMultiple) * TO_SCORE) > m_pGameDeskPtr->GetUserScore(indexs)))
		{
			indexs = INVALID_CHAIR;
			continue;
		}

		for (size_t j = 0; j < ithb->second.vOpenHBList.size(); ++j)
		{
			if(ithb->second.vOpenHBList[j].wChairID == indexs)
			{
				indexs = INVALID_CHAIR;
				break;
			}
		}
		if(indexs != INVALID_CHAIR)
		{
			break;
		}
	}
	if(indexs != INVALID_CHAIR)
	{
		CMD_C_OpenHB OpenHB2;
		OpenHB2.wChairID = indexs;
		OpenHB2.dwHBID = dwHBID;
		OpenHB2.dwUserID = m_pGameDeskPtr->GetUserID(indexs);
		OpenHB(indexs,&OpenHB2, sizeof(OpenHB2));
	}
}

//设置游戏黑名单
CT_VOID CGameProcess::SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList)
{
    for (auto i = vecBlackList.begin(); i != vecBlackList.end(); ++i)
    {
		auto user = m_mapBlackList.find(i->dwUserID);
		if(user == m_mapBlackList.end())
		{
			tagHBGameBlackList hbGameBlack;
			memset(&hbGameBlack, 0, sizeof(hbGameBlack));
			hbGameBlack.dwUserID = i->dwUserID;
			hbGameBlack.wUserLostRatio = i->wUserLostRatio;
			hbGameBlack.llControlScore = i->dwControlScore;
			if(hbGameBlack.llControlScore > 0)
            {
                hbGameBlack.llControlScore *= -1;
            }
			m_mapBlackList.insert(std::make_pair(i->dwUserID, hbGameBlack));
		}
		else
		{
			user->second.wUserLostRatio = i->wUserLostRatio;
			user->second.llControlScore = i->dwControlScore;
            if(user->second.llControlScore > 0)
            {
                user->second.llControlScore *= -1;
            }
		}
    }
}

CT_BOOL CGameProcess::IsBlackListUserOnline()
{
	for (auto i = m_mapBlackList.begin(); i != m_mapBlackList.end(); ++i)
	{
		if(INVALID_CHAIR != m_pGameDeskPtr->GetUserChairID(i->first))
		{
			return true;
		}
	}
	return false;
}
//将机器人的与黑名单用户信息互换
CT_VOID CGameProcess::GrabLastOneHBBlackListUserDear(tagHBInfo & stHBInfo, int index, int iRobotIndex, CT_BOOL bIsSelfFaSelfGrab)
{
	if(INVALID_CHAIR == index || INVALID_CHAIR == iRobotIndex)
	{
		return;
	}
	//记录机器人原值，减扣库存
	CT_BOOL bRobotSourceIsThunder = stHBInfo.stAssignHBInfo.bIsThunder[iRobotIndex];
	CT_LONGLONG llSourceScore = (stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex]-stHBInfo.stAssignHBInfo.llThunderBackAmount[iRobotIndex] - stHBInfo.stAssignHBInfo.iRevenue[iRobotIndex]+ stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[iRobotIndex]);
	m_sllStockScore -= (stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex] - stHBInfo.stAssignHBInfo.llThunderBackAmount[iRobotIndex]);


	int iTempThunder = stHBInfo.stAssignHBInfo.llOutHB[index] % 10;
	CT_LONGLONG llScore = (stHBInfo.stFaHBInfo.cbThunderNO-iTempThunder);

	if(((stHBInfo.stAssignHBInfo.llOutHB[index] + llScore) <= 0) || ((stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex] - llScore) <= 0))
	{
		return;
	}

	//计算最后一个红包的信息
	stHBInfo.stAssignHBInfo.llOutHB[index] += llScore;
	if(stHBInfo.stAssignHBInfo.llOutHB[index] && (stHBInfo.stAssignHBInfo.llOutHB[index]%10 == stHBInfo.stFaHBInfo.cbThunderNO))
	{
		if(!bIsSelfFaSelfGrab)
		{
			stHBInfo.stAssignHBInfo.llThunderBackAmount[index] = stHBInfo.stFaHBInfo.dMultiple*stHBInfo.stAssignHBInfo.llTotalAmount;
		}
		stHBInfo.stAssignHBInfo.bIsThunder[index] = true;
		stHBInfo.stAssignHBInfo.iThunderCount++;
	}
	else
    {
        stHBInfo.stAssignHBInfo.llThunderBackAmount[index] = 0;
    }
    int iWinPrizeIndex = m_GameLogic.JudgeWinPrize(stHBInfo, index);
    m_GameLogic.CalculateColorPrize(stHBInfo, index, iWinPrizeIndex, false, m_mUserBaseData[stHBInfo.stFaHBInfo.dwUserID].bRobot);


    //重新计算机器人的红包信息
	stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex] -= llScore;
    if(stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex] && (stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex]%10 == stHBInfo.stFaHBInfo.cbThunderNO))
    {
        if(stHBInfo.stAssignHBInfo.dwUserID[iRobotIndex] != stHBInfo.stFaHBInfo.dwUserID)
        {
            stHBInfo.stAssignHBInfo.llThunderBackAmount[iRobotIndex] = stHBInfo.stFaHBInfo.dMultiple*stHBInfo.stAssignHBInfo.llTotalAmount;
        }
        stHBInfo.stAssignHBInfo.bIsThunder[iRobotIndex] = true;
        if(!bRobotSourceIsThunder)
        {
            stHBInfo.stAssignHBInfo.iThunderCount++;
            stHBInfo.stAssignHBInfo.iRobotThunderCount++;
        }
    }
    else if(bRobotSourceIsThunder)
	{
		stHBInfo.stAssignHBInfo.iThunderCount--;
		stHBInfo.stAssignHBInfo.iRobotThunderCount--;
		stHBInfo.stAssignHBInfo.llThunderBackAmount[iRobotIndex] = 0;
        stHBInfo.stAssignHBInfo.bIsThunder[iRobotIndex] = false;
	}
    stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[iRobotIndex] = 0;
    iWinPrizeIndex = m_GameLogic.JudgeWinPrize(stHBInfo, iRobotIndex);
    m_GameLogic.CalculateColorPrize(stHBInfo, iRobotIndex, iWinPrizeIndex, false, m_mUserBaseData[stHBInfo.stFaHBInfo.dwUserID].bRobot);
    CT_LONGLONG llTotalWin = stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex] - stHBInfo.stAssignHBInfo.llThunderBackAmount[iRobotIndex] + stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[iRobotIndex];
    if (llTotalWin > 0 && (stHBInfo.stAssignHBInfo.dwUserID[iRobotIndex] != stHBInfo.stFaHBInfo.dwUserID))
    {
        stHBInfo.stAssignHBInfo.iRevenue[iRobotIndex] = (CT_INT32) m_pGameDeskPtr->CalculateRevenue(stHBInfo.stAssignHBInfo.wChairID[iRobotIndex], llTotalWin);
    }
    else if(stHBInfo.stAssignHBInfo.dwUserID[iRobotIndex] == stHBInfo.stFaHBInfo.dwUserID &&  stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[iRobotIndex] > 0)
    {
        stHBInfo.stAssignHBInfo.iRevenue[iRobotIndex] = (CT_INT32) m_pGameDeskPtr->CalculateRevenue(stHBInfo.stAssignHBInfo.wChairID[iRobotIndex], stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[iRobotIndex]);
    }
    else
    {
        stHBInfo.stAssignHBInfo.iRevenue[iRobotIndex] = 0;
    }
    CT_LONGLONG llCurrScore = (stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex]-stHBInfo.stAssignHBInfo.llThunderBackAmount[iRobotIndex] - stHBInfo.stAssignHBInfo.iRevenue[iRobotIndex]+ stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[iRobotIndex]);
    //    //修改机器人输赢分数
    m_pGameDeskPtr->AddUserScore(stHBInfo.stAssignHBInfo.wChairID[iRobotIndex],llCurrScore - llSourceScore ,true);
	m_sllGrabWinScore += (llCurrScore - llSourceScore);
	//重新计算机器人库存
	m_sllStockScore += (stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex] - stHBInfo.stAssignHBInfo.llThunderBackAmount[iRobotIndex]);
	for (auto i = stHBInfo.vOpenHBList.begin(); i != stHBInfo.vOpenHBList.end(); ++i)
	{
		if(i->wChairID == stHBInfo.stAssignHBInfo.wChairID[iRobotIndex] && i->dwUserID == stHBInfo.stAssignHBInfo.dwUserID[iRobotIndex])
		{
			i->dWinningAmount = stHBInfo.stAssignHBInfo.llOutHB[iRobotIndex] * TO_DOUBLE;
			i->cbIsThunder = stHBInfo.stAssignHBInfo.bIsThunder[iRobotIndex];
			i->dWinLoseScore = llCurrScore*TO_DOUBLE;
			i->dWinColorPotAmount = stHBInfo.stAssignHBInfo.llGrabWinColorPoolAmount[iRobotIndex]*TO_DOUBLE;
			break;
		}
	}

	RecordScoreInfo recordScoreInfo;
	memset(&recordScoreInfo, 0, sizeof(recordScoreInfo));
	recordScoreInfo.dwUserID = m_pGameDeskPtr->GetUserID(stHBInfo.stAssignHBInfo.wChairID[iRobotIndex]);
	recordScoreInfo.llSourceScore = stHBInfo.stAssignHBInfo.llSourceScore[iRobotIndex];
	recordScoreInfo.cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(stHBInfo.stAssignHBInfo.wChairID[iRobotIndex]);
	recordScoreInfo.cbStatus = 1;
	recordScoreInfo.iScore = llTotalWin;
	recordScoreInfo.dwRevenue = stHBInfo.stAssignHBInfo.iRevenue[iRobotIndex];
	auto pRecord = m_mRecordScoreInfo.find(stHBInfo.stFaHBInfo.dwHBID);
	if(pRecord != m_mRecordScoreInfo.end())
	{
		for (size_t i = 0; i < pRecord->second.size(); ++i)
		{
			if(pRecord->second[i].dwUserID == recordScoreInfo.dwUserID)
			{
				pRecord->second[i].llSourceScore = recordScoreInfo.llSourceScore;
				pRecord->second[i].cbIsAndroid = recordScoreInfo.cbIsAndroid;
				pRecord->second[i].iScore = recordScoreInfo.iScore;
				pRecord->second[i].dwRevenue = recordScoreInfo.dwRevenue;
				break;
			}
		}
	}
	else
	{
		LOG(ERROR) << "m_mRecordScoreInfo error" << stHBInfo.stFaHBInfo.dwHBID;
	}
}