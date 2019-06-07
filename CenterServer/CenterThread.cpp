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
#include "ActivityMgr.h"
#include "DuoBaoMgr.h"
#include "RedPacketMgr.h"

extern CNetConnector *pNetDB;

#define		IDI_CONNECT_DBSERVER			(100)
#define		TIME_CONNECT_DBSERVER			(1000)				//1s重连一次

#define		IDI_REMOVE_EMPTY_ROOM			(101)
#define		TIME_REMOVE_EMPTY_ROOM  		(1000*60*1)			//每两分钟检测一次

#define		IDI_TIMER_TICK					(102)
#define		TIME_TICK						(1000*60)

#define		IDI_TIMER_REMOVE_OFFLINE_USER	(103)
#ifdef _DEBUG
#define		TIME_REMOVE_OFFLINE_USER		(1000*60)			//每小时检测一次离线玩家
#else
#define		TIME_REMOVE_OFFLINE_USER		(1000*3600)			//每小时检测一次离线玩家
#endif

#define		IDI_TIMER_BROADCAST_SYS_MSG		(104)				//广播系统消息
#define		TIME_BROADCAST_SYS_MSG			(1000*10)

#define 	IDI_CHECK_USER_OFFLINE			(105)				//检测玩家是否离线
#define 	TIME_CHECK_USER_OFFLINE			(1000*100)
#define 	OFFLINE_MAX_TIME				(60*15)

#define     IDI_WEALTGHGODCOMING_BROADCAST  (106)
#define     TIME_WEALTGHGODCOMING_BROADCAST (1000*25)  			//25秒广播同步一次排名信息

#define 	IDI_DUOBAO_CHECK				(107)
#define		TIME_DUOBAO_CHECK				(1000*2)			//2秒钟检测一次夺宝的结果

#define 	IDI_REDPACKET_GRAB				(108)				//启动时发送昨天没发送的红包
#define		TIME_REDPACKET_GRAB				(1000*5)

#define     IDI_CALC_GROUP_INCOME           (109)               //15分钟统计一次群收入
#define     TIME_CALC_GROUP_INCOME          (1000*60)

#define     IDI_REMOVE_GROUP_PROOM          (110)               //2分钟统计一次群的
#define     TIME_REMOVE_GROUP_PROOM         (1000*60*2)         //

#define		IDI_TIMER_TEST					(200)
#define		TIME_TEST						(20*1000)

CCenterThread::CCenterThread(void)
	//: m_updateHallUserTick(0)
	//, m_updateServerUserTick(0)
	: m_updateUserCountTick(0)
	, m_redisClient(CServerCfg::m_RedisAddress, CServerCfg::m_RedisConTimeOut, CServerCfg::m_RedisRwTimeOut)
	, m_bInitServer(false)
	, m_iTodayTime(time(NULL))
	, m_iWeekTime(time(NULL))
	, m_bRechargeJudge(true)
{
	srand(unsigned(time(NULL)));

	//m_mapHallUser.clear();
	m_mapWxRoomNum.clear();
	m_mapPlatformProxyUser.clear();
	m_mapPlatformMachineUser.clear();

	CGroupMgr::get_instance().SetRedisPtr(&m_redis);
	CUserMgr::get_instance().SetRedisPtr(&m_redis);
	CRankMgr::get_instance().SetRedisPtr(&m_redis);
}

CCenterThread::~CCenterThread(void)
{

}

CT_VOID CCenterThread::OnTCPSocketLink(CNetConnector* pConnector)
{
	stConnectParam& connectParam = pConnector->GetConnectParam();
	if (connectParam.conType == CONNECT_DB)
	{
		LOG(INFO) << "connect to db server succ!";
		GoDBServerRegister();
		if (!m_bInitServer)
		{
			m_redisClient.set_password(CServerCfg::m_RedisPwd);
			m_redis.set_client(&m_redisClient);

			//LoadAllGroupInfo();
			//LoadHongBaoInfo();
			LoadTaskInfo();
			//CNetModule::getSingleton().SetTimer(IDI_REMOVE_EMPTY_ROOM, TIME_REMOVE_EMPTY_ROOM, this, true);
			CNetModule::getSingleton().SetTimer(IDI_TIMER_TICK, TIME_TICK, this, true);
			CNetModule::getSingleton().SetTimer(IDI_TIMER_REMOVE_OFFLINE_USER, TIME_REMOVE_OFFLINE_USER, this, true);
			CNetModule::getSingleton().SetTimer(IDI_TIMER_BROADCAST_SYS_MSG, TIME_BROADCAST_SYS_MSG, this, true);
			CNetModule::getSingleton().SetTimer(IDI_CHECK_USER_OFFLINE, TIME_CHECK_USER_OFFLINE, this, true);
			CNetModule::getSingleton().SetTimer(IDI_WEALTGHGODCOMING_BROADCAST, TIME_WEALTGHGODCOMING_BROADCAST, this, true);
			CNetModule::getSingleton().SetTimer(IDI_DUOBAO_CHECK, TIME_DUOBAO_CHECK, this, true);
			CNetModule::getSingleton().SetTimer(IDI_CALC_GROUP_INCOME, TIME_CALC_GROUP_INCOME, this, true);
            CNetModule::getSingleton().SetTimer(IDI_REMOVE_GROUP_PROOM, TIME_REMOVE_GROUP_PROOM, this, true);
            //CNetModule::getSingleton().SetTimer(IDI_TIMER_TEST, TIME_TEST, this, true);
			m_bInitServer = true;
		}
	}
}

CT_VOID CCenterThread::OnTCPSocketShut(CNetConnector* pConnector)
{
	if (pConnector == NULL)
	{
		return;
	}

	stConnectParam& connectParam = pConnector->GetConnectParam();
	if (connectParam.conType == CONNECT_DB)
	{
		LOG(WARNING) << "db server is disconnect, after " << TIME_CONNECT_DBSERVER / 1000 << "s will be reconnect!";
		CNetModule::getSingleton().SetTimer(IDI_CONNECT_DBSERVER, TIME_CONNECT_DBSERVER, this, false);
	}
}

CT_VOID CCenterThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{

}

CT_VOID CCenterThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
	if (pSocket)
	{
		CServerMgr::get_instance().DeleteServer(pSocket);
	}
}

//网络bit数据到来
CT_VOID CCenterThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wLen)
{
	if (pSocket == NULL)
	{
		return;
	}

	switch (pMc->dwMainID)
	{
	case MSG_PCS_MAIN:
	{
		OnProxyServerMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_GCS_MAIN:
	{
		OnGameServerMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_LCS_MAIN:
	{
		OnLoginServerMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_DIPCS_MAIN:
	{
		OnDipServerMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_MCS_MAIN:
	{
		OnMaintainMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_CSDB_MAIN:
	{
		OnDBServerMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_GROUP_MAIN:
	{
		OnGroupMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_HONGBAO_MAIN:
	{
		OnHongBaoMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_TASK_MAIN:
	{
		OnTaskMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_RANK_MAIN:
	{
		OnRankMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_CONFIG_MSG_MAIN:
	{
		OnConfigOptionMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
	case MSG_CUSTOMERCS_MAIN:
	{
		OnCustomerServerMsg(pMc->dwSubID, pData, wLen, pSocket);
	}
	break;
    case MSG_WEALTHGODCOMING_MAIN:
    {
        OnWealthGodComingMsg(pMc->dwSubID, pData, wLen, pSocket);
    }
    break;
    case MSG_DUOBAO_MAIN:
    {
        CYYDuoBaoMgr::get_instance().DuoBaoMsg(pMc->dwSubID, pData, wLen);
    }
    break;
    case MSG_REDPACKET_MAIN:
    {
        CRedPacketMgr::get_instance().RedPacketMsg(pMc->dwSubID, pData, wLen);
    }
    break;
	default:
		break;
	}
}


//网络定时器
CT_VOID CCenterThread::OnTimer(CT_DWORD dwTimerID)
{
	switch (dwTimerID)
	{
	case IDI_CONNECT_DBSERVER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetDB);
	}
	break;
	case IDI_REMOVE_EMPTY_ROOM:
	{
		RemoveTimeOutPRoom();
	}
	break;
	case IDI_TIMER_TICK:
	{
		//检测是否
		AcrossTheDayEvent();
        CActivityMgr::get_instance().CheckWealthGodComingStartEnd();
        CRedPacketMgr::get_instance().CheckRedPacketStartEnd();
	}
	break;
	case IDI_TIMER_REMOVE_OFFLINE_USER:
	{
		CUserMgr::get_instance().RemoveOffLineUser();
	}
	break;
	case IDI_TIMER_BROADCAST_SYS_MSG:
	{
		CSystemMsgMgr::get_instance().BroadcastSystemMsg();
	}
	break;
	case IDI_CHECK_USER_OFFLINE:
	{
		CheckUserOffLine();
	}
	break;
    case IDI_WEALTGHGODCOMING_BROADCAST:
    {
        CActivityMgr::get_instance().BroadcastRealTimeRankInfo();
    }
    break;
	case IDI_TIMER_TEST:
	{
		MSG_G2CS_WinScore_Info winScore;
		winScore.llWinScore = 1000;
		_snprintf_info(winScore.szNickName,  sizeof(winScore.szNickName), "%s", "hello boy!");
		_snprintf_info(winScore.szServerName, sizeof(winScore.szServerName), "%s", Utility::Ansi2Utf8("斗地主初级房").c_str());
		CSystemMsgMgr::get_instance().BroadcastGameScoreMsg(&winScore);
		//CSystemMsgMgr::get_instance().InsertSystemMsg();
	}
	break;
	case IDI_DUOBAO_CHECK:
	{
	    CYYDuoBaoMgr::get_instance().CheckDuoBaoStartEnd();
	}
	break;
	case IDI_REDPACKET_GRAB:
	{
		LOG(WARNING) << "send yesterday redpacket mail.";
		CRedPacketMgr::get_instance().SendRedPacketAward();
	}
	break;
    case IDI_CALC_GROUP_INCOME:
    {
        CGroupMgr::get_instance().CalcGroupUserIncome(false);
    }
    break;
    case IDI_REMOVE_GROUP_PROOM:
    {
        CGroupMgr::get_instance().CheckRemoveEmptyPRoom();
    }
    break;

    default:
		break;
	}
}

//打开
CT_VOID CCenterThread::Open()
{
	//m_redis.set_client(&m_redisClient);
}

CT_VOID CCenterThread::AcrossTheDayEvent()
{
	if (m_SystemStatus.cbScoreMode == 0)
	{
		return;
	}

	CT_INT64 nTimeNow = time(NULL);
	CT_BOOL bAcrossDay = isAcrossTheDay(m_iTodayTime, nTimeNow);
	//LOG(WARNING) << "center server check across day.";
	if (bAcrossDay)
	{
		LOG(WARNING) << "center server is across day.";
		CUserTaskModel::get_instance().AcrossTheDay();
		CRedPacketMgr::get_instance().SendRedPacketAward();
		m_iTodayTime = nTimeNow;

		CT_INT32 nWeek = diffTimeWeek(m_iWeekTime, nTimeNow);
		if (nWeek > 0)
		{
			m_iWeekTime = nTimeNow;
			CUserTaskModel::get_instance().AcrossTheWeek();
		}

		//重新刷新今天的pcu
		AcrossTheDayUpdatePCU();

		//重新计算一下昨天的群组收入
        CGroupMgr::get_instance().CalcGroupUserIncome(true);
	}
}

CT_VOID CCenterThread::CheckUserOffLine()
{
	CT_DWORD  dwNow = Utility::GetTime();
	MapGlobalUser& mapGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
	for (auto& it : mapGlobalUser)
	{
		CT_DWORD dwLastHeartBeatTime = it.second->GetLastHeartBeatTime();
		if (it.second->IsOnline() && dwNow - dwLastHeartBeatTime >= OFFLINE_MAX_TIME)
		{
			//CT_DWORD dwProxyServerID = it.second->GetUserProxyID();
			CT_DWORD dwUserID = it.first;
            //CT_DWORD dwPlatformId = it.second->GetPlatformId();
            //DecreaseHallUser(dwPlatformId, dwProxyServerID, dwUserID);

			//设置玩家
			CUserMgr::get_instance().SetUserOnline(dwUserID, false);
		}
	}
}

CT_VOID CCenterThread::OnProxyServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_P2CS_REGISTER:
		{
			if (wDataSize != sizeof(CMD_ProxyServer_Info))
			{
				return;
			}
			CMD_ProxyServer_Info* pServerInfo = (CMD_ProxyServer_Info*)pData;
			CServerMgr::get_instance().AddProxyServerInfo(pSocket, pServerInfo);
			CServerMgr::get_instance().SendGameServerList(pSocket, MSG_PCS_MAIN, SUB_CS2P_SERVER_INSERT, SUB_CS2P_SERVER_FINISH);
			CServerMgr::get_instance().SendLoginServerList(pSocket);
			CServerMgr::get_instance().SendCustomerServerList(pSocket);
			CGroupMgr::get_instance().SendAllPRoomToProxyServer(pSocket);
		}
		break;
	case SUB_P2CS_CREATE_PRIVATE_ROOM:
		{
			if (wDataSize != sizeof(MSG_PS_CreatePrivateRoom))
			{
				return;
			}

			MSG_PS_CreatePrivateRoom* pPrivateRoom = (MSG_PS_CreatePrivateRoom*)pData;
			if (pPrivateRoom == NULL)
			{
				return;
			}

			//判断玩家是否已经有房间
			if (CheckUserHasRoom(pPrivateRoom->dwUserID))
			{
				return;
			}

			// 查找一个合适的游戏服务器
			CMD_GameServer_Info *pGameServer = CServerMgr::get_instance()
                .FindSuitTableGameServer(pPrivateRoom->wGameID, pPrivateRoom->wKindID/*, pPrivateRoom->wUserCount*/);
			if (pGameServer == NULL)
			{
				LOG(WARNING) << "not find suit game server";
				MSG_CS2P_CreatePrivateRoomFail createFail;
				createFail.dwUserID = pPrivateRoom->dwUserID;
				createFail.cbErrorCode = CREATE_PROOM_GAMESERVER_NOENOUGH;
				createFail.uClientSock = pPrivateRoom->uClientSock;
				CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_CREATE_PRIVATE_FAIL, &createFail, sizeof(createFail));

				return;
			}

			CT_BOOL bCheck = CServerMgr::get_instance().CheckCreatePRoomParam(pGameServer->dwServerID, pPrivateRoom);
			if (bCheck == false)
			{
				MSG_CS2P_CreatePrivateRoomFail createFail;
				createFail.dwUserID = pPrivateRoom->dwUserID;
				createFail.cbErrorCode = CREATE_PROOM_ERROR_PARAM;
				createFail.uClientSock = pPrivateRoom->uClientSock;
				CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_CREATE_PRIVATE_FAIL, &createFail, sizeof(createFail));
				return;
			}

			// 判断玩家的钻石是否足够
			CT_DWORD dwGem = GetUserGem(pPrivateRoom->dwUserID);
			CT_WORD wNeedGem = CServerMgr::get_instance().GetCreatePRoomNeedGem(pGameServer->dwServerID, pPrivateRoom->cbPlayCount);

			if (dwGem < wNeedGem)
			{
				MSG_CS2P_CreatePrivateRoomFail createFail;
				createFail.dwUserID = pPrivateRoom->dwUserID;
				createFail.cbErrorCode = CREATE_PROOM_GEM_NOENOUGH;
				createFail.uClientSock = pPrivateRoom->uClientSock;
				CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_CREATE_PRIVATE_FAIL, &createFail, sizeof(createFail));
				LOG(WARNING) << "create private room gem not enough, need: " << wNeedGem << ", has gem: " << dwGem << ", userid:" << pPrivateRoom->dwUserID;
				return;
			}

			CT_DWORD dwSeed = (CT_DWORD)time(NULL);
			srand(dwSeed);
			CT_DWORD dwRoom = CreateRoomNum(pGameServer->dwServerID);
			while (CheckRoomNumIsExist(dwRoom))
			{
				dwSeed += 9527;
				srand(dwSeed);
				dwRoom = CreateRoomNum(pGameServer->dwServerID);
				LOG(WARNING) << "create same room num!!";
			}

			CT_BOOL bSetCache = SetRoomNumToCache(pPrivateRoom->dwUserID, pGameServer->dwServerID, pGameServer->wGameID, pGameServer->wKindID,\
				pPrivateRoom->wUserCount, pPrivateRoom->cbPlayCount,/* pPrivateRoom->wMaxHuxi, 100,*/ pPrivateRoom->cbIsVip, pPrivateRoom->szOtherParam, dwRoom);
			if (!bSetCache)
			{
				return;
			}

			//创建成功，将结果发回前端
			MSG_CS2P_CreatePrivateRoom createRoom;
			createRoom.dwRoomNum = dwRoom;
			createRoom.dwUserID = pPrivateRoom->dwUserID;
			createRoom.wGameID = pPrivateRoom->wGameID;
			createRoom.wKindID = pPrivateRoom->wKindID;
			createRoom.uClientSock = pPrivateRoom->uClientSock;
			CNetModule::getSingleton().Send(pSocket, MSG_PCS_MAIN, SUB_CS2P_CREATE_PRIVATE_ROOM, &createRoom, sizeof(createRoom));
		}
		break;
	case SUB_P2CS_REGISTER_HALL_USER:
	{
		if (wDataSize != sizeof(MSG_P2CS_Register_User))
		{
			return;
		}

		MSG_P2CS_Register_User* pRegisterUser = (MSG_P2CS_Register_User*)pData;
		if (pRegisterUser == NULL)
		{
			return;
		}

		//此函数和CUserMgr::get_instance().InsertUser的调用顺序不能调换
		//IncreaseHallUser(pRegisterUser->cbPlatformId, pRegisterUser->dwProxyServerID, pRegisterUser->cbMachineType, pRegisterUser->dwUserID);

		//注册用户
        CUserMgr::get_instance().InsertUser(pRegisterUser, m_SystemStatus.cbScoreMode);
		//CRankMgr::get_instance().SendScoreRankToClient(pRegisterUser->dwUserID);
		//发送任务信息
		CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pRegisterUser->dwUserID);
		if (pUserTask)
		{
			pUserTask->SendAllTaskToClient();
		}

		//发送充值渠道等各种配置信息
		CConfigOption::get_instance().SendAnnouncementInfo(pRegisterUser->dwUserID);
		CConfigOption::get_instance().SendPlatformRechargeCfg(pRegisterUser->dwUserID, pRegisterUser->cbPlatformId);
		CConfigOption::get_instance().SendRechargeChannelInfoToClient(pRegisterUser->dwUserID, pRegisterUser->dwChannelID);
		//CConfigOption::get_instance().SendRechargePromoterListToClient(pRegisterUser->dwUserID, 0);
		CConfigOption::get_instance().SendRechargePromoterListToClient(pRegisterUser->dwUserID, pRegisterUser->cbPlatformId, pRegisterUser->cbProvinceCode);
		CConfigOption::get_instance().SendExchangeChannelStatusToClient(pRegisterUser->dwUserID);
		CConfigOption::get_instance().SendGeneralizePromoterToClient(pRegisterUser->dwUserID, pRegisterUser->cbPlatformId);
		CConfigOption::get_instance().SendClientChannelDomain(pRegisterUser->dwUserID, 0);
        CActivityMgr::get_instance().SendDoingWealthGodComingToClient(pRegisterUser->dwUserID);
        CRedPacketMgr::get_instance().SendRedPacketStatusToClient(pRegisterUser->dwUserID);
        CRedPacketMgr::get_instance().SendRedPacketDoingStatusToClient(pRegisterUser->dwUserID);
        CGroupMgr::get_instance().SendAllGroupInfoToClient(pRegisterUser->dwUserID);

        //统计在线用户
        CalcOnlineUser();

		//if (_CHECK_BIT(pRegisterUser->cbPlayMode, _BIT32(PRIMARY_MODE)))
		/************************************************
		2017年11月2日 16:38:07 屏蔽不需要的数据
		if (m_SystemStatus.cbPrivateMode)
		{
			//发送群组信息给玩家
			CGroupMgr::get_instance().SendAllGroupInfoToClient(pRegisterUser->dwUserID);
		}
		//if (_CHECK_BIT(pRegisterUser->cbPlayMode, _BIT32(SCORE_MODE)))
		if (m_SystemStatus.cbScoreMode)
		{
			//发送红包
			CHongBaoMgr::get_instance().SendHongBaoCountToClient(pRegisterUser->dwUserID);
			//发送排行榜
			CRankMgr::get_instance().SendScoreRankToClient(pRegisterUser->dwUserID);
			//发送任务信息
			CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pRegisterUser->dwUserID);
			if (pUserTask)
			{
				pUserTask->SendAllTaskToClient();
			}
		}
		**************************************************/
	}
	break;
	case SUB_P2CS_UNREGISTER_HALL_USER:
	{
		if (wDataSize != sizeof(MSG_P2CS_UnRegister_User))
		{
			return;
		}

		MSG_P2CS_UnRegister_User* pUnRegisterUser = (MSG_P2CS_UnRegister_User*)pData;
		if (pUnRegisterUser == NULL)
		{
			return;
		}

		//DecreaseHallUser(pUnRegisterUser->cbPlatformId, pUnRegisterUser->dwProxyServerID, pUnRegisterUser->dwUserID);
		
		//注销用户
		//CUserMgr::get_instance().DeleteUser(pUnRegisterUser->dwUserID);
		//设置玩家
		CUserMgr::get_instance().SetUserOnline(pUnRegisterUser->dwUserID, false);

		//统计在线用户
		CalcOnlineUser();
	}
	break;
    case SUB_P2CS_HEART_BEAT:
    {
        if (wDataSize != sizeof(MSG_P2CS_HeartBeat))
        {
            return;
        }

		MSG_P2CS_HeartBeat* pHeartBeat = (MSG_P2CS_HeartBeat*)pData;
       	CUserMgr::get_instance().SetUserHeartBeatTime(pHeartBeat->dwUserID, Utility::GetTime());
    }
    break;

	default:
		break;
	}
}

CT_VOID CCenterThread::OnGameServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_G2CS_REGISTER:
	{
		if (wDataSize != sizeof(CMD_GameServer_Info))
		{
			return;
		}
		CMD_GameServer_Info* pGameServerInfo = (CMD_GameServer_Info*)pData;
		CServerMgr::get_instance().AddGameServerInfo(pSocket, pGameServerInfo);
		CGroupMgr::get_instance().SendGroupToGameServer(pSocket, pGameServerInfo);
		//m_wxGroupMgr.SendGroupToGameServer(pSocket);
	}
	break;
	case SUB_G2CS_UPDATE_USER:
	{
		if (wDataSize != sizeof(CMD_Update_GS_User))
		{
			return;
		}

		CMD_Update_GS_User* pGSUser = (CMD_Update_GS_User*)pData;
		CServerMgr::get_instance().UpdateGameServerUser(pSocket, pGSUser);

		//3表示是更新机器人数据
		if (pGSUser->cbUpdateMode != 3)
		{
		    CT_BYTE cbPlatformId = 0;
            tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pGSUser->dwUserID);
            if (pUserInfo)
            {
                cbPlatformId = pUserInfo->cbPlatformId;
            }

			if (pGSUser->cbUpdateMode == 1)
			{
				CUserMgr::get_instance().UpdateUserGsInfo(pGSUser->dwUserID, pGSUser->dwServerID);
			}
			else
			{
				CUserMgr::get_instance().UpdateUserGsInfo(pGSUser->dwUserID, 0);
			}

			//if (cbPlatformId != 0)
            //{
            //    UpdateServerUser(cbPlatformId, pGSUser->wGameID, pGSUser->wKindID, pGSUser->wRoomKindID);
            //}

            if (cbPlatformId != 0)
            {
                CalcOnlineUser();
            }
		}
	}
	break;
	case SUB_G2CS_UPDATE_PROOM:
	{
		if (wDataSize != sizeof(MSG_G2CS_UpdatePRoom))
		{
			return;
		}
		
		MSG_G2CS_UpdatePRoom* pRemovePRoom = (MSG_G2CS_UpdatePRoom*)pData;
		if (pRemovePRoom == NULL)
		{
			return;
		}

		//LOG(WARNING) << "update proom: " << pRemovePRoom->dwGroupID << ", remove: " << (int)pRemovePRoom->bRemoveGroupRoom;
		if (pRemovePRoom->dwGroupID != 0 && pRemovePRoom->bRemoveGroupRoom)
		{
			if (pRemovePRoom->cbGroupType == WX_GROUP_ROOM)
			{
				RemovePRoomNum(pRemovePRoom->dwGroupID, pRemovePRoom->wGameID, pRemovePRoom->wKindID, pRemovePRoom->dwRoomNum);
			}
			else
			{
				CGroupMgr::get_instance().RemovePRoomNum(pRemovePRoom->dwGroupID, pRemovePRoom->wGameID, pRemovePRoom->wKindID, pRemovePRoom->dwRoomNum);
				//LOG(WARNING) << "delete group proom : " << pRemovePRoom->dwRoomNum;
			}
		}

		auto it = m_mapPRoomInfo.find(pRemovePRoom->dwRoomNum);
		if (it != m_mapPRoomInfo.end())
		{
			if (pRemovePRoom->cbRoomState == PROOM_END)
			{
				m_mapPRoomInfo.erase(it);
				//LOG(WARNING) << "delete proom : " << pRemovePRoom->dwRoomNum;
			}
			else
			{
				it->second.bStart = true;
			}
		}
	}
	break;
	case SUB_G2CS_QUERY_PROOM_INFO:
	{
		if (wDataSize != sizeof(MSG_CS2D_QueryPRoom_Info))
		{
			return;
		}

		acl::aio_socket_stream* pDipSock = CServerMgr::get_instance().FindDipServer(1000);
		if (pDipSock)
		{
			CNetModule::getSingleton().Send(pDipSock, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_PROOM_INFO, pData, wDataSize);
		}
	}
	break;
	case SUB_G2CS_DISMISS_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS2D_Dismiss_PRoom))
		{
			return;
		}

		MSG_CS2D_Dismiss_PRoom *pDismissPRoom = (MSG_CS2D_Dismiss_PRoom*)pData;
		if (pDismissPRoom->uWebSock != 0)
		{
			acl::aio_socket_stream* pDipSock = CServerMgr::get_instance().FindDipServer(1000);
			if (pDipSock)
			{
				CNetModule::getSingleton().Send(pDipSock, MSG_DIPCS_MAIN, SUB_CS2DIP_DISMISS_PROOM, pData, wDataSize);
			}
		}
	}
	break;
	case SUB_G2CS_TICK_USER:
	{
		if (wDataSize != sizeof(MSG_CS2D_Tick_User))
		{
			return;
		}

		acl::aio_socket_stream* pDipSock = CServerMgr::get_instance().FindDipServer(1000);
		if (pDipSock)
		{
			CNetModule::getSingleton().Send(pDipSock, MSG_DIPCS_MAIN, SUB_CS2DIP_TICK_USER, pData, wDataSize);
		}
	}
	break;
	case SUB_G2CS_RETURN_GROUPPROOM_GEM:
	{
		if (wDataSize != sizeof(MSG_G2CS_Return_GroupPRoom_Gem))
		{
			return;
		}

		MSG_G2CS_Return_GroupPRoom_Gem* pReturnGem = (MSG_G2CS_Return_GroupPRoom_Gem*)pData;
		
		//更新内存的数据
		long long int llNewGem = 0;
		UpdateUserGem(pReturnGem->dwMasterID, pReturnGem->wReturnGem, llNewGem, 1);
		CUserMgr::get_instance().AddUserGemEx(pReturnGem->dwMasterID, (CT_INT32)pReturnGem->wReturnGem);

		//更新数据库
		MSG_CS2DB_Return_GroupPRoom_Gem returnGroupRoomGem;
		returnGroupRoomGem.dwGroupID = pReturnGem->dwGroupID;
		returnGroupRoomGem.dwRecordID = pReturnGem->dwRecordID;
		returnGroupRoomGem.dwMasterID = pReturnGem->dwMasterID;
		returnGroupRoomGem.wReturnGem = pReturnGem->wReturnGem;

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_RETURN_GROUPPROOM_GEM, &returnGroupRoomGem, sizeof(returnGroupRoomGem));
	}
	break;

	case SUB_G2CS_UPDATE_USER_GEM:
	{
		if (wDataSize != sizeof(MSG_UpdateUser_Gem))
		{
			return;
		}
		MSG_UpdateUser_Gem* pUpdataGem = (MSG_UpdateUser_Gem*)pData;
		CUserMgr::get_instance().AddUserGemEx(pUpdataGem->dwUserID, pUpdataGem->nAddGem);
	}
	break;
	case SUB_G2CS_UPDATE_USER_SCORE:
	{
		if (wDataSize != sizeof(MSG_UpdateUser_Score))
		{
			return;
		}
		MSG_UpdateUser_Score* pUpdataScore = (MSG_UpdateUser_Score*)pData;
		CUserMgr::get_instance().AddUserScoreEx(pUpdataScore->dwUserID, pUpdataScore->llAddScore);

		//如果是积分场,检测任务
		/*if (m_SystemStatus.cbScoreMode && pUpdataScore->enType == PLAY_GAME)
		{
			CUserTask* pTask = CUserMgr::get_instance().GetUserTaskPtr(pUpdataScore->dwUserID);
			if (pTask)
			{
				pTask->OnUserAddScore(pUpdataScore->wRoomKind, pUpdataScore->llAddScore);
			}
		}*/
	}
	break;
	case SUB_G2CS_UPDATE_USER_SCORE_INFO:
	{
		if (wDataSize != sizeof(MSG_Update_ScoreInfo))
		{
			return;
		}
		MSG_Update_ScoreInfo* pScoreInfo = (MSG_Update_ScoreInfo*)pData;

		CUserMgr::get_instance().UpdateUserScoreInfo(pScoreInfo->dwUserID, pScoreInfo);
	}
	break;
	case SUB_G2CS_GROUPPROOM_CLEARING:
	{
		if (wDataSize != sizeof(MSG_G2CS_GroupPRoom_Clearing))
		{
			return;
		}
		MSG_G2CS_GroupPRoom_Clearing* pClearing = (MSG_G2CS_GroupPRoom_Clearing*)pData;
		CGroupMgr::get_instance().InsertGroupPRoomClearing(pClearing);
	}
	break;
    case SUB_G2CS_QUERY_TABLE_INFO:
    {
        if (wDataSize != sizeof(MSG_CS2D_Query_TableInfo))
        {
            return;
        }

        acl::aio_socket_stream* pDipSock = CServerMgr::get_instance().FindDipServer(1000);
        if (pDipSock)
        {
            CNetModule::getSingleton().Send(pDipSock, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_SERVERID_TABLE_MSG, pData, wDataSize);
           // LOG(INFO) << "center server query game server table information, cmd main:" << MSG_DIPCS_MAIN << " sub:" << SUB_CS2DIP_QUERY_SERVERID_TABLE_MSG;
        }
    }
    break;
	case SUB_G2CS_USER_WIN_SCORE:
	{
		if (wDataSize != sizeof(MSG_G2CS_WinScore_Info))
		{
			return;
		}
		MSG_G2CS_WinScore_Info* pWinScore = (MSG_G2CS_WinScore_Info*)pData;
		CSystemMsgMgr::get_instance().BroadcastGameScoreMsg(pWinScore);
	}
	break;
	case SUB_G2CS_USER_PLAY_GAME:
	{
		if (wDataSize != sizeof(MSG_G2CS_PlayGame))
		{
			return;
		}
		MSG_G2CS_PlayGame* pPlayGame = (MSG_G2CS_PlayGame*)pData;
		CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pPlayGame->dwUserID);
		if (pUserTask)
		{
			pUserTask->OnUserPlayGame(pPlayGame->wGameID, pPlayGame->wKindID, pPlayGame->wRoomKindID);
			CRedPacketMgr::get_instance().CollectUserGameData(pPlayGame);
			CGroupMgr::get_instance().CollectUserGameData(pPlayGame);
		}
		//检测现在有没有财神降临活动(有任务系统的玩家是真正的玩家)
		CActivityMgr::get_instance().UserCheckDoingWealthGodComing(pPlayGame);
	}
	break;
	case SUB_G2CS_FISH_FIRE_COUNT:
	{
		if (wDataSize != sizeof(MSG_G2CS_PlayGame))
		{
			return;
		}
		MSG_G2CS_PlayGame* pPlayGame = (MSG_G2CS_PlayGame*)pData;
		CRedPacketMgr::get_instance().CollectUserGameData(pPlayGame);
	}
	break;
    case SUB_G2CS_SEND_MAIL:
    {
        if (wDataSize != sizeof(tagUserMail))
        {
            return;
        }

        tagUserMail* pUserMail = (tagUserMail*)pData;
        CUserMgr::get_instance().SendNewMailTip(pUserMail->dwUserID);
        CUserMgr::get_instance().SendUserMail(pUserMail);
    }
    break;
    case SUB_G2CS_UPDATE_GROUP_PROOM_USER:
    {
        MSG_G2CS_GroupPRoom_Update* pPRoomUpdate = (MSG_G2CS_GroupPRoom_Update*)pData;
        CGroupMgr::get_instance().UpdateGroupPRoomUser(pPRoomUpdate);
        break;
    }
    case SUB_G2CS_REMOVE_GROUP_PROOM:
    {
        MSG_G2CS_Remove_GroupPRoom* pRemovePRoom = (MSG_G2CS_Remove_GroupPRoom*)pData;
        CGroupMgr::get_instance().RemoveGroupPRoom(pRemovePRoom);
        break;
    }
        default:
		break;
	}
}

