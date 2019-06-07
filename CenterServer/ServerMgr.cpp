#include "stdafx.h"
#include "ServerMgr.h"
#include "sysdata.h"
#include "CTData.h"
#include "NetModule.h"
#include "DuoBaoMgr.h"

CServerMgr::CServerMgr()
{
	m_mapPRoomNeedGem.clear();
	m_mapPRoomUserCount.clear();
}

CServerMgr::~CServerMgr()
{

}

CT_BOOL CServerMgr::AddProxyServerInfo(acl::aio_socket_stream* pSocket, const CMD_ProxyServer_Info* pProxyInfo)
{
	if (m_mapProxyServer.find(pSocket) != m_mapProxyServer.end())
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "add proxy server failed, socket is repeat, remote ip: " << strIp; //pSocket->get_peer(true);
		}

		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	acl::aio_socket_stream* pFindSocket = FindProxyServer(pProxyInfo->dwServerID);
	if (pFindSocket != NULL)
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pFindSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "proxy server id : " << pProxyInfo->dwServerID << " is repeat?  original is: " << strIp; //pFindSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	CMD_ProxyServer_Info proxyServerInfo;
	memset(&proxyServerInfo, 0, sizeof(proxyServerInfo));
	memcpy(&proxyServerInfo, pProxyInfo, sizeof(proxyServerInfo));

	m_mapProxyServer.insert(std::make_pair(pSocket, proxyServerInfo));

	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "register one proxy server: " << strIp;//pSocket->get_peer(true);
	}
	return true;
}

acl::aio_socket_stream* CServerMgr::FindProxyServer(CT_DWORD dwServerID)
{
	for (auto& it : m_mapProxyServer)
	{
		if (it.second.dwServerID == dwServerID)
		{
			return (acl::aio_socket_stream*)(it.first);
		}
	}

	return NULL;
}

CMD_ProxyServer_Info* CServerMgr::FindProxyServerInfo(acl::aio_socket_stream* pSocket)
{
	auto it = m_mapProxyServer.find(pSocket);
	if (it != m_mapProxyServer.end())
	{
		return &it->second;
	}

	return NULL;
}

//获取当时的ps合集
const MapProxyServer* CServerMgr::GetProxyServerMap()
{
    return &m_mapProxyServer;
}

