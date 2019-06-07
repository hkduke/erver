#include "stdafx.h"
#include "UserMgr.h"
#include "NetModule.h"
#include "ServerMgr.h"
#include "UserTask.h"
#include "CMD_Plaza.h"
#include "timeFunction.h"
#include "ConfigOption.h"
#include "ActivityMgr.h"
#include "RedPacketMgr.h"
#include "GroupMgr.h"

extern CNetConnector *pNetDB;

CGlobalUser::CGlobalUser()
	:m_pUserTask(nullptr)
{
	memset(&m_userInfo, 0, sizeof(m_userInfo));
	memset(&m_scoreData, 0, sizeof(m_scoreData));
	memset(&m_rechargeTips, 0, sizeof(m_rechargeTips));
}

CGlobalUser::~CGlobalUser()
{
	//LOG(WARNING) << "remove global user!";
}

CT_VOID	CGlobalUser::SetUserInfo(tagGlobalUserInfo* pUserInfo)
{
	if (pUserInfo == NULL)
	{
		return;
	}

	memcpy(&m_userInfo, pUserInfo, sizeof(m_userInfo));
}

CT_VOID	CGlobalUser::SetRechargeTips(CT_LONGLONG llRecharge, CT_BYTE cbType)
{
	m_rechargeTips.bNewTips = true;
	m_rechargeTips.cbType = cbType;
	m_rechargeTips.m_llLastRecharge = llRecharge;
}

CT_VOID	CGlobalUser::ResetRechargeTips()
{
	memset(&m_rechargeTips, 0, sizeof(m_rechargeTips));
}

CT_VOID	CGlobalUser::SetUserScoreInfo(UserScoreData* pScoreInfo)
{
	if (pScoreInfo == NULL)
	{
		return;
	}

	memcpy(&m_scoreData, pScoreInfo, sizeof(m_scoreData));
}

tagGlobalUserInfo*  CGlobalUser::GetUserInfo()
{
	return &m_userInfo;
}

inline CT_VOID	CGlobalUser::SetOnline(CT_BOOL bOnline) 
{
	m_userInfo.bOnline = bOnline; 

	if (!bOnline)
	{
		m_dwOffTime = Utility::GetTime();
		m_userInfo.dwProxyServerID = 0;
	}

}

UserScoreData*		CGlobalUser::GetUserScoreInfo()
{
	return &m_scoreData;
}

CT_VOID	CGlobalUser::SetUserTask(std::unique_ptr<CUserTask>& pUserTask)
{
	m_pUserTask = std::move(pUserTask);
}

CT_VOID	CGlobalUser::SendRechargeTips()
{
	if (m_rechargeTips.bNewTips && m_userInfo.dwProxyServerID != 0)
	{
		acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(m_userInfo.dwProxyServerID);
		if (pProxySock)
		{
			MSG_SC_RechargeInfo rechargeInfo;
			rechargeInfo.dwMainID = MSG_CONFIG_MSG_MAIN;
			rechargeInfo.dwSubID = SUB_S2C_RECHARGE_TIPS_INFO;
			rechargeInfo.dwValue2 = m_userInfo.dwUserID;
			rechargeInfo.cbResult = m_rechargeTips.cbType;
			rechargeInfo.llRecharge = (CT_LONGLONG)m_rechargeTips.m_llLastRecharge*TO_DOUBLE;
			CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &rechargeInfo, sizeof(rechargeInfo));
		}
	}
}

/////////////////////////////////////////////UserMgr//////////////////////////////////
CUserMgr::CUserMgr()
	:m_pRedis(NULL)
{
}

CUserMgr::~CUserMgr()
{
}


CT_VOID	CUserMgr::SetRedisPtr(acl::redis* pRedis)
{
	m_pRedis = pRedis;
}

