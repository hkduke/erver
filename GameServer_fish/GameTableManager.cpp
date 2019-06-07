#include "GameTableManager.h"
#include "ServerCfg.h"
#include "glog_wrapper.h"
#include "IGameOtherData.h"
#include "Utility.h"
#include "NetModule.h"
#include <algorithm>

#define  __LOAD_DL_LIB__

#ifdef __LOAD_DL_LIB__
#ifndef _OS_WIN32_CODE
#include <dlfcn.h>
#endif
#else
#include "../logic_ld_fpf/DefGameLogicExport.h"
#endif
//#include "DBFiber.h"
extern CNetConnector *pNetCenter;
extern CNetConnector *pNetDB;

CGameTableManager::CGameTableManager()
{

}

CGameTableManager::~CGameTableManager()
{
	Clear();
}

void CGameTableManager::InitAllTable(CT_DWORD dwTableCount, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis, acl::mysql_manager* pdbManager)
{
	if (pGameKind == NULL || pRoomKind == NULL)
	{
		return;
	}
	m_pGameKind = pGameKind;
	m_pRoomKind = pRoomKind;

	m_pdbManager = pdbManager;
	m_accountDBKey.format("%s@%s:%d", CServerCfg::m_AccountdbName, CServerCfg::m_AccountdbIP, CServerCfg::m_AccountdbPort);
	m_recordDBKey.format("%s@%s:%d", CServerCfg::m_RecorddbName, CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);
	m_platformDBKey.format("%s@%s:%d", CServerCfg::m_PlatformdbName, CServerCfg::m_PlatformdbIP, CServerCfg::m_PlatformdbPort);

	if (pRoomKind->wRoomKindID == PRIVATE_ROOM)
	{
		std::string strPlayCount;
		std::string strNeedGem;

		acl::json proomJson(pRoomKind->szPRoomCond);
		const acl::json_node* pNodePlayCount = proomJson.getFirstElementByTagName("playcount");
		if (pNodePlayCount)
		{
			strPlayCount = pNodePlayCount->get_string();
		}

		const acl::json_node* pNodeNeedGem = proomJson.getFirstElementByTagName("needgem");
		if (pNodeNeedGem)
		{
			strNeedGem = pNodeNeedGem->get_string();
		}

		std::vector<std::string> vecPlayCount;
		Utility::stringSplit(strPlayCount, ",", vecPlayCount);

		std::vector<std::string> vecNeedGem;
		Utility::stringSplit(strNeedGem, ",", vecNeedGem);

		if (vecPlayCount.size() != vecNeedGem.size())
		{
			LOG(WARNING) << "need gem and playcount not equal!";
			exit(0);
		}

		//
		for (auto it = vecPlayCount.begin(); it != vecPlayCount.end(); ++it)
		{
			auto nIndexNeedGem = distance(vecPlayCount.begin(), it);
			m_mapPRoomNeedGem.insert(std::make_pair(atoi(it->c_str()), atoi(vecNeedGem[nIndexNeedGem].c_str())));
		}
	}

	//CUpdateGemFiber gemFiber(GetAccountdbPool(), GetFiberSem());
	//fiber::schedule();

	/*LOG(ERROR) << "get pool";
	acl::db_pool* pool = CGameTableManager::get_instance().GetAccountdbPool();
	if (pool == NULL)
	{
		LOG(WARNING) << "get account db pool fail.";
		return;
	}
	LOG(ERROR) << "get open";
	acl::db_handle* db = pool->peek_open();
	if (db == NULL)
	{
		LOG(WARNING) << "get account db handle fail.";
		return;
	}

	LOG(ERROR) << "create sql";
	acl::query query;
	query.create_sql("update userinfo set gem = :gemval where userid = :useridval").set_format("gemval", "%u", 888).set_format("useridval", "%u", 13);

	LOG(ERROR) << "exec sql";
	if (db->exec_update(query) == false)
	{
		LOG(WARNING) << "update db gem fail, user id: " << 13;
		return;
	}

	db->free_result(); */

#ifdef __LOAD_DL_LIB__	//加载动态库
	assert(dwTableCount != 0);
	std::string strDllPath = Utility::GetCurrentDirectory();
	strDllPath.append("/");
	std::string strDllName(pGameKind->szServerDll);
	if (pRoomKind->wRoomKindID != PRIVATE_ROOM)
	{
		strDllName.append("_score");
	}

#ifdef _OS_WIN32_CODE
#ifdef _DEBUG
	strDllName.append("_d.dll");
#else
	strDllName.append(".dll");
#endif
	
#endif
#ifdef _OS_LINUX_CODE
	strDllName.insert(0, "lib");
	strDllName.append(".so");
#endif
#ifdef _OS_MAC_CODE
	strDllName.append(".dylib");
#endif

	LpDllNewDesk   pFunNewDesk = 0;
	LpDllValidate  pValidateRoom = 0;
	strDllName.insert(0, strDllPath);

#ifdef _OS_WIN32_CODE
	HMODULE hDll = 0; //DLL句柄
	hDll = LoadLibrary(strDllName.c_str());//加载DLL,需要将DLL放到工程目录下.
	assert(hDll != 0);
	if (hDll == 0)
	{
		LOG(ERROR) << "Can't find dll: " << strDllName.c_str();
		exit(0);
		return;
	}

	if (hDll != 0)
	{
		pFunNewDesk = (LpDllNewDesk)GetProcAddress(hDll, DLL_NEW_DESK_FN_NAME);
		if (pFunNewDesk == 0)
		{
			CT_CHAR buf[BUFSIZ] = { 0 };
			_snprintf_info(buf, BUFSIZ, "Can't find function: %s : errno: %d", DLL_NEW_DESK_FN_NAME, GetLastError());
			LOG(ERROR) << buf;
			exit(0);
			return;
		}

		pValidateRoom = (LpDllValidate)GetProcAddress(hDll, DLL_VALIDATE_ROOM_CONFIG);
		if (pValidateRoom == 0)
		{
			CT_CHAR buf[BUFSIZ] = { 0 };
			_snprintf_info(buf, BUFSIZ, "Can't find function: %s : errno: %d", DLL_VALIDATE_ROOM_CONFIG, GetLastError());
			LOG(ERROR) << buf;
			exit(0);
			return;
		}
	}

#else 
	CT_VOID*   dp = 0;
	CT_CHAR*  err = 0;
	dp = dlopen(strDllName.c_str(), RTLD_LAZY);
	if (dp == 0)
	{
		err = dlerror();
		CT_CHAR buf[BUFSIZ] = { 0 };
		_snprintf_info(buf, BUFSIZ, "Can't open dll: %s : errno: %s", strDllName.c_str(), err);
		LOG(ERROR) << buf;
		exit(0);
	}
	if (dp)
	{
		pFunNewDesk = (LpDllNewDesk)dlsym(dp, DLL_NEW_DESK_FN_NAME);
		if (pFunNewDesk == 0)
		{
			err = dlerror();
			CT_CHAR buf[BUFSIZ] = { 0 };
			_snprintf_info(buf, BUFSIZ, "Can't find function: %s : errno: %s", DLL_NEW_DESK_FN_NAME, err);
			LOG(ERROR) << buf;
			exit(0);
			return;
		}
		pValidateRoom = (LpDllValidate)dlsym(dp, DLL_VALIDATE_ROOM_CONFIG);
		if (pValidateRoom == 0)
		{
			err = dlerror();
			CT_CHAR buf[BUFSIZ] = { 0 };
			_snprintf_info(buf, BUFSIZ, "Can't find function: %s : errno: %s", DLL_VALIDATE_ROOM_CONFIG, err);
			LOG(ERROR) << buf;
			exit(0);
			return;
		}
	}

#endif

#else
	printf("use static library load game lib!");
#endif

	for (CT_DWORD i = 0; i < dwTableCount; i++)
	{
#ifdef __LOAD_DL_LIB__	//加载动态库
		ITableFrameSink* pSink = pFunNewDesk();
#else
		ITableFrameSink* pSink = GetTableInstance();
#endif

		assert(pSink != NULL);
		ITableFrame* pTable = NULL;
		if (pRoomKind->wRoomKindID == PRIVATE_ROOM)
		{
 			pTable = new CTableFrame();
		}
		else
		{
			pTable = new CTableFrameForScore();
		}
		
		assert(pTable != NULL);
		if (NULL == pSink || NULL == pTable)
		{
			SAFE_DELETE_PTR(pSink);
			SAFE_DELETE_PTR(pTable);
			LOG(ERROR)<< "Can't find important function in dll";
			exit(0);
			return;
		}

		TableState tableState;
		memset(&tableState, 0, sizeof(TableState));
		tableState.dwTableID = i;
		tableState.bIsLock = CT_FALSE;
		//tableState.bIsLookOn =  CT_FALSE;
		pTable->Init(pSink, tableState, pGameKind, pRoomKind, pRedis);
		m_vecGameTable.push_back(pTable);
		m_listFreeTable.push_back(pTable);

		pSink->SetTableFramePtr(pTable);

		//百人游戏添加库存控制相关数据
		if (m_pRoomKind->wGameID == GAME_BR)
		{
			pSink->SetTableStock(m_StockInfo.llStorageControl);
			pSink->SetTableStockLowerLimit(m_StockInfo.llStorageLowerLimit);
			pSink->SetAndroidStock(m_StockInfo.llAndroidStorage);
			pSink->SetAndStockLowerLimit(m_StockInfo.llAndroidStorageLowerLimit);
			pSink->SetSystemAllKillRatio(m_StockInfo.wSystemAllKillRatio);
		}
	}	
}

