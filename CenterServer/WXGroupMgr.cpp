#include "stdafx.h"
#include "WXGroupMgr.h"
#include "CMD_Inner.h"
#include "NetModule.h"

CWXGroupMgr::CWXGroupMgr()
{
	m_mapGroupUser.clear();
	m_mapGroupInfo.clear();
}

CWXGroupMgr::~CWXGroupMgr()
{
}


CT_VOID CWXGroupMgr::InsertGroupInfo(CT_DWORD dwGroupID, CT_DWORD dwBindUserPlay)
{
	auto it = m_mapGroupInfo.find(dwGroupID);
	if (it != m_mapGroupInfo.end())
	{
		it->second = dwBindUserPlay;
	}
	else
	{
		m_mapGroupInfo.insert(std::make_pair(dwGroupID, dwBindUserPlay));
	}
}

CT_VOID CWXGroupMgr::InsertGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwGroupID);
	if (it != m_mapGroupUser.end())
	{
		it->second.insert(dwUserID);
	}
	else
	{
		std::set<CT_DWORD> setUserID;
		setUserID.insert(dwUserID);
		m_mapGroupUser.insert(std::make_pair(dwGroupID, setUserID));
	}
}

CT_VOID CWXGroupMgr::RemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwGroupID);
	if (it != m_mapGroupUser.end())
	{
		it->second.erase(dwUserID);
	}
}

CT_VOID CWXGroupMgr::SendGroupToGameServer(acl::aio_socket_stream* pSocket)
{
	for (auto& it : m_mapGroupUser)
	{
		auto& userList = it.second;

		CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		MSG_C2GS_WXGroupInfo* pGroupInfo = (MSG_C2GS_WXGroupInfo*)szBuffer;
		pGroupInfo->dwGroupID = it.first;
		auto itMaster = m_mapGroupInfo.find(it.first);
		if (itMaster == m_mapGroupInfo.end())
		{
			continue;
		}
		pGroupInfo->dwBindUserPlay = itMaster->second;

		pGroupInfo->dwUserCount = 0;
		for (auto& itSet : userList)
		{
			CT_DWORD dwUserID = itSet;
			memcpy(szBuffer + sizeof(MSG_C2GS_WXGroupInfo) + sizeof(dwUserID)*pGroupInfo->dwUserCount, &dwUserID, sizeof(dwUserID));
			pGroupInfo->dwUserCount++;
		}
		CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_WXGROUP_INFO, szBuffer, sizeof(MSG_C2GS_WXGroupInfo) + pGroupInfo->dwUserCount * sizeof(CT_DWORD));
	}
}
