#include "GameServerThread.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "ServerCfg.h"
#include "FishServerCfg.h"
#include "CMD_Game.h"
#include "CMD_ShareMsg.h"
#include "ServerUserItem.h"
#include <stdlib.h>
#include <random>
#include "Utility.h"
#include "GroupMgr.h"
#include "ServerMgr.h"
#include "AndroidUserMgr.h"
#include "FishControl.h"
#ifdef _OS_LINUX_CODE
#include "DdzAiThread.h"
#include "DdzAiStruct.h"
#endif // _OS_LINUX_CODE

//中心服务器连接
extern CNetConnector *pNetCenter;
extern CNetConnector *pNetDB;

#define		IDI_INIT_GAME_DATA			(100)
#define		TIME_INIT_GAME_DATA			(1000)

#define		IDI_TIMER_SYS_CLOCK			(101)
#define		TIME_SYS_CLOCK				(1000)
#define     TIME_SYS_CLOCK_FISH         (200)//(100)

//
#define		IDI_CONNECT_CENTER			(102)
#define		TIME_CONNECT_CENTER			(1000)

//重连DB服务器
#define		IDI_CONNECT_DBSERVER		(103)
#define		TIME_CONNECT_DBSERVER		(1000)

//机器人时钟
#define		IDI_TIMER_ANDROID_CLOCK		(104)	
#define		TIME_ANDROID_CLOCK			(1000)
#define		TIME_ANDROID_CLOCK_BR		(100)

//机器人进入时钟
#define		IDI_TIMER_ANDROID_IN		(105)
#define		TIME_ANDROID_IN				(3000)
#define 	TIME_ANDROID_IN_DDZ			(10000)
#define 	TIME_ANDROID_IN_JZNC		(2000)

//分配桌子时钟distribute
#define		IDI_TIMER_DISTRIBUTE_TABLE	(106)
#define		TIME_DISTRIBUTE_TABLE		(3000)

//AI线程消息读取定时器
#define		IDI_DDZ_AI_ACTION			(107)
#define		TIME_DDZ_AI_ACTION			(1000)

//在GS中定时计算捕鱼血池状态值     定时器ID:108已被使用 存放在GameTableManager.h中
//#define		IDI_FISH_BLOODPOOL_STATE_VALUE (108)
//#define     TIME_FISH_BLOODPOOL_STATE_VALUE 3*60*1000

CGameServerThread::CGameServerThread(void)
	//: m_dwWaitDistributeTime(0)
	: m_redisClient(CServerCfg::m_RedisAddress, CServerCfg::m_RedisConTimeOut, CServerCfg::m_RedisRwTimeOut)
	, m_bInitServer(false)
{
	acl::string charset;
#ifdef _OS_WIN32_CODE
	acl::db_handle::set_loadpath("libmysql.dll");
	charset.format("%s", "gbk");
#elif defined(_OS_LINUX_CODE)
	std::string strlibMysqlPath = Utility::GetCurrentDirectory();
	strlibMysqlPath.append("/libmysqlclient_r.so");
	acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
	charset.format("%s", "utf8");
#else
	std::string strlibMysqlPath = Utility::GetCurrentDirectory();
	strlibMysqlPath.append("/libmysqlclient_r.dylib");
	acl::db_handle::set_loadpath(strlibMysqlPath.c_str());
	charset.format("%s", "utf8");
#endif

	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_AccountdbIP, CServerCfg::m_AccountdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_AccountdbName, CServerCfg::m_AccountdbUser, CServerCfg::m_AccountdbPwd, 1, 0, true, 60, 60, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_RecorddbName, CServerCfg::m_RecorddbUser, CServerCfg::m_RecorddbPwd, 1, 0, true, 60, 60, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbUser, CServerCfg::m_PlatformdbPwd, 1, 0, true, 60, 60, charset.c_str());
}

CGameServerThread::~CGameServerThread(void)
{

}

CT_VOID CGameServerThread::OnTCPSocketLink(CNetConnector* pConnector)
{
	stConnectParam& connectParam = pConnector->GetConnectParam();
	if (connectParam.conType == CONNECT_CENTER)
	{
		if (!m_bInitServer)
		{
			InitServer();
			return;
		}
		GoCenterServerRegister();
		LOG(INFO) << "connect to center server succ!";
	}
	else if(connectParam.conType == CONNECT_DB)
	{
		LOG(INFO) << "connect to db server succ!";
		//CGameTableManager::get_instance().TestWritePlayback();
		//CGameTableManager::get_instance().TestUpdateUserGem();
		//CGameTableManager::get_instance().TestPlayCountRecord();
		//CGameTableManager::get_instance().TestClearingInfo();
	}
}

CT_VOID CGameServerThread::OnTCPSocketShut(CNetConnector* pConnector)
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
	else if (connectParam.conType == CONNECT_DB)
	{
		LOG(WARNING) << "db server is disconnect, after " << TIME_CONNECT_DBSERVER / 1000 << "s will be reconnect!";
		CNetModule::getSingleton().SetTimer(IDI_CONNECT_DBSERVER, TIME_CONNECT_DBSERVER, this, false);
	}
}

CT_VOID CGameServerThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{
	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "bind one connect: " << strIp; //pSocket->get_peer(true);
	}
}

CT_VOID CGameServerThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
	if (pSocket)
	{
		CServerMgr::get_instance().DeleteProxyServer(pSocket);
		CServerUserManager::get_instance().SetUserOffLine(pSocket, this);
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
CT_VOID CGameServerThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (pSocket == NULL)
	{
		return;
	}

	switch (pMc->dwMainID)
	{
	case TEST_CMD:
	{
		sPGTest* pPGTest = (sPGTest*)pBuf;
		acl::string key;
		key.format("%u", pPGTest->nIndex);
		m_redis.expire(key.c_str(), 600);

		CNetModule::getSingleton().Send(pSocket, pMc->dwMainID+1, pMc->dwSubID, pPGTest, wLen);

		break;
	}
	case MSG_GCS_MAIN:
	{
		OnCenterServerMsg(pSocket, pMc, pBuf, wLen);
		break;
	}
	case MSG_PGS_MAIN:
	{
		OnProxyServerMsg(pSocket, pMc, pBuf, wLen);
		break;
	}
	case MSG_FRAME_MAIN:
	{//客户端发来的框架消息
		OnClientFrameMsg(pSocket, pMc, pBuf, wLen);
		break;
	}
	case  MSG_GAME_MAIN:
	{//客户端发来的游戏消息
		OnClientGameMsg(pSocket, pMc, pBuf, wLen);
		break;
	}
	case MSG_GDB_MAIN:
	{
		OnDBServerMsg(pSocket, pMc, pBuf, wLen);
		break;
	}
	default:
		break;
	}
}

