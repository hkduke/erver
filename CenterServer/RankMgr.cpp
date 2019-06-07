#include "stdafx.h"
#include "UserMgr.h"
#include "ServerMgr.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "NetModule.h"
#include "RankMgr.h"
#include "UserTask.h"

extern CNetConnector *pNetDB;

CRankMgr::CRankMgr()
{
	m_pRedis = NULL;
}

CRankMgr::~CRankMgr()
{

}

CT_VOID	CRankMgr::SetRedisPtr(acl::redis* pRedis)
{
	m_pRedis = pRedis;
}

/*CT_VOID	CRankMgr::ClearScoreRank()
{
	m_vecScoreRank.clear();
}

CT_VOID CRankMgr::InsertScoreRank(tagScoreRank* pScoreRank)
{
	if (m_vecScoreRank.size() > SCORE_RANK_COUNT)
	{
		return;
	}

	//检测排行榜任务
	CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pScoreRank->dwUserID);
	if (pUserTask)
	{
		pUserTask->OnUserInRank();
	}

	m_vecScoreRank.push_back(*pScoreRank);
}*/


CT_VOID	CRankMgr::ClearYesterdayScoreRank()
{
	m_vecYesterdayScoreRank.clear();
}

CT_VOID CRankMgr::InsertYesterdayScoreRank(tagScoreRank* pScoreRank)
{
	if (m_vecYesterdayScoreRank.size() > SCORE_RANK_COUNT)
	{
		return;
	}

	//检测排行榜任务
	//CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pScoreRank->dwUserID);
	//if (pUserTask)
	//{
	//	pUserTask->OnUserInRank();
	//}

	m_vecYesterdayScoreRank.push_back(*pScoreRank);
}

/*CT_VOID CRankMgr::ClearShareRank()
{
	m_vecShareRank.clear();
	m_mapShareRank.clear();
}

CT_VOID CRankMgr::InsertShareRank(tagShareRank* pShareRank)
{

	m_vecShareRank.push_back(*pShareRank);
	tagShareRank* pVecShareRank = &m_vecShareRank.back();
	m_mapShareRank.insert(std::make_pair(pShareRank->dwUserID, pVecShareRank));
}

CT_VOID CRankMgr::ClearSignInRank()
{
	m_vecSignInRank.clear();
}

CT_VOID CRankMgr::InsertSignInRank(tagSignInRank* pSigninRank)
{
	if (m_vecSignInRank.size() > SCORE_RANK_COUNT)
	{
		return;
	}

	//检测排行榜任务
	CUserTask* pUserTask = CUserMgr::get_instance().GetUserTaskPtr(pSigninRank->dwUserID);
	if (pUserTask)
	{
		pUserTask->OnUserInRank();
	}

	m_vecSignInRank.push_back(*pSigninRank);
}*/

CT_LONGLONG CRankMgr::GetUserEarnScore(CT_DWORD dwUserID)
{
	acl::string strEarnScore;
	acl::string earnKey;
	earnKey.format("earnscore_%u", dwUserID);

	m_pRedis->clear();
	if (m_pRedis->get(earnKey.c_str(), strEarnScore) == false)
	{
		LOG(WARNING) << "get earn score info fail, user id: " << dwUserID;
		return 0;
	}

	return atoll(strEarnScore.c_str());
}

CT_DWORD CRankMgr::GetUeserTodayOnlineTime(CT_DWORD dwUserID)
{
    acl::string strOnlineTime;
    acl::string accountKey;
    accountKey.format("account_%u", dwUserID);

	m_pRedis->clear();
    if (m_pRedis->hget(accountKey.c_str(), "online", strOnlineTime) == false)
    {
        LOG(WARNING) << "get earn score info fail, user id: " << dwUserID;
        return 0;
    }

    return atoi(strOnlineTime.c_str());
}

CT_VOID  CRankMgr::GetUserTaskFinishInfo(CT_DWORD dwUserID, CT_WORD& wFinishCount, CT_DWORD& dwRewardScore)
{
    acl::string accountKey;
	accountKey.format("account_%u", dwUserID);

    std::vector<acl::string> vecAttr;
    vecAttr.push_back("taskfinish");
    vecAttr.push_back("taskscore");
    std::vector<acl::string> vecResult;

	m_pRedis->clear();
    if (m_pRedis->hmget(accountKey, vecAttr,  &vecResult) == false)
    {
        LOG(WARNING) << "get user today task finish info fail,userid  " << dwUserID;
        return;
    }

    if (vecResult.size() != 2)
	{
		return;
	}

	wFinishCount = atoi(vecResult[0].c_str());
    dwRewardScore = atoi(vecResult[1].c_str());
}

/*tagShareRank* CRankMgr::GetUserShareRank(CT_DWORD dwUserID)
{
	auto it = m_mapShareRank.find(dwUserID);
	if (it != m_mapShareRank.end())
	{
		return it->second;
	}

	return NULL;
}*/