//添加游戏服务器
CT_BOOL CServerMgr::AddGameServerInfo(acl::aio_socket_stream* pSocket, const CMD_GameServer_Info* pGameInfo)
{
	if (m_mapGameServer.find(pSocket) != m_mapGameServer.end())
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "add game server failed, socket is repeat, remote ip: " << strIp; //pSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	acl::aio_socket_stream* pFindSocket = FindGameServer(pGameInfo->dwServerID);
	if (pFindSocket != NULL)
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pFindSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "game server id : " << pGameInfo->dwServerID << " is repeat?  original is: " << strIp;//pFindSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	CMD_GameServer_Info gameServerInfo;
	memset(&gameServerInfo, 0, sizeof(gameServerInfo));
	memcpy(&gameServerInfo, pGameInfo, sizeof(gameServerInfo));
	//gameServerInfo.dwServerID = pGameInfo->dwServerID;
	//gameServerInfo.wGameID = pGameInfo->wGameID;
	//gameServerInfo.wKindID = pGameInfo->wKindID;
	//gameServerInfo.wRoomKindID = pGameInfo->wRoomKindID;
	//gameServerInfo.wUserCount = pGameInfo->wUserCount;
	//gameServerInfo.cbPlayCount1 = pGameInfo->cbPlayCount1;
	//gameServerInfo.wCreateRoomGem1 = pGameInfo->wCreateRoomGem1;
	//gameServerInfo.wCreateRoomHuxi1 = pGameInfo->wCreateRoomHuxi1;
	//gameServerInfo.cbPlayCount2 = pGameInfo->cbPlayCount2;
	//gameServerInfo.wCreateRoomGem2 = pGameInfo->wCreateRoomGem2;
	//gameServerInfo.wCreateRoomHuxi2 = pGameInfo->wCreateRoomHuxi2;
	//gameServerInfo.dwFullCount = pGameInfo->dwFullCount;
	//gameServerInfo.dwOnlineCount = pGameInfo->dwOnlineCount;
	//_snprintf_info(gameServerInfo.szServerIP, sizeof(gameServerInfo.szServerIP), "%s", pGameInfo->szServerIP);
	//_snprintf_info(gameServerInfo.szServerName, sizeof(gameServerInfo.szServerName), "%s", pGameInfo->szServerName);
	gameServerInfo.cbState = 1;
	std::string strPlayCount = pGameInfo->szPRoomPlayCountCond;
	std::string strNeedGem = pGameInfo->szPRoomNeedGem;

	std::vector<std::string> vecPlayCount;
	Utility::stringSplit(strPlayCount, ",", vecPlayCount);

	std::vector<std::string> vecNeedGem;
	Utility::stringSplit(strNeedGem, ",", vecNeedGem);

	if (vecPlayCount.size() != vecNeedGem.size())
	{
		LOG(WARNING) << "need gem and playcount not equal!";
	}

	/*std::map<CT_WORD, CT_WORD> mapNeedGem;
	for (auto it = vecPlayCount.begin(); it != vecPlayCount.end(); ++it)
	{
		auto nIndexNeedGem = distance(vecPlayCount.begin(), it);
		mapNeedGem.insert(std::make_pair(atoi(it->c_str()), atoi(vecNeedGem[nIndexNeedGem].c_str())));
	}
	m_mapPRoomNeedGem.insert(std::make_pair(pGameInfo->dwServerID, mapNeedGem));*/

	acl::json proomJson(pGameInfo->szPRoomOtherCond);
	const acl::json_node* pNodeUserCount = proomJson.getFirstElementByTagName("usercount");
	if (pNodeUserCount)
	{
		std::string strUserCount = pNodeUserCount->get_string();
		std::vector<std::string> vecUserCount;
		Utility::stringSplit(strUserCount, ",", vecUserCount);
		std::set<CT_WORD> setUserCount;
		for (auto& it : vecUserCount)
		{
			CT_WORD wUserCount = atoi(it.c_str());
			setUserCount.insert(wUserCount);
		}

		m_mapPRoomUserCount.insert(std::make_pair(pGameInfo->dwServerID, setUserCount));
	}
	
	m_mapGameServer.insert(std::make_pair(pSocket, gameServerInfo));
	m_mapGameServer2.insert(std::make_pair(gameServerInfo.dwServerID, gameServerInfo));
	
	//把游戏服的消息广播给所有的proxy
	for (auto it : m_mapProxyServer)
	{
		//CMD_GameServer_Info gameServer;
		//gameServer.dwServerID = pGameInfo->dwServerID;
		//gameServer.wGameID = pGameInfo->wGameID;
		//gameServer.wKindID = pGameInfo->wKindID;
		//gameServer.dwOnlineCount = pGameInfo->dwOnlineCount;
		//gameServer.dwFullCount = pGameInfo->dwFullCount;
		//_snprintf_info(gameServer.szServerIP, sizeof(gameServer.szServerIP), "%s", pGameInfo->szServerIP);
		//_snprintf_info(gameServer.szServerName, sizeof(gameServer.szServerName), "%s", pGameInfo->szServerName);
		CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_SERVER_INSERT, &gameServerInfo, sizeof(gameServerInfo));
	}

	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "register one game server: " << strIp; //pSocket->get_peer(true);
	}
	return true;
}

//查找游戏服
acl::aio_socket_stream* CServerMgr::FindGameServer(CT_DWORD dwServerID)
{
	for (auto& it : m_mapGameServer)
	{
		if (it.second.dwServerID == dwServerID)
		{
			return (acl::aio_socket_stream*)(it.first);
		}
	}

	return NULL;
}

//查找游戏服
acl::aio_socket_stream* CServerMgr::FindGameServer(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID)
{
	for (auto& it : m_mapGameServer)
	{
		if (it.second.wGameID == wGameID && it.second.wKindID == wKindID && it.second.wRoomKindID == wRoomKindID)
		{
			return (acl::aio_socket_stream*)(it.first);
		}
	}

	return NULL;
}

