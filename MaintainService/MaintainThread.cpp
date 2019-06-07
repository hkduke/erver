#include "stdafx.h"
#include "MaintainThread.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "CMD_ShareMsg.h"
#include "CMD_Game.h"
#include "Utility.h"
#include "CMD_Maintain.h"

//中心服务器连接
extern CNetConnector *pNetCenter;

//定时器
#define		IDI_CONNECT_CENTER				(100)
#define		IDI_CONNECT_SERVER				(101)
#define		IDI_CONNECT_DBSERVER			(102)

#define		TIME_CONNECT_CENTER				(10000)
#define		TIME_CONNECT_SERVER				(10000)
#define		TIME_CONNECT_DBSERVER			(10000)

CMaintainThread::CMaintainThread(void)
{
}

CMaintainThread::~CMaintainThread(void)
{
}

CT_VOID CMaintainThread::OnTCPSocketLink(CNetConnector* pConnector)
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
	}
}

CT_VOID CMaintainThread::OnTCPSocketShut(CNetConnector* pConnector)
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

CT_VOID CMaintainThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{

}

CT_VOID CMaintainThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
}

//网络bit数据到来
CT_VOID CMaintainThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (pSocket == NULL)
	{
		return;
	}

	switch (pMc->dwMainID)
	{
	case MSG_MCS_MAIN:
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
CT_VOID CMaintainThread::OnTimer(CT_DWORD dwTimerID)
{

}

//打开
CT_VOID CMaintainThread::Open()
{
}

CT_VOID CMaintainThread::GoCenterServerRegister()
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		MSG_M2CS_QueryServer queryServer;
		//registerProxy.dwServerID = CServerCfg::m_nServerID;
		//_snprintf_info(registerProxy.szServerIP, sizeof(registerProxy.szServerIP), "%s", CServerCfg::m_LocalAddress);
		//_snprintf_info(registerProxy.szServerName, sizeof(registerProxy.szServerName), "%s", CServerCfg::m_ServerName);
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_MCS_MAIN, SUB_M2CS_QUERY_ALL_SERVER, &queryServer, sizeof(queryServer));
		return;
	}
	LOG(WARNING) << "go center server read server info failed, center net is not running...";
}