CT_VOID CRankMgr::ClearTodayOnlineRank()
{
	m_vecTodayOnlineRank.clear();
}

CT_VOID CRankMgr::InsertTodayOnlineRank(tagTodayOnlineRank* pOnlineRank)
{
	m_vecTodayOnlineRank.push_back(*pOnlineRank);
}

CT_VOID CRankMgr::ClearTodayTaskRank()
{
	m_vecTodayTaskRank.clear();
}

CT_VOID CRankMgr::InsertTodayTaskRank(tagTodayTaskFinishRank* pTaskRank)
{
	m_vecTodayTaskRank.push_back(*pTaskRank);
}


CT_VOID CRankMgr::SendScoreRankToClient(CT_DWORD dwUserID)
{
	SendYesterScoreRank(dwUserID);
	//SendTodayOnlineRank(dwUserID);
	//SendTodayTaskRank(dwUserID);
}

CT_VOID CRankMgr::SendYesterScoreRank(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_CLIENT] = { 0 };
	CT_DWORD dwSendSize = 0;
	MSG_GameMsgDownHead msgHead;

	//发送昨天赚金榜
	memset(&msgHead, 0, sizeof(msgHead));
	dwSendSize = 0;
	msgHead.dwMainID = MSG_RANK_MAIN;
	msgHead.dwSubID = SUB_SC_TODAY_EARN_SCORE_RANK;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	//先查询本人的数据
	MSG_SC_Score_Rank scoreRankMyself;
	scoreRankMyself.cbRank = 0;
	scoreRankMyself.dwUserID = pUserInfo->dwUserID;
	scoreRankMyself.cbVip2 = pUserInfo->cbVip2;
	scoreRankMyself.cbGender = pUserInfo->cbSex;
	scoreRankMyself.cbHeadID = pUserInfo->cbHeadID;
	scoreRankMyself.dScore = GetUserEarnScore(dwUserID)*TO_DOUBLE;
	_snprintf_info(scoreRankMyself.szNickName, sizeof(scoreRankMyself.szNickName), "%s", pUserInfo->szNickName);

	//先增加长度，不拷贝数据
	dwSendSize += sizeof(MSG_SC_Score_Rank);

	MSG_SC_Score_Rank scoreRank;
	for (auto& it : m_vecYesterdayScoreRank)
	{
		//发送数据
		if ((dwSendSize + sizeof(MSG_SC_Score_Rank)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			msgHead.dwMainID = MSG_RANK_MAIN;
			msgHead.dwSubID = SUB_SC_TODAY_EARN_SCORE_RANK;
			msgHead.dwValue2 = dwUserID;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}

		memset(&scoreRank, 0, sizeof(scoreRank));
		scoreRank.cbRank = it.cbRank;
		scoreRank.dwUserID = it.dwUserID;
		scoreRank.dScore = it.dScore;
		scoreRank.cbVip2 = it.cbVip2;
		scoreRank.cbGender = it.cbGender;
		scoreRank.cbHeadID = it.cbHeadID;
		_snprintf_info(scoreRank.szNickName, sizeof(scoreRank.szNickName), "%s", it.szNickName);
		if (it.dwUserID == dwUserID)
		{
			scoreRankMyself.cbRank = it.cbRank;
		}

		memcpy(szBuffer + dwSendSize, &scoreRank, sizeof(MSG_SC_Score_Rank));
		dwSendSize += sizeof(MSG_SC_Score_Rank);
	}
	//拷贝自己的数据
	memcpy(szBuffer + sizeof(MSG_GameMsgDownHead), &scoreRankMyself, sizeof(MSG_SC_Score_Rank));

	if (dwSendSize > sizeof(MSG_GameMsgDownHead))
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}