void CGameServerThread::OnCenterServerMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize)
{
	switch (pMc->dwSubID)
	{
	case SUB_C2GS_USER_RECHARGE:
	{
		if (wDataSize != sizeof(MSG_C2GS_User_Recharge))
		{
			return;
		}

		MSG_C2GS_User_Recharge* pUserRecharge = (MSG_C2GS_User_Recharge*)pData;
		if (pUserRecharge && pUserRecharge->cbType == 1)
		{
			CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(pUserRecharge->dwUserID);
			if (pUserItem)
			{
				CT_DWORD  dwTempRechargeCount = pUserItem->GetUserTempRechargeCount();
				if (pUserRecharge->dwRechargeTempCount > dwTempRechargeCount)
				{
					if (m_GamePlayKind.wGameID == GAME_FISH && m_GamePlayKind.wRoomKindID == FREE_ROOM)
					{
						return;
					}

					pUserItem->SetUserScore(pUserItem->GetUserScore() + pUserRecharge->llRecharge);
					pUserItem->SetUserTempRechargeCount(pUserRecharge->dwRechargeTempCount);
					CT_WORD wTableID = pUserItem->GetTableID();
					CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(CGameTableManager::get_instance().GetTable(wTableID));
					if (pTableFrame)
					{
						pTableFrame->BroadcastUserScore(pUserItem);
						ITableFrameSink* pTableSink = pTableFrame->GetTableFrameSink();
						if (pTableSink)
						{
							pTableSink->OnUserRecharge(pUserItem->GetChairID(), pUserRecharge->llRecharge);
						}
					}
				}
			}
		}

		else if (pUserRecharge && pUserRecharge->cbType == 2)
		{
			CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(pUserRecharge->dwUserID);
			if (pUserItem)
			{
				pUserItem->SetUserGem(pUserItem->GetUserGem() + (CT_INT32)pUserRecharge->llRecharge);
			}
		}
	}
	break;
	case SUB_C2GS_UPDATE_USER_VIP:
	{
		if (wDataSize != sizeof(MSG_C2GS_Update_UserVip))
		{
			return;
		}

		MSG_C2GS_Update_UserVip* pUserVip = (MSG_C2GS_Update_UserVip*)pData;
		CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(pUserVip->dwUserID);
		if (pUserItem)
		{
			pUserItem->SetUserVip(pUserVip->cbVipLevel);
			pUserItem->SetUserVip2(pUserVip->cbVipLevle2);
		}
	}
	break;
	case SUB_C2GS_DISMISS_PROOM:
	{
		/*if (wDataSize != sizeof(MSG_C2GS_DismissPRoom))
		{
			return;
		}

		MSG_C2GS_DismissPRoom *pDismissPRoom = (MSG_C2GS_DismissPRoom*)pData;
		if (pDismissPRoom == NULL)
		{
			return;
		}

		if (pDismissPRoom->dwRoomNum == 0)
		{
			for (int i = 0; i < m_GamePlayKind.wTableCount; ++i)
			{
				CTableFrame*  pTable = static_cast<CTableFrame*>(CGameTableManager::get_instance().GetTable(i));
				if (pTable && pTable->GetTableisLock())
				{
					pTable->DismissGame();
				}
			}
		}
		else
		{
			CTableFrame*  pTable = CGameTableManager::get_instance().FindTableByRoomNum(pDismissPRoom->dwRoomNum);
			if (pTable && pTable->GetTableisLock())
			{
				pTable->DismissGame();
			}
			else
			{
				DelPRoomInfo(pDismissPRoom->dwRoomNum);
			}
		}*/
	}
	break;
	case SUB_C2GS_GROUP_INFO:
	{
		if (wDataSize % sizeof(tagGroupInfo) != 0)
		{
			return;
		}

		CT_DWORD dwCount = wDataSize / sizeof(tagGroupInfo);
		tagGroupInfo* pGroupInfo = (tagGroupInfo*)pData;
		if (pGroupInfo == NULL)
		{
			return;
		}

		for (CT_DWORD i = 0; i < dwCount; ++i)
		{
			CGSGroupMgr::get_instance().InsertGroupInfo(pGroupInfo++);
		}
	}
	break;
	case SUB_C2GS_GROUP_USRE_INFO:
	{
		if ((wDataSize - sizeof(CT_DWORD)) % sizeof(tagGroupUserInfo) != 0)
		{
			return;
		}

		CT_DWORD dwCount = (wDataSize - sizeof(CT_DWORD)) / sizeof(tagGroupUserInfo);

		CT_DWORD dwGroupID = *((CT_DWORD*)pData);

		tagGroupUserInfo* pGroupInfo = (tagGroupUserInfo*)((CT_BYTE*)pData + sizeof(CT_DWORD));
		if (pGroupInfo == NULL)
		{
			return;
		}

		for (CT_DWORD i = 0; i != dwCount; ++i)
		{
			CGSGroupMgr::get_instance().InsertGroupUser(dwGroupID, pGroupInfo++);
		}
	}
	break;
	case SUB_C2GS_ADD_GROUP:
	{
		if (wDataSize != sizeof(tagGroupInfo))
		{
			return;
		}

		tagGroupInfo* pGroupInfo = (tagGroupInfo*)pData;
		if (pGroupInfo == NULL)
		{
			return;
		}

		CGSGroupMgr::get_instance().InsertGroupInfo(pGroupInfo);
	}
	break;
	case SUB_C2GS_ADD_GROUP_USER:
	{
		if ((wDataSize - sizeof(CT_DWORD)) != sizeof(tagGroupUserInfo))
		{
			return;
		}

		CT_DWORD dwGroupID = *((CT_DWORD*)pData);
		tagGroupUserInfo* pGroupInfo = (tagGroupUserInfo*)((CT_BYTE*)pData + sizeof(CT_DWORD));
		if (pGroupInfo == NULL)
		{
			return;
		}

		CGSGroupMgr::get_instance().InsertGroupUser(dwGroupID, pGroupInfo++);
	}
	break;
	case SUB_C2GS_REMOVE_GROUP:
	{
		if (wDataSize != sizeof(MSG_C2GS_Remove_Group))
		{
			return;
		}

		MSG_C2GS_Remove_Group* pGroup = (MSG_C2GS_Remove_Group*)pData;
		if (pGroup == NULL)
		{
			return;
		}
		CGSGroupMgr::get_instance().RemoveGroup(pGroup->dwGroupID);
	}
	break;
	case SUB_C2GS_REMOVE_GROUP_USER:
	{
		if (wDataSize != sizeof(MSG_C2GS_Remove_GroupUser))
		{
			return;
		}

		MSG_C2GS_Remove_GroupUser* pRemoveUser = (MSG_C2GS_Remove_GroupUser*)pData;
		if (pRemoveUser == NULL)
		{
			return;
		}

		CGSGroupMgr::get_instance().RemoveGroupUser(pRemoveUser->dwGroupID, pRemoveUser->dwUserID);
	}
	break;
    case SUB_C2GS_ADD_GROUP_PROOM:
    {
        if (wDataSize != sizeof(MSG_C2GS_Add_GroupPRoom))
        {
            return;
        }

        MSG_C2GS_Add_GroupPRoom* pAddGroupPRoom = (MSG_C2GS_Add_GroupPRoom*)pData;
        if (pAddGroupPRoom == NULL)
        {
            return;
        }

        CGSGroupMgr::get_instance().AddGroupPRoom(pAddGroupPRoom);
    }
    break;
    case SUB_C2GS_REMOVE_GROUP_PROOM:
    {
        if (wDataSize != sizeof(MSG_C2GS_Remove_GroupPRoom))
        {
            return;
        }
        MSG_C2GS_Remove_GroupPRoom* pRemoveGroupPRoom = (MSG_C2GS_Remove_GroupPRoom*)pData;
        CTableFrameForScore* pTableFrame = CGameTableManager::get_instance().FindTableByGroupRoomNum(pRemoveGroupPRoom->dwRoomNum);
        if (pTableFrame == NULL)
        {
            RemoveGroupPRoom(pRemoveGroupPRoom->dwRoomNum);
        }

        break;
    }
	case SUB_C2GS_QUERY_PROOM_INFO:
	{
		if (wDataSize != sizeof(MSG_C2GS_Query_PRoomInfo))
		{
			return;
		}

		MSG_C2GS_Query_PRoomInfo* pQueryInfo = (MSG_C2GS_Query_PRoomInfo*)pData;
		if (pQueryInfo == NULL)
		{
			return;
		}

		MSG_CS2D_QueryPRoom_Info queryPRoomInfo;
		memset(&queryPRoomInfo, 0, sizeof(queryPRoomInfo));
		queryPRoomInfo.cbResult = 0;
		queryPRoomInfo.uWebSock = pQueryInfo->uWebSock;
		CTableFrame* pTableFrame = CGameTableManager::get_instance().FindTableByRoomNum(pQueryInfo->dwRoomNum);
		if (pTableFrame != NULL)
		{
			for (int i = 0; i < m_GamePlayKind.wStartMaxPlayer; ++i)
			{
				CServerUserItem* pUserItem = pTableFrame->GetTableUserItem(i);
				if (pUserItem)
				{
					queryPRoomInfo.dwUserID[queryPRoomInfo.cbUserCount] = pUserItem->GetUserID();
					_snprintf_info(queryPRoomInfo.szNickName[queryPRoomInfo.cbUserCount], NICKNAME_LEN, "%s", pUserItem->GetUserNickName());
					if (pUserItem->IsBreakLine())
					{
						queryPRoomInfo.cbState[queryPRoomInfo.cbUserCount] = 0;
					}
					else
					{
						queryPRoomInfo.cbState[queryPRoomInfo.cbUserCount] = 1;
					}
					++queryPRoomInfo.cbUserCount;
				}
			}
		}

		CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_G2CS_QUERY_PROOM_INFO, &queryPRoomInfo, sizeof(queryPRoomInfo));
	}
	break;
	case SUB_C2GS_DISMISS_PROOM_AGENT:
	{
		if (wDataSize != sizeof(MSG_C2GS_DismissPRoom_ForAgent))
		{
			return;
		}

		MSG_C2GS_DismissPRoom_ForAgent* pDismissPRoom = (MSG_C2GS_DismissPRoom_ForAgent*)pData;
		if (pDismissPRoom == NULL)
		{
			return;
		}

		CTableFrame* pTableFrame = CGameTableManager::get_instance().FindTableByRoomNum(pDismissPRoom->dwRoomNum);
		CT_WORD wGem = CGameTableManager::get_instance().GetPRoomNeedGem(pDismissPRoom->wPlayCount);
		CT_BYTE disMissResult = 0;
		if (pTableFrame)
		{
			if (pTableFrame->GetGameRoundPhase() == en_GameRound_Free || pDismissPRoom->cbDismissType == 2) //未开始或者是管理员解散
			{
				CT_BYTE cbDismissType = CTableFrame::en_Timeout_LeavePRoom;
				if (pDismissPRoom->cbDismissType == 1 || pDismissPRoom->cbDismissType == 2)
				{
					cbDismissType = CTableFrame::en_Dismiss_PRoom_ByAdmin;
				}
	
				pTableFrame->DismissGame(cbDismissType);
			}
			else
			{
				disMissResult = 3;
			}
		}
		else
		{
			DelPRoomInfo(pDismissPRoom->dwRoomNum);
			CGameTableManager::get_instance().GoCenterUpdatePRoom(pDismissPRoom->wGameID, pDismissPRoom->wKindID, pDismissPRoom->dwRoomNum, pDismissPRoom->dwGroupID, pDismissPRoom->cbGroupType, true, PROOM_END);
			if (pDismissPRoom->dwGroupID != 0)
			{
				if (pDismissPRoom->cbGroupType == GX_GROUP_ROOM)
				{
					CGameTableManager::get_instance().SendBackGroupPRoomGem(pDismissPRoom->dwGroupID, pDismissPRoom->dwRecordID, pDismissPRoom->dwMasterID, wGem);
				}
				else
				{
					CGameTableManager::get_instance().SendBackWXGroupPRoomGem(pDismissPRoom->dwGroupID, pDismissPRoom->dwRecordID);
				}
			}
		}
		if (pDismissPRoom->uWebSock != 0)
		{
			MSG_CS2D_Dismiss_PRoom dismissPRoom;
			dismissPRoom.uWebSock = pDismissPRoom->uWebSock;
			dismissPRoom.cbResult = disMissResult;
			CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_G2CS_DISMISS_PROOM, &dismissPRoom, sizeof(dismissPRoom));
		}
	}
	break;
	case SUB_C2GS_TICK_USER_AGENT:
	{
		if (wDataSize != sizeof(MSG_C2GS_TickUser_ForAgent))
		{
			return;
		}

		MSG_C2GS_TickUser_ForAgent* pTickUser = (MSG_C2GS_TickUser_ForAgent*)pData;
		if (pTickUser == NULL)
		{
			return;
		}

		std::vector<std::string> vecUserID;
		Utility::stringSplit(pTickUser->szUserList, ",", vecUserID);

		MSG_CS2D_Tick_User tickUserResult;
		tickUserResult.cbResult = 0;
		for (auto it : vecUserID)
		{
			CT_DWORD dwUserID = atoi(it.c_str());
			CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
			if (pUserItem == NULL)
			{
				continue;
			}
			ITableFrame *pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
			if (pTableFrame)
			{
				if (pTableFrame->GetGameRoundPhase() == en_GameRound_Free)
				{
					//房卡场强制用户离开时不广播状态给自己
					OnUserLeft(dwUserID, false, true);
				}
				else
				{
					tickUserResult.cbResult = 3;
				}
			}
		}
		tickUserResult.uWebSock = pTickUser->uWebSock;
		CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_G2CS_TICK_USER, &tickUserResult, sizeof(tickUserResult));
	}
	break;
	case SUB_C2GS_UPDATE_GROUP_INFO:
	{
		if (wDataSize != sizeof(MSG_C2GS_Update_GroupInfo))
		{
			return;
		}
		MSG_C2GS_Update_GroupInfo* pGroupInfo = (MSG_C2GS_Update_GroupInfo*)pData;
		if (pGroupInfo == NULL)
		{
			return;
		}

		CGSGroupMgr::get_instance().ModifyGroupOptions(pGroupInfo->dwGroupID, pGroupInfo->dwBindUserPlay);
	}
	break;
	case SUB_C2GS_WXGROUP_INFO:
	{
		if (wDataSize < sizeof(MSG_C2GS_WXGroupInfo))
		{
			return;
		}

		MSG_C2GS_WXGroupInfo* pGroupInfo = (MSG_C2GS_WXGroupInfo*)pData;
		if (pGroupInfo == NULL)
		{
			return;
		}

		CT_DWORD* pUserID = (CT_DWORD*)((CT_BYTE*)pData + sizeof(MSG_C2GS_WXGroupInfo));
		m_wxGroupMgr.InsertGroupInfo(pGroupInfo->dwGroupID, pGroupInfo->dwBindUserPlay);
		while (pGroupInfo->dwUserCount > 0)
		{
			m_wxGroupMgr.InsertGroupUser(pGroupInfo->dwGroupID, *pUserID);
			pGroupInfo->dwUserCount--;
			pUserID++;
		}
	}
	break;
	case SUB_C2GS_ADD_WXGROUP_USER:
	case SUB_C2GS_REMOVE_WXGROUP_USER:
	{
		if (wDataSize < sizeof(MSG_C2GS_AddOrRemove_WXGroupUser))
		{
			return;
		}

		MSG_C2GS_AddOrRemove_WXGroupUser* pUserList = (MSG_C2GS_AddOrRemove_WXGroupUser*)pData;
		if (pUserList == NULL)
		{
			return;
		}

		CT_DWORD* pUserID = (CT_DWORD*)((CT_BYTE*)pData + sizeof(MSG_C2GS_AddOrRemove_WXGroupUser));
		CT_WORD wUserCount = pUserList->wUserCount;
		while (wUserCount > 0)
		{
			if (pMc->dwSubID == SUB_C2GS_ADD_WXGROUP_USER)
			{
				m_wxGroupMgr.InsertGroupUser(pUserList->dwGroupID, *pUserID);
			}
			else if (pMc->dwSubID == SUB_C2GS_REMOVE_WXGROUP_USER)
			{
				m_wxGroupMgr.RemoveGroupUser(pUserList->dwGroupID, *pUserID);
			}
			wUserCount--;
			pUserID++;
		}
		m_wxGroupMgr.InsertGroupInfo(pUserList->dwGroupID, pUserList->dwBindUserPlay);
	}
	break;
    case SUB_C2GS_QUERY_TABLE_INFO:
    {
        if (wDataSize != sizeof(MSG_C2GS_Query_Table_Info))
        {
            return;
        }
        MSG_C2GS_Query_Table_Info* pTableInfo = (MSG_C2GS_Query_Table_Info*)pData;
        if (pTableInfo == NULL)
        {
            return;
        }

        MSG_CS2D_Query_TableInfo stTableInfo;
        stTableInfo.uWebSock = pTableInfo->uWebSock;
        acl::string strJson = OnQueryTableInfo(pTableInfo->cbPlatformId);
        if (strJson.empty() || strJson.size() > SYS_NET_SENDBUF_SIZE-128)
        {
            _snprintf_info(stTableInfo.szJsonUserList, sizeof(stTableInfo.szJsonUserList), "{\"code\":1, \"message\":\"桌子上没有玩家\"}");
            CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_G2CS_QUERY_TABLE_INFO, &stTableInfo, sizeof(stTableInfo));
           // LOG(WARNING) << "game server query game server table information, can not find table information.";
            return;
        }
        strJson = strJson.left(strJson.length() - 1);       // 去掉最后的逗号
        _snprintf_info(stTableInfo.szJsonUserList, sizeof(stTableInfo.szJsonUserList), "{\"code\":0, \"message\":\"\", \"data\":[%s]}", strJson.c_str());
        CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_G2CS_QUERY_TABLE_INFO, &stTableInfo, sizeof(stTableInfo));
       // LOG(INFO) << "game server query game server table information, cmd main:" << MSG_GCS_MAIN << " sub:" << SUB_G2CS_QUERY_TABLE_INFO;
    }
    break;
	case SUB_C2GS_UPDATE_ANDROID:
	{
		LoadAndroidParam();
	}
		break;
	case SUB_C2GS_UPDATE_ANDROID_COUNT:
	{
		if (wDataSize != sizeof(MSG_C2GS_UpdateAndroid_PlayCount))
		{
			return;
		}

		MSG_C2GS_UpdateAndroid_PlayCount* pAndroidCount = (MSG_C2GS_UpdateAndroid_PlayCount*)pData;
		CAndroidUserMgr::get_instance().SetActiveAndroidCount(pAndroidCount->dwAndroidCount);
	}
	break;
	case SUB_C2GS_UPDATE_GS_STATE:
	{
		if (wDataSize != sizeof(MSG_C2GS_Update_GameServer_State))
		{
			return;
		}

		MSG_C2GS_Update_GameServer_State* pState = (MSG_C2GS_Update_GameServer_State*)pData;
		//更新状态
		m_GamePlayKind.cbState = pState->cbState;

		if (m_GamePlayKind.cbState == SERVER_STOP)
		{
			//踢出所有捕鱼玩家
			if (m_GamePlayKind.wGameID == GAME_FISH)
			{
				if(m_GamePlayKind.wKindID == 3)
				{
					//深海捕鱼
					CGameTableManager::get_instance().TickOutAllPlayer();
					//捕鱼服务器关闭的时候存储控制数据
					CFishControl::get_instance().SaveFishControlData(m_dbManager);
					CFishControl::get_instance().PrintFishControlData();
					//捕鱼关服的时候保存当日总吸分和总放分
					CFishControl::get_instance().SaveFishTodayTotalWinLoseScore();
					//CFishControl::get_instance().StatisticFishDeathPro(m_dbManager);
				}
				else if(m_GamePlayKind.wKindID == 4)
				{
					//极速捕鱼
					CGameTableManager::get_instance().TickOutAllPlayer();
                    //关服保存极速捕鱼库存等系统信息
					if(CGameTableManager::get_instance().GetTableFrame().size() > 0)
					{
						//设置极速捕鱼的系统信息和控制信息
						VecTableFrame::iterator itTable = CGameTableManager::get_instance().GetTableFrame().begin();
						for( ; itTable != CGameTableManager::get_instance().GetTableFrame().end(); itTable++)
						{
							ITableFrameSink *pTableFrameSink = (*itTable)->GetTableFrameSink();
							if(pTableFrameSink)
							{
								CT_BYTE cbParam = 1;
								pTableFrameSink->SetJSFishData(en_SaveSysInfo_type, &cbParam);
								break;
							}
						}
					}
				}
				else
				{
					LOG(ERROR) << "捕鱼服务器关服时无效的kindid: "<<m_GamePlayKind.wKindID;
				}
			}
			else if (m_GamePlayKind.wGameID == GAME_FXGZ)
			{
				CGameTableManager::get_instance().TickOutAllPlayer();
			}
		}
	}
	break;
	case SUB_C2GS_UPDATE_GS_STOCK:
	{
		if (wDataSize != sizeof(MSG_C2GS_Update_GameServer_Stock))
		{
			return;
		}

		MSG_C2GS_Update_GameServer_Stock* pStockInfo = (MSG_C2GS_Update_GameServer_Stock*)pData;
		CGameTableManager::get_instance().UpdateTableStockInfo(pStockInfo);
	}
	break;
    case SUB_C2GS_BRGAME_RATIO:
    {
        LoadBrGameChangeCardRatio();
    }
    break;
	case SUB_C2GS_BRGAME_BLACKLIST:
	{
		LoadBrGameBlackList();
	}
	break;
	case SUB_C2GS_FISH_DIANKONG:
	{
		if (m_GamePlayKind.wGameID == GAME_FISH)
		{
			CGameTableManager::get_instance().SetOnlineUserDianKongData(pData, wDataSize);
		}
		else /*if (m_GamePlayKind.wGameID == GAME_FXGZ)*/
		{
			MSG_D2CS_Set_FishDiankong* pDianKong = (MSG_D2CS_Set_FishDiankong*)pData;
			std::vector<MSG_D2CS_Set_FishDiankong> vecDianKongData;
			vecDianKongData.push_back(*pDianKong);
			CGameTableManager::get_instance().UpdateDianKongInfo(vecDianKongData);
		}
	}
	break;
	case SUB_C2GS_ZJH_CARDRATIO:
	{
	    if (wDataSize != sizeof(MSG_D2CS_Reload_ZjhDepot_Ratio))
        {
            return;
        }

        MSG_D2CS_Reload_ZjhDepot_Ratio* pRatio = (MSG_D2CS_Reload_ZjhDepot_Ratio*)pData;

        tagZjhCardDepotRatio zjhCardDepotRatio;
        zjhCardDepotRatio.cbDepotID = pRatio->cbDepotID;
        zjhCardDepotRatio.cbCard1Ration = pRatio->cbCard1Ration;
        zjhCardDepotRatio.cbCard2Ration = pRatio->cbCard2Ration;
        zjhCardDepotRatio.cbCard3Ration = pRatio->cbCard3Ration;
        zjhCardDepotRatio.cbCard4Ration = pRatio->cbCard4Ration;
        zjhCardDepotRatio.cbCard5Ration = pRatio->cbCard5Ration;
        zjhCardDepotRatio.cbCard6Ration = pRatio->cbCard6Ration;
        CGameTableManager::get_instance().UpdateZjhCardDepotRatio(&zjhCardDepotRatio);
	}
	break;
	case SUB_C2GS_BENEFIT_CFG:
	{
		if (wDataSize != sizeof(tagBenefitReward))
		{
			return;
		}
		tagBenefitReward* pBenefitReward = (tagBenefitReward*)pData;
		CServerCfg::UpdateBenefitCfg(pBenefitReward);
	}
	break;
	case SUB_C2GS_BLACK_CHANNEL:
	{
		if (wDataSize != sizeof(tagBlackChannelEx))
		{
			return;
		}
		tagBlackChannelEx* pBlackChannel = (tagBlackChannelEx*)pData;
		CT_DWORD dwGameIndex = CGameTableManager::get_instance().GetGameIndex();
		if (pBlackChannel->dwGameIndex != dwGameIndex)
		{
			return;
		}

		std::vector<tagBlackChannel> vecBlackChannel;
        tagBlackChannel blackChannel;
        blackChannel.dwChannelID = pBlackChannel->dwChannelID;
        blackChannel.wBadCardRation = pBlackChannel->wBadCardRation;
		vecBlackChannel.push_back(blackChannel);
		CGameTableManager::get_instance().UpdateBlackChannel(vecBlackChannel);
	}
	break;
	default:
		break;
	}
}

//客户端发来的框架消息
void CGameServerThread::OnProxyServerMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize)
{
	switch (pMc->dwSubID)
	{
	case SUB_P2GS_BIND:
	{
		if (wDataSize != sizeof(PS_BindData))
		{
			return;
		}

		PS_BindData* pBindData = (PS_BindData*)pData;
		CServerMgr::get_instance().BindProxyServer(pSocket, pBindData);
		//for test
		//CNetModule::getSingleton().CloseSocket(pSocket);
		break;
	}
	case SUB_P2GS_USER_OFFLINE:
	{
		if (sizeof(MSG_P2GS_UserOffLine) != wDataSize)
		{
			return;
		}
		MSG_P2GS_UserOffLine* pOffLine = (MSG_P2GS_UserOffLine*)pData;
		OnUserOffLine(pOffLine->dwUserID);
		break;;
	}
	default:
		break;
	}
}

//客户端发来的框架消息
void CGameServerThread::OnClientFrameMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize)
{
	switch (pMc->dwSubID)
	{
	case SUB_C2S_ENTER_PROOM:
	{
	    /*
		if (wDataSize != sizeof(MSG_PG_EnterPrivateRoom))
		{
			return;
		}

		MSG_PG_EnterPrivateRoom* pEnterRoom = (MSG_PG_EnterPrivateRoom*)pData;
		if (pEnterRoom == NULL)
		{
			return;
		}

		OnUserEnterPrivateRoom(pSocket, pEnterRoom);
	    */
        if (wDataSize != sizeof(MSG_PG_EnterGroupPRoom))
        {
            return;
        }

        MSG_PG_EnterGroupPRoom* pRoom = (MSG_PG_EnterGroupPRoom*)pData;
        if (pRoom == NULL)
        {
            return;
        }

        if (CheckUserReadyEnterRoom(pRoom->dwUserID))
        {
            LOG(WARNING) << "User ready enter room. user id: " << pRoom->dwUserID;

            //给玩家准备进入一个过期时间
            ExpireUserReadyEnterRoom(pRoom->dwUserID);
            return;
        }

        OnUserEnterGroupPRoom(pSocket, pRoom);

        DelUserReadyEnterRoom(pRoom->dwUserID);

		break;
	}
	case SUB_C2S_ENTER_ROOM:
	{
		if (wDataSize != sizeof(MSG_PG_EnterRoom))
		{
			return;
		}

		MSG_PG_EnterRoom* pRoom = (MSG_PG_EnterRoom*)pData;
		if (pRoom == NULL)
		{
			return;
		}
		
		if (CheckUserReadyEnterRoom(pRoom->dwUserID))
		{
			LOG(WARNING) << "User ready enter room. user id: " << pRoom->dwUserID;
			
			//给玩家准备进入一个过期时间
			ExpireUserReadyEnterRoom(pRoom->dwUserID);
			return;
		}
		
		OnUserEnterRoom(pSocket, pRoom);
		
		DelUserReadyEnterRoom(pRoom->dwUserID);
		break;
	}
	case SUB_C2S_USER_READY:
	{
		if (wDataSize != sizeof(MSG_CS_UserReady))
		{
			return;
		}

		MSG_CS_UserReady* pReady = (MSG_CS_UserReady*)pData;
		OnUserReadyMsg(pReady->dwUserID);
		break;
	}
	case SUB_C2S_USER_LEFT:
	{
		if (wDataSize != sizeof(MSG_CS_UserLeft))
		{
			return;
		}

		MSG_CS_UserLeft* pLeft = (MSG_CS_UserLeft*)pData;
		OnUserLeft(pLeft->dwUserID, true, false);
		break;
	}
	case SUB_C2S_APPLY_DISMISS_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS_ApplyDismissPRoom))
		{
			return;
		}

		MSG_CS_ApplyDismissPRoom* pApplyDismissPRoom = (MSG_CS_ApplyDismissPRoom*)pData;
		OnUserApplyDismissPRoom(pApplyDismissPRoom->dwUserID);
		break;
	}
	case SUB_C2S_OPERATE_DISMISS_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS_OperateDismissPRoom))
		{
			return;
		}

		MSG_CS_OperateDismissPRoom* pOperateDismissPRoom = (MSG_CS_OperateDismissPRoom*)pData;
		OnUserOperateDismissPRoom(pOperateDismissPRoom->dwUserID, pOperateDismissPRoom->cbOperate);
		break;
	}
	case SUB_C2S_VOICE_CHAT:
	{
		if (wDataSize != sizeof(MSG_CS_Voice_Chat))
		{
			return;
		}

		MSG_CS_Voice_Chat* pVoiceChat = (MSG_CS_Voice_Chat*)pData;
		if (pVoiceChat)
		{
			OnUserVoiceChat(pVoiceChat->dwUserID, pVoiceChat->szChatUrl);
		}
		break;
	}
	case SUB_C2S_TEXT_CHAT:
	{
		if (wDataSize != sizeof(MSG_CS_Text_Chat))
		{
			return;
		}

		MSG_CS_Text_Chat* pTextChat = (MSG_CS_Text_Chat*)pData;
		if (pTextChat)
		{
			OnUserTextChat(pTextChat->dwUserID, pTextChat->cbType, pTextChat->cbIndex);
		}
		break;
	}
	case SUB_C2S_PULL_APPLY_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS_PullApplyPRoom))
		{
			return;
		}

		MSG_CS_PullApplyPRoom* pPullApply = (MSG_CS_PullApplyPRoom*)pData;
		if (pPullApply)
		{
			OnPRoomOwnerPullApply(pPullApply->dwUserID);
		}
		break;
	}
	case SUB_C2S_OPERATE_APPLY_PROOM:
	{
		if (wDataSize != sizeof(MSG_CS_OperateApplyPRoom))
		{
			return;
		}

		MSG_CS_OperateApplyPRoom* pOperateApply = (MSG_CS_OperateApplyPRoom*)pData;
		if (pOperateApply)
		{
			OnPRoomOwnerOperateApply(pOperateApply->dwOwnerUserID, pOperateApply->dwUserID, pOperateApply->cbOperate);
		}
		break;
	}
	case SUB_C2S_ENTER_ROOM_EX:
	{
		if (wDataSize != sizeof(MSG_PG_EnterRoom))
		{
			return;
		}
		MSG_PG_EnterRoom* pEnterRoomEx = (MSG_PG_EnterRoom*)pData;
		if (pEnterRoomEx)
		{
			OnUserEnterRoomEx(pSocket, pEnterRoomEx);
		}
		break;
	}
	case SUB_C2S_ADD_WAIT_LIST:
	{
		if (wDataSize != sizeof(MSG_PG_EnterRoom))
		{
			return;
		}
		MSG_PG_EnterRoom* pAddWaitList = (MSG_PG_EnterRoom*)pData;
		if (pAddWaitList)
		{
			OnAddUserWaitList(pSocket, pAddWaitList);
		}
		break;
	}
	case SUB_C2S_CHANGE_TABLE:
	{
		if (wDataSize != sizeof(MSG_CS_Change_Table))
		{
			return;
		}
		MSG_CS_Change_Table* pChangeTable = (MSG_CS_Change_Table*)pData;
		if (pChangeTable)
		{
			OnUserChangeTable(pChangeTable->dwUserID);
		}
		break;
	}
	case SUB_C2S_REPORT_USER:
	{
		if (wDataSize !=sizeof(MSG_CS_Report_User))
		{
			return;
		}
		
		MSG_CS_Report_User* pReportUser = (MSG_CS_Report_User*)pData;
		if (pReportUser)
		{
			OnUserReportUser(pReportUser);
		}
		break;
	}
	default:
		break;
	}
}

