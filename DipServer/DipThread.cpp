#include "stdafx.h"
#include "DipThread.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "CMD_Dip.h"
#include "ServerCfg.h"
#include "ServerMgr.h"
#include "CMD_Plaza.h"
#include "CMD_ShareMsg.h"
#include "CMD_Game.h"
#include "Utility.h"

//中心服务器连接
extern CNetConnector *pNetCenter;
extern CNetConnector *pNetDB;

//定时器
#define		IDI_CONNECT_CENTER				(100)

#define		TIME_CONNECT_CENTER				(10000)


CDipThread::CDipThread(void)
	:m_bInitServer(false)
	, m_redisClient(CServerCfg::m_RedisAddress, CServerCfg::m_RedisConTimeOut, CServerCfg::m_RedisRwTimeOut)
{
}

CDipThread::~CDipThread(void)
{
}

CT_VOID CDipThread::OnTCPSocketLink(CNetConnector* pConnector)
{
	if (pConnector == NULL)
	{
		return;
	}

	stConnectParam& connectParam = pConnector->GetConnectParam();
	if (connectParam.conType == CONNECT_CENTER)
	{
		if (!m_bInitServer)
		{
			InitServer();
			m_bInitServer = true;
		}	
		//注册到中心服务器
		GoCenterServerRegister();
		LOG(INFO) << "link to center server succ!";
	}
}

CT_VOID CDipThread::OnTCPSocketShut(CNetConnector* pConnector)
{
	if (pConnector == NULL)
	{
		return;
	}

	stConnectParam& connectParam = pConnector->GetConnectParam();
	if (connectParam.conType == CONNECT_CENTER)
	{
		LOG(WARNING) << "center server is disconnect, after " << TIME_CONNECT_CENTER / 1000 << "s will be reconnect!";
		CNetModule::getSingleton().SetTimer(IDI_CONNECT_CENTER, TIME_CONNECT_CENTER, this, false);
	}
}

CT_VOID CDipThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{
}

CT_VOID CDipThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
}

//网络bit数据到来
CT_VOID CDipThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (pSocket == NULL)
	{
		return;
	}

	switch (pMc->dwMainID)
	{
	case MSG_WDIP_MAIN:
	{
		OnWebMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_DIPCS_MAIN:
	{
		OnCenterServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	default:
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		break;
	}
	}
}

//网络定时器
CT_VOID CDipThread::OnTimer(CT_DWORD dwTimerID)
{
	switch (dwTimerID)
	{
	case IDI_CONNECT_CENTER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetCenter);
	}
	break;
	default:
		break;
	}
}

//打开
CT_VOID CDipThread::Open()
{
}

CT_VOID CDipThread::GoCenterServerRegister()
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CMD_DipServer_Info registerDip;
		registerDip.dwServerID = CServerCfg::m_nServerID;
		_snprintf_info(registerDip.szServerIP, sizeof(registerDip.szServerIP), "%s", CServerCfg::m_LocalAddress);
		_snprintf_info(registerDip.szServerName, sizeof(registerDip.szServerName), "%s", CServerCfg::m_ServerName);
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_REGSTER, &registerDip, sizeof(registerDip));
		return;
	}
}

CT_VOID CDipThread::InitServer()
{
	m_redisClient.set_password(CServerCfg::m_RedisPwd);
	m_redis.set_client(&m_redisClient);
}

CT_VOID CDipThread::OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CS2DIP_CREATE_PROOM_SUCC:
	{
		if (wDataSize != sizeof(MSG_CS2D_CreatePRoom_PROOM_Succ))
		{
			return;
		}

		MSG_CS2D_CreatePRoom_PROOM_Succ* pCreatePRoomSucc = (MSG_CS2D_CreatePRoom_PROOM_Succ*)pData;
		if (pCreatePRoomSucc == NULL)
		{
			return;
		}

		acl::json jsonObject;

		acl::json_node& root = jsonObject.get_root();
		acl::json_node *node0, *node1, *node2, *node3, *node4, *node5;

		node0 = &root;

		node1 = &jsonObject.create_node("code", "0");
		node0->add_child(node1);

		node2 = &jsonObject.create_node("message", "");
		node0->add_child(node2);

		node3 = &jsonObject.create_node();
		acl::string strRoomNum;
		strRoomNum.format("%u", pCreatePRoomSucc->dwRoomNum);
		node4 = &jsonObject.create_node("roomnum", strRoomNum.c_str());
		node3->add_child(node4);
		acl::string strCurrUser;
		strCurrUser.format("%u", pCreatePRoomSucc->wCurrUserCount);
		node4 = &jsonObject.create_node("curUserCount", strCurrUser.c_str());
		node3->add_child(node4);
		acl::string strNeedGem;
		strNeedGem.format("%u", pCreatePRoomSucc->wNeedGem);
		node4 = &jsonObject.create_node("needGem", strNeedGem.c_str());
		node3->add_child(node4);
		node5 = &jsonObject.create_node("data", node3);
		node0->add_child(node5);

		acl::string strJson;
		jsonObject.build_json(strJson);
		//std::cout << strJson.c_str() << std::endl;
		acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pCreatePRoomSucc->uWebSock;
		auto dwLen = strJson.length();
		CNetModule::getSingleton().Send(pWebSock, strJson.c_str(), (CT_DWORD)dwLen);
	}
	break;

	case SUB_CS2DIP_CREATE_PROOM_FAIL:
	{
		if (wDataSize != sizeof(MSG_CS2D_CreatePRoom_Failed))
		{
			return;
		}

		MSG_CS2D_CreatePRoom_Failed* pCreatePRoomFail = (MSG_CS2D_CreatePRoom_Failed*)pData;
		if (pCreatePRoomFail == NULL)
		{
			return;
		}

		acl::json json;

		acl::json_node& root = json.get_root();
		acl::json_node *node0, *node1, *node2, *node3;

		node0 = &root;
	
		acl::string strMessage;
		if (pCreatePRoomFail->cbErrorCode == EN_GAMESERVER_NOENOUGH)
		{
			strMessage = "not find suit game server";
			node1 = &json.create_node("code", "101");
			node0->add_child(node1);
		}
		else if (pCreatePRoomFail->cbErrorCode == EN_ERROR_PARAM)
		{
			strMessage = "param error";
			node1 = &json.create_node("code", "102");
			node0->add_child(node1);
		}
		node2 = &json.create_node("message", strMessage.c_str());
		node0->add_child(node2);

		node3 = &json.create_node("data", "");
		node0->add_child(node3);

		acl::string strJson;
		json.build_json(strJson);
		acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pCreatePRoomFail->uWebSock;
		CNetModule::getSingleton().Send(pWebSock, strJson.c_str(), (CT_DWORD)strJson.length());
	}
	break;
	case SUB_CS2DIP_QUERY_PROOM_INFO:
	{
		 if (wDataSize != sizeof(MSG_CS2D_QueryPRoom_Info))
		 {
			 return;
		 }

		 MSG_CS2D_QueryPRoom_Info* pQueryInfo = (MSG_CS2D_QueryPRoom_Info*)pData;

		 acl::json jsonObject;

		 acl::json_node& root = jsonObject.get_root();
		 acl::json_node *node0, *node1, *node2, *node3, *node4, *node5, *node6;

		 node0 = &root;

		 char szBuffer[64] = { 0 };
		 _snprintf_info(szBuffer, sizeof(szBuffer), "%d", (int)pQueryInfo->cbResult);
		 node1 = &jsonObject.create_node("code", szBuffer);
		 node0->add_child(node1);

		 if (pQueryInfo->cbResult == 1)
		 {
			 node2 = &jsonObject.create_node("message", "房间不存在");
			 node0->add_child(node2);
		 }
		 else if (pQueryInfo->cbResult == 2)
		 {
			 node2 = &jsonObject.create_node("message", "该房间并非您的群成员创建，您无权限查询此房间信息！");
			 node0->add_child(node2);
		 }
		 else
		 {
			 node2 = &jsonObject.create_node("message", "查询成功");
			 node0->add_child(node2);
		 }

		 if (pQueryInfo->cbUserCount == 0)
		 {
			 node4 = &jsonObject.create_null("data");
			 node0->add_child(node4);
		 }
		 else
		 {
			 node4 = &jsonObject.create_array();
			 node5 = &jsonObject.create_node("data", node4);
			 node0->add_child(node5);
			 for (int i = 0; i < pQueryInfo->cbUserCount; ++i)
			 {
				 node6 = &jsonObject.create_node();
				 node4->add_child(node6);
				 _snprintf_info(szBuffer, sizeof(szBuffer), "%u", pQueryInfo->dwUserID[i]);
				 node1 = &jsonObject.create_node("userid", szBuffer);
				 _snprintf_info(szBuffer, sizeof(szBuffer), "%s",Utility::Utf82Ansi(pQueryInfo->szNickName[i]).c_str());
				 node2 = &jsonObject.create_node("nickname", szBuffer);
				 _snprintf_info(szBuffer, sizeof(szBuffer), "%d", (int)pQueryInfo->cbState[i]);
				 node3 = &jsonObject.create_node("status", szBuffer);

				 (*node6).add_child(node1).add_child(node2).add_child(node3);
			 }
		 }

		 acl::string strJson;
		 jsonObject.build_json(strJson);
		 //std::cout << strJson.c_str() << std::endl;
		 acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pQueryInfo->uWebSock;
		 //std::string strUrlEncodeJson = Utility::urlEncodeCPP(Utility::Ansi2Utf8(strJson.c_str()));
		 auto dwLen = strJson.length();
		 CNetModule::getSingleton().Send(pWebSock, strJson.c_str(), (CT_DWORD)dwLen);

	}
	break;
	case SUB_CS2DIP_DISMISS_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS2D_Dismiss_PRoom))
		{
			return;
		}

		MSG_CS2D_Dismiss_PRoom* pDismissPRoom = (MSG_CS2D_Dismiss_PRoom*)pData;

		acl::json jsonObject;

		acl::json_node& root = jsonObject.get_root();
		acl::json_node *node0, *node1, *node2, *node3;

		node0 = &root;

		char szBuffer[64] = { 0 };
		_snprintf_info(szBuffer, sizeof(szBuffer), "%d", (int)pDismissPRoom->cbResult);
		node1 = &jsonObject.create_node("code", szBuffer);
		node0->add_child(node1);

		if (pDismissPRoom->cbResult == 1)
		{
			node2 = &jsonObject.create_node("message", "房间不存在");
			node0->add_child(node2);
		}
		else if (pDismissPRoom->cbResult == 2)
		{
			node2 = &jsonObject.create_node("message", "你无权解散此房间");
			node0->add_child(node2);
		}
		else if (pDismissPRoom->cbResult == 3)
		{
			node2 = &jsonObject.create_node("message", "游戏已经开始,不能解散");
			node0->add_child(node2);
		}
		else
		{
			node2 = &jsonObject.create_node("message", "解散成功");
			node0->add_child(node2);
		}

		node3 = &jsonObject.create_node("data", "");
		node0->add_child(node3);


		acl::string strJson;
		jsonObject.build_json(strJson);
		std::cout << strJson.c_str() << std::endl;
		acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pDismissPRoom->uWebSock;
		auto dwLen = strJson.length();
		CNetModule::getSingleton().Send(pWebSock, strJson.c_str(), (CT_DWORD)dwLen);
	}
	break;
	case SUB_CS2DIP_TICK_USER:
	{
		if (wDataSize != sizeof(MSG_CS2D_Tick_User))
		{
			return;
		}

		MSG_CS2D_Tick_User* pTickUser = (MSG_CS2D_Tick_User*)pData;

		acl::json jsonObject;

		acl::json_node& root = jsonObject.get_root();
		acl::json_node *node0, *node1, *node2, *node3;

		node0 = &root;

		char szBuffer[64] = { 0 };
		_snprintf_info(szBuffer, sizeof(szBuffer), "%d", (int)pTickUser->cbResult);
		node1 = &jsonObject.create_node("code", szBuffer);
		node0->add_child(node1);

		if (pTickUser->cbResult == 1)
		{
			//std::string message = Utility::base64(Utility::Ansi2Utf8("玩家不在游戏中"));;
			node2 = &jsonObject.create_node("message", "玩家不在游戏中");
			node0->add_child(node2);
		}
		else if (pTickUser->cbResult == 2)
		{
			//std::string message = Utility::base64(Utility::Ansi2Utf8("玩家所在的房间不属于当前代理"));
			node2 = &jsonObject.create_node("message", "该房间并非您的群成员创建，您无权限查询此房间信息！");
			node0->add_child(node2);
		}
		else if (pTickUser->cbResult == 3)
		{
			//std::string message = Utility::base64(Utility::Ansi2Utf8("玩家所在的房间不属于当前代理"));
			node2 = &jsonObject.create_node("message", "游戏已开始，不能进行踢人操作！");
			node0->add_child(node2);
		}
		else
		{
			//std::string message = Utility::base64(Utility::Ansi2Utf8("剔除成功🔥"));
			node2 = &jsonObject.create_node("message", "操作成功");
			node0->add_child(node2);
		}

		node3 = &jsonObject.create_node("data", "");
		node0->add_child(node3);


		acl::string strJson;
		jsonObject.build_json(strJson);
		//std::cout << strJson.c_str() << std::endl;
		acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pTickUser->uWebSock;

		//std::string strUtf8 = Utility::Ansi2Utf8(strJson.c_str());
		auto dwLen = strJson.length();
		CNetModule::getSingleton().Send(pWebSock, strJson.c_str(), (CT_DWORD)dwLen);
	}
	break;
	case SUB_CS2DIP_APPLY_ADD_GROUP:
	{
		if (wDataSize != sizeof(MSG_CS2D_Apply_Add_Group))
		{
			return;
		}
		MSG_CS2D_Apply_Add_Group* pApplyAddGroupResult = (MSG_CS2D_Apply_Add_Group*)pData;

		acl::json jsonObject;
		acl::json_node& root = jsonObject.get_root();
		acl::json_node *node0, *node1, *node2, *node3;

		node0 = &root;

		char szBuffer[64] = { 0 };
		_snprintf_info(szBuffer, sizeof(szBuffer), "%d", (int)pApplyAddGroupResult->cbResult);
		node1 = &jsonObject.create_node("code", szBuffer);
		node0->add_child(node1);

		if (pApplyAddGroupResult->cbResult == 1)
		{
			node2 = &jsonObject.create_node("message", "没有找到这个群");
			node0->add_child(node2);
		}
		else if (pApplyAddGroupResult->cbResult == 2)
		{
			node2 = &jsonObject.create_node("message", "你已经是本群成员！");
			node0->add_child(node2);
		}
		else if (pApplyAddGroupResult->cbResult == 3)
		{
			node2 = &jsonObject.create_node("message", "你已经提交过申请，请耐心等待群主通过！");
			node0->add_child(node2);
		}
		else if (pApplyAddGroupResult->cbResult == 4)
		{
			node2 = &jsonObject.create_node("message", "群已经满员，不能申请加入！");
			node0->add_child(node2);
		}
		else
		{
			node2 = &jsonObject.create_node("message", "操作成功");
			node0->add_child(node2);
		}

		node3 = &jsonObject.create_node("data", "");
		node0->add_child(node3);


		acl::string strJson;
		jsonObject.build_json(strJson);
		//std::cout << strJson.c_str() << std::endl;
		acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pApplyAddGroupResult->uWebSock;

		auto dwLen = strJson.length();
		CNetModule::getSingleton().Send(pWebSock, strJson.c_str(), (CT_DWORD)dwLen);
	}
	break;
	case SUB_CS2DIP_QUERY_ONLINE_USER:
	{
		if (wDataSize != sizeof(MSG_CS2D_Query_OnlineUser))
		{
			return;
		}

		MSG_CS2D_Query_OnlineUser* pOnlineUser = (MSG_CS2D_Query_OnlineUser*)pData;

		acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pOnlineUser->uWebSock;
		CNetModule::getSingleton().Send(pWebSock, pOnlineUser->szJsonUserList, (CT_DWORD)strlen(pOnlineUser->szJsonUserList));
	}
	break;
    case SUB_CS2DIP_QUERY_WAITLLIST_USER:
    {
        if (wDataSize != sizeof(MSG_CS2D_Query_WaitList))
        {
            return;
        }

        MSG_CS2D_Query_WaitList* pWaitList = (MSG_CS2D_Query_WaitList*)pData;

        acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pWaitList->uWebSock;
        CNetModule::getSingleton().Send(pWebSock, pWaitList->szJsonUserList, (CT_DWORD)strlen(pWaitList->szJsonUserList));
    }
    break;
    case SUB_CS2DIP_QUERY_SERVERID_TABLE_MSG:
    {
        if (wDataSize != sizeof(MSG_CS2D_Query_TableInfo))
        {
            return;
        }

        MSG_CS2D_Query_TableInfo* pTableInfo = (MSG_CS2D_Query_TableInfo*)pData;

        acl::aio_socket_stream* pWebSock = (acl::aio_socket_stream*)pTableInfo->uWebSock;
        CNetModule::getSingleton().Send(pWebSock, pTableInfo->szJsonUserList, (CT_DWORD)strlen(pTableInfo->szJsonUserList));
    }
    break;
	default:
		break;
	}
}