CT_VOID CCenterThread::OnLoginServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	    case SUB_L2CS_REGISTER:
	    {
	    	if (wDataSize != sizeof(CMD_LoginServer_Info))
	    	{
	    		return;
	    	}

	    	CMD_LoginServer_Info* pLoginServer = (CMD_LoginServer_Info*)pData;
	    	CServerMgr::get_instance().AddLoginServerInfo(pSocket, pLoginServer);
	    }
	    break;
	    case SUB_L2CS_ADD_USER_GEM:
	    {
	    	if (wDataSize != sizeof(MSG_UpdateUser_Gem))
	    	{
	    		return;
	    	}
	    	MSG_UpdateUser_Gem* pUpdataGem = (MSG_UpdateUser_Gem*)pData;
	    	CUserMgr::get_instance().AddUserGemEx(pUpdataGem->dwUserID, pUpdataGem->nAddGem, pUpdataGem->bUpdateToClient);
	    }
	    break;
	    case SUB_L2CS_ADD_USER_SCORE:
	    {
	    	if (wDataSize != sizeof(MSG_UpdateUser_Score))
	    	{
	    		return;
	    	}
	    	MSG_UpdateUser_Score* pUpdataGem = (MSG_UpdateUser_Score*)pData;
	    	CUserMgr::get_instance().AddUserScoreEx(pUpdataGem->dwUserID, pUpdataGem->llAddScore, pUpdataGem->bUpdateToClient);
	    }
	    break;
	    case SUB_L2CS_ADD_USER_BANKSCORE:
	    {
	    	if (wDataSize != sizeof(MSG_UpdateUser_BankScore))
	    	{
	    		return;
	    	}

	    	MSG_UpdateUser_BankScore* pUpdataBankScore = (MSG_UpdateUser_BankScore*)pData;
	    	CUserMgr::get_instance().AddUserBankScoreEx(pUpdataBankScore->dwUserID, pUpdataBankScore->llAddBankScore, pUpdataBankScore->bUpdateToClient);
	    }
	    break;
	    case SUB_L2CS_ADD_GROUP_USER:
        {
            if (wDataSize != sizeof(MSG_L2CS_AddGroupUser))
            {
                return;
            }

            MSG_L2CS_AddGroupUser* addGroupUser = (MSG_L2CS_AddGroupUser*)pData;
            OnLoginServerAddGroupUser(addGroupUser);
            break;
        }

	default:
		break;
	}
}

//客服服消息
CT_VOID CCenterThread::OnCustomerServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CUSTOMER2CS_REGSTER:
	{
		if (wDataSize != sizeof(CMD_CustomerServer_Info))
		{
			return;
		}

		CMD_CustomerServer_Info* pCustomerServer = (CMD_CustomerServer_Info*)pData;
		CServerMgr::get_instance().AddCustomerServerInfo(pSocket, pCustomerServer);
	}
	break;
	}
}

