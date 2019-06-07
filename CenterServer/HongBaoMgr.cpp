#include "stdafx.h"
#include "UserMgr.h"
#include "ServerMgr.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "NetModule.h"
#include "HongBaoMgr.h"

extern CNetConnector *pNetDB;
#define ONEDAY_SECOND 86400

CHongBaoMgr::CHongBaoMgr()
	:m_dwHongBaoCount(0)
{

}

CHongBaoMgr::~CHongBaoMgr()
{

}

CT_VOID CHongBaoMgr::InsertHongBaoInfo(tagHongBaoInfo* pHongBaoInfo, CT_BOOL bUpdateToClient)
{
	auto it = m_mapHongbaoInfo.find(pHongBaoInfo->dwID);
	if (it != m_mapHongbaoInfo.end())
	{
		LOG(WARNING) << "insert hongbao repeat? hongbao id: " << pHongBaoInfo->dwID;
		return;
	}

	m_mapHongbaoInfo.insert(std::make_pair(pHongBaoInfo->dwID, *pHongBaoInfo));
	++m_dwHongBaoCount;

	if (bUpdateToClient)
	{
		SendHongBaoCountToClient(0);
	}
}

CT_VOID CHongBaoMgr::SendHongBao(MSG_CS_Send_HongBao* pSendHongBao)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pSendHongBao->dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pSendHongBao->dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	//红包金币不足
	if (pUserInfo->llScore < pSendHongBao->dwScore)
	{
		MSG_SC_Send_HongBao sendResult;
		sendResult.dwMainID = MSG_HONGBAO_MAIN;
		sendResult.dwSubID = SUB_SC_SEND_HONGBAO;
		sendResult.dwValue2 = pSendHongBao->dwUserID;
		sendResult.cbResult = 1;

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &sendResult, sizeof(sendResult));
		return;
	}

	//在游戏中不能发红包
	if (pUserInfo->dwGameServerID != 0)
	{
		MSG_SC_Send_HongBao sendResult;
		sendResult.dwMainID = MSG_HONGBAO_MAIN;
		sendResult.dwSubID = SUB_SC_SEND_HONGBAO;
		sendResult.dwValue2 = pSendHongBao->dwUserID;
		sendResult.cbResult = 2;

		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &sendResult, sizeof(sendResult));
		return;
	}

	//更新玩家金币
	CT_LONGLONG llAddScore = 0;
	llAddScore -= (CT_LONGLONG)pSendHongBao->dwScore;
	CUserMgr::get_instance().AddUserScore(pSendHongBao->dwUserID, llAddScore);

	//到数据库中插入红包数据
	MSG_CS2DB_Insert_HongBao insertHongBao;
	insertHongBao.dwUserID = pSendHongBao->dwUserID;
	insertHongBao.dwHongBaoScore = pSendHongBao->dwScore;
	insertHongBao.dwSendTime = Utility::GetTime();
	_snprintf_info(insertHongBao.szHongBaoName, sizeof(insertHongBao.szHongBaoName), "%s", pSendHongBao->szName);
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_INSERT_HONGBAO, &insertHongBao, sizeof(insertHongBao));
}

