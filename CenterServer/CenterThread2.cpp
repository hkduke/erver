#include "stdafx.h"
#include "CenterThread.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "CMD_Game.h"
#include "CMD_Dip.h"
#include "GlobalEnum.h"
#include "ServerCfg.h"
#include "UserMgr.h"
#include "CMD_ShareMsg.h"
#include "CMD_Maintain.h"
#include "CMD_Customer.h"
#include "SystemMsgMgr.h"
#include "UserTask.h"
#include "ConfigOption.h"
#include <algorithm>
#include "timeFunction.h"

extern CNetConnector *pNetDB;

////////////////////////////////////////////////////DB返回操作///////////////////////////////////////////////////////////
CT_VOID CCenterThread::OnSubLoadGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize % sizeof(tagGroupInfo) != 0)
    {
        return;
    }

    CT_DWORD dwCount = wDataSize / sizeof(tagGroupInfo);
    tagGroupInfo* pGroupInfo = (tagGroupInfo*)pData;
    if (pGroupInfo == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i < dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupInfo(pGroupInfo++);
    }
}

CT_VOID CCenterThread::OnSubLoadGroupUser(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if ((wDataSize - sizeof(CT_DWORD)) % sizeof(tagGroupUserInfo) != 0)
    {
        return;
    }

    CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(tagGroupUserInfo);

    CT_DWORD dwGroupID = *((CT_DWORD*)pData);

    tagGroupUserInfo* pGroupInfo = (tagGroupUserInfo*)((CT_BYTE*)pData + sizeof(CT_DWORD));
    if (pGroupInfo == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupUser(dwGroupID, pGroupInfo++);
    }
}

CT_VOID CCenterThread::OnSubLoadGroupUserRevenue(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if ((wDataSize - sizeof(CT_DWORD)) % sizeof(tagGroupUserRevenue) != 0)
    {
        return;
    }

    CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(tagGroupUserRevenue);

    CT_DWORD dwGroupID = *((CT_DWORD*)pData);

    tagGroupUserRevenue* pGroupUserRevenue = (tagGroupUserRevenue*)((CT_BYTE*)pData + sizeof(CT_DWORD));
    if (pGroupUserRevenue == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupUserTodayRevenue(dwGroupID, pGroupUserRevenue++);
    }
}

CT_VOID CCenterThread::OnSubLoadGroupUserDirIncome(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if ((wDataSize - sizeof(CT_DWORD)) % sizeof(tagGroupUserDateIncomeDetail) != 0)
    {
        return;
    }

    CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(tagGroupUserDateIncomeDetail);

    CT_DWORD dwGroupID = *((CT_DWORD*)pData);

    tagGroupUserDateIncomeDetail* pGroupUserDateDirIncome = (tagGroupUserDateIncomeDetail*)((CT_BYTE*)pData + sizeof(CT_DWORD));
    if (pGroupUserDateDirIncome == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupUserDateDirIncome(dwGroupID, pGroupUserDateDirIncome++);
    }
}

CT_VOID CCenterThread::OnSubLoadGroupUserSubIncome(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if ((wDataSize - sizeof(CT_DWORD)) % sizeof(tagGroupUserDateIncomeDetail) != 0)
    {
        return;
    }

    CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(tagGroupUserDateIncomeDetail);

    CT_DWORD dwGroupID = *((CT_DWORD*)pData);

    tagGroupUserDateIncomeDetail* pGroupUserDateSubIncome = (tagGroupUserDateIncomeDetail*)((CT_BYTE*)pData + sizeof(CT_DWORD));
    if (pGroupUserDateSubIncome == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupUserDateSubIncome(dwGroupID, pGroupUserDateSubIncome++);
    }
}

CT_VOID CCenterThread::OnSubLoadGroupUserTotalIncome(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if ((wDataSize - sizeof(CT_DWORD)) % sizeof(tagGroupUserTotalIncome) != 0)
    {
        return;
    }

    CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(tagGroupUserTotalIncome);

    CT_DWORD dwGroupID = *((CT_DWORD*)pData);

    tagGroupUserTotalIncome* pGroupUserTotalIncome = (tagGroupUserTotalIncome*)((CT_BYTE*)pData + sizeof(CT_DWORD));
    if (pGroupUserTotalIncome == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupUserTotalIncome(dwGroupID, pGroupUserTotalIncome++);
    }
}

CT_VOID CCenterThread::OnSubLoadGroupChatMsgId(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(CT_DWORD))
    {
        return;
    }

    CT_DWORD* pMsgId = (CT_DWORD*)pData;
    CGroupMgr::get_instance().SetChatMsgId(*pMsgId);
}

CT_VOID CCenterThread::OnSubLoadGroupUnreadChatMsg(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if ((wDataSize - sizeof(CT_DWORD)) % sizeof(tagGroupChat) != 0)
    {
        return;
    }

    CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(tagGroupChat);

    CT_DWORD dwGroupID = *((CT_DWORD*)pData);

    tagGroupChat* groupChat = (tagGroupChat*)((CT_BYTE*)pData + sizeof(CT_DWORD));
    if (groupChat == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupChatMsg(dwGroupID, groupChat++);
    }
}

CT_VOID CCenterThread::OnSubQueryGroupExchange(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if ((wDataSize - sizeof(CT_DWORD)) % sizeof(MSG_SC_Query_Group_Settle_Record) != 0)
    {
        return;
    }

    //CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(MSG_SC_Query_Group_Settle_Record);

    CT_DWORD dwUserID = *((CT_DWORD*)pData);

    MSG_SC_Query_Group_Settle_Record* pGroupSettleRecord = (MSG_SC_Query_Group_Settle_Record*)((CT_BYTE*)pData + sizeof(CT_DWORD));
    if (pGroupSettleRecord == NULL)
    {
        return;
    }

    CGroupMgr::get_instance().QueryGroupUserSettleRecord(dwUserID, pGroupSettleRecord, wDataSize - sizeof(CT_DWORD));
}

