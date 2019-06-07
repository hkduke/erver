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
	//m_mapGroupInfo.insert(std::make_pair(dwGroupID, dwBindUserPlay));
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
	//for (auto& it : m_mapGroupUser)
	//{
		//auto& userList = it.second;


		/*CT_BYTE szBuffer[SYS_NET_SENDBUF_SIZE] = { 0 };
		MSG_C2GS_GroupInfo* pGroupInfo = (MSG_C2GS_GroupInfo*)szBuffer;
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
			pGroupInfo->dwUserCount++;
			CT_DWORD dwUserID = itSet;
			memcpy(szBuffer + sizeof(MSG_C2GS_GroupInfo), &dwUserID, sizeof(dwUserID));
		}
		CNetModule::getSingleton().Send(pSocket, MSG_GCS_MAIN, SUB_C2GS_GROUP_INFO, szBuffer, sizeof(MSG_C2GS_GroupInfo) + pGroupInfo->dwUserCount * sizeof(CT_DWORD));*/
	//}
}

CT_DWORD CWXGroupMgr::GetBindUserPlay(CT_DWORD dwGroupID)
{
	auto it = m_mapGroupInfo.find(dwGroupID);
	if (it != m_mapGroupInfo.end())
	{
		return it->second;
	}

	return 0;
}

CT_BOOL  CWXGroupMgr::CheckCanEnterPRoom(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	CT_DWORD dwBindUserPlay = GetBindUserPlay(dwGroupID);
	if (dwBindUserPlay != 0)
	{
		auto it = m_mapGroupUser.find(dwGroupID);

		std::set<CT_DWORD>& setGroupUser = it->second;

		auto itSet = setGroupUser.find(dwUserID);
		if (itSet == setGroupUser.end())
		{
			return false;
		}
		return true;
	}

	return true;
}