CT_VOID CCenterThread::OnDipServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_DIP2CS_REGSTER:
	{
		if (wDataSize != sizeof(CMD_DipServer_Info))
		{
			return;
		}
		CMD_DipServer_Info* pServerInfo = (CMD_DipServer_Info*)pData;
		CServerMgr::get_instance().AddDipServerInfo(pSocket, pServerInfo);
	}
	break;
	case SUB_DIP2CS_RECHARGE:
	{
		if (wDataSize != sizeof(MSG_D2CS_User_Recharge))
		{
			return;
		}

		MSG_D2CS_User_Recharge* pRecharge = (MSG_D2CS_User_Recharge*)pData;
		if (pRecharge == NULL)
		{
			return;
		}

		if (pRecharge->cbType == RECHARGE_SCORE)
		{
			CT_BOOL bSucc = UpdateUserScore(pRecharge->dwUserID, pRecharge->llRecharge, pRecharge->dwTotalCash);
			if (pRecharge->llRecharge > 0 && bSucc)
				CUserMgr::get_instance().UserRecharge(pRecharge->dwUserID, pRecharge->llRecharge, pRecharge->cbType);
		}
		else if (pRecharge->cbType == RECHARGE_GEM)
		{
			long long llNewGem = 0;
			UpdateUserGem(pRecharge->dwUserID, (int)pRecharge->llRecharge, llNewGem);
		}
		else if (pRecharge->cbType == RECHARGE_BANK)
		{
			long long llNewBankScore = 0;
			UpdateUserBankScore(pRecharge->dwUserID, pRecharge->llRecharge, llNewBankScore);
		}
		else if(pRecharge->cbType == RECHARGE_FAIL)
		{
			CUserMgr::get_instance().UserRecharge(pRecharge->dwUserID, pRecharge->llRecharge, pRecharge->cbType);
		}

		//LOG(WARNING) << "score mode : " << (CT_DWORD)m_SystemStatus.cbScoreMode << ", curr cash: " << pRecharge->wCurrCash;
		//如果有积分场，则设置玩家的vip信息
		if (/*m_SystemStatus.cbScoreMode &&*/ pRecharge->wCurrCash != 0)
		{
			CUserMgr::get_instance().UpdateUserVipLevel(pRecharge->dwUserID, pRecharge->llRecharge, pRecharge->dwTotalCash, pRecharge->cbVipLevel, pRecharge->cbVipLevel2);
			/*CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pRecharge->dwUserID);
			if (pUserTask)
			{
				pUserTask->OnUserRecharge(pRecharge->dwTotalCash);
			}*/
		}
	}
	break;
	case SUB_DIP2CS_WX_CREATE_PROOM:
	{
		if (wDataSize != sizeof(MSG_D2CS_AutoCreate_PRoom))
		{
			return;
		}

		MSG_D2CS_AutoCreate_PRoom *pAutoPRoom = (MSG_D2CS_AutoCreate_PRoom*)pData;
		if (pAutoPRoom == NULL)
		{
			return;
		}

		acl::json proomJson(pAutoPRoom->szJsonParam);
		const acl::json_node* pGroupID = proomJson.getFirstElementByTagName("gid");
		if (pGroupID == NULL)
		{
			return;
		}
		CT_DWORD dwGroupID = (CT_DWORD)*pGroupID->get_int64();

		const acl::json_node* pRerordID = proomJson.getFirstElementByTagName("rid");
		if (pRerordID == NULL)
		{
			return;
		}
		CT_DWORD dwRecordID = (CT_DWORD)*pRerordID->get_int64();

		const acl::json_node* pNodeGameID = proomJson.getFirstElementByTagName("gameid");
		if (pNodeGameID == NULL)
		{
			return;
		}
		CT_WORD wGameID = (CT_WORD)*pNodeGameID->get_int64();

		const acl::json_node* pNodeKindID = proomJson.getFirstElementByTagName("kindid");
		if (pNodeKindID == NULL)
		{
			return;
		}
		CT_WORD wKindID = (CT_WORD)*pNodeKindID->get_int64();

		CT_DWORD dwRoom = 0;
		CT_WORD	 wCurrUserCount = 0;
		CT_WORD wNeedGem = 0;
		//std::cout << "create auto room: " << pAutoPRoom->szJsonParam << std::endl;
		CT_BOOL bHasFreeRoom = FindPRoomInfo(dwGroupID, wGameID, wKindID, /*pAutoPRoom->szJsonParam,*/ dwRoom, wCurrUserCount);
		if (!bHasFreeRoom)
		{
			/*const acl::json_node* pNodeVip = proomJson.getFirstElementByTagName("vip");
			if (pNodeVip == NULL)
			{
			return;
			}*/

			CT_BYTE cbIsVip = 0; //(CT_BYTE)*pNodeVip->get_int64();


								 /*CT_WORD wMaxHuXi = 0;
								 if (wGameID == GAME_PHZ)
								 {
								 const acl::json_node* pMaxHuXi = proomJson.getFirstElementByTagName("max_huxi");
								 if (pMaxHuXi == NULL)
								 {
								 return;
								 }
								 wMaxHuXi = (CT_WORD)*pMaxHuXi->get_int64();
								 }*/

			CMD_GameServer_Info *pGameServer = CServerMgr::get_instance().FindSuitTableGameServer(wGameID, wKindID, 0);
			if (pGameServer == NULL)
			{
				LOG(WARNING) << "auto create room not find suit game server";

				MSG_CS2D_CreatePRoom_Failed createFail;
				createFail.cbErrorCode = EN_GAMESERVER_NOENOUGH;
				createFail.uWebSock = pAutoPRoom->uWebSock;
				CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_CREATE_PROOM_FAIL, &createFail, sizeof(createFail));
				return;
			}

			CT_WORD wPlayCount = 0;
			const acl::json_node* pPlayCount = proomJson.getFirstElementByTagName("playCount");
			if (pPlayCount == NULL)
			{
				LOG(WARNING) << "auto create room can not find play count param!";
				return;
			}
			wPlayCount = (CT_WORD)*pPlayCount->get_int64();

			wNeedGem = CServerMgr::get_instance().GetCreatePRoomNeedGem(pGameServer->dwServerID, wPlayCount);

			CT_DWORD dwSeed = (CT_DWORD)time(NULL);
			srand(dwSeed);
			dwRoom = CreateRoomNum(pGameServer->dwServerID);
			while (CheckRoomNumIsExist(dwRoom))
			{
				dwSeed += 9527;
				srand(dwSeed);
				dwRoom = CreateRoomNum(pGameServer->dwServerID);
				LOG(WARNING) << "create same room num!!!";
			}

			CT_WORD wUserCount = pGameServer->wMaxUserCount;
			const acl::json_node* pUserCount = proomJson.getFirstElementByTagName("userCount");
			if (pUserCount != NULL)
			{
				wUserCount = (CT_WORD)*pUserCount->get_int64();
			}


			CT_BOOL bSetCache = SetRoomNumToCache(0, pGameServer->dwServerID, pGameServer->wGameID, pGameServer->wKindID, \
				wUserCount, wPlayCount, /*wMaxHuXi, 100,*/ cbIsVip, pAutoPRoom->szJsonParam, dwRoom,/* 1,*/ dwGroupID, dwRecordID, WX_GROUP_ROOM);
			if (!bSetCache)
			{
				LOG(WARNING) << "set room info to cache fail.";
			}

			std::shared_ptr<tagGroupPRoomInfo> pRoomInfo = std::make_shared<tagGroupPRoomInfo>();
			pRoomInfo->wGameID = pGameServer->wGameID;
			pRoomInfo->wKindID = pGameServer->wKindID;
			pRoomInfo->dwRoomNum = dwRoom;
			//_snprintf_info(pRoomInfo->szParam, sizeof(pRoomInfo->szParam), "%s", pAutoPRoom->szJsonParam);
			AddPRoomNum(dwGroupID, pRoomInfo);
		}

		MSG_CS2D_CreatePRoom_PROOM_Succ createSucc;
		createSucc.dwRoomNum = dwRoom;
		createSucc.wNeedGem = wNeedGem;
		createSucc.wCurrUserCount = wCurrUserCount;
		createSucc.uWebSock = pAutoPRoom->uWebSock;
		CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_CREATE_PROOM_SUCC, &createSucc, sizeof(createSucc));
	}
	break;
	case SUB_DIP2CS_WX_ADD_GROUP_USER:
	case SUB_DIP2CS_WX_REMOVE_GROUP_USER:
	{
		if (wDataSize < sizeof(MSG_D2CS_AddOrRemove_GroupUser))
		{
			return;
		}

		MSG_D2CS_AddOrRemove_GroupUser* pUserList = (MSG_D2CS_AddOrRemove_GroupUser*)pData;
		if (pUserList == NULL)
		{
			return;
		}

		CT_DWORD* pUserID = (CT_DWORD*)((CT_BYTE*)pData + sizeof(MSG_D2CS_AddOrRemove_GroupUser));
		CT_WORD wUserCount = pUserList->wUserCount;
		while (wUserCount > 0)
		{
			if (wSubCmdID == SUB_DIP2CS_WX_ADD_GROUP_USER)
			{
				m_wxGroupMgr.InsertGroupUser(pUserList->dwGroupID, *pUserID);
			}
			else if (wSubCmdID == SUB_DIP2CS_WX_REMOVE_GROUP_USER)
			{
				m_wxGroupMgr.RemoveGroupUser(pUserList->dwGroupID, *pUserID);
			}
			wUserCount--;
			pUserID++;
		}
		m_wxGroupMgr.InsertGroupInfo(pUserList->dwGroupID, pUserList->dwBindUserPlay);

		const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
		if (pMapGameServer)
		{
			auto it = pMapGameServer->begin();
			for (; it != pMapGameServer->end(); ++it)
			{
				if (wSubCmdID == SUB_DIP2CS_WX_ADD_GROUP_USER)
				{
					CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_ADD_WXGROUP_USER, pData, wDataSize);
				}
				else if (wSubCmdID == SUB_DIP2CS_WX_REMOVE_GROUP_USER)
				{
					CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_REMOVE_WXGROUP_USER, pData, wDataSize);
				}
			}
		}
	}
	break;
	case SUB_DIP2CS_UPDATE_GROUP_OPTIONS:
	{
		if (wDataSize !=  sizeof(MSG_D2CS_Update_GroupOptions))
		{
			return;
		}

		MSG_D2CS_Update_GroupOptions *pGroupOptions = (MSG_D2CS_Update_GroupOptions*)pData;
		if (pGroupOptions == NULL)
		{
			return;
		}

		CGroupMgr::get_instance().ModifyGroupOptions(pGroupOptions->dwGroupID, pGroupOptions->wBindUserPlay, pGroupOptions->dwDefaultKindID);
		
	}
	break;
	case SUB_DIP2CS_UPDATE_GROUP_ROOM_CFG:
	{
	    /*
		if (wDataSize != sizeof(MSG_D2CS_Update_GroupRoom_Cfg))
		{
			return;
		}
		
		MSG_D2CS_Update_GroupRoom_Cfg* pRoomCfg = (MSG_D2CS_Update_GroupRoom_Cfg*)pData;
		if (pRoomCfg == NULL)
		{
			return;
		}

		tagGroupRoomConfig roomConfig;
		roomConfig.dwGroupID = pRoomCfg->dwGroupID;
		roomConfig.wGameID = pRoomCfg->wGameID;
		roomConfig.wKindID = pRoomCfg->wKindID;
		_snprintf_info(roomConfig.szConfig, sizeof(roomConfig.szConfig), "%s", pRoomCfg->szPRoomCfg);
		CGroupMgr::get_instance().InsertGroupPRoomConfig(&roomConfig);
	     */
	}
	break;
	case SUB_DIP2CS_REGSTER_PROMOTER:
	{
		if (wDataSize != sizeof(MSG_D2CS_Regster_Promoter))
		{
			return;
		}
		MSG_D2CS_Regster_Promoter* pPromoter = (MSG_D2CS_Regster_Promoter*)pData;
		if (pPromoter == NULL)
		{
			return;
		}

		SetUserPromoterID(pPromoter->dwUserID, pPromoter->dwPromoterID);
	}
	break;
	case SUB_DIP2CS_QUERY_PROOM_INFO:
	{
		if (wDataSize != sizeof(MSG_D2CS_Query_PRoomInfo))
		{
			return;
		}

		MSG_D2CS_Query_PRoomInfo* pQueryInfo = (MSG_D2CS_Query_PRoomInfo*)pData;
		if (pQueryInfo == NULL)
		{
			return;
		}

		PrivateRoomInfo proomInfo;
		CT_BOOL bSucc = GetPRoomInfo(pQueryInfo->dwRoomNum, proomInfo);
		if (!bSucc)
		{
			MSG_CS2D_QueryPRoom_Info proomInfo;
			memset(&proomInfo, 0, sizeof(proomInfo));
			proomInfo.cbResult = 1;
			proomInfo.uWebSock = pQueryInfo->uWebSock;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_PROOM_INFO, &proomInfo, sizeof(proomInfo));
			return;
		}

		CT_CHAR szGroupID[32] = { 0 };
		_snprintf_info(szGroupID, 32, "%u", proomInfo.dwGroupID);
		std::string strGroupID(szGroupID);

		std::string strGroupIDList(pQueryInfo->szGroupIDList);
		std::vector<std::string> vecGroupID;
		Utility::stringSplit(strGroupIDList, ",", vecGroupID);

		auto it = std::find(vecGroupID.begin(), vecGroupID.end(), strGroupID);
		if (it == vecGroupID.end())
		{
			MSG_CS2D_QueryPRoom_Info proomInfo;
			memset(&proomInfo, 0, sizeof(proomInfo));
			proomInfo.cbResult = 2;
			proomInfo.uWebSock = pQueryInfo->uWebSock;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_PROOM_INFO, &proomInfo, sizeof(proomInfo));
			return;
		}
		
		//查找房间所在的服务器
		acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(proomInfo.dwServerID);
		if (pSocket)
		{
			MSG_C2GS_Query_PRoomInfo queryPRoomInfo;
			queryPRoomInfo.dwRoomNum = pQueryInfo->dwRoomNum;
			queryPRoomInfo.uWebSock = pQueryInfo->uWebSock;
			CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_QUERY_PROOM_INFO, &queryPRoomInfo, sizeof(queryPRoomInfo));
		}	
	}
	break;
	case SUB_DIP2CS_DISMISS_PROOM:
	{
		if (wDataSize != sizeof(MSG_D2CS_Dismiss_PRoom))
		{
			return;
		}

		MSG_D2CS_Dismiss_PRoom* pDismissPRoom = (MSG_D2CS_Dismiss_PRoom*)pData;
		if (pDismissPRoom == NULL)
		{
			return;
		}

		PrivateRoomInfo proomInfo;
		CT_BOOL bSucc = GetPRoomInfo(pDismissPRoom->dwRoomNum, proomInfo);
		if (!bSucc)
		{
			MSG_CS2D_Dismiss_PRoom proomInfo;
			memset(&proomInfo, 0, sizeof(proomInfo));
			proomInfo.cbResult = 1;
			proomInfo.uWebSock = pDismissPRoom->uWebSock;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_DISMISS_PROOM, &proomInfo, sizeof(proomInfo));
			return;
		}

		CT_CHAR szGroupID[32] = { 0 };
		_snprintf_info(szGroupID, 32, "%u", proomInfo.dwGroupID);
		std::string strGroupID(szGroupID);

		std::string strGroupIDList(pDismissPRoom->szGroupIDList);
		std::vector<std::string> vecGroupID;
		Utility::stringSplit(strGroupIDList, ",", vecGroupID);

		auto it = std::find(vecGroupID.begin(), vecGroupID.end(), strGroupID);
		if (it == vecGroupID.end())
		{
			MSG_CS2D_QueryPRoom_Info proomInfo;
			memset(&proomInfo, 0, sizeof(proomInfo));
			proomInfo.cbResult = 2;
			proomInfo.uWebSock = pDismissPRoom->uWebSock;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_DISMISS_PROOM, &proomInfo, sizeof(proomInfo));
			return;
		}

		//查找房间所在的服务器
		acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(proomInfo.dwServerID);
		if (pSocket)
		{
			MSG_C2GS_DismissPRoom_ForAgent dismissPRoom;
			dismissPRoom.dwRoomNum = pDismissPRoom->dwRoomNum;
			dismissPRoom.uWebSock = pDismissPRoom->uWebSock;
			dismissPRoom.dwGroupID = proomInfo.dwGroupID;
			dismissPRoom.dwRecordID = proomInfo.dwRecordID;
			dismissPRoom.cbGroupType = proomInfo.cbGroupType;
			dismissPRoom.wGameID = proomInfo.wGameID;
			dismissPRoom.wKindID = proomInfo.wKindID;
			dismissPRoom.wPlayCount = proomInfo.wPlayCount;
			dismissPRoom.cbDismissType = 1;
			dismissPRoom.dwMasterID = CGroupMgr::get_instance().GetGroupMasterID(proomInfo.dwGroupID);
			CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_DISMISS_PROOM_AGENT, &dismissPRoom, sizeof(dismissPRoom));
		}
	}
	break;
	case SUB_DIP2CS_TICK_USER:
	{
		if (wDataSize != sizeof(MSG_D2CS_Tick_User))
		{
			return;
		}

		MSG_D2CS_Tick_User* pTickUser = (MSG_D2CS_Tick_User*)pData;
		if (pTickUser == NULL)
		{
			return;
		}

		std::vector<std::string> vecUserID;
		Utility::stringSplit(pTickUser->szUserList, ",", vecUserID);

		CT_DWORD dwRoomNum1 = 0;
		CT_DWORD dwRoomNum2 = 0;

		CT_DWORD dwGroupID1 = 0;
		CT_DWORD dwGroupID2 = 0;

		CT_DWORD dwServerID = 0;
		for (auto it : vecUserID)
		{
			CT_DWORD dwUserID = atoi(it.c_str());
			CT_DWORD dwRoomNum = GetUserRoomNum(dwUserID);
			if (dwRoomNum == 0)
			{
				MSG_CS2D_Tick_User tickUserResult;
				tickUserResult.uWebSock = pTickUser->uWebSock;
				tickUserResult.cbResult = 1;
				CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_TICK_USER, &tickUserResult, sizeof(tickUserResult));
				return;
			}
			if (dwRoomNum1 == 0)
			{
				dwRoomNum1 = dwRoomNum;
			}

			dwRoomNum2 = dwRoomNum;

			if (dwRoomNum1 != dwRoomNum2)
			{
				MSG_CS2D_Tick_User tickUserResult;
				tickUserResult.uWebSock = pTickUser->uWebSock;
				tickUserResult.cbResult = 1;
				CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_TICK_USER, &tickUserResult, sizeof(tickUserResult));
				return;
			}

			PrivateRoomInfo proomInfo;
			CT_BOOL bSucc = GetPRoomInfo(dwRoomNum, proomInfo);
			if (!bSucc)
			{
				MSG_CS2D_Tick_User tickUserResult;
				tickUserResult.uWebSock = pTickUser->uWebSock;
				tickUserResult.cbResult = 1;
				CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_TICK_USER, &tickUserResult, sizeof(tickUserResult));
				return;
			}

			if (dwGroupID1 == 0)
			{
				dwGroupID1 = proomInfo.dwGroupID;
			}

			dwGroupID2 = proomInfo.dwGroupID;
			if (dwGroupID2 != dwGroupID1)
			{
				MSG_CS2D_Tick_User tickUserResult;
				tickUserResult.uWebSock = pTickUser->uWebSock;
				tickUserResult.cbResult = 1;
				CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_TICK_USER, &tickUserResult, sizeof(tickUserResult));
				return;
			}

			if (dwServerID == 0)
			{
				dwServerID = proomInfo.dwServerID;
			}
		}
		
		//acl::string strGroupID;
		//strGroupID.format("%d", dwGroupID1);
		//acl::string strGroupIDList(pTickUser->szGroupIDList);
		//if (strGroupIDList.find(strGroupID.c_str()) == NULL)
		
		CT_CHAR szGroupID[32] = { 0 };
		_snprintf_info(szGroupID, 32, "%u", dwGroupID1);
		std::string strGroupID(szGroupID);

		std::string strGroupIDList(pTickUser->szGroupIDList);
		std::vector<std::string> vecGroupID;
		Utility::stringSplit(strGroupIDList, ",", vecGroupID);

		auto it = std::find(vecGroupID.begin(), vecGroupID.end(), strGroupID);
		if (it == vecGroupID.end())
		{
			MSG_CS2D_Tick_User tickUserResult;
			tickUserResult.uWebSock = pTickUser->uWebSock;
			tickUserResult.cbResult = 2;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_TICK_USER, &tickUserResult, sizeof(tickUserResult));
			return;
		}

		//查找房间所在的服务器
		acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(dwServerID);
		if (pSocket)
		{
			MSG_C2GS_TickUser_ForAgent tickUser;
			_snprintf_info(tickUser.szUserList, sizeof(tickUser.szUserList), "%s", pTickUser->szUserList);
			tickUser.uWebSock = pTickUser->uWebSock;
			CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_TICK_USER_AGENT, &tickUser, sizeof(tickUser));
		}
	}
	break;
	case SUB_DIP2CS_APPLY_ADD_GROUP:
	{
		if (wDataSize != sizeof(MSG_D2CS_Apply_Add_Group))
		{
			return;
		}
		MSG_D2CS_Apply_Add_Group* pAddGroup = (MSG_D2CS_Apply_Add_Group*)pData;
		if (pAddGroup == NULL)
		{
			return;
		}
		MSG_CS2D_Apply_Add_Group addGroupResult;
		addGroupResult.uWebSock = pAddGroup->uWebSock;
		//检查是否有这个群
		CT_BOOL bHasGroup = CGroupMgr::get_instance().HasGroup(pAddGroup->dwGroupID);
		if (!bHasGroup)
		{
			addGroupResult.cbResult = 1;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_APPLY_ADD_GROUP, &addGroupResult, sizeof(addGroupResult));
			return;
		}

		//检查玩家是否已经群成员
		CT_BOOL bIsGroupUser = CGroupMgr::get_instance().IsGroupUser(pAddGroup->dwGroupID, pAddGroup->dwUserID);
		if (bIsGroupUser)
		{
			addGroupResult.cbResult = 2;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_APPLY_ADD_GROUP, &addGroupResult, sizeof(addGroupResult));
			return;
		}

		//检测是否已经申请
		CT_BOOL bIsApplyUser = CGroupMgr::get_instance().IsApplyAddGroup(pAddGroup->dwGroupID, pAddGroup->dwUserID);
		if (bIsApplyUser)
		{
			addGroupResult.cbResult = 3;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_APPLY_ADD_GROUP, &addGroupResult, sizeof(addGroupResult));
			return;
		}

		//检查群是否满员
		CT_DWORD dwGroupUserCount = CGroupMgr::get_instance().GetGroupUserCount(pAddGroup->dwGroupID);
		if (dwGroupUserCount >= MAX_GROUP_USER_COUNT)
		{
			addGroupResult.cbResult = 4;
			CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_APPLY_ADD_GROUP, &addGroupResult, sizeof(addGroupResult));
			return;
		}

		//检查玩家在不在线,不在线则不处理
		tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pAddGroup->dwUserID);
		if (pUserInfo == NULL)
		{
			MSG_CS2DB_Query_BeAdd_GroupUser beAddUser;
			beAddUser.dwGroupID = pAddGroup->dwGroupID;
			beAddUser.dwUserID = pAddGroup->dwUserID;
			beAddUser.dwOperateUserID = 0;
			beAddUser.cbType = 3;
			CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_QUERY_BE_ADD_GROUP_USER, &beAddUser, sizeof(beAddUser));
		}
		else
		{
			//CGroupMgr::get_instance().InsertApplyGroupInfo(pAddGroup->dwGroupID, pAddGroup->dwUserID, pUserInfo->szNickName, pUserInfo->szHeadUrl, 1, 1);
		}
		addGroupResult.cbResult = 0;
		CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_APPLY_ADD_GROUP, &addGroupResult, sizeof(addGroupResult));
	}
	break;
	case SUB_DIP2CS_QUERY_ONLINE_USER:
	{
		CUserMgr::get_instance().QueryOnlineUser(pData, wDataSize, pSocket);
	}
	break;
    case SUB_DIP2CS_QUERY_WAITLLIST_USER:
    {
        OnSubDip2csQueryWaitListUser(pData, wDataSize, pSocket);
    }
    break;
    case SUB_DIP2CS_QUERY_SERVERID_TABLE_MSG:
    {
        OnSubDip2csQueryTableMsg(pData, wDataSize, pSocket);
    }
    break;
	case SUB_DIP2CS_UPDATE_ANDROID:
	{
		OnSubUpdateAndroid(pData, wDataSize, pSocket);
	}
	break;
	case SUB_DIP2CS_UPDATE_ANDROID_PLAY_COUNT:
	{
		OnSubUpdateAndroidpPlayCount(pData, wDataSize, pSocket);
	}
	break;
	case SUB_DIP2CS_RELOAD_RECHARGE_CHANNEL:
	{
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_RELOAD_RECHARGE_CHANNEL);
	}
	break;
	case SUB_DIP2CS_NOTIFY_NEWMAIL:
	{
		CUserMgr::get_instance().NotifyNewMail(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_SEND_SYSMSG:
	{
		CSystemMsgMgr::get_instance().InsertSystemMsg(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_PROMOTER_LIST:
	{
		OnSubDipUpdateRechargePromoterList(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_EXCHANGE_CHANNEL_STATUS:
	{
		OnDipSubUpdateExchangeChannelStatus(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_UPDATE_USER_STATUS:
	{
		OnDipSubUpdateUserStatus(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_GENERALIZE_PROMOTER:
	{
		OnDipSubUpdateGeneralizePromoter(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_CLIENTCHANNEL_DOMAIN:
	{
		OnDipSubUpdateClientChannelDomain(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_QUANMIN_INFO:
	{
		OnDipSubUpdateQuanMinChannel(pData, wDataSize);
	}
	break;
  	case SUB_DIP2CS_CHANNEL_PRESENTSCORE:
  	{
		OnDipSubUpdateChannelPresentScore(pData, wDataSize);
  	}
	break;
	case SUB_DIP2CS_UPDATE_SERVER_STOCK:
	{
		OnSubUpdateServerStockInfo(pData, wDataSize, pSocket);
	}
	break;
	case SUB_DIP2CS_REMOVE_SYSMSG:
	{
		CSystemMsgMgr::get_instance().RemoveSystemMsg(pData, wDataSize);
	}
	break;
    case SUB_DIP2CS_TICK_FISH_USER:
    {
        OnSubDipTickFishUser(pData, wDataSize, pSocket);
    }
    break;
	case SUB_DIP2CS_RATIO_CONTROL_FOR_BRNN:
	{
		OnSubBRGameRationControl(pData, wDataSize, pSocket);
	}
	break;
	case SUB_DIP2CS_BRGAME_BLACKLIST:
	{
		OnSubBRGameBlackList(pData, wDataSize, pSocket);
	}
	break;
    case SUB_DIP2CS_WEALTHGODCOMING_CFG:
    {
    	OnSubDipUpdateWealthGodComing(pData, wDataSize);
    }
    break;
    case SUB_DIP2CS_WEALTHGODCOMING_REWARD:
    {
    	OnSubDipUpdateWealthGodComingRank(pData, wDataSize);
    }
    break;
    case SUB_DIP2CS_RECHARGE_AMOUNT:
    {
		OnDipSubUpdateRechargeAmount(pData, wDataSize);
    }
    break;
    case SUB_DIP2CS_EXCHANGE_AMOUNT:
    {
		OnDipSubUpdateExchangeAmount(pData, wDataSize);
    }
    break;
	case SUB_DIP2CS_FISH_DIANKONG:
	{
		OnDipSubSetFishDianKong(pData, wDataSize);
	}
	break;
	case SUB_DIP2CS_ZJH_CARDRATIO:
	{
		OnSubDipUpdateZjhCardRatio(pData, wDataSize);
	}
	break;
    case SUB_DIP2CS_DUOBAO_CFG:
    {
        OnSubDipUpdateDuoBaoCfg(pData, wDataSize);
        break;
    }
	case SUB_DIP2CS_UPDATE_BENEFIT_CFG:
	{
		OnSubDipUpdateBenefitCfg(pData, wDataSize);
		break;
	}
    case SUB_DIP2CS_UPDATE_PLATFORM_RECHARGE_CFG:
    {
		OnSubDipUpdatePlatformRechargeCfg(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_UPDATE_SMS_PID:
    {
		OnSubDipUpdateSMSPid(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_REDPACKET_STATUS:
    {
        OnSubDipUpdateRedPacketStatus(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_REDPACKET_ROOM_CFG:
    {
        OnSubDipUpdateRedPacketRoomCfg(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_REDPACKET_INDEX:
    {
        OnSubDipUpdateRedPacketIndex(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_CHAT_UPLOAD_URL:
    {
		OnSubDipUpdateChatUploadUrl(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_UPDATE_ANNOUNCEMENT:
    {
        OnSubDipUpdateAnnouncement(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_UPDATE_BLACK_CHANNEL:
    {
        OnSubDipUpdateBlackChannel(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_UPDATE_GROUP_INFO:
    {
        CGroupMgr::get_instance().UpdateGroupInfoForDip(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_UPDATE_GROUP_USER_INFO:
    {
        CGroupMgr::get_instance().UpdateGroupUserInfoForDip(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_UPDATE_GROUP_LEVEL_CFG:
    {
        CGroupMgr::get_instance().UpdateGroupLevelConfigForDip(pData, wDataSize);
        break;
    }
    case SUB_DIP2CS_ADD_GROUP_USER:
    {
        if (wDataSize != sizeof(MSG_L2CS_AddGroupUser))
        {
            return;
        }
        MSG_L2CS_AddGroupUser* pAddGroupUser = (MSG_L2CS_AddGroupUser*)pData;
        OnLoginServerAddGroupUser(pAddGroupUser);
        break;
    }
    default:break;
	}
}

//维服工具
CT_VOID CCenterThread::OnMaintainMsg(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_M2CS_QUERY_ALL_SERVER:
	{
		if (wDataSize != sizeof(MSG_M2CS_QueryServer))
		{
			return;
		}

		CServerMgr::get_instance().SendGameServerList(pSocket, MSG_MCS_MAIN, SUB_CS2M_QUERY_ALL_SERVER, SUB_CS2M_QUERY_SERVER_FINISH);
	}
	break;
	case SUB_M2CS_SET_SERVER_STATE:
	{
		if (wDataSize != sizeof(MSG_M2CS_SetServerState))
		{
			return;
		}

		MSG_M2CS_SetServerState* pGSState = (MSG_M2CS_SetServerState*)pData;
		CServerMgr::get_instance().SetGameServerState(pGSState->dwServerID, pGSState->cbState);

		CNetModule::getSingleton().Send(pSocket, MSG_MCS_MAIN, SUB_CS2M_SET_SERVER_STATE_SUCC, pGSState, sizeof(MSG_M2CS_SetServerState));
	}
	break;
	case SUB_M2CS_DISMISS_SERVER_PROOM:
	{
		if (wDataSize != sizeof(MSG_M2CS_DismissPRoom))
		{
			return;
		}

		MSG_M2CS_DismissPRoom* pDismissPRoom = (MSG_M2CS_DismissPRoom*)pData;

		//CServerMgr::get_instance().DismissServerPRoom(pDismissPRoom->dwServerID, 0);
		for (auto& it : m_mapPRoomInfo)
		{
			//超过半个小时的房间
			stPRoomInfo& proomInfo = it.second;
			MSG_C2GS_DismissPRoom_ForAgent dismissPRoom;
			dismissPRoom.uWebSock = 0;

			if (pDismissPRoom->dwServerID == 0 || pDismissPRoom->dwServerID==proomInfo.dwServerID)
			{
				acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(proomInfo.dwServerID);
				if (pSocket)
				{
					dismissPRoom.dwRoomNum = it.first;
					dismissPRoom.dwGroupID = proomInfo.dwGroupID;
					dismissPRoom.dwRecordID = proomInfo.dwRecordID;
					dismissPRoom.cbGroupType = proomInfo.cbGroupType;
					dismissPRoom.wGameID = proomInfo.wGameID;
					dismissPRoom.wKindID = proomInfo.wKindID;
					dismissPRoom.wPlayCount = proomInfo.wPlayCount;
					dismissPRoom.dwMasterID = proomInfo.dwMasterID;
					dismissPRoom.cbDismissType = 2;
					CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_DISMISS_PROOM_AGENT, &dismissPRoom, sizeof(dismissPRoom));
				}
			}
		}

		CNetModule::getSingleton().Send(pSocket, MSG_MCS_MAIN, SUB_CS2M_DISMISS_SERVER_RPOOM_SUCC, pDismissPRoom, sizeof(MSG_M2CS_DismissPRoom));
	}
	break;
	case SUB_M2CS_DISMISS_SERVER_PROOM_BY_ROOMNUM:
	{
		if (wDataSize != sizeof(MSG_M2CS_DismissPRoom_ByRoomNum))
		{
			return;
		}

		MSG_M2CS_DismissPRoom_ByRoomNum* pDismissPRoom = (MSG_M2CS_DismissPRoom_ByRoomNum*)pData;
		//PrivateRoomInfo roomInfo;
		//CT_BOOL bSucc = GetPRoomInfo(pDismissPRoom->dwRoomNum, roomInfo);
		//if (bSucc)
		//{
		//	CServerMgr::get_instance().DismissServerPRoom(roomInfo.dwServerID, pDismissPRoom->dwRoomNum);
		//}
		auto it = m_mapPRoomInfo.find(pDismissPRoom->dwRoomNum);
		if (it != m_mapPRoomInfo.end())
		{
			stPRoomInfo& proomInfo = it->second;
			MSG_C2GS_DismissPRoom_ForAgent dismissPRoom;
			dismissPRoom.uWebSock = 0;
			acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(proomInfo.dwServerID);
			if (pSocket)
			{
				dismissPRoom.dwRoomNum = pDismissPRoom->dwRoomNum;
				dismissPRoom.dwGroupID = proomInfo.dwGroupID;
				dismissPRoom.dwRecordID = proomInfo.dwRecordID;
				dismissPRoom.cbGroupType = proomInfo.cbGroupType;
				dismissPRoom.wGameID = proomInfo.wGameID;
				dismissPRoom.wKindID = proomInfo.wKindID;
				dismissPRoom.wPlayCount = proomInfo.wPlayCount;
				dismissPRoom.dwMasterID = proomInfo.dwMasterID;
				dismissPRoom.cbDismissType = 2;
				CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_DISMISS_PROOM_AGENT, &dismissPRoom, sizeof(dismissPRoom));
			}
		}
	
		CNetModule::getSingleton().Send(pSocket, MSG_MCS_MAIN, SUB_CS2M_DISMISS_SERVER_RPOOM_BYNUM_SUCC, pDismissPRoom, sizeof(MSG_M2CS_DismissPRoom_ByRoomNum));
	}
	break;
	case SUB_M2CS_REMOVE_USER_OUT_CACHE:
	{
		if (wDataSize != sizeof(MSG_M2CS_RemoveUserOutCache))
		{
			return;
		}

		MSG_M2CS_RemoveUserOutCache* pRemoveUser = (MSG_M2CS_RemoveUserOutCache*)pData;

		CT_DWORD dwRoomNum = GetUserRoomNum(pRemoveUser->dwUserID);
		if (dwRoomNum != 0)
		{
			PrivateRoomInfo roomInfo;
			CT_BOOL bSucc = GetPRoomInfo(dwRoomNum, roomInfo);
			//只有在线信息，没有房间信息的玩家数据删除
			if (!bSucc)
			{
				RemoveUserPRoomNum(pRemoveUser->dwUserID);
				MSG_CS2M_CommonResult removeResult;
				removeResult.cbResult = 0;
				CNetModule::getSingleton().Send(pSocket, MSG_MCS_MAIN, SUB_CS2M_REMOVE_USER_OUT_CACHE_RESULT, &removeResult, sizeof(removeResult));
				return;
			}
		}

		MSG_CS2M_CommonResult removeResult;
		removeResult.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_MCS_MAIN, SUB_CS2M_REMOVE_USER_OUT_CACHE_RESULT, &removeResult, sizeof(removeResult));
	}
	break;
	case SUB_M2CS_SEND_SYS_MESSAGE:
	{
		if (wDataSize != sizeof(MSG_M2CS_SendSysMsg))
		{
			return;
		}

		MSG_M2CS_SendSysMsg* SysMsg = (MSG_M2CS_SendSysMsg*)pData;
		CSystemMsgMgr::get_instance().InsertSystemMsg(SysMsg);
		MSG_CS2M_CommonResult result;
		result.cbResult = 0;
		CNetModule::getSingleton().Send(pSocket, MSG_MCS_MAIN, SUB_CS2M_SEND_SYS_MESSAGE, &result, sizeof(result));
	}
	break;
    case SUB_M2CS_SET_RECHARGE_JUDGE:
    {
        if (wDataSize != sizeof(CT_DWORD))
        {
            return;
        }

        CT_DWORD* pJudgeFlag = (CT_DWORD*)pData;
        if (*pJudgeFlag == 1)
        {
            m_bRechargeJudge = true;
        }
        else
        {
            m_bRechargeJudge = false;
        }

        LOG(WARNING) << "set rechargeJudge: " << m_bRechargeJudge;


        break;
    }
	default:
		break;
	}
}

CT_VOID CCenterThread::OnDBServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
		case SUB_DB2CS_SYSTEM_STATUS:
		{
			if (wDataSize != sizeof(tagSystemStatus))
			{
				return;
			}
			tagSystemStatus* pSystemStatus = (tagSystemStatus*)pData;
			memcpy(&m_SystemStatus, pSystemStatus, sizeof(tagSystemStatus));
			break;
		}
		case SUB_DB2CS_GROUP_INFO:
		{
			OnSubLoadGroupInfo(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_GROUP_USER:
		{
			OnSubLoadGroupUser(pData, wDataSize);
			break;
		}
	    case SUB_DB2CS_GROUP_USER_REVENUE:
        {
            OnSubLoadGroupUserRevenue(pData, wDataSize);
            break;
        }
	    case SUB_DB2CS_GROUP_USER_DIR_INCOME:
        {
            OnSubLoadGroupUserDirIncome(pData, wDataSize);
            break;
        }
	    case SUB_DB2CS_GROUP_USER_SUB_INCOME:
        {
            OnSubLoadGroupUserSubIncome(pData, wDataSize);
            break;
        }
	    case SUB_DB2CS_GROUP_USER_TOTAL_INCOME:
        {
            OnSubLoadGroupUserTotalIncome(pData, wDataSize);
            break;
        }
	    case SUB_DB2CS_GROUP_EXCHANGE:
        {
            OnSubQueryGroupExchange(pData, wDataSize);
            break;
        }
	    case SUB_DB2CS_GROUP_CHAT_ID:
        {
            OnSubLoadGroupChatMsgId(pData, wDataSize);
            break;
        }
	    case SUB_DB2CS_GROUP_UNREAD_CHAT:
        {
            OnSubLoadGroupUnreadChatMsg(pData, wDataSize);
            break;
        }
		case SUB_DB2CS_QUERY_BE_ADD_GROUP_USER:
		{
			OnSubLoadQueryBeAddGroupUser(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_DEFAULT_PROOM_CONFIG:
		{
			OnSubLoadDefaultPRoomConfig(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_GROUP_PROOM_CONFIG:
		{
			OnSubLoadPRoomConfig(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_GROUP_INFO_FINISH:
		{
			OnSubLoadGroupInfoFinish(pData, wDataSize);
			break;
		}
        case SUB_DB2CS_CREATE_GROUP:
        {
            OnSubDBCreateGroup(pData, wDataSize);
			break;
        }
		case SUB_DB2CS_DEDUCT_GROUP_MASTER_GEM:
		{
			OnSubDBDeductGroupMaterGem(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_GROUP_ROOM_INFO:
		{
			OnSubDBGroupRoomInfo(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_GROUP_LEVEL_CFG:
		{
			OnSubLoadGroupLevelCfg(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_WXGROUP_INFO:
		{
			OnSubDBWXGroupInfo(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_WXGROUP_INFO_FINISH:
		{
			OnSubDBWxGroupFinish(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_HONGBAO_INFO:
		{
			OnSubLoadHongBaoInfo(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_INSERT_HONGBAO:
		{
			OnSubInsertHongBao(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_SCORE_RANK:
		{
			OnSubLoadScoreRank(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_YESTERDAY_SCORE_RANK:
		{
			OnSubLoadYesterdayScoreRank(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_CLEAR_ONLINE_RANK:
		{
			CRankMgr::get_instance().ClearTodayOnlineRank();
			break;
		}
		case SUB_DB2CS_TODAY_ONLINE_RANK:
		{
			OnSubLoadTodayOnlineRank(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_CLEAR_TASK_FINISH_RANK:
		{
			CRankMgr::get_instance().ClearTodayTaskRank();
			break;
		}
		case SUB_DB2CS_TASK_FINISH_RANK:
		{
			OnSubLoadTaskRankFinish(pData, wDataSize);
			break;
		}
		/*case SUB_DB2CS_SIGNIN_RANK:
        {
            OnSubLoadSigninRank(pData, wDataSize);
        }
        break;
        case SUB_DB2CS_SHARE_RANK:
        {
           OnSubLoadShareRank(pData, wDataSize);
        }
        break;
        case SUB_DB2CS_CLEAR_SHARE_RANK:
        {
            CRankMgr::get_instance().ClearShareRank();
        }
		break;*/
		case SUB_DB2CS_TASK_INFO:
		{
			OnSubLoadTaskInfo(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_PCU_INFO:
		{
			OnSubLoadPCUInfo(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_RECHARGE_CHANNEL_INFO:
		{
			OnSubLoadRechargeChannelInfo(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_RECHARGE_PROMOTER_INFO:
		{
			OnSubLoadRechargePromoterInfo(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_EXCHANGE_CHANNEL_STATUS:
		{
			OnSubLoadExchangeChannelStatus(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_GENERALIZE_PROMOTER_INFO:
		{
			OnSubLoadGeneralizePromoterInfo(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_CHANNEL_DOMAIN:
		{
			OnSubLoadClientChannelDomain(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_QUANMIN_CHANNEL:
		{
			OnSubLoadQuanMinChannelInfo(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_VIP2_CONFIG:
		{
			OnSubLoadVip2Config(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_CHANNEL_SHOW_EXCHANGE:
		{
			OnSubLoadChannelShowExchange(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_RECHANGE_AMOUNT:
		{
			OnSubLoadRechargeChannelAmount(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_EXCHANGE_AMOUNT:
		{
			OnSubLoadExchangeChannelAmount(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_BENEFIT_CONFIG:
		{
			OnSubLoadBenefitConfig(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_PLATFORM_RECHARGE_CFG:
		{
			OnSubLoadPlatformRechargeCfg(pData, wDataSize);
		}
		break;
    	case SUB_DB2CS_CHAT_UPLOAD_URL:
    	{
    	    OnSubLoadChatUploadUrl(pData, wDataSize);
    	}
    	break;
		case SUB_DB2CS_ANNOUNCEMENT_INFO:
		{
			OnSubLoadAnnouncementInfo(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_WEALTH_GOD_CONFIG:
		{
			OnSubLoadWealthGodConfig(pData, wDataSize);
		}
		break;
		case SUB_DB2CS_WEALTH_GOD_REWARD:
		{
			OnSubLoadWealthGodReward(pData, wDataSize);
		}
		break;
    	case SUB_DB2CS_WEALTH_GOD_INDEX:
    	{
    	    OnSubLoadWealthGodIndex(pData, wDataSize);
    	}
    	break;
		case SUB_DB2CS_WEALTH_GOD_HISTORY_DATA:
		{
    	    OnSubLoadWealthGodHistoryData(pData, wDataSize);
		}
		break;
    	case SUB_DB2CS_WEALTH_GOD_HISTORY_USER:
    	{
    	    OnSubLoadWealthGodHistoryUser(pData, wDataSize);
    	}
    	break;
	    case SUB_DB2CS_DUOBAO_CONFIG:
        {
			OnSubLoadDuoBaoConfig(pData, wDataSize);
            break;
        }
		case SUB_DB2CS_DUOBAO_DOING_DATA:
		{
			OnSubLoadDuoBaoDoingData(pData, wDataSize);
            break;
		}
		case SUB_DB2CS_DUOBAO_DOING_USER:
		{
			OnSubLoadDuoBaoDoingUser(pData, wDataSize);
            break;
		}
		case SUB_DB2CS_DUOBAO_HISTORY_DATA:
		{
			OnSubLoadDuoBaoHistoryData(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_DUOBAO_HISTORY_USER:
		{
			OnSubLoadDuoBaoHistoryUser(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_DUOBAO_FINISH:
		{
			CYYDuoBaoMgr::get_instance().SetLoadDataFinish(true);
			break;
		}
		case SUB_DB2CS_REDPACKET_STATUS:
		{
			OnSubLoadRedPacketStatus(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_REDPACKET_ROOM_CFG:
		{
			OnSubLoadRedPacketRoomCfg(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_REDPACKET_INDEX:
		{
			OnSubLoadRedPacketIndex(pData, wDataSize);
			break;
		}
		case SUB_DB2CS_REDPACKET_GRAB_INFO:
		{
			OnSubLoadRedPacketGrabInfo(pData, wDataSize);
			break;
		}
		default:
			break;
	}
}

CT_VOID CCenterThread::OnGroupMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	    case SUB_CS_CREATE_GROUP:
	    {
            OnGroupSubCreateGroup(pData, wDataSize);
            break;
        }
        case SUB_CS_MODIFY_GROUP_NAME:
        {
            OnGroupSubModifyGroupName(pData, wDataSize);
            break;
        }
        case SUB_CS_MODIFY_GROUP_NOTICE:
        {
            OnGroupSubModifyGroupNotice(pData, wDataSize);
            break;
        }
        case SUB_CS_MODIFY_GROUP_ICON:
        {
            OnGroupSubModifyGroupIcon(pData, wDataSize);
            break;
        }
        case SUB_CS_UPGRADE_GROUP:
        {
            OnGroupSubUpgradeGroup(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_DIR_USER_LIST:
        {
            OnGroupSubQueryDirGroupUserList(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_DIR_USER_INFO:
        {
            OnGroupQueryGroupDirInfo(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_SUB_USER_LIST:
        {
            OnGroupQueryGroupSubInfoList(pData, wDataSize);
            break;
        }
        case SUB_CS_MODIFY_GROUP_USER_REMARKS:
        {
            OnGroupSubModifyGroupUserRemarks(pData, wDataSize);
            break;
        }
	    case SUB_CS_UPGRADE_GROUP_USER_INCOMERATE:
        {
            OnGroupSubUpgradeGroupUserIncomeRate(pData, wDataSize);
            break;
        }
        case SUB_CS_SEND_GROUP_CHAT_MSG:
        {
            OnGroupSubChat(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_INCOME:
        {
            OnGroupSubQueryGroupIncome(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_YESTERDAY_INCOME:
        {
            OnGroupSubQueryGroupYesterDayIncome(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_SETTLE_RECORD:
        {
            OnGroupSubQueryGroupSettleRecord(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_SETTLE_INFO:
        {
            OnGroupSubQueryGroupSettleInfo(pData, wDataSize);
            break;
        }
	    case SUB_CS_SETTLE_INCOME:
        {
            OnGroupSubSettleIncome(pData, wDataSize);
            break;
        }
	    case SUB_CS_CREATE_GROUP_PROOM:
        {
            OnGroupSubCreatPRoom(pData, wDataSize);
            break;
        }
	    case SUB_CS_OPEN_PROOM_LIST_UI:
        {
            OnGroupSubOpenGameUI(pData, wDataSize);
            break;
        }
	    case SUB_CS_CLOSE_PROOM_LIST_UI:
        {
            OnGroupSubCloseGameUI(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_INCOME_DETAIL:
        {
            OnGroupSubQueryGroupIncomeDetail(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_ONE_GROUP_DIR_USER:
        {
            CGroupMgr::get_instance().QueryOneDirGroupUser(pData, wDataSize);
            break;
        }
	    case SUB_CS_GIFT_SCORE_TO_DIR_USER:
        {
            CGroupMgr::get_instance().GiftScoreToDirGroupUser(pData, wDataSize);
            break;
        }
	    case SUB_CS_READ_GROUP_MSG:
        {
            CGroupMgr::get_instance().UserReadMsg(pData, wDataSize);
            break;
        }
            /*case SUB_CS_APPLY_ADD_GROUP:
            {
                OnGroupSubApplyAddGroup(pData, wDataSize);
            }
            break;
            case SUB_CS_OPERATE_APPLY_ADD_GROUP:
            {
                OnGroupSubOpApplyAddGroup(pData, wDataSize);
            }
            break;
            case SUB_CS_QUIT_GROUP:
            {
                OnGroupSubQuitGroup(pData, wDataSize);
            }
            break;
            case SUB_CS_TICK_GROUP_USER:
            {
                OnGroupSubTickUser(pData, wDataSize);
            }
            break;*/
	    /*case SUB_CS_GROUP_CREATE_PROOM:
	    {
            OnGroupSubCreatePRoom(pData, wDataSize);
            break;
        }*/
	    /*case SUB_CS_DIMISS_GROUP:
	    {
            OnGroupSubDimissGroup(pData, wDataSize);
	    }
	    break;*/
	    /*case SUB_CS_MODIFY_GROUP_INFO:
	    {
	        OnGroupSubModifyGroupInfo(pData, wDataSize);
	    }
	    break;*/
/*	    case SUB_CS_QUERY_GROUP_USER_INFO:
	    {
	    	OnGroupSubQueryUserInfo(pData, wDataSize, pSocket);
            break;
        }
        case SUB_CS_CHANGE_GROUP_POSITION:
        {
            OnGroupSubChangePosition(pData, wDataSize);
            break;
        }
	    case SUB_CS_QUERY_GROUP_INFO:
	    {
	    	OnGroupSubQueryGroupInfo(pData, wDataSize, pSocket);
            break;
        }
	    case SUB_CS_PULL_USER_INTO_GROUP:
	    {
            OnGroupSubPullUserIntoGroup(pData, wDataSize);
            break;
        }
	    case SUB_CS_RECOMMEND_ADD_GROUP:
	    {
            OnGroupSubRecommendAddGroup(pData, wDataSize);
            break;
        }*/

	    default:break;
	}
}

//红包消息
CT_VOID CCenterThread::OnHongBaoMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CS_QUERY_HONGBAO_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_HongBao))
		{
			return;
		}

		MSG_CS_Query_HongBao* pQueryHongBao = (MSG_CS_Query_HongBao*)pData;
		CHongBaoMgr::get_instance().SendAllHongBaoInfoToClient(pQueryHongBao->dwUserID);
	}
	break;
	case SUB_CS_SEND_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS_Send_HongBao))
		{
			return;
		}

		MSG_CS_Send_HongBao* pSendHongBao = (MSG_CS_Send_HongBao*)pData;
		CHongBaoMgr::get_instance().SendHongBao(pSendHongBao);

	}
	break;
	case SUB_CS_GUESS_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS_Guess_HongBao))
		{
			return;
		}

		MSG_CS_Guess_HongBao* pTakeHongBao = (MSG_CS_Guess_HongBao*)pData;
		CHongBaoMgr::get_instance().GuessHongBao(pTakeHongBao);
	}
	break;
	case SUB_CS_QUERY_MYSELF_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_Myself_HongBao))
		{
			return;
		}
		MSG_CS_Query_Myself_HongBao* pQueryMyselfHongBao = (MSG_CS_Query_Myself_HongBao*)pData;
		CHongBaoMgr::get_instance().QueryMyselfHongBao(pQueryMyselfHongBao);
	}
	break;
	case SUB_CS_OPERATE_GUESSOK_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS_Operate_GuessOk_HongBao))
		{
			return;
		}
		MSG_CS_Operate_GuessOk_HongBao* pOperateGuessOkHongBao = (MSG_CS_Operate_GuessOk_HongBao*)pData;
		CHongBaoMgr::get_instance().OperateGuessOkHongBao(pOperateGuessOkHongBao);
	}
	break;
	case SUB_CS_TAKE_TIMEOUT_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS_Take_TimeOut_HongBao))
		{
			return;
		}
		MSG_CS_Take_TimeOut_HongBao* pTakeTimeOutHongBao = (MSG_CS_Take_TimeOut_HongBao*)pData;
		CHongBaoMgr::get_instance().TakeTimeOutHongBao(pTakeTimeOutHongBao);
	}
	break;
	default:
		break;
	}
}

//任务消息
CT_VOID CCenterThread::OnTaskMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_CS_GET_TASK_REWARD:
	{
		if (wDataSize != sizeof(MSG_CS_Get_TaskReward))
		{
			return;
		}

		MSG_CS_Get_TaskReward* pTaskReward = (MSG_CS_Get_TaskReward*)pData;
		CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pTaskReward->dwUserID);
		if (pUserTask)
		{
			pUserTask->GetTaskReward(pTaskReward->wTaskID);
		}
	}
	break;
	default:
		break;
	}
}

CT_VOID CCenterThread::OnRankMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
		case SUB_CS_TODAY_EARN_SCORE_RANK:
		{
			if (wDataSize != sizeof(MSG_CS_Query_Score_Rank))
			{
				return;
			}

			MSG_CS_Query_Score_Rank* pQueryRank = (MSG_CS_Query_Score_Rank*)pData;
			CRankMgr::get_instance().SendYesterScoreRank(pQueryRank->dwUserID);
		}
		break;
		case SUB_CS_TODAY_ONLINE_RANK:
		{
			if (wDataSize != sizeof(MSG_CS_Query_Score_Rank))
			{
				return;
			}

			MSG_CS_Query_Score_Rank* pQueryRank = (MSG_CS_Query_Score_Rank*)pData;
			CRankMgr::get_instance().SendTodayOnlineRank(pQueryRank->dwUserID);
		}
		break;
		case SUB_CS_TODAY_TASK_RANK:
		{
			if (wDataSize != sizeof(MSG_CS_Query_Score_Rank))
			{
				return;
			}

			MSG_CS_Query_Score_Rank* pQueryRank = (MSG_CS_Query_Score_Rank*)pData;
			CRankMgr::get_instance().SendTodayTaskRank(pQueryRank->dwUserID);
		}
		break;
	default:
		break;
	}
}

CT_VOID CCenterThread::OnConfigOptionMsg(CT_WORD wSubCmdID, const CT_VOID *pData, CT_DWORD wDataSize, acl::aio_socket_stream *pSocket)
{
	switch (wSubCmdID)
	{
        case SUB_C2S_REMOVE_RECHARGE_INFO:
        {
            if (wDataSize != sizeof(MSG_CS_Remove_RechargeInfo))
            {
                return;
            }

            MSG_CS_Remove_RechargeInfo* pRemoveTips = (MSG_CS_Remove_RechargeInfo*)pData;
            CGlobalUser* pUser = CUserMgr::get_instance().GetGlobalUser(pRemoveTips->dwUserID);
            if (pUser)
            {
                pUser->ResetRechargeTips();
            }
            break;
        }
	    case SUB_C2S_QUERY_CHATUPLOAD_URL:
        {
            if (wDataSize != sizeof(MSG_CS_ChatUpload_Url))
			{
				return;
			}

            MSG_CS_ChatUpload_Url* pChatUploadUrl = (MSG_CS_ChatUpload_Url*)pData;
            CConfigOption::get_instance().SendChatUploadUrl(pChatUploadUrl->dwUserID);
            break;
        }
		case SUB_C2S_QUERY_ANNOUNCEMENT_CONTENT:
		{
			if (wDataSize != sizeof(MSG_CS_AnnouncementContent))
			{
				return;
			}

			MSG_CS_AnnouncementContent* pAnnouncementContent = (MSG_CS_AnnouncementContent*)pData;
			CConfigOption::get_instance().SendAnnouncementContent(pAnnouncementContent->dwUserID, pAnnouncementContent->cbAnnouncementID);
			break;
		}
	default:
		break;
	}
}

CT_VOID CCenterThread::OnWealthGodComingMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
    switch(wSubCmdID)
    {
        case SUB_CS_WEALTHGODCOMING_MAIN_INFO:
        {
            CActivityMgr::get_instance().QueryMainInfo(pData, wDataSize);
        }
        break;
        case SUB_CS_WEALTHGODCOMING_REWARD_RULE:
        {
        	CActivityMgr::get_instance().QueryRewardRule(pData, wDataSize);
        }
        break;
        case SUB_CS_WEALTHGODCOMING_DETAIL:
        {
        	CActivityMgr::get_instance().QueryDetail(pData, wDataSize);
        }
        break;
        case SUB_CS_WEALTHGODCOMING_REWARD_LIST:
        {
        	CActivityMgr::get_instance().QueryRewardList(pData, wDataSize);
        }
        break;
        default:break;
    }
}

CT_VOID CCenterThread::OnSubDip2csQueryWaitListUser(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_D2CS_Query_WaitList))
	{
		return;
	}
	MSG_D2CS_Query_WaitList* pWaitList = (MSG_D2CS_Query_WaitList*)pData;
	if (pWaitList == NULL)
	{
		return;
	}

	MSG_CS2D_Query_WaitList stWaitList;
	stWaitList.uWebSock = pWaitList->uWebSock;
	const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
	if ((NULL == pMapGameServer) || pMapGameServer->empty())
	{
		_snprintf_info(stWaitList.szJsonUserList, sizeof(stWaitList.szJsonUserList), "{\"code\":1, \"message\":\"\"}");
		CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_WAITLLIST_USER, &stWaitList, sizeof(stWaitList));
		LOG(WARNING) << "center server query wait list user failed.";
		return;
	}
	acl::string strJson("");
	MapGameServer::const_iterator citer = pMapGameServer->begin();
	for (; citer != pMapGameServer->end(); ++citer)
	{
		if (citer->second.wGameID == GAME_DDZ)
		{
			CT_DWORD dwWaitTotal = GetWaitListUserByServerId(citer->second.dwServerID, citer->second.wGameID, citer->second.wRoomKindID);
			strJson.format_append("{\"serverid\":%u, \"servername\" : \"%s\", \"lack\" : %u, \"usercount\":%u,\"online\":%u},", citer->second.dwServerID, \
			citer->second.szServerName, dwWaitTotal, citer->second.wMaxUserCount, citer->second.dwOnlineCount);
		}
		else if (citer->second.wGameID == GAME_ZJH)
		{
			strJson.format_append("{\"serverid\":%u, \"servername\" : \"%s\", \"lack\" : %u, \"usercount\":%u, \"online\":%u},", citer->second.dwServerID, \
			citer->second.szServerName, 0, citer->second.wMaxUserCount, citer->second.dwOnlineCount);
		}
	}
	strJson = strJson.left(strJson.length() - 1);       // 去掉最后的逗号
	_snprintf_info(stWaitList.szJsonUserList, sizeof(stWaitList.szJsonUserList), "{\"code\":0, \"message\":\"\", \"data\": [%s]}", strJson.c_str());
	CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_WAITLLIST_USER, &stWaitList, sizeof(stWaitList));
	LOG(INFO) << "center server query wait list user-> cmd main:" << MSG_DIPCS_MAIN << " sub:" << SUB_CS2DIP_QUERY_WAITLLIST_USER;
}

CT_DWORD CCenterThread::GetWaitListUserByServerId(CT_DWORD dwServerID, CT_WORD wGameID, CT_WORD wRoomKindID)
{
	acl::string key;
	acl::string attr, value;
	key.format("waitlist_%u", dwServerID);
	attr.format("%u:%u", wGameID, wRoomKindID);

	m_redis.clear();
	if (m_redis.hget(key.c_str(), attr.c_str(), value) == false)
	{
		return 0;
	}

	CT_DWORD dwWaitTotal = atoi(value.c_str());

	return dwWaitTotal;
}

CT_VOID CCenterThread::OnSubDip2csQueryTableMsg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	if (wDataSize != sizeof(MSG_D2CS_Query_TableInfo))
	{
		return;
	}
	MSG_D2CS_Query_TableInfo* pTableInfo = (MSG_D2CS_Query_TableInfo*)pData;
	if (pTableInfo == NULL)
	{
		return;
	}

	const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
	if ((NULL == pMapGameServer) || pMapGameServer->empty())
	{
		MSG_CS2D_Query_TableInfo stTableInfo;
		stTableInfo.uWebSock = pTableInfo->uWebSock;
		_snprintf_info(stTableInfo.szJsonUserList, sizeof(stTableInfo.szJsonUserList), "{\"code\":1, \"message\":\"\"}");
		CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_SERVERID_TABLE_MSG, &stTableInfo, sizeof(stTableInfo));
		LOG(WARNING) << "center server query game server table information, can not find game server socket.";
		return;
	}

	acl::aio_socket_stream* pGameServerSocket = NULL;
	MapGameServer::const_iterator citer = pMapGameServer->begin();
	for (; citer != pMapGameServer->end(); ++citer)
	{
		if (citer->second.dwServerID == pTableInfo->nServerID)
		{
			pGameServerSocket = citer->first;
		}
	}

	if (NULL == pGameServerSocket)
	{
		MSG_CS2D_Query_TableInfo stTableInfo;
		stTableInfo.uWebSock = pTableInfo->uWebSock;
		_snprintf_info(stTableInfo.szJsonUserList, sizeof(stTableInfo.szJsonUserList), "{\"code\":1, \"message\":\"ServerID末找到\"}");
		CNetModule::getSingleton().Send(pSocket, MSG_DIPCS_MAIN, SUB_CS2DIP_QUERY_SERVERID_TABLE_MSG, &stTableInfo, sizeof(stTableInfo));
		LOG(WARNING) << "center server query game server table information, can not find game server id.";
		return;
	}

	MSG_C2GS_Query_Table_Info stTableInfo;
	stTableInfo.cbPlatformId = pTableInfo->cbPlatformId;
	stTableInfo.uWebSock = pTableInfo->uWebSock;
	CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_QUERY_TABLE_INFO, &stTableInfo, sizeof(stTableInfo));
	LOG(INFO) << "center server query game server table information, cmd main:" << MSG_GCS_MAIN << " sub:" << SUB_C2GS_QUERY_TABLE_INFO;
}

//重新更新机器人
CT_VOID CCenterThread::OnSubUpdateAndroid(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	MSG_D2CS_UpdateAndroid* pUpdateAndroid = (MSG_D2CS_UpdateAndroid*)pData;
	
	acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(pUpdateAndroid->dwServerID);
	if (pGameServerSocket == NULL)
	{
		LOG(WARNING) << "update android config, can not find game server, server, server id: " << pUpdateAndroid->dwServerID;
		return;
	}
	
	CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_UPDATE_ANDROID);
}

//更新机器人同时上桌人数
CT_VOID CCenterThread::OnSubUpdateAndroidpPlayCount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	MSG_D2CS_UpdateAndroid_PlayGameCount* pUpdateAndroidCount = (MSG_D2CS_UpdateAndroid_PlayGameCount*)pData;

	acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(pUpdateAndroidCount->dwServerID);
	if (pGameServerSocket == NULL)
	{
		return;
	}
	MSG_C2GS_UpdateAndroid_PlayCount updateAndroidCount;
	updateAndroidCount.dwAndroidCount = pUpdateAndroidCount->dwAndroidCount;
	CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_UPDATE_ANDROID_COUNT, &updateAndroidCount, sizeof(updateAndroidCount));
}

CT_VOID CCenterThread::OnSubUpdateServerStockInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	MSG_D2CS_Update_StockInfo* pUpdateServerStock = (MSG_D2CS_Update_StockInfo*)pData;

	acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(pUpdateServerStock->dwServerID);
	if (pGameServerSocket == NULL)
	{
		LOG(WARNING) << "OnSubUpdateServerStockInfo can not find game server info. server id: " << pUpdateServerStock->dwServerID;
		return;
	}

	MSG_C2GS_Update_GameServer_Stock stockInfo;
	stockInfo.llTotalStockLowerLimit = pUpdateServerStock->llTotalStockLowerLimit;
	stockInfo.llTodayStockHighLimit = pUpdateServerStock->llTodayStockHighLimit;
	stockInfo.wChangeCardRatio = pUpdateServerStock->wChangeCardRatio;
	stockInfo.wSystemAllKillRatio = pUpdateServerStock->wSystemAllKillRatio;

	CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_UPDATE_GS_STOCK, &stockInfo, sizeof(MSG_C2GS_Update_GameServer_Stock));
}

CT_VOID CCenterThread::OnSubDipTickFishUser(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	MSG_D2CS_Tick_FishUser* pTickFishUser = (MSG_D2CS_Tick_FishUser*)pData;
	
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pTickFishUser->dwUserID);
	if (pUserInfo)
	{
		acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(pUserInfo->dwGameServerID);
		if (pGameServerSocket == NULL)
		{
			return;
		}
		MSG_C2GS_Tick_FishUser tickFishUser;
		tickFishUser.dwUserID = pTickFishUser->dwUserID;
		CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_TICK_FISH_USER, &tickFishUser, sizeof(tickFishUser));
	}
}

CT_VOID CCenterThread::OnSubBRGameRationControl(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	MSG_D2CS_Reload_BrGameRatio* pBrGameRatio = (MSG_D2CS_Reload_BrGameRatio*)pData;
	
	acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(pBrGameRatio->dwServerID);
	if (pGameServerSocket == NULL)
	{
		return;
	}

	CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_BRGAME_RATIO);
}

CT_VOID CCenterThread::OnSubBRGameBlackList(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	MSG_D2CS_Reload_BrGameBlackList* pBrGameBlackList = (MSG_D2CS_Reload_BrGameBlackList*)pData;
	
	acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(pBrGameBlackList->dwServerID);
	if (pGameServerSocket == NULL)
	{
		return;
	}
	
	CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_BRGAME_BLACKLIST);
}

CT_VOID CCenterThread::OnSubDipUpdateWealthGodComing(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagWealthGodComingCfg))
		return;
	
	
	tagWealthGodComingCfg* pCfg = (tagWealthGodComingCfg*)pData;
	
	CActivityMgr::get_instance().UpdateWealthGodComing(pCfg);
}

CT_VOID CCenterThread::OnSubDipUpdateWealthGodComingRank(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagWealthGodComingReward))
		return;
	
	
	tagWealthGodComingReward* pReward = (tagWealthGodComingReward*)pData;
	
	CActivityMgr::get_instance().InsertWealthGodComingReward(pReward);
}

//更新夺宝配置
CT_VOID CCenterThread::OnSubDipUpdateDuoBaoCfg(const CT_VOID * pData, CT_DWORD wDataSize)
{
    //LOG(WARNING) << "recv OnSubDipUpdateDuoBaoCfg 1";
    if (wDataSize != sizeof(tagDuoBaoCfg))
        return;


    //LOG(WARNING) << "recv OnSubDipUpdateDuoBaoCfg 2";
    tagDuoBaoCfg* pCfg = (tagDuoBaoCfg*)pData;

    CYYDuoBaoMgr::get_instance().UpdateDuoBaoCfg(pCfg);
}

//更新救济金配置
CT_VOID CCenterThread::OnSubDipUpdateBenefitCfg(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagBenefitReward))
    {
        return;
    }

    //更新中心数据
    tagBenefitReward* pBenefitReward = (tagBenefitReward*)pData;
    CConfigOption::get_instance().SetBenefitReward(pBenefitReward);

    //更新登录服数据
    CServerMgr::get_instance().SendMsgToAllLoginServer(MSG_LCS_MAIN, SUB_CS2L_UPDATE_BENEFIT_CONFIG, (CT_VOID*)pData, wDataSize);

    //更新游戏服数据
    CServerMgr::get_instance().SendMsgToAllGameServer(MSG_GCS_MAIN, SUB_C2GS_BENEFIT_CFG, (CT_VOID*)pData, wDataSize);
}

//更新平台充值配置信息
CT_VOID CCenterThread::OnSubDipUpdatePlatformRechargeCfg(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagPlatformRechargeCfg))
	{
		return;
	}

	tagPlatformRechargeCfg* pPlatformRechargeCfg = (tagPlatformRechargeCfg*)pData;

	CConfigOption::get_instance().InsertPlatformRechargeCfg(pPlatformRechargeCfg);
}

//更新短信平台
CT_VOID CCenterThread::OnSubDipUpdateSMSPid(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CT_DWORD))
	{
		return;
	}

	CServerMgr::get_instance().SendMsgToAllLoginServer(MSG_LCS_MAIN, SUB_CS2L_UPDATE_SMS_PID, pData, wDataSize);
}

//更新新年红包的状态
CT_VOID CCenterThread::OnSubDipUpdateRedPacketStatus(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagRedPacketStatus))
        return;

    tagRedPacketStatus* pPacketStatus = (tagRedPacketStatus*)pData;

    CRedPacketMgr::get_instance().SetRedPacketStatus(pPacketStatus);
	CRedPacketMgr::get_instance().SendRedPacketStatusToClient(0);
}

//更新红包的房间配置
CT_VOID CCenterThread::OnSubDipUpdateRedPacketRoomCfg(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagRedPacketRoomCfg))
        return;

    tagRedPacketRoomCfg* pPacketRoomCfg = (tagRedPacketRoomCfg*)pData;

    CRedPacketMgr::get_instance().InsertRedPacketRoomCfg(pPacketRoomCfg);
}

//更新红包的期数配置
CT_VOID CCenterThread::OnSubDipUpdateRedPacketIndex(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagRedPacketIndex))
        return;

    tagRedPacketIndex* pRedPacketIndex = (tagRedPacketIndex*)pData;

    CRedPacketMgr::get_instance().InsertRedPacketIndex(pRedPacketIndex);
}

//更新聊天上传图片
CT_VOID CCenterThread::OnSubDipUpdateChatUploadUrl(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagChatUpLoadUrl))
	{
		return;
	}
	tagChatUpLoadUrl* pChatUploadUrl = (tagChatUpLoadUrl*)pData;

	CConfigOption::get_instance().SetChatUploadUrl(pChatUploadUrl);
}

//更新公告类型
CT_VOID CCenterThread::OnSubDipUpdateAnnouncement(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagAnnouncementInfo))
    {
        return;
    }
    tagAnnouncementInfo* pAnnouncementInfo = (tagAnnouncementInfo*)pData;

    CConfigOption::get_instance().InsertAnnouncementInfo(pAnnouncementInfo);
}

CT_VOID CCenterThread::OnSubDipUpdateBlackChannel(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize != sizeof(tagBlackChannelEx))
    {
        return;
    }

    CServerMgr::get_instance().SendMsgToAllGameServer(MSG_GCS_MAIN, SUB_C2GS_BLACK_CHANNEL, (CT_VOID *) pData, wDataSize);
}


CT_VOID CCenterThread::OnSubDipUpdateRechargePromoterList(const CT_VOID * pData, CT_DWORD wDataSize)
{
	const char* pPromoterList = (const char*)pData;
	std::string strPromoterList(pPromoterList);
	CConfigOption::get_instance().InsertRechargePromoterList(strPromoterList);

	//渠道ID
	/*
	acl::json promoterJson(pPromoterList);
	acl::json_node* pChannelID = promoterJson.getFirstElementByTagName("code"); //code里面保存的是channelID
	if (pChannelID == NULL)
	{
		return;
	}
	acl::string strChannelID = pChannelID->get_string();
	CT_WORD wChannelID = atoi(strChannelID.c_str());
	CConfigOption::get_instance().SendRechargePromoterListToClient(0, wChannelID);
	 */
}

CT_VOID CCenterThread::OnDipSubUpdateExchangeChannelStatus(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_D2CS_ExchangeStatus))
	{
		return;
	}

	MSG_D2CS_ExchangeStatus* pExchangeChannelStatus = (MSG_D2CS_ExchangeStatus*)pData;
	CConfigOption::get_instance().SetExchangeChannelStatus(pExchangeChannelStatus->cbExchangeType, pExchangeChannelStatus->cbStatus);
}

CT_VOID CCenterThread::OnDipSubUpdateUserStatus(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_D2CS_UserStatus))
	{
		return;
	}
	
	MSG_D2CS_UserStatus* pUserStatus = (MSG_D2CS_UserStatus*)pData;
	if (pUserStatus && pUserStatus->cbStatus == 2)
	{
		acl::aio_socket_stream* pUserProxySock = CUserMgr::get_instance().FindUserProxySocksPtr(pUserStatus->dwUserID);
		if (pUserProxySock)
		{
			MSG_GS2P_CloseConnect closeConnect;
			closeConnect.dwUserID = pUserStatus->dwUserID;
			CNetModule::getSingleton().Send(pUserProxySock, MSG_PGS_MAIN, SUB_GS2P_CLOSE_USER_CONNECT, &closeConnect, sizeof(closeConnect));
		}
	}
}

CT_VOID CCenterThread::OnDipSubUpdateGeneralizePromoter(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagGeneralizePromoterInfo))
	{
		return;
	}

	tagGeneralizePromoterInfo* pGeneralizePromoter = (tagGeneralizePromoterInfo*)pData;
	CConfigOption::get_instance().SetGeneralizePromoter(pGeneralizePromoter);
	CConfigOption::get_instance().SendGeneralizePromoterToClient(0, pGeneralizePromoter->cbPlatformId);
}

CT_VOID CCenterThread::OnDipSubUpdateClientChannelDomain(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_D2CS_ClientChannel_Domain))
	{
		return;
	}

	MSG_D2CS_ClientChannel_Domain* pDomain = (MSG_D2CS_ClientChannel_Domain*)pData;

	CT_BOOL bModifyDomain = false;
	//CT_BOOL bModifyLock = false;
	tagClientChannelDomain* pChannelInfo = CConfigOption::get_instance().GetClientChannelInfo(pDomain->dwChannelID);
	if (pChannelInfo)
	{
		/*if (pChannelInfo->cbLock != pDomain->cbLocked)
		{
			bModifyLock = true;
		}*/

		if (strcmp(pChannelInfo->szDomain, pDomain->szDomain) != 0 || strcmp(pChannelInfo->szDomain2, pDomain->szDomain2) != 0)
		{
			bModifyDomain = true;
		}
	}

	tagClientChannelDomain domain;
	domain.dwChannelID = pDomain->dwChannelID;
	domain.cbLock = pDomain->cbLocked;
	_snprintf_info(domain.szDomain, sizeof(domain.szDomain), "%s", pDomain->szDomain);
	_snprintf_info(domain.szDomain2, sizeof(domain.szDomain2), "%s", pDomain->szDomain2);
	CConfigOption::get_instance().SetClientChannelDomain(&domain);

	if (bModifyDomain)
	{
		CConfigOption::get_instance().SendClientChannelDomain(0, pDomain->dwChannelID);
	}
	/*if (bModifyLock)
	{
		CConfigOption::get_instance().SendRechargePromoterListToClient(0, pDomain->dwChannelID);
	}*/

	CT_DWORD  dwOldShowExchangeCond = CConfigOption::get_instance().GetChannelShowExchangeCond(pDomain->dwChannelID);
	if (pDomain->dwShowExchangeCond != dwOldShowExchangeCond)
	{
		//通知登录服服务器
		tagChannelShowExchangeCond showExchangCond;
		showExchangCond.dwChannelID = pDomain->dwChannelID;
		showExchangCond.dwTotalRecharge = pDomain->dwShowExchangeCond;
		CConfigOption::get_instance().SetChannelShowExchange(&showExchangCond);
		CServerMgr::get_instance().SendMsgToAllLoginServer(MSG_LCS_MAIN, SUB_CS2L_UPDATE_SHOWEXCHANGE, &showExchangCond, sizeof(showExchangCond));
	}
}

CT_VOID CCenterThread::OnDipSubUpdateQuanMinChannel(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagQuanMinChannel))
	{
		return;
	}

	tagQuanMinChannel* pQuanMinChannelInfo = (tagQuanMinChannel*)pData;
	if (pQuanMinChannelInfo)
	{
		CConfigOption::get_instance().SetQuanMinChannelInfo(pQuanMinChannelInfo);
	}
}

CT_VOID CCenterThread::OnDipSubUpdateChannelPresentScore(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagChannelPresentScore))
	{
		return;
	}

	CServerMgr::get_instance().SendMsgToAllLoginServer(MSG_LCS_MAIN, SUB_CS2L_UPDATE_CHANNLE_PRESENT_SCORE, (CT_VOID*)pData, wDataSize);
}

CT_VOID CCenterThread::OnDipSubUpdateRechargeAmount(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize !=sizeof(tagRechargeChannelAmount))
		return;
	
	tagRechargeChannelAmount* pAmount = (tagRechargeChannelAmount*)pData;
	
	CConfigOption::get_instance().InsertRechargeChannelAmount(pAmount);
	CConfigOption::get_instance().MatchRechargeChannelAmount();

	//同步到客服端
	CConfigOption::get_instance().SendRechargeChannelInfoToClient(0, 0);
}

CT_VOID CCenterThread::OnDipSubUpdateExchangeAmount(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize !=sizeof(tagExchangeChannelAmount))
		return;
	
	tagExchangeChannelAmount* pAmount = (tagExchangeChannelAmount*)pData;
	
	CConfigOption::get_instance().InsertExchangeChannelAmount(pAmount);
    
    CServerMgr::get_instance().SendMsgToAllLoginServer(MSG_LCS_MAIN, SUB_CS2L_UPDATE_EXCHANGE_INFO, (CT_VOID*)pData, wDataSize);

	CConfigOption::get_instance().SendExchangeChannelStatusToClient(0);
}

CT_VOID CCenterThread::OnDipSubSetFishDianKong(const CT_VOID * pData, CT_DWORD wDataSize)
{
	MSG_D2CS_Set_FishDiankong* pFishDianKong = (MSG_D2CS_Set_FishDiankong*)pData;
	/*
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pFishDianKong->dwUserID);
	if (pUserInfo)
	{
		acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(pUserInfo->dwGameServerID);
		if (pGameServerSocket == NULL)
		{
			return;
		}

		CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_FISH_DIANKONG, pData, wDataSize);
	}*/
    CT_WORD wGameID = pFishDianKong->dwGameIndex / 10000;
    CT_WORD wKindID = (pFishDianKong->dwGameIndex % 10000) / 100;
    CT_WORD wRoomKindID = (pFishDianKong->dwGameIndex % 10000) % 100;
    acl::aio_socket_stream* pGameServerSocket = CServerMgr::get_instance().FindGameServer(wGameID, wKindID, wRoomKindID);
    if (pGameServerSocket)
    {
        CNetModule::getSingleton().Send(pGameServerSocket, MSG_GCS_MAIN, SUB_C2GS_FISH_DIANKONG, pData, wDataSize);
    }
}

CT_VOID CCenterThread::OnSubDipUpdateZjhCardRatio(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(MSG_D2CS_Reload_ZjhDepot_Ratio))
	{
		return;
	}

	//把数据发送给炸金花游戏服务器
	const MapGameServer* pMapGameServer = CServerMgr::get_instance().GetGameServerMap();
	if (pMapGameServer)
	{
		auto it = pMapGameServer->begin();
		for (; it != pMapGameServer->end(); ++it)
		{
			if (it->second.wGameID == GAME_ZJH)
			{
				CNetModule::getSingleton().Send(it->first, MSG_GCS_MAIN, SUB_C2GS_ZJH_CARDRATIO, pData, wDataSize);
			}
		}
	}
}

CT_VOID CCenterThread::OnSubLoadHongBaoInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize % sizeof(tagHongBaoInfo) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagHongBaoInfo);
	tagHongBaoInfo* pHongBaoInfo = (tagHongBaoInfo*)pData;
	if (pHongBaoInfo == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CHongBaoMgr::get_instance().InsertHongBaoInfo(pHongBaoInfo++, false);
	}
}


CT_VOID CCenterThread::OnSubInsertHongBao(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagHongBaoInfo))
	{
		return;
	}

	tagHongBaoInfo* pHongBaoInfo = (tagHongBaoInfo*)pData;
	tagGlobalUserInfo* pUserInfo =CUserMgr::get_instance().GetUserInfo(pHongBaoInfo->dwSenderID);
	if (pUserInfo)
	{
		_snprintf_info(pHongBaoInfo->szSenderHeadUrl, sizeof(pHongBaoInfo->szSenderHeadUrl), "%s", pUserInfo->szHeadUrl);
		_snprintf_info(pHongBaoInfo->szSenderNickName, sizeof(pHongBaoInfo->szSenderNickName), "%s", pUserInfo->szNickName);

		//发送给玩家
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pHongBaoInfo->dwSenderID);
		if (pSocket == NULL)
		{
			return;
		}

		MSG_SC_Send_HongBao sendResult;
		sendResult.dwMainID = MSG_HONGBAO_MAIN;
		sendResult.dwSubID = SUB_SC_SEND_HONGBAO;
		sendResult.dwValue2 = pHongBaoInfo->dwSenderID;
		sendResult.cbResult = 0;

		sendResult.hongbaoInfo.dwHongBaoID = pHongBaoInfo->dwID;
		sendResult.hongbaoInfo.dwSenderUserID = pHongBaoInfo->dwSenderID;

		_snprintf_info(sendResult.hongbaoInfo.szHongBaoName, sizeof(sendResult.hongbaoInfo.szHongBaoName), "%s", pHongBaoInfo->szHongBaoName);
		_snprintf_info(sendResult.hongbaoInfo.szSenderNickName, sizeof(sendResult.hongbaoInfo.szSenderNickName), "%s", pHongBaoInfo->szSenderNickName);
		_snprintf_info(sendResult.hongbaoInfo.szHeadUrl, sizeof(sendResult.hongbaoInfo.szHeadUrl), "%s", pHongBaoInfo->szSenderHeadUrl);

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &sendResult, sizeof(sendResult));

	}
	else
	{

		GetUserHeadUrl(pHongBaoInfo->dwSenderID, pHongBaoInfo->szSenderHeadUrl);
		GetUserNickName(pHongBaoInfo->dwSenderID, pHongBaoInfo->szSenderNickName);
	}

	CHongBaoMgr::get_instance().InsertHongBaoInfo(pHongBaoInfo, true);
}

CT_VOID CCenterThread::OnSubLoadScoreRank(const CT_VOID * pData, CT_DWORD wDataSize)
{
	/*
    if (wDataSize == 0)
    {
        CRankMgr::get_instance().ClearScoreRank();
        return;
    }
    if (wDataSize % sizeof(tagScoreRank) != 0)
    {
        return;
    }

    CT_DWORD dwCount = wDataSize / sizeof(tagScoreRank);
    tagScoreRank* pScoreRank = (tagScoreRank*)pData;
    if (pScoreRank == NULL)
    {
        return;
    }

    CRankMgr::get_instance().ClearScoreRank();
    for (CT_DWORD i = 0; i < dwCount; ++i)
    {
        CRankMgr::get_instance().InsertScoreRank(pScoreRank++);
    }*/
}

CT_VOID CCenterThread::OnSubLoadYesterdayScoreRank(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize == 0)
	{
		CRankMgr::get_instance().ClearYesterdayScoreRank();
		return;
	}
	if (wDataSize % sizeof(tagScoreRank) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagScoreRank);
	tagScoreRank* pScoreRank = (tagScoreRank*)pData;
	if (pScoreRank == NULL)
	{
		return;
	}

	CRankMgr::get_instance().ClearYesterdayScoreRank();
	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CRankMgr::get_instance().InsertYesterdayScoreRank(pScoreRank++);
	}
}

CT_VOID CCenterThread::OnSubLoadTodayOnlineRank(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize == 0)
	{
		return;
	}
	if (wDataSize % sizeof(tagTodayOnlineRank) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagTodayOnlineRank);
	tagTodayOnlineRank* pTodayOnlineRank = (tagTodayOnlineRank*)pData;
	if (pTodayOnlineRank == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CRankMgr::get_instance().InsertTodayOnlineRank(pTodayOnlineRank++);
	}
}

CT_VOID CCenterThread::OnSubLoadTaskRankFinish(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize == 0)
	{
		return;
	}
	if (wDataSize % sizeof(tagTodayTaskFinishRank) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagTodayTaskFinishRank);
	tagTodayTaskFinishRank* pTodayTaskRank = (tagTodayTaskFinishRank*)pData;
	if (pTodayTaskRank == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CRankMgr::get_instance().InsertTodayTaskRank(pTodayTaskRank++);
	}
}

CT_VOID CCenterThread::OnSubLoadSigninRank(const CT_VOID * pData, CT_DWORD wDataSize)
{
	/*
    if (wDataSize == 0)
    {
        CRankMgr::get_instance().ClearSignInRank();
        return;
    }
    if (wDataSize % sizeof(tagSignInRank) != 0)
    {
        return;
    }

    CT_DWORD dwCount = wDataSize / sizeof(tagSignInRank);
    tagSignInRank* pSignInRank = (tagSignInRank*)pData;
    if (pSignInRank == NULL)
    {
        return;
    }

    CRankMgr::get_instance().ClearSignInRank();
    for (CT_DWORD i = 0; i < dwCount; ++i)
    {
        CRankMgr::get_instance().InsertSignInRank(pSignInRank++);
    }
     */
}

CT_VOID CCenterThread::OnSubLoadShareRank(const CT_VOID * pData, CT_DWORD)
{
	/*
    if(wDataSize == 0)
    {
        CRankMgr::get_instance().ClearShareRank();
        return;
    }
    if (wDataSize % sizeof(tagShareRank) != 0)
    {
        return;
    }

    CT_DWORD dwCount = wDataSize / sizeof(tagShareRank);
    tagShareRank* pScoreRank = (tagShareRank*)pData;
    if (pScoreRank == NULL)
    {
        return;
    }

    CRankMgr::get_instance().ClearShareRank();
    for (CT_DWORD i = 0; i < dwCount; ++i)
    {
        CRankMgr::get_instance().InsertShareRank(pScoreRank++);
    }
     */
}

CT_VOID CCenterThread::OnSubLoadTaskInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize % sizeof(tagTaskModel) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagTaskModel);
	tagTaskModel* pTaskModel = (tagTaskModel*)pData;
	if (pTaskModel == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CUserTaskModel::get_instance().InsertTaskModel(pTaskModel++);
	}
}

CT_VOID CCenterThread::OnSubLoadPCUInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagPcuInfo))
	{
		return;
	}

	tagPcuInfo* pPcuInfo = (tagPcuInfo*)pData;

	//memcpy(&m_pcuInfo, pPcuInfo, sizeof(m_pcuInfo));
	m_mapPcuInfo.insert(std::make_pair(pPcuInfo->cbPlatformId, *pPcuInfo));
	SetPCUInfoToCache(*pPcuInfo);
}

CT_VOID CCenterThread::OnSubLoadRechargeChannelInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize % sizeof(tagRechargeChannelInfo) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagRechargeChannelInfo);
	tagRechargeChannelInfo* pRechargeInfo = (tagRechargeChannelInfo*)pData;
	if (pRechargeInfo == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().InsertRechargeChannel(pRechargeInfo++);
	}
}

CT_VOID CCenterThread::OnSubLoadRechargePromoterInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
	const char* pPromoterList = (const char*)pData;

	std::string strPromoterList(pPromoterList);
	CConfigOption::get_instance().InsertRechargePromoterList(strPromoterList);
}

CT_VOID CCenterThread::OnSubLoadExchangeChannelStatus(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagExchangeChannelStatus))
	{
		return;
	}

	tagExchangeChannelStatus* pChannelStatus = (tagExchangeChannelStatus*)pData;

	for (int i = 0; i < (int)(sizeof(pChannelStatus->cbChannelID) / sizeof(CT_BYTE)); ++i)
	{
		CConfigOption::get_instance().SetExchangeChannelStatus(pChannelStatus->cbChannelID[i], pChannelStatus->cbStatus[i]);
	}
}

CT_VOID CCenterThread::OnSubLoadGeneralizePromoterInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagGeneralizePromoterInfo))
	{
		return;
	}

	tagGeneralizePromoterInfo* pGeneralizePromoterInfo = (tagGeneralizePromoterInfo*)pData;

	CConfigOption::get_instance().SetGeneralizePromoter(pGeneralizePromoterInfo);
	
}

CT_VOID CCenterThread::OnSubLoadClientChannelDomain(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagClientChannelDomain))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagClientChannelDomain);
	tagClientChannelDomain* pDomain = (tagClientChannelDomain*)pData;
	if (pDomain == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().SetClientChannelDomain(pDomain++);
	}
}

CT_VOID CCenterThread::OnSubLoadQuanMinChannelInfo(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagQuanMinChannel))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize / sizeof(tagQuanMinChannel);
	tagQuanMinChannel* pQuanMin = (tagQuanMinChannel*)pData;
	if (pQuanMin == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().SetQuanMinChannelInfo(pQuanMin++);
	}
}