ITableFrame* CGameTableManager::GetTable(CT_DWORD id)
{
	if (id < m_vecGameTable.size())
	{
		assert(NULL != m_vecGameTable[id]);
		return (m_vecGameTable[id]);
	}
	return NULL;
}

CTableFrame* CGameTableManager::FindOneUnlockTable()
{
	for (auto it : m_vecGameTable)
	{
		CTableFrame* pTableFrame = static_cast<CTableFrame*>(it);
		if (pTableFrame->GetTableisLock() == false)
		{
			if (pTableFrame->GetPlayerCount() != 0)
			{
				TableState tableState;
				it->GetTableInfo(tableState);
				LOG(WARNING) << "find one unlock table, but player count is not 0, table id: " << tableState.dwTableID;
				continue;
			}
			return pTableFrame;
		}
	}

	return NULL;
}

CTableFrame* CGameTableManager::FindTableByRoomNum(CT_DWORD dwRoomNum)
{
	for (auto it : m_vecGameTable)
	{
		CTableFrame* pTableFrame = static_cast<CTableFrame*>(it);
		if (pTableFrame->GetTableisLock() == true)
		{
			PrivateTableInfo& privateTableInfo = pTableFrame->GetPrivateTableInfo();
			if (privateTableInfo.dwRoomNum == dwRoomNum)
			{
				return pTableFrame;
			}
		}
	}

	return NULL;
}

