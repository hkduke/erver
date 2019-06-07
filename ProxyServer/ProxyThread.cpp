#include "stdafx.h"
#include "ProxyThread.h"
#include "NetModule.h"
#include "ServerCfg.h"
#include "ServerMgr.h"
#include "CMD_Plaza.h"
#include "CMD_ShareMsg.h"
#include "CMD_Game.h"
#include "CMD_Customer.h"
#include "CMD_ConFish.h"
#include "Utility.h"

//中心服务器连接
extern CNetConnector *pNetCenter;
extern CNetConnector *pNetDB;

//定时器
#define		IDI_CONNECT_CENTER				(100)
#define		IDI_CONNECT_DBSERVER			(101)
#define		IDI_CONNECT_CUSTOMERSERVER		(102)
#define 	IDI_CENTER_HEART_BEAT			(103)		//proxy和中心服的心跳
#define		IDI_CONNECT_SERVER				(1000)		//其他定时器ID不能超过这个ID

#define		TIME_CONNECT_CENTER				(3000)
#define		TIME_CONNECT_SERVER				(3000)
#define		TIME_CONNECT_DBSERVER			(3000)
#define		TIME_CONNECT_CUSTOMERSERVER		(3000)
#define 	TIME_CENTER_HEART_BEAT			(3*60*1000)	 //3分钟做一次心跳(中心服15分钟没有收到心跳则认为玩家已经离线)

CT_DWORD CProxyThread::m_dwServerLinkCount = 0;
CProxyThread::CProxyThread(void)
	//: m_redisClient(CServerCfg::m_RedisAddress, CServerCfg::m_RedisConTimeOut, CServerCfg::m_RedisRwTimeOut)
	: m_bInitServer(false)
{
	m_mapClientSockIp.clear();
	m_mapGSUserInfo.clear();
	m_mapUserSock.clear();
	m_mapHallUser.clear();
}

CProxyThread::~CProxyThread(void)
{
}

CT_VOID CProxyThread::OnTCPSocketLink(CNetConnector* pConnector)
{
	if (pConnector == NULL)
	{
		return;
	}

	++m_dwServerLinkCount;
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

		if (m_dwServerLinkCount < 10000)
		{
			LOG(INFO) << "connect to center server succ.";
		}
	}
	else if (connectParam.conType == CONNECT_LOGIN)
	{
		//跟登录服务器绑定
		GoLoginServerBind(pConnector);

		if (m_dwServerLinkCount < 10000)
		{
			LOG(INFO) << "connect to login server succ, server id:" << connectParam.dwServerID;
		}
	}
	else if (connectParam.conType == CONNECT_GAME)
	{
		//跟游戏服务器绑定
		GoGameServerBind(pConnector);

		if (m_dwServerLinkCount < 10000)
		{
			LOG(INFO) << "connect to game server succ, server id:" << connectParam.dwServerID;
		}
	}
	else if (connectParam.conType == CONNECT_DB)
	{
		if (m_dwServerLinkCount < 10000)
			LOG(INFO) << "connect to db server succ";
	}
	else if (connectParam.conType == CONNECT_CUSTOMER)
	{
		//跟客服服务器绑定
		GoCustomerServerBind(pConnector);

		if (m_dwServerLinkCount < 10000)
			LOG(INFO) << "connect to customer server succ";
	}
}

CT_VOID CProxyThread::OnTCPSocketShut(CNetConnector* pConnector)
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
	else if (connectParam.conType == CONNECT_LOGIN || connectParam.conType == CONNECT_GAME)
	{
		CT_BOOL bExist = CServerMgr::get_instance().IsExistServer(connectParam.dwServerID);
		if (!bExist)
		{
			CNetModule::getSingleton().DeleteConnector(pConnector);
			SAFE_DELETE_PTR(pConnector);
			return;
		}

		std::string strServerName = "login server";
		if (connectParam.conType == CONNECT_GAME)
		{
			strServerName = "game server";
			CloseUserSockInThisGS(pConnector->GetSocket());
		}
		LOG(WARNING) << strServerName << " is disconnect, after " << TIME_CONNECT_SERVER / 1000 << "s will be reconnect!, server id: " << connectParam.dwServerID;
		CNetModule::getSingleton().SetTimer(IDI_CONNECT_SERVER+connectParam.dwServerID, TIME_CONNECT_SERVER, this, false);
	}
	else if (connectParam.conType == CONNECT_DB)
	{
		LOG(WARNING) << "db server is disconnect, after " << TIME_CONNECT_DBSERVER / 1000 << "s will be reconnect!";
		CNetModule::getSingleton().SetTimer(IDI_CONNECT_DBSERVER, TIME_CONNECT_DBSERVER, this, false);
	}
	else if (connectParam.conType == CONNECT_CUSTOMER)
	{
		CT_BOOL bExist = CServerMgr::get_instance().IsExistCustomerServer(connectParam.dwServerID);
		if (!bExist)
		{
			CNetModule::getSingleton().DeleteConnector(pConnector);
			SAFE_DELETE_PTR(pConnector);
			return;
		}
		LOG(WARNING) << "customer server is disconnect, after " << TIME_CONNECT_CUSTOMERSERVER / 1000 << "s will be reconnect!";
		CNetModule::getSingleton().SetTimer(IDI_CONNECT_CUSTOMERSERVER, TIME_CONNECT_CUSTOMERSERVER, this, false);
	}
}

CT_VOID CProxyThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{
	m_mapClientSockIp.insert(std::make_pair(pSocket, 0));
}

CT_VOID CProxyThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
	/*auto itAndroid = m_mapAndroidSock.find(pSocket);
	if (itAndroid != m_mapAndroidSock.end())
	{
		m_mapAndroidSock.erase(itAndroid);
	}*/

	auto itClient = m_mapClientSockIp.find(pSocket);
	if (itClient != m_mapClientSockIp.end())
	{
		m_mapClientSockIp.erase(itClient);
	}

	auto itUserSock = m_mapUserSock.find(pSocket);
	if (itUserSock != m_mapUserSock.end())
	{
		CT_DWORD dwUserID = itUserSock->second;

		GoLoginServerLogout(dwUserID);

		/* 改动到登录服去删除这些消息 2017年9月11日 17:44:28

		//删除玩家的session信息
		DelUserSession(dwUserID);

		//删除玩家PS信息
		DelProxyServerInfo(dwUserID);

		//设置玩家帐号信息的过期时间
		SetAccountInfoExpire(dwUserID);

		//设置救济金信息过期时间
		SetBenefitExpire(dwUserID);

		//设置玩家金币信息过期时间
		SetScoreInfoExpire(dwUserID);

		//删除玩家大局信息
		DelGameRoundCount(dwUserID);
		*/

		auto itUserInfo = m_mapGSUserInfo.find(dwUserID);
		if (itUserInfo != m_mapGSUserInfo.end())
		{
			SendUserOffLine(itUserInfo->second.pGSSocket, dwUserID);
			m_mapGSUserInfo.erase(itUserInfo);
		}

		//找到这个玩家大厅信息
		auto itHall = m_mapHallUser.find(dwUserID);
		if (itHall != m_mapHallUser.end())
		{
			//UpdateUserCount(SUB_P2CS_UNREGISTER_HALL_USER, itHall->second.wGameID);
			UnRegisterUser(itHall->second.cbPlatformId, dwUserID);
			m_mapHallUser.erase(itHall);
		}

		m_mapUserSock.erase(itUserSock);
	}
}


#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif
struct sTest
{
	unsigned int nIndex;
	char szBuf[64];
};

struct sPGTest : public sTest
{
	CT_UINT64 uClientAddr;
};
#define TEST_CMD 10000


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

