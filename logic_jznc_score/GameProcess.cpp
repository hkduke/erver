
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
#define		PRINT_LOG_INFO						0
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//#define IDI_CHECK_TABLE			100					//检查桌子
//#define IDI_CHECK_TRUSTEE			101					//断线托管

//#define IDI_AUTO_READY				200					//自动准备
#define IDI_CHECK_BET				100					//检测下注
#define IDI_THROW_DICE				101					//检测投掷骰子
#define IDI_CHECK_END				102					//检测游戏结束
#define IDI_DELETE_USER			    104					//删除用户信息
#define IDI_TICK_USER				105					//剔除用户
#define ID_TIME_READY				106				    //准备定时器
#define IDI_SEND_END				107				    //延时发送结算

#define TIME_CHECK_BET				5					//检测下注
#define TIME_THROW_DICE				5					//检测下注
#define TIME_CHECK_END				6					//检测桌子结束
#define TIME_DELETE_USER		    (60*60)			    //检查(20分钟删除一次在线超过3*24小时的玩家信息)
#define TIME_READY					(1)				    //准备
#define TIME_SEND_END				(1)				//延时结算
//网络补偿时间
#define TIME_NETWORK_COMPENSATE     1					//网络补偿
#define COMPARE_POINTS              0                   //点数房
#define COMPARE_SIZE                1                   //大小房

////////////////////////////////////////////////////////////////////////
CT_DOUBLE CGameProcess::m_sdStockScore = 0.0;              //库存
std::map<CT_DWORD, GS_SimpleUserInfo>	CGameProcess::m_mapUserBaseInfo;
std::map<CT_DWORD, MSG_G2DB_User_PointControl> CGameProcess::m_mapUserPointControl;
std::map<CT_DWORD, tagJZNCGameBlackList> CGameProcess::m_mapBlackList;
CT_LONGLONG CGameProcess::m_sllStatisticsPointCount[6] = {0};
CT_LONGLONG CGameProcess::m_sllRealWinTotalScore = 0;
CT_LONGLONG CGameProcess::m_sllRealLoseTotalScore = 0;
CT_LONGLONG CGameProcess::m_sllRealTotalRevenue = 0;
CT_LONGLONG CGameProcess::m_sllRealWinTotalCount = 0;
CT_LONGLONG CGameProcess::m_sllRealLoseTotalCount = 0;
CT_LONGLONG CGameProcess::m_sllRealPlayTotalCount = 0;
///////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////
CGameProcess::CGameProcess(void) 
	:m_dwlCellScore(1), m_pGameDeskPtr(NULL), m_pGameRoomKindInfo(NULL), m_gen(m_rd()), m_iProba(0,99)
{
	//清理游戏数据
	ClearGameData();
	//初始化数据
	InitGameData();
    m_dwRandIndex = 0;
    m_bIsAllowSame = 0;
    m_dwRollDice = TIME_THROW_DICE;
    m_dwBetCoin = TIME_CHECK_BET;
    m_dwReadyMatch = TIME_READY;
    m_dwOldTime = 0;
}