ITableFrame * CGameTableManager::FindNormalSuitTable(CT_DWORD wExceptTableID /*= INVALID_CHAIR*/)
{
	//for (auto it : m_vecGameTable)
	//{
	//	CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(it);
	//	if (/*(pTableFrame->GetTableisLock() == false) &&*/ (pTableFrame->GetPlayerCount() < pTableFrame->GetMaxPlayerCount()))
	//	{
	//		return pTableFrame;
	//	}
	//}
	for (auto& it : m_listUsedTable)
	{
		CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(it);

		if (wExceptTableID != INVALID_CHAIR && pTableFrame->GetTableID() == wExceptTableID)
			continue;

		if (pTableFrame->GetGameRoundPhase() == en_GameRound_Free || m_pRoomKind->wGameID == GAME_BR || m_pRoomKind->wGameID == GAME_ZJH)
		{
			if (pTableFrame->GetPlayerCount() < pTableFrame->GetMaxPlayerCount())
			{
				return pTableFrame;
			}
		}
	}

	for (auto it = m_listFreeTable.begin(); it != m_listFreeTable.end(); ++it)
	{
		CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(*it);
		if (pTableFrame->GetPlayerCount() < pTableFrame->GetMaxPlayerCount())
		{
			if (pTableFrame->GetGameRoundPhase() == en_GameRound_Free || m_pRoomKind->wGameID == GAME_BR || m_pRoomKind->wGameID == GAME_ZJH)
			{
				m_listUsedTable.push_back(pTableFrame);
				m_listFreeTable.erase(it);
				return pTableFrame;
			}
		}
	}
	
	return NULL;
}