//网络bit数据到来
CT_VOID CProxyThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (pSocket == NULL)
	{
		return;
	}

	switch (pMc->dwMainID)
	{
	case TEST_CMD:
		{
			sTest* pTest = (sTest*)pBuf;
			//CNetModule::getSingleton().Send(pSocket, pMc->dwMainID, pMc->dwSubID, pTest, wLen);
			acl::string key;
			key.format("%u", pTest->nIndex);

			acl::string value;
			value.format("%u", pTest->nIndex);
			//m_redis.set(key.c_str(), value.c_str());

			stGameServerInfo* pGSInfo = CServerMgr::get_instance().FindGameServer(50);
			if (pGSInfo)
			{
				sPGTest pgTest;
				pgTest.nIndex = pTest->nIndex;
				_snprintf_info(pgTest.szBuf, sizeof(pgTest.szBuf), "%s", pTest->szBuf);
				pgTest.uClientAddr = (CT_UINT64)pSocket;
				CNetModule::getSingleton().Send(pGSInfo->pConnector->GetSocket(), pMc->dwMainID, pMc->dwSubID, &pgTest, sizeof(sPGTest));
			}
		}
		break;
	case TEST_CMD + 1:
		{
			sPGTest* pTest = (sPGTest*)pBuf;
			acl::aio_socket_stream* pClientScoket = (acl::aio_socket_stream*)pTest->uClientAddr;
			sTest test;
			test.nIndex = pTest->nIndex;
			_snprintf_info(test.szBuf, sizeof(test.szBuf), "%s", pTest->szBuf);
			CNetModule::getSingleton().Send(pClientScoket, pMc->dwMainID, pMc->dwSubID, &test, sizeof(sTest));
		}
		break;
	case MSG_PCS_MAIN:
	{
		OnCenterServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_PLS_MAIN:
	{
		OnLoginServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_PGS_MAIN:
	{
		OnGameServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_LOGIN_MAIN:
	{
		OnClientLoginMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_FRAME_MAIN:
	{
		OnClientFrameMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_GAME_MAIN:
	case MDM_GF_GAME:
	case MSG_FISH_GAME_MAIN:
	{
		OnClientGameMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_GROUP_MAIN:
	{
		OnClientGroupMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_PROXY_MAIN:
	{
		OnProxyMainMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_QUERY_MAIN:
	{
		OnClientQueryMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_HONGBAO_MAIN:
	{
		OnClientHongBaoMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_TASK_MAIN:
	{
		OnClientTaskMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_RANK_MAIN:
	{
		OnClientRankMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_CONFIG_MSG_MAIN:
	{
		OnClientRechargeMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_CUSTOMER_MAIN:
	{
		OnClientCustomerMain(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_WEALTHGODCOMING_MAIN:
	{
		OnClientWealthGodComing(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_DUOBAO_MAIN:
	{
		OnClientDuoBaoMsg(pMc, pBuf, wLen, pSocket);
	}
	break;
	case MSG_REDPACKET_MAIN:
	{
		OnClientRedPacketMsg(pMc, pBuf, wLen, pSocket);
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
CT_VOID CProxyThread::OnTimer(CT_DWORD dwTimerID)
{
	switch (dwTimerID)
	{
	case IDI_CONNECT_CENTER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetCenter);
	}
	break;
	case IDI_CONNECT_DBSERVER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetDB);
	}
	break;
	case IDI_CONNECT_CUSTOMERSERVER:
	{
		CServerMgr::get_instance().CheckDisconnectCustomerServer();
	}
	break;
	//case IDI_CONNECT_SERVER:
	//{
	//	CServerMgr::get_instance().CheckDisconnectServer();
	//}
	//break;
	case IDI_CENTER_HEART_BEAT:
		{
			SendHeartBeatToCenter();
		}
		break;
	default:
		break;
	}

	if (dwTimerID >= IDI_CONNECT_SERVER)
	{
		CT_DWORD dwServerID = dwTimerID - IDI_CONNECT_SERVER;
		CServerMgr::get_instance().CheckDisconnectServer(dwServerID);
	}
}

//打开
CT_VOID CProxyThread::Open()
{
}

CT_VOID CProxyThread::GoCenterServerRegister()
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CMD_ProxyServer_Info registerProxy;
		registerProxy.dwServerID = CServerCfg::m_nServerID;
		_snprintf_info(registerProxy.szServerIP, sizeof(registerProxy.szServerIP), "%s", CServerCfg::m_LocalAddress);
		_snprintf_info(registerProxy.szServerName, sizeof(registerProxy.szServerName), "%s", CServerCfg::m_ServerName);
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_PCS_MAIN, SUB_P2CS_REGISTER, &registerProxy, sizeof(registerProxy));
		return;
	}
	LOG(WARNING) << "go center server register failed, center net is not running...";
}

CT_VOID CProxyThread::GoLoginServerBind(CNetConnector *pConnector)
{
	if (pConnector != NULL && pConnector->IsRunning())
	{
		PS_BindData bindData;
		bindData.dwProxyServerID = CServerCfg::m_nServerID;
		CNetModule::getSingleton().Send(pConnector->GetSocket(), MSG_PLS_MAIN, SUB_P2LS_BIND, &bindData, sizeof(PS_BindData));
		return;
	}

	LOG(WARNING) << "bind log server failed, connector is not running...";
}

CT_VOID CProxyThread::GoGameServerBind(CNetConnector *pConnector)
{
	if (pConnector != NULL && pConnector->IsRunning())
	{
		PS_BindData bindData;
		bindData.dwProxyServerID = CServerCfg::m_nServerID;
		bindData.dwProxyType = CServerCfg::m_nProxyType;
		CNetModule::getSingleton().Send(pConnector->GetSocket(), MSG_PGS_MAIN, SUB_P2GS_BIND, &bindData, sizeof(PS_BindData));
		return;
	}

	LOG(WARNING) << "bind game server failed, connector is not running...";
}

//跟客服服务器绑定
CT_VOID CProxyThread::GoCustomerServerBind(CNetConnector *pConnector)
{
	if (pConnector != NULL && pConnector->IsRunning())
	{
		PS_BindData bindData;
		bindData.dwProxyServerID = CServerCfg::m_nServerID;
		bindData.dwProxyType = CServerCfg::m_nProxyType;
		CNetModule::getSingleton().Send(pConnector->GetSocket(), MSG_CUSTOMERPS_MAIN, SUB_PS2CUSTOMER_BIND, &bindData, sizeof(PS_BindData));
		return;
	}

	LOG(WARNING) << "bind customer server failed, connector is not running...";
}

CT_VOID CProxyThread::InitServer()
{
	//m_redisClient.set_password(CServerCfg::m_RedisPwd);
	//m_redis.set_client(&m_redisClient);
	CNetModule::getSingleton().SetTimer(IDI_CENTER_HEART_BEAT, TIME_CENTER_HEART_BEAT, this, true);
}

CT_VOID CProxyThread::OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CS2P_LOGIN_INSERT:
	{
		if (wDataSize % sizeof(CMD_LoginServer_Info) != 0)
		{
			return;
		}

		CT_DWORD wCount = wDataSize / sizeof(CMD_LoginServer_Info);
		CMD_LoginServer_Info* pLoginInfo = (CMD_LoginServer_Info*)pData;
		for (CT_WORD i = 0; i < wCount; ++i)
		{
			CServerMgr::get_instance().AddLoginServerList(pLoginInfo++);
		}
	}
	break;
	case SUB_CS2P_LOGIN_FINISH:
	{
		LOG(INFO) << "get login server list finish!";
	}
	break;
	case SUB_CS2P_LOGIN_REMOVE:
	{
		if (wDataSize != sizeof(CMD_LoginServer_Info))
			return;
		
		CMD_LoginServer_Info* pLoginInfo = (CMD_LoginServer_Info*)pData;
		CServerMgr::get_instance().DeleteLoginServer(pLoginInfo);
	}
	break;
	case SUB_CS2P_SERVER_INSERT:
	{
		if (wDataSize % sizeof(CMD_GameServer_Info) != 0)
		{
			return;
		}

		CT_DWORD wCount = wDataSize / sizeof(CMD_GameServer_Info);
		CMD_GameServer_Info* pGameInfo = (CMD_GameServer_Info*)pData;
		for (CT_WORD i = 0; i < wCount; ++i)
		{
			CServerMgr::get_instance().AddGameServerList(pGameInfo++);
		}
	}
	break;
	case SUB_CS2P_SERVER_FINISH:
	{
		LOG(INFO) << "get game server list finish!";
	}
	break;
	case SUB_CS2P_SERVER_REMOVE:
	{
		if (wDataSize != sizeof(CMD_GameServer_Info))
			return;

		CMD_GameServer_Info* pGameInfo = (CMD_GameServer_Info*)pData;
		CServerMgr::get_instance().DeleteGameServer(pGameInfo);
	}
	break;
	case SUB_CS2P_CUSTOMER_INSERT:
	{
		if (wDataSize % sizeof(CMD_CustomerServer_Info) != 0)
		{
			return;
		}

		CT_DWORD wCount = wDataSize / sizeof(CMD_CustomerServer_Info);
		CMD_CustomerServer_Info* pCustomerInfo = (CMD_CustomerServer_Info*)pData;
		for (CT_WORD i = 0; i < wCount; ++i)
		{
			CServerMgr::get_instance().AddCustomerServerList(pCustomerInfo++);
		}
	}
	break;
	case SUB_CS2P_CUSTOMER_FINISH:
	{
		LOG(INFO) << "get customer server list finish!";
	}
	break;
	case SUB_CS2P_CUSTOMER_REMOVE:
	{
		if (wDataSize != sizeof(CMD_CustomerServer_Info))
			return;

		CMD_CustomerServer_Info* pCustomerInfo = (CMD_CustomerServer_Info*)pData;
		CServerMgr::get_instance().DeleteCustomerServer(pCustomerInfo);
	}
	break;
	case SUB_CS2P_CREATE_PRIVATE_ROOM:
	{
		if (wDataSize != sizeof(MSG_CS2P_CreatePrivateRoom))
			return;

		MSG_CS2P_CreatePrivateRoom* pCreatePrivateRoom = (MSG_CS2P_CreatePrivateRoom*)pData;
		if (pCreatePrivateRoom == NULL)
		{
			return;
		}

		acl::aio_socket_stream* pClientSock = (acl::aio_socket_stream*)pCreatePrivateRoom->uClientSock;
		if (m_mapClientSockIp.find(pClientSock) == m_mapClientSockIp.end())
		{
			return;
		}

		MSG_SC_CreatePrivateRoom createPrivateRoom;
		createPrivateRoom.dwRoomNum = pCreatePrivateRoom->dwRoomNum;
		createPrivateRoom.wGameID = pCreatePrivateRoom->wGameID;
		createPrivateRoom.wKindID = pCreatePrivateRoom->wKindID;
		CNetModule::getSingleton().Send(pClientSock, MSG_FRAME_MAIN, SUB_S2C_CREATE_PROOM, &createPrivateRoom, sizeof(createPrivateRoom));
	}
	break;
	case SUB_CS2P_CREATE_PRIVATE_FAIL:
	{
		if (wDataSize != sizeof(MSG_CS2P_CreatePrivateRoomFail))
		{
			return;
		}
		MSG_CS2P_CreatePrivateRoomFail* pCreateFail = (MSG_CS2P_CreatePrivateRoomFail*)pData;
		if (pCreateFail == NULL)
		{
			return;
		}

		acl::aio_socket_stream* pClientSock = (acl::aio_socket_stream*)pCreateFail->uClientSock;
		if (m_mapClientSockIp.find(pClientSock) == m_mapClientSockIp.end())
		{
			return;
		}
		SendGameErrorCode(pClientSock, MSG_FRAME_MAIN, SUB_S2C_CREATE_PROOM_FAIL, pCreateFail->cbErrorCode);
	}
	break;
	case SUB_CS2P_SYN_USER_ONLINE_COUNT:
	{
		if (wDataSize != sizeof(CMD_GameServer_OnlineCount))
		{
			return;
		}
		CMD_GameServer_OnlineCount* pGameOnlineCount = (CMD_GameServer_OnlineCount*)pData;
		stGameServerInfo* pGameInfo = CServerMgr::get_instance().FindGameServer(pGameOnlineCount->dwServerID);
		if (pGameInfo != NULL)
		{
			pGameInfo->dwOnlineCount = pGameOnlineCount->dwOnlineCount;
		}	
	}
	break;
	case SUB_CS2P_UPDATE_GS_STATE:
	{
		if (wDataSize != sizeof(MSG_CS2P_Update_GS_State))
		{
			return;
		}
		MSG_CS2P_Update_GS_State* pState = (MSG_CS2P_Update_GS_State*)pData;
		stGameServerInfo* pGameInfo = CServerMgr::get_instance().FindGameServer(pState->dwServerID);
		if (pGameInfo != NULL)
		{
			pGameInfo->cbState = pState->cbState;
		}
	}
	break;
    case SUB_CS2P_ADD_GROUP_PROOM:
    {
        if (wDataSize != sizeof(MSG_CS2P_Add_GroupPRoom))
        {
            return;
        }

        MSG_CS2P_Add_GroupPRoom* pAddGroupPRoom = (MSG_CS2P_Add_GroupPRoom*)pData;
        m_mapGroupPRoom[pAddGroupPRoom->dwRoomNum] = pAddGroupPRoom->dwServerID;
    }
    break;
    case SUB_CS2P_REMOVE_GROUP_PROOM:
    {
        if (wDataSize != sizeof(CT_DWORD))
        {
            return;
        }

        CT_DWORD* pRemoveGroupPRoom = (CT_DWORD*)pData;
        auto it = m_mapGroupPRoom.find(*pRemoveGroupPRoom);
        if (it != m_mapGroupPRoom.end())
        {
            m_mapGroupPRoom.erase(it);
        }
    }
    break;
	default:
		break;
	}
}

CT_VOID CProxyThread::OnLoginServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_LS2P_USER_LOGIN_SUCC:
	{
		if (wDataSize != sizeof(MSG_LS2P_UserLoginSucc))
		{
			return;
		}

		MSG_LS2P_UserLoginSucc* pLoginSuc = (MSG_LS2P_UserLoginSucc*)pData;
		if (pLoginSuc == NULL)
		{
			return;
		}

		//检测玩家是否掉线
		acl::aio_socket_stream* pSocket = (acl::aio_socket_stream*)pLoginSuc->dwNetAddr;
		if (m_mapClientSockIp.find(pSocket) == m_mapClientSockIp.end())
		{
			return;
		}

		//记录玩家socket
		m_mapUserSock[pSocket] = pLoginSuc->dwUserID;
		//记录玩家的大厅信息
		stClientUserHallInfo userHallInfo;
		//userHallInfo.wGameID = pLoginSuc->wGameID;
		userHallInfo.cbPlatformId = pLoginSuc->cbPlatformId;
		userHallInfo.pClientSocket = pSocket;
		m_mapHallUser[pLoginSuc->dwUserID] = userHallInfo;

		//上报中心服人数更新
		RegisterUser(pLoginSuc);

		break;
	}
	case SUB_LS2P_KICK_USER:
	{
		if (wDataSize != sizeof(MSG_LS2P_KickUser))
		{
			return;
		}

		MSG_LS2P_KickUser* pKickUser = (MSG_LS2P_KickUser*)pData;
		if (pKickUser == NULL)
		{
			return;
		}
		acl::aio_socket_stream* pSocket = (acl::aio_socket_stream*)pKickUser->dwNetAddr;
		auto it = m_mapUserSock.find(pSocket);
		if (it != m_mapUserSock.end())
		{
			//找到这个玩家
			if (it->second == pKickUser->dwUserID)
			{
				//在游戏中
				auto itUser = m_mapGSUserInfo.find(it->second);
				if (itUser != m_mapGSUserInfo.end())
				{
					m_mapGSUserInfo.erase(itUser);
				}
			}

			//找到这个玩家大厅信息
			auto itHall = m_mapHallUser.find(pKickUser->dwUserID);
			if (itHall != m_mapHallUser.end())
			{
				//被T的人，不需要更新人数？
				//UnRegisterUser(itHall->second.wGameID, pKickUser->dwUserID);
				m_mapHallUser.erase(itHall);
			}

			//TODO: 下发玩家被T数据
			if (pKickUser->bSendToClient)
			{
				MSG_SC_LongDistance_Login longDistanceLogin;
				longDistanceLogin.dwUserID = pKickUser->dwUserID;
				CNetModule::getSingleton().Send(pSocket, MSG_LOGIN_MAIN, SUB_SC_LONG_DISTANCE_LOGIN, &longDistanceLogin, sizeof(longDistanceLogin));
			}

			//关闭玩家连接
			m_mapUserSock.erase(it);
			//暂时不关闭连接，因为关闭连接，客户端会马上自动重连
			//CNetModule::getSingleton().CloseSocket(pSocket);
		}
		break;
	}
	default:
		break;
	}
}

CT_VOID CProxyThread::OnGameServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_GS2P_ADD_USER:
	{
		if (sizeof(MSG_GS2P_AddGameUserInfo) != wDataSize)
		{
			return;
		}

		MSG_GS2P_AddGameUserInfo* pUser = (MSG_GS2P_AddGameUserInfo*)pData;
		acl::aio_socket_stream* pClientSock = (acl::aio_socket_stream*)pUser->uClientNetAddr;
		//如果玩家连接不存在则通知gameserver玩家离线
		if (m_mapClientSockIp.find(pClientSock) == m_mapClientSockIp.end())
		{
			SendUserOffLine(pSocket, pUser->dwUserID);
			return;
		}

		stGsServerUserBaseInfo userInfo;
		userInfo.dwUserID = pUser->dwUserID;
		userInfo.dwServerID = pUser->dwServerID;
		userInfo.pClientSocket = (acl::aio_socket_stream*)pUser->uClientNetAddr;
		userInfo.pGSSocket = pSocket;

		m_mapGSUserInfo[pUser->dwUserID] = userInfo;
		m_mapUserSock[pClientSock] = pUser->dwUserID;
		break;
	}
	case SUB_GS2P_DEL_USER:
	{
		if (sizeof(MSG_GS2P_DelGameUser) != wDataSize)
		{
			return;
		}

		MSG_GS2P_DelGameUser* pDelUser = (MSG_GS2P_DelGameUser*)pData;
		auto itUserInfo = m_mapGSUserInfo.find(pDelUser->dwUserID);
		if (itUserInfo != m_mapGSUserInfo.end())
		{
			/*auto itUserSock = m_mapUserSock.find(itUserInfo->second.pClientSocket);
			if (itUserSock != m_mapUserSock.end())
			{
				m_mapUserSock.erase(itUserSock);
			}*/
			m_mapGSUserInfo.erase(itUserInfo);
		}

		break;
	}
	case SUB_GS2P_CLOSE_USER_CONNECT:
	{
		if (sizeof(MSG_GS2P_CloseConnect) != wDataSize)
		{
			return;
		}

		MSG_GS2P_CloseConnect* pCloseConnect = (MSG_GS2P_CloseConnect*)pData;
		auto itHallUser = m_mapHallUser.find(pCloseConnect->dwUserID);
		if (itHallUser != m_mapHallUser.end())
		{
			CNetModule::getSingleton().CloseSocket(itHallUser->second.pClientSocket);
		}
		/*auto itUserInfo = m_mapGSUserInfo.find(pCloseConnect->dwUserID);
		if (itUserInfo != m_mapGSUserInfo.end())
		{
			CNetModule::getSingleton().CloseSocket(itUserInfo->second.pClientSocket);
		}*/

		break;
	}
	default:
		break;
	}

}

CT_VOID CProxyThread::OnProxyMainMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_PROXY_FASTSENDMSGTOC:
	{
		CT_DWORD dwHeadSize = sizeof(MSG_GameMsgDownHead);
		if (wDataSize < dwHeadSize)
		{
			return;
		}
		MSG_GameMsgDownHead* pMsgHead = (MSG_GameMsgDownHead*)pData;
		if (pMsgHead == NULL)
		{
			return;
		}

		acl::aio_socket_stream* pClientSocket = (acl::aio_socket_stream*)pMsgHead->uValue1;
		auto it = m_mapClientSockIp.find(pClientSocket);
		if (it == m_mapClientSockIp.end())
		{
			LOG(WARNING) << "client socket not found" << ", main id: " << pMsgHead->dwMainID << ", sub id: " << pMsgHead->dwSubID;
			return;
		}
		
		if (pMsgHead->dwMainID == MSG_LOGIN_MAIN && pMsgHead->dwSubID == SUB_SC_LOGIN_SUCCESS)
		{
			//登录时，如果IP不相同，说明不是同一个登录，不下发消息
			if (it->second != pMsgHead->dwValue2)
				return;
		}

		CT_CHAR* pBuf = (CT_CHAR*)pData;
		CNetModule::getSingleton().Send(pClientSocket, pMsgHead->dwMainID, pMsgHead->dwSubID, pBuf + dwHeadSize, wDataSize - dwHeadSize);
	}
	break;
	case SUB_PROXY_FASTSENDMSGTOC_NODATA:
	{
		CT_DWORD dwHeadSize = sizeof(MSG_GameMsgDownHead);
		if (wDataSize != dwHeadSize)
		{
			return;
		}
		MSG_GameMsgDownHead* pMsgHead = (MSG_GameMsgDownHead*)pData;
		if (pMsgHead == NULL)
		{
			return;
		}

		acl::aio_socket_stream* pClientSocket = (acl::aio_socket_stream*)pMsgHead->uValue1;
		if (m_mapClientSockIp.find(pClientSocket) == m_mapClientSockIp.end())
		{
			LOG(WARNING) << "client socket not found";
			return;
		}

		CNetModule::getSingleton().Send(pClientSocket, pMsgHead->dwMainID, pMsgHead->dwSubID);
	}
	break;
	case SUB_PROXY_FASTSENDMSGTOC_BY_USERID:
	{
		CT_DWORD dwHeadSize = sizeof(MSG_GameMsgDownHead);
		if (wDataSize < dwHeadSize)
		{
			return;
		}
		MSG_GameMsgDownHead* pMsgHead = (MSG_GameMsgDownHead*)pData;
		if (pMsgHead == NULL)
		{
			return;
		}

		auto it = m_mapHallUser.find(pMsgHead->dwValue2);
		if (it == m_mapHallUser.end())
		{
			LOG(WARNING) << "can not find hall user!, user id: " << pMsgHead->dwValue2 << ", main id: " << pMsgHead->dwMainID << ", sub id: " << pMsgHead->dwSubID;
			return;
		}

		acl::aio_socket_stream* pClientSocks = it->second.pClientSocket;

		CT_CHAR* pBuf = (CT_CHAR*)pData;
		if (wDataSize - dwHeadSize > 0)
		{
			CNetModule::getSingleton().Send(pClientSocks, pMsgHead->dwMainID, pMsgHead->dwSubID, pBuf + dwHeadSize, wDataSize - dwHeadSize);
		}
		else
		{
			CNetModule::getSingleton().Send(pClientSocks, pMsgHead->dwMainID, pMsgHead->dwSubID);
		}
	}
	break;
	case SUB_PROXY_FASTSENDMSGTO_ANDROID_C:
	{
		CT_DWORD dwHeadSize = sizeof(MSG_GameMsgDownHead);
		if (wDataSize < dwHeadSize)
		{
			return;
		}
		MSG_GameMsgDownHead* pMsgHead = (MSG_GameMsgDownHead*)pData;
		if (pMsgHead == NULL)
		{
			return;
		}

		for (auto& it : m_mapAndroidSock)
		{
			auto itUserSock = m_mapUserSock.find(it.first);
			if (itUserSock != m_mapUserSock.end() && m_mapGSUserInfo.find(itUserSock->second) == m_mapGSUserInfo.end())
			{
			
				acl::aio_socket_stream* pClientSocks = it.first;
				CT_CHAR* pBuf = (CT_CHAR*)pData;
				if (wDataSize - dwHeadSize > 0)
				{
					CNetModule::getSingleton().Send(pClientSocks, pMsgHead->dwMainID, pMsgHead->dwSubID, pBuf + dwHeadSize, wDataSize - dwHeadSize);
				}
				else
				{
					CNetModule::getSingleton().Send(pClientSocks, pMsgHead->dwMainID, pMsgHead->dwSubID);
				}
				return;
			}	
		}
	}
	break;
	default:
		break;
	}
}

CT_VOID CProxyThread::OnClientLoginMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CS_ACCOUNTS_LOGIN:
	case SUB_CS_ACCOUNTS_REGISTER:
	{
		OnAccountRegister(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_ACCOUNTS_LOGIN_REGISTER:
	{
		OnAccountLoginOrRegister(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_ACCUNNTS_SIGN_IN:
	{
		OnUserSignIn(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_PULL_MAIL:
	{
		OnUserPullMail(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_READ_MAIL:
	{
		OnUserReadMail(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_DELETE_MAIL:
	{
		OnUserDelMail(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_GET_BENEFIT:
	{
		OnUserGetBenefit(wSubCmdID, pData, wDataSize, pSocket);
		break;;
	}
	case SUB_CS_EXCHANGE_SCORE:
	{
		OnUserExchangeScore(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_BIND_PROMOTER:
	{
		OnUserBindPromoter(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_BIND_MOBILE:
	{
		OnUserBindMobile(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_PAY_SCORE_BY_APPLY:
	{
		OnUserPayScoreByApply(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_SET_BANK_PWD:
	case SUB_CS_MODIFY_BANK_PWD:
	case SUB_CS_SAVE_SCORE_TO_BANK:
	case SUB_CS_TAKE_SCORE_FROM_BANK:
	{
		OnUserBankOperate(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_GET_VERIFICATION_CODE:
	{
		OnUserGetVerificationCode(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_RESET_PWD:
	{
		OnUserResetPwd(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_MODIFY_PERSONAL_INFO:
	{
		OnUserModifyPersonalInfo(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	case SUB_CS_ANDROID_CONNECT:
	{
		OnAndroidConnect(wSubCmdID, pData, wDataSize, pSocket);
		break;
	}
	default: //通用处理消息
	{
		OnTranspondToLoginServer(wSubCmdID, pData, wDataSize, pSocket);
	}
	break;
	}
}

CT_VOID CProxyThread::OnClientQueryMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	auto it = m_mapUserSock.find(pSocket);
	if (it == m_mapUserSock.end())
	{
		LOG(WARNING) << "query client request, can not find client socks.";
		return;
	}
	switch (pMc->dwSubID)
	{
	case SUB_CS_QUERY_PROOM_GAMEROUND_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_PRoom_GameRound))
		{
			return;
		}

		MSG_CS_Query_PRoom_GameRound* pQueryGameRound = (MSG_CS_Query_PRoom_GameRound*)pData;

		MSG_P2DB_Query_PRoom_GameRound queryGameRound;
		queryGameRound.dwUserID = pQueryGameRound->dwUserID;
		queryGameRound.wGameID = pQueryGameRound->wGameID;
		queryGameRound.wKindID = pQueryGameRound->wKindID;
		queryGameRound.uClientAddr = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_PROOM_GAMEROUND, &queryGameRound, sizeof(queryGameRound));
	}
	break;
	case SUB_CS_QUERY_PROOM_PLAYCOUNT_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_PRoom_PlayCount))
		{
			return;
		}

		MSG_CS_Query_PRoom_PlayCount* pPlayCount = (MSG_CS_Query_PRoom_PlayCount*)pData;
		MSG_P2DB_Query_PRoom_PlayCount queryPlayCount;
		queryPlayCount.uGameRoundID = pPlayCount->uGameRoundID;
		queryPlayCount.uClientAddr = (CT_UINT64)pSocket;
		queryPlayCount.dwUserID = pPlayCount->dwUserID;
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_PROOM_PALYCOUNT, &queryPlayCount, sizeof(queryPlayCount));
	}
	break;
	case SUB_CS_QUERY_USER_SCORE_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_UserScore_Info))
		{
			return;
		}

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_USER_SCORE_INFO, pData, wDataSize);
	}
	break;
	case SUB_CS_QUERY_USER_HONGBAO_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_UserHongBao_Info))
		{
			return;
		}

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_USER_HONGBAO_INFO, pData, wDataSize);
	}
	break;
	case SUB_CS_QUERY_GAME_RECORD:
	{
		if (wDataSize !=sizeof(MSG_CS_Query_GameRecord))
		{
			return;
		}
		
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_GAME_RECORD, pData, wDataSize);
	}
	break;
	case SUB_CS_QUERY_REPORT_GAME_RECORD:
	{
		if (wDataSize !=sizeof(MSG_CS_Query_Report_GameRecord))
		{
			return;
		}
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_REPORT_GAME_RECORD, pData, wDataSize);
	}
	break;
	case SUB_CS_QUERY_NORMAL_GAME_RECORD:
	{
		if (wDataSize !=sizeof(MSG_CS_Query_Normal_GameRecord))
		{
			return;
		}
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_NORMAL_GAME_RECORD, pData, wDataSize);
	}
	break;
    case SUB_CS_QUERY_HBSL_RECORD:
    {
        if (wDataSize != sizeof(MSG_CS_Query_Hbsl_Record))
        {
            return;
        }

        MSG_CS_Query_Hbsl_Record* pQueryRecord = (MSG_CS_Query_Hbsl_Record*)pData;
        auto it = m_mapHallUser.find(pQueryRecord->dwUserID);
        if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
        {
            CNetModule::getSingleton().CloseSocket(pSocket);
            return;
        }

        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_HBSL_RECORD, pData, wDataSize);
    }
    break;
	case SUB_CS_QUERY_HBSL_FA_RECORD:
	{
		if (wDataSize != sizeof(MSG_CS_Query_Hbsl_Record))
		{
			return;
		}

		MSG_CS_Query_Hbsl_Record* pQueryRecord = (MSG_CS_Query_Hbsl_Record*)pData;
		auto it = m_mapHallUser.find(pQueryRecord->dwUserID);
		if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
		{
			CNetModule::getSingleton().CloseSocket(pSocket);
			return;
		}

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_HBSL_FA_RECORD, pData, wDataSize);
	}
	break;
	case SUB_CS_QUERY_HBSL_WIN_COLOR_PRIZE_RECORD:
	{
		if (wDataSize != sizeof(MSG_CS_Query_Hbsl_Record))
		{
			return;
		}

		MSG_CS_Query_Hbsl_Record* pQueryRecord = (MSG_CS_Query_Hbsl_Record*)pData;
		auto it = m_mapHallUser.find(pQueryRecord->dwUserID);
		if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
		{
			CNetModule::getSingleton().CloseSocket(pSocket);
			return;
		}

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_PDB_MAIN, SUB_P2DB_QUERY_HBSL_WIN_COLOR_PRIZE_RECORD, pData, wDataSize);
	}
	break;
	}
}

CT_VOID CProxyThread::OnClientFrameMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (pMc->dwSubID)
	{
	/*
	case SUB_C2S_CREATE_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS_CreatePrivateRoom))
		{
			return;
		}

		MSG_CS_CreatePrivateRoom *pCreateRoom = (MSG_CS_CreatePrivateRoom*)pData;
		if (pCreateRoom == NULL)
		{
			return;
		}

		//判断玩家的链接是否存在
		if (IsExistUserSession(pCreateRoom->dwUserID, pCreateRoom->szUUID) == false)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_CREATE_PROOM_FAIL, CREATE_PROOM_ERROR_SESSION);
			CNetModule::getSingleton().CloseSocket(pSocket);
			return;
		}

		//判断玩家是否已经有房间
		CT_DWORD dwRoomNum = 0;
		if (IsUserHasPrivateRoom(pCreateRoom->dwUserID, dwRoomNum) == true)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_CREATE_PROOM_FAIL, CREATE_PROOM_HAS_PROOM);
			return;
			//PrivateRoomInfo roomInfo;
			//if (GetPrivateRoomInfo(dwRoomNum, roomInfo) == true)
			//{
			//	MSG_SC_CreatePrivateRoom privateRoom;
			//	privateRoom.dwRoomNum = roomInfo.dwRoomNum;
			//	privateRoom.wGameID = roomInfo.wGameID;
			//	privateRoom.wKindID = roomInfo.wKindID;
			//	CNetModule::getSingleton().Send(pSocket, MSG_FRAME_MAIN, SUB_S2C_CREATE_PROOM, &privateRoom, sizeof(privateRoom));
			//	return;
			//}
		}

		MSG_PS_CreatePrivateRoom createPrivateRoom;
		createPrivateRoom.dwUserID = pCreateRoom->dwUserID;
		createPrivateRoom.wGameID = pCreateRoom->wGameID;
		createPrivateRoom.wKindID = pCreateRoom->wKindID;
		createPrivateRoom.wUserCount = pCreateRoom->wUserCount;
		createPrivateRoom.cbPlayCount = pCreateRoom->cbPlayCount;
		createPrivateRoom.wMaxHuxi = pCreateRoom->wMaxHuxi;
		createPrivateRoom.cbIsVip = pCreateRoom->cbIsVip;
		_snprintf_info(createPrivateRoom.szOtherParam, sizeof(createPrivateRoom.szOtherParam), "%s", pCreateRoom->szOtherParam);
		createPrivateRoom.uClientSock = (CT_INT64)pSocket;

		// 发送到中心服务器创建房间
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_PCS_MAIN, SUB_P2CS_CREATE_PRIVATE_ROOM, &createPrivateRoom, sizeof(createPrivateRoom));
		break;
	}
	case SUB_C2S_ENTER_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS_EnterPrivateRoom))
		{
			return;
		}

		MSG_CS_EnterPrivateRoom* pEnterRoom = (MSG_CS_EnterPrivateRoom*)pData;
		if (pEnterRoom == NULL)
		{
			return;
		}

		//判断玩家的链接是否存在
		if (IsExistUserSession(pEnterRoom->dwUserID, pEnterRoom->szUUID) == false)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_PROOM_ERROR_SESSION);
			CNetModule::getSingleton().CloseSocket(pSocket);
			return;
		}

		//判断玩家是否有自己的房间号
		CT_DWORD dwRoomNum = 0;
		if (IsUserHasPrivateRoom(pEnterRoom->dwUserID, dwRoomNum) == true)
		{
			if (dwRoomNum != pEnterRoom->dwRoomNum)
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_PROOM_HAS_OTHER_PROOM);
				return;
			}
		}

		PrivateRoomInfo roomInfo;
		if (GetPrivateRoomInfo(pEnterRoom->dwRoomNum, roomInfo) == false)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_PROOM_NOTFIND);
			return;
		}

		//if (roomInfo.wGameID != pEnterRoom->wCurGameID)
		//{
		//	SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_PROOM_NOT_THIS_GAME);
		//	return;
		//}

		//检查这个房间是否属于当前玩家登录的游戏
		//CT_WORD wGameID = GetUserLoginGameID(pEnterRoom->dwUserID);
		//if (wGameID != roomInfo.wGameID)
		//{
		//	SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_PROOM_NOT_THIS_GAME);
		//	return;
		//}

		stGameServerInfo* pGameServerInfo = CServerMgr::get_instance().FindGameServer(roomInfo.dwServerID);
		if (pGameServerInfo == NULL)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_PROOM_GAMENOEXIST);
			return;
		}

		MSG_PG_EnterPrivateRoom enterPrivateRoom;
		enterPrivateRoom.dwUserID = pEnterRoom->dwUserID;
		enterPrivateRoom.dLongitude = pEnterRoom->dLongitude;
		enterPrivateRoom.dLatitude = pEnterRoom->dLatitude;
		_snprintf_info(enterPrivateRoom.szLocation, sizeof(enterPrivateRoom.szLocation), "%s", pEnterRoom->szLocation);

		enterPrivateRoom.dwOwnerUserID = roomInfo.dwOwerUserID;
		enterPrivateRoom.dwRoomNum = pEnterRoom->dwRoomNum;
		enterPrivateRoom.wGameID = roomInfo.wGameID;
		enterPrivateRoom.wKindID = roomInfo.wKindID;
		enterPrivateRoom.wUserCount = roomInfo.wUserCount;
		enterPrivateRoom.wPlayCount = roomInfo.wPlayCount;
		//enterPrivateRoom.wEndLimit = roomInfo.wEndLimit;
		//enterPrivateRoom.wWinLimit = roomInfo.wWinLimit;
		enterPrivateRoom.cbIsVip = roomInfo.cbIsVip;
		//enterPrivateRoom.cbIsAuto = roomInfo.cbIsAuto;
		enterPrivateRoom.dwGroupID = roomInfo.dwGroupID;
		enterPrivateRoom.dwRecordID = roomInfo.dwRecordID;
		enterPrivateRoom.cbGroupType = roomInfo.cbGroupType;
		_snprintf_info(enterPrivateRoom.szOtherParam, sizeof(enterPrivateRoom.szOtherParam), "%s", roomInfo.szOtherParam);
		enterPrivateRoom.uValue1 = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pGameServerInfo->pConnector->GetSocket(), MSG_FRAME_MAIN, SUB_C2S_ENTER_PROOM, &enterPrivateRoom, sizeof(enterPrivateRoom));
		break;
	}*/

    case SUB_C2S_ENTER_PROOM:
    {
        if (wDataSize != sizeof(MSG_CS_EnterPrivateRoom))
        {
            return;
        }

        MSG_CS_EnterPrivateRoom* pEnterRoom = (MSG_CS_EnterPrivateRoom*)pData;
        if (pEnterRoom == NULL)
        {
            return;
        }

        //检测玩家是否在线
        auto it = m_mapHallUser.find(pEnterRoom->dwUserID);
        if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
        {
            SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_ERROR_SESSION);
            CNetModule::getSingleton().CloseSocket(pSocket);
            return;
        }

        auto itPRoom = m_mapGroupPRoom.find(pEnterRoom->dwRoomNum);
        if (itPRoom == m_mapGroupPRoom.end())
        {
            SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_PROOM_NOTFIND);
            return;
        }

        stGameServerInfo* pGameServerInfo = CServerMgr::get_instance().FindGameServer(itPRoom->second);
        if (pGameServerInfo == NULL)
        {
            SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_GAMENOEXIST);
            return;
        }

        MSG_PG_EnterGroupPRoom enterRoom;
        enterRoom.dwUserID = pEnterRoom->dwUserID;
        enterRoom.dwRoomNum = pEnterRoom->dwRoomNum;
        enterRoom.uValue1 = (CT_UINT64)pSocket;
        CNetModule::getSingleton().Send(pGameServerInfo->pConnector->GetSocket(), MSG_FRAME_MAIN, SUB_C2S_ENTER_PROOM, &enterRoom, sizeof(enterRoom));
        break;
    }
	case SUB_C2S_ENTER_ROOM:
	{
		if (wDataSize != sizeof(MSG_CS_EnterGame))
		{
			return;
		}

		MSG_CS_EnterGame * pEnterGame = (MSG_CS_EnterGame*)pData;

		//判断玩家的链接是否存在
		/*if (IsExistUserSession(pEnterGame->dwUserID, pEnterGame->szUUID) == false)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, ENTER_ROOM_ERROR_SESSION);
			return;
		}*/

		//检测玩家是否在线
		auto it = m_mapHallUser.find(pEnterGame->dwUserID);
		if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_ERROR_SESSION);
			CNetModule::getSingleton().CloseSocket(pSocket);
			return;
		}

		CT_BYTE cbServerState = 0;
		stGameServerInfo* pGameServerInfo = CServerMgr::get_instance().FindReqGameServer(pEnterGame->wGameID, pEnterGame->wKindID, pEnterGame->wRoomKindID, cbServerState);
		if (pGameServerInfo == NULL)
		{
			if (cbServerState == 0)
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_GAMENOEXIST);
			}
			else
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_SEAT_FULL);
			}
			return;
		}

		MSG_PG_EnterRoom enterRoom;
		enterRoom.dwUserID = pEnterGame->dwUserID;
		enterRoom.uValue1 = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pGameServerInfo->pConnector->GetSocket(), MSG_FRAME_MAIN, SUB_C2S_ENTER_ROOM, &enterRoom, sizeof(enterRoom));
		break;
	}
	case SUB_C2S_QUERY_SCORE_ROOM_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_QueryScoreRoomInfo))
		{
			return;
		}

		MSG_CS_QueryScoreRoomInfo * pRoomInfo = (MSG_CS_QueryScoreRoomInfo*)pData;

		//判断玩家的链接是否存在
		//if (IsExistUserSession(pRoomInfo->dwUserID, pRoomInfo->szUUID) == false)
		//{
		//	SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_ERROR_SESSION);
		//	CNetModule::getSingleton().CloseSocket(pSocket);
		//	return;
		//}

		//检测玩家是否在线
		auto it = m_mapHallUser.find(pRoomInfo->dwUserID);
		if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_ERROR_SESSION);
			CNetModule::getSingleton().CloseSocket(pSocket);
			return;
		}
		
		MSG_SC_QueryScoreRoomInfo roomInfo;
		//memset(&roomInfo, 0, sizeof(MSG_SC_QueryScoreRoomInfo));

		//roomInfo.uValue1 = (CT_UINT64)pSocket;
		//roomInfo.dwUserID = pRoomInfo->dwUserID;
		for (CT_BYTE i = 0; i < 4; ++i)
		{
			stGameServerInfo* pGameServerInfo = CServerMgr::get_instance().FindReqGameServerEx(pRoomInfo->wGameID, pRoomInfo->wKindID, i+1);
			if (pGameServerInfo == NULL)
			{
				continue;
			}

			roomInfo.wGameID = pGameServerInfo->wGameID;
			roomInfo.wKindID = pGameServerInfo->wKindID;
			roomInfo.cbRoomKindID[roomInfo.cbRoomKindCount] = (CT_BYTE)pGameServerInfo->wRoomKindID;
			roomInfo.dEnterMinScore[roomInfo.cbRoomKindCount] = pGameServerInfo->dwEnterMinScore*0.01f;
			roomInfo.dEnterMaxScore[roomInfo.cbRoomKindCount] = pGameServerInfo->dwEnterMaxScore*0.01f;
			roomInfo.dwPlayerCount[roomInfo.cbRoomKindCount] = pGameServerInfo->dwOnlineCount;
			roomInfo.dBaseScore[roomInfo.cbRoomKindCount] = pGameServerInfo->dwCellScore*0.01f;
			
			//房间状态
			if (pGameServerInfo->wGameID == GAME_BR)
			{
				if (pGameServerInfo->dwOnlineCount  >= 60)
				{
					roomInfo.cbRoomStatus[roomInfo.cbRoomKindCount] = 3;
				}
				else if (pGameServerInfo->dwOnlineCount >= 30)
				{
					roomInfo.cbRoomStatus[roomInfo.cbRoomKindCount] = 2;
				}
			}
			else
			{
				if (pGameServerInfo->dwOnlineCount >= 20)
				{
					roomInfo.cbRoomStatus[roomInfo.cbRoomKindCount] = 3;
				}
				else if (pGameServerInfo->dwOnlineCount >= 10)
				{
					roomInfo.cbRoomStatus[roomInfo.cbRoomKindCount] = 2;
				}
			}
			++roomInfo.cbRoomKindCount;

			/*printf("---------------------------------------------------------------\n");
			printf("dwEnterMinScore_%d = %d \n",i, roomInfo.dwEnterMinScore[i]);
			printf("dwEnterMaxScore_%d = %d \n", i, roomInfo.dwEnterMaxScore[i]);
			printf("dwPlayerCount_%d = %d \n", i, roomInfo.dwPlayerCount[i]); 
			printf("dwBaseScore_%d = %d \n", i, roomInfo.dwBaseScore[i]);*/
		}

		CNetModule::getSingleton().Send(pSocket, MSG_FRAME_MAIN, SUB_S2C_QUERY_SCORE_ROOM_INFO, &roomInfo, sizeof(roomInfo));
		break;
	}
	case SUB_C2S_ENTER_ROOM_EX:
	case SUB_C2S_ADD_WAIT_LIST:
	{
		if (wDataSize != sizeof(MSG_CS_Add_WaitList))
		{
			return;
		}

		MSG_CS_Add_WaitList * pAddWaitList = (MSG_CS_Add_WaitList*)pData;

		//检测玩家合法性
		auto it = m_mapHallUser.find(pAddWaitList->dwUserID);
		if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
		{
			CNetModule::getSingleton().CloseSocket(pSocket);
			return;
		}

		//检测玩家是否在游戏房间里面
		//bool bCanEnter = CheckUserCanEnterRoom(pAddWaitList->dwUserID, pAddWaitList->wGameID, pAddWaitList->wKindID, pAddWaitList->wRoomKindID, pSocket, false);

		//if (bCanEnter)
		//{
		//	stGameServerInfo* pGameServerInfo = CServerMgr::get_instance().FindReqGameServer(pAddWaitList->wGameID, pAddWaitList->wKindID, pAddWaitList->wRoomKindID);
		//	if (pGameServerInfo == NULL)
		//	{
		//		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_GAMENOEXIST);
		//		return;
		//	}

		//	MSG_PG_EnterRoom enterRoom;
		//	enterRoom.dwUserID = pAddWaitList->dwUserID;
		//	enterRoom.uValue1 = (CT_UINT64)pSocket;
		//	CNetModule::getSingleton().Send(pGameServerInfo->pConnector->GetSocket(), MSG_FRAME_MAIN, pMc->dwSubID, &enterRoom, sizeof(enterRoom));
		//}

		CT_BYTE cbServerState = 0;
		stGameServerInfo* pGameServerInfo = CServerMgr::get_instance().FindReqGameServer(pAddWaitList->wGameID, pAddWaitList->wKindID, pAddWaitList->wRoomKindID, cbServerState);
		if (pGameServerInfo == NULL)
		{
			if (cbServerState == 0)
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_GAMENOEXIST);
			}
			else
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_SEAT_FULL);
			}
			return;
		}

		MSG_PG_EnterRoom enterRoom;
		enterRoom.dwUserID = pAddWaitList->dwUserID;
		enterRoom.uValue1 = (CT_UINT64)pSocket;
		CNetModule::getSingleton().Send(pGameServerInfo->pConnector->GetSocket(), MSG_FRAME_MAIN, pMc->dwSubID, &enterRoom, sizeof(enterRoom));

		break;
	}
	//case SUB_C2S_USER_READY:
	//case SUB_C2S_USER_LEFT:
	//case SUB_C2S_APPLY_DISMISS_PROOM:
	//case SUB_C2S_OPERATE_DISMISS_PROOM:
	//case SUB_C2S_VOICE_CHAT:
	//case SUB_C2S_TEXT_CHAT:
	//case SUB_C2S_PULL_APPLY_PROOM:
	//case SUB_C2S_OPERATE_APPLY_PROOM:
	//case SUB_C2S_CHANGE_TABLE:
	default:
	{
		auto itUserSock = m_mapUserSock.find(pSocket);
		if (itUserSock != m_mapUserSock.end())
		{
			auto itUserInfo = m_mapGSUserInfo.find(itUserSock->second);
			if (itUserInfo != m_mapGSUserInfo.end())
			{
			
				CNetModule::getSingleton().Send(itUserInfo->second.pGSSocket, pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
			}
		} else
		{
			CNetModule::getSingleton().CloseSocket(pSocket);
		}
		break;
	}
	//default:
	//	break;
	}
}