CT_VOID CDipThread::OnWebMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_W2D_RECHARGE:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize -1);

		//const char* pJson = (const char*)szJson;
		acl::json rechargeJson(szJson);

		const acl::json_node* pUserid = rechargeJson.getFirstElementByTagName("userid");
		if (pUserid == NULL)
		{
			return;
		}
		//std::string  strUserID;
		//strUserID = pUserid->get_string();

		const acl::json_node* pRecharge = rechargeJson.getFirstElementByTagName("recharge");
		if (pRecharge == NULL)
		{
			return;
		}
		//std::string strRecharge;
		//strRecharge = pRecharge->get_string();

		const acl::json_node* pCash = rechargeJson.getFirstElementByTagName("cash");
		if (pCash == NULL)
		{
			return;
		}

		const acl::json_node* pTotalCash = rechargeJson.getFirstElementByTagName("totalCash");
		if (pTotalCash == NULL)
		{
			return;
		}

		const acl::json_node* pVip = rechargeJson.getFirstElementByTagName("vip");
		if (pVip == NULL)
		{
			return;
		}

		const acl::json_node* pVip2 = rechargeJson.getFirstElementByTagName("vip2");
		if (pVip2 == NULL)
		{
			return;
		}

		const acl::json_node* pType = rechargeJson.getFirstElementByTagName("type");
		if (pType == NULL)
		{
			return;
		}
		//std::string strType;
		//strType = pType->get_string();

		MSG_D2CS_User_Recharge recharge;
		recharge.dwUserID = (CT_DWORD)*pUserid->get_int64();
		recharge.llRecharge = (CT_LONGLONG)*pRecharge->get_int64();
		recharge.dwTotalCash = (CT_DWORD)*pTotalCash->get_int64();
		recharge.wCurrCash = (CT_WORD)*pCash->get_int64();
		recharge.cbVipLevel = (CT_BYTE)*pVip->get_int64();
		recharge.cbVipLevel2 = (CT_BYTE)*pVip2->get_int64();
		recharge.cbType = (CT_BYTE)*pType->get_int64();

		//recharge.dwUserID = atoi(strUserID.c_str());
		//recharge.llRecharge = atoll(strRecharge.c_str());
		//recharge.cbType = (CT_BYTE)atoi(strType.c_str());
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_RECHARGE, &recharge, sizeof(recharge));
		LOG(WARNING) << "recv recharge msg: " << szJson;

		ReturnCommonToWeb(pSocket, "0", "succ");
	}
	break;
	case SUB_W2D_WX_CREATE_PROOM:
	{
		MSG_D2CS_AutoCreate_PRoom createPRoom;
		createPRoom.uWebSock = (CT_UINT64)pSocket;
		memcpy(createPRoom.szJsonParam, pData, wDataSize);
		//java过来的数据最后一位是0故截取掉
		createPRoom.szJsonParam[wDataSize - 1] = '\0';

		LOG(INFO) << createPRoom.szJsonParam;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_WX_CREATE_PROOM, &createPRoom, sizeof(createPRoom));
	}
	break;
	case SUB_W2D_WX_ADD_GROUP_USER:
	case SUB_W2D_WX_REMOVE_GROUP_USER:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json Json(szJson);
		const acl::json_node* pGroupID = Json.getFirstElementByTagName("gid");
		if (pGroupID == NULL)
		{
			LOG(WARNING) << "add group user fail, not find gid.";
			return;
		}
		CT_DWORD dwGroupID = (CT_DWORD)*pGroupID->get_int64();

		const acl::json_node* pUserList = Json.getFirstElementByTagName("userlist");
		if (pUserList == NULL)
		{
			LOG(WARNING) << "add group user fail, not find user list.";
			return;
		}

		const acl::json_node* pBindUserPlay = Json.getFirstElementByTagName("bindUserPlay");
		if (pBindUserPlay == NULL)
		{
			LOG(WARNING) << "add group user fail, not find bind User Play param.";
			return;
		}

		CT_DWORD dwBindUserPlay = (CT_DWORD)*pBindUserPlay->get_int64();

		std::string strUserList(pUserList->get_string());
		std::vector<std::string> vecUserList;
		Utility::stringSplit(strUserList, ",", vecUserList);

		if (vecUserList.size() == 0)
		{
			LOG(WARNING) << "add group user fail, user list is empty!";
			return;
		}

		CT_BYTE szBuffer[1024] = { 0 };
		MSG_D2CS_AddOrRemove_GroupUser* pGroupInfo = (MSG_D2CS_AddOrRemove_GroupUser*)szBuffer;
		pGroupInfo->dwGroupID = dwGroupID;
		pGroupInfo->wUserCount = 0;
		pGroupInfo->dwBindUserPlay = dwBindUserPlay;

		for (auto& it : vecUserList)
		{
			CT_DWORD dwUserID = atoi(it.c_str());
			memcpy(szBuffer + sizeof(MSG_D2CS_AddOrRemove_GroupUser) + sizeof(dwUserID)*pGroupInfo->wUserCount, &dwUserID, sizeof(dwUserID));
			pGroupInfo->wUserCount += 1;
		}

		if (wSubCmdID == SUB_W2D_WX_ADD_GROUP_USER)
		{
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_WX_ADD_GROUP_USER, szBuffer, sizeof(MSG_D2CS_AddOrRemove_GroupUser) + sizeof(CT_DWORD)*pGroupInfo->wUserCount);
		}
		else if (wSubCmdID == SUB_W2D_WX_REMOVE_GROUP_USER)
		{
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_WX_REMOVE_GROUP_USER, szBuffer, sizeof(MSG_D2CS_AddOrRemove_GroupUser) + sizeof(CT_DWORD)*pGroupInfo->wUserCount);
		}

		ReturnCommonToWeb(pSocket, "0", "");
	}
	break;
	case SUB_W2D_UPDATE_GROUP_OPTIONS:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json proomJson(szJson);

		const acl::json_node* pGroupId = proomJson.getFirstElementByTagName("gid");
		if (pGroupId == NULL)
		{
			return;
		}

		const acl::json_node* pBindGroupUser = proomJson.getFirstElementByTagName("bindUserPlay");
		if (pBindGroupUser == NULL)
		{
			return;
		}

		const acl::json_node* pDefaultKindid = proomJson.getFirstElementByTagName("defaultKindid");
		if (pBindGroupUser == NULL)
		{
			return;
		}

		MSG_D2CS_Update_GroupOptions groupOptions;
		groupOptions.dwGroupID = (CT_DWORD)*pGroupId->get_int64();
		groupOptions.wBindUserPlay = (CT_WORD)*pBindGroupUser->get_int64();
		groupOptions.dwDefaultKindID = (CT_DWORD)*pDefaultKindid->get_int64();
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_GROUP_OPTIONS, &groupOptions, sizeof(groupOptions));

		ReturnCommonToWeb(pSocket, "0", "修改成功");
	}
	break;
	case SUB_W2D_UPDATE_GROUP_ROOM_CFG:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json proomJson(szJson);

		const acl::json_node* pGroupId = proomJson.getFirstElementByTagName("gid");
		if (pGroupId == NULL)
		{
			return;
		}

		const acl::json_node* pGameID = proomJson.getFirstElementByTagName("gameid");
		if (pGameID == NULL)
		{
			return;
		}

		const acl::json_node* pKindID = proomJson.getFirstElementByTagName("kindid");
		if (pKindID == NULL)
		{
			return;
		}

		/*const acl::json_node* pRoomCfg = proomJson.getFirstElementByTagName("roomCfg");
		if (pRoomCfg == NULL)
		{
			return;
		}*/

		MSG_D2CS_Update_GroupRoom_Cfg roomCfg;
		roomCfg.dwGroupID = (CT_DWORD)*pGroupId->get_int64();
		roomCfg.wGameID = (CT_WORD)*pGameID->get_int64();
		roomCfg.wKindID = (CT_DWORD)*pKindID->get_int64();
		//acl::string strCfg = pRoomCfg->to_string();
		_snprintf_info(roomCfg.szPRoomCfg, sizeof(roomCfg.szPRoomCfg), "%s", szJson);
		LOG(INFO) << "modify cfg: " << szJson;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_GROUP_ROOM_CFG, &roomCfg, sizeof(roomCfg));

		ReturnCommonToWeb(pSocket, "0", "修改成功");
	}
	break;
	case SUB_W2D_REGISTER_PROMOTER:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json proomJson(szJson);

		const acl::json_node* pUserID = proomJson.getFirstElementByTagName("userid");
		if (pUserID == NULL)
		{
			return;
		}

		const acl::json_node* pPromoterID = proomJson.getFirstElementByTagName("promoterid");
		if (pPromoterID == NULL)
		{
			return;
		}

		MSG_D2CS_Regster_Promoter regsterPromoter;
		regsterPromoter.dwUserID = (CT_DWORD)*pUserID->get_int64();
		regsterPromoter.dwPromoterID = (CT_DWORD)*pPromoterID->get_int64();
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_REGSTER_PROMOTER, &regsterPromoter, sizeof(regsterPromoter));

		ReturnCommonToWeb(pSocket, "0", "设置成功");
	}
	break;
	case SUB_W2D_QUERY_PROOM_INFO:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json Json(szJson);
		const acl::json_node* pGroupID = Json.getFirstElementByTagName("gid");
		if (pGroupID == NULL)
		{
			LOG(WARNING) << "query room info fail, not find gid.";
			return;
		}

		const acl::json_node* pRoomNum = Json.getFirstElementByTagName("roomNum");
		if (pRoomNum == NULL)
		{
			LOG(WARNING) << "query room info fail, not find room num.";
			return;
		}
		CT_DWORD dwRoomNum = (CT_DWORD)*pRoomNum->get_int64();

		MSG_D2CS_Query_PRoomInfo queryPRoomInfo;
		_snprintf_info(queryPRoomInfo.szGroupIDList, sizeof(queryPRoomInfo.szGroupIDList), "%s", pGroupID->get_string());
		queryPRoomInfo.dwRoomNum = dwRoomNum;
		queryPRoomInfo.uWebSock = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_QUERY_PROOM_INFO, &queryPRoomInfo, sizeof(queryPRoomInfo));
	}
	break;
	case SUB_W2D_DISMISS_PROOM:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json Json(szJson);
		const acl::json_node* pGroupID = Json.getFirstElementByTagName("gid");
		if (pGroupID == NULL)
		{
			LOG(WARNING) << "dismiss room info fail, not find gid.";
			return;
		}

		const acl::json_node* pRoomNum = Json.getFirstElementByTagName("roomNum");
		if (pRoomNum == NULL)
		{
			LOG(WARNING) << "dismiss room info fail, not find room num.";
			return;
		}
		CT_DWORD dwRoomNum = (CT_DWORD)*pRoomNum->get_int64();

		MSG_D2CS_Dismiss_PRoom dismissPRoom;
		_snprintf_info(dismissPRoom.szGroupIDList, sizeof(dismissPRoom.szGroupIDList), "%s", pGroupID->get_string());
		dismissPRoom.dwRoomNum = dwRoomNum;
		dismissPRoom.uWebSock = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_DISMISS_PROOM, &dismissPRoom, sizeof(dismissPRoom));
	}
	break;
	case SUB_W2D_TICK_USER:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json Json(szJson);
		const acl::json_node* pGroupID = Json.getFirstElementByTagName("gid");
		if (pGroupID == NULL)
		{
			LOG(WARNING) << "tick user fail, not find gid.";
			return;
		}

		const acl::json_node* pUserID = Json.getFirstElementByTagName("userlist");
		if (pUserID == NULL)
		{
			LOG(WARNING) << "tick user fail, not find userid.";
			return;
		}
		//CT_DWORD dwUserID = (CT_DWORD)*pUserID->get_int64();

		MSG_D2CS_Tick_User tickUser;
		_snprintf_info(tickUser.szGroupIDList, sizeof(tickUser.szGroupIDList), "%s", pGroupID->get_string());
		_snprintf_info(tickUser.szUserList, sizeof(tickUser.szUserList), "%s", pUserID->get_string());
		if (strlen(tickUser.szGroupIDList) == 0 || strlen(tickUser.szUserList) == 0)
		{
			return;
		}

		tickUser.uWebSock = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_TICK_USER, &tickUser, sizeof(tickUser));
	}
	break;
	case SUB_W2D_APPLY_ADD_GROUP:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json Json(szJson);
		const acl::json_node* pGroupID = Json.getFirstElementByTagName("gid");
		if (pGroupID == NULL)
		{
			LOG(WARNING) << "apply add group fail, not find gid.";
			return;
		}

		const acl::json_node* pUserID = Json.getFirstElementByTagName("userid");
		if (pUserID == NULL)
		{
			LOG(WARNING) << "tick user fail, not find userid.";
			return;
		}

		if (!pGroupID->is_number() || !pGroupID->is_number())
		{
			return;
		}

		MSG_D2CS_Apply_Add_Group addGroup;
		addGroup.dwGroupID = (CT_DWORD)*pGroupID->get_int64();
		addGroup.dwUserID = (CT_DWORD)*pUserID->get_int64();
		addGroup.uWebSock = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_APPLY_ADD_GROUP, &addGroup, sizeof(addGroup));
	}
	break;
	case SUB_W2D_QUERY_ONLINE_USER:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json Json(szJson);
		const acl::json_node* pQueryType = Json.getFirstElementByTagName("type");
		if (pQueryType == NULL)
		{
			LOG(WARNING) << "query online user, can not find query type.";
			ReturnCommonToWeb(pSocket, "1", "找不到查询类型");
			return;
		}

		const acl::json_node* pPlatform = Json.getFirstElementByTagName("platformId");
		if (pPlatform == NULL)
		{
			LOG(WARNING) << "query online user fail, can find platformId.";
			ReturnCommonToWeb(pSocket, "1", "找不到平台ID");
			return;
		}

		MSG_D2CS_Query_OnlineUser queryOnline;
		queryOnline.cbType = (CT_BYTE)*pQueryType->get_int64();
		queryOnline.cbPlatformId = (CT_BYTE)*pPlatform->get_int64();
		queryOnline.uWebSock = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_QUERY_ONLINE_USER, &queryOnline, sizeof(queryOnline));
	}
	break;
    case SUB_W2D_QUERY_WAITLLIST_USER:
    {
        MSG_D2CS_Query_WaitList queryWaitList;
        queryWaitList.uWebSock = (CT_UINT64)pSocket;
        CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_QUERY_WAITLLIST_USER, &queryWaitList, sizeof(queryWaitList));
        LOG(INFO) << "dip server query wait list user-> cmd main:" << MSG_DIPCS_MAIN << " sub:" << SUB_DIP2CS_QUERY_WAITLLIST_USER;
    }
    break;
    case SUB_W2D_QUERY_SERVERID_TABLE_MSG:
    {
        char szJson[1024] = { 0 };
        strncpy(szJson, (const char*)pData, wDataSize - 1);

        acl::json Json(szJson);
        const acl::json_node* pQueryServerID = Json.getFirstElementByTagName("serverid");
        if (pQueryServerID == NULL)
        {
            LOG(WARNING) << "dip server query game server table information, can not find server id.";
            return;
        }

        const acl::json_node* pPlatformId = Json.getFirstElementByTagName("platformId");
        if (pPlatformId == NULL)
        {
            return;
        }

        MSG_D2CS_Query_TableInfo queryTableInfo;
        queryTableInfo.nServerID = (CT_DWORD)*pQueryServerID->get_int64();
        queryTableInfo.cbPlatformId = (CT_BYTE)*pPlatformId->get_int64();
        queryTableInfo.uWebSock = (CT_UINT64)pSocket;
        CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_QUERY_SERVERID_TABLE_MSG, &queryTableInfo, sizeof(queryTableInfo));
        LOG(INFO) << "dip server query game server table information, cmd main:" << MSG_DIPCS_MAIN << " sub:" << SUB_DIP2CS_QUERY_SERVERID_TABLE_MSG;
    }
    break;
	case SUB_W2D_QUERY_USER_IS_ONLINE:
	{
		char szJson[1024] = { 0 };
		strncpy(szJson, (const char*)pData, wDataSize - 1);

		acl::json Json(szJson);
		const acl::json_node* pUserID = Json.getFirstElementByTagName("userid");
		if (pUserID == NULL)
		{
			LOG(WARNING) << "query user is online fail, can not find user id.";
			return;
		}

		CT_DWORD dwUserID = (CT_DWORD)(*pUserID->get_int64());
		if (CheckUserIsOnline(dwUserID))
		{
			ReturnCommonToWeb(pSocket, "1", "玩家在游戏中，不能进行此操作");
		}
		else
		{
			ReturnCommonToWeb(pSocket, "0", "");
		}
	}
	break;
	    case SUB_W2D_QUERY_ONLINE_INFO:
	    {
	    	QueryOnlineInfo(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_UPDATE_ANDROID_CFG:
	    {
	    	UpdateAndroid(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_ANDROID_PLAY_COUNT:
	    {
	    	UpdateAndroidPlayGameCount(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_RELOAD_RECHARGE_CHANNEL:
	    {
	    	ReloadRechargeChannel(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_NEW_MAIL:
	    {
	    	HasNewMail(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_SEND_SYSMSG:
	    {
	    	SendNewSystemMsg(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_PROMOTER_LIST:
	    {
	    	RechargePromoterList(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_ACCOUNT_STATUS:
	    {
	    	SetAccountStatus(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_EXCHANGE_ACCOUNT_STATUS:
	    {
	    	SetExchangeAccountStatus(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_PROMOTER_GENERALIZE:
	    {
	    	SetGeneralizePromoterInfo(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_CLIENT_CHANNEL_DOMAIN:
	    {
	    	//设置客户端渠道的主页
	    	SetClientChannelDomain(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_QUANMIN_PROMOTER:
	    {
	    	UpdateQuanMinChannel(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_CHANNEL_PRESENT_SCORE:
	    {
	    	UpdateChannelPresentScore(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_UPDATE_STOCK_CONTROL:
	    {
	    	UpdateStockInfo(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_REMOVE_SYSMSG:
	    {
	    	RemoveSystemMsg(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_TICK_USER_FOR_FISH:
	    {
	    	TickFishUser(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_RATIO_CONTROL_FOR_BRNN:
	    {
	    	ReloadBrGameRatioControl(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_BRNN_BLACK_LIST:
	    {
	    	AddBlackListToBrGame(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_WEALTHGODCOMING_CFG:
	    {
	    	UpdateWealthGodComingCfg(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_WEALTHGODCOMING_REWARD:
	    {
	    	UpdateWealthGodComingReward(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_RECHARGE_AMOUNT:
	    {
	    	UpdateRechargeAmount(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_EXCHANGE_AMOUNT:
	    {
	    	UpdateExchangeAmount(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_DIANKONG:
	    {
	    	SetUserDianKong(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_ZJH_CARDRATIO:
	    {
	    	UpdateZjhDepotCardRation(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_DUOBAO_CFG:
	    {
	    	UpdateDuoBaoCfg(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_BENEFIT_CFG:
	    {
            UpdateBenefitCfg(pData, wDataSize, pSocket);
	    }
	    break;
        case SUB_W2D_PLATFORM_RECHARGE_CFG:
        {
            UpdatePlatformRechargeCfg(pData, wDataSize, pSocket);
        }
        break;
	    case SUB_W2D_SMS_PLATFORM_ID:
	    {
	    	UpdateSMSPlatformId(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_REDPACKET_STATUS:
	    {
	    	UpdateRedPacketStatus(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_REDPACKET_ROOM_CFG:
	    {
	    	UpdateRedPacketRoomCfg(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_REDPACKET_INDEX:
	    {
	    	UpdateRedPacketIndex(pData, wDataSize, pSocket);
	    }
	    break;
	    case SUB_W2D_CHAT_UPLOAD_URL:
	    {
	    	UpdateChatUploadUrl(pData, wDataSize, pSocket);
	    	break;
	    }
        case SUB_W2D_UPDATE_ANNOUNCEMENT:
        {
            UpdateAnnouncementInfo(pData, wDataSize, pSocket);
            break;
        }
        case SUB_W2D_UPDATE_BLACK_CHANNEL:
	    {
	    	UpdateBlackChannel(pData, wDataSize, pSocket);
	    	break;
	    }
        case SUB_W2D_UPDATE_GROUP_INFO:
        {
            UpdateGroupInfo(pData, wDataSize, pSocket);
            break;
        }
        case SUB_W2D_UPDATE_GROUP_USER_INFO:
        {
            UpdateGroupUserInfo(pData, wDataSize, pSocket);
            break;
        }
        case SUB_W2D_UPDATE_GROUP_LEVEL_CFG:
        {
            UpdateGroupLevelCfg(pData, wDataSize, pSocket);
            break;
        }
        case SUB_W2D_USER_BIND_GROUP:
        {
            UserBindGroup(pData, wDataSize, pSocket);
            break;
        }
        default:
	    {
	    	CNetModule::getSingleton().CloseSocket(pSocket);
	    }
	    break;
	}
}

CT_VOID CDipThread::ReturnCommonToWeb(acl::aio_socket_stream* pSocket, std::string code, std::string strMessage)
{
	acl::json json;

	acl::json_node& root = json.get_root();
	acl::json_node *node0, *node1, *node2, *node3;

	node0 = &root;

	node1 = &json.create_node("code", code.c_str());
	node0->add_child(node1);

	node2 = &json.create_node("message", strMessage.c_str());
	node0->add_child(node2);

	node3 = &json.create_node("data", "");
	node0->add_child(node3);

	acl::string strJson;
	json.build_json(strJson);
	CNetModule::getSingleton().Send(pSocket, strJson.c_str(), (CT_DWORD)strJson.length());
}

CT_BOOL CDipThread::CheckUserIsOnline(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("online_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == true)
	{
		return true;
	}

	return false;
}

CT_VOID CDipThread::QueryOnlineInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize -1);

	acl::json onlineInfoJson(szJson);
	const acl::json_node* pPlatformId = onlineInfoJson.getFirstElementByTagName("platformId");
	if (pPlatformId == NULL)
	{
		return;
	}

	CT_BYTE cbPlatformId = (CT_BYTE)*pPlatformId->get_int64();

	acl::string key;
	std::map<acl::string, acl::string> result;
	key.format("totalonline");

	m_redis.clear();
	if (m_redis.hgetall(key.c_str(), result) == false)
	{
		ReturnCommonToWeb(pWebSock, "1", "no data!");
		return;
	}

	if (result.empty())
	{
		ReturnCommonToWeb(pWebSock, "1", "no data!");
		return ;
	}

#ifdef _OS_WIN32_CODE
	__int64 nPcu 		= 0;
	__int64 nIosPcu 	= 0;
	__int64 nAndroidPcu = 0;
	__int64 nCcu 		= 0; //当前在线人数
	__int64 nIosCcu 	= 0;
	__int64 nAndroidCcu = 0;
	__int64 nGameOnline = 0;
#else
	long long int nPcu 		  = 0;
	long long int nIosPcu 	  = 0;
	long long int nAndroidPcu = 0;
	long long int nCcu		  = 0; //当前在线人数
	long long int nIosCcu 	  = 0;
	long long int nAndroidCcu = 0;
	long long int nGameOnline = 0;
#endif

	if (cbPlatformId == 255)
	{
		for (auto& it : result)
		{
			std::string att = it.first.c_str();
			if (att.find("pcu_hall_") != std::string::npos)
			{
				nPcu += atoi(it.second.c_str());
			}
			else if (att.find("pcu_ios") != std::string::npos)
			{
				nIosPcu += atoi(it.second.c_str());
			}
			else if (att.find("pcu_android") != std::string::npos)
			{
				nAndroidPcu += atoi(it.second.c_str());
			}
			else if (att.find("ccu_hall_") != std::string::npos)
			{
				nCcu += atoi(it.second.c_str());
			}
			else if (att.find("ccu_ios_") != std::string::npos)
			{
				nIosCcu += atoi(it.second.c_str());
			}
			else if (att.find("ccu_android_") != std::string::npos)
			{
				nAndroidCcu += atoi(it.second.c_str());
			}
		}
	}
	else
	{
		acl::string strPcuHall;
		acl::string strPcuIos;
		acl::string strPcuAndroid;
		acl::string strCcuHall;
		acl::string strCcuIos;
		acl::string strCcuAndroid;

		strPcuHall.format("pcu_hall_%d", cbPlatformId);
		strPcuIos.format("pcu_ios_%d", cbPlatformId);;
		strPcuAndroid.format("pcu_android_%d", cbPlatformId);;
		strCcuHall.format("ccu_hall_%d", cbPlatformId);;
		strCcuIos.format("ccu_ios_%d", cbPlatformId);;
		strCcuAndroid.format("ccu_android_%d", cbPlatformId);;

		nPcu = (atoi)(result[strPcuHall.c_str()].c_str());
		nIosPcu = (atoi)(result[strPcuIos.c_str()].c_str());
		nAndroidPcu = (atoi)(result[strPcuAndroid.c_str()].c_str());
		nCcu = (atoi)(result[strCcuHall.c_str()].c_str()); //当前在线人数
		nIosCcu = (atoi)(result[strCcuIos.c_str()].c_str());
		nAndroidCcu = (atoi)(result[strCcuAndroid.c_str()].c_str());
	}

	acl::json jsonObject;

	acl::json_node& root = jsonObject.get_root();
	acl::json_node *node0, *node1, *node2, *node3, *node4, *node5, *node6, *node7, *node8, *node9, *node10, *node11;

	node0 = &root;

	node1 = &jsonObject.create_node("code", "0");
	node0->add_child(node1);

	node2 = &jsonObject.create_node("message", "query succ!");
	node0->add_child(node2);

	node3 = &jsonObject.create_node();

	node4 = &jsonObject.create_node("PCU", nPcu);
	node3->add_child(node4);

	node4 = &jsonObject.create_node("iosPCU", nIosPcu);
	node3->add_child(node4);

	node4 = &jsonObject.create_node("androidPCU", nAndroidPcu);
	node3->add_child(node4);

	node4 = &jsonObject.create_node("CCU", nCcu);
	node3->add_child(node4);

	node4 = &jsonObject.create_node("iosCCU", nIosCcu);
	node3->add_child(node4);

	node4 = &jsonObject.create_node("androidCCU", nAndroidCcu);
	node3->add_child(node4);
	
	//各游戏在线人数
	node4 = &jsonObject.create_array();
	node5 = &jsonObject.create_node("game", node4);
	node3->add_child(node5);

	if (cbPlatformId == 255)
	{
		for (auto& it : result)
		{
			std::string att = it.first.c_str();

			if (att.find("server") != std::string::npos)
			{
				auto pos = att.find_last_of("_");
				std::string strGameID = att.substr(pos + 1);
				CT_DWORD dwTotalGameID = atoi(strGameID.c_str());
				CT_WORD wGameID = dwTotalGameID / 100;
				CT_WORD wKindID = (dwTotalGameID % 100) / 10;
				CT_WORD wRoomKindID = (dwTotalGameID % 100) % 10;

				CT_WORD dwUserCount = (CT_DWORD)atoi(it.second.c_str());
				nGameOnline += dwUserCount;

				node6 = &jsonObject.create_node();
				node4->add_child(node6);

				node7 = &jsonObject.create_node("gameid", (long long int)wGameID);
				node8 = &jsonObject.create_node("kindid", (long long int)wKindID);
				node9 = &jsonObject.create_node("roomKindid", (long long int)wRoomKindID);
				node10 = &jsonObject.create_node("value", (long long int)dwUserCount);
				(*node6).add_child(node7).add_child(node8).add_child(node9).add_child(node10);
			}
		}
	}
	else
	{
		acl::string strFind;
		strFind.format("server_%d", cbPlatformId);
		for (auto& it : result)
		{
			std::string att = it.first.c_str();

			if (att.find(strFind.c_str()) != std::string::npos)
			{
				auto pos = att.find_last_of("_");
				std::string strGameID = att.substr(pos + 1);
				CT_DWORD dwTotalGameID = atoi(strGameID.c_str());
				CT_WORD wGameID = dwTotalGameID / 100;
				CT_WORD wKindID = (dwTotalGameID % 100) / 10;
				CT_WORD wRoomKindID = (dwTotalGameID % 100) % 10;

				CT_WORD dwUserCount = (CT_DWORD)atoi(it.second.c_str());
				nGameOnline += dwUserCount;

				node6 = &jsonObject.create_node();
				node4->add_child(node6);

				node7 = &jsonObject.create_node("gameid", (long long int)wGameID);
				node8 = &jsonObject.create_node("kindid", (long long int)wKindID);
				node9 = &jsonObject.create_node("roomKindid", (long long int)wRoomKindID);
				node10 = &jsonObject.create_node("value", (long long int)dwUserCount);
				(*node6).add_child(node7).add_child(node8).add_child(node9).add_child(node10);
			}
		}
	}

	node4 = &jsonObject.create_node("CGU", nGameOnline);
	node3->add_child(node4);
	
	node11 = &jsonObject.create_node("data", node3);
	node0->add_child(node11);

	acl::string strJson;
	jsonObject.build_json(strJson);
	auto dwLen = strJson.length();
	CNetModule::getSingleton().Send(pWebSock, strJson.c_str(), (CT_DWORD)dwLen);
	
	//LOG(WARNING) << "query online info : " << strJson.c_str();
}

//通知重新加载机器人
CT_VOID CDipThread::UpdateAndroid(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pServerid = Json.getFirstElementByTagName("serverid");
	if (pServerid == NULL)
	{
		LOG(WARNING) << "update android fail, can not find server id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到server id");
		return;
	}

	//通知游戏服务器重新加载机器人配置
	MSG_D2CS_UpdateAndroid updateAndroid;
	updateAndroid.dwServerID = (CT_DWORD)*pServerid->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_ANDROID, &updateAndroid, sizeof(updateAndroid));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "succ");
}

//通知更新机器人游戏数量
CT_VOID CDipThread::UpdateAndroidPlayGameCount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pServerid = Json.getFirstElementByTagName("serverid");
	if (pServerid == NULL)
	{
		LOG(WARNING) << "update android count fail, can not find server id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到server id");
		return;
	}

	const acl::json_node* pCount = Json.getFirstElementByTagName("count");
	if (pCount == NULL)
	{
		LOG(WARNING) << "update android count fail, can not find android count! ";
		ReturnCommonToWeb(pWebSock, "1", "没有找到机器人个数！");
		return;
	}

	//通知游戏服务器更新机器人上桌人数
	MSG_D2CS_UpdateAndroid_PlayGameCount androidCount;
	androidCount.dwServerID = (CT_DWORD)*pServerid->get_int64();
	androidCount.dwAndroidCount = (CT_DWORD)*pCount->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_ANDROID_PLAY_COUNT, &androidCount, sizeof(androidCount));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "succ");
}

//通知重新加载充值渠道信息
CT_VOID CDipThread::ReloadRechargeChannel(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_RELOAD_RECHARGE_CHANNEL);
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::HasNewMail(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[128] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pUserID = Json.getFirstElementByTagName("userid");
	if (pUserID == NULL)
	{
		LOG(WARNING) << "notify has new mail fail, can not find user id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到用户ID");
		return;
	}

	MSG_D2CS_Notify_NewMail newMail;
	newMail.dwUserID = (CT_DWORD)*pUserID->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_NOTIFY_NEWMAIL, &newMail, sizeof(newMail));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::SendNewSystemMsg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pID = Json.getFirstElementByTagName("id");
	if (pID == NULL)
	{
		LOG(WARNING) << "insert new system msg fail, can not find message id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到消息ID");
		return;
	}
	
	const acl::json_node* pMessage = Json.getFirstElementByTagName("message");
	if (pMessage == NULL)
	{
		LOG(WARNING) << "insert new system msg fail, can not find message.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到消息内容");
		return;
	}

	const acl::json_node* pInterval = Json.getFirstElementByTagName("interval");
	if (pInterval == NULL)
	{
		LOG(WARNING) << "insert new system msg fail, can not find interval time.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到消息的时间间隔");
		return;
	}

	const acl::json_node* pValidTime = Json.getFirstElementByTagName("validtime");
	if (pValidTime == NULL)
	{
		LOG(WARNING) << "insert new system msg fail, can not find validtime.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到消息的结束时间");
		return;
	}

	const acl::json_node* pPlatformId = Json.getFirstElementByTagName("platformId");
	if (pPlatformId == NULL)
	{
		LOG(WARNING) << "insert new system msg fail, can not find platformId.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到消息的平台ID");
		return;
	}

	MSG_D2CS_SendSysMsg newSysMsg;
	newSysMsg.dwMsgID = (CT_DWORD)*pID->get_int64();
	newSysMsg.dwInterval = (CT_DWORD)*pInterval->get_int64();
	newSysMsg.dwValidTime = (CT_DWORD)*pValidTime->get_int64();
	newSysMsg.cbPlatformId = (CT_BYTE)*pPlatformId->get_int64();
	std::string strMessage(pMessage->get_string());
	_snprintf_info(newSysMsg.szMessage, sizeof(newSysMsg.szMessage), "%s", Utility::urlDecodeCPP(strMessage).c_str());
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_SEND_SYSMSG, &newSysMsg, sizeof(newSysMsg));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::RechargePromoterList(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
    if (wDataSize >= 2048)
    {
        ReturnCommonToWeb(pWebSock, "1", "数据长度过长");
        return;
    }
    
	char szJson[2048] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	std::string strJson(szJson);
	std::string strJsonDecode = Utility::urlDecodeCPP(strJson);

	CT_BYTE		cbBuffer[2048];
	CT_DWORD	dwSendSize = 0;

	CT_DWORD dwLen = (CT_DWORD)strJsonDecode.length();
	memcpy(cbBuffer, strJsonDecode.c_str(), sizeof(cbBuffer));
	dwSendSize += dwLen;

	cbBuffer[dwSendSize] = '\0';
	dwSendSize += 1;

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_PROMOTER_LIST, cbBuffer, dwSendSize);

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::SetAccountStatus(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[128] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pUserid = Json.getFirstElementByTagName("userid");
	if (pUserid == NULL)
	{
		LOG(WARNING) << "set account status fail, can not find userid.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到玩家ID");
		return;
	}

	const acl::json_node* pStatus = Json.getFirstElementByTagName("status");
	if (pStatus == NULL)
	{
		LOG(WARNING) << "set account status fail, can not find status.";
		ReturnCommonToWeb(pWebSock, "1", "设置玩家状态，没有找到玩家的状态");
		return;
	}

	CT_DWORD dwUserID = (CT_DWORD)*pUserid->get_int64();
	CT_DWORD dwStatus = (CT_DWORD)*pStatus->get_int64();

	acl::string key;
	key.format("account_%u", dwUserID);

	acl::string status;
	status.format("%d", dwStatus);

	m_redis.clear();
	if (m_redis.exists(key) == true)
	{
		m_redis.hset(key.c_str(), "status", status.c_str());
	}

	MSG_D2CS_UserStatus userStatus;
	userStatus.dwUserID = dwUserID;
	userStatus.cbStatus = (CT_BYTE)dwStatus;
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_USER_STATUS, &userStatus, sizeof(userStatus));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//设置兑换帐号状态
CT_VOID CDipThread::SetExchangeAccountStatus(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[128] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pType = Json.getFirstElementByTagName("type");
	if (pType == NULL)
	{
		LOG(WARNING) << "set exchange account status fail, can not find exchange type.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到帐号类型");
		return;
	}

	const acl::json_node* pStatus = Json.getFirstElementByTagName("status");
	if (pStatus == NULL)
	{
		LOG(WARNING) << "set exchange account status fail, can not find status.";
		ReturnCommonToWeb(pWebSock, "1", "设置兑换帐号状态，没有找到状态");
		return;
	}

	CT_BYTE cbType = (CT_DWORD)*pType->get_int64();
	CT_BYTE cbStatus = (CT_DWORD)*pStatus->get_int64();

	MSG_D2CS_ExchangeStatus exchangeStatus;
	exchangeStatus.cbExchangeType = cbType;
	exchangeStatus.cbStatus = cbStatus;
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_EXCHANGE_CHANNEL_STATUS, &exchangeStatus, sizeof(exchangeStatus));
	 
	//LOG(WARNING) << "update exchange status: " << (int)cbType << ", " << (int)cbStatus;
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//设置推广代理
CT_VOID CDipThread::SetGeneralizePromoterInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pWeixin1 = Json.getFirstElementByTagName("weixin1");
	if (pWeixin1 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find weixin1.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到wexin1");
		return;
	}

	const acl::json_node* pNick1 = Json.getFirstElementByTagName("nick1");
	if (pNick1 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find nick1.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到nick1");
		return;
	}
	
	const acl::json_node* pType1 = Json.getFirstElementByTagName("type1");
	if (pType1 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find type1.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到type1");
		return;
	}

	const acl::json_node* pWeixin2 = Json.getFirstElementByTagName("weixin2");
	if (pWeixin2 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find weixin2.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到wexin2");
		return;
	}

	const acl::json_node* pNick2 = Json.getFirstElementByTagName("nick2");
	if (pNick2 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find nick2.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到nick2");
		return;
	}
	
	const acl::json_node* pType2 = Json.getFirstElementByTagName("type2");
	if (pType2 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find type2.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到type2");
		return;
	}

	const acl::json_node* pWeixin3 = Json.getFirstElementByTagName("weixin3");
	if (pWeixin3 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find weixin3.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到wexin3");
		return;
	}

	const acl::json_node* pNick3 = Json.getFirstElementByTagName("nick3");
	if (pNick3 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find nick3.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到nick3");
		return;
	}
	
	const acl::json_node* pType3 = Json.getFirstElementByTagName("type3");
	if (pType3 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find type3.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到type3");
		return;
	}

	const acl::json_node* pPlatformId = Json.getFirstElementByTagName("platformId");
	if (pPlatformId == NULL)
    {
        LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find platformId.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到platformId");
        return;
    }
	
	tagGeneralizePromoterInfo generalizePromoter;
	memset(&generalizePromoter, 0, sizeof(generalizePromoter));
	std::string strNick1 = pNick1->get_string();
	_snprintf_info(generalizePromoter.szNickName[0], sizeof(generalizePromoter.szNickName[0]), "%s", Utility::urlDecodeCPP(strNick1).c_str());
	_snprintf_info(generalizePromoter.szAccount[0], sizeof(generalizePromoter.szAccount[0]), "%s", pWeixin1->get_string());
	generalizePromoter.cbType[0] = (CT_BYTE)*pType1->get_int64();
	if (strlen(generalizePromoter.szNickName[0]) != 0)
		++generalizePromoter.cbAccountCount;

	std::string strNick2 = pNick2->get_string();
	_snprintf_info(generalizePromoter.szNickName[1], sizeof(generalizePromoter.szNickName[1]), "%s", Utility::urlDecodeCPP(strNick2).c_str());
	_snprintf_info(generalizePromoter.szAccount[1], sizeof(generalizePromoter.szAccount[1]), "%s", pWeixin2->get_string());
	generalizePromoter.cbType[1] = (CT_BYTE)*pType2->get_int64();
	if (strlen(generalizePromoter.szNickName[1]) != 0)
		++generalizePromoter.cbAccountCount;

	std::string strNick3 = pNick3->get_string();
	_snprintf_info(generalizePromoter.szNickName[2], sizeof(generalizePromoter.szNickName[2]), "%s", Utility::urlDecodeCPP(strNick3).c_str());
	_snprintf_info(generalizePromoter.szAccount[2], sizeof(generalizePromoter.szAccount[2]), "%s", pWeixin3->get_string());
	generalizePromoter.cbType[2] = (CT_BYTE)*pType3->get_int64();
	generalizePromoter.cbPlatformId = (CT_BYTE)*pPlatformId->get_int64();
	if (strlen(generalizePromoter.szNickName[2]) != 0)
		++generalizePromoter.cbAccountCount;

/*	for (int i = 0; i < 3; i++)
	{
		LOG(WARNING) << "recv new weixin name: " << generalizePromoter.szAccount[i];
		LOG(WARNING) << "recv new nick1 name: " << generalizePromoter.szNickName[i];
	}
*/
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_GENERALIZE_PROMOTER, &generalizePromoter, sizeof(generalizePromoter));
	 
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//设置客户端渠道的主页
CT_VOID CDipThread::SetClientChannelDomain(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pChannelId = Json.getFirstElementByTagName("id");
	if (pChannelId == NULL)
	{
		LOG(WARNING) << "SetClientChannelDomain fail, can not find channel id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到渠道ID");
		return;
	}

	const acl::json_node* pDomain = Json.getFirstElementByTagName("domain");
	if (pDomain == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find domain.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到主页名");
		return;
	}

	const acl::json_node* pDomain2 = Json.getFirstElementByTagName("domain2");
	if (pDomain2 == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find domain2.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到主页名2");
		return;
	}

	const acl::json_node* pLock = Json.getFirstElementByTagName("locked");
	if (pLock == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find locked.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到主页名");
		return;
	}

	const acl::json_node* pShowExchangeCond = Json.getFirstElementByTagName("showExchangeCond");
	if (pShowExchangeCond == NULL)
	{
		LOG(WARNING) << "SetGeneralizePromoterInfo fail, can not find show exchange cond.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到显示兑换的条件");
		return;
	}

	MSG_D2CS_ClientChannel_Domain domain;
	domain.dwChannelID = (CT_DWORD)*pChannelId->get_int64();
	domain.cbLocked = (CT_BYTE)*pLock->get_int64();
	domain.dwShowExchangeCond = (CT_DWORD)*pShowExchangeCond->get_int64();
	_snprintf_info(domain.szDomain, sizeof(domain.szDomain), "%s", pDomain->get_string());
	_snprintf_info(domain.szDomain2, sizeof(domain.szDomain2), "%s", pDomain2->get_string());
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_CLIENTCHANNEL_DOMAIN, &domain, sizeof(domain));

	LOG(WARNING) << "recv channel id domain: " << domain.szDomain;

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UpdateQuanMinChannel(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pQuanMinId = Json.getFirstElementByTagName("quanmin_userid");
	if (pQuanMinId == NULL)
	{
		LOG(WARNING) << "UpdateQuanMinChannel fail, can not find quan min id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到全民渠道ID");
		return;
	}

	const acl::json_node* pStatus = Json.getFirstElementByTagName("status");
	if (pStatus == NULL)
	{
		LOG(WARNING) << "UpdateQuanMinChannel fail, can not find status.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到全民代理状态");
		return;
	}

	tagQuanMinChannel quanMinChannel;
	quanMinChannel.dwChannelID = (CT_DWORD)*pQuanMinId->get_int64();
	quanMinChannel.cbStatus = (CT_BYTE)*pStatus->get_int64();

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_QUANMIN_INFO, &quanMinChannel, sizeof(quanMinChannel));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UpdateChannelPresentScore(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pChannel = Json.getFirstElementByTagName("channel");
	if (pChannel == NULL)
	{
		LOG(WARNING) << "UpdateChannelPresentScore fail, can not find channel id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到渠道ID");
		return;
	}

	const acl::json_node* pInitScore = Json.getFirstElementByTagName("initScore");
	if (pInitScore == NULL)
	{
		LOG(WARNING) << "UpdateChannelPresentScore fail, can not find initScore;";
		ReturnCommonToWeb(pWebSock, "1", "没有找到注册赠送金币");
		return;
	}

	const acl::json_node* pBindScore = Json.getFirstElementByTagName("bindScore");
	if (pBindScore == NULL)
	{
		LOG(WARNING) << "UpdateChannelPresentScore fail, can not find bindScore";
		ReturnCommonToWeb(pWebSock, "1", "没有找到绑定赠送金币");
		return;
	}

	tagChannelPresentScore channelPrsentScore;
	channelPrsentScore.dwChannelID = (CT_DWORD)*pChannel->get_int64();
	channelPrsentScore.dwRegisterScore = (CT_DWORD)*pInitScore->get_int64();
	channelPrsentScore.dwBindMobileScore = (CT_DWORD)*pBindScore->get_int64();

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_CHANNEL_PRESENTSCORE, &channelPrsentScore, sizeof(channelPrsentScore));
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//更新游戏库存相关信息
CT_VOID CDipThread::UpdateStockInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pServerid = Json.getFirstElementByTagName("serverid");
	if (pServerid == NULL)
	{
		LOG(WARNING) << "UpdateStockInfo fail, can not find server id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到server id");
		return;
	}

	const acl::json_node* pTotalStockLowerLimit = Json.getFirstElementByTagName("totalStockLowerLimit");
	if (pTotalStockLowerLimit == NULL)
	{
		LOG(WARNING) << "UpdateStockInfo fail, can not find totalStockLowerLimit.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到totalStockLowerLimit");
		return;
	}

	const acl::json_node* pTodayStockHighLimit = Json.getFirstElementByTagName("todayStockHighLimit");
	if (pTodayStockHighLimit == NULL)
	{
		LOG(WARNING) << "UpdateStockInfo fail, can not find todayStockHighLimit.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到todayStockHighLimit");
		return;
	}

	const acl::json_node* pSystemAllKillRatio = Json.getFirstElementByTagName("systemAllKillRatio");
	if (pSystemAllKillRatio == NULL)
	{
		LOG(WARNING) << "UpdateStockInfo fail, can not find systemAllKillRatio.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到systemAllKillRatio");
		return;
	}

	const acl::json_node* pChangeCardRatio = Json.getFirstElementByTagName("changeCardRatio");
	if (pChangeCardRatio == NULL)
	{
		LOG(WARNING) << "UpdateStockInfo fail, can not find changeCardRatio.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到changeCardRatio");
		return;
	}

	//通知游戏服务器修改库存控制数据
	MSG_D2CS_Update_StockInfo updateStockInfo;
	updateStockInfo.dwServerID = (CT_DWORD)*pServerid->get_int64();
	updateStockInfo.llTotalStockLowerLimit = (CT_LONGLONG)*pTotalStockLowerLimit->get_int64();
	updateStockInfo.llTodayStockHighLimit = (CT_LONGLONG)*pTodayStockHighLimit->get_int64();
	updateStockInfo.wSystemAllKillRatio = (CT_WORD)*pSystemAllKillRatio->get_int64();
	updateStockInfo.wChangeCardRatio = (CT_WORD)*pChangeCardRatio->get_int64();

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_SERVER_STOCK, &updateStockInfo, sizeof(updateStockInfo));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "succ");
}

//剔除捕鱼用户
CT_VOID CDipThread::TickFishUser(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize);
	
	acl::json Json(szJson);
	const acl::json_node* pUserID = Json.getFirstElementByTagName("userid");
	if (pUserID == NULL)
	{
		LOG(WARNING) << "TickFishUser fail, can not find user id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到玩家ID");
		return;
	}
	
	MSG_D2CS_Tick_FishUser tickFishUser;
	tickFishUser.dwUserID = (CT_DWORD)*pUserID->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_TICK_FISH_USER, &tickFishUser, sizeof(tickFishUser));
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "succ");
}

//重新加载
CT_VOID CDipThread::ReloadBrGameRatioControl(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize);
	
	acl::json Json(szJson);
	const acl::json_node* pServerID = Json.getFirstElementByTagName("serverid");
	if (pServerID == NULL)
	{
		LOG(WARNING) << "ReloadBrGameRatioControl fail, can not find server id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到服务器ID");
		return;
	}
	
	MSG_D2CS_Reload_BrGameRatio reloadBrGameRatio;
	reloadBrGameRatio.dwServerID = (CT_DWORD)*pServerID->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_RATIO_CONTROL_FOR_BRNN, &reloadBrGameRatio, sizeof(reloadBrGameRatio));
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "操作成功");
}

//百人牛牛增加黑名单
CT_VOID CDipThread::AddBlackListToBrGame(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize);
	
	acl::json Json(szJson);
	const acl::json_node* pServerID = Json.getFirstElementByTagName("serverid");
	if (pServerID == NULL)
	{
		LOG(WARNING) << "ReloadBrGameRatioControl fail, can not find server id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到服务器ID");
		return;
	}
	
	MSG_D2CS_Reload_BrGameBlackList reloadBrGameBlackList;
	reloadBrGameBlackList.dwServerID = (CT_DWORD)*pServerID->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_BRGAME_BLACKLIST, &reloadBrGameBlackList, sizeof(reloadBrGameBlackList));
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "操作成功");
}

//删除系统消息
CT_VOID CDipThread::RemoveSystemMsg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);
	
	acl::json Json(szJson);
	const acl::json_node* pID = Json.getFirstElementByTagName("id");
	if (pID == NULL)
	{
		LOG(WARNING) << "RemoveSystemMsg fail, can not find msg id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到消息id");
		return;
	}

    const acl::json_node* pPlatformId = Json.getFirstElementByTagName("platformId");
    if (pPlatformId == NULL)
    {
        LOG(WARNING) << "RemoveSystemMsg fail, can not find platform id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到平台id");
        return;
    }
	
	MSG_D2CS_RemoveSysMsg removeSysMsg;
	removeSysMsg.dwMsgID = (CT_DWORD)*pID->get_int64();
	removeSysMsg.cbPlatformId = (CT_BYTE)*pPlatformId->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_REMOVE_SYSMSG, &removeSysMsg, sizeof(removeSysMsg));
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "succ");
}

//增加或者修改财神降临活动配置
CT_VOID CDipThread::UpdateWealthGodComingCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);
	
	acl::json Json(szJson);
	const acl::json_node* pID = Json.getFirstElementByTagName("id");
	if (pID == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到活动id");
		return;
	}
	
	const acl::json_node* pGameID = Json.getFirstElementByTagName("gameid");
	if (pGameID == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingReward fail, can not find cfg gameid.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到gameid");
		return;
	}
	
	const acl::json_node* pKindID = Json.getFirstElementByTagName("kindid");
	if (pKindID == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg kindid.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到kindid");
		return;
	}
	
	const acl::json_node* pRoomKindID = Json.getFirstElementByTagName("roomkind");
	if (pRoomKindID == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg roomkind id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到活动roomkind id");
		return;
	}
    
    const acl::json_node* pStartTime = Json.getFirstElementByTagName("startTime");
    if (pStartTime == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg start time.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到活动start time");
        return;
    }
    
    const acl::json_node* pEndTime = Json.getFirstElementByTagName("endTime");
    if (pEndTime == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg end time.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到活动end time");
        return;
    }
    
    const acl::json_node* pCompleteCount = Json.getFirstElementByTagName("value");
    if (pCompleteCount == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg complete count.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到活动complete count");
        return;
    }
    
    const acl::json_node* pStandard = Json.getFirstElementByTagName("standard");
    if (pStandard == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg standard.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到活动standard");
        return;
    }
	
	const acl::json_node* pMinUser = Json.getFirstElementByTagName("minUser");
	if (pMinUser == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg minUser.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到活动最低发奖人数");
		return;
	}
    
    const acl::json_node* pRewardRatio = Json.getFirstElementByTagName("rewardRatio");
    if (pRewardRatio == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg reward ratio.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到活动奖励比例");
        return;
    }
    
    const acl::json_node* pStatus = Json.getFirstElementByTagName("status");
    if (pStatus == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg fail, can not find cfg status.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到活动状态");
        return;
    }
    
    tagWealthGodComingCfg wealthGodComingCfg;
    wealthGodComingCfg.wID = (CT_WORD)*pID->get_int64();
    wealthGodComingCfg.wGameID = (CT_WORD)*pGameID->get_int64();
    wealthGodComingCfg.wKindID = (CT_WORD)*pKindID->get_int64();
    wealthGodComingCfg.wRoomKindID = (CT_WORD)*pRoomKindID->get_int64();
    wealthGodComingCfg.wCompleteCount = (CT_WORD)*pCompleteCount->get_int64();
    wealthGodComingCfg.wRewardRatio = (CT_WORD)*pRewardRatio->get_int64();
    wealthGodComingCfg.wSendRewardMinUser = (CT_WORD)*pMinUser->get_int64();
    wealthGodComingCfg.cbJudgeStander = (CT_BYTE)*pStandard->get_int64();
    wealthGodComingCfg.cbState = (CT_BYTE)*pStatus->get_int64();
    
    std::string strStartTime = pStartTime->get_string();
    std::vector<std::string> vecStartTime;
    Utility::stringSplit(strStartTime, ":", vecStartTime);
    if (vecStartTime.size() != 3)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg start time config error. id: " << wealthGodComingCfg.wID;
        ReturnCommonToWeb(pWebSock, "1", "活动开始时间配置有错误");
        return;
    }
    wealthGodComingCfg.cbStartHour = (CT_BYTE)atoi(vecStartTime[0].c_str());
    wealthGodComingCfg.cbStartMin = (CT_BYTE)atoi(vecStartTime[1].c_str());
    
    std::string strEndTime = pEndTime->get_string();
    std::vector<std::string> vecEndTime;
    Utility::stringSplit(strEndTime, ":", vecEndTime);
    if (vecEndTime.size() != 3)
    {
        LOG(WARNING) << "UpdateWealthGodComingCfg end time config error. id: " << wealthGodComingCfg.wID;
        ReturnCommonToWeb(pWebSock, "1", "活动结束时间配置有错误");
        return;
    }
    wealthGodComingCfg.cbEndHour = (CT_BYTE)atoi(vecEndTime[0].c_str());
    wealthGodComingCfg.cbEndMin = (CT_BYTE)atoi(vecEndTime[1].c_str());
    
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_WEALTHGODCOMING_CFG, &wealthGodComingCfg, sizeof(wealthGodComingCfg));
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//增加或者修改财神降临活动奖励
CT_VOID CDipThread::UpdateWealthGodComingReward(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);
	
	acl::json Json(szJson);
	const acl::json_node* pGameID = Json.getFirstElementByTagName("gameid");
	if (pGameID == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingReward fail, can not find gameid.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到gameid");
		return;
	}
	
	const acl::json_node* pKindID = Json.getFirstElementByTagName("kindid");
	if (pKindID == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingReward fail, can not find kindid.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到kindid");
		return;
	}
	
	const acl::json_node* pRoomKindID = Json.getFirstElementByTagName("roomkind");
	if (pRoomKindID == NULL)
	{
		LOG(WARNING) << "UpdateWealthGodComingReward fail, can not find roomkind id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到roomkind id");
		return;
	}
    
    const acl::json_node* pRank = Json.getFirstElementByTagName("rank");
    if (pRank == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingReward fail, can not find rank.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到rank");
        return;
    }
    
    const acl::json_node* pMinScore = Json.getFirstElementByTagName("minScore");
    if (pMinScore == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingReward fail, can not find min score.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到min score");
        return;
    }
    
    const acl::json_node* pRateScore = Json.getFirstElementByTagName("rateScore");
    if (pRateScore == NULL)
    {
        LOG(WARNING) << "UpdateWealthGodComingReward fail, can not find min score.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到rate score");
        return;
    }
    
    tagWealthGodComingReward wealthGodComingReward;
    wealthGodComingReward.wGameID = (CT_WORD)*pGameID->get_int64();
    wealthGodComingReward.wKindID = (CT_WORD)*pKindID->get_int64();
    wealthGodComingReward.wRoomKindID = (CT_WORD)*pRoomKindID->get_int64();
    wealthGodComingReward.cbRank = (CT_BYTE)*pRank->get_int64();
    wealthGodComingReward.dwRewardScore = (CT_DWORD)*pMinScore->get_int64();
    wealthGodComingReward.cbRewardRate = (CT_BYTE)*pRateScore->get_int64();
    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_WEALTHGODCOMING_REWARD, &wealthGodComingReward, sizeof(wealthGodComingReward));
    
    //给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//修改充值渠道的充值金额