void CGameTableManager::FreeNomalTable(ITableFrame* pTableFrame)
{
	auto it = std::find(m_listUsedTable.begin(), m_listUsedTable.end(), pTableFrame);
	if (it != m_listUsedTable.end())
	{
		m_listFreeTable.insert(m_listFreeTable.begin(), *it);
		m_listUsedTable.erase(it);
	}
}

void CGameTableManager::OnTimePulse()
{
	for (auto& it : m_vecGameTable)
	{
		ITableFrame* pTableFrame = it;
		if (pTableFrame)
		{
			pTableFrame->OnTimePulse();
		}
	}
}

void CGameTableManager::Clear()
{
	for (auto it : m_vecGameTable)
	{
		delete it;
	}
	m_vecGameTable.clear();
}

CT_VOID CGameTableManager::InsertGameRoundRewardCfg(CT_DWORD dwGameRoundCount, CT_DWORD dwReward)
{
	m_mapGameRoundReward.insert(std::make_pair(dwGameRoundCount, dwReward));
}

CT_BOOL CGameTableManager::CheckGameRoundReward(CT_DWORD dwGameRoundCount, CT_DWORD& dwReward)
{
	auto it = m_mapGameRoundReward.find(dwGameRoundCount);
	if (it == m_mapGameRoundReward.end())
	{
		return false;
	}

	dwReward = it->second;
	return true;
}

CT_VOID CGameTableManager::GoCenterUpdatePRoom(CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwRoomNum, CT_DWORD dwGroupID, CT_BYTE cbGroupType, CT_BOOL bRemoveGroupRoom, CT_BYTE cbRoomState)
{
	MSG_G2CS_UpdatePRoom updatePRoom;
	updatePRoom.wGameID = wGameID;
	updatePRoom.wKindID = wKindID;
	updatePRoom.dwRoomNum = dwRoomNum;
	updatePRoom.dwGroupID = dwGroupID;
	updatePRoom.cbGroupType = cbGroupType;
	updatePRoom.cbRoomState = cbRoomState;
	updatePRoom.bRemoveGroupRoom = bRemoveGroupRoom;

	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_UPDATE_PROOM, &updatePRoom, sizeof(updatePRoom));
}

CT_VOID CGameTableManager::SendBackGroupPRoomGem(CT_DWORD dwGroupID, CT_DWORD dwRecordID, CT_DWORD dwMasterID, CT_WORD wGem)
{
	MSG_G2CS_Return_GroupPRoom_Gem returnGem;
	returnGem.dwGroupID = dwGroupID;
	returnGem.dwRecordID = dwRecordID;
	returnGem.dwMasterID = dwMasterID;
	returnGem.wReturnGem = wGem;
	CNetModule::getSingleton().Send(pNetCenter->GetSocket(), MSG_GCS_MAIN, SUB_G2CS_RETURN_GROUPPROOM_GEM, &returnGem, sizeof(returnGem));
}