CT_VOID CUserMgr::InsertUser(MSG_P2CS_Register_User* pRegisterUser, CT_BYTE cbScoreMode)
{
    tagGlobalUserInfo* pUserInfo = NULL;
	auto it = m_mapGlobalUser.find(pRegisterUser->dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		pUserInfo = it->second->GetUserInfo();
		if (pUserInfo)
		{
			pUserInfo->dwUserID = pRegisterUser->dwUserID;
			pUserInfo->dwChannelID = pRegisterUser->dwChannelID;
			pUserInfo->dwClientChannelID = pRegisterUser->dwClientChannelID;
			pUserInfo->dwProxyServerID = pRegisterUser->dwProxyServerID;
			pUserInfo->llScore = pRegisterUser->llScore;
			pUserInfo->llBankScore = pRegisterUser->llBankScore;
			pUserInfo->dwTotalRecharge = pRegisterUser->dwRecharge;
			pUserInfo->cbSex = pRegisterUser->cbSex;
			pUserInfo->cbVip = pRegisterUser->cbVipLevel;
			pUserInfo->cbVip2 = pRegisterUser->cbVipLevel2;
			pUserInfo->cbHeadID = pRegisterUser->cbHeadID;
			pUserInfo->cbMachineType = pRegisterUser->cbMachineType;
			pUserInfo->cbShowExchange = pRegisterUser->cbShowExchange;
			pUserInfo->cbPlatformId = pRegisterUser->cbPlatformId;
			pUserInfo->cbProvinceCode = pRegisterUser->cbProvinceCode;
			//标记玩家已经上线
			pUserInfo->bOnline = true;
			pUserInfo->dwLastHeartBeatTime = time(NULL);
			_snprintf_info(pUserInfo->szNickName, sizeof(pUserInfo->szNickName), "%s", pRegisterUser->szNickName);
			it->second->SendRechargeTips();
			//_snprintf_info(pUserInfo->szHeadUrl, sizeof(pUserInfo->szHeadUrl), "%s", pRegisterUser->szHeadUrl);
		}
		//LoadUserTaskInfo(pRegisterUser->dwUserID, it->second);
		//如果启用金币场
		//if (cbScoreMode)
		//{
			//加载积分
			//UserScoreData scoreInfo;
			//scoreInfo.cbVipLevel = pRegisterUser->cbVipLevel;
			//pUserInfo->cbVip = pRegisterUser->cbVipLevel;
			//LoadUserScoreInfo(pRegisterUser->dwUserID, scoreInfo);
			//it->second->SetUserScoreInfo(&scoreInfo);

			//加载任务
			//LoadUserTaskInfo(pRegisterUser->dwUserID, it->second);

			//玩家登录任务
			//std::unique_ptr<CUserTask>& pUserTask = it->second->GetUserTask();
			//if (pUserTask.get())
			//{
			//	pUserTask->OnUserLogin();
			//}
		//}

		//LOG(WARNING) << "insert the proxy user already exists?  user id: " << pRegisterUser->dwUserID;
	}
	else
	{
		tagGlobalUserInfo userInfo;
		userInfo.dwUserID = pRegisterUser->dwUserID;
		userInfo.dwChannelID = pRegisterUser->dwChannelID;
		userInfo.dwClientChannelID = pRegisterUser->dwClientChannelID;
		userInfo.dwProxyServerID = pRegisterUser->dwProxyServerID;
		userInfo.dwGem = pRegisterUser->dwGem;
		userInfo.llScore = pRegisterUser->llScore;
		userInfo.llBankScore = pRegisterUser->llBankScore;
		userInfo.dwTotalRecharge = pRegisterUser->dwRecharge;
		userInfo.cbPlatformId = pRegisterUser->cbPlatformId;
		userInfo.cbSex = pRegisterUser->cbSex;
		userInfo.cbVip = pRegisterUser->cbVipLevel;
		userInfo.cbVip2 = pRegisterUser->cbVipLevel2;
		userInfo.cbHeadID = pRegisterUser->cbHeadID;
		userInfo.cbMachineType = pRegisterUser->cbMachineType;
		userInfo.cbShowExchange = pRegisterUser->cbShowExchange;
		//标记玩家已经上线
		userInfo.bOnline = true;
		userInfo.dwLastHeartBeatTime = time(NULL);
		_snprintf_info(userInfo.szNickName, sizeof(userInfo.szNickName), "%s", pRegisterUser->szNickName);
		//_snprintf_info(userInfo.szHeadUrl, sizeof(userInfo.szHeadUrl), "%s", pRegisterUser->szHeadUrl);

		std::shared_ptr<CGlobalUser> pGlobalUser = std::make_shared<CGlobalUser>();
		pGlobalUser->SetUserInfo(&userInfo);
        pUserInfo = pGlobalUser->GetUserInfo();

		//加载任务
		LoadUserTaskInfo(pRegisterUser->dwUserID, pGlobalUser);

		//如果启用金币场
		//if (cbScoreMode)
		//{
			//UserScoreData scoreInfo;
			//scoreInfo.cbVipLevel = pRegisterUser->cbVipLevel;
			//userInfo.cbVip = pRegisterUser->cbVipLevel;
			//LoadUserScoreInfo(pRegisterUser->dwUserID, scoreInfo);
			//pGlobalUser->SetUserScoreInfo(&scoreInfo);

			//加载任务
			//LoadUserTaskInfo(pRegisterUser->dwUserID, pGlobalUser);

			//玩家登录任务
			//std::unique_ptr<CUserTask>& pUserTask = pGlobalUser->GetUserTask();
			//if (pUserTask.get())
			//{
			//	pUserTask->OnUserLogin();
			//}
		//}

		m_mapGlobalUser.insert(std::make_pair(pRegisterUser->dwUserID, pGlobalUser));
	}

	//检测玩家是否公会成员
	/*if (pRegisterUser->dwParentUserID != 0)
    {
	    //查找玩家公会ID
	    CT_DWORD dwGroupID = CGroupMgr::get_instance().GetUserGroupID(pRegisterUser->dwUserID);
        if (dwGroupID != 0)
        {
            CGroupMgr::get_instance().SetGroupUserOnline(dwGroupID, pRegisterUser->dwUserID, true);
            pUserInfo->dwGroupID = dwGroupID;

        }
        else
        {
            //插入公会成员列表
            dwGroupID = CGroupMgr::get_instance().GetUserGroupID(pRegisterUser->dwParentUserID);
            if (dwGroupID != 0)
            {
                std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(dwGroupID);
                if (pGroup)
                {
                    tagGroupUserInfo* pParentGroupUser = (*pGroup)->GetGroupUserInfo(pRegisterUser->dwParentUserID);
                    if (pParentGroupUser)
                    {
                        tagGroupUserInfo userInfo;
                        userInfo.dwUserID = pRegisterUser->dwUserID;
                        userInfo.dwParentUserId = pRegisterUser->dwParentUserID;
                        userInfo.llTotalIncome = 0;
                        userInfo.dwRecharge = 0;
                        userInfo.cbSex = pRegisterUser->cbSex;
                        userInfo.cbHeadId = pRegisterUser->cbHeadID;
                        userInfo.cbVip2 = pRegisterUser->cbVipLevel2;
                        userInfo.cbInComeRate = (CT_BYTE)pParentGroupUser->cbInComeRate*0.3;
                        userInfo.cbMemLevel = pParentGroupUser->cbMemLevel + 1;
                        userInfo.bOnline = true;
                        CGroupMgr::get_instance().AddGroupUser(dwGroupID, &userInfo);
                        pUserInfo->dwGroupID = dwGroupID;
                    }
                }
            }
        }
    }*/
}

