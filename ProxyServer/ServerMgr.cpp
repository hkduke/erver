#include "stdafx.h"
#include "ServerMgr.h"
#include "ServerCfg.h"
#include "NetModule.h"
#include <algorithm>

CT_VOID CServerMgr::AddGameServerList(const CMD_GameServer_Info* pGameServer)
{
	if (pGameServer == NULL)
	{
		return;
	}

	auto it = m_mapGameServer.find(pGameServer->dwServerID);
	if (it != m_mapGameServer.end())
	{
		LOG(WARNING) << "repeated add game server, game server id:" << pGameServer->dwServerID;
		return;
	}

	stGameServerInfo GSInfo;
	//GSInfo.dwServerID = pGameServer->dwServerID;
	//GSInfo.wGameID = pGameServer->wGameID;
	//GSInfo.wKindID = pGameServer->wKindID;
	//GSInfo.dwOnlineCount = pGameServer->dwOnlineCount;
	//GSInfo.dwFullCount = pGameServer->dwFullCount;
	//_snprintf_info(GSInfo.szServerIP, sizeof(GSInfo.szServerIP), "%s", pGameServer->szServerIP);
	//_snprintf_info(GSInfo.szServerName, sizeof(GSInfo.szServerName), "%s", pGameServer->szServerName);
	memcpy(&GSInfo, pGameServer, sizeof(CMD_GameServer_Info));

	stConnectParam connParam;
	connParam.conType = CONNECT_GAME;
	connParam.dwServerID = pGameServer->dwServerID;
	if (CServerCfg::m_nUsePublicAddress == 0)
		GSInfo.pConnector = CNetModule::getSingleton().AddConnector(pGameServer->szServerIP, false, connParam);
	else
		GSInfo.pConnector = CNetModule::getSingleton().AddConnector(pGameServer->szPublicIP, false, connParam);
	

	LOG(WARNING) << "add game server, game server id:" << pGameServer->dwServerID;
	m_mapGameServer.insert(std::make_pair(GSInfo.dwServerID, GSInfo));
	
	std::set<CT_WORD>& setKind = m_mapGameKind[GSInfo.wGameID];
	if (setKind.find(GSInfo.wKindID) != setKind.end())
	{
		setKind.insert(GSInfo.wKindID);
	}
}

CT_VOID CServerMgr::AddLoginServerList(const CMD_LoginServer_Info* pLoginServer)
{
	if (pLoginServer == NULL)
	{
		return;
	}

	auto it = m_mapLoginServer.find(pLoginServer->dwServerID);
	if (it != m_mapLoginServer.end())
	{
		LOG(WARNING) << "repeated add login server, login server id:" << pLoginServer->dwServerID;
		return;
	}

	stLoginServerInfo LSInfo;
	LSInfo.dwServerID = pLoginServer->dwServerID;
	LSInfo.cbMainServer = pLoginServer->cbMainServer;
	_snprintf_info(LSInfo.szServerIP, sizeof(LSInfo.szServerIP), "%s", pLoginServer->szServerIP);
	_snprintf_info(LSInfo.szServerPubIP, sizeof(LSInfo.szServerPubIP), "%s", pLoginServer->szServerPubIP);
	_snprintf_info(LSInfo.szServerName, sizeof(LSInfo.szServerName), "%s", pLoginServer->szServerName);

	stConnectParam connParam;
	connParam.conType = CONNECT_LOGIN;
	connParam.dwServerID = pLoginServer->dwServerID;
	if (CServerCfg::m_nUsePublicAddress == 0)
		LSInfo.pConnector = CNetModule::getSingleton().AddConnector(LSInfo.szServerIP, false, connParam);
	else
		LSInfo.pConnector = CNetModule::getSingleton().AddConnector(LSInfo.szServerPubIP, false, connParam);

	LOG(WARNING) << "add login server, login server id:" << pLoginServer->dwServerID;
	m_mapLoginServer.insert(std::make_pair(LSInfo.dwServerID, LSInfo));

	//如果是主登陆服
	if (pLoginServer->cbMainServer)
	{
		m_vecMainLoginServer.push_back(LSInfo.dwServerID);
	}
	else
	{
		m_vecLoginServer.push_back(LSInfo.dwServerID);
	}
}

