#include "stdafx.h"
#include "Utility.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "ServerCfg.h"
#include "CustomerThread.h"
#include "ServerMgr.h"

#define		IDI_TEST_INSERT					(100)
#define		TIME_TEST_INSERT				(10000)				//10s重连一次

#define		IDI_CHECK_DDZ_USERCOUNT			(101)
#define		TIME_CHECK_DDZ_USERCOUNT		(2000)

//中心服务器连接
extern CNetConnector *pNetCenter;

//定时器
#define		IDI_CONNECT_CENTER				(102)
#define		TIME_CONNECT_CENTER				(5000)

#define 	IDI_SEND_UNREAD_MSG				(103)
#define 	TIME_SEND_UNREAD_MSG			(5000)

CCustomerThread::CCustomerThread(void)
	:m_redisClient(CServerCfg::m_RedisAddress, CServerCfg::m_RedisConTimeOut, CServerCfg::m_RedisRwTimeOut)
	, m_bInitServer(false)
{
  // CNetModule::getSingleton().SetTimer(IDI_TEST_INSERT, TIME_TEST_INSERT, this, true);

	acl::string charset;
#ifdef _OS_WIN32_CODE
	acl::db_handle::set_loadpath("libmysql.dll");
	charset.format("%s", "gbk");
#elif defined(_OS_LINUX_CODE)
	std::string strlibMysqlPath = Utility::GetCurrentDirectory();
	auto it = strlibMysqlPath.find_last_of("/");
	if (it != std::string::npos)
	{
		strlibMysqlPath.erase(it);
	}
	strlibMysqlPath.append("/libmysqlclient_r.so");
	acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
	charset.format("%s", "utf8");
#else
    std::string strlibMysqlPath = Utility::GetCurrentDirectory();
    auto it = strlibMysqlPath.find_last_of("/");
    if (it != std::string::npos)
    {
        strlibMysqlPath.erase(it);
    }
    strlibMysqlPath.append("/libmysqlclient_r.dylib");
    acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
    charset.format("%s", "utf8");
#endif

	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_chatdbIP, CServerCfg::m_chatdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_chatdbName, CServerCfg::m_chatdbUser, CServerCfg::m_chatdbPwd, 1, 0, true, 60, 120, charset.c_str());

	m_chatDBKey.format("%s@%s:%d", CServerCfg::m_chatdbName, CServerCfg::m_chatdbIP, CServerCfg::m_chatdbPort);
}

CCustomerThread::~CCustomerThread(void)
{
}

CT_VOID CCustomerThread::InitServer()
{
	m_redisClient.set_password(CServerCfg::m_RedisPwd);
	m_redis.set_client(&m_redisClient);
	//CNetModule::getSingleton().SetTimer(IDI_CHECK_DDZ_USERCOUNT, TIME_CONNECT_CENTER, this, true);
	//CNetModule::getSingleton().SetTimer(IDI_SEND_UNREAD_MSG, TIME_SEND_UNREAD_MSG, this, true);

    std::unique_ptr<CQueryRecordThread> queryRecordThread(new CQueryRecordThread(false));
    m_QueryRecordThreadPtr = std::move(queryRecordThread);

    m_QueryRecordThreadPtr->set_detachable(true);
    if (m_QueryRecordThreadPtr->start() == false)
    {
        LOG(WARNING) << "start query record thread failed!";
    }
}

CT_VOID CCustomerThread::OnTCPSocketLink(CNetConnector* pConnector)
{
	if (pConnector == NULL)
	{
		return;
	}

	stConnectParam& connectParam = pConnector->GetConnectParam();
	if (connectParam.conType == CONNECT_CENTER)
	{
		//注册到中心服务器
		GoCenterServerRegister();
		LOG(INFO) << "link to center server succ!";
		if (!m_bInitServer)
		{
			InitServer();
			m_bInitServer = true;
		}
	}
}

CT_VOID CCustomerThread::OnTCPSocketShut(CNetConnector* pConnector)
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

CT_VOID CCustomerThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{

}

CT_VOID CCustomerThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
	CServerMgr::get_instance().DeleteProxyServer(pSocket);
}