CT_VOID CProxyThread::OnClientGameMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	auto itUserSock = m_mapUserSock.find(pSocket);
	if (itUserSock != m_mapUserSock.end())
	{
		auto itUserInfo = m_mapGSUserInfo.find(itUserSock->second);
		if (itUserInfo != m_mapGSUserInfo.end())
		{
			static CT_BYTE cbDataBuffer[SOCKET_TCP_PACKET];
			MSG_GameMsgUpHead MsgHead;
			MsgHead.dwUserID = itUserSock->second;
			CT_DWORD upHeadSize = (CT_DWORD)sizeof(MSG_GameMsgUpHead);
			memcpy(cbDataBuffer, &MsgHead, upHeadSize);
			memcpy(cbDataBuffer + upHeadSize, pData, wDataSize);
			CNetModule::getSingleton().Send(itUserInfo->second.pGSSocket, pMc->dwMainID, pMc->dwSubID, cbDataBuffer, wDataSize+upHeadSize);
		}
	}
}

CT_VOID CProxyThread::OnClientGroupMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	//直接转发给中心服
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

CT_VOID CProxyThread::OnClientHongBaoMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	//直接转发给中心服
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

CT_VOID CProxyThread::OnClientTaskMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	//直接转发给中心服
	if (pNetCenter != NULL)
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

