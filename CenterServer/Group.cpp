#include "stdafx.h"
#include "Group.h"
#include "UserMgr.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "NetModule.h"
#include "GroupMgr.h"

extern CNetConnector *pNetDB;
#define MAX_GROUP_PROOM_CLEARING 20

CGroup::CGroup()
{
	m_GroupInfo.reset();
}

CGroup::~CGroup()
{

}

CT_VOID	CGroup::InsertGroupInfo(const tagGroupInfo* pGroupInfo)
{
	std::unique_ptr<tagGroupInfo> ptrGroupInfo(new tagGroupInfo);
	m_GroupInfo = std::move(ptrGroupInfo);
	m_GroupInfo->dwGroupID = pGroupInfo->dwGroupID;
	m_GroupInfo->dwMasterID = pGroupInfo->dwMasterID;
	m_GroupInfo->wUserCount = pGroupInfo->wUserCount;
	m_GroupInfo->cbIcon = pGroupInfo->cbIcon;
	m_GroupInfo->cbLevel = pGroupInfo->cbLevel;
	m_GroupInfo->dwContribution = pGroupInfo->dwContribution;
	m_GroupInfo->cbIncomeRate = pGroupInfo->cbIncomeRate;
	m_GroupInfo->cbSettleDays = pGroupInfo->cbSettleDays;
	_snprintf_info(m_GroupInfo->szGroupName, sizeof(m_GroupInfo->szGroupName), "%s", pGroupInfo->szGroupName);
	_snprintf_info(m_GroupInfo->szNotice, sizeof(m_GroupInfo->szNotice), "%s", pGroupInfo->szNotice);

	//防止重连的时候引起数据造成的问题
    //m_mapGroupUser.clear();
    //m_mapDirGroupUser.clear();
    //m_mapSubGroupUser.clear();
    //m_mapGroupChat.clear();
    //m_mapDateRevenue.clear();
    //m_mapDateDirIncome.clear();
    //m_mapDateSubIncome.clear();
    //m_mapTotalIncome.clear();
}

CT_VOID CGroup::InsertGroupUser(const tagGroupUserInfo* pGroupUser)
{
	std::unique_ptr<tagGroupUserInfo> userInfoPtr(new tagGroupUserInfo);
	userInfoPtr->dwUserID = pGroupUser->dwUserID;
    userInfoPtr->dwParentUserId = pGroupUser->dwParentUserId;
    userInfoPtr->cbSex = pGroupUser->cbSex;
    userInfoPtr->cbHeadId = pGroupUser->cbHeadId;
    userInfoPtr->cbVip2 = pGroupUser->cbVip2;
    userInfoPtr->cbMemLevel = pGroupUser->cbMemLevel;
    userInfoPtr->cbInComeRate = pGroupUser->cbInComeRate;
    userInfoPtr->wDeductRate = pGroupUser->wDeductRate;
    userInfoPtr->wDeductTop = pGroupUser->wDeductTop;
    userInfoPtr->wDeductIncre = pGroupUser->wDeductIncre;
    userInfoPtr->dwDeductStart = pGroupUser->dwDeductStart;
    userInfoPtr->dwLastIncomeDate = pGroupUser->dwLastIncomeDate;
    userInfoPtr->dwLastTakeIncomeDate = pGroupUser->dwLastTakeIncomeDate;
    userInfoPtr->wLinkIncomeDays = pGroupUser->wLinkIncomeDays;
    userInfoPtr->llTotalIncome = pGroupUser->llTotalIncome;
	userInfoPtr->dwRecharge = pGroupUser->dwRecharge;
    userInfoPtr->dwRegDate = pGroupUser->dwRegDate;
    userInfoPtr->dwLastDate = pGroupUser->dwLastDate;
	userInfoPtr->bOnline = pGroupUser->bOnline;
	_snprintf_info(userInfoPtr->szRemarks, sizeof(userInfoPtr->szRemarks), "%s", pGroupUser->szRemarks);

    m_mapGroupUser.insert(std::make_pair(pGroupUser->dwUserID, std::move(userInfoPtr)));

	//群组人数增加
	m_GroupInfo->wUserCount += 1;

	//LOG(WARNING) << "group userid : " << pGroupUser->dwUserID << ", parent userid : " << pGroupUser->dwParentUserId;
    if (pGroupUser->dwParentUserId != 0)
    {
        //增加直属成员的关系
        tagGroupUserInfo* pParentUserInfo = GetGroupUserInfo(pGroupUser->dwParentUserId);
        if (pParentUserInfo)
        {
            tagGroupUserInfo* pUserInfo = GetGroupUserInfo(pGroupUser->dwUserID);
            if (pUserInfo)
            {
                std::vector<tagGroupUserInfo*>& vecGroupUser = m_mapDirGroupUser[pParentUserInfo->dwUserID];
                vecGroupUser.push_back(pUserInfo);
                LOG(WARNING) << "insert dir user: " << pParentUserInfo->dwUserID << ", " << pGroupUser->dwUserID;

                //增加附属成员有关系
                pParentUserInfo = GetGroupUserInfo(pParentUserInfo->dwParentUserId);
                while (pParentUserInfo)
                {
                    std::vector<tagGroupUserInfo*>& vecGroupUser = m_mapSubGroupUser[pParentUserInfo->dwUserID];
                    vecGroupUser.push_back(pUserInfo);
                    //LOG(WARNING) << "insert sub user: " << pParentUserInfo->dwUserID << ", " << pGroupUser->dwUserID;
                    if (pParentUserInfo->dwParentUserId == 0)
                    {
                        break;
                    }
                    pParentUserInfo = GetGroupUserInfo(pParentUserInfo->dwParentUserId);
                }
            }
        }
    }
}