//网络bit数据到来
CT_VOID CCustomerThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
    if (pSocket == NULL)
    {
        return;
    }
    switch (pMc->dwMainID)
    {
    case MSG_CUSTOMER_MAIN:
    {
        OnMsgCustomerMain(pSocket, pMc, pBuf, wLen);
        break;
    }
	case MSG_CUSTOMERPS_MAIN:
	{
		OnMsgProxyServerMain(pSocket, pMc, pBuf, wLen);
		break;
	}
    default:
    {
        CNetModule::getSingleton().CloseSocket(pSocket);
        break;
    }
    }
}

CT_VOID CCustomerThread::OnMsgCustomerMain(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
    switch (pMc->dwSubID)
    {
	case SUB_CUSTOMER2SERVER_CHAT:
	{
		OnSubCustomerChat(pSocket, pBuf, wLen);
	}
	break;
	case SUB_CLIENT2SERVER_USER_VISIT:
	{
		OnSubUserVisit(pSocket, pBuf, wLen);
	}
	break;
	case SUB_CLIENT2SERVER_CHAT:
    {
		OnSubUserChat(pSocket, pBuf, wLen);
    }
	break;
	case SUB_CLINET2SERVER_READ_MSG:
	{
		OnSubUserReadMsg(pBuf, wLen);
		break;
	}
    default:
    {
        CNetModule::getSingleton().CloseSocket(pSocket);
    }
    break;
    }
}

CT_VOID CCustomerThread::OnMsgProxyServerMain(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
	switch (pMc->dwSubID)
	{
	case SUB_PS2CUSTOMER_BIND:
	{
		if (wLen != sizeof(PS_BindData))
		{
			return;
		}

		PS_BindData* pBindData = (PS_BindData*)pBuf;
		CServerMgr::get_instance().BindProxyServer(pSocket, pBindData);
	}
	break;
	default:break;
	}
}

//客服发聊天
CT_VOID CCustomerThread::OnSubCustomerChat(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD wLen)
{
	char szJson[1024] = { 0 };
	strncpy(szJson, (const char*)pBuf, wLen-1);
	
	std::string strJson(szJson);
	std::string strJsonDecode = Utility::urlDecodeCPP(strJson);
	
	acl::json json(strJsonDecode.c_str());
	
	const acl::json_node* pUserid = json.getFirstElementByTagName("userid");
	if (pUserid == NULL)
	{
	    LOG(WARNING) << "can not find userid.";
		return;
	}
	
	const acl::json_node* pText = json.getFirstElementByTagName("text");
	if (pText == NULL)
	{
        LOG(WARNING) << "can not find text.";
        return;
	}

	/*
	 * type: 1 玩家发送 2 客服发送
	 * msgType: 1 普通 2 举报 4 充值 5 兑换
	 * ContentType: 1 文字 2 图片
	 */

	const acl::json_node* pType = json.getFirstElementByTagName("type");
	if (pType == NULL)
	{
		LOG(WARNING) << "can not find Type.";
		return;
	}
	
	const acl::json_node* pMsgType = json.getFirstElementByTagName("msgType");
	if (pMsgType == NULL)
	{
        LOG(WARNING) << "can not find msgType.";
        return;
	}

	const acl::json_node* pContentType = json.getFirstElementByTagName("contentType");
	if (pContentType == NULL)
	{
		LOG(WARNING) << "can not find context type.";
		return;
	}

    const acl::json_node* pId = json.getFirstElementByTagName("id");
    if (pId == NULL)
    {
        LOG(WARNING) << "can not find id.";
        return;
    }

    LOG(WARNING) << "recv chat: " << pText->get_string();

    int nType = (int)*pType->get_int64();
    int nMsgType = (int)*pMsgType->get_int64();
	CT_DWORD dwUserID = (CT_DWORD)*pUserid->get_int64();
	CT_DWORD dwMsgID = (CT_DWORD)*pId->get_int64();

	if (nType == 2)
	{
		MSG_S2C_Chat_To_User chatToUser;
		chatToUser.dwMsgId = dwMsgID;
		chatToUser.cbContentType = (int)*pContentType->get_int64();
		_snprintf_info(chatToUser.szContent, sizeof(chatToUser.szContent), "%s", pText->get_string());
		_snprintf_info(chatToUser.szTime, sizeof(chatToUser.szTime), "%s", Utility::GetTimeNowString().c_str());

		acl::aio_socket_stream* pProxySock = GetUserProxySock(dwUserID);
		if (pProxySock)
		{
			SendMsgToUser(pProxySock, dwUserID, SUB_SERVER2CLIENT_CHAT, &chatToUser, sizeof(chatToUser), nMsgType);
			// LOG(WARNING) << "send chat: " << pText->get_string();
		}
	}
	else
	{
	    //主要针对图片连接
		MSG_S2C_ChatRecord_To_User chatToUser;
		chatToUser.dwMsgId = dwMsgID;
		chatToUser.cbType = nType;
		chatToUser.cbIsRead = 1;
		chatToUser.cbContentType = (int)*pContentType->get_int64();
		_snprintf_info(chatToUser.szContent, sizeof(chatToUser.szContent), "%s", pText->get_string());
		_snprintf_info(chatToUser.szTime, sizeof(chatToUser.szTime), "%s", Utility::GetTimeNowString().c_str());

		acl::aio_socket_stream* pProxySock = GetUserProxySock(dwUserID);
		if (pProxySock)
		{
			SendMsgToUser(pProxySock, dwUserID, SUB_SERVER2CLIENT_CHAT_RECORD, &chatToUser, sizeof(chatToUser), nMsgType);
		}
	}


    ReturnCommonToWeb(pSocket, "0", "发送成功");
	//CNetModule::getSingleton().CloseSocket(pSocket);
}


