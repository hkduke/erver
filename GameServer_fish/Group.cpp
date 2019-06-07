#include "Group.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "NetModule.h"
#include "GroupMgr.h"

CGSGroup::CGSGroup()
{
	m_GroupInfo.reset();
}

CGSGroup::~CGSGroup()
{

}

CT_VOID	CGSGroup::InsertGroupInfo(const tagGroupInfo* pGroupInfo)
{
	std::unique_ptr<tagGroupInfo> ptrGroupInfo(new tagGroupInfo);
	m_GroupInfo = std::move(ptrGroupInfo);
	m_GroupInfo->dwGroupID = pGroupInfo->dwGroupID;
	m_GroupInfo->dwMasterID = pGroupInfo->dwMasterID;
	m_GroupInfo->wUserCount = pGroupInfo->wUserCount;
	//m_GroupInfo->dwKindID = pGroupInfo->dwKindID;
	//m_GroupInfo->wBindUserPlay = pGroupInfo->wBindUserPlay;
	_snprintf_info(m_GroupInfo->szGroupName, sizeof(m_GroupInfo->szGroupName), "%s", pGroupInfo->szGroupName);
	//_snprintf_info(m_GroupInfo->szMasterName, sizeof(m_GroupInfo->szMasterName), "%s", pGroupInfo->szMasterName);
	_snprintf_info(m_GroupInfo->szNotice, sizeof(m_GroupInfo->szNotice), "%s", pGroupInfo->szNotice);

}

CT_VOID CGSGroup::InsertGroupUser(const tagGroupUserInfo* pGroupUser)
{
	std::unique_ptr<tagGroupUserInfo> userInfoPtr(new tagGroupUserInfo);
	userInfoPtr->dwUserID = pGroupUser->dwUserID;
	//_snprintf_info(userInfoPtr->szNickName, sizeof(userInfoPtr->szNickName), "%s", pGroupUser->szNickName);
	//_snprintf_info(userInfoPtr->szHeadUrl, sizeof(userInfoPtr->szHeadUrl), "%s", pGroupUser->szHeadUrl);

	m_mapGroupUser.insert(std::make_pair(pGroupUser->dwUserID, std::move(userInfoPtr)));

	//群组人数增加
	m_GroupInfo->wUserCount += 1;
}

CT_VOID	CGSGroup::RemoveGroupUser(CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwUserID);

	if (it != m_mapGroupUser.end())
	{
		m_mapGroupUser.erase(it);
	}

	//群组人数减少
	m_GroupInfo->wUserCount -= 1;
}

CT_DWORD CGSGroup::GetGroupMasterID()
{
	return m_GroupInfo->dwMasterID;
}

CT_VOID	CGSGroup::AddGroupUser(tagGroupUserInfo* pGroupUser)
{
	CGSGroupMgr::get_instance().AddGroupUser(m_GroupInfo->dwGroupID, pGroupUser);
}

CT_VOID	CGSGroup::QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwUserID);
	if (it == m_mapGroupUser.end())
	{
		return;
	}

	//内存数据删除
	CGSGroupMgr::get_instance().RemoveGroupUser(dwGroupID, dwUserID);

}

CT_BOOL	CGSGroup::IsGroupUser(CT_DWORD dwUserID)
{
	auto it = m_mapGroupUser.find(dwUserID);
	if (it != m_mapGroupUser.end())
	{
		return true;
	}

	return false;
}

CT_DWORD CGSGroup::GetGroupUserCount()
{
	return (CT_DWORD)m_mapGroupUser.size();
}