//查找游戏服
CMD_GameServer_Info* CServerMgr::FindGameServerEx(CT_DWORD dwServerID)
{
	for (auto& it : m_mapGameServer)
	{
		if (it.second.dwServerID == dwServerID)
		{
			return &(it.second);
		}
	}

	return NULL;
}

CMD_GameServer_Info* CServerMgr::FindGameServer2(CT_DWORD dwServerID)
{
	auto it = m_mapGameServer2.find(dwServerID);
	if (it != m_mapGameServer2.end())
	{
		return &it->second;
	}
	
	return NULL;
}

//更新游戏服的人数
CT_VOID CServerMgr::UpdateGameServerUser(acl::aio_socket_stream* pSocket, const CMD_Update_GS_User* pGameInfo)
{
	if (pSocket == NULL || pGameInfo == NULL)
	{
		return;
	}

	auto it = m_mapGameServer.find(pSocket);
	if (it == m_mapGameServer.end())
	{
		LOG(ERROR) << "update game server user count failed, not found game server ";
		return;
	}

	CMD_GameServer_Info* pGameServerInfo = &it->second;
	if (pGameServerInfo->dwServerID != pGameInfo->dwServerID
		|| pGameServerInfo->wGameID != pGameInfo->wGameID
		|| pGameServerInfo->wKindID != pGameInfo->wKindID
		|| pGameServerInfo->wRoomKindID != pGameInfo->wRoomKindID)
	{
		LOG(ERROR) << "update game server user count failed, server param error!";
		return;
	}

	//更新真实人数
	pGameServerInfo->dwOnlineCount = pGameInfo->wRealUserCount;

	//更新人数给
	CMD_GameServer_OnlineCount GameServerOnlineCount;
	GameServerOnlineCount.dwServerID = pGameInfo->dwServerID;
	GameServerOnlineCount.dwOnlineCount = pGameInfo->wUserCount;

	//把游戏服的人数消息广播给所有的proxyserver
	for (auto& it : m_mapProxyServer)
	{	
		acl::aio_socket_stream* pProxySocket = (acl::aio_socket_stream*)it.first;
		if (pProxySocket != NULL)
		{
			CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_SYN_USER_ONLINE_COUNT, &GameServerOnlineCount, sizeof(GameServerOnlineCount));
		}
	}
}

//计算某个游戏类型的在线人数
CT_VOID CServerMgr::CalGameServerUserCount(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID, CT_DWORD& dwKindUserCount, CT_DWORD& dwRoomKindUserCount)
{
	//CT_DWORD dwUserCount = 0;
	for (auto& it : m_mapGameServer)
	{
		if (it.second.wGameID == wGameID && it.second.wKindID == wKindID)
		{
			dwKindUserCount += it.second.dwOnlineCount;
			
			if (it.second.wRoomKindID == wRoomKindID)
				dwRoomKindUserCount += it.second.dwOnlineCount;
		}
	}
}

std::string CServerMgr::GetGameServerName(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID)
{
	for (auto& it : m_mapGameServer)
	{
		if (it.second.wGameID == wGameID && it.second.wKindID == wKindID && it.second.wRoomKindID == wRoomKindID)
		{
			return std::string(it.second.szServerName);
		}
	}
	
	return  std::string("游戏房间");
}

//添加登录服务器
CT_VOID CServerMgr::SetGameServerState(CT_DWORD dwServerID, CT_BYTE cbState)
{
	if (dwServerID == 0)
	{
		for (auto& it : m_mapGameServer)
		{
			it.second.cbState = cbState;
			BroadcastGameServerStateToProxy(it.second.dwServerID, cbState);
			SendGameServerStateToGame(it.second.dwServerID, cbState);
		}

		CYYDuoBaoMgr::get_instance().SetSystemState(cbState);
		return;
	}
	else
	{
		for (auto& it : m_mapGameServer)
		{
			if (it.second.dwServerID == dwServerID)
			{
				it.second.cbState = cbState;
				BroadcastGameServerStateToProxy(it.second.dwServerID, cbState);
				SendGameServerStateToGame(it.second.dwServerID, cbState);
				return;
			}
		}
	}


	LOG(ERROR) << "set game server state failed, not found game server ";
}

