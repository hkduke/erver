#include "stdafx.h"
#include "GroupMgr.h"
#include "UserMgr.h"
#include "ServerMgr.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "NetModule.h"
#include <time.h>
#include <random>

extern CNetConnector *pNetDB;

CGroupMgr::CGroupMgr()
{
	m_pRedis = NULL;
    m_dwChatMsgId = 0;
    m_dwMaxGroupContribution = 0;
}

CGroupMgr::~CGroupMgr()
{
}

CT_VOID	CGroupMgr::SetRedisPtr(acl::redis* pRedis)
{
	m_pRedis = pRedis;
}

CT_VOID CGroupMgr::InsertGroupInfo(tagGroupInfo* pGroupInfo)
{
	auto it = m_mapGroup.find(pGroupInfo->dwGroupID);
	if (it != m_mapGroup.end())
	{
		LOG(WARNING) << "insert group repeat? group id: " << pGroupInfo->dwGroupID;
		return;
	}

	std::unique_ptr<CGroup> groupPtr(new CGroup);
	groupPtr->InsertGroupInfo(pGroupInfo);

	m_mapGroup.insert(std::make_pair(pGroupInfo->dwGroupID, std::move(groupPtr)));
}

CT_VOID CGroupMgr::InsertGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		LOG(WARNING) << "insert group user fail, can not find group info. group id: " << dwGroupID;
		return;
	}

    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(pGroupUser->dwUserID);
	if (pGroupUserInfo != NULL)
    {
	    LOG(WARNING) << "insert group user repeat? group id: " << dwGroupID << ", user id: " << pGroupUser->dwUserID;
        return;
    }

	it->second->InsertGroupUser(pGroupUser);
	InsertUserGroup(pGroupUser->dwUserID, dwGroupID);
}

CT_VOID  CGroupMgr::InsertGroupUserTodayRevenue(CT_DWORD dwGroupID, tagGroupUserRevenue* pGroupUserRevenue)
{
    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    MapGroupUserDateRevenue& mapGroupUserDateRevenue = itGroup->second->GetGroupUserDateRevenue();

    //计算时间值(格式:20190429)
    unsigned int nTodayInt = CalcDayInt(0);

    //记录当天的税收
    MapGroupUserRevenue& mapGroupUserRevenue = mapGroupUserDateRevenue[nTodayInt];
    auto itMapUserRevenue = mapGroupUserRevenue.find(pGroupUserRevenue->dwUserID);
    if (itMapUserRevenue != mapGroupUserRevenue.end())
    {
        itMapUserRevenue->second += pGroupUserRevenue->dwRevenue;
    }
    else
    {
        mapGroupUserRevenue.insert(std::make_pair(pGroupUserRevenue->dwUserID, pGroupUserRevenue->dwRevenue));
    }

    LOG(WARNING) << "group user id: " << pGroupUserRevenue->dwUserID << ", revenue: " << pGroupUserRevenue->dwRevenue;
}

CT_VOID CGroupMgr::InsertGroupUserDateDirIncome(CT_DWORD dwGroupID, tagGroupUserDateIncomeDetail* pGroupUserDateIncomeDetail)
{
    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    bool bInsertNew = true;
    MapGroupAllUserDateIncome& mapGroupAllUserDateDirIncome = itGroup->second->GetGroupUserDateDirIncome();
    MapGroupUserDateIncome& mapGroupUserDateDirIncome = mapGroupAllUserDateDirIncome[pGroupUserDateIncomeDetail->dwUserID];
    auto itGroupUserIncome = mapGroupUserDateDirIncome.find(pGroupUserDateIncomeDetail->dwDateInt);
    if (itGroupUserIncome != mapGroupUserDateDirIncome.end())
    {
        MapGroupUserIncome& mapGroupUserIncome = itGroupUserIncome->second;
        auto itIncome = mapGroupUserIncome.find(pGroupUserDateIncomeDetail->dwSubUserID);
        if (itIncome != mapGroupUserIncome.end())
        {
            std::unique_ptr<tagGroupUserDateIncome>& userIncomePtr = itIncome->second;
            userIncomePtr->dwOriginalIncome = pGroupUserDateIncomeDetail->dwOriginalIncome;
            userIncomePtr->wDeductRate = pGroupUserDateIncomeDetail->wDeductRate;
            userIncomePtr->dwDeductIncome = pGroupUserDateIncomeDetail->dwDeductIncome;
            bInsertNew = false;
        }

    }
    if (bInsertNew)
    {
        std::unique_ptr<tagGroupUserDateIncome> userIncomePtr(new tagGroupUserDateIncome);
        userIncomePtr->dwOriginalIncome = pGroupUserDateIncomeDetail->dwOriginalIncome;
        userIncomePtr->wDeductRate = pGroupUserDateIncomeDetail->wDeductRate;
        userIncomePtr->dwDeductIncome = pGroupUserDateIncomeDetail->dwDeductIncome;;
        mapGroupUserDateDirIncome[pGroupUserDateIncomeDetail->dwDateInt].insert(std::make_pair(pGroupUserDateIncomeDetail->dwSubUserID, std::move(userIncomePtr)));
    }
}

CT_VOID CGroupMgr::InsertGroupUserDateSubIncome(CT_DWORD dwGroupID, tagGroupUserDateIncomeDetail* pGroupUserDateIncomeDetail)
{
    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    bool bInsertNew = true;
    MapGroupAllUserDateIncome& mapGroupAllUserDateSubIncome = itGroup->second->GetGroupUserDateSubIncome();
    MapGroupUserDateIncome& mapGroupUserDateSubIncome = mapGroupAllUserDateSubIncome[pGroupUserDateIncomeDetail->dwUserID];
    auto itGroupUserIncome = mapGroupUserDateSubIncome.find(pGroupUserDateIncomeDetail->dwDateInt);
    if (itGroupUserIncome != mapGroupUserDateSubIncome.end())
    {
        MapGroupUserIncome& mapGroupUserIncome = itGroupUserIncome->second;
        auto itIncome = mapGroupUserIncome.find(pGroupUserDateIncomeDetail->dwSubUserID);
        if (itIncome != mapGroupUserIncome.end())
        {
            std::unique_ptr<tagGroupUserDateIncome>& userIncomePtr = itIncome->second;
            userIncomePtr->dwOriginalIncome = pGroupUserDateIncomeDetail->dwOriginalIncome;
            userIncomePtr->wDeductRate = pGroupUserDateIncomeDetail->wDeductRate;
            userIncomePtr->dwDeductIncome = pGroupUserDateIncomeDetail->dwDeductIncome;
            bInsertNew = false;
        }

    }

    if (bInsertNew)
    {
        std::unique_ptr<tagGroupUserDateIncome> userIncomePtr(new tagGroupUserDateIncome);
        userIncomePtr->dwOriginalIncome = pGroupUserDateIncomeDetail->dwOriginalIncome;
        userIncomePtr->wDeductRate = pGroupUserDateIncomeDetail->wDeductRate;
        userIncomePtr->dwDeductIncome = pGroupUserDateIncomeDetail->dwDeductIncome;;
        mapGroupUserDateSubIncome[pGroupUserDateIncomeDetail->dwDateInt].insert(std::make_pair(pGroupUserDateIncomeDetail->dwSubUserID, std::move(userIncomePtr)));
    }
}

CT_VOID CGroupMgr::InsertGroupUserTotalIncome(CT_DWORD dwGroupID, tagGroupUserTotalIncome* pGroupUserDateIncomeDetail)
{
    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    MapGroupParentUserTotalIncome& mapGroupParentUserTotalIncome = itGroup->second->GetParentUserTotalIncome();
    MapGroupUserTotalIncome& mapGroupUserTotalIncome  = mapGroupParentUserTotalIncome[pGroupUserDateIncomeDetail->dwUserID];
    auto itTotalIncome = mapGroupUserTotalIncome.find(pGroupUserDateIncomeDetail->dwSubUserID);
    if (itTotalIncome != mapGroupUserTotalIncome.end())
    {
        itTotalIncome->second += pGroupUserDateIncomeDetail->llTotalIncome;
    }
    else
    {
        mapGroupUserTotalIncome.insert(std::make_pair(pGroupUserDateIncomeDetail->dwSubUserID, pGroupUserDateIncomeDetail->llTotalIncome));
    }
}

CT_VOID CGroupMgr::InsertGroupChatMsg(CT_DWORD dwGroupID, tagGroupChat* pGroupChat)
{
    auto it = m_mapGroup.find(dwGroupID);
    if (it == m_mapGroup.end())
    {
        LOG(WARNING) << "insert group msg fail, can not find group info. group id: " << dwGroupID;
        return;
    }

    it->second->InsertGroupChat(pGroupChat);
}

CT_VOID CGroupMgr::InsertGroupLevelConfig(tagGroupLevelCfg* pGroupLevel)
{
	auto it = m_mapGroupLevelCfg.find(pGroupLevel->cbLevel);
	if (it != m_mapGroupLevelCfg.end())
	{
		it->second = *pGroupLevel;
	}
	else
	{
		m_mapGroupLevelCfg.insert(std::make_pair(pGroupLevel->cbLevel, *pGroupLevel));
	}

	//设置群组最大贡献度
	if (pGroupLevel->dwMaxContribution > m_dwMaxGroupContribution)
    {
	    m_dwMaxGroupContribution = pGroupLevel->dwMaxContribution;
    }
}

/*CT_VOID	CGroupMgr::InsertDefaultPRoomConfig(tagDefaultRoomConfig* pDefaultConfig)
{
	std::shared_ptr<tagDefaultRoomConfig> defaultConfig = std::make_shared<tagDefaultRoomConfig>();
	defaultConfig->wGameID = pDefaultConfig->wGameID;
	defaultConfig->wKindID = pDefaultConfig->wKindID;
	_snprintf_info(defaultConfig->szConfig, sizeof(defaultConfig->szConfig), "%s", pDefaultConfig->szConfig);

	for (auto& it : m_vecRoomDefaultCfg)
	{
		if (it->wGameID == pDefaultConfig->wGameID && it->wKindID == pDefaultConfig->wKindID)
		{
			it.reset();
			it = defaultConfig;
			return;
		}
	}

	m_vecRoomDefaultCfg.push_back(defaultConfig);
}*/

/*CT_VOID CGroupMgr::InsertGroupPRoomConfig(tagGroupRoomConfig* pRoomConfig)
{
	auto it = m_mapGroup.find(pRoomConfig->dwGroupID);
	if (it == m_mapGroup.end())
	{
		LOG(WARNING) << "insert group proom cfg fail, can not find group info. group id: " << pRoomConfig->dwGroupID;
		return;
	}

	it->second->InsertPRoomCfg(pRoomConfig);
}*/

CT_VOID	CGroupMgr::InsertGroupPRoomClearing(MSG_G2CS_GroupPRoom_Clearing* pClearing)
{
    /*
	auto it = m_mapGroup.find(pClearing->dwGroupID);
	if (it == m_mapGroup.end())
	{
		return;
	}

	it->second->InsertPRoomClearingInfo(pClearing);

	std::list<std::unique_ptr<stPRoomClearingInfo>>& allClearingInfo = it->second->GetClearingInfo();
	std::unique_ptr<stPRoomClearingInfo>& lastClearingInfo = allClearingInfo.back();

	MSG_SC_GroupPRoom_Clearing clearingInfo;
	clearingInfo.cbCount = lastClearingInfo->cbCount;
	clearingInfo.dwGroupID = pClearing->dwGroupID;
	clearingInfo.dwRoomNum = pClearing->dwRoomNum;
	clearingInfo.wGameID = lastClearingInfo->wGameID;
	clearingInfo.wKindID = lastClearingInfo->wKindID;
	_snprintf_info(clearingInfo.szTime, sizeof(clearingInfo.szTime), "%s", lastClearingInfo->szTime);
	for (int i = 0; i < lastClearingInfo->cbCount; ++i)
	{
		clearingInfo.iScore[i] = lastClearingInfo->iScore[i];
		_snprintf_info(clearingInfo.szNickName[i], sizeof(clearingInfo.szNickName[i]), "%s", lastClearingInfo->szNickName[i]);
	}

	//通知群内所有玩家
	std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = it->second->GetAllGroupUserInfo();
	MSG_GameMsgDownHead headMsg;
	headMsg.dwMainID = MSG_GROUP_MAIN;
	headMsg.dwSubID = SUB_SC_GROUP_PROOM_CLEARING_INFO;

	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE];
	memcpy(szBuffer + sizeof(headMsg), &clearingInfo, sizeof(clearingInfo));
	for (auto& it : mapGroupUserInfo)
	{
		acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
		if (pProxySocket)
		{
			headMsg.dwValue2 = it.first;
			memcpy(szBuffer, &headMsg, sizeof(headMsg));
			CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, sizeof(headMsg) +  sizeof(clearingInfo));
		}
	}
     */
}

CT_VOID CGroupMgr::RemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		LOG(WARNING) << "remove group user fail, can not find group info. group id: " << dwGroupID;
		return;
	}

	it->second->RemoveGroupUser(dwUserID);
	RemoveUserGroup(dwUserID, dwGroupID);
}

CT_VOID CGroupMgr::InsertUserGroup(CT_DWORD dwUserID, CT_DWORD dwGroupID)
{
	auto it = m_mapUserGroup.find(dwUserID);
	if (it == m_mapUserGroup.end())
	{
		//std::set<CT_DWORD>& setGroupID = it->second;
		//setGroupID.insert(dwGroupID);
		m_mapUserGroup.insert(std::make_pair(dwUserID, dwGroupID));
	}

	//std::set<CT_DWORD> setGroupID;
	//setGroupID.insert(dwGroupID);
	//m_mapUserGroup.insert(std::make_pair(dwUserID, setGroupID));
}

CT_BOOL CGroupMgr::IsUserHasGroup(CT_DWORD dwUserID)
{
	auto it = m_mapUserGroup.find(dwUserID);
	if (it != m_mapUserGroup.end())
	{
		return true;
	}

	return false;
}

CT_DWORD  CGroupMgr::GetUserGroupID(CT_DWORD dwUserID)
{
    auto it = m_mapUserGroup.find(dwUserID);
    if (it != m_mapUserGroup.end())
    {
        return it->second;
    }

    return 0;
}


CT_VOID	CGroupMgr::RemoveUserGroup(CT_DWORD dwUserID, CT_DWORD dwGroupID)
{
	auto it = m_mapUserGroup.find(dwUserID);
	if (it != m_mapUserGroup.end())
	{
		/*std::set<CT_DWORD>& setGroupID = it->second;
		auto itGroup = setGroupID.find(dwGroupID);
		if (itGroup != setGroupID.end())
		{
			setGroupID.erase(itGroup);
		}

		if (setGroupID.empty())
		{
			m_mapUserGroup.erase(it);
		}*/
		m_mapUserGroup.erase(it);
	}
}

CT_VOID CGroupMgr::AddGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser)
{
	InsertGroupUser(dwGroupID, pGroupUser);

	MSG_CS2DB_Add_GroupUser addGroupUser;
	addGroupUser.dwGroupID = dwGroupID;
	addGroupUser.dwUserID = pGroupUser->dwUserID;
	addGroupUser.dwParentUserID = pGroupUser->dwParentUserId;
	addGroupUser.cbIncomeRate = pGroupUser->cbInComeRate;
    addGroupUser.wDeductRate = pGroupUser->wDeductRate;
    addGroupUser.wDeductTop = pGroupUser->wDeductTop;
    addGroupUser.wDeductIncre = pGroupUser->wDeductIncre;
    addGroupUser.dwDeductStart = pGroupUser->dwDeductStart;
	addGroupUser.llIncome = pGroupUser->llTotalIncome;
	addGroupUser.cbMemLevel = pGroupUser->cbMemLevel;

	//写入DB
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_ADD_GROUP_USER, &addGroupUser, sizeof(addGroupUser));
}

CT_VOID	CGroupMgr::InsertApplyGroupInfo(CT_DWORD dwGroupID, CT_DWORD dwApplyUser, tagGlobalUserInfo* pUserInfo)
{
	tagApplyAddGroupInfo applyGroup;
	applyGroup.dwApplyUserID = dwApplyUser;
	applyGroup.cbSex = pUserInfo->cbSex;
	applyGroup.cbHeadId = pUserInfo->cbHeadID;
	applyGroup.cbVip2 = pUserInfo->cbVip2;
	applyGroup.dwApplyTime = time(NULL);
	_snprintf_info(applyGroup.szNickName, sizeof(applyGroup.szNickName), "%s", pUserInfo->szNickName);
	//_snprintf_info(applyGroup.szHeadUrl, sizeof(applyGroup.szHeadUrl), "%s", pHeadUrl);
	//_snprintf_info(applyGroup.szApplyTime, sizeof(applyGroup.szApplyTime), "%s", Utility::GetTimeNowString("%Y-%m-%d %H:%M").c_str());

	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
        return;
	}

    it->second->InsertApplyGroupInfo(&applyGroup);

    //如果群主在线给群主发送申请请求
    std::unique_ptr<tagGroupInfo>& groupInfo = it->second->GetGroupInfo();

	tagGlobalUserInfo* pMasterUserInfo = CUserMgr::get_instance().GetUserInfo(groupInfo->dwMasterID);
	if (pMasterUserInfo != NULL)
	{
        acl::aio_socket_stream* pMasterProxySock = CServerMgr::get_instance().FindProxyServer(pMasterUserInfo->dwProxyServerID);
        if (pMasterProxySock != NULL)
        {
            CT_BYTE szBuffer[128] = { 0 };
            CT_DWORD dwSendSize = 0;

            MSG_GameMsgDownHead msgHead;
            msgHead.dwMainID = MSG_GROUP_MAIN;
            msgHead.dwSubID = SUB_SC_APPLY_ADD_GROUP;
            msgHead.dwValue2 = groupInfo->dwMasterID;
            memcpy(szBuffer, &msgHead, sizeof(msgHead));
            dwSendSize += sizeof(MSG_GameMsgDownHead);

            MSG_SC_ApplyGroup applyGroupToMaster;
            applyGroupToMaster.dwGroupID = dwGroupID;
            applyGroupToMaster.dwUserID = dwApplyUser;
            applyGroupToMaster.dwApplyTime = applyGroup.dwApplyTime;
            applyGroupToMaster.cbSex = applyGroup.cbSex;
            applyGroupToMaster.cbHeadId = applyGroup.cbHeadId;
            applyGroupToMaster.cbVip2 = applyGroup.cbVip2;
            memcpy(szBuffer + dwSendSize, &applyGroupToMaster, sizeof(applyGroupToMaster));
            dwSendSize += sizeof(MSG_SC_ApplyGroup);

            CNetModule::getSingleton().Send(pMasterProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, dwSendSize);
        }
	}

	//给副群主发送申请请求
	/*if (groupInfo->dwDeputyID1 != 0)
    {
	    tagGlobalUserInfo* pDeputyUserInfo = CUserMgr::get_instance().GetUserInfo(groupInfo->dwDeputyID1);
        if (pDeputyUserInfo != NULL)
        {
            acl::aio_socket_stream* pDeputyProxySock = CServerMgr::get_instance().FindProxyServer(pDeputyUserInfo->dwProxyServerID);
            if (pDeputyProxySock != NULL)
            {
                CT_BYTE szBuffer[128] = { 0 };
                CT_DWORD dwSendSize = 0;

                MSG_GameMsgDownHead msgHead;
                msgHead.dwMainID = MSG_GROUP_MAIN;
                msgHead.dwSubID = SUB_SC_APPLY_ADD_GROUP;
                msgHead.dwValue2 = groupInfo->dwDeputyID1;
                memcpy(szBuffer, &msgHead, sizeof(msgHead));
                dwSendSize += sizeof(MSG_GameMsgDownHead);

                MSG_SC_ApplyGroup applyGroupToMaster;
                applyGroupToMaster.dwGroupID = dwGroupID;
                applyGroupToMaster.dwUserID = dwApplyUser;
                applyGroupToMaster.dwApplyTime = applyGroup.dwApplyTime;
                applyGroupToMaster.cbSex = applyGroup.cbSex;
                applyGroupToMaster.cbHeadId = applyGroup.cbHeadId;
                applyGroupToMaster.cbVip2 = applyGroup.cbVip2;
                memcpy(szBuffer + dwSendSize, &applyGroupToMaster, sizeof(applyGroupToMaster));
                dwSendSize += sizeof(MSG_SC_ApplyGroup);

                CNetModule::getSingleton().Send(pDeputyProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, dwSendSize);
            }
        }

    }

	if (groupInfo->dwDeputyID2 != 0)
    {
        tagGlobalUserInfo* pDeputyUserInfo = CUserMgr::get_instance().GetUserInfo(groupInfo->dwDeputyID2);
        if (pDeputyUserInfo != NULL)
        {
            acl::aio_socket_stream* pDeputyProxySock = CServerMgr::get_instance().FindProxyServer(pDeputyUserInfo->dwProxyServerID);
            if (pDeputyProxySock != NULL)
            {
                CT_BYTE szBuffer[128] = { 0 };
                CT_DWORD dwSendSize = 0;

                MSG_GameMsgDownHead msgHead;
                msgHead.dwMainID = MSG_GROUP_MAIN;
                msgHead.dwSubID = SUB_SC_APPLY_ADD_GROUP;
                msgHead.dwValue2 = groupInfo->dwDeputyID2;
                memcpy(szBuffer, &msgHead, sizeof(msgHead));
                dwSendSize += sizeof(MSG_GameMsgDownHead);

                MSG_SC_ApplyGroup applyGroupToMaster;
                applyGroupToMaster.dwGroupID = dwGroupID;
                applyGroupToMaster.dwUserID = dwApplyUser;
                applyGroupToMaster.dwApplyTime = applyGroup.dwApplyTime;
                applyGroupToMaster.cbSex = applyGroup.cbSex;
                applyGroupToMaster.cbHeadId = applyGroup.cbHeadId;
                applyGroupToMaster.cbVip2 = applyGroup.cbVip2;
                memcpy(szBuffer + dwSendSize, &applyGroupToMaster, sizeof(applyGroupToMaster));
                dwSendSize += sizeof(MSG_SC_ApplyGroup);

                CNetModule::getSingleton().Send(pDeputyProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, dwSendSize);
            }
        }
    }*/
}

//CT_VOID	CGroupMgr::InsertApplyGroupInfo(CT_DWORD dwGroupID, tagApplyAddGroupInfo* pApplyInfo)
//{
//	auto it = m_mapGroup.find(dwGroupID);
//	if (it != m_mapGroup.end())
//	{
//		it->second->InsertApplyGroupInfo(pApplyInfo);
//	}
//}

CT_DWORD CGroupMgr::GetGroupMasterID(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->GetGroupMasterID();
	}
	return 0;
}

CT_VOID	CGroupMgr::OperateApplyAddGroup(CT_DWORD dwGroupID, CT_DWORD dwApplyUserID, CT_DWORD dwMasterID, CT_BYTE cbOperateCode)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		it->second->OperateApplyAddGroup(dwGroupID, dwApplyUserID, dwMasterID, cbOperateCode);
	}
}


CT_VOID	CGroupMgr::QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		it->second->QuitGroup(dwGroupID, dwUserID);
	}
}