CT_VOID	CUserMgr::UpdateUserGsInfo(CT_DWORD dwUserID, CT_DWORD dwServerID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		tagGlobalUserInfo* pUserInfo = it->second->GetUserInfo();
		if (pUserInfo)
		{
			pUserInfo->dwGameServerID = dwServerID;
			pUserInfo->dwLastHeartBeatTime = Utility::GetTime();
			//pUserInfo->bOnline = true;
		}
	}
	else
	{
		std::shared_ptr<CGlobalUser> pGlobalUser = std::make_shared<CGlobalUser>();
		tagGlobalUserInfo userInfo;
		userInfo.dwGameServerID = dwServerID;
		userInfo.dwLastHeartBeatTime = Utility::GetTime();
		pGlobalUser->SetUserInfo(&userInfo);
		//加载任务
		LoadUserTaskInfo(dwUserID, pGlobalUser);
		m_mapGlobalUser.insert(std::make_pair(dwUserID, pGlobalUser));
		//LOG(WARNING) << "insert the game user is not exists?  user id: " << dwUserID;
	}
	if (dwServerID != 0)
	{
		CActivityMgr::get_instance().BroadcastRealTimeRankInfo(dwUserID);
		CRedPacketMgr::get_instance().SendRedPacketStartToClient(dwUserID);
	}

	//玩家进入游戏房间，检测是否进入了财神降临
	/*CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServer2(dwServerID);
	if (pGameServerInfo)
	{
		MSG_G2CS_PlayGame playGame;
		playGame.dwUserID = dwUserID;
		playGame.wGameID = pGameServerInfo->wGameID;
		playGame.wKindID = pGameServerInfo->wKindID;
		playGame.wRoomKindID = pGameServerInfo->wRoomKindID;
		playGame.dwJettonScore = 0;
		playGame.iWinScore = 0;
		playGame.dwRevenue = 0;
		CActivityMgr::get_instance().UserCheckDoingWealthGodComing(&playGame);
	}*/
}

CT_VOID	CUserMgr::DeleteUser(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		m_mapGlobalUser.erase(it);
		return;
	}

	LOG(WARNING) << "delete user is not exists? user id: " << dwUserID;
}

CT_VOID	CUserMgr::SetUserOnline(CT_DWORD dwUserID, CT_BOOL bOnline)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		it->second->SetOnline(bOnline);

		//玩家离线，设置工会离线
		CT_DWORD dwGroupID = it->second->GetGroupID();
		if (dwGroupID != 0)
        {
		    CGroupMgr::get_instance().SetGroupUserOnline(dwGroupID, dwUserID, bOnline);
        }
	}
}

CT_BOOL	CUserMgr::IsUserOnline(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it == m_mapGlobalUser.end())
	{
		return false;
	}

	return it->second->IsOnline();
}

acl::aio_socket_stream* CUserMgr::FindUserProxySocksPtr(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);

	if (it != m_mapGlobalUser.end())
	{
		tagGlobalUserInfo* pUserInfo = it->second->GetUserInfo();
		if (pUserInfo && pUserInfo->bOnline)
		{
			return CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
		}
		else
		{
			return NULL;
		}
	}

	return NULL;
}

tagGlobalUserInfo* CUserMgr::GetUserInfo(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		return it->second->GetUserInfo();
	}
	return NULL;
}

CGlobalUser* CUserMgr::GetGlobalUser(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		return it->second.get();
	}
	return NULL;
}

CUserTask* CUserMgr::GetUserTaskPtr(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		return it->second->GetUserTask().get();
	}
	return NULL;
}

CT_DWORD CUserMgr::GetUserChannelID(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		return it->second->GetChannelID();
	}
	return 0;
}


CT_DWORD CUserMgr::GetUserClientChannelID(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		return it->second->GetClientChannelID();
	}
	return 0;
}

CT_VOID CUserMgr::SetUserHeartBeatTime(CT_DWORD dwUserID, CT_DWORD dwTime)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		it->second->SetLastHeartBeatTime(dwTime);
	}
}

CT_DWORD CUserMgr::GetUserHeartBeatTime(CT_DWORD dwUserID)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it != m_mapGlobalUser.end())
	{
		return it->second->GetLastHeartBeatTime();
	}

	return 0;
}

CT_LONGLONG CUserMgr::GetUserScore(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		return pUserInfo->llScore;
	}

	return 0;
}