CT_VOID CGameTableManager::SendBackWXGroupPRoomGem(CT_DWORD dwGroupID, CT_DWORD dwRecordID)
{
	//退还微信群组开房的钻石
	MSG_G2DB_Return_WXGroupPRoom_Gem returnGem;
	returnGem.dwGroupID = dwGroupID;
	returnGem.dwRecordID = dwRecordID;

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_RETURN_WXGROUPPROOM_GEM, &returnGem, sizeof(returnGem));
}


CT_VOID CGameTableManager::SetTableStockInfo(tagStockInfo& stockInfo)
{
	m_StockInfo.llStorageControl = stockInfo.llStorageControl;
	m_StockInfo.llStorageLowerLimit = stockInfo.llStorageLowerLimit;
	m_StockInfo.llAndroidStorage = stockInfo.llAndroidStorage;
	m_StockInfo.llAndroidStorageLowerLimit = stockInfo.llAndroidStorageLowerLimit;
	m_StockInfo.wSystemAllKillRatio = stockInfo.wSystemAllKillRatio;
}

#include <fstream>
CT_VOID CGameTableManager::TestWritePlayback()
{
	MSG_G2DB_PRoom_PlaybackRecord playback;
	playback.uGameRoundID = 1;
	playback.wPlayCount = 2;
	_snprintf_info(playback.szEndTime, sizeof(playback.szEndTime), "%s", Utility::GetTimeNowString().c_str());

	std::ifstream t;
	int length;
	t.open("video.txt", std::ios::binary);      // open input file  
	t.seekg(0, std::ios::end);    // go to the end  
	length = (int)t.tellg();           // report location (this is the length)  
	t.seekg(0, std::ios::beg);    // go back to the beginning  
	char* buffer = new char[length];    // allocate memory for a buffer of appropriate dimension  
	t.read(buffer, length);       // read the whole file into the buffer  
	t.close();                    // close file handle  

	playback.uBinaryLen = length;
	memcpy(playback.szBinary, buffer, length);
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_PROOM_PLAYBACK_RECORD, &playback, sizeof(playback));

	delete[] buffer;
}

CT_VOID CGameTableManager::TestUpdateUserGem()
{
	MSG_UpdateUser_Gem updateGemToDB;
	updateGemToDB.dwUserID = 24;
	updateGemToDB.nAddGem = 887;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_UPDATE_USER_GEM, &updateGemToDB, sizeof(updateGemToDB));
}

CT_VOID CGameTableManager::TestPlayCountRecord()
{
	MSG_G2DB_PRoom_PlayCountRecord playCountRecord;
	playCountRecord.uGameRoundID = 1;
	playCountRecord.wCurrPlayCount = 1;
	playCountRecord.dwUserID = 24;
	playCountRecord.llScore = 100;
	playCountRecord.wChairID = 1;
	_snprintf_info(playCountRecord.szNickName, sizeof(playCountRecord.szNickName), "%s", "zixuekuang");
	_snprintf_info(playCountRecord.szEndTime, sizeof(playCountRecord.szEndTime), "%s", Utility::GetTimeNowString().c_str());
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_PROOM_PALYCOUNT_RECORD, &playCountRecord, sizeof(playCountRecord));
}

//保存私人场的结算信息
CT_VOID CGameTableManager::TestClearingInfo()
{
	MSG_G2DB_PRoom_GameRoundRecord bigRecord;
	bigRecord.uCount = 2;
	bigRecord.uGameRoundID = 1;
	bigRecord.dwArrUserID[0] = 23;
	bigRecord.dwArrUserID[1] = 24;
	bigRecord.llArrScore[0] = 100;
	bigRecord.llArrScore[1] = 200;

	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_PROOM_GAMEROUND_RECORD, &bigRecord, sizeof(bigRecord));
}