CT_VOID CHongBaoMgr::GuessHongBao(MSG_CS_Guess_HongBao* pTakeHongBao)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pTakeHongBao->dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pTakeHongBao->dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	MSG_SC_Guess_HongBao takeHongBao;
	takeHongBao.dwMainID = MSG_HONGBAO_MAIN;
	takeHongBao.dwSubID = SUB_SC_GUESS_HONGBAO;
	takeHongBao.dwValue2 = pTakeHongBao->dwUserID;
	takeHongBao.dwHongbaoID = pTakeHongBao->dwHongBaoID;

	auto it = m_mapHongbaoInfo.find(pTakeHongBao->dwHongBaoID);
	if (it == m_mapHongbaoInfo.end())
	{
		takeHongBao.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &takeHongBao, sizeof(takeHongBao));
		return;
	}

	//更新红包竞猜次数
	it->second.wGuessCount += 1;

	//检测红包输入金额是否正确 或者 已经有人猜中就不能再猜
	if ((it->second.dwHongBaoScore != pTakeHongBao->dwGuessScore) || it->second.dwGuessUserID != 0)
	{
		takeHongBao.cbResult = 2;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &takeHongBao, sizeof(takeHongBao));
		return;
	}

	//检测红包是否已经过期（大于等于24小时） 
	CT_DWORD dwNowTime = Utility::GetTime();
	if (dwNowTime - it->second.dwSendTime > ONEDAY_SECOND)
	{
		takeHongBao.cbResult = 3;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &takeHongBao, sizeof(takeHongBao));
		return;
	}

	takeHongBao.cbResult = 0;
	takeHongBao.dwHongBaoScore = it->second.dwHongBaoScore;
	//把红包设置为猜中状态
	it->second.dwGuessUserID = pTakeHongBao->dwUserID;
	_snprintf_info(it->second.szGuessNickName, sizeof(it->second.szGuessNickName),  "%s", pUserInfo->szNickName);

	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &takeHongBao, sizeof(takeHongBao));

	//更新红包状态到数据库
	MSG_CS2DB_Update_HongBao updateHongBao;
	updateHongBao.dwHongBaoID = it->second.dwID;
	updateHongBao.dwGuessUserID = it->second.dwGuessUserID;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_HONGBAO, &updateHongBao, sizeof(updateHongBao));
}

CT_VOID	CHongBaoMgr::QueryMyselfHongBao(MSG_CS_Query_Myself_HongBao* pMyselfHongBao)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pMyselfHongBao->dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pMyselfHongBao->dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	MSG_SC_Query_Myself_HongBao mySelfHongBaoInfo;
	mySelfHongBaoInfo.dwMainID = MSG_HONGBAO_MAIN;
	mySelfHongBaoInfo.dwSubID = SUB_SC_QUERY_MYSELF_HONGBAO;
	mySelfHongBaoInfo.dwValue2 = pMyselfHongBao->dwUserID;
	mySelfHongBaoInfo.dwHongBaoID = pMyselfHongBao->dwHongBaoID;

	auto it = m_mapHongbaoInfo.find(pMyselfHongBao->dwHongBaoID);
	if (it == m_mapHongbaoInfo.end())
	{
		mySelfHongBaoInfo.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &mySelfHongBaoInfo, sizeof(mySelfHongBaoInfo));
		return;
	}

	tagHongBaoInfo& hongbaoInfo = it->second;
	if (hongbaoInfo.dwSenderID != pMyselfHongBao->dwUserID)
	{
		mySelfHongBaoInfo.cbResult = 2;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &mySelfHongBaoInfo, sizeof(mySelfHongBaoInfo));
		return;
	}

	//剩余时间
	CT_DWORD dwNowTime = Utility::GetTime();
	mySelfHongBaoInfo.wGuessCount = hongbaoInfo.wGuessCount;
	mySelfHongBaoInfo.dwHongBaoScore = hongbaoInfo.dwHongBaoScore;
	CT_INT32 nRemainTime = hongbaoInfo.dwSendTime + ONEDAY_SECOND - dwNowTime;
	mySelfHongBaoInfo.dwRemainTime = nRemainTime > 0 ? nRemainTime : 0;
	mySelfHongBaoInfo.cbResult = 0;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &mySelfHongBaoInfo, sizeof(mySelfHongBaoInfo));
}