//解散所有游戏的私人房间
CT_VOID CServerMgr::DismissServerPRoom(CT_DWORD dwServerID, CT_DWORD dwRoomNum)
{
	if (dwServerID == 0)
	{
		for (auto& it : m_mapGameServer)
		{
			MSG_C2GS_DismissPRoom dismissPRoom;
			dismissPRoom.dwServerID = it.second.dwServerID;
			dismissPRoom.dwRoomNum = dwRoomNum;
			CNetModule::getSingleton().Send(it.first, MSG_GCS_MAIN, SUB_C2GS_DISMISS_PROOM, &dismissPRoom, sizeof(dismissPRoom));
		}
		return;
	}
	else
	{
		for (auto& it : m_mapGameServer)
		{
			if (it.second.dwServerID == dwServerID)
			{
				MSG_C2GS_DismissPRoom dismissPRoom;
				dismissPRoom.dwServerID = it.second.dwServerID;
				dismissPRoom.dwRoomNum = dwRoomNum;
				CNetModule::getSingleton().Send(it.first, MSG_GCS_MAIN, SUB_C2GS_DISMISS_PROOM, &dismissPRoom, sizeof(dismissPRoom));
				return;
			}
		}
	}
	LOG(ERROR) << "dismiss game server proom failed, not found game server ";
}

//获取当时的gs合集
const MapGameServer* CServerMgr::GetGameServerMap()
{
	return &m_mapGameServer;
}

CT_VOID CServerMgr::SendMsgToAllGameServer(CT_WORD mainId, CT_WORD subId, CT_VOID* pData, CT_DWORD dwLen)
{
	for (auto& it : m_mapGameServer)
	{
		CNetModule::getSingleton().Send((acl::aio_socket_stream*)it.first, mainId, subId, pData, dwLen);
	}
}

//增加平台游戏人数
/*CT_VOID CServerMgr::IncreaseGameUserCount(CT_BYTE cbPlatformId, CMD_Update_GS_User* pUpdateUserCount)
{
	CT_DWORD dwGameIndex = (CT_DWORD)(pUpdateUserCount->wGameID*100+pUpdateUserCount->wKindID*10+pUpdateUserCount->wRoomKindID);
	auto it = m_mapGameUserCount.find(cbPlatformId);
	if (it != m_mapGameUserCount.end())
	{
		std::map<CT_DWORD, CT_DWORD>& mapUserCount = it->second;
		mapUserCount[dwGameIndex] += 1;
	}
	else
	{
		std::map<CT_DWORD, CT_DWORD> mapUserCount;
		mapUserCount.insert(std::make_pair(dwGameIndex, 1));
		m_mapGameUserCount.insert(std::make_pair(cbPlatformId, mapUserCount));
	}
}

//减少平台游戏人数
CT_VOID CServerMgr::DecreaseGameUserCount(CT_BYTE cbPlatformId, CMD_Update_GS_User* pUpdateUserCount)
{
	CT_DWORD dwGameIndex = (CT_DWORD)(pUpdateUserCount->wGameID*100+pUpdateUserCount->wKindID*10+pUpdateUserCount->wRoomKindID);
	auto it = m_mapGameUserCount.find(cbPlatformId);
	if (it != m_mapGameUserCount.end())
	{
		std::map<CT_DWORD, CT_DWORD>& mapUserCount = it->second;
		auto itUserCount = mapUserCount.find(dwGameIndex);
		if (itUserCount != mapUserCount.end() && itUserCount->second > 0)
		{
			mapUserCount[dwGameIndex] -= 1;
		}
	}
}

//获取某个平台的游戏人数
CT_DWORD CServerMgr::GetGameUserCount(CT_BYTE cbPlatformId, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID)
{
	CT_DWORD dwGameIndex = (CT_DWORD)(wGameID*100+wKindID*10+wRoomKindID);
	auto it = m_mapGameUserCount.find(cbPlatformId);
	if (it != m_mapGameUserCount.end())
	{
		std::map<CT_DWORD, CT_DWORD>& mapUserCount = it->second;
		auto itUserCount = mapUserCount.find(dwGameIndex);
		return itUserCount->second;
	}

	return 0;
}*/