CT_VOID CGroup::InsertGroupChat(const tagGroupChat* pGroupChat)
{
    std::vector<tagGroupChat>& vecChat = m_mapGroupChat[pGroupChat->dwToUserID];
    vecChat.push_back(*pGroupChat);
    if (vecChat.size() > 50)
    {
        vecChat.erase(vecChat.begin());
    }

    LOG(WARNING) << "insert group chat, from user id: " << pGroupChat->dwFromUserID << ", to user id: " << pGroupChat->dwToUserID << ", msg id: " << pGroupChat->dwMsgId;
}

CT_VOID CGroup::InsertPRoomCfg(const tagGroupRoomConfig* pGroupCfg)
{
    /*
	std::unique_ptr<tagGroupRoomConfig> roomCfgPtr(new tagGroupRoomConfig);
	roomCfgPtr->dwGroupID = pGroupCfg->dwGroupID;
	roomCfgPtr->wGameID = pGroupCfg->wGameID;
	roomCfgPtr->wKindID = pGroupCfg->wKindID;
	_snprintf_info(roomCfgPtr->szConfig, sizeof(roomCfgPtr->szConfig), "%s", pGroupCfg->szConfig);

	for (auto it = m_vecPRoomCfg.begin(); it != m_vecPRoomCfg.end(); ++it)
	{
		if ((*it)->wGameID == pGroupCfg->wGameID && (*it)->wKindID == pGroupCfg->wKindID)
		{
			m_vecPRoomCfg.erase(it);
			m_vecPRoomCfg.push_back(std::move(roomCfgPtr));
			return;
		}
	}
	m_vecPRoomCfg.push_back(std::move(roomCfgPtr));
     */
}

CT_VOID	CGroup::RemoveGroupUser(CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwUserID);

	if (it != m_mapGroupUser.end())
	{
		m_mapGroupUser.erase(it);
	}

	//群组人数减少
	m_GroupInfo->wUserCount -= 1;
}

CT_VOID CGroup::InsertApplyGroupInfo(const tagApplyAddGroupInfo* pApplyInfo)
{
    /*
	auto it = m_mapApplyUser.find(pApplyInfo->dwApplyUserID);
	if (it != m_mapApplyUser.end())
	{
		return;
	}

	std::unique_ptr<tagApplyAddGroupInfo> applyPtr(new tagApplyAddGroupInfo);
	applyPtr->dwApplyUserID = pApplyInfo->dwApplyUserID;
	applyPtr->cbSex = pApplyInfo->cbSex;
	applyPtr->cbHeadId = pApplyInfo->cbHeadId;
	applyPtr->cbVip2 = pApplyInfo->cbVip2;
	applyPtr->dwApplyTime = pApplyInfo->dwApplyTime;
	_snprintf_info(applyPtr->szNickName, sizeof(applyPtr->szNickName), "%s", pApplyInfo->szNickName);
	//_snprintf_info(applyPtr->szHeadUrl, sizeof(applyPtr->szHeadUrl), "%s", pApplyInfo->szHeadUrl);
	//_snprintf_info(applyPtr->szApplyTime, sizeof(applyPtr->szApplyTime), "%s", pApplyInfo->szApplyTime);

	m_mapApplyUser.insert(std::make_pair(pApplyInfo->dwApplyUserID, std::move(applyPtr)));
     */
}

