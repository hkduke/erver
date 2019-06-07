#include "GroupMgr.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "NetModule.h"

CGSGroupMgr::CGSGroupMgr()
{
}

CGSGroupMgr::~CGSGroupMgr()
{
}

CT_VOID CGSGroupMgr::InsertGroupInfo(tagGroupInfo* pGroupInfo)
{
	auto it = m_mapGroup.find(pGroupInfo->dwGroupID);
	if (it != m_mapGroup.end())
	{
		LOG(WARNING) << "insert group repeat? group id: " << pGroupInfo->dwGroupID;
		return;
	}

	std::unique_ptr<CGSGroup> groupPtr(new CGSGroup);
	groupPtr->InsertGroupInfo(pGroupInfo);

	m_mapGroup.insert(std::make_pair(pGroupInfo->dwGroupID, std::move(groupPtr)));
}


CT_VOID CGSGroupMgr::InsertGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		LOG(WARNING) << "insert group user fail, can not find group info. group id: " << dwGroupID;
		return;
	}

	it->second->InsertGroupUser(pGroupUser);
}

CT_VOID CGSGroupMgr::RemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		LOG(WARNING) << "remove group user fail, can not find group info. group id: " << dwGroupID;
		return;
	}

	it->second->RemoveGroupUser(dwUserID);
}


CT_VOID CGSGroupMgr::AddGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser)
{
	InsertGroupUser(dwGroupID, pGroupUser);
}

CT_DWORD CGSGroupMgr::GetGroupMasterID(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->GetGroupMasterID();
	}
	return 0;
}

CT_VOID	CGSGroupMgr::QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		it->second->QuitGroup(dwGroupID, dwUserID);
	}
}

CT_VOID	CGSGroupMgr::RemoveGroup(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		m_mapGroup.erase(it);
	}
}


CT_BOOL	CGSGroupMgr::IsGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->IsGroupUser(dwUserID);
	}
	return false;
}

CT_BOOL	CGSGroupMgr::HasGroup(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return true;
	}
	return false;
}

CT_DWORD CGSGroupMgr::GetGroupUserCount(CT_DWORD dwGroupID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it != m_mapGroup.end())
	{
		return it->second->GetGroupUserCount();;
	}
	return false;
}

CT_VOID	CGSGroupMgr::ModifyGroupInfo(MSG_CS_Modify_GroupInfo* pModifyGroupInfo)
{
	auto it = m_mapGroup.find(pModifyGroupInfo->dwGroupID);
	if (it == m_mapGroup.end())
	{
		return;
	}

	std::unique_ptr<tagGroupInfo>& pGroupInfo = it->second->GetGroupInfo();
	if (pModifyGroupInfo->dwMasterID != pModifyGroupInfo->dwMasterID)
	{
		return;
	}

	//修改群公告或者群名称
	_snprintf_info(pGroupInfo->szGroupName, sizeof(pGroupInfo->szGroupName), "%s", pModifyGroupInfo->szGroupName);
	_snprintf_info(pGroupInfo->szNotice, sizeof(pGroupInfo->szNotice), "%s", pModifyGroupInfo->szNotice);
}

CT_DWORD CGSGroupMgr::CalUserGroupCount(CT_DWORD dwUserID)
{
	CT_DWORD dwGroupCount = 0;
	for (auto& it : m_mapGroup)
	{
		if (dwUserID == it.second->GetGroupMasterID())
		{
			++dwGroupCount;
		}
	}

	return dwGroupCount;
}

CT_VOID	CGSGroupMgr::ModifyGroupOptions(CT_DWORD dwGroupID, CT_WORD wBindUserPlay)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		return;
	}

	/*std::unique_ptr<tagGroupInfo>& groupInfo = it->second->GetGroupInfo();
	if (groupInfo->wBindUserPlay != wBindUserPlay)
	{
		groupInfo->wBindUserPlay = wBindUserPlay;
	}*/
}


CT_BOOL	CGSGroupMgr::CheckCanEnterPRoom(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroup.find(dwGroupID);
	if (it == m_mapGroup.end())
	{
		return false;
	}

	/*std::unique_ptr<tagGroupInfo>& groupInfo = it->second->GetGroupInfo();
	if (groupInfo->wBindUserPlay == 0)
	{
		return true;
	}*/

	return it->second->IsGroupUser(dwUserID);
}