//添加登录服务器
CT_BOOL CServerMgr::AddLoginServerInfo(acl::aio_socket_stream* pSocket, const CMD_LoginServer_Info* pLoginInfo)
{
	if (m_mapLoginServer.find(pSocket) != m_mapLoginServer.end())
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "add login server failed, socket is repeat, remote ip: " << strIp; //pSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	acl::aio_socket_stream* pFindSocket = FindLoginServerInfo(pLoginInfo->dwServerID);
	if (pFindSocket != NULL)
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pFindSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "login server id : " << pLoginInfo->dwServerID << " is repeat?  original is: " << strIp; //pFindSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	CMD_LoginServer_Info loginServerInfo;
	memset(&loginServerInfo, 0, sizeof(loginServerInfo));
	memcpy(&loginServerInfo, pLoginInfo, sizeof(loginServerInfo));

	m_mapLoginServer.insert(std::make_pair(pSocket, loginServerInfo));

	//把登录服的消息广播给所有的proxy
	for (auto it : m_mapProxyServer)
	{
		CMD_LoginServer_Info loginServer;
		loginServer.dwServerID = pLoginInfo->dwServerID;
		loginServer.cbMainServer = pLoginInfo->cbMainServer;
		_snprintf_info(loginServer.szServerIP, sizeof(loginServer.szServerIP), "%s", pLoginInfo->szServerIP);
		_snprintf_info(loginServer.szServerPubIP, sizeof(loginServer.szServerPubIP), "%s", pLoginInfo->szServerPubIP);
		_snprintf_info(loginServer.szServerName, sizeof(loginServer.szServerName), "%s", pLoginInfo->szServerName);
		CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_LOGIN_INSERT, &loginServer, sizeof(loginServer));
	}

	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "register one login server: " << strIp; //pSocket->get_peer(true);
	}
	return true;
}

//查找游戏服
acl::aio_socket_stream* CServerMgr::FindLoginServerInfo(CT_DWORD dwServerID)
{
	for (auto& it : m_mapLoginServer)
	{
		if (it.second.dwServerID == dwServerID)
		{
			return (acl::aio_socket_stream*)it.first;
		}
	}

	return NULL;
}

CT_VOID CServerMgr::SendMsgToAllLoginServer(CT_WORD mainId, CT_WORD subId, const CT_VOID* pData, CT_DWORD dwLen)
{
	for (auto& it : m_mapLoginServer)
	{
		CNetModule::getSingleton().Send((acl::aio_socket_stream*)it.first, mainId, subId, pData, dwLen);
	}
}

CT_BOOL CServerMgr::AddDipServerInfo(acl::aio_socket_stream* pSocket, const CMD_DipServer_Info* pDipInfo)
{
	if (m_mapDipServer.find(pSocket) != m_mapDipServer.end())
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "add dip server failed, socket is repeat, remote ip: " << strIp; //pSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	acl::aio_socket_stream* pFindSocket = FindDipServer(pDipInfo->dwServerID);
	if (pFindSocket != NULL)
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pFindSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "dip server id : " << pDipInfo->dwServerID << " is repeat?  original is: " << strIp;//pFindSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	CMD_DipServer_Info dipServerInfo;
	memset(&dipServerInfo, 0, sizeof(dipServerInfo));
	memcpy(&dipServerInfo, pDipInfo, sizeof(dipServerInfo));

	m_mapDipServer.insert(std::make_pair(pSocket, dipServerInfo));

	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "register one dip server: " << strIp; //pSocket->get_peer(true);
	}

	return true;
}

//查找DIP服
acl::aio_socket_stream* CServerMgr::FindDipServer(CT_DWORD dwServerID)
{
	for (auto& it : m_mapDipServer)
	{
		if (it.second.dwServerID == dwServerID)
		{
			return (acl::aio_socket_stream*)it.first;
		}
	}

	return NULL;
}

