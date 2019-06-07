#include "AndroidUserMgr.h"
#include "GameTableManager.h"
#include "Utility.h"
#include "glog_wrapper.h"
#include "GlobalEnum.h"
#include "NetModule.h"
#include "GameServerThread.h"

#ifndef _OS_WIN32_CODE
#include <dlfcn.h>
#endif

extern CNetConnector *pNetDB;

CAndroidUserMgr::CAndroidUserMgr()
	: m_pGameKindInfo(NULL)
	, m_pRoomKindInfo(NULL)
	, m_pGameServerThread(NULL)
	, m_dwActiveAndroidCount(0)
	, m_pFunNewAndroidSink(NULL)
{

}

CAndroidUserMgr::~CAndroidUserMgr()
{
	for (auto it : m_AndroidUserItemFree)
	{
		delete it;
	}

	for (auto it : m_AndroidUserItemSitting)
	{
		delete it;
	}

	m_AndroidUserItemFree.clear();
	m_AndroidUserItemSitting.clear();

	m_AndroidUserMap.clear();
}

void CAndroidUserMgr::Initialization(tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, CGameServerThread* pGameserverThread)
{
	m_pGameKindInfo = pGameKind;
	m_pRoomKindInfo = pRoomKind;
	m_pGameServerThread = pGameserverThread;

	if (m_listAndroidParamStore.empty())
	{
		return;
	}

	//开始加载机器人模块
	std::string strDllPath = Utility::GetCurrentDirectory();
	strDllPath.append("/");
	std::string strDllName(pGameKind->szServerDll);
	strDllName.append("_score_android");

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
	strDllName.insert(0, "lib");
	strDllName.append(".dylib");
#endif

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
		m_pFunNewAndroidSink = (LpDllNewAndroid)GetProcAddress(hDll, DLL_NEW_ANDROID_FN_NAME);
		if (m_pFunNewAndroidSink == 0)
		{
			CT_CHAR buf[BUFSIZ] = { 0 };
			_snprintf_info(buf, BUFSIZ, "Can't find function: %s : errno: %d", DLL_NEW_ANDROID_FN_NAME, GetLastError());
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
		m_pFunNewAndroidSink = (LpDllNewAndroid)dlsym(dp, DLL_NEW_ANDROID_FN_NAME);
		if (m_pFunNewAndroidSink == 0)
		{
			err = dlerror();
			CT_CHAR buf[BUFSIZ] = { 0 };
			_snprintf_info(buf, BUFSIZ, "Can't find function: %s : errno: %s", DLL_NEW_ANDROID_FN_NAME, err);
			LOG(ERROR) << buf;
			exit(0);
			return;
		}
	}
#endif
}

void CAndroidUserMgr::OnTimePulse()
{
	for (auto& it : m_AndroidUserItemSitting)
	{
		it->OnTimePulse();
	}
}