CT_LONGLONG CUserMgr::GetUserBankScore(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		return pUserInfo->llBankScore;
	}

	return 0;
}

CT_DWORD CUserMgr::GetUserRecharge(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		return pUserInfo->dwTotalRecharge;
	}

	return 0;
}

CT_VOID CUserMgr::SetUserRecharge(CT_DWORD dwUserID, CT_DWORD dwRecharge)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		pUserInfo->dwTotalRecharge = dwRecharge;
	}
}

CT_BYTE	CUserMgr::GetUserShowExchange(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		return pUserInfo->cbShowExchange;
	}

	return 0;
}

CT_VOID	CUserMgr::AddUserGem(CT_DWORD dwUserID, CT_INT32 nAddGem)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_pRedis->clear();
	long long llGemResult = 0;
	if (m_pRedis->exists(key) == true)
	{
		m_pRedis->clear();
		if (m_pRedis->hincrby(key, "gem", nAddGem, &llGemResult) == false)
		{
			LOG(WARNING) << "add user redis gem fail, user id: " << dwUserID << ", add gem: " << nAddGem;
		}

	}

	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		//更新内存
		pUserInfo->dwGem += nAddGem;

		//更新游戏服
		if (pUserInfo->dwGameServerID != 0)
		{
			acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(pUserInfo->dwGameServerID);
			if (pSocket)
			{
				MSG_C2GS_User_Recharge recharge;
				recharge.dwUserID = dwUserID;
				recharge.llRecharge = nAddGem;
				recharge.cbType = RECHARGE_GEM;
				CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_USER_RECHARGE, &recharge, sizeof(recharge));
			}
		}

		//更新客户端
		acl::aio_socket_stream* pClientSock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
		if (pClientSock)
		{
			MSG_SC_GameUserGem updateGem;
			updateGem.dwMainID = MSG_FRAME_MAIN;
			updateGem.dwSubID = SUB_S2C_USER_GEM;
			updateGem.dwUserID = dwUserID;
			updateGem.dwGem = (CT_DWORD)llGemResult;
			updateGem.dwValue2 = dwUserID;
			CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateGem, sizeof(updateGem));
		}
	}

	//更新数据库
	MSG_UpdateUser_Gem addGemToDB;
	addGemToDB.dwUserID = dwUserID;
	addGemToDB.nAddGem = nAddGem;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_ADD_USER_GEM, &addGemToDB, sizeof(addGemToDB));
}

CT_VOID	CUserMgr::AddUserScore(CT_DWORD dwUserID, CT_LONGLONG llAddScore)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	long long llScoreResult = 0;
	m_pRedis->clear();
	if (m_pRedis->exists(key) == true)
	{
		m_pRedis->clear();
		if (m_pRedis->hincrby(key, "score", llAddScore, &llScoreResult) == false)
		{
			LOG(WARNING) << "add user redis score fail, user id: " << dwUserID << ", add score: " << llAddScore;
		}
	}

	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		//更新内存
		pUserInfo->llScore += llAddScore;

		//更新游戏服
		if (pUserInfo->dwGameServerID != 0)
		{
			acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(pUserInfo->dwGameServerID);
			if (pSocket)
			{
				MSG_C2GS_User_Recharge recharge;
				recharge.dwUserID = dwUserID;
				recharge.llRecharge = llAddScore;
				recharge.cbType = RECHARGE_SCORE;
				CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_USER_RECHARGE, &recharge, sizeof(recharge));
			}
		}
		
		//更新客户端
		acl::aio_socket_stream* pClientSock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
		if (pClientSock)
		{
			MSG_SC_UserScoreInfo updateScore;
			updateScore.dwMainID = MSG_FRAME_MAIN;
			updateScore.dwSubID = SUB_S2C_USER_SCORE;
			updateScore.dwUserID = dwUserID;
			updateScore.wTableID = INVALID_CHAIR;
			updateScore.wChairID = INVALID_CHAIR;
			updateScore.dScore = llScoreResult*0.01f;
			updateScore.dwValue2 = dwUserID;
			CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateScore, sizeof(updateScore));

			//下发不能领取救济金按钮（有点简单粗暴）
			tagBenefitReward* pBenefit = CConfigOption::get_instance().GetBenefitReward(pUserInfo->cbVip2);
			if (pBenefit && llScoreResult >= pBenefit->dwLessScore)
			{
				MSG_SC_HasBenefit hasBenefit;
				hasBenefit.dwMainID = MSG_FRAME_MAIN;
				hasBenefit.dwSubID = SUB_S2C_HAS_BENEFIT;
				hasBenefit.dwValue2 = dwUserID;
				hasBenefit.cbState = 0;
				CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &hasBenefit, sizeof(hasBenefit));
			}
		}
	}

	//更新数据库
	MSG_UpdateUser_Score addScoreToDB;
	addScoreToDB.dwUserID = dwUserID;
	addScoreToDB.llAddScore = llAddScore;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_ADD_USER_SCORE, &addScoreToDB, sizeof(addScoreToDB));
}