CT_VOID CDipThread::UpdateRechargeAmount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);
	
	acl::json Json(szJson);
	const acl::json_node* pType = Json.getFirstElementByTagName("type");
	if (pType == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find type.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到type");
		return;
	}
	
	const acl::json_node* pAmount1 = Json.getFirstElementByTagName("amount1");
	if (pAmount1 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount1.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount1");
		return;
	}
	
	const acl::json_node* pAmount2 = Json.getFirstElementByTagName("amount2");
	if (pAmount2 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount2.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount2");
		return;
	}
	
	const acl::json_node* pAmount3 = Json.getFirstElementByTagName("amount3");
	if (pAmount3 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount3.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount3");
		return;
	}
	
	const acl::json_node* pAmount4 = Json.getFirstElementByTagName("amount4");
	if (pAmount4 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount4.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount4");
		return;
	}
	
	const acl::json_node* pAmount5 = Json.getFirstElementByTagName("amount5");
	if (pAmount5 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount5.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount5");
		return;
	}
	
	const acl::json_node* pAmount6 = Json.getFirstElementByTagName("amount6");
	if (pAmount6 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount6.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount6");
		return;
	}
	
	const acl::json_node* pMaxAmount = Json.getFirstElementByTagName("maxAmount");
	if (pMaxAmount == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find max amount.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到max amount.");
		return;
	}
	
	
	tagRechargeChannelAmount rechargeChannelAmount;
	rechargeChannelAmount.cbChannelID = (CT_BYTE)*pType->get_int64();
	rechargeChannelAmount.dwAmount[0] = (CT_DWORD)*pAmount1->get_int64();
	rechargeChannelAmount.dwAmount[1] = (CT_DWORD)*pAmount2->get_int64();
	rechargeChannelAmount.dwAmount[2] = (CT_DWORD)*pAmount3->get_int64();
	rechargeChannelAmount.dwAmount[3] = (CT_DWORD)*pAmount4->get_int64();
	rechargeChannelAmount.dwAmount[4] = (CT_DWORD)*pAmount5->get_int64();
	rechargeChannelAmount.dwAmount[5] = (CT_DWORD)*pAmount6->get_int64();
	rechargeChannelAmount.dwAmount[6] = (CT_DWORD)*pMaxAmount->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_RECHARGE_AMOUNT, &rechargeChannelAmount, sizeof(rechargeChannelAmount));
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//修改兑换渠道的
CT_VOID CDipThread::UpdateExchangeAmount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);
	
	acl::json Json(szJson);
	const acl::json_node* pType = Json.getFirstElementByTagName("type");
	if (pType == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find type.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到type");
		return;
	}
	
	const acl::json_node* pAmount1 = Json.getFirstElementByTagName("amount1");
	if (pAmount1 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount1.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount1");
		return;
	}
	
	const acl::json_node* pAmount2 = Json.getFirstElementByTagName("amount2");
	if (pAmount2 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount2.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount2");
		return;
	}
	
	const acl::json_node* pAmount3 = Json.getFirstElementByTagName("amount3");
	if (pAmount3 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount3.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount3");
		return;
	}
	
	const acl::json_node* pAmount4 = Json.getFirstElementByTagName("amount4");
	if (pAmount4 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount4.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount4");
		return;
	}
	
	const acl::json_node* pAmount5 = Json.getFirstElementByTagName("amount5");
	if (pAmount5 == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find amount5.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到amount5");
		return;
	}
	
	const acl::json_node* pMaxAmount = Json.getFirstElementByTagName("maxAmount");
	if (pMaxAmount == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find max amount.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到max amount.");
		return;
	}
	
	const acl::json_node* pMinAmount = Json.getFirstElementByTagName("minAmount");
	if (pMinAmount == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find min amount.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到min amount.");
		return;
	}
	
	const acl::json_node* pDayCountLimit = Json.getFirstElementByTagName("dayCountLimit");
	if (pDayCountLimit == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find day count limit.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到日兑换限制.");
		return;
	}
	
	const acl::json_node* pHandlingFee = Json.getFirstElementByTagName("handlingFee");
	if (pHandlingFee == NULL)
	{
		LOG(WARNING) << "UpdateRechargeAmount fail, can not find day handling fee.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到兑换手续费.");
		return;
	}
	
	tagExchangeChannelAmount exchangeChannelAmount;
	exchangeChannelAmount.cbChannelID = (CT_BYTE)*pType->get_int64();
	exchangeChannelAmount.dwAmount[0] = (CT_DWORD)*pAmount1->get_int64();
	exchangeChannelAmount.dwAmount[1] = (CT_DWORD)*pAmount2->get_int64();
	exchangeChannelAmount.dwAmount[2] = (CT_DWORD)*pAmount3->get_int64();
	exchangeChannelAmount.dwAmount[3] = (CT_DWORD)*pAmount4->get_int64();
	exchangeChannelAmount.dwAmount[4] = (CT_DWORD)*pAmount5->get_int64();
	exchangeChannelAmount.dwMaxAmount = (CT_DWORD)*pMaxAmount->get_int64();
	exchangeChannelAmount.dwMinAmount = (CT_DWORD)*pMinAmount->get_int64();
	exchangeChannelAmount.cbDayCountLimite = (CT_BYTE)*pDayCountLimit->get_int64();
	exchangeChannelAmount.wHandlingFee = (CT_WORD)*pHandlingFee->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_EXCHANGE_AMOUNT, &exchangeChannelAmount, sizeof(exchangeChannelAmount));
	
	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//捕鱼点控数据