CT_VOID CProxyThread::OnClientRankMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	//直接转发给中心服
	if (pNetCenter != NULL)
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

CT_VOID CProxyThread::OnClientRechargeMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	//直接转发给中心服
	if (pNetCenter != NULL)
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

CT_VOID CProxyThread::OnClientCustomerMain(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}
	
	if (pMc->dwSubID == SUB_CLIENT2SERVER_CHAT)
	{
		if (wDataSize != sizeof(MSG_C2S_Chat_To_Customer))
		{
			return;
		}
		//MSG_C2S_Chat_To_Customer* pCustomer = (MSG_C2S_Chat_To_Customer*)pData;
		MSG_C2S_Chat_To_CustomerEx customerEx;
		memcpy(&customerEx, pData, wDataSize);
		_snprintf_info(customerEx.szClientIp, sizeof(customerEx.szClientIp), "%s", CNetModule::getSingleton().GetRemoteIp(pSocket, false).c_str());
		//直接转发给客服服
		acl::aio_socket_stream* pCustomerSock = CServerMgr::get_instance().SelectOneCustomerServer();
		if (pCustomerSock)
		{
			CNetModule::getSingleton().Send(pCustomerSock, pMc->dwMainID, pMc->dwSubID, &customerEx, sizeof(customerEx));
		}
	}
	else
	{
		//直接转发给客服服
		acl::aio_socket_stream* pCustomerSock = CServerMgr::get_instance().SelectOneCustomerServer();
		if (pCustomerSock)
		{
			CNetModule::getSingleton().Send(pCustomerSock, pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
		}
	}
}