CT_VOID CMaintainThread::OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CS2M_QUERY_ALL_SERVER:
	{
		if (wDataSize % sizeof(CMD_GameServer_Info) != 0)
		{
			return;
		}

		CT_DWORD wCount = wDataSize / sizeof(CMD_GameServer_Info);
		CMD_GameServer_Info* pGameInfo = (CMD_GameServer_Info*)pData;
		for (CT_WORD i = 0; i < wCount; ++i)
		{
			AddGameServerList(pGameInfo++);
		}
	}
	break;

	case SUB_CS2M_QUERY_SERVER_FINISH:
	{
		SelectOperation();
	}
	break;
	case SUB_CS2M_SET_SERVER_STATE_SUCC:
	{
		if (wDataSize !=  sizeof(MSG_M2CS_SetServerState))
		{
			return;
		}
		MSG_M2CS_SetServerState* pState = (MSG_M2CS_SetServerState*)pData;

		if (pState->dwServerID != 0)
		{
			auto it = m_mapGameServer.find(pState->dwServerID);
			it->second.cbState = pState->cbState;
			std::cout << "game server(" << pState->dwServerID << ") set state: " << (CT_WORD)pState->cbState << std::endl;
		}
		else
		{
			for (auto& it : m_mapGameServer)
			{
				it.second.cbState = pState->cbState;
			}

			std::cout << "all game server set state: " << (CT_WORD)pState->cbState << std::endl;
		}
	

		SelectOperation();
	}
	break;
	case SUB_CS2M_DISMISS_SERVER_RPOOM_SUCC:
	{
		if (wDataSize != sizeof(MSG_M2CS_DismissPRoom))
		{
			return;
		}
		MSG_M2CS_DismissPRoom* pDismiss  = (MSG_M2CS_DismissPRoom*)pData;

		std::cout << "game server(" << pDismiss->dwServerID << ")  is dismiss proom!"<< std::endl;

		SelectOperation();
	}
	break;
	case SUB_CS2M_DISMISS_SERVER_RPOOM_BYNUM_SUCC:
	{
		if (wDataSize != sizeof(MSG_M2CS_DismissPRoom_ByRoomNum))
		{
			return;
		}
		MSG_M2CS_DismissPRoom_ByRoomNum* pDismiss = (MSG_M2CS_DismissPRoom_ByRoomNum*)pData;

		std::cout << "private room (" << pDismiss->dwRoomNum << ")  is dismissed!" << std::endl;

		SelectOperation();
	}
	break;
	case SUB_CS2M_REMOVE_USER_OUT_CACHE_RESULT:
	{
		if (wDataSize != sizeof(MSG_CS2M_CommonResult))
		{
			return;
		}
		MSG_CS2M_CommonResult* pRemove = (MSG_CS2M_CommonResult*)pData;

		if (pRemove->cbResult == 0)
		{
			std::cout << "remove user out cache succ!" << std::endl;
		}
		else
		{
			std::cout << "remove user out cache fail, the room info is integrity!" << std::endl;
		}
		

		SelectOperation();
	}
	break;
	case SUB_CS2M_SEND_SYS_MESSAGE:
	{
		if (wDataSize != sizeof(MSG_CS2M_CommonResult))
		{
			return;
		}
		MSG_CS2M_CommonResult* pRemove = (MSG_CS2M_CommonResult*)pData;

		if (pRemove->cbResult == 0)
		{
			std::cout << "send system message succ!" << std::endl;
		}
		else
		{
			std::cout << "send system message failed!" << std::endl;
		}

		SelectOperation();
	}
	break;
	default:
		break;
	}
}

CT_VOID CMaintainThread::AddGameServerList(const CMD_GameServer_Info* pGameServer)
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

	CMD_GameServer_Info GSInfo;
	memcpy(&GSInfo, pGameServer, sizeof(CMD_GameServer_Info));

	LOG(INFO) << "add game server, game server id:" << pGameServer->dwServerID;
	m_mapGameServer.insert(std::make_pair(GSInfo.dwServerID, GSInfo));
}

