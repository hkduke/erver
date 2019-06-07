#pragma once
#include "acl_cpp/lib_acl.hpp"
#include <map>
#include <set>
#include "CTType.h"

typedef std::map<CT_DWORD, std::set<CT_DWORD>> MapGroupUser;
typedef std::map<CT_DWORD, CT_DWORD> MapGroupInfo;

class CWXGroupMgr
{
public:
	CWXGroupMgr();
	~CWXGroupMgr();

public:
	CT_VOID InsertGroupInfo(CT_DWORD dwGroupID, CT_DWORD dwBindUserPlay);
	CT_VOID InsertGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID);
	CT_VOID RemoveGroupUser(CT_DWORD dwGroupID, CT_DWORD dwUserID);

	CT_VOID SendGroupToGameServer(acl::aio_socket_stream* pSocket);

	CT_DWORD GetBindUserPlay(CT_DWORD dwGroupID);
	CT_BOOL  CheckCanEnterPRoom(CT_DWORD dwGroupID, CT_DWORD dwUserID);

private:
	MapGroupUser	m_mapGroupUser;
	MapGroupInfo	m_mapGroupInfo;
}; 