CT_VOID CCustomerThread::OnSubUserVisit(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD wLen)
{
    if (sizeof(MSG_C2S_User_Visit) != wLen)
    {
        return;
    }

    MSG_C2S_User_Visit* pVisitLogin = (MSG_C2S_User_Visit*)pBuf;
   
	//发送玩家
	//SendUnReadMsgToClient(pSocket, pVisitLogin->dwUserID);
	//查询玩家最近10条消息
	//ReadLast10Msg(pSocket, pVisitLogin->dwUserID, 1);
	//InsertQueryRecord(pVisitLogin->dwUserID, pSocket, 1);
	InsertQueryRecord(pVisitLogin->dwUserID, pSocket, pVisitLogin->cbMsgType);
}

CT_VOID CCustomerThread::OnSubUserChat(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (wLen != sizeof(MSG_C2S_Chat_To_CustomerEx))
	{
		return;
	}

	MSG_C2S_Chat_To_CustomerEx *pChat2Customer = (MSG_C2S_Chat_To_CustomerEx*)pBuf;
	
	PostMsgToCustomer(pChat2Customer->dwUserID, pChat2Customer->szContent, pChat2Customer->szClientIp, pChat2Customer->cbType);
}

CT_VOID CCustomerThread::OnSubUserReadMsg(const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (wLen != sizeof(MSG_C2S_User_Read_CustomerMsg))
	{
		return;
	}

	MSG_C2S_User_Read_CustomerMsg* pReadCustomerMsg = (MSG_C2S_User_Read_CustomerMsg*)pBuf;
	if (pReadCustomerMsg == NULL)
	{
		return;
	}

	acl::string serverAddr;
	serverAddr.format("%s:%d", CServerCfg::m_CustomerServerAddr, CServerCfg::m_CustomerServerPort);
	acl::socket_stream conn;
	if (conn.open(serverAddr, 10, 10) == false)
	{
		LOG(WARNING) << "connect " << serverAddr.c_str() << "error :" << acl::last_serror();
		return;
	}

	char sendline[2048] = {0};
	char recvline[1024] = {0};
	char params[2048];
	_snprintf_info(params, sizeof(params), "{\"id\":\"%u\"}", pReadCustomerMsg->dwMsgID);
	acl::string urlEncodeMsg(params); //(Utility::urlEncodeCPP(params).c_str());

	//CT_CHAR szBuffer[2028] = { 0 };
	//m_encrypt.encrypt((CT_UCHAR*)urlEncodeMsg.c_str(), urlEncodeMsg.length(), (CT_UCHAR *)szBuffer);

	//std::string HexMsg = toHexString((CT_UCHAR*)urlEncodeMsg.c_str(),  urlEncodeMsg.length());
	_snprintf_info(params, sizeof(params), "id=%d", pReadCustomerMsg->dwMsgID);

	_snprintf_info(sendline, 2048,
				   "POST %s HTTP/1.1\r\n"
				   "Host: %s\r\n"
				   "Content-type: application/x-www-form-urlencoded\r\n"
                   //"Content-type: application/json\r\n"
				   "Content-length: %zu\r\n\r\n"
				   "%s", "/chat/status", CServerCfg::m_CustomerServerAddr, strlen(params), params);
	LOG(INFO) << sendline;
	if (conn.write(sendline, strlen(sendline)) == (-1))
	{
		LOG(INFO) << "http pose msg fail.";
	}
	ssize_t n;
	while ((n = conn.read(recvline, 1024 -1, false)) > 0)
	{
		recvline[n] = '\0';
		LOG(INFO) << Utility::Utf82Ansi(recvline).c_str();
		conn.close();
	}
	if (conn.alive())
		conn.close();
}

