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
	, m_dwAndroidTimePulse(0)
	, m_pDdzAiThread(NULL)
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

	/*if (m_listAndroidParamStore.empty())
	{
		return;
	}*/

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

	//判断有没有机器人模块
	bool bExist = Utility::IsFileExist(strDllName.c_str());
	if (!bExist)
	{
		return;
	}

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
	if (m_pGameKindInfo->wGameID == GAME_DDZ || m_pGameKindInfo->wGameID == GAME_PDK)
	{
		CheckDDZGameAndroidIn();
		return;
	}
	else if (m_pGameKindInfo->wGameID == GAME_JZNC)
    {
        CheckJZNCGameAndroidIn();
        return;
    }
	//其他正常入桌的游戏，则分配入桌
	if (m_pGameKindInfo->wGameID == GAME_FISH || m_pRoomKindInfo->wGameID == GAME_FXGZ)
	{
        CheckFishGameAndroidIn();
	}
	else if (m_pRoomKindInfo->wGameID == GAME_HBSL)
    {
        CheckHongBaoSLAndroidIn();
    }
	else
	{
        CheckNormalGameAndroidIn();
    }
}

//检测斗地主机器人进入房间
void CAndroidUserMgr::CheckDDZGameAndroidIn()
{
	if (m_dwActiveAndroidCount <= 0)
		return;

	CT_DWORD dwWaitlistCount = m_pGameServerThread->GetWaitListCount();
	CT_DWORD dwStartCount = m_pRoomKindInfo->wStartMaxPlayer;
	if (dwWaitlistCount > 0 && dwWaitlistCount < dwStartCount)
	{
		CT_DWORD dwNeedCount = dwStartCount - dwWaitlistCount;
		if (dwNeedCount > m_dwActiveAndroidCount)
			return;

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
				//SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.dwScore);
				//itBaseData->second.cbHeadIndex = ptrAndroidParam->cbHeadId;
			}
			else
			{
				GS_UserBaseData baseData;
				memset(&baseData, 0, sizeof(baseData));

				baseData.dwUserID = ptrAndroidParam->dwUserID;
				baseData.dwGem = ptrAndroidParam->dwGem;
				baseData.cbGender = ptrAndroidParam->cbSex;
				baseData.cbHeadIndex = ptrAndroidParam->cbHeadId;
				baseData.cbVipLevel2 = ptrAndroidParam->cbVip2;
				baseData.dwPlatformID = 1;
				_snprintf_info(baseData.szNickName, sizeof(baseData.szNickName), "%s", ptrAndroidParam->szNickName);
				baseData.llScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
				//baseData.dwQznnCount = 100;
				//baseData.dwZjhCount = 100;
				//baseData.cbQznnParam = 0;
				//baseData.cbZjhParam = 0;

				//SetAndroidUserScore(ptrAndroidParam->dwUserID, baseData.dwScore);
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
}

void CAndroidUserMgr::CheckJZNCGameAndroidIn()
{
    //m_pGameServerThread->DistributeTable();

    if (m_dwActiveAndroidCount <= 0)
        return;

    CT_DWORD dwWaitlistCount = m_pGameServerThread->GetWaitListCount();
    CT_DWORD dwStartCount = m_pRoomKindInfo->wStartMinPlayer;
    CT_DWORD dwMaxCount = m_pRoomKindInfo->wStartMaxPlayer;

    if (dwWaitlistCount > 0 && dwWaitlistCount < dwMaxCount)
    {
        CT_INT32 iMinCount = ((int)dwStartCount - (int)dwWaitlistCount);
        CT_DWORD dwNeedCount = ((iMinCount > 0) ? iMinCount:0);
//        if (dwNeedCount > m_dwActiveAndroidCount)
//            return;

        CT_DWORD dwMaxNeedCount  = dwMaxCount - dwWaitlistCount;
        if(dwMaxNeedCount > m_dwActiveAndroidCount)
        {
            dwMaxNeedCount = m_dwActiveAndroidCount;
        }
        if(dwNeedCount > m_dwActiveAndroidCount)
        {
            dwNeedCount = m_dwActiveAndroidCount;
        }
        //LOG(WARNING) << "need android count: " << dwNeedCount  << " dwMaxNeedCount:" << dwMaxNeedCount;
        if (dwMaxNeedCount > dwNeedCount)
        {
            dwNeedCount = (rand()% (dwMaxNeedCount-dwNeedCount+1))+dwNeedCount;
        }
        //LOG(WARNING) << "need android count: " << dwNeedCount  << " dwMaxNeedCount:" << dwMaxNeedCount;
        if(dwNeedCount < 2 && m_dwActiveAndroidCount > 2 && dwMaxNeedCount > 2)
        {
            dwNeedCount = (rand()% (dwMaxNeedCount-2+1))+2;
        }

        //LOG(WARNING) << "need android count: " << dwNeedCount  << " dwMaxNeedCount:" << dwMaxNeedCount;

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
                //SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.dwScore);
                //itBaseData->second.cbHeadIndex = ptrAndroidParam->cbHeadId;
            }
            else
            {
                GS_UserBaseData baseData;
                memset(&baseData, 0, sizeof(baseData));

                baseData.dwUserID = ptrAndroidParam->dwUserID;
                baseData.dwGem = ptrAndroidParam->dwGem;
                baseData.cbGender = ptrAndroidParam->cbSex;
                baseData.cbHeadIndex = ptrAndroidParam->cbHeadId;
                baseData.cbVipLevel2 = ptrAndroidParam->cbVip2;
                baseData.dwPlatformID = 1;
                _snprintf_info(baseData.szNickName, sizeof(baseData.szNickName), "%s", ptrAndroidParam->szNickName);
                baseData.llScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
                //baseData.dwQznnCount = 100;
                //baseData.dwZjhCount = 100;
                //baseData.cbQznnParam = 0;
                //baseData.cbZjhParam = 0;

                //SetAndroidUserScore(ptrAndroidParam->dwUserID, baseData.dwScore);
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
}

//检测普通游戏机器人进入房间
void CAndroidUserMgr::CheckNormalGameAndroidIn()
{
	const ListTableFrame&  vecTableFrame = CGameTableManager::get_instance().GetUsedTableFrame();
	CT_WORD wAndroidCount = m_dwActiveAndroidCount;//1 + rand() % 2; //随机每桌机器人数

	//牛牛游戏需要让长时间不入桌子的机器人离开桌子

	CT_DWORD dwNow = (CT_DWORD)time(NULL);
	for (auto& it :vecTableFrame)
	{
		CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(it);
		if (pTableFrame)
		{
			//百人游戏的机器人随时可以进入
			if (m_pGameKindInfo->wGameID != GAME_BR && pTableFrame->GetGameRoundPhase() != en_GameRound_Free)
			{
				continue;
			}

			//私人房不能进入机器人
			if (pTableFrame->GetTableisLock())
            {
                continue;
            }

			CT_WORD wMaxPlayerCount = pTableFrame->GetMaxPlayerCount();
			CT_WORD wRealPlayerCount = pTableFrame->GetRealPlayerCount();
			CT_WORD wPlayerCount = pTableFrame->GetPlayerCount();
			if (m_pGameKindInfo->wGameID == GAME_BR)
			{
				/*CT_DWORD dwAndroidLeaveTime = pTableFrame->GetAndroidLeaveTime();
				if (dwNow - dwAndroidLeaveTime < 30)
				{
					continue;
				}*/

				if (wRealPlayerCount >= 180)
				{
					wAndroidCount = 0;
				}
				else if (wRealPlayerCount >= 150)
				{
					wAndroidCount = m_dwActiveAndroidCount;
				}
				else
				{
					wAndroidCount = m_dwActiveAndroidCount + (m_dwActiveAndroidCount == 0 ? 0 : rand() % m_dwActiveAndroidCount);
				}
			}

			if (m_pGameKindInfo->wGameID == GAME_NN || m_pGameKindInfo->wGameID == GAME_ZJH)
			{
				//pTableFrame->LetAndroidLeaveNoRealPlayerTabel();
				if (wRealPlayerCount == 1 || wRealPlayerCount == 0)
				{
					wAndroidCount = m_dwActiveAndroidCount;
				}
				else if (wRealPlayerCount == 2)
				{
					wAndroidCount = 1;
				}
				else if (wRealPlayerCount == 3)
				{
					wAndroidCount = rand() % 2 ? 1 : 0;
				}
				else
				{
					wAndroidCount = 0;
				}

				if (wAndroidCount > m_dwActiveAndroidCount)
				{
					wAndroidCount = m_dwActiveAndroidCount;
				}

				CT_DWORD dwRandTime = 0;

				//现在的机器人个人数已经达到最大值，则不再添加机器人
				CT_WORD wNowAndroidCount = pTableFrame->GetAndroidPlayerCount();
				if (wNowAndroidCount >= wAndroidCount)
					continue;

				//如果原来有机器人则要更久一点才进来
				if (wNowAndroidCount > 0)
				{
					dwRandTime = rand() % 15 + 20;
				}
				else
				{
					dwRandTime = rand() % 15 + 3;
				}

				CT_DWORD dwAndroidLeaveTime = pTableFrame->GetAndroidLeaveTime();
				if (dwNow - dwAndroidLeaveTime < dwRandTime)
				{
					continue;
				}
			}

			if (m_pGameKindInfo->wGameID == GAME_NN || m_pGameKindInfo->wGameID == GAME_ZJH || m_pGameKindInfo->wGameID == GAME_BR || (wRealPlayerCount > 0 && wPlayerCount < wMaxPlayerCount) )
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
					CreateAndroidUserItem(&pUserItem, ptrAndroidParam->dwUserID, ptrAndroidParam->cbBigJetton);
					if (pUserItem == NULL)
					{
						continue;
					}

					auto itBaseData = m_mapAndroidUserData.find(ptrAndroidParam->dwUserID);
					if (itBaseData != m_mapAndroidUserData.end())
					{
						//检测分数是否符合标准
						if (m_pRoomKindInfo->wGameID == GAME_BR)
                        {
                            CT_LONGLONG llTakeScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
                            itBaseData->second.llScore = llTakeScore;
                        }
                        else
                        {
                            if (itBaseData->second.llScore < m_pRoomKindInfo->dwEnterMinScore)
                            {
                                //itBaseData->second.llScore += m_pRoomKindInfo->dwEnterMinScore;
                                //itBaseData->second.llScore = m_pRoomKindInfo->dwEnterMinScore + (rand() % m_pRoomKindInfo->dwEnterMaxScore);
								CT_LONGLONG llTakeScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
								itBaseData->second.llScore = llTakeScore;
                            }
                            else if (itBaseData->second.llScore > m_pRoomKindInfo->dwEnterMaxScore)
                            {
                                if (m_pRoomKindInfo->dwEnterMaxScore != 0)
                                {
                                    itBaseData->second.llScore = m_pRoomKindInfo->dwEnterMinScore + (rand() % m_pRoomKindInfo->dwEnterMaxScore);
                                }
                                //SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.dwScore);
                            }
                        }
						//SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.dwScore);
						//itBaseData->second.cbHeadIndex = ptrAndroidParam->cbHeadId;
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
						baseData.cbVipLevel2 = ptrAndroidParam->cbVip2;
						baseData.dwPlatformID = 1;
						_snprintf_info(baseData.szNickName, sizeof(baseData.szNickName), "%s", ptrAndroidParam->szNickName);
						baseData.llScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
						//baseData.dwQznnCount = 100;
						//baseData.dwZjhCount = 100;
						//baseData.cbQznnParam = 0;
						//baseData.cbZjhParam = 0;

						SetAndroidUserScore(ptrAndroidParam->dwUserID, baseData.llScore);
						pUserItem->SetUserBaseData(baseData);

						m_mapAndroidUserData.insert(std::make_pair(baseData.dwUserID, baseData));
					}

					GS_UserScoreData userScoreData;
					userScoreData.dwUserID = ptrAndroidParam->dwUserID;
					userScoreData.dwEnterTime = Utility::GetTime();
					pUserItem->SetUserScoreData(userScoreData);

					pUserItem->SetClientNetAddr(0);
					pUserItem->SetUserCon(NULL);
					pTableFrame->RoomSitChair(pUserItem);
					m_mapAndroidParamUsed.insert(std::make_pair(ptrAndroidParam->dwUserID, ptrAndroidParam));

					IAndroidUserItemSink* pSink = pUserItem->GetAndroidUserItemSink();
					if (pSink)
					{
						pSink->Initialization(pTableFrame, pUserItem->GetChairID(), pUserItem);
					}
					//到DB更新机器人状态
					UpdateAndroidStatus(ptrAndroidParam->dwUserID, 1);
					m_listAndroidParamStore.pop_front();
					//让每次都只进一个机器人
					break;
				}
			}
		}
	}
}
//检测捕鱼机器人进入房间
void CAndroidUserMgr::CheckFishGameAndroidIn()
{
	const VecTableFrame& vecTableFrame = CGameTableManager::get_instance().GetTableFrame();
	CT_WORD wAndroidCount = m_dwActiveAndroidCount;//1 + rand() % 2; //随机每桌机器人数
	for (auto& it :vecTableFrame)
	{
		CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(it);
		if (pTableFrame)
		{
		    if (pTableFrame->GetTableisLock())
                continue;

			CT_WORD wMaxPlayerCount = pTableFrame->GetMaxPlayerCount();
			CT_WORD wPlayerCount = pTableFrame->GetPlayerCount();

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
				CreateAndroidUserItem(&pUserItem, ptrAndroidParam->dwUserID, ptrAndroidParam->cbBigJetton);
				if (pUserItem == NULL)
				{
					continue;
				}

				auto itBaseData = m_mapAndroidUserData.find(ptrAndroidParam->dwUserID);
				if (itBaseData != m_mapAndroidUserData.end())
				{
					//检测分数是否符合标准
					/*
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
							if (m_pRoomKindInfo->wGameID == GAME_BR)
							{
								CT_LONGLONG llTakeScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
								itBaseData->second.llScore = llTakeScore;
							}
						}

						//SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.dwScore);
					}
					 */
					CT_LONGLONG llTakeScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
					itBaseData->second.llScore = llTakeScore;
					//SetAndroidUserScore(ptrAndroidParam->dwUserID, itBaseData->second.dwScore);
					//itBaseData->second.cbHeadIndex = ptrAndroidParam->cbHeadId;
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
					baseData.cbVipLevel2 = ptrAndroidParam->cbVip2;
					baseData.dwPlatformID = 1;
					_snprintf_info(baseData.szNickName, sizeof(baseData.szNickName), "%s", ptrAndroidParam->szNickName);
					baseData.llScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));

					SetAndroidUserScore(ptrAndroidParam->dwUserID, baseData.llScore);
					pUserItem->SetUserBaseData(baseData);

					m_mapAndroidUserData.insert(std::make_pair(baseData.dwUserID, baseData));
				}

				GS_UserScoreData userScoreData;
				userScoreData.dwUserID = ptrAndroidParam->dwUserID;
				userScoreData.dwEnterTime = Utility::GetTime();
				pUserItem->SetUserScoreData(userScoreData);

				pUserItem->SetClientNetAddr(0);
				pUserItem->SetUserCon(NULL);
				pTableFrame->RoomSitChair(pUserItem);
				m_mapAndroidParamUsed.insert(std::make_pair(ptrAndroidParam->dwUserID, ptrAndroidParam));

				IAndroidUserItemSink* pSink = pUserItem->GetAndroidUserItemSink();
				if (pSink)
				{
					pSink->Initialization(pTableFrame, pUserItem->GetChairID(), pUserItem);
				}
				//到DB更新机器人状态
				UpdateAndroidStatus(ptrAndroidParam->dwUserID, 1);
				m_listAndroidParamStore.pop_front();
				//让每次都只进一个机器人
				break;
			}
		}
	}
}