//客户端发来的游戏消息
void CGameServerThread::OnClientGameMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize)
{
	MSG_GameMsgUpHead* pMsgHend = (MSG_GameMsgUpHead*)pData;
	if (NULL == pMsgHend)
	{
		return;
	}
	//玩家ID
	CT_DWORD dwUserID = pMsgHend->dwUserID;
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (NULL==pUserItem)
	{
		return;
	}
	ITableFrame *pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
	if (NULL == pTableFrame)
	{
		return;
	}
	//游戏消息
	pTableFrame->OnGameEvent(pUserItem->GetChairID(), pMc->dwSubID, pData, wDataSize);
}

void CGameServerThread::OnDBServerMsg(acl::aio_socket_stream* pSocket, const CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wDataSize)
{
	switch (pMc->dwSubID)
	{
	case SUB_DB2G_PROOM_INSERT_GAMEROUND:
	{
		if (wDataSize != sizeof(MSG_DB2G_PRoom_InsertGameRound))
		{
			return;
		}

		MSG_DB2G_PRoom_InsertGameRound* pInsertGameRound = (MSG_DB2G_PRoom_InsertGameRound*)pData;
		CTableFrame* pTable = static_cast<CTableFrame*> (CGameTableManager::get_instance().GetTable(pInsertGameRound->dwTableID));
		if (pTable)
		{
			pTable->InsertGameRoundResult(pInsertGameRound->uGameRoundID);
		}
		break;
	}
	case SUB_DB2G_USER_DIANKONG_DATA:
	{
		CGameTableManager::get_instance().SetOnlineUserDianKongData(pData, wDataSize);
		break;
	}
	case SUB_DB2G_USER_MISS_DATA:
	{
		CGameTableManager::get_instance().SetOnlineUserMissData(pData, wDataSize);
		break;
	}
	default:
		break;
	}
}


CT_VOID CGameServerThread::OnUserReadyMsg(CT_DWORD dwUserID)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}
	ITableFrame* pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
	if (pTableFrame)
	{
		pTableFrame->SetUserReady(pUserItem->GetChairID());
	}
}

CT_VOID CGameServerThread::OnUserOffLine(CT_DWORD dwUserID)
{
	for (auto it = m_listWaitList.begin(); it != m_listWaitList.end(); ++it)
	{
		if (it->dwUserID == dwUserID)
		{
			DelGameUserToProxy(it->dwUserID, it->pProxySock);
			m_listWaitList.erase(it);
            //UpdateServerIDWaitListCount((CT_DWORD)m_listWaitList.size());
			return;
		}
	}

	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}

	ITableFrame *pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
	if (pTableFrame)
	{
		pTableFrame->OnUserOffLine(pUserItem);
	}
}

CT_VOID CGameServerThread::OnUserLeft(CT_DWORD dwUserID, CT_BOOL bSendStateMyself /*= true*/, CT_BOOL bForceLeave /*= false*/)
{
	for (auto it = m_listWaitList.begin(); it != m_listWaitList.end(); ++it)
	{
		if (it->dwUserID == dwUserID)
		{
			DelGameUserToProxy(it->dwUserID, it->pProxySock);
			m_listWaitList.erase(it);
            //UpdateServerIDWaitListCount((CT_DWORD)m_listWaitList.size());
			return;
		}
	}

	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}
	ITableFrame *pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
	if (pTableFrame)
	{
		pTableFrame->OnUserLeft(pUserItem, bSendStateMyself, bForceLeave);
	}
}

//玩家申请解散房间
CT_VOID CGameServerThread::OnUserApplyDismissPRoom(CT_DWORD dwUserID)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}
	CTableFrame *pTableFrame =  static_cast<CTableFrame*>(CGameTableManager::get_instance().GetTable(pUserItem->GetTableID()));
	if (pTableFrame)
	{
		pTableFrame->ApplyDismissPRoom(pUserItem);
	}
}

//玩家操作解散房间
CT_VOID CGameServerThread::OnUserOperateDismissPRoom(CT_DWORD dwUserID, CT_BYTE cbOperateCode)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}
	CTableFrame *pTableFrame = static_cast<CTableFrame*>(CGameTableManager::get_instance().GetTable(pUserItem->GetTableID()));
	if (pTableFrame)
	{
		pTableFrame->OperateDismissPRoom(pUserItem, cbOperateCode);
	}
}

//玩家语音
CT_VOID CGameServerThread::OnUserVoiceChat(CT_DWORD dwUserID, CT_CHAR* pChatUrl)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}
	ITableFrame *pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
	if (pTableFrame)
	{
		pTableFrame->UserVoiceChat(pUserItem, pChatUrl);
	}

}

 //玩家聊天
CT_VOID CGameServerThread::OnUserTextChat(CT_DWORD dwUserID, CT_BYTE cbType, CT_BYTE cbIndex)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}
	ITableFrame *pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
	if (pTableFrame)
	{
		pTableFrame->UserTextChat(pUserItem, cbType, cbIndex);
	}
}

//房主拉取私人房信息
CT_VOID CGameServerThread::OnPRoomOwnerPullApply(CT_DWORD dwUserID)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem == NULL)
	{
		return;
	}
	CTableFrame *pTableFrame = static_cast<CTableFrame*> (CGameTableManager::get_instance().GetTable(pUserItem->GetTableID()));
	if (pTableFrame)
	{
		pTableFrame->SendAllVipPRoomApplyToOwner();
	}
}

//房主处理申请入私人房信息
CT_VOID CGameServerThread::OnPRoomOwnerOperateApply(CT_DWORD dwOwnerID, CT_DWORD dwUserID, CT_BYTE cbOperateCode)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwOwnerID);
	if (pUserItem == NULL)
	{
		return;
	}
	CTableFrame *pTableFrame = static_cast<CTableFrame*>(CGameTableManager::get_instance().GetTable(pUserItem->GetTableID()));
	if (pTableFrame)
	{
		pTableFrame->OwnerOperateApply(dwOwnerID, dwUserID, cbOperateCode);
	}
}

acl::string CGameServerThread::OnQueryTableInfo(CT_BYTE cbPlatformId)
{
    acl::string strJson;
    ListTableFrame lstTableInfo = CGameTableManager::get_instance().GetUsedTableFrame();
    if (lstTableInfo.empty())
    {
        return strJson;
    }

    ListTableFrame::const_iterator citer = lstTableInfo.begin();
    for (; citer != lstTableInfo.end(); ++citer)
    {
		CTableFrameForScore* pTable = dynamic_cast<CTableFrameForScore*>(*citer);
        if (NULL == pTable)
        {
            continue;
        }
        CT_WORD wMaxPlayer = pTable->GetMaxPlayerCount();
        for (CT_INT32 i = 0; i < wMaxPlayer; ++i)
        {
            CServerUserItem* pUserItem = pTable->GetTableUserItem(i);
            if (NULL != pUserItem)
            {
            	CT_BYTE cbUserPlatformId = (CT_BYTE)pUserItem->GetPlatformID();
            	if (cbPlatformId != 255 && cbUserPlatformId != cbPlatformId)
					continue;

                strJson.format_append("{\"tableid\":%u, \"chairid\":%u, \"userid\":%u, \"username\":\"%s\", \"location\":\"%s\"},",
                    pUserItem->GetTableID(), pUserItem->GetChairID(), pUserItem->GetUserID(),
                    pUserItem->GetUserNickName(), pUserItem->GetUserLocation());
            }
        }
    }
    return strJson;
}

//网络定时器
CT_VOID CGameServerThread::OnTimer(CT_DWORD dwTimerID)
{
	switch (dwTimerID)
	{
	case IDI_INIT_GAME_DATA:
	{
		/*LoadGameKind();
		LoadGameKind();
		LoadGameRoundReward();
		LoadBenefitConfig();

		//注册到中心服务器
		GoCenterServerRegister();
		CServerUserManager::get_instance().Init(&m_GamePlayKind);
		CGameTableManager::get_instance().InitAllTable(m_GamePlayKind.wTableCount, &m_GameKind, &m_GamePlayKind, &m_redis, &m_dbManager);

		if (m_GamePlayKind.wRoomKindID != PRIVATE_ROOM)
		{
			CNetModule::getSingleton().SetTimer(IDI_TIMER_ANDROID_CLOCK, TIME_ANDROID_CLOCK, this, true);
			CNetModule::getSingleton().SetTimer(IDI_TIMER_ANDROID_IN, TIME_ANDROID_IN, this, true);
			//加载机器人列表
			LoadAndroidParam();
			CAndroidUserMgr::get_instance().Initialization(&m_GameKind, &m_GamePlayKind);
		}*/

		LOG(INFO) << "init server succ!";
		break;
	}
	case IDI_TIMER_SYS_CLOCK:
	{
		CGameTableManager::get_instance().OnTimePulse();
		break;
	}
	case IDI_CONNECT_CENTER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetCenter);
		break;
	}
	case IDI_CONNECT_DBSERVER:
	{
		CNetModule::getSingleton().InsertReconnector(pNetDB);
		break;
	}
	case IDI_TIMER_ANDROID_CLOCK:
	{
		CAndroidUserMgr::get_instance().OnTimePulse();
		break;
	}
	case IDI_TIMER_ANDROID_IN:
	{
		CAndroidUserMgr::get_instance().OnTimeCheckUserIn();
		break;
	}
	case IDI_TIMER_DISTRIBUTE_TABLE:
	{
		DistributeTable();
		break;
	}
	case IDI_DDZ_AI_ACTION:
	{
		HandleDdzAiAction();
		break;
	}
	case IDI_SAVE_TODAY_SERVER_INFO:
	{
		CFishControl::get_instance().SaveFishTodayTotalWinLoseScore();
		break;
	}
	case IDI_FISH_BLOODPOOL_STATE_VALUE:
	{
		//CGameTableManager::get_instance().CheckFishBloodPoolStateValue();
		CFishControl::get_instance().CheckBaseCapturePro();
		break;
	}
	case IDI_SAVE_FISH_CONTROL_DATA:
	{
		CFishControl::get_instance().SaveFishControlData(m_dbManager);
		break;
	}
	case IDI_SAVE_KUCUN_DATA:
	{
		CFishControl::get_instance().SaveKuCunData();
		break;
	}
	case IDI_SAVE_STATISTICSFISHINFO:
	{
		CFishControl::get_instance().StatisticFishInfo();
		break;
	}
	default:
		break;
	}
}

//打开
CT_VOID CGameServerThread::Open()
{
	//std::cout << "CWorkThread open!" << std::endl;
}

CT_VOID CGameServerThread::InitServer()
{
	m_redisClient.set_password(CServerCfg::m_RedisPwd);
	m_redis.set_client(&m_redisClient);
	//m_platformDBCon.Init();
	//CT_BOOL bRet = ConnectPlatformDB();

	LoadGameKind();
	LoadGameRoomKind();
	LoadGameRoundReward();
	InsertGameServerRoomInfo();

	//注册到中心服务器
	GoCenterServerRegister();
	CServerUserManager::get_instance().Init(&m_GamePlayKind, &m_redis);
	if (m_GamePlayKind.wGameID == GAME_FISH)
	{
		if(m_GamePlayKind.wKindID == 3)
		{
			//深海捕鱼
			//捕鱼桌子初始化之前，读取控制数据
			CT_BOOL bReadSuccess = CFishControl::get_instance().ReadFishControlData(m_dbManager);
			if (bReadSuccess)
			{
				//捕鱼必须成功读取控制数据才能创建桌子.
				CGameTableManager::get_instance().InitAllTable(m_GamePlayKind.wTableCount,this, &m_GameKind, &m_GamePlayKind, &m_redis, &m_dbManager);
				//所有桌子创建之后,启动定时器计算捕鱼血池状态值
				CNetModule::getSingleton().SetTimer(IDI_FISH_BLOODPOOL_STATE_VALUE, TIME_FISH_BLOODPOOL_STATE_VALUE, this, true);
				//每间隔5分钟保存捕鱼控制信息到数据库
				CNetModule::getSingleton().SetTimer(IDI_SAVE_FISH_CONTROL_DATA, CFishServerCfg::Timer_SaveFishControlInfo * 1000, this, true);
				//统计服务器当天税收等信息
				CNetModule::getSingleton().SetTimer(IDI_SAVE_TODAY_SERVER_INFO, TIME_SAVE_TODAY_SERVER_INFO, this, true);
				if (CFishServerCfg::IsSaveCurrentKuCun == 1)
				{
					CNetModule::getSingleton().SetTimer(IDI_SAVE_KUCUN_DATA, CFishServerCfg::Timer_SaveCurrentKuCun * 1000, this, true);
				}
				if (CFishServerCfg::IsStatisticsFishInfo == 1)
				{
					CNetModule::getSingleton().SetTimer(IDI_SAVE_STATISTICSFISHINFO, CFishServerCfg::Timer_StatisticsFishInfo * 1000, this, true);
				}
			}
			else
			{
				LOG(ERROR) << "深海捕鱼启动失败!";
			}
		}
		else if(m_GamePlayKind.wKindID == 4)
		{
			//极速捕鱼
            tagJSFishSystemInfo jsSysInfo;
            tagJSFishControlInfo jsContrlInfo;
            //获取系统信息 和 控制信息
			CT_BOOL bLoadSysInfoSucc = LoadJSFishSystemInfo(m_dbManager, jsSysInfo);
			CT_BOOL bLoadControlSucc = LoadJSFishControlInfo(m_dbManager, jsContrlInfo);

			if(bLoadSysInfoSucc && bLoadControlSucc)
			{
				//修正系统信息:库存不能为0，必须在概率杀分和概率放分之间
				if(jsSysInfo.llSystemStock == 0 && jsSysInfo.llSystemLoseScore == 0 && jsSysInfo.llSystemWinScore == 0 &&
					jsSysInfo.llSystemTax == 0 && jsSysInfo.llSystemDiankongValue == 0)
				{
					CT_INT64 llTempStock = (jsContrlInfo.llProAwardValue + jsContrlInfo.llProKillValue) / 2;
                    jsSysInfo.llSystemStock = llTempStock;
					LOG(ERROR) <<"系统第一次启动,系统当前库存为: "<< jsSysInfo.llSystemStock;
					/*if(llTempStock != 0)
					{
						jsSysInfo.llSystemStock = llTempStock;
						LOG(WARNING) << "库存为0,修正库存为: " << jsSysInfo.llSystemStock << " 系统赢分:"<<jsSysInfo.llSystemWinScore
						<<" 系统输分: "<<jsSysInfo.llSystemLoseScore;
					}
					else
					{
						LOG(ERROR) << "系统库存为0，修正失败, 原因:" << " jsContrlInfo.llProAwardValue:"<< jsContrlInfo.llProAwardValue
						<< " jsContrlInfo.llProKillValue:"<<jsContrlInfo.llProKillValue;
					}*/
				}

				//创建所有桌子
				CGameTableManager::get_instance().InitAllTable(m_GamePlayKind.wTableCount,this, &m_GameKind, &m_GamePlayKind, &m_redis, &m_dbManager);

				//设置系统信息和控制信息到极速捕鱼子游戏
				if(CGameTableManager::get_instance().GetTableFrame().size() > 0)
				{
					//设置极速捕鱼的系统信息和控制信息
					VecTableFrame::iterator itTable = CGameTableManager::get_instance().GetTableFrame().begin();
					for( ; itTable != CGameTableManager::get_instance().GetTableFrame().end(); itTable++)
					{
						ITableFrameSink *pTableFrameSink = (*itTable)->GetTableFrameSink();
						if(pTableFrameSink)
						{
							pTableFrameSink->SetJSFishData(en_SysInfo_type, (void*)&jsSysInfo);
							pTableFrameSink->SetJSFishData(en_ControlInfo_type, (void *)&jsContrlInfo);
							break;
						}
					}
				}
				else
				{
					LOG(ERROR) << "极速捕鱼启动失败 桌子初始化失败!";
				}
			}
			else
			{
				LOG(ERROR) << "极速捕鱼启动失败!";
			}
		}
		else
		{
			LOG(ERROR) << "捕鱼类游戏无效的KindID: "<< m_GamePlayKind.wKindID;
		}
	}
	else
	{
		CGameTableManager::get_instance().InitAllTable(m_GamePlayKind.wTableCount, this, &m_GameKind, &m_GamePlayKind, &m_redis, &m_dbManager);

		//荒野行动加血控信息
		if (m_GamePlayKind.wGameID == GAME_FXGZ)
		{
			//读取血池控制信息
			LoadStockControlInfo();
		}
        //读取点控信息
        LoadDianKongInfo();
		//读取黑渠道信息(针对斗地主)
        LoadBlackChannel();
	}

	if (m_GamePlayKind.wRoomKindID != PRIVATE_ROOM)
	{
		LoadBenefitConfig();
		//加载机器人列表
		LoadAndroidParam();
		CAndroidUserMgr::get_instance().Initialization(&m_GameKind, &m_GamePlayKind, this);
		LoadBrGameBlackList();
		if (m_GamePlayKind.wGameID == GAME_ZJH)
            LoadZjhCardDepotRatio();

		//百人游戏机器人的时间脉冲设置为200ms,其他类型游戏为1s
		if (m_GamePlayKind.wGameID == GAME_BR || m_GamePlayKind.wGameID == GAME_FISH)
		{
			CNetModule::getSingleton().SetTimer(IDI_TIMER_ANDROID_CLOCK, TIME_ANDROID_CLOCK_BR, this, true);
			CAndroidUserMgr::get_instance().SetAndroidTimePlus(TIME_ANDROID_CLOCK_BR);
			//百人牛牛游戏换牌概率
			LoadBrGameChangeCardRatio();
		}
		else
		{
			CNetModule::getSingleton().SetTimer(IDI_TIMER_ANDROID_CLOCK, TIME_ANDROID_CLOCK, this, true);
			CAndroidUserMgr::get_instance().SetAndroidTimePlus(TIME_ANDROID_CLOCK);
		}

		if (m_GamePlayKind.wGameID == GAME_DDZ || m_GamePlayKind.wGameID == GAME_PDK)
			CNetModule::getSingleton().SetTimer(IDI_TIMER_ANDROID_IN, TIME_ANDROID_IN_DDZ, this, true);
		else if(m_GamePlayKind.wGameID != GAME_JZNC)
			CNetModule::getSingleton().SetTimer(IDI_TIMER_ANDROID_IN, TIME_ANDROID_IN, this, true);
	}

	//CNetModule::getSingleton().SetTimer(IDI_INIT_GAME_DATA, TIME_INIT_GAME_DATA, this, false);
	if (m_GamePlayKind.wGameID != GAME_FISH)
	    CNetModule::getSingleton().SetTimer(IDI_TIMER_SYS_CLOCK, TIME_SYS_CLOCK, this, true);
    else
        CNetModule::getSingleton().SetTimer(IDI_TIMER_SYS_CLOCK, TIME_SYS_CLOCK_FISH, this, true);
	//CNetModule::getSingleton().SetTimer(IDI_TIMER_DISTRIBUTE_TABLE, TIME_DISTRIBUTE_TABLE, this, true);

	m_bInitServer = true;

	LOG(INFO) << "init server succ!";
}