//发送消息给客户端
CT_VOID CCustomerThread::SendMsgToUser(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BYTE cbType)
{
	CT_DWORD dwDownSize = sizeof(MSG_GameMsgDownHead);
	CT_CHAR chBuf[4096] = { 0 };
	MSG_GameMsgDownHead DownData;
	memset(&DownData, 0, dwDownSize);
	DownData.dwMainID = MSG_CUSTOMER_MAIN;
	DownData.dwSubID = wSubCmdID;
	DownData.dwValue2 = dwUserID;
	memcpy(chBuf, &DownData, dwDownSize);
	memcpy(chBuf + dwDownSize, &cbType, sizeof(CT_BYTE));
	if (dwLen != 0)
		memcpy(&chBuf[dwDownSize+ sizeof(CT_BYTE)], pBuf, dwLen);

	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, chBuf, dwDownSize + sizeof(CT_BYTE) + dwLen);
}

//网络定时器
CT_VOID CCustomerThread::OnTimer(CT_DWORD dwTimerID)
{
    switch (dwTimerID)
    {
	case IDI_CONNECT_CENTER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetCenter);
	}
	break;
	case IDI_SEND_UNREAD_MSG:
	{
		/*
		for(auto it = m_mapUserUnReadMsg.begin(); it != m_mapUserUnReadMsg.end(); )
		{
			//检测玩家是否在线
			acl::aio_socket_stream* pUserSock = GetUserProxySock(it->first);
			if (pUserSock == NULL)
			{
				++it;
				continue;
			}
			
			std::vector<MSG_S2C_Chat_To_User >& vecChat = it->second;
			for (auto& itChat : vecChat)
			{
				SendMsgToUser(pUserSock, it->first, SUB_SERVER2CLIENT_CHAT, &itChat, sizeof(MSG_S2C_Chat_To_User));
			}
			
			it = m_mapUserUnReadMsg.erase(it);
		}
		
		for(auto it = m_mapUserReportUnReadMsg.begin(); it != m_mapUserReportUnReadMsg.end(); )
		{
			//检测玩家是否在线
			acl::aio_socket_stream* pUserSock = GetUserProxySock(it->first);
			if (pUserSock == NULL)
			{
				++it;
				continue;
			}
			
			std::vector<MSG_S2C_Chat_To_User >& vecChat = it->second;
			for (auto& itChat : vecChat)
			{
				SendMsgToUser(pUserSock, it->first, SUB_SERVER2CLIENT_CHAT_REPORT, &itChat, sizeof(MSG_S2C_Chat_To_User));
			}
			
			it = m_mapUserReportUnReadMsg.erase(it);
		}
		 */
	}
	break;
    default:
        break;
    }
}

//打开
CT_VOID CCustomerThread::Open()
{

}

//去总控服务器注册
CT_VOID CCustomerThread::GoCenterServerRegister()
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CMD_CustomerServer_Info registerCustomer;
		registerCustomer.dwServerID = CServerCfg::m_nServerID;
		_snprintf_info(registerCustomer.szServerIP, sizeof(registerCustomer.szServerIP), "%s", CServerCfg::m_LocalAddress);
		_snprintf_info(registerCustomer.szServerPubIP, sizeof(registerCustomer.szServerPubIP), "%s", CServerCfg::m_PublicAddress);
		_snprintf_info(registerCustomer.szServerName, sizeof(registerCustomer.szServerName), "%s", CServerCfg::m_ServerName);
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_CUSTOMERCS_MAIN, SUB_CUSTOMER2CS_REGSTER, &registerCustomer, sizeof(registerCustomer));
		return;
	}
}