void CAndroidUserMgr::OnTimeCheckUserIn()
{
	// 如果机器人没有库存了
	if (m_listAndroidParamStore.empty())
	{
		return;
	}

	//如果是斗地主机器人，则应该是分配等待
	if (m_pGameKindInfo->wGameID == GAME_DDZ)
	{
		CT_DWORD dwWaitlistCount = m_pGameServerThread->GetWaitListCount();
		CT_DWORD dwStartCount = m_pRoomKindInfo->wStartMaxPlayer;
		if (dwWaitlistCount > 0 && dwWaitlistCount < dwStartCount)
		{
			CT_DWORD dwNeedCount = dwStartCount - dwWaitlistCount;

			for (CT_DWORD i = 0; i < dwNeedCount; i++)
			{
				if (m_listAndroidParamStore.empty())
				{
					return;
				}

				std::shared_ptr<tagAndroidUserParameter>& ptrAndroidParam = m_listAndroidParamStore.front();
				auto itBaseData = m_mapAndroidUserData.find(ptrAndroidParam->dwUserID);
				if (itBaseData != m_mapAndroidUserData.end())
				{
					//检测分数是否符合标准
					if (itBaseData->second.llScore < m_pRoomKindInfo->dwEnterMinScore)
					{
						itBaseData->second.llScore += m_pRoomKindInfo->dwEnterMinScore;
					}
					if (itBaseData->second.llScore > m_pRoomKindInfo->dwEnterMaxScore)
					{
						if (m_pRoomKindInfo->dwEnterMaxScore != 0)
						{
							itBaseData->second.llScore = m_pRoomKindInfo->dwEnterMinScore + (rand() % m_pRoomKindInfo->dwEnterMaxScore);
						}
						else
						{
							CT_LONGLONG llTakeScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
							itBaseData->second.llScore = llTakeScore;
						}
					}
					SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.llScore);
					itBaseData->second.cbHeadIndex = ptrAndroidParam->cbHeadId;
				}
				else
				{
					GS_UserBaseData baseData;
					memset(&baseData, 0, sizeof(baseData));

					baseData.dwUserID = ptrAndroidParam->dwUserID;
					baseData.dwGem = ptrAndroidParam->dwGem;
					baseData.cbGender = ptrAndroidParam->cbSex;
					baseData.cbHeadIndex = ptrAndroidParam->cbHeadId;
					_snprintf_info(baseData.szNickName, sizeof(baseData.szNickName), "%s", ptrAndroidParam->szNickName);
					baseData.llScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));

					SetAndroidUserScore(ptrAndroidParam->dwUserID, baseData.llScore);
					m_mapAndroidUserData.insert(std::make_pair(baseData.dwUserID, baseData));
				}

				//进入等待列表
				MSG_PG_EnterRoom enterRoom;
				enterRoom.dwUserID = ptrAndroidParam->dwUserID;
				enterRoom.uValue1 = 0;
				m_pGameServerThread->OnAddUserWaitList(NULL, &enterRoom);

				m_mapAndroidParamUsed.insert(std::make_pair(ptrAndroidParam->dwUserID, ptrAndroidParam));
				//到DB更新机器人状态
				UpdateAndroidStatus(ptrAndroidParam->dwUserID, 1);
				m_listAndroidParamStore.pop_front();
			}
		}
		return;
	}

	//其他正常入桌的游戏，则分配入桌
	const ListTableFrame&  vecTableFrame = CGameTableManager::get_instance().GetUsedTableFrame();
	CT_WORD wAndroidCount = 1 + rand() % 2; //随机每桌机器人数

	for (auto& it :vecTableFrame)
	{
		CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(it);
		if (pTableFrame && pTableFrame->GetGameRoundPhase() == en_GameRound_Free)
		{
			if (m_pGameKindInfo->wGameID == GAME_BR)
			{
				wAndroidCount = m_dwActiveAndroidCount;
			}

			CT_WORD wMaxPlayerCount = pTableFrame->GetMaxPlayerCount();
			CT_WORD wRealPlayerCount = pTableFrame->GetRealPlayerCount();
			CT_WORD wPlayerCount = pTableFrame->GetPlayerCount();

			if ((wRealPlayerCount > 0 && wPlayerCount < wMaxPlayerCount) || m_pGameKindInfo->wGameID == GAME_BR)
			{
				CT_WORD wNeedCount = wMaxPlayerCount - wPlayerCount;
				for (int i = 0; i < wNeedCount; ++i)
				{
					if (pTableFrame->GetAndroidPlayerCount() >= wAndroidCount)
					{
						break;
					}
					CServerUserItem *pUserItem = NULL;
					if (m_listAndroidParamStore.empty())
					{
						return;
					}
					std::shared_ptr<tagAndroidUserParameter>& ptrAndroidParam = m_listAndroidParamStore.front();
					CreateAndroidUserItem(&pUserItem, ptrAndroidParam->dwUserID);
					if (pUserItem == NULL)
					{
						continue;
					}

					auto itBaseData = m_mapAndroidUserData.find(ptrAndroidParam->dwUserID);
					if (itBaseData != m_mapAndroidUserData.end())
					{
						//检测分数是否符合标准
						if (itBaseData->second.llScore < m_pRoomKindInfo->dwEnterMinScore)
						{
							itBaseData->second.llScore += m_pRoomKindInfo->dwEnterMinScore;
						}
						if (itBaseData->second.llScore > m_pRoomKindInfo->dwEnterMaxScore)
						{
							if (m_pRoomKindInfo->dwEnterMaxScore != 0)
							{
								itBaseData->second.llScore = m_pRoomKindInfo->dwEnterMinScore + (rand() % m_pRoomKindInfo->dwEnterMaxScore);
							}
							else
							{
								CT_LONGLONG llTakeScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
								itBaseData->second.llScore = llTakeScore;
							}

							//SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.llScore);
						}
						SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.llScore);
						itBaseData->second.cbHeadIndex = ptrAndroidParam->cbHeadId;
						pUserItem->SetUserBaseData(itBaseData->second);
					}
					else
					{
						GS_UserBaseData baseData;
						memset(&baseData, 0, sizeof(baseData));

						baseData.dwUserID = ptrAndroidParam->dwUserID;
						baseData.dwGem = ptrAndroidParam->dwGem;
						baseData.cbGender = ptrAndroidParam->cbSex;
						baseData.cbHeadIndex = ptrAndroidParam->cbHeadId;
						_snprintf_info(baseData.szNickName, sizeof(baseData.szNickName), "%s", ptrAndroidParam->szNickName);
						baseData.llScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));

						SetAndroidUserScore(ptrAndroidParam->dwUserID, baseData.llScore);
						pUserItem->SetUserBaseData(baseData);

						m_mapAndroidUserData.insert(std::make_pair(baseData.dwUserID, baseData));
					}
					//GS_UserBaseData& baseData = pUserItem->GetUserBaseData();

					GS_UserScoreData userScoreData;
					userScoreData.dwUserID = ptrAndroidParam->dwUserID;
					userScoreData.dwEnterTime = Utility::GetTime();
					pUserItem->SetUserScoreData(userScoreData);

					pUserItem->SetClientNetAddr(0);
					pUserItem->SetUserCon(NULL);
					pTableFrame->RoomSitChair(pUserItem);
					IAndroidUserItemSink* pSink = pUserItem->GetAndroidUserItemSink();
					if (pSink)
					{
						pSink->Initialization(pTableFrame, pUserItem->GetChairID(), pUserItem);
					}

					m_mapAndroidParamUsed.insert(std::make_pair(ptrAndroidParam->dwUserID, ptrAndroidParam));
					//到DB更新机器人状态
					UpdateAndroidStatus(ptrAndroidParam->dwUserID, 1);
					m_listAndroidParamStore.pop_front();
				}
			}
		}
	}
}