CT_VOID CGameServerThread::GoCenterServerRegister()
{
	if (pNetCenter != NULL && pNetCenter->IsRunning())
	{
		CMD_GameServer_Info registerGame;
		registerGame.dwServerID = CServerCfg::m_nServerID;
		registerGame.wGameID = CServerCfg::m_nGameID;
		registerGame.wKindID = CServerCfg::m_nGameKindID;
		registerGame.wRoomKindID = CServerCfg::m_nRoomKindID;
		registerGame.wMaxUserCount = m_GamePlayKind.wStartMaxPlayer;
		registerGame.wMinUserCount = m_GamePlayKind.wStartMinPlayer;
		registerGame.dwFullCount = m_GamePlayKind.dwMaxUserCount;//CServerCfg::m_nMaxCount;
		registerGame.dwOnlineCount = 0;
		registerGame.dwEnterMinScore = m_GamePlayKind.dwEnterMinScore;
		registerGame.dwEnterMaxScore = m_GamePlayKind.dwEnterMaxScore;
		registerGame.dwCellScore = m_GamePlayKind.dwCellScore;
		//_snprintf_info(registerGame.szPRoomPlayCountCond, sizeof(registerGame.szPRoomPlayCountCond), "%s", Utility::Ansi2Utf8(m_GamePlayKind.szPRoomPlayCountCond).c_str());
		//_snprintf_info(registerGame.szPRoomNeedGem, sizeof(registerGame.szPRoomNeedGem), "%s", Utility::Ansi2Utf8(m_GamePlayKind.szPRoomNeedGem).c_str());

		acl::json proomJson(m_GamePlayKind.szPRoomCond);
		const acl::json_node* pNodePlayCount = proomJson.getFirstElementByTagName("playcount");
		if (pNodePlayCount)
		{
			_snprintf_info(registerGame.szPRoomPlayCountCond, sizeof(registerGame.szPRoomPlayCountCond), "%s", Utility::Ansi2Utf8(pNodePlayCount->get_string()).c_str());
		}

		const acl::json_node* pNodeNeedGem = proomJson.getFirstElementByTagName("needgem");
		if (pNodeNeedGem)
		{
			_snprintf_info(registerGame.szPRoomNeedGem, sizeof(registerGame.szPRoomNeedGem), "%s", Utility::Ansi2Utf8(pNodeNeedGem->get_string()).c_str());
		}
		_snprintf_info(registerGame.szPRoomOtherCond, sizeof(registerGame.szPRoomOtherCond), "%s", Utility::Ansi2Utf8(m_GamePlayKind.szPRoomCond).c_str());
		_snprintf_info(registerGame.szServerIP, sizeof(registerGame.szServerIP), "%s", CServerCfg::m_LocalAddress);
		_snprintf_info(registerGame.szPublicIP, sizeof(registerGame.szPublicIP), "%s", CServerCfg::m_PublicAddress);
		_snprintf_info(registerGame.szServerName, sizeof(registerGame.szServerName), "%s", m_GamePlayKind.szRoomKindName);
		_snprintf_info(registerGame.szRoomName, sizeof(registerGame.szRoomName), "%s", CServerCfg::m_ServerName);
		CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_REGISTER, &registerGame, sizeof(registerGame));
		return;
	}
	LOG(WARNING) << "go center server register failed, center net is not running...";
}

CT_BOOL CGameServerThread::ConnectPlatformDB()
{
	/*CT_BOOL ret = m_platformDBCon.Connect(CServerCfg::m_PlatformdbIP,
		CServerCfg::m_PlatformdbUser,
		CServerCfg::m_PlatformdbPwd,
		CServerCfg::m_PlatformdbName,
		CServerCfg::m_PlatformdbPort);

	if (!ret)
	{
		LOG(ERROR) << "connect platform db failed, after 10s will be close service...";
		CNetModule::getSingleton().SetTimer(IDI_KILL_PROCESS, TIME_KILL_PROCESS, this, false);
	}


	return ret;*/
	return true;
}

CT_VOID CGameServerThread::LoadGameKind()
{
	/*m_platformDBCon.Reset();
	m_platformDBCon.SetPcName("G_OUT_LoadGameKindItemByGameID");
	m_platformDBCon.AddParm("in_GameID", CServerCfg::m_nGameID);
	m_platformDBCon.AddParm("in_KindID", CServerCfg::m_nGameKindID);

	if (m_platformDBCon.Exec())
	{
		CCDBResult* pResult = m_platformDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			while (pResult->MoveNextRow())
			{
				pResult->GetValue(0, m_GameKind.wGameID);
				pResult->GetValue(1, m_GameKind.wKindID);
				pResult->GetValue(2, m_GameKind.wSortID);
				pResult->GetValue(3, m_GameKind.szServerDll, PROCESSNAME_STR_LEN);
				std::string strKindName;
				pResult->GetValue(4, strKindName);
				std::string strUtf8KindName = Utility::Ansi2Utf8(strKindName);
				_snprintf_info(m_GameKind.szKindName, sizeof(m_GameKind.szKindName), "%s", strUtf8KindName.c_str());
				break;
			}
			pResult->Release();
		}
	}
	else
	{
		LOG(ERROR) << "load game item failed, after 10s will be close service...";
		CNetModule::getSingleton().SetTimer(IDI_KILL_PROCESS, TIME_KILL_PROCESS, this, false);
	}*/

	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("select gameid, kindid, sortid, serverDllName, kindname from gamekind where gameid = :gameid and kindid = :kindid")
		.set_parameter("gameid", CServerCfg::m_nGameID)
		.set_parameter("kindid", CServerCfg::m_nGameKindID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query game kind fail.";
		pool->put(db);
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_GameKind.wGameID = (CT_WORD)atoi((*row)["gameid"]);
		m_GameKind.wKindID = (CT_WORD)atoi((*row)["kindid"]);
		m_GameKind.wSortID = (CT_WORD)atoi((*row)["sortid"]);
		std::string strServerDllName = (*row)["serverDllName"];
		std::string strKindName = (*row)["kindname"];
		_snprintf_info(m_GameKind.szServerDll, sizeof(m_GameKind.szServerDll), "%s", Utility::Ansi2Utf8(strServerDllName).c_str());
		_snprintf_info(m_GameKind.szKindName, sizeof(m_GameKind.szKindName), "%s", Utility::Ansi2Utf8(strKindName).c_str());
		break;
	}

	db->free_result();

	pool->put(db);
}

CT_VOID CGameServerThread::LoadGameRoomKind()
{
	/*m_platformDBCon.Reset();
	m_platformDBCon.SetPcName("G_OUT_LoadGameRoomKindItemByID");
	m_platformDBCon.AddParm("in_GameID", CServerCfg::m_nGameID);
	m_platformDBCon.AddParm("in_KindID", CServerCfg::m_nGameKindID);
	m_platformDBCon.AddParm("in_RoomKindID", CServerCfg::m_nRoomKindID);

	if (m_platformDBCon.Exec())
	{
		CCDBResult* pResult = m_platformDBCon.GetResult();
		if (NULL != pResult && pResult->GetResult())
		{
			while (pResult->MoveNextRow())
			{
				pResult->GetValue(0, m_GamePlayKind.wGameID);
				pResult->GetValue(1, m_GamePlayKind.wKindID);
				pResult->GetValue(2, m_GamePlayKind.wRoomKindID);
				std::string strRoomKindName;
				pResult->GetValue(3, strRoomKindName);
				std::string strUtf8RoomKindName = Utility::Ansi2Utf8(strRoomKindName);
				_snprintf_info(m_GamePlayKind.szRoomKindName, sizeof(m_GamePlayKind.szRoomKindName), "%s", strUtf8RoomKindName.c_str());
				pResult->GetValue(4, m_GamePlayKind.wSortID);
				pResult->GetValue(5, m_GamePlayKind.dwCellScore);
				pResult->GetValue(6, m_GamePlayKind.wTableCount);
				pResult->GetValue(7, m_GamePlayKind.wStartMinPlayer);
				pResult->GetValue(8, m_GamePlayKind.wStartMaxPlayer);
				pResult->GetValue(9, m_GamePlayKind.dwEnterMinScore);
				pResult->GetValue(10, m_GamePlayKind.dwEnterMaxScore);
				pResult->GetValue(11, m_GamePlayKind.wEnterVipLevel);
				pResult->GetValue(12, m_GamePlayKind.dwRevenue);
				pResult->GetValue(13, m_GamePlayKind.cbCreateRoomPlayCount1);
				pResult->GetValue(14, m_GamePlayKind.wCreateRoomNeedGem1);
				pResult->GetValue(15, m_GamePlayKind.wCreateRoomHuxi1);
				pResult->GetValue(16, m_GamePlayKind.cbCreateRoomPlayCount2);
				pResult->GetValue(17, m_GamePlayKind.wCreateRoomNeedGem2);
				pResult->GetValue(18, m_GamePlayKind.wCreateRoomHuxi2);
				break;
			}
			pResult->Release();
		}
	}
	else
	{
		LOG(ERROR) << "load game item failed, after 10s will be close service...";
		CNetModule::getSingleton().SetTimer(IDI_KILL_PROCESS, TIME_KILL_PROCESS, this, false);
	}*/

	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("SELECT * FROM gameroomkind where gameid = :gameid AND kindid = :kindid AND roomKindid = :roomKindid")
		.set_parameter("gameid", CServerCfg::m_nGameID)
		.set_parameter("kindid", CServerCfg::m_nGameKindID)
		.set_parameter("roomKindid", CServerCfg::m_nRoomKindID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query game room kind fail.";
		pool->put(db);
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		m_GamePlayKind.wGameID			= (CT_WORD)atoi((*row)["gameid"]);
		m_GamePlayKind.wKindID			= (CT_WORD)atoi((*row)["kindid"]);
		m_GamePlayKind.wRoomKindID		= (CT_WORD)atoi((*row)["roomKindid"]);
		std::string strRoomKindName		= (*row)["roomKindname"];
		_snprintf_info(m_GamePlayKind.szRoomKindName, sizeof(m_GamePlayKind.szRoomKindName), "%s", Utility::Ansi2Utf8(strRoomKindName).c_str());
		//m_GamePlayKind.wSortID			= (CT_WORD)atoi((*row)["sortid"]);
		m_GamePlayKind.dwCellScore		= (CT_DWORD)atoi((*row)["cellScore"]);
		m_GamePlayKind.wTableCount		= (CT_WORD)atoi((*row)["tableCount"]);
		m_GamePlayKind.wStartMinPlayer	= (CT_WORD)atoi((*row)["startMinPlayer"]);
		m_GamePlayKind.wStartMaxPlayer	= (CT_WORD)atoi((*row)["startMaxPlayer"]);
		m_GamePlayKind.dwEnterMinScore	= (CT_DWORD)atoi((*row)["enterMinScore"]);
		m_GamePlayKind.dwEnterMaxScore	= (CT_DWORD)atoi((*row)["enterMaxScore"]);
		m_GamePlayKind.dwCellScore		= (CT_DWORD)atoi((*row)["cellScore"]);
		m_GamePlayKind.wEnterVipLevel	= (CT_WORD)atoi((*row)["enterVipLevel"]);
		m_GamePlayKind.dwRevenue		= (CT_DWORD)atoi((*row)["revenue"]);
		m_GamePlayKind.dwMaxUserCount	= (CT_DWORD)atoi((*row)["maxUserCount"]);//服务器承载最大人数
		m_GamePlayKind.iBroadcastScore = atoi((*row)["broadcastScore"]);
		std::string strPRoomOtherCond = (*row)["proomOtherCond"];
		_snprintf_info(m_GamePlayKind.szPRoomCond, sizeof(m_GamePlayKind.szPRoomCond), "%s", Utility::Ansi2Utf8(strPRoomOtherCond).c_str());
		m_GamePlayKind.cbState = SERVER_RUN;
		break;
	}

	db->free_result();

	pool->put(db);
}

CT_VOID CGameServerThread::LoadGameRoundReward()
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("select gameRoundCount, rewardGem from gameroundcfg");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query game round cfg fail.";
		pool->put(db);
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		CT_DWORD dwGameRoundCount = 0;
		CT_DWORD dwReward = 0;

		const acl::db_row* row = (*db)[i];
		dwGameRoundCount = (CT_DWORD)atoi((*row)["gameRoundCount"]);
		dwReward = (CT_DWORD)atoi((*row)["rewardGem"]);
		CGameTableManager::get_instance().InsertGameRoundRewardCfg(dwGameRoundCount, dwReward);
	}

	db->free_result();

	pool->put(db);
}