CT_VOID	CGroupMgr::TickOutGroupUser(CT_DWORD dwGroupID, CT_DWORD dwMasterID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		it->second->TickOutGroupUser(dwGroupID, dwMasterID, dwUserID);
	}
}

CT_VOID	CGroupMgr::RemoveGroup(CT_DWORD dwGroupID, CT_DWORD dwMasterID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		//判断群是否有房间
		/*
		auto itRoom = m_mapRoomNum.find(dwGroupID);
		if (itRoom != m_mapRoomNum.end())
		{
			SendGroupErrorMsg(dwMasterID, GROUP_HAS_PROOM);
			return;
		}
		*/

		//到DB删除
		MSG_CS2DB_Remove_Group removeGroup;
		removeGroup.dwGroupID = dwGroupID;
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_DEL_GROUP, &removeGroup, sizeof(removeGroup));

		//通知群内所有玩家
		std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = it->second->GetAllGroupUserInfo();
		MSG_SC_Dismiss_Group dismissGroup;
		dismissGroup.dwMainID = MSG_GROUP_MAIN;
		dismissGroup.dwSubID = SUB_SC_DISMISS_GROUP;
		dismissGroup.dwGroupID = dwGroupID;

		for (auto& it : mapGroupUserInfo)
		{
			RemoveUserGroup(it.first, dwGroupID);
			acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pProxySocket)
			{
				dismissGroup.dwValue2 = it.first;
				CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &dismissGroup, sizeof(dismissGroup));
			}
		}

		SendAllGameServerRemoveGroup(dwGroupID);
		m_mapGroup.erase(it);
	}
}


CT_BOOL	CGroupMgr::IsGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->IsGroupUser(dwUserID);
	}
	return false;
}

CT_BOOL	CGroupMgr::IsApplyAddGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->IsApplyAddGroup(dwUserID);
	}
	return false;
}

CT_BOOL	CGroupMgr::HasGroup(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return true;
	}
	return false;
}

CT_DWORD CGroupMgr::GetGroupUserCount(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->GetGroupUserCount();;
	}
	return false;
}

CT_VOID	CGroupMgr::ModifyGroupInfo(MSG_CS_Modify_GroupInfo* pModifyGroupInfo)
{
	auto it = m_mapGroup.find(pModifyGroupInfo->dwGroupID);
	if (it == m_mapGroup.end())
	{
		//返回错误
		SendGroupErrorMsg(pModifyGroupInfo->dwMasterID, NOT_FOUND_GROUP);
		return;
	}

	bool bIsMaster = false;

	std::unique_ptr<tagGroupInfo>& pGroupInfo = it->second->GetGroupInfo();
	if (pGroupInfo->dwMasterID == pModifyGroupInfo->dwMasterID)
	{
		bIsMaster = true;
	}
	/*else if (pGroupInfo->dwDeputyID1 != 0 && pGroupInfo->dwDeputyID1 == pModifyGroupInfo->dwMasterID)
    {
	    bIsMaster = true;
    }
    else if (pGroupInfo->dwDeputyID2 != 0 && pGroupInfo->dwDeputyID2 == pModifyGroupInfo->dwMasterID)
    {
        bIsMaster = true;
    }*/

	if (!bIsMaster)
    {
        SendGroupErrorMsg(pModifyGroupInfo->dwMasterID, PERMISSION_DENIED);
        return;
    }

	//修改群公告或者群名称
	_snprintf_info(pGroupInfo->szGroupName, sizeof(pGroupInfo->szGroupName), "%s", pModifyGroupInfo->szGroupName);
	_snprintf_info(pGroupInfo->szNotice, sizeof(pGroupInfo->szNotice), "%s", pModifyGroupInfo->szNotice);

	//到DB修改群公告和群名称
	MSG_CS2DB_Modify_GroupInfo modifyGroupInfo;
	modifyGroupInfo.dwGroupID = pGroupInfo->dwGroupID;
	_snprintf_info(modifyGroupInfo.szGroupName, sizeof(modifyGroupInfo.szGroupName), "%s", pModifyGroupInfo->szGroupName);
	_snprintf_info(modifyGroupInfo.szNotice, sizeof(modifyGroupInfo.szNotice), "%s", pModifyGroupInfo->szNotice);;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_MODIFY_GROUP_INFO, &modifyGroupInfo, sizeof(modifyGroupInfo));

	//广播给所有群用户
	std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = it->second->GetAllGroupUserInfo();
	MSG_SC_Modify_GroupInfo groupInfoToClient;
	groupInfoToClient.dwMainID = MSG_GROUP_MAIN;
	groupInfoToClient.dwSubID = SUB_SC_MODIFY_GROUP_INFO;
	_snprintf_info(groupInfoToClient.szGroupName, sizeof(groupInfoToClient.szGroupName), "%s", pModifyGroupInfo->szGroupName);
	_snprintf_info(groupInfoToClient.szNotice, sizeof(groupInfoToClient.szNotice), "%s", pModifyGroupInfo->szNotice);

	for (auto& it : mapGroupUserInfo)
	{
		acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
		if (pProxySocket)
		{
			groupInfoToClient.dwValue2 = it.first;
			CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupInfoToClient, sizeof(groupInfoToClient));
		}
	}
}