void CAndroidUserMgr::InsertAndroidUserParam(tagAndroidUserParameter* pAndroidParam)
{
	std::shared_ptr<tagAndroidUserParameter> ptrAndroidParam = std::make_shared<tagAndroidUserParameter>();
	ptrAndroidParam->dwUserID = pAndroidParam->dwUserID;
	ptrAndroidParam->dwEnterTime = pAndroidParam->dwEnterTime;
	ptrAndroidParam->dwLeaveTime = pAndroidParam->dwLeaveTime;
	ptrAndroidParam->llTakeMinScore = pAndroidParam->llTakeMinScore;
	ptrAndroidParam->llTakeMaxScore = pAndroidParam->llTakeMaxScore;
	ptrAndroidParam->dwGem = pAndroidParam->dwGem;
	ptrAndroidParam->cbSex = pAndroidParam->cbSex;
	ptrAndroidParam->cbHeadId = pAndroidParam->cbHeadId;
	_snprintf_info(ptrAndroidParam->szNickName, sizeof(ptrAndroidParam->szNickName), "%s", pAndroidParam->szNickName);

	m_listAndroidParamStore.push_back(ptrAndroidParam);
	m_setAndroidParamAll.insert(ptrAndroidParam);
}

void CAndroidUserMgr::ClearAndroidUserParam()
{
	m_listAndroidParamStore.clear();
	m_setAndroidParamAll.clear();
}

CT_BOOL CAndroidUserMgr::CreateAndroidUserItem(CServerUserItem** pIServerUserResult, CT_DWORD dwUserID)
{
	CServerUserItem* pServerUserItem = NULL;
	if (m_AndroidUserItemFree.size() > 0)
	{
		pServerUserItem = m_AndroidUserItemFree.back();
		m_AndroidUserItemFree.pop_back();
	}
	else
	{
		try
		{
			CAndroidUserItem* pAndroidUserItem = new CAndroidUserItem;
			pServerUserItem = pAndroidUserItem;
			IAndroidUserItemSink* pSink = m_pFunNewAndroidSink();
			pAndroidUserItem->SetAndroidUserItemSink(pSink);
		}
		catch (...)
		{
			assert(false);
			return false;
		}
	}

	m_AndroidUserItemSitting.push_back(pServerUserItem);
	m_AndroidUserMap[dwUserID] = pServerUserItem;
	*pIServerUserResult = pServerUserItem;

	return false;
}