CT_VOID	CGameServerThread::LoadAndroidParam()
{
	acl::db_pool* pool = CGameTableManager::get_instance().GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("SELECT androidCfg.userID, androidCfg.enterTime, androidCfg.leaveTime, androidCfg.takeMinScore, androidCfg.takeMaxScore, androidCfg.bigJetton,\
		`user`.nickname, userinfo.gem, userinfo.gender, userinfo.headId, userdata.vip2 from androidCfg \
		INNER JOIN `user` ON `user`.userid = androidCfg.userID \
		INNER JOIN userinfo ON userinfo.userid = androidCfg.userID \
		INNER JOIN userdata ON userdata.userid = androidCfg.userID \
		WHERE androidCfg.serverID = :serverid and androidCfg.`status` = 1")
		.set_format("serverid", "%u", CServerCfg::m_nServerID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query android cfg fail.";
		pool->put(db);
		return;
	}
	CAndroidUserMgr::get_instance().ClearAndroidUserParam();
	tagAndroidUserParameter androidUserParam;
	for (size_t i = 0; i < db->length(); i++)
	{
		memset(&androidUserParam, 0, sizeof(androidUserParam));
		const acl::db_row* row = (*db)[i];
		androidUserParam.dwUserID = atoi((*row)["userID"]);
		androidUserParam.dwEnterTime = atoi((*row)["enterTime"]);
		androidUserParam.dwLeaveTime = atoi((*row)["leaveTime"]);
		androidUserParam.llTakeMinScore = atoll((*row)["takeMinScore"]);
		androidUserParam.llTakeMaxScore = atoll((*row)["takeMaxScore"]);
		androidUserParam.dwGem = atoi((*row)["gem"]);
		androidUserParam.cbBigJetton = (CT_BYTE)atoi((*row)["bigJetton"]);
		androidUserParam.cbSex = (CT_BYTE)atoi((*row)["gender"]);
		androidUserParam.cbHeadId = (CT_BYTE)atoi((*row)["headId"]);
		androidUserParam.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
		std::string strNickName = (*row)["nickname"];
		_snprintf_info(androidUserParam.szNickName, sizeof(androidUserParam.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());
		CAndroidUserMgr::get_instance().InsertAndroidUserParam(&androidUserParam);
	}
	LOG(INFO) << "load android parameter succ, android count : " << (int)db->length();
	
	//如果斗地主游戏，则启动AI线程
	if (m_GamePlayKind.wGameID == GAME_DDZ)
	{
		if (db->length() != 0)
		{
			BuildDdzAiThread();
			CNetModule::getSingleton().SetTimer(IDI_DDZ_AI_ACTION, TIME_DDZ_AI_ACTION, this, true);
			LOG(INFO) << "SET DDZ AI TIMER.";
		}
	}

	db->free_result();

	pool->put(db);

	
}

CT_VOID	CGameServerThread::LoadBenefitConfig()
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}

	acl::query query;

	query.reset();
	query.create_sql("SELECT StatusValue from systemstatus where statusName = 'benefitStatus'");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select register score fail, error: " << db->get_errno();
		pool->put(db);
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CServerCfg::m_nBenefitStatus = (CT_BYTE)atoi((*row)["statusValue"]);
	}
	db->free_result();

	query.reset();
	query.create_sql("SELECT * FROM benefit_config");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query benefit config fail.";
		pool->put(db);
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
        tagBenefitReward benefitReward;
        const acl::db_row* row = (*db)[i];
		benefitReward.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
		benefitReward.cbRewardCount = (CT_BYTE)atoi((*row)["totalCount"]);
		benefitReward.dwRewardScore = atoi((*row)["rewardScore"]);
		benefitReward.dwLessScore = atoi((*row)["lessScore"]);
		CServerCfg::InsertBenefitCfg(&benefitReward);
	}

	if (db->length() > 0)
	{
		LOG(INFO) << "load benefit config succ!";
	}
	else
	{
		LOG(WARNING) << "load benefit config failed!";
	}

	db->free_result();
	pool->put(db);
}

CT_VOID CGameServerThread::InsertGameServerRoomInfo()
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("INSERT INTO roomserver(serverid, gameid, kindid, roomKindid, roomServerName) VALUES(:serverid, :gameid, :kindid, :roomkind, :servername) ON DUPLICATE KEY UPDATE \
		roomServerName = :servername")
		.set_format("serverid",  "%d", CServerCfg::m_nServerID)
		.set_format("gameid",    "%d", m_GamePlayKind.wGameID)
		.set_format("kindid",    "%d", m_GamePlayKind.wKindID)
		.set_format("roomkind",  "%d", m_GamePlayKind.wRoomKindID)
		.set_format("servername","%s", Utility::Utf82Ansi(CServerCfg::m_ServerName).c_str());

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert room server info fail, error: " << db->get_errno();
		pool->put(db);
		return;
	}
	db->free_result();

	query.reset();
	query.create_sql("SELECT androidCount, totalStock, totalStockLowerLimit, todayStock, todayStockHighLimit, androidStock, \
			androidStockLowerLimit, systemAllKillRatio, changeCardRatio from roomserver WHERE serverid = :serverid")
		.set_format("serverid", "%d", CServerCfg::m_nServerID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select room server info fail, error: " << db->get_errno();
		pool->put(db);
		return;
	}

	CAndroidUserMgr::get_instance().SetActiveAndroidCount(0);
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CT_DWORD dwAndroidCount = (CT_DWORD)atoi((*row)["androidCount"]);
		CT_LONGLONG llTotalStock = (CT_LONGLONG)atoll((*row)["totalStock"]);
		CT_LONGLONG llTotalStockLowerLimit = (CT_LONGLONG)atoll((*row)["totalStockLowerLimit"]);
		CT_LONGLONG	llTodayStock = (CT_LONGLONG)atoll((*row)["todayStock"]);
		CT_LONGLONG	llTodayStockLimit = (CT_LONGLONG)atoll((*row)["todayStockHighLimit"]);
		CT_LONGLONG llAndroidStock = (CT_LONGLONG)atoll((*row)["androidStock"]);
		CT_LONGLONG llAndroidStockLowerLimit = (CT_LONGLONG)atoll((*row)["androidStockLowerLimit"]);
		CT_WORD wSystemAllKillRatio = atoi((*row)["systemAllKillRatio"]);
		CT_WORD wChangeCardRatio = atoi((*row)["changeCardRatio"]);
		tagStockInfo stockInfo;
		stockInfo.llStorageControl = llTotalStock;
		stockInfo.llStorageLowerLimit = llTotalStockLowerLimit;
		stockInfo.llAndroidStorage = llAndroidStock;
		stockInfo.llAndroidStorageLowerLimit = llAndroidStockLowerLimit;
		stockInfo.wSystemAllKillRatio = wSystemAllKillRatio;
		stockInfo.llTodayStorageControl = llTodayStock;
		stockInfo.llTodayStorageHighLimit = llTodayStockLimit;
		stockInfo.wChangeCardRatio = wChangeCardRatio;

		CAndroidUserMgr::get_instance().SetActiveAndroidCount(dwAndroidCount);
		CGameTableManager::get_instance().SetTableStockInfo(stockInfo);
	}

	db->free_result();
	pool->put(db);
}


CT_VOID CGameServerThread::LoadBrGameChangeCardRatio()
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}
	
	acl::query query;
	query.create_sql("SELECT * FROM roomserver_operate_ratio WHERE serverid = :serverid ORDER BY todayStock")
	.set_format("serverid", "%d", CServerCfg::m_nServerID);
	
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select room server roomserver_operate_ratio ratio fail, error: " << db->get_errno();
		pool->put(db);
		return;
	}
	
	std::vector<tagBrGameOperateRatio> vecChangeCardRatio;
	tagBrGameOperateRatio operateRatio;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		operateRatio.llTotayStock = atoll((*row)["todayStock"]);
		operateRatio.wChangeCardRatio = (CT_WORD)atoi((*row)["changeCardRatio"]);
		operateRatio.wSystemAllKillRatio = (CT_WORD)atoi((*row)["allKillRatio"]);
		vecChangeCardRatio.push_back(operateRatio);
	}
	
	if (!vecChangeCardRatio.empty())
	{
		CGameTableManager::get_instance().UpdateTableChangeCardRatio(vecChangeCardRatio);
	}
	
	db->free_result();
	pool->put(db);
}

CT_VOID CGameServerThread::LoadBrGameBlackList()
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}
	
	acl::query query;
	query.create_sql("SELECT * FROM roomserver_blacklist WHERE serverid = :serverid")
	.set_format("serverid", "%d", CServerCfg::m_nServerID);
	
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select room server black list fail, error: " << db->get_errno() << ", error: " << db->get_error();
		pool->put(db);
		return;
	}
	
	std::vector<tagBrGameBlackList> vecBlackList;
	tagBrGameBlackList blackList;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		blackList.dwUserID = atoll((*row)["userid"]);
		blackList.wUserLostRatio = (CT_WORD)atoi((*row)["lostRatio"]);
		blackList.dwControlScore = (CT_DWORD)atoi((*row)["controlScore"]);
		vecBlackList.push_back(blackList);
	}
	
	CGameTableManager::get_instance().UpdateBrGameBlackList(vecBlackList);
	
	db->free_result();
	pool->put(db);
}

CT_VOID CGameServerThread::LoadZjhCardDepotRatio()
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("SELECT * FROM zjh_card_ratio");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select zjh card depot ratio fail, error: " << db->get_errno() << ", error: " << db->get_error();
		pool->put(db);
		return;
	}

    tagZjhCardDepotRatio zjhCardDepotRatio;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
        zjhCardDepotRatio.cbDepotID = (CT_BYTE)atoi((*row)["depot_id"]);
        zjhCardDepotRatio.cbCard1Ration = (CT_BYTE)atoi((*row)["ratio_1"]);
        zjhCardDepotRatio.cbCard2Ration = (CT_BYTE)atoi((*row)["ratio_2"]);
        zjhCardDepotRatio.cbCard3Ration = (CT_BYTE)atoi((*row)["ratio_3"]);
        zjhCardDepotRatio.cbCard4Ration = (CT_BYTE)atoi((*row)["ratio_4"]);
        zjhCardDepotRatio.cbCard5Ration = (CT_BYTE)atoi((*row)["ratio_5"]);
        zjhCardDepotRatio.cbCard6Ration = (CT_BYTE)atoi((*row)["ratio_6"]);
        CGameTableManager::get_instance().UpdateZjhCardDepotRatio(&zjhCardDepotRatio);
	}

	db->free_result();
	pool->put(db);
}

CT_VOID CGameServerThread::LoadStockControlInfo()
{
    acl::string platformDBKey;
    platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
    acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
    if (pool == NULL)
    {
        LOG(WARNING) << "get platform db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get platform db handle fail.";
        return;
    }

    std::vector<tagStockControlInfo> vecStockControlInfo;
	for (int nRoomId = 0; nRoomId < 4; ++nRoomId)
	{
		acl::query query;
		CT_DWORD dwServerID = CServerCfg::m_nServerID*100+nRoomId;
		query.create_sql("SELECT * FROM stock_control_data WHERE ServerID = :serverID")
			.set_format("serverID", "%u", dwServerID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "LoadStockControlInfo fail, error: " << db->get_errno() << ", error: " << db->get_error();
			pool->put(db);
			return;
		}

		tagStockControlInfo stockControlInfo;
		memset(&stockControlInfo, 0, sizeof(stockControlInfo));
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			stockControlInfo.dwServerID = dwServerID;
			stockControlInfo.llTotalWinScore = atoll((*row)["TotalWinScore"]);
			stockControlInfo.llTotalLostScore = atoll((*row)["TotalLoseScore"]);
			stockControlInfo.llStock = atoll((*row)["KuCun"]);
			stockControlInfo.llBlackTax = atoll((*row)["BlackTax"]);
			stockControlInfo.llDianKongWinLost = atoll((*row)["BloodPoolDianKongLoseWin"]);
			stockControlInfo.iStockStatus = atoi((*row)["KuCunStatus"]);
		}
		vecStockControlInfo.push_back(stockControlInfo);
		db->free_result();
	}
	CGameTableManager::get_instance().SetTableStockControlInfo(vecStockControlInfo);

    pool->put(db);
}

CT_VOID CGameServerThread::LoadDianKongInfo()
{
    acl::string platformDBKey;
    platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
    acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
    if (pool == NULL)
    {
        LOG(WARNING) << "get platform db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get platform db handle fail.";
        return;
    }

    acl::query query;
    query.create_sql("SELECT * FROM game.user_diankong_data WHERE gameIndex = :gameIndex")
        .set_format("gameIndex", "%u", m_GamePlayKind.wGameID*10000+m_GamePlayKind.wKindID*100+m_GamePlayKind.wRoomKindID);

    if (db->exec_select(query) == false)
    {
        LOG(WARNING) << "LoadDianKongInfo fail, error: " << db->get_errno() << ", error: " << db->get_error();
        pool->put(db);
        return;
    }

    std::vector<MSG_D2CS_Set_FishDiankong> vecDianKongData;
    MSG_D2CS_Set_FishDiankong diankongData;
    for (size_t i = 0; i < db->length(); i++)
    {
        const acl::db_row* row = (*db)[i];
        diankongData.dwUserID = atoi((*row)["userID"]);
        diankongData.iDianKongZhi = atoi((*row)["dianKongZhi"]);
        diankongData.llDianKongFen = atoll((*row)["dianKongFen"]);
        diankongData.llCurrDianKongFen = atoll((*row)["currDianKongFen"]);
        vecDianKongData.push_back(diankongData);
    }
    CGameTableManager::get_instance().UpdateDianKongInfo(vecDianKongData);

    db->free_result();
    pool->put(db);
}

CT_BOOL CGameServerThread::LoadJSFishSystemInfo(acl::mysql_manager &dbManager, tagJSFishSystemInfo& info)
{
	acl::string platformDBKey;
	platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	acl::db_pool* pool = (acl::db_pool*)dbManager.get(platformDBKey.c_str(), false, true);
	if (pool == NULL)
	{
		LOG(ERROR) << "LoadJSFishSystemInfo get platform db pool fail.";
		return CT_FALSE;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(ERROR) << "LoadJSFishSystemInfo get platform db handle fail.";
		return CT_FALSE;
	}

	acl::query query;
	query.create_sql("select TotalWinScore, TotalLoseScore, KuCun, BlackTax, BloodPoolDianKongLoseWin from stock_control_data where ServerID = :ServerID")
			.set_parameter("ServerID", CServerCfg::m_nServerID);
	if (db->exec_select(query) == false)
	{
		LOG(ERROR) << "LoadJSFishSystemInfo query game kind fail." << "ServerID: " << CServerCfg::m_nServerID << " db->get_errno: " << db->get_errno()
				   << " db->get_error: " << db->get_error();
		pool->put(db);
		return CT_FALSE;
	}

	if (db->length() == 0)
	{
		//初次启动
		info.llSystemDiankongValue = 0;
		info.llSystemLoseScore = 0;
		info.llSystemStock = 0;
		info.llSystemTax = 0;
		info.llSystemWinScore = 0;
	}
	else if(db->length() == 1)
	{
		const acl::db_row* row = (*db)[0];
		info.llSystemTax = atoll((*row)["BlackTax"]);;
		info.llSystemStock = atoll((*row)["KuCun"]);
		info.llSystemLoseScore = atoll((*row)["TotalLoseScore"]);
		info.llSystemWinScore = atoll((*row)["TotalWinScore"]);
		info.llSystemDiankongValue = atoll((*row)["BloodPoolDianKongLoseWin"]);
	}
	else
	{
		LOG(ERROR) << "LoadJSFishSystemInfo return DataSet count: " << db->length();
		db->free_result();
		pool->put(db);
		return CT_FALSE;
	}

	LOG(INFO) << "systemwinscore: " << info.llSystemWinScore << " systemlosescore:" << info.llSystemLoseScore
			  << " systemstock:" << info.llSystemStock << " systemtax:" << info.llSystemTax
			  << " systemdiankong: "<< info.llSystemDiankongValue;
	db->free_result();
	pool->put(db);

	return CT_TRUE;
}

CT_BOOL CGameServerThread::LoadJSFishControlInfo(acl::mysql_manager &dbManager, tagJSFishControlInfo &info)
{
    acl::string platformDBKey;
    platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
    acl::db_pool* pool = (acl::db_pool*)dbManager.get(platformDBKey.c_str(), false, true);
    if (pool == NULL)
    {
        LOG(ERROR) << "LoadJSFishControlInfo get platform db pool fail.";
        return CT_FALSE;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(ERROR) << "LoadJSFishControlInfo get platform db handle fail.";
        return CT_FALSE;
    }

    acl::query query;
    query.create_sql("select MustKillValue, ProKillValue, ProAwardValue, MustAwardValue, RevenueCount from jsfish_control_info where ServerID = :ServerID")
            .set_parameter("ServerID", CServerCfg::m_nServerID);
    if (db->exec_select(query) == false)
    {
        LOG(ERROR) << "LoadJSFishControlInfo query game kind fail." << "ServerID: " << CServerCfg::m_nServerID << " db->get_errno: " << db->get_errno()
                   << " db->get_error: " << db->get_error();
        pool->put(db);
        return CT_FALSE;
    }

    if (db->length() == 0)
    {
    	//LOG(ERROR) << "请在platform.jsfish_control_info表中配置此服务器的控制信息。"
		db->free_result();

		int nMusetKillValue = 1000000;
		int nProKillValue = 20000000;
		int nProAwardValue = 22000000;
		int nMustAwardValue = 72000000;
		int nRevenueCount = 100;
		//如果数据库中没有配置就默认插入
		acl::query query_insert;
		query_insert.create_sql("insert into jsfish_control_info(ServerID, MustKillValue, ProKillValue, ProAwardValue, MustAwardValue, RevenueCount) \
						  VALUES(:ServerID, :MustKillValue, :ProKillValue, :ProAwardValue, :MustAwardValue, :RevenueCount)")
						  .set_parameter("ServerID", CServerCfg::m_nServerID)
						  .set_parameter("MustKillValue", nMusetKillValue)
						  .set_parameter("ProKillValue", nProKillValue)
						  .set_parameter("ProAwardValue", nProAwardValue)
						  .set_parameter("MustAwardValue", nMustAwardValue)
						  .set_parameter("RevenueCount", nRevenueCount);
		if(db->exec_update(query_insert) == false)
		{
			LOG(ERROR) << "服务器启动失败，jsfish_control_info中没有该服务器的数据，并且向表中插入默认数据也失败!";
			pool->put(db);
			return CT_FALSE;
		}

		db->free_result();
		pool->put(db);
		info.llMustKillValue = nMusetKillValue;
		info.llProKillValue = nProKillValue;
		info.llProAwardValue = nProAwardValue;
		info.nRevenueCount = nRevenueCount;
		info.llMustAwardValue = nMustAwardValue;
        return CT_TRUE;
    }
    else if(db->length() == 1)
    {
        const acl::db_row* row = (*db)[0];
        info.llMustKillValue = atoll((*row)["MustKillValue"]);;
        info.llProKillValue = atoll((*row)["ProKillValue"]);
        info.llProAwardValue = atoll((*row)["ProAwardValue"]);
        info.llMustAwardValue = atoll((*row)["MustAwardValue"]);
        info.nRevenueCount = atoi((*row)["RevenueCount"]);
    }
    else
    {
        LOG(ERROR) << "LoadJSFishControlInfo return DataSet count: " << db->length();
        db->free_result();
        pool->put(db);
        return CT_FALSE;
    }

    LOG(INFO) << "llMustKillValue: " << info.llMustKillValue << " llProKillValue:" << info.llProKillValue
              << " llProAwardValue:" << info.llProAwardValue << " llMustAwardValue:" << info.llMustAwardValue
              << " nRevenueCount: "<< info.nRevenueCount;
    db->free_result();
    pool->put(db);

    return CT_TRUE;
}