CT_VOID CServerMgr::AddCustomerServerList(const CMD_CustomerServer_Info* pCustomerServer)
{
	if (pCustomerServer == NULL)
	{
		return;
	}

	auto it = m_mapCustomerServer.find(pCustomerServer->dwServerID);
	if (it != m_mapCustomerServer.end())
	{
		LOG(WARNING) << "repeated add customer server, customer server id:" << pCustomerServer->dwServerID;
		return;
	}

	stCustomerServerInfo customerInfo;
	customerInfo.dwServerID = pCustomerServer->dwServerID;
	_snprintf_info(customerInfo.szServerIP, sizeof(customerInfo.szServerIP), "%s", pCustomerServer->szServerIP);
	_snprintf_info(customerInfo.szServerPubIP, sizeof(customerInfo.szServerPubIP), "%s", pCustomerServer->szServerPubIP);
	_snprintf_info(customerInfo.szServerName, sizeof(customerInfo.szServerName), "%s", pCustomerServer->szServerName);

	stConnectParam connParam;
	connParam.conType = CONNECT_CUSTOMER;
	connParam.dwServerID = pCustomerServer->dwServerID;
	customerInfo.pConnector = CNetModule::getSingleton().AddConnector(customerInfo.szServerPubIP, true, connParam);

	LOG(WARNING) << "add customer server, customer server id:" << pCustomerServer->dwServerID;
	m_mapCustomerServer.insert(std::make_pair(customerInfo.dwServerID, customerInfo));
}

CT_VOID CServerMgr::DeleteGameServer(const CMD_GameServer_Info* pGameServer)
{
	if (pGameServer == NULL)
	{
		return;
	}

	auto it = m_mapGameServer.find(pGameServer->dwServerID);
	if (it != m_mapGameServer.end())
	{
		//如果连接还是正常则返回?
		stGameServerInfo& gameInfo = it->second;
		//if (gameInfo.pConnector->IsRunning())
		//{
		//	return;
		//}
		if (gameInfo.pConnector->IsRunning())
		{
			LOG(WARNING) << "remove game server, but the link is alive, game server id:"<< pGameServer->dwServerID;
			CNetModule::getSingleton().CloseSocket(gameInfo.pConnector->GetSocket());
		}
		//这里不需要做delete netconnector操作,

		LOG(WARNING) << "remove one game server, game server id:" << pGameServer->dwServerID;
		m_mapGameServer.erase(it);
		return;
	}
}

CT_VOID CServerMgr::DeleteLoginServer(const CMD_LoginServer_Info* pLoginServer)
{
	if (pLoginServer == NULL)
	{
		return;
	}

	auto it = m_mapLoginServer.find(pLoginServer->dwServerID);
	if (it != m_mapLoginServer.end())
	{
		//如果连接还是正常则返回?
		stLoginServerInfo& gameInfo = it->second;
		//if (gameInfo.pConnector->IsRunning())
		//{
		//	return;
		//}
		if (gameInfo.pConnector->IsRunning())
		{
			LOG(WARNING) << "remove login server, but the link is alive, login server id:" << pLoginServer->dwServerID;
			CNetModule::getSingleton().CloseSocket(gameInfo.pConnector->GetSocket());
		}

		LOG(WARNING) << "remove one login server, login server id:" << pLoginServer->dwServerID;
		m_mapLoginServer.erase(it);

		VecLoginServerID::iterator itVec = std::find(m_vecLoginServer.begin(), m_vecLoginServer.end(), pLoginServer->dwServerID);
		if (itVec != m_vecLoginServer.end())
		{
			m_vecLoginServer.erase(itVec);
			m_SelLoginIndex = 0;
		}

		itVec = std::find(m_vecMainLoginServer.begin(), m_vecMainLoginServer.end(), pLoginServer->dwServerID);
		if (itVec != m_vecMainLoginServer.end())
		{
			m_vecMainLoginServer.erase(itVec);
			m_SelMainLoginIndex = 0;
		}

		return;
	}
}

CT_VOID CServerMgr::DeleteCustomerServer(const CMD_CustomerServer_Info* pCustomerServer)
{
	if (pCustomerServer == NULL)
	{
		return;
	}

	auto it = m_mapCustomerServer.find(pCustomerServer->dwServerID);
	if (it != m_mapCustomerServer.end())
	{
		//如果连接还是正常则返回?
		stCustomerServerInfo& gameInfo = it->second;
		//if (gameInfo.pConnector->IsRunning())
		//{
		//	return;
		//}
		if (gameInfo.pConnector->IsRunning())
		{
			LOG(WARNING) << "remove customer server, but the link is alive, game server id:" << pCustomerServer->dwServerID;
			CNetModule::getSingleton().CloseSocket(gameInfo.pConnector->GetSocket());
		}
		//这里不需要做delete netconnector操作,

		LOG(WARNING) << "remove one customer server, customer server id:" << pCustomerServer->dwServerID;
		m_mapCustomerServer.erase(it);
		return;
	}
}

acl::aio_socket_stream* CServerMgr::SelectOneLoginServer()
{
	if (m_mapLoginServer.empty())
	{
		return NULL;
	}

	if (!m_vecLoginServer.empty())
    {
        if (m_SelLoginIndex >= m_vecLoginServer.size())
            m_SelLoginIndex = 0;

        CT_DWORD dwServerID = m_vecLoginServer[m_SelLoginIndex++];
        auto it = m_mapLoginServer.find(dwServerID);
        if (it != m_mapLoginServer.end())
        {
            CNetConnector *pConnector = it->second.pConnector;
            return pConnector->GetSocket();
        }
    }

	return SelectMainLoginServer();
}