CT_VOID CCenterThread::OnSubLoadQueryBeAddGroupUser(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_DB2CS_BeAdd_Group_User))
    {
        return;
    }

    MSG_DB2CS_BeAdd_Group_User* pBeAddUser = (MSG_DB2CS_BeAdd_Group_User*)pData;
    if (pBeAddUser->cbResult == 1)
    {
        if (pBeAddUser->cbType == 0)
        {
            /*MSG_SC_Query_UserInfo queryUserResult;
            queryUserResult.dwMainID = MSG_GROUP_MAIN;
            queryUserResult.dwSubID = SUB_SC_QUERY_USER_INFO;
            queryUserResult.dwValue2 = pBeAddUser->dwOperateUserID;
            queryUserResult.dwUserID = pBeAddUser->dwUserID;
            _snprintf_info(queryUserResult.szNickName, sizeof(queryUserResult.szNickName), "%s", pBeAddUser->szNickName);
            queryUserResult.cbState = 0;

            acl::aio_socket_stream* pMasterProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(pBeAddUser->dwOperateUserID);
            if (pMasterProxySock != NULL)
            {
                CNetModule::getSingleton().Send(pMasterProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &queryUserResult, sizeof(queryUserResult));
            }*/
        }
        else if (pBeAddUser->cbType == 1)
        {
            std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(pBeAddUser->dwGroupID);
            if (pGroup == NULL)
            {
                return;
            }

            tagGroupUserInfo groupUserInfo;
            groupUserInfo.dwUserID = pBeAddUser->dwUserID;
            //_snprintf_info(groupUserInfo.szNickName, sizeof(groupUserInfo.szNickName), "%s", pBeAddUser->szNickName);
            //_snprintf_info(groupUserInfo.szHeadUrl, sizeof(groupUserInfo.szHeadUrl), "%s", pBeAddUser->szHeadUrl);

            (*pGroup)->AddGroupUser(&groupUserInfo);
            CGroupMgr::get_instance().SendAllGameSserverAddGroupUser(pBeAddUser->dwGroupID, &groupUserInfo);
        }
        else if (pBeAddUser->cbType == 2 || pBeAddUser->cbType == 3)
        {
            //CGroupMgr::get_instance().InsertApplyGroupInfo(pBeAddUser->dwGroupID, pBeAddUser->dwUserID, pBeAddUser->szNickName, pBeAddUser->szHeadUrl, 1, 1);
        }
    }
    else
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pBeAddUser->dwOperateUserID, USER_NOT_ONLINE);
    }
}

CT_VOID CCenterThread::OnSubLoadDefaultPRoomConfig(const CT_VOID * pData, CT_DWORD wDataSize)
{
    /*if (wDataSize%sizeof(tagDefaultRoomConfig) != 0)
    {
        return;
    }

    CT_DWORD dwCount = wDataSize / sizeof(tagDefaultRoomConfig);
    tagDefaultRoomConfig* pGroupInfo = (tagDefaultRoomConfig*)pData;
    if (pGroupInfo == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertDefaultPRoomConfig(pGroupInfo++);
    }*/
}

CT_VOID CCenterThread::OnSubLoadPRoomConfig(const CT_VOID * pData, CT_DWORD wDataSize)
{
    /*if (wDataSize % sizeof(tagGroupRoomConfig) != 0)
    {
        return;
    }

    CT_DWORD dwCount = wDataSize / sizeof(tagGroupRoomConfig);
    tagGroupRoomConfig* pGroupInfo = (tagGroupRoomConfig*)pData;
    if (pGroupInfo == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupPRoomConfig(pGroupInfo++);
    }*/
}

CT_VOID CCenterThread::OnSubLoadGroupInfoFinish(const CT_VOID * pData, CT_DWORD wDataSize)
{
    //把数据发送给所有gs
    const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
    if (pMapGameServer)
    {
        auto it = pMapGameServer->begin();
        for (; it != pMapGameServer->end(); ++it)
        {
            CGroupMgr::get_instance().SendGroupToGameServer(it->first, (CMD_GameServer_Info*)&it->second);
        }
    }
}

CT_VOID CCenterThread::OnSubDBCreateGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagGroupInfo))
    {
        return;
    }

    tagGroupInfo* pGroupInfo = (tagGroupInfo*)pData;

    if (pGroupInfo == NULL)
    {
        return;
    }

    //增加群组
    CGroupMgr::get_instance().InsertGroupInfo(pGroupInfo);

    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pGroupInfo->dwMasterID);
    if (pUserInfo)
    {
        //把群主赠加到成员列表
        tagGroupLevelCfg* pGroupLevelCfg = CGroupMgr::get_instance().GetLevelCfg(pGroupInfo->cbLevel);
        tagGroupUserInfo userInfo;
        userInfo.dwUserID = pGroupInfo->dwMasterID;
        userInfo.dwParentUserId = 0;
        userInfo.llTotalIncome = 0;
        userInfo.dwRecharge = 0;
        userInfo.cbSex = pUserInfo->cbSex;
        userInfo.cbHeadId = pUserInfo->cbHeadID;
        userInfo.cbVip2 = pUserInfo->cbVip2;
        userInfo.cbInComeRate = pGroupInfo->cbIncomeRate;
        userInfo.wDeductRate = (pGroupLevelCfg != NULL ? pGroupLevelCfg->wDeductRate : 500);
        userInfo.wDeductTop = (pGroupLevelCfg != NULL ? pGroupLevelCfg->wDeductTop : 700);
        userInfo.wDeductIncre = (pGroupLevelCfg != NULL ? pGroupLevelCfg->wDeductIncre : 2);
        userInfo.dwDeductStart = (pGroupLevelCfg != NULL ? pGroupLevelCfg->dwDeductStart : 30000);
        userInfo.dwRegDate = time(NULL);
        userInfo.dwLastDate = userInfo.dwRegDate;
        CGroupMgr::get_instance().AddGroupUser(pGroupInfo->dwGroupID, &userInfo);

        pUserInfo->dwGroupID = pGroupInfo->dwGroupID;

        if (pUserInfo->bOnline)
        {
            //下发送创建成功的信息
            acl::aio_socket_stream* pProxySock = CServerMgr::get_instance().FindProxyServer(pUserInfo->dwProxyServerID);
            if (pProxySock)
            {
                MSG_SC_CreateGroup createGroup;
                createGroup.dwMainID = MSG_GROUP_MAIN;
                createGroup.dwSubID = SUB_SC_CREATE_GROUP;
                createGroup.dwValue2 = pGroupInfo->dwMasterID;

                tagGroupLevelCfg* pGroupLevelCfg = CGroupMgr::get_instance().GetLevelCfg(pGroupInfo->cbLevel);
                createGroup.groupInfo.dwGroupID = pGroupInfo->dwGroupID;
                createGroup.groupInfo.dwMasterID = pGroupInfo->dwMasterID;
                createGroup.groupInfo.dwContribution = 0;
                createGroup.groupInfo.dwNextContribution = (pGroupLevelCfg != NULL ? pGroupLevelCfg->dwMaxContribution : 999999999);
                createGroup.groupInfo.cbLevel = pGroupInfo->cbLevel;
                createGroup.groupInfo.cbIcon = pGroupInfo->cbIcon;
                createGroup.groupInfo.cbSettleDays = pGroupInfo->cbSettleDays;
                createGroup.groupInfo.cbIncomeRate = pGroupInfo->cbIncomeRate;
                //createGroup.groupInfo.cbPosition = en_Master;
                createGroup.groupInfo.dwGroupUserCount = 1;
                _snprintf_info(createGroup.groupInfo.szGroupName, sizeof(createGroup.groupInfo.szGroupName), "%s", pGroupInfo->szGroupName);
                _snprintf_info(createGroup.groupInfo.szNotice, sizeof(createGroup.groupInfo.szNotice), "%s", pGroupInfo->szNotice);
                _snprintf_info(createGroup.groupInfo.szDomain, sizeof(createGroup.groupInfo.szDomain), "%s", pGroupLevelCfg != NULL ? pGroupLevelCfg->szDomain : "http://test.cdn.ske666.com/");
                //createGroup.groupUser.dwUserID = userInfo.dwUserID;
                //createGroup.groupUser.dWinScore = 0;
                //createGroup.groupUser.dwRecharge = 0;

                CNetModule::getSingleton().Send(pProxySock, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &createGroup, sizeof(MSG_SC_CreateGroup));
            }
        }

        //同步给所有GS
        CGroupMgr::get_instance().SendAllGameServerAddGroup(pGroupInfo);
        CGroupMgr::get_instance().SendAllGameSserverAddGroupUser(pGroupInfo->dwGroupID, &userInfo);
    }
}