CT_VOID CGroupMgr::ModifyGroupName(MSG_CS_Modify_GroupName* pGroupName)
{
    auto itGroupID = m_mapUserGroup.find(pGroupName->dwMasterID);
    if (itGroupID == m_mapUserGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupName->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    auto itGroup = m_mapGroup.find(itGroupID->second);
    if (itGroup == m_mapGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupName->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    std::unique_ptr<tagGroupInfo>& pGroupInfo = itGroup->second->GetGroupInfo();
    if (pGroupInfo->dwMasterID != pGroupName->dwMasterID)
    {
        SendGroupErrorMsg(pGroupName->dwMasterID, PERMISSION_DENIED);
        return;
    }

    //修改群名称
    _snprintf_info(pGroupInfo->szGroupName, sizeof(pGroupInfo->szGroupName), "%s", pGroupName->szGroupName);

    //到DB修改群公告和群名称
    UpdateGroupInfoToDB(pGroupInfo);

    //广播给所有群用户
    std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = itGroup->second->GetAllGroupUserInfo();
    MSG_SC_Modify_GroupInfo groupInfoToClient;
    groupInfoToClient.dwMainID = MSG_GROUP_MAIN;
    groupInfoToClient.dwSubID = SUB_SC_MODIFY_GROUP_INFO;
    groupInfoToClient.cbLevel = pGroupInfo->cbLevel;
    groupInfoToClient.cbIcon = pGroupInfo->cbIcon;
    groupInfoToClient.cbSettleDays = pGroupInfo->cbSettleDays;
    //groupInfoToClient.cbIncomeRate = pGroupInfo->cbIncomeRate;
    groupInfoToClient.dwNextContribution = GetLevelMaxContribution(pGroupInfo->cbLevel);
    _snprintf_info(groupInfoToClient.szGroupName, sizeof(groupInfoToClient.szGroupName), "%s", pGroupInfo->szGroupName);
    _snprintf_info(groupInfoToClient.szNotice, sizeof(groupInfoToClient.szNotice), "%s", pGroupInfo->szNotice);

    for (auto& it : mapGroupUserInfo)
    {
        acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
        if (pProxySocket)
        {
            groupInfoToClient.dwValue2 = it.first;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupInfoToClient, sizeof(groupInfoToClient));
        }
    }
}

CT_VOID CGroupMgr::ModifyGroupNotice(MSG_CS_Modify_GroupNotice* pGroupNotice)
{
    auto itGroupID = m_mapUserGroup.find(pGroupNotice->dwMasterID);
    if (itGroupID == m_mapUserGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupNotice->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    auto itGroup = m_mapGroup.find(itGroupID->second);
    if (itGroup == m_mapGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupNotice->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    std::unique_ptr<tagGroupInfo>& pGroupInfo = itGroup->second->GetGroupInfo();
    if (pGroupInfo->dwMasterID != pGroupNotice->dwMasterID)
    {
        SendGroupErrorMsg(pGroupNotice->dwMasterID, PERMISSION_DENIED);
        return;
    }

    //修改群公告
    _snprintf_info(pGroupInfo->szNotice, sizeof(pGroupInfo->szNotice), "%s", pGroupNotice->szNotice);

    //到DB修改群公告和群名称
    UpdateGroupInfoToDB(pGroupInfo);

    //广播给所有群用户
    std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = itGroup->second->GetAllGroupUserInfo();
    MSG_SC_Modify_GroupInfo groupInfoToClient;
    groupInfoToClient.dwMainID = MSG_GROUP_MAIN;
    groupInfoToClient.dwSubID = SUB_SC_MODIFY_GROUP_INFO;
    groupInfoToClient.cbLevel = pGroupInfo->cbLevel;
    groupInfoToClient.cbIcon = pGroupInfo->cbIcon;
    groupInfoToClient.cbSettleDays = pGroupInfo->cbSettleDays;
    //groupInfoToClient.cbIncomeRate = pGroupInfo->cbIncomeRate;
    groupInfoToClient.dwNextContribution = GetLevelMaxContribution(pGroupInfo->cbLevel);
    _snprintf_info(groupInfoToClient.szGroupName, sizeof(groupInfoToClient.szGroupName), "%s", pGroupInfo->szGroupName);
    _snprintf_info(groupInfoToClient.szNotice, sizeof(groupInfoToClient.szNotice), "%s", pGroupInfo->szNotice);

    for (auto& it : mapGroupUserInfo)
    {
        acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
        if (pProxySocket)
        {
            groupInfoToClient.dwValue2 = it.first;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupInfoToClient, sizeof(groupInfoToClient));
        }
    }
}

CT_VOID CGroupMgr::ModifyGroupIcon(MSG_CS_Modify_GroupIcon* pGroupIcon)
{
    auto itGroupID = m_mapUserGroup.find(pGroupIcon->dwMasterID);
    if (itGroupID == m_mapUserGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupIcon->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    auto itGroup = m_mapGroup.find(itGroupID->second);
    if (itGroup == m_mapGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupIcon->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    std::unique_ptr<tagGroupInfo>& pGroupInfo = itGroup->second->GetGroupInfo();
    if (pGroupInfo->dwMasterID != pGroupIcon->dwMasterID)
    {
        SendGroupErrorMsg(pGroupIcon->dwMasterID, PERMISSION_DENIED);
        return;
    }

    //修改群图标
    pGroupInfo->cbIcon = pGroupIcon->cbIcon;

    //到DB修改群公告和群名称
    UpdateGroupInfoToDB(pGroupInfo);

    //广播给所有群用户
    std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = itGroup->second->GetAllGroupUserInfo();
    MSG_SC_Modify_GroupInfo groupInfoToClient;
    groupInfoToClient.dwMainID = MSG_GROUP_MAIN;
    groupInfoToClient.dwSubID = SUB_SC_MODIFY_GROUP_INFO;
    groupInfoToClient.cbLevel = pGroupInfo->cbLevel;
    groupInfoToClient.cbIcon = pGroupInfo->cbIcon;
    groupInfoToClient.cbSettleDays = pGroupInfo->cbSettleDays;
    //groupInfoToClient.cbIncomeRate = pGroupInfo->cbIncomeRate;
    groupInfoToClient.dwNextContribution = GetLevelMaxContribution(pGroupInfo->cbLevel);
    _snprintf_info(groupInfoToClient.szGroupName, sizeof(groupInfoToClient.szGroupName), "%s", pGroupInfo->szGroupName);
    _snprintf_info(groupInfoToClient.szNotice, sizeof(groupInfoToClient.szNotice), "%s", pGroupInfo->szNotice);

    for (auto& it : mapGroupUserInfo)
    {
        acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
        if (pProxySocket)
        {
            groupInfoToClient.dwValue2 = it.first;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupInfoToClient, sizeof(groupInfoToClient));
        }
    }
}

CT_VOID CGroupMgr::UpgradeGroup(MSG_CS_Upgrade_Group* pUpgradeGroup)
{
    auto itGroupID = m_mapUserGroup.find(pUpgradeGroup->dwMasterID);
    if (itGroupID == m_mapUserGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pUpgradeGroup->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    auto itGroup = m_mapGroup.find(itGroupID->second);
    if (itGroup == m_mapGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pUpgradeGroup->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    std::unique_ptr<tagGroupInfo>& pGroupInfo = itGroup->second->GetGroupInfo();
    if (pGroupInfo->dwMasterID != pUpgradeGroup->dwMasterID)
    {
        SendGroupErrorMsg(pUpgradeGroup->dwMasterID, PERMISSION_DENIED);
        return;
    }

    //查找群主信息
    tagGroupUserInfo* pGroupUserInfo = itGroup->second->GetGroupUserInfo(pUpgradeGroup->dwMasterID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    CT_DWORD dwMaxContributeion = GetLevelMaxContribution(pGroupInfo->cbLevel);

    //检测群是否可以升级
    if (pGroupInfo->dwContribution < dwMaxContributeion)
    {
        SendGroupErrorMsg(pUpgradeGroup->dwMasterID, UPGRADE_CONTRIBUTE_NOT_ENOUGH);
        return;
    }

    //检测群是否达到最大等级
    tagGroupLevelCfg* pGroupLevelCfg = GetLevelCfg(pGroupInfo->cbLevel+1);
    if (pGroupLevelCfg == NULL)
    {
        SendGroupErrorMsg(pUpgradeGroup->dwMasterID, UPGRADE_GROUP_MAX);
        return;
    }

    //升级群信息
    pGroupInfo->cbLevel +=1;
    if (pGroupInfo->cbSettleDays > pGroupLevelCfg->cbSettleDays)
    {
        pGroupInfo->cbSettleDays = pGroupLevelCfg->cbSettleDays;
    }

    if (pGroupInfo->cbIncomeRate < pGroupLevelCfg->wIncomeRate/10)
    {
        pGroupInfo->cbIncomeRate = (CT_BYTE)(pGroupLevelCfg->wIncomeRate/10);
        //修改群主的收入比例
        pGroupUserInfo->cbInComeRate =  pGroupInfo->cbIncomeRate;
        UpdateGroupUserInfoToDB(itGroupID->second, pGroupUserInfo);
    }

    //到DB修改群公告和群名称
    UpdateGroupInfoToDB(pGroupInfo);

    //广播给所有群用户
    std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = itGroup->second->GetAllGroupUserInfo();
    MSG_SC_Upgrade_Group upgradeGroup;
    upgradeGroup.dwMainID = MSG_GROUP_MAIN;
    upgradeGroup.dwSubID = SUB_SC_UPGRADE_GROUP;
    upgradeGroup.cbLevel = pGroupInfo->cbLevel;
    //groupInfoToClient.cbIcon = pGroupInfo->cbIcon;
    upgradeGroup.cbSettleDays = pGroupInfo->cbSettleDays;
    upgradeGroup.cbIncomeRate = pGroupInfo->cbIncomeRate;
    upgradeGroup.dwNextContribution = pGroupLevelCfg->dwMaxContribution;
    //_snprintf_info(groupInfoToClient.szGroupName, sizeof(groupInfoToClient.szGroupName), "%s", pGroupInfo->szGroupName);
    //_snprintf_info(groupInfoToClient.szNotice, sizeof(groupInfoToClient.szNotice), "%s", pGroupInfo->szNotice);

    for (auto& it : mapGroupUserInfo)
    {
        acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
        if (pProxySocket)
        {
            upgradeGroup.dwValue2 = it.first;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &upgradeGroup, sizeof(MSG_SC_Upgrade_Group));
        }
    }
}

CT_VOID CGroupMgr::ModifyGroupUserRemarks(MSG_CS_DirGroupUser_Remarks* pGroupUserRemarks)
{
    auto itGroupID = m_mapUserGroup.find(pGroupUserRemarks->dwUserID);
    if (itGroupID == m_mapUserGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupUserRemarks->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    auto itGroup = m_mapGroup.find(itGroupID->second);
    if (itGroup == m_mapGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupUserRemarks->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    //查找上级信息
    tagGroupUserInfo* pGroupUserInfo = itGroup->second->GetGroupUserInfo(pGroupUserRemarks->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    //查找玩家是否下级
    MapGroupLinkUser& mapDirLinkUser = itGroup->second->GetDirGroupUser();
    auto itDirLinkUser = mapDirLinkUser.find(pGroupUserRemarks->dwUserID);
    if (itDirLinkUser == mapDirLinkUser.end())
    {
        return;
    }

    bool bCanModify = false;
    std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirLinkUser->second;
    for (auto& itVecDirGroupUser : vecDirGroupUser)
    {
        if (itVecDirGroupUser->dwUserID == pGroupUserRemarks->dwDirUserID)
        {
            bCanModify = true;
            break;
        }
    }

    if (!bCanModify)
    {
        return;
    }

    //查找直属下级信息
    tagGroupUserInfo* pDirGroupUserInfo = itGroup->second->GetGroupUserInfo(pGroupUserRemarks->dwDirUserID);
    if (pDirGroupUserInfo == NULL)
    {
        return;
    }
    _snprintf_info(pDirGroupUserInfo->szRemarks, sizeof(pDirGroupUserInfo->szRemarks), "%s", pGroupUserRemarks->szRemarks);

    //更新数据库
    UpdateGroupUserInfoToDB(itGroupID->second, pDirGroupUserInfo);

    acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(pGroupUserInfo->dwUserID);
    if (pProxySocket)
    {
        MSG_SC_DirGroupUser_Remarks groupUserRemarks;
        groupUserRemarks.dwMainID = MSG_GROUP_MAIN;
        groupUserRemarks.dwSubID = SUB_SC_MODIFY_GROUP_USER_REMARKS;
        groupUserRemarks.dwValue2 = pGroupUserInfo->dwUserID;
        groupUserRemarks.dwDirUserID = pGroupUserRemarks->dwDirUserID;
        _snprintf_info(groupUserRemarks.szRemarks, sizeof(groupUserRemarks.szRemarks), "%s", pDirGroupUserInfo->szRemarks);
        CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupUserRemarks, sizeof(MSG_SC_DirGroupUser_Remarks));
    }
}

CT_VOID CGroupMgr::UpgradeGroupUserIncomeRate(MSG_CS_Upgrade_GroupUser_IncomeRate* pGroupUserIncomeRate)
{
    auto itGroupID = m_mapUserGroup.find(pGroupUserIncomeRate->dwUserID);
    if (itGroupID == m_mapUserGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupUserIncomeRate->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    auto itGroup = m_mapGroup.find(itGroupID->second);
    if (itGroup == m_mapGroup.end())
    {
        //返回错误
        SendGroupErrorMsg(pGroupUserIncomeRate->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    //查找上级信息
    tagGroupUserInfo* pGroupUserInfo = itGroup->second->GetGroupUserInfo(pGroupUserIncomeRate->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    //上级最低要保留1%
    if (pGroupUserInfo->cbInComeRate - 1 < pGroupUserIncomeRate->cbNewIncomeRate)
    {
        SendGroupErrorMsg(pGroupUserIncomeRate->dwUserID, UPGRADE_INCOMERATE_TOO_MUCH);
        return;
    }

    //查找玩家是否下级
    MapGroupLinkUser& mapDirLinkUser = itGroup->second->GetDirGroupUser();
    auto itDirLinkUser = mapDirLinkUser.find(pGroupUserIncomeRate->dwUserID);
    if (itDirLinkUser == mapDirLinkUser.end())
    {
        return;
    }

    bool bCanModify = false;
    std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirLinkUser->second;
    for (auto& itVecDirGroupUser : vecDirGroupUser)
    {
        if (itVecDirGroupUser->dwUserID == pGroupUserIncomeRate->dwDirUserID)
        {
            bCanModify = true;
            break;
        }
    }

    if (!bCanModify)
    {
        return;
    }

    //查找直属下级信息
    tagGroupUserInfo* pDirGroupUserInfo = itGroup->second->GetGroupUserInfo(pGroupUserIncomeRate->dwDirUserID);
    if (pDirGroupUserInfo == NULL)
    {
        return;
    }

    pDirGroupUserInfo->cbInComeRate = pGroupUserIncomeRate->cbNewIncomeRate;

    //更新数据库
    UpdateGroupUserInfoToDB(itGroupID->second, pDirGroupUserInfo);

    acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(pGroupUserInfo->dwUserID);
    if (pProxySocket)
    {
        MSG_SC_Upgrade_GroupUser_IncomeRate groupUserIncomeRate;
        groupUserIncomeRate.dwMainID = MSG_GROUP_MAIN;
        groupUserIncomeRate.dwSubID = SUB_SC_UPGRADE_GROUP_USER_INCOMERATE;
        groupUserIncomeRate.dwValue2 = pGroupUserInfo->dwUserID;
        groupUserIncomeRate.dwDirUserID = pGroupUserIncomeRate->dwDirUserID;
        groupUserIncomeRate.cbIncomeRate = pDirGroupUserInfo->cbInComeRate;
        CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupUserIncomeRate, sizeof(MSG_SC_Upgrade_GroupUser_IncomeRate));
    }
}

CT_DWORD CGroupMgr::CalUserGroupCount(CT_DWORD dwUserID)
{
	CT_DWORD dwGroupCount = 0;
	for (auto& it : m_mapGroup)
	{
		if (dwUserID == it.second->GetGroupMasterID())
		{
			++dwGroupCount;
		}
	}

	return dwGroupCount;
}

CT_VOID CGroupMgr::ModifyGroupOptions(CT_DWORD dwGroupID, CT_WORD wBindUserPlay, CT_DWORD dwDefaultKindID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		return;
	}

	/*std::unique_ptr<tagGroupInfo>& groupInfo = it->second->GetGroupInfo();
	if (groupInfo->wBindUserPlay != wBindUserPlay)
	{
		groupInfo->wBindUserPlay = wBindUserPlay;

		//更新给各GS
		const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
		if (pMapGameServer)
		{
			auto it = pMapGameServer->begin();
			for (; it != pMapGameServer->end(); ++it)
			{
				UpdateGroupInfoToGameServer(it->first, dwGroupID);
			}
		}
	}

	if (groupInfo->dwKindID != dwDefaultKindID)
	{
		groupInfo->dwKindID = dwDefaultKindID;

		//广播给前端
		std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = it->second->GetAllGroupUserInfo();
		MSG_SC_Modify_GroupDefaultGame defaultGame;
		defaultGame.dwMainID = MSG_GROUP_MAIN;
		defaultGame.dwSubID = SUB_SC_MODIFY_GROUP_DEFAULT_GAME;
		defaultGame.dwGroupID = dwGroupID;
		defaultGame.dwDefaultGameKind = dwDefaultKindID;

		for (auto& it : mapGroupUserInfo)
		{
			acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pProxySocket)
			{
				defaultGame.dwValue2 = it.first;
				CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &defaultGame, sizeof(defaultGame));
			}
		}
	}*/
}

CT_VOID  CGroupMgr::SetGroupUserOnline(CT_DWORD dwGroupID, CT_DWORD dwUserID, CT_BOOL bOnline)
{
    auto it = m_mapGroup.find(dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(dwUserID);
    if (pGroupUserInfo == NULL)
        return;

    pGroupUserInfo->bOnline = bOnline;

    if (pGroupUserInfo->bOnline)
    {
        pGroupUserInfo->dwLastDate = time(NULL);
    }

    //检测上级是否在线
    MSG_SC_Group_Online_State onlineState;
    if (pGroupUserInfo->dwParentUserId != 0) {
        acl::aio_socket_stream
            *pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(pGroupUserInfo->dwParentUserId);
        if (pProxySocket != NULL) {
            onlineState.dwMainID = MSG_GROUP_MAIN;
            onlineState.dwSubID = SUB_SC_GROUP_USER_ONLINE_STATE;
            onlineState.dwValue2 = pGroupUserInfo->dwParentUserId;
            onlineState.dwUserID = dwUserID;
            onlineState.cbOnline = bOnline;

            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &onlineState, sizeof(MSG_SC_Group_Online_State));
        }
    }

    //更新状态给直属下级
    MapGroupLinkUser& mapGroupDirLinkUser = it->second->GetDirGroupUser();
    auto itDirUser = mapGroupDirLinkUser.find(dwUserID);
    if (itDirUser != mapGroupDirLinkUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecDirUser = itDirUser->second;
        for (auto& itvecDirUser : vecDirUser)
        {
            pGroupUserInfo = itvecDirUser;
            acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(pGroupUserInfo->dwUserID);
            if (pProxySocket != NULL)
            {
                onlineState.dwMainID = MSG_GROUP_MAIN;
                onlineState.dwSubID = SUB_SC_GROUP_USER_ONLINE_STATE;
                onlineState.dwValue2 = pGroupUserInfo->dwUserID;
                onlineState.dwUserID = dwUserID;
                onlineState.cbOnline = bOnline;

                CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &onlineState, sizeof(MSG_SC_Group_Online_State));
            }
        }
    }
}

CT_VOID CGroupMgr::ChangeGroupMemberPosition(MSG_CS_Change_GroupPosition* pChangeGroupPosition)
{
    auto it = m_mapGroup.find(pChangeGroupPosition->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    if (pChangeGroupPosition->dwMasterID == pChangeGroupPosition->dwUserID)
    {
        return;
    }

    std::unique_ptr<tagGroupInfo>& groupInfo = it->second->GetGroupInfo();
    //判断权限
    if (groupInfo->dwMasterID != pChangeGroupPosition->dwMasterID)
    {
        SendGroupErrorMsg(pChangeGroupPosition->dwMasterID, PERMISSION_DENIED);
        return;
    }

    //判断玩家是否群成员
    bool bIsGroupUser = it->second->IsGroupUser(pChangeGroupPosition->dwUserID);
    if (!bIsGroupUser)
    {
        SendGroupErrorMsg(pChangeGroupPosition->dwMasterID, NOT_GROUP_USER);
        return;
    }

    if (pChangeGroupPosition->cbOperateCode == 1)       //设为副群主
    {
        /*if (groupInfo->dwDeputyID1 != 0 && groupInfo->dwDeputyID2 != 0)
        {
            SendGroupErrorMsg(pChangeGroupPosition->dwMasterID, GROUP_DEPUTY_FULL);
            return;
        }

        if (groupInfo->dwDeputyID1 == 0)
        {
            groupInfo->dwDeputyID1 = pChangeGroupPosition->dwUserID;
            SendOneGroupInfoToClient(pChangeGroupPosition->dwGroupID, pChangeGroupPosition->dwUserID);
        }
        else if (groupInfo->dwDeputyID2 == 0)
        {
            groupInfo->dwDeputyID2 = pChangeGroupPosition->dwUserID;
            SendOneGroupInfoToClient(pChangeGroupPosition->dwGroupID, pChangeGroupPosition->dwUserID);
        }*/
    }
    else if (pChangeGroupPosition->cbOperateCode == 2)  //设为成员
    {
        /*bool bSetSucc = false;

        if (groupInfo->dwDeputyID1 == pChangeGroupPosition->dwUserID)
        {
            groupInfo->dwDeputyID1 = 0;
            bSetSucc = true;
        }
        else if (groupInfo->dwDeputyID2 == pChangeGroupPosition->dwUserID)
        {
            groupInfo->dwDeputyID2 = 0;
            bSetSucc = true;
        }

        if (!bSetSucc)
        {
            return;
        }*/
    }

    //设置成功
    MSG_SC_Change_GroupPosition changeGroupPosition;
    changeGroupPosition.dwUserID = pChangeGroupPosition->dwUserID;
    changeGroupPosition.dwGroupID = pChangeGroupPosition->dwGroupID;
    changeGroupPosition.cbPosition = it->second->GetMemberPosition(pChangeGroupPosition->dwUserID);
    changeGroupPosition.dwMainID = MSG_GROUP_MAIN;
    changeGroupPosition.dwSubID = SUB_SC_CHANGE_GROUP_POSITION;

    //发送给管理员
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(groupInfo->dwMasterID);
    if (pUserInfo != NULL)
    {
        acl::aio_socket_stream* pMasterProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
        if (pMasterProxySock != NULL)
        {
            changeGroupPosition.dwValue2 = groupInfo->dwMasterID;
            CNetModule::getSingleton().Send(pMasterProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &changeGroupPosition, sizeof(MSG_SC_Change_GroupPosition));
        }
    }

    /*if (groupInfo->dwDeputyID1 != 0 && groupInfo->dwDeputyID1 != pChangeGroupPosition->dwUserID)
    {
        pUserInfo = CUserMgr::get_instance().GetUserInfo(groupInfo->dwDeputyID1);
        if (pUserInfo != NULL)
        {
            acl::aio_socket_stream* pMasterProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
            if (pMasterProxySock != NULL)
            {
                changeGroupPosition.dwValue2 = groupInfo->dwDeputyID1;
                CNetModule::getSingleton().Send(pMasterProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &changeGroupPosition, sizeof(MSG_SC_Change_GroupPosition));
            }
        }
    }

    if (groupInfo->dwDeputyID2 != 0 && groupInfo->dwDeputyID2 != pChangeGroupPosition->dwUserID)
    {
        pUserInfo = CUserMgr::get_instance().GetUserInfo(groupInfo->dwDeputyID2);
        if (pUserInfo != NULL)
        {
            acl::aio_socket_stream* pMasterProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
            if (pMasterProxySock != NULL)
            {
                changeGroupPosition.dwValue2 = groupInfo->dwDeputyID2;
                CNetModule::getSingleton().Send(pMasterProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &changeGroupPosition, sizeof(MSG_SC_Change_GroupPosition));
            }
        }
    }*/

    //发送给被改变的玩家
    pUserInfo = CUserMgr::get_instance().GetUserInfo(pChangeGroupPosition->dwUserID);
    if (pUserInfo != NULL)
    {
        acl::aio_socket_stream* pMasterProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
        if (pMasterProxySock != NULL)
        {
            changeGroupPosition.dwValue2 = pChangeGroupPosition->dwUserID;
            CNetModule::getSingleton().Send(pMasterProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &changeGroupPosition, sizeof(MSG_SC_Change_GroupPosition));
        }
    }

    //操作DB
    MSG_CS2DB_GroupPosition groupPosition;
    groupPosition.dwGroupID = groupInfo->dwGroupID;
    //groupPosition.dwDeputyID1 = groupInfo->dwDeputyID1;
    //groupPosition.dwDeputyID2 = groupInfo->dwDeputyID2;
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_GROUP_POSITION, &groupPosition, sizeof(MSG_CS2DB_GroupPosition));
}

CT_VOID CGroupMgr::QueryDirGroupUserList(MSG_CS_Query_DirGroupUser_List *pQueryDirGroupUser)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryDirGroupUser->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    CT_BYTE szBuffer[2048] = { 0 };
    CT_DWORD dwSendSize = 0;
    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_GROUP_MAIN;
    msgHead.dwSubID = SUB_SC_GROUP_DIR_USER_LIST;
    msgHead.dwValue2 = pQueryDirGroupUser->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MapGroupLinkUser& mapDirGroupUser = it->second->GetDirGroupUser();
    MapGroupLinkUser& mapSubGroupUser = it->second->GetSubGroupUser();

    MSG_SC_DirGroupUser_Summary dirGroupUserSummary;

    //先增加总附属人数
    auto itSubUser = mapSubGroupUser.find(pQueryDirGroupUser->dwUserID);
    if (itSubUser != mapSubGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
        dirGroupUserSummary.wDirSubUserCount += vecSubGroupUser.size();
    }

    auto itDirUser = mapDirGroupUser.find(pQueryDirGroupUser->dwUserID);
    if (itDirUser == mapDirGroupUser.end())
    {
        dirGroupUserSummary.wCurrPage = pQueryDirGroupUser->wPage;
        dirGroupUserSummary.wDirUserCount = 0;
        dirGroupUserSummary.wTotalPage = 0;
        dirGroupUserSummary.wDirSubUserCount = 0;
        memcpy(szBuffer + dwSendSize, &dirGroupUserSummary, sizeof(MSG_SC_DirGroupUser_Summary));
        dwSendSize += sizeof(MSG_SC_DirGroupUser_Summary);
    }
    else
    {
        std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
        CT_WORD wDirUserCount = (CT_WORD)vecDirGroupUser.size();
        CT_WORD wTotalPage = (wDirUserCount % PAGE_ITEM_COUNT == 0 ? wDirUserCount / PAGE_ITEM_COUNT : wDirUserCount / PAGE_ITEM_COUNT + 1);

        if (pQueryDirGroupUser->wPage > wTotalPage || pQueryDirGroupUser->wPage < 1)
        {
            return;
        }
        dirGroupUserSummary.wCurrPage = pQueryDirGroupUser->wPage;
        dirGroupUserSummary.wDirUserCount = wDirUserCount;
        dirGroupUserSummary.wTotalPage = wTotalPage;
        dirGroupUserSummary.wDirSubUserCount += wDirUserCount; //直属玩家
        memcpy(szBuffer + dwSendSize, &dirGroupUserSummary, sizeof(MSG_SC_DirGroupUser_Summary));
        dwSendSize += sizeof(MSG_SC_DirGroupUser_Summary);

        int nPageCount = 0;
        for (size_t i = 0 + (pQueryDirGroupUser->wPage- 1)*PAGE_ITEM_COUNT; i < vecDirGroupUser.size(); ++i)
        {
            tagGroupUserInfo* pGroupUserInfo = vecDirGroupUser[i];

            MSG_SC_DirGroupUser dirGroupUser;
            dirGroupUser.dwUserID = pGroupUserInfo->dwUserID;
            dirGroupUser.dwLastLoginTime = pGroupUserInfo->dwLastDate;
            dirGroupUser.cbIncomeRate = pGroupUserInfo->cbInComeRate;
            dirGroupUser.cbOnline = pGroupUserInfo->bOnline;
            _snprintf_info(dirGroupUser.szRemarks, sizeof(dirGroupUser.szRemarks), "%s", pGroupUserInfo->szRemarks);
            auto itDirUser2 = mapDirGroupUser.find(pGroupUserInfo->dwUserID);
            if (itDirUser2 != mapDirGroupUser.end())
            {
                //直属下级
                dirGroupUser.wSubUserCount += (CT_WORD)itDirUser2->second.size();
                //附属下级
                auto itSubUser2 = mapSubGroupUser.find(pGroupUserInfo->dwUserID);
                if (itSubUser2 != mapSubGroupUser.end())
                {
                    dirGroupUser.wSubUserCount += (CT_WORD)itSubUser2->second.size();
                }
            }
            memcpy(szBuffer + dwSendSize, &dirGroupUser, sizeof(MSG_SC_DirGroupUser));
            dwSendSize += sizeof(MSG_SC_DirGroupUser);

            ++nPageCount;
            if (nPageCount >= PAGE_ITEM_COUNT)
            {
                break;
            }
        }
    }
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

CT_VOID CGroupMgr::QueryDirGroupUserInfo(MSG_CS_Query_DirGroupUser_Info* pQueryDirGroupUserInfo)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryDirGroupUserInfo->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //判断是否上下级关系
    tagGroupUserInfo* pParentGroupUserInfo = it->second->GetGroupUserInfo(pQueryDirGroupUserInfo->dwUserID);
    if (pParentGroupUserInfo == NULL)
    {
        return;
    }
    tagGroupUserInfo* pDirGroupUserInfo = it->second->GetGroupUserInfo(pQueryDirGroupUserInfo->dwDirUserID);
    if (pDirGroupUserInfo == NULL)
    {
        return;
    }
    if (pDirGroupUserInfo->dwParentUserId != pQueryDirGroupUserInfo->dwUserID)
    {
        return;
    }

    //查找玩家的所有收入
    CT_LONGLONG llParentTotalIncome = 0;
    MapGroupParentUserTotalIncome& mapGroupParentUserTotalIncome = it->second->GetParentUserTotalIncome();
    auto itParentTotalIncome = mapGroupParentUserTotalIncome.find(pQueryDirGroupUserInfo->dwUserID);
    if (itParentTotalIncome != mapGroupParentUserTotalIncome.end())
    {
        MapGroupUserTotalIncome& mapGroupUserTotalIncome = itParentTotalIncome->second;
        auto itTotalIncome = mapGroupUserTotalIncome.find(pQueryDirGroupUserInfo->dwDirUserID);
        if (itTotalIncome != mapGroupUserTotalIncome.end())
        {
            llParentTotalIncome = itTotalIncome->second;
        }
    }

    CT_WORD wTotalUserCount = 0;
    MapGroupLinkUser& mapDirGroupUser = it->second->GetDirGroupUser();
    auto itDirUser = mapDirGroupUser.find(pQueryDirGroupUserInfo->dwDirUserID);
    if (itDirUser != mapDirGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
        wTotalUserCount += vecDirGroupUser.size();
    }

    MapGroupLinkUser& mapSubGroupUser = it->second->GetSubGroupUser();
    auto itSubUser = mapSubGroupUser.find(pQueryDirGroupUserInfo->dwDirUserID);
    if (itSubUser != mapSubGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
        wTotalUserCount += vecSubGroupUser.size();
    }

    MSG_SC_DirGroupUser_Info dirGroupUserInfo;
    dirGroupUserInfo.dwMainID = MSG_GROUP_MAIN;
    dirGroupUserInfo.dwSubID = SUB_SC_QUERY_GROUP_DIR_USER_INFO;
    dirGroupUserInfo.dwValue2 = pQueryDirGroupUserInfo->dwUserID;
    dirGroupUserInfo.dwUserID = pDirGroupUserInfo->dwUserID;
    dirGroupUserInfo.dwRecharge = pDirGroupUserInfo->dwRecharge;
    dirGroupUserInfo.dIncome = llParentTotalIncome*TO_DOUBLE;
    dirGroupUserInfo.wSubUserCount = wTotalUserCount;
    dirGroupUserInfo.cbSelfIncomRate = pParentGroupUserInfo->cbInComeRate;
    dirGroupUserInfo.cbDirIncomeRate = pDirGroupUserInfo->cbInComeRate;
    _snprintf_info(dirGroupUserInfo.szRemarks, sizeof(dirGroupUserInfo.szRemarks), "%s", pDirGroupUserInfo->szRemarks);
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &dirGroupUserInfo,
                                    sizeof(MSG_SC_DirGroupUser_Info));
}

CT_VOID CGroupMgr::QueryOneDirGroupUser(const CT_VOID * pData, CT_DWORD dwDataSize)
{
    if (dwDataSize != sizeof(MSG_CS_Query_One_Group_DirUser))
    {
        return;
    }

    MSG_CS_Query_One_Group_DirUser* pQueryOneGroupDirUser = (MSG_CS_Query_One_Group_DirUser*)pData;

    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryOneGroupDirUser->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    MSG_SC_One_DirGroupUser oneDirGroupUser;
    oneDirGroupUser.dwMainID = MSG_GROUP_MAIN;
    oneDirGroupUser.dwSubID = SUB_SC_QUERY_ONE_GROUP_DIR_USER;
    oneDirGroupUser.dwValue2 = pQueryOneGroupDirUser->dwUserID;

    MapGroupLinkUser& mapDirGroupUser = it->second->GetDirGroupUser();
    MapGroupLinkUser& mapSubGroupUser = it->second->GetSubGroupUser();

    auto itDirUser = mapDirGroupUser.find(pQueryOneGroupDirUser->dwUserID);
    if (itDirUser != mapDirGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;

        for (size_t i = 0 ; i < vecDirGroupUser.size(); ++i)
        {
            tagGroupUserInfo* pGroupUserInfo = vecDirGroupUser[i];
            if (pGroupUserInfo->dwUserID != pQueryOneGroupDirUser->dwBeQueryUserID)
                continue;

            oneDirGroupUser.dirUserInfo.dwUserID = pGroupUserInfo->dwUserID;
            oneDirGroupUser.dirUserInfo.dwLastLoginTime = pGroupUserInfo->dwLastDate;
            oneDirGroupUser.dirUserInfo.cbIncomeRate = pGroupUserInfo->cbInComeRate;
            oneDirGroupUser.dirUserInfo.cbOnline = pGroupUserInfo->bOnline;
            _snprintf_info(oneDirGroupUser.dirUserInfo.szRemarks, sizeof(oneDirGroupUser.dirUserInfo.szRemarks), "%s", pGroupUserInfo->szRemarks);
            oneDirGroupUser.cbResult = 0;

            auto itDirUser2 = mapDirGroupUser.find(pGroupUserInfo->dwUserID);
            if (itDirUser2 != mapDirGroupUser.end())
            {
                //直属下级
                oneDirGroupUser.dirUserInfo.wSubUserCount += (CT_WORD)itDirUser2->second.size();
                //附属下级
                auto itSubUser2 = mapSubGroupUser.find(pGroupUserInfo->dwUserID);
                if (itSubUser2 != mapSubGroupUser.end())
                {
                    oneDirGroupUser.dirUserInfo.wSubUserCount += (CT_WORD)itSubUser2->second.size();
                }
            }
        }
    }

    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &oneDirGroupUser,
                                    sizeof(MSG_SC_One_DirGroupUser));
}

CT_VOID CGroupMgr::GiftScoreToDirGroupUser(const CT_VOID * pData, CT_DWORD dwDataSize)
{
    if (dwDataSize != sizeof(MSG_CS_Gift_Score_DirUser))
    {
        return;
    }

    MSG_CS_Gift_Score_DirUser* pGiftScore = (MSG_CS_Gift_Score_DirUser*)pData;

    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pGiftScore->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }


    MSG_SC_Gift_Score_DirUser giftScoreDirUser;
    giftScoreDirUser.dwMainID = MSG_GROUP_MAIN;
    giftScoreDirUser.dwSubID = SUB_SC_GIFT_SCORE_TO_DIR_USER;
    giftScoreDirUser.dwValue2 = pGiftScore->dwUserID;

    if (pUserInfo->dwGameServerID != 0)
    {
        giftScoreDirUser.cbResult = 3;
        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &giftScoreDirUser, sizeof(MSG_SC_Gift_Score_DirUser));
        return;
    }

    tagGroupUserInfo* pDirUserInfo = it->second->GetGroupUserInfo(pGiftScore->dwDirUserID);
    if (pDirUserInfo == NULL)
    {
        return;
    }

    if (pDirUserInfo->dwParentUserId != pGiftScore->dwUserID)
    {
        giftScoreDirUser.cbResult = 1;
        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &giftScoreDirUser, sizeof(MSG_SC_Gift_Score_DirUser));
        return;
    }

    CT_LONGLONG llGiftScore = pGiftScore->dwGiftScore*TO_LL;
    CT_LONGLONG llSourceScore = pUserInfo->llScore;
   // CT_LONGLONG llSourceBankScore = pUserInfo->llBankScore;
    if (llGiftScore > llSourceScore)
    {
        giftScoreDirUser.cbResult = 2;
        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &giftScoreDirUser, sizeof(MSG_SC_Gift_Score_DirUser));
        return;
    }

    //给玩家发送奖励
    CUserMgr::get_instance().AddUserScore(pGiftScore->dwUserID, -llGiftScore);
    //扣除玩家金币
    CUserMgr::get_instance().InsertUserScoreChangeRecord(pGiftScore->dwUserID, 0, llSourceScore, 0, -llGiftScore, PRESENTED_SCORE);

    //
    tagUserMail mail;
    mail.cbState = 0;
    mail.cbMailType = 2;
    mail.dwSendTime = Utility::GetTime();
    mail.dwExpiresTime = mail.dwSendTime + 89600 * 30; //默认一个月的有效期
    mail.dwUserID = pGiftScore->dwDirUserID;
    mail.llScore = llGiftScore;
    mail.cbScoreChangeType = PRESENTED_SCORE;
    _snprintf_info(mail.szTitle, sizeof(mail.szTitle), "俱乐部上级赠送");
    _snprintf_info(mail.szContent, sizeof(mail.szContent), "您的俱乐部上级给您赠送%0.2f金币!", llGiftScore*TO_DOUBLE);
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_SEND_MAIL, &mail, sizeof(mail));
    //发送邮件消息提醒
    CUserMgr::get_instance().SendNewMailTip(mail.dwUserID);

    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &giftScoreDirUser, sizeof(MSG_SC_Gift_Score_DirUser));
}

