#include "ServerMgr.h"
#include "glog_wrapper.h"
#include "GlobalEnum.h"
#include "Utility.h"
#include "NetModule.h"


CServerMgr::CServerMgr()
{

}

CServerMgr::~CServerMgr() 
{
	for (auto it = m_mapGameKind.begin(); it != m_mapGameKind.end(); it++)
	{
		std::set<tagGameKind*>& setGameKind = it->second;
		for (auto& itSet : setGameKind)
		{
			tagGameKind* pGameKind = itSet;
			SAFE_DELETE(pGameKind);
		}
	}

	for (auto it = m_mapGamePlayKind.begin(); it != m_mapGamePlayKind.end(); it++)
	{
		std::set<tagGameRoomKind*>& setGamePlayKind = it->second;
		for (auto& itSet : setGamePlayKind)
		{
			tagGameRoomKind* pGamePlayKind = itSet;
			SAFE_DELETE(pGamePlayKind);
		}
	}
}

CT_VOID CServerMgr::BindProxyServer(acl::aio_socket_stream* pSocket, const PS_BindData* pBindData)
{
	if (pBindData == NULL)
	{
		return;
	}

	if (m_mapProxyServer.find(pBindData->dwProxyServerID) != m_mapProxyServer.end())
	{
		LOG(WARNING) << "repeat bind proxy server, server id :" << pBindData->dwProxyServerID;
		return;
	}

	m_mapProxyServer[pBindData->dwProxyServerID] = pSocket;

	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "bind one proxy server, ip:" << strIp;//pSocket->get_peer(true);
	}
}

acl::aio_socket_stream* CServerMgr::FindProxyServer(CT_DWORD dwServerID)
{
	auto it = m_mapProxyServer.find(dwServerID);
	if (it != m_mapProxyServer.end())
	{
		return (acl::aio_socket_stream*)it->second;
	}

	return NULL;
}

CT_DWORD CServerMgr::FindProxyServerID(acl::aio_socket_stream* pSocket)
{
	for (auto it = m_mapProxyServer.begin(); it != m_mapProxyServer.end(); ++it)
	{
		if (it->second == pSocket)
		{
			return it->first;
		}
	}

	return 0;
}

CT_VOID CServerMgr::DeleteProxyServer(acl::aio_socket_stream* pSocket)
{
	for (auto it = m_mapProxyServer.begin(); it != m_mapProxyServer.end(); ++it)
	{
		if (it->second == pSocket)
		{
			LOG(WARNING) << "remove one proxy server, id: " << it->first;
			m_mapProxyServer.erase(it);
			return;
		}
	}
}

CT_BOOL CServerMgr::InsertGameKind(tagGameKind* pGameKind)
{
	if (pGameKind == NULL)
	{
		return false;
	}

	std::set<tagGameKind*>& setGameKind = m_mapGameKind[pGameKind->wGameID];
	if (setGameKind.find(pGameKind) != setGameKind.end())
	{
		return false;
	}

	for (auto it = setGameKind.begin(); it != setGameKind.end(); it++)
	{
		tagGameKind *pKind = *it;
		if (pKind && pKind->wGameID == pGameKind->wGameID && pKind->wKindID == pGameKind->wKindID)
		{
			return false;
		}
	}

	setGameKind.insert(pGameKind);
	return true;
}

CT_BOOL CServerMgr::InsertGamePlayKind(tagGameRoomKind* pPlayKind)
{
	if (pPlayKind == NULL)
	{
		return false;
	}

	std::set<tagGameRoomKind*>& setGamePlayKind = m_mapGamePlayKind[pPlayKind->wGameID];
	if (setGamePlayKind.find(pPlayKind) != setGamePlayKind.end())
	{
		return false;
	}

	for (auto it = setGamePlayKind.begin(); it != setGamePlayKind.end(); ++it)
	{
		tagGameRoomKind* pKind = *it;
		if (pKind && pKind->wGameID == pPlayKind->wGameID && pKind->wKindID == pPlayKind->wKindID && pKind->wRoomKindID == pPlayKind->wRoomKindID)
		{
			return false;
		}
	}

	setGamePlayKind.insert(pPlayKind);
	return true;

}