CT_VOID  CDipThread::SetUserDianKong(const CT_VOID *pData, CT_DWORD wDataSize, acl::aio_socket_stream *pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pGameIndex = Json.getFirstElementByTagName("gameIndex");
	if (pGameIndex == NULL)
	{
		LOG(WARNING) << "SetUserDianKong fail, can not find gameIndex.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到gameIndex");
		return;
	}

	const acl::json_node* pUserID = Json.getFirstElementByTagName("UserID");
	if (pUserID == NULL)
	{
		LOG(WARNING) << "SetUserDianKong fail, can not find UserID.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到UserID");
		return;
	}

	const acl::json_node* pDianKongZhi = Json.getFirstElementByTagName("DianKongZhi");
	if (pDianKongZhi == NULL)
	{
		LOG(WARNING) << "SetUserDianKong fail, can not find DianKongZhi.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到DianKongZhi");
		return;
	}

	const acl::json_node* pDianKongFen = Json.getFirstElementByTagName("DianKongFen");
	if (pDianKongFen == NULL)
	{
		LOG(WARNING) << "SetUserDianKong fail, can not find DianKongFen.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到DianKongFen");
		return;
	}

	if (!pUserID->is_number())
	{
		LOG(WARNING) << "pUserID is not number.";
		ReturnCommonToWeb(pWebSock, "1", "pUserID is not number");
		return;
	}

	if (!pDianKongZhi->is_number())
	{
		LOG(WARNING) << "pDianKongZhi is not number.";
		ReturnCommonToWeb(pWebSock, "1", "pDianKongZhi is not number");
		return;
	}

	if (!pDianKongFen->is_number())
	{
		LOG(WARNING) << "pDianKongFen is not number.";
		ReturnCommonToWeb(pWebSock, "1", "pDianKongFen is not number");
		return;
	}

	MSG_D2CS_Set_FishDiankong fishDianKong;
	fishDianKong.bySource = 1;
	fishDianKong.dwGameIndex = (CT_DWORD)*pGameIndex->get_int64();
	fishDianKong.dwUserID = (CT_DWORD)*pUserID->get_int64();
	fishDianKong.iDianKongZhi = (CT_INT32)*pDianKongZhi->get_int64();
	fishDianKong.llDianKongFen = (CT_LONGLONG)*pDianKongFen->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_FISH_DIANKONG, &fishDianKong, sizeof(fishDianKong));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//炸金花牌库概率
