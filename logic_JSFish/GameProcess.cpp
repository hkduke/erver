
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
#include "F_JsBu.h"

//打印日志
#define		PRINT_LOG_INFO						1
#define		PRINT_LOG_WARNING					1

bool CGameProcess::m_load_config = false;
bool CGameProcess::m_start__timer_save_stock = false;
extern BYStockConfig g_bystockconfig;

CGameProcess::CGameProcess(void)
{
	m_pGame_Table_Logic = new GameTableLogic(this);
	m_chair_info.clear();
}

CGameProcess::~CGameProcess(void)
{
	if(m_pGameDeskPtr)
	{
		delete m_pGame_Table_Logic;
	}
}

//游戏开始
void CGameProcess::OnEventGameStart()
{
	
}

//游戏结束
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{

}

//发送场景
void CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{

}

//定时器事件
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch(dwTimerID)
	{
		case IDI_GAME_LOOP:
		{
			m_pGame_Table_Logic->OnTimerMessage(dwTimerID, dwParam);

			for(map_chair_info::iterator it = m_chair_info.begin(); it != m_chair_info.end(); it++)
			{
				if(time(NULL) - it->second.user_last_fire_time >= 120)
				{
					//120秒不开炮就踢掉
					OnUserLeft(it->first, false);
					//一次只能检测一个
					break;
				}
			}
		}
		break;
		case IDI_SAVE_JSFISH_STOCK:
		{
		    //保存库存数据，用于显示波动图
			js_fish_save_info jfsi;
			jfsi.save_stock.serverID = 0;
			jfsi.save_stock.llCurKuCun = g_bystockconfig.GetCurValue();
			jfsi.save_stock.llCurTime = time(NULL);
		    m_pGameDeskPtr->SaveJSFishInfo(stock_info, jfsi);

			if(diffTimeDay(g_bystockconfig.m_today_time, time(NULL)))
			{
				//重置当日数据
				g_bystockconfig.ResetTodayInfo();
			}
		}
		break;
		case IDI_SAVE_JSFISH_SYSINFO:
		{
			//保存总库存总税收等系统信息
			js_fish_save_info jfsi;
			jfsi.sys_info.llSystemLoseScore = g_bystockconfig.GetSysLoseScore();
			jfsi.sys_info.llSystemWinScore = g_bystockconfig.GetSysWinScore();
			jfsi.sys_info.llSystemStock = g_bystockconfig.GetCurValue();
			jfsi.sys_info.llSystemTax = g_bystockconfig.GetCurrentRevenue();
			jfsi.sys_info.llSystemDiankongValue = 0;
		    m_pGameDeskPtr->SaveJSFishInfo(sys_info, jfsi);
		}
		break;
		case IDI_SAVE_TODAY_INFO:
		{
			js_fish_save_info jfsi;
			jfsi.todayinfo.dwTotalXiFenCount = g_bystockconfig.m_dwTodayXiFenCount;
			jfsi.todayinfo.dwTotalRewardCount = g_bystockconfig.m_dwTodayRewardCount;
			jfsi.todayinfo.dwTotalPoChanCount = g_bystockconfig.m_dwTodayTotalPlayerCount_Bankrupted;
			jfsi.todayinfo.dwTotalWinCount = g_bystockconfig.m_dwTodayTotalPlayerCount_Win;
			jfsi.todayinfo.dwTotalEnterCount = g_bystockconfig.m_dwTodayTotalEnterCount;
			jfsi.todayinfo.llTodayDianKongWinLoseTotalScore = 0;
			jfsi.todayinfo.llTodayTotalKuCun = g_bystockconfig.m_llTodayTotalKuCun;
			jfsi.todayinfo.llTodayTotalTax = g_bystockconfig.m_llTodayTotalTax;
			jfsi.todayinfo.llTotalLoseScore = g_bystockconfig.m_llTodayTotalLoseScore;
			jfsi.todayinfo.llTotalWinScore = g_bystockconfig.m_llTodayTotalWinScore;
			m_pGameDeskPtr->SaveJSFishInfo(today_info, jfsi);

			g_bystockconfig.m_llTodayTotalKuCun = 0;
			g_bystockconfig.m_llTodayTotalTax = 0;
			g_bystockconfig.m_llTodayTotalLoseScore = 0;
			g_bystockconfig.m_llTodayTotalWinScore = 0;
		}
		break;
		default:
		{
			//LOG(ERROR) << "未处理的定时器ID: "<< dwTimerID << " dwParam: "<< dwParam;
		}
		break;
	}
}