CT_VOID CProxyThread::OnClientWealthGodComing(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;
	
	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}
	
	//直接转发给中心服
	if (pNetCenter != NULL)
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

CT_VOID CProxyThread::OnClientDuoBaoMsg(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	//直接转发给中心服
	if (pNetCenter != NULL)
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

CT_VOID CProxyThread::OnClientRedPacketMsg(CMD_Command* pMc, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	//直接转发给中心服
	if (pNetCenter != NULL)
	{
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), pMc->dwMainID, pMc->dwSubID, pData, wDataSize);
	}
}

/*CT_BOOL CProxyThread::IsExistUserSession(CT_DWORD dwUserID, const CT_CHAR* pSession)
{
	acl::string key, val;
	key.format("session_%u", dwUserID);

	m_redis.clear();
	if (m_redis.get(key, val) != false)
	{
		if (val == pSession)
		{
			return true;
		}
		return false;
	}

	return false;
}*/

/*CT_BOOL	CProxyThread::IsUserHasPrivateRoom(CT_DWORD dwUserID, CT_DWORD& dwRoomNum)
{
	acl::string key;
	acl::string roomNum;
	key.format("online_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hget(key, "roomnum", roomNum) == false)
	{
		return false;
	}

	if (roomNum.empty())
	{
		return false;
	}

	dwRoomNum = atoi(roomNum.c_str());

	return true;
}*/

