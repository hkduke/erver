#include "ServerUserItem.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "CMD_ConFish.h"
#include "ServerCfg.h"
#include "GameServerThread.h"

extern CNetConnector *pNetCenter;

CServerUserItem::CServerUserItem()
{
	ResetUserItem();
}

CServerUserItem::~CServerUserItem()
{

}

void CServerUserItem::ResetUserItem()
{
	m_wTableID = ERROR_TABLE_ID;
	m_wChairID = ERROR_CHAIR_ID;

	m_status = sFree;
	m_bTrusteeship = false;
	m_bWriteScore = false;

	m_pSocket = NULL;		
	m_pDBSocket = NULL;
	m_uClientNetAddr = 0;
	m_pThirdGSConnector = NULL;

	memset(&m_UserBaseData, 0, sizeof(m_UserBaseData));
	memset(&m_UserScoreData, 0, sizeof(m_UserScoreData)); 
	memset(&m_UserPRoomData, 0, sizeof(m_UserPRoomData));
}

void CServerUserItem::SetUserBaseData(const GS_UserBaseData&  udb)
{
	memcpy(&(m_UserBaseData), &udb, sizeof(GS_UserBaseData));
}


GS_UserBaseData& CServerUserItem::GetUserBaseData()
{
	return m_UserBaseData;
}


void CServerUserItem::SetUserScoreData(const GS_UserScoreData& ugs)
{
	memcpy(&(m_UserScoreData), &ugs, sizeof(GS_UserScoreData));
}

GS_UserScoreData& CServerUserItem::GetUserScoreData()
{
	return m_UserScoreData;
}

void CServerUserItem::SetUserPRoomData(const GS_UserPRoomData& proomData)
{
	memcpy(&(m_UserPRoomData), &proomData, sizeof(m_UserPRoomData));
}

void CServerUserItem::GetUserBaseMsgData(MSG_SC_UserBaseInfo& basedata)
{
	basedata.dwUserID = m_UserBaseData.dwUserID;
	basedata.cbGender = m_UserBaseData.cbGender;
	basedata.dwGem = m_UserBaseData.dwGem;
	basedata.llScore = m_UserBaseData.llScore;
	_snprintf_info(basedata.szNickName, sizeof(basedata.szNickName), "%s", m_UserBaseData.szNickName);
	//_snprintf_info(basedata.szHeadUrl, sizeof(basedata.szHeadUrl), "%s", m_UserBaseData.szHeadUrl);
	_snprintf_info(basedata.szIP, sizeof(basedata.szIP), "%s", m_UserBaseData.szIP);
	_snprintf_info(basedata.szLocation, sizeof(basedata.szLocation), "%s", m_UserBaseData.szLocation);
	basedata.dLongitude = m_UserBaseData.dLongitude;
	basedata.dLatitude = m_UserBaseData.dLatitude;
	basedata.wTableID = m_wTableID;
	basedata.wChairID = m_wChairID;
	basedata.usStatus = m_status;
}

void CServerUserItem::GetUserBaseMsgDataEx(MSG_SC_UserBaseInfo_ForScore& basedata)
{
	basedata.dwUserID = m_UserBaseData.dwUserID;
	basedata.cbGender = m_UserBaseData.cbGender;
	basedata.cbVipLevel = m_UserBaseData.cbVipLevel;
	basedata.cbVipLevel2 = m_UserBaseData.cbVipLevel2;
	basedata.dwGem = m_UserBaseData.dwGem;
	basedata.dScore = m_UserBaseData.llScore*0.01f;
	basedata.cbHeadIndex = m_UserBaseData.cbHeadIndex;
	_snprintf_info(basedata.szNickName, sizeof(basedata.szNickName), "%s", m_UserBaseData.szNickName);
	//_snprintf_info(basedata.szHeadUrl, sizeof(basedata.szHeadUrl), "%s", m_UserBaseData.szHeadUrl);
	//_snprintf_info(basedata.szLocation, sizeof(basedata.szLocation), "%s", m_UserBaseData.szLocation);
	basedata.wTableID = m_wTableID;
	basedata.wChairID = m_wChairID;
	basedata.usStatus = m_status;
}

void CServerUserItem::SendUserMessage(CT_DWORD mainID, CT_DWORD subID)
{
	if (m_pSocket != NULL && m_status != sOffLine)
	{
		CNetModule::getSingleton().Send(m_pSocket, mainID, subID);
	}
}

void CServerUserItem::SendUserMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len)
{
	if (m_pSocket != NULL && m_status != sOffLine)
	{
		CNetModule::getSingleton().Send(m_pSocket, mainID, subID, pBuf, len);
	}
}