CT_VOID CDipThread::UpdateZjhDepotCardRation(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize);

	acl::json Json(szJson);
	const acl::json_node* pDepotID = Json.getFirstElementByTagName("depotid");
	if (pDepotID == NULL)
	{
		LOG(WARNING) << "UpdateZjhDepotCardRation fail, can not find depot id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到牌库ID");
		return;
	}

    const acl::json_node* pCard1Ratio = Json.getFirstElementByTagName("card1Ratio");
    if (pCard1Ratio == NULL)
    {
        LOG(WARNING) << "UpdateZjhDepotCardRation fail, can not find pCard1Ratio.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到散牌概率");
        return;
    }

	const acl::json_node* pCard2Ratio = Json.getFirstElementByTagName("card2Ratio");
	if (pCard2Ratio == NULL)
	{
		LOG(WARNING) << "UpdateZjhDepotCardRation fail, can not find pCard2Ratio.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到对子概率");
		return;
	}

	const acl::json_node* pCard3Ratio = Json.getFirstElementByTagName("card3Ratio");
	if (pCard3Ratio == NULL)
	{
		LOG(WARNING) << "UpdateZjhDepotCardRation fail, can not find pCard3Ration .";
		ReturnCommonToWeb(pWebSock, "1", "没有找到顺子概率");
		return;
	}

	const acl::json_node* pCard4Ratio = Json.getFirstElementByTagName("card4Ratio");
	if (pCard4Ratio == NULL)
	{
		LOG(WARNING) << "UpdateZjhDepotCardRation fail, can not find pCard4Ratio.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到同花概率");
		return;
	}

	const acl::json_node* pCard5Ratio = Json.getFirstElementByTagName("card5Ratio");
	if (pCard5Ratio == NULL)
	{
		LOG(WARNING) << "UpdateZjhDepotCardRation fail, can not find pCard5Ratio.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到同花顺概率");
		return;
	}

	const acl::json_node* pCard6Ratio = Json.getFirstElementByTagName("card6Ratio");
	if (pCard6Ratio == NULL)
	{
		LOG(WARNING) << "UpdateZjhDepotCardRation fail, can not find pCard6Ratio.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到豹子概率");
		return;
	}

    MSG_D2CS_Reload_ZjhDepot_Ratio zjhDepotRatio;
	zjhDepotRatio.cbDepotID = (CT_BYTE)*pDepotID->get_int64();
	zjhDepotRatio.cbCard1Ration = (CT_BYTE)*pCard1Ratio->get_int64();
	zjhDepotRatio.cbCard2Ration = (CT_BYTE)*pCard2Ratio->get_int64();
	zjhDepotRatio.cbCard3Ration = (CT_BYTE)*pCard3Ratio->get_int64();
	zjhDepotRatio.cbCard4Ration = (CT_BYTE)*pCard4Ratio->get_int64();
	zjhDepotRatio.cbCard5Ration = (CT_BYTE)*pCard5Ratio->get_int64();
	zjhDepotRatio.cbCard6Ration = (CT_BYTE)*pCard6Ratio->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_ZJH_CARDRATIO, &zjhDepotRatio, sizeof(zjhDepotRatio));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "操作成功");
}