CT_VOID	CUserMgr::AddUserGemEx(CT_DWORD dwUserID, CT_INT32 nAddGem, CT_BOOL bUpdateClient/* = false*/)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		//只更新内存
		pUserInfo->dwGem += nAddGem;

		if (bUpdateClient)
		{
			//更新游戏服
			if (pUserInfo->dwGameServerID != 0)
			{
				acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(pUserInfo->dwGameServerID);
				if (pSocket)
				{
					MSG_C2GS_User_Recharge recharge;
					recharge.dwUserID = dwUserID;
					recharge.llRecharge = nAddGem;
					recharge.cbType = 2;
					CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_USER_RECHARGE, &recharge, sizeof(recharge));
				}
			}
		
			//同步到客户端
			acl::aio_socket_stream* pClientSock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
			if (pClientSock)
			{
				MSG_SC_GameUserGem updateScore;
				updateScore.dwMainID = MSG_FRAME_MAIN;
				updateScore.dwSubID = SUB_S2C_USER_GEM;
				updateScore.dwUserID = dwUserID;
				updateScore.dwGem = pUserInfo->dwGem;
				updateScore.dwValue2 = dwUserID;
				CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateScore, sizeof(updateScore));
			}
		}
		
	}
}

CT_VOID	CUserMgr::AddUserScoreEx(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_BOOL bUpdateClient/* = false*/)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		//只更新内存
		pUserInfo->llScore += llAddScore;

		if (bUpdateClient)
		{
			//更新游戏服
			if (pUserInfo->dwGameServerID != 0)
			{
				acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(pUserInfo->dwGameServerID);
				if (pSocket)
				{
					MSG_C2GS_User_Recharge recharge;
					recharge.dwUserID = dwUserID;
					recharge.llRecharge = llAddScore;
					recharge.cbType = 1;
					CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_USER_RECHARGE, &recharge, sizeof(recharge));
				}
			}
			else //如果不在游戏服中，则直接同步到客户端
			{
				//更新客户端
				acl::aio_socket_stream* pClientSock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
				if (pClientSock)
				{
					MSG_SC_UserScoreInfo updateScore;
					updateScore.dwMainID = MSG_FRAME_MAIN;
					updateScore.dwSubID = SUB_S2C_USER_SCORE;
					updateScore.dwUserID = dwUserID;
					updateScore.wTableID = INVALID_CHAIR;
					updateScore.wChairID = INVALID_CHAIR;
					updateScore.dScore = pUserInfo->llScore*0.01f;
					updateScore.dwValue2 = dwUserID;
					CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateScore, sizeof(updateScore));
				}
			}
		}
	}
}

CT_VOID	CUserMgr::AddUserBankScoreEx(CT_DWORD dwUserID, CT_LONGLONG llAddBankScore, CT_BOOL bUpdateClient /*= false*/)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		//只更新内存
		pUserInfo->llBankScore += llAddBankScore;

		if (bUpdateClient)
		{
			//更新客户端
			acl::aio_socket_stream* pClientSock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
			if (pClientSock)
			{
				MSG_SC_Update_BankScore bankScore;
				bankScore.dwMainID = MSG_LOGIN_MAIN;
				bankScore.dwSubID = SUB_SC_UPDATE_BANK_SCORE;
				bankScore.dBankScore = pUserInfo->llBankScore*0.01f;
				bankScore.dwValue2 = dwUserID;
				CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &bankScore, sizeof(bankScore));
			}
		}
	}
}

CT_BYTE	CUserMgr::GetUserPlatformId(CT_DWORD dwUserID)
{
    tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
    if (pUserInfo)
    {
        return pUserInfo->cbPlatformId;
    }

	return 0;
}

CT_VOID	CUserMgr::InsertUserScoreChangeRecord(CT_DWORD dwUserID, CT_LONGLONG llSourceBankScore, CT_LONGLONG llSourceScore, \
	CT_LONGLONG llAddBankScore, CT_LONGLONG llAddScore, enScoreChangeType eType)
{
	MSG_Insert_ScoreChange_Record scoreChangeRecord;
	scoreChangeRecord.dwUserID = dwUserID;
	scoreChangeRecord.llSourceScore = llSourceScore;
	scoreChangeRecord.llSourceBankScore = llSourceBankScore;
	scoreChangeRecord.llChangeScore = llAddScore;
	scoreChangeRecord.llChangeBankScore = llAddBankScore;
	scoreChangeRecord.enType = eType;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_INSERT_SCORE_CHANGE, &scoreChangeRecord, sizeof(scoreChangeRecord));
}