/*CT_VOID	CGroup::InsertPRoomClearingInfo(MSG_G2CS_GroupPRoom_Clearing* pClearing)
{
	std::unique_ptr<stPRoomClearingInfo> clearingInfo(new stPRoomClearingInfo);
	clearingInfo->wGameID = pClearing->wGameID;
	clearingInfo->wKindID = pClearing->wKindID;
	clearingInfo->dwRoomNum = pClearing->dwRoomNum;
	clearingInfo->cbCount = pClearing->cbCount;
	_snprintf_info(clearingInfo->szTime, sizeof(clearingInfo->szTime), "%s", Utility::GetTimeNowString().c_str());
	for (int i = 0; i < pClearing->cbCount; ++i)
	{
		clearingInfo->iScore[i] = pClearing->iScore[i];
		_snprintf_info(clearingInfo->szNickName[i], sizeof(clearingInfo->szNickName[i]), "%s", pClearing->szNickName[i]);
	}

	m_listPRoomClearing.push_back(std::move(clearingInfo));

	if (m_listPRoomClearing.size() > MAX_GROUP_PROOM_CLEARING)
	{
		m_listPRoomClearing.pop_front();
	}
}*/

CT_DWORD CGroup::GetGroupMasterID()
{
	return m_GroupInfo->dwMasterID;
}

CT_VOID	CGroup::AddGroupUser(tagGroupUserInfo* pGroupUser)
{
	CGroupMgr::get_instance().AddGroupUser(m_GroupInfo->dwGroupID, pGroupUser);

	//给父成员发送新加入消息
	if (pGroupUser->dwParentUserId != 0)
    {
        acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(pGroupUser->dwParentUserId);
        if (pProxySocket != NULL)
        {
            MSG_SC_Add_GroupUser addGroupUser;
            addGroupUser.dwMainID = MSG_GROUP_MAIN;
            addGroupUser.dwSubID = SUB_SC_GROUP_ADD_DIR_USER;
            addGroupUser.dwUserID = pGroupUser->dwUserID;
            addGroupUser.cbSex = pGroupUser->cbSex;
            addGroupUser.cbHeadId = pGroupUser->cbHeadId;
            addGroupUser.cbVip2 = pGroupUser->cbVip2;
            addGroupUser.cbOnline = pGroupUser->bOnline;

            //给上级发消息
            addGroupUser.dwValue2 = pGroupUser->dwParentUserId;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &addGroupUser, sizeof(addGroupUser));
        }
    }

	//给群主更新公会人数的消息
    acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwMasterID);
    if (pProxySocket != NULL)
    {
        MSG_SC_Update_GroupUser_Count groupUserCount;
        groupUserCount.dwMainID = MSG_GROUP_MAIN;
        groupUserCount.dwSubID = SUB_SC_UPDATE_GROUP_USER_COUNT;
        groupUserCount.dwValue2 = m_GroupInfo->dwMasterID;
        groupUserCount.wUserCount = m_GroupInfo->wUserCount;

        CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupUserCount, sizeof(MSG_SC_Update_GroupUser_Count));
    }

    CGroupMgr::get_instance().SendAllGameSserverAddGroupUser(m_GroupInfo->dwGroupID, pGroupUser);
}