CT_VOID CGroupMgr::QuerySubGroupUserList(MSG_CS_Query_SubGroupUser_List* pQuerySubGroupUser)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQuerySubGroupUser->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //查找成员信息
    tagGroupUserInfo* pDirGroupUserInfo = it->second->GetGroupUserInfo(pQuerySubGroupUser->dwDirUserID);
    if (pDirGroupUserInfo == NULL)
    {
        return;
    }

    if (pDirGroupUserInfo->dwParentUserId != pQuerySubGroupUser->dwUserID)
    {
        return;
    }

    CT_BYTE szBuffer[2048] = { 0 };
    CT_DWORD dwSendSize = 0;
    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_GROUP_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_GROUP_SUB_USER_LIST;
    msgHead.dwValue2 = pQuerySubGroupUser->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MapGroupLinkUser& mapDirGroupUser = it->second->GetDirGroupUser();
    MapGroupLinkUser& mapSubGroupUser = it->second->GetSubGroupUser();
    MapGroupParentUserTotalIncome& mapParentTotalIncome = it->second->GetParentUserTotalIncome();

    MSG_SC_SubGroupUser_Summary subGroupUserSummary;
    auto itDirUser = mapDirGroupUser.find(pQuerySubGroupUser->dwDirUserID);
    if (itDirUser == mapDirGroupUser.end())
    {
        subGroupUserSummary.wCurrPage = pQuerySubGroupUser->wPage;
        subGroupUserSummary.wTotalPage = 0;
        memcpy(szBuffer + dwSendSize, &subGroupUserSummary, sizeof(MSG_SC_SubGroupUser_Summary));
        dwSendSize += sizeof(MSG_SC_SubGroupUser_Summary);
    }
    else
    {
        std::vector<tagGroupUserInfo*> vecAllSubGroupUser;
        {
            std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
            vecAllSubGroupUser.assign(vecDirGroupUser.begin(), vecDirGroupUser.end());
        }
        auto itSubUser = mapSubGroupUser.find(pQuerySubGroupUser->dwDirUserID);
        if (itSubUser != mapSubGroupUser.end())
        {
            std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
            for (auto& itVecSub : vecSubGroupUser)
            {
                vecAllSubGroupUser.push_back(itVecSub);
            }
        }
        CT_WORD wDirUserCount = (CT_WORD)vecAllSubGroupUser.size();

        CT_WORD wTotalPage = (wDirUserCount % PAGE_ITEM_COUNT == 0 ? wDirUserCount / PAGE_ITEM_COUNT : wDirUserCount / PAGE_ITEM_COUNT + 1);
        if (pQuerySubGroupUser->wPage > wTotalPage || pQuerySubGroupUser->wPage < 1)
        {
            return;
        }
        subGroupUserSummary.wCurrPage = pQuerySubGroupUser->wPage;
        subGroupUserSummary.wTotalPage = wTotalPage;
        memcpy(szBuffer + dwSendSize, &subGroupUserSummary, sizeof(MSG_SC_SubGroupUser_Summary));
        dwSendSize += sizeof(MSG_SC_SubGroupUser_Summary);

        int nPageCount = 0;
        for (size_t i = 0 + (pQuerySubGroupUser->wPage- 1)*PAGE_ITEM_COUNT; i < vecAllSubGroupUser.size(); ++i)
        {
            tagGroupUserInfo* pGroupUserInfo = vecAllSubGroupUser[i];

            CT_LONGLONG llTotalIncome = 0;
            auto itMapParentIncome = mapParentTotalIncome.find(pQuerySubGroupUser->dwUserID);
            if (itMapParentIncome != mapParentTotalIncome.end())
            {
                MapGroupUserTotalIncome& mapTotalIncome = itMapParentIncome->second;
                auto itMapIncome = mapTotalIncome.find(pGroupUserInfo->dwUserID);
                if (itMapIncome != mapTotalIncome.end())
                {
                    llTotalIncome = itMapIncome->second;
                }
            }

            MSG_SC_SubGroupUser subGroupUser;
            subGroupUser.dwUserID = pGroupUserInfo->dwUserID;
            subGroupUser.dwRegTime = pGroupUserInfo->dwRegDate;
            subGroupUser.dwLastLoginTime = pGroupUserInfo->dwLastDate;
            subGroupUser.dwRecharge = pGroupUserInfo->dwRecharge;
            subGroupUser.dIncome = llTotalIncome*TO_DOUBLE;
            memcpy(szBuffer + dwSendSize, &subGroupUser, sizeof(MSG_SC_SubGroupUser));
            dwSendSize += sizeof(MSG_SC_SubGroupUser);

            ++nPageCount;
            if (nPageCount >= PAGE_ITEM_COUNT)
            {
                break;
            }
        }
    }
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

CT_VOID CGroupMgr::QueryGroupUserIncome(MSG_CS_Query_Group_Income* pQueryGroupUserIncome)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryGroupUserIncome->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }


    std::unique_ptr<tagGroupInfo>& ptrGroupInfo = it->second->GetGroupInfo();

    //检测有没有这个玩家
    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(pQueryGroupUserIncome->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    unsigned int nTodayInt = CalcDayInt(0);
    unsigned int nYesterdayInt = CalcDayInt(1);
    unsigned int nLastTakeIncomeDate = pGroupUserInfo->dwLastTakeIncomeDate;

    CT_LONGLONG llTodayDirIncome = 0;
    CT_LONGLONG llTodaySubIncome = 0;
    CT_LONGLONG llYesterdayDirIncome = 0;
    CT_LONGLONG llYesterdaySubIncome = 0;
    CT_LONGLONG llRemainingIncome = 0;

    MapGroupAllUserDateIncome& mapGroupAllUserDateDirIncome = it->second->GetGroupUserDateDirIncome();
    MapGroupAllUserDateIncome& mapGroupAllUserDateSubIncome = it->second->GetGroupUserDateSubIncome();

    //直属成员的收入
    {
        auto itAllDateDirIncome = mapGroupAllUserDateDirIncome.find(pQueryGroupUserIncome->dwUserID);
        if (itAllDateDirIncome != mapGroupAllUserDateDirIncome.end())
        {
            MapGroupUserDateIncome& mapDateIncome = itAllDateDirIncome->second;
            for (auto& itAllDayIncome : mapDateIncome)
            {
                if (itAllDayIncome.first == nTodayInt)
                {
                    MapGroupUserIncome& mapUserIncome = itAllDayIncome.second;
                    for (auto& itTodayIncome : mapUserIncome)
                    {
                        std::unique_ptr<tagGroupUserDateIncome>& ptrTodayIncome = itTodayIncome.second;
                        llTodayDirIncome += ptrTodayIncome->dwDeductIncome;
                    }
                }
                else if (itAllDayIncome.first == nYesterdayInt)
                {
                    MapGroupUserIncome& mapUserIncome = itAllDayIncome.second;
                    for (auto& itYesterdayIncome : mapUserIncome)
                    {
                        std::unique_ptr<tagGroupUserDateIncome>& ptrYesterdayIncome = itYesterdayIncome.second;
                        llYesterdayDirIncome += ptrYesterdayIncome->dwDeductIncome;

                        //如果昨天的收入还没有提取,则记录昨天的收入
                        if (itAllDayIncome.first > nLastTakeIncomeDate)
                        {
                            llRemainingIncome += ptrYesterdayIncome->dwDeductIncome;
                        }
                    }
                }
                else if(itAllDayIncome.first > nLastTakeIncomeDate)
                {
                    MapGroupUserIncome& mapUserIncome = itAllDayIncome.second;
                    for (auto& itYesterdayIncome : mapUserIncome)
                    {
                        std::unique_ptr<tagGroupUserDateIncome>& ptrdayIncome = itYesterdayIncome.second;
                        /*llYesterdayDirIncome += ptrdayIncome->dwDeductIncome;

                        if (itAllDayIncome.first > nLastTakeIncomeDate)
                        {
                            llRemainingIncome += ptrdayIncome->dwDeductIncome;
                        }*/
                        llRemainingIncome += ptrdayIncome->dwDeductIncome;
                    }
                }
            }
        }
    }

    //附属成员的收入
    auto itAllDateSubIncome = mapGroupAllUserDateSubIncome.find(pQueryGroupUserIncome->dwUserID);
    if (itAllDateSubIncome != mapGroupAllUserDateSubIncome.end())
    {
        MapGroupUserDateIncome& mapDateIncome = itAllDateSubIncome->second;
        for (auto& itAllDayIncome : mapDateIncome)
        {
            if (itAllDayIncome.first == nTodayInt)
            {
                MapGroupUserIncome& mapUserIncome = itAllDayIncome.second;
                for (auto& itTodayIncome : mapUserIncome)
                {
                    std::unique_ptr<tagGroupUserDateIncome>& ptrTodayIncome = itTodayIncome.second;
                    llTodaySubIncome += ptrTodayIncome->dwDeductIncome;
                }
            }
            else if (itAllDayIncome.first == nYesterdayInt)
            {
                MapGroupUserIncome& mapUserIncome = itAllDayIncome.second;
                for (auto& itYesterdayIncome : mapUserIncome)
                {
                    std::unique_ptr<tagGroupUserDateIncome>& ptrYesterdayIncome = itYesterdayIncome.second;
                    llYesterdaySubIncome += ptrYesterdayIncome->dwDeductIncome;

                    //如果昨天的收入还没有提取,则记录昨天的收入
                    if (itAllDayIncome.first > nLastTakeIncomeDate)
                    {
                        llRemainingIncome += ptrYesterdayIncome->dwDeductIncome;
                    }
                }
            }
            else if(itAllDayIncome.first > nLastTakeIncomeDate)
            {
                MapGroupUserIncome& mapUserIncome = itAllDayIncome.second;
                for (auto& itYesterdayIncome : mapUserIncome)
                {
                    std::unique_ptr<tagGroupUserDateIncome>& ptrdayIncome = itYesterdayIncome.second;
                    /*llYesterdayDirIncome += ptrdayIncome->dwDeductIncome;

                    if (itAllDayIncome.first > nLastTakeIncomeDate)
                    {
                        llRemainingIncome += ptrdayIncome->dwDeductIncome;
                    }*/
                    llRemainingIncome += ptrdayIncome->dwDeductIncome;
                }
            }
        }
    }

    MSG_SC_Query_Group_Income userIncome;
    userIncome.dwMainID = MSG_GROUP_MAIN;
    userIncome.dwSubID = SUB_SC_QUERY_GROUP_INCOME;
    userIncome.dwValue2 = pQueryGroupUserIncome->dwUserID;

    //计算军团人数
    MapGroupLinkUser& mapDirGroupUser = it->second->GetDirGroupUser();
    MapGroupLinkUser& mapSubGroupUser = it->second->GetSubGroupUser();

    //直属军团人数
    auto itDirUser = mapDirGroupUser.find(pQueryGroupUserIncome->dwUserID);
    if (itDirUser != mapDirGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
        userIncome.wSubGroupUserCount += vecDirGroupUser.size();
    }

    //附属军团人数
    auto itSubUser = mapSubGroupUser.find(pQueryGroupUserIncome->dwUserID);
    if (itSubUser != mapSubGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
        userIncome.wSubGroupUserCount += vecSubGroupUser.size();
    }

    userIncome.dTodayDirIncome = llTodayDirIncome*TO_DOUBLE;
    userIncome.dTodaySubIncome = llTodaySubIncome*TO_DOUBLE;
    userIncome.cbSelfIncomeRate = pGroupUserInfo->cbInComeRate;
    userIncome.cbSettleDays = ptrGroupInfo->cbSettleDays;
    userIncome.dYesterDayIncome = (llYesterdayDirIncome+llYesterdaySubIncome)*TO_DOUBLE;
    userIncome.dTotalIncome = pGroupUserInfo->llTotalIncome*TO_DOUBLE;
    userIncome.dRemainingIncome = llRemainingIncome*TO_DOUBLE;
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &userIncome,
                                    sizeof(MSG_SC_Query_Group_Income));
}

CT_LONGLONG CGroupMgr::FindDateIncome(std::unique_ptr<CGroup>& ptrGroup, CT_DWORD dwParentUserID, CT_DWORD dwDirUserID, CT_DWORD dwSubUserID, CT_DWORD dwDate)
{
    if (dwDirUserID != 0)
    {
        MapGroupAllUserDateIncome& mapGroupAllUserDateDirIncome = ptrGroup->GetGroupUserDateDirIncome();
        auto itAllUserDateDirIncome = mapGroupAllUserDateDirIncome.find(dwParentUserID);
        if (itAllUserDateDirIncome != mapGroupAllUserDateDirIncome.end())
        {
            MapGroupUserDateIncome &mapGroupUserDateIncome = itAllUserDateDirIncome->second;
            auto itDateIncome = mapGroupUserDateIncome.find(dwDate);
            if (itDateIncome != mapGroupUserDateIncome.end())
            {
                MapGroupUserIncome &mapGroupUserIncome = itDateIncome->second;
                auto itIncome = mapGroupUserIncome.find(dwDirUserID);
                if (itIncome != mapGroupUserIncome.end())
                {
                    return itIncome->second->dwDeductIncome;
                }
            }
        }
    }

    if (dwSubUserID != 0)
    {
        MapGroupAllUserDateIncome& mapGroupAllUserDateSubIncome = ptrGroup->GetGroupUserDateSubIncome();
        auto itAllUserDateSubIncome = mapGroupAllUserDateSubIncome.find(dwParentUserID);
        if (itAllUserDateSubIncome != mapGroupAllUserDateSubIncome.end())
        {
            MapGroupUserDateIncome &mapGroupUserDateIncome = itAllUserDateSubIncome->second;
            auto itDateIncome = mapGroupUserDateIncome.find(dwDate);
            if (itDateIncome != mapGroupUserDateIncome.end())
            {
                MapGroupUserIncome &mapGroupUserIncome = itDateIncome->second;
                auto itIncome = mapGroupUserIncome.find(dwSubUserID);
                if (itIncome != mapGroupUserIncome.end())
                {
                    return itIncome->second->dwDeductIncome;
                }
            }
        }
    }

    return 0;
}

CT_LONGLONG CGroupMgr::FindDateTotalIncome(std::unique_ptr<CGroup>& ptrGroup, CT_DWORD dwParentUserID, CT_DWORD dwDate1, CT_DWORD dwDate2)
{
    CT_LONGLONG llRemainingIncome = 0;

    {
        MapGroupAllUserDateIncome& mapGroupAllUserDateDirIncome = ptrGroup->GetGroupUserDateDirIncome();
        auto itAllUserDateDirIncome = mapGroupAllUserDateDirIncome.find(dwParentUserID);
        if (itAllUserDateDirIncome != mapGroupAllUserDateDirIncome.end())
        {
            MapGroupUserDateIncome &mapGroupUserDateIncome = itAllUserDateDirIncome->second;
            for (auto& itMapDateIncome : mapGroupUserDateIncome)
            {
                CT_DWORD dwDate = itMapDateIncome.first;
                //在规定的时间内
                if (dwDate > dwDate1 && dwDate <= dwDate2)
                {
                    MapGroupUserIncome& mapGroupUserIncome = itMapDateIncome.second;
                    for (auto& itUserIncome : mapGroupUserIncome)
                    {
                        llRemainingIncome += itUserIncome.second->dwDeductIncome;
                    }
                }
            }
        }
    }

    {
        MapGroupAllUserDateIncome& mapGroupAllUserDateSubIncome = ptrGroup->GetGroupUserDateSubIncome();
        auto itAllUserDateSubIncome = mapGroupAllUserDateSubIncome.find(dwParentUserID);
        if (itAllUserDateSubIncome != mapGroupAllUserDateSubIncome.end())
        {
            MapGroupUserDateIncome &mapGroupUserDateIncome = itAllUserDateSubIncome->second;
            for (auto& itMapDateIncome : mapGroupUserDateIncome)
            {
                CT_DWORD dwDate = itMapDateIncome.first;
                //在规定的时间内
                if (dwDate > dwDate1 && dwDate <= dwDate2)
                {
                    MapGroupUserIncome& mapGroupUserIncome = itMapDateIncome.second;
                    for (auto& itUserIncome : mapGroupUserIncome)
                    {
                        llRemainingIncome += itUserIncome.second->dwDeductIncome;
                    }
                }
            }
        }
    }

    return llRemainingIncome;
}

CT_LONGLONG CGroupMgr::FindTotalIncome(std::unique_ptr<CGroup>& ptrGroup, CT_DWORD dwParentUserID, CT_DWORD dwSubUserID)
{
    MapGroupParentUserTotalIncome& mapGroupParentUserTotalIncome = ptrGroup->GetParentUserTotalIncome();
    auto itAllUserTotalIncome = mapGroupParentUserTotalIncome.find(dwParentUserID);
    if (itAllUserTotalIncome != mapGroupParentUserTotalIncome.end())
    {
        MapGroupUserTotalIncome &mapGroupUserTotalIncome = itAllUserTotalIncome->second;
        auto itMapIncome = mapGroupUserTotalIncome.find(dwSubUserID);
        if (itMapIncome != mapGroupUserTotalIncome.end())
        {
            return itMapIncome->second;
        }

    }

    return  0;
}

CT_VOID CGroupMgr::QueryGroupUserYesterdayIncome(MSG_CS_Query_Group_Yesterday_Income* pQueryGroupUserYesterdayIncome)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryGroupUserYesterdayIncome->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //检测有没有这个玩家
    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(pQueryGroupUserYesterdayIncome->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    CT_BYTE szBuffer[2048] = { 0 };
    CT_DWORD dwSendSize = 0;
    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_GROUP_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_GROUP_YESTERDAY_INCOME;
    msgHead.dwValue2 = pQueryGroupUserYesterdayIncome->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    unsigned int nYesterdayInt = CalcDayInt(1);
    MSG_SC_Yesterday_Summary yesterdaySummary;
    yesterdaySummary.cbIncomeRate = pGroupUserInfo->cbInComeRate;

    MapGroupLinkUser& mapDirGroupUser = it->second->GetDirGroupUser();
    MapGroupLinkUser& mapSubGroupUser = it->second->GetSubGroupUser();

/*    //计算直属人员数量
    auto itDirUser = mapDirGroupUser.find(pQueryGroupUserYesterdayIncome->dwUserID);
    if (itDirUser != mapDirGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
        yesterdaySummary.wDirSubGroupUserCount += vecDirGroupUser.size();
    }*/

    //先增加总附属人数
    CT_LONGLONG  llTotalSubIncome = 0;
    CT_LONGLONG  llTotalDirIncome = 0;
    auto itSubUser = mapSubGroupUser.find(pQueryGroupUserYesterdayIncome->dwUserID);
    if (itSubUser != mapSubGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
        yesterdaySummary.wDirSubGroupUserCount += vecSubGroupUser.size();
        //计算所有附属红利
        for (auto& itSubUser : vecSubGroupUser)
        {
            llTotalSubIncome +=  FindDateIncome(it->second, pQueryGroupUserYesterdayIncome->dwUserID, 0, itSubUser->dwUserID, nYesterdayInt);
        }
    }

    auto itDirUser = mapDirGroupUser.find(pQueryGroupUserYesterdayIncome->dwUserID);
    if (itDirUser == mapDirGroupUser.end())
    {
        yesterdaySummary.wCurrPage = pQueryGroupUserYesterdayIncome->wPage;
        yesterdaySummary.wTotalPage = 0;
        memcpy(szBuffer + dwSendSize, &yesterdaySummary, sizeof(MSG_SC_Yesterday_Summary));
        dwSendSize += sizeof(MSG_SC_Yesterday_Summary);
    }
    else
    {
        std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
        //计算所有直属红利
        for (auto& itDirUser : vecDirGroupUser)
        {
            llTotalDirIncome +=  FindDateIncome(it->second, pQueryGroupUserYesterdayIncome->dwUserID, itDirUser->dwUserID, 0, nYesterdayInt);
        }

        CT_WORD wDirUserCount = (CT_WORD)vecDirGroupUser.size();
        CT_WORD wTotalPage = (wDirUserCount % PAGE_ITEM_COUNT == 0 ? wDirUserCount / PAGE_ITEM_COUNT : wDirUserCount / PAGE_ITEM_COUNT + 1);

        if (pQueryGroupUserYesterdayIncome->wPage > wTotalPage || pQueryGroupUserYesterdayIncome->wPage < 1)
        {
            return;
        }
        yesterdaySummary.wCurrPage = pQueryGroupUserYesterdayIncome->wPage;
        yesterdaySummary.wDirGroupUserCount = wDirUserCount;
        yesterdaySummary.wTotalPage = wTotalPage;
        yesterdaySummary.wDirSubGroupUserCount += wDirUserCount; //直属玩家
        dwSendSize += sizeof(MSG_SC_Yesterday_Summary);

        //玩家的所有收入
        int nPageCount = 0;
        for (size_t i = 0 + (pQueryGroupUserYesterdayIncome->wPage- 1)*PAGE_ITEM_COUNT; i < vecDirGroupUser.size(); ++i)
        {
            tagGroupUserInfo* pGroupDirUserInfo = vecDirGroupUser[i];

            MSG_SC_Query_Group_Yesterday_Income yesterdayIncome;
            yesterdayIncome.dwUserID = pGroupDirUserInfo->dwUserID;
            yesterdayIncome.cbIncomeRate = pGroupDirUserInfo->cbInComeRate;
            yesterdayIncome.dDirIncome = FindDateIncome(it->second, pQueryGroupUserYesterdayIncome->dwUserID, pGroupDirUserInfo->dwUserID, 0, nYesterdayInt)*TO_DOUBLE;
            _snprintf_info(yesterdayIncome.szRemarks, sizeof(yesterdayIncome.szRemarks), "%s", pGroupDirUserInfo->szRemarks);

            CT_LONGLONG llSubIncome = 0;
            auto itDirUser = mapDirGroupUser.find(pGroupDirUserInfo->dwUserID);
            if (itDirUser != mapDirGroupUser.end())
            {
                std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
                yesterdayIncome.wSubGroupUserCount += vecDirGroupUser.size();

                for (auto& itDirUser : vecDirGroupUser)
                {
                    llSubIncome += FindDateIncome(it->second, pQueryGroupUserYesterdayIncome->dwUserID, 0, itDirUser->dwUserID, nYesterdayInt);
                }
            }

            auto itSubUser = mapSubGroupUser.find(pGroupDirUserInfo->dwUserID);
            if (itSubUser != mapSubGroupUser.end())
            {
                std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
                yesterdayIncome.wSubGroupUserCount += vecSubGroupUser.size();

                for (auto& itSubUser : vecSubGroupUser)
                {
                    llSubIncome += FindDateIncome(it->second, pQueryGroupUserYesterdayIncome->dwUserID, 0, itSubUser->dwUserID, nYesterdayInt);
                }
            }
            yesterdayIncome.dSubIncome = llSubIncome*TO_DOUBLE;

            memcpy(szBuffer + dwSendSize, &yesterdayIncome, sizeof(MSG_SC_Query_Group_Yesterday_Income));
            dwSendSize += sizeof(MSG_SC_Query_Group_Yesterday_Income);

            ++nPageCount;
            if (nPageCount >= PAGE_ITEM_COUNT)
            {
                break;
            }
        }
    }
    yesterdaySummary.dDirIncome = llTotalDirIncome*TO_DOUBLE;
    yesterdaySummary.dSubIncome = llTotalSubIncome*TO_DOUBLE;
    memcpy(szBuffer + sizeof(MSG_GameMsgDownHead), &yesterdaySummary, sizeof(MSG_SC_Yesterday_Summary));
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, dwSendSize);
}