CT_VOID	CMaintainThread::SelectOperation()
{
	std::cout << "game server list:" << std::endl;
	for (auto& it : m_mapGameServer)
	{
		std::cout << it.first << " ";
#ifdef _OS_WIN32_CODE
		std::string strServerName = Utility::Utf82Ansi(it.second.szRoomName);
#else
		std::string strServerName = Utility::Utf82Ansi(it.second.szRoomName);
#endif // _OS_WIN32_CODE
		
		std::cout << strServerName.c_str() << "," <<"\tstate is:" << (CT_WORD)it.second.cbState << std::endl;
	}

	std::cout << "what you want? " << std::endl;

	std::cout << "1. set the server state(e.g: 50 0)." << std::endl;
	std::cout << "2. send system message." << std::endl;
	std::cout << "3. delete system message." << std::endl;
	std::cout << "4. set recharge judge." << std::endl;
	//std::cout << "2. dismiss server all room(e.g: 50)." << std::endl;
	//std::cout << "3. dismiss room num(e.g: 500041)." << std::endl;
	//std::cout << "4. remove the user in the cache(e.g: 10008)." << std::endl;

	CT_WORD wOption = 0;

	while (true)
	{
		std::cin >> wOption;

		if (wOption == 1)
		{
			std::cout << "which game server state will you change ?" << std::endl;
			CT_DWORD dwServerID;
			CT_WORD wState;
			std::cin >> dwServerID >> wState;

			MSG_M2CS_SetServerState serverState;
			serverState.dwServerID = dwServerID;
			serverState.cbState = (CT_BYTE)wState;
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_MCS_MAIN, SUB_M2CS_SET_SERVER_STATE, &serverState, sizeof(serverState));
			break;
		}
		else if (wOption == 2)
		{
			std::cout << "please enter message: " << std::endl;
			std::string strMessage;
			std::cin >> strMessage;
			std::cout << "please enter message interval(second): " << std::endl;
			CT_DWORD dwInterval;
			std::cin >> dwInterval;
			std::cout << "please enter message valid time(second):  " << std::endl;
			CT_DWORD dwValidTime;
			std::cin >> dwValidTime;
			dwValidTime += (CT_DWORD)Utility::GetTime();
			MSG_M2CS_SendSysMsg sysMsg;
			sysMsg.dwInterval = dwInterval;
			sysMsg.dwValidTime = dwValidTime;
			sysMsg.cbPlatformId = 1;
			_snprintf_info(sysMsg.szMessage, sizeof(sysMsg.szMessage), "%s", strMessage.c_str());
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_MCS_MAIN, SUB_M2CS_SEND_SYS_MESSAGE, &sysMsg, sizeof(sysMsg));
			break;
		}
		else if (wOption == 3)
		{
			std::cout << "please enter platform id: " << std::endl;
			CT_DWORD  dwPlatformId;
			std::cin >> dwPlatformId;
			std::cout << "please enter message id: " << std::endl;
			CT_DWORD dwMessageId;
			std::cin >> dwMessageId;

			MSG_D2CS_RemoveSysMsg removeSysMsg;
			removeSysMsg.dwMsgID = dwMessageId;
			removeSysMsg.cbPlatformId = dwPlatformId;

			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_DIPCS_MAIN, SUB_DIP2CS_REMOVE_SYSMSG, &removeSysMsg, sizeof(removeSysMsg));
			break;
		}
		else if (wOption == 4)
		{
			std::cout << "please enter recharge judge, 1 for yes or 0 for no judge: " << std::endl;
			CT_DWORD  dwjudge;
			std::cin >> dwjudge;

			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_MCS_MAIN, SUB_M2CS_SET_RECHARGE_JUDGE, &dwjudge, sizeof(CT_DWORD));
            break;
		}
		/*else if (wOption == 2)
		{
			std::cout << "which game server do you want dismiss ?" << std::endl;
			CT_DWORD dwServerID;
			std::cin >> dwServerID;

			MSG_M2CS_DismissPRoom dismissPRoom;
			dismissPRoom.dwServerID = dwServerID;
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_MCS_MAIN, SUB_M2CS_DISMISS_SERVER_PROOM, &dismissPRoom, sizeof(dismissPRoom));
			break;
		}
		else if (wOption == 3)
		{
			std::cout << "which room num do you want dismiss ?" << std::endl;
			CT_DWORD dwRoomNum;
			std::cin >> dwRoomNum;

			MSG_M2CS_DismissPRoom_ByRoomNum dismissPRoom;
			dismissPRoom.dwRoomNum = dwRoomNum;
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_MCS_MAIN, SUB_M2CS_DISMISS_SERVER_PROOM_BY_ROOMNUM, &dismissPRoom, sizeof(dismissPRoom));
			break;
		}
		else if (wOption == 4)
		{
			std::cout << "which user id do you want remove ?" << std::endl;
			CT_DWORD dwUserID;
			std::cin >> dwUserID;

			MSG_M2CS_RemoveUserOutCache removeUserCache;
			removeUserCache.dwUserID = dwUserID;
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_MCS_MAIN, SUB_M2CS_REMOVE_USER_OUT_CACHE, &removeUserCache, sizeof(removeUserCache));
			break;
		}*/
		else
		{
			std::cout << "input error";
		}
	}
    if (wOption == 3 || wOption == 4)
    {
        SelectOperation();
    }

}