CT_VOID	CUserMgr::UpdateUserVipLevel(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_DWORD dwTotalRecharge,  CT_BYTE cbVipLevel, CT_BYTE cbVipLevel2)
{
	//更新缓存
	acl::string key;
	key.format("account_%u", dwUserID);

	m_pRedis->clear();
	if (m_pRedis->exists(key) == true)
	{
		m_pRedis->clear();
		acl::string strVip, strVip2;
		strVip.format("%d", cbVipLevel);
		strVip2.format("%d", cbVipLevel2);
		acl::string strTotalRecharge;
		strTotalRecharge.format("%u", dwTotalRecharge);
		std::map<acl::string, acl::string> mapVip;
		mapVip["vip"] = strVip;
		mapVip["vip2"] = strVip2;
		mapVip["pay"] = strTotalRecharge;
		if (m_pRedis->hmset(key, mapVip) == false)
		{
			LOG(WARNING) << "update user redis recharge info fail, user id: " << dwUserID << ", pay: " \
				<< dwTotalRecharge << ", vip:" << (int)cbVipLevel << ", vip2:" << (int)cbVipLevel2;
		}
	}

	//更新内存
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		//VIP等级更新
		pUserInfo->cbVip = cbVipLevel;
		pUserInfo->cbVip2 = cbVipLevel2;

		//判断公会数据
        if (pUserInfo->dwGroupID != 0)
        {
            CGroupMgr::get_instance().OnUserRecharge(dwUserID, pUserInfo->dwGroupID, llAddScore, dwTotalRecharge, pUserInfo->cbVip2);
        }
		if (pUserInfo->dwGameServerID != 0)
		{
			acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(pUserInfo->dwGameServerID);
			if (pSocket)
			{
				//更新游戏服
				MSG_C2GS_Update_UserVip userVip;
				userVip.dwUserID = dwUserID;
				userVip.cbVipLevel = cbVipLevel;
				userVip.cbVipLevle2 = cbVipLevel2;
			
				CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_UPDATE_USER_VIP, &userVip, sizeof(userVip));
			}
		}

		//更新客户端
		acl::aio_socket_stream* pClientSock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
		if (pClientSock)
		{
			MSG_SC_UserVipLevel updateVip;
			updateVip.dwMainID = MSG_FRAME_MAIN;
			updateVip.dwSubID = SUB_S2C_UPDATE_RECHARGE_VIP;
			updateVip.dwUserID = dwUserID;
			updateVip.dwTotalRecharge = dwTotalRecharge;
			updateVip.cbVipLevel = cbVipLevel;
			updateVip.cbVipLevel2 = cbVipLevel2;
			CConfigOption::get_instance().GetNextVip2LevelInfo(updateVip.cbVipLevel2, updateVip.cbNextVipLevel2, updateVip.dwNextLevel2NeedRecharge);
			updateVip.dwValue2 = dwUserID;
			CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateVip, sizeof(updateVip));
		}

		//检测玩家是否可以显示兑换按钮
		CT_BYTE cbShowExchange = CUserMgr::get_instance().GetUserShowExchange(dwUserID);
		if (cbShowExchange == 0)
		{
			CT_DWORD dwChannelID = CUserMgr::get_instance().GetUserChannelID(dwUserID);
			CT_DWORD dwShowExchangeCond = CConfigOption::get_instance().GetChannelShowExchangeCond(dwChannelID);
			if (dwTotalRecharge >= dwShowExchangeCond)
			{
				SetUserShowExchange(dwUserID, 1);
				//更新客户端
				if (pClientSock)
				{
					MSG_SC_ShowExchange showExchange;
					showExchange.dwMainID = MSG_FRAME_MAIN;
					showExchange.dwSubID = SUB_S2C_SHOW_EXCHANGE;
					showExchange.dwValue2 = dwUserID;
					showExchange.cbShowExchange = 1;
					CNetModule::getSingleton().Send(pClientSock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &showExchange, sizeof(showExchange));
				}
			}
		}
	}
}

CT_VOID	CUserMgr::LoadUserScoreInfo(CT_DWORD dwUserID, UserScoreData& scoreInfo)
{
	acl::string strKey;
	strKey.format("scoreinfo_%u", dwUserID);
	std::map<acl::string, acl::string> result;

	m_pRedis->clear();
	if (m_pRedis->hgetall(strKey.c_str(), result) == false)
	{
		LOG(WARNING) << "load user score info failed, userid: " << dwUserID;
		return;
	}

	scoreInfo.dwRevenue = atoi(result["revenue"].c_str());
	scoreInfo.dwWinCount = atoi(result["winCount"].c_str());
	scoreInfo.dwLostCount = atoi(result["lostCount"].c_str());
	scoreInfo.dwDrawCount = atoi(result["drawCount"].c_str());
	scoreInfo.dwFleeCount = atoi(result["fleeCount"].c_str());
	scoreInfo.dwPlayTime = atoi(result["playTime"].c_str());
}

