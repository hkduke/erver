#include "stdafx.h"
#include "SystemMsgMgr.h"
#include "CMD_Plaza.h"
#include "UserMgr.h"
#include "NetModule.h"

CSystemMsgMgr::CSystemMsgMgr()
{
}

CSystemMsgMgr::~CSystemMsgMgr()
{
}

CT_VOID CSystemMsgMgr::BroadcastGameScoreMsg(MSG_G2CS_WinScore_Info* pWinScoreInfo)
{
	MSG_SC_Broadcast_GameMsg gameMsg;
	gameMsg.dwMainID = MSG_BROADCAST_MSG_MAIN;
	gameMsg.dwSubID = SUB_SC_GAME_MSG;
	_snprintf_info(gameMsg.szNickname, sizeof(gameMsg.szNickname), "%s", pWinScoreInfo->szNickName);
	_snprintf_info(gameMsg.szServerName, sizeof(gameMsg.szServerName), "%s", pWinScoreInfo->szServerName);
	gameMsg.wGameID = pWinScoreInfo->wGameID;
	gameMsg.cbVip2 = pWinScoreInfo->cbVip2;
	gameMsg.cbCardType = pWinScoreInfo->cbCardType;
	gameMsg.dWinScore = pWinScoreInfo->llWinScore*TO_DOUBLE;

	MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
	for (auto& it : mapAllGlobalUser)
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
		if (pSocket == NULL)
		{
			continue;
		}
		gameMsg.dwValue2 = it.first;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &gameMsg, sizeof(gameMsg));
	}
}


CT_VOID	CSystemMsgMgr::InsertSystemMsg(MSG_M2CS_SendSysMsg* pSystemMsg)
{
	tagSystemMsg systemMsg;
	systemMsg.dwID = 0;
	systemMsg.dwInterval = pSystemMsg->dwInterval;
	systemMsg.dwValidTime = pSystemMsg->dwValidTime;
	systemMsg.dwLastBroadcastTime = 0;
	_snprintf_info(systemMsg.szMessage, sizeof(systemMsg.szMessage), "%s", pSystemMsg->szMessage);

	std::vector<tagSystemMsg>& vecSystemMsg = m_mapSystemMsg[pSystemMsg->cbPlatformId];
	vecSystemMsg.push_back(systemMsg);
	BroadcastSystemMsg();
}

CT_VOID	CSystemMsgMgr::InsertSystemMsg(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_D2CS_SendSysMsg))
	{
		return;
	}
	MSG_D2CS_SendSysMsg* pSystemMsg = (MSG_D2CS_SendSysMsg*)pData;
	tagSystemMsg systemMsg;
	systemMsg.dwID = pSystemMsg->dwMsgID;
	systemMsg.dwInterval = pSystemMsg->dwInterval;
	systemMsg.dwValidTime = pSystemMsg->dwValidTime;
	systemMsg.dwLastBroadcastTime = 0;
	_snprintf_info(systemMsg.szMessage, sizeof(systemMsg.szMessage), "%s", pSystemMsg->szMessage);

	std::vector<tagSystemMsg>& vecSystemMsg = m_mapSystemMsg[pSystemMsg->cbPlatformId];
	vecSystemMsg.push_back(systemMsg);
	BroadcastSystemMsg();
}

CT_VOID	CSystemMsgMgr::BroadcastSystemMsg()
{
	if (m_mapSystemMsg.empty())
	{
		return;
	}

	MSG_SC_Broadcast_SysMsg gameMsg;
	gameMsg.dwMainID = MSG_BROADCAST_MSG_MAIN;
	gameMsg.dwSubID = SUB_SC_SYS_MSG;
	

	CT_DWORD dwNow = Utility::GetTime();

	for (auto itMap = m_mapSystemMsg.begin(); itMap != m_mapSystemMsg.end(); )
	{
		std::vector<tagSystemMsg>& vecSystemMsg = itMap->second;
		for (auto itVec = vecSystemMsg.begin(); itVec != vecSystemMsg.end(); )
		{
			if (dwNow < itVec->dwValidTime)
			{
				CT_DWORD dwInterval = dwNow - itVec->dwLastBroadcastTime;
				if (dwInterval < itVec->dwInterval)
				{
					++itVec;
					continue;
				}

				_snprintf_info(gameMsg.szSysMsg, sizeof(gameMsg.szSysMsg), "%s", itVec->szMessage);

				MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
				for (auto& itUser : mapAllGlobalUser)
				{
					tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(itUser.first);
					if (pUserInfo == NULL || !pUserInfo->bOnline)
					{
						continue;
					}

					//255标识全平台广播
					if (itMap->first != 255 && pUserInfo->cbPlatformId != itMap->first)
					{
						continue;
					}

					acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(itUser.first);
					if (pSocket == NULL)
					{
						continue;
					}
					gameMsg.dwValue2 = itUser.first;
					CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &gameMsg, sizeof(gameMsg));
				}

				itVec->dwLastBroadcastTime = dwNow;
				++itVec;
			}
			else
			{
				itVec = vecSystemMsg.erase(itVec);
			}
		}

		if (!vecSystemMsg.empty())
		{
			++itMap;
		}
		else
		{
			itMap = m_mapSystemMsg.erase(itMap);
		}
	}
}

CT_VOID CSystemMsgMgr::SendSystemMsg(CT_DWORD dwUserID, CT_CHAR* szSystemMsg)
{
	MSG_SC_Broadcast_SysMsg gameMsg;
	gameMsg.dwMainID = MSG_BROADCAST_MSG_MAIN;
	gameMsg.dwSubID = SUB_SC_SYS_MSG;
	_snprintf_info(gameMsg.szSysMsg, sizeof(gameMsg.szSysMsg), "%s", szSystemMsg);
	
	if (dwUserID == 0)
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& itUser : mapAllGlobalUser)
		{
			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(itUser.first);
			if (pSocket == NULL)
			{
				continue;
			}
			gameMsg.dwValue2 = itUser.first;
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &gameMsg, sizeof(gameMsg));
		}
	}
	else
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}
		
		gameMsg.dwValue2 = dwUserID;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &gameMsg, sizeof(gameMsg));
	}
}

CT_VOID CSystemMsgMgr::RemoveSystemMsg(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_D2CS_RemoveSysMsg))
	{
		return;
	}
	
	MSG_D2CS_RemoveSysMsg* pRemoveSysMsg = (MSG_D2CS_RemoveSysMsg*)pData;

	std::vector<tagSystemMsg>& vecSystemMsg = m_mapSystemMsg[pRemoveSysMsg->cbPlatformId];
	CT_DWORD  dwMsgID = pRemoveSysMsg->dwMsgID;
	
	for (auto it = vecSystemMsg.begin(); it != vecSystemMsg.end(); ++it)
	{
		if (dwMsgID == it->dwID)
		{
			vecSystemMsg.erase(it);
			if (vecSystemMsg.empty())
				m_mapSystemMsg.erase(pRemoveSysMsg->cbPlatformId);
			return;
		}
	}
}