CT_VOID CCenterThread::OnSubLoadVip2Config(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagVip2Config))
	{
		return;
	}
	CT_DWORD dwCount = wDataSize / sizeof(tagVip2Config);
	tagVip2Config* pVip2Config = (tagVip2Config*)pData;
	if (pVip2Config == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().SetVip2Config(pVip2Config++);
	}
}

CT_VOID CCenterThread::OnSubLoadChannelShowExchange(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagChannelShowExchangeCond))
	{
		return;
	}
	CT_DWORD dwCount = wDataSize / sizeof(tagChannelShowExchangeCond);
	tagChannelShowExchangeCond* pChannelShowExchange = (tagChannelShowExchangeCond*)pData;
	if (pChannelShowExchange == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().SetChannelShowExchange(pChannelShowExchange++);
	}
}

CT_VOID CCenterThread::OnSubLoadRechargeChannelAmount(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagRechargeChannelAmount))
	{
		return;
	}
	
	CT_DWORD dwCount = wDataSize / sizeof(tagRechargeChannelAmount);
	tagRechargeChannelAmount* pRechargeAmount = (tagRechargeChannelAmount*)pData;
	if (pRechargeAmount == NULL)
	{
		return;
	}
	
	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().InsertRechargeChannelAmount(pRechargeAmount++);
	}
	
	CConfigOption::get_instance().MatchRechargeChannelAmount();
}