CT_VOID CCenterThread::OnSubDBDeductGroupMaterGem(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_DB2CS_Deduct_GroupMaster_Gem))
    {
        return;
    }

    MSG_DB2CS_Deduct_GroupMaster_Gem* pDeductResult = (MSG_DB2CS_Deduct_GroupMaster_Gem*)pData;
    if (pDeductResult->cbResult == 0)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pDeductResult->dwUserID, MASTER_GEM_NOT_ENOUGH);
        return;
    }

    // 更新cache的钻石
    long long int llNewGem = 0;
    int nNeedGem = -(pDeductResult->wNeedGem);
    UpdateUserGem(pDeductResult->dwMasterID, nNeedGem, llNewGem, 1);

    //查找群组的房间
    const CT_CHAR* pPRoomJson = NULL;
    /*std::unique_ptr<tagGroupRoomConfig>* pGroupRoomCfg = CGroupMgr::get_instance().GetGroupRoomCfg(pDeductResult->dwGroupID, pDeductResult->wGameID, pDeductResult->wKindID);
    if (pGroupRoomCfg == NULL)
    {
        std::shared_ptr<tagDefaultRoomConfig>* pDefaultRoomCfg = CGroupMgr::get_instance().GetDefaultRoomCfg(pDeductResult->wGameID, pDeductResult->wKindID);
        if (pDefaultRoomCfg == NULL)
        {
            return;
        }

        pPRoomJson = (*pDefaultRoomCfg)->szConfig;
    }
    else
    {
        pPRoomJson = (*pGroupRoomCfg)->szConfig;
    }*/

    CT_DWORD dwRoomNum = 0;
    CT_WORD	 wCurrUserCount = 0;
    CT_BOOL bSucc = CreatePRoomForGroup(pPRoomJson, pDeductResult->dwGroupID, dwRoomNum, wCurrUserCount);
    if (bSucc)
    {
        acl::json proomJson(pPRoomJson);
        acl::json_node& root = proomJson.get_root();
        acl::json_node *node;

#ifdef  _OS_WIN32_CODE
        node = &proomJson.create_node("roomnum", (__int64)dwRoomNum);
			root.add_child(node);
			node = &proomJson.create_node("curUserCount", (__int64)wCurrUserCount);
			root.add_child(node);
#else
        node = &proomJson.create_node("roomnum", (long long int)dwRoomNum);
        root.add_child(node);
        node = &proomJson.create_node("curUserCount", (long long int)wCurrUserCount);
        root.add_child(node);

#endif //  _OS_WIN32_CODE
        node = &proomJson.create_node("time", Utility::GetTimeNowString().c_str());
        root.add_child(node);

        MSG_SC_Create_GroupPRoom proomResult;
        proomResult.dwMainID = MSG_GROUP_MAIN;
        proomResult.dwSubID = SUB_SC_GROUP_CREATE_PROOM;
        proomResult.dwGroupID = pDeductResult->dwGroupID;
        proomResult.dwUserID = pDeductResult->dwUserID;
        acl::string jsonResult;
        proomJson.build_json(jsonResult);
        _snprintf_info(proomResult.szJsonResult, sizeof(proomResult.szJsonResult), "%s", jsonResult.c_str());

        std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(pDeductResult->dwGroupID);
        if (pGroup)
        {
            std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = (*pGroup)->GetAllGroupUserInfo();
            for (auto& it : mapGroupUserInfo)
            {
                acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
                if (pProxySocket)
                {
                    proomResult.dwValue2 = it.first;
                    CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &proomResult, sizeof(proomResult));
                }
            }
        }

        //到DB写入房间记录数据
        MSG_CS2DB_GroupRoom_Info groupRoomInfo;
        groupRoomInfo.dwGroupID = pDeductResult->dwGroupID;
        groupRoomInfo.wNeedGem = pDeductResult->wNeedGem;
        groupRoomInfo.dwRoomNum = dwRoomNum;
        groupRoomInfo.wGameID = pDeductResult->wGameID;
        groupRoomInfo.wKindID = pDeductResult->wKindID;
        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_GROUP_ROOM_INFO, &groupRoomInfo, sizeof(groupRoomInfo));
    }
}

CT_VOID CCenterThread::OnSubDBGroupRoomInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_DB2CS_GroupRoom_Info))
    {
        return;
    }

    MSG_DB2CS_GroupRoom_Info* pGroupRoom = (MSG_DB2CS_GroupRoom_Info*)pData;
    SetGroupPRoomRecordID(pGroupRoom->dwRoomNum, pGroupRoom->dwRecordID);
}