CGameProcess::~CGameProcess(void)
{

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
	CT_BYTE cbSumeCount = 0;
	CT_WORD wCount = 0;
    do
    {
        cbSumeCount = 0;
        m_wPlayerCount = 0;
        int iTempDice = 0;
        for (size_t i = 0; i < GAME_PLAYER; ++i)
        {
            if(!m_pGameDeskPtr->IsExistUser(i))
            {
                continue;
            }
            if(m_pGameDeskPtr->GetUserScore(i) < m_dwlCellScore)
            {
                m_pGameDeskPtr->ClearTableUser(i, true, true);
                continue;
            }
            else
            {
                m_cbThrowDice[i] = m_GameLogic.RandDice(GetCtrlEffectKey(i));
                m_RecordScoreInfo[i].llSourceScore = m_pGameDeskPtr->GetUserScore(i);
            }
            iTempDice=m_cbThrowDice[i];
            m_wPlayerCount++;
        }

        for (int j = 0; j < GAME_PLAYER; ++j)
        {
            if(!m_pGameDeskPtr->IsExistUser(j))
            {
                continue;
            }

            if(COMPARE_POINTS == m_dwGameType && iTempDice == m_cbThrowDice[j])
            {
                cbSumeCount++;
            }
            else if(COMPARE_SIZE == m_dwGameType && m_cbThrowDice[j] > 3)
            {
                cbSumeCount++;
            }
        }
        wCount++;
        if(wCount > 3)
        {
            break;
        }
    }while ((!m_bIsAllowSame) && ((cbSumeCount == m_wPlayerCount) || (0 == cbSumeCount && COMPARE_SIZE == m_dwGameType)));
    //LOG(ERROR) << "ThrowDice:" << (int)m_cbThrowDice[0] << (int)m_cbThrowDice[1] << (int)m_cbThrowDice[2] << (int)m_cbThrowDice[3] << (int)m_cbThrowDice[4] << (int)m_cbThrowDice[5] << (int)m_cbThrowDice[6]<< (int)m_cbThrowDice[7];
    //LOG(ERROR) << "m_bIsAllowSame:" << m_bIsAllowSame << " cbSumeCount:" << (int)cbSumeCount << " m_wPlayerCount:" << m_wPlayerCount;
	m_cbGameStatus = GAME_SCENE_BET;
	CMD_S_StartGame startGame;
	memset(&startGame, 0, sizeof(startGame));
	startGame.wTimeLeft = m_dwBetCoin;
	m_dwSysTime = time(NULL);
	SendTableData(INVALID_CHAIR, SUB_S_START_GAME, &startGame, sizeof(startGame),false);
	m_pGameDeskPtr->SetGameTimer(IDI_CHECK_BET,(m_dwBetCoin+TIME_NETWORK_COMPENSATE+TIME_ANIMATION)*1000);
	return;
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//清除所有定时器
	//ClearAllTimer();
    m_cbGameStatus = GAME_SCENE_END;
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
    //m_cbGameStatus = GAME_SCENE_FREE;
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
			//发送数据
			SendTableData(dwChairID, SC_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree),false);
			break;
		}
		case GAME_SCENE_BET:	//游戏状态
		{
            CMD_S_StatusBet StatusBet;
            memset(&StatusBet, 0, sizeof(StatusBet));
            StatusBet.dCellScore = m_dwlCellScore*TO_DOUBLE;
            StatusBet.wTimeLeft = TIME_CHECK_BET+TIME_ANIMATION - (time(NULL) - m_dwSysTime);
            memcpy(StatusBet.cbBet, m_PlayerBet.cbIsBet, sizeof(StatusBet.cbBet));
            //发送场景
            SendTableData(dwChairID, SC_GAMESCENE_BET, &StatusBet, sizeof(StatusBet),false);
            break;
		}
        case GAME_SCENE_THROWDICE:	//游戏状态
        {
            CMD_S_StatusThrowDice StatusThrowDice;
            memset(&StatusThrowDice, 0, sizeof(StatusThrowDice));
			StatusThrowDice.dCellScore = m_dwlCellScore*TO_DOUBLE;
			StatusThrowDice.wTimeLeft = TIME_THROW_DICE - (time(NULL) - m_dwSysTime);
            for (size_t i = 0; i < GAME_PLAYER; ++i)
            {
                if(m_PlayerDice.cbIsDice[i])
                {
                    StatusThrowDice.cbThrowDice[i] = m_cbThrowDice[i];
                }
            }
            //发送场景
            SendTableData(dwChairID, SC_GAMESCENE_THROWDICE, &StatusThrowDice, sizeof(StatusThrowDice),false);
            break;
        }
		case GAME_SCENE_END://这个场景只有几秒时间
		{
			//场景数据
			CMD_S_StatusEND StatusEnd;
			memset(&StatusEnd, 0, sizeof(CMD_S_StatusEND));

			//发送结算状态数据
			SendTableData(dwChairID, SC_GAMESCENE_END, &StatusEnd, sizeof(CMD_S_StatusEND), false);
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
	//LOG(WARNING) << "run timer, timer id: " << dwTimerID;

	m_pGameDeskPtr->KillGameTimer(dwTimerID);

	switch (dwTimerID)
	{
    case IDI_CHECK_BET:
    {
        if (GAME_SCENE_BET != m_cbGameStatus)
        {
            return;
        }
        for (int i = 0; i < GAME_PLAYER; ++i)
        {
            if(!m_pGameDeskPtr->IsExistUser(i))
            {
                continue;
            }
            if(!m_PlayerBet.cbIsBet[i])
            {
                CMD_C_UserBet UserBet;
                OnUserBet(i, &UserBet, sizeof(UserBet));
            }
        }
    }
    break;
    case IDI_THROW_DICE://此定时器全程运行，不Kill
	{
        if (GAME_SCENE_THROWDICE != m_cbGameStatus)
        {
            return;
        }
        for (int i = 0; i < GAME_PLAYER; ++i)
        {
            if(!m_pGameDeskPtr->IsExistUser(i))
            {
                continue;
            }
            if(!m_PlayerDice.cbIsDice[i])
            {
                CMD_C_ThrowDice ThrowDice;
                OnUserThrowDice(i, &ThrowDice, sizeof(ThrowDice));
            }
        }
	}
	break;
    case IDI_DELETE_USER://此定时器全程运行，不Kill
    {
        //LOG(INFO)<< "OnTimer IDI_DELETE_USER: time";
        //删除真人信息
        CT_DWORD dwTime = time(NULL);
        for (auto it3 = m_mapUserBaseInfo.begin(); it3 != m_mapUserBaseInfo.end();)
        {
            if((!it3->second.bRobot) && ((dwTime - it3->second.dwFirstTime) > (60*60*24*4)) && (INVALID_CHAIR == m_pGameDeskPtr->GetUserChairID(it3->second.dwUserID)))
            {
                it3 = m_mapUserBaseInfo.erase(it3);
            }
            else if(0 == it3->first)
            {
                LOG(ERROR)<<"IDI_DELETE_USER uid: " << it3->first;
                it3 = m_mapUserBaseInfo.erase(it3);
            }
            else
            {
                it3++;
            }
        }
        m_pGameDeskPtr->SetGameTimer(IDI_DELETE_USER, TIME_DELETE_USER*1000);
    }
    break;
    case IDI_TICK_USER://此定时器全程运行，不Kill
    {
        //LOG(INFO)<< "OnTimer IDI_DELETE_USER: time";
        for (CT_DWORD i= 0; i < GAME_PLAYER; ++i)
        {
            if (!m_pGameDeskPtr->IsExistUser(i))
            {
                continue;
            }

            //斗地主游戏剔除用户时不需要广播状态给客户端
            m_pGameDeskPtr->ClearTableUser(i, false);
        }

        std::string strTime = Utility::GetTimeNowString();
        m_cbGameStatus = GAME_SCENE_FREE;
        m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());

    }
    break;
    case ID_TIME_READY://此定时器全程运行，不Kill
    {
        if (GAME_SCENE_FREE != m_cbGameStatus)
        {
            return;
        }
        //LOG(INFO)<< "OnTimer ID_TIME_READY: time";
        for (CT_DWORD i= 0; i < GAME_PLAYER; ++i)
        {
            if (!m_pGameDeskPtr->IsExistUser(i))
            {
                continue;
            }
            //斗地主游戏剔除用户时不需要广播状态给客户端
            m_pGameDeskPtr->SetUserReady(i);
        }
        if(GAME_SCENE_FREE == m_cbGameStatus)
        {
            m_pGameDeskPtr->SetGameTimer(ID_TIME_READY, 1000);
        }
    }
    break;
    case IDI_SEND_END://此定时器全程运行，不Kill
    {
        DelayWriteScore();
        m_pGameDeskPtr->SetGameTimer(IDI_TICK_USER, 10);
    }
    break;
	default:
		break;
	}

}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
    bool bRet = true;
	switch (dwSubCmdID)
	{
		case SUB_C_BET:	//用户下注
		{
            bRet = OnUserBet(wChairID, pDataBuffer, dwDataSize);
            break;
		}
		case SUB_C_THROW_DICE: //投掷骰子
		{
            bRet = OnUserThrowDice(wChairID, pDataBuffer, dwDataSize);
			break;
		}
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
    CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
    int iVipLevel2 = 0;
    if(m_pGameDeskPtr->GetUserBaseData(dwChairID))
    {
        iVipLevel2 = m_pGameDeskPtr->GetUserBaseData(dwChairID)->cbVipLevel2;
    }
    auto pUser = m_mapUserBaseInfo.find(dwUserID);
    if(pUser == m_mapUserBaseInfo.end())
    {
        GS_SimpleUserInfo simpleUserInfo;
        simpleUserInfo.dwUserID = dwUserID;
        simpleUserInfo.llWinLoseScore = 0;
        simpleUserInfo.bRobot = m_pGameDeskPtr->IsAndroidUser(dwChairID);
        simpleUserInfo.cbGender = m_pGameDeskPtr->GetUserSex(dwChairID);						//性别
        simpleUserInfo.cbVipLevel = iVipLevel2;							//VIP等级
        simpleUserInfo.cbHeadIndex = m_pGameDeskPtr->GetHeadIndex(dwChairID);						//头像索引
        simpleUserInfo.dwFirstTime = time(NULL);
        m_mapUserBaseInfo.insert(std::make_pair(dwUserID, simpleUserInfo));
    }
    else
    {
        pUser->second.cbVipLevel = iVipLevel2;
        pUser->second.bRobot = m_pGameDeskPtr->IsAndroidUser(dwChairID);
    }
    int iCount = 0;
    for (int i = 0; i < GAME_PLAYER; ++i)
    {
        if(m_pGameDeskPtr->IsExistUser(i))
        {
            iCount++;
        }
    }
    //LOG(INFO) << "OnUserEnter iCount:" << iCount;
    if(1 == iCount)
    {
        std::uniform_int_distribution<unsigned int> disTime(2, m_dwReadyMatch);
        m_pGameDeskPtr->SetGameTimer(ID_TIME_READY, TIME_READY*1000);
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
    DeleteUserByChairID(dwChairID);
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
    m_GameLogic.SetGameRoomKindInfo(m_pGameRoomKindInfo);
    m_pGameDeskPtr->SetGameTimer(IDI_DELETE_USER, TIME_DELETE_USER*1000);
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

	memset(&m_PlayerBet, 0, sizeof(m_PlayerBet));
    memset(&m_PlayerDice, 0, sizeof(m_PlayerDice));
	memset(m_cbThrowDice, 0, sizeof(m_cbThrowDice));
    memset(m_RecordScoreInfo, 0, sizeof(m_RecordScoreInfo));
    m_wPlayerCount = 0;
    m_wEffectRobotCount = 0;
    m_vEffectRobotChairID.clear();
	if (NULL != m_pGameDeskPtr)
	{
	    CT_DWORD dwNewTime = time(NULL);
        if((dwNewTime-m_dwOldTime) > 5*60)
        {
            ReadJsonCfg();
            m_dwOldTime = dwNewTime;
        }

        //获取私人房信息
		//m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
		m_dwlCellScore = m_pGameDeskPtr->GetGameCellScore();
		m_pGameRoomKindInfo = m_pGameDeskPtr->GetGameKindInfo();
		m_vRobotChairID.clear();

        for (int i = 0; i < GAME_PLAYER; ++i)
        {
            if(!m_pGameDeskPtr->IsExistUser(i))
            {
                continue;
            }
            if(m_pGameDeskPtr->IsAndroidUser(i))
            {
                m_vRobotChairID.push_back(i);
            }
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
    std::string file("./script/gameserver_jznc.json");

	std::string data;
	ReadFile(file,data);
	acl::json json;
	json.update(data.c_str());
    if(!json.getElementsByTagName("IsAllowSame").empty())
    {
        m_bIsAllowSame = (CT_BOOL)(*(json.getElementsByTagName("IsAllowSame")[0])->get_int64());
    }
    if(!json.getElementsByTagName("ReadyMatch").empty())
    {
        m_dwReadyMatch = (CT_DWORD)(*(json.getElementsByTagName("ReadyMatch")[0])->get_int64());
    }
    if(!json.getElementsByTagName("RollDice").empty())
    {
        m_dwRollDice = (CT_DWORD)(*(json.getElementsByTagName("RollDice")[0])->get_int64());
    }
    if(!json.getElementsByTagName("BetCoin").empty())
    {
        m_dwBetCoin = (CT_DWORD)(*(json.getElementsByTagName("BetCoin")[0])->get_int64());
    }
    if(!json.getElementsByTagName("CtrlEffect").empty())
    {
        std::map<int , std::vector<CT_WORD >>  ctrlEffect;
        acl::json jsonTmp(*(json.getElementsByTagName("CtrlEffect")[0]));
        acl::json_node *node3 = jsonTmp.getElementsByTagName("-2")[0]->first_child();
        acl::json_node *node4 = node3->first_child();
        std::vector<CT_WORD > vCount;
        while(node3 != NULL && node4 != NULL)
        {
            vCount.push_back((CT_WORD)(*(node4->get_int64())));
            node4 = node3->next_child();
        }
        ctrlEffect.insert(std::make_pair(-2,vCount));
        vCount.clear();
        node3 = jsonTmp.getElementsByTagName("-1")[0]->first_child();
        node4 = node3->first_child();
        while(node3 != NULL && node4 != NULL)
        {
            vCount.push_back((CT_WORD)(*(node4->get_int64())));
            node4 = node3->next_child();
        }
        ctrlEffect.insert(std::make_pair(-1,vCount));
        vCount.clear();
        node3 = jsonTmp.getElementsByTagName("0")[0]->first_child();
        node4 = node3->first_child();
        while(node3 != NULL && node4 != NULL)
        {
            vCount.push_back((CT_WORD)(*(node4->get_int64())));
            node4 = node3->next_child();
        }
        ctrlEffect.insert(std::make_pair(0,vCount));
        vCount.clear();
        node3 = jsonTmp.getElementsByTagName("1")[0]->first_child();
        node4 = node3->first_child();
        while(node3 != NULL && node4 != NULL)
        {
            vCount.push_back((CT_WORD)(*(node4->get_int64())));
            node4 = node3->next_child();
        }
        ctrlEffect.insert(std::make_pair(1,vCount));
        vCount.clear();
        node3 = jsonTmp.getElementsByTagName("2")[0]->first_child();
        node4 = node3->first_child();
        while(node3 != NULL && node4 != NULL)
        {
            vCount.push_back((CT_WORD)(*(node4->get_int64())));
            node4 = node3->next_child();
        }
        ctrlEffect.insert(std::make_pair(2,vCount));
        vCount.clear();
        m_GameLogic.SetCtrlEffect(ctrlEffect);
    }

    //库存配置
    if(!json.getElementsByTagName("AIStockCtrl").empty())
    {
        m_mAIStockCtrlCfg.clear();
        acl::json_node *node1 = json.getElementsByTagName("AIStockCtrl")[0]->first_child();
        acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
        {
            acl::json jsonTmp(*node2);
            CT_DWORD dwRoomKind = 0;
            if(jsonTmp.getElementsByTagName("RoomKind").size() > 0)
            {
                dwRoomKind = (CT_DWORD)(*(jsonTmp.getElementsByTagName("RoomKind")[0])->get_int64());
                if(dwRoomKind <= PRIVATE_ROOM || dwRoomKind > MATCH_ROOM)
                {
                    LOG(ERROR) << "AIStockCtrl error RoomKind:" << dwRoomKind;
                    break;
                }
            }
            std::vector<tagAIStockCtrlCfg> & vecStockCtrlCfg = m_mAIStockCtrlCfg[dwRoomKind];
            if(!jsonTmp.getElementsByTagName("iStockCtrlTable").empty())
            {
                acl::json_node *node3 = jsonTmp.getElementsByTagName("iStockCtrlTable")[0]->first_child();
                acl::json_node *node4 = node3->first_child();
                while(node3 != NULL && node4 != NULL)
                {
                    acl::json jsonTmp2(*node4);
                    tagAIStockCtrlCfg aiStockCtrl;
                    memset(&aiStockCtrl, 0, sizeof(aiStockCtrl));

                    if(jsonTmp2.getElementsByTagName("llStockLowerScore").size() > 0)
                    {
                        aiStockCtrl.llStockLowerScore = (*(jsonTmp2.getElementsByTagName("llStockLowerScore")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("llStockUpperScore").size() > 0)
                    {
                        aiStockCtrl.llStockUpperScore = (*(jsonTmp2.getElementsByTagName("llStockUpperScore")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iActiveProb").size() > 0)
                    {
                        aiStockCtrl.dwActiveProb = (CT_DWORD)(*(jsonTmp2.getElementsByTagName("iActiveProb")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iUnitCtrlStep").size() > 0)
                    {
                        aiStockCtrl.iUnitCtrlStep = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iUnitCtrlStep")[0])->get_int64());
                        if(aiStockCtrl.iUnitCtrlStep <= 0)
                        {
                            aiStockCtrl.iUnitCtrlStep = 2;
                        }
                    }
                    if(jsonTmp2.getElementsByTagName("iCtrlMinCnt").size() > 0)
                    {
                        aiStockCtrl.iCtrlMinCnt = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iCtrlMinCnt")[0])->get_int64());
                    }
                    if(jsonTmp2.getElementsByTagName("iCtrlEffect").size() > 0)
                    {
                        aiStockCtrl.iCtrlEffect = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iCtrlEffect")[0])->get_int64());
                    }
                    vecStockCtrlCfg.push_back(aiStockCtrl);
                    node4 = node3->next_child();
                }
            }
            node2 = node1->next_child();
        }
    }

    if(m_pGameDeskPtr->GetGameKindInfo()->wRoomKindID % 2 == 0)
    {
        m_dwGameType = COMPARE_POINTS;
    }
    else
    {
        m_dwGameType = COMPARE_SIZE;
    }

    if(COMPARE_POINTS == m_dwGameType)
    {
        //自动控制配置
        if(!json.getElementsByTagName("PointsAutoCtrl").empty())
        {
            m_vWinCtrlCfg.clear();
            m_vLoseCtrlCfg.clear();
            acl::json jsonTmp(*(json.getElementsByTagName("PointsAutoCtrl")[0]));
            AnalysisJsonAutoCtrl(jsonTmp, "pAutoWinCtrlTable", m_vWinCtrlCfg);
            AnalysisJsonAutoCtrl(jsonTmp, "pAutoLossCtrlTable", m_vLoseCtrlCfg);
        }
    }
    else
    {
        //自动控制配置
        if(!json.getElementsByTagName("SizeAutoCtrl").empty())
        {
            m_vWinCtrlCfg.clear();
            m_vLoseCtrlCfg.clear();
            acl::json jsonTmp(*(json.getElementsByTagName("SizeAutoCtrl")[0]));
            AnalysisJsonAutoCtrl(jsonTmp, "pAutoWinCtrlTable", m_vWinCtrlCfg);
            AnalysisJsonAutoCtrl(jsonTmp, "pAutoLossCtrlTable", m_vLoseCtrlCfg);
        }
    }
    //点控配置
    if(!json.getElementsByTagName("PointCtrl").empty())
    {
        m_vPointtrlCfg.clear();
        acl::json_node *node1 = json.getElementsByTagName("PointCtrl")[0]->first_child();
        acl::json_node *node2 = node1->first_child();
        while(node1 != NULL && node2 != NULL)
        {
            acl::json jsonTmp(*node2);
            tagPointCtrlCfg pointCtrlCfg;
            memset(&pointCtrlCfg, 0, sizeof(pointCtrlCfg));
            if(jsonTmp.getElementsByTagName("iCheatRatioLower").size() > 0)
            {
                pointCtrlCfg.iCheatRatioLower = (CT_INT32)(*(jsonTmp.getElementsByTagName("iCheatRatioLower")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("iCheatRatioUpper").size() > 0)
            {
                pointCtrlCfg.iCheatRatioUpper = (CT_INT32)(*(jsonTmp.getElementsByTagName("iCheatRatioUpper")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("iCtrlProb").size() > 0)
            {
                pointCtrlCfg.iCtrlProb = (CT_DWORD)(*(jsonTmp.getElementsByTagName("iCtrlProb")[0])->get_int64());
            }
            if(jsonTmp.getElementsByTagName("iExeEffect").size() > 0)
            {
                pointCtrlCfg.iExeEffect = (CT_INT32)(*(jsonTmp.getElementsByTagName("iExeEffect")[0])->get_int64());
            }
            m_vPointtrlCfg.push_back(pointCtrlCfg);
            node2= node1->next_child();
        }
    }

	return;
}
void CGameProcess::AnalysisJsonAutoCtrl(acl::json & jsonTmp, const char * pValue, std::vector<tagPlayerAutoCtrlCfg> & WinLossCtrl)
{
    acl::json_node *node1 = jsonTmp.getElementsByTagName(pValue)[0]->first_child();
    acl::json_node *node2 = node1->first_child();
    while(node1 != NULL && node2 != NULL)
    {
        acl::json jsonTmp2(*node2);
        tagPlayerAutoCtrlCfg AutoCtrlCfg;
        memset(&AutoCtrlCfg, 0, sizeof(AutoCtrlCfg));
        if(jsonTmp2.getElementsByTagName("iContWinLossLowerCnt").size() > 0)
        {
            AutoCtrlCfg.iContWinLossLowerCnt = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iContWinLossLowerCnt")[0])->get_int64());
        }
        if(jsonTmp2.getElementsByTagName("iContWinLossUpperCnt").size() > 0)
        {
            AutoCtrlCfg.iContWinLossUpperCnt = (CT_INT32)(*(jsonTmp2.getElementsByTagName("iContWinLossUpperCnt")[0])->get_int64());
        }
        if(jsonTmp2.getElementsByTagName("iCtrlProb").size() > 0)
        {
            AutoCtrlCfg.iCtrlProb = (*(jsonTmp2.getElementsByTagName("iCtrlProb")[0])->get_int64());
        }
        if(jsonTmp2.getElementsByTagName("iCtrlEffect").size() > 0)
        {
            AutoCtrlCfg.iCtrlEffect = (*(jsonTmp2.getElementsByTagName("iCtrlEffect")[0])->get_int64());
        }
        WinLossCtrl.push_back(AutoCtrlCfg);
        node2 = node1->next_child();
    }
    return;
}
//清除所有定时器
void CGameProcess::ClearAllTimer()
{
	//开始删除所有定时器
	m_pGameDeskPtr->KillGameTimer(IDI_CHECK_END);
}

//普通场游戏结算
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwHBID)
{
    if(m_cbGameStatus != GAME_SCENE_END)
    {
        LOG(ERROR) << "NormalGameEnd status:" << m_cbGameStatus;
    }
    std::vector<CT_WORD> vecMaxDiceChairID;
    CT_BYTE cbMaxDice = 0;
    CT_LONGLONG llTotalScore = 0;
    for (size_t i = 0; i < GAME_PLAYER; ++i)
    {
        if(!m_PlayerBet.cbIsBet[i])
        {
            continue;
        }

        if(!m_pGameDeskPtr->IsAndroidUser(i))
        {
            m_sllStatisticsPointCount[m_cbThrowDice[i]-1]++;
        }

        llTotalScore += m_dwlCellScore;
        if(COMPARE_POINTS == m_dwGameType)
        {
            if(cbMaxDice == m_cbThrowDice[i])
            {
                vecMaxDiceChairID.push_back(i);
            }
            else if(cbMaxDice < m_cbThrowDice[i])
            {
                cbMaxDice = m_cbThrowDice[i];
                vecMaxDiceChairID.clear();
                vecMaxDiceChairID.push_back(i);
            }
        }
        else
        {
            if(m_cbThrowDice[i] > 3)
            {
                vecMaxDiceChairID.push_back(i);
            }
        }


    }

    CMD_S_GameEnd gameEnd;
    memset(&gameEnd, 0, sizeof(gameEnd));
    memset(m_llScore, 0, sizeof(m_llScore));
    memset(m_llWinLoseScore, 0, sizeof(m_llWinLoseScore));
    memset(m_dwRevenue, 0, sizeof(m_dwRevenue));
    for (size_t j = 0; (j < GAME_PLAYER) && (COMPARE_POINTS == m_dwGameType); ++j)
    {
        if(!m_PlayerBet.cbIsBet[j])
        {
            continue;
        }
        for (size_t i = 0; i < vecMaxDiceChairID.size(); ++i)
        {
            if(j == vecMaxDiceChairID[i])
            {
                m_llWinLoseScore[j] =  ((CT_DOUBLE)(llTotalScore - (vecMaxDiceChairID.size() * m_dwlCellScore))/vecMaxDiceChairID.size());
                m_dwRevenue[j] = m_pGameDeskPtr->CalculateRevenue(j, m_llWinLoseScore[j]);
                m_llScore[j] = m_llWinLoseScore[j] + m_dwlCellScore - m_dwRevenue[j];
            }
            else if(0 == m_llWinLoseScore[j] && vecMaxDiceChairID.size() != m_wPlayerCount)
            {
                m_llWinLoseScore[j] -= m_dwlCellScore;
                m_llScore[j] = m_llWinLoseScore[j];
            }
        }
        if(vecMaxDiceChairID.size() == 0)
        {
            m_llScore[j] = m_dwlCellScore;
        }
    }
    int iCount = 0;
    int iUseScore = 0;
    for (size_t j = 0; (j < GAME_PLAYER) && (COMPARE_SIZE == m_dwGameType); ++j)
    {
        if(!m_PlayerBet.cbIsBet[j])
        {
            continue;
        }

        for (size_t i = 0; i < vecMaxDiceChairID.size(); ++i)
        {
            if(j == vecMaxDiceChairID[i] && 1 == (vecMaxDiceChairID.size()-iCount))
            {
                CT_INT32 iScores = ((llTotalScore - (vecMaxDiceChairID.size() * m_dwlCellScore) - iUseScore)/(vecMaxDiceChairID.size()-iCount));
                m_llWinLoseScore[j] = iScores;
                m_dwRevenue[j] = m_pGameDeskPtr->CalculateRevenue(j, m_llWinLoseScore[j]);
                m_llScore[j] = m_llWinLoseScore[j] + m_dwlCellScore - m_dwRevenue[j];
                iUseScore += m_llWinLoseScore[j];
                iCount++;
            }
            else if(j == vecMaxDiceChairID[i])
            {
                CT_INT32 iScores = ((llTotalScore - (vecMaxDiceChairID.size() * m_dwlCellScore) - iUseScore)/(vecMaxDiceChairID.size()-iCount));
                std::uniform_int_distribution<unsigned int> disValue((int)iScores*0.5, (int)(iScores*1.5));
                m_llWinLoseScore[j] =  disValue(m_gen);
                m_dwRevenue[j] = m_pGameDeskPtr->CalculateRevenue(j, m_llWinLoseScore[j]);
                m_llScore[j] = m_llWinLoseScore[j] + m_dwlCellScore - m_dwRevenue[j];
                iUseScore += m_llWinLoseScore[j];
                iCount++;
            }
            else if(0 == m_llWinLoseScore[j] && vecMaxDiceChairID.size() != m_wPlayerCount)
            {
                m_llWinLoseScore[j] -= m_dwlCellScore;
                m_llScore[j] = m_llWinLoseScore[j];
            }
        }
        if(vecMaxDiceChairID.size() == 0)
        {
            m_llScore[j] = m_dwlCellScore;
        }
    }

    for (size_t k = 0; k < GAME_PLAYER; ++k)
    {
        if(!m_PlayerBet.cbIsBet[k])
        {
            continue;
        }
        if(m_pGameDeskPtr->IsAndroidUser(k))
        {
            m_sdStockScore += (m_llWinLoseScore[k]);
        }
        else
        {
            if(m_llWinLoseScore[k] > 0)
            {
                m_sllRealWinTotalScore += (m_llWinLoseScore[k]-m_dwRevenue[k]);
                m_sllRealTotalRevenue += m_dwRevenue[k];
                m_sllRealWinTotalCount++;
            }
            else if(m_llWinLoseScore[k] < 0)
            {
                m_sllRealLoseTotalScore += m_llWinLoseScore[k];
                m_sllRealLoseTotalCount++;
            }
            m_sllRealPlayTotalCount++;
        }
        gameEnd.dWinScore[k] = (m_llScore[k]*TO_DOUBLE);
    }
    SendTableData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &gameEnd, sizeof(gameEnd), false);

    m_pGameDeskPtr->SetGameTimer(IDI_SEND_END, TIME_SEND_END);

    if((time(NULL) - m_dwOldTime) > 1*60)
    {
        LOG(INFO) << "NormalGameEnd TableID:" << m_pGameDeskPtr->GetTableID() <<" StockScore:"<< m_sdStockScore;
        CT_DOUBLE dRTP = 95.0;
        if(m_sllRealLoseTotalScore != 0)
        {
            dRTP = (CT_DWORD)(((CT_DOUBLE)m_sllRealWinTotalScore/(CT_DOUBLE)m_sllRealLoseTotalScore)*-100);
        }
        LOG(INFO) << "RealWinTotalScore:" << m_sllRealWinTotalScore << " RealLoseTotalScore:" << m_sllRealLoseTotalScore << " RTP:" << dRTP  << " RealTotalRevenue:"<< m_sllRealTotalRevenue << " RealTotalWin:" << m_sllRealWinTotalScore+m_sllRealTotalRevenue;
        LOG(INFO) << "1:" << m_sllStatisticsPointCount[0] << " 2:" << m_sllStatisticsPointCount[1] << " 3:" << m_sllStatisticsPointCount[2] << " 4:" << m_sllStatisticsPointCount[3] << " 5:" << m_sllStatisticsPointCount[4] << " 6:" << m_sllStatisticsPointCount[5];
        LOG(INFO) << "RealWinTotalCount:" << m_sllRealWinTotalCount << " RealLoseTotalCount:" << m_sllRealLoseTotalCount << " RealPlayTotalCount:" << m_sllRealPlayTotalCount;
    }

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
    auto itUser = m_mapUserBaseInfo.find(pDianKongData->dwUserID);
    if (it != m_mapUserPointControl.end())
    {
		it->second.dwUserID = pDianKongData->dwUserID;
		it->second.dwGameIndex = pDianKongData->dwGameIndex;
		it->second.iDianKongZhi = pDianKongData->iDianKongZhi;
        it->second.llDianKongFen = pDianKongData->llDianKongFen;
        if(it->second.iDianKongZhi > 0)
        {
            //点控输
            it->second.llDianKongFen *= -1;
        }
        LOG(INFO) << "WEB modify DianKong " << pDianKongData->dwUserID << " iDianKongZhi:"<<pDianKongData->iDianKongZhi<< " llDianKongFen:"<<pDianKongData->llDianKongFen;
    }
    else if(en_NoCtrl != pDianKongData->iDianKongZhi)
    {
        if(itUser != m_mapUserBaseInfo.end() && pDianKongData->iDianKongZhi < 0 && itUser->second.llWinLoseScore > 0)
        {
            LOG(ERROR) << "WEB join DianKong Failure" << pDianKongData->dwUserID << " iDianKongZhi:" << pDianKongData->iDianKongZhi <<" llDianKongFen:" << pDianKongData->llDianKongFen << " llWinLoseScore:" << itUser->second.llWinLoseScore;
            return true;
        }
		MSG_G2DB_User_PointControl User_PointControl;
		memset(&User_PointControl,0,sizeof(User_PointControl));
		User_PointControl.dwUserID = pDianKongData->dwUserID;
		User_PointControl.dwGameIndex = pDianKongData->dwGameIndex;
		User_PointControl.iDianKongZhi = pDianKongData->iDianKongZhi;
        User_PointControl.llDianKongFen = pDianKongData->llDianKongFen;
        if(User_PointControl.iDianKongZhi > 0)
        {
            //点控输
            User_PointControl.llDianKongFen *= -1;
        }
        User_PointControl.llCurrDianKongFen = pDianKongData->llCurrDianKongFen;
        m_mapUserPointControl.insert(std::make_pair(pDianKongData->dwUserID, User_PointControl));
        LOG(INFO) << "WEB join DianKong " << pDianKongData->dwUserID << " iDianKongZhi:"<<pDianKongData->iDianKongZhi<< " llDianKongFen:"<<pDianKongData->llDianKongFen;
    }
    return true;
}
//设置游戏黑名单
CT_VOID CGameProcess::SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList)
{
    //关闭黑名单功能
    /*for (auto i = vecBlackList.begin(); i != vecBlackList.end(); ++i)
    {
        auto user = m_mapBlackList.find(i->dwUserID);
        if(user == m_mapBlackList.end())
        {
            tagJZNCGameBlackList hbGameBlack;
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
    }*/
}
CT_BOOL CGameProcess::OnUserBet(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
    //变量定义
    //CMD_C_UserBet * pNetInfo = (CMD_C_UserBet *)pDataBuffer;
    if(dwDataSize != sizeof(CMD_C_UserBet))
    {
        LOG(ERROR) << "OnUserBet  dwDataSize:" << (int)dwDataSize <<" != " << "sizeof(CMD_C_Apply):"<<(int)sizeof(CMD_C_UserBet);
        return false;
    }
    if(GAME_SCENE_BET != m_cbGameStatus)
    {
        //LOG(ERROR) << "OnUserBet  m_cbGameStatus:" << (int)m_cbGameStatus;
        return true;
    }

    if(m_PlayerBet.cbIsBet[wChairID])
    {
        LOG(ERROR) << "OnUserBet  m_cbPlayerBet:" << (int)m_PlayerBet.cbIsBet[wChairID];
        return true;
    }
    CMD_S_Bet SendBet;
    memset(&SendBet, 0, sizeof(SendBet));
    SendBet.wChairID = wChairID;
    if(m_pGameDeskPtr->GetUserScore(wChairID) < m_dwlCellScore)
    {
        SendBet.cbIsFail = 1;
        SendTableData(wChairID, SUB_S_BET, &SendBet, sizeof(SendBet),false);
        return true;
    }
    m_PlayerBet.cbCount++;
    m_PlayerBet.cbIsBet[wChairID] = true;
    SendTableData(INVALID_CHAIR, SUB_S_BET, &SendBet, sizeof(SendBet),false);
    CT_LONGLONG llScore = 0;
    llScore -= m_dwlCellScore;
    m_pGameDeskPtr->AddUserScore(wChairID,llScore, true);

    if(m_wPlayerCount <= m_PlayerBet.cbCount)
    {
        m_cbGameStatus = GAME_SCENE_THROWDICE;
        CMD_S_StartThrowDice StartThrowDice;
        memset(&StartThrowDice, 0, sizeof(StartThrowDice));
        StartThrowDice.wTimeLeft = m_dwRollDice;
        SendTableData(INVALID_CHAIR, SUB_S_START_THROW_DICE, &StartThrowDice, sizeof(StartThrowDice),false);
        m_dwSysTime = time(NULL);
        m_pGameDeskPtr->SetGameTimer(IDI_THROW_DICE,(m_dwRollDice+TIME_ANIMATION_DICE+TIME_NETWORK_COMPENSATE)*1000);
    }
    return true;
}
CT_BOOL CGameProcess::OnUserThrowDice(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
//变量定义
    //CMD_C_ThrowDice * pNetInfo = (CMD_C_ThrowDice *)pDataBuffer;
    if(dwDataSize != sizeof(CMD_C_ThrowDice))
    {
        LOG(ERROR) << "OnUserThrowDice  dwDataSize:" << (int)dwDataSize <<" != " << "sizeof(CMD_C_Apply):"<<(int)sizeof(CMD_C_UserBet);
        return false;
    }
    if(GAME_SCENE_THROWDICE != m_cbGameStatus)
    {
        //LOG(ERROR) << "OnUserThrowDice  m_cbGameStatus:" << (int)m_cbGameStatus;
        return true;
    }

    if(m_PlayerDice.cbIsDice[wChairID])
    {
        LOG(ERROR) << "OnUserThrowDice  m_cbPlayerBet:" << (int)m_cbThrowDice[wChairID];
        return true;
    }

    CMD_S_ThrowDice ThrowDice;
    memset(&ThrowDice, 0, sizeof(ThrowDice));
    ThrowDice.wChairID = wChairID;
    ThrowDice.cbDice = m_cbThrowDice[wChairID];
    m_PlayerDice.cbCount++;
    m_PlayerDice.cbIsDice[wChairID] = true;
    SendTableData(INVALID_CHAIR, SUB_S_THROW_DICE, &ThrowDice, sizeof(ThrowDice),false);

    if(m_wPlayerCount <= m_PlayerDice.cbCount)
    {
        OnEventGameEnd(INVALID_CHAIR, GER_NORMAL);
    }

    return true;
}
int CGameProcess::GetCtrlEffectKey(CT_WORD wChairID)
{
    CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(wChairID);
    auto pUser = m_mapUserBaseInfo.find(dwUserID);
    auto userCtrl = m_mapUserPointControl.find(dwUserID);
    auto userBlack = m_mapBlackList.find(dwUserID);
    if(userBlack != m_mapBlackList.end() && (pUser != m_mapUserBaseInfo.end()) && (!pUser->second.bRobot) && (m_iProba(m_gen) < userBlack->second.wUserLostRatio))
    {
        //黑名单用户固定用2
        return 2;
    }
    else if(userCtrl != m_mapUserPointControl.end() && (pUser != m_mapUserBaseInfo.end()) && (!pUser->second.bRobot))
    {
        for (size_t i = 0; i < m_vPointtrlCfg.size(); ++i)
        {
            if(m_vPointtrlCfg[i].iCheatRatioLower <= userCtrl->second.iDianKongZhi && m_vPointtrlCfg[i].iCheatRatioUpper > userCtrl->second.iDianKongZhi
               && m_iProba(m_gen) < m_vPointtrlCfg[i].iCtrlProb)
            {
                return m_vPointtrlCfg[i].iExeEffect;
            }
        }
    }
    else if((pUser != m_mapUserBaseInfo.end()) && (!pUser->second.bRobot))
    {
        for (size_t i = 0; i < m_vWinCtrlCfg.size(); ++i)
        {
            if((m_vWinCtrlCfg[i].iContWinLossLowerCnt <= (int)pUser->second.dwContinueLoseCount)
            && (m_vWinCtrlCfg[i].iContWinLossUpperCnt > (int)pUser->second.dwContinueLoseCount) && (m_iProba(m_gen) < m_vLoseCtrlCfg[i].iCtrlProb))
            {
                return m_vWinCtrlCfg[i].iCtrlEffect;
            }
        }
        for (size_t i = 0; i < m_vLoseCtrlCfg.size(); ++i)
        {
            if((m_vLoseCtrlCfg[i].iContWinLossLowerCnt <= (int)pUser->second.dwContinueWinCount)
            && (m_vLoseCtrlCfg[i].iContWinLossUpperCnt > (int)pUser->second.dwContinueWinCount) && (m_iProba(m_gen) < m_vLoseCtrlCfg[i].iCtrlProb))
            {
                return m_vLoseCtrlCfg[i].iCtrlEffect;
            }
        }
    }
    else if((pUser != m_mapUserBaseInfo.end()) && (pUser->second.bRobot))
    {
        auto pAICtrl = m_mAIStockCtrlCfg.find(m_pGameRoomKindInfo->wRoomKindID);
        for (size_t i = 0;(pAICtrl != m_mAIStockCtrlCfg.end()) && (i < pAICtrl->second.size()); ++i)
        {
            if((pAICtrl->second[i].llStockLowerScore <= m_sdStockScore) && (pAICtrl->second[i].llStockUpperScore > m_sdStockScore))
            {
                if(0 == m_vEffectRobotChairID.size())
                {
                    CT_DWORD wdRobotCount = m_vRobotChairID.size();
                    CT_DWORD iRobotSize = m_vRobotChairID.size()/pAICtrl->second[i].iUnitCtrlStep;
                    if(iRobotSize >= wdRobotCount)
                    {
                        m_vEffectRobotChairID.assign(m_vRobotChairID.begin(),m_vRobotChairID.end());
                    }
                    else
                    {
                        for (size_t j = 0; (j < wdRobotCount+10) && m_vEffectRobotChairID.size() < iRobotSize; ++j)
                        {
                            size_t k = 0;
                            CT_WORD wTmpChairID = m_vRobotChairID[RAND_NUM(m_dwRandIndex++)%wdRobotCount];
                            if(!m_pGameDeskPtr->IsExistUser(wTmpChairID))
                            {
                                continue;
                            }
                            for (k = 0; k < m_vEffectRobotChairID.size(); ++k)
                            {
                                if(wTmpChairID == m_vEffectRobotChairID[k])
                                {
                                    break;
                                }
                            }
                            if(k == m_vEffectRobotChairID.size())
                            {
                                m_vEffectRobotChairID.push_back(wTmpChairID);
                            }
                        }

                        //LOG(INFO) << "m_vEffectRobotChairID:" << m_vEffectRobotChairID[0]<<" " << m_vEffectRobotChairID[1] << " " << m_vEffectRobotChairID[2];

                    }
                }
                if((m_wEffectRobotCount < pAICtrl->second[i].iCtrlMinCnt)
                || ((m_wEffectRobotCount < m_vEffectRobotChairID.size())&& (m_iProba(m_gen) < pAICtrl->second[i].dwActiveProb)))
                {

                    for (size_t j = 0; j < m_vEffectRobotChairID.size(); ++j)
                    {
                        //LOG(INFO) << "wChairID:" << wChairID <<" m_vEffectRobotChairID["<<j<<"]:" << m_vEffectRobotChairID[j];
                        if(wChairID == m_vEffectRobotChairID[j])
                        {
                            m_wEffectRobotCount++;
                            return pAICtrl->second[i].iCtrlEffect;
                        }
                    }
                }
            }
        }
    }

    return 0;
}
//删除用户战绩及排行榜
CT_VOID CGameProcess::DeleteUserByChairID(CT_WORD wChairID)
{
    CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(wChairID);

    //删除机器人信息
    if(m_mapUserBaseInfo[dwUserID].bRobot)
    {
        m_mapUserBaseInfo.erase(dwUserID);
    }
    return;
}
CT_VOID CGameProcess::DelayWriteScore()
{
    //写分
    for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
    {
        if(!m_PlayerBet.cbIsBet[i])
        {
            continue;
        }
        CT_DWORD dwUserID = m_pGameDeskPtr->GetUserID(i);
        auto pUser = m_mapUserBaseInfo.find(dwUserID);
        auto userCtrl = m_mapUserPointControl.find(dwUserID);
        m_RecordScoreInfo[i].dwUserID = m_pGameDeskPtr->GetUserID(i);
        m_RecordScoreInfo[i].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(i);
        m_RecordScoreInfo[i].cbStatus = 1;
        m_RecordScoreInfo[i].iScore = m_llWinLoseScore[i];
        m_RecordScoreInfo[i].dwRevenue = m_dwRevenue[i];

        if(m_llScore[i] > 0)
        {
            ScoreInfo  ScoreData;
            memset(&ScoreData, 0, sizeof(ScoreInfo));
            ScoreData.dwUserID = i;
            ScoreData.bBroadcast = true;
            ScoreData.llScore = m_llScore[i];
            ScoreData.llRealScore= m_llScore[i];;
            m_pGameDeskPtr->WriteUserScore(i, ScoreData);
            if(userCtrl == m_mapUserPointControl.end())
            {
                pUser->second.dwContinueLoseCount = 0;
                pUser->second.dwContinueWinCount++;
            }
        }
        else
        {
            if(userCtrl == m_mapUserPointControl.end())
            {
                pUser->second.dwContinueLoseCount++;
                pUser->second.dwContinueWinCount = 0;
            }
        }
        pUser->second.llWinLoseScore += (m_llWinLoseScore[i]-m_dwRevenue[i]);
        if(userCtrl != m_mapUserPointControl.end())
        {
            userCtrl->second.llCurrDianKongFen += (m_llWinLoseScore[i]-m_dwRevenue[i]);
            MSG_G2DB_User_PointControl data;
            memset(&data, 0, sizeof(data));
            data = userCtrl->second;
            if(data.iDianKongZhi > 0)
            {
                data.llDianKongFen *= -1;
            }
            m_pGameDeskPtr->SavePlayerDianKongData(data);
            //点控赢
            if((userCtrl->second.iDianKongZhi < 0) && (userCtrl->second.llCurrDianKongFen >= userCtrl->second.llDianKongFen))
            {
                userCtrl->second.iDianKongZhi = en_NoCtrl;
                m_pGameDeskPtr->SavePlayerDianKongData(userCtrl->second);
                pUser->second.dwPointCtlEndTime = time(NULL);
                LOG(INFO)<<"DianKong Delete " << userCtrl->first <<" iDianKongZhi:"<< userCtrl->second.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(userCtrl->second.llDianKongFen*TO_DOUBLE) << " CurrDianKongFen:" << (CT_DOUBLE)(userCtrl->second.llCurrDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(pUser->second.llWinLoseScore*TO_DOUBLE);
                m_mapUserPointControl.erase(userCtrl);
            }
                //点控输
            else if((userCtrl->second.iDianKongZhi > 0) && (userCtrl->second.llCurrDianKongFen <= userCtrl->second.llDianKongFen))
            {
                userCtrl->second.iDianKongZhi = en_NoCtrl;
                MSG_G2DB_User_PointControl data;
                memset(&data, 0, sizeof(data));
                data = userCtrl->second;
                data.llDianKongFen *= -1;
                m_pGameDeskPtr->SavePlayerDianKongData(data);
                pUser->second.dwPointCtlEndTime = time(NULL);
                LOG(INFO)<<"DianKong Delete " << userCtrl->first <<" iDianKongZhi:"<< userCtrl->second.iDianKongZhi << " DianKongFen:"<<(CT_DOUBLE)(userCtrl->second.llDianKongFen*TO_DOUBLE) << " CurrDianKongFen:" << (CT_DOUBLE)(userCtrl->second.llCurrDianKongFen*TO_DOUBLE) << " WinLoseScore:" << (CT_DOUBLE)(pUser->second.llWinLoseScore*TO_DOUBLE);
                m_mapUserPointControl.erase(userCtrl);
            }
        }

    }
    m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, NULL, 0, 0, (CT_LONGLONG)m_sdStockScore, 0, 0, 0, 0);

    for (CT_DWORD i= 0; i < GAME_PLAYER; ++i)
    {
        if (!m_pGameDeskPtr->IsExistUser(i))
        {
            continue;
        }
        if(m_pGameDeskPtr->GetUserStatus(i) == sOffLine)
        {
            //斗地主游戏剔除用户时不需要广播状态给客户端
            m_pGameDeskPtr->ClearTableUser(i, false);
        }
    }
}