void CGameProcess::save_sysinfo_shutserver()
{
	//保存总库存总税收等系统信息
	js_fish_save_info jfsi;
	jfsi.sys_info.llSystemLoseScore = g_bystockconfig.GetSysLoseScore();
	jfsi.sys_info.llSystemWinScore = g_bystockconfig.GetSysWinScore();
	jfsi.sys_info.llSystemStock = g_bystockconfig.GetCurValue();
	jfsi.sys_info.llSystemTax = g_bystockconfig.GetCurrentRevenue();
	jfsi.sys_info.llSystemDiankongValue = 0;
	m_pGameDeskPtr->SaveJSFishInfo(sys_info, jfsi);
}

//游戏消息
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	if(m_pGame_Table_Logic)
	{
		CT_VOID *pData = const_cast<CT_VOID *>(pDataBuffer);
		m_pGame_Table_Logic->OnGameMessage(wChairID, dwSubCmdID, pData, dwDataSize);
	}
	return CT_TRUE;
}

void CGameProcess::OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if(m_chair_info.find(dwChairID) != m_chair_info.end())
	{
		//发送场景
		m_pGame_Table_Logic->OnEventSendGameScene(dwChairID, 0 , false);
		return;
	}

	if(m_chair_info.size() == 0)
	{
		//第一个人进房间，初始化所有逻辑数据
		m_pGame_Table_Logic->Initialization();
		m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Start);
	}

	//初始化所有玩家相关信息
	InitAllPlayerInfo(dwChairID);

	m_pGame_Table_Logic->user_enter(dwChairID);
}

//用户离开
void CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag)
{
	m_pGame_Table_Logic->user_left(dwChairID);

	//卸载所有玩家的相关信息
	UnInitAllPlayerInfo(dwChairID);

	if (m_pGameDeskPtr->GetGameRoundPhase()  == en_GameRound_Start)
	{
		if (m_pGameDeskPtr->GetGameKindInfo()->cbState == SERVER_STOP)
		{
			//安全关服，传ENTER_ROOM_SERVER_STOP给客户端，显示安全关服信息
			m_pGameDeskPtr->ClearTableUser(dwChairID, true, true, ENTER_ROOM_SERVER_STOP);
		}
		else
		{
			m_pGameDeskPtr->ClearTableUser(dwChairID, true, true);
		}
	}

    if(m_chair_info.size() == 0)
	{
    	m_pGameDeskPtr->KillGameTimer(IDI_GAME_LOOP);
    	m_pGame_Table_Logic->CleanUp();
		m_pGameDeskPtr->SetGameRoundPhase(en_GameRound_Free);
	}


}

void CGameProcess::InitAllPlayerInfo(CT_DWORD dwChairID)
{
	chair_info chair;
	chair.user_id = m_pGameDeskPtr->GetUserID(dwChairID);
	chair.user_enter_score = m_pGameDeskPtr->GetUserScore(dwChairID);
	chair.user_current_score = chair.user_enter_score;
	chair.user_win_lose_score = 0;
	chair.user_miss_stock = 0;
	chair.user_stream = 0;
	chair.user_tax = 0;
	chair.user_send_pao_count = 0;
	chair.user_last_fire_time = time(NULL);
	m_chair_info[dwChairID] = chair;
	m_pGame_Table_Logic->OnEventSendGameScene(dwChairID, 0 , false);
	m_pGame_Table_Logic->InitRoleInfo(dwChairID, false);

	if(m_pGameDeskPtr->IsAndroidUser(dwChairID))
    {
	    //如果是机器人就发机器人的信息下去。
        S_Enter_Android_info info;
        info.chair_id = dwChairID;
        info.score = (double)get_user_score(dwChairID) / 100.0f;
        m_pGameDeskPtr->SendTableData(dwChairID, SUB_S_ENTER_ANDROID_INFO, &info, sizeof(info));

       // LOG(ERROR) << " 极速捕鱼机器人: "<< m_pGameDeskPtr->GetUserID(dwChairID) << "进入";
    }

    g_bystockconfig.m_dwTodayTotalEnterCount++;
}