CT_VOID CGroupMgr::QueryGroupUserIncomeDetail(MSG_CS_Query_Group_Income_Detail* pQueryGroupUserIncomeDetail)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryGroupUserIncomeDetail->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //检测有没有这个玩家
    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(pQueryGroupUserIncomeDetail->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    CT_BYTE szBuffer[2048] = { 0 };
    CT_DWORD dwSendSize = 0;
    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_GROUP_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_GROUP_INCOME_DETAIL;
    msgHead.dwValue2 = pQueryGroupUserIncomeDetail->dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_Yesterday_Summary yesterdaySummary;
    yesterdaySummary.cbIncomeRate = pGroupUserInfo->cbInComeRate;

    MapGroupLinkUser& mapDirGroupUser = it->second->GetDirGroupUser();
    MapGroupLinkUser& mapSubGroupUser = it->second->GetSubGroupUser();

    //先增加总附属人数
    CT_LONGLONG  llTotalSubIncome = 0;
    CT_LONGLONG  llTotalDirIncome = 0;
    auto itSubUser = mapSubGroupUser.find(pQueryGroupUserIncomeDetail->dwUserID);
    if (itSubUser != mapSubGroupUser.end())
    {
        std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
        yesterdaySummary.wDirSubGroupUserCount += vecSubGroupUser.size();
        //计算所有附属红利
        for (auto& itSubUser : vecSubGroupUser)
        {
            llTotalSubIncome +=  FindTotalIncome(it->second, pQueryGroupUserIncomeDetail->dwUserID, itSubUser->dwUserID);
        }
    }

    auto itDirUser = mapDirGroupUser.find(pQueryGroupUserIncomeDetail->dwUserID);
    if (itDirUser == mapDirGroupUser.end())
    {
        yesterdaySummary.wCurrPage = pQueryGroupUserIncomeDetail->wPage;
        yesterdaySummary.wTotalPage = 0;
        memcpy(szBuffer + dwSendSize, &yesterdaySummary, sizeof(MSG_SC_Yesterday_Summary));
        dwSendSize += sizeof(MSG_SC_Yesterday_Summary);
    }
    else
    {
        std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
        //计算所有直属红利
        for (auto& itDirUser : vecDirGroupUser)
        {
            llTotalDirIncome +=  FindTotalIncome(it->second, pQueryGroupUserIncomeDetail->dwUserID, itDirUser->dwUserID);
        }

        CT_WORD wDirUserCount = (CT_WORD)vecDirGroupUser.size();
        CT_WORD wTotalPage = (wDirUserCount % PAGE_ITEM_COUNT == 0 ? wDirUserCount / PAGE_ITEM_COUNT : wDirUserCount / PAGE_ITEM_COUNT + 1);

        if (pQueryGroupUserIncomeDetail->wPage > wTotalPage || pQueryGroupUserIncomeDetail->wPage < 1)
        {
            return;
        }
        yesterdaySummary.wCurrPage = pQueryGroupUserIncomeDetail->wPage;
        yesterdaySummary.wDirGroupUserCount = wDirUserCount;
        yesterdaySummary.wTotalPage = wTotalPage;
        yesterdaySummary.wDirSubGroupUserCount += wDirUserCount; //直属玩家
        dwSendSize += sizeof(MSG_SC_Yesterday_Summary);

        //玩家的所有收入
        int nPageCount = 0;
        for (size_t i = 0 + (pQueryGroupUserIncomeDetail->wPage- 1)*PAGE_ITEM_COUNT; i < vecDirGroupUser.size(); ++i)
        {
            tagGroupUserInfo* pGroupDirUserInfo = vecDirGroupUser[i];

            MSG_SC_Query_Group_Yesterday_Income yesterdayIncome;
            yesterdayIncome.dwUserID = pGroupDirUserInfo->dwUserID;
            yesterdayIncome.cbIncomeRate = pGroupDirUserInfo->cbInComeRate;
            yesterdayIncome.dDirIncome = FindTotalIncome(it->second, pQueryGroupUserIncomeDetail->dwUserID, pGroupDirUserInfo->dwUserID)*TO_DOUBLE;
            _snprintf_info(yesterdayIncome.szRemarks, sizeof(yesterdayIncome.szRemarks), "%s", pGroupDirUserInfo->szRemarks);

            CT_LONGLONG llSubIncome = 0;
            auto itDirUser = mapDirGroupUser.find(pGroupDirUserInfo->dwUserID);
            if (itDirUser != mapDirGroupUser.end())
            {
                std::vector<tagGroupUserInfo*>& vecDirGroupUser = itDirUser->second;
                yesterdayIncome.wSubGroupUserCount += vecDirGroupUser.size();

                for (auto& itDirUser : vecDirGroupUser)
                {
                    llSubIncome += FindTotalIncome(it->second, pQueryGroupUserIncomeDetail->dwUserID, itDirUser->dwUserID);
                }
            }

            auto itSubUser = mapSubGroupUser.find(pGroupDirUserInfo->dwUserID);
            if (itSubUser != mapSubGroupUser.end())
            {
                std::vector<tagGroupUserInfo*>& vecSubGroupUser = itSubUser->second;
                yesterdayIncome.wSubGroupUserCount += vecSubGroupUser.size();

                for (auto& itSubUser : vecSubGroupUser)
                {
                    llSubIncome += FindTotalIncome(it->second, pQueryGroupUserIncomeDetail->dwUserID, itSubUser->dwUserID);
                }
            }
            yesterdayIncome.dSubIncome = llSubIncome*TO_DOUBLE;

            memcpy(szBuffer + dwSendSize, &yesterdayIncome, sizeof(MSG_SC_Query_Group_Yesterday_Income));
            dwSendSize += sizeof(MSG_SC_Query_Group_Yesterday_Income);

            ++nPageCount;
            if (nPageCount >= PAGE_ITEM_COUNT)
            {
                break;
            }
        }
    }
    yesterdaySummary.dDirIncome = llTotalDirIncome*TO_DOUBLE;
    yesterdaySummary.dSubIncome = llTotalSubIncome*TO_DOUBLE;
    memcpy(szBuffer + sizeof(MSG_GameMsgDownHead), &yesterdaySummary, sizeof(MSG_SC_Yesterday_Summary));
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, dwSendSize);
}

CT_VOID CGroupMgr::QueryGroupUserSettleRecord(MSG_CS_Query_Group_Settle_Record* pQueryGroup)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pQueryGroup->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    MSG_CS2DB_Query_GroupIncome_Exchange queryGroupIncomeExchange;
    queryGroupIncomeExchange.dwUserID = pQueryGroup->dwUserID;
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_QUERY_GROUP_EXCHANGE, &queryGroupIncomeExchange, sizeof(MSG_CS2DB_Query_GroupIncome_Exchange));
}

CT_VOID CGroupMgr::QueryGroupUserSettleRecord(CT_DWORD dwUserID, MSG_SC_Query_Group_Settle_Record* pSettleResult, CT_DWORD dwDataSize)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    CT_BYTE szBuffer[1024];
    CT_DWORD dwSendSize = 0;

    MSG_GameMsgDownHead msgHead;
    msgHead.dwMainID = MSG_GROUP_MAIN;
    msgHead.dwSubID = SUB_SC_QUERY_GROUP_SETTLE_RECORD;
    msgHead.dwValue2 = dwUserID;
    memcpy(szBuffer, &msgHead, sizeof(msgHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    if (dwDataSize > 0)
    {
        memcpy(szBuffer+dwSendSize, pSettleResult, dwDataSize);
        dwSendSize += dwDataSize;
    }
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, dwSendSize);
}

CT_VOID CGroupMgr::SendGroupChatMsg(MSG_CS_Send_GroupChatMsg* pGroupChatMsg)
{
    //查找玩家的groupID
    auto itGroupId = m_mapUserGroup.find(pGroupChatMsg->dwFromUserID);
    if (itGroupId == m_mapUserGroup.end())
    {
        return;
    }

    CT_DWORD dwFromGroupId = itGroupId->second;

    itGroupId = m_mapUserGroup.find(pGroupChatMsg->dwToUserID);
    if (itGroupId == m_mapUserGroup.end())
    {
        return;
    }

    CT_DWORD dwToGroupId = itGroupId->second;

    //判断是否同一群
    if (dwFromGroupId != dwToGroupId)
    {
        return;
    }

    auto it = m_mapGroup.find(dwToGroupId);
    if (it == m_mapGroup.end())
    {
        return;
    }

    tagGroupUserInfo* pGroupFromUserInfo = it->second->GetGroupUserInfo(pGroupChatMsg->dwFromUserID);
    if (pGroupFromUserInfo == NULL)
    {
        return;
    }

    bool bCanSend = false;
    if (pGroupFromUserInfo->dwParentUserId == pGroupChatMsg->dwToUserID)
    {
        bCanSend = true;
    }

    if (!bCanSend)
    {
        MapGroupLinkUser& dirLinkUser = it->second->GetDirGroupUser();

        auto itDirUser = dirLinkUser.find(pGroupChatMsg->dwFromUserID);
        if (itDirUser != dirLinkUser.end())
        {
            std::vector<tagGroupUserInfo*>& vecDirGroupUserInfo = itDirUser->second;
            for (auto& itVecDir : vecDirGroupUserInfo)
            {
                if (itVecDir->dwUserID == pGroupChatMsg->dwToUserID)
                {
                    bCanSend = true;
                    break;
                }
            }
        }
    }

    if (!bCanSend)
    {
        return;
    }

    //判断接收玩家是否在线
    tagGroupChat groupChat;
    groupChat.dwMsgId = ++m_dwChatMsgId;
    groupChat.dwToUserID = pGroupChatMsg->dwToUserID;
    groupChat.dwFromUserID = pGroupChatMsg->dwFromUserID;
    groupChat.dwSendTime = time(NULL);
    _snprintf_info(groupChat.szChat, sizeof(groupChat.szChat), "%s", pGroupChatMsg->szContent);

    MSG_CS2DB_Insert_GroupChat groupChatTodb;
    memcpy(&groupChatTodb, &groupChat, sizeof(tagGroupChat));
    groupChatTodb.dwGroupID = dwFromGroupId;
    groupChatTodb.cbStatus = 0;

    //插入缓存列表
    it->second->InsertGroupChat(&groupChat);

    acl::aio_socket_stream* pToUserProxy = CUserMgr::get_instance().FindUserProxySocksPtr(pGroupChatMsg->dwToUserID);
    if (pToUserProxy != NULL)
    {
        MSG_SC_Send_GroupChatMsg chatMsg;
        chatMsg.dwMainID = MSG_GROUP_MAIN;
        chatMsg.dwSubID = SUB_SC_REC_GROUP_CHAT_MSG;
        chatMsg.dwValue2 = groupChat.dwToUserID;
        chatMsg.dwMsgID =  groupChat.dwMsgId;
        chatMsg.dwToUserID = groupChat.dwToUserID;
        chatMsg.dwFromUserID = groupChat.dwFromUserID;
        chatMsg.dwSendTime = groupChat.dwSendTime;
        _snprintf_info(chatMsg.szContent, sizeof(chatMsg.szContent), "%s", pGroupChatMsg->szContent);
        CNetModule::getSingleton().Send(pToUserProxy, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &chatMsg,
                                        sizeof(MSG_SC_Send_GroupChatMsg));
    }
    /*
    else
    {
        groupChatTodb.cbStatus = 1;
    }
    */

    //保存到数据库
    InsertGroupChatToDB(&groupChatTodb);
}

CT_VOID CGroupMgr::UserReadMsg(const CT_VOID * pData, CT_DWORD dwDataSize)
{
    if (dwDataSize != sizeof(MSG_CS_Read_Group_Msg))
    {
        return;
    }

    MSG_CS_Read_Group_Msg* pReadGroupMsg = (MSG_CS_Read_Group_Msg*)pData;

    auto itGroupId = m_mapUserGroup.find(pReadGroupMsg->dwUserID);
    if (itGroupId == m_mapUserGroup.end())
    {
        return;
    }

    CT_DWORD dwGroupId = itGroupId->second;

    auto itGroup = m_mapGroup.find(dwGroupId);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    MapGroupChat& mapGroupChat = itGroup->second->GetGroupChat();
    auto itChat = mapGroupChat.find(pReadGroupMsg->dwUserID);
    if (itChat != mapGroupChat.end())
    {
        std::vector<tagGroupChat>& vecChat = itChat->second;
        for (auto itVecChat = vecChat.begin(); itVecChat != vecChat.end();)
        {
            MSG_CS2DB_Update_GroupChat updateGroupChat;
            updateGroupChat.dwMsgID = itVecChat->dwMsgId;
            updateGroupChat.cbStatus = 1;
            UpdateGroupChatStatusToDB(&updateGroupChat);
            vecChat.erase(itVecChat);
            LOG(WARNING) << "read msg id: " << updateGroupChat.dwMsgID;
            break;
        }

        if (vecChat.empty())
        {
            LOG(WARNING) << "read msg id2: " << pReadGroupMsg->dwMsgID;
            mapGroupChat.erase(itChat);
        }
    }
}

CT_VOID CGroupMgr::QuerySettleInfo(MSG_CS_Query_Group_Settle_Info* pGroupSettleInfo)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pGroupSettleInfo->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //检测有没有这个玩家
    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(pGroupSettleInfo->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    std::unique_ptr<CGroup>& ptrGroup = it->second;
    std::unique_ptr<tagGroupInfo>& groupInfo = ptrGroup->GetGroupInfo();
    MSG_SC_Query_Group_Settle_Info settleInfo;
    settleInfo.dwMainID = MSG_GROUP_MAIN;
    settleInfo.dwSubID = SUB_SC_QUERY_GROUP_SETTLE_INFO;
    settleInfo.dwValue2 = pGroupSettleInfo->dwUserID;
    settleInfo.cbSettleDays = groupInfo->cbSettleDays;

    CT_DWORD dwCanSettleDate = CalcDayInt(groupInfo->cbSettleDays);
    CT_DWORD dwYesterDay = dwCanSettleDate;
    if (groupInfo->cbSettleDays != 1)
    {
        dwYesterDay = CalcDayInt(1);
    }
    if (dwCanSettleDate > pGroupUserInfo->dwLastTakeIncomeDate)
    {
        settleInfo.dCanSettleIncome = FindDateTotalIncome(ptrGroup, pGroupSettleInfo->dwUserID, pGroupUserInfo->dwLastTakeIncomeDate, dwCanSettleDate) * TO_DOUBLE;
        if (groupInfo->cbSettleDays != 1)
            settleInfo.dRemainingIncome = FindDateTotalIncome(ptrGroup, pGroupUserInfo->dwUserID, pGroupUserInfo->dwLastTakeIncomeDate, dwYesterDay) * TO_DOUBLE;
        else
            settleInfo.dRemainingIncome = settleInfo.dCanSettleIncome;
    }

    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &settleInfo, sizeof(MSG_SC_Query_Group_Settle_Info));
}

CT_VOID CGroupMgr::SettleIncome(MSG_CS_Settle_Income* pSettleIncome)
{
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pSettleIncome->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //检测有没有这个玩家
    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(pSettleIncome->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    std::unique_ptr<CGroup>& ptrGroup = it->second;
    std::unique_ptr<tagGroupInfo>& groupInfo = ptrGroup->GetGroupInfo();

    tagGroupLevelCfg* pGroupLevelCfg = GetLevelCfg(groupInfo->cbLevel);
    if (pGroupLevelCfg == NULL)
    {
        return;
    }

    CT_DWORD dwCanSettleDate = CalcDayInt(groupInfo->cbSettleDays);
    CT_DWORD dwYesterDay = CalcDayInt(1);

    MSG_SC_Settle_Income settleIncome;
    settleIncome.dwMainID = MSG_GROUP_MAIN;
    settleIncome.dwSubID = SUB_SC_SETTLE_INCOME;
    settleIncome.dwValue2 = pSettleIncome->dwUserID;

    if (dwCanSettleDate <= pGroupUserInfo->dwLastTakeIncomeDate)
    {
        if (dwYesterDay > dwCanSettleDate)
        {
            CT_LONGLONG llIncome = FindDateTotalIncome(ptrGroup, pSettleIncome->dwUserID, pGroupUserInfo->dwLastTakeIncomeDate, dwYesterDay);
            settleIncome.dRemainingIncome = llIncome*TO_DOUBLE;
        }
        settleIncome.cbResult = 1;
    }
    else
    {
        CT_LONGLONG llIncome = FindDateTotalIncome(ptrGroup, pSettleIncome->dwUserID, pGroupUserInfo->dwLastTakeIncomeDate, dwCanSettleDate);

        if (llIncome < pGroupLevelCfg->dwSettleMinIncome)
        {
            settleIncome.dRemainingIncome = llIncome*TO_DOUBLE;
            settleIncome.dSettleMinIncome = pGroupLevelCfg->dwSettleMinIncome*TO_DOUBLE;
            settleIncome.cbResult = 2;
            CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &settleIncome, sizeof(MSG_SC_Settle_Income));
            return;
        }

        if (llIncome > 0)
        {
            pGroupUserInfo->dwLastTakeIncomeDate = CalcDayInt(1);
            MSG_CS2DB_Insert_Exchange_GroupIncome exchangeGroupIncome;
            exchangeGroupIncome.dwUserID= pSettleIncome->dwUserID;
            exchangeGroupIncome.dwGroupID = groupInfo->dwGroupID;
            exchangeGroupIncome.llIncome = llIncome;
            exchangeGroupIncome.dwTime = (CT_DWORD)time(NULL);
            CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_INSERT_GROUP_EXCHANGE, &exchangeGroupIncome, sizeof(MSG_CS2DB_Insert_Exchange_GroupIncome));

            //结算红利增加贡献度(每满100块增加2点)
            CT_LONGLONG llAddContribution = (llIncome / 10000)*2;
            if (llAddContribution > 0)
                UpdateGroupContribution(pUserInfo->dwGroupID, (CT_DWORD)llAddContribution);

            //重新查可结算数据
            if (dwYesterDay > dwCanSettleDate)
                settleIncome.dRemainingIncome = FindDateTotalIncome(ptrGroup, pSettleIncome->dwUserID, pGroupUserInfo->dwLastTakeIncomeDate, dwCanSettleDate)*TO_DOUBLE;
            UpdateGroupUserInfoToDB(pUserInfo->dwGroupID, pGroupUserInfo);
        }
        else
        {
            if (dwYesterDay > dwCanSettleDate)
                settleIncome.dRemainingIncome = FindDateTotalIncome(ptrGroup, pSettleIncome->dwUserID, pGroupUserInfo->dwLastTakeIncomeDate, dwYesterDay)*TO_DOUBLE;
            settleIncome.cbResult = 1;
        }
    }

    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &settleIncome, sizeof(MSG_SC_Settle_Income));
}

CT_VOID CGroupMgr::CreatePRoom(MSG_CS_Create_Group_PRoom * pCreatePRoom)
{
    //判断玩家
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pCreatePRoom->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //检测有没有这个玩家
    tagGroupUserInfo* pGroupUserInfo = it->second->GetGroupUserInfo(pCreatePRoom->dwUserID);
    if (pGroupUserInfo == NULL)
    {
        return;
    }

    MSG_SC_Create_Group_PRoom createGroupPRoom;
    createGroupPRoom.dwMainID = MSG_GROUP_MAIN;
    createGroupPRoom.dwSubID = SUB_SC_CREATE_GROUP_PROOM;
    createGroupPRoom.dwValue2 = pCreatePRoom->dwUserID;

    //查找房间服务器信息
    CMD_GameServer_Info* pGameServer = CServerMgr::get_instance().FindSuitTableGameServer(pCreatePRoom->wGameID, pCreatePRoom->wKindID, pCreatePRoom->wRoomKindID);
    if (pGameServer == NULL)
    {
        createGroupPRoom.cbResult = 2;
        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &createGroupPRoom, sizeof(MSG_SC_Create_Group_PRoom));
        return;
    }

    //判断玩家是否足够
    if (pGameServer->dwEnterMinScore > pUserInfo->llScore)
    {
        createGroupPRoom.cbResult = 3;
        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &createGroupPRoom, sizeof(MSG_SC_Create_Group_PRoom));
        return;
    }

    //获取所属群组的所有房间信息
    MapGroupPRoom& mapGroupPRoom = it->second->GetGroupPRoom();
    //先判断是否足够的房间
    CT_DWORD dwGameIndex = pCreatePRoom->wGameID*10000+pCreatePRoom->wKindID*100;
    auto itGroupPRoom = mapGroupPRoom.find(dwGameIndex);
    if (itGroupPRoom != mapGroupPRoom.end())
    {
        VecGroupPRoom& vecGroupPRoom = itGroupPRoom->second;
        if (vecGroupPRoom.size() >= 5)
        {
            createGroupPRoom.cbResult = 1;
            CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &createGroupPRoom, sizeof(MSG_SC_Create_Group_PRoom));
            return;
        }
    }

    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> disRoomNum(10000, 99999);

    CT_DWORD dwRoomNum = 0;
    int nRandCount = 0;
    while (true)
    {
        dwRoomNum = disRoomNum(gen);
        auto itRoomNum = m_mapRoomNum.find(dwRoomNum);
        if (itRoomNum == m_mapRoomNum.end())
        {
            break;
        }

        ++nRandCount;
        if (nRandCount > 10000)
        {
            dwRoomNum = 0;
            break;
        }
    }

    if (dwRoomNum == 0)
    {
        createGroupPRoom.cbResult = 2;
        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &createGroupPRoom, sizeof(MSG_SC_Create_Group_PRoom));
        return;
    }

    //插入房间信息
    tagGroupPRoomInfo groupPRoomInfo;
    groupPRoomInfo.wGameID = pCreatePRoom->wGameID;
    groupPRoomInfo.wKindID = pCreatePRoom->wKindID;
    groupPRoomInfo.wRoomKindID = pCreatePRoom->wRoomKindID;
    groupPRoomInfo.dwRoomNum = dwRoomNum;
    groupPRoomInfo.dwServerID = pGameServer->dwServerID;
    groupPRoomInfo.cbLock = pCreatePRoom->cbLock;
    groupPRoomInfo.dwOwnerID = pCreatePRoom->dwUserID;
    VecGroupPRoom& vecGroupPRoom = mapGroupPRoom[dwGameIndex];
    vecGroupPRoom.push_back(groupPRoomInfo);

    m_mapRoomNum.insert(std::make_pair(dwRoomNum, pUserInfo->dwGroupID));

    createGroupPRoom.wGameID = pCreatePRoom->wGameID;
    createGroupPRoom.wKindID = pCreatePRoom->wKindID;
    createGroupPRoom.dwRoomNum = dwRoomNum;
    createGroupPRoom.dwServerID = pGameServer->dwServerID;
    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &createGroupPRoom, sizeof(MSG_SC_Create_Group_PRoom));

    //新增房间同步GS
    SendAllGameServerAddPRoom(pUserInfo->dwGroupID, &groupPRoomInfo);
    //新增房间同步PS
    SendAllProxyServerAddPRoom(&groupPRoomInfo);

    //给玩家同步新增房间(是否考虑)
    MapGroupOpenUIUser& mapGroupOpenUiUser = it->second->GetGroupOpenUIUserID();
    if (!mapGroupOpenUiUser.empty())
    {
        CT_BYTE cbBuffer[SYS_NET_SENDBUF_CLIENT];
        CT_DWORD dwSendSize = 0;
        MSG_GameMsgDownHead gameMsgDownHead;
        gameMsgDownHead.dwMainID = MSG_GROUP_MAIN;
        gameMsgDownHead.dwSubID = SUB_SC_PROOM_ADD;

        MSG_SC_Group_PRoomID groupPRoomId;
        groupPRoomId.wGameID = groupPRoomInfo.wGameID;
        groupPRoomId.wKindID = groupPRoomInfo.wKindID;

        MSG_SC_Group_PRoom_List groupPRoomList;
        groupPRoomList.wRoomKindID = groupPRoomInfo.wRoomKindID;
        groupPRoomList.dwRoomNum = groupPRoomInfo.dwRoomNum;
        groupPRoomList.dwOwnerID = groupPRoomInfo.dwOwnerID;
        groupPRoomList.dwServerID = groupPRoomInfo.dwServerID;
        groupPRoomList.cbLock = groupPRoomInfo.cbLock;
        for (int i = 0; i < MAX_PLAYER; ++i)
        {
            groupPRoomList.dwUserID[i] = groupPRoomInfo.userInfo[i].dwUserID;
            groupPRoomList.cbHeadID[i] = groupPRoomInfo.userInfo[i].cbHeadId;
            groupPRoomList.cbSex[i] = groupPRoomInfo.userInfo[i].cbSex;
            groupPRoomList.cbVip2[i] = groupPRoomInfo.userInfo[i].cbVip2;
        }

        for (auto itOpenUI = mapGroupOpenUiUser.begin(); itOpenUI != mapGroupOpenUiUser.end(); )
        {
            if (itOpenUI->first == pCreatePRoom->dwUserID)
            {
                ++itOpenUI;
                continue;
            }

            if (itOpenUI->second != dwGameIndex)
            {
                ++itOpenUI;
                continue;
            }

            pProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(itOpenUI->first);
            if (pProxySock == NULL)
            {
                itOpenUI = mapGroupOpenUiUser.erase(itOpenUI);
            }
            else
            {
                dwSendSize = 0;
                gameMsgDownHead.dwValue2 = itOpenUI->first;
                memcpy(cbBuffer, &gameMsgDownHead, sizeof(MSG_GameMsgDownHead));
                dwSendSize += sizeof(MSG_GameMsgDownHead);
                memcpy(cbBuffer+dwSendSize, &groupPRoomId, sizeof(MSG_SC_Group_PRoomID));
                dwSendSize += sizeof(MSG_SC_Group_PRoomID);
                memcpy(cbBuffer+dwSendSize, &groupPRoomList, sizeof(MSG_SC_Group_PRoom_List));
                dwSendSize += sizeof(MSG_SC_Group_PRoom_List);
                CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &cbBuffer, dwSendSize);
                ++itOpenUI;
            }
        }
    }
}