acl::aio_socket_stream* CCustomerThread::GetUserProxySock(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("psinfo_%u", dwUserID);
	m_redis.clear();
	acl::string strProxyID;
	if (m_redis.hget(key, "psid", strProxyID) == true)
	{
		CT_DWORD dwProxyServerID = atoi(strProxyID.c_str());
		return CServerMgr::get_instance().FindProxyServer(dwProxyServerID);
	}
	return NULL;
}

CT_VOID CCustomerThread::SendUnReadMsgToClient(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID)
{
	/*
	//发送缓存中聊天消息
	auto itUnReadMsg = m_mapUserUnReadMsg.find(dwUserID);
	if (itUnReadMsg != m_mapUserUnReadMsg.end())
	{
		std::vector<MSG_S2C_Chat_To_User >& vecChat = itUnReadMsg->second;
		for (auto& it : vecChat)
		{
			SendMsgToUser(pSocket, dwUserID, SUB_SERVER2CLIENT_CHAT, &it, sizeof(MSG_S2C_Chat_To_User));
		}

		m_mapUserUnReadMsg.erase(itUnReadMsg);
	}

	//发送缓存中的举报反馈消息
	auto itUnReadReportMsg = m_mapUserReportUnReadMsg.find(dwUserID);
	if (itUnReadReportMsg != m_mapUserReportUnReadMsg.end())
	{
		std::vector<MSG_S2C_Chat_To_User >& vecChat = itUnReadMsg->second;
		for (auto& it : vecChat)
		{
			SendMsgToUser(pSocket, dwUserID, SUB_SERVER2CLIENT_CHAT_REPORT, &it, sizeof(MSG_S2C_Chat_To_User));
		}

		m_mapUserReportUnReadMsg.erase(itUnReadReportMsg);
	}
	 */
}

CT_DWORD CCustomerThread::GetWaitListUserByServerId(CT_DWORD dwServerID, CT_WORD wGameID, CT_WORD wKindID)
{
	acl::string key;
	acl::string attr, value;
	key.format("waitlist_%u", dwServerID);
	attr.format("%u:%u", wGameID, wKindID);

	m_redis.clear();
	if (m_redis.hget(key.c_str(), attr.c_str(), value) == false)
	{
		return 0;
	}

	CT_DWORD dwWaitTotal = atoi(value.c_str());

	return dwWaitTotal;
}

CT_VOID CCustomerThread::ReadLast10Msg(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_BYTE cbType)
{
    acl::db_pool* pool = GetChatdbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get chat db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get chat db handle fail.";
        return;
    }

    std::vector<MSG_S2C_ChatRecord_To_User> vecChatRecord;
    do
    {
        acl::query query;
        query.create_sql("select * from (select * from record.record_im_chat where userid = :userid and msgType = :msgType union all \n"
                         "select a.* from record.record_im_chat a inner join record.record_im_chat b on a.replyid = b.id \n"
                         "where b.userid = :userid and b.msgType = :msgType) x order by id desc LIMIT 10")
        .set_format("userid", "%u", dwUserID)
        .set_format("msgType", "%d", cbType);

        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "Selct record_im_chat fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            return;
        }

        MSG_S2C_ChatRecord_To_User chatRecord;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            chatRecord.dwMsgId = atoi((*row)["id"]);
            chatRecord.cbType = (CT_BYTE)atoi((*row)["type"]);
            chatRecord.cbIsRead = (CT_BYTE)atoi((*row)["isRead"]);
            std::string strMsg = (*row)["body"];
            _snprintf_info(chatRecord.szContent, sizeof(chatRecord.szContent), "%s", strMsg.c_str());
            CT_DWORD dwTime = (CT_DWORD)atoi((*row)["fixdate"]);
            _snprintf_info(chatRecord.szTime, sizeof(chatRecord.szTime), "%s", Utility::ChangeTimeToString(dwTime).c_str());
            vecChatRecord.push_back(chatRecord);
        }
    }while (0);

    db->free_result();
    pool->put(db);


    if (!vecChatRecord.empty())
    {
        CT_BYTE cbBuff[4096];
        CT_DWORD dwSendSize = 0;
        for (auto it = vecChatRecord.rbegin(); it != vecChatRecord.rend(); ++it)
        {
            memcpy(cbBuff+dwSendSize, &(*it), sizeof(MSG_S2C_ChatRecord_To_User));
            dwSendSize += sizeof(MSG_S2C_ChatRecord_To_User);
        }


		SendMsgToUser(pSocket, dwUserID, SUB_SERVER2CLIENT_CHAT_RECORD, cbBuff, dwSendSize, cbType);
    }
}


