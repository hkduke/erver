#include "ServerUserItem.h"
#include "NetModule.h"
#include "CMD_Inner.h"
#include "ServerCfg.h"
#include "GameServerThread.h"
#include "AndroidUserMgr.h"

extern CNetConnector *pNetCenter;
extern CNetConnector *pNetDB;
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

	m_pSocket = NULL;		
	m_pDBSocket = NULL;
	m_uClientNetAddr = 0;

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

void CServerUserItem::GetUserBaseMsgDataEx2(MSG_SC_UserBaseInfo_ForScoreEx& basedata)
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
		CNetModule::getSingleton().Send(m_pSocket, mainID, subID);
	}
}

void CServerUserItem::SendDBMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len)
{
	if (m_pDBSocket != NULL)
	{
		CNetModule::getSingleton().Send(m_pSocket, mainID, subID, pBuf, len);
	}
}

CServerUserManager::CServerUserManager()
	:m_pGameRoomKindInfo(NULL)
	,m_pRedis(NULL)
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

CT_VOID CServerUserManager::Init(tagGameRoomKind* pGameRoomKind, acl::redis* pRedis)
{
	if (pGameRoomKind != NULL)
	{
		m_pGameRoomKindInfo = pGameRoomKind;
	}

	if (pRedis != NULL)
	{
		m_pRedis = pRedis;
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

		//记录玩家金币增量数据
		//UpdateUserScoreData(pServerUserItem);
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
		CT_WORD wAndroidCount = CAndroidUserMgr::get_instance().GetOnlineAndroidCount();
		updateUser.wRealUserCount = (CT_WORD)m_UserMap.size();
		updateUser.wUserCount = updateUser.wRealUserCount + wAndroidCount;

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

	std::vector<CT_DWORD> vecOffLineUser;

	//先把本proxy的玩家筛选出来
	for (auto it = m_UserMap.begin(); it != m_UserMap.end(); ++it)
	{
		acl::aio_socket_stream* pProxySock = it->second->GetUserCon();
		if (pProxySock == pSocket)
		{
			//pServerThread->OnUserOffLine(it->first);
			vecOffLineUser.push_back(it->first);
		}
	}

	//把节点上的所有玩家改成离线状态
	for (auto& it :vecOffLineUser)
	{
		pServerThread->OnUserOffLine(it);
	}
}

//更新玩家的好牌信息
CT_VOID CServerUserManager::UpdateGoodCardInfo(CServerUserItem* pUserItem)
{
	/*if (pUserItem == NULL)
	{
		return;
	}

	if (m_pGameRoomKindInfo->wGameID == GAME_ZJH)
	{
		//更新内存数据
		acl::string key;
		key.format("account_%u", pUserItem->GetUserID());

		acl::string strZjhCount;
		acl::string strZjhParam;
		strZjhCount.format("%u", pUserItem->GetZjhCount());
		strZjhParam.format("%d", pUserItem->GetZjhParam());

		std::map<acl::string, acl::string> mapGoodCard;
		mapGoodCard["zjhcount"] = strZjhCount;
		mapGoodCard["zjhparam"] =  strZjhParam;

		m_pRedis->clear();
		if (m_pRedis->hmset(key, mapGoodCard) == false)
		{
			LOG(WARNING) << "set user zjh good card info fail, user id: " << pUserItem->GetUserID();
		}

		MSG_G2DB_GoodCard_Info goodCardInfo;
		goodCardInfo.dwUserID = pUserItem->GetUserID();
		goodCardInfo.wGameID = m_pGameRoomKindInfo->wGameID;
		goodCardInfo.wKindID = m_pGameRoomKindInfo->wKindID;
		goodCardInfo.dwPlayCount = pUserItem->GetZjhCount();
		goodCardInfo.cbGoodCardParam = pUserItem->GetZjhParam();

		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_UPDATE_GOODCARD_INFO, &goodCardInfo, sizeof(MSG_G2DB_GoodCard_Info));
		//LOG(WARNING) << "send good card info to db.";
	}
	else if (m_pGameRoomKindInfo->wGameID == GAME_NN)
	{
		//更新内存数据
		acl::string key;
		key.format("account_%u", pUserItem->GetUserID());

		acl::string strQznnCount;
		acl::string strQznnParam;
		strQznnCount.format("%u", pUserItem->GetQznnCount());
		strQznnParam.format("%d", pUserItem->GetQznnParam());

		std::map<acl::string, acl::string> mapGoodCard;
		mapGoodCard["qznncount"] = strQznnCount;
		mapGoodCard["qznnparam"] = strQznnParam;

		m_pRedis->clear();
		if (m_pRedis->hmset(key, mapGoodCard) == false)
		{
			LOG(WARNING) << "set user qznn good card info fail, user id: " << pUserItem->GetUserID();
		}

        MSG_G2DB_GoodCard_Info goodCardInfo;
        goodCardInfo.dwUserID = pUserItem->GetUserID();
        goodCardInfo.wGameID = m_pGameRoomKindInfo->wGameID;
        goodCardInfo.wKindID = m_pGameRoomKindInfo->wKindID;
        goodCardInfo.dwPlayCount = pUserItem->GetQznnCount();
        goodCardInfo.cbGoodCardParam = pUserItem->GetQznnParam();

        CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_UPDATE_GOODCARD_INFO, &goodCardInfo, sizeof(MSG_G2DB_GoodCard_Info));
	}*/
}

CT_VOID CServerUserManager::UpdateUserScoreData(CServerUserItem* pUserItem)
{
    if (pUserItem == NULL)
	{
		return;
	}

    GS_UserScoreData& userScoreData = pUserItem->GetUserScoreData();
    if (userScoreData.dwWinCount == 0 && userScoreData.dwLostCount == 0)
        return;

    MSG_G2DB_ScoreData scoreData;
    scoreData.wGameID = m_pGameRoomKindInfo->wGameID;
    scoreData.wKindID = m_pGameRoomKindInfo->wKindID;
    scoreData.wRoomKindID = m_pGameRoomKindInfo->wRoomKindID;
    scoreData.dwUserID = pUserItem->GetUserID();
    scoreData.llWinScore = userScoreData.llWinScore;
    scoreData.llLostScore = userScoreData.llLostScore;
    scoreData.dwWinCount = userScoreData.dwWinCount;
    scoreData.dwLostCount = userScoreData.dwLostCount;
    scoreData.dwRevenue = userScoreData.dwRevenue;

    CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_GDB_MAIN, SUB_G2DB_UPDATE_USER_SCORE_DATA, &scoreData, sizeof(MSG_G2DB_ScoreData));
}
