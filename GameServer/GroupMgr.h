﻿#pragma once
#include "acl_cpp/lib_acl.hpp"
#include <map>
#include <set>
#include <memory>
#include "CTType.h"
#include "GlobalStruct.h"
#include "Group.h"
#include "CMD_Plaza.h"
#include "CMD_Inner.h"

#define MAX_GROUP_COUNT		 10
#define MAX_GROUP_USER_COUNT 100

typedef std::map<CT_DWORD, std::unique_ptr<CGSGroup>> MapGroup;
typedef std::map<CT_DWORD, CT_DWORD> MapGroupPRoom; //roomnum, groupid

class CGSGroupMgr : public acl::singleton<CGSGroupMgr>
{
public:
	CGSGroupMgr();
	~CGSGroupMgr();

public:
	CT_VOID		InsertGroupInfo(tagGroupInfo* pGroupInfo);
	CT_VOID		InsertGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser);
	CT_VOID		RemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID);

	CT_VOID		AddGroupUser(CT_DWORD dwGroupID, tagGroupUserInfo* pGroupUser);
	CT_DWORD	GetGroupMasterID(CT_DWORD dwGroupID);
	CT_VOID		QuitGroup(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_VOID		RemoveGroup(CT_DWORD dwGroupID);
	CT_BOOL		IsGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_BOOL		HasGroup(CT_DWORD dwGroupID);
	CT_DWORD	GetGroupUserCount(CT_DWORD dwGroupID);
	CT_VOID		ModifyGroupInfo(MSG_CS_Modify_GroupInfo* pGroupInfo);
	CT_DWORD	CalUserGroupCount(CT_DWORD dwUserID);
	CT_VOID		ModifyGroupOptions(CT_DWORD dwGroupID, CT_WORD wBindUserPlay);

	CT_VOID     AddGroupPRoom(MSG_C2GS_Add_GroupPRoom* pGroupPRoom);
	CT_VOID     RemoveGroupPRoom(CT_DWORD dwRoomNum);
	CT_DWORD    GetPRoomGroupID(CT_DWORD dwRoomNum);

	CT_BOOL		CheckCanEnterPRoom(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_BOOL     CheckCanEnterGroupPRoom(CT_DWORD dwUserID, CT_DWORD dwRoomNum);

private:
	MapGroup				m_mapGroup;
    MapGroupPRoom           m_mapGroupPRoom;
};