void CServerUserItem::SendDBMessage(CT_DWORD mainID, CT_DWORD subID)
{
	if (m_pDBSocket != NULL)
	{
		CNetModule::getSingleton().Send(m_pDBSocket, mainID, subID);
	}
}

void CServerUserItem::SendDBMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len)
{
	if (m_pDBSocket != NULL)
	{
		CNetModule::getSingleton().Send(m_pDBSocket, mainID, subID, pBuf, len);
	}
}

//发送捕鱼消息给前端
CT_VOID CServerUserItem::SendFishMsgToClient(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD dwLen)
{
	if (m_pSocket != NULL && m_status != sOffLine)
	{
		static CT_BYTE cbDataBuffer[SOCKET_TCP_PACKET];
		CT_DWORD dwDataSize = 0;

		CT_DWORD dwDownSize = sizeof(MSG_GameMsgDownHead);
		MSG_GameMsgDownHead DownData;
		memset(&DownData, 0, dwDownSize);
		DownData.dwMainID = MSG_FISH_GAME_MAIN;
		DownData.dwSubID = SUB_S2C_FISH_GAME;
		DownData.uValue1 = m_uClientNetAddr;
		memcpy(cbDataBuffer, &DownData, dwDownSize);
		dwDataSize += dwDownSize;

		CMD_Command FishMsgHead;
		FishMsgHead.dwMainID = mainID;
		FishMsgHead.dwSubID = subID;
		FishMsgHead.dwDataSize = dwLen;

		CT_DWORD dwFishHeadSize = (CT_DWORD)sizeof(CMD_Command);
		memcpy(cbDataBuffer + dwDataSize , &FishMsgHead, dwFishHeadSize);
		dwDataSize += dwFishHeadSize;
		memcpy(cbDataBuffer + dwDataSize, pBuf, dwLen);
		dwDataSize += dwLen;
		CNetModule::getSingleton().Send(m_pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, cbDataBuffer, dwDataSize);
	}
}

////////////////////////////////////////////////////////
CServerUserManager::CServerUserManager()
	:m_pGameRoomKindInfo(NULL)
{

}

CServerUserManager::~CServerUserManager()
{
	for (auto it : m_UserItemStore)
	{
		delete it;
	}

	for (auto it : m_UserItemArray)
	{
		delete it;
	}

	m_UserItemStore.clear();
	m_UserItemArray.clear();
	m_UserMap.clear();
}

CT_VOID CServerUserManager::Init(tagGameRoomKind* pGameRoomKind)
{
	if (pGameRoomKind != NULL)
	{
		m_pGameRoomKindInfo = pGameRoomKind;
	}
}

CT_BOOL CServerUserManager::InsertUserItem(CServerUserItem** pIServerUserResult, CT_DWORD dwUserID)
{
	CServerUserItem* pServerUserItem = NULL;
	if (m_UserItemStore.size() > 0)
	{
		pServerUserItem = m_UserItemStore.back();
		m_UserItemStore.pop_back();
		pServerUserItem->ResetUserItem();
	}
	else
	{
		try
		{
			pServerUserItem = new CServerUserItem;
		}
		catch (...)
		{
			assert(false);
			return false;
		}
		
	}

	m_UserItemArray.push_back(pServerUserItem);
	m_UserMap[dwUserID] = pServerUserItem;
	*pIServerUserResult = pServerUserItem;
	UpdateUserCount(dwUserID, 1);

	return true;
}

CT_BOOL	CServerUserManager::DeleteUserItem(CServerUserItem* pServerUserItem)
{
	if (pServerUserItem == NULL)
	{
		return false;
	}

	CServerUserItem* pTempUserItem = NULL;
	for (auto it = m_UserItemArray.begin(); it != m_UserItemArray.end(); ++it)
	{
		pTempUserItem = *it;
		if (pTempUserItem != pServerUserItem)
		{
			continue;
		}
		CT_DWORD dwUserID = pTempUserItem->GetUserID();
		m_UserItemArray.erase(it);
		m_UserMap.erase(dwUserID);
		pTempUserItem->ResetUserItem();
		m_UserItemStore.push_back(pTempUserItem);
		UpdateUserCount(dwUserID, 0);
		return true;
	}

	return false;
}

CServerUserItem* CServerUserManager::FindUserItem(CT_DWORD dwUserID)
{
	auto it = m_UserMap.find(dwUserID);
	if (it != m_UserMap.end())
	{
		return it->second;
	}

	return NULL;
}