CT_VOID CHongBaoMgr::OperateGuessOkHongBao(MSG_CS_Operate_GuessOk_HongBao* pOperateGuessOk)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pOperateGuessOk->dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pOperateGuessOk->dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	auto it = m_mapHongbaoInfo.find(pOperateGuessOk->dwHongBaoID);
	if (it == m_mapHongbaoInfo.end())
	{
		return;
	}

	MSG_SC_Operate_GuessOK_HongBao operateGuessOkHongBao;
	operateGuessOkHongBao.dwMainID = MSG_HONGBAO_MAIN;
	operateGuessOkHongBao.dwSubID = SUB_SC_OPERATE_GUESSOK_HONGBAO;
	operateGuessOkHongBao.dwValue2 = pOperateGuessOk->dwUserID;
	operateGuessOkHongBao.dwHongBaoID = pOperateGuessOk->dwHongBaoID;

	tagHongBaoInfo& hongbaoInfo = it->second;
	if (hongbaoInfo.dwGuessUserID == 0)
	{
		operateGuessOkHongBao.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &operateGuessOkHongBao, sizeof(operateGuessOkHongBao));
		return;
	}

	if (pOperateGuessOk->cbOperateCode == 1)
	{
		CUserMgr::get_instance().AddUserScore(hongbaoInfo.dwGuessUserID, hongbaoInfo.dwHongBaoScore);	

		//到数据库删除数据
		MSG_CS2DB_Delete_HongBao deleteHongBao;
		deleteHongBao.dwHongBaoID = hongbaoInfo.dwID;
		deleteHongBao.dwTakeUserID = hongbaoInfo.dwGuessUserID;
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_DELETE_HONGBAO, &deleteHongBao, sizeof(deleteHongBao));
		
		//删除内存数据
		m_mapHongbaoInfo.erase(it);
	}
	else
	{
		//把红包重新设置成可以猜的状态
		hongbaoInfo.dwGuessUserID = 0;
		memset(hongbaoInfo.szGuessNickName, 0, sizeof(hongbaoInfo.szGuessNickName));

		//更新红包状态到数据库
		MSG_CS2DB_Update_HongBao updateHongBao;
		updateHongBao.dwHongBaoID = hongbaoInfo.dwID;
		updateHongBao.dwGuessUserID = hongbaoInfo.dwGuessUserID;
		CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_UPDATE_HONGBAO, &updateHongBao, sizeof(updateHongBao));
		
	}
	operateGuessOkHongBao.cbResult = 0;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &operateGuessOkHongBao, sizeof(operateGuessOkHongBao));
}

CT_VOID CHongBaoMgr::TakeTimeOutHongBao(MSG_CS_Take_TimeOut_HongBao* pTakeTimeOutHongBao)
{
	tagGlobalUserInfo* pUserInfo = CUserMgr::get_instance().GetUserInfo(pTakeTimeOutHongBao->dwUserID);
	if (pUserInfo == NULL)
	{
		return;
	}

	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(pTakeTimeOutHongBao->dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	auto it = m_mapHongbaoInfo.find(pTakeTimeOutHongBao->dwHongBaoID);
	if (it == m_mapHongbaoInfo.end())
	{
		return;
	}


	MSG_SC_Take_TimeOut_HongBao takeTimeOutHongBao;
	takeTimeOutHongBao.dwMainID = MSG_HONGBAO_MAIN;
	takeTimeOutHongBao.dwSubID = SUB_SC_TAKE_TIMEOUT_HONGBAO;
	takeTimeOutHongBao.dwValue2 = pTakeTimeOutHongBao->dwUserID;
	takeTimeOutHongBao.dwHongBaoID = pTakeTimeOutHongBao->dwHongBaoID;

	tagHongBaoInfo& hongbaoInfo = it->second;
	//检查红包是否超时
	CT_DWORD dwNowTime = Utility::GetTime();
	if (hongbaoInfo.dwSendTime +  ONEDAY_SECOND > dwNowTime)
	{
		takeTimeOutHongBao.cbResult = 1;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &takeTimeOutHongBao, sizeof(takeTimeOutHongBao));
		return;
	}

	//给玩家补回金币
	CUserMgr::get_instance().AddUserScore(hongbaoInfo.dwSenderID, hongbaoInfo.dwHongBaoScore);
	
	//到数据库删除数据
	MSG_CS2DB_Delete_HongBao deleteHongBao;
	deleteHongBao.dwHongBaoID = hongbaoInfo.dwID;
	deleteHongBao.dwTakeUserID = hongbaoInfo.dwSenderID;
	CNetModule::getSingleton().Send(pNetDB->GetSocket(), MSG_CSDB_MAIN, SUB_CS2DB_DELETE_HONGBAO, &deleteHongBao, sizeof(deleteHongBao));

	//删除内存数据
	m_mapHongbaoInfo.erase(it);
	takeTimeOutHongBao.cbResult = 0;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &takeTimeOutHongBao, sizeof(takeTimeOutHongBao));
}