CT_VOID CCenterThread::OnSubLoadExchangeChannelAmount(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagExchangeChannelAmount))
	{
		return;
	}
	
	CT_DWORD dwCount = wDataSize / sizeof(tagExchangeChannelAmount);
	tagExchangeChannelAmount* pExchangeAmount = (tagExchangeChannelAmount*)pData;
	if (pExchangeAmount == NULL)
	{
		return;
	}
	
	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().InsertExchangeChannelAmount(pExchangeAmount++);
	}
}

CT_VOID CCenterThread::OnSubLoadBenefitConfig(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagBenefitReward))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize/sizeof(tagBenefitReward);
	tagBenefitReward* pBenefitReward = (tagBenefitReward*)pData;
	if (pBenefitReward == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().SetBenefitReward(pBenefitReward++);
	}
}

CT_VOID	CCenterThread::OnSubLoadPlatformRechargeCfg(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagPlatformRechargeCfg))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize/sizeof(tagPlatformRechargeCfg);
	tagPlatformRechargeCfg* pPlatformRechargeCfg = (tagPlatformRechargeCfg*)pData;
	if (pPlatformRechargeCfg == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().InsertPlatformRechargeCfg(pPlatformRechargeCfg++);
	}
}

CT_VOID CCenterThread::OnSubLoadWealthGodConfig(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagWealthGodComingCfg))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize/sizeof(tagWealthGodComingCfg);
	tagWealthGodComingCfg* pWealthGodComing = (tagWealthGodComingCfg*)pData;
	if (pWealthGodComing == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i) {
		CActivityMgr::get_instance().InsertWealthGodComing(pWealthGodComing++);
	}
}