CT_VOID CCenterThread::OnSubDBWXGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize < sizeof(MSG_DB2CS_WXGroupInfo))
    {
        return;
    }

    MSG_DB2CS_WXGroupInfo* pGroupInfo = (MSG_DB2CS_WXGroupInfo*)pData;
    if (pGroupInfo == NULL)
    {
        return;
    }

    CT_DWORD* pUserID = (CT_DWORD*)((CT_BYTE*)pData + sizeof(MSG_DB2CS_WXGroupInfo));
    m_wxGroupMgr.InsertGroupInfo(pGroupInfo->dwGroupID, pGroupInfo->dwBindUserPlay);
    while (pGroupInfo->dwUserCount > 0)
    {
        m_wxGroupMgr.InsertGroupUser(pGroupInfo->dwGroupID, *pUserID);
        pGroupInfo->dwUserCount--;
        pUserID++;
    }
}

CT_VOID CCenterThread::OnSubDBWxGroupFinish(const CT_VOID * pData, CT_DWORD wDataSize)
{
    //把数据发送给所有gs
    const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
    if (pMapGameServer)
    {
        auto it = pMapGameServer->begin();
        for (; it != pMapGameServer->end(); ++it)
        {
            m_wxGroupMgr.SendGroupToGameServer(it->first);
        }
    }
}

CT_VOID CCenterThread::OnSubLoadGroupLevelCfg(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize % sizeof(tagGroupLevelCfg) != 0)

    {
        return;
    }

    CT_DWORD dwCount = wDataSize / sizeof(tagGroupLevelCfg);
    tagGroupLevelCfg* pGroupLevel = (tagGroupLevelCfg*)pData;
    if (pGroupLevel == NULL)
    {
        return;
    }

    for (CT_DWORD i = 0; i != dwCount; ++i)
    {
        CGroupMgr::get_instance().InsertGroupLevelConfig(pGroupLevel++);
    }
}

/////////////////////////////////////////////////////俱乐部消息//////////////////////////////////////////////////////////
CT_VOID CCenterThread::OnGroupSubCreateGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Create_Group))
    {
        return;
    }

    MSG_CS_Create_Group* pCreateGroup = (MSG_CS_Create_Group*)pData;
    if (pCreateGroup == NULL)
    {
        return;
    }

    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pCreateGroup->dwUserID);
    if (pUserInfo == NULL)
    {
        LOG(WARNING) << "create group not find user info!， user id: " << pCreateGroup->dwUserID;
        return;
    }

    //检测目前玩家是否有群
    CT_BOOL bHasGroup = CGroupMgr::get_instance().IsUserHasGroup(pCreateGroup->dwUserID);
    if (bHasGroup)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pCreateGroup->dwUserID, HAS_GROUP_CANNOT_CREATE);
        return;
    }

    //判断玩家的VIP等级情况
    if (pUserInfo->cbVip2 < CAN_CREATE_GROUP_VIP)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pCreateGroup->dwUserID, CREATE_GROUP_VIP_NOT_ENOUGH);
        return;
    }

    //检测群数量(目前接入的数量为1)
    /*CT_DWORD dwGroupCount = CGroupMgr::get_instance().CalUserGroupCount(pCreateGroup->dwUserID);
    if (dwGroupCount >= MAX_GROUP_COUNT)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pCreateGroup->dwUserID, GROUP_COUNT_MAX);
        return;
    }*/

    if (strlen(pCreateGroup->szGroupName) == 0 || strlen(pCreateGroup->szNotice) == 0)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pCreateGroup->dwUserID, GROUP_NAME_ERROR);
        return;
    }

    CT_WORD  wBaseIncomeRate = CGroupMgr::get_instance().GetBaseIncomeRate();
    CT_BYTE  cbSettleDays = CGroupMgr::get_instance().GetBaseSettleDays();
    //db创建组
    MSG_CS2DB_CreateGroup createGroup;
    createGroup.dwMasterID = pCreateGroup->dwUserID;
    createGroup.cbPlatformID = pUserInfo->cbPlatformId;
    createGroup.cbLevel = 1;
    createGroup.cbIcon = pCreateGroup->cbIcon;
    createGroup.cbIncomeRate = (CT_BYTE)(wBaseIncomeRate/10);
    createGroup.cbSettleDays = cbSettleDays;
    //_snprintf_info(createGroup.szNickName, sizeof(createGroup.szNickName), "%s", pUserInfo->szNickName);
    _snprintf_info(createGroup.szGroupName, sizeof(createGroup.szGroupName), "%s", pCreateGroup->szGroupName);
    _snprintf_info(createGroup.szNotice, sizeof(createGroup.szNotice), "%s", pCreateGroup->szNotice);

    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_CREATE_GROUP, &createGroup, sizeof(MSG_CS2DB_CreateGroup));
}

CT_VOID CCenterThread::OnGroupSubApplyAddGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Apply_AddGroup))
    {
        return;
    }

    MSG_CS_Apply_AddGroup* pAddGroup = (MSG_CS_Apply_AddGroup*)pData;

    //检查玩家在不在线,不在线则不处理
    tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pAddGroup->dwUserID);
    if (pUserInfo == NULL)
    {
        LOG(WARNING) << "add group user Info not find, userid : " << pAddGroup->dwUserID;
        return;
    }

    //检查是否有这个群
    CT_BOOL bHasGroup = CGroupMgr::get_instance().HasGroup(pAddGroup->dwGroupID);
    if (!bHasGroup)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pAddGroup->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    //检查玩家是否已经有群(玩家只能拥有一个群)
    CT_BOOL bUserHasGroup = CGroupMgr::get_instance().IsUserHasGroup(pAddGroup->dwUserID);
    if (bUserHasGroup)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pAddGroup->dwUserID, HAS_GROUP_CANNOT_APPLY);
        return;
    }

    //检查玩家是否已经群成员
    /*CT_BOOL bIsGroupUser = CGroupMgr::get_instance().IsGroupUser(pAddGroup->dwGroupID, pAddGroup->dwUserID);
    if (bIsGroupUser)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pAddGroup->dwUserID, GROUP_USER_ALREADY);
        return;
    }*/

    //检测是否已经申请
    CT_BOOL bIsApplyUser = CGroupMgr::get_instance().IsApplyAddGroup(pAddGroup->dwGroupID, pAddGroup->dwUserID);
    if (bIsApplyUser)
    {
        return;
    }

    //检查群是否满员
    CT_DWORD dwGroupUserCount = CGroupMgr::get_instance().GetGroupUserCount(pAddGroup->dwGroupID);
    if (dwGroupUserCount >= MAX_GROUP_USER_COUNT)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pAddGroup->dwUserID, GROUP_USER_FULL);
        return;
    }

    CGroupMgr::get_instance().InsertApplyGroupInfo(pAddGroup->dwGroupID, pAddGroup->dwUserID, pUserInfo);
}