void CGameProcess::UnInitAllPlayerInfo(CT_DWORD dwChairID)
{
	m_pGame_Table_Logic->UnInitRoleInfo(dwChairID, false);
	map_chair_info::iterator it = m_chair_info.find(dwChairID);
	if(it != m_chair_info.end())
	{
		int64_t addScore = get_user_win_lose_score(dwChairID);
		if(addScore > 0)
		{
			g_bystockconfig.m_dwTodayTotalPlayerCount_Win++;
		}
		else if(it->second.user_current_score < g_game_config_xml.cannon_mulriple_[0])
		{
			g_bystockconfig.m_dwTodayTotalPlayerCount_Bankrupted++;
		}
		else if(addScore < 0)
		{
			g_bystockconfig.m_dwTodayTotalPlayerCount_Lose++;
		}
		else if(addScore == 0)
		{
			g_bystockconfig.m_dwTodayTatalPlayerCount_NoWinLose++;
		}

		ScoreInfo  ScoreData;
		ScoreData.dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
		ScoreData.bBroadcast = true;
		ScoreData.llScore = addScore;
		ScoreData.llRealScore = addScore;
		RecordScoreInfo scoreInfo[GAME_PLAYER];
		scoreInfo[dwChairID].dwUserID = ScoreData.dwUserID;
		scoreInfo[dwChairID].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(dwChairID);
		scoreInfo[dwChairID].cbStatus = 1;
		scoreInfo[dwChairID].llSourceScore = it->second.user_enter_score;
		scoreInfo[dwChairID].iScore = addScore;
		//CT_LONGLONG llPlayerTotalWinLoseScores = it->second.llPlayerTotalWinScores + it->second.llPlayerTotalLoseScores;
		//CT_DWORD dWTaxPro = m_pGameDeskPtr->GetFishTaxPro();
		scoreInfo[dwChairID].dwRevenue = it->second.user_tax;
		scoreInfo[dwChairID].iAreaJetton[0] = it->second.user_send_pao_count;

		if (m_pGameDeskPtr->GetGameKindInfo()->wRoomKindID != FREE_ROOM)
		{
			//写分
			m_pGameDeskPtr->WriteUserScore(dwChairID, ScoreData);
			//写游戏记录
			m_pGameDeskPtr->RecordRawInfo(scoreInfo, GAME_PLAYER, NULL, 0, 0, 0, 0, 0, 0, 0);
		}

		if(it->second.user_miss_stock > 0)
        {
            //记录miss库
            js_fish_save_info jsfi;
            jsfi.missInfo.dwUserID = m_pGameDeskPtr->GetUserID(dwChairID);
            jsfi.missInfo.miss = it->second.user_miss_stock;
            m_pGameDeskPtr->SaveJSFishInfo(miss_info, jsfi);

           // LOG(ERROR) << "保存玩家: " << jsfi.missInfo.dwUserID << " miss库:"<<jsfi.missInfo.miss;
        }

		m_chair_info.erase(it);
	}
}

//用户换桌
void CGameProcess::OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce)
{

}

//设置指针
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	//assert(NULL != pTableFrame);
	m_pGameDeskPtr = pTableFrame;

	//配置只加载一次
	if(CGameProcess::m_load_config == false)
	{
		CGameProcess::m_load_config = true;
        read_conf();
	}

	//整个服务器只启动一个记录库存的定时器
	if(CGameProcess::m_start__timer_save_stock == false)
	{
		CGameProcess::m_start__timer_save_stock = true;
		m_pGameDeskPtr->SetGameTimer(IDI_SAVE_JSFISH_STOCK, TIME_SAVE_JSFISH_STOCK);
		m_pGameDeskPtr->SetGameTimer(IDI_SAVE_JSFISH_SYSINFO, TIME_SAVE_JSFISH_SYSINFO);
		m_pGameDeskPtr->SetGameTimer(IDI_SAVE_TODAY_INFO, TIME_SAVE_TODAY_INFO);
	}
}