CT_VOID CCenterThread::OnSubLoadWealthGodReward(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagWealthGodComingReward))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize/sizeof(tagWealthGodComingReward);
	tagWealthGodComingReward* pWealthGodComingReward = (tagWealthGodComingReward*)pData;
	if (pWealthGodComingReward == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i) {
		CActivityMgr::get_instance().InsertWealthGodComingReward(pWealthGodComingReward++);
	}
}

CT_VOID CCenterThread::OnSubLoadWealthGodIndex(const CT_VOID * pData, CT_DWORD wDataSize)
{
    if (wDataSize < sizeof(tagWealthGodComingLastIndex))
    {
        return;
    }
    
    CT_DWORD dwCount = wDataSize/sizeof(tagWealthGodComingLastIndex);
    tagWealthGodComingLastIndex*  pWealthGodComingIndex = (tagWealthGodComingLastIndex*)pData;
    if (pWealthGodComingIndex == NULL)
    {
        return;
    }
    
    for (CT_DWORD i = 0; i < dwCount; ++i)
    {
        CActivityMgr::get_instance().InsertWealthGodComingIndex(pWealthGodComingIndex++);
    }
}

CT_VOID CCenterThread::OnSubLoadWealthGodHistoryData(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagWealthGodComingData))
	{
		return;
	}
	
	CT_DWORD dwCount = wDataSize /sizeof(tagWealthGodComingData);
	tagWealthGodComingData* pWealthGodComingData = (tagWealthGodComingData*)pData;
	if (pWealthGodComingData == NULL)
	{
		return;
	}
	
	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
	    CActivityMgr::get_instance().InsertWealthGodComingHistoryData(pWealthGodComingData++);
	}
	LOG(WARNING) << "OnSubLoadWealthGodHistoryData: " << dwCount;
}

CT_VOID CCenterThread::OnSubLoadWealthGodHistoryUser(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagWealthGodComingUserInfoEx))
	{
		return;
	}
	
	CT_DWORD dwCount = wDataSize /sizeof(tagWealthGodComingUserInfoEx);
	tagWealthGodComingUserInfoEx* pWealthGodComingUser = (tagWealthGodComingUserInfoEx*)pData;
	if (pWealthGodComingUser == NULL)
	{
		return;
	}
	
	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
        CActivityMgr::get_instance().InsertWealthGodComingHistoryUser(pWealthGodComingUser++);
	}
}

CT_VOID CCenterThread::OnSubLoadDuoBaoConfig(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagDuoBaoCfg))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize/sizeof(tagDuoBaoCfg);
	tagDuoBaoCfg* pDuoBaoCfg = (tagDuoBaoCfg*)pData;
	if (pDuoBaoCfg == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CYYDuoBaoMgr::get_instance().InsertDuoBaoCfg(pDuoBaoCfg++);
	}
}

CT_VOID CCenterThread::OnSubLoadDuoBaoDoingData(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagDuoBaoData))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagDuoBaoData);
	tagDuoBaoData* pDuoBaoData = (tagDuoBaoData*)pData;
	if (pDuoBaoData == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CYYDuoBaoMgr::get_instance().InsertDoingDuoBaoData(pDuoBaoData++);
	}
}

CT_VOID CCenterThread::OnSubLoadDuoBaoDoingUser(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagDuoBaoUserInfo))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagDuoBaoUserInfo);
	tagDuoBaoUserInfo* pDuobaoUserInfo = (tagDuoBaoUserInfo*)pData;
	if (pDuobaoUserInfo == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CYYDuoBaoMgr::get_instance().InsertDoingDuoBaoUser(pDuobaoUserInfo++);
	}
}

CT_VOID CCenterThread::OnSubLoadDuoBaoHistoryData(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagDuoBaoData))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagDuoBaoData);
	tagDuoBaoData* pDuoBaoData = (tagDuoBaoData*)pData;
	if (pDuoBaoData == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CYYDuoBaoMgr::get_instance().InsertHistoryDuoBaoData(pDuoBaoData++);
	}
}

CT_VOID CCenterThread::OnSubLoadDuoBaoHistoryUser(const CT_VOID * pData, CT_DWORD wDataSize)
{
	if (wDataSize < sizeof(tagDuoBaoUserInfo))
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagDuoBaoUserInfo);
	tagDuoBaoUserInfo* pDuobaoUserInfo = (tagDuoBaoUserInfo*)pData;
	if (pDuobaoUserInfo == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CYYDuoBaoMgr::get_instance().InsertHistoryDuoBaoUser(pDuobaoUserInfo++);
	}
}

CT_VOID CCenterThread::OnSubLoadRedPacketStatus(const CT_VOID *pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagRedPacketStatus))
	{
		return;
	}

	tagRedPacketStatus* pPacketStatus = (tagRedPacketStatus*)pData;
	CRedPacketMgr::get_instance().SetRedPacketStatus(pPacketStatus);
}

CT_VOID CCenterThread::OnSubLoadRedPacketRoomCfg(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize % sizeof(tagRedPacketRoomCfg) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagRedPacketRoomCfg);
	tagRedPacketRoomCfg* pRedPacketRoomCfg = (tagRedPacketRoomCfg*)pData;
	if (pRedPacketRoomCfg == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CRedPacketMgr::get_instance().InsertRedPacketRoomCfg(pRedPacketRoomCfg++);
	}
}

CT_VOID CCenterThread::OnSubLoadRedPacketIndex(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize % sizeof(tagRedPacketIndex) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagRedPacketIndex);
	tagRedPacketIndex* pRedPacketIndex = (tagRedPacketIndex*)pData;
	if (pRedPacketIndex == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CRedPacketMgr::get_instance().InsertRedPacketIndex(pRedPacketIndex++);
	}
}

CT_VOID CCenterThread::OnSubLoadRedPacketGrabInfo(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize % sizeof(tagRedPacketUserGrabData) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagRedPacketUserGrabData);
	tagRedPacketUserGrabData* pRedPacketGrab = (tagRedPacketUserGrabData*)pData;
	if (pRedPacketGrab == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CRedPacketMgr::get_instance().InsertRedPacketGrabInfo(pRedPacketGrab++);
	}

	//发送昨天的红包
	//CRedPacketMgr::get_instance().SendRedPacketAward();
	CNetModule::getSingleton().SetTimer(IDI_REDPACKET_GRAB, TIME_REDPACKET_GRAB, this, false);
}

CT_VOID CCenterThread::OnSubLoadChatUploadUrl(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(tagChatUpLoadUrl))
	{
		return;
	}
	tagChatUpLoadUrl* pChatUploadUrl = (tagChatUpLoadUrl*)pData;

	CConfigOption::get_instance().SetChatUploadUrl(pChatUploadUrl);
}

CT_VOID CCenterThread::OnSubLoadAnnouncementInfo(const CT_VOID* pData, CT_DWORD wDataSize)
{
	if (wDataSize % sizeof(tagAnnouncementInfo) != 0)
	{
		return;
	}

	CT_DWORD dwCount = wDataSize /sizeof(tagAnnouncementInfo);
	tagAnnouncementInfo* pAnnouncementInfo = (tagAnnouncementInfo*)pData;
	if (pAnnouncementInfo == NULL)
	{
		return;
	}

	for (CT_DWORD i = 0; i < dwCount; ++i)
	{
		CConfigOption::get_instance().InsertAnnouncementInfo(pAnnouncementInfo++);
	}
}

CT_BOOL CCenterThread::CheckRoomNumIsExist(CT_DWORD dwRoomNum)
{
	acl::string key;
	key.format("proom_info_%u", dwRoomNum);
	m_redis.clear();
	if (m_redis.exists(key.c_str()) == true)
	{
		return true;
	}

	return false;
}


CT_BOOL	CCenterThread::GetPRoomInfo(CT_DWORD dwRoomNum, PrivateRoomInfo& roomInfo)
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
	roomInfo.dwOwerUserID = (atoi)(result["owner"].c_str());
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
}


CT_DWORD CCenterThread::CreateRoomNum(CT_DWORD dwServerID)
{
	CT_DWORD dwRoomNum = 0;
	if (dwServerID < 10)
	{
		dwRoomNum = dwServerID * 100000 + rand() % 100000;
	}
	if (dwServerID >= 10 && dwServerID <= 99)
	{
		dwRoomNum = dwServerID * 10000 + rand() % 10000;
	}
	else if (dwServerID >= 100 && dwServerID <= 999)
	{
		dwRoomNum = dwServerID * 1000 + rand() % 1000;
	}
	else
	{
		dwRoomNum = 100000 + rand() % 100000;
	}

	return dwRoomNum;
}

CT_BOOL CCenterThread::SetRoomNumToCache(CT_DWORD dwUserID, CT_DWORD dwServerID, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wUserCount, \
	CT_WORD wPlayCount, /*CT_WORD wWinLimit, CT_WORD wEndLimit,*/ CT_BYTE bIsVip, CT_CHAR* pOtherParam,
	CT_DWORD dwRoomNum, CT_DWORD dwGroupID/* = 0*/, CT_DWORD dwRecordID/* = 0*/, CT_BYTE cbGroupType /*= 0*/)
{	
	if (dwUserID != 0)
	{
		acl::string key;
		key.format("online_%u", dwUserID);

		acl::string att_gameid, att_kindid, att_roomkind, att_roomnum;
		acl::string val_gameid, val_kindid, val_roomkind, val_roomnum;
		std::map<acl::string, acl::string> onlineInfo;
		att_gameid.format("%s", "gameid");
		att_kindid.format("%s", "kindid");
		att_roomkind.format("%s", "roomkind");
		att_roomnum.format("%s", "roomnum");

		val_gameid.format("%u", wGameID);
		val_kindid.format("%u", wKindID);
		val_roomkind.format("%u", 0);
		val_roomnum.format("%u", dwRoomNum);

		onlineInfo[att_gameid] = val_gameid;
		onlineInfo[att_kindid] = val_kindid;
		onlineInfo[att_roomkind] = val_roomkind;
		onlineInfo[att_roomnum] = val_roomnum;

		m_redis.clear();
		if (m_redis.hmset(key, onlineInfo) == false)
		{
			return false;
		}
	}

	m_redis.clear();
	acl::string key1;
	acl::string attr_owner, attr_server, attr_gameid, attr_kindid, att_usercount, att_playcount, att_currUserCount, \
		/*att_winupperlimit, att_endupperlimit,*/ att_isvip, /*att_auto,*/ att_groupid, att_recordid, att_groupType, att_OtherParam, att_createTime;
	acl::string val_owner, val_server, val_gameid, val_kindid, val_usercount, val_playcount, val_currUserCount, \
		/*val_winupperlimit, val_endupperlimit,*/ val_isvip, /*val_auto,*/ val_groupid, val_recordid, val_groupType, val_OtherParam,val_createTime;
	std::map<acl::string, acl::string> roomInfo;

	key1.format("proom_info_%u", dwRoomNum);
	attr_owner.format("owner");
	attr_server.format("serverid");
	attr_gameid.format("gameid");
	attr_kindid.format("kindid");
	att_usercount.format("usercount");
	att_playcount.format("playcount");
	att_currUserCount.format("curruser");
	//att_winupperlimit.format("winlimit");
	//att_endupperlimit.format("endlimit");
	att_isvip.format("vip");
	//att_auto.format("auto");
	att_groupid.format("gid");
	att_recordid.format("rid");
	att_groupType.format("gtype");
	att_OtherParam.format("otherparam");
	att_createTime.format("time");

	val_owner.format("%u", dwUserID);
	val_server.format("%u", dwServerID);
	val_gameid.format("%u", wGameID);
	val_kindid.format("%u", wKindID);
	val_usercount.format("%u", wUserCount);
	val_playcount.format("%u", wPlayCount);
	val_currUserCount.format("%d", 0);
	//val_winupperlimit.format("%u", wWinLimit);
	//val_endupperlimit.format("%u", wEndLimit);
	val_isvip.format("%u", bIsVip);
	//val_auto.format("%d", bIsAuto);
	val_groupid.format("%u", dwGroupID);
	val_recordid.format("%u", dwRecordID);
	val_groupType.format("%d", cbGroupType);
	val_OtherParam.format("%s", pOtherParam);
	val_createTime.format("%s", Utility::GetTimeNowString("%Y-%m-%d %H:%M:%S").c_str());

	roomInfo[attr_owner] = val_owner;
	roomInfo[attr_server] = val_server;
	roomInfo[attr_gameid] = val_gameid;
	roomInfo[attr_kindid] = val_kindid;
	roomInfo[att_usercount] = val_usercount;
	roomInfo[att_playcount] = val_playcount;
	roomInfo[att_currUserCount] = val_currUserCount;
	//roomInfo[att_winupperlimit] = val_winupperlimit;
	//roomInfo[att_endupperlimit] = val_endupperlimit;
	roomInfo[att_isvip] = val_isvip;
	//roomInfo[att_auto] = val_auto;
	roomInfo[att_groupid] = val_groupid;
	roomInfo[att_recordid] = val_recordid;
	roomInfo[att_OtherParam] = val_OtherParam;
	roomInfo[att_createTime] = val_createTime;
	roomInfo[att_groupType] = val_groupType;

	if (m_redis.hmset(key1, roomInfo) == false)
	{
		if (dwUserID != 0)
		{
			acl::string key;
			key.format("online_%u", dwUserID);

			m_redis.clear();
			m_redis.del_one(key);
		}
		return false;
	}

	stPRoomInfo proomInfo;
	proomInfo.dwServerID = dwServerID;
	proomInfo.dwGroupID = dwGroupID;
	proomInfo.dwRecordID = dwRecordID;
	proomInfo.cbGroupType = cbGroupType;
	proomInfo.dwMasterID = (dwGroupID == 0 ? 0 : CGroupMgr::get_instance().GetGroupMasterID(dwGroupID));
	proomInfo.dwCreateTime = Utility::GetTime();
	proomInfo.wPlayCount = wPlayCount;
	proomInfo.wGameID = wGameID;
	proomInfo.wKindID = wKindID;
	proomInfo.bStart = false;
	m_mapPRoomInfo.insert(std::make_pair(dwRoomNum, proomInfo));

	return true;
}