CT_VOID CDipThread::UpdateDuoBaoCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pID = Json.getFirstElementByTagName("id");
	if (pID == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find cfg id.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝id");
		return;
	}

	const acl::json_node* pSingleScore = Json.getFirstElementByTagName("singleScore");
	if (pSingleScore == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find singleScore.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝单注金额");
		return;
	}

	const acl::json_node* pUserLeaseScore = Json.getFirstElementByTagName("userLeaseScore");
	if (pUserLeaseScore == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find userLeaseScore.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝单注金额");
		return;
	}

	const acl::json_node* pLimitCount = Json.getFirstElementByTagName("limitCount");
	if (pLimitCount == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find limitCount.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝购买次数限制");
		return;
	}

	const acl::json_node* pTotalCount = Json.getFirstElementByTagName("totalCount");
	if (pTotalCount == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find totalCount.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝开奖次数");
		return;
	}

	const acl::json_node* pTime = Json.getFirstElementByTagName("time");
	if (pTime == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find time.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝过期时间");
		return;
	}

	const acl::json_node* pReward = Json.getFirstElementByTagName("reward");
	if (pReward == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find reward.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝奖励金额");
		return;
	}

	const acl::json_node* pRevenueRation = Json.getFirstElementByTagName("revenueRation");
	if (pRevenueRation == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find revenueRation.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝奖励税收比例");
		return;
	}

	const acl::json_node* pIndex = Json.getFirstElementByTagName("index");
	if (pIndex == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find index.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝期数");
		return;
	}

	const acl::json_node* pStatus = Json.getFirstElementByTagName("status");
	if (pStatus == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find status.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝状态");
		return;
	}

	const acl::json_node* pTitle = Json.getFirstElementByTagName("title");
	if (pTitle == NULL)
	{
		LOG(WARNING) << "UpdateDuoBaoCfg fail, can not find title.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到夺宝标题");
		return;
	}

	tagDuoBaoCfg duoBaoCfg;
	duoBaoCfg.wID = (CT_WORD)*pID->get_int64();
	duoBaoCfg.wSingleScore = (CT_WORD)*pSingleScore->get_int64();
	duoBaoCfg.dwUserLeaseScore = (CT_DWORD)*pUserLeaseScore->get_int64();
	duoBaoCfg.wlimitCount = (CT_WORD)*pLimitCount->get_int64();
	duoBaoCfg.dwTotalCount = (CT_DWORD)*pTotalCount->get_int64();
	duoBaoCfg.dwTimeLeft = (CT_DWORD)*pTime->get_int64();
	duoBaoCfg.dwReward = (CT_DWORD)*pReward->get_int64();
	duoBaoCfg.wRevenueRation = (CT_WORD)*pRevenueRation->get_int64();
	duoBaoCfg.dwIndex = (CT_DWORD)*pIndex->get_int64();
	duoBaoCfg.cbState = (CT_BYTE)*pStatus->get_int64();
	std::string strTitle(pTitle->get_string());
	_snprintf_info(duoBaoCfg.szTitle, sizeof(duoBaoCfg.szTitle), "%s", Utility::urlDecodeCPP(strTitle).c_str());

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_DUOBAO_CFG, &duoBaoCfg, sizeof(duoBaoCfg));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UpdateBenefitCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pVip2 = Json.getFirstElementByTagName("vip2");
	if (pVip2 == NULL)
	{
		LOG(WARNING) << "UpdateBenefitCfg fail, can not find vip2.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到vip2");
		return;
	}

	const acl::json_node* pTotalCount = Json.getFirstElementByTagName("totalCount");
	if (pTotalCount == NULL)
	{
		LOG(WARNING) << "UpdateBenefitCfg fail, can not find totalCount.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到奖励总次数");
		return;
	}

	const acl::json_node* pRewardScore = Json.getFirstElementByTagName("rewardScore");
	if (pRewardScore == NULL)
	{
		LOG(WARNING) << "UpdateBenefitCfg fail, can not find rewardScore.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到奖励金额");
		return;
	}

	const acl::json_node* pLessScore = Json.getFirstElementByTagName("lessScore");
	if (pLessScore == NULL)
	{
		LOG(WARNING) << "UpdateBenefitCfg fail, can not find lessScore.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到最少金币值");
		return;
	}

	tagBenefitReward benefitReward;
	benefitReward.cbVip2 = (CT_BYTE)*pVip2->get_int64();
	benefitReward.cbRewardCount = (CT_BYTE)*pTotalCount->get_int64();
	benefitReward.dwRewardScore = (CT_DWORD)*pRewardScore->get_int64();
	benefitReward.dwLessScore = (CT_DWORD)*pLessScore->get_int64();

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_BENEFIT_CFG, &benefitReward, sizeof(benefitReward));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UpdatePlatformRechargeCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
    char szJson[2048] = { 0 };
    strncpy(szJson, (const char*)pData, wDataSize - 1);

    acl::json Json(szJson);
    const acl::json_node* pPlatformId = Json.getFirstElementByTagName("platformId");
    if (pPlatformId == NULL)
    {
        LOG(WARNING) << "UpdatePlatformRechargeCfg fail, can not find platformId.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到platformId");
        return;
    }

    const acl::json_node* pRechargeDomain = Json.getFirstElementByTagName("rechargeDomain");
    if (pRechargeDomain == NULL)
    {
        LOG(WARNING) << "UpdatePlatformRechargeCfg fail, can not find rechargeDomain.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到rechargeDomain");
        return;
    }

    const acl::json_node* pPromoterRechargeType = Json.getFirstElementByTagName("PromoterRechargeType");
    if (pPromoterRechargeType == NULL)
    {
        LOG(WARNING) << "UpdatePlatformRechargeCfg fail, can not find PromoterRechargeType.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到PromoterRechargeType");
        return;
    }

    const acl::json_node* pPromoterRechargeUrl = Json.getFirstElementByTagName("PromoterRechargeUrl");
    if (pPromoterRechargeUrl == NULL)
    {
        LOG(WARNING) << "UpdatePlatformRechargeCfg fail, can not find PromoterRechargeUrl.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到PromoterRechargeUrl");
        return;
    }

    const acl::json_node* pPromoterRechargeUpload = Json.getFirstElementByTagName("PromoterRechargeUpload");
    if (pPromoterRechargeUpload == NULL)
    {
        LOG(WARNING) << "UpdatePlatformRechargeCfg fail, can not find PromoterRechargeUpload.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到PromoterRechargeUpload");
        return;
    }

    const acl::json_node* pPromoterRechargeMerchant = Json.getFirstElementByTagName("PromoterRechargeMerchant");
    if (pPromoterRechargeMerchant == NULL)
    {
        LOG(WARNING) << "UpdatePlatformRechargeCfg fail, can not find PromoterRechargeMerchant.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到PromoterRechargeMerchant");
        return;
    }

    const acl::json_node* pPromoterRechargeToken = Json.getFirstElementByTagName("PromoterRechargeToken");
    if (pPromoterRechargeToken == NULL)
    {
        LOG(WARNING) << "UpdatePlatformRechargeCfg fail, can not find PromoterRechargeToken.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到PromoterRechargeToken");
        return;
    }

    tagPlatformRechargeCfg platformRechargeCfg;
    platformRechargeCfg.cbPlatformId = (CT_BYTE)*pPlatformId->get_int64();
    platformRechargeCfg.cbPromoterRechargeType = (CT_BYTE)*pPromoterRechargeType->get_int64();
    _snprintf_info(platformRechargeCfg.szRechargeDomain, sizeof(platformRechargeCfg.szRechargeDomain), "%s", pRechargeDomain->get_string());
    _snprintf_info(platformRechargeCfg.szPromoterRechargeUrl, sizeof(platformRechargeCfg.szPromoterRechargeUrl), "%s", pPromoterRechargeUrl->get_string());
    _snprintf_info(platformRechargeCfg.szPromoterRechargeUpload, sizeof(platformRechargeCfg.szPromoterRechargeUpload), "%s", pPromoterRechargeUpload->get_string());
    _snprintf_info(platformRechargeCfg.szPromoterRechargeMerchant, sizeof(platformRechargeCfg.szPromoterRechargeMerchant), "%s", pPromoterRechargeMerchant->get_string());
    _snprintf_info(platformRechargeCfg.szPromoterRechargeToken, sizeof(platformRechargeCfg.szPromoterRechargeToken), "%s", pPromoterRechargeToken->get_string());

    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_PLATFORM_RECHARGE_CFG, &platformRechargeCfg, sizeof(tagPlatformRechargeCfg));

    //给web返回通用信息
    ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UpdateSMSPlatformId(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[64] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pSmsId = Json.getFirstElementByTagName("smsPid");
	if (pSmsId == NULL)
	{
		LOG(WARNING) << "UpdateSMSPlatformId fail, can not find smsPid.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到短信平台ID");
		return;
	}

	CT_DWORD dwSMSPid = (CT_DWORD)*pSmsId->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_SMS_PID, &dwSMSPid, sizeof(CT_DWORD));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}