CT_VOID CGroupMgr::OpenGameUI(MSG_CS_Open_Group_Game * pOpenGroupGameUI)
{
    //判断玩家
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pOpenGroupGameUI->dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
    if (pProxySock == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    //设定玩家打开UI
    CT_DWORD dwGameIndex = pOpenGroupGameUI->wGameID*10000+pOpenGroupGameUI->wKindID*100;
    MapGroupOpenUIUser& mapGroupOpenUiUser = it->second->GetGroupOpenUIUserID();
    mapGroupOpenUiUser[pOpenGroupGameUI->dwUserID] = dwGameIndex;

    CT_BYTE cbBuffer[SYS_NET_SENDBUF_CLIENT];
    CT_DWORD dwSendSize = 0;
    MSG_GameMsgDownHead gameMsgDownHead;
    gameMsgDownHead.dwMainID = MSG_GROUP_MAIN;
    gameMsgDownHead.dwSubID = SUB_SC_PROOM_LIST;
    gameMsgDownHead.dwValue2 = pOpenGroupGameUI->dwUserID;
    memcpy(cbBuffer, &gameMsgDownHead, sizeof(MSG_GameMsgDownHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

    MSG_SC_Group_PRoomID groupPRoomId;
    groupPRoomId.wGameID = pOpenGroupGameUI->wGameID;
    groupPRoomId.wKindID = pOpenGroupGameUI->wKindID;

    memcpy(cbBuffer+dwSendSize, &groupPRoomId, sizeof(MSG_SC_Group_PRoomID));
    dwSendSize += sizeof(MSG_SC_Group_PRoomID);

    CT_DWORD dwListSize = 0;
    MapGroupPRoom& mapGroupPRoom = it->second->GetGroupPRoom();
    auto itPRoom = mapGroupPRoom.find(dwGameIndex);
    if (itPRoom != mapGroupPRoom.end())
    {
        MSG_SC_Group_PRoom_List groupPRoomList;
        VecGroupPRoom& vecGroupPRoom = itPRoom->second;
        for (auto& itVecPRoom : vecGroupPRoom)
        {
            if (dwSendSize+dwListSize + sizeof(MSG_SC_Group_PRoom_List) >= SYS_NET_SENDBUF_CLIENT)
            {
                CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &cbBuffer, dwSendSize+dwListSize);
                dwListSize = 0;
            }
            groupPRoomList.wRoomKindID = itVecPRoom.wRoomKindID;
            groupPRoomList.dwRoomNum = itVecPRoom.dwRoomNum;
            groupPRoomList.dwOwnerID = itVecPRoom.dwOwnerID;
            groupPRoomList.dwServerID = itVecPRoom.dwServerID;
            groupPRoomList.cbLock = itVecPRoom.cbLock;
            for (int i = 0; i < MAX_PLAYER; ++i)
            {
                groupPRoomList.dwUserID[i] = itVecPRoom.userInfo[i].dwUserID;
                groupPRoomList.cbHeadID[i] = itVecPRoom.userInfo[i].cbHeadId;
                groupPRoomList.cbSex[i] = itVecPRoom.userInfo[i].cbSex;
                groupPRoomList.cbVip2[i] = itVecPRoom.userInfo[i].cbVip2;
            }
            memcpy(cbBuffer+dwSendSize+dwListSize, &groupPRoomList, sizeof(MSG_SC_Group_PRoom_List));
            dwListSize += sizeof(MSG_SC_Group_PRoom_List);
        }
    }

    CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &cbBuffer, dwSendSize+dwListSize);
}

CT_VOID CGroupMgr::CloseGameUI(CT_DWORD dwUserID)
{
    //判断玩家
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    //本身没有工会则返回
    if (pUserInfo->dwGroupID == 0)
    {
        return;
    }

    auto it = m_mapGroup.find(pUserInfo->dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    MapGroupOpenUIUser& mapGroupOpenUiUser = it->second->GetGroupOpenUIUserID();
    auto itOpenUser = mapGroupOpenUiUser.find(dwUserID);
    if (itOpenUser != mapGroupOpenUiUser.end())
    {
        mapGroupOpenUiUser.erase(itOpenUser);
    }
}

CT_VOID CGroupMgr::UpdateGroupContribution(CT_DWORD dwGroupID, CT_DWORD dwAddContribution)
{
    auto it = m_mapGroup.find(dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    std::unique_ptr<tagGroupInfo>& groupInfo = it->second->GetGroupInfo();
    if (groupInfo->dwContribution >= m_dwMaxGroupContribution)
    {
        return;
    }

    groupInfo->dwContribution += dwAddContribution;
    if (groupInfo->dwContribution > m_dwMaxGroupContribution)
    {
        groupInfo->dwContribution = m_dwMaxGroupContribution;
    }

    //更新最新的贡献度给会长
    acl::aio_socket_stream* pMasterProxy = CUserMgr::get_instance().FindUserProxySocksPtr(groupInfo->dwMasterID);
    if (pMasterProxy)
    {
        MSG_SC_Update_Group_Contribution updateGroupContribution;
        updateGroupContribution.dwMainID = MSG_GROUP_MAIN;
        updateGroupContribution.dwSubID = SUB_SC_UPDATE_GROUP_CONTRIBUTION;
        updateGroupContribution.dwValue2 = groupInfo->dwMasterID;
        updateGroupContribution.dwContribution = groupInfo->dwContribution;
        CNetModule::getSingleton().Send(pMasterProxy, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateGroupContribution,
                                        sizeof(MSG_SC_Update_Group_Contribution));
    }

    //更新到数据库
    MSG_CS2DB_Update_GroupContribution addGroupContribution;
    addGroupContribution.dwGroupID = dwGroupID;
    addGroupContribution.dwAddContribution = dwAddContribution;
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_GROUP_CONTRIBUTION, &addGroupContribution, sizeof(MSG_CS2DB_Update_GroupContribution));
}

CT_VOID CGroupMgr::UpdateGroupPRoomUser(MSG_G2CS_GroupPRoom_Update* pRoomUpdate)
{
    if (pRoomUpdate->wChairID >= MAX_PLAYER)
        return;

    auto itGroupRoom = m_mapRoomNum.find(pRoomUpdate->dwRoomNum);
    if (itGroupRoom == m_mapRoomNum.end())
    {
        return;
    }

    CT_DWORD dwGroupID = itGroupRoom->second;
    auto it = m_mapGroup.find(dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    CT_DWORD dwGameIndex = pRoomUpdate->wGameID*10000 + pRoomUpdate->wKindID*100;
    MapGroupPRoom& mapGroupPRoom = it->second->GetGroupPRoom();
    auto itMapPRoom = mapGroupPRoom.find(dwGameIndex);
    if (itMapPRoom == mapGroupPRoom.end())
        return;

    VecGroupPRoom& vecGroupPRoom = itMapPRoom->second;
    for (auto& itVecPRoom : vecGroupPRoom)
    {
        if (itVecPRoom.dwRoomNum == pRoomUpdate->dwRoomNum)
        {
            tagGroupPRoomInfo& groupPRoomInfo = itVecPRoom;
            groupPRoomInfo.userInfo[pRoomUpdate->wChairID].dwUserID = pRoomUpdate->dwUserID;
            groupPRoomInfo.userInfo[pRoomUpdate->wChairID].cbHeadId = pRoomUpdate->cbHeadId;
            groupPRoomInfo.userInfo[pRoomUpdate->wChairID].cbSex = pRoomUpdate->cbSex;
            groupPRoomInfo.userInfo[pRoomUpdate->wChairID].cbVip2 = pRoomUpdate->cbVip2;

            //判断人数
            CT_WORD wSitUserCount = 0;
            for (int i = 0; i < MAX_PLAYER; ++i)
            {
                if (groupPRoomInfo.userInfo[i].dwUserID != 0)
                {
                    ++wSitUserCount;
                }
            }
            groupPRoomInfo.wSitUserCount = wSitUserCount;
            if (groupPRoomInfo.wSitUserCount == 0)
            {
                groupPRoomInfo.dwEmptyTime = (CT_DWORD)time(NULL);
            }

            MapGroupOpenUIUser& mapGroupOpenUiUser = it->second->GetGroupOpenUIUserID();
            if (!mapGroupOpenUiUser.empty())
            {
                CT_BYTE cbBuffer[SYS_NET_SENDBUF_CLIENT];
                CT_DWORD dwSendSize = 0;
                MSG_GameMsgDownHead gameMsgDownHead;
                gameMsgDownHead.dwMainID = MSG_GROUP_MAIN;
                gameMsgDownHead.dwSubID = SUB_SC_PROOM_UPDATE;

                MSG_SC_Group_PRoomID groupPRoomId;
                groupPRoomId.wGameID = groupPRoomInfo.wGameID;
                groupPRoomId.wKindID = groupPRoomInfo.wKindID;

                MSG_SC_Group_PRoom_List groupPRoomList;
                groupPRoomList.wRoomKindID = groupPRoomInfo.wRoomKindID;
                groupPRoomList.dwRoomNum = groupPRoomInfo.dwRoomNum;
                groupPRoomList.dwOwnerID = groupPRoomInfo.dwOwnerID;
                groupPRoomList.dwServerID = groupPRoomInfo.dwServerID;
                groupPRoomList.cbLock = groupPRoomInfo.cbLock;
                for (int i = 0; i < MAX_PLAYER; ++i)
                {
                    groupPRoomList.dwUserID[i] = groupPRoomInfo.userInfo[i].dwUserID;
                    groupPRoomList.cbHeadID[i] = groupPRoomInfo.userInfo[i].cbHeadId;
                    groupPRoomList.cbSex[i] = groupPRoomInfo.userInfo[i].cbSex;
                    groupPRoomList.cbVip2[i] = groupPRoomInfo.userInfo[i].cbVip2;
                }

                for (auto itOpenUI = mapGroupOpenUiUser.begin(); itOpenUI != mapGroupOpenUiUser.end(); )
                {
                    if (itOpenUI->second != dwGameIndex)
                    {
                        ++itOpenUI;
                        continue;
                    }

                    acl::aio_socket_stream* pProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(itOpenUI->first);
                    if (pProxySock == NULL)
                    {
                        itOpenUI = mapGroupOpenUiUser.erase(itOpenUI);
                    }
                    else
                    {
                        dwSendSize = 0;
                        gameMsgDownHead.dwValue2 = itOpenUI->first;
                        memcpy(cbBuffer, &gameMsgDownHead, sizeof(MSG_GameMsgDownHead));
                        dwSendSize += sizeof(MSG_GameMsgDownHead);
                        memcpy(cbBuffer+dwSendSize, &groupPRoomId, sizeof(MSG_SC_Group_PRoomID));
                        dwSendSize += sizeof(MSG_SC_Group_PRoomID);
                        memcpy(cbBuffer+dwSendSize, &groupPRoomList, sizeof(MSG_SC_Group_PRoom_List));
                        dwSendSize += sizeof(MSG_SC_Group_PRoom_List);
                        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &cbBuffer, dwSendSize);
                        ++itOpenUI;
                    }
                }
            }
            break;
        }
    }
}

CT_VOID CGroupMgr::RemoveGroupPRoom(MSG_G2CS_Remove_GroupPRoom* pRemovePRoom)
{
    auto itGroupRoom = m_mapRoomNum.find(pRemovePRoom->dwRoomNum);
    if (itGroupRoom == m_mapRoomNum.end())
    {
        return;
    }
    CT_DWORD dwGroupID = itGroupRoom->second;
    auto it = m_mapGroup.find(dwGroupID);
    if (it == m_mapGroup.end())
    {
        return;
    }

    m_mapRoomNum.erase(itGroupRoom);
    SendAllProxyServerRemovePRoom(pRemovePRoom->dwRoomNum);

    CT_DWORD dwGameIndex = pRemovePRoom->wGameID*10000 + pRemovePRoom->wKindID*100;
    MapGroupPRoom& mapGroupPRoom = it->second->GetGroupPRoom();
    auto itMapPRoom = mapGroupPRoom.find(dwGameIndex);
    if (itMapPRoom == mapGroupPRoom.end())
        return;

    VecGroupPRoom& vecGroupPRoom = itMapPRoom->second;
    for (auto itVecPRoom = vecGroupPRoom.begin(); itVecPRoom != vecGroupPRoom.end(); ++itVecPRoom)
    {
        if (itVecPRoom->dwRoomNum == pRemovePRoom->dwRoomNum)
        {
            MapGroupOpenUIUser& mapGroupOpenUiUser = it->second->GetGroupOpenUIUserID();
            if (!mapGroupOpenUiUser.empty())
            {
                MSG_SC_Group_PRoom_Remove removePRoom;
                removePRoom.dwMainID = MSG_GROUP_MAIN;
                removePRoom.dwSubID = SUB_SC_PROOM_REMOVE;
                removePRoom.wGameID= pRemovePRoom->wGameID;
                removePRoom.wKindID = pRemovePRoom->wKindID;
                removePRoom.dwRoomNum = itVecPRoom->dwRoomNum;

                for (auto itOpenUI = mapGroupOpenUiUser.begin(); itOpenUI != mapGroupOpenUiUser.end(); )
                {
                    if (itOpenUI->second != dwGameIndex)
                    {
                        ++itOpenUI;
                        continue;
                    }

                    acl::aio_socket_stream* pProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(itOpenUI->first);
                    if (pProxySock == NULL)
                    {
                        itOpenUI = mapGroupOpenUiUser.erase(itOpenUI);
                    }
                    else
                    {
                        removePRoom.dwValue2 = itOpenUI->first;
                        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removePRoom, sizeof(MSG_SC_Group_PRoom_Remove));
                        ++itOpenUI;
                    }
                }
            }

            vecGroupPRoom.erase(itVecPRoom);
            break;
        }
    }
}

CT_VOID CGroupMgr::UpdateGroupInfoForDip(const CT_VOID * pData, CT_DWORD dwDataSize)
{
    if (dwDataSize != sizeof(tagGroupInfo))
    {
        return;
    }

    tagGroupInfo* pGroupInfo = (tagGroupInfo*)pData;

    auto itGroup = m_mapGroup.find(pGroupInfo->dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    std::unique_ptr<tagGroupInfo>& ptrGroupInfo = itGroup->second->GetGroupInfo();
    CT_BYTE cbOldIncomeRate = ptrGroupInfo->cbIncomeRate;
    //ptrGroupInfo->dwMasterID = pGroupInfo->dwMasterID;
    //ptrGroupInfo->cbIcon = pGroupInfo->cbIcon;
    ptrGroupInfo->cbLevel = pGroupInfo->cbLevel;
    ptrGroupInfo->dwContribution = pGroupInfo->dwContribution;
    ptrGroupInfo->cbSettleDays = pGroupInfo->cbSettleDays;
    _snprintf_info(ptrGroupInfo->szGroupName, sizeof(ptrGroupInfo->szGroupName), "%s", pGroupInfo->szGroupName);
    _snprintf_info(ptrGroupInfo->szNotice, sizeof(ptrGroupInfo->szNotice), "%s", pGroupInfo->szNotice);

    //如果提升工会的收入比例，则提升群主的的比例
    if (pGroupInfo->cbIncomeRate > cbOldIncomeRate)
    {
        ptrGroupInfo->cbIncomeRate = pGroupInfo->cbIncomeRate;
        tagGroupUserInfo* pGroupUserInfo = itGroup->second->GetGroupUserInfo(ptrGroupInfo->dwMasterID);
        if (pGroupUserInfo)
        {
            pGroupUserInfo->cbInComeRate = ptrGroupInfo->cbIncomeRate;
            UpdateGroupUserInfoToDB(ptrGroupInfo->dwGroupID, pGroupUserInfo);
        }

        acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(ptrGroupInfo->dwMasterID);
        if (pProxySocket)
        {
            tagGroupLevelCfg* pGroupLevelCfg = GetLevelCfg(ptrGroupInfo->cbLevel+1);
            if (pGroupLevelCfg == NULL)
            {
                return;
            }
            MSG_SC_Upgrade_Group upgradeGroup;
            upgradeGroup.dwMainID = MSG_GROUP_MAIN;
            upgradeGroup.dwSubID = SUB_SC_UPGRADE_GROUP;
            upgradeGroup.cbLevel = ptrGroupInfo->cbLevel;
            upgradeGroup.cbSettleDays = ptrGroupInfo->cbSettleDays;
            upgradeGroup.cbIncomeRate = ptrGroupInfo->cbIncomeRate;
            upgradeGroup.dwNextContribution = pGroupLevelCfg->dwMaxContribution;
            upgradeGroup.dwValue2 = ptrGroupInfo->dwMasterID;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &upgradeGroup, sizeof(MSG_SC_Upgrade_Group));
        }
    }
}

CT_VOID CGroupMgr::UpdateGroupUserInfoForDip(const CT_VOID * pData, CT_DWORD dwDataSize)
{
    if (dwDataSize != sizeof(tagGroupUserInfo) + sizeof(CT_DWORD))
    {
        return;
    }

    CT_DWORD dwGroupID = *((CT_DWORD*)pData);

    tagGroupUserInfo* pNewGroupUserInfo = (tagGroupUserInfo*)(((CT_BYTE*)pData + sizeof(CT_DWORD)));

    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    tagGroupUserInfo* pGroupUserInfo = itGroup->second->GetGroupUserInfo(pNewGroupUserInfo->dwUserID);
    if (pGroupUserInfo)
    {
       pGroupUserInfo->cbInComeRate = pNewGroupUserInfo->cbInComeRate;
       pGroupUserInfo->wDeductRate = pNewGroupUserInfo->wDeductRate;
       pGroupUserInfo->wDeductTop  = pNewGroupUserInfo->wDeductTop;
       pGroupUserInfo->wDeductIncre = pNewGroupUserInfo->wDeductIncre;
       pGroupUserInfo->dwDeductStart = pNewGroupUserInfo->dwDeductStart;
       //pGroupUserInfo->dwLastTakeIncomeDate = pNewGroupUserInfo->dwLastTakeIncomeDate;
       //pGroupUserInfo->dwLastIncomeDate = pNewGroupUserInfo->dwLastIncomeDate;
       //pGroupUserInfo->wLinkIncomeDays = pNewGroupUserInfo->wLinkIncomeDays;
       //pGroupUserInfo->llTotalIncome = pNewGroupUserInfo->llTotalIncome;
    }
}

CT_VOID CGroupMgr::UpdateGroupLevelConfigForDip(const CT_VOID * pData, CT_DWORD dwDataSize)
{
    if (dwDataSize != sizeof(tagGroupLevelCfg))
    {
        return;
    }

    tagGroupLevelCfg* pNewGroupLevelCfg = (tagGroupLevelCfg*)pData;
    tagGroupLevelCfg* pGroupLevelCfg = GetLevelCfg(pNewGroupLevelCfg->cbLevel);

    if (pGroupLevelCfg != NULL)
    {
        if (strcmp(pGroupLevelCfg->szDomain, pNewGroupLevelCfg->szDomain) != 0)
        {
            MSG_SC_Update_GroupPromotion_Domain updateGroupPromotionDomain;
            updateGroupPromotionDomain.dwMainID = MSG_GROUP_MAIN;
            updateGroupPromotionDomain.dwSubID = SUB_SC_UPDATE_GROUP_PROMOTION_DOMAIN;
            _snprintf_info(updateGroupPromotionDomain.szDomain, sizeof(updateGroupPromotionDomain.szDomain), "%s", pNewGroupLevelCfg->szDomain);

            for (auto& itGroup : m_mapGroup)
            {
                std::unique_ptr<tagGroupInfo>& ptrGroupInfo = itGroup.second->GetGroupInfo();
                if (ptrGroupInfo->cbLevel == pNewGroupLevelCfg->cbLevel)
                {
                    acl::aio_socket_stream* pProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(ptrGroupInfo->dwMasterID);
                    if (pProxySock)
                    {
                        updateGroupPromotionDomain.dwValue2 = ptrGroupInfo->dwMasterID;
                        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &updateGroupPromotionDomain, sizeof(MSG_SC_Update_GroupPromotion_Domain));
                    }
                }
            }
        }
    }

    InsertGroupLevelConfig(pNewGroupLevelCfg);
}

/*std::unique_ptr<tagGroupRoomConfig>* CGroupMgr::GetGroupRoomCfg(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->GetGroupRoomCfg(wGameID, wKindID);
	}
	return NULL;
}*/

/*std::shared_ptr<tagDefaultRoomConfig>* CGroupMgr::GetDefaultRoomCfg(CT_WORD wGameID, CT_WORD wKindID)
{
	for (auto& it : m_vecRoomDefaultCfg)
	{
		if (it->wGameID == wGameID && it->wKindID == wKindID)
		{
			return &it;
		}
	}

	return NULL;
}*/