CT_VOID CGroup::OperateApplyAddGroup(CT_DWORD dwGroupID, CT_DWORD dwApplyUserID, CT_DWORD dwMasterID, CT_BYTE cbOperateCode)
{
    /*
	auto it = m_mapApplyUser.find(dwApplyUserID);
	if (it == m_mapApplyUser.end())
	{
		return;
	}

	//不是群主或者副群主操做
	bool  bIsAdmin = false;
	if (dwMasterID == m_GroupInfo->dwMasterID)
	{
		bIsAdmin = true;
	}

	if (!bIsAdmin)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(dwMasterID, PERMISSION_DENIED);
        return;
    }

    if (cbOperateCode == 1)
	{
		if (GetGroupUserCount() >= MAX_GROUP_USER_COUNT)
		{
			CGroupMgr::get_instance().SendGroupErrorMsg(dwMasterID, GROUP_USER_FULL);
		}
		else
		{
			tagGroupUserInfo groupUserInfo;
			groupUserInfo.dwUserID = it->second->dwApplyUserID;
			groupUserInfo.cbSex = it->second->cbSex;
			groupUserInfo.cbHeadId = it->second->cbHeadId;
			groupUserInfo.cbVip2 = it->second->cbVip2;
			groupUserInfo.dwRecharge = 0;
			groupUserInfo.llTotalIncome = 0;
			groupUserInfo.dwLastDate = it->second->dwApplyTime;
			//_snprintf_info(groupUserInfo.szNickName, sizeof(groupUserInfo.szNickName), "%s", it->second->szNickName);
			//_snprintf_info(groupUserInfo.szHeadUrl, sizeof(groupUserInfo.szHeadUrl), "%s", it->second->szHeadUrl);

			AddGroupUser(&groupUserInfo);
			CGroupMgr::get_instance().SendAllGameSserverAddGroupUser(dwGroupID, &groupUserInfo);
		}
	}

    //返回操作给管理员
    MSG_SC_Op_Apply_AddGroup opApplyAddGroup;
    opApplyAddGroup.dwGroupID = dwGroupID;
    opApplyAddGroup.dwUserID = dwApplyUserID;
    SendMsgToAdministrator(SUB_SC_OP_APPLY_ADD_GROUP, &opApplyAddGroup, sizeof(MSG_SC_Op_Apply_AddGroup));

	//删除申请记录
	m_mapApplyUser.erase(it);
    */
}

CT_VOID	CGroup::QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwUserID);
	if (it == m_mapGroupUser.end())
	{
		return;
	}

	//db删除
	RemoveGroupUserFromDB(dwGroupID, dwUserID);

	//下发消息
	MSG_SC_Remove_GroupUser removeUser;
	removeUser.dwMainID = MSG_GROUP_MAIN;
	removeUser.dwSubID = SUB_SC_QUIT_GROUP_USER;
	removeUser.dwGroupID = dwGroupID;
	removeUser.dwUserID = dwUserID;

	//下发删除的消息给本人
	acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pProxySocket)
	{
		removeUser.dwValue2 = dwUserID;
		CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
	}

	//推送给群主
    pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwMasterID);
    if (pProxySocket)
    {
        removeUser.dwValue2 = m_GroupInfo->dwMasterID;
        CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
    }

    //推送给副群主
    /*if (m_GroupInfo->dwDeputyID1 != 0)
	{
		pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwDeputyID1);
		if (pProxySocket)
		{
			removeUser.dwValue2 = m_GroupInfo->dwDeputyID1;
			CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
		}
	}

	if (m_GroupInfo->dwDeputyID2 != 0)
	{
		pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwDeputyID2);
		if (pProxySocket)
		{
			removeUser.dwValue2 = m_GroupInfo->dwDeputyID2;
			CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
		}
	}*/

	/*for (auto& itUser : m_mapGroupUser)
	{
		acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(itUser.second->dwUserID);
		if (pProxySocket)
		{
			removeUser.dwValue2 = itUser.second->dwUserID;
			CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
		}
	}*/

	//内存数据删除
	CGroupMgr::get_instance().RemoveGroupUser(dwGroupID, dwUserID);
	CGroupMgr::get_instance().SendAllGameServerRemoveGroupUser(dwGroupID, dwUserID);

}