//更新红包状态
CT_VOID CDipThread::UpdateRedPacketStatus(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[4096] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pRedPacket = Json.getFirstElementByTagName("redPacket");
	if (pRedPacket == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketStatus fail, can not find redPacket status.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到红包状态");
		return;
	}

	const acl::json_node* pRedPacketDes = Json.getFirstElementByTagName("redPacketDes");
	if (pRedPacketDes == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketStatus fail, can not find redPacket des.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到红包描述");
		return;
	}

	tagRedPacketStatus redPacketStatus;
	redPacketStatus.cbStatus = (CT_BYTE)*pRedPacket->get_int64();
	std::string des(pRedPacketDes->get_string());
	_snprintf_info(redPacketStatus.szDes, sizeof(redPacketStatus.szDes), "%s",  Utility::urlDecodeCPP(des).c_str());
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_REDPACKET_STATUS, &redPacketStatus, sizeof(tagRedPacketStatus));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//更新红包的房间配置
CT_VOID CDipThread::UpdateRedPacketRoomCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pGameIndex = Json.getFirstElementByTagName("gameIndex");
	if (pGameIndex == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketRoomCfg fail, can not find gameIndex.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到gameIndex");
		return;
	}

	const acl::json_node* pValue = Json.getFirstElementByTagName("value");
	if (pValue == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketRoomCfg fail, can not find value.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到value");
		return;
	}

	tagRedPacketRoomCfg redPacketRoomCfg;
	redPacketRoomCfg.dwGameIndex = (CT_DWORD)*pGameIndex->get_int64();
	redPacketRoomCfg.dwRedPacketValue = (CT_DWORD)*pValue->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_REDPACKET_ROOM_CFG, &redPacketRoomCfg, sizeof(tagRedPacketRoomCfg));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//更新红包的期数配置
CT_VOID CDipThread::UpdateRedPacketIndex(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pIndex = Json.getFirstElementByTagName("index");
	if (pIndex == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find index.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到gameIndex");
		return;
	}

	const acl::json_node* pIdList = Json.getFirstElementByTagName("idList");
	if (pIdList == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find idList.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到idList");
		return;
	}

	const acl::json_node* pTotalScore = Json.getFirstElementByTagName("totalScore");
	if (pTotalScore == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find TotalScore.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到TotalScore");
		return;
	}

	/*const acl::json_node* pGrabScore = Json.getFirstElementByTagName("grabScore");
	if (pGrabScore == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find GrabScore.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到GrabScore");
		return;
	}*/

	const acl::json_node* pStartDate = Json.getFirstElementByTagName("startDate");
	if (pStartDate == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find StartDate.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到StartDate");
		return;
	}

	const acl::json_node* pReadyTime = Json.getFirstElementByTagName("readyTime");
	if (pReadyTime == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find readyTime.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到readyTime");
		return;
	}

	const acl::json_node* pStartTime = Json.getFirstElementByTagName("startTime");
	if (pStartTime == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find readyTime.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到readyTime");
		return;
	}

	const acl::json_node* pEndTime = Json.getFirstElementByTagName("endTime");
	if (pEndTime == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find endTime.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到endTime");
		return;
	}

	const acl::json_node* pStatus = Json.getFirstElementByTagName("status");
	if (pStatus == NULL)
	{
		LOG(WARNING) << "UpdateRedPacketIndex fail, can not find status.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到status");
		return;
	}

	/*
	 * CT_DWORD 	dwIndex;                            //红包期数
	CT_DWORD 	dwGameIndex[RED_PACKET_ROOM_COUNT];	//活动配置ID(指是哪个房间)
	std::string startDate;                          //开始日期
    CT_BYTE     cbReadyHour;                        //准备时间
    CT_BYTE     cbReadyMin;                         //准备时间
    CT_BYTE     cbStartHour;                        //开始时间
    CT_BYTE     cbStartMin;                         //开始时间
    CT_BYTE     cbEndHour;                          //结束时间
    CT_BYTE     cbEndMin;                           //结束时间
    CT_DWORD 	dwRedPacketScore;			        //红包金币
    CT_DWORD    dwRedRacketGrabbedScore;            //红包被抢红包
    CT_WORD 	wRedPacketCount;			        //红包个数
    CT_BYTE 	cbStage;                            //活动阶段(0: 未开始 1: 活动准备阶段 2: 活动进行中 3: 活动结束未发奖 4:活动结束已发奖)
    CT_BYTE     cbState;                            //状态(1、启用 2、停用)
	 */

	tagRedPacketIndex redPacketIndex;
	redPacketIndex.dwIndex = (CT_DWORD)*pIndex->get_int64();
	std::string strIdList(pIdList->get_string());
	std::vector<std::string> vecRoomCfgID;
	Utility::stringSplit(strIdList, ",", vecRoomCfgID);
	int nIdCount = 0;
	for (auto& it : vecRoomCfgID)
	{
		redPacketIndex.dwGameIndex[nIdCount++] = (CT_DWORD)atoi(it.c_str());
		if (nIdCount > RED_PACKET_ROOM_COUNT)
		{
			break;
		}
	}

	_snprintf_info(redPacketIndex.szStartDate, sizeof(redPacketIndex.szStartDate), "%s", pStartDate->get_string());

	//准备时间
	{
		std::string strReadyTime(pReadyTime->get_string());
		std::vector<std::string> vecReadyTime;
		Utility::stringSplit(strReadyTime, ":", vecReadyTime);
		if (vecReadyTime.size() != 3)
		{
			LOG(WARNING) << "red packet ready time config error. red packet index: " << redPacketIndex.dwIndex;
			ReturnCommonToWeb(pWebSock, "1", "没有找到readyTime");
			return;
		}
		redPacketIndex.cbReadyHour = (CT_BYTE)atoi(vecReadyTime[0].c_str());
		redPacketIndex.cbReadyMin = (CT_BYTE)atoi(vecReadyTime[1].c_str());
	}

	//开始时间
	{
		std::string strStartTime(pStartTime->get_string());
		std::vector<std::string> vecStartTime;
		Utility::stringSplit(strStartTime, ":", vecStartTime);
		if (vecStartTime.size() != 3)
		{
			LOG(WARNING) << "red packet start time config error. red packet index: " << redPacketIndex.dwIndex;
			ReturnCommonToWeb(pWebSock, "1", "没有找到startTime");
			return;
		}
		redPacketIndex.cbStartHour = (CT_BYTE)atoi(vecStartTime[0].c_str());
		redPacketIndex.cbStartMin = (CT_BYTE)atoi(vecStartTime[1].c_str());
	}

	//结束时间
	{
		std::string strEndTime(pEndTime->get_string());
		std::vector<std::string> vecEndTime;
		Utility::stringSplit(strEndTime, ":", vecEndTime);
		if (vecEndTime.size() != 3)
		{
			LOG(WARNING) << "red packet end time config error. red packet index: " << redPacketIndex.dwIndex;
			ReturnCommonToWeb(pWebSock, "1", "没有找到endTime");
			return;
		}
		redPacketIndex.cbEndHour = (CT_BYTE)atoi(vecEndTime[0].c_str());
		redPacketIndex.cbEndMin = (CT_BYTE)atoi(vecEndTime[1].c_str());
	}

	redPacketIndex.dwRedPacketScore = (CT_DWORD)*pTotalScore->get_int64();
	redPacketIndex.cbState = (CT_BYTE)*pStatus->get_int64();
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_REDPACKET_INDEX, &redPacketIndex, sizeof(tagRedPacketIndex));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UpdateChatUploadUrl(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pUploadUrl = Json.getFirstElementByTagName("uploadUrl");
	if (pUploadUrl == NULL)
	{
		LOG(WARNING) << "UpdateChatUploadUrl fail, can not find upload url.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到upload url");
		return;
	}

	tagChatUpLoadUrl chatUpLoadUrl;
	_snprintf_info(chatUpLoadUrl.szUploadUrl, sizeof(chatUpLoadUrl.szUploadUrl), "%s", pUploadUrl->get_string());
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_CHAT_UPLOAD_URL, &chatUpLoadUrl, sizeof(tagChatUpLoadUrl));

	//给web返回通用信息
	ReturnCommonToWeb(pWebSock, "0", "成功");
}

//更新公告内容
CT_VOID CDipThread::UpdateAnnouncementInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
    char szJson[SYS_NET_SENDBUF_SIZE] = { 0 };
    strncpy(szJson, (const char*)pData, wDataSize - 1);

    acl::json Json(szJson);
    const acl::json_node* pId = Json.getFirstElementByTagName("Id");
    if (pId == NULL)
    {
        LOG(WARNING) << "UpdateAnnouncementInfo fail, can not find id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到id");
        return;
    }

    const acl::json_node* pSortId = Json.getFirstElementByTagName("sortId");
    if (pSortId == NULL)
    {
        LOG(WARNING) << "UpdateAnnouncementInfo fail, can not find sort id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到sort id");
        return;
    }

    const acl::json_node* pName = Json.getFirstElementByTagName("name");
    if (pName == NULL)
    {
        LOG(WARNING) << "UpdateAnnouncementInfo fail, can not find name.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到name");
        return;
    }

    const acl::json_node* pContent = Json.getFirstElementByTagName("content");
    if (pContent == NULL)
    {
        LOG(WARNING) << "UpdateAnnouncementInfo fail, can not find content.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到content");
        return;
    }

	const acl::json_node* pStatus = Json.getFirstElementByTagName("status");
	if (pStatus == NULL)
	{
		LOG(WARNING) << "UpdateAnnouncementInfo fail, can not find status.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到status");
		return;
	}

    tagAnnouncementInfo announcementInfo;
    announcementInfo.cbAnnouncementID = (CT_BYTE)*pId->get_int64();
    announcementInfo.cbSortID = (CT_BYTE)*pSortId->get_int64();
    announcementInfo.cbStatus = (CT_BYTE)*pStatus->get_int64();
    std::string name(pName->get_string());
    std::string content(pContent->get_string());
    _snprintf_info(announcementInfo.szName, sizeof(announcementInfo.szName), "%s", Utility::urlDecodeCPP(name).c_str());
    _snprintf_info(announcementInfo.szContent, sizeof(announcementInfo.szContent), "%s", Utility::urlDecodeCPP(content).c_str());
    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_ANNOUNCEMENT, &announcementInfo, sizeof(tagAnnouncementInfo));

    //LOG(WARNING) << "recv annoucement info, id: " << (int)announcementInfo.cbAnnouncementID << ", name: " << announcementInfo.szName << ", content: " << announcementInfo.szContent;

    //给web返回通用信息
    ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UpdateBlackChannel(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
	char szJson[512] = { 0 };
	strncpy(szJson, (const char*)pData, wDataSize - 1);

	acl::json Json(szJson);
	const acl::json_node* pGameIndex = Json.getFirstElementByTagName("gameIndex");
	if (pGameIndex == NULL)
	{
		LOG(WARNING) << "UpdateBlackChannel fail, can not find gameIndex.";
		ReturnCommonToWeb(pWebSock, "1", "没有找到gameindex");
		return;
	}
    const acl::json_node* pChannelId = Json.getFirstElementByTagName("channelId");
    if (pChannelId == NULL)
    {
        LOG(WARNING) << "UpdateBlackChannel fail, can not find channelId.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 channelId");
        return;
    }
    const acl::json_node* pBadCardRatio = Json.getFirstElementByTagName("badCardRatio");
    if (pBadCardRatio == NULL)
    {
        LOG(WARNING) << "UpdateBlackChannel fail, can not find badCardRatio.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到badCardRatio");
        return;
    }

    tagBlackChannelEx blackChannelEx;
    blackChannelEx.dwChannelID = (CT_DWORD)*pChannelId->get_int64();
    blackChannelEx.wBadCardRation = (CT_WORD)*pBadCardRatio->get_int64();
    blackChannelEx.dwGameIndex = (CT_DWORD)*pGameIndex->get_int64();
    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_BLACK_CHANNEL, &blackChannelEx, sizeof(tagBlackChannelEx));

    //给web返回通用信息
    ReturnCommonToWeb(pWebSock, "0", "成功");
}