acl::aio_socket_stream* CServerMgr::SelectMainLoginServer()
{
	if (m_mapLoginServer.empty() || m_vecMainLoginServer.empty())
	{
		return NULL;
	}

	//直接取第一个登录服
	if (m_SelMainLoginIndex >= m_vecMainLoginServer.size())
		m_SelMainLoginIndex = 0;

	CT_DWORD dwServerID = m_vecMainLoginServer[m_SelMainLoginIndex++];
	auto it = m_mapLoginServer.find(dwServerID);
	if (it != m_mapLoginServer.end())
	{
		CNetConnector *pConnector = it->second.pConnector;
		return pConnector->GetSocket();
	}

	return NULL;
}

acl::aio_socket_stream* CServerMgr::SelectOneCustomerServer()
{
	for (auto&it : m_mapCustomerServer)
	{
		CNetConnector *pConnector = it.second.pConnector;
		if (pConnector)
		{
			return pConnector->GetSocket();
		}
	}

	return NULL;
}

CT_BOOL CServerMgr::IsExistServer(CT_DWORD dwServerID)
{
	auto itLogin = m_mapLoginServer.find(dwServerID);
	if (itLogin != m_mapLoginServer.end())
	{
		return true;
	}

	auto itGame = m_mapGameServer.find(dwServerID);
	if (itGame != m_mapGameServer.end())
	{
		return true;
	}

	return false;
}

CT_BOOL CServerMgr::IsExistCustomerServer(CT_DWORD dwServerID)
{
	auto itCustomer = m_mapCustomerServer.find(dwServerID);
	if (itCustomer != m_mapCustomerServer.end())
	{
		return true;
	}

	return false;
}

CT_VOID CServerMgr::CheckDisconnectServer(CT_DWORD dwServerID)
{
	bool bReconnect = false;
	/*for (auto it : m_mapLoginServer)
	{
		if (it.second.pConnector->IsClose())
		{
			CNetModule::getSingleton().InsertReconnector(it.second.pConnector);
			bReconnect = true;
		}
	}*/

	auto itLogin = m_mapLoginServer.find(dwServerID);
	if (itLogin != m_mapLoginServer.end())
	{
		if (itLogin->second.pConnector->IsClose())
		{
			CNetModule::getSingleton().InsertReconnector(itLogin->second.pConnector);
			bReconnect = true;
		}
	}

	/*for (auto it : m_mapGameServer)
	{
		if (it.second.pConnector->IsClose())
		{
			CNetModule::getSingleton().InsertReconnector(it.second.pConnector);
			bReconnect = true;
		}
	}*/

	auto itGame = m_mapGameServer.find(dwServerID);
	if (itGame != m_mapGameServer.end())
	{
		if (itGame->second.pConnector->IsClose())
		{
			CNetModule::getSingleton().InsertReconnector(itGame->second.pConnector);
			bReconnect = true;
		}
	}

	if (!bReconnect)
	{
		LOG(WARNING) << "disconnect login or game server is remove or register again, no server reconnect...";
	}
}

CT_VOID CServerMgr::CheckDisconnectCustomerServer()
{
	bool bReconnect = false;
	for (auto it : m_mapCustomerServer)
	{
		if (it.second.pConnector->IsClose())
		{
			CNetModule::getSingleton().InsertReconnector(it.second.pConnector);
			bReconnect = true;
		}
	}

	if (!bReconnect)
	{
		LOG(WARNING) << "disconnect customer server is remove or register again, no server reconnect...";
	}
}

stGameServerInfo* CServerMgr::FindGameServer(CT_DWORD dwServerID)
{
	auto it = m_mapGameServer.find(dwServerID);
	if (it != m_mapGameServer.end())
	{
		return &it->second;
	}

	return NULL;
}

stGameServerInfo* CServerMgr::FindReqGameServer(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID, CT_BYTE& cbState)
{
	for (auto& it : m_mapGameServer)
	{
		if (it.second.cbState == 0)
		{
			continue;
		}

		if (it.second.wGameID == wGameID && it.second.wKindID == wKindID && it.second.wRoomKindID == wRoomKindID)
		{
			if (cbState == 0)
				cbState = 1;

			if (it.second.dwOnlineCount < it.second.dwFullCount)
			{
				return &it.second;
			}
		}
	}

	return NULL;
}


stGameServerInfo* CServerMgr::FindReqGameServerEx(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID)
{
	for (auto& it : m_mapGameServer)
	{
		if (it.second.wGameID == wGameID && it.second.wKindID == wKindID && it.second.wRoomKindID == wRoomKindID)
		{
			//if (it.second.dwOnlineCount < it.second.dwFullCount)
			{
				return &it.second;
			}
		}
	}

	return NULL;
}