CT_VOID CGameServerThread::LoadBlackChannel()
{
    acl::string platformDBKey;
    platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
    acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(platformDBKey.c_str(), false, true);
    if (pool == NULL)
    {
        LOG(WARNING) << "get platform db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get platform db handle fail.";
        return;
    }

    acl::query query;
    query.create_sql("SELECT * FROM black_channel WHERE gameIndex = :gameIndex")
        .set_format("gameIndex", "%u", m_GamePlayKind.wGameID*10000+m_GamePlayKind.wKindID*100+m_GamePlayKind.wRoomKindID);

    if (db->exec_select(query) == false)
    {
        LOG(WARNING) << "LoadBlackChannel fail, error: " << db->get_errno() << ", error: " << db->get_error();
        pool->put(db);
        return;
    }

    std::vector<tagBlackChannel> vecBlackChannel;
    for (size_t i = 0; i < db->length(); i++)
    {
        const acl::db_row* row = (*db)[i];
        tagBlackChannel blackChannel;
        blackChannel.dwChannelID = (CT_DWORD)atoi((*row)["channelId"]);
        blackChannel.wBadCardRation = (CT_WORD)atoi((*row)["badCardRatio"]);
        vecBlackChannel.push_back(blackChannel);
    }
    CGameTableManager::get_instance().UpdateBlackChannel(vecBlackChannel);

    db->free_result();
    pool->put(db);
}

CT_VOID	CGameServerThread::OnUserEnterPrivateRoom(acl::aio_socket_stream* pSocket, MSG_PG_EnterPrivateRoom* pPrivateRoom)
{
	CTableFrame* pTableFrame = CGameTableManager::get_instance().FindTableByRoomNum(pPrivateRoom->dwRoomNum);

	if (pTableFrame == NULL)
	{
		//检测是否还有这个房间
		CT_BOOL bHasThisPRoom = CheckHasThisPRoom(pPrivateRoom->dwRoomNum);
		if (!bHasThisPRoom)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_NOTFIND);
			return;
		}

		//VIP房间只有房主在的时候才能申请
		 if (pPrivateRoom->cbIsVip && pPrivateRoom->dwUserID != pPrivateRoom->dwOwnerUserID)
		 {
			 SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_VIP_OWNER_OFFLINE);
			 return;
		 }

		pTableFrame = CGameTableManager::get_instance().FindOneUnlockTable();
		if (pTableFrame == NULL)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_TABLE_NOENOUGH);
			return;
		}

		//pTableFrame->SetTableLock(CT_TRUE);
		pTableFrame->SetGameRoundPhase(en_GameRound_Free);
		PrivateTableInfo tableInfo;
		tableInfo.dwOwnerUserID = pPrivateRoom->dwOwnerUserID;
		tableInfo.dwRoomNum = pPrivateRoom->dwRoomNum;
		tableInfo.wCurrPlayCount = 0;
		tableInfo.wTotalPlayCount = pPrivateRoom->wPlayCount;
		tableInfo.wUserCount = pPrivateRoom->wUserCount;
		//tableInfo.wEndLimit = pPrivateRoom->wEndLimit;
		//tableInfo.wWinLimit = pPrivateRoom->wWinLimit;
		tableInfo.cbIsVip = pPrivateRoom->cbIsVip;
		//tableInfo.cbAuto = pPrivateRoom->cbIsAuto;
		tableInfo.dwGroupID = pPrivateRoom->dwGroupID;
		tableInfo.dwRecordID = pPrivateRoom->dwRecordID;
		tableInfo.cbGroupType = pPrivateRoom->cbGroupType;
		_snprintf_info(tableInfo.szOtherParam, sizeof(tableInfo.szOtherParam), "%s", pPrivateRoom->szOtherParam);
		pTableFrame->SetPrivateTableInfo(tableInfo);
	}
	
	//VIP房间非房主进入房间，向房主发送申请
	if (pTableFrame->GetGameRoundPhase() == en_GameRound_Free && pPrivateRoom->cbIsVip && pPrivateRoom->dwUserID != pPrivateRoom->dwOwnerUserID)
	{
		//如果游戏已经开始了
		/*if (pTableFrame->GetGameRoundPhase() != en_GameRound_Free)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_SEAT_FULL);
			return;
		}*/

		CT_BOOL bCanEnter = false;
		CT_BYTE bState = pTableFrame->GetVipPRoomApplyState(pPrivateRoom->dwUserID);
		if (bState == CTableFrame::en_PRoomApply_Undisposed)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_VIP_ALREADY_APPLY);
			return;
		}
		else if (bState == CTableFrame::en_PRoomApply_Unapply)
		{
			//插入申请列表
			GS_UserBaseData userBaseData;
			CT_BOOL bUserBase = GetUserBaseInfo(pPrivateRoom->dwUserID, userBaseData);
			if (bUserBase == false)
			{
				LOG(WARNING) << "can not find user base info in apply vip rpoom, user id : " << pPrivateRoom->dwUserID;
				return;
			}

			CT_BYTE cbAddResult = pTableFrame->AddApplyVipPRoom(pPrivateRoom->dwUserID, userBaseData);
			if (cbAddResult == CTableFrame::en_Add_Succ)
			{
				MSG_SC_ApplyEnterVipPRoomSucc applyVipPRoomSucc;
				applyVipPRoomSucc.dwMainID = MSG_FRAME_MAIN;
				applyVipPRoomSucc.dwSubID = SUB_S2C_APPLY_PROOM_SUCC;
				applyVipPRoomSucc.dwRoomNum = pPrivateRoom->dwRoomNum;
				applyVipPRoomSucc.uValue1 = pPrivateRoom->uValue1;
				SendMsg(pSocket, &applyVipPRoomSucc, sizeof(applyVipPRoomSucc));

				pTableFrame->SendVipPRoomApplyToOwner(pPrivateRoom->dwUserID);
				return;
			}
			else if (cbAddResult == CTableFrame::en_Add_Max)
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_VIP_FULL_APPLY);
				return;
			}
		}
		else if (bState == CTableFrame::en_PRoomApply_Agree)
		{
			bCanEnter = true;
		}

		if (!bCanEnter)
		{
			return;
		}
	}

	CServerUserItem* pServerUserItem = CServerUserManager::get_instance().FindUserItem(pPrivateRoom->dwUserID);
	if (pServerUserItem != NULL)
	{
		GS_UserBaseData userBaseData;
		CT_BOOL bUserBase = GetUserBaseInfo(pPrivateRoom->dwUserID, userBaseData);
		if (bUserBase == false)
		{
			LOG(WARNING) << "can not find user base info, user id : " << pPrivateRoom->dwUserID;
			return;
		}
		userBaseData.dLongitude = pPrivateRoom->dLongitude;
		userBaseData.dLatitude = pPrivateRoom->dLatitude;
		_snprintf_info(userBaseData.szLocation, sizeof(userBaseData.szLocation), "%s", pPrivateRoom->szLocation);
		pServerUserItem->SetUserBaseData(userBaseData);
		SwitchUserItem(pServerUserItem, pSocket, pPrivateRoom->uValue1);
		return;
	}

	//如果游戏已经开始，则不能再进入
	if (pTableFrame->GetGameRoundPhase() != en_GameRound_Free && m_GamePlayKind.wGameID != GAME_NN)
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_GAME_START_ALREADY);
		return;
	}

	//获取私人房信息
	PrivateTableInfo& privateTableInfo = pTableFrame->GetPrivateTableInfo();

	//牛牛，检查是否配置了高级选项：0:游戏开始后可以加入 1：游戏开始后禁止加入 
	if (m_GamePlayKind.wGameID == GAME_NN)
	{
		CT_WORD wEnterLimit = 0;
		acl::json jsonCond(privateTableInfo.szOtherParam);
		const acl::json_node* nodeEnterLimit = jsonCond.getFirstElementByTagName("enterLimit");
		if (nodeEnterLimit != NULL)
		{
			wEnterLimit = (CT_WORD)*nodeEnterLimit->get_int64();
		}
		if (wEnterLimit == 1)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_GAME_START_ALREADY);
			return;
		}
	}
	
	// 判断桌子上是否已经坐满人
	CT_WORD wPlayerCount = pTableFrame->GetPlayerCount();
	if (privateTableInfo.dwGroupID != 0)
	{
		if (wPlayerCount >= (privateTableInfo.wUserCount))
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_SEAT_FULL);
			return;
		}

		//判断是否需要群成员才能加入
		CT_BOOL bCanEnter = false;
		if (privateTableInfo.cbGroupType == GX_GROUP_ROOM)
		{
			bCanEnter = CGSGroupMgr::get_instance().CheckCanEnterPRoom(privateTableInfo.dwGroupID, pPrivateRoom->dwUserID);
		}
		else
		{
			bCanEnter = m_wxGroupMgr.CheckCanEnterPRoom(privateTableInfo.dwGroupID, pPrivateRoom->dwUserID);
		}

		if (!bCanEnter)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_NOT_IN_GROUP);
			return;
		}
	}
	else
	{
		CServerUserItem* pOwnerUserItem = pTableFrame->GetTableUserItemByUserID(pPrivateRoom->dwOwnerUserID);
		if (pOwnerUserItem == NULL)
		{
			if (pPrivateRoom->dwOwnerUserID != pPrivateRoom->dwUserID)
			{
				if (wPlayerCount >= (privateTableInfo.wUserCount - 1))
				{
					SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_SEAT_FULL);
					return;
				}
			}
		}
		else
		{
			if (wPlayerCount >= (privateTableInfo.wUserCount))
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_SEAT_FULL);
				return;
			}
		}
	}

	//创建玩家对象
	CServerUserManager::get_instance().InsertUserItem(&pServerUserItem, pPrivateRoom->dwUserID);
	if (pServerUserItem == NULL)
	{
		LOG(ERROR) << "get user item fail!";
		return;
	}

	GS_UserBaseData userBaseData;
	CT_BOOL bUserBase = GetUserBaseInfo(pPrivateRoom->dwUserID, userBaseData);
	if (bUserBase == false)
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_USERNOEXIST);
		return;
	}
	userBaseData.dLongitude = pPrivateRoom->dLongitude;
	userBaseData.dLatitude = pPrivateRoom->dLatitude;
	_snprintf_info(userBaseData.szLocation, sizeof(userBaseData.szLocation), "%s", pPrivateRoom->szLocation);
	pServerUserItem->SetUserBaseData(userBaseData);
	
	GS_UserPRoomData userPRoomData;
	memset(&userPRoomData, 0, sizeof(userPRoomData));
	/*CT_BOOL bUserPRoom = */GetUserPRoomInfo(pPrivateRoom->dwUserID, userPRoomData);
	//if (bUserPRoom == false)
	//{
	//	LOG(WARNING) << "get user proom info fail";
	//}
	pServerUserItem->SetUserPRoomData(userPRoomData);
	pServerUserItem->SetClientNetAddr(pPrivateRoom->uValue1);
	pServerUserItem->SetUserCon(pSocket);
	pTableFrame->PrivateRoomSitChair(pServerUserItem);

	//将桌子锁定
	if (pTableFrame->GetTableisLock() == false)
	{
		pTableFrame->SetTableLock(true);
	}
}

CT_VOID CGameServerThread::OnUserEnterGroupPRoom(acl::aio_socket_stream* pSocket, MSG_PG_EnterGroupPRoom* pPrivateRoom)
{
    CServerUserItem* pServerUserItem = CServerUserManager::get_instance().FindUserItem(pPrivateRoom->dwUserID);
    if (pServerUserItem != NULL)
    {
        GS_UserBaseData userBaseData;
        CT_BOOL bUserBase = GetUserBaseInfo(pPrivateRoom->dwUserID, userBaseData);
        if (bUserBase == false)
        {
            LOG(WARNING) << "can not find user base info, user id : " << pPrivateRoom->dwUserID;
            SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pPrivateRoom->uValue1, ENTER_ROOM_USERNOEXIST);
            return;
        }

        pServerUserItem->SetUserBaseData(userBaseData);
        SwitchUserItem(pServerUserItem, pSocket, pPrivateRoom->uValue1);
        return;
    }

    //首先检测玩家是否合法
    bool bCanEnter = CGSGroupMgr::get_instance().CheckCanEnterGroupPRoom(pPrivateRoom->dwUserID, pPrivateRoom->dwRoomNum);
    if (!bCanEnter)
    {
        SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_NOT_IN_GROUP);
        return;
    }

    //检查是否在其他房间
    bCanEnter = CheckUserCanEnterRoom(pPrivateRoom->dwUserID, pSocket, pPrivateRoom->uValue1);
    if (!bCanEnter)
    {
        LOG(WARNING) << "can not enter group private room, in other room, user id: " << pPrivateRoom->dwUserID;
        return;
    }

    CTableFrameForScore* pTableFrame = CGameTableManager::get_instance().FindTableByGroupRoomNum(pPrivateRoom->dwRoomNum);
    if (pTableFrame == NULL)
    {
        pTableFrame = CGameTableManager::get_instance().FindOneUnlockScoreTable();
        if (pTableFrame == NULL)
        {
            SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_PROOM_FAIL, pPrivateRoom->uValue1, ENTER_PROOM_TABLE_NOENOUGH);
            return;
        }

        pTableFrame->SetGameRoundPhase(en_GameRound_Free);
        PrivateTableInfo tableInfo;
        tableInfo.dwRoomNum = pPrivateRoom->dwRoomNum;
        pTableFrame->SetPrivateTableInfo(tableInfo);
    }

    GS_UserBaseData userBaseData;
    //pSocket为空是机器人
    if (pSocket != NULL)
    {
        CT_BOOL bUserBase = GetUserBaseInfo(pPrivateRoom->dwUserID, userBaseData);
        if (bUserBase == false)
        {
            SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pPrivateRoom->uValue1, ENTER_ROOM_USERNOEXIST);
            return;
        }
    }

    //判断金币是否达到要求
    if (m_GamePlayKind.wGameID != GAME_HBSL)
    {
        if (userBaseData.llScore < pTableFrame->GetUserEnterMinScore())
        {
            SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pPrivateRoom->uValue1, ENTER_ROOM_SCORE_NOT_ENOUGH);
            return;
        }

        if (pTableFrame->GetUserEnterMaxScore() > 0)
        {
            if (userBaseData.llScore > pTableFrame->GetUserEnterMaxScore())
            {
                SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pPrivateRoom->uValue1, ENTER_ROOM_SCORE_LIMIT);
                return;
            }
        }
    }

    //如果游戏已经开始，则不能再进入
    if (pTableFrame->GetGameRoundPhase() != en_GameRound_Free && m_GamePlayKind.wGameID != GAME_ZJH && m_GamePlayKind.wGameID != GAME_NN)
    {
        SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pPrivateRoom->uValue1, ENTER_ROOM_GAME_START_ALREADY);
        return;
    }

    // 判断桌子上是否已经坐满人
    if (pTableFrame->GetPlayerCount() >= pTableFrame->GetMaxPlayerCount())
    {
        SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pPrivateRoom->uValue1, ENTER_ROOM_SEAT_FULL);
        return;
    }

    //创建玩家对象
    if (pSocket != NULL)
    {
        CServerUserManager::get_instance().InsertUserItem(&pServerUserItem, pPrivateRoom->dwUserID);
    }

    if (pServerUserItem == NULL)
    {
        LOG(ERROR) << "get user item fail!";
        return;
    }

    pServerUserItem->SetUserBaseData(userBaseData);
    GS_UserScoreData userScoreData;
    userScoreData.dwUserID = pPrivateRoom->dwUserID;
    if (m_GamePlayKind.wGameID == GAME_ZJH)
    {
        userScoreData.dwChangeTableCount = 99999999; //保证私人房不换桌
    }
    userScoreData.dwEnterTime = Utility::GetTime();
    pServerUserItem->SetUserScoreData(userScoreData);

    pServerUserItem->SetClientNetAddr(pPrivateRoom->uValue1);
    pServerUserItem->SetUserCon(pSocket);
    pTableFrame->RoomSitChair(pServerUserItem, false);

    //将桌子锁定
    if (pTableFrame->GetTableisLock() == false)
    {
        pTableFrame->SetTableLock(true);
    }

    //通知中心服务器更新玩家列表
    UpdateGroupPRoomUserInfo(pServerUserItem, pPrivateRoom->dwRoomNum, true);
}