CT_BOOL CServerMgr::EmumGameKindItem(CT_DWORD dwGameID, std::vector<MSG_SC_GameItem>& vecGameItem)
{
	vecGameItem.clear();
	auto it = m_mapGameKind.find(dwGameID);
	if (it == m_mapGameKind.end())
	{
		return false;
	}

	std::set<tagGameKind*>& setGameKind = it->second;
	for (auto& it : setGameKind)
	{
		tagGameKind* pGameKind = it;
		if (pGameKind != NULL)
		{
			MSG_SC_GameItem gameItem;
			gameItem.wGameID = pGameKind->wGameID;
			gameItem.wKindID = pGameKind->wKindID;
			gameItem.wSortID = pGameKind->wSortID;
			_snprintf_info(gameItem.szKindName, sizeof(gameItem.szKindName), "%s", pGameKind->szKindName);
			vecGameItem.push_back(gameItem);
		}
	}

	return true;
}

CT_BOOL CServerMgr::EmumGameRoomKindItem(CT_DWORD dwGameID, std::vector<MSG_SC_RoomKindItem>& vecPlayKindItem)
{
	vecPlayKindItem.clear();
	auto it = m_mapGamePlayKind.find(dwGameID);
	if (it == m_mapGamePlayKind.end())
	{
		return false;
	}

	std::set<tagGameRoomKind*>& setGamePlayKind = it->second;
	for (auto& it : setGamePlayKind)
	{
		tagGameRoomKind* pGameRoomKind = it;
		if (pGameRoomKind != NULL && pGameRoomKind->wRoomKindID != PRIVATE_ROOM)
		{
			MSG_SC_RoomKindItem roomKind;
			roomKind.wGameID = pGameRoomKind->wGameID;
			roomKind.wKindID = pGameRoomKind->wKindID;
			roomKind.wRoomKindID = pGameRoomKind->wRoomKindID;
			roomKind.wSortID = 0;/*pGameRoomKind->wSortID*/;
			roomKind.bHasServer = false;
			roomKind.dwCellScore = pGameRoomKind->dwCellScore;
			roomKind.wStartMaxPlayer = pGameRoomKind->wStartMaxPlayer;
			roomKind.dwEnterMinScore = pGameRoomKind->dwEnterMinScore;
			roomKind.dwEnterMaxScore = pGameRoomKind->dwEnterMaxScore;
			roomKind.dwCellScore = pGameRoomKind->dwCellScore;
			vecPlayKindItem.push_back(roomKind);
		}
	}

	return true;
}

CT_BOOL CServerMgr::EmumPrivateRoomInfo(CT_DWORD dwGameID, std::vector<MSG_SC_PrivateRoomInfo>& vecPrivateItem)
{
	vecPrivateItem.clear();
	if (dwGameID == 0)
	{
		for (auto& it : m_mapGamePlayKind)
		{
			std::set<tagGameRoomKind*>& setGamePlayKind = it.second;
			for (auto& itKind : setGamePlayKind)
			{
				tagGameRoomKind* pGameRoomKind = itKind;
				if (pGameRoomKind != NULL && pGameRoomKind->wRoomKindID == PRIVATE_ROOM)
				{
					MSG_SC_PrivateRoomInfo PrivateRoom;
					PrivateRoom.wGameID = pGameRoomKind->wGameID;
					PrivateRoom.wKindID = pGameRoomKind->wKindID;
					_snprintf_info(PrivateRoom.szJsonParam, sizeof(PrivateRoom.szJsonParam), "%s", pGameRoomKind->szPRoomCond);

					vecPrivateItem.push_back(PrivateRoom);
				}
			}
		}
		return true;
	}

	auto it = m_mapGamePlayKind.find(dwGameID);
	if (it == m_mapGamePlayKind.end())
	{
		return false;
	}

	std::set<tagGameRoomKind*>& setGamePlayKind = it->second;
	for (auto& it : setGamePlayKind)
	{
		tagGameRoomKind* pGameRoomKind = it;
		if (pGameRoomKind != NULL && pGameRoomKind->wRoomKindID == PRIVATE_ROOM)
		{
			MSG_SC_PrivateRoomInfo PrivateRoom;
			PrivateRoom.wGameID = pGameRoomKind->wGameID;
			PrivateRoom.wKindID = pGameRoomKind->wKindID;
			_snprintf_info(PrivateRoom.szJsonParam, sizeof(PrivateRoom.szJsonParam), "%s", pGameRoomKind->szPRoomCond);
		
			vecPrivateItem.push_back(PrivateRoom);
		}
	}

	return true;
}