CT_VOID CGroupMgr::SendGroupToGameServer(acl::aio_socket_stream* pSocket, CMD_GameServer_Info* pGameServerInfo)
{
	//发送群组信息
	CT_DWORD dwGameIndex = pGameServerInfo->wGameID*10000+pGameServerInfo->wKindID*100;
	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
	CT_DWORD dwSendSize = 0;
	for (auto&it : m_mapGroup)
	{
		std::unique_ptr<tagGroupInfo>& pGroupInfo = it.second->GetGroupInfo();
		//发送数据
		if ((dwSendSize + sizeof(tagGroupInfo)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_GROUP_INFO, szBuffer, dwSendSize);
			dwSendSize = 0;
		}

		memcpy(szBuffer + dwSendSize, pGroupInfo.get(), sizeof(tagGroupInfo));
		dwSendSize += sizeof(tagGroupInfo);
	}
	if (dwSendSize > 0)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_GROUP_INFO, szBuffer, dwSendSize);
	}

	dwSendSize = 0;
	for (auto& it : m_mapGroup)
	{
		std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& groupUserInfo = it.second->GetAllGroupUserInfo();
		memcpy(szBuffer, &it.first, sizeof(CT_DWORD));
		dwSendSize += sizeof(CT_DWORD);
		for (auto& itUser : groupUserInfo)
		{
			//发送数据
			if ((dwSendSize + sizeof(tagGroupUserInfo)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_GROUP_USRE_INFO, szBuffer, dwSendSize);
				dwSendSize = 0;
				memcpy(szBuffer, &it.first, sizeof(CT_DWORD));
				dwSendSize += sizeof(CT_DWORD);
			}

			memcpy(szBuffer + dwSendSize, &itUser, sizeof(tagGroupUserInfo));
			dwSendSize += sizeof(tagGroupUserInfo);
		}
		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_GROUP_USRE_INFO, szBuffer, dwSendSize);
			dwSendSize = 0;
		}

        MapGroupPRoom& mapGroupPRoom = it.second->GetGroupPRoom();
		auto itMapGroupPRoom = mapGroupPRoom.find(dwGameIndex);
		if (itMapGroupPRoom != mapGroupPRoom.end())
        {
            VecGroupPRoom& vecGroupPRoom = itMapGroupPRoom->second;
            for (auto& itVecPRoom : vecGroupPRoom)
            {
                if (itVecPRoom.dwServerID == pGameServerInfo->dwServerID)
                {
                    CGroupMgr::get_instance().SendAllGameServerAddPRoom(it.first, &itVecPRoom);
                }
            }
        }
    }
}

CT_VOID	CGroupMgr::UpdateGroupInfoToGameServer(acl::aio_socket_stream* pSocket, CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		return;
	}
	//std::unique_ptr<tagGroupInfo>& groupInfo = it->second->GetGroupInfo();

	MSG_C2GS_Update_GroupInfo updateGroupInfo;
	updateGroupInfo.dwGroupID = dwGroupID;
	//updateGroupInfo.dwBindUserPlay = groupInfo->wBindUserPlay;
	CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_UPDATE_GROUP_INFO, &updateGroupInfo, sizeof(updateGroupInfo));
}

std::unique_ptr<CGroup>* CGroupMgr::GetGroup(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return &it->second;
	}
	return NULL;
}

CT_DWORD CGroupMgr::GetLevelMaxContribution(CT_BYTE cbLevel)
{
    auto it = m_mapGroupLevelCfg.find(cbLevel);
    if (it == m_mapGroupLevelCfg.end())
    {
        return 999999999;
    }

    return it->second.dwMaxContribution;
}

tagGroupLevelCfg*  CGroupMgr::GetLevelCfg(CT_BYTE cbLevel)
{
    auto it = m_mapGroupLevelCfg.find(cbLevel);
    if (it != m_mapGroupLevelCfg.end())
    {
        return &it->second;
    }
    return NULL;
}

CT_WORD CGroupMgr::GetBaseIncomeRate()
{
    auto it = m_mapGroupLevelCfg.find(1);
    if (it == m_mapGroupLevelCfg.end())
    {
        return 0;
    }

    return it->second.wIncomeRate;
}

CT_BYTE  CGroupMgr::GetBaseSettleDays()
{
    auto it = m_mapGroupLevelCfg.find(1);
    if (it == m_mapGroupLevelCfg.end())
    {
        return 3;
    }

    return it->second.cbSettleDays;
}

CT_VOID CGroupMgr::SendAllGroupInfoToClient(CT_DWORD dwUserID)
{
    auto it = m_mapUserGroup.find(dwUserID);
    if (it == m_mapUserGroup.end())
    {
        return;
    }

    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
    if (pUserInfo == NULL)
    {
        return;
    }

    //设置玩家群组ID
    CT_DWORD dwGroupID = it->second;
    if (pUserInfo->dwGroupID == 0)
    {
        pUserInfo->dwGroupID = dwGroupID;
    }

	//查找玩家proxy信息
	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	//设置玩家在线
    SetGroupUserOnline(dwGroupID, dwUserID, true);

	////////////////////////////////以下是群列表///////////////////////////////////////////
	//群组信息
	static CT_BYTE szBuffer[SYS_NET_SENDBUF_CLIENT] = { 0 };
	CT_DWORD dwSendSize = 0;

	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_GROUP_MAIN;
	msgHead.dwSubID = SUB_SC_GROUP_LIST;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }
    std::unique_ptr<tagGroupInfo>& pGroupInfo = itGroup->second->GetGroupInfo();
    //MapGroupUser& mapGroupUser = itGroup->second->GetAllGroupUserInfo();

    MSG_SC_GroupInfo groupInfo;
    groupInfo.dwGroupID = pGroupInfo->dwGroupID;
    groupInfo.dwMasterID = pGroupInfo->dwMasterID;
    groupInfo.cbIcon = pGroupInfo->cbIcon;
    groupInfo.cbLevel = pGroupInfo->cbLevel;
    groupInfo.dwContribution = pGroupInfo->dwContribution;
    //查找本等级最大值贡献值(下一个等级的贡献值开始值)
    tagGroupLevelCfg* pGroupLevelCfg = GetLevelCfg(pGroupInfo->cbLevel);
    groupInfo.dwNextContribution = (pGroupLevelCfg != NULL ? pGroupLevelCfg->dwMaxContribution : 999999999); //GetLevelMaxContribution(pGroupInfo->cbLevel);
    groupInfo.dwGroupUserCount = pGroupInfo->wUserCount;
    groupInfo.cbIncomeRate = pGroupInfo->cbIncomeRate;
    groupInfo.cbSettleDays = pGroupInfo->cbSettleDays;
    _snprintf_info(groupInfo.szGroupName, sizeof(groupInfo.szGroupName), "%s", pGroupInfo->szGroupName);
    _snprintf_info(groupInfo.szNotice, sizeof(groupInfo.szNotice), "%s", pGroupInfo->szNotice);
    _snprintf_info(groupInfo.szDomain, sizeof(groupInfo.szDomain), "%s", pGroupLevelCfg != NULL ? pGroupLevelCfg->szDomain : "http://test.cdn.ske666.com/");

    memcpy(szBuffer + dwSendSize, &groupInfo, sizeof(MSG_SC_GroupInfo));
    dwSendSize += sizeof(MSG_SC_GroupInfo);
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);

	////////////////////////////////以下是群成员列表///////////////////////////////////////////
	tagGroupUserInfo* pGroupUserInfo = itGroup->second->GetGroupUserInfo(dwUserID);
    if (pGroupInfo)
    {
        //MapGroupUser& mapGroupUserInfo = itGroup->second->GetAllGroupUserInfo();
        //父成员
        if (pGroupUserInfo->dwParentUserId != 0)
        {
            tagGroupUserInfo* pGroupParentUserInfo = itGroup->second->GetGroupUserInfo(pGroupUserInfo->dwParentUserId);
            if (pGroupParentUserInfo)
            {
                dwSendSize = 0;
                msgHead.dwSubID = SUB_SC_GROUP_PARENT_USER;
                memcpy(szBuffer, &msgHead, sizeof(msgHead));
                dwSendSize += sizeof(MSG_GameMsgDownHead);

                MSG_SC_GroupUser groupUserInfo;
                groupUserInfo.dwUserID = pGroupParentUserInfo->dwUserID;
                groupUserInfo.cbSex = pGroupParentUserInfo->cbSex;
                groupUserInfo.cbHeadId = pGroupParentUserInfo->cbHeadId;
                groupUserInfo.cbVip2 = pGroupParentUserInfo->cbVip2;
                groupUserInfo.cbOnline = pGroupParentUserInfo->bOnline;
                //_snprintf_info(groupUserInfo.szRemarks, sizeof(groupUserInfo.szRemarks), "%s", pGroupParentUserInfo->szRemarks);

                memcpy(szBuffer + dwSendSize, &groupUserInfo, sizeof(MSG_SC_GroupUser));
                dwSendSize += sizeof(MSG_SC_GroupUser);
                CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
            }
        }

        //直属成员
        MapGroupLinkUser& mapGroupDirLinkUser = itGroup->second->GetDirGroupUser();
        auto itLinkUser = mapGroupDirLinkUser.find(dwUserID);
        if (itLinkUser != mapGroupDirLinkUser.end())
        {
            std::vector<tagGroupUserInfo*>& vecLinkUser = itLinkUser->second;
            if (vecLinkUser.size() != 0)
            {
                dwSendSize = 0;
                msgHead.dwSubID = SUB_SC_GROUP_USER_LIST;
                memcpy(szBuffer, &msgHead, sizeof(msgHead));
                dwSendSize += sizeof(MSG_GameMsgDownHead);

                MSG_SC_GroupUser groupUserInfo;
                for (auto& itvecLinkUser : vecLinkUser)
                {
                    if ((dwSendSize + sizeof(MSG_SC_GroupUser)) > sizeof(szBuffer))
                    {
                        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
                        dwSendSize = 0;
                        memcpy(szBuffer, &msgHead, sizeof(msgHead));
                        dwSendSize += sizeof(MSG_GameMsgDownHead);
                    }

                    groupUserInfo.dwUserID = itvecLinkUser->dwUserID;
                    groupUserInfo.cbSex = itvecLinkUser->cbSex;
                    groupUserInfo.cbHeadId = itvecLinkUser->cbHeadId;
                    groupUserInfo.cbVip2 = itvecLinkUser->cbVip2;
                    groupUserInfo.cbOnline = itvecLinkUser->bOnline;
                    _snprintf_info(groupUserInfo.szRemarks, sizeof(groupUserInfo.szRemarks), "%s", itvecLinkUser->szRemarks);

                    memcpy(szBuffer + dwSendSize, &groupUserInfo, sizeof(MSG_SC_GroupUser));
                    dwSendSize += sizeof(MSG_SC_GroupUser);
                }

                if (dwSendSize > sizeof(MSG_GameMsgDownHead))
                {
                    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
                }
            }
        }
    }

    ////////////////////////////////以下未读取的群消息///////////////////////////////////////////
    MapGroupChat& mapGroupChat = itGroup->second->GetGroupChat();
    auto itChat = mapGroupChat.find(dwUserID);
    if (itChat != mapGroupChat.end())
    {
        std::vector<tagGroupChat>& vecChat = itChat->second;
        for (auto& itVecChat : vecChat)
        {
            MSG_SC_Send_GroupChatMsg chatMsg;
            chatMsg.dwMainID = MSG_GROUP_MAIN;
            chatMsg.dwSubID = SUB_SC_REC_GROUP_CHAT_MSG;
            chatMsg.dwValue2 = dwUserID;
            chatMsg.dwMsgID =  itVecChat.dwMsgId;
            chatMsg.dwToUserID = itVecChat.dwToUserID;
            chatMsg.dwFromUserID = itVecChat.dwFromUserID;
            chatMsg.dwSendTime = itVecChat.dwSendTime;
            _snprintf_info(chatMsg.szContent, sizeof(chatMsg.szContent), "%s", itVecChat.szChat);
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &chatMsg,
                                            sizeof(MSG_SC_Send_GroupChatMsg));

            /*MSG_CS2DB_Update_GroupChat updateGroupChat;
            updateGroupChat.dwMsgID = itVecChat.dwMsgId;
            updateGroupChat.cbStatus = 1;
            UpdateGroupChatStatusToDB(&updateGroupChat);*/
        }
        //mapGroupChat.erase(itChat);
    }

	////////////////////////////////以下是群申请列表///////////////////////////////////////////
	//新版本没有申请列表
	/*
	if (!bHasMasterGroup)
	{
		return;
	}

	//发送申请者列表
	dwSendSize = 0;
	msgHead.dwSubID = SUB_SC_GROUP_APPLY_LIST;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

    std::map<CT_DWORD, std::unique_ptr<tagApplyAddGroupInfo>>& mapApplyInfo = itGroup->second->GetAllApplyInfo();
    for (auto& itApplyInfo : mapApplyInfo)
    {
        if ((dwSendSize + sizeof(MSG_SC_ApplyGroup)) > sizeof(szBuffer))
        {
            CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
            dwSendSize = 0;
            memcpy(szBuffer, &msgHead, sizeof(msgHead));
            dwSendSize += sizeof(MSG_GameMsgDownHead);
        }

        MSG_SC_ApplyGroup applyInfo;
        applyInfo.dwGroupID = dwGroupID;
        applyInfo.dwUserID = itApplyInfo.first;
        applyInfo.dwApplyTime = itApplyInfo.second->dwApplyTime;
        applyInfo.cbSex = itApplyInfo.second->cbSex;
        applyInfo.cbHeadId = itApplyInfo.second->cbHeadId;
        applyInfo.cbVip2 = itApplyInfo.second->cbVip2;

        memcpy(szBuffer + dwSendSize, &applyInfo, sizeof(MSG_SC_ApplyGroup));
        dwSendSize += sizeof(MSG_SC_ApplyGroup);
    }

    if (dwSendSize > sizeof(MSG_GameMsgDownHead))
    {
        CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
    }
    */
}

CT_VOID	CGroupMgr::SendGroupInfoToClient(CT_DWORD dwUserID)
{
	/*auto it = m_mapUserGroup.find(dwUserID);
	if (it == m_mapUserGroup.end())
	{
	    //如果没有公会,则判断玩家是否有资格
	    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
	    if (pUserInfo == NULL || !pUserInfo->bOnline)
        {
            return;
        }

        acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
	    if (pProxySock == NULL)
        {
            return;
        }

        MSG_SC_UserGroup_State groupState;
	    groupState.dwMainID = MSG_GROUP_MAIN;
	    groupState.dwSubID = SUB_SC_USER_GROUP_STATE;
	    groupState.dwValue2 = dwUserID;

	    if (pUserInfo->dwTotalRecharge >= CAN_CREATE_GROUP_RECHARGE)
        {
	        groupState.cbState = 1;
        }
        else
        {
            groupState.cbState = 2;
        }
        CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupState,
                                        sizeof(MSG_SC_UserGroup_State));
    }
	else
	{
		//std::set<CT_DWORD>& setGroupId = it->second;
		//SendOneGroupInfoToClient(*setGroupId.begin(), dwUserID);
        SendAllGroupInfoToClient(dwUserID);
	}*/
    SendAllGroupInfoToClient(dwUserID);
}

CT_VOID	CGroupMgr::SendOneGroupInfoToClient(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	//查找玩家proxy信息
	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	//群组信息
	static CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
	CT_DWORD dwSendSize = 0;

	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_GROUP_MAIN;
	msgHead.dwSubID = SUB_SC_ADD_GROUP_INFO;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	//群信息
	auto itGroup = m_mapGroup.find(dwGroupID);
	if (itGroup == m_mapGroup.end())
	{
		return;
	}

	std::unique_ptr<tagGroupInfo>& pGroupInfo = itGroup->second->GetGroupInfo();

	MSG_SC_GroupInfo groupInfo;
	groupInfo.dwGroupID = pGroupInfo->dwGroupID;
	groupInfo.dwMasterID = pGroupInfo->dwMasterID;
	//groupInfo.wUserCount = pGroupInfo->wUserCount;
	groupInfo.cbIcon = pGroupInfo->cbIcon;
	groupInfo.cbLevel = pGroupInfo->cbLevel;
	groupInfo.dwContribution = pGroupInfo->dwContribution;
	//查找本等级最大值贡献值(下一个等级的贡献值开始值)
	groupInfo.dwNextContribution = GetLevelMaxContribution(pGroupInfo->cbLevel);

	//玩家职位
	/*groupInfo.cbPosition = 3;
	if (pGroupInfo->dwMasterID == dwUserID)
    {
	    groupInfo.cbPosition = 1;
    }*/
	/*else if ((pGroupInfo->dwDeputyID1 != 0 && pGroupInfo->dwDeputyID1 == dwUserID)
	            || (pGroupInfo->dwDeputyID2 != 0 && pGroupInfo->dwDeputyID2 == dwUserID))
    {
	    groupInfo.cbPosition = 2;
    }*/
	_snprintf_info(groupInfo.szGroupName, sizeof(groupInfo.szGroupName), "%s", pGroupInfo->szGroupName);
	_snprintf_info(groupInfo.szNotice, sizeof(groupInfo.szNotice), "%s", pGroupInfo->szNotice);

	memcpy(szBuffer + dwSendSize, &groupInfo, sizeof(MSG_SC_GroupInfo));
	dwSendSize += sizeof(MSG_SC_GroupInfo);
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);

	bool bIsMaster = false;
	if (dwUserID == pGroupInfo->dwMasterID
	/*|| dwUserID == pGroupInfo->dwDeputyID1
	|| dwUserID == pGroupInfo->dwDeputyID2*/)
	{
		bIsMaster = true;
	}

	if (bIsMaster)
	{
		//群组成员信息
		dwSendSize = 0;
		msgHead.dwSubID = SUB_SC_ADD_GROUP_USER_LIST;
		memcpy(szBuffer, &msgHead, sizeof(msgHead));
		dwSendSize += sizeof(MSG_GameMsgDownHead);

		memcpy(szBuffer + dwSendSize, &dwGroupID, sizeof(CT_DWORD));
		dwSendSize += sizeof(CT_DWORD);
		std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = itGroup->second->GetAllGroupUserInfo();
		for (auto& itUserInfo : mapGroupUserInfo)
		{
			if ((dwSendSize + sizeof(MSG_SC_GroupUser)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
				dwSendSize = 0;
				memcpy(szBuffer, &msgHead, sizeof(msgHead));
				dwSendSize += sizeof(MSG_GameMsgDownHead);
			}

			MSG_SC_GroupUser groupUserInfo;
			groupUserInfo.dwUserID = itUserInfo.second->dwUserID;
			//groupUserInfo.dwRecharge = itUserInfo.second->dwRecharge;
			//groupUserInfo.dWinScore = itUserInfo.second->llTotalIncome*TO_DOUBLE;
			groupUserInfo.cbSex = itUserInfo.second->cbSex;
			groupUserInfo.cbHeadId = itUserInfo.second->cbHeadId;
			groupUserInfo.cbVip2 = itUserInfo.second->cbVip2;
			//groupUserInfo.dwLastLoginDate = itUserInfo.second->dwLastDate;
			//groupUserInfo.cbPosition = 3;
            //if (pGroupInfo->dwMasterID == groupUserInfo.dwUserID)
            //{
            //    groupUserInfo.cbPosition = 1;
            //}
            /*else if ((pGroupInfo->dwDeputyID1 != 0 && pGroupInfo->dwDeputyID1 == groupUserInfo.dwUserID)
                || (pGroupInfo->dwDeputyID2 != 0 && pGroupInfo->dwDeputyID2 == groupUserInfo.dwUserID))
            {
                groupUserInfo.cbPosition = 2;
            }*/

			memcpy(szBuffer + dwSendSize, &groupUserInfo, sizeof(MSG_SC_GroupUser));
			dwSendSize += sizeof(MSG_SC_GroupUser);
		}
		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}
	}
}

CT_VOID	CGroupMgr::SendAllGameServerAddGroup(tagGroupInfo* pGroupInfo)
{
	//给所有GS同步
	const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
	if (pMapGameServer)
	{
		auto it = pMapGameServer->begin();
		for (; it != pMapGameServer->end(); ++it)
		{
			CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_ADD_GROUP, pGroupInfo, sizeof(tagGroupInfo));
		}
	}
}

CT_VOID	CGroupMgr::SendAllGameSserverAddGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser)
{
	//给所有GS同步
	const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
	if (pMapGameServer)
	{
		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		CT_DWORD dwSendSize = 0;
		memcpy(szBuffer, &dwGroupID, sizeof(CT_DWORD));
		dwSendSize += sizeof(CT_DWORD);
		memcpy(szBuffer + dwSendSize, pGroupUser, sizeof(tagGroupUserInfo));
		dwSendSize += sizeof(tagGroupUserInfo);

		auto it = pMapGameServer->begin();
		for (; it != pMapGameServer->end(); ++it)
		{
			CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_ADD_GROUP_USER, szBuffer, dwSendSize);
		}
	}
}


CT_VOID	CGroupMgr::SendAllGameServerRemoveGroup(CT_DWORD dwGroupID)
{
	//给所有GS同步
	const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
	if (pMapGameServer)
	{
		MSG_C2GS_Remove_Group removeGroup;
		removeGroup.dwGroupID = dwGroupID;
		auto it = pMapGameServer->begin();
		for (; it != pMapGameServer->end(); ++it)
		{
			CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_REMOVE_GROUP, &removeGroup, sizeof(removeGroup));
		}
	}
}

CT_VOID	CGroupMgr::SendAllGameServerRemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
	if (pMapGameServer)
	{
		MSG_C2GS_Remove_GroupUser removeGroupUser;
		removeGroupUser.dwGroupID = dwGroupID;
		removeGroupUser.dwUserID = dwUserID;

		auto it = pMapGameServer->begin();
		for (; it != pMapGameServer->end(); ++it)
		{
			CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_REMOVE_GROUP_USER, &removeGroupUser, sizeof(removeGroupUser));
		}
	}
}

CT_VOID CGroupMgr::SendAllGameServerAddPRoom(CT_DWORD dwGroupID, tagGroupPRoomInfo* pGroupPRoomInfo)
{
    const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
    if (pMapGameServer)
    {
        MSG_C2GS_Add_GroupPRoom addGroupPRoom;
        addGroupPRoom.dwGroupID = dwGroupID;
        addGroupPRoom.dwRoomNum = pGroupPRoomInfo->dwRoomNum;

        auto it = pMapGameServer->begin();
        for (; it != pMapGameServer->end(); ++it)
        {
            if (it->second.dwServerID == pGroupPRoomInfo->dwServerID)
            {
                CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_ADD_GROUP_PROOM, &addGroupPRoom, sizeof(MSG_C2GS_Add_GroupPRoom));
                break;
            }
        }
    }
}

CT_VOID CGroupMgr::SendAllGameServerRemovePRoom(tagGroupPRoomInfo* pGroupPRoomInfo)
{
    const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
    if (pMapGameServer)
    {
        MSG_C2GS_Remove_GroupPRoom removeGroupPRoom;
        removeGroupPRoom.dwRoomNum = pGroupPRoomInfo->dwRoomNum;

        auto it = pMapGameServer->begin();
        for (; it != pMapGameServer->end(); ++it)
        {
            if (it->second.dwServerID == pGroupPRoomInfo->dwServerID)
            {
                CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_REMOVE_GROUP_PROOM, &removeGroupPRoom, sizeof(MSG_C2GS_Remove_GroupPRoom));
                break;
            }
        }
    }
}

CT_VOID CGroupMgr::SendAllProxyServerAddPRoom(tagGroupPRoomInfo* pGroupPRoomInfo)
{
    const MapProxyServer* pMapProxyServer = CServerMgr::get_instance().GetProxyServerMap();
    if (pMapProxyServer)
    {
        MSG_CS2P_Add_GroupPRoom addGroupPRoom;
        addGroupPRoom.dwServerID = pGroupPRoomInfo->dwServerID;
        addGroupPRoom.dwRoomNum = pGroupPRoomInfo->dwRoomNum;

        auto it = pMapProxyServer->begin();
        for (; it != pMapProxyServer->end(); ++it)
        {
            CNetModule::getSingleton().Send(it->first, MSG_PCS_MAIN, SUB_CS2P_ADD_GROUP_PROOM, &addGroupPRoom, sizeof(MSG_CS2P_Add_GroupPRoom));
        }
    }
}