CT_BOOL CCenterThread::CheckUserHasRoom(CT_DWORD dwUserID)
{
	acl::string key;
	//acl::string attr, value;
	key.format("online_%u", dwUserID);
	//attr.format("roomnum");

	m_redis.clear();
	if (m_redis.exists(key.c_str()) == true)
	{
		return true;
	}
	/*if (m_redis.hget(key.c_str(), attr.c_str(), value) == false)
	{
		return false;
	}

	if (attr.empty() || value.empty())
	{
		return false;
	}*/

	return false;
}


CT_DWORD CCenterThread::GetUserGem(CT_DWORD dwUserID)
{
	acl::string key;
	acl::string attr, value;
	key.format("account_%u", dwUserID);
	attr.format("gem");

	m_redis.clear();
	if (m_redis.hget(key.c_str(), attr.c_str(), value) == false)
	{
		return 0;
	}
	
	CT_DWORD dwGem = atoi(value.c_str());

	return dwGem;
}

CT_BOOL CCenterThread::AddUserGemToCache(CT_DWORD dwUserID, int iAddGem, long long& dwGemResult, CT_BYTE cbType/* = 0*/)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == false)
	{
		if (cbType == 0)
		{
			LOG(WARNING) << "user excharge fail, can not find account info in the redis! userid: " << dwUserID << ", add gem: " << iAddGem;
		}
		return false;
	}

	m_redis.clear();
	if (m_redis.hincrby(key, "gem", iAddGem, &dwGemResult) == false)
	{
		LOG(WARNING) << "user excharge update redis fail, user id: " << dwUserID << ", add gem: " << iAddGem;
		return false;
	}

	return true;
}

CT_VOID	CCenterThread::UpdateUserGem(CT_DWORD dwUserID, int iAddGem, long long& llGemResult, CT_BYTE cbType/* = 0*/)
{
	CT_BOOL bSuc = AddUserGemToCache(dwUserID, iAddGem, llGemResult, cbType);
	if (bSuc)
	{
		CT_DWORD dwGameServerID = GetUserGameServerID(dwUserID);
		if (dwGameServerID != 0)
		{
			acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(dwGameServerID);
			if (pSocket)
			{
				MSG_C2GS_User_Recharge recharge;
				recharge.dwUserID = dwUserID;
				recharge.llRecharge = iAddGem;
				recharge.cbType = RECHARGE_GEM;
				CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_USER_RECHARGE, &recharge, sizeof(recharge));
			}
		}

		CT_DWORD dwProxyServerID = 0;
		CT_UINT64 uClientAddr = 0;
		GetUserProxyServerID(dwUserID, dwProxyServerID, uClientAddr);
		if (dwProxyServerID != 0)
		{
			acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindProxyServer(dwProxyServerID);
			if (pSocket)
			{
				MSG_SC_GameUserGem updateGem;
				updateGem.dwMainID = MSG_FRAME_MAIN;
				updateGem.dwSubID = SUB_S2C_USER_GEM;
				updateGem.dwUserID = dwUserID;
				updateGem.dwGem = (CT_DWORD)llGemResult;
				updateGem.uValue1 = uClientAddr;
				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, &updateGem, sizeof(updateGem));
			}
		}
	}
}

//警告: 充值专用接口
CT_BOOL	 CCenterThread::AddUserScoreToCache(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_DWORD dwTotalCash, CT_LONGLONG& llScoreResult, CT_LONGLONG& llRechargeTempCount)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == false)
	{
		//LOG(WARNING) << "user excharge score fail, can not find account info in the redis! userid: " << dwUserID << ", add score: " << llAddScore;
		return false;
	}

	//判断是否应该给玩家充值
	if (llAddScore > 0)
	{
		m_redis.clear();
		acl::string strTotalPay;
		if (m_redis.hget(key, "pay", strTotalPay) == false)
		{
			LOG(WARNING) << "AddUserScoreToCache get user total pay fail, user id: " << dwUserID << ", add score: " << llAddScore;
			return false;
		}

		CT_DWORD dwPreTotalPay = atoi(strTotalPay.c_str());
		if (dwPreTotalPay >= dwTotalCash)
		{
			LOG(WARNING) << "AddUserScoreToCache user total pay more than current total pay. user id: " << dwUserID << ", add score: " << llAddScore
			<< ", pre total pay: " << dwPreTotalPay << ", new total pay: " << dwTotalCash;
			if (m_bRechargeJudge)
				return false;
		}
	}

	m_redis.clear();
	if (m_redis.hincrby(key, "score", llAddScore, &llScoreResult) == false)
	{
		LOG(WARNING) << "user excharge score update redis fail, user id: " << dwUserID << ", add score: " << llAddScore;
		return false;
	}

	//增加临时充值次数(不是总的充值次数, 一般地指当前在线时充值次数)
    m_redis.clear();
    if (m_redis.hincrby(key, "rechargecount", 1, &llRechargeTempCount) == false)
    {
        LOG(WARNING) << "update redis rechargecount fail, user id: " << dwUserID << ", add score: " << llAddScore;
    }

	/*if (m_redis.hincrby(key, "recharge", llAddScore, &llScoreResult) == false)
	{
		LOG(WARNING) << "user excharge score update redis fail, user id: " << dwUserID << ", add score: " << llAddScore;
		return false;
	}*/

	return true;
}

CT_BOOL CCenterThread::UpdateUserScore(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_DWORD dwTotalCash)
{
    CT_LONGLONG llNewScore = 0;
    CT_LONGLONG llRechargeCount = 0;
    CT_BOOL bSuc = AddUserScoreToCache(dwUserID, llAddScore, dwTotalCash, llNewScore, llRechargeCount);

	if (bSuc)
	{
        tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
        CT_BYTE cbVip2 = 0;
        CT_LONGLONG llOldScore = 0;
        if (pUserInfo)
        {
            //检测内存数据是否够扣
            /*CT_LONGLONG llResultScore = pUserInfo->llScore + llAddScore;
            if (llResultScore < 0)
            {
                llAddScore -= llResultScore;
            }*/

            //更新内存
            llOldScore = pUserInfo->llScore;
            pUserInfo->llScore += llAddScore;
			cbVip2 = pUserInfo->cbVip2;
			pUserInfo->dwTotalRecharge = dwTotalCash;//玩家总充数据
        }

		if (pUserInfo != NULL)
		{
			CT_DWORD dwGameServerID = pUserInfo->dwGameServerID;//GetUserGameServerID(dwUserID);
			if (dwGameServerID != 0)
			{
				acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(dwGameServerID);
				if (pSocket)
				{
					MSG_C2GS_User_Recharge recharge;
					recharge.dwUserID = dwUserID;
					recharge.llRecharge = llAddScore;
					recharge.dwRechargeTempCount = (CT_DWORD)llRechargeCount;
					recharge.cbType = RECHARGE_SCORE;
					CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_USER_RECHARGE, &recharge, sizeof(recharge));
				}
			}

			CT_DWORD dwProxyServerID = 0;
			CT_UINT64 uClientAddr = 0;
			GetUserProxyServerID(dwUserID, dwProxyServerID, uClientAddr);
			if (dwProxyServerID != 0)
			{
				acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindProxyServer(dwProxyServerID);
				if (pSocket)
				{
					MSG_SC_UserScoreInfo updateScore;
					updateScore.dwMainID = MSG_FRAME_MAIN;
					updateScore.dwSubID = SUB_S2C_USER_SCORE;
					updateScore.dwUserID = dwUserID;
					updateScore.wTableID = INVALID_CHAIR;
					updateScore.wChairID = INVALID_CHAIR;
					updateScore.dScore = llNewScore*0.01f;
					updateScore.uValue1 = uClientAddr;
					CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, &updateScore, sizeof(updateScore));

					//下发不能领取救济金按钮（有点简单粗暴）
					tagBenefitReward* pBenefit = CConfigOption::get_instance().GetBenefitReward(cbVip2);
					if (pBenefit && llOldScore < pBenefit->dwLessScore && llNewScore >= pBenefit->dwLessScore)
					{
						MSG_SC_HasBenefit hasBenefit;
						hasBenefit.dwMainID = MSG_FRAME_MAIN;
						hasBenefit.dwSubID = SUB_S2C_HAS_BENEFIT;
						hasBenefit.dwValue2 = dwUserID;
						hasBenefit.cbState = 0;
						CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &hasBenefit, sizeof(hasBenefit));
					}
				}
			}
		}
	}

    return bSuc;
}

CT_BOOL  CCenterThread::AddUserBankScoreToCache(CT_DWORD dwUserID, long long llAddBankScore, long long& llBankResult)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == false)
	{
		LOG(WARNING) << "update user redis bank score fail,  can not find account info. user id: " << dwUserID << ", add bank score: " << llAddBankScore;
		return false;
	}

	m_redis.clear();
	if (m_redis.hincrby(key, "bankscore", llAddBankScore, &llBankResult) == false)
	{
		LOG(WARNING) << "update user redis bank score fail, user id: " << dwUserID << ", add score: " << llAddBankScore;
		return false;
	}

	return true; 
}

CT_VOID  CCenterThread::UpdateUserBankScore(CT_DWORD dwUserID, long long llAddBankScore, long long& llBankResult)
{
	CT_BOOL bSuc = AddUserBankScoreToCache(dwUserID, llAddBankScore, llBankResult);
	if (bSuc)
	{
		tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
		if (pUserInfo)
		{
			CT_DWORD dwGameServerID = pUserInfo->dwGameServerID;//GetUserGameServerID(dwUserID);
			if (dwGameServerID != 0)
			{
				acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(dwGameServerID);
				if (pSocket)
				{
					MSG_C2GS_User_Recharge recharge;
					recharge.dwUserID = dwUserID;
					recharge.llRecharge = llAddBankScore;
					recharge.cbType = RECHARGE_BANK;
					CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_USER_RECHARGE, &recharge, sizeof(recharge));
				}
			}
		}
	}
}

CT_VOID CCenterThread::SetGroupPRoomRecordID(CT_DWORD dwRoomNum, CT_DWORD dwRecordID)
{
	//缓存插入
	acl::string key, value;
	key.format("proom_info_%u", dwRoomNum);
	value.format("%u", dwRecordID);

	m_redis.clear();
	if (m_redis.hset(key, "rid", value.c_str()) == -1)
	{
		LOG(WARNING) << "set group proom record id fail, room num : " << dwRoomNum << ", record id : " << dwRecordID;
		return;
	}

	//内存插入
	auto it = m_mapPRoomInfo.find(dwRoomNum);
	if (it != m_mapPRoomInfo.end())
	{
		it->second.dwRecordID = dwRecordID;
	}

	return;
}

CT_VOID	 CCenterThread::SetUserPromoterID(CT_DWORD dwUserID, CT_DWORD dwPromoterID)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == false)
	{
		return;
	}

	m_redis.clear();

	acl::string value;
	value.format("%u", dwPromoterID);
	if (m_redis.hset(key, "promoterid", value.c_str()) < 0)
	{
		LOG(WARNING) << "set promoter id fail, user id: " << dwUserID << ", promoter id: " << dwPromoterID;
		return;
	}

	return;
}

/*
 CT_VOID CCenterThread::UpdateServerUser(CT_BYTE cbPlatformId, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID)
{
	//CT_DWORD dwTick = Utility::GetTick();
	//if (dwTick - m_updateServerUserTick > 60*1000)
	{
		//CT_DWORD  dwKindUserCount = 0;
		//CT_DWORD  dwRoomKindUserCount = 0;
		//CServerMgr::get_instance().CalGameServerUserCount(wGameID, wKindID, wRoomKindID, dwKindUserCount, dwRoomKindUserCount);
		acl::string key;
		acl::string  attrRoomKind, valueRoomKind;
		key.format("totalonline");
		
		std::map<acl::string, acl::string> mapValue;
		//attrKind.format("game_%u", wGameID*100+wKindID);
		//valueKind.format("%u", dwKindUserCount);
		//mapValue[attrKind] = valueKind;

		CT_DWORD dwPlatformUserCount = 0;//CServerMgr::get_instance().GetGameUserCount(cbPlatformId, wGameID, wKindID, wRoomKindID);
		attrRoomKind.format("server_%d_%u", cbPlatformId, wGameID*100+wKindID*10+wRoomKindID);
		valueRoomKind.format("%u", dwPlatformUserCount);
		mapValue[attrRoomKind] = valueRoomKind;

		m_redis.clear();
		if (m_redis.hmset(key.c_str(), mapValue) == false)
		{
			LOG(WARNING) << "update game server user fail, game id: " << wGameID << ", kind id : " << wKindID << ", room kind id: " << wRoomKindID;
		}
		//m_updateServerUserTick = dwTick;
	}
}

CT_VOID CCenterThread::UpdateHallUser(CT_BYTE cbPlatformId, CT_DWORD dwProxyServerID, CT_BYTE cbMachineType, CT_BOOL bUpdatePcu)
{	
	static acl::string key;
	static acl::string hall_attr, hall_value, proxy_attr, proxy_value, ccu_attr, ccu_value, pcu_hall, pcu_ios, pcu_android;
	bool bUpdateHallPcu = false;
	bool bUpdateiOSPcu = false;
	bool bUpdateAndroidPcu = false;

	key.format("totalonline");

	//hall user
	hall_attr.format("ccu_hall_%u", cbPlatformId);
	CT_DWORD dwHallCount = 0;
	auto itPlatform = m_mapPlatformProxyUser.find(cbPlatformId);
	if (itPlatform != m_mapPlatformProxyUser.end())
    {
	    MapProxyUser& proxyUser = itPlatform->second;
        for (auto& it : proxyUser)
        {
            std::set<CT_DWORD>& setUser = it.second;
            dwHallCount += (CT_DWORD)setUser.size();
        }
    }

	hall_value.format("%d", dwHallCount);

    tagPcuInfo& pcuInfo = m_mapPcuInfo[cbPlatformId];
	if (bUpdatePcu && dwHallCount > pcuInfo.dwPCU)
	{
		pcu_hall.format("%d", dwHallCount);
        pcuInfo.dwPCU = dwHallCount;
		bUpdateHallPcu = true;
		UpdatePCUToDB(cbPlatformId, 0, pcuInfo.dwPCU);
	}

	//proxy user
	proxy_attr.format("proxy_%u", dwProxyServerID);
    CT_DWORD dwProxyUserCount = 0;
	for (auto& itProxy : m_mapPlatformProxyUser)
    {
        MapProxyUser& proxyUser = itProxy.second;
        std::set<CT_DWORD>& setUser = proxyUser[dwProxyServerID];
        dwProxyUserCount += (CT_DWORD)setUser.size();
    }
    proxy_value.format("%d", dwProxyUserCount);

	//machine user
	MapMachineUser& mapMachineUser = m_mapPlatformMachineUser[cbPlatformId];
	std::set<CT_DWORD>& setMachineUser = mapMachineUser[cbMachineType];
	CT_DWORD dwMachineUserCount = (CT_DWORD)setMachineUser.size();
	ccu_value.format("%d", dwMachineUserCount);

	//判断ios和android的最高在线和当前在线
	if (cbMachineType == MACHINE_TYPE_IOS)
	{
		ccu_attr.format("ccu_ios_%d", cbPlatformId);
		if (bUpdatePcu && dwMachineUserCount > pcuInfo.dwPCUiOS)
		{
			pcu_ios.format("%d", dwMachineUserCount);
            pcuInfo.dwPCUiOS = dwMachineUserCount;
			bUpdateiOSPcu = true;
			UpdatePCUToDB(cbPlatformId, MACHINE_TYPE_IOS, pcuInfo.dwPCUiOS);
		}
	}
	else
	{
		ccu_attr.format("ccu_android_%d", cbPlatformId);
		if (bUpdatePcu && dwMachineUserCount > pcuInfo.dwPCUAndroid)
		{
			pcu_android.format("%d", dwMachineUserCount);
            pcuInfo.dwPCUAndroid = dwMachineUserCount;
			bUpdateAndroidPcu = true;
			UpdatePCUToDB(cbPlatformId, MACHINE_TYPE_ANDROID, pcuInfo.dwPCUAndroid);
		}
	}

	std::map<acl::string, acl::string> attrs;
	attrs.insert(std::make_pair(hall_attr, hall_value));
	attrs.insert(std::make_pair(proxy_attr, proxy_value));
	attrs.insert(std::make_pair(ccu_attr, ccu_value));
	if (bUpdateHallPcu)
	{
	    static acl::string pcu_hall_attr;
	    pcu_hall_attr.format("pcu_hall_%d", cbPlatformId);
		attrs.insert(std::make_pair(pcu_hall_attr, pcu_hall));
	}
	if (bUpdateiOSPcu)
	{
        static acl::string pcu_ios_attr;
        pcu_ios_attr.format("pcu_ios_%d", cbPlatformId);
		attrs.insert(std::make_pair(pcu_ios_attr, pcu_ios));
	}
	if (bUpdateAndroidPcu)
	{
        static acl::string pcu_android_attr;
        pcu_android_attr.format("pcu_android_%d", cbPlatformId);
		attrs.insert(std::make_pair(pcu_android_attr, pcu_android));
	}

	m_redis.clear();
	if (m_redis.hmset(key.c_str(), attrs) == false)
	{
		LOG(WARNING) << "update hall and proxy user fail, platformId: " << (int)cbPlatformId << ", proxy server id: " << dwProxyServerID;
	}
}

CT_VOID CCenterThread::IncreaseHallUser(CT_BYTE cbPlatformId, CT_DWORD dwProxyServerID, CT_BYTE cbMachineType, CT_DWORD dwUserID)
{
	//online
	MapProxyUser& mapProxyUser = m_mapPlatformProxyUser[cbPlatformId];
	std::set<CT_DWORD>& setUser = mapProxyUser[dwProxyServerID];
	auto it = setUser.find(dwUserID);
	if (it == setUser.end())
	{
		setUser.insert(dwUserID);
	}

	//machine online
    MapMachineUser& mapMachineUser = m_mapPlatformMachineUser[cbPlatformId];
	std::set<CT_DWORD>& setMachineUser = mapMachineUser[cbMachineType];
	auto itMachine = setMachineUser.find(dwUserID);
	if (itMachine == setMachineUser.end())
	{
		setMachineUser.insert(dwUserID);
	}

	//change account handle
	tagGlobalUserInfo* pUserOnline = CUserMgr::get_instance().GetUserInfo(dwUserID);
	if (pUserOnline)
	{
		//如果玩家连接的不是原来的proxy,则在原来的proxy删除
		if ((pUserOnline->dwProxyServerID != 0) && pUserOnline->dwProxyServerID != dwProxyServerID)
		{
			MapProxyUser& oldMapProxyUser =  m_mapPlatformProxyUser[cbPlatformId];
			std::set<CT_DWORD>& setUser = oldMapProxyUser[pUserOnline->dwProxyServerID];
			auto itUser = setUser.find(dwUserID);
			if (itUser != setUser.end())
				setUser.erase(itUser);
		}
		//如果玩家登录的设备不是原来的设备，则在原来的设备删除
		if (pUserOnline->cbMachineType != 0 && pUserOnline->cbMachineType != cbMachineType)
		{
			MapMachineUser& mapMachineUserEx = m_mapPlatformMachineUser[cbPlatformId];
			std::set<CT_DWORD>& setMachineUserEx = mapMachineUserEx[pUserOnline->cbMachineType];
			auto itMacheineUser = setMachineUserEx.find(dwUserID);
			if (itMacheineUser != setMachineUserEx.end())
				setMachineUserEx.erase(itMacheineUser);
		}
	}
	UpdateHallUser(cbPlatformId, dwProxyServerID, cbMachineType, true);
}

CT_VOID CCenterThread::DecreaseHallUser(CT_BYTE cbPlatformId, CT_DWORD dwProxyServerID, CT_DWORD dwUserID)
{
	MapProxyUser& mapProxyUser = m_mapPlatformProxyUser[cbPlatformId];
	std::set<CT_DWORD>& setUser = mapProxyUser[dwProxyServerID];
	auto it = setUser.find(dwUserID);
	if (it != setUser.end())
	    setUser.erase(it);

	tagGlobalUserInfo* pUserOnline = CUserMgr::get_instance().GetUserInfo(dwUserID);
	if (pUserOnline)
	{
	    MapMachineUser& mapMachineUser = m_mapPlatformMachineUser[cbPlatformId];
		std::set<CT_DWORD>& setMachineUser = mapMachineUser[pUserOnline->cbMachineType];
		auto itMachine = setMachineUser.find(dwUserID);
		if (itMachine != setMachineUser.end())
		    setMachineUser.erase(itMachine);

		UpdateHallUser(cbPlatformId, dwProxyServerID, pUserOnline->cbMachineType, false);
	}
}*/