void CCustomerThread::PostMsgToCustomer(CT_DWORD dwUserID, CT_CHAR* pMsg, CT_CHAR* pIp, CT_BYTE cbType)
{
	acl::string serverAddr;
	serverAddr.format("%s:%d", CServerCfg::m_CustomerServerAddr, CServerCfg::m_CustomerServerPort);
	//serverAddr.format("%s:%d", "192.168.0.105", 94);
	acl::socket_stream conn;
	if (conn.open(serverAddr, 10, 10) == false)
	{
		LOG(WARNING) << "connect " << serverAddr.c_str() << "error :" << acl::last_serror();
		
		return;
	}
	
	
	char sendline[2048] = {0};
    char recvline[1024] = {0};
	char params[2048];
	_snprintf_info(params, sizeof(params), "{\"userid\":\"%u\",\"text\":\"%s\",\"ip\":\"%s\",\"msgType\":%d}", dwUserID, pMsg, pIp, (int)cbType);
	acl::string urlEncodeMsg(Utility::urlEncodeCPP(params).c_str());
    //acl::string urlEncodeMsgUpper = urlEncodeMsg.upper();
    
    CT_CHAR szBuffer[2028] = { 0 };
	m_encrypt.encrypt((CT_UCHAR*)urlEncodeMsg.c_str(), urlEncodeMsg.length(), (CT_UCHAR *)szBuffer);
    
    std::string HexMsg = toHexString((CT_UCHAR*)szBuffer,  urlEncodeMsg.length());
    _snprintf_info(params, sizeof(params), "key=%s", HexMsg.c_str());
	
	_snprintf_info(sendline, 2048,
				   "POST %s HTTP/1.1\r\n"
				   "Host: %s\r\n"
				   "Content-type: application/x-www-form-urlencoded\r\n"
				   "Content-length: %zu\r\n\r\n"
				   "%s", "/chat/receive", CServerCfg::m_CustomerServerAddr, strlen(params), params);
	LOG(INFO) << sendline;
	if (conn.write(sendline, strlen(sendline)) == (-1))
	{
		//printf("http pose msg fail.\n");
		LOG(INFO) << "http pose msg fail.";
	}
    ssize_t n;
    while ((n = conn.read(recvline, 1024 -1, false)) > 0) {
        recvline[n] = '\0';
        //printf("%s\n", Utility::Utf82Ansi(recvline).c_str());
        LOG(INFO) << Utility::Utf82Ansi(recvline).c_str();
        conn.close();
    }
    if (conn.alive())
    	conn.close();
}

std::string CCustomerThread::toHexString(const unsigned char* input, const int datasize)
{
    std::string output;
    char ch[3];
    for (int i = 0; i < datasize; ++i)
    {
    _snprintf_info(ch, 3, "%02x", input[i]);
    output += ch;
    }
    return output;
}

CT_VOID CCustomerThread::ReturnCommonToWeb(acl::aio_socket_stream* pSocket, std::string code, std::string strMessage)
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

CT_VOID CCustomerThread::InsertQueryRecord(CT_DWORD dwUserID, acl::aio_socket_stream* pSocket, CT_BYTE cbMsgType)
{
    std::shared_ptr<tagChatUser> chatUserPtr = m_QueryRecordThreadPtr->GetFreeChatUserQue();
    chatUserPtr->dwUserID = dwUserID;
    chatUserPtr->cbMsgType = cbMsgType;
    chatUserPtr->pSocket = pSocket;

    m_QueryRecordThreadPtr->InsertQueryChatRecord(chatUserPtr);
}