CT_VOID CGroupMgr::SendAllPRoomToProxyServer(acl::aio_socket_stream* pSocket)
{
    for (auto& it : m_mapGroup)
    {
        MapGroupPRoom&  mapGroupPRoom = it.second->GetGroupPRoom();
        for (auto& itmapPRoom : mapGroupPRoom)
        {
            VecGroupPRoom &vecGroupPRoom = itmapPRoom.second;
            for (auto &itVecPRoom : vecGroupPRoom)
            {
                MSG_CS2P_Add_GroupPRoom addGroupPRoom;
                addGroupPRoom.dwServerID = itVecPRoom.dwServerID;
                addGroupPRoom.dwRoomNum = itVecPRoom.dwRoomNum;
                CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_ADD_GROUP_PROOM, &addGroupPRoom, sizeof(MSG_CS2P_Add_GroupPRoom));
            }
        }
    }
}

CT_VOID CGroupMgr::SendAllProxyServerRemovePRoom(CT_DWORD dwRoomNum)
{
    const MapProxyServer* pMapProxyServer = CServerMgr::get_instance().GetProxyServerMap();
    if (pMapProxyServer)
    {
        auto it = pMapProxyServer->begin();
        for (; it != pMapProxyServer->end(); ++it)
        {
            CNetModule::getSingleton().Send(it->first, MSG_PCS_MAIN, SUB_CS2P_REMOVE_GROUP_PROOM, &dwRoomNum, sizeof(CT_DWORD));
        }
    }
}

CT_VOID	CGroupMgr::AddPRoomNum(CT_DWORD dwGroupID, std::shared_ptr<tagGroupPRoomInfo> pRoomInfo)
{
	/*auto it = m_mapRoomNum.find(dwGroupID);
	if (it == m_mapRoomNum.end())
	{
		std::vector<std::shared_ptr<tagGroupPRoomInfo>> setRoomInfo;
		setRoomInfo.push_back(pRoomInfo);
		m_mapRoomNum.insert(std::make_pair(dwGroupID, setRoomInfo));
	}
	else
	{
		it->second.push_back(pRoomInfo);
	}*/
}

CT_VOID	CGroupMgr::RemovePRoomNum(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwRoomNum)
{
	//LOG(WARNING) << "remove proom!";
	/*
	auto it = m_mapRoomNum.find(dwGroupID);
	if (it != m_mapRoomNum.end())
	{
		std::vector<std::shared_ptr<tagGroupPRoomInfo>>& setPRoomInfo = it->second;

		for (auto itSet = setPRoomInfo.begin(); itSet != setPRoomInfo.end(); ++itSet)
		{
			if ((*itSet)->wGameID == wGameID && (*itSet)->wKindID == wKindID && (*itSet)->dwRoomNum == dwRoomNum)
			{
				setPRoomInfo.erase(itSet);
				if (setPRoomInfo.empty())
				{
					m_mapRoomNum.erase(it);
					//LOG(WARNING) << "delete proom set!";
				}
				//LOG(WARNING) << "proom set:" << setPRoomInfo.size();
				return;
			}
		}
	}
	else
	{
		LOG(WARNING) << "remove room num fail, room num: " << dwRoomNum;
	}
	 */
}

CT_BOOL	CGroupMgr::FindPRoomInfo(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID, CT_DWORD& dwRoomNum, CT_WORD& userCount)
{
    /*
	auto it = m_mapRoomNum.find(dwGroupID);
	if (it == m_mapRoomNum.end())
	{
		return false;
	}

	std::vector<std::shared_ptr<tagGroupPRoomInfo>>& setRoomInfo = it->second;
	for (auto itSet : setRoomInfo)
	{
		std::shared_ptr<tagGroupPRoomInfo>& pRoomInfo = itSet;
		if (pRoomInfo->wGameID == wGameID && pRoomInfo->wKindID == wKindID && (strcmp(pRoomInfo->szParam, szParam) == 0))
		{
			dwRoomNum = pRoomInfo->dwRoomNum;

			acl::string key;
			acl::string att_usercount, att_curruser;
			const char* attrs[2];
			std::vector<acl::string> result;
			att_usercount.format("usercount");
			att_curruser.format("curruser");

			attrs[0] = att_usercount.c_str();
			attrs[1] = att_curruser.c_str();

			key.format("proom_info_%u", dwRoomNum);

			m_pRedis->clear();
			if (m_pRedis->hmget(key, attrs, 2, &result) == false)
			{
				LOG(WARNING) << "get proom info fail, room num: " << dwRoomNum;
				continue;
			}

			if (result.size() != 2)
			{
				continue;
			}
			CT_WORD wUserCount = atoi(result[0].c_str());
			userCount = atoi(result[1].c_str());
			if (userCount < wUserCount)
			{
				return true;
			}
		}
	}
	*/

	return false;
}

CT_VOID CGroupMgr::CollectUserGameData(MSG_G2CS_PlayGame* pPlayGame)
{
    if (pPlayGame->dwRevenue == 0)
        return;

    //检测玩家是否是公会成员
    auto it = m_mapUserGroup.find(pPlayGame->dwUserID);
    if (it == m_mapUserGroup.end())
    {
        return;
    }

    CT_DWORD dwGroupID = it->second;
    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup == m_mapGroup.end())
    {
        return;
    }

    MapGroupUserDateRevenue& mapGroupUserDateRevenue = itGroup->second->GetGroupUserDateRevenue();

    //计算时间值(格式:20190429)
    /*struct tm tm;
    time_t recordTime = pPlayGame->dwRecordTime;
    tm = *localtime(&recordTime);
    unsigned int nIntTime = (tm.tm_year + 1900)*100+tm.tm_mon+1;*/
    unsigned int nIntTime = CalcDayInt(0);

    //记录当天的税收
    MapGroupUserRevenue& mapGroupUserRevenue = mapGroupUserDateRevenue[nIntTime];
    auto itMapUserRevenue = mapGroupUserRevenue.find(pPlayGame->dwUserID);
    if (itMapUserRevenue != mapGroupUserRevenue.end())
    {
        itMapUserRevenue->second += pPlayGame->dwRevenue;
    }
    else
    {
        mapGroupUserRevenue.insert(std::make_pair(pPlayGame->dwUserID, pPlayGame->dwRevenue));
    }
}

CT_VOID CGroupMgr::CalcGroupUserIncome(bool bCrossDays)
{
    //计算时间值(格式:20190429)
    unsigned int nTodayIntTime = 0;
    unsigned int nYesterdayIntTime = 0;

    if (bCrossDays)
    {
        nTodayIntTime = CalcDayInt(1);
        nYesterdayIntTime = CalcDayInt(2);
    }
    else
    {
        nTodayIntTime = CalcDayInt(0);
        nYesterdayIntTime = CalcDayInt(1);
    }
    /*struct tm tm;
    time_t now = time(NULL);
    tm = *localtime(&now);
    unsigned int nsIntTime = (tm.tm_year + 1900)*100+tm.tm_mon+1;*/

    for (auto& itGroup : m_mapGroup)
    {
        std::vector<tagGroupUserInfo*> vecNeedUpdateUserInfo;
        MapGroupUserDateRevenue& mapGroupUserDateRevenue = itGroup.second->GetGroupUserDateRevenue();
        auto itGroupDateRevenue = mapGroupUserDateRevenue.find(nTodayIntTime);
        if (itGroupDateRevenue == mapGroupUserDateRevenue.end())
        {
            continue;
        }

        MapGroupUserRevenue& mapGroupUserRevenue = itGroupDateRevenue->second;
        for (auto& itGroupUserRevenue : mapGroupUserRevenue)
        {
            CT_DWORD dwUserID = itGroupUserRevenue.first;
            CT_LONGLONG llRevenue = itGroupUserRevenue.second;
            tagGroupUserInfo* pGroupUserInfo = itGroup.second->GetGroupUserInfo(dwUserID);
            if (pGroupUserInfo == NULL)
            {
                continue;
            }

            //如果是群主不计算收入
            if (pGroupUserInfo->dwParentUserId == 0)
                continue;

            //计算玩家的上级玩家的收入
            tagGroupUserInfo* pParentUserInfo = itGroup.second->GetGroupUserInfo(pGroupUserInfo->dwParentUserId);
            if (pParentUserInfo == NULL)
            {
                continue;
            }

            //直属收入
            {
                CT_WORD wDeductRate = 0;
                CT_DWORD dwParentUserIncome = (CT_DWORD)ceil((pParentUserInfo->cbInComeRate * llRevenue) / 100);
                CT_DWORD dwDeductIncome = dwParentUserIncome;
                //如果需要扣量, 则扣量
                if (dwParentUserIncome > pParentUserInfo->dwDeductStart)
                {
                    wDeductRate = pParentUserInfo->wDeductRate;
                    if (pParentUserInfo->dwLastIncomeDate == nYesterdayIntTime)
                    {
                        CT_WORD wCalDuctRate = pParentUserInfo->wDeductRate + (pParentUserInfo->wDeductIncre * (pParentUserInfo->wLinkIncomeDays));
                        wDeductRate = (wCalDuctRate < pParentUserInfo->wDeductTop ?  wCalDuctRate : pParentUserInfo->wDeductTop);
                    }
                    dwDeductIncome = pParentUserInfo->dwDeductStart +  (dwParentUserIncome  - pParentUserInfo->dwDeductStart) * (1000 - wDeductRate) / 1000 ;
                }

                bool bInsertNew = true;
                MapGroupAllUserDateIncome& mapGroupAllUserDateDirIncome = itGroup.second->GetGroupUserDateDirIncome();
                MapGroupUserDateIncome& mapGroupUserDateDirIncome = mapGroupAllUserDateDirIncome[pParentUserInfo->dwUserID];
                auto itGroupUserIncome = mapGroupUserDateDirIncome.find(nTodayIntTime);
                if (itGroupUserIncome != mapGroupUserDateDirIncome.end())
                {
                    MapGroupUserIncome& mapGroupUserIncome = itGroupUserIncome->second;
                    auto itUserIncome = mapGroupUserIncome.find(dwUserID);
                    if (itUserIncome != mapGroupUserIncome.end())
                    {
                        std::unique_ptr<tagGroupUserDateIncome>& userIncomePtr = itUserIncome->second;
                        userIncomePtr->dwOriginalIncome = dwParentUserIncome;
                        userIncomePtr->wDeductRate = wDeductRate;
                        userIncomePtr->dwDeductIncome = dwDeductIncome;
                        bInsertNew = false;
                    }
                }

                if (bInsertNew)
                {
                    std::unique_ptr<tagGroupUserDateIncome> userIncomePtr(new tagGroupUserDateIncome);
                    userIncomePtr->dwOriginalIncome = dwParentUserIncome;
                    userIncomePtr->wDeductRate = wDeductRate;
                    userIncomePtr->dwDeductIncome = dwDeductIncome;
                    mapGroupUserDateDirIncome[nTodayIntTime].insert(std::make_pair(dwUserID, std::move(userIncomePtr)));
                }

                if (bCrossDays)
                {
                    /*if (pParentUserInfo->dwLastIncomeDate == nYesterdayIntTime)
                    {
                        pParentUserInfo->wLinkIncomeDays += 1;
                    }
                    else
                    {
                        pParentUserInfo->wLinkIncomeDays = 0;
                    }
                    pParentUserInfo->dwLastIncomeDate = nTodayIntTime;*/
                    pParentUserInfo->llTotalIncome += dwDeductIncome;

                    //合并总收入
                    MapGroupParentUserTotalIncome& mapGroupParentUserTotalIncome = itGroup.second->GetParentUserTotalIncome();
                    MapGroupUserTotalIncome& mapGroupUserTotalIncome  = mapGroupParentUserTotalIncome[pParentUserInfo->dwUserID];
                    auto itTotalIncome = mapGroupUserTotalIncome.find(dwUserID);
                    if (itTotalIncome != mapGroupUserTotalIncome.end())
                    {
                        itTotalIncome->second += dwDeductIncome;
                    }
                    else
                    {
                        mapGroupUserTotalIncome.insert(std::make_pair(dwUserID, dwDeductIncome));
                    }

                    auto itVecUserInfo = std::find(vecNeedUpdateUserInfo.begin(), vecNeedUpdateUserInfo.end(), pParentUserInfo);
                    if (itVecUserInfo == vecNeedUpdateUserInfo.end())
                        vecNeedUpdateUserInfo.push_back(pParentUserInfo);
                }
            }

            while (pParentUserInfo)
            {
                tagGroupUserInfo* pParentParentUserInfo = itGroup.second->GetGroupUserInfo(pParentUserInfo->dwParentUserId);
                if (pParentParentUserInfo == NULL)
                {
                    break;
                }

                if (pParentParentUserInfo->cbInComeRate < pParentUserInfo->cbInComeRate)
                {
                    break;
                }

                CT_WORD wDeductRate = 0;
                CT_DWORD dwParentParentUserIncome = (CT_DWORD)ceil(((pParentParentUserInfo->cbInComeRate - pParentUserInfo->cbInComeRate) * llRevenue) / 100);
                CT_DWORD dwDeductIncome = dwParentParentUserIncome;
                //如果需要扣量, 则扣量
                if (dwParentParentUserIncome > pParentParentUserInfo->dwDeductStart)
                {
                    wDeductRate = pParentParentUserInfo->wDeductRate;
                    if (pParentParentUserInfo->dwLastIncomeDate == nYesterdayIntTime)
                    {
                        CT_WORD wCalDuctRate = pParentParentUserInfo->wDeductRate + (pParentParentUserInfo->wDeductIncre * (pParentParentUserInfo->wLinkIncomeDays));
                        wDeductRate = (wCalDuctRate < pParentParentUserInfo->wDeductTop ?  wCalDuctRate : pParentParentUserInfo->wDeductTop);
                    }
                    dwDeductIncome = pParentParentUserInfo->dwDeductStart +  (dwParentParentUserIncome  - pParentParentUserInfo->dwDeductStart) * (1000 - wDeductRate) / 1000 ;
                }

                bool bInsertNew = true;
                MapGroupAllUserDateIncome& mapGroupAllUserDateSubIncome = itGroup.second->GetGroupUserDateSubIncome();
                MapGroupUserDateIncome& mapGroupUserDateSubIncome = mapGroupAllUserDateSubIncome[pParentParentUserInfo->dwUserID];
                auto itGroupUserIncome = mapGroupUserDateSubIncome.find(nTodayIntTime);
                if (itGroupUserIncome != mapGroupUserDateSubIncome.end())
                {
                    MapGroupUserIncome& mapGroupUserIncome = itGroupUserIncome->second;
                    auto itUserIncome = mapGroupUserIncome.find(dwUserID);
                    if (itUserIncome != mapGroupUserIncome.end())
                    {
                        std::unique_ptr<tagGroupUserDateIncome>& userIncomePtr = itUserIncome->second;
                        userIncomePtr->dwOriginalIncome = dwParentParentUserIncome;
                        userIncomePtr->wDeductRate = wDeductRate;
                        userIncomePtr->dwDeductIncome = dwDeductIncome;
                        bInsertNew = false;
                    }
                }
                if (bInsertNew)
                {
                    std::unique_ptr<tagGroupUserDateIncome> userIncomePtr(new tagGroupUserDateIncome);
                    userIncomePtr->dwOriginalIncome = dwParentParentUserIncome;
                    userIncomePtr->wDeductRate = wDeductRate;
                    userIncomePtr->dwDeductIncome = dwDeductIncome;
                    mapGroupUserDateSubIncome[nTodayIntTime].insert(std::make_pair(dwUserID, std::move(userIncomePtr)));
                }

                if (bCrossDays)
                {
                    /*if (pParentParentUserInfo->dwLastIncomeDate == nYesterdayIntTime)
                    {
                        pParentParentUserInfo->wLinkIncomeDays += 1;
                    }
                    else
                    {
                        pParentParentUserInfo->wLinkIncomeDays = 0;
                    }
                    pParentParentUserInfo->dwLastIncomeDate = nTodayIntTime;*/
                    pParentParentUserInfo->llTotalIncome += dwDeductIncome;

                    //合并总收入
                    MapGroupParentUserTotalIncome& mapGroupParentUserTotalIncome = itGroup.second->GetParentUserTotalIncome();
                    MapGroupUserTotalIncome& mapGroupUserTotalIncome  = mapGroupParentUserTotalIncome[pParentParentUserInfo->dwUserID];
                    auto itTotalIncome = mapGroupUserTotalIncome.find(dwUserID);
                    if (itTotalIncome != mapGroupUserTotalIncome.end())
                    {
                        itTotalIncome->second += dwDeductIncome;
                    }
                    else
                    {
                        mapGroupUserTotalIncome.insert(std::make_pair(dwUserID, dwDeductIncome));
                    }

                    auto itVecUserInfo = std::find(vecNeedUpdateUserInfo.begin(), vecNeedUpdateUserInfo.end(), pParentUserInfo);
                    if (itVecUserInfo == vecNeedUpdateUserInfo.end())
                        vecNeedUpdateUserInfo.push_back(pParentUserInfo);
                }

                pParentUserInfo = pParentParentUserInfo;
            }
        }

        if (bCrossDays)
        {
            //跨天删除前一天的税收
            mapGroupUserDateRevenue.erase(itGroupDateRevenue);
            for (auto& itVecUser : vecNeedUpdateUserInfo)
            {
                tagGroupUserInfo* pGroupUser = itVecUser;
                if (pGroupUser->dwLastIncomeDate == nYesterdayIntTime || pGroupUser->dwLastIncomeDate == 0)
                {
                    pGroupUser->wLinkIncomeDays += 1;
                }
                else
                {
                    pGroupUser->wLinkIncomeDays = 0;
                }
                pGroupUser->dwLastIncomeDate = nTodayIntTime;
                UpdateGroupUserInfoToDB(itGroup.first, itVecUser);
            }

        }
        itGroup.second->UpdateIncomeTodb(nTodayIntTime, bCrossDays);
    }
}

CT_VOID CGroupMgr::CheckRemoveEmptyPRoom()
{
    CT_DWORD dwNow = time(NULL);
    for (auto& itGroup : m_mapGroup) 
    {
        MapGroupPRoom &mapGroupPRoom = itGroup.second->GetGroupPRoom();
        for (auto& itMapGroupPRoom : mapGroupPRoom)
        {
            VecGroupPRoom& vecGroupPRoom = itMapGroupPRoom.second;
            for (auto& itVecGroupPRoom : vecGroupPRoom)
            {
                //到游戏服务器看看能不能解散
                if (itVecGroupPRoom.wSitUserCount == 0 && dwNow - itVecGroupPRoom.dwEmptyTime > 5*60)
                {
                    SendAllGameServerRemovePRoom(&itVecGroupPRoom);
                }
            }
        }
    }
}

CT_VOID CGroupMgr::OnUserRecharge(CT_DWORD dwUserID, CT_DWORD dwGroupID, CT_LONGLONG llAddScore, CT_DWORD dwTotalCash, CT_BYTE cbVip2)
{
    //每满100元增加1点
    CT_LONGLONG llAddContribution = llAddScore / 10000;
    if (llAddContribution > 0)
        UpdateGroupContribution(dwGroupID, llAddContribution);

    //更新玩家的总充值
    auto itGroup = m_mapGroup.find(dwGroupID);
    if (itGroup != m_mapGroup.end())
    {
        tagGroupUserInfo* pGroupUserInfo = itGroup->second->GetGroupUserInfo(dwUserID);
        if (pGroupUserInfo)
        {
            pGroupUserInfo->dwRecharge = dwTotalCash;
            if (pGroupUserInfo->cbVip2 != cbVip2)
            {
                pGroupUserInfo->cbVip2 = cbVip2;
            }
        }
    }
}

CT_DWORD CGroupMgr::CalcDayInt(int nBeforeDays)
{
    /* 取得N天前的日期 */
    time_t          t;
    struct tm      *m;

    t = time(NULL) - 24 * 3600 * nBeforeDays;
    m = localtime(&t);
    unsigned int nIntTime = (m->tm_year + 1900)*10000+(m->tm_mon+1)*100+m->tm_mday;

    return nIntTime;
}

CT_VOID	CGroupMgr::SendGroupErrorMsg(CT_DWORD dwUserID, CT_BYTE cbErrorCode)
{
	acl::aio_socket_stream* pProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pProxySock != NULL)
	{
		MSG_SC_Group_Error error;
		error.cbErrorCode = cbErrorCode;
		error.dwMainID = MSG_GROUP_MAIN;
		error.dwSubID = SUB_SC_GROUP_ERROR;
		error.dwValue2 = dwUserID;

		CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &error, sizeof(error));
	}
}

CT_VOID CGroupMgr::UpdateGroupInfoToDB(std::unique_ptr<tagGroupInfo>& pGroupInfo)
{
    MSG_CS2DB_Modify_GroupInfo modifyGroupInfo;
    modifyGroupInfo.dwGroupID = pGroupInfo->dwGroupID;
    modifyGroupInfo.cbIcon = pGroupInfo->cbIcon;
    modifyGroupInfo.cbLevel = pGroupInfo->cbLevel;
    modifyGroupInfo.cbSettleDays = pGroupInfo->cbSettleDays;
    modifyGroupInfo.cbIncomeRate = pGroupInfo->cbIncomeRate;
    _snprintf_info(modifyGroupInfo.szGroupName, sizeof(modifyGroupInfo.szGroupName), "%s", pGroupInfo->szGroupName);
    _snprintf_info(modifyGroupInfo.szNotice, sizeof(modifyGroupInfo.szNotice), "%s", pGroupInfo->szNotice);
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_MODIFY_GROUP_INFO, &modifyGroupInfo, sizeof(modifyGroupInfo));
}

CT_VOID CGroupMgr::UpdateGroupUserInfoToDB(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser)
{
    MSG_CS2DB_Update_GroupUserInfo groupUserInfo;
    groupUserInfo.dwUserID = pGroupUser->dwUserID;
    groupUserInfo.dwGroupID = dwGroupID;
    groupUserInfo.cbIncomeRate = pGroupUser->cbInComeRate;
    groupUserInfo.dwLastTakeIncomeDate = pGroupUser->dwLastTakeIncomeDate;
    groupUserInfo.dwLastIncomeDate = pGroupUser->dwLastIncomeDate;
    groupUserInfo.wLinkIncomeDays = pGroupUser->wLinkIncomeDays;
    groupUserInfo.llTotalIncome = pGroupUser->llTotalIncome;
    _snprintf_info(groupUserInfo.szRemarks, sizeof(groupUserInfo.szRemarks), "%s", pGroupUser->szRemarks);

    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_GROUP_USER_INFO, &groupUserInfo, sizeof(groupUserInfo));
}

CT_VOID CGroupMgr::InsertGroupChatToDB(MSG_CS2DB_Insert_GroupChat* pGroupChat)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_GROUP_CHAT, pGroupChat, sizeof(MSG_CS2DB_Insert_GroupChat));
}

CT_VOID CGroupMgr::UpdateGroupChatStatusToDB(MSG_CS2DB_Update_GroupChat* pGroupChat)
{
    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_GROUP_CHAT, pGroupChat, sizeof(MSG_CS2DB_Update_GroupChat));
}