/*CT_BOOL	CProxyThread::GetPrivateRoomInfo(CT_DWORD dwRoomNum, PrivateRoomInfo& roomInfo)
{
	acl::string key;
	std::map<acl::string, acl::string> result;
	key.format("proom_info_%u", dwRoomNum);

	m_redis.clear();
	if (m_redis.hgetall(key.c_str(), result) == false)
	{
		return false;
	}

	if (result.empty())
	{
		return false;
	}

	roomInfo.dwRoomNum = dwRoomNum;
	roomInfo.dwOwerUserID =(atoi)(result["owner"].c_str());
	roomInfo.dwServerID = (atoi)(result["serverid"].c_str());
	roomInfo.wGameID = (atoi)(result["gameid"].c_str());
	roomInfo.wKindID = (atoi)(result["kindid"].c_str());
	roomInfo.wUserCount = (atoi)(result["usercount"].c_str());
	roomInfo.wPlayCount = (atoi)(result["playcount"].c_str());
	//roomInfo.wWinLimit = (atoi)(result["winlimit"].c_str());
	//roomInfo.wEndLimit = (atoi)(result["endlimit"].c_str());
	roomInfo.cbIsVip = (atoi)(result["vip"].c_str());
	//roomInfo.cbIsAuto = (atoi)(result["auto"].c_str());
	roomInfo.dwGroupID = (atoi)(result["gid"].c_str());
	roomInfo.dwRecordID = (atoi)(result["rid"].c_str());
	roomInfo.cbGroupType = (atoi)(result["gtype"].c_str());
	_snprintf_info(roomInfo.szOtherParam, sizeof(roomInfo.szOtherParam), "%s", result["otherparam"].c_str());
	return true;
}*/

/*CT_WORD	CProxyThread::GetUserLoginGameID(CT_DWORD dwUserID)
{
	acl::string key;
	std::map<acl::string, acl::string> psInfo;
	key.format("psinfo_%u", dwUserID);

	m_redis.clear();
	acl::string strGameID;
	if (m_redis.hget(key, "gameid", strGameID) == true)
	{
		return atoi(strGameID.c_str());
	}

	return 0;
}*/


/*CT_VOID CProxyThread::DelUserSession(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "session_" << dwUserID;
	m_redis.clear();
	if (m_redis.del_one(strKey.str().c_str()) <= 0)
	{
		LOG(WARNING) << "delete user session fail, userid: " << dwUserID;
	}
}*/

/*CT_VOID CProxyThread::DelProxyServerInfo(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "psinfo_" << dwUserID;
	m_redis.clear();
	if (m_redis.del_one(strKey.str().c_str()) <= 0)
	{
		LOG(WARNING) << "delete user proxy server info fail, userid: " << dwUserID;
	}
}*/

/*CT_VOID CProxyThread::SetAccountInfoExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "account_" << dwUserID;

	acl::string strAccountName;
	m_redis.clear();
	if (m_redis.hget(strKey.str().c_str(), "account", strAccountName) == true)
	{
		std::stringstream strAccountKey;
		strAccountKey << "userid_" << strAccountName.c_str();
		if (m_redis.expire(strAccountKey.str().c_str(), 10800) <= 0)
		{
			LOG(WARNING) << "expire user account name fail, userid: " << dwUserID;
		}
	}
	
	m_redis.clear();
	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire user account info fail, userid: " << dwUserID;
	}
}*/