//更新公会信息
CT_VOID CDipThread::UpdateGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
    char szJson[4096] = { 0 };
    strncpy(szJson, (const char*)pData, wDataSize - 1);

    acl::json Json(szJson);
    const acl::json_node* pGroupId = Json.getFirstElementByTagName("id");
    if (pGroupId == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find group id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到group id");
        return;
    }

    /*const acl::json_node* pMasterId = Json.getFirstElementByTagName("masterId");
    if (pMasterId == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find master id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 master id");
        return;
    }

    const acl::json_node* pPlatformId = Json.getFirstElementByTagName("platformId");
    if (pPlatformId == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find platform id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 platform id");
        return;
    }*/

    const acl::json_node* pContribution = Json.getFirstElementByTagName("contribution");
    if (pContribution == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find contribution.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 contribution");
        return;
    }

    /*const acl::json_node* pIcon = Json.getFirstElementByTagName("icon");
    if (pIcon == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find icon.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 icon");
        return;
    }*/

    const acl::json_node* pLevel = Json.getFirstElementByTagName("level");
    if (pLevel == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find level.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 level");
        return;
    }

    const acl::json_node* pIncomeRate = Json.getFirstElementByTagName("incomeRate");
    if (pIncomeRate == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find incomeRate.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到incomeRate");
        return;
    }

    const acl::json_node* pSettleDays = Json.getFirstElementByTagName("settleDays");
    if (pSettleDays == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find settleDays.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 settleDays");
        return;
    }

    const acl::json_node* pGroupName = Json.getFirstElementByTagName("groupName");
    if (pGroupName == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find group name.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 group name");
        return;
    }

    const acl::json_node* pNotice = Json.getFirstElementByTagName("notice");
    if (pNotice == NULL)
    {
        LOG(WARNING) << "UpdateGroupInfo fail, can not find notice.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 notice");
        return;
    }

    tagGroupInfo groupInfo;
    groupInfo.dwGroupID = (CT_DWORD)*pGroupId->get_int64();
    //groupInfo.dwMasterID = (CT_DWORD)*pMasterId->get_int64();
    groupInfo.dwContribution = (CT_DWORD)*pContribution->get_int64();
    groupInfo.cbIncomeRate = (CT_BYTE)*pIncomeRate->get_int64();
    groupInfo.cbSettleDays = (CT_BYTE)*pSettleDays->get_int64();
    groupInfo.cbLevel = (CT_BYTE)*pLevel->get_int64();
    //groupInfo.cbIcon = (CT_BYTE)*pIcon->get_int64();
    std::string groupName(pGroupName->get_string());
    std::string groupNotice(pNotice->get_string());
    _snprintf_info(groupInfo.szGroupName, sizeof(groupInfo.szGroupName), "%s", Utility::urlDecodeCPP(groupName).c_str());
    _snprintf_info(groupInfo.szNotice, sizeof(groupInfo.szNotice), "%s", Utility::urlDecodeCPP(groupNotice).c_str());
    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_GROUP_INFO, &groupInfo, sizeof(tagGroupInfo));

    //LOG(WARNING) << "UpdateGroupInfo, income rate: " << (int)groupInfo.cbIncomeRate << ", settle day: " << groupInfo.cbSettleDays;

    //给web返回通用信息
    ReturnCommonToWeb(pWebSock, "0", "成功");
}

//更新公会成员信息
CT_VOID CDipThread::UpdateGroupUserInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
    char szJson[1024] = { 0 };
    strncpy(szJson, (const char*)pData, wDataSize - 1);

    acl::json Json(szJson);
    const acl::json_node* pGroupId = Json.getFirstElementByTagName("groupId");
    if (pGroupId == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find group id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到group id");
        return;
    }
    const acl::json_node* pUserId = Json.getFirstElementByTagName("userId");
    if (pUserId == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find user id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 user id");
        return;
    }

    /*const acl::json_node* pParentUserId = Json.getFirstElementByTagName("parentUserId");
    if (pParentUserId == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find parent user id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 parent user id");
        return;
    }*/

    const acl::json_node* pIncomeRate = Json.getFirstElementByTagName("incomeRate");
    if (pIncomeRate == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find income rate.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 income rate");
        return;
    }

    const acl::json_node* pDeductRate = Json.getFirstElementByTagName("deductRate");
    if (pDeductRate == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find deduct rate.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 deduct rate");
        return;
    }

    const acl::json_node* pDeductTop = Json.getFirstElementByTagName("deductTop");
    if (pDeductTop == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find deduct top.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 deduct top");
        return;
    }

    const acl::json_node* pDeductIncre = Json.getFirstElementByTagName("deductIncre");
    if (pDeductIncre == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find deductIncre.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 deductIncre");
        return;
    }

    const acl::json_node* pDeductStart = Json.getFirstElementByTagName("deductStart");
    if (pDeductStart == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find deductStart.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 deductStart");
        return;
    }

    /*const acl::json_node* pLastTakeIncomeDate = Json.getFirstElementByTagName("lastTakeIncomeDate");
    if (pLastTakeIncomeDate == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find lastTakeIncomeDate.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 lastTakeIncomeDate");
        return;
    }

    const acl::json_node* pLastIncomeDate = Json.getFirstElementByTagName("lastIncomeDate");
    if (pLastIncomeDate == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find lastIncomeDate.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 lastIncomeDate");
        return;
    }

    const acl::json_node* pLinkIncomeDays = Json.getFirstElementByTagName("linkIncomeDays");
    if (pLinkIncomeDays == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find linkIncomeDays.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 linkIncomeDays");
        return;
    }

    const acl::json_node* pTotalIncome = Json.getFirstElementByTagName("totalIncome");
    if (pTotalIncome == NULL)
    {
        LOG(WARNING) << "UpdateGroupUserInfo fail, can not find totalIncome.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 totalIncome");
        return;
    }*/

    /*
     * CT_DWORD		dwUserID;					    //群成员ID
	CT_DWORD        dwParentUserId;                 //上级成员ID(群主的上级成员为0)
	CT_BYTE 		cbSex;						    //性别
	CT_BYTE 		cbHeadId;					    //头像
	CT_BYTE 		cbVip2;						    //成员Vip2
	CT_BYTE         cbMemLevel;                     //群成员等级(群主是总代等级为0)
	CT_BYTE         cbInComeRate;                   //收入比例
    CT_WORD         wDeductRate;                    //扣量比例(千分比)
    CT_WORD         wDeductTop;                     //扣量封顶(千分比)
    CT_WORD         wDeductIncre;                   //扣量递增(千分比)
    CT_DWORD        dwDeductStart;                  //扣量起征点(单位:分)
    CT_DWORD        dwLastTakeIncomeDate;           //最后提取收入时间
    CT_DWORD        dwLastIncomeDate;               //最后产生收入的时间
    CT_WORD         wLinkIncomeDays;                //连续产生收益的
    CT_LONGLONG 	llTotalIncome;				    //成员总收入
	CT_DWORD        dwRecharge;                     //群成员充值
	CT_DWORD        dwRegDate;                      //注册时间
	CT_DWORD        dwLastDate;					    //最后登录时间
	CT_BOOL         bOnline;                        //玩家是否在线
	CT_CHAR         szRemarks[GROUP_REMARKS_LEN];   //上级给他的备注
     */

    tagGroupUserInfo groupUserInfo;
    groupUserInfo.dwUserID = (CT_DWORD)*pUserId->get_int64();
    //groupUserInfo.dwParentUserId = (CT_DWORD)*pParentUserId->get_int64();
    groupUserInfo.cbInComeRate = (CT_BYTE)*pIncomeRate->get_int64();
    groupUserInfo.wDeductRate = (CT_WORD)*pDeductRate->get_int64();
    groupUserInfo.wDeductTop = (CT_WORD)*pDeductTop->get_int64();
    groupUserInfo.wDeductIncre = (CT_WORD)*pDeductIncre->get_int64();
    groupUserInfo.dwDeductStart = (CT_WORD)*pDeductStart->get_int64();
    //groupUserInfo.dwLastTakeIncomeDate = (CT_DWORD)*pLastTakeIncomeDate->get_int64();
    //groupUserInfo.dwLastIncomeDate = (CT_DWORD)*pLastIncomeDate->get_int64();
    //groupUserInfo.wLinkIncomeDays = (CT_DWORD)*pLinkIncomeDays->get_int64();
    //groupUserInfo.llTotalIncome = (CT_LONGLONG)*pTotalIncome->get_int64();

    CT_DWORD dwGroupID = (CT_DWORD)*pGroupId->get_int64();

    CT_BYTE     szBuffer[256];
    CT_DWORD    dwSendSize = 0;
    memcpy(szBuffer, &dwGroupID, sizeof(CT_DWORD));
    dwSendSize += sizeof(CT_DWORD);
    memcpy(szBuffer+dwSendSize, &groupUserInfo, sizeof(tagGroupUserInfo));
    dwSendSize += sizeof(tagGroupUserInfo);
    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_GROUP_USER_INFO, szBuffer, dwSendSize);

    //LOG(WARNING) << "UpdateGroupUserInfo, income rate: " << (int)groupUserInfo.cbInComeRate << ", wDeductRate: " << groupUserInfo.wDeductRate
    //<< ", wDeductTop: " << groupUserInfo.wDeductTop << ", wDeductIncre: " << groupUserInfo.wDeductIncre << ", dwDeductStart: " << groupUserInfo.dwDeductStart;

    //给web返回通用信息
    ReturnCommonToWeb(pWebSock, "0", "成功");
}

//更新公会等级配置信息
CT_VOID CDipThread::UpdateGroupLevelCfg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
    char szJson[1024] = { 0 };
    strncpy(szJson, (const char*)pData, wDataSize - 1);

    acl::json Json(szJson);
    const acl::json_node* pLevel = Json.getFirstElementByTagName("level");
    if (pLevel == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find level.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 level");
        return;
    }

    const acl::json_node* pMaxContribution = Json.getFirstElementByTagName("maxContribution");
    if (pMaxContribution == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find maxContribution.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 maxContribution");
        return;
    }

    const acl::json_node* pIncomeRate = Json.getFirstElementByTagName("incomeRate");
    if (pIncomeRate == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find incomeRate.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 incomeRate");
        return;
    }

    const acl::json_node* pDeductRate = Json.getFirstElementByTagName("DeductRate");
    if (pDeductRate == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find DeductRate.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 DeductRate");
        return;
    }

    const acl::json_node* pDeductTop = Json.getFirstElementByTagName("DeductTop");
    if (pDeductTop == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find DeductTop.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 DeductTop");
        return;
    }

    const acl::json_node* pDeductIncre = Json.getFirstElementByTagName("DeductIncre");
    if (pDeductIncre == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find DeductIncre.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 DeductIncre");
        return;
    }

    const acl::json_node* pDeductStart = Json.getFirstElementByTagName("DeductStart");
    if (pDeductStart == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find DeductStart.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 DeductStart");
        return;
    }

    const acl::json_node* pSettleDays = Json.getFirstElementByTagName("settleDays");
    if (pSettleDays == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find settleDays.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 settleDays");
        return;
    }

    const acl::json_node* pSettleMinIncome = Json.getFirstElementByTagName("settleMinIncome");
    if (pSettleMinIncome == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find settleMinIncome.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 settleMinIncome");
        return;
    }

    const acl::json_node* pSettleMaxIncome = Json.getFirstElementByTagName("settleMaxIncome");
    if (pSettleMaxIncome == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find settleMaxIncome.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 settleMaxIncome");
        return;
    }

    const acl::json_node* pDomain = Json.getFirstElementByTagName("domain");
    if (pDomain == NULL)
    {
        LOG(WARNING) << "UpdateGroupLevelCfg fail, can not find domain.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 domain");
        return;
    }

    /*
    CT_BYTE 		cbLevel;                   //等级
	CT_BYTE         cbSettleDays;              //结算天数
	CT_WORD         wIncomeRate;               //本等级默认收益比例(百分比)
	CT_WORD         wDeductRate;               //扣量比例(千分比)
	CT_WORD         wDeductTop;                //扣量封顶(千分比)
	CT_WORD         wDeductIncre;              //扣量递增(千分比)
	CT_DWORD        dwDeductStart;             //扣量起征点(单位:分)
	CT_DWORD 		dwMaxContribution;         //本等级最大贡献值(下一等级开始值)
	CT_CHAR         szDomain[GROUP_PROMOTION_DOMIAN]; //推广链接
     */
    tagGroupLevelCfg groupLevelCfg;
    groupLevelCfg.cbLevel = (CT_BYTE)*pLevel->get_int64();
    groupLevelCfg.cbSettleDays = (CT_BYTE)*pSettleDays->get_int64();
    groupLevelCfg.wIncomeRate = (CT_WORD)*pIncomeRate->get_int64();
    groupLevelCfg.wDeductRate = (CT_WORD)*pDeductRate->get_int64();
    groupLevelCfg.wDeductTop = (CT_WORD)*pDeductTop->get_int64();
    groupLevelCfg.wDeductIncre = (CT_WORD)*pDeductIncre->get_int64();
    groupLevelCfg.dwDeductStart = (CT_DWORD)*pDeductStart->get_int64();
    groupLevelCfg.dwMaxContribution = (CT_DWORD)*pMaxContribution->get_int64();
    groupLevelCfg.dwSettleMinIncome = (CT_DWORD)*pSettleMinIncome->get_int64();
    groupLevelCfg.dwSettleMaxIncome = (CT_DWORD)*pSettleMaxIncome->get_int64();
    std::string strDomain(pDomain->get_string());
    _snprintf_info(groupLevelCfg.szDomain, sizeof(groupLevelCfg.szDomain), "%s", Utility::urlDecodeCPP(strDomain).c_str());
    //LOG(WARNING) << "UpdateGroupLevelCfg, domain: " << pDomain->get_string();

    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_UPDATE_GROUP_LEVEL_CFG, &groupLevelCfg,
                                    sizeof(tagGroupLevelCfg));

    //给web返回通用信息
    ReturnCommonToWeb(pWebSock, "0", "成功");
}

CT_VOID CDipThread::UserBindGroup(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pWebSock)
{
    char szJson[128] = { 0 };
    strncpy(szJson, (const char*)pData, wDataSize - 1);

    acl::json Json(szJson);
    const acl::json_node* pUserID = Json.getFirstElementByTagName("userId");
    if (pUserID == NULL)
    {
        LOG(WARNING) << "UserBindGroup fail, can not find user id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 user id");
        return;
    }

    const acl::json_node* pParentUserID = Json.getFirstElementByTagName("parentUserId");
    if (pParentUserID == NULL)
    {
        LOG(WARNING) << "UserBindGroup fail, can not find parent user id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 parent user id");
        return;
    }

    const acl::json_node* pSex = Json.getFirstElementByTagName("gender");
    if (pSex == NULL)
    {
        LOG(WARNING) << "UserBindGroup fail, can not find sex.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到性别");
        return;
    }

    const acl::json_node* pHeadId = Json.getFirstElementByTagName("headId");
    if (pHeadId == NULL)
    {
        LOG(WARNING) << "UserBindGroup fail, can not find head id.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到头像");
        return;
    }

    const acl::json_node* pVip2 = Json.getFirstElementByTagName("vip2");
    if (pVip2 == NULL)
    {
        LOG(WARNING) << "UserBindGroup fail, can not find vip2.";
        ReturnCommonToWeb(pWebSock, "1", "没有找到 vip2");
        return;
    }


    MSG_L2CS_AddGroupUser addGroupUser;
    addGroupUser.dwUserID = (CT_DWORD)*pUserID->get_int64();
    addGroupUser.dwParentUserID = (CT_DWORD)*pParentUserID->get_int64();
    addGroupUser.cbSex = (CT_BYTE)*pSex->get_int64();
    addGroupUser.cbHeadId = (CT_BYTE)*pHeadId->get_int64();
    addGroupUser.cbVip2 = (CT_BYTE)*pVip2->get_int64();
    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_ADD_GROUP_USER, &addGroupUser,
                                    sizeof(MSG_L2CS_AddGroupUser));

    //给web返回通用信息
    ReturnCommonToWeb(pWebSock, "0", "成功");
}