CT_VOID	CHongBaoMgr::SendHongBaoCountToClient(CT_DWORD dwUserID)
{
	MSG_SC_HongBao_Count hongbaoCount;
	hongbaoCount.dwMainID = MSG_HONGBAO_MAIN;
	hongbaoCount.dwSubID = SUB_SC_HONGBAO_COUNT;
	hongbaoCount.dwCount = m_dwHongBaoCount;

	if (dwUserID != 0)
	{
		acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
		if (pSocket == NULL)
		{
			return;
		}
		hongbaoCount.dwValue2 = dwUserID;
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &hongbaoCount, sizeof(hongbaoCount));
	}
	else
	{
		MapGlobalUser& mapAllGlobalUser = CUserMgr::get_instance().GetAllGlobalUser();
		for (auto& it : mapAllGlobalUser)
		{
			acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(it.first);
			if (pSocket == NULL)
			{
				continue;
			}
			hongbaoCount.dwValue2 = it.first;
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &hongbaoCount, sizeof(hongbaoCount));
		}
	}
}

CT_VOID CHongBaoMgr::SendAllHongBaoInfoToClient(CT_DWORD dwUserID)
{
	acl::aio_socket_stream* pSocket = CUserMgr::get_instance().FindUserProxySocksPtr(dwUserID);
	if (pSocket == NULL)
	{
		return;
	}

	CT_BYTE szBuffer[SYS_NET_SENDBUF_CLIENT] = { 0 };
	CT_DWORD dwSendSize = 0;

	//红包信息
	MSG_GameMsgDownHead msgHead;
	msgHead.dwMainID = MSG_HONGBAO_MAIN;
	msgHead.dwSubID = SUB_SC_HONGBAO_INFO;
	msgHead.dwValue2 = dwUserID;
	memcpy(szBuffer, &msgHead, sizeof(msgHead));
	dwSendSize += sizeof(MSG_GameMsgDownHead);

	std::vector<tagHongBaoInfo*> vecGuessOkHongBao;
	std::vector<tagHongBaoInfo*> vecTimeOutHongBao;

	CT_DWORD dwNowTime = Utility::GetTime();
	for (auto& it : m_mapHongbaoInfo)
	{
		//发送数据
		if ((dwSendSize + sizeof(MSG_SC_HongBao_Info)) > sizeof(szBuffer))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
			dwSendSize = 0;
			msgHead.dwMainID = MSG_HONGBAO_MAIN;
			msgHead.dwSubID = SUB_SC_HONGBAO_INFO;
			msgHead.dwValue2 = dwUserID;
			memcpy(szBuffer, &msgHead, sizeof(msgHead));
			dwSendSize += sizeof(MSG_GameMsgDownHead);
		}

		//已经被猜中的
		if (it.second.dwGuessUserID != 0)
		{
			vecGuessOkHongBao.push_back(&it.second);
			continue;
		}

		//已经过期
		if (it.second.dwSendTime + ONEDAY_SECOND < dwNowTime)
		{
			vecTimeOutHongBao.push_back(&it.second);
			continue;
		}

		MSG_SC_HongBao_Info hongbaoInfo;
		hongbaoInfo.dwHongBaoID = it.second.dwID;
		hongbaoInfo.dwSenderUserID = it.second.dwSenderID;
		
		_snprintf_info(hongbaoInfo.szHongBaoName, sizeof(hongbaoInfo.szHongBaoName), "%s", it.second.szHongBaoName);
		_snprintf_info(hongbaoInfo.szSenderNickName, sizeof(hongbaoInfo.szSenderNickName), "%s", it.second.szSenderNickName);
		_snprintf_info(hongbaoInfo.szHeadUrl, sizeof(hongbaoInfo.szHeadUrl), "%s", it.second.szSenderHeadUrl);

		memcpy(szBuffer + dwSendSize, &hongbaoInfo, sizeof(MSG_SC_HongBao_Info));
		dwSendSize += sizeof(MSG_SC_HongBao_Info);
	}
	//还有红包数据
	if (dwSendSize > sizeof(MSG_GameMsgDownHead))
	{
		CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
	}

	//已经猜中的红包信息
	if (!vecGuessOkHongBao.empty())
	{
		dwSendSize = 0;
		msgHead.dwMainID = MSG_HONGBAO_MAIN;
		msgHead.dwSubID = SUB_SC_GUESS_OK_HONGBAO;
		msgHead.dwValue2 = dwUserID;
		memcpy(szBuffer, &msgHead, sizeof(msgHead));
		dwSendSize += sizeof(MSG_GameMsgDownHead);

		MSG_SC_Guess_OK_HongBao guessOkHongBao;
		for (auto& it : vecGuessOkHongBao)
		{
			if (it->dwSenderID != dwUserID)
			{
				continue;
			}

			if ((dwSendSize + sizeof(MSG_SC_Guess_OK_HongBao)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
				dwSendSize = 0;
				msgHead.dwMainID = MSG_HONGBAO_MAIN;
				msgHead.dwSubID = SUB_SC_GUESS_OK_HONGBAO;
				msgHead.dwValue2 = dwUserID;
				memcpy(szBuffer, &msgHead, sizeof(msgHead));
				dwSendSize += sizeof(MSG_GameMsgDownHead);
			}
			guessOkHongBao.dwHongBaoID = it->dwID;
			guessOkHongBao.dwGuessOkUserID = it->dwGuessUserID;
			guessOkHongBao.dwHongBaoScore = it->dwHongBaoScore;
			_snprintf_info(guessOkHongBao.szHongBaoName, sizeof(guessOkHongBao.szHongBaoName), "%s", it->szHongBaoName);
			_snprintf_info(guessOkHongBao.szGuessOkNickName, sizeof(guessOkHongBao.szGuessOkNickName), "%s", it->szGuessNickName);

			memcpy(szBuffer + dwSendSize, &guessOkHongBao, sizeof(MSG_SC_Guess_OK_HongBao));
			dwSendSize += sizeof(MSG_SC_Guess_OK_HongBao);
		}

		//还有猜中红包数据
		if (dwSendSize > sizeof(MSG_GameMsgDownHead))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
		}
	}

	//已经过期的红包信息
	if (!vecTimeOutHongBao.empty())
	{
		dwSendSize = 0;
		msgHead.dwMainID = MSG_HONGBAO_MAIN;
		msgHead.dwSubID = SUB_SC_TIMEOUT_HONGBAO;
		msgHead.dwValue2 = dwUserID;
		memcpy(szBuffer, &msgHead, sizeof(msgHead));
		dwSendSize += sizeof(MSG_GameMsgDownHead);

		MSG_SC_TimeOut_HongBao timeoutHongBao;
		for (auto& it : vecTimeOutHongBao)
		{
			if (it->dwSenderID != dwUserID)
			{
				continue;
			}

			if ((dwSendSize + sizeof(MSG_SC_TimeOut_HongBao)) > sizeof(szBuffer))
			{
				CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
				dwSendSize = 0;
				msgHead.dwMainID = MSG_HONGBAO_MAIN;
				msgHead.dwSubID = SUB_SC_TIMEOUT_HONGBAO;
				msgHead.dwValue2 = dwUserID;
				memcpy(szBuffer, &msgHead, sizeof(msgHead));
				dwSendSize += sizeof(MSG_GameMsgDownHead);
			}
			timeoutHongBao.dwHongBaoID = it->dwID;
			timeoutHongBao.dwHongBaoScore = it->dwHongBaoScore;
			_snprintf_info(timeoutHongBao.szHongBaoName, sizeof(timeoutHongBao.szHongBaoName), "%s", it->szHongBaoName);

			memcpy(szBuffer + dwSendSize, &timeoutHongBao, sizeof(MSG_SC_TimeOut_HongBao));
			dwSendSize += sizeof(MSG_SC_TimeOut_HongBao);
		}

		//还有过期红包数据
		if (dwSendSize > sizeof(MSG_GameMsgDownHead))
		{
			CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, szBuffer, dwSendSize);
		}
	}

	//如果有过期的红包或者猜中的红包,广播一下红包数量
	if (!vecGuessOkHongBao.empty() || !vecTimeOutHongBao.empty())
	{
		m_dwHongBaoCount = (CT_DWORD)(m_mapHongbaoInfo.size() - vecGuessOkHongBao.size() - vecTimeOutHongBao.size());
		SendHongBaoCountToClient(0);
	}

	memset(&msgHead, 0, sizeof(msgHead));
	msgHead.dwMainID = MSG_HONGBAO_MAIN;
	msgHead.dwSubID = SUB_SC_HONGBAO_INFO_FINISH;
	msgHead.dwValue2 = dwUserID;
	CNetModule::getSingleton().Send(pSocket, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTOC_BY_USERID, &msgHead, sizeof(msgHead));
}