//添加客服服务器
CT_BOOL CServerMgr::AddCustomerServerInfo(acl::aio_socket_stream* pSocket, const CMD_CustomerServer_Info* pCustomer)
{
	if (m_mapCustomerServer.find(pSocket) != m_mapCustomerServer.end())
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "add customer server failed, socket is repeat, remote ip: " << strIp; //pSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	acl::aio_socket_stream* pFindSocket = FindCustomerServerInfo(pCustomer->dwServerID);
	if (pFindSocket != NULL)
	{
		std::string strIp = CNetModule::getSingleton().GetRemoteIp(pFindSocket);
		if (!strIp.empty())
		{
			LOG(ERROR) << "customer server id : " << pCustomer->dwServerID << " is repeat?  original is: " << strIp; //pFindSocket->get_peer(true);
		}
		CNetModule::getSingleton().CloseSocket(pSocket);
		return false;
	}

	CMD_CustomerServer_Info customerServerInfo;
	memset(&customerServerInfo, 0, sizeof(customerServerInfo));
	memcpy(&customerServerInfo, pCustomer, sizeof(customerServerInfo));

	m_mapCustomerServer.insert(std::make_pair(pSocket, customerServerInfo));

	//把登录服的消息广播给所有的proxy
	for (auto it : m_mapProxyServer)
	{
		CMD_CustomerServer_Info customerServer;
		customerServer.dwServerID = pCustomer->dwServerID;
		_snprintf_info(customerServer.szServerIP, sizeof(customerServer.szServerIP), "%s", pCustomer->szServerIP);
		_snprintf_info(customerServer.szServerPubIP, sizeof(customerServer.szServerPubIP), "%s", pCustomer->szServerPubIP);
		_snprintf_info(customerServer.szServerName, sizeof(customerServer.szServerName), "%s", pCustomer->szServerName);
		CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_CUSTOMER_INSERT, &customerServer, sizeof(customerServer));
	}

	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "register one customer server: " << strIp; //pSocket->get_peer(true);
	}
	return true;
}

//查找登录服
acl::aio_socket_stream* CServerMgr::FindCustomerServerInfo(CT_DWORD dwServerID)
{
	for (auto& it : m_mapCustomerServer)
	{
		if (it.second.dwServerID == dwServerID)
		{
			return (acl::aio_socket_stream*)it.first;
		}
	}

	return NULL;
}

CMD_GameServer_Info* CServerMgr::FindSuitTableGameServer(CT_WORD wGameID, CT_WORD wKindID/*, CT_WORD wUserCount*/)
{
	CMD_GameServer_Info* pGameServer = NULL;
	CMD_GameServer_Info* pGameServerRet = NULL;
	for (auto& it : m_mapGameServer)
	{
		pGameServer = &it.second;
		if (pGameServer->wGameID != wGameID || pGameServer->wKindID != wKindID || pGameServer->wRoomKindID != 0 || pGameServer->cbState == 0)
		{
			continue;
		}

		if (pGameServer->dwOnlineCount < (pGameServer->dwFullCount - 50))
		{
			if (pGameServerRet == NULL)
				pGameServerRet = pGameServer;
			else if (pGameServer->dwOnlineCount > pGameServerRet->dwOnlineCount)
				pGameServerRet = pGameServer;
		}
	}
	return pGameServerRet;
}

CMD_GameServer_Info* CServerMgr::FindSuitTableGameServer(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID)
{
    CMD_GameServer_Info* pGameServer = NULL;
    for (auto& it : m_mapGameServer)
    {
        pGameServer = &it.second;
        if (pGameServer->wGameID == wGameID && pGameServer->wKindID == wKindID && pGameServer->wRoomKindID == wRoomKindID)
        {
            if (pGameServer->cbState == 0)
                continue;

            if (pGameServer->dwOnlineCount < (pGameServer->dwFullCount - 100))
            {
                return pGameServer;
            }
        }
    }
    return NULL;
}

