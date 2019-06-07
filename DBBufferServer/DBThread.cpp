#include "DBThread.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "ServerCfg.h"
#include "CDBConnector.h"
#include "CMD_Plaza.h"
#include "GlobalStruct.h"
#include "ServerMgr.h"
#include "timeFunction.h"
#include <map>
#include <set>

//定时器
#define		IDI_TIMER_UPDATE_USER_COUNT				(100)
#define		TIME_UPDATE_USER_COUNT					(300*1000)

#define		IDI_TIMER_LOAD_SCORE_RANK				(101)
#define		TIME_LOAD_SCORE_RANK					(3600*2*1000)

#define		IDI_TIMER_LOAD_TODAY_RANK				(102)	//加载今日赚金，在线榜，任务完成榜
#define		TIMER_LOAD_TODAY_RANK					(60*10*1000)

#define		IDI_TIMER_ACROSS_DAY					(108)
#define		TIME_ACROSS_DAY							(60*1000)

DBThread::DBThread(void)
	: m_redisClient(CServerCfg::m_RedisAddress, CServerCfg::m_RedisConTimeOut, CServerCfg::m_RedisRwTimeOut)
	, m_dwTodayTime(Utility::GetTime())
	//, m_bNeedLoadNewShareRank(false)
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
	//LOG(WARNING) << "mysql client :" << strlibMysqlPath.c_str();
	charset.format("%s", "utf8mb4");
#endif

	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_AccountdbIP, CServerCfg::m_AccountdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_AccountdbName, CServerCfg::m_AccountdbUser, CServerCfg::m_AccountdbPwd, 1, 0, true, 60, 120, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_RecorddbName, CServerCfg::m_RecorddbUser, CServerCfg::m_RecorddbPwd, 1, 0, true, 60, 120, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_RecordScoredbIP, CServerCfg::m_RecordScoredbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_RecordScoredbName, CServerCfg::m_RecordScoredbUser, CServerCfg::m_RecordScoredbPwd, 2, 0, true, 60, 120, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_PlaybackdbIP, CServerCfg::m_PlaybackdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_PlaybackdbName, CServerCfg::m_PlaybackdbUser, CServerCfg::m_PlaybackdbPwd, 1, 0, true, 60, 120, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_PromotiondbIP, CServerCfg::m_PromotiondbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_PromotiondbName, CServerCfg::m_PromotiondbUser, CServerCfg::m_PromotiondbPwd, 1, 0, true, 60, 60, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_GamedbIP, CServerCfg::m_GamedbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_GamedbName, CServerCfg::m_GamedbUser, CServerCfg::m_GamedbPwd, 1, 0, true, 60, 120, charset.c_str());

	dbaddr.format("%s:%d", CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);
	m_dbManager.add(dbaddr, CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbUser, CServerCfg::m_PlatformdbPwd, 1, 0, true, 60, 120, charset.c_str());

	m_accountDBKey.format("%s@%s:%d", CServerCfg::m_AccountdbName, CServerCfg::m_AccountdbIP, CServerCfg::m_AccountdbPort);
	m_recordDBKey.format("%s@%s:%d", CServerCfg::m_RecorddbName, CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);
	m_recordScoreDBKey.format("%s@%s:%d", CServerCfg::m_RecordScoredbName, CServerCfg::m_RecordScoredbIP, CServerCfg::m_RecordScoredbPort);
	m_playbackDBKey.format("%s@%s:%d", CServerCfg::m_PlaybackdbName, CServerCfg::m_PlaybackdbIP, CServerCfg::m_PlaybackdbPort);
	m_promotionDBKey.format("%s@%s:%d", CServerCfg::m_PromotiondbName, CServerCfg::m_PromotiondbIP, CServerCfg::m_PromotiondbPort);
	m_gameDBKey.format("%s@%s:%d", CServerCfg::m_GamedbName, CServerCfg::m_GamedbIP, CServerCfg::m_GamedbPort);
	m_platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);

	//设置redis
	m_redisClient.set_password(CServerCfg::m_RedisPwd);
	m_redis.set_client(&m_redisClient);
}

DBThread::~DBThread(void)
{

}

CT_VOID DBThread::OnTCPSocketLink(CNetConnector* pConnector)
{

}

CT_VOID DBThread::OnTCPSocketShut(CNetConnector* pConnector)
{

}

CT_VOID DBThread::OnTCPNetworkBind(acl::aio_socket_stream* pSocket)
{
	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "bind one connect: " << strIp; //pSocket->get_peer(true);
	}
	if (!m_bInitServer)
	{
		InitServer();
	}
}

CT_VOID DBThread::OnTCPNetworkShut(acl::aio_socket_stream* pSocket)
{
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo->pCenterSocket == pSocket)
	{
		pCenterInfo->pCenterSocket = NULL;
	}
}

//网络bit数据到来
CT_VOID DBThread::OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen)
{
	if (pSocket == NULL)
	{
		return;
	}

	switch (pMc->dwMainID)
	{
	case MSG_GDB_MAIN:
	{
		OnGameServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_PDB_MAIN:
	{
		OnProxyServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;
	case MSG_CSDB_MAIN:
	{
		OnCenterServerMsg(pMc->dwSubID, pBuf, wLen, pSocket);
	}
	break;

	default:
		break;
	}
}

//网络定时器
CT_VOID DBThread::OnTimer(CT_DWORD dwTimerID)
{
	switch (dwTimerID)
	{
	case IDI_TIMER_UPDATE_USER_COUNT:
	{
		UpdateUserCount();
	}
	break;
	case IDI_TIMER_LOAD_SCORE_RANK:
	{
		LoadScoreRank();
	}
	break;
	case IDI_TIMER_LOAD_TODAY_RANK:
	{
		LoadTodayScoreRank();
		LoadTodayOnlineRank();
		LoadTodayTaskFinishRank();
		/*if (m_bNeedLoadNewShareRank)
		{
			LoadShareScoreRank();
		}*/
	}
	break;
	case IDI_TIMER_ACROSS_DAY:
	{
		AcrossTheDayEvent();
	}
	break;
	default:
		break;
	}
}

//打开
CT_VOID DBThread::Open()
{
	
}

CT_VOID DBThread::InitServer()
{
	//m_redisClient.set_password(CServerCfg::m_RedisPwd);
	//m_redis.set_client(&m_redisClient);

	LoadSystemStatus();
	if (CServerCfg::m_MainDBServer == 1)
    {
        if (m_systemStatus.cbScoreMode)
        {
            CNetModule::getSingleton().SetTimer(IDI_TIMER_UPDATE_USER_COUNT, TIME_UPDATE_USER_COUNT, this);
            //CNetModule::getSingleton().SetTimer(IDI_TIMER_LOAD_SCORE_RANK, TIME_LOAD_SCORE_RANK, this);
            CNetModule::getSingleton().SetTimer(IDI_TIMER_LOAD_TODAY_RANK, TIMER_LOAD_TODAY_RANK, this);
            CNetModule::getSingleton().SetTimer(IDI_TIMER_ACROSS_DAY, TIME_ACROSS_DAY, this);
        }
        LOG(WARNING) << "main db server set time ok!";
    }

	m_bInitServer = true;			
	/*MSG_G2DB_Record_Draw_Score drawScore[200];
	for (int i = 0; i != 200; i++)
	{
		drawScore[i].dwUserID = i;
		drawScore[i].llSourceScore = 10000;
		drawScore[i].iScore = 100;
		drawScore[i].dwRevenue = 10;
		drawScore[i].cbIsbot = 0;
	}
	InsertScoreChangeRecordForPlayGame(drawScore, 200);*/
	//InsertScoreChangeRecord(100, 100, 200, 0, 10, EXCHANGE_SCORE);
	
	/*CT_DWORD  dwTime1 = Utility::GetTick();
    for (int i = 0; i < 20000; ++i)
    {
        acl::string strAccount;
        strAccount.format("account_%u", i);
        m_redis.hset(strAccount.c_str(), "userid", "10");
    }
    LOG(WARNING) << "init server time: " << Utility::GetTick() - dwTime1;*/

	LOG(INFO) << "init db server succ!";
}

CT_VOID DBThread::OnGameServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_G2DB_UPDATE_USER_GEM:
	{
		if (wDataSize != sizeof(MSG_UpdateUser_Gem))
		{
			return;
		}

		MSG_UpdateUser_Gem* pUpdateGem = (MSG_UpdateUser_Gem*)pData;

		acl::db_pool* pool = GetAccountdbPool();
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

		UpdateUserGem(db, pUpdateGem->dwUserID, pUpdateGem->nAddGem);

		pool->put(db);

		break;
	}
	case SUB_G2DB_UPDATE_USER_SCORE:
	{
		if (wDataSize != sizeof(MSG_UpdateUser_Score))
		{
			return;
		}

		MSG_UpdateUser_Score* pUpdateScore = (MSG_UpdateUser_Score*)pData;

		//真人的分数更新才写进数据库
		if (pUpdateScore->bAndroid == false)
		{
			acl::db_pool* pool = GetAccountdbPool();
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

			UpdateUserScore(db, pUpdateScore->dwUserID, pUpdateScore->llAddScore);

			pool->put(db);
		}

		break;
	}
	case SUB_G2DB_UPDATE_BENEFIT_INFO:
	{
		if (wDataSize != sizeof(MSG_G2GB_Update_BenefitInfo))
		{
			return;
		}
		MSG_G2GB_Update_BenefitInfo* pBenefitInfo = (MSG_G2GB_Update_BenefitInfo*)pData;
		UpdateUserBenefitInfo(pBenefitInfo);

		break;
	}
	case SUB_G2DB_SET_USER_SCORE:
	{
		if (wDataSize != sizeof(MSG_UpdateUser_Score))
		{
			return;
		}

		MSG_UpdateUser_Score* pUpdateScore = (MSG_UpdateUser_Score*)pData;

		SetUserScore(pUpdateScore->dwUserID, pUpdateScore->llAddScore);
	}
	break;
	case SUB_G2DB_GEM_CONSUME:
	{
		if (wDataSize != sizeof(MSG_G2DB_Gem_Consume))
		{
			return;
		}

		MSG_G2DB_Gem_Consume* pConsumeGem = (MSG_G2DB_Gem_Consume*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertGemConsumeRecord(db, pConsumeGem);

		pool->put(db);
		break;
	}
	case SUB_G2DB_PROOM_PALYCOUNT_RECORD:
	{
		if (wDataSize != sizeof(MSG_G2DB_PRoom_PlayCountRecord))
		{
			return;
		}
		MSG_G2DB_PRoom_PlayCountRecord* pPlayCountRecord = (MSG_G2DB_PRoom_PlayCountRecord*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertPRoomPlayCount(db, pPlayCountRecord);
		pool->put(db);
		break;
	}
	case SUB_G2DB_PROOM_GAMEROUND_RECORD:
	{
		if (wDataSize != sizeof(MSG_G2DB_PRoom_GameRoundRecord))
		{
			return;
		}
		MSG_G2DB_PRoom_GameRoundRecord* pGameRound = (MSG_G2DB_PRoom_GameRoundRecord*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}
		InsertPRoomGameRound(db, pGameRound);
		pool->put(db);
		break;
	}
	case SUB_G2DB_PROOM_PLAYBACK_RECORD:
	{
		if (wDataSize != sizeof(MSG_G2DB_PRoom_PlaybackRecord))
		{
			return;
		}

		MSG_G2DB_PRoom_PlaybackRecord* pRecord = (MSG_G2DB_PRoom_PlaybackRecord*)pData;

		acl::db_pool* pool = GetPlaybackdbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get playback db pool fail.";
			return;
		}

		acl::db_mysql* db = (db_mysql*)pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get playback db handle fail.";
			return;
		}

		InsertPlaybackRecord(db, pRecord);
		pool->put(db);
		break;
	}

	case SUB_G2DB_PROOM_GAMEROUND_COUNT:
	{
		if (wDataSize != sizeof(MSG_G2DB_PRoom_GameRoundCount))
		{
			return;
		}

		MSG_G2DB_PRoom_GameRoundCount* pRecord = (MSG_G2DB_PRoom_GameRoundCount*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertPRoomGameRoundCount(db, pRecord);
		pool->put(db);
		break;
	}
	case SUB_G2DB_PROOM_GAME_WINLOST:
	{
		if (wDataSize != sizeof(MSG_G2DB_WinLostRecord))
		{
			return;
		}

		MSG_G2DB_WinLostRecord* pRecord = (MSG_G2DB_WinLostRecord*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertWinLostRecord(db, pRecord);
		pool->put(db);
		break;
	}
	case SUB_G2DB_PROOM_INSERT_GAMEROUND:
	{
		if (wDataSize != sizeof(MSG_G2DB_PRoom_InsertGameRound))
		{
			return;
		}

		MSG_G2DB_PRoom_InsertGameRound* pRecord = (MSG_G2DB_PRoom_InsertGameRound*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertGameRoundRecord(pSocket, db, pRecord);
		pool->put(db);
		break;
	}
	case SUB_G2DB_RETURN_GROUPPROOM_GEM:
	{
		/*if (wDataSize != sizeof(MSG_G2DB_Return_GroupPRoom_Gem))
		{
			return;
		}

		MSG_G2DB_Return_GroupPRoom_Gem* pReturnGem = (MSG_G2DB_Return_GroupPRoom_Gem*)pData;

		acl::db_pool* pool = GetAccountdbPool();
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

		ReturnGroupPRoomGem(db, pReturnGem);
		pool->put(db);


		pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		SetGroupPRoomState(db, pReturnGem);
		pool->put(db);*/

		break;
	}
	case SUB_G2DB_GROUP_PROOM_RECORD:
	{
		if (wDataSize != sizeof(MSG_G2DB_Record_GroupPRoom))
		{
			return;
		}

		MSG_G2DB_Record_GroupPRoom* pAutoPRoom = (MSG_G2DB_Record_GroupPRoom*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertGroupPRoomRecord(db, pAutoPRoom);
		pool->put(db);

		break;
	}
	case SUB_G2DB_RETURN_WXGROUPPROOM_GEM:
	{
		if (wDataSize != sizeof(MSG_G2DB_Return_WXGroupPRoom_Gem))
		{
			return;
		}

		MSG_G2DB_Return_WXGroupPRoom_Gem* pReturnGem = (MSG_G2DB_Return_WXGroupPRoom_Gem*)pData;

		acl::db_pool* pool = GetPromotiondbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get promotion db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get promotion db handle fail.";
			return;
		}

		ReturnWXGroupPRoomGem(db, pReturnGem);
		pool->put(db);

		break;
	}
	case SUB_G2DB_WXGROUP_PROOM_RECORD:
	{
		if (wDataSize != sizeof(MSG_G2DB_Record_WXGroupPRoom))
		{
			return;
		}

		MSG_G2DB_Record_WXGroupPRoom* pAutoPRoom = (MSG_G2DB_Record_WXGroupPRoom*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertWXGroupPRoomRecord(db, pAutoPRoom);
		pool->put(db);

		break;
	}
	case SUB_G2DB_UPDATE_USER_SCORE_INFO:
	{
		if (wDataSize != sizeof(MSG_Update_ScoreInfo))
		{
			return;
		}

		MSG_Update_ScoreInfo* pScoreInfo = (MSG_Update_ScoreInfo*)pData;
		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}
		UpdateUserScoreInfo(db, pScoreInfo);

		pool->put(db);

		break;
	}
	case SUB_G2DB_RECORD_DRAW_INFO:
	{
		MSG_G2DB_Record_Draw_Info* pRecordDrawInfo = (MSG_G2DB_Record_Draw_Info*)pData;
        if ((wDataSize !=  (sizeof(MSG_G2DB_Record_Draw_Info)) +  pRecordDrawInfo->wUserCount*sizeof(MSG_G2DB_Record_Draw_Score) + pRecordDrawInfo->dwRecordLen))
        {
            return;
        }
		
		acl::db_pool* pool = GetRecordScoredbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record score db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record score db handle fail.";
			return;
		}

		MSG_G2DB_Record_Draw_Score *pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pData + sizeof(MSG_G2DB_Record_Draw_Info));
		CT_WORD wScoreCount = pRecordDrawInfo->wUserCount; //(CT_WORD)(wDataSize - sizeof(MSG_G2DB_Record_Draw_Info)) / sizeof(MSG_G2DB_Record_Draw_Score);
		
		//LOG(WARNING) << "start record draw info.";
		//CT_DWORD dwTime = Utility::GetTick();
		CT_DWORD dwDrawID = 0;
		SaveRecordDrawInfo(db, pRecordDrawInfo, pDrawScore, wScoreCount, dwDrawID);
		pool->put(db);
		
		if (pRecordDrawInfo->dwRecordLen > 0)
		{
			CT_VOID* pRecordData = (CT_BYTE*)pData + sizeof(MSG_G2DB_Record_Draw_Info) +sizeof(MSG_G2DB_Record_Draw_Score)*pRecordDrawInfo->wUserCount;
			CT_DWORD dwRecordDataLen = wDataSize - sizeof(MSG_G2DB_Record_Draw_Info) - sizeof(MSG_G2DB_Record_Draw_Score)*pRecordDrawInfo->wUserCount;
			InsertPlaybackRecord2(pRecordData, dwRecordDataLen, dwDrawID);
		}

		//LOG(WARNING) << "record draw info time: " << Utility::GetTick() - dwTime;
		//dwTime = Utility::GetTick();
		
		//更新游戏库存(存在刚好库存为零的时候停服了?)
		if (pRecordDrawInfo->llStock != 0)
		{
			UpdateServerStockInfo(pRecordDrawInfo);

			//LOG(WARNING) << "update stock time: " << Utility::GetTick() - dwTime;
			//dwTime = Utility::GetTick();
		}
		if (pRecordDrawInfo->wGameID == GAME_BR || pRecordDrawInfo->wGameID == GAME_FXGZ)
		{
			SaveBaiRenScoreInfo(pRecordDrawInfo, pDrawScore, wScoreCount, dwDrawID);

			//LOG(WARNING) << "update bai ren score time: " << Utility::GetTick() - dwTime;
			//dwTime = Utility::GetTick();
		}

		//记录用户金币金币记录
		InsertScoreChangeRecordForPlayGame(pRecordDrawInfo, pDrawScore, wScoreCount,dwDrawID);

		//LOG(WARNING) << "update score change time: " << Utility::GetTick() - dwTime;
		//dwTime = Utility::GetTick();

		//记录用户输赢记录
		//UpdateWinLostCount(pRecordDrawInfo, pDrawScore, wScoreCount);
        UpddateWinLostCountEx(pRecordDrawInfo, pDrawScore, wScoreCount);

		//记录用户今日在线时长
        UpdateUserTodayOnlineTime(pRecordDrawInfo, pDrawScore, wScoreCount);
        
        //插入今天赚金榜，机器人也插入,这样可以看到比较多的数据(只算赢钱不算输钱)
        InsertTodayEarnScore(pRecordDrawInfo, pDrawScore, wScoreCount);
        
        //LOG(WARNING) << "update win lost time: " << Utility::GetTick() - dwTime;
		//dwTime = Utility::GetTick();

		//std::cout << std::endl;

		break;
	}
	case SUB_G2DB_ANDROID_STATUE:
	{
		MSG_G2DB_Android_Status* pStatus = (MSG_G2DB_Android_Status*)pData;
		if (pStatus)
			UpdateAndroidStatus(pStatus);
		break;
	}
	case SUB_G2DB_INSERT_REPORT:
	{
		if (wDataSize %sizeof(MSG_G2DB_ReportInfo) != 0)
			return;
		
		CT_WORD  wCount = (CT_WORD)wDataSize / sizeof(MSG_G2DB_ReportInfo);
		MSG_G2DB_ReportInfo* pReport = (MSG_G2DB_ReportInfo*)pData;
		if (pReport)
			InsertReportInfo(pReport, wCount);
		break;
	}
	case SUB_G2DB_UPDATE_GOODCARD_INFO:
	{
		if (wDataSize != sizeof(MSG_G2DB_GoodCard_Info))
			return;

		MSG_G2DB_GoodCard_Info* pGoodCardInfo = (MSG_G2DB_GoodCard_Info*)pData;
		UpdateGoodCardInfo(pGoodCardInfo);
		break;
	}
	case SUB_G2DB_SAVE_USER_DIANKONG_DATA:
	{
		if (wDataSize != sizeof(MSG_G2DB_User_PointControl))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << " is not equal to sizeof(MSG_D2CS_Set_FishDiankong): " << sizeof(MSG_D2CS_Set_FishDiankong);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_SAVE_USER_DIANKONG_DATA pData is NULL";
			return;
		}

		MSG_G2DB_User_PointControl *pDianKongData = (MSG_G2DB_User_PointControl *)pData;
		SaveUserDianKongData(pDianKongData);
		break;
	}
	case SUB_G2DB_READ_USER_DIANKONG_DATA:
	{
		if (wDataSize != sizeof(MSG_GS2DB_ReadDianKongData))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << " is not equal to sizeof(MSG_GS2DB_ReadDianKongData): " << sizeof(MSG_GS2DB_ReadDianKongData);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_READ_USER_DIANKONG_DATA pData is NULL";
			return;
		}

		MSG_GS2DB_ReadDianKongData * pReadDianKongData = (MSG_GS2DB_ReadDianKongData*)pData;
		ReadUserDianKongData(pReadDianKongData, pSocket);
		break;
	}
	case SUB_G2DB_READ_USER_MISS_INFO:
	{
		if(wDataSize != sizeof(JSFish_Player_Miss_Info))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << " is not equal to sizeof(JSFish_Player_Miss_Info): " << sizeof(JSFish_Player_Miss_Info);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_READ_USER_MISS_INFO pData is NULL";
			return;
		}

		JSFish_Player_Miss_Info *pReadMissData = (JSFish_Player_Miss_Info*)pData;
		ReadUserMissInfo(pReadMissData, pSocket);
		break;
	}
	case SUB_G2DB_DEL_USER_DIANKONG_DATA:
	{
		if (wDataSize != sizeof(MSG_GS2DB_DelUserDianKongData))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << " is not equal to sizeof(MSG_GS2DB_DelUserDianKongData): " << sizeof(MSG_GS2DB_DelUserDianKongData);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_DEL_USER_DIANKONG_DATA pData is NULL";
			return;
		}

		MSG_GS2DB_DelUserDianKongData *pDelDianKongData = (MSG_GS2DB_DelUserDianKongData *)pData;
		DelUserDianKongData(pDelDianKongData);
		break;
	}
	case SUB_G2DB_SAVE_FISH_TOTALWINLOSESCORE:
	{
		//测试
		//LOG(ERROR) << "收到SUB_G2DB_SAVE_FISH_TOTALWINLOSESCORE";
		if (wDataSize != sizeof(MSG_GS2DB_FishTotalWinLoseScore))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << "is not equal to sizeof(MSG_GS2DB_FishTotalWinLoseScore): " << sizeof(MSG_GS2DB_FishTotalWinLoseScore);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_SAVE_FISH_TOTALWINLOSESCORE pData is NULL";
			return;
		}

		MSG_GS2DB_FishTotalWinLoseScore *pWinLoseScore = (MSG_GS2DB_FishTotalWinLoseScore *)pData;
		SaveTotalWinLoseScore(pWinLoseScore);
		break;
	}
	case SUB_G2DB_SAVE_MISS_INFO:
	{
		if(wDataSize != sizeof(JSFish_Player_Miss_Info))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << "is not equal to sizeof(JSFish_Player_Miss_Info) :" << sizeof(JSFish_Player_Miss_Info);
			return;
		}
		if(pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_SAVE_MISS_INFO pData is NULL";
			return;
		}

		JSFish_Player_Miss_Info *pMissData = (JSFish_Player_Miss_Info *)pData;
		SaveMissInfo(pMissData);
		break;
	}
	case SUB_G2DB_SAVE_PLAYER_FISH_INFO:
	{
        if (wDataSize != sizeof(MSG_GS2DB_PlayerFishInfo))
        {
            LOG(WARNING) << "wDataSize: " << wDataSize << "is not equal to sizeof(MSG_GS2DB_PlayerFishInfo): " << sizeof(MSG_GS2DB_PlayerFishInfo);
            return;
        }
        if (pData == NULL)
        {
            LOG(WARNING) << "SUB_G2DB_SAVE_PLAYER_FISH_INFO pData is NULL";
            return;
        }

		MSG_GS2DB_PlayerFishInfo *pPlayerFishInfo = (MSG_GS2DB_PlayerFishInfo *)pData;
        SavePlayerFishInfo(pPlayerFishInfo);
	    break;
	}
	case SUB_G2DB_SAVE_FISH_STATISTICSINFO:
	{
		if (wDataSize != sizeof(MSG_GS2DB_FishStatisticsInfo))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << "is not equal to sizeof(MSG_GS2DB_FishStatisticsInfo): " << sizeof(MSG_GS2DB_FishStatisticsInfo);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_SAVE_FISH_STATISTICSINFO pData is NULL";
			return;
		}

		MSG_GS2DB_FishStatisticsInfo *pFishStatisticsInfo = (MSG_GS2DB_FishStatisticsInfo *)pData;
		SaveFishStatisticsInfo(pFishStatisticsInfo);
		break;
	}
	case SUB_G2DB_SAVE_FISH_CONTROL_DATA:
	{
		if (wDataSize != sizeof(MSG_GS2DB_SaveFishControlData))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << "is not equal to sizeof(MSG_GS2DB_SaveFishControlData): " << sizeof(MSG_GS2DB_SaveFishControlData);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_SAVE_FISH_CONTROL_DATA pData is NULL";
			return;
		}

		MSG_GS2DB_SaveFishControlData *pControlData = (MSG_GS2DB_SaveFishControlData*)pData;
		SaveFishControlData(pControlData);
		break;
	}
	case SUB_G2DB_SAVE_FISH_CURRENT_KUCUN:
	{
		if (wDataSize != sizeof(MSG_GS2DB_SaveFishKuCun))
		{
			LOG(WARNING) << "wDataSize: " << wDataSize << "is not equal to sizeof(MSG_GS2DB_SaveFishKuCun): " << sizeof(MSG_GS2DB_SaveFishKuCun);
			return;
		}
		if (pData == NULL)
		{
			LOG(WARNING) << "SUB_G2DB_SAVE_FISH_CURRENT_KUCUN pData is NULL";
			return;
		}
		
		MSG_GS2DB_SaveFishKuCun *pCurKuCunData = (MSG_GS2DB_SaveFishKuCun *)pData;
		SaveFishCurrentKuCun(pCurKuCunData);
		break;
	}
	    case SUB_G2DB_UPDATE_USER_SCORE_DATA:
        {
            if (wDataSize != sizeof(MSG_G2DB_ScoreData))
            {
                return;
            }

            MSG_G2DB_ScoreData* pScoreData = (MSG_G2DB_ScoreData*)pData;
			UpdateUserScoreData(pScoreData);
            break;
        }
		case SUB_G2DB_RECORD_HBSL_INFO:
		{
            MSG_G2DB_Record_HBSL_Info* pRecordHBSLInfo = (MSG_G2DB_Record_HBSL_Info*)pData;
			if ((wDataSize !=  (sizeof(MSG_G2DB_Record_HBSL_Info)) +  pRecordHBSLInfo->wUserCount*sizeof(MSG_G2DB_Record_HBSL_GrabInfo)))
			{
			    LOG(WARNING) << "record hbsl info len fail.";
				return;
			}

			MSG_G2DB_Record_HBSL_GrabInfo *pRecordHBSLGrabInfo = (MSG_G2DB_Record_HBSL_GrabInfo*)((CT_BYTE*)pData + sizeof(MSG_G2DB_Record_HBSL_Info));
			CT_WORD& wUserCount = pRecordHBSLInfo->wUserCount;
			SaveRecordHBSLInfo(pRecordHBSLInfo, pRecordHBSLGrabInfo, wUserCount);
			break;
		}
        case SUB_G2DB_BLACK_LIST_USER:
        {
            MSG_GS2DB_BlackList_User* pBlackListUser = (MSG_GS2DB_BlackList_User*)pData;
            UpdateBlackListUser(pBlackListUser);
            break;
        }
	default:
		break;
	}
}

CT_VOID DBThread::OnProxyServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case SUB_P2DB_QUERY_PROOM_GAMEROUND:
	{
		if (wDataSize != sizeof(MSG_P2DB_Query_PRoom_GameRound))
		{
			return;
		}

		MSG_P2DB_Query_PRoom_GameRound* pGameRound = (MSG_P2DB_Query_PRoom_GameRound*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		QueryPRoomGameRound(pSocket, db, pGameRound);

		pool->put(db);

	}
	break;
	case SUB_P2DB_QUERY_PROOM_PALYCOUNT:
	{
		if (wDataSize != sizeof(MSG_P2DB_Query_PRoom_PlayCount))
		{
			return;
		}
		
		MSG_P2DB_Query_PRoom_PlayCount* pPlayCount = (MSG_P2DB_Query_PRoom_PlayCount*)pData;

		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		QueryPRoomPlayCount(pSocket, db, pPlayCount);
		pool->put(db);
	}
	break;

	case SUB_P2DB_QUERY_USER_SCORE_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_UserScore_Info))
		{
			return;
		}

		MSG_CS_Query_UserScore_Info* pQueryUserScore = (MSG_CS_Query_UserScore_Info*)pData;

		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		QueryUserScoreInfo(pSocket, db, pQueryUserScore);
		pool->put(db);
	}
	break;
	case SUB_P2DB_QUERY_USER_HONGBAO_INFO:
	{
		if (wDataSize != sizeof(MSG_CS_Query_UserHongBao_Info))
		{
			return;
		}

		MSG_CS_Query_UserHongBao_Info* pQueryUserHongBao = (MSG_CS_Query_UserHongBao_Info*)pData;

		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		QueryUserHongBaoInfo(pSocket, db, pQueryUserHongBao);
		pool->put(db);
	}
	break;
    case SUB_P2DB_QUERY_GAME_RECORD:
    {
    	if (wDataSize != sizeof(MSG_CS_Query_GameRecord))
		{
			return;
		}
		
        MSG_CS_Query_GameRecord* pQueryGameRecord = (MSG_CS_Query_GameRecord*)pData;
		QueryGameRecord(pSocket, pQueryGameRecord);
    }
    break;
	case SUB_P2DB_QUERY_REPORT_GAME_RECORD:
	{
		if (wDataSize != sizeof(MSG_CS_Query_Report_GameRecord))
		{
			return;
		}
		
		MSG_CS_Query_Report_GameRecord* pQueryGameRecord = (MSG_CS_Query_Report_GameRecord*)pData;
		QueryReportRecord(pSocket, pQueryGameRecord);
	}
	break;
	case SUB_P2DB_QUERY_NORMAL_GAME_RECORD:
	{
		if (wDataSize != sizeof(MSG_CS_Query_Normal_GameRecord))
		{
			return;
		}
		MSG_CS_Query_Normal_GameRecord* pQueryGameRecord = (MSG_CS_Query_Normal_GameRecord*)pData;
		QueryNormalRecord(pSocket, pQueryGameRecord);
	}
	break;
	case SUB_P2DB_QUERY_HBSL_RECORD:
	{
		if (wDataSize != sizeof(MSG_CS_Query_Hbsl_Record))
		{
			return;
		}
		MSG_CS_Query_Hbsl_Record* pQueryHbslRecord = (MSG_CS_Query_Hbsl_Record*)pData;
		QueryHBSLRecord(pSocket, pQueryHbslRecord);
	}
	break;
    case SUB_P2DB_QUERY_HBSL_FA_RECORD:
    {
        if (wDataSize != sizeof(MSG_CS_Query_Hbsl_Record))
        {
            return;
        }
        MSG_CS_Query_Hbsl_Record* pQueryHbslRecord = (MSG_CS_Query_Hbsl_Record*)pData;
		QueryHBSLFaRecord(pSocket, pQueryHbslRecord);
    }
    break;
	case SUB_P2DB_QUERY_HBSL_WIN_COLOR_PRIZE_RECORD:
	{
		if (wDataSize != sizeof(MSG_CS_Query_Hbsl_Record))
		{
			return;
		}
		MSG_CS_Query_Hbsl_Record* pQueryHbslRecord = (MSG_CS_Query_Hbsl_Record*)pData;
		QueryHBSLWinColorPrizeRecord(pSocket, pQueryHbslRecord);
	}
	break;
	default:
		break;
	}
}

CT_VOID DBThread::OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket)
{
	switch (wSubCmdID)
	{
	case MSG_CS2DB_REGISTER:
	{
		if (wDataSize != sizeof(CMD_CenterServer_Info))
		{
			return;
		}
		CMD_CenterServer_Info* pCenterInfo = (CMD_CenterServer_Info*)pData;
		CServerMgr::get_instance().RegisterCenter(pCenterInfo, pSocket);
		SendSystemStatusToCenter();

		if (m_systemStatus.cbScoreMode)
		{
			//LoadScoreRank();
			//LoadSignInRank();
			LoadTodayScoreRank();
			//LoadShareScoreRank();
			LoadPCUInfo();
			LoadRechargeChannelInfo(0);
			//LoadRechargeQQWXList();
			LoadGeneralizePromoter();
			LoadExchangeChannelStatus();
			LoadQuanMinChannelInfo();
			LoadClientChannelDomain();
			LoadVip2Config();
			LoadChannelShowExchangeCond();
			LoadTodayOnlineRank();
			LoadTodayTaskFinishRank();
			LoadWealthGodComingCfg();
			LoadWealthGodComingRewardCfg();
            LoadWealthGodComingIndex();
            LoadWealthGodComingHistoryData();
            LoadRechargeChannelAmount();
			LoadExchangeChannelAmount();
            LoadDuoBaoCfg();
            LoadDuoBaoDoingData();
			LoadBenefitConfig();
			LoadPlatformRechargeCfg();
			//新年红包
			LoadRedPacketStatus();
            LoadRedPacketCfg();
            LoadRedPacketIndex();
            LoadChatUploadUrl();
			LoadAnnouncementInfo();
			//群组
            LoadGroupLevelCfg();
			LoadGroupInfo();
            LoadGroupUserRevenue();
            LoadGroupChatMsg();
        }
	}
	break;
	case SUB_CS2DB_LOAD_GROUP_INFO:
	{
		//查询群组信息和对应的群成员信息
		/*acl::db_pool* pool = GetAccountdbPool();
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

		QueryGroupInfo(pSocket, db);

		pool->put(db);

		//查询到自动开房间的游戏配置信息
		pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}
		QueryGroupPRoomConfig(pSocket, db);

		pool->put(db);

		//微信群组
		{

			acl::db_pool* pool = GetPromotiondbPool();
			if (pool == NULL)
			{
				LOG(WARNING) << "get promotion db pool fail.";
				return;
			}

			acl::db_handle* db = pool->peek_open();
			if (db == NULL)
			{
				LOG(WARNING) << "get promotion db handle fail.";
				return;
			}

			QueryWXGroupInfo(pSocket, db);
			pool->put(db);
		}*/
	}
	break;
	case SUB_CS2DB_CREATE_GROUP:
	{
		if (wDataSize != sizeof(MSG_CS2DB_CreateGroup))
		{
			return;
		}

		MSG_CS2DB_CreateGroup* pCreateGroup = (MSG_CS2DB_CreateGroup*)pData;
		acl::db_pool* pool = GetAccountdbPool();
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

		InsertGroup(pSocket, db, pCreateGroup);

		pool->put(db);

	}
	break;
	case SUB_CS2DB_ADD_GROUP_USER:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Add_GroupUser))
		{
			return;
		}

		MSG_CS2DB_Add_GroupUser* pAddUser = (MSG_CS2DB_Add_GroupUser*)pData;
		acl::db_pool* pool = GetAccountdbPool();
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

		AddGroupUser(db, pAddUser);

		pool->put(db);

	}
	break;
	case SUB_CS2DB_QUERY_BE_ADD_GROUP_USER:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Query_BeAdd_GroupUser))
		{
			LOG(WARNING) << "msg error";
			return;
		}

		MSG_CS2DB_Query_BeAdd_GroupUser* pBeAddGroupUser = (MSG_CS2DB_Query_BeAdd_GroupUser*)pData;

		acl::db_pool* pool = GetAccountdbPool();
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

		QueryBeAddGroupUser(pSocket, db, pBeAddGroupUser);

		pool->put(db);
	}
	break;
	case SUB_CS2DB_DEL_GROUP_USER:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Remove_GroupUser))
		{
			return;
		}

		MSG_CS2DB_Remove_GroupUser* pRemoveUser = (MSG_CS2DB_Remove_GroupUser*)pData;
		acl::db_pool* pool = GetAccountdbPool();
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

		RemoveGroupUser(db, pRemoveUser);

		pool->put(db);

	}
	break;
	case SUB_CS2DB_DEL_GROUP:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Remove_Group))
		{
			return;
		}

		MSG_CS2DB_Remove_Group* pRemoveGroup = (MSG_CS2DB_Remove_Group*)pData;
		acl::db_pool* pool = GetAccountdbPool();
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

		RemoveGroup(db, pRemoveGroup);

		pool->put(db);

		//删除群设置
		{
			acl::db_pool* pool = GetRecorddbPool();
			if (pool == NULL)
			{
				LOG(WARNING) << "get record db pool fail.";
				return;
			}

			acl::db_handle* db = pool->peek_open();
			if (db == NULL)
			{
				LOG(WARNING) << "get record db handle fail.";
				return;
			}

			RemoveGroupRoomCfg(db, pRemoveGroup);

			pool->put(db);
		}
	}
	break;
	case SUB_CS2DB_MODIFY_GROUP_INFO:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Modify_GroupInfo))
		{
			return;
		}

		MSG_CS2DB_Modify_GroupInfo* pModifyGroupInfo = (MSG_CS2DB_Modify_GroupInfo*)pData;
		acl::db_pool* pool = GetAccountdbPool();
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

		ModifyGroupInfo(db, pModifyGroupInfo);

		pool->put(db);
	}
	break;
    case SUB_CS2DB_GROUP_CONTRIBUTION:
    {
        if (wDataSize != sizeof(MSG_CS2DB_Update_GroupContribution))
        {
            return;
        }

        MSG_CS2DB_Update_GroupContribution* pAddGroupContribution = (MSG_CS2DB_Update_GroupContribution*)pData;
        acl::db_pool* pool = GetAccountdbPool();
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
        AddGroupContribution(db, pAddGroupContribution);

        pool->put(db);
        break;
    }
	case SUB_CS2DB_DEDUCT_GROUP_MASTER_GEM:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Deduct_GroupMaster_Gem))
		{
			return;
		}
		MSG_CS2DB_Deduct_GroupMaster_Gem* pDeductGem = (MSG_CS2DB_Deduct_GroupMaster_Gem*)pData;

		acl::db_pool* pool = GetAccountdbPool();
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

		DeductGroupMasterGem(pSocket, db, pDeductGem);

		pool->put(db);

		return;

	}
	break;
	case SUB_CS2DB_GROUP_ROOM_INFO:
	{
		if (wDataSize != sizeof(MSG_CS2DB_GroupRoom_Info))
		{
			return;
		}

		MSG_CS2DB_GroupRoom_Info* pGroupRoomInfo = (MSG_CS2DB_GroupRoom_Info*)pData;
		acl::db_pool* pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		InsertGroupPRoomInfo(pSocket, db, pGroupRoomInfo);

		pool->put(db);
	}
	break;
    case SUB_CS2DB_GROUP_POSITION:
    {
        MSG_CS2DB_GroupPosition* pGroupPosition = (MSG_CS2DB_GroupPosition*)pData;
        ChangeGroupMemberPosition(pGroupPosition);
    }
    break;
    case SUB_CS2DB_GROUP_USER_INFO:
    {
        MSG_CS2DB_Update_GroupUserInfo* groupUserInfo = (MSG_CS2DB_Update_GroupUserInfo*)pData;
        UpdateGroupUserInfo(groupUserInfo);
    }
    break;
    case SUB_CS2DB_GROUP_CHAT:
    {
        MSG_CS2DB_Insert_GroupChat* pGroupChat = (MSG_CS2DB_Insert_GroupChat*)pData;
        InsertGroupChatMsg(pGroupChat);
        break;
    }
    case SUB_CS2DB_UPDATE_GROUP_CHAT:
    {
        MSG_CS2DB_Update_GroupChat* pGroupChatStatus = (MSG_CS2DB_Update_GroupChat*)pData;
        UpdateGroupChatMsgStatus(pGroupChatStatus);
        break;
    }
    case SUB_CS2DB_UPDATE_GROUP_DIR_INCOME:
    {
        if (wDataSize % sizeof(MSG_CS2DB_Update_GroupIncome) != 0)
        {
            return;
        }

        CT_DWORD dwCount = wDataSize / sizeof(MSG_CS2DB_Update_GroupIncome);
        MSG_CS2DB_Update_GroupIncome* pGroupIncome = (MSG_CS2DB_Update_GroupIncome*)pData;
        UpdateGroupDirIncome(pGroupIncome, dwCount);
        break;
    }
    case SUB_CS2DB_UPDATE_GROUP_SUB_INCOME:
    {
        if (wDataSize % sizeof(MSG_CS2DB_Update_GroupIncome) != 0)
        {
            return;
        }

        CT_DWORD dwCount = wDataSize / sizeof(MSG_CS2DB_Update_GroupIncome);
        MSG_CS2DB_Update_GroupIncome* pGroupIncome = (MSG_CS2DB_Update_GroupIncome*)pData;
        UpdateGroupSubIncome(pGroupIncome, dwCount);
        break;
    }
    case SUB_CS2DB_INSERT_GROUP_EXCHANGE:
    {
        MSG_CS2DB_Insert_Exchange_GroupIncome* pExchangeGroupIncome = (MSG_CS2DB_Insert_Exchange_GroupIncome*)pData;
        InsertGroupIncomeExchange(pExchangeGroupIncome);
    }
    break;
    case SUB_CS2DB_QUERY_GROUP_EXCHANGE:
    {
        MSG_CS2DB_Query_GroupIncome_Exchange* pQueryExchange = (MSG_CS2DB_Query_GroupIncome_Exchange*)pData;
        QueryGroupIncomeExchange(pSocket, pQueryExchange);
    }
    break;
	case SUB_CS2DB_RETURN_GROUPPROOM_GEM:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Return_GroupPRoom_Gem))
		{
			return;
		}

		MSG_CS2DB_Return_GroupPRoom_Gem* pReturnGem = (MSG_CS2DB_Return_GroupPRoom_Gem*)pData;

		acl::db_pool* pool = GetAccountdbPool();
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

		ReturnGroupPRoomGem(db, pReturnGem);
		pool->put(db);


		pool = GetRecorddbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get record db pool fail.";
			return;
		}

		db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get record db handle fail.";
			return;
		}

		SetGroupPRoomState(db, pReturnGem);
		pool->put(db);

	}
	break;
	case SUB_CS2DB_LOAD_HONGBAO_INFO:
	{
		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		LoadHongBaoInfo(pSocket, db);

		pool->put(db);
	}
	break;
	case SUB_CS2DB_ADD_USER_GEM:
	{
		if (wDataSize != sizeof(MSG_UpdateUser_Gem))
		{
			return;
		}

		MSG_UpdateUser_Gem* pUpdateGem = (MSG_UpdateUser_Gem*)pData;

		acl::db_pool* pool = GetAccountdbPool();
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

		UpdateUserGem(db, pUpdateGem->dwUserID, pUpdateGem->nAddGem);

		pool->put(db);
	}
	break;

	case SUB_CS2DB_ADD_USER_SCORE:
	{
		if (wDataSize != sizeof(MSG_UpdateUser_Score))
		{
			return;
		}

		MSG_UpdateUser_Score* pUpdateScore = (MSG_UpdateUser_Score*)pData;

		acl::db_pool* pool = GetAccountdbPool();
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

		UpdateUserScore(db, pUpdateScore->dwUserID, pUpdateScore->llAddScore);

		pool->put(db);
	}
	break;
	case SUB_CS2DB_INSERT_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Insert_HongBao))
		{
			return;
		}
		MSG_CS2DB_Insert_HongBao* pInsertHongBao = (MSG_CS2DB_Insert_HongBao*)pData;

		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		InsertHongbaoInfo(pSocket, db, pInsertHongBao);

		pool->put(db);
	}
	break;
	case SUB_CS2DB_DELETE_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Delete_HongBao))
		{
			return;
		}
		MSG_CS2DB_Delete_HongBao* pDeleteHongBao = (MSG_CS2DB_Delete_HongBao*)pData;

		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		DeleteHongbaoInfo(db, pDeleteHongBao);

		pool->put(db);

	}
	break;
	case SUB_CS2DB_UPDATE_HONGBAO:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Update_HongBao))
		{
			return;
		}
		MSG_CS2DB_Update_HongBao* pUpdateHongBao = (MSG_CS2DB_Update_HongBao*)pData;

		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		UpdateHongbaoInfo(db, pUpdateHongBao);

		pool->put(db);
	}
	break;
	case SUB_CS2DB_LOAD_TASK:
	{
		acl::db_pool* pool = GetPlatformdbPool();
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

		LoadTaskInfo(pSocket, db);

		pool->put(db);
	}
	break;

	case SUB_CS2DB_INSERT_TASK:
	{
		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		MSG_CS2DB_Insert_Task* pInsertTask = (MSG_CS2DB_Insert_Task*)pData;
		InsertTaskInfo(db, pInsertTask);

		pool->put(db);
	}
	break;
	case SUB_CS2DB_UPDATE_TASK:
	{
		acl::db_pool* pool = GetGamedbPool();
		if (pool == NULL)
		{
			LOG(WARNING) << "get game db pool fail.";
			return;
		}

		acl::db_handle* db = pool->peek_open();
		if (db == NULL)
		{
			LOG(WARNING) << "get game db handle fail.";
			return;
		}

		MSG_CS2DB_Update_Task* pUpdateTask = (MSG_CS2DB_Update_Task*)pData;
		UpdateTaskInfo(db, pUpdateTask);

		pool->put(db);
	}
	break;
	case SUB_CS2DB_RESET_TASK:
	{
		LOG(WARNING) << "reset daily task start. ";
		CT_DWORD dwTime1 = Utility::GetTick();
		ResetDailyTask();
		LOG(WARNING) << "reset daily task end. user time: " << Utility::GetTick() - dwTime1;
	}
	break;
	case SUB_CS2DB_UPDATE_PCU:
	{
		MSG_CS2DB_Update_PCU* pPcu = (MSG_CS2DB_Update_PCU*)pData;
		UpdatePCU(pPcu);
	}
	break;
	case SUB_CS2DB_INSERT_SCORE_CHANGE:
	{
		if (wDataSize != sizeof(MSG_Insert_ScoreChange_Record))
		{
			return;
		}
		MSG_Insert_ScoreChange_Record* pChangeScoreRecord = (MSG_Insert_ScoreChange_Record*)pData;

		InsertScoreChangeRecord(pChangeScoreRecord->dwUserID, pChangeScoreRecord->llSourceBankScore, \
			pChangeScoreRecord->llSourceScore, pChangeScoreRecord->llChangeBankScore, pChangeScoreRecord->llChangeScore, pChangeScoreRecord->enType);
	}
	break;
	case SUB_CS2DB_UPDAET_USER_SHOWEXCHANGE:
	{
		if (wDataSize != sizeof(MSG_Update_ShowExchange))
		{
			return;
		}
		MSG_Update_ShowExchange* pShowExchange = (MSG_Update_ShowExchange*)pData;
		UpdateUserShowExchange(pShowExchange->dwUserID, pShowExchange->cbShowExchange);
	}
	break;
	case SUB_CS2DB_INSERT_TASK_REWARD:
	{
		if (wDataSize != sizeof(MSG_CS2DB_Insert_TaskReward))
		{
			return;
		}
		MSG_CS2DB_Insert_TaskReward* pTaskReward = (MSG_CS2DB_Insert_TaskReward*)pData;
		InsertTaskRewardRecord(pTaskReward->dwUserID, pTaskReward->wTaskID, pTaskReward->llReward);
		UpdateUserTodayTaskRewardCount(pTaskReward);
	}
	break;
	case SUB_CS2DB_RELOAD_RECHARGE_CHANNEL:
	{
		LoadRechargeChannelInfo(1);
	}
	break;
    case SUB_CS2DB_UPDATE_WGC_INDEX:
    {
    	if (wDataSize !=sizeof(tagWealthGodComingLastIndex))
			return;
	
		tagWealthGodComingLastIndex* pLastIndex = (tagWealthGodComingLastIndex*)pData;
    	UpdateWealthGodComingIndex(pLastIndex);
    }
    break;
	case SUB_CS2DB_INSERT_WGC_DATA:
	{
		if (wDataSize != sizeof (MSG_CS2DB_Insert_WGC_Data))
			return;
		
		MSG_CS2DB_Insert_WGC_Data* pWgcData = (MSG_CS2DB_Insert_WGC_Data*)pData;
		InsertWealthGodComingData(pWgcData);
	}
	break;
    case SUB_CS2DB_UPDATE_WGC_DATA:
    {
    	if (wDataSize != sizeof(MSG_CS2DB_Update_WGC_Data))
			return;
	
		MSG_CS2DB_Update_WGC_Data* pWgcData = (MSG_CS2DB_Update_WGC_Data*)pData;
		UpdateWealthGodComingData(pWgcData);
    }
    break;
    case SUB_CS2DB_UPDATE_WGC_USERINFO:
    {
    	if (wDataSize !=sizeof(MSG_CS2DB_Update_WGC_UserInfo))
			return;
	
		MSG_CS2DB_Update_WGC_UserInfo* pUserInfo = (MSG_CS2DB_Update_WGC_UserInfo*)pData;
		UpdateWealthGodComingUserInfo(pUserInfo);
    }
    break;
    case SUB_CS2DB_UPDATE_WGC_ALLUSERINFO:
    {
        if (wDataSize < sizeof(MSG_CS2DB_Update_WGC_UserInfo))
            return;
        
        CT_DWORD dwCount = wDataSize /sizeof(MSG_CS2DB_Update_WGC_UserInfo);
        MSG_CS2DB_Update_WGC_UserInfo* pUserInfo = (MSG_CS2DB_Update_WGC_UserInfo*)pData;
        for (CT_DWORD i = 0 ; i < dwCount; ++i)
        {
            UpdateWealthGodComingUserInfo(pUserInfo++);
        }
    }
    break;
		case SUB_CS2DB_UPADATE_DUOBAO_DATA:
		{
			if (wDataSize != sizeof(MSG_CS2DB_Update_DuoBao_Data))
				return;

			MSG_CS2DB_Update_DuoBao_Data* pDuoBaoData = (MSG_CS2DB_Update_DuoBao_Data*)pData;
            UpdateDuoBaoData(pDuoBaoData);

			break;
		}
		case SUB_CS2DB_INSERT_DUOBAO_USERINFO:
		{
			if (wDataSize != sizeof(MSG_CS2DB_Insert_DuoBao_User))
				return;

			MSG_CS2DB_Insert_DuoBao_User* pDuoBaoUser = (MSG_CS2DB_Insert_DuoBao_User*)pData;
			InsertDuoBaoUser(pDuoBaoUser);

			break;
		}
		case SUB_CS2DB_UPDATE_REDPACKET_STAGE:
		{
			if (wDataSize != sizeof(MSG_CS2DB_Update_RedPacket_Stage))
				return;

			MSG_CS2DB_Update_RedPacket_Stage* pRedPacketStage = (MSG_CS2DB_Update_RedPacket_Stage*)pData;
            UpdateRedPacketStage(pRedPacketStage);
			break;
		}
		case SUB_CS2DB_REDPACKET_GRAB_INFO:
		{
            if (wDataSize != sizeof(MSG_CS2DB_Insert_RadPacket_GrabInfo))
                return;

            MSG_CS2DB_Insert_RadPacket_GrabInfo* pRedPacketGrab = (MSG_CS2DB_Insert_RadPacket_GrabInfo*)pData;
            InsertRedPacketGrab(pRedPacketGrab);
			break;
		}
	case SUB_CS2DB_SEND_MAIL:
	{
		if (wDataSize < sizeof(tagUserMail))
			return;
		
		CT_DWORD dwCount = wDataSize /sizeof(tagUserMail);
		tagUserMail* pUserMail = (tagUserMail*)pData;
		for (CT_DWORD i = 0 ; i < dwCount; ++i)
		{
			SendMail((*pUserMail++));
		}
	}
	break;
	default:
		break;
	}
}

CT_VOID DBThread::UpdateUserCount()
{
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
	

	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	CT_DWORD dwTotalHallUserCount = 0; //包括大厅中的人数
	std::map<CT_DWORD, std::map<CT_BYTE, CT_DWORD>> mapGamePlatformUserCount;
	for (auto& it : result)
	{
		std::string att = it.first.c_str();
		if (att.find("ccu_hall_") != std::string::npos)
		{
			auto pos = att.find_last_of("_");
			std::string strPlatformId = att.substr(pos + 1);
			CT_BYTE cbPlatformId = (CT_BYTE)atoi(strPlatformId.c_str());
			std::string strHallPlayerCount = it.second.c_str();
			CT_DWORD dwHallPlayerCount = atoi(strHallPlayerCount.c_str());

			if (cbPlatformId == 0)
				continue;

			InsertOnlineRecord(db, cbPlatformId, 0, 0, dwHallPlayerCount, 0);

			dwTotalHallUserCount += dwHallPlayerCount;
		}
		/*else if (att.find("game") != std::string::npos)
		{
			auto pos = att.find("_");
			std::string strGameID = att.substr(pos + 1);
			CT_WORD wGameKindID = atoi(strGameID.c_str());
			CT_WORD wGameID = wGameKindID / 100;
			CT_WORD wKindID = wGameKindID % 100;
			std::string strGamePlayerCount = it.second.c_str();
			CT_DWORD dwGamePlayerCount = atoi(strGamePlayerCount.c_str());

			InsertOnlineRecord(db, 0, wGameID,wKindID, dwGamePlayerCount, 2);
		}*/
		else if (att.find("server_") != std::string::npos)
		{
			auto nPosPlatformId = att.find("_");
			auto nPosGameId = att.find_last_of("-");

			std::string strPlatformId = att.substr(nPosPlatformId + 1, nPosGameId);
			CT_BYTE cbPlatformId = (CT_BYTE)atoi(strPlatformId.c_str());

			std::string strGameKindID = att.substr(nPosGameId + 1);
			CT_WORD wGameKindID = atoi(strGameKindID.c_str());
			CT_WORD wGameID = wGameKindID / 100;
			CT_WORD wKindID = (wGameKindID % 100) / 10;
			CT_DWORD dwUserCount = (CT_DWORD)atoi(it.second.c_str());

			//计算
			std::map<CT_BYTE, CT_DWORD>& mapPlatformUserCount = mapGamePlatformUserCount[wGameID*100+wKindID];
			mapPlatformUserCount[cbPlatformId] += dwUserCount;
		}
		else if (att.find("ccu_") != std::string::npos)
		{
			CT_BYTE cbMachineType = MACHINE_TYPE_IOS;
			auto posMachine = att.find("android");
			if (posMachine != std::string::npos)
			{
				cbMachineType = MACHINE_TYPE_ANDROID;
			}
			auto posPlatformId = att.find_last_of("_");
			std::string strPlatformId = att.substr(posPlatformId + 1, std::string::npos);
			CT_BYTE cbPlatformId = (CT_BYTE)atoi(strPlatformId.c_str());

			std::string strMachineCount = it.second.c_str();
			CT_DWORD dwMachineUserCount = atoi(strMachineCount.c_str());
			InsertMachineOnlineRecord(db, cbPlatformId, dwMachineUserCount, cbMachineType);
		}
	}

	//写入总平台人数
	InsertOnlineRecord(db, 0, 0, 0, dwTotalHallUserCount, 0);
	//写入各个游戏的人数

	for (auto& it : mapGamePlatformUserCount)
	{
		CT_WORD wGameID = it.first / 100;
		CT_WORD wKindID = it.first % 100;
		CT_DWORD dwTotalGameUserCount = 0;
		std::map<CT_BYTE, CT_DWORD>& mapPlatformUserCount = it.second;
		for (auto& itUserCount : mapPlatformUserCount)
		{
			dwTotalGameUserCount += itUserCount.second;

			//每个分平台的人数
			InsertOnlineRecord(db, itUserCount.first, wGameID, wKindID, itUserCount.second, 2);
		}
		//总平台的游戏人数
		InsertOnlineRecord(db, 0, wGameID, wKindID, dwTotalGameUserCount, 2);
	}

	pool->put(db);
}

CT_BOOL DBThread::IsAcrossTheDay()
{
	//UTC时间+8小时，等于东八区时间
	unsigned int time1 = m_dwTodayTime + 28800;
	unsigned int time2 = Utility::GetTime() + 28800;

	int today = time1 / 86400;
	int Now = time2 / 86400;

	if (today == Now)
	{
		return false;
	}
	else
	{
	 	//LOG(WARNING) << "not the same day.";
		m_dwTodayTime = time2;
		return true;
	}

	/*LOG(WARNING) << "check the same day.";
	struct timezone tvTimeZone;
	gettimeofday(NULL, &tvTimeZone);
	int iTimeZoneSeconds = (-tvTimeZone.tz_minuteswest*60);
	int iDayOld = static_cast<int>((time1+iTimeZoneSeconds)/DAY_24HOUR_SECOND);
	int iDayNew = static_cast<int>((time2+iTimeZoneSeconds)/DAY_24HOUR_SECOND);
	if(iDayNew > iDayOld)
	{
		m_dwTodayTime = time2;
		LOG(WARNING) << "IS the same day.";
		return true;
	}
	else
	{
		return false;
	}*/
}

CT_VOID DBThread::AcrossTheDayEvent()
{
	CT_BOOL bAcrossDay = IsAcrossTheDay();
	if (bAcrossDay)
	{
		//跟踪每天凌晨db操作很慢的原因
		LOG(WARNING) << "start  AcrossTheDayEvent...";
		CT_DWORD  dwTime1 = Utility::GetTick();

		SwitchTodayScoreRank();
		CT_DWORD  dwTime2 = Utility::GetTick();
		LOG(WARNING) << "SwitchTodayScoreRank use time : " << dwTime2 - dwTime1;
		dwTime1 = dwTime2;

		ResetTodayScoreRankInCache();
		dwTime2 = Utility::GetTick();
		LOG(WARNING) << "ResetTodayScoreRankInCache use time : " << dwTime2 - dwTime1;
		dwTime1 = dwTime2;

        ResetTodayRank();
		dwTime2 = Utility::GetTick();
		LOG(WARNING) << "ResetTodayRank use time : " << dwTime2 - dwTime1;
		std::cout << std::endl;
		//m_bNeedLoadNewShareRank = true;
		//LoadSignInRank();
	}

	//LOG(WARNING) << "AcrossTheDayEvent use time : " << Utility::GetTick() - dwTime1;
}

CT_VOID DBThread::UpdateUserGem(acl::db_handle* db, CT_DWORD dwUserID, CT_INT32 nAddGem)
{
	acl::query query;
	query.create_sql("update userinfo set gem = gem + :gemval where userid = :useridval").set_format("gemval", "%d", nAddGem).set_format("useridval", "%u", dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "add db gem fail, add gem: "<< nAddGem <<", user id: " << dwUserID;
		db->free_result();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::InsertGemConsumeRecord(acl::db_handle* db, MSG_G2DB_Gem_Consume* pConsume)
{
	acl::query query;
	query.create_sql("insert into record_room_gem(gameid, kindid,roomid, userid, gem, remain, fixdate) values(:gameid, :kindid, :roomid, :userid, :gem, :remain, UNIX_TIMESTAMP(NOW()))")
		.set_format("gameid", "%u", pConsume->wGameID)
		.set_format("kindid", "%u", pConsume->wKindID)
		.set_format("roomid", "%u", pConsume->dwRoomNum)
		.set_format("userid", "%u", pConsume->dwUserID)
		.set_format("gem", "%u", pConsume->dwConsumeGem)
		.set_format("remain", "%u", pConsume->dwRemainGem);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "exec sql fail:  " << query.to_string().c_str();
		db->free_result();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::InsertPRoomPlayCount(acl::db_handle* db, MSG_G2DB_PRoom_PlayCountRecord* pPlayCount)
{
	std::string date(pPlayCount->szEndTime);
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string table("insert into record_small_detail_");
	table.append(year);
	table.append(month);
	
	acl::query query;
	query.create_sql(" (id, sid, userid, chairid, score, nickname, fixdate) values(:id, :sid, :userid, :chairid, :score, :nickname, :fixdate)")
		.set_parameter("id", (CT_LONGLONG)pPlayCount->uGameRoundID)
		.set_parameter("sid", pPlayCount->wCurrPlayCount)
		.set_format("userid", "%u", pPlayCount->dwUserID)
		.set_format("chairid", "%u", pPlayCount->wChairID)
		.set_parameter("score", pPlayCount->llScore)
		.set_format("nickname", "%s", pPlayCount->szNickName)
		.set_format("fixdate", "%s", pPlayCount->szEndTime);
		
	table.append(query.to_string());

	/*std::ostringstream strSQL;
	strSQL << "insert into " << table << "(id, sid, userid, chairid, score, nickname, fixdate) values("
		<< pPlayCount->uGameRoundID << ", "
		<< pPlayCount->wCurrPlayCount << ", "
		<< pPlayCount->dwUserID << ", "
		<< pPlayCount->wChairID << ", "
		<< pPlayCount->dwScore << ", '"
		<< pPlayCount->szNickName << "', '"
		<< pPlayCount->szEndTime << "');";

	std::string sql = strSQL.str();*/
	
	if (db->sql_update(table.c_str()) == false)
	{
		LOG(WARNING) << "exec sql fail:" << table.c_str() << ", error: " << db->get_errno();
	}
	db->free_result();
}

CT_VOID DBThread::InsertPRoomGameRound(acl::db_handle* db, MSG_G2DB_PRoom_GameRoundRecord* pGameRound)
{
	std::string date(pGameRound->szEndTime);
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string table("insert into record_big_detail_");
	table += year;
	table += month;

	//插入每条记录
	acl::query query;
	for (int i = 0; i < pGameRound->uCount; ++i)
	{
		query.reset();
		query.create_sql(" (id, userid, nickname, score) values(:id, :userid, :nickname, :score)")
			.set_parameter("id", (CT_LONGLONG)pGameRound->uGameRoundID)
			.set_format("userid", "%u", pGameRound->dwArrUserID[i])
			.set_format("nickname", "%s", pGameRound->szNickname[i])
			.set_parameter("score", pGameRound->llArrScore[i]);

		std::string sql(table);
		sql.append(query.to_string());
		/*if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert proom game round fail," <<
				"game round id : " << pGameRound->uGameRoundID <<
				", user id: " << pGameRound->dwArrUserID[i] <<
				", score: " << pGameRound->llArrScore[i];
			return;
		}*/

		/*std::ostringstream strSQL;
		strSQL << "insert into " << table << "(id, userid, nickname, score) values("
			<< pGameRound->uGameRoundID << ", "
			<< pGameRound->dwArrUserID[i] << ", '"
			<< pGameRound->szNickname[i] << "', "
			<< pGameRound->llArrScore[i] << ");";

		std::string sql = strSQL.str();*/
		
		if (db->sql_update(sql.c_str()) == false)
		{
			LOG(WARNING) << "exec sql fail:" << sql.c_str() << ", error: " << db->get_errno();
		}

		db->free_result();
	}

	//更新gameround结束时间
	query.reset();
	query.create_sql("update record_big_score set endTime = :endTime where id= :id")
		//.set_date("endTime", time(NULL))
		.set_format("endTime", "%s", pGameRound->szEndTime)
		.set_parameter("id", (CT_LONGLONG)pGameRound->uGameRoundID);
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update record_big_score end time fail," <<
			"game round id : " << pGameRound->uGameRoundID;
		return;
	}
	db->free_result();
}

CT_VOID DBThread::InsertPRoomGameRoundCount(acl::db_handle* db, MSG_G2DB_PRoom_GameRoundCount* pGameRoundCount)
{
	acl::query query;
	if (pGameRoundCount->dwGameRoundCount == 1)
	{
		query.create_sql("insert into record_user_gameround(userid, gameid, kindid, gameRoundCount) values(:userid, :gameid, :kindid, :gameRoundCount)")
			.set_format("userid", "%u", pGameRoundCount->dwUserID)
			.set_format("gameid", "%u", pGameRoundCount->wGameID)
			.set_format("kindid", "%u", pGameRoundCount->wKindID)
			.set_format("gameRoundCount", "%u", pGameRoundCount->dwGameRoundCount);
	}
	else
	{
		query.create_sql("update record_user_gameround set gameRoundCount = :gameRoundCount where userid= :userid and gameid = :gameid and kindid = :kindid")
			.set_format("gameRoundCount", "%u", pGameRoundCount->dwGameRoundCount)
			.set_format("gameid", "%u", pGameRoundCount->wGameID)
			.set_format("kindid", "%u", pGameRoundCount->wKindID)
			.set_format("userid", "%u", pGameRoundCount->dwUserID);
	}

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update record_user_gameround fail," << "user id: " << pGameRoundCount->dwUserID
			<< ", game id: " << pGameRoundCount->wGameID
			<< ", kind id: " << pGameRoundCount->wKindID
			<< ", game round count: " << pGameRoundCount->dwGameRoundCount
			<< ", error: " << db->get_errno();
		return;
	}
	db->free_result();

	//插入活动大局数字
	query.reset();
	query.create_sql("INSERT INTO record_user_lottery(userid, playcount) VALUES (:userid, 1) ON DUPLICATE KEY UPDATE playcount = playcount + 1")
	.set_format("userid", "%u", pGameRoundCount->dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update record_user_lottery fail," << "user id: " << pGameRoundCount->dwUserID << ", error: " << db->get_errno();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::InsertPlaybackRecord(acl::db_mysql* db, MSG_G2DB_PRoom_PlaybackRecord* pRecord)
{
	//if (db->begin_transaction() == false)
	//{
	//	LOG(WARNING) << "begin transaction false: " << db->get_error();
	//	return;
	//}

	std::string date(pRecord->szEndTime);
	if (date.empty())
	{
		date = Utility::GetTimeNowString();
	}
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string table("playbackdata_");
	table.append(year);
	table.append(month);

	std::ostringstream strSQL;
	strSQL << "insert into " << table << "(gameRoundId, playId, data) values (" << pRecord->uGameRoundID
		<< "," << pRecord->wPlayCount <<  ",'" << CCDBConnector::ConvertBinaryToString(db->get_conn(), pRecord->szBinary, pRecord->uBinaryLen) << "')";
	
	//必须生成一次std::string?
	std::string strSQL2 = strSQL.str();
	if (db->sql_update(strSQL2.c_str()) == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "insert playback record fail, error = " << nError << ", error info: " << db->get_error() <<", game round id: " << pRecord->uGameRoundID << ", play count: " << pRecord->wPlayCount;
		return;
	}

	//if (db->commit() == false)
	//{
	//	LOG(WARNING) << "commit error :" << db->get_errno();
	//	return;
	//}

	db->free_result();
}

CT_VOID DBThread::InsertWinLostRecord(acl::db_handle* db, MSG_G2DB_WinLostRecord* pWinLostRecord)
{
	acl::query query;
	for (int i = 0; i < pWinLostRecord->uCount; ++i)
	{
		query.reset();	
		query.create_sql("insert into stat_game_round(fixdate, gameid , userid, winCount, lostCount, drawCount, playTime) \
			values(UNIX_TIMESTAMP(NOW()), :gameid, :userid, :winCount, :lostCount, :drawCount, :playTime)")
			.set_format("gameid", "%u", pWinLostRecord->wGameID)
			.set_format("userid", "%u", pWinLostRecord->dwArrUserID[i])
			.set_format("winCount", "%d", pWinLostRecord->cbWinCount[i])
			.set_format("lostCount", "%d", pWinLostRecord->cbLostCount[i])
			.set_format("drawCount", "%d", pWinLostRecord->cbDrawCount[i])
			.set_format("playTime", "%u", pWinLostRecord->dwPlayTime);


		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert stat_game_round fail," << "user id: " << pWinLostRecord->dwArrUserID[i]
				<< ", game id: " << pWinLostRecord->wGameID
				<< ", userid: " << pWinLostRecord->dwArrUserID[i]
				<< ", winCount: " << pWinLostRecord->cbWinCount[i]
				<< ", lostCount: " << pWinLostRecord->cbLostCount[i]
				<< ", drawCount: " << pWinLostRecord->cbDrawCount[i]
				<< ", error: " << db->get_errno();
			continue;
		}
		db->free_result();
	}
}

CT_VOID DBThread::InsertGameRoundRecord(acl::aio_socket_stream*pSocket, acl::db_handle* db, MSG_G2DB_PRoom_InsertGameRound* pInsertGameRound)
{
	//if (db->begin_transaction() == false)
	//{
	//	LOG(WARNING) << "begin transaction false: " << db->get_error();
	//	return;
	//}

	acl::query query;
	query.create_sql("insert into record_big_score(gameid, kindid,userid,roomNum,param, startTime,endTime) values (:gameidval, :kindidval, :userid, :roomNumval, :param, :startTimeval, :endTimeval)")
		.set_format("gameidval", "%d", pInsertGameRound->wGameID)
		.set_format("kindidval", "%d", pInsertGameRound->wKindID)
		.set_format("userid", "%d", pInsertGameRound->dwUserID)
		.set_format("roomNumval", "%u", pInsertGameRound->dwRoomNum)
		.set_format("param", "%s", pInsertGameRound->szParam)
		.set_date("startTimeval", time(NULL))
		.set_date("endTimeval", time(NULL));


	if (db->exec_update(query) == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "insert game round record fail, error = " << nError;
		return;
	}

	MSG_DB2G_PRoom_InsertGameRound insertResult;
	insertResult.dwTableID = pInsertGameRound->dwTableID;
	db->free_result();
	if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "select game round id record fail, error = " << nError;
		return;
	}

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		insertResult.uGameRoundID = atoi((*row)["LAST_INSERT_ID()"]);
		LOG(INFO) << "insert game round id: " << insertResult.uGameRoundID;
	}

	if (db->length() == 0)
	{
		LOG(ERROR) << "get game round id fail! ";
	}

	//if (db->commit() == false)
	//{
	//	LOG(WARNING) << "commit error :" << db->get_errno();
	//	return;
	//}

	CNetModule::getSingleton().Send(pSocket, MSG_GDB_MAIN, SUB_DB2G_PROOM_INSERT_GAMEROUND, &insertResult, sizeof(insertResult));
	db->free_result();
}


CT_VOID DBThread::QueryPRoomGameRound(acl::aio_socket_stream*pSocket, acl::db_handle* db, MSG_P2DB_Query_PRoom_GameRound* pGameRound)
{
	CT_CHAR szSql[2048] = { 0 };
	/*_snprintf_info(szSql, sizeof(szSql), "SELECT m.id, m.userid, m.nickname, n.score, p.roomNum, p.endTime FROM\
		(\
			SELECT a.id, ifnull(group_concat(b.userid ORDER BY b.userid ASC), '')  AS userid,\
			ifnull(group_concat(b.nickname ORDER BY b.userid ASC SEPARATOR '/'), '')  AS nickname\
			FROM record.record_big_detail a INNER JOIN account.user b ON a.userid = b.userid INNER JOIN\
			(SELECT y.id FROM record.record_big_detail x INNER JOIN record.record_big_score y ON x.id = y.id\
				WHERE x.userid = %u) c\
			WHERE a.id = c.id\
			GROUP BY a.id\
			) m INNER  JOIN(\
				SELECT a.id, ifnull(group_concat(score ORDER BY userid ASC), '') AS score\
				FROM record.record_big_detail a INNER JOIN(SELECT id FROM record.record_big_detail WHERE userid = %u GROUP BY id ORDER BY id) b\
				WHERE a.id = b.id\
				GROUP BY a.id\
			) n ON m.id = n.id INNER JOIN record.record_big_score p ON m.id = p.id\
		ORDER BY m.id DESC LIMIT 10", pGameRound->dwUserID, pGameRound->dwUserID);*/

std::string date = Utility::GetTimeNowString();
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string thisMonth = year + month;
	std::string lastMonth = Utility::GetLastYearMonth(date);

	_snprintf_info(szSql, sizeof(szSql), "select * from (SELECT m.id, m.userid, m.nickname, n.score, p.roomNum, p.endTime FROM \
		(\
			SELECT a.id, ifnull(group_concat(a.userid ORDER BY a.userid ASC), '')  AS userid,\
			ifnull(group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/'), '')  AS nickname\
			FROM record.record_big_detail_%s a INNER JOIN \
			(SELECT y.id FROM record.record_big_detail_%s x INNER JOIN record.record_big_score y ON x.id = y.id  and gameid = %u and kindid = %u \
				WHERE x.userid = %u) c \
			WHERE a.id = c.id\
			GROUP BY a.id\
			) m INNER  JOIN(\
				SELECT a.id, ifnull(group_concat(score ORDER BY userid ASC), '') AS score\
				FROM record.record_big_detail_%s a INNER JOIN(SELECT id FROM record.record_big_detail_%s WHERE userid = %u GROUP BY id ORDER BY id) b\
				WHERE a.id = b.id\
				GROUP BY a.id\
			) n ON m.id = n.id INNER JOIN record.record_big_score p ON m.id = p.id \
		union \
		SELECT m.id, m.userid, m.nickname, n.score, p.roomNum, p.endTime FROM \
		(\
			SELECT a.id, ifnull(group_concat(a.userid ORDER BY a.userid ASC), '')  AS userid, \
			ifnull(group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/'), '')  AS nickname \
			FROM record.record_big_detail_%s a INNER JOIN \
			(SELECT y.id FROM record.record_big_detail_%s x INNER JOIN record.record_big_score y ON x.id = y.id  and gameid = %u and kindid = %u \
				WHERE x.userid = %u) c \
			WHERE a.id = c.id \
			GROUP BY a.id \
		) m INNER  JOIN( \
			SELECT a.id, ifnull(group_concat(score ORDER BY userid ASC), '') AS score \
			FROM record.record_big_detail_%s a INNER JOIN(SELECT id FROM record.record_big_detail_%s WHERE userid = %u GROUP BY id ORDER BY id) b \
			WHERE a.id = b.id \
			GROUP BY a.id \
		) n ON m.id = n.id INNER JOIN record.record_big_score p ON m.id = p.id \
	) x ORDER BY x.id DESC LIMIT 10;", lastMonth.c_str(), lastMonth.c_str(), pGameRound->wGameID, pGameRound->wKindID, pGameRound->dwUserID, lastMonth.c_str(), lastMonth.c_str(), pGameRound->dwUserID, \
		thisMonth.c_str(), thisMonth.c_str(), pGameRound->wGameID, pGameRound->wKindID, pGameRound->dwUserID, thisMonth.c_str(), thisMonth.c_str(), pGameRound->dwUserID);

	if (db->sql_select(szSql) == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "query proom game round fail, error = " << nError;
		return;
	}

	MSG_SC_Query_PRoom_Gameround gameRound;
	memset(&gameRound, 0, sizeof(gameRound));
	gameRound.dwMainID = MSG_QUERY_MAIN;
	gameRound.dwSubID = SUB_SC_QUERY_PROOM_GAMEROUND_INFO;
	gameRound.uValue1 = pGameRound->uClientAddr;

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];

#ifdef _OS_WIN32_CODE
		gameRound.uGameRoundID = _atoi64((*row)["id"]);
#else
		gameRound.uGameRoundID = atoll((*row)["id"]);
#endif

		std::string strUserID = (*row)["userid"];
		std::vector<std::string> vecUserID;
		Utility::stringSplit(strUserID, ",", vecUserID);
	
		std::string strNickName = (*row)["nickname"];
		std::vector<std::string> vecNickName;
		Utility::stringSplit(strNickName, "/", vecNickName);

		std::string strScore = (*row)["score"];
		std::vector<std::string> vecScore;
		Utility::stringSplit(strScore, ",", vecScore);

		if (vecUserID.size() != vecNickName.size() || vecUserID.size() != vecScore.size())
		{
			LOG(WARNING) << "query game round fail, data array not equal!";
 			continue;
		}

		gameRound.cbUserCount = (CT_BYTE)vecUserID.size();

		for (auto i = 0; i != (int)vecUserID.size(); ++i)
		{
			gameRound.arrUserID[i] = atoi(vecUserID[i].c_str());
			_snprintf_info(gameRound.arrNickname[i], sizeof(gameRound.arrNickname[i]), "%s", Utility::Ansi2Utf8(vecNickName[i]).c_str());
			gameRound.arrScore[i] = atoi(vecScore[i].c_str());
		}

		std::string strRoomNum = (*row)["roomNum"];
		gameRound.dwRoomNum = atoi(strRoomNum.c_str());

		std::string strEndTime = (*row)["endTime"];
		_snprintf_info(gameRound.szEndDate, sizeof(gameRound.szEndDate), "%s", strEndTime.substr(0, 10).c_str());
		_snprintf_info(gameRound.szEndTime, sizeof(gameRound.szEndTime), "%s", strEndTime.substr(11).c_str());

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, &gameRound, sizeof(gameRound));
	}

	MSG_GameMsgDownHead gameRoundFinish;
	gameRoundFinish.dwMainID = MSG_QUERY_MAIN;
	gameRoundFinish.dwSubID = SUB_SC_QUERY_PROOM_GAMEROUND_FINISH;
	gameRoundFinish.uValue1 = pGameRound->uClientAddr;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_NODATA, &gameRoundFinish, sizeof(gameRoundFinish));

	db->free_result();
}

CT_VOID DBThread::QueryPRoomPlayCount(acl::aio_socket_stream*pSocket, acl::db_handle* db, MSG_P2DB_Query_PRoom_PlayCount* pPlayCount)
{
	acl::query query;
	query.create_sql("select roomNum, endTime from record_big_score where id=:id")
		.set_parameter("id", (CT_LONGLONG)pPlayCount->uGameRoundID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query proom play count(roomnum) fail.";
		return;
	}

	MSG_SC_Query_PRoom_PlayCount playCountInfo;
	memset(&playCountInfo, 0, sizeof(playCountInfo));
	playCountInfo.uGameRoundID = pPlayCount->uGameRoundID;
	playCountInfo.uValue1 = pPlayCount->uClientAddr;
	playCountInfo.dwMainID = MSG_QUERY_MAIN;
	playCountInfo.dwSubID = SUB_SC_QUERY_PROOM_PLAYCOUNT_INFO;

	std::string gameRoundEndTime;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		std::string strRoomNum = (*row)["roomNum"];
		playCountInfo.dwRoomNum = atoi(strRoomNum.c_str());
		gameRoundEndTime = (*row)["endTime"];
		break;
	}
	std::string year = gameRoundEndTime.substr(0, 4);
	std::string month = gameRoundEndTime.substr(5, 2);
	std::string thisMonth = year + month;
	std::string lastMonth = Utility::GetLastYearMonth(gameRoundEndTime);

	db->free_result();

	//查询玩家椅子ID
	/*query.reset();
	query.create_sql("select chairid from record_small_detail where id=:id and sid=1 and userid = :userid")
		.set_parameter("id", (CT_LONGLONG)pPlayCount->uGameRoundID)
		.set_format("userid", "%u", pPlayCount->dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "query proom play count(user chairid) fail.";
		return;
	}*/

	std::string smallDetailTable("record_small_detail_");
	smallDetailTable.append(thisMonth);

	std::ostringstream strSQL;
	strSQL << "select chairid from " << smallDetailTable << " where id = " << pPlayCount->uGameRoundID << " and userid = " << pPlayCount->dwUserID << " LIMIT 1;";
	std::string sql = strSQL.str();
	if (db->sql_select(sql.c_str()) == false)
	{
		LOG(WARNING) << "exec sql fail:  " << sql.c_str() << " error: " << db->get_errno();
		return;
	}
	
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		std::string strChairid = (*row)["chairid"];
		playCountInfo.wSelfChairID = atoi(strChairid.c_str());
		break;
	}

	db->free_result();

	CT_CHAR szSql[2048] = { 0 };
#ifdef _OS_WIN32_CODE
	/*_snprintf_info(szSql, sizeof(szSql), "SELECT m.sid, m.userid, m.nickname, n.score, n.endTime FROM ( \
		SELECT a.sid, ifnull(group_concat(b.userid ORDER BY b.userid ASC), '')  AS userid, \
		ifnull(group_concat(b.nickname ORDER BY b.userid ASC SEPARATOR '/'), '')  AS nickname \
		FROM record.record_small_detail a INNER JOIN account.user b ON a.userid = b.userid INNER JOIN \
		(SELECT DISTINCT sid FROM record.record_small_detail WHERE id = %I64u) c ON a.sid = c.sid \
		WHERE a.id = %I64u \
		GROUP BY a.sid \
	) m INNER  JOIN( \
		SELECT a.sid, max(fixdate) as endTime, ifnull(group_concat(score ORDER BY userid ASC), '') AS score \
		FROM record.record_small_detail a INNER JOIN(SELECT DISTINCT sid FROM record.record_small_detail WHERE id = %I64u) b ON a.sid = b.sid \
		WHERE a.id = %I64u \
		GROUP BY a.sid \
	) n ON m.sid = n.sid \
		ORDER BY m.sid DESC", pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, pPlayCount->uGameRoundID);*/
	
	_snprintf_info(szSql, sizeof(szSql), "SELECT m.sid, m.userid, m.nickname, n.score, n.endTime \
		FROM(SELECT a.sid, group_concat(a.userid ORDER BY a.userid ASC) AS userid, group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/') AS nickname \
			FROM record.record_small_detail_%s a \
			INNER JOIN \
			(SELECT DISTINCT sid \
				FROM record.record_small_detail_%s \
				WHERE id = %I64u) c ON a.sid = c.sid \
			WHERE a.id = %I64u \
			GROUP BY a.sid) m INNER JOIN(SELECT a.sid, max(fixdate) AS endTime, group_concat(score ORDER BY userid ASC) AS score \
				FROM record.record_small_detail_%s a INNER JOIN(SELECT DISTINCT sid \
					FROM record.record_small_detail_%s \
					WHERE id = %I64u) b ON a.sid = b.sid \
				WHERE a.id = %I64u \
				GROUP BY a.sid) n ON m.sid = n.sid \
		UNION \
		SELECT m.sid, m.userid, m.nickname, n.score, n.endTime \
		FROM(SELECT a.sid, group_concat(a.userid ORDER BY a.userid ASC) AS userid, group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/') AS nickname \
			FROM record.record_small_detail_%s a \
			INNER JOIN \
			(SELECT DISTINCT sid \
				FROM record.record_small_detail_%s \
				WHERE id = %I64u) c ON a.sid = c.sid \
			WHERE a.id = %I64u \
			GROUP BY a.sid) m INNER JOIN(SELECT a.sid, max(fixdate) AS endTime, group_concat(score ORDER BY userid ASC) AS score \
				FROM record.record_small_detail_%s a INNER JOIN(SELECT DISTINCT sid \
					FROM record.record_small_detail_%s \
					WHERE id = %I64u) b ON a.sid = b.sid \
				WHERE a.id = %I64u  \
				GROUP BY a.sid) n ON m.sid = n.sid", lastMonth.c_str(), lastMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, lastMonth.c_str(), lastMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, \
		thisMonth.c_str(), thisMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, thisMonth.c_str(), thisMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID);

#elif defined(_OS_LINUX_CODE)
	_snprintf_info(szSql, sizeof(szSql), "SELECT m.sid, m.userid, m.nickname, n.score, n.endTime \
		FROM(SELECT a.sid, group_concat(a.userid ORDER BY a.userid ASC) AS userid, group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/') AS nickname \
			FROM record.record_small_detail_%s a \
			INNER JOIN \
			(SELECT DISTINCT sid \
				FROM record.record_small_detail_%s \
				WHERE id = %lu) c ON a.sid = c.sid \
			WHERE a.id = %lu \
			GROUP BY a.sid) m INNER JOIN(SELECT a.sid, max(fixdate) AS endTime, group_concat(score ORDER BY userid ASC) AS score \
				FROM record.record_small_detail_%s a INNER JOIN(SELECT DISTINCT sid \
					FROM record.record_small_detail_%s \
					WHERE id = %lu) b ON a.sid = b.sid \
				WHERE a.id = %lu \
				GROUP BY a.sid) n ON m.sid = n.sid \
		UNION \
		SELECT m.sid, m.userid, m.nickname, n.score, n.endTime \
		FROM(SELECT a.sid, group_concat(a.userid ORDER BY a.userid ASC) AS userid, group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/') AS nickname \
			FROM record.record_small_detail_%s a \
			INNER JOIN \
			(SELECT DISTINCT sid \
				FROM record.record_small_detail_%s \
				WHERE id = %lu) c ON a.sid = c.sid \
			WHERE a.id = %lu \
			GROUP BY a.sid) m INNER JOIN(SELECT a.sid, max(fixdate) AS endTime, group_concat(score ORDER BY userid ASC) AS score \
				FROM record.record_small_detail_%s a INNER JOIN(SELECT DISTINCT sid \
					FROM record.record_small_detail_%s \
					WHERE id = %lu) b ON a.sid = b.sid \
				WHERE a.id = %lu  \
				GROUP BY a.sid) n ON m.sid = n.sid", lastMonth.c_str(), lastMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, lastMonth.c_str(), lastMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, \
		thisMonth.c_str(), thisMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, thisMonth.c_str(), thisMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID);
#else
	_snprintf_info(szSql, sizeof(szSql), "SELECT m.sid, m.userid, m.nickname, n.score, n.endTime \
		FROM(SELECT a.sid, group_concat(a.userid ORDER BY a.userid ASC) AS userid, group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/') AS nickname \
			FROM record.record_small_detail_%s a \
			INNER JOIN \
			(SELECT DISTINCT sid \
				FROM record.record_small_detail_%s \
				WHERE id = %llu) c ON a.sid = c.sid \
			WHERE a.id = %llu \
			GROUP BY a.sid) m INNER JOIN(SELECT a.sid, max(fixdate) AS endTime, group_concat(score ORDER BY userid ASC) AS score \
				FROM record.record_small_detail_%s a INNER JOIN(SELECT DISTINCT sid \
					FROM record.record_small_detail_%s \
					WHERE id = %llu) b ON a.sid = b.sid \
				WHERE a.id = %llu \
				GROUP BY a.sid) n ON m.sid = n.sid \
		UNION \
		SELECT m.sid, m.userid, m.nickname, n.score, n.endTime \
		FROM(SELECT a.sid, group_concat(a.userid ORDER BY a.userid ASC) AS userid, group_concat(a.nickname ORDER BY a.userid ASC SEPARATOR '/') AS nickname \
			FROM record.record_small_detail_%s a \
			INNER JOIN \
			(SELECT DISTINCT sid \
				FROM record.record_small_detail_%s \
				WHERE id = %llu) c ON a.sid = c.sid \
			WHERE a.id = %llu \
			GROUP BY a.sid) m INNER JOIN(SELECT a.sid, max(fixdate) AS endTime, group_concat(score ORDER BY userid ASC) AS score \
				FROM record.record_small_detail_%s a INNER JOIN(SELECT DISTINCT sid \
					FROM record.record_small_detail_%s \
					WHERE id = %llu) b ON a.sid = b.sid \
				WHERE a.id = %llu  \
				GROUP BY a.sid) n ON m.sid = n.sid", lastMonth.c_str(), lastMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, lastMonth.c_str(), lastMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, \
		thisMonth.c_str(), thisMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID, thisMonth.c_str(), thisMonth.c_str(), pPlayCount->uGameRoundID, pPlayCount->uGameRoundID);
#endif

	if (db->sql_select(szSql) == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "query proom game play count info fail, error = " << nError;
		return;
	}


	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		playCountInfo.cbPlayCountID = atoi((*row)["sid"]);

		std::string strUserID = (*row)["userid"];
		std::vector<std::string> vecUserID;
		Utility::stringSplit(strUserID, ",", vecUserID);

		std::string strNickName = (*row)["nickname"];
		std::vector<std::string> vecNickName;
		Utility::stringSplit(strNickName, "/", vecNickName);

		std::string strScore = (*row)["score"];
		std::vector<std::string> vecScore;
		Utility::stringSplit(strScore, ",", vecScore);

		if (vecUserID.size() != vecNickName.size() || vecUserID.size() != vecScore.size())
		{
			LOG(WARNING) << "query game round fail, data array not equal!";
			continue;
		}
		playCountInfo.cbUserCount = (CT_BYTE)vecUserID.size();


		for (auto i = 0; i < (int)vecUserID.size(); ++i)
		{
			playCountInfo.arrUserID[i] = atoi(vecUserID[i].c_str());
			_snprintf_info(playCountInfo.arrNickname[i], sizeof(playCountInfo.arrNickname[i]), "%s", Utility::Ansi2Utf8(vecNickName[i]).c_str());
			playCountInfo.arrScore[i] = atoi(vecScore[i].c_str());
		}

		std::string strEndTime = (*row)["endTime"];
		_snprintf_info(playCountInfo.szEndDate, sizeof(playCountInfo.szEndDate), "%s", strEndTime.substr(0, 10).c_str());
		_snprintf_info(playCountInfo.szEndTime, sizeof(playCountInfo.szEndTime), "%s", strEndTime.substr(11).c_str());

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC, &playCountInfo, sizeof(playCountInfo));
	}

	MSG_GameMsgDownHead playCountFinish;
	playCountFinish.dwMainID = MSG_QUERY_MAIN;
	playCountFinish.dwSubID = SUB_SC_QUERY_PROOM_PLAYCOUNT_FINISH;
	playCountFinish.uValue1 = pPlayCount->uClientAddr;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_NODATA, &playCountFinish, sizeof(playCountFinish));

	db->free_result();
}

CT_VOID DBThread::InsertOnlineRecord(acl::db_handle* db, CT_BYTE cbPlatformId, CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwOnlineCount, CT_BYTE cbType)
{
	acl::query query;
	query.create_sql("insert into stat_game_online(platformId, gameid, kindid, statType, statValue, fixdate) values(:platformId, :gameid, :kindid, :statType, :statValue, :fixdate)")
			.set_format("platformId", "%d", (int)cbPlatformId)
	        .set_format("gameid", "%u", wGameID)
			.set_format("kindid", "%u", wKindID)
			.set_format("statType", "%d", (int)cbType)
			.set_format("statValue", "%u", dwOnlineCount)
			.set_format("fixdate", "%d", (CT_DWORD)time(NULL));
	
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert stat_game_online fail," << "game id: " << wGameID
			<< ", statType: " << (int)cbType << ", statValue:" << dwOnlineCount << ", error no: " << db->get_errno() << ", error msg: " << db->get_error();
		return;
	}

	//LOG(WARNING) << "insert online record: " <<  query.to_string().c_str();
	db->free_result();
}

CT_VOID DBThread::InsertMachineOnlineRecord(acl::db_handle* db, CT_BYTE cbPlatformId, CT_DWORD dwOnlineCount, CT_BYTE cbType)
{
	acl::query query;
	query.create_sql("insert into stat_machine_online(platformId, machineType, statValue, fixdate) values(:platformId, :machineType, :statValue, :fixdate)")
		.set_format("platformId", "%d", (int)cbPlatformId)
		.set_format("machineType", "%d", (int)cbType)
		.set_format("statValue", "%u", dwOnlineCount)
		.set_format("fixdate", "%d", (CT_DWORD)time(NULL));

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert stat_machine_online fail," << "machine types: " << (int)cbType << ", statValue:" << dwOnlineCount 
			<< ", error no: " << db->get_errno() << ", error msg: " << db->get_error();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::LoadGroupChatMsg()
{
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }

    do
    {
        //读取最大的消息ID
        acl::query query;
        query.create_sql("SELECT * FROM group_chat ORDER BY msgId DESC LIMIT 1");

        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "select group chat id fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            break;
        }

        CT_DWORD dwGroupMsgId = 0;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row *row = (*db)[i];
            dwGroupMsgId = (CT_DWORD) atoi((*row)["msgId"]);
        }
        db->free_result();

        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_CHAT_ID, &dwGroupMsgId,
                                            sizeof(CT_DWORD));
        }

        //未读消息(默认读取10天内没有读取的消息)
        query.reset();
        query.create_sql("SELECT * FROM group_chat WHERE `status` = 0 and time > unix_timestamp(subdate(now(), interval 10 day))");

        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "select group unread chat fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            break;
        }

        std::map<CT_DWORD, std::vector<tagGroupChat>> mapGroupChat;
        tagGroupChat groupChat;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row *row = (*db)[i];
            groupChat.dwMsgId = (CT_DWORD) atoi((*row)["msgId"]);
            CT_DWORD dwGroupID = (CT_DWORD)atoi((*row)["groupId"]);
            groupChat.dwFromUserID = (CT_DWORD) atoi((*row)["fromUserId"]);
            groupChat.dwToUserID = (CT_DWORD) atoi((*row)["toUserId"]);
            groupChat.dwSendTime = (CT_DWORD) atoi((*row)["time"]);
            _snprintf_info(groupChat.szChat, sizeof(groupChat.szChat), "%s", (*row)["chat"]);
            mapGroupChat[dwGroupID].push_back(groupChat);
        }
        db->free_result();

        CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
        CT_DWORD dwSendSize = 0;
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            for (auto& it : mapGroupChat)
            {
                memcpy(szBuffer, &it.first, sizeof(CT_DWORD));
                dwSendSize += sizeof(CT_DWORD);
                std::vector<tagGroupChat>& vecGroupChat = it.second;

                for (auto& itChat : vecGroupChat)
                {
                    if ((dwSendSize + sizeof(tagGroupChat)) > sizeof(szBuffer))
                    {
                        CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_UNREAD_CHAT, szBuffer, dwSendSize);
                        dwSendSize = 0;
                        memcpy(szBuffer, &it.first, sizeof(CT_DWORD));
                        dwSendSize += sizeof(CT_DWORD);
                    }

                    memcpy(szBuffer + dwSendSize, &itChat, sizeof(tagGroupChat));
                    dwSendSize += sizeof(tagGroupChat);
                }
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_UNREAD_CHAT, szBuffer, dwSendSize);
            }
        }

    } while (0);

    pool->put(db);
}

CT_DWORD DBThread::CalcDayInt(int nBeforeDays)
{
    /* 取得N天前的日期 */
    time_t          t;
    struct tm      *m;

    t = time(NULL) - 24 * 3600 * nBeforeDays;
    m = localtime(&t);
    unsigned int nIntTime = (m->tm_year + 1900)*10000+(m->tm_mon+1)*100+m->tm_mday;

    return nIntTime;
}

CT_VOID DBThread::LoadGroupInfo()
{
	acl::db_pool* pool = GetAccountdbPool();
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

	std::map<CT_DWORD, tagGroupInfo> mapGroupInfo;
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM `group`");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select group fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		tagGroupInfo groupInfo;
		memset(&groupInfo, 0, sizeof(groupInfo));
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			groupInfo.dwGroupID = (CT_DWORD)atoi((*row)["id"]);
			groupInfo.dwMasterID = (CT_DWORD)atoi((*row)["masterId"]);
			//groupInfo.dwDeputyID1 = (CT_DWORD)atoi((*row)["deputyId1"]);
            //groupInfo.dwDeputyID2 = (CT_DWORD)atoi((*row)["deputyId2"]);
			groupInfo.dwContribution = (CT_DWORD)atoi((*row)["contribution"]);
			groupInfo.cbIncomeRate = (CT_BYTE)atoi((*row)["incomeRate"]);
			groupInfo.cbSettleDays = (CT_BYTE)atoi((*row)["settleDays"]);
            groupInfo.cbIcon = (CT_BYTE)atoi((*row)["icon"]);
            groupInfo.cbLevel = (CT_BYTE)atoi((*row)["level"]);
			std::string strGroupName = (*row)["groupName"];
			std::string strNotice = (*row)["notice"];
			_snprintf_info(groupInfo.szGroupName, sizeof(groupInfo.szGroupName), "%s", Utility::Ansi2Utf8(strGroupName).c_str());
			_snprintf_info(groupInfo.szNotice, sizeof(groupInfo.szNotice), "%s", Utility::Ansi2Utf8(strNotice).c_str());

			mapGroupInfo.insert(std::make_pair(groupInfo.dwGroupID, groupInfo));
		}
		db->free_result();

		//发送群组信息
		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		CT_DWORD dwSendSize = 0;

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (!pCenterInfo || !pCenterInfo->pCenterSocket)
		{
			break;
		}

		for (auto&it : mapGroupInfo)
		{
			//发送数据
			if ((dwSendSize + sizeof(tagGroupInfo)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_INFO, szBuffer, dwSendSize);
				dwSendSize = 0;
			}

			memcpy(szBuffer + dwSendSize, &it.second, sizeof(tagGroupInfo));
			dwSendSize += sizeof(tagGroupInfo);
		}
		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_INFO, szBuffer, dwSendSize);
		}

		//读取群组信息
		std::map<CT_DWORD, std::vector<tagGroupUserInfo>> mapGroupUser;
		for (auto& it : mapGroupInfo)
		{
			query.reset();
			query.create_sql("select a.*, b.`gender`, b.`headId`, b.`regdate`, b.`lastdate`, c.`vip2`, c.`pay` from `groupuser` a "
					"INNER JOIN `userinfo` b on a.`userId` = b.`userid` INNER JOIN `userdata` c on a.`userId` = c.`userId` "
	                " where a.`groupId` = :gid ORDER BY a.date")
	                .set_format("gid", "%d", it.first);

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << "select group user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				break;
			}

			std::vector<tagGroupUserInfo> vecGroupUser;
			for (size_t i = 0; i < db->length(); i++)
			{
				tagGroupUserInfo groupUser;
				const acl::db_row* row = (*db)[i];
				groupUser.dwUserID = (CT_DWORD)atoi((*row)["userId"]);
                groupUser.dwParentUserId = (CT_DWORD)atoi((*row)["parentUserId"]);
                groupUser.cbSex = (CT_BYTE)atoi((*row)["gender"]);
				groupUser.cbHeadId = (CT_BYTE)atoi((*row)["headId"]);
				groupUser.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
				groupUser.cbMemLevel = (CT_BYTE)atoi((*row)["memlevel"]);
                groupUser.cbInComeRate = (CT_BYTE)atoi((*row)["incomeRate"]);
                groupUser.wDeductRate = (CT_WORD)atoi((*row)["deductRate"]);
                groupUser.wDeductTop = (CT_WORD)atoi((*row)["deductTop"]);
                groupUser.wDeductIncre = (CT_WORD)atoi((*row)["deductIncre"]);
                groupUser.dwDeductStart = (CT_DWORD)atoi((*row)["deductStart"]);
                groupUser.dwLastTakeIncomeDate = (CT_DWORD)atoi((*row)["lastTakeIncomeDate"]);
                groupUser.dwLastIncomeDate = (CT_DWORD)atoi((*row)["lastIncomeDate"]);
                groupUser.wLinkIncomeDays = (CT_WORD)atoi((*row)["linkIncomeDays"]);
                groupUser.llTotalIncome = atoll((*row)["totalIncome"]);;
                groupUser.dwRecharge = (CT_DWORD)atoi((*row)["pay"]);
                groupUser.dwRegDate = (CT_DWORD)atoi((*row)["regdate"]);
                groupUser.dwLastDate = (CT_DWORD)atoi((*row)["lastDate"]);
                groupUser.bOnline = false;
                _snprintf_info(groupUser.szRemarks, sizeof(groupUser.szRemarks), "%s", (*row)["remarks"]);
                vecGroupUser.push_back(groupUser);
			}
			mapGroupUser.insert(std::make_pair(it.first, vecGroupUser));

			db->free_result();
		}

		//发送群组成员信息
		dwSendSize = 0;
		for (auto& it : mapGroupUser)
		{
			std::vector<tagGroupUserInfo>& vecGroupUser = it.second;
			memcpy(szBuffer, &it.first, sizeof(CT_DWORD));
			dwSendSize += sizeof(CT_DWORD);
			for (auto& itSet : vecGroupUser)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagGroupUserInfo)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER, szBuffer, dwSendSize);
					dwSendSize = 0;
					memcpy(szBuffer, &it.first, sizeof(CT_DWORD));
					dwSendSize += sizeof(CT_DWORD);
				}

				memcpy(szBuffer + dwSendSize, &itSet, sizeof(tagGroupUserInfo));
				dwSendSize += sizeof(tagGroupUserInfo);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER, szBuffer, dwSendSize);
				dwSendSize = 0;
			}
		}
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_INFO_FINISH);

		//读取成员的结算信息
        std::map<CT_DWORD, std::vector<tagGroupUserDateIncomeDetail>> mapGroupUserDirIncomeDetail;
        std::map<CT_DWORD, std::vector<tagGroupUserDateIncomeDetail>> mapGroupUserSubIncomeDetail;
        std::map<CT_DWORD, std::vector<tagGroupUserTotalIncome>>      mapGroupUserTotalIncome;
        CT_DWORD dwIntYesterDay = CalcDayInt(1);
        for (auto& it : mapGroupUser)
        {
            std::vector<tagGroupUserInfo>& vecGroupUserInfo = it.second;
            for (auto& itVecGroupUser : vecGroupUserInfo)
            {
                query.reset();
                CT_DWORD dwLastDate = itVecGroupUser.dwLastTakeIncomeDate;
                if (dwLastDate != 0)
                {
                    dwLastDate = dwLastDate > dwIntYesterDay ? dwIntYesterDay : itVecGroupUser.dwLastTakeIncomeDate;
                }
                query.create_sql("SELECT * from record.group_dir_income WHERE dateInt >= :dateInt AND userId = :userId")
                .set_format("dateInt", "%u", dwLastDate)
                .set_format("userId", "%u", itVecGroupUser.dwUserID);

                if (db->exec_select(query) == false)
                {
                    LOG(WARNING) << "select group user dir income fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
                    break;
                }
                for (size_t i = 0; i < db->length(); i++)
                {
                    std::vector<tagGroupUserDateIncomeDetail>& vecGroupUserDateIncome = mapGroupUserDirIncomeDetail[it.first];

                    tagGroupUserDateIncomeDetail groupUserDateIncomeDetail;
                    const acl::db_row* row = (*db)[i];
                    groupUserDateIncomeDetail.dwDateInt = (CT_DWORD)atoi((*row)["dateInt"]);
                    groupUserDateIncomeDetail.dwUserID = (CT_DWORD)atoi((*row)["userId"]);
                    groupUserDateIncomeDetail.dwSubUserID = (CT_DWORD)atoi((*row)["dirUserId"]);
                    groupUserDateIncomeDetail.dwOriginalIncome = (CT_DWORD)atoi((*row)["originalIncome"]);
                    groupUserDateIncomeDetail.dwDeductIncome = (CT_DWORD)atoi((*row)["deductIncome"]);
                    groupUserDateIncomeDetail.wDeductRate = (CT_WORD)atoi((*row)["deductRate"]);
                    vecGroupUserDateIncome.push_back(groupUserDateIncomeDetail);
                }

                query.reset();
                query.create_sql("SELECT * from record.group_sub_income WHERE dateInt >= :dateInt AND userId = :userId")
                    .set_format("dateInt", "%u", dwLastDate)
                    .set_format("userId", "%u", itVecGroupUser.dwUserID);

                if (db->exec_select(query) == false)
                {
                    LOG(WARNING) << "select group user sub income fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
                    break;
                }
                for (size_t i = 0; i < db->length(); i++)
                {
                    std::vector<tagGroupUserDateIncomeDetail>& vecGroupUserDateIncome = mapGroupUserSubIncomeDetail[it.first];

                    tagGroupUserDateIncomeDetail groupUserDateIncomeDetail;
                    const acl::db_row* row = (*db)[i];
                    groupUserDateIncomeDetail.dwDateInt = (CT_DWORD)atoi((*row)["dateInt"]);
                    groupUserDateIncomeDetail.dwUserID = (CT_DWORD)atoi((*row)["userId"]);
                    groupUserDateIncomeDetail.dwSubUserID = (CT_DWORD)atoi((*row)["subUserId"]);
                    groupUserDateIncomeDetail.dwOriginalIncome = (CT_DWORD)atoi((*row)["originalIncome"]);
                    groupUserDateIncomeDetail.dwDeductIncome = (CT_DWORD)atoi((*row)["deductIncome"]);
                    groupUserDateIncomeDetail.wDeductRate = (CT_WORD)atoi((*row)["deductRate"]);
                    vecGroupUserDateIncome.push_back(groupUserDateIncomeDetail);
                }

                query.reset();
                query.create_sql("SELECT * from record.group_total_income WHERE userId = :userId")
                .set_format("userId", "%u", itVecGroupUser.dwUserID);

                if (db->exec_select(query) == false)
                {
                    LOG(WARNING) << "select group user total income fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
                    break;
                }
                for (size_t i = 0; i < db->length(); i++)
                {
                    std::vector<tagGroupUserTotalIncome>& vecGroupUserTotalIncome = mapGroupUserTotalIncome[it.first];

                    tagGroupUserTotalIncome groupUserTotalIncome;
                    const acl::db_row* row = (*db)[i];
                    groupUserTotalIncome.dwUserID = (CT_DWORD)atoi((*row)["userId"]);
                    groupUserTotalIncome.dwSubUserID = (CT_DWORD)atoi((*row)["subUserId"]);
                    groupUserTotalIncome.llTotalIncome = atoll((*row)["totalIncome"]);
                    vecGroupUserTotalIncome.push_back(groupUserTotalIncome);
                }

            }
        }

        //发送直属成员收入
        dwSendSize = 0;
        for (auto& itMapDirIncome : mapGroupUserDirIncomeDetail)
        {
            memcpy(szBuffer, &itMapDirIncome.first, sizeof(CT_DWORD));
            dwSendSize += sizeof(CT_DWORD);

            std::vector<tagGroupUserDateIncomeDetail>& vecGroupUserDirIncome = itMapDirIncome.second;
            for (auto& itVecDirIncome : vecGroupUserDirIncome)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagGroupUserDateIncomeDetail)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_DIR_INCOME, szBuffer, dwSendSize);
                    dwSendSize = 0;
                    memcpy(szBuffer, &itMapDirIncome, sizeof(CT_DWORD));
                    dwSendSize += sizeof(CT_DWORD);
                }

                memcpy(szBuffer + dwSendSize, &itVecDirIncome, sizeof(tagGroupUserDateIncomeDetail));
                dwSendSize += sizeof(tagGroupUserDateIncomeDetail);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_DIR_INCOME, szBuffer, dwSendSize);
                dwSendSize = 0;
            }
        }

        //发送附属成员收入
        dwSendSize = 0;
        for (auto& itMapSubIncome : mapGroupUserSubIncomeDetail)
        {
            memcpy(szBuffer, &itMapSubIncome.first, sizeof(CT_DWORD));
            dwSendSize += sizeof(CT_DWORD);

            std::vector<tagGroupUserDateIncomeDetail>& vecGroupUserSubIncome = itMapSubIncome.second;
            for (auto& itVecSubIncome : vecGroupUserSubIncome)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagGroupUserDateIncomeDetail)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_SUB_INCOME, szBuffer, dwSendSize);
                    dwSendSize = 0;
                    memcpy(szBuffer, &itMapSubIncome, sizeof(CT_DWORD));
                    dwSendSize += sizeof(CT_DWORD);
                }

                memcpy(szBuffer + dwSendSize, &itVecSubIncome, sizeof(tagGroupUserDateIncomeDetail));
                dwSendSize += sizeof(tagGroupUserDateIncomeDetail);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_SUB_INCOME, szBuffer, dwSendSize);
                dwSendSize = 0;
            }
        }

        //总收入
        for (auto& itMapTotalIncome : mapGroupUserTotalIncome)
        {
            memcpy(szBuffer, &itMapTotalIncome.first, sizeof(CT_DWORD));
            dwSendSize += sizeof(CT_DWORD);

            std::vector<tagGroupUserTotalIncome>& vecGroupUserTotalIncome = itMapTotalIncome.second;
            for (auto& itVecTotalIncome : vecGroupUserTotalIncome)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagGroupUserTotalIncome)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_TOTAL_INCOME, szBuffer, dwSendSize);
                    dwSendSize = 0;
                    memcpy(szBuffer, &itMapTotalIncome, sizeof(CT_DWORD));
                    dwSendSize += sizeof(CT_DWORD);
                }

                memcpy(szBuffer + dwSendSize, &itVecTotalIncome, sizeof(tagGroupUserTotalIncome));
                dwSendSize += sizeof(tagGroupUserTotalIncome);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_TOTAL_INCOME, szBuffer, dwSendSize);
                dwSendSize = 0;
            }
        }
    }while (0);

	pool->put(db);
}

CT_VOID DBThread::LoadGroupUserRevenue()
{
    acl::db_pool* pool = GetAccountdbPool();
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

    std::map<CT_DWORD, std::map<CT_DWORD, CT_DWORD>> mapGroupUserRevenue;
    do
    {
        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (!pCenterInfo || !pCenterInfo->pCenterSocket)
        {
            break;
        }

        acl::query query;
        query.create_sql("SELECT a.groupId, a.userId, b.revenue FROM account.groupuser a INNER JOIN record.src_user_today b ON a.userId = b.userid WHERE b.writeDate = CURDATE()");
        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "LoadGroupUserRevenue fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            break;
        }

        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row *row = (*db)[i];
            CT_DWORD dwGroupID = (CT_DWORD)atoi((*row)["groupId"]);
            CT_DWORD dwUserID = (CT_DWORD)atoi((*row)["userId"]);
            CT_DWORD dwRevenue = (CT_DWORD)atoi((*row)["revenue"]);

            std::map<CT_DWORD, CT_DWORD>& mapUserRevenue = mapGroupUserRevenue[dwGroupID];
            auto itUser = mapUserRevenue.find(dwUserID);
            if (itUser == mapUserRevenue.end())
            {
                mapUserRevenue.insert(std::make_pair(dwUserID, dwRevenue));
            }
            else
            {
                itUser->second += dwRevenue;
            }
        }

        CT_BYTE     szBuffer[SYS_NET_RECVBUF_SIZE];
        CT_DWORD    dwSendSize = 0;
        tagGroupUserRevenue userRevenue;
        for (auto& itGroupUserRevenue : mapGroupUserRevenue)
        {
            CT_DWORD    dwGroupID = itGroupUserRevenue.first;
            memcpy(szBuffer, &dwGroupID, sizeof(CT_DWORD));
            dwSendSize += sizeof(CT_DWORD);

            std::map<CT_DWORD, CT_DWORD>& mapUserRevenue = itGroupUserRevenue.second;
            for (auto& itUser : mapUserRevenue)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagGroupUserRevenue)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_REVENUE, szBuffer, dwSendSize);
                    dwSendSize = 0;
                    memcpy(szBuffer, &dwGroupID, sizeof(CT_DWORD));
                    dwSendSize += sizeof(CT_DWORD);
                }

                userRevenue.dwUserID = itUser.first;
                userRevenue.dwRevenue = itUser.second;
                memcpy(szBuffer + dwSendSize, &userRevenue, sizeof(tagGroupUserRevenue));
                dwSendSize += sizeof(tagGroupUserRevenue);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_USER_REVENUE, szBuffer, dwSendSize);
                dwSendSize = 0;
            }
        }
    }while (0);

    pool->put(db);
}

CT_VOID DBThread::LoadGroupLevelCfg()
{
	acl::db_pool* pool = GetPlatformdbPool();
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

	do
	{
		acl::query query1;
		query1.create_sql("SELECT * FROM group_level_config");

		if (db->exec_select(query1) == false)
		{
			LOG(WARNING) << "LoadGroupLevelCfg fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		std::vector<tagGroupLevelCfg> vecGroupLevelCfg;
		for (size_t i = 0; i < db->length(); i++)
		{
			tagGroupLevelCfg groupLevelCfg;
			const acl::db_row *row = (*db)[i];
			groupLevelCfg.cbLevel = (CT_BYTE) atoi((*row)["level"]);
            groupLevelCfg.wIncomeRate = (CT_WORD) atoi((*row)["incomeRate"]);
            groupLevelCfg.wDeductRate = (CT_WORD) atoi((*row)["DeductRate"]);
            groupLevelCfg.wDeductTop = (CT_WORD) atoi((*row)["DeductTop"]);
            groupLevelCfg.wDeductIncre = (CT_WORD) atoi((*row)["DeductIncre"]);
            groupLevelCfg.dwDeductStart = (CT_WORD) atoi((*row)["DeductStart"]);
			groupLevelCfg.cbSettleDays = (CT_BYTE)atoi((*row)["settleDays"]);
            groupLevelCfg.dwMaxContribution = (CT_DWORD)atoi((*row)["maxContribution"]);
            groupLevelCfg.dwSettleMinIncome = (CT_DWORD)atoi((*row)["settleMinIncome"]);
            groupLevelCfg.dwSettleMaxIncome = (CT_DWORD)atoi((*row)["settleMaxIncome"]);
            _snprintf_info(groupLevelCfg.szDomain, sizeof(groupLevelCfg.szDomain), "%s", (*row)["domain"]);
            vecGroupLevelCfg.push_back(groupLevelCfg);
		}

		//发送群组信息
		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		CT_DWORD dwSendSize = 0;

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (!pCenterInfo || !pCenterInfo->pCenterSocket)
		{
			break;
		}

		for (auto&it : vecGroupLevelCfg)
		{
			memcpy(szBuffer + dwSendSize, &it, sizeof(tagGroupLevelCfg));
			dwSendSize += sizeof(tagGroupLevelCfg);
		}

		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_LEVEL_CFG, szBuffer, dwSendSize);
	}while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::QueryGroupPRoomConfig(acl::aio_socket_stream* pSocket, acl::db_handle* db)
{
	acl::query query;
	query.create_sql("select * from robotConfig");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select group default room config fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::vector<tagDefaultRoomConfig> vecDefaultRoomConfig;
	tagDefaultRoomConfig defaultConfig;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		defaultConfig.wGameID = atoi((*row)["gameid"]);
		defaultConfig.wKindID = atoi((*row)["kindid"]);
		std::string strConfig = (*row)["config"];
		_snprintf_info(defaultConfig.szConfig, sizeof(defaultConfig.szConfig), "%s", Utility::Ansi2Utf8(strConfig).c_str());

		vecDefaultRoomConfig.push_back(defaultConfig);
	}
	db->free_result();

	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
	CT_DWORD dwSendSize = 0;
	for (auto& it : vecDefaultRoomConfig)
	{
		//发送数据
		if ((dwSendSize + sizeof(tagDefaultRoomConfig)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_DEFAULT_PROOM_CONFIG, szBuffer, dwSendSize);
			dwSendSize = 0;
		}

		memcpy(szBuffer + dwSendSize, &it, sizeof(tagDefaultRoomConfig));
		dwSendSize += sizeof(tagDefaultRoomConfig);

		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_DEFAULT_PROOM_CONFIG, szBuffer, dwSendSize);
		}
	}

	query.reset();

	query.create_sql("SELECT * FROM roomConfig");
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select group room config fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::vector<tagGroupRoomConfig> vecRoomConfig;
	tagGroupRoomConfig groupRoomConfig;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		groupRoomConfig.dwGroupID = atoi((*row)["gid"]);
		groupRoomConfig.wGameID = atoi((*row)["gameid"]);
		groupRoomConfig.wKindID = atoi((*row)["kindid"]);
		std::string strConfig = (*row)["config"];
		_snprintf_info(groupRoomConfig.szConfig, sizeof(groupRoomConfig.szConfig), "%s", Utility::Ansi2Utf8(strConfig).c_str());

		vecRoomConfig.push_back(groupRoomConfig);
	}
	db->free_result();

	dwSendSize = 0;
	for (auto& it : vecRoomConfig)
	{
		//发送数据
		if ((dwSendSize + sizeof(tagGroupRoomConfig)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_PROOM_CONFIG, szBuffer, dwSendSize);
			dwSendSize = 0;
		}

		memcpy(szBuffer + dwSendSize, &it, sizeof(tagGroupRoomConfig));
		dwSendSize += sizeof(tagGroupRoomConfig);

		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_PROOM_CONFIG, szBuffer, dwSendSize);
		}
	}
}

CT_VOID DBThread::SetGroupPRoomState(acl::db_handle* db, MSG_CS2DB_Return_GroupPRoom_Gem* pReturnGem)
{
	acl::query query;
	query.create_sql("update roomSale set status = 2 where id = :id and gid = :gid")
		.set_format("id", "%d", pReturnGem->dwRecordID)
		.set_format("gid", "%d", pReturnGem->dwGroupID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update roomSale status fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::ReturnGroupPRoomGem(acl::db_handle* db, MSG_CS2DB_Return_GroupPRoom_Gem* pReturnGem)
	{
	acl::query query;
	query.create_sql("update userinfo set gem = gem + :gem where userid = :userid")
		.set_format("gem", "%u", pReturnGem->wReturnGem)
		.set_format("userid", "%u", pReturnGem->dwMasterID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "return group proom gem fail, return gem: " << pReturnGem->wReturnGem << ", master id: " << pReturnGem->dwMasterID;
		db->free_result();
		return;
	}
}

CT_VOID DBThread::InsertGroupPRoomRecord(acl::db_handle* db, MSG_G2DB_Record_GroupPRoom* pAutoPRoom)
{
	acl::query query;
	query.create_sql("insert into record_group_room(rid, gid, roundid, fixdate) values(:rid, :gid, :roundid, UNIX_TIMESTAMP(NOW()))")
		.set_format("rid", "%u", pAutoPRoom->dwRecordID)
		.set_format("gid", "%u", pAutoPRoom->dwGroupID)
#ifdef _OS_MAC_CODE
        .set_format("roundid", "%llu", pAutoPRoom->uGameRoundID);
#else
		.set_format("roundid", "%lu", pAutoPRoom->uGameRoundID);
#endif

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert record group room fail. errno:  " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::InsertGroupPRoomInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_GroupRoom_Info* pGroupRoomInfo)
{
	acl::query query;
	query.create_sql("insert into roomSale(gid, gameid, kindid, gem, roomid, fixdate) values(:gid, :gameid, :kindid, :gem, :roomid, UNIX_TIMESTAMP(NOW()))")
		.set_format("gid", "%u", pGroupRoomInfo->dwGroupID)
		.set_format("gameid", "%u", pGroupRoomInfo->wGameID)
		.set_format("kindid", "%u", pGroupRoomInfo->wKindID)
		.set_format("gem", "%u", pGroupRoomInfo->wNeedGem)
		.set_format("roomid", "%u", pGroupRoomInfo->dwRoomNum);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert roomsale fail. errno:  " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}

	if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "select room sale record id fail, error = " << nError;
		return;
	}


	MSG_DB2CS_GroupRoom_Info groupRoomInfo;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		groupRoomInfo.dwRecordID = atoi((*row)["LAST_INSERT_ID()"]);
	}

	groupRoomInfo.dwRoomNum = pGroupRoomInfo->dwRoomNum;
	CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_ROOM_INFO, &groupRoomInfo, sizeof(groupRoomInfo));

	db->free_result();
}

CT_VOID DBThread::InsertGroup(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_CreateGroup* pGroupInfo)
{
	acl::query query;
	query.create_sql("INSERT INTO `group`(masterId, platformId, icon, incomeRate, settleDays, groupName, notice, createTime) "
                  "VALUES(:masterId, :pid, :icon, :incomeRate, :settleDays, :groupName, :notice, :createTime)")
		.set_format("masterId",	"%u", pGroupInfo->dwMasterID)
		.set_format("pid", "%d", pGroupInfo->cbPlatformID)
		.set_format("icon", "%d", pGroupInfo->cbIcon)
		.set_format("incomeRate", "%d", pGroupInfo->cbIncomeRate)
		.set_format("settleDays","%d", pGroupInfo->cbSettleDays)
		.set_format("groupName","%s", Utility::Utf82Ansi(pGroupInfo->szGroupName).c_str())
		.set_format("notice",	"%s", Utility::Utf82Ansi(pGroupInfo->szNotice).c_str())
		.set_date("createTime", time(NULL));

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert group fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}

	if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "select group id record fail, error = " << nError;
		return;
	}

	tagGroupInfo GroupInfo;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		GroupInfo.dwGroupID = (CT_DWORD)atoi((*row)["LAST_INSERT_ID()"]);
	}
	db->free_result();

	GroupInfo.dwMasterID = pGroupInfo->dwMasterID;
	GroupInfo.wUserCount = 0;
	GroupInfo.cbIcon = pGroupInfo->cbIcon;
	GroupInfo.cbLevel = pGroupInfo->cbLevel;
	GroupInfo.dwContribution = 0;
	GroupInfo.cbIncomeRate = pGroupInfo->cbIncomeRate;
	GroupInfo.cbSettleDays = pGroupInfo->cbSettleDays;
	_snprintf_info(GroupInfo.szGroupName, sizeof(GroupInfo.szGroupName), "%s", pGroupInfo->szGroupName);
	_snprintf_info(GroupInfo.szNotice, sizeof(GroupInfo.szNotice), "%s", pGroupInfo->szNotice);

	CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_CREATE_GROUP, &GroupInfo, sizeof(GroupInfo));
}

CT_VOID DBThread::AddGroupUser(acl::db_handle* db, MSG_CS2DB_Add_GroupUser* pAddUser)
{
	acl::query query;
	query.create_sql("INSERT INTO groupuser(groupId, userId, parentUserId, incomeRate,deductRate,deductTop,deductIncre,deductStart,totalIncome, memLevel, date) "
                  "VALUES(:gid, :userid, :parentUserId,:incomeRate,:deductRate,:deductTop,:deductIncre,:deductStart, :totalIncome, :memLevel, :date)")
		.set_format("gid", "%u", pAddUser->dwGroupID)
		.set_format("userid", "%u", pAddUser->dwUserID)
		.set_format("parentUserId", "%u", pAddUser->dwParentUserID)
		.set_format("incomeRate", "%d", pAddUser->cbIncomeRate)
        .set_format("deductRate", "%d", pAddUser->wDeductRate)
        .set_format("deductTop", "%d", pAddUser->wDeductTop)
        .set_format("deductIncre", "%d", pAddUser->wDeductIncre)
        .set_format("deductStart", "%d", pAddUser->dwDeductStart)
		.set_format("totalIncome", "%lld", pAddUser->llIncome)
		.set_format("memLevel", "%d", pAddUser->cbMemLevel)
		.set_date("date", time(NULL));

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert group user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}

	if (pAddUser->dwParentUserID != 0)
    {
        query.reset();
        query.create_sql("INSERT INTO group_dir_user(groupId, parentUserId, dirUserId) "
                         "VALUES(:gid, :parentUserId, :dirUserId)")
            .set_format("gid", "%u", pAddUser->dwGroupID)
            .set_format("parentUserId", "%u", pAddUser->dwParentUserID)
            .set_format("dirUserId", "%u", pAddUser->dwUserID);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "insert group dir user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            db->free_result();
            return;
        }

        CT_DWORD parentUserId = pAddUser->dwParentUserID;
        while (parentUserId != 0)
        {
            query.reset();
            query.create_sql("SELECT parentUserId from groupuser WHERE groupId = :gid AND userId = :parentUserId LIMIT 1")
                .set_format("gid", "%u", pAddUser->dwGroupID)
                .set_format("parentUserId", "%u", parentUserId);
            if (db->exec_select(query) == false)
            {
                LOG(WARNING) << "select group parent user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
                db->free_result();
                return;
            }

            for (size_t i = 0; i < db->length(); i++)
            {
                const acl::db_row* row = (*db)[i];
                parentUserId = (CT_DWORD)atoi((*row)["parentUserId"]);
            }
            db->free_result();

            if (parentUserId == 0)
            {
                break;
            }

            query.create_sql("INSERT INTO group_sub_user(groupId, parentUserId, subUserId) "
                             "VALUES(:gid, :parentUserId, :subUserId)")
                .set_format("gid", "%u", pAddUser->dwGroupID)
                .set_format("parentUserId", "%u", parentUserId)
                .set_format("subUserId", "%u", pAddUser->dwUserID);

            if (db->exec_update(query) == false)
            {
                LOG(WARNING) << "insert group sub user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
                db->free_result();
                return;
            }
        }
    }

    db->free_result();
}

CT_VOID DBThread::QueryBeAddGroupUser(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_Query_BeAdd_GroupUser* pBeAddGroupUser)
{
	acl::query query;
	query.create_sql("select `user`.nickname, userinfo.headimage FROM `user` INNER JOIN userinfo ON `user`.userid = userinfo.userid WHERE `user`.userid = :userid")
		.set_format("userid", "%u", pBeAddGroupUser->dwUserID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select be add user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	MSG_DB2CS_BeAdd_Group_User beAddGroupUser;
	beAddGroupUser.cbResult = 0;
	beAddGroupUser.cbType = pBeAddGroupUser->cbType;
	beAddGroupUser.dwOperateUserID = pBeAddGroupUser->dwOperateUserID;
	beAddGroupUser.dwUserID = pBeAddGroupUser->dwUserID;
	beAddGroupUser.dwGroupID = pBeAddGroupUser->dwGroupID;
	for (size_t i = 0; i < db->length(); i++)
	{
		beAddGroupUser.cbResult = 1;
		const acl::db_row* row = (*db)[i];
		std::string strNickName = (*row)["nickname"];
		std::string strHeadUrl = (*row)["headimage"];
		_snprintf_info(beAddGroupUser.szNickName, sizeof(beAddGroupUser.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());
		_snprintf_info(beAddGroupUser.szHeadUrl, sizeof(beAddGroupUser.szHeadUrl), "%s", Utility::Ansi2Utf8(strHeadUrl).c_str());
	}

	CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_QUERY_BE_ADD_GROUP_USER, &beAddGroupUser, sizeof(beAddGroupUser));
	db->free_result();
}

CT_VOID DBThread::RemoveGroupUser(acl::db_handle* db, MSG_CS2DB_Remove_GroupUser* pRemoveUser)
{
	acl::query query;
	query.create_sql("DELETE FROM groupuser WHERE groupId=:gid AND userId = :userid")
		.set_format("gid", "%u", pRemoveUser->dwGroupID)
		.set_format("userid", "%u", pRemoveUser->dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete group user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::RemoveGroup(acl::db_handle* db, MSG_CS2DB_Remove_Group* pRemoveGroup)
{
	//删除群
	acl::query query;
	query.create_sql("DELETE FROM `group` WHERE id=:gid")
		.set_format("gid", "%u", pRemoveGroup->dwGroupID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete group fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}
	db->free_result();

	//删除群成员
	query.reset();
	query.create_sql("DELETE FROM groupuser WHERE groupId=:gid ")
		.set_format("gid", "%u", pRemoveGroup->dwGroupID);
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete all group user fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}
	db->free_result();
}

CT_VOID	DBThread::RemoveGroupRoomCfg(acl::db_handle* db, MSG_CS2DB_Remove_Group* pRemoveGroup)
{
	//删除群设置
	acl::query query;
	query.create_sql("DELETE FROM roomconfig WHERE gid=:gid ")
		.set_format("gid", "%u", pRemoveGroup->dwGroupID);
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete group room config fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}
	db->free_result();
}

CT_VOID	DBThread::ModifyGroupInfo(acl::db_handle* db, MSG_CS2DB_Modify_GroupInfo* pModifyInfo)
{
	acl::query query;
	query.create_sql("UPDATE `group` SET icon = :icon, `level` = :level, incomeRate=:incomeRate, settleDays = :settleDays, groupName = :groupName, notice = :notice WHERE id=:gid")
	    .set_format("icon", "%d", pModifyInfo->cbIcon)
	    .set_format("level", "%d", pModifyInfo->cbLevel)
        .set_format("incomeRate", "%d", pModifyInfo->cbIncomeRate)
	    .set_format("settleDays", "%d", pModifyInfo->cbSettleDays)
		.set_format("groupName", "%s", Utility::Utf82Ansi(pModifyInfo->szGroupName).c_str())
		.set_format("notice", "%s", Utility::Utf82Ansi(pModifyInfo->szNotice).c_str())
		.set_format("gid", "%u", pModifyInfo->dwGroupID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "modify group info fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::AddGroupContribution(acl::db_handle* db, MSG_CS2DB_Update_GroupContribution* pAddContribution)
{
    acl::query query;
    query.create_sql("UPDATE `group` SET contribution = contribution+:contribution WHERE id=:gid")
        .set_format("contribution", "%u", pAddContribution->dwAddContribution)
        .set_format("gid", "%u", pAddContribution->dwGroupID);

    if (db->exec_update(query) == false)
    {
        LOG(WARNING) << "add group contribution fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
        return;
    }
    db->free_result();
}

CT_VOID	DBThread::DeductGroupMasterGem(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_Deduct_GroupMaster_Gem* pDeductGem)
{
	acl::query query;
	query.create_sql("SELECT gem FROM userinfo WHERE userid = :userid")
		.set_format("userid", "%u", pDeductGem->dwMasterID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select group master gem fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}

	CT_DWORD dwGem = 0;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		dwGem = atoi((*row)["gem"]);
	}

	db->free_result();

	MSG_DB2CS_Deduct_GroupMaster_Gem deductResult;
	deductResult.dwGroupID = pDeductGem->dwGroupID;
	deductResult.dwMasterID = pDeductGem->dwMasterID;
	deductResult.dwUserID = pDeductGem->dwUserID;
	deductResult.wNeedGem = pDeductGem->wNeedGem;
	deductResult.wGameID = pDeductGem->wGameID;
	deductResult.wKindID = pDeductGem->wKindID;

	if (dwGem < 10 || dwGem < pDeductGem->wNeedGem)
	{
		deductResult.cbResult = 0;
	}
	else
	{
		query.reset();
		query.create_sql("update userinfo set gem = gem - :needgem where userid = :userid")
			.set_format("needgem", "%u", pDeductGem->wNeedGem)
			.set_format("userid", "%u", pDeductGem->dwMasterID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "deduct group master gem fail, has gem: " << dwGem << ", need gem: " << pDeductGem->wNeedGem << ", master id: " << pDeductGem->dwMasterID;
			db->free_result();
			return;
		}
		deductResult.cbResult = 1;
	}

	CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_DEDUCT_GROUP_MASTER_GEM, &deductResult, sizeof(deductResult));

	db->free_result();
}

CT_VOID DBThread::ChangeGroupMemberPosition(MSG_CS2DB_GroupPosition* pGroupPosition)
{
    acl::db_pool* pool = GetAccountdbPool();
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
    do
    {
        acl::query query;
        query.create_sql("update `group` set `deputyId1` = :deputyId1, `deputyId2` = :deputyId2 where `id` = :groupId")
        .set_format("deputyId1", "%u", pGroupPosition->dwDeputyID1)
        .set_format("deputyId2", "%u", pGroupPosition->dwDeputyID2)
        .set_format("groupId", "%u", pGroupPosition->dwGroupID);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "ChangeGroupMemberPosition fail, group id: " << pGroupPosition->dwGroupID << ", deputy1" << pGroupPosition->dwDeputyID1 << ", deputy2:"<< pGroupPosition->dwDeputyID2
            << ", errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    } while (0);
    pool->put(db);
}

CT_VOID DBThread::UpdateGroupUserInfo(MSG_CS2DB_Update_GroupUserInfo* groupUserInfo)
{
    acl::db_pool* pool = GetAccountdbPool();
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
    do
    {
        acl::query query;
        query.create_sql("UPDATE groupuser SET incomeRate = :incomeRate, lastTakeIncomeDate=:lastTakeIncomeDate,lastIncomeDate=:lastIncomeDate, "
                         "linkIncomeDays=:linkIncomeDays, totalIncome=:totalIncome,remarks=:remarks where groupId = :groupId and userId = :userId LIMIT 1")
            .set_format("incomeRate", "%d", groupUserInfo->cbIncomeRate)
            .set_format("lastTakeIncomeDate", "%u", groupUserInfo->dwLastTakeIncomeDate)
            .set_format("lastIncomeDate", "%u", groupUserInfo->dwLastIncomeDate)
            .set_format("linkIncomeDays", "%u", groupUserInfo->wLinkIncomeDays)
            .set_format("totalIncome", "%lld", groupUserInfo->llTotalIncome)
            .set_format("remarks", "%s", groupUserInfo->szRemarks)
            .set_format("groupId", "%u", groupUserInfo->dwGroupID)
            .set_format("userId", "%u", groupUserInfo->dwUserID);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "UpdateGroupUserInfo fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    } while (0);
    pool->put(db);
}

CT_VOID DBThread::InsertGroupChatMsg(MSG_CS2DB_Insert_GroupChat* pGroupChat)
{
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }
    do
    {
        acl::query query;
        query.create_sql("INSERT INTO group_chat VALUES (:msgId,:groupId,:fromUserId,:toUserId,:chat,:time,:status)")
            .set_format("msgId", "%u", pGroupChat->dwMsgId)
            .set_format("groupId", "%u", pGroupChat->dwGroupID)
            .set_format("fromUserId", "%u", pGroupChat->dwFromUserID)
            .set_format("toUserId", "%u", pGroupChat->dwToUserID)
            .set_format("chat", "%s", pGroupChat->szChat)
            .set_format("time", "%u", pGroupChat->dwSendTime)
            .set_format("status", "%d", pGroupChat->cbStatus);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "InsertGroupChatMsg fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    } while (0);
    pool->put(db);
}

CT_VOID DBThread::UpdateGroupChatMsgStatus(MSG_CS2DB_Update_GroupChat* pGroupChatStatus)
{
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }
    do
    {
        acl::query query;
        query.create_sql("UPDATE group_chat SET `status` = :status WHERE msgId = :msgId")
            .set_format("msgId", "%u", pGroupChatStatus->dwMsgID)
            .set_format("status", "%d", pGroupChatStatus->cbStatus);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "UpdateGroupChatMsgStatus fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    } while (0);
    pool->put(db);
}

CT_VOID DBThread::UpdateGroupDirIncome(MSG_CS2DB_Update_GroupIncome* pGroupIncome, CT_DWORD dwCount)
{
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }

    std::stringstream strSql;
    strSql << "INSERT INTO group_dir_income (dateInt, userId, dirUserId, groupId, originalIncome, deductIncome, deductRate, updateTime) VALUES ";

    std::stringstream streamUpdateValue;
    streamUpdateValue << " ON DUPLICATE KEY UPDATE " << " originalIncome=VALUES(originalIncome) " << ", deductIncome=VALUES(deductIncome) "
                      << ", deductRate=VALUES(deductRate) " <<  ", updateTime=VALUES(updateTime)";

    std::stringstream strSqlTotal;
    strSqlTotal << "INSERT INTO group_total_income (groupId, userId, parentUserId, subUserId, totalIncome) VALUES ";

    std::stringstream streamUpdateValueTotal;
    streamUpdateValueTotal << " ON DUPLICATE KEY UPDATE " << "totalIncome=VALUES(totalIncome) + totalIncome";


    std::stringstream streamInsertValue;
    std::stringstream streamInsertValueTotal;
    CT_DWORD dwNow = time(NULL);
    do
    {
        for (CT_DWORD i = 0; i < dwCount; ++i)
        {
            streamInsertValue << "(" << pGroupIncome->dwDateInt << "," << pGroupIncome->dwUserID << "," <<  pGroupIncome->dwSubUserID << "," << pGroupIncome->dwGroupID
            << "," << pGroupIncome->dwOriginalIncome << ","<< pGroupIncome->dwDeductIncome << "," << pGroupIncome->wDeductRate << "," << dwNow << "),";

            if (pGroupIncome->cbCrossDays)
            {
                streamInsertValueTotal << "(" << pGroupIncome->dwGroupID << "," << pGroupIncome->dwUserID << "," << pGroupIncome->dwParentUserID << "," <<  pGroupIncome->dwSubUserID
                                       << "," << pGroupIncome->dwDeductIncome << "),";
            }

            pGroupIncome = (MSG_CS2DB_Update_GroupIncome*)((CT_BYTE*)pGroupIncome + sizeof(MSG_CS2DB_Update_GroupIncome));
        }

        {
            std::string strInsertValue = streamInsertValue.str();
            if (strInsertValue.empty())
            {
                return;
            }

            auto itPos = strInsertValue.find_last_of(",");
            if (itPos != std::string::npos)
            {
                strInsertValue.erase(itPos);
            }
            strSql << strInsertValue << streamUpdateValue.str();

            if (db->sql_update(strSql.str().c_str()) == false)
            {
                LOG(WARNING) << "UpdateGroupDirIncome fail" << ", errno: " << db->get_errno() <<  ", error: " << db->get_error();
                break;
            }
        }

        if (!streamInsertValueTotal.str().empty())
        {
            std::string strInsertValueTotal = streamInsertValueTotal.str();
            if (strInsertValueTotal.empty())
            {
                return;
            }

            auto itPosTotal = strInsertValueTotal.find_last_of(",");
            if (itPosTotal != std::string::npos)
            {
                strInsertValueTotal.erase(itPosTotal);
            }
            strSqlTotal << strInsertValueTotal << streamUpdateValueTotal.str();

            if (db->sql_update(strSqlTotal.str().c_str()) == false)
            {
                LOG(WARNING) << "UpdateGroupDirIncome total fail" << ", errno: " << db->get_errno() <<  ", error: " << db->get_error() << ", sql: " << strSqlTotal.str().c_str();
                break;
            }
        }
    }while (0);
    pool->put(db);
}

CT_VOID DBThread::UpdateGroupSubIncome(MSG_CS2DB_Update_GroupIncome* pGroupIncome, CT_DWORD dwCount)
{
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }

    std::stringstream strSql;
    strSql << "INSERT INTO group_sub_income (dateInt, userId, subUserId, groupId, originalIncome, deductIncome, deductRate, updateTime) VALUES ";

    std::stringstream streamUpdateValue;
    streamUpdateValue << " ON DUPLICATE KEY UPDATE " << " originalIncome=VALUES(originalIncome) " << ", deductIncome=VALUES(deductIncome) "
                      << ", deductRate=VALUES(deductRate) " <<  ", updateTime=VALUES(updateTime)";

    std::stringstream strSqlTotal;
    strSqlTotal << "INSERT INTO group_total_income (groupId, userId, parentUserId, subUserId, totalIncome) VALUES ";

    std::stringstream streamUpdateValueTotal;
    streamUpdateValueTotal << " ON DUPLICATE KEY UPDATE " << "totalIncome=VALUES(totalIncome) + totalIncome";


    std::stringstream streamInsertValue;
    std::stringstream streamInsertValueTotal;
    CT_DWORD dwNow = time(NULL);
    do
    {
        for (CT_DWORD i = 0; i < dwCount; ++i)
        {
            streamInsertValue << "(" << pGroupIncome->dwDateInt << "," << pGroupIncome->dwUserID << "," <<  pGroupIncome->dwSubUserID << "," << pGroupIncome->dwGroupID << ","
                << pGroupIncome->dwOriginalIncome << ","<< pGroupIncome->dwDeductIncome << "," << pGroupIncome->wDeductRate << "," << dwNow << "),";

            if (pGroupIncome->cbCrossDays)
            {
                streamInsertValueTotal << "(" << pGroupIncome->dwGroupID << "," << pGroupIncome->dwUserID << "," << pGroupIncome->dwParentUserID << "," <<  pGroupIncome->dwSubUserID
                                       << "," << pGroupIncome->dwDeductIncome << "),";
            }

            pGroupIncome = (MSG_CS2DB_Update_GroupIncome*)((CT_BYTE*)pGroupIncome + sizeof(MSG_CS2DB_Update_GroupIncome));
        }

        {
            std::string strInsertValue = streamInsertValue.str();
            if (strInsertValue.empty())
            {
                return;
            }

            auto itPos = strInsertValue.find_last_of(",");
            if (itPos != std::string::npos)
            {
                strInsertValue.erase(itPos);
            }
            strSql << strInsertValue << streamUpdateValue.str();

            if (db->sql_update(strSql.str().c_str()) == false)
            {
                LOG(WARNING) << "UpdateGroupSubIncome fail" << ", errno: " << db->get_errno() <<  ", error: " << db->get_error();
                break;
            }
        }

        if (!streamInsertValueTotal.str().empty())
        {
            std::string strInsertValueTotal = streamInsertValueTotal.str();
            if (strInsertValueTotal.empty())
            {
                return;
            }

            auto itPosTotal = strInsertValueTotal.find_last_of(",");
            if (itPosTotal != std::string::npos)
            {
                strInsertValueTotal.erase(itPosTotal);
            }
            strSqlTotal << strInsertValueTotal << streamUpdateValueTotal.str();

            if (db->sql_update(strSqlTotal.str().c_str()) == false)
            {
                LOG(WARNING) << "UpdateGroupSubIncome total fail" << ", errno: " << db->get_errno() <<  ", error: " << db->get_error();
                break;
            }
        }
    }while (0);
    pool->put(db);
}

std::string DBThread::GetTimeName()
{
    time_t tt = time(NULL);
    struct tm *stm = localtime(&tt);

    struct timeval tv;
    gettimeofday(&tv, NULL);
    int ms = tv.tv_usec / 1000;

    char tmp[32];
    sprintf(tmp, "%04d%02d%02d%02d%02d%02d%03d", 1900 + stm->tm_year, 1 + stm->tm_mon, stm->tm_mday, stm->tm_hour,
            stm->tm_min, stm->tm_sec, ms);

    return tmp;
}

CT_VOID DBThread::InsertGroupIncomeExchange(MSG_CS2DB_Insert_Exchange_GroupIncome* pExchangeIncome)
{
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }
    do
    {
        acl::query query;
        query.create_sql("INSERT INTO treasure.exchange_group(userid, groupid, fixdate, price, pay, applydate, `status`, orderid) "
                         "VALUES (:userid, :gid, :fixdate, :price, :pay, :applydate, :status, :orderid)")
            .set_format("userid", "%u", pExchangeIncome->dwUserID)
            .set_format("gid", "%u", pExchangeIncome->dwGroupID)
            .set_format("fixdate", "%u", pExchangeIncome->dwTime)
            .set_format("price", "%lld", pExchangeIncome->llIncome)
            .set_format("pay", "%lld", pExchangeIncome->llIncome)
            .set_format("applydate", "%u", 0)
            .set_format("status", "%d", 1)
            .set_format("orderid", "G%s", GetTimeName().c_str());

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "InsertGroupIncomeExchange fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    } while (0);
    pool->put(db);
}

CT_VOID DBThread::QueryGroupIncomeExchange(acl::aio_socket_stream* pSocket, MSG_CS2DB_Query_GroupIncome_Exchange* pQueryExchange)
{
    acl::db_pool* pool = GetRecorddbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get record db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get record db handle fail.";
        return;
    }
    do
    {
        acl::query query;
        query.create_sql("SELECT * from treasure.exchange_group WHERE userid = :userid ORDER BY id desc LIMIT 20")
            .set_format("userid", "%u", pQueryExchange->dwUserID);

        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "QueryGroupIncomeExchange fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }

        CT_BYTE szBuffer[1024];
        CT_DWORD dwSendSize = 0;
        memcpy(szBuffer, &pQueryExchange->dwUserID, sizeof(CT_DWORD));
        dwSendSize += sizeof(CT_DWORD);

        MSG_SC_Query_Group_Settle_Record settleRecord;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            settleRecord.dwSubmitTime = (CT_DWORD)atoi((*row)["fixdate"]);
            _snprintf_info(settleRecord.szOrderNum, sizeof(settleRecord.szOrderNum), "%s", (*row)["orderid"]);
            settleRecord.dAmount = atoi((*row)["price"])*TO_DOUBLE;
            settleRecord.cbStatus = (CT_BYTE)atoi((*row)["status"]);
            settleRecord.dwHandleTime = (CT_DWORD)atoi((*row)["applydate"]);
            memcpy(szBuffer + dwSendSize, &settleRecord, sizeof(MSG_SC_Query_Group_Settle_Record));
            dwSendSize += sizeof(MSG_SC_Query_Group_Settle_Record);
        }

        CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_GROUP_EXCHANGE, &szBuffer, dwSendSize);
    } while (0);

    db->free_result();
    pool->put(db);
}

CT_VOID DBThread::QueryWXGroupInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db)
{
	acl::query query;
	query.create_sql("select * from weixinGroup;");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select weixinGroup fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::map<CT_DWORD, CT_DWORD> mapGroupInfo;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		CT_DWORD dwGroupID = atoi((*row)["gid"]);
		CT_DWORD dwBindUserPlay = atoi((*row)["bindUserPlay"]);
		mapGroupInfo.insert(std::make_pair(dwGroupID, dwBindUserPlay));
	}
	db->free_result();

	for (auto& it : mapGroupInfo)
	{
		query.reset();
		query.create_sql("select userid from weixinUser where gid = :gid ")
			.set_format("gid", "%d", it.first);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select weixinUser fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			return;
		}

		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		MSG_DB2CS_WXGroupInfo* pGroupInfo = (MSG_DB2CS_WXGroupInfo*)szBuffer;
		pGroupInfo->dwGroupID = it.first;
		pGroupInfo->dwBindUserPlay = it.second;
		pGroupInfo->dwUserCount = 0;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			CT_DWORD dwUserID = atoi((*row)["userid"]);
			memcpy(szBuffer + sizeof(MSG_DB2CS_WXGroupInfo) + sizeof(dwUserID)*pGroupInfo->dwUserCount, &dwUserID, sizeof(dwUserID));
			pGroupInfo->dwUserCount += 1;
		}

		CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_WXGROUP_INFO, &szBuffer, sizeof(MSG_DB2CS_WXGroupInfo) + sizeof(CT_DWORD)*pGroupInfo->dwUserCount);
		db->free_result();
	}

	CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_WXGROUP_INFO_FINISH);
}

CT_VOID DBThread::ReturnWXGroupPRoomGem(acl::db_handle* db, MSG_G2DB_Return_WXGroupPRoom_Gem* pReturnGem)
{
	acl::query query;
	query.create_sql("select gem from roomSale where id = :id and gid = :gid")
		.set_format("id", "%d", pReturnGem->dwRecordID)
		.set_format("gid", "%d", pReturnGem->dwGroupID);

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select roomSale gem fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	CT_DWORD dwGem = 0;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		dwGem = atoi((*row)["gem"]);
	}
	db->free_result();

	query.reset();
	query.create_sql("update roomSale set status = 2 where id = :id and gid = :gid")
		.set_format("id", "%d", pReturnGem->dwRecordID)
		.set_format("gid", "%d", pReturnGem->dwGroupID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update roomSale status fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}
	db->free_result();

	query.reset();
	query.create_sql("UPDATE promoter a INNER JOIN weixinGroup b ON a.userid = b.pid SET a.gem = a.gem + :returnGem WHERE b.gid = :gid;")
		.set_format("returnGem", "%d", dwGem)
		.set_format("gid", "%d", pReturnGem->dwGroupID);
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update promoter gem fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::InsertWXGroupPRoomRecord(acl::db_handle* db, MSG_G2DB_Record_WXGroupPRoom* pAutoPRoom)
{
	acl::query query;
	query.create_sql("insert into record_weixin_room(rid, gid, roundid, fixdate) values(:rid, :gid, :roundid, UNIX_TIMESTAMP(NOW()))")
		.set_format("rid", "%u", pAutoPRoom->dwRecordID)
		.set_format("gid", "%u", pAutoPRoom->dwGroupID)
#ifdef _OS_MAC_CODE
		.set_format("roundid", "%llu", pAutoPRoom->uGameRoundID);
#else
		.set_format("roundid", "%lu", pAutoPRoom->uGameRoundID);
#endif

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert record weixin room fail. errno:  " << db->get_errno() << ", err msg: " << db->get_error();
		db->free_result();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::UpdateUserScore(acl::db_handle* db, CT_DWORD dwUserID, CT_LONGLONG llAddScore)
{
	acl::query query;
	query.create_sql("update userinfo set score = score +:scoreval where userid = :useridval").set_format("scoreval", "%lld", llAddScore).set_format("useridval", "%u", dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "add db score fail, add score: " << llAddScore << ", user id: " << dwUserID
		<< ", errno: " << db->get_errno() << ", error: " << db->get_error();
		db->free_result();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::SetUserScore(CT_DWORD dwUserID, CT_LONGLONG llScore)
{
	acl::db_pool* pool = GetAccountdbPool();
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

	do 
	{
		acl::query query;
		query.create_sql("update userinfo set score = :score where userid = :userid")
			.set_format("score", "%lld", llScore)
			.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "set db score fail, add score: " << llScore << ", user id: " << dwUserID;
			db->free_result();
			break;
		}

		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID DBThread::UpdateUserBenefitInfo( MSG_G2GB_Update_BenefitInfo* pBenefitInfo)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("update record_benefit  set count=:count ,lastTime = :lastTime where userid = :userid")
			.set_format("count", "%d", pBenefitInfo->cbRewardCount)
			.set_format("lastTime", "%u", pBenefitInfo->dwLastTime)
			.set_format("userid", "%u", pBenefitInfo->dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update benefit info fail, user id: " << pBenefitInfo->dwUserID << ", errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
		}

		query.reset();
        query.create_sql("insert into record.record_score_change(userid, sourceBank, sourceScore, changeBank, changeScore, type, fixdate)\
            values(:userid, 0, :sourceScore, 0, :changeScore, :type, :lastTime)")
            .set_format("userid", "%u", pBenefitInfo->dwUserID)
            .set_format("sourceScore", "%lld", pBenefitInfo->llSourceScore)
            .set_format("changeScore", "%u", pBenefitInfo->dwRewardScore)
            .set_format("lastTime", "%u", pBenefitInfo->dwLastTime)
            .set_format("type", "%d", BENEFIT_REWARD);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "update benefit score change info fail, user id: " << pBenefitInfo->dwUserID << ", errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }

	} while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::UpdateUserScoreInfo(acl::db_handle* db, MSG_Update_ScoreInfo* pScoreInfo)
{
	if (!pScoreInfo->bIsAndroid)
	{
		//先更新redis
		acl::string key;
		key.format("scoreinfo_%u", pScoreInfo->dwUserID);

		m_redis.clear();
		if (pScoreInfo->dwAddWinCount != 0)
		{
			m_redis.hincrby(key.c_str(), "winCount", pScoreInfo->dwAddWinCount);
		}
		else if (pScoreInfo->dwAddLostCount != 0)
		{
			m_redis.hincrby(key.c_str(), "lostCount", pScoreInfo->dwAddLostCount);
		}
		else if (pScoreInfo->dwAddDrawCount != 0)
		{
			m_redis.hincrby(key.c_str(), "drawCount", pScoreInfo->dwAddDrawCount);
		}
		else if (pScoreInfo->dwAddFleeCount != 0)
		{
			m_redis.hincrby(key.c_str(), "fleeCount", pScoreInfo->dwAddFleeCount);
		}
	}
	
	//更新db
	acl::query query;
	query.create_sql("INSERT INTO gameScoreData(userid,revenue, winCount,lostCount,drawCount,fleeCount,playTime) VALUES (:userid, :revenue, :winCount, :lostCount, :drawCount, :fleeCount, :playTime) \
		ON DUPLICATE KEY UPDATE revenue=revenue+:revenue, winCount=winCount+:winCount, lostCount=lostCount+:lostCount,drawCount=drawCount+:drawCount, fleeCount=fleeCount+:fleeCount,playTime=playTime+:playTime")
		.set_format("revenue", "%d", pScoreInfo->dwAddRevenue)
		.set_format("winCount", "%d", pScoreInfo->dwAddWinCount)
		.set_format("lostCount", "%d", pScoreInfo->dwAddLostCount)
		.set_format("drawCount", "%d", pScoreInfo->dwAddDrawCount)
		.set_format("fleeCount", "%d", pScoreInfo->dwAddFleeCount)
		.set_format("playTime", "%d", pScoreInfo->dwAddPlayTime)
		.set_format("userid", "%u", pScoreInfo->dwUserID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update game score info fail, user id: " << pScoreInfo->dwUserID;
		return;
	}

	db->free_result();
}

CT_VOID DBThread::UpdateUserScoreData(MSG_G2DB_ScoreData* pScoreData)
{
	acl::db_pool *pool = GetRecorddbPool();
	if (pool == NULL) {
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle *db = pool->peek_open();
	if (db == NULL) {
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	do
	{
		CT_DWORD dwNow = (CT_DWORD) time(NULL);
		//更新db
		acl::query query;
		query.create_sql("INSERT INTO src_user_stat (gameid, kindid, roomKindid, userid, firstPlayTime, lastPlayTime, "
						 "winCount, lostCount, winScore, lostScore, revenue, winCountToday, lostCountToday, winScoreToday, lostScoreToday, revenueToday) "
						 " VALUES (:gameid, :kindid, :roomKindid, :userid, :firstPlayTime, :lastPlayTime, "
						 ":winCount, :lostCount, :winScore, :lostScore, :revenue, :winCountToday, :lostCountToday, :winScoreToday, :lostScoreToday, :revenueToday) "
						 "ON DUPLICATE KEY UPDATE lastPlayTime=:lastPlayTime, winCount=winCount+:winCount, lostCount=lostCount+:lostCount, winScore=winScore+:winScore,"
						 "lostScore=lostScore+:lostScore, revenue= revenue+:revenue, winCountToday= winCountToday+:winCountToday, lostCountToday=lostCountToday+:lostCountToday, "
						 "winScoreToday=winScoreToday+:winScoreToday, lostScoreToday=lostScoreToday+:lostScoreToday, revenueToday= revenueToday+:revenueToday")
			.set_format("gameid", "%d", pScoreData->wGameID)
			.set_format("kindid", "%d", pScoreData->wKindID)
			.set_format("roomKindid", "%d", pScoreData->wRoomKindID)
			.set_format("userid", "%u", pScoreData->dwUserID)
			.set_date("firstPlayTime", dwNow, "%Y-%m-%d")
			.set_date("lastPlayTime", dwNow, "%Y-%m-%d")
			.set_format("winCount", "%u", pScoreData->dwWinCount)
			.set_format("lostCount", "%u", pScoreData->dwLostCount)
			.set_format("winScore", "%lld", pScoreData->llWinScore)
			.set_format("lostScore", "%lld", pScoreData->llLostScore)
			.set_format("revenue", "%u", pScoreData->dwRevenue)
			.set_format("winCountToday", "%u", pScoreData->dwWinCount)
			.set_format("lostCountToday", "%u", pScoreData->dwLostCount)
			.set_format("winScoreToday", "%lld", pScoreData->llWinScore)
			.set_format("lostScoreToday", "%lld", pScoreData->llLostScore)
			.set_format("revenueToday", "%u", pScoreData->dwRevenue);

		if (db->exec_update(query) == false) {
			LOG(WARNING) << "insert src_user_stat fail, user id: " << pScoreData->dwUserID << ", errno: "
						 << db->get_errno() << ", error: " << db->get_error();
			break;
		}
	}
	while (0);


	db->free_result();
	pool->put(db);
}


CT_VOID DBThread::QueryUserScoreInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS_Query_UserScore_Info* pUserScoreInfo)
{
	acl::string key;
	std::map<acl::string, acl::string> mapAccountInfo;
	key.format("account_%u", pUserScoreInfo->dwQueryUserID);

	UserAccountsBaseData userBaseInfo;
	memset(&userBaseInfo, 0, sizeof(userBaseInfo));

	UserScoreData scoreData;
	memset(&scoreData, 0, sizeof(scoreData));

	CT_BOOL bInRedis = false;

	m_redis.clear();
	if (m_redis.hgetall(key.c_str(), mapAccountInfo) == true)
	{
		if (mapAccountInfo.size() != 0)
		{
			userBaseInfo.cbGender =  atoi(mapAccountInfo["gender"].c_str());
			userBaseInfo.llScore = atoll(mapAccountInfo["score"].c_str());
			userBaseInfo.cbVipLevel = (CT_BYTE)atoi(mapAccountInfo["vip"].c_str());

			m_redis.clear();
			acl::string key;
			key.format("scoreinfo_%u", pUserScoreInfo->dwQueryUserID);

			std::map<acl::string, acl::string> scoreResult;
			m_redis.clear();
			if (m_redis.hgetall(key, scoreResult) == true)
			{
				if (scoreResult.size() != 0)
				{
					scoreData.dwRevenue = atoi(scoreResult["revenue"].c_str());
					scoreData.dwWinCount = atoi(scoreResult["winCount"].c_str());
					scoreData.dwLostCount = atoi(scoreResult["lostCount"].c_str());
					scoreData.dwDrawCount = atoi(scoreResult["drawCount"].c_str());
					scoreData.dwFleeCount = atoi(scoreResult["fleeCount"].c_str());
					scoreData.dwPlayTime = atoi(scoreResult["playTime"].c_str());
				}
			}

			bInRedis = true;
		}
	}

	if (bInRedis == false)
	{
		acl::query query;
		query.create_sql("SELECT gameScoreData.winCount, gameScoreData.lostCount,gameScoreData.drawCount, gameScoreData.fleeCount, account.userinfo.gender,\
			account.userinfo.score,account.userdata.vip from gameScoreData INNER JOIN account.userinfo on account.userinfo.userid = gameScoreData.userid \
			INNER JOIN account.userdata on account.userdata.userid = gameScoreData.userid WHERE gameScoreData.userid =:userid")
			.set_format("userid", "%u", pUserScoreInfo->dwQueryUserID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select user base score info fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			return;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			userBaseInfo.cbGender  = (CT_BYTE)atoi((*row)["gender"]);
			userBaseInfo.llScore = atoll((*row)["score"]);
			scoreData.dwWinCount = (CT_BYTE)atoi((*row)["winCount"]);
			scoreData.dwLostCount = (CT_BYTE)atoi((*row)["lostCount"]);
			scoreData.dwDrawCount = (CT_BYTE)atoi((*row)["drawCount"]);
			scoreData.dwFleeCount = (CT_BYTE)atoi((*row)["fleeCount"]);
			userBaseInfo.cbVipLevel = (CT_BYTE)atoi((*row)["vip"]);
		}
		db->free_result();
	}

	//返回给玩家
	MSG_SC_Query_UserScore_Info userScoreInfo;
	userScoreInfo.dwMainID = MSG_QUERY_MAIN;
	userScoreInfo.dwSubID = SUB_SC_QUERY_USER_SCORE_INFO;
	userScoreInfo.dwValue2 = pUserScoreInfo->dwUserID;

	userScoreInfo.dwUserID = pUserScoreInfo->dwQueryUserID;
	userScoreInfo.cbSex = userBaseInfo.cbGender;
	userScoreInfo.llScore = userBaseInfo.llScore;
	userScoreInfo.dwWinCount = scoreData.dwWinCount;
	userScoreInfo.dwLostCount = scoreData.dwLostCount;
	userScoreInfo.dwDrawCount = scoreData.dwDrawCount;
	userScoreInfo.dwFleeCount = scoreData.dwFleeCount;
	userScoreInfo.cbVipLevel = userBaseInfo.cbVipLevel;

	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &userScoreInfo, sizeof(userScoreInfo));
}


CT_VOID DBThread::UpdateUserShowExchange(CT_DWORD dwUserID, CT_BYTE cbShowExchange)
{
	acl::db_pool* pool = GetAccountdbPool();
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
	do 
	{
		acl::query query;query.create_sql("update userinfo set showExchange = :showExchange where userid=:userid limit 1")
		.set_format("showExchange", "%d", cbShowExchange)
		.set_format("userid", "%u", dwUserID);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update user show exchange fail, show: " << (int)cbShowExchange << ", user id: " << dwUserID << ", errno: " << db->get_errno() << ", error: " << db->get_error();;
			return;
		}
	} while (0);

	pool->put(db);
}

CT_VOID DBThread::LoadHongBaoInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db)
{
	acl::query query;
	query.create_sql("select hongbao.id, hongbao.userid, hongbao.`name`, hongbao.score, UNIX_TIMESTAMP(hongbao.sendTime) AS sendTime,hongbao.guessUserID, account.`user`.nickname, account.userinfo.headimage FROM hongbao \
		INNER JOIN account.`user` ON  account.`user`.userid = hongbao.userid \
		INNER JOIN account.userinfo ON account.userinfo.userid = hongbao.userid");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select hongbao fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::vector<tagHongBaoInfo> vecHongbaoInfo;
	tagHongBaoInfo hongbaoInfo;
	for (size_t i = 0; i < db->length(); i++)
	{
		memset(&hongbaoInfo, 0, sizeof(tagHongBaoInfo));
		const acl::db_row* row = (*db)[i];
		hongbaoInfo.dwID = atoi((*row)["id"]);
		hongbaoInfo.dwSenderID = atoi((*row)["userid"]);
		hongbaoInfo.dwHongBaoScore = atoi((*row)["score"]);
		hongbaoInfo.dwSendTime = atoi((*row)["sendTime"]);
		hongbaoInfo.dwGuessUserID = atoi((*row)["guessUserID"]);
		std::string strHongBaoName = (*row)["name"];
		std::string strSenderNickName = (*row)["nickname"];
		std::string strHeadUrl = (*row)["headimage"];
		_snprintf_info(hongbaoInfo.szHongBaoName, sizeof(hongbaoInfo.szHongBaoName), "%s", Utility::Ansi2Utf8(strHongBaoName).c_str());
		_snprintf_info(hongbaoInfo.szSenderNickName, sizeof(hongbaoInfo.szSenderNickName), "%s", Utility::Ansi2Utf8(strSenderNickName).c_str());
		_snprintf_info(hongbaoInfo.szSenderHeadUrl, sizeof(hongbaoInfo.szSenderHeadUrl), "%s", Utility::Ansi2Utf8(strHeadUrl).c_str());

		vecHongbaoInfo.push_back(hongbaoInfo);
	}
	db->free_result();

	for (auto& it : vecHongbaoInfo)
	{
		if (it.dwGuessUserID != 0)
		{
			query.reset();
			query.create_sql("select nickname from account.`user` WHERE userid = :userid")
				.set_format("userid", "%u", it.dwGuessUserID);

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << "select guess user nickname fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				return;
			}
			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				std::string strNickname = (*row)["nickname"];
				_snprintf_info(it.szGuessNickName, sizeof(it.szGuessNickName), "%s", Utility::Ansi2Utf8(strNickname).c_str());
			}
			db->free_result();
		}
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
	CT_DWORD dwSendSize = 0;
	for (auto&it : vecHongbaoInfo)
	{
		//发送数据
		if ((dwSendSize + sizeof(tagHongBaoInfo)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_HONGBAO_INFO, szBuffer, dwSendSize);
			dwSendSize = 0;
		}

		memcpy(szBuffer + dwSendSize, &it, sizeof(tagHongBaoInfo));
		dwSendSize += sizeof(tagHongBaoInfo);
	}
	if (dwSendSize > 0)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_HONGBAO_INFO, szBuffer, dwSendSize);
	}
}

CT_VOID DBThread::InsertHongbaoInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_Insert_HongBao* pHongBaoInfo)
{
	acl::query query;
	query.create_sql("INSERT INTO hongbao(userid, score, `name`, sendTime) VALUES (:userid, :score, :name, :sendTime)")
		.set_format("userid", "%u", pHongBaoInfo->dwUserID)
		.set_format("score", "%u", pHongBaoInfo->dwHongBaoScore)
		.set_format("name", "%s", Utility::Utf82Ansi(pHongBaoInfo->szHongBaoName).c_str())
		.set_date("sendTime", pHongBaoInfo->dwSendTime);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert hongbao fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}	
	
	db->free_result();
	if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "select game round id record fail, error = " << nError;
		return;
	}

	tagHongBaoInfo hongbaoInfo;
	memset(&hongbaoInfo, 0, sizeof(hongbaoInfo));
	hongbaoInfo.dwSenderID = pHongBaoInfo->dwUserID;
	hongbaoInfo.dwHongBaoScore = pHongBaoInfo->dwHongBaoScore;
	hongbaoInfo.dwSendTime = pHongBaoInfo->dwSendTime;
	_snprintf_info(hongbaoInfo.szHongBaoName, sizeof(hongbaoInfo.szHongBaoName), "%s", pHongBaoInfo->szHongBaoName);
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		hongbaoInfo.dwID = atoi((*row)["LAST_INSERT_ID()"]);
	}
	CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_INSERT_HONGBAO, &hongbaoInfo, sizeof(hongbaoInfo));

	db->free_result();
}

CT_VOID DBThread::DeleteHongbaoInfo(acl::db_handle* db, MSG_CS2DB_Delete_HongBao* pDeleteHongBao)
{
	//先插入记录
	acl::query query;
	query.create_sql("INSERT INTO hongbaoRecord SELECT *, NOW() FROM hongbao WHERE id = :id;")
		.set_format("id", "%u", pDeleteHongBao->dwHongBaoID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert hongbao record fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		//继续删除红包
		//return;
	}
	db->free_result();

	query.reset();
	query.create_sql("DELETE FROM hongbao WHERE id = :id")
		.set_format("id", "%u", pDeleteHongBao->dwHongBaoID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete hongbao fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::UpdateHongbaoInfo(acl::db_handle * db, MSG_CS2DB_Update_HongBao * pHongBaoInfo)
{
	//更新记录
	acl::query query;
	query.create_sql("UPDATE hongbao SET guessUserID = :guessUserID WHERE id = :id;")
		.set_format("guessUserID", "%u", pHongBaoInfo->dwGuessUserID)
		.set_format("id", "%u", pHongBaoInfo->dwHongBaoID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update hongbao fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}
	db->free_result();
}

CT_VOID DBThread::QueryUserHongBaoInfo(acl::aio_socket_stream * pSocket, acl::db_handle * db, MSG_CS_Query_UserHongBao_Info * pUserHongBaoInfo)
{
	acl::query query;
	//查询猜中红包信息
	query.create_sql("select hongbaoRecord.userid,hongbaoRecord.`name`, hongbaoRecord.score , hongbaoRecord.recvUserid FROM hongbaoRecord\
		INNER JOIN account.`user` ON  account.`user`.userid = hongbaoRecord.userid \
		where hongbaoRecord.recvUserid = :recvUserid OR hongbaoRecord.userid = :userid ORDER BY hongbaoRecord.recvTime desc LIMIT 10")
		.set_format("recvUserid", "%u", pUserHongBaoInfo->dwUserID)
		.set_format("userid", "%u", pUserHongBaoInfo->dwUserID);
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select hongbaoRecord fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::vector<MSG_SC_Query_UserHongBaoRecord_Info> vecHongbaoInfo;
	std::vector<CT_DWORD> vecUserID;
	for (size_t i = 0; i < db->length(); i++)
	{
		MSG_SC_Query_UserHongBaoRecord_Info userHongBaoInfo;
		memset(&userHongBaoInfo, 0, sizeof(MSG_SC_Query_UserHongBaoRecord_Info));
		const acl::db_row* row = (*db)[i];
	
		userHongBaoInfo.dwHongBaoScore = atoi((*row)["score"]);
		std::string strHongBaoName = (*row)["name"];
		CT_DWORD dwUserid = (CT_DWORD)atoi((*row)["userid"]);	
		CT_DWORD dwRecvUserid = (CT_DWORD)atoi((*row)["recvUserid"]);
		_snprintf_info(userHongBaoInfo.szHongBaoName, sizeof(userHongBaoInfo.szHongBaoName), "%s", Utility::Ansi2Utf8(strHongBaoName).c_str());

		if (dwRecvUserid == 0)
		{
			userHongBaoInfo.cbType = 2;//回收
			vecUserID.push_back(0);
		}
		else if (dwUserid == pUserHongBaoInfo->dwUserID)
		{
			userHongBaoInfo.cbType = 0;//同意
			vecUserID.push_back(dwRecvUserid);
		}
		else if(dwRecvUserid == pUserHongBaoInfo->dwUserID)
		{
			userHongBaoInfo.cbType = 1;//猜中
			vecUserID.push_back(dwUserid);
		}
		
		vecHongbaoInfo.push_back(userHongBaoInfo);
	}
	db->free_result();

	for (auto i = 0; i != (int)vecUserID.size(); ++i)
	{
		if (vecUserID[i] != 0)
		{
			query.reset();
			query.create_sql("select account.`user`.nickname FROM account.`user` where account.`user`.userid = :userid ")
				.set_format("userid", "%u", vecUserID[i]);

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << "select account fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				continue;
			}
			for (size_t j = 0; j < db->length(); j++)
			{
				const acl::db_row* row = (*db)[j];
				std::string strNickname = (*row)["nickname"];
				_snprintf_info(vecHongbaoInfo[i].szNickName, sizeof(vecHongbaoInfo[i].szNickName), "%s", Utility::Ansi2Utf8(strNickname).c_str());
			}
			db->free_result();
		}
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
	CT_DWORD dwSendSize = 0;
	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_QUERY_MAIN;
	msgHead.dwSubID = SUB_SC_QUERY_USER_HONGBAO_RECORD_INFO;
	msgHead.dwValue2 = pUserHongBaoInfo->dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	for (auto&it : vecHongbaoInfo)
	{
		//发送数据
		if ((dwSendSize + sizeof(MSG_SC_Query_UserHongBaoRecord_Info)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			msgHead.dwMainID = MSG_QUERY_MAIN;
			msgHead.dwSubID = SUB_SC_QUERY_USER_HONGBAO_RECORD_INFO;
			msgHead.dwValue2 = pUserHongBaoInfo->dwUserID;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}

		memcpy(szBuffer + dwSendSize, &it, sizeof(MSG_SC_Query_UserHongBaoRecord_Info));
		dwSendSize += sizeof(MSG_SC_Query_UserHongBaoRecord_Info);
	}
	if (dwSendSize > 0)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}

CT_VOID DBThread::LoadTaskInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT * FROM taskConfig");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select task fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::vector<tagTaskModel> vecTaskModel;
	tagTaskModel taskModel;
	for (size_t i = 0; i < db->length(); i++)
	{
		memset(&taskModel, 0, sizeof(tagTaskModel));
		const acl::db_row* row = (*db)[i];
		taskModel.wTaskID = atoi((*row)["taskID"]);
		taskModel.wTaskType = atoi((*row)["taskType"]);
		taskModel.wCompleteType = atoi((*row)["completeType"]);
		taskModel.wGameID = atoi((*row)["gameID"]);
		taskModel.wKindID = atoi((*row)["kindID"]);
		taskModel.wRoomKindID = atoi((*row)["roomKindID"]);
		taskModel.dwTotalProgress = atoi((*row)["totalProgress"]);
		taskModel.dwReward = (CT_DWORD)(atoi((*row)["reward"]) * TO_DOUBLE);
		//std::string strTaskDesc = (*row)["reward"];
		//_snprintf_info(taskModel.szTaskDesc, sizeof(taskModel.szTaskDesc), "%s", Utility::Ansi2Utf8(strTaskDesc).c_str());

		vecTaskModel.push_back(taskModel);
	}
	db->free_result();

	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
	CT_DWORD dwSendSize = 0;
	for (auto&it : vecTaskModel)
	{
		//发送数据
		if ((dwSendSize + sizeof(tagTaskModel)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_TASK_INFO, szBuffer, dwSendSize);
			dwSendSize = 0;
		}

		memcpy(szBuffer + dwSendSize, &it, sizeof(tagTaskModel));
		dwSendSize += sizeof(tagTaskModel);
	}
	if (dwSendSize > 0)
	{
		CNetModule::getSingleton().Send(pSocket, MSG_CSDB_MAIN, SUB_DB2CS_TASK_INFO, szBuffer, dwSendSize);
	}
}

CT_VOID DBThread::InsertTaskInfo(acl::db_handle* db, MSG_CS2DB_Insert_Task* pTask)
{
	acl::query query;
	query.create_sql("INSERT INTO userTask(userid, taskid, currProgress, recvTime, finish) VALUES (:userid, :taskid, 0, :recvTime, 0)")
		.set_format("userid", "%u", pTask->dwUserID)
		.set_format("taskid", "%u", pTask->wTaskID)
		.set_format("recvTime", "%u", CT_DWORD(time(NULL)));

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert task fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::UpdateTaskInfo(acl::db_handle* db, MSG_CS2DB_Update_Task* pTask)
{
	acl::query query;
	query.create_sql("UPDATE userTask SET currProgress = :progress, recvTime = :recvTime, finish = :finish WHERE userid = :userid AND taskid = :taskid  LIMIT 1")
		.set_format("progress", "%u", pTask->dwCurrProgress)
		.set_format("recvTime", "%u", pTask->dwReceiveTime)
		.set_format("finish", "%d", pTask->bFinish)
		.set_format("userid", "%u", pTask->dwUserID)
		.set_format("taskid", "%u", pTask->wTaskID);

	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update task fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::ResetDailyTask()
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	do 
	{
		acl::query query;
		//query.create_sql("UPDATE usertask INNER JOIN platform.taskconfig ON usertask.taskid = platform.taskconfig.taskID
		//	set usertask.currProgress = 0, usertask.recvTime = UNIX_TIMESTAMP(), usertask.finish = 0  WHERE platform.taskconfig.taskType = 1");
		query.create_sql("update usertask SET usertask.currProgress = 0, usertask.recvTime = UNIX_TIMESTAMP(), usertask.finish = 0");
		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "reset task fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
	} while (0);
	
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::LoadWealthGodComingCfg()
{
	acl::db_pool* pool = GetPlatformdbPool();
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
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM wealth_god_config ORDER BY startTime ");
		if (db->exec_select(query) ==false)
        {
            LOG(WARNING) << "load wealth god coming fail. errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }

        std::vector<tagWealthGodComingCfg> vecWealthGodComing;
        tagWealthGodComingCfg wealthGodComing;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            wealthGodComing.wID = (CT_WORD)atoi((*row)["id"]);
            wealthGodComing.wGameID = (CT_WORD)atoi((*row)["gameid"]);
            wealthGodComing.wKindID = (CT_WORD)atoi((*row)["kindid"]);
            wealthGodComing.wRoomKindID = (CT_WORD)atoi((*row)["roomkind"]);

            //开始时间
            std::string strStartTime = (*row)["startTime"];
            std::vector<std::string> vecStartTime;
            Utility::stringSplit(strStartTime, ":", vecStartTime);
            if (vecStartTime.size() != 3)
            {
                LOG(WARNING) << "wealth god start time config error. id: " << wealthGodComing.wID;
                continue;
            }
            wealthGodComing.cbStartHour = (CT_BYTE)atoi(vecStartTime[0].c_str());
            wealthGodComing.cbStartMin = (CT_BYTE)atoi(vecStartTime[1].c_str());

            //结束时间
            std::string strEndTime = (*row)["EndTime"];
            std::vector<std::string> vecEndTime;
            Utility::stringSplit(strEndTime, ":", vecEndTime);
            if (vecEndTime.size() != 3)
            {
                LOG(WARNING) << "wealth god end time config error. id: " << wealthGodComing.wID;
                continue;
            }
            wealthGodComing.cbEndHour = (CT_BYTE)atoi(vecEndTime[0].c_str());
            wealthGodComing.cbEndMin = (CT_BYTE)atoi(vecEndTime[1].c_str());

            wealthGodComing.wRewardRatio = (CT_WORD)atoi((*row)["rewardRatio"]);
            wealthGodComing.wCompleteCount = (CT_WORD)atoi((*row)["value"]);
            wealthGodComing.wSendRewardMinUser = (CT_WORD)atoi((*row)["minUser"]);
            wealthGodComing.cbJudgeStander = (CT_BYTE)atoi((*row)["standard"]);
			wealthGodComing.cbState = (CT_BYTE)atoi((*row)["status"]);
			vecWealthGodComing.emplace_back(wealthGodComing);
        }

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
			CT_DWORD dwSendSize = 0;
			for (auto&it : vecWealthGodComing)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagWealthGodComingCfg)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_CONFIG, szBuffer, dwSendSize);
					dwSendSize = 0;
				}

				memcpy(szBuffer + dwSendSize, &it, sizeof(tagWealthGodComingCfg));
				dwSendSize += sizeof(tagWealthGodComingCfg);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_CONFIG, szBuffer, dwSendSize);
			}
		}

	}while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::LoadWealthGodComingRewardCfg()
{
	acl::db_pool* pool = GetPlatformdbPool();
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
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM wealth_god_reward");
		if (db->exec_select(query)==false)
		{
			LOG(WARNING) << "load wealth god coming reward fail. errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		std::vector<tagWealthGodComingReward> vecWealthGodReward;
		tagWealthGodComingReward wealthGodComingReward;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row *row = (*db)[i];
			wealthGodComingReward.wGameID = (CT_WORD)atoi((*row)["gameid"]);
            wealthGodComingReward.wKindID = (CT_WORD)atoi((*row)["kindid"]);
            wealthGodComingReward.wRoomKindID = (CT_WORD)atoi((*row)["roomkind"]);
			wealthGodComingReward.cbRank = (CT_BYTE)atoi((*row)["rank"]);
			wealthGodComingReward.cbRewardRate = (CT_BYTE)atoi((*row)["rateScore"]);
			wealthGodComingReward.dwRewardScore = (CT_DWORD)atoi((*row)["minScore"]);
			vecWealthGodReward.push_back(wealthGodComingReward);
		}

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
			CT_DWORD dwSendSize = 0;
			for (auto&it : vecWealthGodReward)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagWealthGodComingReward)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_REWARD, szBuffer, dwSendSize);
					dwSendSize = 0;
				}

				memcpy(szBuffer + dwSendSize, &it, sizeof(tagWealthGodComingReward));
				dwSendSize += sizeof(tagWealthGodComingReward);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_REWARD, szBuffer, dwSendSize);
			}
		}
	}while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::LoadWealthGodComingIndex()
{
    acl::db_pool* pool = GetPlatformdbPool();
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
    do
    {
        acl::query query;
        query.create_sql("SELECT gameid, kindid, roomKind, `index` FROM wealth_god_index");
        
        if (db->exec_select(query)==false)
        {
            LOG(WARNING) << "load wealth god coming index fail. errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
        
        std::vector<tagWealthGodComingLastIndex> vecWealthGodComingIndex;
		tagWealthGodComingLastIndex wealthGodComingIndex;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row *row = (*db)[i];
			wealthGodComingIndex.wGameID = (CT_WORD)atoi((*row)["gameid"]);
			wealthGodComingIndex.wKindID = (CT_WORD)atoi((*row)["kindid"]);
			wealthGodComingIndex.wRoomKindID = (CT_WORD)atoi((*row)["roomKind"]);
			wealthGodComingIndex.dwLastIndex = (CT_DWORD)atoi((*row)["index"]);
			
			vecWealthGodComingIndex.push_back(wealthGodComingIndex);
        }
        
        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
            CT_DWORD dwSendSize = 0;
            for (auto&it : vecWealthGodComingIndex)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagWealthGodComingLastIndex)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_INDEX, szBuffer, dwSendSize);
                    dwSendSize = 0;
                }
                
                memcpy(szBuffer + dwSendSize, &it, sizeof(tagWealthGodComingLastIndex));
                dwSendSize += sizeof(tagWealthGodComingLastIndex);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_INDEX, szBuffer, dwSendSize);
            }
        }
    }while (0);
    
    db->free_result();
    pool->put(db);
}

struct tagWealtghGodComingGame
{
	CT_WORD 	wGameID;
	CT_WORD 	wKindID;
	CT_WORD 	wRoomKindID;
};
bool operator < (struct tagWealtghGodComingGame const &a,struct tagWealtghGodComingGame const &b)
{
	return a.wGameID*100+a.wKindID*10+a.wRoomKindID < b.wGameID*100+b.wKindID*10+b.wRoomKindID;
}

CT_VOID DBThread::LoadWealthGodComingHistoryData()
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}
	do
	{
		acl::query query;
		query.create_sql("SELECT * from platform.wealth_god_config");
		if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "select wealth god coming id fail. errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
		
        std::set<tagWealtghGodComingGame> setWgcGame;
		tagWealtghGodComingGame wgcGame;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row *row = (*db)[i];
			wgcGame.wGameID = (CT_WORD)atoi((*row)["gameid"]);
			wgcGame.wKindID = (CT_WORD)atoi((*row)["kindid"]);
			wgcGame.wRoomKindID = (CT_WORD)atoi((*row)["roomKind"]);
			
			if (setWgcGame.find(wgcGame) == setWgcGame.end())
				setWgcGame.insert(wgcGame);
        }
        
        struct tagWealtghGodComingGameIndex
		{
			CT_WORD 	wGameID;
			CT_WORD 	wKindID;
			CT_WORD 	wRoomKindID;
			CT_DWORD 	dwIndex;
		};
        
        for (auto& it : setWgcGame)
        {
            query.reset();
            query.create_sql("SELECT * FROM wealth_god_data where gameid = :gameid AND kindid = :kindid AND roomKind = :roomKind order by `index` desc LIMIT 10")
              .set_format("gameid", "%u", it.wGameID)
			  .set_format("kindid", "%u", it.wKindID)
			  .set_format("roomKind", "%u", it.wRoomKindID);
			
            if (db->exec_select(query)==false)
            {
                LOG(WARNING) << "load wealth god coming history data fail. errno: " << db->get_errno() << ", error: " << db->get_error();
                break;
            }
            
            std::vector<tagWealthGodComingData> vecWealthGodComingData;
            std::vector<tagWealtghGodComingGameIndex> vecWealthGodComingGameIndex;
            tagWealthGodComingData wealthGodComingData;
			tagWealtghGodComingGameIndex wealthGodComingGameIndex;
            for (size_t i = 0; i < db->length(); i++)
            {
            	memset(&wealthGodComingGameIndex, 0 , sizeof(wealthGodComingGameIndex));
                const acl::db_row *row = (*db)[i];
                wealthGodComingData.wID = (CT_WORD)atoi((*row)["id"]);
                wealthGodComingData.dwIndex = (CT_WORD)atoi((*row)["index"]);
                wealthGodComingData.llRevenue = atoll((*row)["revenue"]);
                std::string startTime = (*row)["startTime"];
                std::string endTime = (*row)["endTime"];
	
				wealthGodComingGameIndex.wGameID = (CT_WORD)atoi((*row)["gameid"]);
				wealthGodComingGameIndex.wKindID = (CT_WORD)atoi((*row)["kindid"]);
				wealthGodComingGameIndex.wRoomKindID = (CT_WORD)atoi((*row)["roomKind"]);
				wealthGodComingGameIndex.dwIndex = wealthGodComingData.dwIndex;
				vecWealthGodComingGameIndex.push_back(wealthGodComingGameIndex);
				
                //将时间字符串转化为tm时间
                tm tmStart, tmEnd;
                #ifdef _OS_WIN32_CODE
                int year, month, day, hour, minute,second;
                sscanf(startTime.c_str(),"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
                tmStart.tm_year  = year-1900;
                tmStart.tm_mon   = month-1;
                tmStart.tm_mday  = day;
                tmStart.tm_hour  = hour;
                tmStart.tm_min   = minute;
                tmStart.tm_sec   = second;
                tmStart.tm_isdst = 0;
                
                sscanf(endTime.c_str(),"%d-%d-%d %d:%d:%d", &year, &month, &day, &hour, &minute, &second);
                tmEnd.tm_year  = year-1900;
                tmEnd.tm_mon   = month-1;
                tmEnd.tm_mday  = day;
                tmEnd.tm_hour  = hour;
                tmEnd.tm_min   = minute;
                tmEnd.tm_sec   = second;
                tmEnd.tm_isdst = 0;
                #else
                strptime(startTime.c_str(), "%Y-%m-%d %H:%M:%S", &tmStart); //将字符串转换为tm时间
                strptime(endTime.c_str(), "%Y-%m-%d %H:%M:%S", &tmEnd); 	//将字符串转换为tm时间
                #endif
                
                _snprintf_info(wealthGodComingData.szDate, sizeof(wealthGodComingData.szDate), "%d-%02d-%02d", tmStart.tm_year+1900, tmStart.tm_mon+1, tmStart.tm_mday);
                _snprintf_info(wealthGodComingData.szTime, sizeof(wealthGodComingData.szTime), "%02d:%02d-%02d:%02d", tmStart.tm_hour,tmStart.tm_min, tmEnd.tm_hour, tmEnd.tm_min);
                vecWealthGodComingData.push_back(wealthGodComingData);
            }
			db->free_result();
	
			//发送每场活动的历史数据
			CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
			if (pCenterInfo && pCenterInfo->pCenterSocket)
			{
				CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
				CT_DWORD dwSendSize = 0;
				for (auto&it : vecWealthGodComingData)
				{
					//发送数据
					if ((dwSendSize + sizeof(tagWealthGodComingData)) > sizeof(szBuffer))
					{
						CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_HISTORY_DATA, szBuffer, dwSendSize);
						dwSendSize = 0;
					}
			
					memcpy(szBuffer + dwSendSize, &it, sizeof(tagWealthGodComingData));
					dwSendSize += sizeof(tagWealthGodComingData);
				}
				if (dwSendSize > 0)
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_HISTORY_DATA, szBuffer, dwSendSize);
				}
			}
	
			std::vector<tagWealthGodComingUserInfoEx> vecWealthGodComingUserInfo;
			tagWealthGodComingUserInfoEx wealthGodComingUserInfoEx;
            //查询对应的玩家资料
			for (auto& itGameIndex : vecWealthGodComingGameIndex)
			{
				query.reset();
				query.create_sql("SELECT wealth_god_userinfo.userid, \n"
				                 "account.`user`.nickname,\n"
								 "wealth_god_userinfo.rank, \n"
								 "wealth_god_userinfo.revenue, \n"
								 "wealth_god_userinfo.completeCount,\n"
								 "wealth_god_userinfo.userScore,\n"
								 "wealth_god_userinfo.rewardScore,\n"
								 "account.userinfo.gender, \n"
								 "account.userinfo.headId, \n"
								 "account.userdata.vip2 \n"
								 "FROM wealth_god_userinfo \n"
								 "INNER JOIN account.userinfo on account.userinfo.userid = wealth_god_userinfo.userid\n"
								 "INNER JOIN account.userdata on account.userdata.userid = wealth_god_userinfo.userid \n"
		                         "INNER JOIN account.`user` on account.`user`.userid = wealth_god_userinfo.userid \n"
		                         "where gameid = :gameid AND kindid = :kindid AND roomKind = :roomKind AND `index` = :index")
				.set_format("gameid", "%u", itGameIndex.wGameID)
				.set_format("kindid", "%u", itGameIndex.wKindID)
				.set_format("roomKind", "%u", itGameIndex.wRoomKindID)
				.set_format("index", "%u", itGameIndex.dwIndex);
				
				if (db->exec_select(query) == false)
				{
					LOG(WARNING) << "load wealth god coming history user fail. errno: " << db->get_errno() << ", error: " << db->get_error();
					break;
				}
				
				wealthGodComingUserInfoEx.wGameID = itGameIndex.wGameID;
				wealthGodComingUserInfoEx.wKindID = itGameIndex.wKindID;
				wealthGodComingUserInfoEx.wRoomKindID = itGameIndex.wRoomKindID;
				wealthGodComingUserInfoEx.dwIndex = itGameIndex.dwIndex;
				for (size_t i = 0; i < db->length(); i++)
				{
					const acl::db_row *row = (*db)[i];
					wealthGodComingUserInfoEx.dwUserID = (CT_DWORD) atoi((*row)["userid"]);
					std::string nickName = (*row)["nickname"];
					_snprintf_info(wealthGodComingUserInfoEx.szNickName, sizeof(wealthGodComingUserInfoEx.szNickName), "%s", nickName.c_str());
					CT_BYTE cbRank = (CT_BYTE) atoi((*row)["rank"]);
					wealthGodComingUserInfoEx.cbRank = cbRank > 0 ? cbRank : 255;
					wealthGodComingUserInfoEx.dwRevenue = (CT_DWORD)atoi((*row)["revenue"]);
					wealthGodComingUserInfoEx.wCompleteCount = (CT_WORD)atoi((*row)["completeCount"]);
					wealthGodComingUserInfoEx.dwScore = (CT_DWORD)atoi((*row)["userScore"]);
					wealthGodComingUserInfoEx.dwRewardScore = (CT_DWORD)atoi((*row)["rewardScore"]);
					wealthGodComingUserInfoEx.cbGender = (CT_BYTE)atoi((*row)["gender"]);
					wealthGodComingUserInfoEx.cbHeadIndex = (CT_BYTE)atoi((*row)["headId"]);
					wealthGodComingUserInfoEx.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
					vecWealthGodComingUserInfo.push_back(wealthGodComingUserInfoEx);
				}
				db->free_result();
			}
			
			std::sort(vecWealthGodComingUserInfo.begin(), vecWealthGodComingUserInfo.end(),  [](tagWealthGodComingUserInfoEx& a, tagWealthGodComingUserInfoEx& b) {
			  return a.cbRank < b.cbRank;
			});
			
			//中心服务器
			if (pCenterInfo && pCenterInfo->pCenterSocket)
			{
				CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
				CT_DWORD dwSendSize = 0;
				for (auto& itUser : vecWealthGodComingUserInfo)
				{
					//发送数据
					if ((dwSendSize + sizeof(tagWealthGodComingUserInfoEx)) > sizeof(szBuffer))
					{
						CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_HISTORY_USER, szBuffer, dwSendSize);
						dwSendSize = 0;
					}
			
					memcpy(szBuffer + dwSendSize, &itUser, sizeof(tagWealthGodComingUserInfoEx));
					dwSendSize += sizeof(tagWealthGodComingUserInfoEx);
				}
				if (dwSendSize > 0)
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_WEALTH_GOD_HISTORY_USER, szBuffer, dwSendSize);
				}
			}
        }
	}while (0);
	
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::UpdateWealthGodComingIndex(tagWealthGodComingLastIndex* pIndex)
{
	acl::db_pool* pool = GetPlatformdbPool();
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
	
	do
	{
		acl::query query;
		query.create_sql("INSERT INTO wealth_god_index VALUES (:gameid, :kindid, :roomkind, :index) ON DUPLICATE KEY UPDATE wealth_god_index.`index` = :index ")
		.set_format("gameid", "%u", pIndex->wGameID)
		.set_format("kindid", "%u", pIndex->wKindID)
		.set_format("roomkind", "%u", pIndex->wRoomKindID)
		.set_format("index", "%d", pIndex->dwLastIndex);
		
		if (db->exec_update(query) ==false)
		{
			LOG(WARNING) << "INSERT wealth god index fail, errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}
		
	}while (0);
	
	pool->put(db);
}

CT_VOID DBThread::InsertWealthGodComingData(MSG_CS2DB_Insert_WGC_Data* pData)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}
	
	do
	{
		acl::query query;
		query.create_sql("INSERT INTO wealth_god_data VALUES (:gameid, :kindid, :roomkind, :index, :id, :revenue, :startTime, :EndTime)")
		.set_format("gameid", "%u", pData->wGameID)
		.set_format("kindid", "%u", pData->wKindID)
		.set_format("roomkind", "%u", pData->wRoomKind)
		.set_format("index", "%d", pData->dwIndex)
		.set_format("id", "%d", pData->wActivityID)
		.set_format("revenue", "%lld", pData->llRevenue)
		.set_format("startTime", "%s", pData->szStartTime)
		.set_format("EndTime", "%s", pData->szEndTime);
		
		if (db->exec_update(query) ==false)
		{
			LOG(WARNING) << "INSERT wealth_god_data fail, errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}
		
	}while (0);
	
	pool->put(db);
}

CT_VOID DBThread::UpdateWealthGodComingData(MSG_CS2DB_Update_WGC_Data* pData)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}
	
	do
	{
		acl::query query;
		query.create_sql("UPDATE wealth_god_data SET revenue = :revenue WHERE gameid = :gameid and kindid = :kindid and roomKind = :roomKind and `index` = :index ")
		.set_format("gameid", "%u", pData->wGameID)
		.set_format("kindid", "%u", pData->wKindID)
		.set_format("roomKind", "%u", pData->wRoomKind)
		.set_format("index", "%d", pData->dwIndex)
		.set_format("revenue", "%lld", pData->llRevenue);
		
		if (db->exec_update(query) ==false)
		{
			LOG(WARNING) << "UPDATE wealth_god_data fail, errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}
		
	}while (0);
	
	pool->put(db);
	
}

CT_VOID DBThread::UpdateWealthGodComingUserInfo(MSG_CS2DB_Update_WGC_UserInfo* pUserInfo)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}
	
	do
	{
		acl::query query;
		query.create_sql("INSERT INTO wealth_god_userinfo VALUES (:gameid,:kindid,:roomKindid,:index,:userid,:rank,:revenue,:completeCount,:userScore, :rewardScore)"
				   " ON DUPLICATE KEY UPDATE rank = :rank,  revenue = :revenue, completeCount = :completeCount, userScore = :userScore, rewardScore = :rewardScore")
		.set_format("gameid", "%u", pUserInfo->wGameID)
		.set_format("kindid", "%u", pUserInfo->wKindID)
		.set_format("roomKindid", "%u", pUserInfo->wRoomKind)
		.set_format("index", "%u", pUserInfo->dwIndex)
		.set_format("userid", "%u", pUserInfo->dwUserID)
		.set_format("rank", "%d", pUserInfo->cbRank)
		.set_format("revenue", "%u", pUserInfo->dwRevenue)
		.set_format("completeCount", "%u", pUserInfo->wCompleteCount)
		.set_format("userScore", "%u", pUserInfo->dwScore)
		.set_format("rewardScore", "%u", pUserInfo->dwRewardScore);
		
		if (db->exec_update(query) ==false)
		{
			LOG(WARNING) << "UPDATE wealth_god_userinfo fail, errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}
		//LOG(WARNING) << "user info sql: " << query.to_string().c_str();
		
	}while (0);
	
	pool->put(db);
}

CT_VOID DBThread::LoadRedPacketStatus()
{
	acl::db_pool* pool = GetPlatformdbPool();
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

	do
	{
		acl::query query;
		query.create_sql("SELECT StatusValue from systemstatus where statusName = 'redPacket'");
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select red packet status cfg fail, error: " << db->get_errno();
			break;
		}

		tagRedPacketStatus redPacketStatus;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			redPacketStatus.cbStatus = (CT_BYTE)atoi((*row)["StatusValue"]);
		}
		db->free_result();

		query.reset();
		query.create_sql("SELECT statusDescription from systemstatus where statusName = 'redPacketDes'");
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select red packet des cfg fail, error: " << db->get_errno();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			_snprintf_info(redPacketStatus.szDes, sizeof(redPacketStatus.szDes), "%s", (*row)["statusDescription"]);
		}
		db->free_result();

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_REDPACKET_STATUS, &redPacketStatus,
											sizeof(tagRedPacketStatus));
		}
	}while (0);

	pool->put(db);
}

//新年红包
CT_VOID DBThread::LoadRedPacketCfg()
{
    acl::db_pool* pool = GetPlatformdbPool();
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

    do
    {
        acl::query query;
        query.create_sql("SELECT * FROM redpacket_room_config");
        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "select red packet room cfg fail, error: " << db->get_errno();
            break;
        }

        std::vector<tagRedPacketRoomCfg> vecRedPacketRoomCfg;
        tagRedPacketRoomCfg redPacketRoomCfg;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            redPacketRoomCfg.dwGameIndex = (CT_DWORD)atoi((*row)["gameIndex"]);
            redPacketRoomCfg.dwRedPacketValue = (CT_DWORD)atoi((*row)["value"]);
            vecRedPacketRoomCfg.push_back(redPacketRoomCfg);
        }
        db->free_result();

        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
            CT_DWORD dwSendSize = 0;
            for (auto&it : vecRedPacketRoomCfg)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagRedPacketRoomCfg)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_REDPACKET_ROOM_CFG, szBuffer, dwSendSize);
                    dwSendSize = 0;
                }

                memcpy(szBuffer + dwSendSize, &it, sizeof(tagRedPacketRoomCfg));
                dwSendSize += sizeof(tagRedPacketRoomCfg);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_REDPACKET_ROOM_CFG, szBuffer, dwSendSize);
            }
        }
    }while (0);

    pool->put(db);
}

CT_VOID DBThread::LoadRedPacketIndex()
{
    acl::db_pool* pool = GetPlatformdbPool();
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

    do
    {
        acl::query query;
        query.create_sql("SELECT * FROM redpacket_index WHERE `status` = 1 and stage != 4");
        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "select red packet index fail, error: " << db->get_errno();
            break;
        }

        std::vector<tagRedPacketIndex> vecRedPacketIndex;
        tagRedPacketIndex redPacketIndex;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            redPacketIndex.dwIndex = (CT_DWORD)atoi((*row)["index"]);
            std::string strIdList = (*row)["idList"];
            std::vector<std::string> vecRoomCfgID;
            Utility::stringSplit(strIdList, ",", vecRoomCfgID);
            int nIdCount = 0;
            for (auto& it : vecRoomCfgID)
            {
                redPacketIndex.dwGameIndex[nIdCount++] = (CT_DWORD)atoi(it.c_str());
                if (nIdCount > RED_PACKET_ROOM_COUNT)
                {
                    break;
                }
            }

            //红包金额
            redPacketIndex.dwRedPacketScore = (CT_DWORD)atoi((*row)["totalScore"]);
			redPacketIndex.dwRedRacketGrabbedScore = (CT_DWORD)atoi((*row)["grabScore"]);

			_snprintf_info(redPacketIndex.szStartDate, sizeof(redPacketIndex.szStartDate), "%s", (*row)["startDate"]);

            //准备时间
            {
                std::string strReadyTime = (*row)["readyTime"];
                std::vector<std::string> vecReadyTime;
                Utility::stringSplit(strReadyTime, ":", vecReadyTime);
                if (vecReadyTime.size() != 3)
                {
                    LOG(WARNING) << "red packet ready time config error. red packet index: " << redPacketIndex.dwIndex;
                    continue;
                }
                redPacketIndex.cbReadyHour = (CT_BYTE)atoi(vecReadyTime[0].c_str());
                redPacketIndex.cbReadyMin = (CT_BYTE)atoi(vecReadyTime[1].c_str());
            }

            //开始时间
            {
                std::string strStartTime = (*row)["startTime"];
                std::vector<std::string> vecStartTime;
                Utility::stringSplit(strStartTime, ":", vecStartTime);
                if (vecStartTime.size() != 3)
                {
                    LOG(WARNING) << "red packet start time config error. red packet index: " << redPacketIndex.dwIndex;
                    continue;
                }
                redPacketIndex.cbStartHour = (CT_BYTE)atoi(vecStartTime[0].c_str());
                redPacketIndex.cbStartMin = (CT_BYTE)atoi(vecStartTime[1].c_str());
            }

            //结束时间
            {
                std::string strEndTime = (*row)["endTime"];
                std::vector<std::string> vecEndTime;
                Utility::stringSplit(strEndTime, ":", vecEndTime);
                if (vecEndTime.size() != 3)
                {
                    LOG(WARNING) << "red packet end time config error. red packet index: " << redPacketIndex.dwIndex;
                    continue;
                }
                redPacketIndex.cbEndHour = (CT_BYTE)atoi(vecEndTime[0].c_str());
                redPacketIndex.cbEndMin = (CT_BYTE)atoi(vecEndTime[1].c_str());
            }

            redPacketIndex.cbStage = (CT_BYTE)atoi((*row)["stage"]);
            redPacketIndex.cbState = (CT_BYTE)atoi((*row)["status"]);
            vecRedPacketIndex.push_back(redPacketIndex);
        }
        db->free_result();

        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
            CT_DWORD dwSendSize = 0;
            for (auto&it : vecRedPacketIndex)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagRedPacketIndex)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_REDPACKET_INDEX, szBuffer, dwSendSize);
                    dwSendSize = 0;
                }

                memcpy(szBuffer + dwSendSize, &it, sizeof(tagRedPacketIndex));
                dwSendSize += sizeof(tagRedPacketIndex);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_REDPACKET_INDEX, szBuffer, dwSendSize);
            }
        }

        //加载玩家已经抢到的红包信息
        std::vector<tagRedPacketUserGrabData> vecRedPacketUserGrabData;
        for (auto&it : vecRedPacketIndex)
        {
            tagRedPacketUserGrabData grabData;
            query.reset();
            query.create_sql("SELECT * FROM game.redpacket_grab WHERE `index` = :index")
            .set_format("index", "%u", it.dwIndex);
            if (db->exec_select(query) == false)
            {
                LOG(WARNING) << "select red packet grab info fail, error: " << db->get_errno() << ", error: " << db->get_error();
                break;
            }

            for (size_t i = 0; i < db->length(); i++)
            {
                const acl::db_row *row = (*db)[i];
                grabData.dwIndex = (CT_DWORD) atoi((*row)["index"]);
                CT_DWORD dwGameIndex = (CT_DWORD) atoi((*row)["gameIndex"]);
				grabData.dwUserID = (CT_DWORD) atoi((*row)["userId"]);
				grabData.wGameID = dwGameIndex / 10000;
                grabData.wKindID = (dwGameIndex % 10000) / 100;
                grabData.wRoomKindID = (dwGameIndex % 10000) % 100;
                grabData.dwLuckyMoney = (CT_DWORD) atoi((*row)["luckyMoney"]);
				vecRedPacketUserGrabData.push_back(grabData);
            }
            db->free_result();
        }

        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
            CT_DWORD dwSendSize = 0;
            for (auto&it : vecRedPacketUserGrabData)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagRedPacketUserGrabData)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_REDPACKET_GRAB_INFO, szBuffer, dwSendSize);
                    dwSendSize = 0;
                }

                memcpy(szBuffer + dwSendSize, &it, sizeof(tagRedPacketUserGrabData));
                dwSendSize += sizeof(tagRedPacketUserGrabData);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_REDPACKET_GRAB_INFO, szBuffer, dwSendSize);
            }
        }



    }while (0);

    pool->put(db);
}

CT_VOID DBThread::UpdateRedPacketStage(MSG_CS2DB_Update_RedPacket_Stage* pRedPacketStage)
{
    acl::db_pool* pool = GetPlatformdbPool();
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

    do
    {
        acl::query query;
        query.create_sql("UPDATE redpacket_index SET stage = :stage WHERE `index` = :index")
            .set_format("stage","%d", pRedPacketStage->cbStage)
            .set_format("index", "%u", pRedPacketStage->dwIndex);

        if (db->exec_update(query) ==false)
        {
            LOG(WARNING) << "update red packet index stage fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    }while (0);

    pool->put(db);
}

CT_VOID DBThread::InsertRedPacketGrab(MSG_CS2DB_Insert_RadPacket_GrabInfo* pRedPacketGrabInfo)
{
    acl::db_pool* pool = GetGamedbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get game db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get game db handle fail.";
        return;
    }

    do
    {
        acl::query query;
        query.create_sql("INSERT into game.redpacket_grab(`index`, gameIndex, userId, luckyMoney, isRobot) "
                         "VALUES(:index, :gameIndex, :userId, :luckyMoney, :isRobot)")
            .set_format("index","%u", pRedPacketGrabInfo->dwIndex)
            .set_format("gameIndex", "%u", pRedPacketGrabInfo->wGameID*10000+pRedPacketGrabInfo->wKindID*100+pRedPacketGrabInfo->wRoomKindID)
            .set_format("userId", "%u", pRedPacketGrabInfo->dwUserID)
            .set_format("luckyMoney", "%u", pRedPacketGrabInfo->dwLuckyMoney)
            .set_format("isRobot", "%d", pRedPacketGrabInfo->cbIsRobot);
        if (db->exec_update(query) ==false)
        {
            LOG(WARNING) << "insert red packet grab info fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }

        query.reset();
        query.create_sql("UPDATE platform.redpacket_index SET grabScore = grabScore + :grabScore WHERE `index` = :index")
            .set_format("index","%u", pRedPacketGrabInfo->dwIndex)
            .set_format("grabScore", "%u", pRedPacketGrabInfo->dwLuckyMoney);
        if (db->exec_update(query) ==false)
        {
            LOG(WARNING) << "update red packet index grab score fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    }while (0);

    pool->put(db);
}


CT_VOID DBThread::LoadDuoBaoCfg()
{
	acl::db_pool* pool = GetPlatformdbPool();
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

	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM duobao_config");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select score mode fail, error: " << db->get_errno();
			break;
		}

		std::vector<tagDuoBaoCfg> vecDuoBaoCfg;
		tagDuoBaoCfg duoBaoCfg;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			duoBaoCfg.wID = (CT_WORD)atoi((*row)["id"]);
			duoBaoCfg.wSingleScore = (CT_WORD)atoi((*row)["singleScore"]);
			duoBaoCfg.dwUserLeaseScore = (CT_DWORD)atoi((*row)["userLeaseScore"]);
			duoBaoCfg.wlimitCount = (CT_WORD)atoi((*row)["limitCount"]);
			duoBaoCfg.dwTotalCount = (CT_WORD)atoi((*row)["totalCount"]);
			duoBaoCfg.dwTimeLeft = (CT_DWORD)atoi((*row)["time"]);
			duoBaoCfg.dwReward = (CT_DWORD)atoi((*row)["reward"]);
			duoBaoCfg.wRevenueRation = (CT_DWORD)atoi((*row)["revenueRation"]);
			duoBaoCfg.dwIndex = (CT_DWORD)atoi((*row)["index"]);
			duoBaoCfg.cbState = (CT_BYTE)atoi((*row)["status"]);
			_snprintf_info(duoBaoCfg.szTitle, sizeof(duoBaoCfg.szTitle), "%s", (*row)["title"]);
			vecDuoBaoCfg.push_back(duoBaoCfg);
		}
		db->free_result();

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
			CT_DWORD dwSendSize = 0;
			for (auto&it : vecDuoBaoCfg)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagDuoBaoCfg)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_CONFIG, szBuffer, dwSendSize);
					dwSendSize = 0;
				}

				memcpy(szBuffer + dwSendSize, &it, sizeof(tagDuoBaoCfg));
				dwSendSize += sizeof(tagDuoBaoCfg);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_CONFIG, szBuffer, dwSendSize);
			}
		}

		//加载历史记录
		for (auto& it : vecDuoBaoCfg)
		{
			LoadDuoBaoHistoryData(it.wID);
		}
	}while (0);

    pool->put(db);
}

CT_VOID DBThread::LoadDuoBaoDoingData()
{
    acl::db_pool* pool = GetGamedbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get game db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get game db handle fail.";
        return;
    }

    do
    {
        acl::query query;
        query.create_sql("SELECT * FROM duobao_data WHERE state = 0");
        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "select duobao_data fail. errno: " << db->get_errno() << ", error: "
                         << db->get_error();
            break;
        }

        std::vector<tagDuoBaoData> vecDuoBaoDoingData;
        tagDuoBaoData duobaoDoingData;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            duobaoDoingData.wID = (CT_WORD)atoi((*row)["id"]);
            duobaoDoingData.dwIndex = (CT_DWORD)atoi((*row)["index"]);
            duobaoDoingData.dwLuckyUserID = (CT_DWORD)atoi((*row)["winnerUser"]);
			duobaoDoingData.dwLuckyNum = (CT_DWORD)atoi((*row)["luckyNum"]);
			duobaoDoingData.dwRewardScore = (CT_DWORD)atoi((*row)["rewardScore"]);
            duobaoDoingData.dwStartTime = (CT_DWORD)atoi((*row)["startTime"]);
            duobaoDoingData.dwEndTime = (CT_DWORD)atoi((*row)["endTime"]);
            duobaoDoingData.cbState = (CT_BYTE)atoi((*row)["state"]);

            vecDuoBaoDoingData.push_back(duobaoDoingData);
        }
        db->free_result();

        std::vector<tagDuoBaoUserInfo> vecDuoBaoDoingUser;
        tagDuoBaoUserInfo duoBaoUserInfo;
        for (auto& it : vecDuoBaoDoingData)
        {
            query.reset();
            query.create_sql("SELECT\n"
							 "\tduobao_userinfo.id,\n"
							 "\tduobao_userinfo.`index`,\n"
							 "\tduobao_userinfo.userId,\n"
							 "\tduobao_userinfo.luckyNum,\n"
							 "\taccount.userinfo.gender,\n"
							 "\taccount.userinfo.headId \n"
							 "FROM\n"
							 "\tduobao_userinfo\n"
							 "\tINNER JOIN account.userinfo ON account.userInfo.userid = duobao_userinfo.userId \n"
							 "WHERE\n"
							 "\tduobao_userinfo.id = :id \n"
							 "\tAND duobao_userinfo.`index` = :index")
                             .set_format("id", "%u", it.wID)
                             .set_format("index", "%u", it.dwIndex);
            if (db->exec_select(query) == false)
            {
                LOG(WARNING) << "select duobao_userinfo fail. errno: " << db->get_errno() << ", error: "
                             << db->get_error();
                break;
            }
            for (size_t i = 0; i < db->length(); i++)
            {
                const acl::db_row* row = (*db)[i];
                duoBaoUserInfo.wID = (CT_WORD)atoi((*row)["id"]);
                duoBaoUserInfo.dwIndex = (CT_DWORD)atoi((*row)["index"]);
                duoBaoUserInfo.dwUserID = (CT_DWORD)atoi((*row)["userId"]);
                duoBaoUserInfo.dwLuckyNum = (CT_DWORD)atoi((*row)["luckyNum"]);
                duoBaoUserInfo.cbSex = (CT_BYTE)atoi((*row)["gender"]);
                duoBaoUserInfo.cbHeadIndex = (CT_BYTE)atoi((*row)["headId"]);
                vecDuoBaoDoingUser.push_back(duoBaoUserInfo);
                //LOG(WARNING) << "load duo bao user. id: " << duoBaoUserInfo.dwUserID << ", lucky num: " << duoBaoUserInfo.dwLuckyNum;
			}
            db->free_result();
        }

        //发送
        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
            CT_DWORD dwSendSize = 0;
            for (auto&it : vecDuoBaoDoingData)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagDuoBaoData)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_DOING_DATA, szBuffer, dwSendSize);
                    dwSendSize = 0;
                }

                memcpy(szBuffer + dwSendSize, &it, sizeof(tagDuoBaoData));
                dwSendSize += sizeof(tagDuoBaoData);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_DOING_DATA, szBuffer, dwSendSize);
            }

            dwSendSize = 0;
            for (auto&it : vecDuoBaoDoingUser)
            {
                //发送数据
                if ((dwSendSize + sizeof(tagDuoBaoUserInfo)) > sizeof(szBuffer))
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_DOING_USER, szBuffer, dwSendSize);
                    dwSendSize = 0;
                }

                memcpy(szBuffer + dwSendSize, &it, sizeof(tagDuoBaoUserInfo));
                dwSendSize += sizeof(tagDuoBaoUserInfo);
            }
            if (dwSendSize > 0)
            {
                CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_DOING_USER, szBuffer, dwSendSize);
            }

			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_FINISH);
        }
    }while (0);

    pool->put(db);
}

CT_VOID	DBThread::LoadDuoBaoHistoryData(CT_WORD wID)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM duobao_data WHERE id = :id AND state != 0  ORDER BY `index` DESC LIMIT 10 ")
		.set_format("id", "%u", wID);

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select duobao_data fail. errno: " << db->get_errno() << ", error: "
						 << db->get_error();
			break;
		}

		std::vector<tagDuoBaoData> vecDuoBaoHistoryData;
		tagDuoBaoData duobaoHistoryData;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			duobaoHistoryData.wID = (CT_WORD)atoi((*row)["id"]);
			duobaoHistoryData.dwIndex = (CT_DWORD)atoi((*row)["index"]);
			duobaoHistoryData.dwLuckyUserID = (CT_DWORD)atoi((*row)["winnerUser"]);
			duobaoHistoryData.dwLuckyNum = (CT_DWORD)atoi((*row)["luckyNum"]);
			duobaoHistoryData.dwStartTime = (CT_DWORD)atoi((*row)["startTime"]);
			duobaoHistoryData.dwEndTime = (CT_DWORD)atoi((*row)["endTime"]);
			duobaoHistoryData.cbState = (CT_BYTE)atoi((*row)["state"]);

			vecDuoBaoHistoryData.push_back(duobaoHistoryData);
		}
		//反过来排序
		std::sort(vecDuoBaoHistoryData.begin(), vecDuoBaoHistoryData.end(), [](tagDuoBaoData& a, tagDuoBaoData& b)
		{
			return a.wID*a.dwIndex < b.wID*b.dwIndex;
		});
		db->free_result();

		std::vector<tagDuoBaoUserInfo> vecDuoBaoHistoryUser;
		tagDuoBaoUserInfo duoBaoUserInfo;
		for (auto& it : vecDuoBaoHistoryData)
		{
			query.reset();
			query.create_sql("SELECT\n"
							 "\tduobao_userinfo.id,\n"
							 "\tduobao_userinfo.`index`,\n"
							 "\tduobao_userinfo.userId,\n"
							 "\tduobao_userinfo.luckyNum,\n"
							 "\taccount.userinfo.gender,\n"
							 "\taccount.userinfo.headId \n"
							 "FROM\n"
							 "\tduobao_userinfo\n"
							 "\tINNER JOIN account.userinfo ON account.userInfo.userid = duobao_userinfo.userId \n"
							 "WHERE\n"
							 "\tduobao_userinfo.id = :id \n"
							 "\tAND duobao_userinfo.`index` = :index ")
				.set_format("id", "%u", it.wID)
				.set_format("index", "%u", it.dwIndex);
			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << "select duobao_userinfo fail. errno: " << db->get_errno() << ", error: "
							 << db->get_error();
				break;
			}
			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				duoBaoUserInfo.wID = (CT_WORD)atoi((*row)["id"]);
				duoBaoUserInfo.dwIndex = (CT_DWORD)atoi((*row)["index"]);
				duoBaoUserInfo.dwUserID = (CT_DWORD)atoi((*row)["userId"]);
				duoBaoUserInfo.dwLuckyNum = (CT_DWORD)atoi((*row)["luckyNum"]);
				duoBaoUserInfo.cbSex = (CT_BYTE)atoi((*row)["gender"]);
				duoBaoUserInfo.cbHeadIndex = (CT_BYTE)atoi((*row)["headId"]);
				vecDuoBaoHistoryUser.push_back(duoBaoUserInfo);
			}
			db->free_result();
		}

		//发送
		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
			CT_DWORD dwSendSize = 0;
			for (auto&it : vecDuoBaoHistoryData)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagDuoBaoData)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_HISTORY_DATA, szBuffer, dwSendSize);
					dwSendSize = 0;
				}

				memcpy(szBuffer + dwSendSize, &it, sizeof(tagDuoBaoData));
				dwSendSize += sizeof(tagDuoBaoData);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_HISTORY_DATA, szBuffer, dwSendSize);
			}

			dwSendSize = 0;
			for (auto&it : vecDuoBaoHistoryUser)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagDuoBaoUserInfo)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_HISTORY_USER, szBuffer, dwSendSize);
					dwSendSize = 0;
				}

				memcpy(szBuffer + dwSendSize, &it, sizeof(tagDuoBaoUserInfo));
				dwSendSize += sizeof(tagDuoBaoUserInfo);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_DUOBAO_HISTORY_USER, szBuffer, dwSendSize);
			}
		}
	}while (0);

	pool->put(db);
}

CT_VOID DBThread::UpdateDuoBaoData(MSG_CS2DB_Update_DuoBao_Data* pDuoBaoInfo)
{
    acl::db_pool* pool = GetGamedbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get game db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get game db handle fail.";
        return;
    }

    do
    {
        acl::query query;
        query.create_sql("INSERT INTO duobao_data VALUES (:id, :index, :winnerUser, :luckyNum, :rewardScore, :startTime, :endTime, :state)"
                         " ON DUPLICATE KEY UPDATE winnerUser = :winnerUser, luckyNum = :luckyNum,  rewardScore = :rewardScore, endTime = :endTime, state = :state")
            .set_format("id","%u", pDuoBaoInfo->wID)
            .set_format("index", "%u", pDuoBaoInfo->dwIndex)
            .set_format("winnerUser", "%u", pDuoBaoInfo->dwLuckyUserID)
            .set_format("luckyNum", "%u", pDuoBaoInfo->dwLuckyNum)
            .set_format("rewardScore", "%u", pDuoBaoInfo->dwRewardScore)
            .set_format("startTime", "%u", pDuoBaoInfo->dwStartTime)
            .set_format("endTime", "%u", pDuoBaoInfo->dwEndTime)
            .set_format("state", "%d", pDuoBaoInfo->cbState);

        if (db->exec_update(query) ==false)
        {
            LOG(WARNING) << "update duo bao data fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }

        if (pDuoBaoInfo->cbState == 0)
        {
            query.reset();
            query.create_sql("UPDATE platform.duobao_config SET `index` = :index WHERE id = :id")
            .set_format("index", "%u", pDuoBaoInfo->dwIndex)
            .set_format("id", "%u", pDuoBaoInfo->wID);

            if (db->exec_update(query) ==false)
            {
                LOG(WARNING) << "update duo bao index fail, errno: " << db->get_errno() << ", error: " << db->get_error();
                break;
            }
        }
    }while (0);

    pool->put(db);
}

CT_VOID DBThread::InsertDuoBaoUser(MSG_CS2DB_Insert_DuoBao_User* pDuoBaoUser)
{
    acl::db_pool* pool = GetGamedbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get game db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get game db handle fail.";
        return;
    }

    do
    {
        acl::query query;
        query.create_sql("INSERT INTO duobao_userinfo VALUES (:id, :index, :userId, :luckyNum, :buyTime)")
            .set_format("id","%u", pDuoBaoUser->wID)
            .set_format("index", "%u", pDuoBaoUser->dwIndex)
            .set_format("userId", "%u", pDuoBaoUser->dwUserID)
            .set_format("luckyNum", "%u", pDuoBaoUser->dwLuckyNum)
            .set_format("buyTime", "%u", (CT_DWORD)time(NULL));

        if (db->exec_update(query) ==false)
        {
            LOG(WARNING) << "insert duo bao user fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }
    }while (0);

    pool->put(db);
}

CT_VOID DBThread::LoadSystemStatus()
{
	acl::db_pool* pool = GetPlatformdbPool();
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
	
	do 
	{
		acl::query query;
		query.create_sql("SELECT StatusValue from systemStatus where statusName = 'scoreMode'");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select score mode fail, error: " << db->get_errno();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			m_systemStatus.cbScoreMode = (CT_BYTE)atoi((*row)["statusValue"]);
		}
		db->free_result();

		query.reset();
		query.create_sql("SELECT StatusValue from systemStatus where statusName = 'privateMode'");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select private mode fail, error: " << db->get_errno();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			m_systemStatus.cbPrivateMode = (CT_BYTE)atoi((*row)["statusValue"]);
		}
		db->free_result();
	} while (0);

	pool->put(db);
}

CT_VOID DBThread::SendSystemStatusToCenter()
{
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SYSTEM_STATUS, &m_systemStatus, sizeof(m_systemStatus));
	}
}

CT_VOID DBThread::LoadScoreRank()
{
	acl::db_pool* pool = GetAccountdbPool();
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

	LoadScoreRank(db);

	pool->put(db);
}

CT_VOID	DBThread::LoadScoreRank(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT userinfo.userid, userinfo.score, userdata.vip2, userinfo.headId, `user`.nickname FROM userinfo \
		INNER JOIN `user` ON userinfo.userid = `user`.userid \
		INNER JOIN userdata ON userinfo.userid = userdata.userid \
		ORDER BY score DESC LIMIT 20");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select score rank fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::vector<tagScoreRank> vecScoreRank;
	for (size_t i = 0; i < db->length(); i++)
	{
		tagScoreRank scoreRank;
		const acl::db_row* row = (*db)[i];
		scoreRank.cbRank = (CT_BYTE)(i + 1);
		scoreRank.dwUserID = atoi((*row)["userid"]);
		scoreRank.dScore = atoll((*row)["score"])*TO_DOUBLE;
		scoreRank.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
		scoreRank.cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
		std::string strNickName = (*row)["nickname"];
		//std::string strHeadUrl = (*row)["headimage"];
		_snprintf_info(scoreRank.szNickName, sizeof(scoreRank.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());
		//_snprintf_info(scoreRank.szHeadUrl, sizeof(scoreRank.szHeadUrl), "%s", Utility::Ansi2Utf8(strHeadUrl).c_str());

		vecScoreRank.push_back(scoreRank);
	}

	db->free_result();

	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		//发送群组成员信息
		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		CT_DWORD dwSendSize = 0;
		for (auto&it : vecScoreRank)
		{
			//发送数据
			if ((dwSendSize + sizeof(tagScoreRank)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SCORE_RANK, szBuffer, dwSendSize);
				dwSendSize = 0;
			}

			memcpy(szBuffer + dwSendSize, &it, sizeof(tagScoreRank));
			dwSendSize += sizeof(tagScoreRank);

			//更新排行榜任务
			//UpdateUserRankTask(it.dwUserID);
		}
		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SCORE_RANK, szBuffer, dwSendSize);
		}
		else
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SCORE_RANK);
		}
	}
}

CT_VOID	DBThread::SwitchTodayScoreRank()
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	SwitchTodayScoreRank(db);

	pool->put(db);
}

CT_VOID DBThread::SwitchTodayScoreRank(acl::db_handle* db)
{
	//if (db->begin_transaction() == false)
	//{
	//	LOG(WARNING) << "begin transaction fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
	//	return;
	//}

	//先删除昨日的赚金榜
	acl::query query;
	query.create_sql("DELETE FROM yesterdayEarnScore");
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "delete yesterdayEarnScore fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		//db->rollback();
		return;
	}
	db->free_result();

	//把今日的赚金榜放到昨日赚金榜上(暂时没有用处，当备份用)
	query.reset();
	//query.create_sql("INSERT INTO yesterdayEarnScore SELECT * FROM todyEarnScore WHERE earnScore > 0 ORDER BY earnScore DESC LIMIT 20");
	query.create_sql("INSERT INTO yesterdayEarnScore SELECT * FROM todayEarnScore");
	if (db->exec_update(query) == false)
	{ 
		LOG(WARNING) << "insert new data to yesterdayEarnScore fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		//db->rollback();
		return;
	}
	db->free_result();

	//if (db->commit() == false)
	//{
	//	LOG(WARNING) << "commit fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
	//	return;
	//}

	//加载今日赚金榜
	LoadTodayScoreRank(db);

	//把今日赚金榜清空
	query.reset();
	query.create_sql("UPDATE todayEarnScore SET earnScore = 0");
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update today earn score to 0 fail, error no: " << db->get_errno() << ", error msg: " << db->get_error();
		return;
	}

	return;
}

CT_VOID	DBThread::ResetTodayScoreRankInCache()
{
	acl::string pattern("earnscore*");
	std::vector<acl::string> vecTodayEarnScoreKey;
	m_redis.clear();
	int keys =m_redis.keys_pattern(pattern.c_str(), &vecTodayEarnScoreKey);
	if (keys > 0)
	{
		std::vector<acl::string> vecVal;
		auto keyCount = vecTodayEarnScoreKey.size();
		for (int i = 0; i != (int)keyCount; ++i)
		{
			vecVal.push_back("0");
		}

		//批量处理
		m_redis.clear();
		m_redis.mset(vecTodayEarnScoreKey, vecVal);
	}
}

CT_VOID	DBThread::LoadTodayScoreRank()
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{

		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	LoadTodayScoreRank(db);

	pool->put(db);
}

CT_VOID DBThread::LoadTodayScoreRank(acl::db_handle* db)
{
	acl::query query;
	query.create_sql("SELECT todayEarnScore.userid, todayEarnScore.earnScore as score, account.userinfo.gender, account.userinfo.headId, account.`user`.nickname, account.userdata.vip2 FROM todayEarnScore \
		INNER JOIN account.`user` ON todayEarnScore.userid = account.`user`.userid \
        INNER JOIN account.userdata ON todayEarnScore.userid = account.userdata.userid \
		INNER JOIN account.userinfo ON todayEarnScore.userid = account.userinfo.userid ORDER BY score DESC LIMIT 50");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select today earn score rank fail . errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return;
	}

	std::vector<tagScoreRank> vecScoreRank;
	for (size_t i = 0; i < db->length(); i++)
	{
		tagScoreRank scoreRank;
		const acl::db_row* row = (*db)[i];
		scoreRank.cbRank = (CT_BYTE)(i + 1);
		scoreRank.dwUserID = atoi((*row)["userid"]);
		scoreRank.dScore = atoll((*row)["score"])*TO_DOUBLE;
		scoreRank.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
		scoreRank.cbGender = (CT_BYTE)atoi((*row)["gender"]);
		scoreRank.cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
		std::string strNickName = (*row)["nickname"];
		_snprintf_info(scoreRank.szNickName, sizeof(scoreRank.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());

		vecScoreRank.push_back(scoreRank);
	}
	SendTodayScoreRankToCenter(vecScoreRank);

	db->free_result();
}

CT_VOID DBThread::SendTodayScoreRankToCenter(std::vector<tagScoreRank>& vecScoreRank)
{
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		//发送群组成员信息
		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		CT_DWORD dwSendSize = 0;
		for (auto&it : vecScoreRank)
		{
			//发送数据
			if ((dwSendSize + sizeof(tagScoreRank)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_YESTERDAY_SCORE_RANK, szBuffer, dwSendSize);
				dwSendSize = 0;
			}

			memcpy(szBuffer + dwSendSize, &it, sizeof(tagScoreRank));
			dwSendSize += sizeof(tagScoreRank);

			//更新排行榜任务
			//UpdateUserRankTask(it.dwUserID);
		}
		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_YESTERDAY_SCORE_RANK, szBuffer, dwSendSize);
		}
		else
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_YESTERDAY_SCORE_RANK);
		}
	}
}

/*CT_VOID DBThread::LoadShareScoreRank()
{
	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}

	std::vector<tagShareRank> vecShareScoreRank;
	do 
	{
		std::string date = Utility::YesterdayString();
		acl::query query;
		query.create_sql("SELECT stat_user_revenue.userid, stat_user_revenue.usercount, stat_user_revenue.revenue, stat_user_revenue.flag, account.userinfo.headId, account.`user`.nickname from stat_user_revenue\
			INNER JOIN account.userinfo ON stat_user_revenue.userid = account.userinfo.userid\
			INNER JOIN account.`user` ON stat_user_revenue.userid = account.`user`.userid\
			WHERE stat_user_revenue.fixdate = :fixdate ORDER BY revenue DESC")
			.set_format("fixdate", "%s", date.c_str());

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select stat_user_revenue share rank fail . errno: " << db->get_errno() << ", err msg: " << db->get_error();
			return;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			tagShareRank shareRank;
			const acl::db_row* row = (*db)[i];
			shareRank.cbRank = (CT_BYTE)(i + 1);
			shareRank.dwUserID = atoi((*row)["userid"]);
			shareRank.dwShareCount = atoi((*row)["usercount"]);
			shareRank.dRewardScore = atoll((*row)["revenue"])*TO_DOUBLE;
			shareRank.cbSendReward = atoi((*row)["flag"]);
			shareRank.cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
			std::string strNickName = (*row)["nickname"];
			_snprintf_info(shareRank.szNickName, sizeof(shareRank.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());

			if (shareRank.dwUserID != 0)
			{
				vecShareScoreRank.push_back(shareRank);
			}
			
			//设置已经加载好新的奖励
			if (m_bNeedLoadNewShareRank)
			{
				m_bNeedLoadNewShareRank = false;
			}
		}

		//给每个玩家发送一封邮件
		tagUserMail mail;
		mail.cbState = 0;
		mail.cbMailType = 2;
		mail.dwSendTime = Utility::GetTime();
		mail.dwExpiresTime = mail.dwSendTime + 89600 * 365; //默认一年的有效期
		for (auto& it :vecShareScoreRank)
		{
			//检测是否已经发送奖励，如果已经发送，则不要再发送了
			if (it.cbSendReward)
				continue;
			
			mail.dwUserID = it.dwUserID;
			mail.dwScore = (CT_LONGLONG)Utility::round(it.dRewardScore*TO_LL, 2);
			_snprintf_info(mail.szTitle, sizeof(mail.szTitle), "分享奖励%.2f", it.dRewardScore);
			_snprintf_info(mail.szContent, sizeof(mail.szContent), "恭喜您昨天通过分享的用户，在游戏中获得%.2f金币的奖励", it.dRewardScore);
			SendMail(mail);
		}

		//将所有用户的奖励标识为已发送奖励
		if (!vecShareScoreRank.empty())
		{
			query.reset();
			query.create_sql("UPDATE stat_user_revenue SET flag = 1 WHERE fixdate = :fixdate")
				.set_format("fixdate", "%s", date.c_str());

			if (db->exec_update(query) == false)
			{
				LOG(WARNING) << "update stat_user_revenue state fail . errno: " << db->get_errno() << ", err msg: " << db->get_error();
				break;
			}
		}
	} while (0);

	pool->put(db);

	if (!vecShareScoreRank.empty())
	{
		SendShareScoreRank(vecShareScoreRank);
	}
}*/


/*CT_VOID DBThread::SendShareScoreRank(std::vector<tagShareRank>& vecShareRank)
{
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		//先通知中心服清空数据
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_CLEAR_SHARE_RANK);

		//发送群组成员信息
		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		CT_DWORD dwSendSize = 0;
		//CT_BYTE cbRank = 0;
		for (auto&it : vecShareRank)
		{
			//发送数据
			if ((dwSendSize + sizeof(tagShareRank)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SHARE_RANK, szBuffer, dwSendSize);
				dwSendSize = 0;
			}

			memcpy(szBuffer + dwSendSize, &it, sizeof(tagShareRank));
			dwSendSize += sizeof(tagShareRank);
			//++cbRank;
			//if (cbRank == 20) //只取前20个
			//{
			//	break;
			//}
		}
		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SHARE_RANK, szBuffer, dwSendSize);
		}
		else
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SHARE_RANK);
		}
	}
}*/

CT_VOID DBThread::LoadTodayOnlineRank()
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game score db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("SELECT\n"
						 "\tgame.today_online_time.userid ,\n"
						 "\tgame.today_online_time.`online` ,\n"
						 "\taccount.userdata.vip2 ,\n"
                         "\taccount.userinfo.gender ,\n"
						 "\taccount.userinfo.headId ,\n"
						 "\taccount.`user`.nickname\n"
						 "FROM\n"
						 "\tgame.today_online_time\n"
						 "INNER JOIN account.`user` ON game.today_online_time.userid = account.`user`.userid\n"
						 "INNER JOIN account.userinfo ON game.today_online_time.userid = account.userinfo.userid\n"
						 "INNER JOIN account.userdata ON game.today_online_time.userid = account.userdata.userid\n"
						 "ORDER BY\n"
						 "\tgame.today_online_time.`online` DESC\n"
						 "LIMIT 50");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select today online rank fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		std::vector<tagTodayOnlineRank> vecTodayOnlineRank;
		for (size_t i = 0; i < db->length(); i++)
		{
			tagTodayOnlineRank onlineRank;
			const acl::db_row* row = (*db)[i];
			onlineRank.cbRank = (CT_BYTE)(i + 1);
			onlineRank.dwUserID = atoi((*row)["userid"]);
			onlineRank.dwOnlineTime = atoi((*row)["online"]);
			onlineRank.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
            onlineRank.cbGender = (CT_BYTE)atoi((*row)["gender"]);
			onlineRank.cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
			std::string strNickName = (*row)["nickname"];
			_snprintf_info(onlineRank.szNickName, sizeof(onlineRank.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());

			vecTodayOnlineRank.push_back(onlineRank);
		}
		db->free_result();

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			//先清空任务今日在线榜
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_CLEAR_ONLINE_RANK);

			//发送群组成员信息
			CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
			CT_DWORD dwSendSize = 0;
			for (auto&it : vecTodayOnlineRank)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagTodayOnlineRank)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_TODAY_ONLINE_RANK, szBuffer, dwSendSize);
					dwSendSize = 0;
				}

				memcpy(szBuffer + dwSendSize, &it, sizeof(tagTodayOnlineRank));
				dwSendSize += sizeof(tagTodayOnlineRank);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_TODAY_ONLINE_RANK, szBuffer, dwSendSize);
			}
			else
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_TODAY_ONLINE_RANK);
			}
		}

	}while (0);

	//释放db
	pool->put(db);
}

CT_VOID DBThread::LoadTodayTaskFinishRank()
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game score db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("SELECT\n"
						 "\tgame.today_task_count.userid ,\n"
						 "\tgame.today_task_count.finishCount ,\n"
						 "\tgame.today_task_count.rewardScore ,\n"
						 "\taccount.userdata.vip2 ,\n"
						 "\taccount.userinfo.gender ,\n"
						 "\taccount.userinfo.headId ,\n"
						 "\taccount.`user`.nickname\n"
						 "FROM\n"
						 "\tgame.today_task_count\n"
						 "INNER JOIN account.`user` ON game.today_task_count.userid = account.`user`.userid\n"
						 "INNER JOIN account.userinfo ON game.today_task_count.userid = account.userinfo.userid\n"
						 "INNER JOIN account.userdata ON game.today_task_count.userid = account.userdata.userid\n"
						 "ORDER BY\n"
						 "\tgame.today_task_count.finishCount DESC\n"
						 "LIMIT 50");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select today task finish rank fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		std::vector<tagTodayTaskFinishRank> vecTaskFinishRank;
		for (size_t i = 0; i < db->length(); i++)
		{
			tagTodayTaskFinishRank TaskFinishRank;
			const acl::db_row* row = (*db)[i];
			TaskFinishRank.cbRank = (CT_BYTE)(i + 1);
			TaskFinishRank.dwUserID = atoi((*row)["userid"]);
			TaskFinishRank.wFinishCount = atoi((*row)["finishCount"]);
			TaskFinishRank.dRewardScore = atoi((*row)["rewardScore"]) * TO_DOUBLE;
			TaskFinishRank.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
			TaskFinishRank.cbGender = (CT_BYTE)atoi((*row)["gender"]);
			TaskFinishRank.cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
			std::string strNickName = (*row)["nickname"];
			_snprintf_info(TaskFinishRank.szNickName, sizeof(TaskFinishRank.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());

			vecTaskFinishRank.push_back(TaskFinishRank);
		}
		db->free_result();

		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			//先清空任务今日在线榜
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_CLEAR_TASK_FINISH_RANK);

			//发送群组成员信息
			CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
			CT_DWORD dwSendSize = 0;
			for (auto&it : vecTaskFinishRank)
			{
				//发送数据
				if ((dwSendSize + sizeof(tagTodayTaskFinishRank)) > sizeof(szBuffer))
				{
					CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_TASK_FINISH_RANK, szBuffer, dwSendSize);
					dwSendSize = 0;
				}

				memcpy(szBuffer + dwSendSize, &it, sizeof(tagTodayTaskFinishRank));
				dwSendSize += sizeof(tagTodayTaskFinishRank);
			}
			if (dwSendSize > 0)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_TASK_FINISH_RANK, szBuffer, dwSendSize);
			}
			else
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_TASK_FINISH_RANK);
			}
		}

	}while (0);

	//释放db
	pool->put(db);
}

CT_VOID DBThread::ResetTodayRank()
{
    acl::db_pool* pool = GetGamedbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get game score db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get game score db handle fail.";
        return;
    }

    do
    {
        acl::query query;
        query.create_sql("DELETE FROM game.today_online_time");

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "delete from today online rank fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            break;
        }

        query.reset();
        query.create_sql("DELETE FROM game.today_task_count");

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "delete from today task finish rank fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
            break;
        }
    }while (0);

    //释放db
    pool->put(db);


    //清空缓存
    acl::string pattern("account_*");
    std::vector<acl::string> vecAccountInfo;
    std::map<acl::string, acl::string> mapAttr;

    mapAttr["online"] = "0";
    mapAttr["taskfinish"] = "0";
    mapAttr["taskscore"] = "0";
    mapAttr["50101_c"] = "0";
    mapAttr["20104_c"] = "0";

    m_redis.clear();
    int keys = m_redis.keys_pattern(pattern.c_str(), &vecAccountInfo);
    if (keys > 0)
    {
        std::vector<acl::string> vecVal;
        auto keyCount = vecAccountInfo.size();
        LOG(WARNING) << "account count: " << keyCount;
        CT_DWORD  dwTime1 = Utility::GetTick();
        for (int i = 0; i != (int)keyCount; ++i)
        {
            //批量处理
            m_redis.clear();
            m_redis.hmset(vecAccountInfo[i].c_str(), mapAttr);
        }
        LOG(WARNING) << "vecAccountInfo use time: " << Utility::GetTick() - dwTime1;
    }
}

/*CT_VOID DBThread::LoadSignInRank()
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	std::vector<tagSignInRank> vecSignInRank;
	do 
	{
		acl::query query;
		query.create_sql("SELECT record_signin.userid, record_signin.seriesDate, account.`user`.nickname, account.userinfo.headimage,account.userdata.vip FROM record_signin \
			INNER JOIN account.`user` ON record_signin.userid = account.`user`.userid \
			INNER JOIN account.userinfo ON record_signin.userid = account.userinfo.userid \
			INNER JOIN account.userdata ON record_signin.userid = account.userdata.userid \
			WHERE TO_DAYS(NOW()) - TO_DAYS(lastDateTime) <= 1 ORDER BY seriesDate desc LIMIT 20");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select score rank fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		tagSignInRank signInRank;
		for (size_t i = 0; i < db->length(); i++)
		{
			memset(&signInRank, 0, sizeof(signInRank));
			const acl::db_row* row = (*db)[i];
			signInRank.cbRank = (CT_BYTE)(i + 1);
			signInRank.dwUserID = atoi((*row)["userid"]);
			signInRank.wSignInCount = atoi((*row)["seriesDate"]);
			signInRank.cbVipLevel = (CT_BYTE)atoi((*row)["vip"]);
			std::string strNickName = (*row)["nickname"];
			std::string strHeadUrl = (*row)["headimage"];
			_snprintf_info(signInRank.szNickName, sizeof(signInRank.szNickName), "%s", Utility::Ansi2Utf8(strNickName).c_str());
			_snprintf_info(signInRank.szHeadUrl, sizeof(signInRank.szHeadUrl), "%s", Utility::Ansi2Utf8(strHeadUrl).c_str());

			vecSignInRank.push_back(signInRank);
		}
		
		db->free_result();

	} while (0);

	pool->put(db);

	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		//发送群组成员信息
		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		CT_DWORD dwSendSize = 0;
		for (auto&it : vecSignInRank)
		{
			//发送数据
			if ((dwSendSize + sizeof(tagSignInRank)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SIGNIN_RANK, szBuffer, dwSendSize);
				dwSendSize = 0;
			}

			memcpy(szBuffer + dwSendSize, &it, sizeof(tagSignInRank));
			dwSendSize += sizeof(tagSignInRank);

			//更新排行榜任务
			UpdateUserRankTask(it.dwUserID);
		}
		if (dwSendSize > 0)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SIGNIN_RANK, szBuffer, dwSendSize);
		}
		else
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_SIGNIN_RANK);
		}
	}
}*/

CT_VOID DBThread::InsertTodayEarnScore(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}
	
	do
	{
		acl::query query;
		for (int i = 0; i < wDrawScoreCount; ++i)
		{
            if (pDrawScore->cbIsbot == 1)
            {
				pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
                continue;
            }

			if (pDrawScore->iScore > 0)
			{
				query.reset();
				query.create_sql("INSERT INTO todayEarnScore(userid, earnScore) VALUES (:userid, :addScore) ON DUPLICATE KEY UPDATE earnScore = earnScore + :addScore")
				.set_format("userid", "%u", pDrawScore->dwUserID)
				.set_format("addScore", "%d", pDrawScore->iScore);
				
				if (db->exec_update(query) == false)
				{
					LOG(WARNING) << "insert today earn score fail! errno: " << db->get_errno() << ", err msg: " << db->get_error();
					break;
				}
			}

			pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
		}
		
	} while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::UpdateUserRankTask(CT_DWORD dwUserID)
{
	//更新redis
	acl::string key;
	key.format("task_%u", dwUserID);

	acl::string name;
	name.format("%u", TASK_ID_IN_ANY_RANK);

	acl::string rankTask;
	m_redis.clear();
	if (m_redis.hget(key.c_str(), name, rankTask) == true)
	{
		if (!rankTask.empty())
		{
			std::vector<acl::string>& taskData = rankTask.split2(",");
			if (taskData.size() != 4)
			{
				LOG(WARNING) << "update rank task fail, task data is wrong!";
				return;
			}
			tagUserTask userTask;
			userTask.wTaskID = atoi(taskData[0].c_str());
			userTask.dwCurrProgress = atoi(taskData[1].c_str());
			userTask.dwReceiveTime = atoi(taskData[2].c_str());
			userTask.bFinish = atoi(taskData[3].c_str()) > 0 ? true : false;

			//如果没有完成任务, 则设置任务完成
			if (userTask.dwCurrProgress != 1)
			{
				acl::string value;
				value.format("%u,%u,%u,%d", userTask.wTaskID, 1, userTask.dwReceiveTime, userTask.bFinish);

				m_redis.clear();
				if (m_redis.hset(key.c_str(), name.c_str(), value.c_str()) == -1)
				{
					LOG(WARNING) << "set user task fail. task id:" << userTask.wTaskID;
				}
			}
		}
	}

	//更新db
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}
	do
	{
		acl::query query;
		query.create_sql("UPDATE userTask SET currProgress = 1 WHERE userid = :userid AND taskid = :taskid AND currProgress != 1 LIMIT 1")
			.set_format("userid", "%u", dwUserID)
			.set_format("taskid", "%u", TASK_ID_IN_ANY_RANK);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update rank task fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		db->free_result();
	} while (0);

	pool->put(db);
}

CT_VOID	DBThread::LoadPCUInfo()
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	std::vector<tagPcuInfo> vecPcuInfo;
	do 
	{
		acl::query query;
		query.create_sql("SELECT DISTINCT(platformId) FROM stat_pcu");
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << query.to_string().c_str();
			LOG(WARNING) << "select pcu platformId fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		std::vector<CT_BYTE> vecPlatformId;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			CT_BYTE cbPlatformId = atoi((*row)["platformId"]);
			vecPlatformId.push_back(cbPlatformId);
		}
		db->free_result();

		tagPcuInfo pcuInfo;
		for (auto& it : vecPlatformId)
		{
			memset(&pcuInfo, 0, sizeof(pcuInfo));
			pcuInfo.cbPlatformId = it;
			query.create_sql("SELECT statValue FROM stat_pcu WHERE platformId = :platformId AND statType = 0 AND FROM_UNIXTIME(fixdate) > :today ORDER BY statValue DESC LIMIT 1")
			    .set_format("platformId", "%d", it)
			    .set_format("today", "%s", Utility::GetTimeNowString("%Y-%m-%d").c_str());

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << query.to_string().c_str();
				LOG(WARNING) << "select pcu fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				break;
			}
			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				pcuInfo.dwPCU = atoi((*row)["statValue"]);
			}
			db->free_result();

			query.reset();
			query.create_sql("SELECT statValue FROM stat_pcu WHERE platformId = :platformId AND statType = :machineType AND FROM_UNIXTIME(fixdate) > :today ORDER BY statValue DESC LIMIT 1 ")
				.set_format("platformId", "%d", it)
			    .set_format("machineType", "%d", MACHINE_TYPE_IOS)
				.set_format("today", "%s", Utility::GetTimeNowString("%Y-%m-%d").c_str());

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << query.to_string().c_str();
				LOG(WARNING) << "select ios pcu fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				break;
			}
			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				pcuInfo.dwPCUiOS = atoi((*row)["statValue"]);
			}
			db->free_result();

			query.reset();
			query.create_sql("SELECT statValue FROM stat_pcu WHERE platformId = :platformId AND statType = :machineType AND FROM_UNIXTIME(fixdate) > :today ORDER BY statValue DESC LIMIT 1 ")
				.set_format("platformId", "%d", it)
			    .set_format("machineType", "%d", MACHINE_TYPE_ANDROID)
				.set_format("today", "%s", Utility::GetTimeNowString("%Y-%m-%d").c_str());
			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << query.to_string().c_str();
				LOG(WARNING) << "select android pcu fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				break;
			}

			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				pcuInfo.dwPCUAndroid = atoi((*row)["statValue"]);
			}
			db->free_result();

			vecPcuInfo.push_back(pcuInfo);
		}
	} while (0);

	pool->put(db);

	//发送给中心服
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		for (auto& it : vecPcuInfo)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_PCU_INFO, &it, sizeof(it));
		}
	}
}

CT_VOID DBThread::UpdatePCU(MSG_CS2DB_Update_PCU* pUpdatePCU)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	do 
	{
		acl::query query;
		query.create_sql("INSERT INTO stat_pcu(platformId, statType, statValue, fixdate) VALUES(:platformId, :type, :value, :fixdate)")
		    .set_format("platformId", "%d", (int)pUpdatePCU->cbPlatformId)
			.set_format("type", "%d", (int)pUpdatePCU->cbType)
			.set_format("value", "%u", pUpdatePCU->dwPCU)
			.set_format("fixdate", "%u", (CT_DWORD)time(NULL));
		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update pcu fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

	} while (0);

	pool->put(db);
}

CT_VOID DBThread::SaveRecordDrawInfo(acl::db_handle * db, MSG_G2DB_Record_Draw_Info * pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount, CT_DWORD& dwDrawID)
{
	/*if (db->begin_transaction() == false)
	{
		LOG(WARNING) << "begin transaction false: " << db->get_error();
		return;
	}*/
	if (wDrawScoreCount == 0)
	{
		return;
	}

	//插入record_score.record_draw_info
	std::string date(Utility::GetTimeNowString());
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string table("insert into record_draw_info_");
	table.append(year);
	table.append(month);

	acl::query query;
	query.create_sql(" (gameid, kindid, roomKindid, tableid, userCount, androidCount, bankUserid, stock, playTime, fixdate) values \
	(:gameid, :kindid, :roomKindid, :tableid, :userCount, :androidCount, :bankUserid, :stock, :playTime, :fixdate)")
		.set_format("gameid", "%d", pRecordDrawInfo->wGameID)
		.set_format("kindid", "%d", pRecordDrawInfo->wKindID)
		.set_format("roomKindid", "%d", pRecordDrawInfo->wRoomKindID)
		.set_format("tableid", "%d", pRecordDrawInfo->wTableID)
		.set_format("userCount", "%d", pRecordDrawInfo->wUserCount)
		.set_format("androidCount", "%d", pRecordDrawInfo->wAndroidCount)
		.set_format("bankUserid", "%u", pRecordDrawInfo->dwBankerUserID)
		.set_format("stock", "%lld", pRecordDrawInfo->llStock)
		.set_format("playTime", "%d", pRecordDrawInfo->dwPlayTime)
		.set_format("fixdate", "%u", pRecordDrawInfo->dwRecordTime);

	table.append(query.to_string());
	if (db->sql_update(table.c_str()) == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "insert record_score.record_draw_info fail, error = " << nError << ", error msg: " << db->get_error();
		//db->rollback();
		return;
	}

	db->free_result();
	if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "select record_score.record_draw_info id record fail, error = " << nError<< ", error msg: " << db->get_error();
		//db->rollback();
		return;
	}
	CT_DWORD drawid = 0;
	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];
		drawid = atoi((*row)["LAST_INSERT_ID()"]);
		dwDrawID = drawid;
		//LOG(INFO) << " record_score.record_draw_info drawid: " << drawid;
	}
	
	if (db->length() == 0)
	{
		LOG(ERROR) << "get record draw info id fail! ";
	}

	db->free_result();

	//插入record_score.record_draw_score
	CT_DWORD dwTime = pRecordDrawInfo->dwRecordTime; //(CT_DWORD)time(NULL);

	std::string drawScoreTable("insert into record_draw_score_");
	drawScoreTable.append(year);
	drawScoreTable.append(month);

	std::string drawScoreInfoSql(drawScoreTable);
	drawScoreInfoSql.append(" (drawid, userid, chairid, score, revenue, fixdate, isbot, location) values ");

	std::stringstream strSqlValue;
	for (int i = 0; i < wDrawScoreCount; ++i)
	{
		std::string strLocation(pDrawScore->szLocation);
		strLocation = strLocation.length() > 0 ? Utility::Utf82Ansi(strLocation).c_str() : "未知";

		strSqlValue << "(" << drawid << "," << pDrawScore->dwUserID << "," << pDrawScore->wChairID << "," << pDrawScore->iScore << "," << pDrawScore->dwRevenue << "," << dwTime \
			<< "," << (int)pDrawScore->cbIsbot << "," << "'" << strLocation.c_str() << "'" << "),";

		++pDrawScore; //= (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
	}

	drawScoreInfoSql.append(strSqlValue.str());
	auto pos = drawScoreInfoSql.find_last_of(",");
	if (pos != std::string::npos)
	{
		drawScoreInfoSql.erase(pos);
	}
	//std::cout << drawScoreInfoSql.c_str() << std::endl;

	if (db->sql_update(drawScoreInfoSql.c_str()) == false)
	{
		int nError = db->get_errno();
		LOG(WARNING) << "insert record_score.record_draw_score fail, error = " << nError << ", error msg: " << db->get_error();
		LOG(WARNING) << drawScoreInfoSql.c_str();
		return;
	}

	db->free_result();
}

CT_VOID DBThread::SaveBaiRenScoreInfo(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount, CT_DWORD dwDrawID)
{
	if (wDrawScoreCount == 0)
	{
		return;
	}

	CT_DWORD dwTime = pRecordDrawInfo->dwRecordTime;//(CT_DWORD)time(NULL);
	std::string date(Utility::GetTimeNowString());
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string drawScoreBRTable("insert into record_score_br_");
	drawScoreBRTable.append(year);
	drawScoreBRTable.append(month);

	std::string scoreInfoBRSql(drawScoreBRTable);
	scoreInfoBRSql.append(" (drawid, userid, area1Jetton, area2Jetton, area3Jetton, area4Jetton,\
		area0CardType, area1CardType, area2CardType, area3CardType, area4CardType, sourceScore, winScore, fixdate) values ");

	std::stringstream strSqlValue;
	int nWriteCount = 0;
	for (int i = 0; i < wDrawScoreCount; ++i)
	{
		//机器人不统计
		/*if (pDrawScore->cbIsbot == 1)
		{
		continue;
		}*/

		CT_INT32 iChangeScore = pDrawScore->iScore;
		if (0 != pDrawScore->dwRevenue)
		{
			iChangeScore -= pDrawScore->dwRevenue;
		}

		strSqlValue << "(" << dwDrawID << "," << pDrawScore->dwUserID << "," << pDrawScore->iAreaJetton[0] << "," << pDrawScore->iAreaJetton[1] << "," << pDrawScore->iAreaJetton[2] << "," << pDrawScore->iAreaJetton[3] \
			<< "," << (CT_WORD)pRecordDrawInfo->cbAreaCardType[0] << "," << (CT_WORD)pRecordDrawInfo->cbAreaCardType[1] << "," << (CT_WORD)pRecordDrawInfo->cbAreaCardType[2] << "," << (CT_WORD)pRecordDrawInfo->cbAreaCardType[3] << "," << (CT_WORD)pRecordDrawInfo->cbAreaCardType[4] \
			 <<"," <<  pDrawScore->llSourceScore << "," << iChangeScore <<  "," << dwTime << "),";

		++pDrawScore; // = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
		++nWriteCount;
	}

	if (nWriteCount == 0)
	{
		return;
	}

	scoreInfoBRSql.append(strSqlValue.str());
	//std::cout << scoreInfoBRSql << std::endl;
	auto pos = scoreInfoBRSql.find_last_of(",");
	if (pos != std::string::npos)
	{
		scoreInfoBRSql.erase(pos);
	}

	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}

	do
	{
		if (db->sql_update(scoreInfoBRSql.c_str()) == false)
		{
			LOG(WARNING) << "insert br game score detail info fail. errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID DBThread::UpdateWinLostCount(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount)
{
	if (wDrawScoreCount == 0)
	{
		return;
	}

	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	acl::query query;
	for (int i = 0; i < wDrawScoreCount; ++i)
	{
		//机器人不统计
		if (pDrawScore->cbIsbot == 1)
		{
			pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
			continue;
		}


		CT_INT32 iChangeScore = pDrawScore->iScore;

		query.reset();
		if (iChangeScore > 0)
		{
			query.create_sql("INSERT INTO gameScoreData(userid, revenue, score, winCount, playTime) \
				VALUES (:userid, :revenue, :score, 1, :playTime) \
				ON DUPLICATE KEY UPDATE revenue=revenue+:revenue, score=score+:score, winCount=winCount+1,playTime=playTime+:playTime")
				.set_format("revenue", "%d", pDrawScore->dwRevenue)
				.set_format("score", "%d", pDrawScore->iScore)
				.set_format("playTime", "%d", pRecordDrawInfo->dwPlayTime)
				.set_format("userid", "%u", pDrawScore->dwUserID);
		}
		else if (iChangeScore < 0)
		{
			query.create_sql("INSERT INTO gameScoreData(userid, revenue, score, lostCount, playTime) \
				VALUES (:userid, :revenue, :score, 1, :playTime) \
				ON DUPLICATE KEY UPDATE revenue=revenue+:revenue, score=score+:score, lostCount=lostCount+1,playTime=playTime+:playTime")
				.set_format("revenue", "%d", pDrawScore->dwRevenue)
				.set_format("score", "%d", pDrawScore->iScore)
				.set_format("playTime", "%d", pRecordDrawInfo->dwPlayTime)
				.set_format("userid", "%u", pDrawScore->dwUserID);
		}
		else
		{
			query.create_sql("INSERT INTO gameScoreData(userid, revenue, score, drawCount, playTime) \
				VALUES (:userid, :revenue, :score, 1, :playTime) \
				ON DUPLICATE KEY UPDATE revenue=revenue+:revenue,score=score+:score, drawCount=drawCount+1,playTime=playTime+:playTime")
				.set_format("revenue", "%d", pDrawScore->dwRevenue)
				.set_format("score", "%d", pDrawScore->iScore)
				.set_format("playTime", "%d", pRecordDrawInfo->dwPlayTime)
				.set_format("userid", "%u", pDrawScore->dwUserID);
		}
		
		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update game score win or lost info fail, user id: " << pDrawScore->dwUserID;
			return;
		}

		pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::UpddateWinLostCountEx(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount)
{
	if (wDrawScoreCount == 0)
	{
		return;
	}

	CT_DWORD dwGameIndex = pRecordDrawInfo->wGameID*10000+pRecordDrawInfo->wKindID*100+pRecordDrawInfo->wRoomKindID;
	std::string strRecordTime = Utility::ChangeTimeToString(pRecordDrawInfo->dwRecordTime, "%Y-%m-%d");

	//记录汇总信息
	std::stringstream streamInsertValue;
	std::stringstream streamTodayInsertValue;
	std::stringstream streamUpdateValue;
	streamUpdateValue << "lastPlayTime = '" << strRecordTime << "', winCount=VALUES(winCount) + winCount " << ", lostCount=VALUES(lostCount) + lostCount "
				   << ", winScore=VALUES(winScore) + winScore" <<  ", lostScore=VALUES(lostScore) + lostScore "  << ", revenue=VALUES(revenue) + revenue "
				   << ", totalPlayTime=VALUES(totalPlayTime)+totalPlayTime ";

	std::stringstream strSql;
	strSql << "INSERT INTO src_user_stat (gameIndex, userid, firstPlayTime, lastPlayTime, winCount, lostCount, winScore, lostScore, revenue, totalPlayTime) "
		   "VALUES ";

	int nRealUserCount = 0;
	for (int i = 0; i < wDrawScoreCount; ++i)
	{
		//机器人不统计
		if (pDrawScore->cbIsbot == 1)
		{
			pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
			continue;
		}


		CT_INT32 iChangeScore = pDrawScore->iScore;
		if (iChangeScore >= 0)
		{
			streamInsertValue << "(" << dwGameIndex << ", " << pDrawScore->dwUserID << ", '" <<  strRecordTime << "', '" << strRecordTime << "',"
			<< 1 << ", " << 0 << ", " << iChangeScore << ", " << 0 << ", " << pDrawScore->dwRevenue << ", " << pRecordDrawInfo->dwPlayTime << "), ";

			streamTodayInsertValue << "('" <<  strRecordTime << "', " << dwGameIndex << ", " << pDrawScore->dwUserID << ","
			<< 1 << ", " << 0 << ", " << iChangeScore << ", " << 0 << ", " << pDrawScore->dwRevenue << ", " << pRecordDrawInfo->dwPlayTime << "), ";

		}
		else
		{
			streamInsertValue << "(" << dwGameIndex << ", " << pDrawScore->dwUserID << ", '" <<  strRecordTime << "', '" << strRecordTime << "',"
			<< 0 << ", " << 1 << ", " << 0 << ", " << iChangeScore << ", " << pDrawScore->dwRevenue << ", " << pRecordDrawInfo->dwPlayTime << "), ";

			streamTodayInsertValue << "('" <<  strRecordTime << "', " << dwGameIndex << ", " << pDrawScore->dwUserID << ","
			<< 0 << ", " << 1 << ", " << 0 << ", " << iChangeScore << ", " << pDrawScore->dwRevenue << ", " << pRecordDrawInfo->dwPlayTime << "), ";
		}

		pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
		++nRealUserCount;
	}

	if (nRealUserCount == 0)
	{
		return;
	}

	std::string strInsertValue = streamInsertValue.str();
	if (strInsertValue.empty())
	{
		return;
	}

	auto itPos = strInsertValue.find_last_of(",");
	if (itPos != std::string::npos)
	{
		strInsertValue.erase(itPos);
	}

	strSql << strInsertValue << " ON DUPLICATE KEY UPDATE " << streamUpdateValue.str();


	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	do
	{
		if (db->sql_update(strSql.str().c_str()) == false)
		{
			LOG(WARNING) << "update user win lost count fail" << ", errno: " << db->get_errno() <<  ", error: " << db->get_error();
			break;
		}
	}while (0);

	//记录今天的信息
	std::stringstream streamTodayUpdateValue;
	streamTodayUpdateValue << " winCount=VALUES(winCount) + winCount " << ", lostCount=VALUES(lostCount) + lostCount "
					  <<  ", winScore=VALUES(winScore) + winScore" <<  ", lostScore=VALUES(lostScore) + lostScore "  << ", revenue=VALUES(revenue) + revenue "
					  << ", playTime=VALUES(playTime) + playTime ";

	std::stringstream strTodaySql;
	strTodaySql << "INSERT INTO src_user_today (writeDate, gameIndex, userid, winCount, lostCount, winScore, lostScore, revenue, playTime) "
			  "VALUES ";

	std::string strTodayInsertValue = streamTodayInsertValue.str();
	auto itPosToday = strTodayInsertValue.find_last_of(",");
	if (itPosToday != std::string::npos)
	{
		strTodayInsertValue.erase(itPosToday);
	}

	strTodaySql << strTodayInsertValue << " ON DUPLICATE KEY UPDATE " << streamTodayUpdateValue.str();

	do
	{
		if (db->sql_update(strTodaySql.str().c_str()) == false)
		{
			LOG(WARNING) << "update today user win lost count fail" << ", errno: " << db->get_errno() << "error: " << db->get_error();
			break;
		}
	}while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SaveRecordHBSLInfo(MSG_G2DB_Record_HBSL_Info* pRecordHBSLInfo, MSG_G2DB_Record_HBSL_GrabInfo* pGrabInfo, CT_WORD wUserCount)
{
	if (wUserCount == 0)
		return;

	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("INSERT INTO record_hbsl_info(roomKindid, hbid, sendUserid, amount, allotCount, multiple, thunderNO, sendTime, endTime, winColorPrize) VALUES "
				   "(:roomKindid, :hbid, :sendUserid, :amount, :allotCount, :multiple, :thunderNO, :sendTime, :endTime, :winColorPrize)")
			.set_format("roomKindid", "%d", pRecordHBSLInfo->wRoomKindID)
			.set_format("hbid", "%d", pRecordHBSLInfo->dwHongbaoID)
			.set_format("sendUserid", "%u", pRecordHBSLInfo->dwSendUserID)
			.set_format("amount", "%d", pRecordHBSLInfo->dwAmount)
			.set_format("allotCount", "%d", pRecordHBSLInfo->wAllotCount)
			.set_format("multiple", "%0.1f", pRecordHBSLInfo->dMultiple)
			.set_format("thunderNO", "%d", pRecordHBSLInfo->cbThunderNO)
			.set_format("sendTime", "%u", pRecordHBSLInfo->dwSendTime)
			.set_format("endTime", "%u", (CT_DWORD)time(NULL))
			.set_format("winColorPrize", "%u", pRecordHBSLInfo->dwWinColorPrize);

		if (db->exec_update(query) == false)
		{
			int nError = db->get_errno();
			LOG(WARNING) << "insert record_score.record_hbsl_info fail, error = " << nError << ", error msg: " << db->get_error();
            break;
		}

		if (db->sql_select("SELECT LAST_INSERT_ID()") == false)
		{
			int nError = db->get_errno();
			LOG(WARNING) << "select record_score.record_hbsl_info id record fail, error = " << nError << ", error msg: " << db->get_error();
            break;
		}

		CT_DWORD dwID = 0;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
            dwID = atoi((*row)["LAST_INSERT_ID()"]);
		}

		if (db->length() == 0)
		{
			LOG(ERROR) << "get record draw info id fail! ";
            break;
		}
		db->free_result();

		std::string hbslscoreInfoSql("insert into record_hbsl_score");
        hbslscoreInfoSql.append(" (id, userid, score, isthunder, winColorPrize) values ");

		std::stringstream strSqlValue;
		for (int i = 0; i < wUserCount; ++i)
		{
			strSqlValue << "(" << dwID << "," << pGrabInfo->dwUserID << "," << pGrabInfo->iScore << "," << (int)pGrabInfo->cbIsThunder << "," << pGrabInfo->dwWinColorPrize << "),";

			++pGrabInfo; //= (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
		}

        hbslscoreInfoSql.append(strSqlValue.str());
		auto pos = hbslscoreInfoSql.find_last_of(",");
		if (pos != std::string::npos)
		{
            hbslscoreInfoSql.erase(pos);
		}
		//std::cout << hbslscoreInfoSql.c_str() << std::endl;

		if (db->sql_update(hbslscoreInfoSql.c_str()) == false)
		{
			int nError = db->get_errno();
			LOG(WARNING) << "insert record_score.record_hbsl_score fail, error = " << nError << ", error msg: " << db->get_error();
            break;
		}
	}while (0);

    db->free_result();
    pool->put(db);
}

CT_VOID DBThread::UpdateBlackListUser(MSG_GS2DB_BlackList_User* pBlackList)
{
    acl::db_pool* pool = GetPlatformdbPool();
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

    do
    {
        acl::query query;
        if (pBlackList->wUserLostRatio != 0)
        {
            query.create_sql("INSERT INTO roomserver_blacklist (serverid, userid, lostRatio, controlScore) VALUES (:serverid, :userid, :lostRatio, :controlScore)")
                .set_format("serverid", "%u", pBlackList->dwServerID)
                .set_format("userid", "%u", pBlackList->dwUserID)
                .set_format("lostRatio", "%u", pBlackList->wUserLostRatio)
                .set_format("controlScore", "%u", pBlackList->dwControlScore);
        }
        else
        {
            query.create_sql("DELETE FROM roomserver_blacklist WHERE serverid = :serverid AND userid = :userid")
                .set_format("serverid", "%u", pBlackList->dwServerID)
                .set_format("userid", "%u", pBlackList->dwUserID);
        }

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "UpdateBlackListUser fail, errno = " << db->get_errno() << ", error msg: " << db->get_error();
            break;
        }
    }while (0);

    db->free_result();
    pool->put(db);
}

CT_VOID DBThread::UpdateUserTodayOnlineTime(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount)
{
	if (wDrawScoreCount == 0)
	{
		return;
	}

	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	acl::query query;
	for (int i = 0; i < wDrawScoreCount; ++i)
	{
	    if (pDrawScore->cbIsbot == 1)
        {
			pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
            continue;
        }

        //更新真人玩家redis的今日在线时长
        query.reset();
        query.create_sql("INSERT INTO today_online_time(userid, online) VALUES (:userid, :playTime) ON DUPLICATE KEY UPDATE online=online+:playTime")
            .set_format("playTime", "%d", pRecordDrawInfo->dwPlayTime)
            .set_format("userid", "%u", pDrawScore->dwUserID);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "update user today online time fail, user id: " << pDrawScore->dwUserID;
            continue;
        }


        m_redis.clear();
        acl::string key;
        key.format("account_%u", pDrawScore->dwUserID);
        m_redis.hincrby(key.c_str(), "online", pRecordDrawInfo->dwPlayTime);

		pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SendMail(tagUserMail& mail)
{
	//更新db
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	do 
	{
		acl::query query;
		query.create_sql("INSERT INTO mail(userid, title, content, gem, score, scoreType, mailType, adminid, state, sendTime, expireTime) VALUES\
						(:userid, :title, :content, 0, :score, :scoreType, :mailType, 0, :state, :sendTime, :expireTime)")
			.set_format("userid", "%u", mail.dwUserID)
			.set_format("title", "%s", mail.szTitle)
			.set_format("content", "%s", mail.szContent)
			.set_format("score", "%lld", mail.llScore)
			.set_format("scoreType", "%d", mail.cbScoreChangeType)
			.set_format("mailType", "%d", mail.cbMailType)
			.set_format("state", "%d", mail.cbState)
			.set_date("sendTime", mail.dwSendTime)
			.set_date("expireTime", mail.dwExpiresTime);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert mail fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
			//LOG(WARNING) << mail.szContent;
			break;
		}
	} while (0);

	pool->put(db);
}

CT_VOID DBThread::UpdateAndroidStatus(MSG_G2DB_Android_Status* pStatus)
{
	//更新db
	acl::db_pool* pool = GetAccountdbPool();
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

	do
	{
		acl::query query;
		query.create_sql("UPDATE androidCfg SET activate = :activate WHERE userID = :userid")
			.set_format("userid", "%u", pStatus->dwAndroidUserID)
			.set_format("activate", "%d", pStatus->cbStatus);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update android status fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
			break;
		}
	} while (0);

	pool->put(db);
}

CT_VOID DBThread::UpdateServerStockInfo(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo)
{
	//更新db
	acl::db_pool* pool = GetPlatformdbPool();
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

	do
	{
		acl::query query;
		
		if (pRecordDrawInfo->wGameID == GAME_BR && pRecordDrawInfo->wKindID == GAME_BR_NN)
		{
			query.create_sql("UPDATE roomserver SET totalStock = :stock, androidStock = :androidStock, todayStock = :todayStock, systemAllKillRatio = :allKillRatio, changeCardRatio = :changeCardRatio WHERE serverid = :serverid")
			.set_format("stock", "%lld", pRecordDrawInfo->llStock)
			.set_format("androidStock", "%lld", pRecordDrawInfo->llAndroidStock)
			.set_format("todayStock", "%lld", pRecordDrawInfo->llTodayStock)
			.set_format("allKillRatio", "%u", pRecordDrawInfo->wSystemAllKillRatio)
			.set_format("changeCardRatio", "%u", pRecordDrawInfo->wChangeCardRatio)
			.set_format("serverid", "%d", pRecordDrawInfo->dwServerID);
		}
		else
		{
			query.create_sql("UPDATE roomserver SET totalStock = :stock, androidStock = :androidStock, todayStock = :todayStock WHERE serverid = :serverid")
			.set_format("stock", "%lld", pRecordDrawInfo->llStock)
			.set_format("androidStock", "%lld", pRecordDrawInfo->llAndroidStock)
			.set_format("todayStock", "%lld", pRecordDrawInfo->llTodayStock)
			.set_format("serverid", "%d", pRecordDrawInfo->dwServerID);
		}

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "update room server stock fail. errno: " << db->get_errno() << ", error msg: " << db->get_error();
			break;
		}
	} while (0);

	pool->put(db);
}

CT_VOID DBThread::InsertScoreChangeRecordForPlayGame(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount, CT_DWORD dwDrawID)
{
	if (wDrawScoreCount == 0)
	{
		return;
	}

	CT_DWORD dwTime = pRecordDrawInfo->dwRecordTime; //(CT_DWORD)time(NULL);
	std::string date(Utility::GetTimeNowString());
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string drawScoreTable("insert into record_score_change_");
	drawScoreTable.append(year);
	drawScoreTable.append(month);

	std::string scoreChangeSql(drawScoreTable);
	scoreChangeSql.append(" (userid, sourceBank, sourceScore, changeBank, changeScore, drawid, type, fixdate) values ");
	std::stringstream strSqlValue;
	int nWriteCount = 0;
	for (int i = 0; i < wDrawScoreCount; ++i)
	{
		//机器人不统计
		if (pDrawScore->cbIsbot == 1)
		{
			pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
			continue;
		}

		CT_INT32 iChangeScore = pDrawScore->iScore;
		if (pRecordDrawInfo->wGameID != GAME_FISH && pRecordDrawInfo->wGameID != GAME_FXGZ && 0 != pDrawScore->dwRevenue)
		{
			iChangeScore -= pDrawScore->dwRevenue;
		}

		/*if (iChangeScore == 0)
		{
			continue;
		}*/

		strSqlValue << "(" << pDrawScore->dwUserID << "," << 0 << "," << pDrawScore->llSourceScore << "," << 0  << "," << iChangeScore \
			<< "," << dwDrawID << ","<< PLAY_GAME << "," << dwTime <<"),";

		pDrawScore = (MSG_G2DB_Record_Draw_Score*)((CT_BYTE*)pDrawScore + sizeof(MSG_G2DB_Record_Draw_Score));
		++nWriteCount;
	}

	if (nWriteCount == 0)
	{
		return;
	}

	scoreChangeSql.append(strSqlValue.str());
	//std::cout << scoreChangeSql << std::endl;
	auto pos = scoreChangeSql.find_last_of(",");
	if (pos != std::string::npos)
	{
		scoreChangeSql.erase(pos);
	}

	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	do
	{
		if (db->sql_update(scoreChangeSql.c_str()) == false)
		{
			LOG(WARNING) << "update score change info fail. errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID DBThread::InsertScoreChangeRecord(CT_DWORD dwUserID, CT_LONGLONG llSourceBank, \
	CT_LONGLONG llSourceScore, CT_LONGLONG llAddBank, CT_LONGLONG llAddScore, CT_BYTE cbType)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	//插入record_score.record_draw_info
	std::string date(Utility::GetTimeNowString());
	std::string year = date.substr(0, 4);
	std::string month = date.substr(5, 2);
	std::string scoreChangeTable("insert into record_score_change_");
	scoreChangeTable.append(year);
	scoreChangeTable.append(month);

	do
	{
		std::string scoreChangeSql(scoreChangeTable);
		acl::query query;
		query.create_sql(" (userid, sourceBank, sourceScore, changeBank, changeScore, type, fixdate) \
				 values(:userid, :sourceBank, :sourceScore, :changeBank, :changeScore, :type, UNIX_TIMESTAMP(NOW()))")
			.set_format("userid", "%u", dwUserID)
			.set_format("sourceBank", "%lld", llSourceBank)
			.set_format("sourceScore", "%lld", llSourceScore)
			.set_format("changeBank", "%lld", llAddBank)
			.set_format("changeScore", "%lld", llAddScore)
			.set_format("type", "%d", cbType);

		scoreChangeSql.append(query.to_string());
		if (db->sql_update(scoreChangeSql.c_str()) == false)
		{
			LOG(WARNING) << "update score change info fail, user id: " << dwUserID << ", type: " << (int)cbType << ", change score: " << llAddScore;
			break;
		}

		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID DBThread::InsertTaskRewardRecord(CT_DWORD dwUserID, CT_WORD wTaskID, CT_LONGLONG llReward)
{
	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("insert into record_task_reward(userid, taskid, rewardscore, fixdate) \
				 values(:userid, :taskid, :rewardscore, UNIX_TIMESTAMP(NOW()))")
			.set_format("userid", "%u", dwUserID)
			.set_format("taskid", "%d", wTaskID)
			.set_format("rewardscore", "%lld", llReward);

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert task reward info fail, user id: " << dwUserID << ", errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		db->free_result();

	} while (0);

	pool->put(db);
}

CT_VOID DBThread::UpdateUserTodayTaskRewardCount(MSG_CS2DB_Insert_TaskReward* pTaskReward)
{
    acl::db_pool* pool = GetGamedbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get game score db pool fail.";
        return;
    }

    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get game score db handle fail.";
        return;
    }

    do
    {
        acl::query query;
        query.create_sql("INSERT INTO today_task_count(userid,finishCount, rewardScore) VALUES (:userid , 1 , :reward) ON DUPLICATE KEY UPDATE finishCount = finishCount + 1,rewardScore = rewardScore + :reward")
          .set_format("userid", "%u", pTaskReward->dwUserID)
		  .set_format("reward", "%lld", pTaskReward->llReward);

        if (db->exec_update(query) == false)
        {
            LOG(WARNING) << "update user today task finish count fail, user id: " << pTaskReward->dwUserID;
            break;
        }

        m_redis.clear();
        acl::string key;
        key.format("account_%u", pTaskReward->dwUserID);
        m_redis.hincrby(key.c_str(), "taskfinish", 1);
        m_redis.hincrby(key.c_str(), "taskscore", pTaskReward->llReward);
    } while (0);

    pool->put(db);
}

CT_VOID DBThread::InsertReportInfo(MSG_G2DB_ReportInfo* pReport, CT_WORD wReportCount)
{
	acl::db_pool* pool = GetRecorddbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record db handle fail.";
		return;
	}

	do
	{
		//举报的数据是否需要太过精确
		CT_DWORD dwDrawID = 0;
		std::string date(Utility::GetTimeNowString());
		std::string year = date.substr(0, 4);
		std::string month = date.substr(5, 2);
		std::stringstream drawScoreTable;
		drawScoreTable << "SELECT drawid from record_score.record_draw_score_" << year << month << " where userid = " << pReport->dwUserID << " ORDER BY drawid desc LIMIT 1";
		
		if (db->sql_select(drawScoreTable.str().c_str()) ==false)
		{
			LOG(WARNING) << "select user last drawid fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			dwDrawID = atoi((*row)["drawid"]);
		}
		db->free_result();
		
		if (dwDrawID == 0)
		{
			break;
		}
		
		/*acl::query query;
		query.create_sql("SELECT id FROM treasure.illegal_player WHERE drawid = :drawid AND userid = :userid AND player = :player")
		.set_format("drawid", "%d", dwDrawID)
		.set_format("userid", "%u", pReportUser->dwUserID)
		.set_format("beReportUser", "%u", pReportUser->dwBeReportUserID);
		
		if (db->length() != 0)
		{
			break;
		}*/
		std::string reportSql("INSERT INTO treasure.illegal_player (drawid, userid, player, fixdate) VALUES ");
		std::stringstream strSqlValue;
		CT_DWORD dwNow =  (CT_DWORD)time(NULL);
		for (int i = 0; i < wReportCount; ++i)
		{
			strSqlValue << "(" << dwDrawID << ", " << pReport->dwUserID << "," << pReport->dwBeReportUserID << ", " << dwNow << "), ";
			pReport++;
		
		}
		reportSql.append(strSqlValue.str());
		auto pos = reportSql.find_last_of(",");
		if (pos != std::string::npos)
		{
			reportSql.erase(pos);
		}

		if (db->sql_update(reportSql.c_str()) == false)
		{
			LOG(WARNING) << "insert report info fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		
	} while (0);
	
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::QueryGameRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_GameRecord* pQueryRecord)
{
	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}
	
	std::vector<MSG_SC_Query_GameRecord> vecGameRecord;
	do
	{
		std::string date = Utility::GetTimeNowString("%Y%m%d");
		std::string thisMonth = date.substr(0, 6);
		if (GetGameRecord(db, pQueryRecord, vecGameRecord, thisMonth, 10) == false)
		{
			break;
		}

		auto recordSize = vecGameRecord.size();
		if (recordSize < 10)
		{
			CT_WORD wNeedCount = 10 - recordSize;
			std::string lastMonth = Utility::GetLastYearMonth(Utility::GetTimeNowString());
			std::string thisMonth = date.substr(0, 6);
			if (GetGameRecord(db, pQueryRecord, vecGameRecord, lastMonth, wNeedCount) == false)
			{
				break;
			}
		}
	}while (0);
	
	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;
	
	MSG_GameMsgDownHead headMsg;
	headMsg.dwMainID = MSG_QUERY_MAIN;
	headMsg.dwSubID = SUB_SC_QUERY_GAME_RECORD;
	headMsg.dwValue2 = pQueryRecord->dwUserID;
	memcpy(szBuffer + dwSendSize, &headMsg, sizeof(headMsg));
	dwSendSize += sizeof(headMsg);
	
	MSG_SC_Query_GameRecord_Head gameRecordHead;
	gameRecordHead.wGameID = pQueryRecord->wGameID;
	gameRecordHead.wKindID = pQueryRecord->wKindID;
	memcpy(szBuffer + dwSendSize, &gameRecordHead, sizeof(gameRecordHead));
	dwSendSize += sizeof(gameRecordHead);
	
	for (auto& it : vecGameRecord)
	{
		memcpy(szBuffer + dwSendSize, &it, sizeof(MSG_SC_Query_GameRecord));
		dwSendSize += sizeof(MSG_SC_Query_GameRecord);
	}
	
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::QueryReportRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Report_GameRecord* pQueryRecord)
{
	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}
	
	std::vector<MSG_SC_Query_GameRecord> vecGameRecord;
	CT_DWORD dwDrawID = 0;
	CT_WORD wGameID = 0;
	CT_WORD wKindID = 0;
	CT_DWORD dwReportUserID = 0;
	std::string drawData;
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM treasure.illegal_player WHERE id = :id")
		.set_format("id", "%d", pQueryRecord->dwReportID);
		
		if (db->exec_select(query) ==false)
		{
			LOG(WARNING) << "QueryGameRecord fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		
		//没有找到举报数据，立刻返回
		if (db->length() == 0)
		{
			db->free_result();
			break;
		}
	
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row *row = (*db)[i];
			dwDrawID = atoi((*row)["drawid"]);
			drawData = (*row)["drawdate"];
			wGameID = (CT_WORD)atoi((*row)["gameid"]);
			wKindID = (CT_WORD)atoi((*row)["kindid"]);
			dwReportUserID = (CT_DWORD)atoi((*row)["userid"]);
		}
		db->free_result();
		
		std::string year = drawData.substr(0, 4);
		std::string month = drawData.substr(5, 2);
		std::string date = year + month;
		
        if (GetGameRecord2(db, dwDrawID, dwReportUserID, vecGameRecord, date, 1) == false)
        {
            break;
        }
		
	}while (0);
	
	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;
	
	MSG_GameMsgDownHead headMsg;
	headMsg.dwMainID = MSG_QUERY_MAIN;
	headMsg.dwSubID = SUB_SC_QUERY_GAME_RECORD;
	headMsg.dwValue2 = pQueryRecord->dwUserID;
	memcpy(szBuffer + dwSendSize, &headMsg, sizeof(headMsg));
	dwSendSize += sizeof(headMsg);
	
	MSG_SC_Query_GameRecord_Head gameRecordHead;
	gameRecordHead.wGameID = wGameID;
	gameRecordHead.wKindID = wKindID;
	memcpy(szBuffer + dwSendSize, &gameRecordHead, sizeof(gameRecordHead));
	dwSendSize += sizeof(gameRecordHead);
	
	for (auto& it : vecGameRecord)
	{
		memcpy(szBuffer + dwSendSize, &it, sizeof(MSG_SC_Query_GameRecord));
		dwSendSize += sizeof(MSG_SC_Query_GameRecord);
	}
	
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	
	pool->put(db);
}
CT_VOID DBThread::QueryNormalRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Normal_GameRecord* pQueryRecord)
{
	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}
	
	std::vector<MSG_SC_Query_GameRecord> vecGameRecord;
	CT_WORD wGameID = 0;
	CT_WORD wKindID = 0;
	do
	{
		std::string date(pQueryRecord->szDate);
		std::stringstream sql;
		sql << "SELECT gameid, kindid from record_score.record_draw_info_" << date << " WHERE drawid = " << pQueryRecord->dwDrawID;
		
		if (db->sql_select(sql.str().c_str()) ==false)
		{
			LOG(WARNING) << "QueryNormalRecord select fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}
		
		if (db->length() == 0)
		{
			break;
		}
		
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row *row = (*db)[i];
			wGameID = (CT_WORD)atoi((*row)["gameid"]);
			wKindID = (CT_WORD)atoi((*row)["kindid"]);
		}
		db->free_result();
		
		if (GetGameRecord2(db,pQueryRecord->dwDrawID, 0, vecGameRecord, date, 1) == false)
		{
			break;
		}
		
	}while (0);
	
	CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;
	
	MSG_GameMsgDownHead headMsg;
	headMsg.dwMainID = MSG_QUERY_MAIN;
	headMsg.dwSubID = SUB_SC_QUERY_GAME_RECORD;
	headMsg.dwValue2 = pQueryRecord->dwUserID;
	memcpy(szBuffer + dwSendSize, &headMsg, sizeof(headMsg));
	dwSendSize += sizeof(headMsg);
	
	MSG_SC_Query_GameRecord_Head gameRecordHead;
	gameRecordHead.wGameID = wGameID;
	gameRecordHead.wKindID = wKindID;
	memcpy(szBuffer + dwSendSize, &gameRecordHead, sizeof(gameRecordHead));
	dwSendSize += sizeof(gameRecordHead);
	
	for (auto& it : vecGameRecord)
	{
		memcpy(szBuffer + dwSendSize, &it, sizeof(MSG_SC_Query_GameRecord));
		dwSendSize += sizeof(MSG_SC_Query_GameRecord);
	}
	
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	
	pool->put(db);
}

CT_BOOL DBThread::GetGameRecord(acl::db_handle* db, MSG_CS_Query_GameRecord* pQueryRecord, std::vector<MSG_SC_Query_GameRecord>& vecGameRecord, std::string& date, CT_WORD wMaxCount)
{
	std::stringstream sql;
	
	if (pQueryRecord->wGameID == GAME_DDZ)
    {
        sql << "select b.drawid, group_concat(b.userid, ',', b.chairid, ',', b.report, ',', b.score SEPARATOR ';') as data, a.roomKindid, a.bankUserid, FROM_UNIXTIME(a.fixdate) as fixdate from \
		(select distinct drawid from record_draw_score_" << date <<  " where userid = " << pQueryRecord->dwUserID << ") c \
		inner join record_draw_info_" << date << " a on c.drawid = a.drawid \
		inner join record_draw_score_" << date << " b on a.drawid = b.drawid \
		inner join playback.playbackdata_" << date << " d on a.drawid = d.drawid \
		where a.gameid = " << pQueryRecord->wGameID << " and a.kindid = " << pQueryRecord->wKindID << " and a.roomKindid != " << PRIMARY_ROOM << " group by b.drawid ORDER BY drawid desc LIMIT " << wMaxCount;
    }
    else
    {
        sql << "select b.drawid, group_concat(b.userid, ',', b.chairid, ',', b.report, ',', b.score SEPARATOR ';') as data, a.roomKindid, a.bankUserid, FROM_UNIXTIME(a.fixdate) as fixdate from \
		(select distinct drawid from record_draw_score_" << date <<  " where userid = " << pQueryRecord->dwUserID << ") c \
		inner join record_draw_info_" << date << " a on c.drawid = a.drawid \
		inner join record_draw_score_" << date << " b on a.drawid = b.drawid \
		inner join playback.playbackdata_" << date << " d on a.drawid = d.drawid \
		where a.gameid = " << pQueryRecord->wGameID << " and a.kindid = " << pQueryRecord->wKindID << " group by b.drawid ORDER BY drawid desc LIMIT " << wMaxCount;
    }
	
	if (db->sql_select(sql.str().c_str()) ==false)
	{
		LOG(WARNING) << "QueryGameRecord fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return false;
	}
	
	MSG_SC_Query_GameRecord gameRecord;
	for (size_t i = 0; i < db->length(); i++)
	{
		memset(&gameRecord, 0, sizeof(gameRecord));
		const acl::db_row* row = (*db)[i];
		gameRecord.dwDrawID = atoi((*row)["drawid"]);
		gameRecord.wRoomKindID = (CT_WORD)atoi((*row)["roomKindid"]);
		gameRecord.dwBankerUserID = (CT_DWORD)atoi((*row)["bankUserid"]);
		std::string strFixdate = (*row)["fixdate"];
		_snprintf_info(gameRecord.szDate, sizeof(gameRecord.szDate), "%s", strFixdate.substr(0, 10).c_str());
		_snprintf_info(gameRecord.szTime, sizeof(gameRecord.szTime), "%s", strFixdate.substr(11).c_str());
		
		std::string data = (*row)["data"];
		std::vector<std::string> vecData;
		Utility::stringSplit(data, ";", vecData);
		for (auto& it : vecData)
		{
			std::string& DataList = it;
			std::vector<std::string> vecList;
			Utility::stringSplit(DataList, ",", vecList);
			if (vecList.size() != 4)
				continue;
			
			CT_DWORD dwUserID = atoi(vecList[0].c_str());
			gameRecord.arrUserID[gameRecord.cbUserCount] = dwUserID;
			if (dwUserID == pQueryRecord->dwUserID)
			{
				gameRecord.wSelfChairID = atoi(vecList[1].c_str());
				gameRecord.cbReport = (CT_BYTE)atoi(vecList[2].c_str());
			}
			gameRecord.arrScore[gameRecord.cbUserCount++] = atoi(vecList[3].c_str())*TO_DOUBLE;
		}
		vecGameRecord.push_back(gameRecord);
	}
	db->free_result();
	
	return true;
}

CT_BOOL DBThread::GetGameRecord2(acl::db_handle* db, CT_DWORD dwDrawID, CT_DWORD dwQueryUserID, std::vector<MSG_SC_Query_GameRecord>& vecGameRecord, std::string& date, CT_WORD wMaxCount)
{
	std::stringstream sql;
    sql << " select b.drawid, group_concat(b.userid, ',', b.chairid, ',', b.report,  ',', b.score SEPARATOR ';') as data, a.roomKindid, a.bankUserid, FROM_UNIXTIME(a.fixdate) as fixdate \
        from record_draw_info_" << date <<  " a inner join record_draw_score_" << date << " b on b.drawid = a.drawid " << " inner join playback.playbackdata_" << date << " c on c.drawid = a.drawid \
        where a.drawid = " << dwDrawID << " GROUP BY b.drawid LIMIT 1";
    
	if (db->sql_select(sql.str().c_str()) ==false)
	{
		LOG(WARNING) << "QueryGameRecord fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
		return false;
	}
	
	MSG_SC_Query_GameRecord gameRecord;
	for (size_t i = 0; i < db->length(); i++)
	{
		memset(&gameRecord, 0, sizeof(gameRecord));
		const acl::db_row* row = (*db)[i];
		gameRecord.dwDrawID = atoi((*row)["drawid"]);
		gameRecord.wRoomKindID = (CT_WORD)atoi((*row)["roomKindid"]);
		gameRecord.dwBankerUserID = (CT_DWORD)atoi((*row)["bankUserid"]);
		std::string strFixdate = (*row)["fixdate"];
		_snprintf_info(gameRecord.szDate, sizeof(gameRecord.szDate), "%s", strFixdate.substr(0, 10).c_str());
		_snprintf_info(gameRecord.szTime, sizeof(gameRecord.szTime), "%s", strFixdate.substr(11).c_str());
		
		std::string data = (*row)["data"];
		std::vector<std::string> vecData;
		Utility::stringSplit(data, ";", vecData);
		for (auto& it : vecData)
		{
			std::string& DataList = it;
			std::vector<std::string> vecList;
			Utility::stringSplit(DataList, ",", vecList);
			if (vecList.size() != 4)
				continue;
			
			CT_DWORD dwUserID = atoi(vecList[0].c_str());
			gameRecord.arrUserID[gameRecord.cbUserCount] = dwUserID;
			if (dwUserID == dwQueryUserID)
			{
				gameRecord.wSelfChairID = atoi(vecList[1].c_str());
				gameRecord.cbReport = (CT_BYTE)atoi(vecList[2].c_str());
			}
			gameRecord.arrScore[gameRecord.cbUserCount++] = atoi(vecList[3].c_str())*TO_DOUBLE;
		}
		vecGameRecord.push_back(gameRecord);
	}
	db->free_result();
	
	return true;
}

CT_VOID DBThread::InsertPlaybackRecord2(CT_VOID* pData, CT_DWORD dwDataSize, CT_DWORD dwDrawID)
{
	acl::db_pool* pool = GetPlaybackdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get play back db pool fail.";
		return;
	}
	
	acl::db_mysql* db = (db_mysql*)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get play back score db handle fail.";
		return;
	}
	
	std::string thisMonth(Utility::GetTimeNowString("%Y%m"));
	std::string table("playbackdata_");
	table += thisMonth;
	//CMD_Command* pMesHead = (CMD_Command*)pData;
	//LOG(WARNING) << "play record main id: " << pMesHead->dwMainID << ", sub id: " << pMesHead->dwSubID << ", data size: " << pMesHead->dwDataSize;
	do
	{
		std::ostringstream strSQL;
		strSQL << "insert into " << table << "(drawId, data) values (" << dwDrawID <<  ",'" << CCDBConnector::ConvertBinaryToString(db->get_conn(), (char*)pData, dwDataSize) << "')";
		
		//必须生成一次std::string?
		std::string strSQL2 = strSQL.str();
		if (db->sql_update(strSQL2.c_str()) == false)
		{
			int nError = db->get_errno();
			LOG(WARNING) << "insert playback record fail, error = " << nError << ", error info: " << db->get_error() <<", draw id: " << dwDrawID;
			break;
		}
	}while(0);
	
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::DelUserDianKongData(MSG_GS2DB_DelUserDianKongData *pDelDianKongData)
{
	acl::db_pool *pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "DelUserDianKongData get game db pool fail.";
		return;
	}

	acl::db_mysql* db = (db_mysql*)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "DelUserDianKongData get game db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("delete from user_diankong_data where UserID=:UserID").set_format("UserID", "%u", pDelDianKongData->dwUserID);
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "DelUserDianKongData fail" << " UserID: " << pDelDianKongData->dwUserID << " db->get_errno: " << db->get_errno()
			<< " db->get_error: " << db->get_error();
		pool->put(db);
		return;
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::QueryHBSLRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Hbsl_Record* pQueryRecord)
{
	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}

	/*
	 * //红包扫雷游戏记录
struct MSG_SC_Query_HBSLRecord
{
	CT_DWORD        dwSendUserId;                           //发送者ID
	CT_BYTE         cbHeadID;                               //头像ID
	CT_BYTE         cbSex;                                  //性别
	CT_BYTE         cbVip2;                                 //VIP2等级
	CT_DOUBLE       dAmount;                                //红包金额
    CT_WORD         wAllotCount;		                    //分包个数
    CT_BYTE         cbMultiple;                             //红包赔率
    CT_BYTE         cbThunderNO;                            //雷号
};
	 */

	std::map<CT_DWORD , std::shared_ptr<MSG_SC_Query_HBSLRecord>> mapHbslRecord;
	std::map<CT_DWORD, std::vector<std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail>>> mapHbslRecordDetail;
	do
	{
		//读取红包主信息
		acl::query query1;
		query1.create_sql("SELECT b.id, b.sendUserid,b.`sendTime`, b.amount, b.allotCount, b.multiple,b.thunderNo, c.`gender`, c.`headId`, d.`vip2` from record_hbsl_score a \n"
						  "INNER JOIN record_hbsl_info b on a.id = b.id \n"
						  "INNER JOIN account.userinfo c ON b.`sendUserid` = c.userid \n"
						  "INNER JOIN account.`userdata` d ON b.`sendUserid` = d.userid\n"
						  "WHERE a.userid = :userid and b.roomKindid = 3 ORDER BY a.id desc LIMIT 20")
						  .set_format("userid", "%u", pQueryRecord->dwUserID);

		if (db->exec_select(query1) == false)
		{
			LOG(WARNING) << "QueryHBSLRecord fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row *row = (*db)[i];
			CT_DWORD dwId = (CT_DWORD)atoi((*row)["id"]);

			std::shared_ptr<MSG_SC_Query_HBSLRecord> hbslRecord = std::make_shared<MSG_SC_Query_HBSLRecord>();
			hbslRecord->dwSendUserId = (CT_DWORD)atoi((*row)["sendUserid"]);
			hbslRecord->dwSendTime = (CT_DWORD)atoi((*row)["sendTime"]);
			hbslRecord->cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
			hbslRecord->cbSex = (CT_BYTE)atoi((*row)["gender"]);
			hbslRecord->cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
			hbslRecord->dAmount = atoi((*row)["amount"])*TO_DOUBLE;
			hbslRecord->wAllotCount = (CT_WORD)atoi((*row)["allotCount"]);
			hbslRecord->dMultiple = Utility::round(atof((*row)["multiple"]), 1);
			hbslRecord->cbThunderNO = (CT_BYTE)atoi((*row)["thunderNo"]);
			mapHbslRecord.insert(std::make_pair(dwId, hbslRecord));
		}

		//读取红包的抢夺信息
		for (auto& it : mapHbslRecord)
		{
            db->free_result();

            query1.reset();
            query1.create_sql("SELECT * FROM record_hbsl_score WHERE id=:id")
                .set_format("id", "%u", it.first);

            if (db->exec_select(query1) == false)
            {
                LOG(WARNING) << "QueryHBSLRecord detail fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
                break;
            }

            for (size_t i = 0; i < db->length(); i++)
            {
                std::vector<std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail>> &vecHbslRecord = mapHbslRecordDetail[it.first];
                std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail> hbslRecordDetail = std::make_shared<MSG_SC_Query_HBSLRecord_Detail>();
                const acl::db_row *row = (*db)[i];
                hbslRecordDetail->dwUserID = (CT_DWORD) atoi((*row)["userid"]);
                hbslRecordDetail->dGrabScore = atoi((*row)["score"]) * TO_DOUBLE;
                hbslRecordDetail->cbIsThunder = (CT_BYTE) atoi((*row)["isthunder"]);
                vecHbslRecord.push_back(hbslRecordDetail);
            }
        }
	}while (0);

	MSG_GameMsgDownHead headMsg;
	headMsg.dwMainID = MSG_QUERY_MAIN;
	headMsg.dwSubID = SUB_SC_QUERY_HBSL_RECORD_START;
	headMsg.dwValue2 = pQueryRecord->dwUserID;
	//发送开始
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &headMsg, sizeof(MSG_GameMsgDownHead));

	for (auto& it : mapHbslRecord)
	{
		CT_BYTE szBuffer[1024];
		CT_DWORD dwSendSize = 0;
		headMsg.dwSubID = SUB_SC_QUERY_HBSL_RECORD;

		memcpy(szBuffer + dwSendSize, &headMsg, sizeof(MSG_GameMsgDownHead));
		dwSendSize += sizeof(MSG_GameMsgDownHead);

		memcpy(szBuffer + dwSendSize, it.second.get(), sizeof(MSG_SC_Query_HBSLRecord));
		dwSendSize += sizeof(MSG_SC_Query_HBSLRecord);

		std::vector<std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail>> &vecHbslRecord = mapHbslRecordDetail[it.first];
		for (auto& itDetail : vecHbslRecord)
		{
			memcpy(szBuffer + dwSendSize, itDetail.get(), sizeof(MSG_SC_Query_HBSLRecord_Detail));
			dwSendSize += sizeof(MSG_SC_Query_HBSLRecord_Detail);
		}
		//发送数据
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer, dwSendSize);
	}

	//发送结束
	headMsg.dwSubID = SUB_SC_QUERY_HBSL_RECORD_FINISH;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &headMsg, sizeof(MSG_GameMsgDownHead));

	db->free_result();
	pool->put(db);
}


CT_VOID DBThread::QueryHBSLFaRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Hbsl_Record* pQueryRecord) {
	acl::db_pool *pool = GetRecordScoredbPool();
	if (pool == NULL) {
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}

	acl::db_handle *db = pool->peek_open();
	if (db == NULL) {
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}

	/*
	 * //红包扫雷游戏记录
struct MSG_SC_Query_HBSLRecord
{
	CT_DWORD        dwSendUserId;                           //发送者ID
	CT_BYTE         cbHeadID;                               //头像ID
	CT_BYTE         cbSex;                                  //性别
	CT_BYTE         cbVip2;                                 //VIP2等级
	CT_DOUBLE       dAmount;                                //红包金额
    CT_WORD         wAllotCount;		                    //分包个数
    CT_BYTE         cbMultiple;                             //红包赔率
    CT_BYTE         cbThunderNO;                            //雷号
};
	 */

	std::map<CT_DWORD, std::shared_ptr<MSG_SC_Query_HBSLRecord>> mapHbslRecord;
	std::map<CT_DWORD, std::vector<std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail>>> mapHbslRecordDetail;
	do {
		//读取红包主信息
		acl::query query1;
		query1.create_sql(
						"SELECT b.id, b.sendUserid,b.`sendTime`, b.amount, b.allotCount, b.multiple,b.thunderNo, c.`gender`, c.`headId`, d.`vip2` from record_hbsl_info b \n"
						"INNER JOIN account.userinfo c ON b.`sendUserid` = c.userid \n"
						"INNER JOIN account.`userdata` d ON b.`sendUserid` = d.userid\n"
						"WHERE b.sendUserid = :userid and b.roomKindid = 3 ORDER BY b.id desc LIMIT 20")
				.set_format("userid", "%u", pQueryRecord->dwUserID);

		if (db->exec_select(query1) == false) {
			LOG(WARNING) << "QueryHBSLRecord fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++) {
			const acl::db_row *row = (*db)[i];
			CT_DWORD dwId = (CT_DWORD) atoi((*row)["id"]);

			std::shared_ptr<MSG_SC_Query_HBSLRecord> hbslRecord = std::make_shared<MSG_SC_Query_HBSLRecord>();
			hbslRecord->dwSendUserId = (CT_DWORD) atoi((*row)["sendUserid"]);
			hbslRecord->dwSendTime = (CT_DWORD) atoi((*row)["sendTime"]);
			hbslRecord->cbHeadID = (CT_BYTE) atoi((*row)["headId"]);
			hbslRecord->cbSex = (CT_BYTE) atoi((*row)["gender"]);
			hbslRecord->cbVip2 = (CT_BYTE) atoi((*row)["vip2"]);
			hbslRecord->dAmount = atoi((*row)["amount"]) * TO_DOUBLE;
			hbslRecord->wAllotCount = (CT_WORD) atoi((*row)["allotCount"]);
			hbslRecord->dMultiple = Utility::round(atof((*row)["multiple"]), 1);
			hbslRecord->cbThunderNO = (CT_BYTE) atoi((*row)["thunderNo"]);
			mapHbslRecord.insert(std::make_pair(dwId, hbslRecord));
		}

		//读取红包的抢夺信息
		for (auto &it : mapHbslRecord) {
			db->free_result();

			query1.reset();
			query1.create_sql("SELECT * FROM record_hbsl_score WHERE id=:id")
					.set_format("id", "%u", it.first);

			if (db->exec_select(query1) == false) {
				LOG(WARNING) << "QueryHBSLRecord detail fail. errno: " << db->get_errno() << ", err msg: "
							 << db->get_error();
				break;
			}

			for (size_t i = 0; i < db->length(); i++) {
				std::vector<std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail>> &vecHbslRecord = mapHbslRecordDetail[it.first];
				std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail> hbslRecordDetail = std::make_shared<MSG_SC_Query_HBSLRecord_Detail>();
				const acl::db_row *row = (*db)[i];
				hbslRecordDetail->dwUserID = (CT_DWORD) atoi((*row)["userid"]);
				hbslRecordDetail->dGrabScore = atoi((*row)["score"]) * TO_DOUBLE;
				hbslRecordDetail->cbIsThunder = (CT_BYTE) atoi((*row)["isthunder"]);
				vecHbslRecord.push_back(hbslRecordDetail);
			}
		}
	} while (0);

	MSG_GameMsgDownHead headMsg;
	headMsg.dwMainID = MSG_QUERY_MAIN;
	headMsg.dwSubID = SUB_SC_QUERY_HBSL_FA_RECORD_START;
	headMsg.dwValue2 = pQueryRecord->dwUserID;
	//发送开始
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &headMsg,
									sizeof(MSG_GameMsgDownHead));

	for (auto &it : mapHbslRecord) {
		CT_BYTE szBuffer[1024];
		CT_DWORD dwSendSize = 0;
		headMsg.dwSubID = SUB_SC_QUERY_HBSL_FA_RECORD;

		memcpy(szBuffer + dwSendSize, &headMsg, sizeof(MSG_GameMsgDownHead));
		dwSendSize += sizeof(MSG_GameMsgDownHead);

		memcpy(szBuffer + dwSendSize, it.second.get(), sizeof(MSG_SC_Query_HBSLRecord));
		dwSendSize += sizeof(MSG_SC_Query_HBSLRecord);

		std::vector<std::shared_ptr<MSG_SC_Query_HBSLRecord_Detail>> &vecHbslRecord = mapHbslRecordDetail[it.first];
		for (auto &itDetail : vecHbslRecord) {
			memcpy(szBuffer + dwSendSize, itDetail.get(), sizeof(MSG_SC_Query_HBSLRecord_Detail));
			dwSendSize += sizeof(MSG_SC_Query_HBSLRecord_Detail);
		}
		//发送数据
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer,
										dwSendSize);
	}

	//发送结束
	headMsg.dwSubID = SUB_SC_QUERY_HBSL_FA_RECORD_FINISH;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &headMsg,
									sizeof(MSG_GameMsgDownHead));

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::QueryHBSLWinColorPrizeRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Hbsl_Record* pQueryRecord)
{
	acl::db_pool* pool = GetRecordScoredbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get record score db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}
	//SUB_SC_QUERY_HBSL_COLOR_PRIZE_RECORD
	//红包扫雷彩金记录
	/*struct MSG_SC_Query_HBSLWinColorPrizeRecord
	{
		CT_DWORD        dwUserID;                               //玩家ID
		CT_BYTE         cbSex;                                  //性别
		CT_BYTE         cbVip2;                                 //VIP2等级
		CT_BYTE         cbHeadID;                               //头像ID
		CT_DOUBLE       dWinColorPrize;                         //中彩金额
		CT_BYTE         cWinPrizeType;                          //抢红包获得彩金1，发红包获得彩金为2
		CT_DWORD 		dwTime;								    //时间
	};*/
	std::map<CT_DWORD, std::shared_ptr<MSG_SC_Query_HBSLWinColorPrizeRecord>> mapHbslFaHBWinColorPrizeRecord;
	std::map<CT_DWORD, std::vector<std::shared_ptr<MSG_SC_Query_HBSLWinColorPrizeRecord>>> mapHbslGrabHBWinColorPrizeRecord;
	do
	{
		//读取红包主信息
		acl::query query1;
		query1.create_sql("SELECT b.id, b.sendUserid,b.`endTime`, b.amount, b.allotCount, b.multiple,b.thunderNo, b.winColorPrize, c.`gender`, c.`headId`, d.`vip2` from record_hbsl_info b \n"
						  "INNER JOIN account.userinfo c ON b.`sendUserid` = c.userid \n"
						  "INNER JOIN account.`userdata` d ON b.`sendUserid` = d.userid\n"
						  "WHERE b.`winColorPrize` > 0 and b.roomKindid = 3 ORDER BY b.id desc LIMIT 5");

		if (db->exec_select(query1) == false)
		{
			LOG(WARNING) << "QueryHBSLRecord fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row *row = (*db)[i];
			CT_DWORD dwId = (CT_DWORD)atoi((*row)["id"]);

			std::shared_ptr<MSG_SC_Query_HBSLWinColorPrizeRecord> hbslWinColorRecord = std::make_shared<MSG_SC_Query_HBSLWinColorPrizeRecord>();
			hbslWinColorRecord->dwUserID = (CT_DWORD)atoi((*row)["sendUserid"]);
			hbslWinColorRecord->dwTime = (CT_DWORD)atoi((*row)["endTime"]);
			hbslWinColorRecord->cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
			hbslWinColorRecord->cbSex = (CT_BYTE)atoi((*row)["gender"]);
			hbslWinColorRecord->cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
			hbslWinColorRecord->dWinColorPrize = atoi((*row)["winColorPrize"])*TO_DOUBLE;
			hbslWinColorRecord->cWinPrizeType = 2;
			mapHbslFaHBWinColorPrizeRecord.insert(std::make_pair(dwId, hbslWinColorRecord));
		}

		//读取红包的抢夺信息
		for (auto& it : mapHbslFaHBWinColorPrizeRecord)
		{
			db->free_result();

			query1.reset();
			query1.create_sql("SELECT a.*, b.endTime, c.`gender`, c.`headId`, d.`vip2` FROM record_hbsl_score a \n"
							  "INNER JOIN record_hbsl_info b on a.id = b.id \n"
							  "INNER JOIN account.userinfo c ON a.`userid` = c.userid \n"
							  "INNER JOIN account.`userdata` d ON a.`userid` = d.userid\n"
		                      "WHERE a.id=:id and a.winColorPrize > 0")
					.set_format("id", "%u", it.first);

			if (db->exec_select(query1) == false)
			{
				LOG(WARNING) << "QueryHBSLRecord detail fail. errno: " << db->get_errno() << ", err msg: " << db->get_error();
				break;
			}

			for (size_t i = 0; i < db->length(); i++)
			{
				std::vector<std::shared_ptr<MSG_SC_Query_HBSLWinColorPrizeRecord>> &vecHbslWinColorRecord = mapHbslGrabHBWinColorPrizeRecord[it.first];
				std::shared_ptr<MSG_SC_Query_HBSLWinColorPrizeRecord> hbslWinColorRecord = std::make_shared<MSG_SC_Query_HBSLWinColorPrizeRecord>();
				const acl::db_row *row = (*db)[i];
				hbslWinColorRecord->dwUserID = (CT_DWORD) atoi((*row)["userid"]);
				hbslWinColorRecord->dwTime = (CT_DWORD)atoi((*row)["endTime"]);
				hbslWinColorRecord->cbHeadID = (CT_BYTE)atoi((*row)["headId"]);
				hbslWinColorRecord->cbSex = (CT_BYTE)atoi((*row)["gender"]);
				hbslWinColorRecord->cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
				hbslWinColorRecord->dWinColorPrize = atoi((*row)["winColorPrize"])*TO_DOUBLE;
				hbslWinColorRecord->cWinPrizeType = 1;
				vecHbslWinColorRecord.push_back(hbslWinColorRecord);
			}
		}
	}while (0);

	MSG_GameMsgDownHead headMsg;
	headMsg.dwMainID = MSG_QUERY_MAIN;
	headMsg.dwSubID = SUB_SC_QUERY_HBSL_COLOR_PRIZE_RECORD;
	headMsg.dwValue2 = pQueryRecord->dwUserID;
	CT_BYTE szBuffer[1024];
	CT_DWORD dwSendSize = 0;
	memcpy(szBuffer, &headMsg, sizeof(MSG_GameMsgDownHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);
	for (std::map<CT_DWORD, std::shared_ptr<MSG_SC_Query_HBSLWinColorPrizeRecord>>::reverse_iterator rit = mapHbslFaHBWinColorPrizeRecord.rbegin();
	rit != mapHbslFaHBWinColorPrizeRecord.rend(); rit++)
	{
		if(dwSendSize + sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord) > sizeof(szBuffer) || (((dwSendSize-sizeof(MSG_GameMsgDownHead))/sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord)) > 9))
		{
			break;
		}
		//按照策划要求，优先显示抢红包玩家的
		std::vector<std::shared_ptr<MSG_SC_Query_HBSLWinColorPrizeRecord>> &vecHbslRecord = mapHbslGrabHBWinColorPrizeRecord[rit->first];
		for (auto &itDetail : vecHbslRecord) {
			memcpy(szBuffer + dwSendSize, itDetail.get(), sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord));
			dwSendSize += sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord);
			if(dwSendSize + sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord) > sizeof(szBuffer)|| (((dwSendSize-sizeof(MSG_GameMsgDownHead))/sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord)) > 9))
			{
				break;
			}
		}
		if(dwSendSize + sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord) > sizeof(szBuffer)|| (((dwSendSize-sizeof(MSG_GameMsgDownHead))/sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord)) > 9))
		{
			break;
		}
		memcpy(szBuffer + dwSendSize, rit->second.get(), sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord));
		dwSendSize += sizeof(MSG_SC_Query_HBSLWinColorPrizeRecord);
	}
	//发送数据
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &szBuffer,
									dwSendSize);
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::ReadUserMissInfo(JSFish_Player_Miss_Info *pData, acl::aio_socket_stream *pSocket)
{
	acl::db_pool *pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "ReadUserMissInfo get game db pool fail.";
		return;
	}

	acl::db_mysql* db = (db_mysql*)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "ReadUserMissInfo get game db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("select * from user_miss where userId = :userID")
					.set_format("userID", "%u", pData->dwUserID);
	if(db->exec_select(query) == false)
	{
		LOG(WARNING) << "ReadUserMissInfo fail" << "userID: " << pData->dwUserID << " db->get->errno:" << db->get_errno() << " db->get_error:"<< db->get_error();
		pool->put(db);
		return;
	}

	if(db->length() == 0)
	{
		db->free_result();
		pool->put(db);
		return;
	}

	const acl::db_row* row = (*db)[0];
	JSFish_Player_Miss_Info jpmi;
	jpmi.dwUserID = pData->dwUserID;
	jpmi.miss = atoll((*row)["miss"]);
	CNetModule::getSingleton().Send(pSocket, MSG_GDB_MAIN, SUB_DB2G_USER_MISS_DATA, &jpmi, sizeof(JSFish_Player_Miss_Info));

	//获取了记录就删除
	acl::query query_del;
	query_del.create_sql("delete from user_miss where userID = :userID").set_format("userID", "%u", pData->dwUserID);
	if(db->exec_update(query_del) == false)
	{
		LOG(WARNING) << "delete miss_info fail " << "userID :"<<pData->dwUserID;
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::ReadUserDianKongData(MSG_GS2DB_ReadDianKongData * pReadDianKongData, acl::aio_socket_stream* pSocket)
{
	acl::db_pool *pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "ReadUserDianKongData get game db pool fail.";
		return;
	}

	acl::db_mysql* db = (db_mysql*)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "ReadUserDianKongData get game db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("select * from user_diankong_data where gameIndex=:gameIndex and UserID=:UserID")
	    .set_format("gameIndex", "%u", pReadDianKongData->dwGameIndex)
		.set_format("UserID", "%u", pReadDianKongData->dwUserID);
	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "ReadUserDianKongData fail " << "UserID: " << pReadDianKongData->dwUserID << " gameIndx:" <<pReadDianKongData->dwGameIndex
			<< " db->get_errno: " << db->get_errno() << " db->get_error: " << db->get_error();
		pool->put(db);
		return;
	}

	if (db->length() == 0)
	{
		//玩家本来就没有点控数据
		db->free_result();
		pool->put(db);
		return;
	}

	if (db->length() != 1)
	{
		db->free_result();
		pool->put(db);
		LOG(WARNING) << "ReadUserDianKongData return result count error: " << db->length();
		return;
	}

	MSG_D2CS_Set_FishDiankong dianKongData;
	dianKongData.dwUserID = pReadDianKongData->dwUserID;
	const acl::db_row* row = (*db)[0];
	dianKongData.iDianKongZhi = atoi((*row)["DianKongZhi"]);
	dianKongData.llDianKongFen = atoll((*row)["DianKongFen"]);
	dianKongData.llCurrDianKongFen = atoll((*row)["CurrDianKongFen"]);
	dianKongData.bySource = 2;

	CNetModule::getSingleton().Send(pSocket, MSG_GDB_MAIN, SUB_DB2G_USER_DIANKONG_DATA, &dianKongData, sizeof(MSG_D2CS_Set_FishDiankong));
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SaveFishCurrentKuCun(MSG_GS2DB_SaveFishKuCun *pKuCunData)
{
	acl::db_pool *pool = GetPlatformdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "SaveFishCurrentKuCun get platform db pool fail.";
		return;
	}

	acl::db_mysql *db = (db_mysql *)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "SaveFishCurrentKuCun get game db handle fail.";
		return;
	}

	/*tm curTime;
	getLocalTime(&curTime, pKuCunData->llCurTime);

	int nyear = curTime.tm_year + 1900;
	int nmon = curTime.tm_mon + 1;
	int nday = curTime.tm_mday;
	int nhour = curTime.tm_hour;
	int nmin = curTime.tm_min;
	int nsec = curTime.tm_sec;*/
	acl::query query;
	query.create_sql("INSERT INTO fishcurrentkucun(ServerID, Date, KuCun) VALUES(:ServerID, :Date, :KuCun)")
		.set_format("ServerID", "%d", pKuCunData->serverID)
		//.set_format("Date", "%d-%d-%d %d:%d:%d", nyear, nmon, nday, nhour, nmin, nsec)
		.set_date("Date", pKuCunData->llCurTime)
		.set_format("KuCun", "%lld", pKuCunData->llCurKuCun);

	if (db->exec_update(query) == false)
	{
		LOG(ERROR) << "SaveFishCurrentKuCun fail " << "ServerID: " << pKuCunData->serverID << " data: " << pKuCunData->llCurTime
			<< " KuCun: " << pKuCunData->llCurKuCun << " db->get_errno(): " << db->get_errno() << " db->get_error():" << db->get_error();
			//<< " nyear: " << nyear << " nmon: " << nmon << " nday: " << nday << " nhour: " << nhour << " nmin: " << nmin << " nsec: " << nsec;
		pool->put(db);
		return;
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SaveFishControlData(MSG_GS2DB_SaveFishControlData *pControlData)
{
	acl::db_pool *pool = GetPlatformdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "SaveFishControlData get platform db pool fail.";
		return;
	}

	acl::db_mysql *db = (db_mysql *)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "SaveFishControlData get game db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("INSERT INTO stock_control_data(ServerID,TotalWinScore,TotalLoseScore,KuCun,BlackTax,BloodPoolDianKongLoseWin,KuCunStatus) VALUES (:ServerID, :TotalWinScore, :TotalLoseScore, :KuCun, :BlackTax, :BloodPoolDianKongLoseWin, :KuCunStatus) \
    ON DUPLICATE KEY UPDATE TotalWinScore=:TotalWinScore, TotalLoseScore=:TotalLoseScore, KuCun=:KuCun,BlackTax=:BlackTax,BloodPoolDianKongLoseWin=:BloodPoolDianKongLoseWin, KuCunStatus=:KuCunStatus")
		.set_format("BlackTax", "%lld", pControlData->llTotalTax)
		.set_format("KuCun", "%lld", pControlData->llTotalKuCun)
		.set_format("TotalLoseScore", "%lld", pControlData->llTotalLoseScore)
		.set_format("TotalWinScore", "%lld", pControlData->llTotalWinScore)
		.set_format("ServerID", "%d", pControlData->nServerID)
		.set_format("BloodPoolDianKongLoseWin", "%lld", pControlData->llBloodPoolDianKongWinLose)
		.set_format("KuCunStatus", "%d", pControlData->nKunCunStatus);

	if (db->exec_update(query) == false)
	{
		LOG(ERROR) << "Save Fish Control Data Fail, ServerID: " << pControlData->nServerID << " TotalWinScore: " << pControlData->llTotalWinScore
			<< " TotalLoseScore: " << pControlData->llTotalLoseScore << " KuCun: " << pControlData->llTotalKuCun 
			<< " BlackTax: " << pControlData->llTotalTax << " BloodPoolDianKongLoseWin: "<< pControlData->llBloodPoolDianKongWinLose
			<< ", kucun status: " << pControlData->nKunCunStatus << ", db->get_errno(): " << db->get_errno() << " db->get_error():" << db->get_error();
		pool->put(db);
		return;
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SaveFishStatisticsInfo(MSG_GS2DB_FishStatisticsInfo *pData)
{
	acl::db_pool *pool = GetPlatformdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "SaveFishStatisticsInfo get platform db pool fail.";
		return;
	}

	acl::db_mysql *db = (db_mysql *)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "SaveFishStatisticsInfo get game db handle fail.";
		return;
	}

	CT_DOUBLE dDeathCount = pData->llDeathCount;
	CT_DOUBLE dHitCount = pData->llHitCount;
	CT_DOUBLE dRealDeathPro = (dDeathCount / dHitCount) * 10000;
	CT_LONGLONG   llRealDeathPro = dRealDeathPro;

	acl::query query;
	query.create_sql("INSERT INTO fishstatisticsinfo(ServerID,FishTypeID,FishName,HitCount,DeathCount,TotalBL,TotalWinScore,TotalLoseScore,WinLose,RealDeathPro,LiLunDeathPro)VALUES(:ServerID,:FishTypeID,:FishName,:HitCount,:DeathCount,:TotalBL,:TotalWinScore,:TotalLoseScore,:WinLose,:RealDeathPro,:LiLunDeathPro) \
		ON DUPLICATE KEY UPDATE FishName=:FishName,HitCount=HitCount+:HitCount, DeathCount=DeathCount+:DeathCount, TotalBL=TotalBL+:TotalBL, TotalWinScore=TotalWinScore+:TotalWinScore, TotalLoseScore=TotalLoseScore+:TotalLoseScore, WinLose=WinLose+:WinLose, RealDeathPro=:RealDeathPro")
		.set_format("ServerID", "%d", pData->ServerID)
		.set_format("FishTypeID", "%lld", pData->fishTypeID)
		.set_format("FishName", "%s", pData->strFishName)
		.set_format("HitCount", "%lld", pData->llHitCount)
		.set_format("DeathCount", "%lld", pData->llDeathCount)
		.set_format("TotalBL", "%lld", pData->llTotalBL)
		.set_format("TotalWinScore", "%lld", pData->llTotalWinScore)
		.set_format("TotalLoseScore", "%lld", pData->llTotalLoseScore)
		.set_format("WinLose", "%lld", pData->llTotalWinScore - pData->llTotalLoseScore)
		.set_format("LiLunDeathPro", "%d", pData->nLiLunDeathPro)
		.set_format("RealDeathPro", "%lld", llRealDeathPro);
		;
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert into fishstatisticsinfo error" 
			<< " ServerID: " << pData->ServerID << " FishTypeID: "<< pData->fishTypeID 
			<<" FishName: "<< pData->strFishName<< " HitCount: " << pData->llHitCount 
			<< " DeathCount: "<<pData->llDeathCount << " TotalBL: "<<pData->llTotalBL
			<<" TotalWinScore: "<<pData->llTotalWinScore << " TotalLoseScore: "<<pData->llTotalLoseScore
			<< " db->get_errno(): " << db->get_errno() << " db->get_error(): " << db->get_error();
		pool->put(db);
		return;
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SavePlayerFishInfo(MSG_GS2DB_PlayerFishInfo *pPlayerFishInfo)
{
	acl::db_pool *pool = GetGamedbPool();
	if(pool == NULL)
    {
	    LOG(WARNING) << "SavePlayerFishInfo get game db pool fail.";
	    return;
    }

    acl::db_mysql *db = (db_mysql *)pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "SavePlayerFishInfo get game db handle fail.";
        return;
    }

    CT_CHAR perFishInfo[512] = {0};
    std::string strFishInfo = "";
    for(int i = 0; i < 39 ; i++)
    {
        memset(perFishInfo, 0, sizeof(perFishInfo));
        _snprintf_info(perFishInfo, 512, "%d|%d|%d|%lld_",i+1, pPlayerFishInfo->nPerFishTypeShootCount[i],
                pPlayerFishInfo->nPerFishTypeDeathCount[i], pPlayerFishInfo->llPerFishTypeScore[i]);
        strFishInfo += perFishInfo;
    }

	tm times;
	Utility::getLocalTime(&times, time(NULL));
	CT_CHAR dateTime[128] = {0};
	_snprintf_info(dateTime, 128, "%d-%d-%d %d:%d:%d",
			times.tm_year + 1900,
			times.tm_mon + 1,
			times.tm_mday,
			times.tm_hour,
			times.tm_min,
			times.tm_sec);


    acl::query query;
    query.create_sql("insert into user_fish_info(gameIndex,UserID,ExitDate,TotalShootCount,TotalShootDeathCount,FishInfo)\
                      VALUES(:gameIndex,:UserID,:ExitDate,:TotalShootCount,:TotalShootDeathCount,:FishInfo)")
    .set_format("gameIndex", "%d", pPlayerFishInfo->gameIndex)
    .set_format("UserID", "%u", pPlayerFishInfo->dwUserID)
    .set_format("ExitDate", "%s", dateTime)
    .set_format("TotalShootCount", "%d", pPlayerFishInfo->nTotalShootCount)
    .set_format("TotalShootDeathCount", "%d", pPlayerFishInfo->nTotalDeathCount)
    .set_format("FishInfo", "%s", strFishInfo.c_str());

    if(db->exec_update(query) == false)
	{
    	LOG(WARNING) << "insert into user_fish_info error "<< "gameIndex: " << pPlayerFishInfo->gameIndex
    	<<"UserID: "<< pPlayerFishInfo->dwUserID << "ExitDate: " << dateTime << "TotalShootCount: "<< pPlayerFishInfo->nTotalShootCount
    	<< "TotalShootDeathCount: "<<pPlayerFishInfo->nTotalDeathCount << "FishInfo:"<<strFishInfo.c_str()
    	<< " errno:" << db->get_errno() << " error:"<<db->get_error();
    	pool->put(db);
    	return;
	}

	db->free_result();
    pool->put(db);
}

CT_VOID DBThread::SaveTotalWinLoseScore(MSG_GS2DB_FishTotalWinLoseScore *pData)
{
	acl::db_pool *pool = GetPlatformdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "SaveTotalWinLoseScore get platform db pool fail.";
		return;
	}

	acl::db_mysql *db = (db_mysql *)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "SaveTotalWinLoseScore get game db handle fail.";
		return;
	}

	std::string strBigFishInfo = "";
	CT_CHAR tempBuf[64];
	for(int i = 0; i < pData->cbbfInfoCount;i++)
	{
		memset(tempBuf, 0, sizeof(tempBuf));
		_snprintf_info(tempBuf, 64, "%d|%lld|%lld_", pData->bfInfo[i].cbFishType, pData->bfInfo[i].bfInfo.llPlayerLostScore, pData->bfInfo[i].bfInfo.llPlayerGetScore);
		strBigFishInfo += tempBuf;
	}

	std::string strProInfo = "";
	for(int i = 0; i < pData->cbproInfoCount; i++)
	{
		memset(tempBuf, 0, sizeof(tempBuf));
		_snprintf_info(tempBuf, 64, "%u|%u_", pData->proInfo[i].dwBasePro, pData->proInfo[i].dwCheckCount);
		strProInfo += tempBuf;
	}

	acl::query query;
	query.create_sql("INSERT INTO SysCurDayTotalWinLoseScore(ServerID,Date,SysTotalWinScore,SysTotalLoseScore,SysTodayTotalKuCun,SysTodayTotalTax,SysTodayDianKongWinLoseTotalScore,SysTodayTotalEnterCount,SysTodayTotalWinCount,SysTodayTotalPoChanCount,SysTodayTotalRewardCount,SysTodayTotalXiFenCount,SysTodayBigFishInfo,SysTodayProInfo)VALUES(:ServerID,:Date,:SysTotalWinScore,:SysTotalLoseScore,:SysTodayTotalKuCun,:SysTodayTotalTax,:SysTodayDianKongWinLoseTotalScore,:SysTodayTotalEnterCount,:SysTodayTotalWinCount,:SysTodayTotalPoChanCount,:SysTodayTotalRewardCount,:SysTodayTotalXiFenCount,:SysTodayBigFishInfo,:SysTodayProInfo) \
		ON DUPLICATE KEY UPDATE SysTotalWinScore=SysTotalWinScore+:SysTotalWinScore, SysTotalLoseScore=SysTotalLoseScore+:SysTotalLoseScore,SysTodayTotalKuCun=SysTodayTotalKuCun+:SysTodayTotalKuCun,SysTodayTotalTax=SysTodayTotalTax+:SysTodayTotalTax,SysTodayDianKongWinLoseTotalScore=SysTodayDianKongWinLoseTotalScore+:SysTodayDianKongWinLoseTotalScore,SysTodayTotalEnterCount=:SysTodayTotalEnterCount,SysTodayTotalWinCount=:SysTodayTotalWinCount,SysTodayTotalPoChanCount=:SysTodayTotalPoChanCount,SysTodayTotalRewardCount=:SysTodayTotalRewardCount,SysTodayTotalXiFenCount=:SysTodayTotalXiFenCount,SysTodayBigFishInfo=:SysTodayBigFishInfo,SysTodayProInfo=:SysTodayProInfo")
		.set_format("ServerID", "%d", pData->serverID)
		.set_format("Date", "%d-%d-%d", pData->nYear, pData->nMonth, pData->nDay)
		.set_format("SysTotalWinScore", "%lld", pData->llTotalWinScore)
		.set_format("SysTotalLoseScore", "%lld", pData->llTotalLoseScore)
		.set_format("SysTodayTotalKuCun", "%lld", pData->llTodayTotalKuCun)
		.set_format("SysTodayTotalTax", "%lld", pData->llTodayTotalTax)
		.set_format("SysTodayDianKongWinLoseTotalScore", "%lld", pData->llTodayDianKongWinLoseTotalScore)
		.set_format("SysTodayTotalEnterCount", "%u", pData->dwTotalEnterCount)
		.set_format("SysTodayTotalWinCount", "%u", pData->dwTotalWinCount)
		.set_format("SysTodayTotalPoChanCount", "%u", pData->dwTotalPoChanCount)
		.set_format("SysTodayTotalRewardCount", "%u", pData->dwTotalRewardCount)
		.set_format("SysTodayTotalXiFenCount", "%u", pData->dwTotalXiFenCount)
		.set_format("SysTodayBigFishInfo", "%s", strBigFishInfo.c_str())
		.set_format("SysTodayProInfo", "%s", strProInfo.c_str());
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert into SysCurDayTotalWinLoseScore error" << "ServerID: " << pData->serverID << " TotalWinScore: "
			<< pData->llTotalWinScore << " TotalLoseScore: " << pData->llTotalLoseScore << " db->get_errno(): " << db->get_errno() << " db->get_error(): " << db->get_error()
			<< " pData->llTodayTotalKuCun: " << pData->llTodayTotalKuCun << " pData->llTodayTotalTax：" << pData->llTodayTotalTax
			<< " pData->llTodayDianKongWinLoseTotalScore: " << pData->llTodayDianKongWinLoseTotalScore;
		pool->put(db);
		return;
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SaveMissInfo(JSFish_Player_Miss_Info * pMissInfo)
{
	acl::db_pool *pool = GetGamedbPool();
	if(pool == NULL)
	{
		LOG(WARNING) << "SaveMissInfo get game db pool fail.";
		return;
	}

	acl::db_mysql* db = (db_mysql*)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "SaveMissInfo get game db handle fail.";
		return;
	}

	acl::query query;
	query.create_sql("insert into user_miss(userId, miss) VALUES(:userID, :miss) ON DUPLICATE KEY UPDATE miss = :miss") \
					.set_format("userID" , "%u", pMissInfo->dwUserID)
					.set_format("miss", "%lld", pMissInfo->miss);
	if(db->exec_update(query) == false)
	{
		LOG(WARNING) << "insert into user_miss fail, userId: " << pMissInfo->dwUserID
					 << ", miss: " << pMissInfo->miss;
	}

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::SaveUserDianKongData(MSG_G2DB_User_PointControl *pDianKongData)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "SaveUserDianKongData get game db pool fail.";
		return;
	}

	acl::db_mysql* db = (db_mysql*)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "SaveUserDianKongData get game db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		if (pDianKongData->iDianKongZhi != 0)
		{
			query.create_sql("insert into user_diankong_data(gameIndex, userID, dianKongZhi, dianKongFen,currDianKongFen) \
             values(:gameIndex, :userID, :dianKongZhi, :dianKongFen, :currDianKongFen) \
		     ON DUPLICATE KEY UPDATE currDianKongFen = :currDianKongFen")
		     .set_format("gameIndex", "%u", pDianKongData->dwGameIndex)
		     .set_format("userID", "%u", pDianKongData->dwUserID)
		     .set_format("dianKongZhi", "%d", pDianKongData->iDianKongZhi)
		     .set_format("dianKongFen", "%lld", pDianKongData->llDianKongFen)
		     .set_format("currDianKongFen", "%lld", pDianKongData->llCurrDianKongFen);
		}
		else
		{
			query.create_sql("delete from user_diankong_data where gameIndex= :gameIndex and userID= :userID")
			.set_format("gameIndex", "%u", pDianKongData->dwGameIndex)
			.set_format("userID", "%u", pDianKongData->dwUserID);
		}

		if (db->exec_update(query) == false)
		{
			LOG(WARNING) << "insert into user_diankong_data fail, gameindex: " << pDianKongData->dwGameIndex
						 << ", UserID: " << pDianKongData->dwUserID
						 << ", DianKongZhi: " << pDianKongData->iDianKongZhi
						 << ", DianKongFen: " << pDianKongData->llDianKongFen
						 << ", errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}
	}while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::UpdateGoodCardInfo(MSG_G2DB_GoodCard_Info* pGoodCardInfo)
{
	acl::db_pool* pool = GetGamedbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get game db pool fail.";
		return;
	}

	acl::db_mysql* db = (db_mysql*)pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get game db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		if (pGoodCardInfo->wGameID == GAME_ZJH)
		{
			query.create_sql("update gameScoreData set zjhCount = :zjhCount, zjhParam = :zjhParam where userid = :userid")
				.set_format("zjhCount", "%u", pGoodCardInfo->dwPlayCount)
				.set_format("zjhParam", "%d", pGoodCardInfo->cbGoodCardParam)
				.set_format("userid", "%u", pGoodCardInfo->dwUserID);

			if (db->exec_update(query) == false)
			{
				LOG(WARNING) << "update zjh good card info fail, user id: " << pGoodCardInfo->dwUserID;
				break;
			}
		}
		else if (pGoodCardInfo->wGameID == GAME_NN && pGoodCardInfo->wKindID == GAME_QZ_NN)
		{
			query.create_sql("update gameScoreData set qznnCount = :zjhCount, qznnParam = :zjhParam where userid = :userid")
				.set_format("qznnCount", "%u", pGoodCardInfo->dwPlayCount)
				.set_format("qznnParam", "%d", pGoodCardInfo->cbGoodCardParam)
				.set_format("userid", "%u", pGoodCardInfo->dwUserID);

			if (db->exec_update(query) == false)
			{
				LOG(WARNING) << "update qznn good card info fail, user id: " << pGoodCardInfo->dwUserID;
				break;
			}
		}
	}while(0);

	pool->put(db);
}

CT_VOID DBThread::LoadRechargeChannelInfo(CT_BYTE cbReload)
{
	acl::db_pool* pool = GetPlatformdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get platform score db handle fail.";
		return;
	}

	std::map<CT_DWORD, tagRechargeChannelInfo> mapRechargeChannelInfo;
	std::map<CT_DWORD, CT_BYTE> mapRechargeChannelSize;
	//tagRechargeChannelInfo channelInfo;
	//memset(&channelInfo, 0, sizeof(channelInfo));
	//channelInfo.cbReload = cbReload;
	do
	{
		acl::query query;
		//query.create_sql("SELECT * from rechargechannel ORDER BY sortid");
		//query.create_sql("select * from treasure.channel_method  where channel in (select id from treasure.channel where type = 1 and status = 1) or channel = 0 ORDER BY concat(channel,sort)");
		query.create_sql("select y.platform, y.channel, x.method, y.sort FROM\n"
                         "  (SELECT 1 AS method, 1 AS kind FROM treasure.service_provider WHERE status = 1 AND type = 1 AND aliType = 2 AND wxType = 2 AND wxpay = 1\n"
                         "   UNION SELECT 2 AS method, 2 AS kind FROM treasure.service_provider WHERE status = 1 AND type = 1 AND aliType = 2 AND wxType = 2 AND alipay = 1\n"
                         "   UNION SELECT 3 AS method, 3 AS kind FROM treasure.service_provider WHERE status = 1 AND type = 1 AND aliType = 2 AND wxType = 2 AND unionpay = 1\n"
                         "   UNION SELECT 4 AS method, 4 AS kind FROM treasure.service_provider WHERE status = 1 AND type = 1 AND aliType = 2 AND wxType = 2 AND qqpay = 1\n"
                         "   UNION SELECT 6 AS method, 6 AS kind FROM treasure.service_provider WHERE status = 1 AND type = 1 AND aliType = 2 AND wxType = 2 AND cloudflashpay = 1\n"
                         "   UNION SELECT 7 AS method, 2 AS kind FROM treasure.service_provider WHERE status = 1 AND type = 1 AND aliType = 1 AND alipay = 1\n"
                         "   UNION SELECT 8 AS method, 1 AS kind FROM treasure.service_provider WHERE status = 1 AND type = 1 AND wxType = 1 AND wxpay = 1\n"
                         "  ) x INNER JOIN treasure.channel_method y ON x.kind = y.method ORDER BY y.channel ASC, y.sort ASC;");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select recharge channel info fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}
		
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			CT_DWORD dwChannel = atoi((*row)["channel"]);

			auto it = mapRechargeChannelInfo.find(dwChannel);
			if (it == mapRechargeChannelInfo.end())
			{
				tagRechargeChannelInfo channelInfo;
				memset(&channelInfo, 0, sizeof(channelInfo));
				channelInfo.dwChannelID = dwChannel;
				channelInfo.cbReload = cbReload;
				mapRechargeChannelInfo.insert(std::make_pair(dwChannel, channelInfo));
			}

			auto itSize = mapRechargeChannelSize.find(dwChannel);
			if (itSize == mapRechargeChannelSize.end())
			{
				mapRechargeChannelSize.insert(std::make_pair(dwChannel, 0));
			}

			CT_BYTE& nIndex = mapRechargeChannelSize[dwChannel];
			if (nIndex >= RECHARGE_COUNT)
				continue;

			tagRechargeChannelInfo& channelInfo = mapRechargeChannelInfo[dwChannel];
			channelInfo.cbRechargeChannelID[nIndex] = atoi((*row)["method"]);
			//CT_BYTE cbDingEType = (CT_BYTE)atoi((*row)["aliType"]); //定额类型（1：启用定额 2：启用非定额 3：启用定额和非定额）
			channelInfo.cbState[nIndex] = 1;
			++nIndex;
			/*if (i == ((sizeof(channelInfo.cbRechargeChannelID) / sizeof(channelInfo.cbRechargeChannelID[0])) - 1))
			{
				break;
			}*/
		}
		db->free_result();

	} while (0);

	//发送给中心服
	CT_BYTE cbBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;

	for (auto& it : mapRechargeChannelInfo)
	{
		tagRechargeChannelInfo& channelInfo = it.second;
		memcpy(cbBuffer+dwSendSize, &channelInfo, sizeof(channelInfo));
		dwSendSize += sizeof(channelInfo);
	}

	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_RECHARGE_CHANNEL_INFO, &cbBuffer, dwSendSize);
	}

	pool->put(db);
}

//充值渠道的金额配置
CT_VOID DBThread::LoadRechargeChannelAmount()
{
    acl::db_pool* pool = GetPromotiondbPool();
    if (pool == NULL)
    {
        LOG(WARNING) << "get promotion db pool fail.";
        return;
    }
    
    acl::db_handle* db = pool->peek_open();
    if (db == NULL)
    {
        LOG(WARNING) << "get promotion db handle fail.";
        return;
    }

    CT_BYTE  szBuffer[SYS_NET_SENDBUF_SIZE];
    CT_DWORD dwSendSize = 0;
    tagRechargeChannelAmount rechargeChannelAmount;
    do
    {
        acl::query query;
        query.create_sql("SELECT * FROM treasure.recharge_amount_cfg ORDER BY type");
        
        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "select recharge amount cfg fail, errno:" << db->get_errno() << ", error: " << db->get_error();
            break;
        }
        
        for (size_t i = 0; i < db->length(); i++)
        {
            memset(&rechargeChannelAmount, 0, sizeof(rechargeChannelAmount));
            const acl::db_row* row = (*db)[i];
			rechargeChannelAmount.cbChannelID = (CT_BYTE)atoi((*row)["type"]);
			rechargeChannelAmount.dwAmount[0] = (CT_DWORD)atoi((*row)["amount1"]);
			rechargeChannelAmount.dwAmount[1] = (CT_DWORD)atoi((*row)["amount2"]);
			rechargeChannelAmount.dwAmount[2] = (CT_DWORD)atoi((*row)["amount3"]);
			rechargeChannelAmount.dwAmount[3] = (CT_DWORD)atoi((*row)["amount4"]);
			rechargeChannelAmount.dwAmount[4] = (CT_DWORD)atoi((*row)["amount5"]);
			rechargeChannelAmount.dwAmount[5] = (CT_DWORD)atoi((*row)["amount6"]);
			rechargeChannelAmount.dwAmount[6] = (CT_DWORD)atoi((*row)["maxAmount"]);
			memcpy(szBuffer + dwSendSize, &rechargeChannelAmount, sizeof(rechargeChannelAmount));
			dwSendSize += sizeof(rechargeChannelAmount);
        }
    } while (0);
    
    //发送给中心服
    CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
    if (pCenterInfo && pCenterInfo->pCenterSocket)
    {
        CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_RECHANGE_AMOUNT, &szBuffer, dwSendSize);
    }
	
	db->free_result();
    pool->put(db);
}

CT_VOID DBThread::LoadRechargeQQWXList()
{
	acl::db_pool* pool = GetPromotiondbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get promotion db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get promotion db handle fail.";
		return;
	}

	CT_BYTE cbBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;
	do
	{
		acl::query query;
		query.create_sql("SELECT b.channel, a.adtype AS code, a.realname, CASE WHEN a.adtype = 1 \
			THEN a.weixin  ELSE a.qq END AS message FROM promotion.promoter a INNER JOIN promotion.agent_channel b ON a.userid = b.promoter \
			WHERE a.type = 3 AND a.status = 1 AND a.checkStatus > 1 ORDER BY a.level DESC");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select promoter info fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		struct sPromoterRechargeInfo
		{
			int nType;
			std::string strMessage;
			std::string strName;

		};

		std::map<CT_WORD, std::vector<sPromoterRechargeInfo>> mapPromoterRechargeList;
		//首先读取所有的渠道信息
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			CT_WORD nChannelID = (atoi)((*row)["channel"]);

			sPromoterRechargeInfo info;
			info.nType = (atoi)((*row)["code"]);
			info.strMessage = (*row)["message"];
			info.strName = (*row)["realname"];
			std::vector<sPromoterRechargeInfo>& vecRechargeInfo = mapPromoterRechargeList[nChannelID];
			vecRechargeInfo.push_back(info);
		}

		for (auto& it : mapPromoterRechargeList)
		{
			dwSendSize = 0;

			acl::json jsonObject;

			acl::json_node& root = jsonObject.get_root();
			acl::json_node *node0, *node1, *node2, *node3, *node4, *node5, *node6;

			node0 = &root;
			node1 = &jsonObject.create_node("code", (long long int)it.first);
			node0->add_child(node1);

			acl::string strChannelID;
			strChannelID.format("%u", it.first);
			node2 = &jsonObject.create_node("message", "");
			node0->add_child(node2);

			node4 = &jsonObject.create_array();
			node5 = &jsonObject.create_node("data", node4);
			node0->add_child(node5);

			std::vector<sPromoterRechargeInfo>& vecRechargeInfo = it.second;
			auto nCount = 0;
			for (auto& itInfo : vecRechargeInfo)
			{
				node6 = &jsonObject.create_node();
				node4->add_child(node6);

				node1 = &jsonObject.create_node("code", (long long int)itInfo.nType);
				node2 = &jsonObject.create_node("message", Utility::Ansi2Utf8(itInfo.strMessage).c_str());
                node3 = &jsonObject.create_node("data", Utility::Ansi2Utf8(itInfo.strName).c_str());

				(*node6).add_child(node1).add_child(node2).add_child(node3);
				++nCount;
				//限制长度
				if (nCount >= 10)
                {
                    break;
                }
			}

			acl::string strJson;
			jsonObject.build_json(strJson);
			CT_DWORD dwLen = (CT_DWORD)strJson.length();

			memcpy(cbBuffer, strJson.c_str(), dwLen);
			dwSendSize += dwLen;

			cbBuffer[dwSendSize] = '\0';
			dwSendSize += 1;

			//发送给中心服
			CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
			if (pCenterInfo && pCenterInfo->pCenterSocket)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_RECHARGE_PROMOTER_INFO, cbBuffer, dwSendSize);
			}
		}
		db->free_result();

	} while (0);
	
	pool->put(db);
}

CT_VOID DBThread::LoadGeneralizePromoter()
{
	acl::db_pool* pool = GetPromotiondbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get promotion db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get promotion db handle fail.";
		return;
	}

	do
	{
		acl::query query;
		query.create_sql("SELECT DISTINCT(platformId) FROM treasure.quanmin_kefu");
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select generalize promoter info platform id fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		std::vector<CT_BYTE> vecPlatformId;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			CT_BYTE nPlatformId = (CT_BYTE)atoi((*row)["platformId"]);
			vecPlatformId.push_back(nPlatformId);
		}
		db->free_result();

		for (auto& itVec : vecPlatformId)
		{
			query.reset();
			query.create_sql("SELECT * FROM treasure.quanmin_kefu WHERE platformId = :platformId")
			.set_format("platformId", "%d", itVec);

			if (db->exec_select(query) == false)
			{
				LOG(WARNING) << "select generalize promoter info fail, errno:" << db->get_errno() << ", error: " << db->get_error();
				break;
			}

			tagGeneralizePromoterInfo generalizeInfo;
			memset(&generalizeInfo, 0, sizeof(generalizeInfo));
			generalizeInfo.cbPlatformId = itVec;
			for (size_t i = 0; i < db->length(); i++)
			{
				const acl::db_row* row = (*db)[i];
				std::string nick = (*row)["nickname"];
				std::string weixin = (*row)["account"];
				generalizeInfo.cbType[i] = (CT_BYTE)atoi((*row)["type"]);
				_snprintf_info(generalizeInfo.szNickName[i], sizeof(generalizeInfo.szNickName[i]), "%s",  Utility::Ansi2Utf8(nick).c_str());
				_snprintf_info(generalizeInfo.szAccount[i], sizeof(generalizeInfo.szAccount[i]), "%s", weixin.c_str());
                ++generalizeInfo.cbAccountCount;
				if (i >= 2)
				{
					break;
				}
			}
			//发送给中心服
			CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
			if (pCenterInfo && pCenterInfo->pCenterSocket)
			{
				CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_GENERALIZE_PROMOTER_INFO, &generalizeInfo, sizeof(generalizeInfo));
			}

			db->free_result();
		}

	} while (0);

	pool->put(db);
}

CT_VOID DBThread::LoadExchangeChannelStatus()
{
	acl::db_pool* pool = GetPlatformdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get platform db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get record score db handle fail.";
		return;
	}

	tagExchangeChannelStatus channeStatus;
	memset(&channeStatus, 0, sizeof(channeStatus));
	do
	{
		acl::query query;
		
		//支付宝状态
		channeStatus.cbChannelID[0] = ALIPAY_CHANNEL;
		query.create_sql("SELECT count(*) FROM treasure.service_provider where type = 2 and alipay = 1 and `status` = 1");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select exchange channel status fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			channeStatus.cbStatus[0] = atoi((*row)["count(*)"]) > 0 ? 1 : 0;
		}
		db->free_result();

		//银行卡状态
		query.reset();
		channeStatus.cbChannelID[1] = UNIONPAY_CHANNEL;
		query.create_sql("SELECT count(*) FROM treasure.service_provider where type = 2 and unionpay = 1 and `status` = 1");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select exchange channel status fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			channeStatus.cbStatus[1] = atoi((*row)["count(*)"]) > 0 ? 1 : 0;
		}
		db->free_result();

	} while (0);

	//发送给中心服
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_EXCHANGE_CHANNEL_STATUS, &channeStatus, sizeof(channeStatus));
	}

	pool->put(db);
}

CT_VOID DBThread::LoadExchangeChannelAmount()
{
	acl::db_pool* pool = GetPromotiondbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get promotion db pool fail.";
		return;
	}
	
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get promotion db handle fail.";
		return;
	}
	
	CT_BYTE  szBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;
	tagExchangeChannelAmount exchangeChannelAmount;
	memset(&exchangeChannelAmount, 0, sizeof(exchangeChannelAmount));
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM treasure.exchange_amount_cfg ORDER BY type");
		
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select excharge amount cfg fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}
		
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			exchangeChannelAmount.cbChannelID = (CT_BYTE)atoi((*row)["type"]);
			exchangeChannelAmount.dwAmount[0] = (CT_DWORD)atoi((*row)["amount1"]);
			exchangeChannelAmount.dwAmount[1] = (CT_DWORD)atoi((*row)["amount2"]);
			exchangeChannelAmount.dwAmount[2] = (CT_DWORD)atoi((*row)["amount3"]);
			exchangeChannelAmount.dwAmount[3] = (CT_DWORD)atoi((*row)["amount4"]);
			exchangeChannelAmount.dwAmount[4] = (CT_DWORD)atoi((*row)["amount5"]);
            exchangeChannelAmount.dwMinAmount = (CT_DWORD)atoi((*row)["minAmount"]);
			exchangeChannelAmount.dwMaxAmount = (CT_DWORD)atoi((*row)["maxAmount"]);
			exchangeChannelAmount.cbDayCountLimite = (CT_BYTE)atoi((*row)["dayCountLimit"]);
			exchangeChannelAmount.wHandlingFee = (CT_WORD)atoi((*row)["handlingFee"]);
			memcpy(szBuffer + dwSendSize, &exchangeChannelAmount, sizeof(exchangeChannelAmount));
			dwSendSize += sizeof(exchangeChannelAmount);
		}
	} while (0);
	
	//发送给中心服
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_EXCHANGE_AMOUNT, &szBuffer, dwSendSize);
	}
	
	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::LoadQuanMinChannelInfo()
{
	acl::db_pool* pool = GetPromotiondbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get promotion db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get promotion db handle fail.";
		return;
	}

	CT_BYTE cbBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;

	tagQuanMinChannel quanMinChannel;
	memset(&quanMinChannel, 0, sizeof(quanMinChannel));
	do
	{
		acl::query query;
		query.create_sql("SELECT id from treasure.channel where (type = 2 || type = 5) and status = 1");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select quanmin channel fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			quanMinChannel.dwChannelID = atoi((*row)["id"]);
			quanMinChannel.cbStatus = 1;

			if (dwSendSize + sizeof(tagQuanMinChannel) >= sizeof(SYS_NET_SENDBUF_SIZE))
            {
                CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
                if (pCenterInfo && pCenterInfo->pCenterSocket)
                {
                    CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_QUANMIN_CHANNEL, &cbBuffer, dwSendSize);
                }
                dwSendSize = 0;
            }

			memcpy(cbBuffer + dwSendSize, &quanMinChannel, sizeof(quanMinChannel));
			dwSendSize += sizeof(tagQuanMinChannel);
		}

        //发送给中心服
        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_QUANMIN_CHANNEL, &cbBuffer, dwSendSize);
        }

        db->free_result();
	} while (0);
	pool->put(db);
}

CT_VOID DBThread::LoadClientChannelDomain()
{
	acl::db_pool* pool = GetPromotiondbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get promotion db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get promotion db handle fail.";
		return;
	}

	CT_BYTE cbBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;

	tagClientChannelDomain channelDomain;
	memset(&channelDomain, 0, sizeof(channelDomain));
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM treasure.channel WHERE type = 1");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select client channel domain fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			channelDomain.dwChannelID = atoi((*row)["id"]);
			channelDomain.cbLock = (CT_BYTE)atoi((*row)["locked"]);
			std::string domain = (*row)["domain"];
			_snprintf_info(channelDomain.szDomain, sizeof(channelDomain.szDomain), "%s", domain.c_str());
			std::string domain2 = (*row)["domain2"];
			_snprintf_info(channelDomain.szDomain2, sizeof(channelDomain.szDomain2), "%s", domain2.c_str());
			memcpy(cbBuffer + dwSendSize, &channelDomain, sizeof(channelDomain));
			dwSendSize += sizeof(channelDomain);
		}
		db->free_result();
	} while (0);

	//发送给中心服
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_CHANNEL_DOMAIN, &cbBuffer, dwSendSize);
	}

	pool->put(db);
}

//某渠道是否显示兑换按钮条件
CT_VOID DBThread::LoadChannelShowExchangeCond()
{
	acl::db_pool* pool = GetPromotiondbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get promotion db pool fail.";
		return;
	}

	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get promotion db handle fail.";
		return;
	}

	CT_BYTE cbBuffer[SYS_NET_SENDBUF_SIZE];
	CT_DWORD dwSendSize = 0;

	tagChannelShowExchangeCond channelShowExchange;
	memset(&channelShowExchange, 0, sizeof(channelShowExchange));
	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM treasure.channel WHERE type = 1 AND showExchangeCond != 0");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select channel show exchange condition fail, errno:" << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			channelShowExchange.dwChannelID = atoi((*row)["id"]);
			channelShowExchange.dwTotalRecharge = atoi((*row)["showExchangeCond"]);
			memcpy(cbBuffer + dwSendSize, &channelShowExchange, sizeof(channelShowExchange));
			dwSendSize += sizeof(channelShowExchange);
		}
		db->free_result();
	} while (0);

	//发送给中心服
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_CHANNEL_SHOW_EXCHANGE, &cbBuffer, dwSendSize);
	}

	pool->put(db);
}

CT_VOID DBThread::LoadPlatformRechargeCfg()
{
	acl::db_pool* pool = GetPlatformdbPool();
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

	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM platform_recharge_cfg");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select platfrom recharge config fail.";
			break;
		}

		CT_BYTE cbBuffer[2048];
		CT_DWORD dwSendSize = 0;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];

            tagPlatformRechargeCfg platformRechargeCfg;
            platformRechargeCfg.cbPlatformId = (CT_BYTE)atoi((*row)["platformId"]);
            platformRechargeCfg.cbPromoterRechargeType = (CT_BYTE)atoi((*row)["PromoterRechargeType"]);
            _snprintf_info(platformRechargeCfg.szRechargeDomain, sizeof(platformRechargeCfg.szRechargeDomain), "%s", (*row)["rechargeDomain"]);
            _snprintf_info(platformRechargeCfg.szPromoterRechargeUrl, sizeof(platformRechargeCfg.szPromoterRechargeUrl), "%s", (*row)["PromoterRechargeUrl"]);
            _snprintf_info(platformRechargeCfg.szPromoterRechargeUpload, sizeof(platformRechargeCfg.szPromoterRechargeUpload), "%s", (*row)["PromoterRechargeUpload"]);
            _snprintf_info(platformRechargeCfg.szPromoterRechargeMerchant, sizeof(platformRechargeCfg.szPromoterRechargeMerchant), "%s", (*row)["PromoterRechargeMerchant"]);
            _snprintf_info(platformRechargeCfg.szPromoterRechargeToken, sizeof(platformRechargeCfg.szPromoterRechargeToken), "%s", (*row)["PromoterRechargeToken"]);
			memcpy(cbBuffer+dwSendSize, &platformRechargeCfg, sizeof(tagPlatformRechargeCfg));
			dwSendSize += sizeof(tagPlatformRechargeCfg);
		}
		//发送给中心服
		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_PLATFORM_RECHARGE_CFG, &cbBuffer, dwSendSize);
		}

		db->free_result();

	}while (0);

	pool->put(db);
}


//加载VIP2等级
CT_VOID DBThread::LoadVip2Config()
{
	acl::db_pool* pool = GetPlatformdbPool();
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

	do
	{
		acl::query query;
		query.create_sql("SELECT * FROM vip2config");

		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "select vip2 config fail.";
			break;
		}

		CT_BYTE cbBuffer[1024];
		CT_DWORD dwSendSize = 0;

		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];

			tagVip2Config vip2Config;
			vip2Config.cbVip2 = (CT_BYTE)atoi((*row)["level"]);
			vip2Config.dwRecharge = atoi((*row)["cash"]);
			memcpy(cbBuffer+dwSendSize, &vip2Config, sizeof(vip2Config));
			dwSendSize += sizeof(vip2Config);
		}
		//发送给中心服
		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_VIP2_CONFIG, &cbBuffer, dwSendSize);
		}

		db->free_result();

	}while (0);

	pool->put(db);
}

//加载救济金配置
CT_VOID DBThread::LoadBenefitConfig()
{
	acl::db_pool* pool = GetPlatformdbPool();
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
	query.create_sql("SELECT * FROM benefit_config");

	if (db->exec_select(query) == false)
	{
		LOG(WARNING) << "select benefit config fail.";
		return;
	}

	CT_BYTE cbBuffer[1024];
	CT_DWORD dwSendSize = 0;

	for (size_t i = 0; i < db->length(); i++)
	{
		const acl::db_row* row = (*db)[i];

		tagBenefitReward benefitReward;
		benefitReward.cbVip2 = (CT_BYTE)atoi((*row)["vip2"]);
		benefitReward.cbRewardCount = (CT_BYTE)atoi((*row)["totalCount"]);
		benefitReward.dwRewardScore = atoi((*row)["rewardScore"]);
		benefitReward.dwLessScore = atoi((*row)["lessScore"]);
		memcpy(cbBuffer+dwSendSize, &benefitReward, sizeof(tagBenefitReward));
		dwSendSize += sizeof(tagBenefitReward);
	}

	db->free_result();

	//发送给中心服
	CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
	if (pCenterInfo && pCenterInfo->pCenterSocket)
	{
		CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_BENEFIT_CONFIG, &cbBuffer, dwSendSize);
	}

	pool->put(db);
}

CT_VOID DBThread::LoadChatUploadUrl()
{
	acl::db_pool* pool = GetPlatformdbPool();
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

	do
	{
		acl::query query;
		query.create_sql("SELECT statusDescription from systemstatus where statusName = 'chatUploadUrl'");
		if (db->exec_select(query) == false)
		{
			LOG(WARNING) << "LoadChatUploadUrl fail, errno: " << db->get_errno() << ", error: " << db->get_error();
			break;
		}

		tagChatUpLoadUrl chatUpLoadUrl;
		for (size_t i = 0; i < db->length(); i++)
		{
			const acl::db_row* row = (*db)[i];
			_snprintf_info(chatUpLoadUrl.szUploadUrl, sizeof(chatUpLoadUrl.szUploadUrl), "%s", (*row)["statusDescription"]);
		}

		//发送给中心服
		CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
		if (pCenterInfo && pCenterInfo->pCenterSocket)
		{
			CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_CHAT_UPLOAD_URL, &chatUpLoadUrl,
											sizeof(tagChatUpLoadUrl));
		}

	} while (0);

	db->free_result();
	pool->put(db);
}

CT_VOID DBThread::LoadAnnouncementInfo()
{
    acl::db_pool* pool = GetPlatformdbPool();
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

    do
    {
        acl::query query;
        query.create_sql("SELECT * FROM announcement_cfg where `status` = 1");
        if (db->exec_select(query) == false)
        {
            LOG(WARNING) << "LoadAnnouncementInfo fail, errno: " << db->get_errno() << ", error: " << db->get_error();
            break;
        }

        CT_BYTE  szBuffer[SYS_NET_SENDBUF_SIZE];
        CT_DWORD dwSendSize = 0;

        tagAnnouncementInfo announcementInfo;
        announcementInfo.cbStatus = 1;
        for (size_t i = 0; i < db->length(); i++)
        {
            const acl::db_row* row = (*db)[i];
            announcementInfo.cbAnnouncementID = (CT_BYTE)atoi((*row)["Id"]);
            announcementInfo.cbSortID = (CT_BYTE)atoi((*row)["sortId"]);
            _snprintf_info(announcementInfo.szName, sizeof(announcementInfo.szName), "%s", (*row)["name"]);
            _snprintf_info(announcementInfo.szContent, sizeof(announcementInfo.szContent), "%s", (*row)["content"]);

            memcpy(szBuffer + dwSendSize, &announcementInfo, sizeof(tagAnnouncementInfo));
            dwSendSize += sizeof(tagAnnouncementInfo);
        }

        //发送给中心服
        CMD_CenterServer_InfoEx* pCenterInfo = CServerMgr::get_instance().GetCenterServerInfo();
        if (pCenterInfo && pCenterInfo->pCenterSocket)
        {
            CNetModule::getSingleton().Send(pCenterInfo->pCenterSocket, MSG_CSDB_MAIN, SUB_DB2CS_ANNOUNCEMENT_INFO, szBuffer,
											dwSendSize);
        }

    } while (0);

    db->free_result();
    pool->put(db);
}