CT_VOID	CUserMgr::UpdateUserScoreInfo(CT_DWORD dwUserID, MSG_Update_ScoreInfo* pAddScoreInfo)
{
	auto it = m_mapGlobalUser.find(dwUserID);
	if (it == m_mapGlobalUser.end())
	{
		LOG(WARNING) << "update user score info fail, can not find user in center! user id:" << dwUserID;
		return;
	}

	UserScoreData*	pUserScoreData = it->second->GetUserScoreInfo();
	if (pUserScoreData)
	{
		// 更新内存
		pUserScoreData->dwRevenue += pAddScoreInfo->dwAddRevenue;
		pUserScoreData->dwWinCount += pAddScoreInfo->dwAddWinCount;
		pUserScoreData->dwLostCount += pAddScoreInfo->dwAddLostCount;
		pUserScoreData->dwDrawCount += pAddScoreInfo->dwAddDrawCount;
		pUserScoreData->dwFleeCount += pAddScoreInfo->dwAddFleeCount;
		pUserScoreData->dwPlayTime += pAddScoreInfo->dwAddPlayTime;

		//更新缓存
		acl::string strKey;
		strKey.format("scoreinfo_%u", dwUserID);

		std::map<acl::string, acl::string> scoreInfo;
		acl::string strRevenue, strWinCount, strLostCount, strDrawCount, strFleeCount, strPlayTime;

		strRevenue.format("%u", pUserScoreData->dwRevenue);
		strWinCount.format("%u", pUserScoreData->dwWinCount);
		strLostCount.format("%u", pUserScoreData->dwLostCount);
		strDrawCount.format("%u", pUserScoreData->dwDrawCount);
		strFleeCount.format("%u", pUserScoreData->dwFleeCount);
		strPlayTime.format("%u", pUserScoreData->dwPlayTime);

		scoreInfo["revenue"] = strRevenue;
		scoreInfo["winCount"] = strWinCount;
		scoreInfo["lostCount"] = strLostCount;
		scoreInfo["drawCount"] = strDrawCount;
		scoreInfo["fleeCount"] = strFleeCount;
		scoreInfo["playTime"] = strPlayTime;

		m_pRedis->clear();
		if (m_pRedis->hgetall(strKey.c_str(), scoreInfo) == false)
		{
			LOG(WARNING) << "set user score info failed, userid: " << dwUserID;
			return;
		}
	}
}

CT_VOID	CUserMgr::LoadUserTaskInfo(CT_DWORD dwUserID, std::shared_ptr<CGlobalUser>& pGlobalUser)
{
	acl::string strKey;
	strKey.format("task_%u", dwUserID);
	std::map<acl::string, acl::string> mapTaskInfo;

	m_pRedis->clear();
	if (m_pRedis->hgetall(strKey.c_str(), mapTaskInfo) == false)
	{
		return;
	}

	std::unique_ptr<CUserTask> pUserTask(new CUserTask);
	pUserTask->Init(pGlobalUser.get(), m_pRedis);
	const MapTaskModel& mapTaskModel = CUserTaskModel::get_instance().GetAllTaskModel();

	if (mapTaskInfo.empty())
	{
		for (auto& it : mapTaskModel)
		{
			pUserTask->InsertNewTask(&it.second);
		}

		pGlobalUser->SetUserTask(pUserTask);
		return;
	}

	//TODO: 检测是否有新任务
	tagUserTask userTask;
	for (auto& itModel : mapTaskModel)
	{
		acl::string strTaskID;
		strTaskID.format("%d", itModel.first);
		auto it = mapTaskInfo.find(strTaskID);
		if (it != mapTaskInfo.end())
		{
			std::vector<acl::string>& taskData = it->second.split2(",");
			if (taskData.size() != 4)
			{
				continue;
			}
			userTask.wTaskID = atoi(taskData[0].c_str());
			userTask.dwCurrProgress = atoi(taskData[1].c_str());
			userTask.dwReceiveTime = atoi(taskData[2].c_str());
			userTask.bFinish = atoi(taskData[3].c_str()) > 0 ? true : false;
			pUserTask->InsertTask(userTask);

			//检测任务是否已经过期
			tagTaskModel* pTaskModel = CUserTaskModel::get_instance().GetTaskModel(userTask.wTaskID);
			if (pTaskModel)
			{
				//if (pTaskModel->wTaskType == TASK_TYPE_NEWBIE)
				//{
				//	continue;
				//}

				CT_INT64 nTimeNow = time(NULL);
				CT_BOOL bAcrossDay = isAcrossTheDay(userTask.dwReceiveTime, nTimeNow);
				if (bAcrossDay)
				{
					switch (pTaskModel->wTaskType)
					{
					case TASK_TYPE_DAILY:
						//case TASK_TYPE_LOGIN:
					{
						pUserTask->ResetTask(userTask.wTaskID);
					}
					break;
					//case TASK_TYPE_WEEKLY:
					//{
					//	CT_INT32 nWeek = diffTimeWeek(userTask.dwReceiveTime, nTimeNow);
					//	if (nWeek > 0)
					//	{
					//		pUserTask->ResetTask(userTask.wTaskID);
					//	}
					//}
					break;
					default:break;
					}
				}
			}
		}
		else
		{
			pUserTask->InsertNewTask(&itModel.second);
		}
	}

	pGlobalUser->SetUserTask(pUserTask);
}

CT_VOID	CUserMgr::SetUserShowExchange(CT_DWORD dwUserID, CT_BYTE cbShow)
{
	//更新缓存
	acl::string key;
	key.format("account_%u", dwUserID);

	m_pRedis->clear();
	acl::string strShowExchange;
	strShowExchange.format("%d", cbShow);
	if (m_pRedis->hset(key.c_str(), "exchange", strShowExchange.c_str()) == -1)
	{
		LOG(WARNING) << "update user show exchange fail, user id: " << dwUserID << ", show: " << (int)cbShow ;
	}

	//更新缓存
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo)
	{
		pUserInfo->cbShowExchange = cbShow;
	}

	//更新DB
	MSG_Update_ShowExchange showExchange;
	showExchange.dwUserID = dwUserID;
	showExchange.cbShowExchange = cbShow;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDAET_USER_SHOWEXCHANGE, &showExchange, sizeof(showExchange));
}