CT_VOID	CGroup::TickOutGroupUser(CT_DWORD dwGroupID, CT_DWORD dwMasterID, CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwUserID);
	if (it == m_mapGroupUser.end())
	{
		CGroupMgr::get_instance().SendGroupErrorMsg(dwMasterID, NOT_GROUP_USER);
		return;
	}

	bool bCanTickUser = false;
	if (m_GroupInfo->dwMasterID == dwMasterID)
    {
	    bCanTickUser = true;
    }
	/*else if (m_GroupInfo->dwDeputyID1 != 0 && m_GroupInfo->dwDeputyID1 == dwMasterID)
    {
	    bCanTickUser = true;
    }
	else if (m_GroupInfo->dwDeputyID2 != 0 && m_GroupInfo->dwDeputyID2 == dwMasterID)
    {
	    bCanTickUser = true;
    }*/

	//权限验证
	if (!bCanTickUser)
	{
        CGroupMgr::get_instance().SendGroupErrorMsg(dwMasterID, PERMISSION_DENIED);
        return;
	}

	//db删除
	RemoveGroupUserFromDB(dwGroupID, dwUserID);

	//下发消息
	MSG_SC_Remove_GroupUser removeUser;
	removeUser.dwMainID = MSG_GROUP_MAIN;
	removeUser.dwSubID = SUB_SC_TICK_GROUP_USER;
	removeUser.dwGroupID = dwGroupID;
	removeUser.dwUserID = dwUserID;

	//给群主和副群主发T人的消息
    acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
    if (pProxySocket)
    {
        removeUser.dwValue2 = dwUserID;
        CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
    }

    //推送给群主
    pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwMasterID);
    if (pProxySocket)
    {
        removeUser.dwValue2 = m_GroupInfo->dwMasterID;
        CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
    }

    //推送给副群主
    /*if (m_GroupInfo->dwDeputyID1 != 0)
    {
        pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwDeputyID1);
        if (pProxySocket)
        {
            removeUser.dwValue2 = m_GroupInfo->dwDeputyID1;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
        }
    }

    if (m_GroupInfo->dwDeputyID2 != 0)
    {
        pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwDeputyID2);
        if (pProxySocket)
        {
            removeUser.dwValue2 = m_GroupInfo->dwDeputyID2;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
        }
    }*/

	/*
	for (auto& itUser : m_mapGroupUser)
	{
		acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(itUser.second->dwUserID);
		if (pProxySocket)
		{
			removeUser.dwValue2 = itUser.second->dwUserID;
			CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &removeUser, sizeof(removeUser));
		}
	}
	*/

	//内存数据删除
	CGroupMgr::get_instance().RemoveGroupUser(dwGroupID, dwUserID);
	CGroupMgr::get_instance().SendAllGameServerRemoveGroupUser(dwGroupID, dwUserID);
}

CT_VOID CGroup::QueryGroupUserInfo(CT_DWORD dwUserID)
{
    acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
    if (pProxySocket == NULL)
    {
        return;
    }

    //查询玩家的游戏数据
	CT_BYTE szBuffer[512];
	CT_DWORD dwSendSize = 0;

	MSG_GameMsgDownHead head;
	head.dwMainID = MSG_GROUP_MAIN;
	head.dwSubID = SUB_SC_QUERY_GROUP_USER_INFO;
	head.dwValue2 = dwUserID;
    memcpy(szBuffer, &head, sizeof(MSG_GameMsgDownHead));
    dwSendSize += sizeof(MSG_GameMsgDownHead);

	//for test
    MSG_SC_Query_GroupUserInfo userInfo;
    userInfo.wGameID = 3;
    userInfo.wKindID = 1;
    userInfo.wRoomKindID = 1;
    userInfo.dwRoomCount = 100;
    userInfo.dIncome = 8888.8f;
    memcpy(szBuffer + dwSendSize, &userInfo, sizeof(MSG_SC_Query_GroupUserInfo));

    CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
}

CT_BOOL	CGroup::IsGroupUser(CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwUserID);
	if (it != m_mapGroupUser.end())
	{
		return true;
	}

	return false;
}

CT_BOOL	CGroup::IsApplyAddGroup(CT_DWORD dwUserID)
{
	/*
	auto it = m_mapApplyUser.find(dwUserID);
	if (it != m_mapApplyUser.end())
	{
		return true;
	}
	*/

	return false;
}

std::unique_ptr<tagGroupRoomConfig>* CGroup::GetGroupRoomCfg(CT_WORD wGameID, CT_WORD wKindID)
{
    /*
	for (auto& it : m_vecPRoomCfg)
	{
		if (it->wGameID == wGameID && it->wKindID == wKindID)
		{
			return &it;
		}
	}
     */

	return NULL;
}