CT_VOID CCenterThread::OnGroupSubOpApplyAddGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Operate_Apply_AddGroup))
    {
        return;
    }
    MSG_CS_Operate_Apply_AddGroup* pOperateApply = (MSG_CS_Operate_Apply_AddGroup*)pData;
    CGroupMgr::get_instance().OperateApplyAddGroup(pOperateApply->dwGroupID, pOperateApply->dwApplyUserID, pOperateApply->dwMasterID, pOperateApply->cbOperateCode);
}

CT_VOID CCenterThread::OnGroupSubQuitGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Quit_Group))
    {
        return;
    }

    MSG_CS_Quit_Group* pQuitGroup = (MSG_CS_Quit_Group*)pData;
    CT_BOOL bIsGroupUser = CGroupMgr::get_instance().IsGroupUser(pQuitGroup->dwGroupID, pQuitGroup->dwUserID);
    if (!bIsGroupUser)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pQuitGroup->dwUserID, NOT_GROUP_USER);
        return;
    }

    CGroupMgr::get_instance().QuitGroup(pQuitGroup->dwGroupID, pQuitGroup->dwUserID);
}

CT_VOID CCenterThread::OnGroupSubTickUser(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Tick_GroupUser))
    {
        return;
    }

    MSG_CS_Tick_GroupUser* pTickUser = (MSG_CS_Tick_GroupUser*)pData;
    CGroupMgr::get_instance().TickOutGroupUser(pTickUser->dwGroupID, pTickUser->dwMasterID, pTickUser->dwUserID);
}

CT_VOID CCenterThread::OnGroupSubCreatePRoom(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Create_GroupPRoom))
    {
        return;
    }

    MSG_CS_Create_GroupPRoom* pCreatePRoom = (MSG_CS_Create_GroupPRoom*)pData;
    if (pCreatePRoom == NULL)
    {
        return;
    }

    //检查用户是否群组成员
    CT_BOOL bGroupUser = CGroupMgr::get_instance().IsGroupUser(pCreatePRoom->dwGroupID, pCreatePRoom->dwUserID);
    if (!bGroupUser)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pCreatePRoom->dwUserID, NOT_GROUP_USER);
        return;
    }

    //查找群组的房间
    const CT_CHAR* pPRoomJson = NULL;
    /*std::unique_ptr<tagGroupRoomConfig>* pGroupRoomCfg = CGroupMgr::get_instance().GetGroupRoomCfg(pCreatePRoom->dwGroupID, pCreatePRoom->wGameID, pCreatePRoom->wKindID);
    if (pGroupRoomCfg == NULL)
    {
        std::shared_ptr<tagDefaultRoomConfig>* pDefaultRoomCfg = CGroupMgr::get_instance().GetDefaultRoomCfg(pCreatePRoom->wGameID, pCreatePRoom->wKindID);
        if (pDefaultRoomCfg == NULL)
        {
            return;
        }

        pPRoomJson = (*pDefaultRoomCfg)->szConfig;
    }
    else
    {
        pPRoomJson = (*pGroupRoomCfg)->szConfig;
    }*/

    CT_DWORD dwRoomNum = 0;
    CT_WORD	 wCurrUserCount = 0;
    CT_BOOL bHasFreeRoom = CGroupMgr::get_instance().FindPRoomInfo(pCreatePRoom->dwGroupID, pCreatePRoom->wGameID, pCreatePRoom->wKindID, dwRoomNum, wCurrUserCount);
    if (bHasFreeRoom)
    {
        acl::json proomJson(pPRoomJson);
        acl::json_node& root = proomJson.get_root();
        acl::json_node *node;

#ifdef  _OS_WIN32_CODE
        node = &proomJson.create_node("roomnum", (__int64)dwRoomNum);
			root.add_child(node);
			node = &proomJson.create_node("curUserCount", (__int64)wCurrUserCount);
			root.add_child(node);
#else
        node = &proomJson.create_node("roomnum", (long long int)dwRoomNum);
        root.add_child(node);
        node = &proomJson.create_node("curUserCount", (long long int)wCurrUserCount);
        root.add_child(node);

#endif //  _OS_WIN32_CODE
        node = &proomJson.create_node("time", Utility::GetTimeNowString().c_str());
        root.add_child(node);

        MSG_SC_Create_GroupPRoom proomResult;
        proomResult.dwMainID = MSG_GROUP_MAIN;
        proomResult.dwSubID = SUB_SC_GROUP_CREATE_PROOM;
        proomResult.dwGroupID = pCreatePRoom->dwGroupID;
        proomResult.dwUserID = pCreatePRoom->dwUserID;
        acl::string jsonResult;
        proomJson.build_json(jsonResult);
        _snprintf_info(proomResult.szJsonResult, sizeof(proomResult.szJsonResult), "%s", jsonResult.c_str());

        std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(pCreatePRoom->dwGroupID);
        if (pGroup)
        {
            std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& mapGroupUserInfo = (*pGroup)->GetAllGroupUserInfo();
            for (auto& it : mapGroupUserInfo)
            {
                acl::aio_socket_stream* pProxySocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
                if (pProxySocket)
                {
                    proomResult.dwValue2 = it.first;
                    CNetModule::getSingleton().Send(pProxySocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &proomResult, sizeof(proomResult));
                }
            }
        }
    }
    else
    {
        acl::json proomJson(pPRoomJson);

        CMD_GameServer_Info *pGameServer =
            CServerMgr::get_instance().FindSuitTableGameServer(pCreatePRoom->wGameID, pCreatePRoom->wKindID, 0);
        if (pGameServer == NULL)
        {
            CGroupMgr::get_instance().SendGroupErrorMsg(pCreatePRoom->dwUserID, NOT_FOUND_GAMESERVER);
            return;
        }

        //到DB去检测钻石
        CT_DWORD dwMasterID = CGroupMgr::get_instance().GetGroupMasterID(pCreatePRoom->dwGroupID);
        MSG_CS2DB_Deduct_GroupMaster_Gem deductGem;
        deductGem.dwGroupID = pCreatePRoom->dwGroupID;
        deductGem.dwMasterID = dwMasterID;
        deductGem.dwUserID = pCreatePRoom->dwUserID;

        CT_WORD wPlayCount = 0;
        const acl::json_node* pPlayCount = proomJson.getFirstElementByTagName("playCount");
        if (pPlayCount == NULL)
        {
            LOG(WARNING) << "auto create room can not find play count param!";
            return;
        }
        wPlayCount = (CT_WORD)*pPlayCount->get_int64();

        deductGem.wNeedGem = CServerMgr::get_instance().GetCreatePRoomNeedGem(pGameServer->dwServerID, wPlayCount);
        deductGem.wGameID = pCreatePRoom->wGameID;
        deductGem.wKindID = pCreatePRoom->wKindID;

        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_DEDUCT_GROUP_MASTER_GEM, &deductGem, sizeof(deductGem));
    }
}