CT_BOOL CServerMgr::CheckCreatePRoomParam(CT_DWORD dwServerID, MSG_PS_CreatePrivateRoom* pPrivateRoom)
{
	 if (pPrivateRoom == NULL)
	 {
		 return false;
	 }

	auto itMapNeedGem = m_mapPRoomNeedGem.find(dwServerID);
	if (itMapNeedGem == m_mapPRoomNeedGem.end())
	{
		return false;
	}

	auto itNeedGem = itMapNeedGem->second.find(pPrivateRoom->cbPlayCount);
	if (itNeedGem == itMapNeedGem->second.end())
	{
		return false;
	}

	auto itMapUserCount = m_mapPRoomUserCount.find(dwServerID);
	if (itMapUserCount == m_mapPRoomUserCount.end())
	{
		return false;
	}

	auto itUserCount = itMapUserCount->second.find(pPrivateRoom->wUserCount);
	if (itUserCount == itMapUserCount->second.end())
	{
		return false;
	}

	return true;
}


CT_WORD CServerMgr::GetCreatePRoomNeedGem(CT_DWORD dwServerID, CT_WORD wPlayCount)
{
	auto it = m_mapPRoomNeedGem.find(dwServerID);
	if (it != m_mapPRoomNeedGem.end())
	{
		auto itNeedGem = it->second.find(wPlayCount);

		if (itNeedGem != it->second.end())
		{
			return itNeedGem->second;
		}
	}

	return 0;
}

//发送游戏服务器列表
CT_BOOL CServerMgr::SendGameServerList(acl::aio_socket_stream* pSocket, CT_WORD mainId, CT_WORD subId, CT_WORD finishSubId)
{
	CT_WORD wPacketSize = 0;
	CT_BYTE cbBuffer[SOCKET_TCP_PACKET];

	for (auto& it : m_mapGameServer)
	{
		CMD_GameServer_Info* pGameServer = (CMD_GameServer_Info*)(cbBuffer + wPacketSize);
		memcpy(pGameServer, &it.second, sizeof(CMD_GameServer_Info));
		wPacketSize += sizeof(CMD_GameServer_Info);

		if (wPacketSize+sizeof(CMD_GameServer_Info) > sizeof(cbBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, mainId, subId, cbBuffer, wPacketSize);
			wPacketSize = 0;
		}
	}

	if (wPacketSize > 0)
	{
		CNetModule::getSingleton().Send(pSocket, mainId, subId, cbBuffer, wPacketSize);
	}

	CNetModule::getSingleton().Send(pSocket, mainId, finishSubId);

	return true;
}

CT_BOOL CServerMgr::SendLoginServerList(acl::aio_socket_stream* pSocket)
{
	CT_WORD wPacketSize = 0;
	CT_BYTE cbBuffer[SOCKET_TCP_PACKET];

	for (auto& it : m_mapLoginServer)
	{
		CMD_LoginServer_Info* pLoginServer = (CMD_LoginServer_Info*)(cbBuffer + wPacketSize);
		memcpy(pLoginServer, &it.second, sizeof(CMD_LoginServer_Info));
		wPacketSize += sizeof(CMD_LoginServer_Info);

		if (wPacketSize + sizeof(CMD_LoginServer_Info) > sizeof(cbBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_LOGIN_INSERT, cbBuffer, wPacketSize);
			wPacketSize = 0;
		}
	}

	if (wPacketSize > 0)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_LOGIN_INSERT, cbBuffer, wPacketSize);
	}

	CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_LOGIN_FINISH);

	return true;
}

CT_BOOL CServerMgr::SendCustomerServerList(acl::aio_socket_stream* pSocket)
{
	CT_WORD wPacketSize = 0;
	CT_BYTE cbBuffer[SOCKET_TCP_PACKET];

	for (auto& it : m_mapCustomerServer)
	{
		CMD_CustomerServer_Info* pLoginServer = (CMD_CustomerServer_Info*)(cbBuffer + wPacketSize);
		memcpy(pLoginServer, &it.second, sizeof(CMD_CustomerServer_Info));
		wPacketSize += sizeof(CMD_CustomerServer_Info);

		if (wPacketSize + sizeof(CMD_CustomerServer_Info) > sizeof(cbBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_CUSTOMER_INSERT, cbBuffer, wPacketSize);
			wPacketSize = 0;
		}
	}

	if (wPacketSize > 0)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_CUSTOMER_INSERT, cbBuffer, wPacketSize);
	}

	CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_CUSTOMER_FINISH);

	return true;
}