//去中心服务器更新人数
CT_VOID CServerUserManager::UpdateUserCount(CT_DWORD dwUserID, CT_BYTE cbUpdateMode)
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CMD_Update_GS_User updateUser;
		updateUser.dwServerID = CServerCfg::m_nServerID;
		updateUser.wGameID = m_pGameRoomKindInfo->wGameID;
		updateUser.wKindID = m_pGameRoomKindInfo->wKindID;
		updateUser.wRoomKindID = m_pGameRoomKindInfo->wRoomKindID;
		updateUser.wUserCount = (CT_WORD)m_UserMap.size();
		updateUser.wRealUserCount = updateUser.wUserCount;

		updateUser.dwUserID = dwUserID;
		updateUser.cbUpdateMode = cbUpdateMode;

		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_UPDATE_USER, &updateUser, sizeof(CMD_Update_GS_User));
	}
}

CT_VOID CServerUserManager::SetUserOffLine(acl::aio_socket_stream* pSocket, CGameServerThread* pServerThread)
{
	if (m_UserMap.empty())
	{
		return;
	}

	for (auto it = m_UserMap.begin(); it != m_UserMap.end(); ++it)
	{
		acl::aio_socket_stream* pProxySock = it->second->GetUserCon();
		if (pProxySock == pSocket)
		{
			pServerThread->OnUserOffLine(it->first);
		}
	}
}

//////////////////////////////////////////////////////////////////////
CUserConnectorManager::CUserConnectorManager()
{

}

CUserConnectorManager::~CUserConnectorManager()
{

}

CT_BOOL CUserConnectorManager::InsertConnector(CNetConnector** pINetConnector, CT_DWORD dwUserID)
{
	CNetConnector* pNetConnector = NULL;
	stConnectParam param;
	param.conType = CONNECT_GAME_THIRD;
	param.dwServerID = dwUserID;

	if (m_NetConnectorStore.size() > 0)
	{
		pNetConnector = m_NetConnectorStore.back();
		m_NetConnectorStore.pop_back();
		pNetConnector->SetConnectParam(param);
		CNetModule::getSingleton().InsertReconnector(pNetConnector);
	}
	else
	{
		try
		{
			pNetConnector = CNetModule::getSingleton().AddConnector(CServerCfg::m_ThirdServerAddress, true, param);
		}
		catch (...)
		{
			assert(false);
			return false;
		}
	}

	m_NetConnectorArray.push_back(pNetConnector);
	m_NetConnectorMap[dwUserID] = pNetConnector;
	*pINetConnector = pNetConnector;

	return true;
}

//删除连接
CT_BOOL CUserConnectorManager::DeleteConnector(CNetConnector* pNetConnector)
{
	if (pNetConnector == NULL)
	{
		return false;
	}

	CNetConnector* pTempNetConnector = NULL;
	for (auto it = m_NetConnectorArray.begin(); it != m_NetConnectorArray.end(); ++it)
	{
		pTempNetConnector = *it;
		if (pTempNetConnector != pNetConnector)
		{
			continue;
		}
		stConnectParam& connectParam = pNetConnector->GetConnectParam();
		if (pNetConnector->IsRunning())
		{
			//如果还是激活状态, 则先关闭连接
			acl::aio_socket_stream* pSocket =  pNetConnector->GetSocket();
			if (pSocket)
			{
				//pSocket->close();
				//不应该跨线程调用close方法
				CNetModule::getSingleton().CloseSocket(pSocket);
			}
		}
		m_NetConnectorArray.erase(it);
		m_NetConnectorMap.erase(connectParam.dwServerID);
		m_FishSocketMap.erase(pNetConnector->GetSocket());
		m_NetConnectorStore.push_back(pTempNetConnector);
		return true;
	}

	return false;
}

//查找连接
CNetConnector* CUserConnectorManager::FindConnector(CT_DWORD dwUserID)
{
	auto it = m_NetConnectorMap.find(dwUserID);
	if (it != m_NetConnectorMap.end())
	{
		return it->second;
	}

	return NULL;
}

CT_VOID CUserConnectorManager::InsertFishSocket(acl::aio_socket_stream* pFishSock, CServerUserItem* pUserItem)
{
	m_FishSocketMap[pFishSock] = pUserItem;
}

CServerUserItem* CUserConnectorManager::FindFishUserBySocket(acl::aio_socket_stream* pFishSock)
{
	auto it = m_FishSocketMap.find(pFishSock);
	if (it != m_FishSocketMap.end())
	{
		return it->second;
	}

	return NULL;
}