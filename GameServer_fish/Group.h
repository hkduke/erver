#pragma once
#include "acl_cpp/lib_acl.hpp"
#include <map>
#include <set>
#include <memory>
#include "CTType.h"
#include "GlobalStruct.h"

class CGSGroup
{
public:
	CGSGroup();
	~CGSGroup();

	CT_VOID		InsertGroupInfo(const tagGroupInfo* pGroupInfo);
	CT_VOID		InsertGroupUser(const tagGroupUserInfo* pGroupUser);
	CT_VOID		RemoveGroupUser(CT_DWORD dwUserID);

	CT_DWORD	GetGroupMasterID();
	CT_VOID		AddGroupUser(tagGroupUserInfo* pGroupUser);
	CT_VOID		QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID);

	CT_BOOL		IsGroupUser(CT_DWORD dwUserID);
	CT_VOID		RemoveGroupUserFromDB(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_DWORD	GetGroupUserCount();

	std::unique_ptr<tagGroupInfo>& GetGroupInfo() { return m_GroupInfo; }
	std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>& GetAllGroupUserInfo() { return m_mapGroupUser; }

private:
	std::unique_ptr<tagGroupInfo> 								m_GroupInfo;
	std::map<CT_DWORD, std::unique_ptr<tagGroupUserInfo>>		m_mapGroupUser;
};