//通过这个函数保证xml path scene 等配置文件整个服务器只会加载一次,并且这些数据都放在静态存储区中.
void CGameProcess::read_conf()
{
	//加载jsby_config.xml
	g_game_config_xml.load_game_config();
	//由于不同场次的服务器都是读取的同一个xml文件，所以在这里修正不同场次下的炮倍信息
	if(m_pGameDeskPtr->GetGameKindInfo()->wRoomKindID == 1)
	{
		for(int i = 0; i < MAX_CANNON_TYPE; i++)
		{
			g_game_config_xml.cannon_mulriple_[i] = g_game_config_xml.cannon_mulriple_[i] / 10;
		}
	}
	else if(m_pGameDeskPtr->GetGameKindInfo()->wRoomKindID == 2)
	{
		//中级场不用修正。
	}
	else if(m_pGameDeskPtr->GetGameKindInfo()->wRoomKindID == 3)
	{
		for(int i = 0; i < MAX_CANNON_TYPE; i++)
		{
			g_game_config_xml.cannon_mulriple_[i] = g_game_config_xml.cannon_mulriple_[i] * 10;
		}
	}
	else
    {
	    LOG(ERROR) << "CGameProcess::read_conf() 无效的RoomKindID: " << m_pGameDeskPtr->GetGameKindInfo()->wRoomKindID;
    }

	//加载其他配置
	m_pGame_Table_Logic->load_conf();
}

//服务器启动，就从数据库中读取系统信息和控制信息，并且设置数据到静态存储区中。
CT_VOID CGameProcess::SetJSFishData(enJSFishDataType type, void* pData)
{
	if(pData == NULL)
	{
		return;
	}

    if(type == enJSFishDataType::en_SysInfo_type)
    {
    	tagJSFishSystemInfo *pInfo = (tagJSFishSystemInfo *)pData;
    	g_bystockconfig.InitSysWinScore(pInfo->llSystemWinScore);
    	g_bystockconfig.InitSysLoseScore(pInfo->llSystemLoseScore);
    	g_bystockconfig.InitCurValue(pInfo->llSystemStock);
    	g_bystockconfig.InitCurrentRevenue(pInfo->llSystemTax);
    }
    else if(type == enJSFishDataType::en_ControlInfo_type)
    {
    	tagJSFishControlInfo *pInfo = (tagJSFishControlInfo *)pData;
		g_bystockconfig.SetValueA(pInfo->llMustKillValue);
		g_bystockconfig.SetValueB(pInfo->llProKillValue);
		g_bystockconfig.SetValueC(pInfo->llProAwardValue);
		g_bystockconfig.SetValueD(pInfo->llMustAwardValue);
		g_bystockconfig.SetRevenueCount(pInfo->nRevenueCount);
    }
    else if(type == enJSFishDataType::en_SaveSysInfo_type)
	{
    	save_sysinfo_shutserver();
	}
	else if(type == enJSFishDataType::en_SetPlayerMiss_type)
    {
	    JSFish_Player_Miss_Info *pMissInfo = (JSFish_Player_Miss_Info *)pData;
	    for(map_chair_info::iterator it = m_chair_info.begin(); it != m_chair_info.end(); it++)
        {
	        if(it->second.user_id == pMissInfo->dwUserID)
            {
	            set_user_miss_stock(it->first, pMissInfo->miss);
	            //LOG(ERROR) << "设置玩家: "<< it->second.user_id << " miss库:"<<pMissInfo->miss;
                break;
            }
        }
    }
    else
	{
    	LOG(ERROR) << "未处理的数据类型: "<<(int)type;
	}
}

//清理游戏数据
void CGameProcess::ClearGameData()
{

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

int64_t CGameProcess::get_user_score(unsigned short chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		return it->second.user_current_score;
	}
	else
	{
		LOG(ERROR) << "get_user_score 错误, chair_id:"<<chair_id;
		return 0;
	}
}

void CGameProcess::change_user_score(unsigned short chair_id, int64_t user_score)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		it->second.user_current_score += user_score;
	}
	else
	{
		LOG(ERROR) << "change_user_score 错误, chair_id:" << chair_id << " user_score:" << user_score;
	}
}