CT_VOID CRankMgr::SendTodayOnlineRank(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_CLIENT] = { 0 };
	CT_DWORD dwSendSize = 0;
	MSG_GameMsgDownHead msgHead;

	//发送昨天赚金榜
	memset(&msgHead, 0, sizeof(msgHead));
	dwSendSize = 0;
	msgHead.dwMainID = MSG_RANK_MAIN;
	msgHead.dwSubID = SUB_SC_TODAY_ONLINE_RANK;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	//先查询本人的数据
	MSG_SC_Online_Rank onlineRankMyself;
	onlineRankMyself.cbRank = 0;
	onlineRankMyself.dwUserID = pUserInfo->dwUserID;
	onlineRankMyself.cbGender = pUserInfo->cbSex;
	onlineRankMyself.cbHeadID = pUserInfo->cbHeadID;
	onlineRankMyself.cbVip2 = pUserInfo->cbVip2;
	onlineRankMyself.dwOnline = GetUeserTodayOnlineTime(dwUserID);
	_snprintf_info(onlineRankMyself.szNickName, sizeof(onlineRankMyself.szNickName), "%s", pUserInfo->szNickName);

	//先增加长度，不拷贝数据
	dwSendSize += sizeof(MSG_SC_Online_Rank);

	MSG_SC_Online_Rank onlineRank;
	for (auto& it : m_vecTodayOnlineRank)
	{
		//发送数据
		if ((dwSendSize + sizeof(MSG_SC_Online_Rank)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			msgHead.dwMainID = MSG_RANK_MAIN;
			msgHead.dwSubID = SUB_SC_TODAY_ONLINE_RANK;
			msgHead.dwValue2 = dwUserID;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}

		memset(&onlineRank, 0, sizeof(onlineRank));
		onlineRank.cbRank = it.cbRank;
		onlineRank.dwUserID = it.dwUserID;
		onlineRank.dwOnline = it.dwOnlineTime;
		onlineRank.cbVip2 = it.cbVip2;
		onlineRank.cbGender = it.cbGender;
		onlineRank.cbHeadID = it.cbHeadID;
		_snprintf_info(onlineRank.szNickName, sizeof(onlineRank.szNickName), "%s", it.szNickName);
		if (it.dwUserID == dwUserID)
		{
			onlineRankMyself.cbRank = it.cbRank;
		}

		memcpy(szBuffer + dwSendSize, &onlineRank, sizeof(MSG_SC_Online_Rank));
		dwSendSize += sizeof(MSG_SC_Online_Rank);
	}
	//拷贝自己的数据
	memcpy(szBuffer + sizeof(MSG_GameMsgDownHead), &onlineRankMyself, sizeof(MSG_SC_Online_Rank));

	if (dwSendSize > sizeof(MSG_GameMsgDownHead))
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}

CT_VOID CRankMgr::SendTodayTaskRank(CT_DWORD dwUserID)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_CLIENT] = { 0 };
	CT_DWORD dwSendSize = 0;
	MSG_GameMsgDownHead msgHead;

	//发送昨天赚金榜
	memset(&msgHead, 0, sizeof(msgHead));
	dwSendSize = 0;
	msgHead.dwMainID = MSG_RANK_MAIN;
	msgHead.dwSubID = SUB_SC_TODAY_TASK_RANK;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	//先查询本人的数据
	MSG_SC_Task_Rank taskRankMyself;
	taskRankMyself.cbRank = 0;
	taskRankMyself.dwUserID = pUserInfo->dwUserID;
	taskRankMyself.cbGender = pUserInfo->cbSex;
	taskRankMyself.cbHeadID = pUserInfo->cbHeadID;
	taskRankMyself.cbVip2 = pUserInfo->cbVip2;
	CT_WORD wTaskFinishCount = 0;
	CT_DWORD dwRewardScore = 0;
	GetUserTaskFinishInfo(dwUserID, wTaskFinishCount, dwRewardScore);
	taskRankMyself.wFinishCount = wTaskFinishCount;
	taskRankMyself.dRewardScore = dwRewardScore*TO_DOUBLE;
	_snprintf_info(taskRankMyself.szNickName, sizeof(taskRankMyself.szNickName), "%s", pUserInfo->szNickName);

	//先增加长度，不拷贝数据
	dwSendSize += sizeof(MSG_SC_Task_Rank);

	MSG_SC_Task_Rank taskRank;
	for (auto& it : m_vecTodayTaskRank)
	{
		//发送数据
		if ((dwSendSize + sizeof(MSG_SC_Task_Rank)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			msgHead.dwMainID = MSG_RANK_MAIN;
			msgHead.dwSubID = SUB_SC_TODAY_TASK_RANK;
			msgHead.dwValue2 = dwUserID;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}

		memset(&taskRank, 0, sizeof(taskRank));
		taskRank.cbRank = it.cbRank;
		taskRank.dwUserID = it.dwUserID;
		taskRank.wFinishCount = it.wFinishCount;
		taskRank.dRewardScore = it.dRewardScore;
		taskRank.cbVip2 = it.cbVip2;
		taskRank.cbGender = it.cbGender;
		taskRank.cbHeadID = it.cbHeadID;
		_snprintf_info(taskRank.szNickName, sizeof(taskRank.szNickName), "%s", it.szNickName);
		if (it.dwUserID == dwUserID)
		{
			taskRankMyself.cbRank = it.cbRank;
		}

		memcpy(szBuffer + dwSendSize, &taskRank, sizeof(MSG_SC_Task_Rank));
		dwSendSize += sizeof(MSG_SC_Task_Rank);
	}
	//拷贝自己的数据
	memcpy(szBuffer + sizeof(MSG_GameMsgDownHead), &taskRankMyself   , sizeof(MSG_SC_Task_Rank));

	if (dwSendSize > sizeof(MSG_GameMsgDownHead))
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}
}