CT_VOID	 CUserMgr::RemoveOffLineUser()
{
	CT_DWORD dwTimeNow = Utility::GetTime();
	for (auto it = m_mapGlobalUser.begin(); it != m_mapGlobalUser.end();)
	{
		//离线超过三小时的玩家
		if (!it->second->IsOnline() && (dwTimeNow - it->second->GetOffLineTime()) > 7200)
		{
			//LOG(WARNING) << "delete user, userid : " << it->first;
			it = m_mapGlobalUser.erase(it);
		}
		else
		{
			++it;
		}
	}

	//LOG(WARNING) << "global user count: " << m_mapGlobalUser.size();
}

CT_VOID	CUserMgr::QueryOnlineUser(const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pDipSocket)
{
	if (wDataSize != sizeof(MSG_D2CS_Query_OnlineUser))
	{
		return;
	}

	MSG_D2CS_Query_OnlineUser* pQueryOnlineUser = (MSG_D2CS_Query_OnlineUser*)pData;

	std::map <CT_WORD, acl::string> mapUserID;
	for (auto& it : m_mapGlobalUser)
	{
		if (!it.second->IsOnline())
		{
			continue; 
		}

		if (pQueryOnlineUser->cbPlatformId != 255 && pQueryOnlineUser->cbPlatformId != it.second->GetPlatformId())
		{
			continue;
		}

		CT_DWORD dwServerID = it.second->GetUserServerID();
		acl::string& strUserID = mapUserID[dwServerID];

		strUserID += it.second->GetUserID();
		strUserID += ",";
	}
	//mapUserID[201] = "41,42,43,";

	acl::string strJson("");
	if (pQueryOnlineUser->cbType == 1)
	{
		//大厅数据(dwServerID 为0 说明是大厅玩家)
		acl::string& strUserID = mapUserID[0];
		strJson.format("{\"code\":%d,", 0);
		if (strUserID.empty())
		{
			strJson.format_append("\"data\":\"\"},");
		}
		else
		{
			//把userid list 的最后一个逗号去掉
			strUserID = strUserID.left(strUserID.length() - 1);
			strJson.format_append("\"data\":\"%s\"},", strUserID.c_str());
		}

		for (auto& it : mapUserID)
		{
			CT_DWORD dwServerID = it.first;
			if (dwServerID == 0)
				continue;
			//CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServerEx(dwServerID);
			//if (pGameServerInfo)
			//{
				strJson.format_append("{\"code\":%d,", dwServerID);
				acl::string strUserID = it.second.left(it.second.length() - 1);
				strJson.format_append("\"data\":\"%s\"},", strUserID.c_str());
			//}
		}
		//把最后一个逗号去掉
		strJson = strJson.left(strJson.length() - 1);
	}
	else if (pQueryOnlineUser->cbType == 2)
	{
		for (auto& it : mapUserID)
		{
			CT_DWORD dwServerID = it.first;
			CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServerEx(dwServerID);
			if (pGameServerInfo)
			{
				strJson.format_append("{\"code\":%d,", 0);
				strJson.format_append("\"message\":\"%s\",", pGameServerInfo->szServerName);
				acl::string strUserID = it.second.left(it.second.length() - 1);
				strJson.format_append("\"data\":\"%s\"},", strUserID.c_str());
			}
		}

		//把最后一个逗号去掉
		strJson = strJson.left(strJson.length() - 1);
	}

	MSG_CS2D_Query_OnlineUser onlineResult;
	onlineResult.uWebSock = pQueryOnlineUser->uWebSock;
	_snprintf_info(onlineResult.szJsonUserList, sizeof(onlineResult.szJsonUserList), "{\"code\":0, \"data\": [%s]}", strJson.c_str());
	CNetModule::getSingleton().Send(pDipSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_ONLINE_USER, &onlineResult, sizeof(onlineResult));
}

CT_VOID	CUserMgr::NotifyNewMail(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_D2CS_Notify_NewMail))
	{
		return;
	}

	MSG_D2CS_Notify_NewMail* pNewMail = (MSG_D2CS_Notify_NewMail*)pData;
	if (pNewMail == NULL)
	{
		return;
	}
	
	SendNewMailTip(pNewMail->dwUserID);
}

CT_VOID	CUserMgr::SendNewMailTip(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = GetUserInfo(dwUserID);
	if (pUserInfo && pUserInfo->bOnline)
	{
		acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
		if (pProxySock)
		{
			//通知客户端
			MSG_GameMsgDownHead msgHead;
			msgHead.dwMainID = MSG_FRAME_MAIN;
			msgHead.dwSubID = SUB_S2C_HAS_NEW_MAIL;
			msgHead.dwValue2 = dwUserID;
			CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &msgHead, sizeof(msgHead));
		}
	}
}

CT_VOID CUserMgr::SendUserMail(tagUserMail* pUserMail)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_SEND_MAIL, pUserMail,
                                    sizeof(tagUserMail));
}

CT_VOID	 CUserMgr::UserRecharge(CT_DWORD dwUserID, CT_LONGLONG llRecharge, CT_BYTE cbType)
{
	CGlobalUser* pGlobalUser = GetGlobalUser(dwUserID);
	if (pGlobalUser)
	{
		pGlobalUser->SetRechargeTips(llRecharge, cbType);

		if (pGlobalUser->IsOnline())
		{
			pGlobalUser->SendRechargeTips();
		}
	}
}