void CAndroidUserMgr::CheckHongBaoSLAndroidIn()
{
	const ListTableFrame&  vecTableFrame = CGameTableManager::get_instance().GetUsedTableFrame();
	CT_WORD wAndroidCount = m_dwActiveAndroidCount;

	//牛牛游戏需要让长时间不入桌子的机器人离开桌子
	CT_DWORD dwNow = (CT_DWORD)time(NULL);
    tm timeNow;
    getLocalTime(&timeNow, dwNow);
    for (auto& it :vecTableFrame)
	{
		CTableFrameForScore* pTableFrame = static_cast<CTableFrameForScore*>(it);
		if (pTableFrame)
		{
		    if (pTableFrame->GetTableisLock())
            {
                continue;
            }
		    /*
		     *  0点到6点：机器人数+玩家人数，5人到10人（每5分钟变换一次），每5到30秒随机自动进入1个机器人。
                6点到19点：机器人数+玩家人数，10人到15人（每5分钟变换一次），每5到30随机秒自动进入1个机器人。
                19点到0点：机器人数+玩家人数，15人到20人（每5分钟变换一次），每5到30秒随机自动进入1个机器人。
                ii.  机器人随机携带金币100~1500
		     */
			CT_WORD wMaxPlayerCount = pTableFrame->GetMaxPlayerCount();
			//CT_WORD wRealPlayerCount = pTableFrame->GetRealPlayerCount();
			CT_WORD wPlayerCount = pTableFrame->GetPlayerCount();

			if (timeNow.tm_hour < 6)
            {
			    if (wPlayerCount >= 10)
                {
                    wAndroidCount = 0;
                }
            }
			else if (timeNow.tm_hour < 19)
            {
			    if (wPlayerCount >= 15)
                {
			        wAndroidCount = 0;
                }
            }
            else
            {
                if (wPlayerCount >= 20)
                {
                    wAndroidCount = 0;
                }
            }

            if (wAndroidCount == 0)
                return;

			if (wPlayerCount < wMaxPlayerCount)
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
                    CreateAndroidUserItem(&pUserItem, ptrAndroidParam->dwUserID, ptrAndroidParam->cbBigJetton);
                    if (pUserItem == NULL)
                    {
                        continue;
                    }

                    auto itBaseData = m_mapAndroidUserData.find(ptrAndroidParam->dwUserID);
                    if (itBaseData != m_mapAndroidUserData.end())
                    {
                        //检测分数是否符合标准
                        CT_LONGLONG llTakeScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
                        itBaseData->second.llScore = llTakeScore;
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
                        baseData.cbVipLevel2 = ptrAndroidParam->cbVip2;
                        baseData.dwPlatformID = 1;
                        _snprintf_info(baseData.szNickName, sizeof(baseData.szNickName), "%s", ptrAndroidParam->szNickName);
                        baseData.llScore = ptrAndroidParam->llTakeMinScore + (rand() % (ptrAndroidParam->llTakeMaxScore - ptrAndroidParam->llTakeMinScore));
                        //baseData.dwQznnCount = 100;
                        //baseData.dwZjhCount = 100;
                        //baseData.cbQznnParam = 0;
                        //baseData.cbZjhParam = 0;

                        SetAndroidUserScore(ptrAndroidParam->dwUserID, baseData.llScore);
                        pUserItem->SetUserBaseData(baseData);

                        m_mapAndroidUserData.insert(std::make_pair(baseData.dwUserID, baseData));
                    }

                    GS_UserScoreData userScoreData;
                    userScoreData.dwUserID = ptrAndroidParam->dwUserID;
                    userScoreData.dwEnterTime = Utility::GetTime();
                    pUserItem->SetUserScoreData(userScoreData);

                    pUserItem->SetClientNetAddr(0);
                    pUserItem->SetUserCon(NULL);
                    pTableFrame->RoomSitChair(pUserItem);
                    m_mapAndroidParamUsed.insert(std::make_pair(ptrAndroidParam->dwUserID, ptrAndroidParam));

                    IAndroidUserItemSink* pSink = pUserItem->GetAndroidUserItemSink();
                    if (pSink)
                    {
                        pSink->Initialization(pTableFrame, pUserItem->GetChairID(), pUserItem);
                    }
                    //到DB更新机器人状态
                    UpdateAndroidStatus(ptrAndroidParam->dwUserID, 1);
                    m_listAndroidParamStore.pop_front();
                    //让每次都只进一个机器人
                    break;
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
	ptrAndroidParam->cbVip2 = pAndroidParam->cbVip2;
	ptrAndroidParam->cbBigJetton = pAndroidParam->cbBigJetton;
	_snprintf_info(ptrAndroidParam->szNickName, sizeof(ptrAndroidParam->szNickName), "%s", pAndroidParam->szNickName);

	m_listAndroidParamStore.push_back(ptrAndroidParam);
	m_setAndroidParamAll.insert(ptrAndroidParam);
}

void CAndroidUserMgr::ClearAndroidUserParam()
{
	m_listAndroidParamStore.clear();
	m_setAndroidParamAll.clear();
}

CT_BOOL CAndroidUserMgr::CreateAndroidUserItem(CServerUserItem** pIServerUserResult, CT_DWORD dwUserID, CT_BYTE cbBigJetton)
{
	CServerUserItem* pServerUserItem = NULL;
	if (m_AndroidUserItemFree.size() > 0)
	{
		pServerUserItem = m_AndroidUserItemFree.front();
		m_AndroidUserItemFree.pop_front();
	}
	else
	{
		try
		{
			CAndroidUserItem* pAndroidUserItem = new CAndroidUserItem;
			pServerUserItem = pAndroidUserItem;
			pAndroidUserItem->SetBigJettionAndroid(cbBigJetton);
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

CT_VOID CAndroidUserMgr::UpdateOnlineCount()
{
	CServerUserManager::get_instance().UpdateUserCount(0, 3);
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

	//更新在线人数
	UpdateOnlineCount();
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