/*CT_VOID CProxyThread::SetBenefitExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "benefit_" << dwUserID;

	m_redis.clear();
	if (m_redis.exists(strKey.str().c_str()) == false)
	{
		return;
	}

	m_redis.clear();
	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire benefit info fail, userid: " << dwUserID;
	}
}*/

/*CT_VOID CProxyThread::SetScoreInfoExpire(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "scoreinfo_" << dwUserID;

	m_redis.clear();
	if (m_redis.exists(strKey.str().c_str()) == false)
	{
		return;
	}

	m_redis.clear();
	if (m_redis.expire(strKey.str().c_str(), 10810) <= 0)
	{
		LOG(WARNING) << "expire score info fail, userid: " << dwUserID;
	}
}*/

/*CT_VOID CProxyThread::DelGameRoundCount(CT_DWORD dwUserID)
{
	std::stringstream strKey;
	strKey << "gameround_" << dwUserID;
	m_redis.clear();
	if (m_redis.del_one(strKey.str().c_str()) <= 0)
	{
		LOG(WARNING) << "delete user game round count fail, userid: " << dwUserID;
	}
}*/

/*CT_VOID CProxyThread::GetUserOnlineInfo(CT_DWORD dwUserID, std::map<acl::string, acl::string>& mapOnlineInfo)
{
	acl::string key;
	key.format("online_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hgetall(key, mapOnlineInfo) == false)
	{
		return;
	}
}*/

/*CT_BOOL CProxyThread::CheckUserCanEnterRoom(CT_DWORD dwUserID, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID, acl::aio_socket_stream* pSocket, CT_BOOL bCheckInSameGame)
{
	//获取在线信息
	std::map<acl::string, acl::string> mapOnline;
	GetUserOnlineInfo(dwUserID, mapOnline);
	if (!mapOnline.empty())
	{
		CT_WORD wSourceGameID = atoi(mapOnline["gameid"].c_str());
		CT_WORD wSourceKindID = atoi(mapOnline["kindid"].c_str());
		CT_WORD wSourceRoomKindID = atoi(mapOnline["roomkind"].c_str());
		CT_DWORD dwRoomNumOrServerID = atoi(mapOnline["roomnum"].c_str());

		if (bCheckInSameGame)
		{
			if (wSourceGameID != wGameID || wSourceKindID != wKindID || wSourceRoomKindID != wRoomKindID)
			{
				MSG_SC_In_OtherRoom otherRoom;
				otherRoom.wGameID = wSourceGameID;
				otherRoom.wKindID = wSourceKindID;
				otherRoom.wRoomKindID = wSourceRoomKindID;
				CNetModule::getSingleton().Send(pSocket, MSG_FRAME_MAIN, SUB_S2C_IN_OTHER_ROOM, &otherRoom, sizeof(otherRoom));
				return false;
			}
		}
		else
		{
			MSG_SC_In_OtherRoom otherRoom;
			otherRoom.wGameID = wSourceGameID;
			otherRoom.wKindID = wSourceKindID;
			otherRoom.wRoomKindID = wSourceRoomKindID;
			CNetModule::getSingleton().Send(pSocket, MSG_FRAME_MAIN, SUB_S2C_IN_OTHER_ROOM, &otherRoom, sizeof(otherRoom));
			return false;
		}
		
		stGameServerInfo* pGameServerInfo = CServerMgr::get_instance().FindGameServer(dwRoomNumOrServerID);
		if (pGameServerInfo == NULL)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, ENTER_ROOM_GAMENOEXIST);
			return false;
		}
	}

	return true;
}*/

CT_VOID CProxyThread::CloseUserSockInThisGS(acl::aio_socket_stream* pGSSocket)
{
	for (auto& it : m_mapGSUserInfo)
	{
		if (it.second.pGSSocket == pGSSocket)
		{
			CNetModule::getSingleton().CloseSocket(it.second.pClientSocket);
		}
	}
}

CT_VOID CProxyThread::OnAccountRegister(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_LoginData))
	{
		return;
	}

	std::string strClientIp = CNetModule::getSingleton().GetRemoteIp(pSocket, false);
	if (strClientIp.empty())
	{
		LOG(WARNING) << "get remote ip fail.";
		return;
	}

	MSG_CS_LoginDataEx registerEx;
	memcpy(&registerEx, pData, wDataSize);
	registerEx.uClientSock = (CT_UINT64)pSocket;
	_snprintf_info(registerEx.szClientIP, sizeof(registerEx.szClientIP), "%s", strClientIp.c_str());

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, &registerEx, sizeof(registerEx));
	
	//记录IP
	auto itSock = m_mapClientSockIp.find(pSocket);
	if (itSock != m_mapClientSockIp.end())
	{
		itSock->second = Utility::ip2Int(strClientIp.c_str());
	}
}

CT_VOID CProxyThread::OnAccountLoginOrRegister(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_LoginData))
	{
		return;
	}

	MSG_CS_LoginData* pLoginData = (MSG_CS_LoginData*)pData;
	if (pLoginData == NULL)
		return;

	if (pSocket == NULL)
	{
		return;
	}

	std::string strClientIp = CNetModule::getSingleton().GetRemoteIp(pSocket, false);
	if (strClientIp.empty())
	{
		LOG(WARNING) << "get remote ip fail.";
		return;
	}

	MSG_CS_LoginDataEx loginDataEx;
	memcpy(&loginDataEx, pData, wDataSize);
	loginDataEx.uClientSock = (CT_UINT64)pSocket;
	_snprintf_info(loginDataEx.szClientIP, sizeof(loginDataEx.szClientIP), "%s", strClientIp.c_str());

	//LOG(WARNING) << "OnAccountLoginOrRegister, channel id: " << pLoginData->dwChannelID;
	//loginDataEx.uClientSock = (CT_UINT64)pSocket;
	//_snprintf_info(loginDataEx.szClientIP, sizeof(loginDataEx.szClientIP), "%s", pClientIp);

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, &loginDataEx, sizeof(loginDataEx));
	
	//记录IP
	auto itSock = m_mapClientSockIp.find(pSocket);
	if (itSock != m_mapClientSockIp.end())
	{
		itSock->second = Utility::ip2Int(strClientIp.c_str());
	}
}

//签到
CT_VOID CProxyThread::OnUserSignIn(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_SignIn))
	{
		return;
	}

	MSG_CS_SignIn* pSignIn = (MSG_CS_SignIn*)pData;
	if (pSignIn == NULL)
		return;

	auto it = m_mapHallUser.find(pSignIn->dwUserID);
	if (it == m_mapHallUser.end())
	{
		SendLoginErrorCode(pSocket, LOGIN_NETBREAK);
		LOG(WARNING) << "can not find user hall info. userid: " << pSignIn->dwUserID;
		return;
	}

	MSG_CS_SignInEx signInEx;
	signInEx.dwUserID = pSignIn->dwUserID;
	signInEx.uClientSock = (CT_UINT64)pSocket;
	//_snprintf_info(signInEx.szUUID, sizeof(signInEx.szUUID), "%s", pSignIn->szUUID);


	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, &signInEx, sizeof(signInEx));
}

CT_VOID CProxyThread::OnUserGetBenefit(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_GetBenefit))
	{
		return;
	}

	MSG_CS_GetBenefit* pGetBenefit = (MSG_CS_GetBenefit*)pData;
	if (pGetBenefit == NULL)
		return;

	auto it = m_mapHallUser.find(pGetBenefit->dwUserID);
	if (it == m_mapHallUser.end())
	{
		SendLoginErrorCode(pSocket, LOGIN_NETBREAK);
		LOG(WARNING) << "can not find user hall info. userid: " << pGetBenefit->dwUserID;
		return;
	}

	MSG_CS_GetBenefitEx benefitEx;
	benefitEx.dwUserID = pGetBenefit->dwUserID;
	benefitEx.uClientSock = (CT_UINT64)pSocket;

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, &benefitEx, sizeof(benefitEx));
}

CT_VOID	CProxyThread::OnUserExchangeScore(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_ExchangeScore))
	{
		return;
	}

	MSG_CS_ExchangeScore* pExchangeScore = (MSG_CS_ExchangeScore*)pData;
	if (pExchangeScore == NULL)
		return;

	auto it = m_mapHallUser.find(pExchangeScore->dwUserID);
	if (it == m_mapHallUser.end())
	{
		SendLoginErrorCode(pSocket, LOGIN_NETBREAK);
		LOG(WARNING) << "can not find user hall info. userid: " << pExchangeScore->dwUserID;
		return;
	}

	MSG_CS_ExchangeScoreEx exchangeScore;
	exchangeScore.dwUserID = pExchangeScore->dwUserID;
	exchangeScore.wGem = pExchangeScore->wGem;
	exchangeScore.uClientSock = (CT_UINT64)pSocket;

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}
	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, &exchangeScore, sizeof(exchangeScore));
}

CT_VOID CProxyThread::OnUserPullMail(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_PullMail))
	{
		return;
	}

	MSG_CS_PullMail* pPullMail = (MSG_CS_PullMail*)pData;
	if (pPullMail == NULL)
		return;

	auto it = m_mapHallUser.find(pPullMail->dwUserID);
	if (it == m_mapHallUser.end())
	{
		LOG(WARNING) << "can not find user hall info. userid: " << pPullMail->dwUserID;
		return;
	}

	MSG_CS_PullMailEx pullMailEx;
	memcpy(&pullMailEx, pPullMail, sizeof(MSG_CS_PullMail));
	pullMailEx.uClientSock = (CT_UINT64)pSocket;

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, SUB_CS_PULL_MAIL, &pullMailEx, sizeof(pullMailEx));

}

CT_VOID CProxyThread::OnUserReadMail(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_ReadMail))
	{
		return;
	}

	MSG_CS_ReadMail* pReadMail = (MSG_CS_ReadMail*)pData;
	if (pReadMail == NULL)
		return;

	auto it = m_mapHallUser.find(pReadMail->dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	MSG_CS_ReadMailEx readMailEx;
	memcpy(&readMailEx, pReadMail, sizeof(MSG_CS_ReadMail));
	readMailEx.uClientSock = (CT_UINT64)pSocket;

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectMainLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, SUB_CS_READ_MAIL, &readMailEx, sizeof(readMailEx));
}