CT_VOID CGameServerThread::OnUserEnterRoom(acl::aio_socket_stream * pSocket, MSG_PG_EnterRoom * pRoom, CT_BOOL bDistribute/* = false*/)
{
	CServerUserItem* pServerUserItem = CServerUserManager::get_instance().FindUserItem(pRoom->dwUserID);
	if (pServerUserItem != NULL)
	{
		GS_UserBaseData userBaseData;
		CT_BOOL bUserBase = GetUserBaseInfo(pRoom->dwUserID, userBaseData);
		if (bUserBase == false)
		{
			LOG(WARNING) << "can not find user base info, user id : " << pRoom->dwUserID;
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_USERNOEXIST);
			return;
		}
		//userBaseData.cbHeadIndex = pServerUserItem->GetHeadIndex();
		pServerUserItem->SetUserBaseData(userBaseData);
		SwitchUserItem(pServerUserItem, pSocket, pRoom->uValue1);
		return;
	}

	//检查是否在其他房间
	CT_BOOL bCanEnter = CheckUserCanEnterRoom(pRoom->dwUserID, pSocket, pRoom->uValue1);
	if (!bCanEnter)
	{
		LOG(WARNING) << "can not enter room, in other room, user id: " << pRoom->dwUserID;
		return;
	}

	//斗地主和跑得快不能自住进入房间，只能走分配模式
	if ((m_GamePlayKind.wGameID == GAME_DDZ || m_GamePlayKind.wGameID == GAME_PDK) && !bDistribute)
    {
        SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_GAME_OVER);
        return;
    }

	CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(CGameTableManager::get_instance().FindNormalSuitTable());
	if (pTableFrame == NULL)
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_TABLE_NOENOUGH);
		return;
	}

	GS_UserBaseData userBaseData;
	//pSocket为空是机器人
	if (pSocket != NULL)
	{
		CT_BOOL bUserBase = GetUserBaseInfo(pRoom->dwUserID, userBaseData);
		if (bUserBase == false)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_USERNOEXIST);
			return;
		}
	}
	else
	{
		GS_UserBaseData* pBaseData = CAndroidUserMgr::get_instance().GetAndroidUserBaseData(pRoom->dwUserID);
		if (pBaseData == NULL)
		{
			LOG(ERROR) << "enter room get android user base data fail. user id: " << pRoom->dwUserID << ", game id: " << \
				m_GamePlayKind.wGameID << "kind id: " << m_GamePlayKind.wKindID << ", room kind id: " << m_GamePlayKind.wRoomKindID;
			return;
		}
		memcpy(&userBaseData, pBaseData, sizeof(userBaseData));
	}

	//判断金币是否达到要求
	if (m_GamePlayKind.wGameID != GAME_HBSL)
	{
		if (userBaseData.llScore < pTableFrame->GetUserEnterMinScore())
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_SCORE_NOT_ENOUGH);
			return;
		}

		if (pTableFrame->GetUserEnterMaxScore() > 0)
		{
			if (userBaseData.llScore > pTableFrame->GetUserEnterMaxScore())
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_SCORE_LIMIT);
				return;
			}
		}
	}

	//如果游戏已经开始，则不能再进入
	if (pTableFrame->GetGameRoundPhase() != en_GameRound_Free && m_GamePlayKind.wGameID != GAME_BR && m_GamePlayKind.wGameID != GAME_ZJH && m_GamePlayKind.wGameID != GAME_NN && m_GamePlayKind.wGameID != GAME_FISH)
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_GAME_START_ALREADY);
		return;
	}

	// 判断桌子上是否已经坐满人
	if (pTableFrame->GetPlayerCount() >= pTableFrame->GetMaxPlayerCount())
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_SEAT_FULL);
		return;
	}

	//创建玩家对象
	if (pSocket != NULL)
	{
		CServerUserManager::get_instance().InsertUserItem(&pServerUserItem, pRoom->dwUserID);
	}
	else
	{
		CAndroidUserMgr::get_instance().CreateAndroidUserItem(&pServerUserItem, pRoom->dwUserID, 0);
	}

	if (pServerUserItem == NULL)
	{
		LOG(ERROR) << "get user item fail!";
		return;
	}
	

	//if (m_GamePlayKind.wGameID == GAME_DDZ)
	//{
	//	userBaseData.cbHeadIndex = userBaseData.cbHeadIndex % 4 + 1;
	//}

	pServerUserItem->SetUserBaseData(userBaseData);
	GS_UserScoreData userScoreData;
	userScoreData.dwUserID = pRoom->dwUserID;
	if (m_GamePlayKind.wGameID == GAME_ZJH)
	{
		userScoreData.dwChangeTableCount = rand() % (5 + 1) + 5;
	}
	userScoreData.dwEnterTime = Utility::GetTime();
	pServerUserItem->SetUserScoreData(userScoreData);

	pServerUserItem->SetClientNetAddr(pRoom->uValue1);
	pServerUserItem->SetUserCon(pSocket);
	pTableFrame->RoomSitChair(pServerUserItem, bDistribute);
}

CT_VOID CGameServerThread::OnUserEnterRoomEx(acl::aio_socket_stream* pSocket, MSG_PG_EnterRoom* pRoom)
{
	//TODO:
	CServerUserItem* pServerUserItem = CServerUserManager::get_instance().FindUserItem(pRoom->dwUserID);
	if (pServerUserItem != NULL)
	{
		GS_UserBaseData userBaseData;
		CT_BOOL bUserBase = GetUserBaseInfo(pRoom->dwUserID, userBaseData);
		if (bUserBase == false)
		{
			LOG(WARNING) << "can not find user base info, user id : " << pRoom->dwUserID;
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_USERNOEXIST);
			return;
		}
		//userBaseData.cbHeadIndex = pServerUserItem->GetHeadIndex();
		pServerUserItem->SetUserBaseData(userBaseData);
		SwitchUserItem(pServerUserItem, pSocket, pRoom->uValue1);
		return;
	}

	//检查是否在其他房间
	CT_BOOL bCanEnter = CheckUserCanEnterRoom(pRoom->dwUserID, pSocket, pRoom->uValue1);
	if (!bCanEnter)
	{
		LOG(WARNING) << "can not enter room ex, in other room, user id: " << pRoom->dwUserID;
		return;
	}

	GS_UserBaseData userBaseData;
	CT_BOOL bUserBase = GetUserBaseInfo(pRoom->dwUserID, userBaseData);
	if (bUserBase == false)
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_USERNOEXIST);
		return;
	}

	//判断金币是否达到要求
	if (userBaseData.llScore < m_GamePlayKind.dwEnterMinScore)
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_SCORE_NOT_ENOUGH);
		return;
	}

	if (m_GamePlayKind.dwEnterMaxScore > 0)
	{
		if (userBaseData.llScore > m_GamePlayKind.dwEnterMaxScore)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_SCORE_LIMIT);
			return;
		}
	}

	MSG_SC_EnterRoomExSucc enterRoomSucc;
	enterRoomSucc.dwMainID = MSG_FRAME_MAIN;
	enterRoomSucc.dwSubID = SUB_S2C_ENTER_ROOM_EX_SUCC;
	enterRoomSucc.uValue1 = pRoom->uValue1;
	enterRoomSucc.wGameID = m_GamePlayKind.wGameID;
	enterRoomSucc.wKindID = m_GamePlayKind.wKindID;
	enterRoomSucc.dwRoomNum = 0;
	//enterRoomSucc.cbHeadIndex = userBaseData.cbHeadIndex % 4 + 1;
	enterRoomSucc.wRoomKindID = m_GamePlayKind.wRoomKindID;
	enterRoomSucc.dCellScore = m_GamePlayKind.dwCellScore*0.01f;

	SendMsg(pSocket, &enterRoomSucc, sizeof(MSG_SC_EnterRoomExSucc));
}

CT_VOID CGameServerThread::OnAddUserWaitList(acl::aio_socket_stream* pSocket, MSG_PG_EnterRoom* pRoom)
{
	CServerUserItem* pServerUserItem = CServerUserManager::get_instance().FindUserItem(pRoom->dwUserID);
	if (pServerUserItem != NULL)
	{
		GS_UserBaseData userBaseData;
		CT_BOOL bUserBase = GetUserBaseInfo(pRoom->dwUserID, userBaseData);
		if (bUserBase == false)
		{
			LOG(WARNING) << "can not find user base info, user id : " << pRoom->dwUserID;
			return;
		}
		//userBaseData.cbHeadIndex = pServerUserItem->GetHeadIndex();
		pServerUserItem->SetUserBaseData(userBaseData);
		SwitchUserItem(pServerUserItem, pSocket, pRoom->uValue1);
		return;
	}

	//检查是否在其他房间
	CT_BOOL bCanEnter = CheckUserCanEnterRoom(pRoom->dwUserID, pSocket, pRoom->uValue1);
	if (!bCanEnter)
	{
		LOG(WARNING) << "add user wait list fail, in other room, user id: " << pRoom->dwUserID;
		return;
	}

	/*CServerUserItem* pServerUserItem = CServerUserManager::get_instance().FindUserItem(pRoom->dwUserID);
	if (pServerUserItem != NULL)
	{
		SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_USER_IN_GAME);
		return;
	}*/

	GS_UserBaseData userBaseData;
	if (pSocket != NULL)
	{	
		CT_BOOL bUserBase = GetUserBaseInfo(pRoom->dwUserID, userBaseData);
		if (bUserBase == false)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_USERNOEXIST);
			return;
		}

		//判断金币是否达到要求
		if (userBaseData.llScore < m_GamePlayKind.dwEnterMinScore)
		{
			SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_SCORE_NOT_ENOUGH);
			return;
		}

		if (m_GamePlayKind.dwEnterMaxScore > 0)
		{
			if (userBaseData.llScore > m_GamePlayKind.dwEnterMaxScore)
			{
				SendGameErrorCode(pSocket, MSG_FRAME_MAIN, SUB_S2C_ENTER_ROOM_FAIL, pRoom->uValue1, ENTER_ROOM_SCORE_LIMIT);
				return;
			}
		}
	}

	//增加到游戏列表
	AddWaitList(pRoom->dwUserID, pRoom->uValue1, pSocket);

	//proxy注册玩家
	if (pSocket)
	{
		AddGameUserToProxy(pRoom->dwUserID, pRoom->uValue1, pSocket);
		MSG_SC_AddWaitList addWaitList;
		addWaitList.dwMainID = MSG_FRAME_MAIN;
		addWaitList.dwSubID = SUB_S2C_ADD_WAIT_LIST;
		addWaitList.uValue1 = pRoom->uValue1;
		addWaitList.cbResult = 0;
		addWaitList.cbHeadIndex = userBaseData.cbHeadIndex;

		SendMsg(pSocket, &addWaitList, sizeof(MSG_SC_AddWaitList));
	}

	if (m_GamePlayKind.wGameID != GAME_JZNC)
    {
        //检测是否可以开始游戏
        DistributeTable();

        //初级房马上匹配机器人陪打
        if (pSocket && (m_GamePlayKind.wRoomKindID == PRIMARY_ROOM || m_GamePlayKind.wRoomKindID == FREE_ROOM))
        {
            //马上匹配机器人
            CAndroidUserMgr::get_instance().OnTimeCheckUserIn();
        }
    }
}

CT_VOID CGameServerThread::OnUserChangeTable(CT_DWORD dwUserID, CT_BOOL bForce)
{
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem != NULL)
	{
		if (pUserItem->GetUserStatus() == sPlaying)
		{
			LOG(WARNING) << "user change table fail. user status: " << (int)pUserItem->GetUserStatus();
			return;
		}

		CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(CGameTableManager::get_instance().GetTable(pUserItem->GetTableID()));
		if (pTableFrame)
		{
		    if (pTableFrame->GetTableisLock())
                return;

		    CT_LONGLONG llUserScore = pUserItem->GetUserScore();
		    //判断玩家是否还符合在场条件
            if (llUserScore < pTableFrame->GetUserEnterMinScore())
            {
                MSG_SC_GameErrorCode gameErrorCode;
                gameErrorCode.dwMainID = MSG_FRAME_MAIN;
                gameErrorCode.dwSubID = SUB_S2C_ENTER_ROOM_FAIL;
                gameErrorCode.iErrorCode = ENTER_ROOM_SCORE_NOT_ENOUGH;
                gameErrorCode.uValue1 = pUserItem->GetClientNetAddr();
                gameErrorCode.dwValue2 = 0;
                acl::aio_socket_stream* pUserProxySock =  pUserItem->GetUserCon();
                if (pUserProxySock)
                {
                    SendMsg(pUserProxySock, &gameErrorCode, sizeof(gameErrorCode));
                }
                pTableFrame->ClearTableUser(pUserItem->GetChairID(), true, true);
                return;
            }

			CTableFrameForScore* pNewTableFrame = static_cast<CTableFrameForScore*>(CGameTableManager::get_instance().FindNormalSuitTable(pTableFrame->GetTableID()));
			if (pNewTableFrame)
			{
				//原来的桌子起立
				/*if (pTableFrame->IsGameStart())
				{
					LOG(WARNING) << "user change table, user status: " << (int)pUserItem->GetUserStatus();
				}

				LOG(ERROR) << "user change table, old table id: " << pTableFrame->GetTableID() << ", new table id: " << pNewTableFrame->GetTableID();*/
				
				//牛牛和炸金花换桌，要考虑把以前的状态清空
				ITableFrameSink* pTableFrameSink = pTableFrame->GetTableFrameSink();
				if (pTableFrameSink)
				{
					pTableFrameSink->OnUserChangeTable(pUserItem->GetChairID(), false);
				}
				
				CT_BOOL bStandUp = pTableFrame->UserStandUp(pUserItem, true, false);
				if (!bStandUp)
				{
					LOG(WARNING) << "user stand up fail.";
				}

				//给玩家发送换桌消息（前端自己清空桌子，服务器离桌不通知玩家）
				MSG_GameMsgDownHead headMsg;
				headMsg.dwMainID = MSG_FRAME_MAIN;
				headMsg.dwSubID = SUB_S2C_CHANGE_TABLE;
				headMsg.uValue1 = pUserItem->GetClientNetAddr();
				pUserItem->SendUserMessage(MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, (CT_CHAR*)&headMsg, sizeof(headMsg));

				//如果没有玩家在线，则释放这个桌子
				if (pTableFrame->GetPlayerCount() == 0)
				{
				    /*if (pTableFrame->GetTableisLock())
                    {
                        RemoveGroupPRoom(pTableFrame->GetRoomNum());
                    }*/
					CGameTableManager::get_instance().FreeNomalTable(pTableFrame);
				}

				//让玩家坐到新桌子上
				pNewTableFrame->RoomSitChair(pUserItem, true);

				//炸金花强迫换桌回调成功
				if(m_GamePlayKind.wGameID == GAME_ZJH && bForce)
				{
					ITableFrameSink* pTableFrameSink = pNewTableFrame->GetTableFrameSink();
					if (pTableFrameSink)
					{
						pTableFrameSink->OnUserChangeTable(pUserItem->GetChairID(), true);
					}
				}
			}
		}
	}
}

CT_VOID CGameServerThread::OnUserReportUser(MSG_CS_Report_User* pReportUser)
{
	CT_DWORD  dwUserID = pReportUser->dwUserID;
	CServerUserItem* pUserItem = CServerUserManager::get_instance().FindUserItem(dwUserID);
	if (pUserItem != NULL)
	{
		CTableFrameForScore *pTableFrame =
		static_cast<CTableFrameForScore *>(CGameTableManager::get_instance().GetTable(pUserItem->GetTableID()));
		if (pTableFrame)
		{
			pTableFrame->InsertUserReportInfo(pUserItem, dwUserID, pReportUser->dwBeReportUserID);
		}
	}
	
}

CT_VOID CGameServerThread::UpdateGroupPRoomUserInfo(CServerUserItem* pUserItem, CT_DWORD dwRoomNum,  bool bEnter)
{
    if (pUserItem == NULL)
        return;

    MSG_G2CS_GroupPRoom_Update groupPRoomUpdate;
    groupPRoomUpdate.dwRoomNum= dwRoomNum;
    groupPRoomUpdate.wChairID = pUserItem->GetChairID();
    groupPRoomUpdate.wGameID = m_GamePlayKind.wGameID;
    groupPRoomUpdate.wKindID = m_GamePlayKind.wKindID;

    if (bEnter)
    {
        groupPRoomUpdate.dwUserID = pUserItem->GetUserID();
        groupPRoomUpdate.cbHeadId = pUserItem->GetHeadIndex();
        groupPRoomUpdate.cbSex = pUserItem->GetGender();
        groupPRoomUpdate.cbVip2 = pUserItem->GetUserVip2();
    }

    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_UPDATE_GROUP_PROOM_USER, &groupPRoomUpdate, sizeof(MSG_G2CS_GroupPRoom_Update));
}

CT_VOID CGameServerThread::RemoveGroupPRoom(CT_DWORD dwRoomNum)
{
    MSG_G2CS_Remove_GroupPRoom removeGroupPRoom;
    removeGroupPRoom.dwRoomNum = dwRoomNum;
    removeGroupPRoom.wGameID = m_GamePlayKind.wGameID;
    removeGroupPRoom.wKindID = m_GamePlayKind.wKindID;
    CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_REMOVE_GROUP_PROOM, &removeGroupPRoom, sizeof(MSG_G2CS_Remove_GroupPRoom));

    CGSGroupMgr::get_instance().RemoveGroupPRoom(dwRoomNum);
}

CT_VOID CGameServerThread::SendMsg(acl::aio_socket_stream* pSocket, const CT_VOID* pBuf, CT_DWORD dwLen)
{
	if (NULL != pSocket)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, pBuf, dwLen);
	}
}

CT_VOID CGameServerThread::CloseUserSocket(acl::aio_socket_stream* pSocket, CT_UINT64 uClientSocket)
{
	if (NULL != pSocket)
	{
		Proxy_CloseUserSocket usersocket;
		memset(&usersocket, 0, sizeof(Proxy_CloseUserSocket));
		usersocket.uUserSocketAddr = uClientSocket;;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_CLOSEUSERSOCKET, &usersocket, sizeof(usersocket));
	}
}

CT_VOID CGameServerThread::SendSitFail(acl::aio_socket_stream* pSocket, CT_DWORD dwClientValue, CT_INT32 iErrorCode)
{

}

CT_VOID CGameServerThread::SendGameErrorCode(acl::aio_socket_stream* pSocket, CT_WORD wMainID, CT_WORD wSubID, CT_UINT64 uClientValue, CT_INT32 iErrorCode)
{
	if (pSocket)
	{
		MSG_SC_GameErrorCode gameErrorCode;
		gameErrorCode.dwMainID = wMainID;
		gameErrorCode.dwSubID = wSubID;
		gameErrorCode.iErrorCode = iErrorCode;
		gameErrorCode.uValue1 = uClientValue;
		SendMsg(pSocket, &gameErrorCode, sizeof(gameErrorCode));
	}
}