CT_BOOL	CAndroidUserMgr::DeleteUserItem(CServerUserItem* pServerUserItem)
{
	if (pServerUserItem == NULL)
	{
		return false;
	}

	CServerUserItem* pTempUserItem = NULL;
	for (auto it = m_AndroidUserItemSitting.begin(); it != m_AndroidUserItemSitting.end(); ++it)
	{
		pTempUserItem = *it;
		if (pTempUserItem != pServerUserItem)
		{
			continue;
		}
		CT_DWORD dwUserID = pTempUserItem->GetUserID();
		//把机器人的分数
		GS_UserBaseData& userBaseData = pTempUserItem->GetUserBaseData();
		userBaseData.llScore = pTempUserItem->GetUserScore();
		m_mapAndroidUserData[dwUserID] = userBaseData;
		auto itParam = m_mapAndroidParamUsed.find(dwUserID);
		if (itParam != m_mapAndroidParamUsed.end())
		{
			//查找机器人是否已经被下架,如果没有被下架则还原到未用列表里
			auto itAndroid = m_setAndroidParamAll.find(itParam->second);
			if (itAndroid != m_setAndroidParamAll.end())
			{
				m_listAndroidParamStore.push_back(itParam->second);
			}
			else
			{
				m_mapAndroidUserData.erase(dwUserID);
			}
			m_mapAndroidParamUsed.erase(itParam);
		}	
		//到DB更新机器人状态
		UpdateAndroidStatus(dwUserID, 0);
		
		m_AndroidUserItemSitting.erase(it);
		m_AndroidUserMap.erase(dwUserID);
		pTempUserItem->ResetUserItem();
		IAndroidUserItemSink* pSink = pTempUserItem->GetAndroidUserItemSink();
		if (pSink)
		{
			pSink->RepositionSink();
		}
		m_AndroidUserItemFree.push_back(pTempUserItem);
		return true;
	}

	return false;
}

CServerUserItem* CAndroidUserMgr::FindAndroidUserItem(CT_DWORD dwUserID)
{
	auto it = m_AndroidUserMap.find(dwUserID);
	if (it != m_AndroidUserMap.end())
	{
		return it->second;
	}

	return NULL;
}

CT_VOID CAndroidUserMgr::SetAndroidUserScore(CT_DWORD dwUserID, CT_LONGLONG llScore)
{
	MSG_UpdateUser_Score updateScoreToDB;
	updateScoreToDB.dwUserID = dwUserID;
	updateScoreToDB.llAddScore = llScore;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_SET_USER_SCORE, &updateScoreToDB, sizeof(updateScoreToDB));
}

CT_VOID CAndroidUserMgr::SetActiveAndroidCount(CT_DWORD dwActiveAndroidCount)
{
	m_dwActiveAndroidCount = dwActiveAndroidCount;
	LOG(INFO) << "set active android count: " << dwActiveAndroidCount;
}

//获取机器用户数量
CT_DWORD CAndroidUserMgr::GetActiveAndroidCount()
{
	return m_dwActiveAndroidCount;
}

CT_VOID CAndroidUserMgr::UpdateAndroidStatus(CT_DWORD dwUserID, CT_BYTE cbStatus)
{
	MSG_G2DB_Android_Status androidStatus;
	androidStatus.dwAndroidUserID = dwUserID;
	androidStatus.cbStatus = cbStatus;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_ANDROID_STATUE, &androidStatus, sizeof(androidStatus));
}

GS_UserBaseData* CAndroidUserMgr::GetAndroidUserBaseData(CT_DWORD dwUserID)
{
	auto it = m_mapAndroidUserData.find(dwUserID);
	if (it != m_mapAndroidUserData.end())
	{
		return &it->second;
	}

	return NULL;
}