CT_VOID CCenterThread::OnGroupSubDimissGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Dismiss_Group))
    {
        return;
    }

    MSG_CS_Dismiss_Group* pDismissGroup = (MSG_CS_Dismiss_Group*)pData;
    CT_DWORD dwMasterID = CGroupMgr::get_instance().GetGroupMasterID(pDismissGroup->dwGroupID);
    if (dwMasterID != pDismissGroup->dwMasterID)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pDismissGroup->dwMasterID, PERMISSION_DENIED);
        return;
    }

    CGroupMgr::get_instance().RemoveGroup(pDismissGroup->dwGroupID, dwMasterID);
}

CT_VOID CCenterThread::OnGroupSubQueryGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
    if (wDataSize != sizeof(MSG_CS_Query_GroupInfo))
    {
        return;
    }

    MSG_CS_Query_GroupInfo* pQueryGroup = (MSG_CS_Query_GroupInfo*)pData;
    std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(pQueryGroup->dwGroupID);
    if (pGroup == NULL)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pQueryGroup->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    std::unique_ptr<tagGroupInfo>& pGroupInfo = (*pGroup)->GetGroupInfo();
    MSG_SC_Query_GroupInfo groupInfo;
    groupInfo.dwMainID = MSG_GROUP_MAIN;
    groupInfo.dwSubID = SUB_SC_QUERY_GROUP_INFO;
    groupInfo.dwValue2 = pQueryGroup->dwUserID;
    groupInfo.dwGroupID = pGroupInfo->dwGroupID;
    groupInfo.dwMasterID = pGroupInfo->dwMasterID;
    groupInfo.wUserCount = pGroupInfo->wUserCount;
    //_snprintf_info(groupInfo.szMasterName, sizeof(groupInfo.szMasterName), "%s", pGroupInfo->szMasterName);
    _snprintf_info(groupInfo.szGroupName, sizeof(groupInfo.szGroupName), "%s", pGroupInfo->szGroupName);
    CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &groupInfo, sizeof(groupInfo));
}

CT_VOID CCenterThread::OnGroupSubModifyGroupName(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Modify_GroupName))
    {
        return;
    }

    MSG_CS_Modify_GroupName* pModifyGroupName = (MSG_CS_Modify_GroupName*)pData;
    if (strlen(pModifyGroupName->szGroupName) == 0)
    {
        return;
    }

    CGroupMgr::get_instance().ModifyGroupName(pModifyGroupName);
}

CT_VOID CCenterThread::OnGroupSubModifyGroupNotice(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Modify_GroupNotice))
    {
        return;
    }

    MSG_CS_Modify_GroupNotice* pModifyGroupNotice = (MSG_CS_Modify_GroupNotice*)pData;
    if (strlen(pModifyGroupNotice->szNotice) == 0)
    {
        return;
    }

    CGroupMgr::get_instance().ModifyGroupNotice(pModifyGroupNotice);
}

CT_VOID CCenterThread::OnGroupSubModifyGroupIcon(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Modify_GroupIcon))
    {
        return;
    }

    MSG_CS_Modify_GroupIcon* pModifyGroupIcon = (MSG_CS_Modify_GroupIcon*)pData;
    CGroupMgr::get_instance().ModifyGroupIcon(pModifyGroupIcon);
}

CT_VOID CCenterThread::OnGroupSubUpgradeGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Upgrade_Group))
    {
        return;
    }

    MSG_CS_Upgrade_Group* pUpgradeGroup = (MSG_CS_Upgrade_Group*)pData;
    CGroupMgr::get_instance().UpgradeGroup(pUpgradeGroup);
}

CT_VOID CCenterThread::OnGroupSubModifyGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Modify_GroupInfo))
    {
        return;
    }

    MSG_CS_Modify_GroupInfo* pModifyGroupInfo = (MSG_CS_Modify_GroupInfo*)pData;
    if (strlen(pModifyGroupInfo->szGroupName) == 0 || strlen(pModifyGroupInfo->szNotice) == 0)
    {
        return;
    }

    CGroupMgr::get_instance().ModifyGroupInfo(pModifyGroupInfo);
}

CT_VOID CCenterThread::OnGroupSubModifyGroupUserRemarks(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_DirGroupUser_Remarks))
    {
        return;
    }

    MSG_CS_DirGroupUser_Remarks* pDirGroupUserRemarks = (MSG_CS_DirGroupUser_Remarks*)pData;

    CGroupMgr::get_instance().ModifyGroupUserRemarks(pDirGroupUserRemarks);
}

CT_VOID CCenterThread::OnGroupSubUpgradeGroupUserIncomeRate(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Upgrade_GroupUser_IncomeRate))
    {
        return;
    }

    MSG_CS_Upgrade_GroupUser_IncomeRate* pGroupUserIncomeRate = (MSG_CS_Upgrade_GroupUser_IncomeRate*)pData;

    CGroupMgr::get_instance().UpgradeGroupUserIncomeRate(pGroupUserIncomeRate);
}

CT_VOID CCenterThread::OnGroupSubQueryDirGroupUserList(const CT_VOID *pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_DirGroupUser_List))
    {
        return;
    }

    MSG_CS_Query_DirGroupUser_List* pQueryDirUser = (MSG_CS_Query_DirGroupUser_List*)pData;
    CGroupMgr::get_instance().QueryDirGroupUserList(pQueryDirUser);
}

CT_VOID CCenterThread::OnGroupSubChat(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Send_GroupChatMsg))
    {
        return;
    }

    MSG_CS_Send_GroupChatMsg* pSendGroupMsg = (MSG_CS_Send_GroupChatMsg*)pData;
    CGroupMgr::get_instance().SendGroupChatMsg(pSendGroupMsg);
}

CT_VOID CCenterThread::OnGroupQueryGroupDirInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_DirGroupUser_Info))
    {
        return;
    }
    MSG_CS_Query_DirGroupUser_Info* pDirGroupUserInfo = (MSG_CS_Query_DirGroupUser_Info*)pData;
    CGroupMgr::get_instance().QueryDirGroupUserInfo(pDirGroupUserInfo);
}