void CGameServerThread::AddGameUserToProxy(CT_DWORD dwUserID, CT_UINT64 uClientAddr, acl::aio_socket_stream* pProxySock)
{
	MSG_GS2P_AddGameUserInfo addUserToPS;
	addUserToPS.dwUserID = dwUserID;
	addUserToPS.dwServerID = CServerCfg::m_nServerID;
	addUserToPS.uClientNetAddr = uClientAddr;
	CNetModule::getSingleton().Send(pProxySock, MSG_PGS_MAIN, SUB_GS2P_ADD_USER, &addUserToPS, sizeof(addUserToPS));
}

void CGameServerThread::DelGameUserToProxy(CT_DWORD dwUserID, acl::aio_socket_stream* pProxySock)
{
	MSG_GS2P_DelGameUser delUserToPS;
	delUserToPS.dwUserID = dwUserID;
	CNetModule::getSingleton().Send(pProxySock, MSG_PGS_MAIN, SUB_GS2P_DEL_USER, &delUserToPS, sizeof(delUserToPS));
}

CT_BOOL CGameServerThread::GetUserBaseInfo(CT_DWORD dwUserID, GS_UserBaseData& userBaseData)
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

	userBaseData.dwUserID = dwUserID;
	//userBaseData.dwPlatformID = atoi(result["pfid"]);
	//userBaseData.dwGem = atoi(result["gem"]);
	//userBaseData.llScore = m_GamePlayKind.wRoomKindID == PRIVATE_ROOM ? 0 : atoll(result["score"]);
	userBaseData.dwRechargeTempCount = atoi(result["rechargecount"]);
	userBaseData.llScore = ((m_GamePlayKind.wGameID == GAME_FISH && m_GamePlayKind.wRoomKindID == FREE_ROOM) ? 1000000 : atoll(result["score"]));
	userBaseData.llBankScore = atoll(result["bankscore"]);
	userBaseData.cbGender = (CT_BYTE)atoi(result["gender"]);
	userBaseData.cbVipLevel = (CT_BYTE)atoi(result["vip"]);
	userBaseData.cbVipLevel2 = (CT_BYTE)atoi(result["vip2"]);
	userBaseData.cbHeadIndex = (CT_BYTE)atoi(result["headid"]);
	userBaseData.dwPlatformID = (CT_DWORD)atoi(result["pid"]);
	userBaseData.dwchannelID = (CT_DWORD)atoi(result["channelid"]);
	userBaseData.cbBenefitCount = (CT_BYTE)atoi(result["almsc"]);
	userBaseData.dwBenefitLastTime = (CT_DWORD)atoi(result["almstime"]);
	acl::string nickName = result["nick"];
	acl::string headUrl = result["head"];
	acl::string ip = result["ip"];
	acl::string szLocation = result["location"];
	_snprintf_info(userBaseData.szNickName, sizeof(userBaseData.szNickName), "%s", nickName.c_str());
	//_snprintf_info(userBaseData.szHeadUrl, sizeof(userBaseData.szHeadUrl), "%s", headUrl.c_str());
	_snprintf_info(userBaseData.szIP, sizeof(userBaseData.szIP), "%s", ip.c_str());
	_snprintf_info(userBaseData.szLocation, sizeof(userBaseData.szLocation), "%s", szLocation.c_str());

	if (m_GamePlayKind.wGameID == GAME_DDZ && m_GamePlayKind.wRoomKindID == PRIMARY_ROOM)
    {
		userBaseData.dwTodayPlayCount = (CT_DWORD)atoi(result["50101_c"]);
		userBaseData.llTotalWinScore = atoll(result["50101_s"]);
    }
	else if (m_GamePlayKind.wGameID == GAME_PDK && m_GamePlayKind.wRoomKindID == FREE_ROOM)
	{
		userBaseData.dwTodayPlayCount = (CT_DWORD)atoi(result["20104_c"]);
	}

	//如果是牛牛或者炸金花拉取好牌数据
	/*if (m_GamePlayKind.wGameID == GAME_ZJH)
	{
		userBaseData.dwZjhCount = (CT_DWORD)atoi(result["zjhcount"]);
		userBaseData.cbZjhParam = (CT_BYTE)atoi(result["zjhparam"]);
		//userBaseData.dwGoodCardTime = (CT_DWORD)atoi(result["goodcardtime"]);
	}
	else if (m_GamePlayKind.wGameID == GAME_NN)
	{
		userBaseData.dwQznnCount = (CT_DWORD)atoi(result["qznncount"]);
		userBaseData.cbQznnParam = (CT_BYTE)atoi(result["qznnparam"]);
		//userBaseData.dwGoodCardTime = (CT_DWORD)atoi(result["goodcardtime"]);
	}*/

	return true;
}

CT_BOOL CGameServerThread::GetUserPRoomInfo(CT_DWORD dwUserID, GS_UserPRoomData& userPRoomData)
{
	acl::string key;
	acl::string result;
	key.format("gameround_%u", dwUserID);

	acl::string att;
	att.format("%u:%u", m_GamePlayKind.wGameID, m_GamePlayKind.wKindID);
	
	m_redis.clear();
	if (m_redis.hget(key.c_str(), att.c_str(), result) == false)
	{
		return false;
	}

	if (result.empty())
	{
		return false;
	}

	userPRoomData.dwGameRoundCount = (CT_DWORD)atoi(result.c_str());
	return true;
}

CT_VOID CGameServerThread::DelPRoomInfo(CT_DWORD dwRoomNum)
{
	acl::string key;
	key.format("proom_info_%u", dwRoomNum);

	m_redis.clear();
	if (m_redis.del_one(key) <= 0)
	{
		LOG(WARNING) << "delete proom info fail, proom num:" << dwRoomNum;
		return;
	}
	return;
}

CT_VOID CGameServerThread::AddUserGem(CT_DWORD dwUserID, int iAddGem)
{
	acl::string key;
	key.format("account_%u", dwUserID);

	m_redis.clear();
	if (m_redis.exists(key) == false)
	{
		return;
	}

	long long dwGemResult = 0;
	m_redis.clear();
	if (m_redis.hincrby(key, "gem", iAddGem, &dwGemResult) == false)
	{
		LOG(WARNING) << "return user gem fail, user id: " << dwUserID << ", add gem: " << iAddGem;
		return ;
	}
}

CT_BOOL CGameServerThread::CheckHasThisPRoom(CT_DWORD dwRoomNum)
{
	acl::string key;
	key.format("proom_info_%u", dwRoomNum);

	m_redis.clear();
	if (m_redis.exists(key) == true)
	{
		return true;
	}
	return false;
}

CT_VOID CGameServerThread::GetUserOnlineInfo(CT_DWORD dwUserID, std::map<acl::string, acl::string>& mapOnlineInfo)
{
	acl::string key;
	key.format("online_%u", dwUserID);

	m_redis.clear();
	if (m_redis.hgetall(key, mapOnlineInfo) == false)
	{
		return;
	}
}

CT_BOOL CGameServerThread::CheckUserCanEnterRoom(CT_DWORD dwUserID, acl::aio_socket_stream* pSocket, CT_UINT64 uClientValue)
{
	//获取在线信息
	std::map<acl::string, acl::string> mapOnline;
	GetUserOnlineInfo(dwUserID, mapOnline);
	if (!mapOnline.empty())
	{
		CT_WORD wSourceGameID = atoi(mapOnline["gameid"].c_str());
		CT_WORD wSourceKindID = atoi(mapOnline["kindid"].c_str());
		CT_WORD wSourceRoomKindID = atoi(mapOnline["roomkind"].c_str());
		//CT_DWORD dwRoomNumOrServerID = atoi(mapOnline["roomnum"].c_str());

		if (wSourceGameID != m_GamePlayKind.wGameID || wSourceKindID != m_GamePlayKind.wKindID || wSourceRoomKindID != m_GamePlayKind.wRoomKindID)
		{
			MSG_SC_In_OtherRoom otherRoom;
			otherRoom.wGameID = wSourceGameID;
			otherRoom.wKindID = wSourceKindID;
			otherRoom.wRoomKindID = wSourceRoomKindID;
			otherRoom.dwMainID = MSG_FRAME_MAIN;
			otherRoom.dwSubID = SUB_S2C_IN_OTHER_ROOM;
			otherRoom.uValue1 = uClientValue;
			SendMsg(pSocket, &otherRoom, sizeof(otherRoom));
			return false;
		}
	}

	return true;
}

CT_BOOL CGameServerThread::CheckUserReadyEnterRoom(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("ready_online_%u", dwUserID);
	
	m_redis.clear();
	if (m_redis.setnx(key, "1") == 1)
	{
		return false;
	}
	
	return true;
}

CT_VOID CGameServerThread::ExpireUserReadyEnterRoom(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("ready_online_%u", dwUserID);
	
	m_redis.clear();
	m_redis.expire(key, 5);
}

CT_VOID CGameServerThread::DelUserReadyEnterRoom(CT_DWORD dwUserID)
{
	acl::string key;
	key.format("ready_online_%u", dwUserID);
	
	m_redis.clear();
	if (m_redis.del_one(key.c_str()) > 0)
	{
		return;
	}
	
	LOG(WARNING) << "DelUserReadyEnterRoom FAIL, user id: " << dwUserID;
}

//操作相关
CT_VOID CGameServerThread::AddWaitList(CT_DWORD dwUserID, CT_UINT64 uClientAddr, acl::aio_socket_stream* pProxySock)
{
	for (auto& it : m_listWaitList)
	{
		if (it.dwUserID == dwUserID)
		{
			it.pProxySock = pProxySock;
			it.uClientAddr = uClientAddr;
			return ;
		}
	}

	tagWaitListInfo waitListInfo;
	waitListInfo.dwUserID = dwUserID;
	waitListInfo.uClientAddr = uClientAddr;
	waitListInfo.pProxySock = pProxySock;

	m_listWaitList.push_back(waitListInfo);

	if(GAME_JZNC == m_GamePlayKind.wGameID)
    {
        if((m_listWaitList.size() == 1))
        {
            CNetModule::getSingleton().SetTimer(IDI_TIMER_DISTRIBUTE_TABLE, TIME_DISTRIBUTE_TABLE, this, false);
            CNetModule::getSingleton().SetTimer(IDI_TIMER_ANDROID_IN, TIME_ANDROID_IN_JZNC, this, false);
        }
        if(m_listWaitList.size() >= m_GamePlayKind.wStartMaxPlayer)
        {
            CNetModule::getSingleton().KillTimer(IDI_TIMER_DISTRIBUTE_TABLE);
            CNetModule::getSingleton().KillTimer(IDI_TIMER_ANDROID_IN);
            DistributeTable();
        }
    }
    //UpdateServerIDWaitListCount((CT_DWORD)m_listWaitList.size());
}

CT_VOID CGameServerThread::UpdateServerIDWaitListCount(const CT_DWORD &dwWaitListCount)
{
    acl::string key;
    key.format("waitlist_%u", CServerCfg::m_nServerID);

    acl::string att;
    att.format("%u:%u", m_GamePlayKind.wGameID, m_GamePlayKind.wRoomKindID);

    acl::string val;
    val.format("%u", dwWaitListCount);

    m_redis.clear();
    if (m_redis.hset(key, att.c_str(), val.c_str()) == -1)
    {
        LOG(WARNING) << "update redis game wait list count fail, server id: " << CServerCfg::m_nServerID;
        return;
    }
}

CT_VOID CGameServerThread::DistributeTable()
{
	//LOG(WARNING) << "wait list:" << m_listWaitList.size();
	if (m_listWaitList.size() < m_GamePlayKind.wStartMinPlayer)
	{
		/*if (m_listWaitList.size() > 0 && m_GamePlayKind.wGameID == GAME_DDZ)
		{
			m_dwWaitDistributeTime += TIME_DISTRIBUTE_TABLE;
			if (m_dwWaitDistributeTime >= 5)
			{
				MSG_SC_MatchTabel_NeedAndroid needAndroid;
				needAndroid.dwMainID = MSG_FRAME_MAIN;
				needAndroid.dwSubID = SUB_S2C_MATCH_TABLE_NEED_ANDROID;
				needAndroid.wGameID = m_GamePlayKind.wGameID;
				needAndroid.wKindID = m_GamePlayKind.wKindID;
				needAndroid.wRoomKindID = m_GamePlayKind.wRoomKindID;
				CServerMgr::get_instance().BroadcastMsgToProxy(&needAndroid, sizeof(needAndroid));
			}
		}*/
		//UpdateServerIDWaitListCount((CT_DWORD)m_listWaitList.size());

		if(GAME_JZNC == m_GamePlayKind.wGameID)
        {
            //人数不够开始不了游戏
            CNetModule::getSingleton().SetTimer(IDI_TIMER_DISTRIBUTE_TABLE, 1000, this, false);
        }
		return;
	}

	//m_dwWaitDistributeTime = 0;
    unsigned int seed = std::chrono::system_clock::now ().time_since_epoch ().count ();
    std::shuffle (m_listWaitList.begin (), m_listWaitList.end (), std::default_random_engine (seed));
    //std::random_shuffle(m_listWaitList.begin(), m_listWaitList.end());
	auto nDistributeCount = m_listWaitList.size() - m_listWaitList.size() % m_GamePlayKind.wStartMaxPlayer;
	for (auto it = m_listWaitList.begin(); it != m_listWaitList.end(); )
	{
		//
		MSG_GameMsgDownHead matchSucc;
		matchSucc.dwMainID = MSG_FRAME_MAIN;
		matchSucc.dwSubID = SUB_S2C_MATCH_TABLE_SUCC;
		matchSucc.uValue1 = it->uClientAddr;
		SendMsg(it->pProxySock, &matchSucc, sizeof(matchSucc));

		static MSG_PG_EnterRoom  enterRoom;
		memset(&enterRoom, 0, sizeof(enterRoom));
		enterRoom.dwUserID = it->dwUserID;
		enterRoom.uValue1 = it->uClientAddr;
		OnUserEnterRoom(it->pProxySock, &enterRoom, true);
		--nDistributeCount;
		it = m_listWaitList.erase(it);
		if (nDistributeCount == 0)
		{
			break;
		}
	}
    if(GAME_JZNC == m_GamePlayKind.wGameID && m_listWaitList.size() > 0)
    {
        //防止剩下的开始不了游戏
        CNetModule::getSingleton().SetTimer(IDI_TIMER_DISTRIBUTE_TABLE, 1000, this, false);
    }
    //UpdateServerIDWaitListCount((CT_DWORD)m_listWaitList.size());
}

/*CT_VOID CGameServerThread::UpdateUserGemToDB(CT_DWORD dwUserID, CT_DWORD dwGem)
{
	MSG_G2DB_UpdateUser_Gem updateGemToDB;
	updateGemToDB.dwUserID = dwUserID;
	updateGemToDB.dwGem = dwGem;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_UPDATE_USER_GEM, &updateGemToDB, sizeof(updateGemToDB));
}*/

CT_VOID CGameServerThread::SwitchUserItem(CServerUserItem *pUserItem, acl::aio_socket_stream* pProxySock, CT_UINT64 dwClient)
{
	if (pUserItem == NULL)
	{
		return;
	}

	pUserItem->SetClientNetAddr(dwClient);
	pUserItem->SetUserCon(pProxySock);


	ITableFrame* pTableFrame = CGameTableManager::get_instance().GetTable(pUserItem->GetTableID());
	if (pTableFrame)
	{
		if (!pTableFrame->IsGameStart())
		{
			if (m_GamePlayKind.wGameID == GAME_BR)
			{
				pUserItem->SetUserStatus(sPlaying);
			}
			else if (m_GamePlayKind.wGameID == GAME_ZJH
                || m_GamePlayKind.wGameID == GAME_MJ
                || m_GamePlayKind.wGameID == GAME_DDZ
                || m_GamePlayKind.wGameID == GAME_PDK)
			{
				pUserItem->SetUserStatus(sReady);
			}
			else
			{
				pUserItem->SetUserStatus(sSit);
			}
		}
		else
		{
			pUserItem->SetUserStatus(sPlaying);
			pUserItem->SetTrustShip(false);
		}

		pTableFrame->OnUserEnterAction(pUserItem, false, true);


		if (m_GamePlayKind.wGameID != GAME_BR && !pTableFrame->IsGameStart())
		{
			if (pTableFrame->CheckGameStart())
			{
				if (pTableFrame->GetGameRoundPhase() == en_GameRound_Free)
				{
					pTableFrame->GameRoundStartDeploy();
				}
				pTableFrame->GameStart();
			}
		}	
	}
	return;
}

void CGameServerThread::HandleDdzAiAction()
{
#ifdef _OS_LINUX_CODE
	CDdzAiThread* pDdzAiThread = CAndroidUserMgr::get_instance().GetDdzAiThread();
	if (pDdzAiThread)
	{
		while (std::shared_ptr<ai_action> aiAction = pDdzAiThread->GetOneAiActionPtr())
		{
			CT_DWORD dwUserID = aiAction->uUserID;
			CServerUserItem* pAndroidUserItem = CAndroidUserMgr::get_instance().FindAndroidUserItem(dwUserID);
			if (pAndroidUserItem)
			{
				IAndroidUserItemSink* pAndroidSink = pAndroidUserItem->GetAndroidUserItemSink();
				if (pAndroidSink)
				{
					pAndroidSink->HandleDdzAiAction(aiAction);
				}
			}
		}
	}
#endif // _OS_LINUX_CODE
}

void CGameServerThread::BuildDdzAiThread(void)
{
#ifdef _OS_LINUX_CODE
	CDdzAiThread* thr = new CDdzAiThread();

	thr->set_detachable(true);
	if (thr->start() == false)
		LOG(ERROR) << "start ddz ai thread failed";

	CAndroidUserMgr::get_instance().SetDdzAiThread(thr);
	LOG(INFO) << "start ddz ai thread...";

#endif // _OS_LINUX_CODEs
}