CT_VOID CProxyThread::OnUserDelMail(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_DelMail))
	{
		return;
	}
	MSG_CS_DelMail* pDelMail = (MSG_CS_DelMail*)pData;
	auto it = m_mapHallUser.find(pDelMail->dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectMainLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, pData, wDataSize);
}

CT_VOID CProxyThread::OnUserBindPromoter(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Bind_Promoter))
	{
		return;
	}

	MSG_CS_Bind_Promoter* pBindPromoter = (MSG_CS_Bind_Promoter*)pData;
	auto it = m_mapHallUser.find(pBindPromoter->dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, pData, wDataSize);
}

CT_VOID CProxyThread::OnUserBindMobile(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Bind_Mobile))
	{
		return;
	}

	MSG_CS_Bind_Mobile* pBindAccount = (MSG_CS_Bind_Mobile*)pData;
	auto it = m_mapHallUser.find(pBindAccount->dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server.";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, pData, wDataSize);
}

CT_VOID CProxyThread::OnUserPayScoreByApply(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Pay_UserScore_ByApple))
	{
		return;
	}

	MSG_CS_Pay_UserScore_ByApple* pBindAccount = (MSG_CS_Pay_UserScore_ByApple*)pData;
	auto it = m_mapHallUser.find(pBindAccount->dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server.";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, pData, wDataSize);
}

CT_VOID CProxyThread::OnUserBankOperate(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectMainLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server.";
		return;
	}

	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, pData, wDataSize);
}

CT_VOID CProxyThread::OnUserGetVerificationCode(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Get_VerificationCode))
	{
		return;
	}

	MSG_CS_Get_VerificationCode* pVerifyCode = (MSG_CS_Get_VerificationCode*)pData;
	
	MSG_CS_Get_VerificationCodeEx verifyCodeEx;
	memcpy(&verifyCodeEx, pVerifyCode, sizeof(MSG_CS_Get_VerificationCode));
	//_snprintf_info(verifyCodeEx.szMobileNum, sizeof(verifyCodeEx.szMobileNum), "%s", pVerifyCode->szMobileNum);
	//verifyCodeEx.cbType = pVerifyCode->cbType;
	verifyCodeEx.uClientSock = (CT_UINT64)pSocket;

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server.";
		return;
	}
	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, &verifyCodeEx, sizeof(verifyCodeEx));
}

CT_VOID	CProxyThread::OnUserResetPwd(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Reset_Pwd))
	{
		return;
	}

	//MSG_CS_Reset_Pwd* pReset = (MSG_CS_Reset_Pwd*)pData;

	MSG_CS_Reset_PwdEx resetPwd;
	memcpy(&resetPwd, pData, sizeof(MSG_CS_Reset_Pwd));
	resetPwd.uClientSock = (CT_UINT64)pSocket;

	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server.";
		return;
	}
	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, &resetPwd, sizeof(resetPwd));
}

CT_VOID CProxyThread::OnUserModifyPersonalInfo(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Modify_PersonalInfo))
	{
		return;
	}

	MSG_CS_Modify_PersonalInfo* pModifyPersonalInfo = (MSG_CS_Modify_PersonalInfo*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(pModifyPersonalInfo->dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}
	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server.";
		return;
	}
	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, pData, wDataSize);
}

CT_VOID CProxyThread::OnTranspondToLoginServer(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	CT_DWORD dwUserID = *(CT_DWORD*)pData;

	//检测玩家是否在线
	auto it = m_mapHallUser.find(dwUserID);
	if (it == m_mapHallUser.end() || it->second.pClientSocket != pSocket)
	{
		CNetModule::getSingleton().CloseSocket(pSocket);
		return;
	}
	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectMainLoginServer();
	if (pLoginSocket == NULL)
	{
		SendLoginErrorCode(pSocket, LOGIN_NOFIND_GAMEITEM);
		LOG(WARNING) << "can not find one valid login server.";
		return;
	}
	CNetModule::getSingleton().Send(pLoginSocket, MSG_LOGIN_MAIN, wSubCmdID, pData, wDataSize);
}

CT_VOID CProxyThread::OnAndroidConnect(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_CS_Android_Connect))
	{
		return;
	}

	MSG_CS_Android_Connect* pAndroid = (MSG_CS_Android_Connect*)pData;

	MSG_SC_AndroidConnect connectResult;
	if (strcmp(pAndroid->szMachineType, "F46D041E6904") == 0)
	{
		connectResult.cbResult = 0;
		m_mapAndroidSock.insert(std::make_pair(pSocket, pAndroid->szMachineType));
	}
	else
	{
		connectResult.cbResult = 1;
	}
	CNetModule::getSingleton().Send(pSocket, MSG_LOGIN_MAIN, SUB_SC_ANDROID_CONNECT, &connectResult, sizeof(connectResult));
}

//发送登录错误码
CT_VOID CProxyThread::SendLoginErrorCode(acl::aio_socket_stream* pSocket, CT_DWORD dwErrorCode)
{
	if (!pSocket)
	{
		return;
	}

	MSG_Share_ErrorCode errorCode;
	errorCode.dwErrorCode = dwErrorCode;

	CNetModule::getSingleton().Send(pSocket, MSG_LOGIN_MAIN, SUB_ERROR_CODE_MSG, &errorCode, sizeof(errorCode));

	//关闭socket
	CNetModule::getSingleton().CloseSocket(pSocket);
}

//发送游戏错误码
CT_VOID CProxyThread::SendGameErrorCode(acl::aio_socket_stream* pSocket, CT_WORD wMainID, CT_WORD wSubID, CT_DWORD dwErrorCode)
{
	if (!pSocket)
	{
		return;
	}

	MSG_SC_GameErrorCodeDirect errorCode;
	errorCode.iErrorCode = dwErrorCode;

	CNetModule::getSingleton().Send(pSocket, wMainID, wSubID, &errorCode, sizeof(errorCode));

	//关闭socket
	//CNetModule::getSingleton().CloseSocket(pSocket);
}

CT_VOID CProxyThread::SendUserOffLine(acl::aio_socket_stream* pGSSock, CT_DWORD dwUserID)
{
	MSG_P2GS_UserOffLine userOffLine;
	userOffLine.dwUserID = dwUserID;
	CNetModule::getSingleton().Send(pGSSock, MSG_PGS_MAIN, SUB_P2GS_USER_OFFLINE, &userOffLine, sizeof(userOffLine));
}


CT_VOID CProxyThread::UpdateUserCount(CT_WORD wSubID, CT_WORD wGameID)
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		MSG_P2CS_IncrOrDecr_Hall_User updateUserCount;
		updateUserCount.wGameID = wGameID;
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_PCS_MAIN, wSubID, &updateUserCount, sizeof(updateUserCount));
	}
}


CT_VOID CProxyThread::RegisterUser(MSG_LS2P_UserLoginSucc* pUserLoginSucc)
{
	MSG_P2CS_Register_User registerUser;
	//registerUser.wGameID = pUserLoginSucc->wGameID;
	registerUser.dwUserID = pUserLoginSucc->dwUserID;
	registerUser.dwChannelID = pUserLoginSucc->dwChannelID;
	registerUser.dwClientChannelID = pUserLoginSucc->dwClientChannelID;
	//registerUser.dwGem = pUserLoginSucc->dwGem;
	registerUser.llScore = pUserLoginSucc->llScore;
	registerUser.llBankScore = pUserLoginSucc->llBankScore;
	registerUser.dwRecharge = pUserLoginSucc->dwRecharge;
	registerUser.dwProxyServerID = CServerCfg::m_nServerID;
	//registerUser.cbPlayMode = pUserLoginSucc->cbPlayMode;
	registerUser.cbPlatformId = pUserLoginSucc->cbPlatformId;
	registerUser.cbSex = pUserLoginSucc->cbSex;
	registerUser.cbVipLevel = pUserLoginSucc->cbVipLevel;
	registerUser.cbVipLevel2 = pUserLoginSucc->cbVipLevel2;
	registerUser.cbHeadID = pUserLoginSucc->cbHeadID;
	registerUser.cbMachineType = pUserLoginSucc->cbMachineType;
	registerUser.cbShowExchange = pUserLoginSucc->cbShowExchange;
	registerUser.cbProvinceCode = pUserLoginSucc->cbProvinceCode;
	_snprintf_info(registerUser.szNickName, sizeof(registerUser.szNickName), "%s", pUserLoginSucc->szNickName);
	//_snprintf_info(registerUser.szHeadUrl, sizeof(registerUser.szHeadUrl), "%s", pUserLoginSucc->szHeadUrl);
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_PCS_MAIN, SUB_P2CS_REGISTER_HALL_USER, &registerUser, sizeof(registerUser));
}

CT_VOID CProxyThread::UnRegisterUser(CT_BYTE cbPlatformId, CT_DWORD dwUserID)
{
	MSG_P2CS_UnRegister_User unregisterUser;
	unregisterUser.cbPlatformId = cbPlatformId;
	unregisterUser.dwUserID = dwUserID;
	unregisterUser.dwProxyServerID = CServerCfg::m_nServerID;
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_PCS_MAIN, SUB_P2CS_UNREGISTER_HALL_USER, &unregisterUser, sizeof(unregisterUser));
}

CT_VOID CProxyThread::GoLoginServerLogout(CT_DWORD dwUserID)
{
	// 选择一个服务器
	acl::aio_socket_stream* pLoginSocket = CServerMgr::get_instance().SelectOneLoginServer();
	if (pLoginSocket != NULL)
	{
		MSG_P2LS_UserLogout userLogout;
		userLogout.dwUserID = dwUserID;
		userLogout.dwProxyServerID = CServerCfg::m_nServerID;
		CNetModule::getSingleton().Send(pLoginSocket, MSG_PLS_MAIN, SUB_P2LS_USER_LOGOUT, &userLogout, sizeof(userLogout));
	}
}

//与中心服的心跳
CT_VOID CProxyThread::SendHeartBeatToCenter()
{
	if (!m_mapHallUser.empty())
	{
		MSG_P2CS_HeartBeat heartBeat;
		for (auto& it : m_mapHallUser)
		{
		    //增加一个判断，如果当前连接已经不存在了，则判断为掉线
		    auto itClientSock = m_mapClientSockIp.find(it.second.pClientSocket);
		    if (itClientSock == m_mapClientSockIp.end())
                continue;

			heartBeat.dwUserID = it.first;
			CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_PCS_MAIN, SUB_P2CS_HEART_BEAT, &heartBeat, sizeof(heartBeat));
		}
	}
}