CT_VOID CServerMgr::BroadcastGameServerStateToProxy(CT_DWORD dwServerID, CT_BYTE cbState)
{
	MSG_CS2P_Update_GS_State updateGameServerState;
	updateGameServerState.dwServerID = dwServerID;
	updateGameServerState.cbState = cbState;

	//把游戏服的人数消息广播给所有的proxyserver
	for (auto& it : m_mapProxyServer)
	{
		acl::aio_socket_stream* pProxySocket = (acl::aio_socket_stream*)it.first;
		if (pProxySocket != NULL)
		{
			CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_UPDATE_GS_STATE, &updateGameServerState, sizeof(updateGameServerState));
		}
	}
}

//发送游戏服务器的状态给gameserver
CT_VOID CServerMgr::SendGameServerStateToGame(CT_DWORD dwServerID, CT_BYTE cbState)
{
	acl::aio_socket_stream* pGsSocket = FindGameServer(dwServerID);
	if (pGsSocket)
	{
		MSG_C2GS_Update_GameServer_State GsState;
		GsState.cbState = cbState;
		CNetModule::getSingleton().Send(pGsSocket, MSG_GCS_MAIN, SUB_C2GS_UPDATE_GS_STATE, &GsState, sizeof(GsState));
	}
}

CT_VOID CServerMgr::DeleteServer(acl::aio_socket_stream* pSocket)
{
	auto itGame = m_mapGameServer.find(pSocket);
	if (itGame != m_mapGameServer.end())
	{
		CMD_GameServer_Info gameServer;
		memcpy(&gameServer, &itGame->second, sizeof(CMD_GameServer_Info));
		for (auto it : m_mapProxyServer)
		{
			CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_SERVER_REMOVE, &gameServer, sizeof(gameServer));
		}

		auto itGem = m_mapPRoomNeedGem.find(itGame->second.dwServerID);
		if (itGem != m_mapPRoomNeedGem.end())
		{
			m_mapPRoomNeedGem.erase(itGem);
		}
		
		auto itUserCount = m_mapPRoomUserCount.find(itGame->second.dwServerID);
		if (itUserCount != m_mapPRoomUserCount.end())
		{
			m_mapPRoomUserCount.erase(itUserCount);
		}

		LOG(WARNING) << "remove one game server, id:" << itGame->second.dwServerID;
		m_mapGameServer2.erase(itGame->second.dwServerID);
		m_mapGameServer.erase(itGame);
		return;
	}

	auto itLogin = m_mapLoginServer.find(pSocket);
	if (itLogin != m_mapLoginServer.end())
	{
		CMD_LoginServer_Info loginServer;
		memcpy(&loginServer, &itLogin->second, sizeof(CMD_LoginServer_Info));
		for (auto it : m_mapProxyServer)
		{
			CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_LOGIN_REMOVE, &loginServer, sizeof(loginServer));
		}

		LOG(WARNING) << "remove one login server, id:" << itLogin->second.dwServerID;
		m_mapLoginServer.erase(itLogin);
		return;
	}

	auto itCustomer = m_mapCustomerServer.find(pSocket);
	if (itCustomer != m_mapCustomerServer.end())
	{
		CMD_CustomerServer_Info customerServer;
		memcpy(&customerServer, &itCustomer->second, sizeof(CMD_CustomerServer_Info));
		for (auto it : m_mapProxyServer)
		{
			CNetModule::getSingleton().Send(it.first, MSG_PCS_MAIN, SUB_CS2P_CUSTOMER_REMOVE, &customerServer, sizeof(customerServer));
		}

		LOG(WARNING) << "remove one customer server, id:" << itCustomer->second.dwServerID;
		m_mapCustomerServer.erase(itCustomer);
		return;
	}

	auto itProxy = m_mapProxyServer.find(pSocket);
	if (itProxy != m_mapProxyServer.end())
	{
		LOG(WARNING) << "remove one proxy server, id:" << itProxy->second.dwServerID;
		m_mapProxyServer.erase(itProxy);
		return;
	}

	auto itDip = m_mapDipServer.find(pSocket);
	if (itDip != m_mapDipServer.end())
	{
		LOG(WARNING) << "remove one dip server, id:" << itDip->second.dwServerID;
		m_mapDipServer.erase(itDip);
		return;
	}
}