CT_VOID	CGroup::RemoveGroupUserFromDB(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	//到数据库去删除用户
	MSG_CS2DB_Remove_GroupUser removeUser;
	removeUser.dwGroupID = dwGroupID;
	removeUser.dwUserID = dwUserID;

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_DEL_GROUP_USER, &removeUser, sizeof(MSG_CS2DB_Remove_GroupUser));
}

CT_DWORD CGroup::GetGroupUserCount()
{
	return (CT_DWORD)m_mapGroupUser.size();
}

CT_VOID  CGroup::ModifyMemberIncomeRate(CT_DWORD dwUserID, CT_BYTE cbIncomeRate)
{
    auto it = m_mapGroupUser.find(dwUserID);
    if (it == m_mapGroupUser.end())
    {
        return;
    }

    std::unique_ptr<tagGroupUserInfo>& uniquePtrUserInfo = it->second;
    //判断上级的收入比例
    if (uniquePtrUserInfo->dwParentUserId != 0)
    {
        tagGroupUserInfo* pParentUserInfo = GetGroupUserInfo(uniquePtrUserInfo->dwParentUserId);
        if (pParentUserInfo->cbInComeRate - 1 < cbIncomeRate)
        {
            //CGroupMgr::get_instance().SendGroupErrorMsg()
            return;
        }
    }

    uniquePtrUserInfo->cbInComeRate = cbIncomeRate;
}

CT_BYTE CGroup::GetMemberPosition(CT_DWORD dwUserID)
{
    CT_BYTE cbPosition = en_Member;
    if (m_GroupInfo->dwMasterID == dwUserID)
    {
        cbPosition = en_Master;
    }
    /*else if (m_GroupInfo->dwDeputyID1 == dwUserID || m_GroupInfo->dwDeputyID2 == dwUserID)
    {
        cbPosition = en_Deputy;
    }*/
    return cbPosition;
}

tagGroupUserInfo* CGroup::GetGroupUserInfo(CT_DWORD dwUserID)
{
    auto it = m_mapGroupUser.find(dwUserID);
    if (it == m_mapGroupUser.end())
    {
        return NULL;
    }

    return it->second.get();
}

CT_VOID  CGroup::SendMsgToAdministrator(CT_DWORD dwSubID, CT_VOID* pData, CT_DWORD dwDataSize)
{
    MSG_GameMsgDownHead head;
    head.dwMainID = MSG_GROUP_MAIN;
    head.dwSubID = dwSubID;

    CT_BYTE szBuffer[1024];
    CT_DWORD dwSendSize = 0;

    //推送给群主
    acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwMasterID);
    if (pProxySocket)
    {
        head.dwValue2 = m_GroupInfo->dwMasterID;
        memcpy(szBuffer, &head, sizeof(MSG_GameMsgDownHead));
        dwSendSize += sizeof(MSG_GameMsgDownHead);

        memcpy(szBuffer+dwSendSize, pData, dwDataSize);
        dwSendSize += dwDataSize;
        CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
    }

    //推送给副群主
    /*if (m_GroupInfo->dwDeputyID1 != 0)
    {
        pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwDeputyID1);
        if (pProxySocket)
        {
            dwSendSize = 0;
            head.dwValue2 = m_GroupInfo->dwDeputyID1;
            memcpy(szBuffer, &head, sizeof(MSG_GameMsgDownHead));
            dwSendSize += sizeof(MSG_GameMsgDownHead);

            memcpy(szBuffer+dwSendSize, pData, dwDataSize);
            dwSendSize += dwDataSize;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
        }
    }

    if (m_GroupInfo->dwDeputyID2 != 0)
    {
        pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(m_GroupInfo->dwDeputyID2);
        if (pProxySocket)
        {
            dwSendSize = 0;
            head.dwValue2 = m_GroupInfo->dwDeputyID2;
            memcpy(szBuffer, &head, sizeof(MSG_GameMsgDownHead));
            dwSendSize += sizeof(MSG_GameMsgDownHead);

            memcpy(szBuffer+dwSendSize, pData, dwDataSize);
            dwSendSize += dwDataSize;
            CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
        }
    }*/
}