CT_VOID  CCenterThread::CalcOnlineUser()
{
	CT_DWORD dwNow = (CT_DWORD)time(NULL);
	if (dwNow - m_updateUserCountTick < 10)
	{
		return;
	}

	/*
	 * 分别会往redis写以下几个键
	 * ccu_hall_pid
	 * ccu_ios_pid
	 * ccu_android_pid
	 * pcu_hall_pid
	 * pcu_ios_pid
	 * pcu_android_pid
	 * proxy_serverid
	 * server_gameid*100+kindid*10+roomid
	 */
    //m_mapPlatformProxyUser.clear();
    //m_mapPlatformMachineUser.clear();

	for (auto& itPPUser : m_mapPlatformProxyUser)
	{
		MapProxyUser& mapProxyUser = itPPUser.second;
		for (auto& itPUser : mapProxyUser)
		{
			itPUser.second = 0;
		}
	}

	for (auto& itPMUser : m_mapPlatformMachineUser)
	{
		MapMachineUser& mapMachineUser = itPMUser.second;
		for (auto& itMUser : mapMachineUser)
		{
			itMUser.second = 0;
		}
	}

	for (auto& itPSUser : m_mapPlatformServerUser)
	{
		MapServerUser& mapServerUser = itPSUser.second;

		for (auto& itSUser : mapServerUser)
		{
			itSUser.second = 0;
		}
	}

    MapGlobalUser& mapGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
    for (auto& it : mapGlobalUser)
    {
		if (!it.second->IsOnline())
			continue;

        CT_DWORD dwProxyServerID = it.second->GetUserProxyID();
        if (dwProxyServerID == 0)
			continue;

        //CT_DWORD dwUserID = it.first;
        CT_BYTE cbPlatformId = it.second->GetPlatformId();
        CT_BYTE  cbMachineType = it.second->GetMacheineType();
        CT_DWORD dwServerID = it.second->GetUserServerID();

        //计算proxy上的玩家人数
		MapProxyUser& mapProxyUser = m_mapPlatformProxyUser[cbPlatformId];
		//std::set<CT_DWORD>& setProxyUser = mapProxyUser[dwProxyServerID];
		//setProxyUser.insert(dwUserID);
		auto itProxyUser = mapProxyUser.find(dwProxyServerID);
		if (itProxyUser != mapProxyUser.end())
		{
			++itProxyUser->second;
		}
		else
		{
			mapProxyUser.insert(std::make_pair(dwProxyServerID, 1));
		}

		//计算设备上的玩家
		MapMachineUser& mapMachineUser = m_mapPlatformMachineUser[cbPlatformId];
		//std::set<CT_DWORD>& setMachineUser = mapMachineUser[cbMachineType];
		//setMachineUser.insert(dwUserID);
		auto itMachineUser = mapMachineUser.find(cbMachineType);
		if (itMachineUser != mapMachineUser.end())
		{
			++itMachineUser->second;
		}
		else
		{
			mapMachineUser.insert(std::make_pair(cbMachineType, 1));
		}

		//计算游戏中的人数
		if (dwServerID != 0)
		{
            CMD_GameServer_Info* pGameServerInfo = CServerMgr::get_instance().FindGameServer2(dwServerID);
            if (pGameServerInfo)
            {
                std::map<CT_DWORD, CT_DWORD >& mapServerUser = m_mapPlatformServerUser[cbPlatformId];
                CT_DWORD dwGameIndex = pGameServerInfo->wGameID*100+pGameServerInfo->wKindID*10+pGameServerInfo->wRoomKindID;
                auto itServerUser = mapServerUser.find(dwGameIndex);
                if (itServerUser != mapServerUser.end())
                {
                    itServerUser->second += 1;
                }
                else
                {
                    mapServerUser.insert(std::make_pair(dwGameIndex, 1));
                }
            }
		}
    }

    //设置游戏人数到redis
    acl::string key;
    key.format("totalonline");

    std::map<acl::string, acl::string> mapAttrs;

    //平台用户
    for (auto& itPPUser : m_mapPlatformProxyUser)
    {
        MapProxyUser& mapProxyUser = itPPUser.second;
        CT_BYTE cbPlatformId = itPPUser.first;
        CT_DWORD dwPlatformHallCount = 0;
        for (auto& itProxyUser : mapProxyUser)
        {
            acl::string strProxy;
            strProxy.format("proxy_%u", itProxyUser.first);

            CT_DWORD dwUserCount = itProxyUser.second;
            dwPlatformHallCount += dwUserCount;

            acl::string strProxyUserCount;
            strProxyUserCount.format("%u", dwUserCount);

           mapAttrs.insert(std::make_pair(strProxy, strProxyUserCount));
        }

        acl::string strPlatform;
        strPlatform.format("ccu_hall_%d", itPPUser.first);
        acl::string strPlatformUserCount;
        strPlatformUserCount.format("%u", dwPlatformHallCount);

        mapAttrs.insert(std::make_pair(strPlatform, strPlatformUserCount));

        tagPcuInfo& pcuInfo = m_mapPcuInfo[cbPlatformId];
        if (dwPlatformHallCount > pcuInfo.dwPCU)
        {
            pcuInfo.dwPCU = dwPlatformHallCount;

            acl::string pcu_hall_attr;
            pcu_hall_attr.format("pcu_hall_%d", cbPlatformId);
            mapAttrs.insert(std::make_pair(pcu_hall_attr, strPlatformUserCount));

            UpdatePCUToDB(cbPlatformId, 0, pcuInfo.dwPCU);
        }
    }

    //机器用户
    for (auto& itPMUser : m_mapPlatformMachineUser)
    {
        MapMachineUser& mapMachineUser = itPMUser.second;
        CT_BYTE cbPlatformId = itPMUser.first;

        for (auto& itMachineUser : mapMachineUser)
        {
            CT_DWORD dwUserCount = itMachineUser.second;

            acl::string strccu;
            if (itMachineUser.first == MACHINE_TYPE_IOS)
            {
                strccu.format("ccu_ios_%d", cbPlatformId);

                tagPcuInfo& pcuInfo = m_mapPcuInfo[cbPlatformId];
                if (dwUserCount > pcuInfo.dwPCUiOS)
                {
                    pcuInfo.cbPlatformId = cbPlatformId;
                    pcuInfo.dwPCUiOS = dwUserCount;


                    acl::string pcu_ios_attr;
                    pcu_ios_attr.format("pcu_ios_%d", cbPlatformId);
                    acl::string pcu_ios_value;
                    pcu_ios_value.format("%u", dwUserCount);

                    mapAttrs.insert(std::make_pair(pcu_ios_attr, pcu_ios_value));

                    UpdatePCUToDB(cbPlatformId, MACHINE_TYPE_IOS, pcuInfo.dwPCUiOS);
                }

            }
            else
            {
                strccu.format("ccu_android_%d", cbPlatformId);

                tagPcuInfo& pcuInfo = m_mapPcuInfo[cbPlatformId];
                if (dwUserCount > pcuInfo.dwPCUAndroid)
                {
                    pcuInfo.cbPlatformId = cbPlatformId;
                    pcuInfo.dwPCUAndroid = dwUserCount;

                    acl::string pcu_android_attr;
                    pcu_android_attr.format("pcu_android_%d", cbPlatformId);
                    acl::string pcu_android_value;
                    pcu_android_value.format("%u", dwUserCount);
                    mapAttrs.insert(std::make_pair(pcu_android_attr, pcu_android_value));

                    UpdatePCUToDB(cbPlatformId, MACHINE_TYPE_ANDROID, pcuInfo.dwPCUAndroid);
                }
            }

            acl::string strMachineUserCount;
            strMachineUserCount.format("%u", dwUserCount);
            mapAttrs.insert(std::make_pair(strccu, strMachineUserCount));
        }
    }

    //房间用户
    for (auto& itPSUser : m_mapPlatformServerUser)
    {
        std::map<CT_DWORD, CT_DWORD>& mapServerUser = itPSUser.second;
        CT_BYTE cbPlatformId = itPSUser.first;

        for (auto& itServerUser : mapServerUser)
        {
            CT_DWORD dwGameIndex = itServerUser.first;
            CT_DWORD dwUserCount = itServerUser.second;
            acl::string attrRoomKind;
            acl::string valueRoomKind;
            attrRoomKind.format("server_%d_%u", cbPlatformId, dwGameIndex);
            valueRoomKind.format("%u", dwUserCount);
            mapAttrs.insert(std::make_pair(attrRoomKind, valueRoomKind));
        }
    }

    m_redis.clear();
    if (m_redis.hmset(key.c_str(), mapAttrs) == false)
    {
        LOG(WARNING) << "update online user fail";
    }

	m_updateUserCountTick = dwNow;
}


CT_VOID  CCenterThread::SetPCUInfoToCache(tagPcuInfo& pcuInfo)
{
	acl::string key;
	key.format("totalonline");
	std::map<acl::string, acl::string> attrs;

	acl::string PCU;
	acl::string pcuValue;
	PCU.format("%d", pcuInfo.dwPCU);
	pcuValue.format("pcu_hall_%d",pcuInfo.cbPlatformId);

	acl::string iosPCU;
	acl::string iosPCUValue;
	iosPCU.format("%d", pcuInfo.dwPCUiOS);
	iosPCUValue.format("pcu_ios_%d", pcuInfo.cbPlatformId);

	acl::string androidPCU;
	acl::string androidPCUValue;
	androidPCU.format("%d", pcuInfo.dwPCUAndroid);
	androidPCUValue.format("pcu_android_%d", pcuInfo.cbPlatformId);

	attrs.insert(std::make_pair(pcuValue, PCU));
	attrs.insert(std::make_pair(iosPCUValue, iosPCU));
	attrs.insert(std::make_pair(androidPCUValue, androidPCU));
	m_redis.clear();
	if (m_redis.hmset(key.c_str(), attrs) == false)
	{
		LOG(WARNING) << "set pcu info fail.";
	}
}

CT_VOID CCenterThread::AcrossTheDayUpdatePCU()
{
	//跨天时的当前在线人数为今日最高在线人数
	acl::string key;
	std::map<acl::string, acl::string> result;
	key.format("totalonline");

	m_redis.clear();
	if (m_redis.hgetall(key.c_str(), result) == false)
	{
		return;
	}

	if (result.empty())
	{
		return;
	}

	for (auto& it : m_mapPlatformMachineUser)
	{
		tagPcuInfo& PcuInfo = m_mapPcuInfo[it.first];

		acl::string hall_attr;
		hall_attr.format("ccu_hall_%d", it.first);
		PcuInfo.dwPCU = (CT_DWORD)(atoi)(result[hall_attr.c_str()].c_str());

		acl::string ios_attr;
		ios_attr.format("ccu_ios_%d", it.first);
		PcuInfo.dwPCUiOS = (CT_DWORD)(atoi)(result[ios_attr.c_str()].c_str());

		acl::string android_attr;
		android_attr.format("ccu_android_%d", it.first);
		PcuInfo.dwPCUAndroid = (CT_DWORD)(atoi)(result[android_attr.c_str()].c_str());

		SetPCUInfoToCache(PcuInfo);
	}
}


CT_VOID  CCenterThread::UpdatePCUToDB(CT_BYTE cbPlatformId, CT_BYTE cbPCUType, CT_DWORD dwPCU)
{
	MSG_CS2DB_Update_PCU updatePCU;
	updatePCU.cbType = cbPCUType; 
	updatePCU.dwPCU = dwPCU;
	updatePCU.cbPlatformId = cbPlatformId;

	CNetModule::getSingletonPtr()->Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_PCU, &updatePCU, sizeof(updatePCU));
}

CT_DWORD CCenterThread::GetUserGameServerID(CT_DWORD dwUserID)
{
	acl::string key;
	acl::string val;
	key.format("online_%u", dwUserID);

	m_redis.clear();

	if (m_redis.exists(key) == false)
	{
		return 0;
	}

	if (m_redis.hget(key, "roomnum",  val) == false)
	{
		return 0;
	}

	key.format("proom_info_%s", val.c_str());
	acl::string result;
	m_redis.clear();
	if (m_redis.hget(key.c_str(), "serverid", result) == false)
	{
		return 0;
	}
	
	return atoi(result.c_str());
}

CT_DWORD CCenterThread::GetUserRoomNum(CT_DWORD dwUserID)
{
	acl::string key;
	acl::string val;
	key.format("online_%u", dwUserID);

	m_redis.clear();

	if (m_redis.exists(key) == false)
	{
		return 0;
	}

	if (m_redis.hget(key, "roomnum", val) == false)
	{
		return 0;
	}

	return atoi(val.c_str());
}

CT_VOID CCenterThread::GetUserProxyServerID(CT_DWORD dwUserID, CT_DWORD& dwProxyServerID, CT_UINT64& uClientAddr)
{
	acl::string key;
	acl::string result;

	key.format("psinfo_%u", dwUserID);

	m_redis.clear();
	std::map<acl::string, acl::string> psInfo;
	if (m_redis.hgetall(key, psInfo) == true)
	{
		if (!psInfo.empty())
		{
			dwProxyServerID = atoi(psInfo["psid"]);
			uClientAddr = Utility::stingToUint64(psInfo["netaddr"].c_str());
		}
	}
}

CT_VOID  CCenterThread::RemoveUserPRoomNum(CT_DWORD dwUserID)
{
	acl::string key;
	/*key.format("proom_%u", dwUserID);
	m_redis.clear();
	int nDel = m_redis.del_one(key.c_str());
	if (nDel <= 0)
	{
		LOG(WARNING) << "del proom_user fail, user id: " << dwUserID;
	}*/

	key.format("online_%u", dwUserID);
	m_redis.clear();
	int nDel = m_redis.del_one(key.c_str());
	if (nDel <= 0)
	{
		LOG(WARNING) << "del online_user fail, user id: " << dwUserID;
	}
}

CT_VOID  CCenterThread::GetUserHeadUrl(CT_DWORD dwUserID, CT_CHAR* szHeadUrl)
{
	acl::string key;
	acl::string attr, value;
	key.format("account_%u", dwUserID);
	attr.format("head");

	m_redis.clear();
	if (m_redis.hget(key.c_str(), attr.c_str(), value) == false)
	{
		return;
	}

	memcpy(szHeadUrl, value.c_str(), value.length());
}

CT_VOID CCenterThread::GetUserNickName(CT_DWORD dwUserID, CT_CHAR* szNickName)
{
	acl::string key;
	acl::string attr, value;
	key.format("account_%u", dwUserID);
	attr.format("nick");

	m_redis.clear();
	if (m_redis.hget(key.c_str(), attr.c_str(), value) == false)
	{
		return;
	}

	memcpy(szNickName, value.c_str(), value.length());
}

CT_BOOL CCenterThread::GetAccountInfo(CT_DWORD dwUserID, UserAccountsBaseData& accountData)
{
	acl::string key;
	std::map<acl::string, acl::string> result;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hgetall(key.c_str(), result) == false)
	{
		return false;
	}

	if (result.empty())
	{
		return false;
	}

	accountData.dwUserID = dwUserID;
	accountData.cbPlatformID = atoi(result["pfid"]);
	//accountData.dwGem = atoi(result["gem"]);
	accountData.cbGender = atoi(result["gender"]);
	acl::string nickName = result["nick"];
	acl::string headUrl = result["head"];
	acl::string ip = result["ip"];
	acl::string szLocation = result["location"];
	_snprintf_info(accountData.szNickName, sizeof(accountData.szNickName), "%s", nickName.c_str());
	//_snprintf_info(accountData.szHeadUrl, sizeof(accountData.szHeadUrl), "%s", headUrl.c_str());
	_snprintf_info(accountData.szIP, sizeof(accountData.szIP), "%s", ip.c_str());
	_snprintf_info(accountData.szLocation, sizeof(accountData.szLocation), "%s", szLocation.c_str());

	return true;
}

CT_VOID	 CCenterThread::AddPRoomNum(CT_DWORD dwGroupID, std::shared_ptr<tagGroupPRoomInfo> pRoomInfo)
{
    /*
	auto it = m_mapWxRoomNum.find(dwGroupID);
	if (it == m_mapWxRoomNum.end())
	{
		std::vector<std::shared_ptr<tagGroupPRoomInfo>> setRoomInfo;
		setRoomInfo.push_back(pRoomInfo);
		m_mapWxRoomNum.insert(std::make_pair(dwGroupID, setRoomInfo));
	}
	else
	{
		it->second.push_back(pRoomInfo);
	}
     */
}

CT_VOID  CCenterThread::RemovePRoomNum(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwRoomNum)
{
    /*
	auto it = m_mapWxRoomNum.find(dwGroupID);
	if (it != m_mapWxRoomNum.end())
	{
		std::vector<std::shared_ptr<tagGroupPRoomInfo>>& setPRoomInfo = it->second;

		for (auto itSet = setPRoomInfo.begin(); itSet != setPRoomInfo.end(); ++itSet)
		{
			if ((*itSet)->wGameID == wGameID && (*itSet)->wKindID == wKindID && (*itSet)->dwRoomNum == dwRoomNum)
			{
				setPRoomInfo.erase(itSet);
				return;
			}
		}
	}
	else
	{
		LOG(WARNING) << "remove room num fail, room num: " << dwRoomNum;
	}
     */
}

CT_VOID	CCenterThread::RemoveTimeOutPRoom()
{
	CT_DWORD dwNow = Utility::GetTime();
	for (auto& it : m_mapPRoomInfo)
	{
		//超过半个小时的房间
		stPRoomInfo& proomInfo = it.second;
		MSG_C2GS_DismissPRoom_ForAgent dismissPRoom;
		dismissPRoom.uWebSock = 0;
		dismissPRoom.cbDismissType = 4;

		if (!proomInfo.bStart && (dwNow - proomInfo.dwCreateTime) >= 60*30)
		{
			acl::aio_socket_stream* pSocket = CServerMgr::get_instance().FindGameServer(proomInfo.dwServerID);
			if (pSocket)
			{
				dismissPRoom.dwRoomNum = it.first;
				dismissPRoom.dwGroupID = proomInfo.dwGroupID;
				dismissPRoom.dwRecordID = proomInfo.dwRecordID;
				dismissPRoom.cbGroupType = proomInfo.cbGroupType;
				dismissPRoom.wGameID = proomInfo.wGameID;
				dismissPRoom.wKindID = proomInfo.wKindID;
				dismissPRoom.wPlayCount = proomInfo.wPlayCount;
				dismissPRoom.dwMasterID = proomInfo.dwMasterID;
				CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_DISMISS_PROOM_AGENT, &dismissPRoom, sizeof(dismissPRoom));
			}
		}
	}
}

CT_BOOL	CCenterThread::FindPRoomInfo(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID, /*CT_CHAR szParam[PROOM_JSON_LEN],*/ CT_DWORD& dwRoomNum, CT_WORD& userCount)
{
	auto it = m_mapWxRoomNum.find(dwGroupID);
	if (it == m_mapWxRoomNum.end())
	{
		return false;
	}

	/*
	std::vector<std::shared_ptr<tagGroupPRoomInfo>>& setRoomInfo = it->second;
	for (auto itSet = setRoomInfo.begin(); itSet != setRoomInfo.end(); ++itSet)
	{
		std::shared_ptr<tagGroupPRoomInfo>& pRoomInfo = *itSet;
		if (pRoomInfo->wGameID == wGameID && pRoomInfo->wKindID == wKindID && (strcmp(pRoomInfo->szParam, szParam) == 0))
		{
			dwRoomNum = pRoomInfo->dwRoomNum;

			acl::string key;
			acl::string att_usercount, att_curruser;
			const char* attrs[2];
			std::vector<acl::string> result;
			att_usercount.format("usercount");
			att_curruser.format("curruser");

			attrs[0] = att_usercount.c_str();
			attrs[1] = att_curruser.c_str();

			key.format("proom_info_%u", dwRoomNum);

			m_redis.clear();
			if (m_redis.hmget(key, attrs, 2, &result) == false)
			{
				LOG(WARNING) << "get proom info fail, room num: " << dwRoomNum;
				continue;
			}

			if (result.size() != 2)
			{
				continue;
			}
			CT_WORD wUserCount = atoi(result[0].c_str());
			userCount = atoi(result[1].c_str());
			if (userCount < wUserCount)
			{
				return true;
			}
		}
	}
	*/

	return false;
}

CT_BOOL CCenterThread::CreatePRoomForGroup(const CT_CHAR* pJson, CT_DWORD dwGroupID, CT_DWORD& dwRoomNum, CT_WORD& wCurrUserCount)
{
	acl::json proomJson(pJson);
	
	const acl::json_node* pNodeGameID = proomJson.getFirstElementByTagName("gameid");
	if (pNodeGameID == NULL)
	{
		return false;
	}
	CT_WORD wGameID = (CT_WORD)*pNodeGameID->get_int64();

	const acl::json_node* pNodeKindID = proomJson.getFirstElementByTagName("kindid");
	if (pNodeKindID == NULL)
	{
		return false;
	}
	CT_WORD wKindID = (CT_WORD)*pNodeKindID->get_int64();
	CT_BYTE cbIsVip = 0;

	CMD_GameServer_Info *pGameServer = CServerMgr::get_instance().FindSuitTableGameServer(wGameID, wKindID, 0);
	if (pGameServer == NULL)
	{
		LOG(WARNING) << "auto create room not find suit game server";
		return false;
	}

	CT_WORD wPlayCount = 0;
	const acl::json_node* pPlayCount = proomJson.getFirstElementByTagName("playCount");
	if (pPlayCount == NULL)
	{
		LOG(WARNING) << "auto create room can not find play count param!";
		return false;
	}
	wPlayCount = (CT_WORD)*pPlayCount->get_int64();

	//CT_WORD wNeedGem = CServerMgr::get_instance().GetCreatePRoomNeedGem(pGameServer->dwServerID, wPlayCount);

	CT_DWORD dwSeed = (CT_DWORD)time(NULL);
	srand(dwSeed);
	dwRoomNum = CreateRoomNum(pGameServer->dwServerID);
	while (CheckRoomNumIsExist(dwRoomNum))
	{
		dwSeed += 9527;
		srand(dwSeed);
		dwRoomNum = CreateRoomNum(pGameServer->dwServerID);
		LOG(WARNING) << "create same room num!!";
	}

	CT_WORD wUserCount = pGameServer->wMaxUserCount;
	const acl::json_node* pUserCount = proomJson.getFirstElementByTagName("userCount");
	if (pUserCount != NULL)
	{
		wUserCount = (CT_WORD)*pUserCount->get_int64();
	}

	CT_BOOL bSetCache = SetRoomNumToCache(0, pGameServer->dwServerID, pGameServer->wGameID, pGameServer->wKindID, \
		wUserCount, wPlayCount, cbIsVip, (CT_CHAR*)pJson, dwRoomNum, dwGroupID, GX_GROUP_ROOM);
	if (!bSetCache)
	{
		LOG(WARNING) << "set room info to cache fail.";
	}

	std::shared_ptr<tagGroupPRoomInfo> pRoomInfo = std::make_shared<tagGroupPRoomInfo>();
	pRoomInfo->wGameID = pGameServer->wGameID;
	pRoomInfo->wKindID = pGameServer->wKindID;
	pRoomInfo->dwRoomNum = dwRoomNum;
	//_snprintf_info(pRoomInfo->szParam, sizeof(pRoomInfo->szParam), "%s", pJson);
	CGroupMgr::get_instance().AddPRoomNum(dwGroupID, pRoomInfo);
	return true;
}

CT_VOID	CCenterThread::GoDBServerRegister()
{
	if (pNetDB != NULL && pNetDB->IsRunning())
	{
		CMD_CenterServer_Info registerCenter;
		registerCenter.dwServerID = 1;
		_snprintf_info(registerCenter.szServerIP, sizeof(registerCenter.szServerIP), "%s", CServerCfg::m_LocalAddress);
		_snprintf_info(registerCenter.szServerName, sizeof(registerCenter.szServerName), "%s", "center server");
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, MSG_CS2DB_REGISTER, &registerCenter, sizeof(registerCenter));
		return;
	}
	LOG(WARNING) << "go db server register failed, db server is not running...";
}

CT_VOID CCenterThread::LoadAllGroupInfo()
{
	if (pNetDB)
	{
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_LOAD_GROUP_INFO);
	}
}

CT_VOID CCenterThread::LoadHongBaoInfo()
{
	if (pNetDB)
	{
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_LOAD_HONGBAO_INFO);
	}
}

CT_VOID CCenterThread::LoadTaskInfo()
{
	if (pNetDB)
	{
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_LOAD_TASK);
	}
}