CT_VOID CCenterThread::OnGroupQueryGroupSubInfoList(const CT_VOID *pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_SubGroupUser_List))
    {
        return;
    }
    MSG_CS_Query_SubGroupUser_List* pQuerySubGroupUserList = (MSG_CS_Query_SubGroupUser_List*)pData;
    CGroupMgr::get_instance().QuerySubGroupUserList(pQuerySubGroupUserList);
}

CT_VOID CCenterThread::OnGroupSubQueryGroupIncome(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Group_Income))
    {
        return;
    }

    MSG_CS_Query_Group_Income* pQueryGroupIncome = (MSG_CS_Query_Group_Income*)pData;
    CGroupMgr::get_instance().QueryGroupUserIncome(pQueryGroupIncome);
}

CT_VOID CCenterThread::OnGroupSubQueryGroupYesterDayIncome(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Group_Yesterday_Income))
    {
        return;
    }
    MSG_CS_Query_Group_Yesterday_Income* pQueryYesterdayIncome = (MSG_CS_Query_Group_Yesterday_Income*)pData;
    CGroupMgr::get_instance().QueryGroupUserYesterdayIncome(pQueryYesterdayIncome);
}

CT_VOID CCenterThread::OnGroupSubQueryGroupIncomeDetail(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Group_Income_Detail))
    {
        return;
    }

    MSG_CS_Query_Group_Income_Detail* pQueryGroupIncomeDetail = (MSG_CS_Query_Group_Income_Detail*)pData;
    CGroupMgr::get_instance().QueryGroupUserIncomeDetail(pQueryGroupIncomeDetail);
}

CT_VOID CCenterThread::OnGroupSubQueryGroupSettleRecord(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Group_Settle_Record))
    {
        return;
    }

    MSG_CS_Query_Group_Settle_Record* pQuerySettleRecord = (MSG_CS_Query_Group_Settle_Record*)pData;
    CGroupMgr::get_instance().QueryGroupUserSettleRecord(pQuerySettleRecord);
}

CT_VOID CCenterThread::OnGroupSubQueryGroupSettleInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Query_Group_Settle_Info))
    {
        return;
    }

    MSG_CS_Query_Group_Settle_Info* pQuerySettleInfo = (MSG_CS_Query_Group_Settle_Info*)pData;
    CGroupMgr::get_instance().QuerySettleInfo(pQuerySettleInfo);
}

CT_VOID CCenterThread::OnGroupSubSettleIncome(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Settle_Income))
    {
        return;
    }
    MSG_CS_Settle_Income* pSettleIncome = (MSG_CS_Settle_Income*)pData;
    CGroupMgr::get_instance().SettleIncome(pSettleIncome);
}

CT_VOID CCenterThread::OnGroupSubCreatPRoom(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Create_Group_PRoom))
    {
        return;
    }

    MSG_CS_Create_Group_PRoom* pCreatePRoom = (MSG_CS_Create_Group_PRoom*)pData;
    CGroupMgr::get_instance().CreatePRoom(pCreatePRoom);
}

CT_VOID CCenterThread::OnGroupSubOpenGameUI(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Open_Group_Game))
    {
        return;
    }
    MSG_CS_Open_Group_Game* pOpenGameUI = (MSG_CS_Open_Group_Game*)pData;
    CGroupMgr::get_instance().OpenGameUI(pOpenGameUI);
}

CT_VOID CCenterThread::OnGroupSubCloseGameUI(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Close_Group_Game))
    {
        return;
    }
    MSG_CS_Close_Group_Game* pCloseGameUI = (MSG_CS_Close_Group_Game*)pData;
    CGroupMgr::get_instance().CloseGameUI(pCloseGameUI->dwUserID);
}

CT_VOID CCenterThread::OnLoginServerAddGroupUser(MSG_L2CS_AddGroupUser* pAddGroupUser)
{
    CT_DWORD dwSelfGroupID = CGroupMgr::get_instance().GetUserGroupID(pAddGroupUser->dwUserID);
    if (dwSelfGroupID != 0)
    {
        LOG(WARNING) << "user id : " << pAddGroupUser->dwUserID << ", has group, group id: " << dwSelfGroupID;
        return;
    }

    CT_DWORD dwGroupID = CGroupMgr::get_instance().GetUserGroupID(pAddGroupUser->dwParentUserID);
    LOG(WARNING) << "dwParentUserID : " << pAddGroupUser->dwParentUserID << ", group id: " << dwGroupID;
    if (dwGroupID != 0)
    {
        std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(dwGroupID);
        if (pGroup)
        {
            tagGroupUserInfo* pParentGroupUser = (*pGroup)->GetGroupUserInfo(pAddGroupUser->dwParentUserID);
            if (pParentGroupUser)
            {
                std::unique_ptr<tagGroupInfo>& ptrGroupInfo = (*pGroup)->GetGroupInfo();
                tagGroupLevelCfg* pGroupLevelCfg = CGroupMgr::get_instance().GetLevelCfg(ptrGroupInfo->cbLevel);
                tagGroupUserInfo userInfo;
                userInfo.dwUserID = pAddGroupUser->dwUserID;
                userInfo.dwParentUserId = pAddGroupUser->dwParentUserID;
                userInfo.llTotalIncome = 0;
                userInfo.dwRecharge = 0;
                userInfo.cbSex = pAddGroupUser->cbSex;
                userInfo.cbHeadId = pAddGroupUser->cbHeadId;
                userInfo.cbVip2 = pAddGroupUser->cbVip2;
                userInfo.cbInComeRate = (CT_BYTE)pParentGroupUser->cbInComeRate*0.3;
                userInfo.wDeductRate = (pGroupLevelCfg != NULL ? pGroupLevelCfg->wDeductRate : 500);
                userInfo.wDeductTop = (pGroupLevelCfg != NULL ? pGroupLevelCfg->wDeductTop : 700);
                userInfo.wDeductIncre = (pGroupLevelCfg != NULL ? pGroupLevelCfg->wDeductIncre : 2);
                userInfo.dwDeductStart = (pGroupLevelCfg != NULL ? pGroupLevelCfg->dwDeductStart : 30000);
                userInfo.dwRegDate = time(NULL);
                userInfo.dwLastDate = userInfo.dwRegDate;
                userInfo.cbMemLevel = pParentGroupUser->cbMemLevel + 1;
                userInfo.bOnline = false;
                (*pGroup)->AddGroupUser(&userInfo);
            }
        }
    }
}