int64_t CGameProcess::get_user_win_lose_score(unsigned short chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		return it->second.user_win_lose_score;
	}
	else
	{
		LOG(ERROR) << " get_user_win_lose_score 错误, chair_id:" << chair_id;
		return 0;
	}

}

void CGameProcess::set_user_win_lose_score(unsigned short chair_id, int64_t user_score)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		it->second.user_win_lose_score += user_score;
	}
	else
	{
		LOG(ERROR) << "set_user_win_lose_score 错误, chair_id:" << chair_id << " user_score:"<< user_score;
		return;
	}
}

void CGameProcess::broadcast_all(unsigned short subID, void *pBuf, unsigned short data_len)
{
    if(pBuf == NULL || data_len == 0)
    {
        LOG(ERROR) << "broadcast_all 错误, " << "subID: " << subID << " data_len:"<<data_len;
        return;
    }

    SendTableData(INVALID_CHAIR, subID, pBuf, data_len, CT_FALSE);
}

bool CGameProcess::is_exist_user(unsigned short chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it == m_chair_info.end())
	{
		return false;
	}
	else
	{
		return true;
	}
}

uint32_t CGameProcess::get_user_id(uint16_t chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it == m_chair_info.end())
	{
		LOG(ERROR) << "get_user_id 错误, chair_id:"<<chair_id;
		return 0;
	}
	else
	{
		return it->second.user_id;
	}
}

void CGameProcess::set_user_last_fire_time(unsigned short chair_id, time_t last_fire_time)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
	    it->second.user_last_fire_time = last_fire_time;
	}
}

time_t CGameProcess::get_user_last_fire_time(unsigned short chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		return it->second.user_last_fire_time;
	}
	else
	{
		LOG(ERROR) << "get_user_last_fire_time player:" << m_pGameDeskPtr->GetUserID(chair_id) << " 查找最后一次发射时间失败";
		return 0;
	}
}

void CGameProcess::set_user_miss_stock(uint16_t  chair_id ,int64_t miss_stock)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it == m_chair_info.end())
	{
		LOG(ERROR) << "set_user_miss_stock 错误, chair_id:"<<chair_id << " miss_stock:" << miss_stock;
	}
	else
	{
		it->second.user_miss_stock += miss_stock;
	}
}

int64_t CGameProcess::get_user_miss_stock(uint16_t  chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it == m_chair_info.end())
	{
		//LOG(ERROR) << "get_user_miss_stock 错误, chair_id:"<<chair_id;
		return 0;
	}
	else
	{
		return it->second.user_miss_stock;
	}
}

void CGameProcess::set_user_stream(uint16_t chair_id, int64_t user_stream)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		it->second.user_stream += user_stream;
	}
	else
	{
		LOG(ERROR) << "set_user_stream 错误, chair_id: " << chair_id << " user_stream: "<< user_stream;
	}
}

int64_t  CGameProcess::get_user_stream(uint16_t chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it == m_chair_info.end())
	{
		LOG(ERROR) << "get_user_stream 错误, chair_id:"<<chair_id;
		return 0;
	}
	else
	{
		return it->second.user_stream;
	}
}

void CGameProcess::set_user_tax(uint16_t chair_id, int64_t user_tax)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		it->second.user_tax += user_tax;
	}
	else
	{
		LOG(ERROR) << "set_user_tax 错误， chair_id: " << chair_id << " user_tax:" << user_tax;
	}
}

void CGameProcess::set_user_send_count(uint16_t chair_id)
{
	map_chair_info::iterator it = m_chair_info.find(chair_id);
	if(it != m_chair_info.end())
	{
		it->second.user_send_pao_count++;
	}
	else
	{
		LOG(ERROR) << "set_user_send_count 错误， chair_id: " << chair_id;
	}
}

void CGameProcess::send_player_data(unsigned short chair_id, unsigned short subCmdId, void *pBuffer, unsigned short len)
{
    SendTableData(chair_id, subCmdId, pBuffer, len);
}

void CGameProcess::set_timer(uint16_t timer_id, uint32_t Interval, uint32_t Param /*= 0*/)
{
    m_pGameDeskPtr->SetGameTimer(timer_id, Interval, Param);
}