CT_VOID CGroup::UpdateIncomeTodb(CT_DWORD dwTodayInt, bool bCrossDays)
{
    CT_BYTE     szBuffer[SYS_NET_SENDBUF_SIZE];
    CT_DWORD    dwSendSize = 0;

    /*
    CT_DWORD    dwDateInt;                              //日期
    CT_DWORD    dwUserID;                               //玩家ID
    CT_DWORD    dwSubUserID;                            //下级玩家ID
    CT_DWORD    dwOriginalIncome;                       //原始收入
    CT_DWORD    dwDeductIncome;                         //扣量后的收入
    CT_WORD     wDeductRate;                            //扣量比例(千分比)
     */
    MSG_CS2DB_Update_GroupIncome groupIncome;
    groupIncome.dwDateInt = dwTodayInt;
    groupIncome.cbCrossDays = bCrossDays;
    groupIncome.dwGroupID = m_GroupInfo->dwGroupID;

    //直属收入
    for (auto& itAllDateDirIncome : m_mapDateDirIncome)
    {
        MapGroupUserDateIncome& mapGroupUserDateIncome = itAllDateDirIncome.second;
        auto itDateDirIncome = mapGroupUserDateIncome.find(dwTodayInt);
        if (itDateDirIncome != mapGroupUserDateIncome.end())
        {
            MapGroupUserIncome& mapGroupUserIncome = itDateDirIncome->second;
            for (auto& itUserIncome : mapGroupUserIncome)
            {
                if (dwSendSize + sizeof(MSG_CS2DB_Update_GroupIncome) >= sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_GROUP_DIR_INCOME, &szBuffer, dwSendSize);
                    dwSendSize = 0;
                }

                tagGroupUserInfo* pGroupUserInfo = GetGroupUserInfo(itUserIncome.first);
                if (pGroupUserInfo == NULL)
                    continue;

                groupIncome.dwUserID = itAllDateDirIncome.first;
                groupIncome.dwSubUserID = itUserIncome.first;
                groupIncome.dwParentUserID = pGroupUserInfo->dwParentUserId;
                groupIncome.dwOriginalIncome = itUserIncome.second->dwOriginalIncome;
                groupIncome.dwDeductIncome = itUserIncome.second->dwDeductIncome;
                groupIncome.wDeductRate = itUserIncome.second->wDeductRate;
                memcpy(szBuffer + dwSendSize, &groupIncome, sizeof(MSG_CS2DB_Update_GroupIncome));
                dwSendSize += sizeof(MSG_CS2DB_Update_GroupIncome);
            }
        }
    }

    if (dwSendSize > 0)
    {
        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_GROUP_DIR_INCOME, &szBuffer, dwSendSize);
        dwSendSize = 0;
    }

    //附属收入
    for (auto& itAllDateSubIncome : m_mapDateSubIncome)
    {
        MapGroupUserDateIncome& mapGroupUserDateIncome = itAllDateSubIncome.second;
        auto itDateDirIncome = mapGroupUserDateIncome.find(dwTodayInt);
        if (itDateDirIncome != mapGroupUserDateIncome.end())
        {
            MapGroupUserIncome& mapGroupUserIncome = itDateDirIncome->second;
            for (auto& itUserIncome : mapGroupUserIncome)
            {
                if (dwSendSize + sizeof(MSG_CS2DB_Update_GroupIncome) >= sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_GROUP_SUB_INCOME, &szBuffer, dwSendSize);
                    dwSendSize = 0;
                }

                tagGroupUserInfo* pGroupUserInfo = GetGroupUserInfo(itUserIncome.first);
                if (pGroupUserInfo == NULL)
                    continue;

                groupIncome.dwUserID = itAllDateSubIncome.first;
                groupIncome.dwSubUserID = itUserIncome.first;
                groupIncome.dwParentUserID = pGroupUserInfo->dwParentUserId;
                groupIncome.dwOriginalIncome = itUserIncome.second->dwOriginalIncome;
                groupIncome.dwDeductIncome = itUserIncome.second->dwDeductIncome;
                groupIncome.wDeductRate = itUserIncome.second->wDeductRate;
                memcpy(szBuffer + dwSendSize, &groupIncome, sizeof(MSG_CS2DB_Update_GroupIncome));
                dwSendSize += sizeof(MSG_CS2DB_Update_GroupIncome);
            }
        }
    }

    if (dwSendSize > 0)
    {
        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_GROUP_SUB_INCOME, &szBuffer, dwSendSize);
    }
}