CT_VOID CCenterThread::OnGroupSubPullUserIntoGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Pull_User_IntoGroup))
    {
        return;
    }

    MSG_CS_Pull_User_IntoGroup* pPullUser = (MSG_CS_Pull_User_IntoGroup*)pData;

    CT_DWORD dwGroupUserCount = CGroupMgr::get_instance().GetGroupUserCount(pPullUser->dwGroupID);
    if (dwGroupUserCount >= MAX_GROUP_USER_COUNT)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pPullUser->dwMasterID, GROUP_USER_FULL);
        return;
    }

    //判断是否群主操作
    CT_DWORD dwMasterID = CGroupMgr::get_instance().GetGroupMasterID(pPullUser->dwGroupID);
    if (dwMasterID != pPullUser->dwMasterID)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pPullUser->dwMasterID, PERMISSION_DENIED);
        return;
    }

    //检查玩家是否已经群成员
    CT_BOOL bIsGroupUser = CGroupMgr::get_instance().IsGroupUser(pPullUser->dwGroupID, pPullUser->dwPulledUserID);
    if (bIsGroupUser)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pPullUser->dwMasterID, GROUP_USER_ALREADY);
        return;
    }

    UserAccountsBaseData userBaseData;
    CT_BOOL bAccountInfo = GetAccountInfo(pPullUser->dwPulledUserID, userBaseData);
    if (bAccountInfo == false)
    {
        //返回错误
        //CGroupMgr::get_instance().SendGroupErrorMsg(pPullUser->dwMasterID, USER_NOT_ONLINE);
        //到数据库查询玩家资料
        MSG_CS2DB_Query_BeAdd_GroupUser beAddUser;
        beAddUser.dwGroupID = pPullUser->dwGroupID;
        beAddUser.dwUserID = pPullUser->dwPulledUserID;
        beAddUser.dwOperateUserID = pPullUser->dwMasterID;
        beAddUser.cbType = 1;
        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_QUERY_BE_ADD_GROUP_USER, &beAddUser, sizeof(beAddUser));
        return;
    }

    std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(pPullUser->dwGroupID);
    if (pGroup == NULL)
    {
        //返回错误
        CGroupMgr::get_instance().SendGroupErrorMsg(pPullUser->dwMasterID, NOT_FOUND_GROUP);
        return;
    }

    tagGroupUserInfo groupUserInfo;
    groupUserInfo.dwUserID = pPullUser->dwPulledUserID;
    //_snprintf_info(groupUserInfo.szNickName, sizeof(groupUserInfo.szNickName), "%s", userBaseData.szNickName);
    //_snprintf_info(groupUserInfo.szHeadUrl, sizeof(groupUserInfo.szHeadUrl), "%s", userBaseData.szHeadUrl);

    (*pGroup)->AddGroupUser(&groupUserInfo);

    CGroupMgr::get_instance().SendAllGameSserverAddGroupUser(pPullUser->dwGroupID, &groupUserInfo);
}

CT_VOID CCenterThread::OnGroupSubRecommendAddGroup(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Recommend_AddGroup))
    {
        return;
    }

    MSG_CS_Recommend_AddGroup* pAddGroup = (MSG_CS_Recommend_AddGroup*)pData;

    //检测拉人玩家是否群组成员
    CT_BOOL bGroupUser = CGroupMgr::get_instance().IsGroupUser(pAddGroup->dwGroupID, pAddGroup->dwUserID);
    if (!bGroupUser)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pAddGroup->dwUserID, NOT_GROUP_USER);
        return;
    }

    //检查玩家是否已经群成员
    CT_BOOL bIsApplyUser = CGroupMgr::get_instance().IsApplyAddGroup(pAddGroup->dwGroupID, pAddGroup->dwRecommendedUserID);
    if (bIsApplyUser)
    {
        return;
    }

    CT_DWORD dwGroupUserCount = CGroupMgr::get_instance().GetGroupUserCount(pAddGroup->dwGroupID);
    if (dwGroupUserCount >= MAX_GROUP_USER_COUNT)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pAddGroup->dwUserID, GROUP_USER_FULL);
        return;
    }

    //检查玩家在不在线,不在线则不处理
    UserAccountsBaseData userBaseData;
    CT_BOOL bAccountInfo = GetAccountInfo(pAddGroup->dwRecommendedUserID, userBaseData);
    if (bAccountInfo == false)
    {
        //返回错误
        //CGroupMgr::get_instance().SendGroupErrorMsg(pAddGroup->dwUserID, USER_NOT_ONLINE);
        MSG_CS2DB_Query_BeAdd_GroupUser beAddUser;
        beAddUser.dwGroupID = pAddGroup->dwGroupID;
        beAddUser.dwUserID = pAddGroup->dwRecommendedUserID;
        beAddUser.dwOperateUserID = pAddGroup->dwUserID;
        beAddUser.cbType = 2;
        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_QUERY_BE_ADD_GROUP_USER, &beAddUser, sizeof(beAddUser));
        return;
    }

    //CGroupMgr::get_instance().InsertApplyGroupInfo(pAddGroup->dwGroupID, pAddGroup->dwRecommendedUserID, userBaseData.szNickName, userBaseData.szNickName/*userBaseData.szHeadUrl*/, 0 , 0);
}

CT_VOID CCenterThread::OnGroupSubQueryUserInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
    if (wDataSize != sizeof(MSG_CS_Query_GroupUserInfo))
    {
        return;
    }

    MSG_CS_Query_GroupUserInfo* pQueryUser = (MSG_CS_Query_GroupUserInfo*)pData;

    //查找这个群信息
    std::unique_ptr<CGroup>* pGroup = CGroupMgr::get_instance().GetGroup(pQueryUser->dwGroupID);
    if (pGroup == NULL)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pQueryUser->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    bool bGroupUser = (*pGroup)->IsGroupUser(pQueryUser->dwUserID);
    if (!bGroupUser)
    {
        CGroupMgr::get_instance().SendGroupErrorMsg(pQueryUser->dwUserID, NOT_FOUND_GROUP);
        return;
    }

    (*pGroup)->QueryGroupUserInfo(pQueryUser->dwUserID);
}

CT_VOID CCenterThread::OnGroupSubChangePosition(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(MSG_CS_Change_GroupPosition))
    {
        return;
    }

    MSG_CS_Change_GroupPosition* pChangeGroupPosition = (MSG_CS_Change_GroupPosition*)pData;
    CGroupMgr::get_instance().ChangeGroupMemberPosition(pChangeGroupPosition);
}