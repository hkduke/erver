#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "CMD_Inner.h"
#include "GlobalStruct.h"
#include <map>
#include <set>
#include "CMD_Plaza.h"

typedef std::map<CT_DWORD, acl::aio_socket_stream*> MapProxyServer;
typedef std::map<CT_DWORD, std::set<tagGameKind*>> MapGameKind;			//游戏类型
typedef std::map<CT_DWORD, std::set<tagGameRoomKind*>> MapGamePlayKind;	//玩法类型

class CServerMgr : public acl::singleton<CServerMgr>
{
public:
	CServerMgr();
	~CServerMgr();

	CT_VOID BindProxyServer(acl::aio_socket_stream* pSocket, const PS_BindData* pBindData);
	acl::aio_socket_stream* FindProxyServer(CT_DWORD dwServerID);
	CT_DWORD FindProxyServerID(acl::aio_socket_stream* pSocket);

	CT_VOID DeleteProxyServer(acl::aio_socket_stream* pSocket);

	CT_BOOL InsertGameKind(tagGameKind* pGameKind);
	CT_BOOL InsertGamePlayKind(tagGameRoomKind* pPlayKind);

	CT_BOOL EmumGameKindItem(CT_DWORD dwGameID, std::vector<MSG_SC_GameItem>& vecGameItem);
	CT_BOOL EmumGameRoomKindItem(CT_DWORD dwGameID, std::vector<MSG_SC_RoomKindItem>& vecPlayKindItem);

	CT_BOOL EmumPrivateRoomInfo(CT_DWORD dwGameID, std::vector<MSG_SC_PrivateRoomInfo>& vecPrivateItem);

#ifdef _OS_WIN32_CODE
#define json_int64 __int64
#else
#define json_int64 long long int 
#endif

private:
	MapProxyServer		m_mapProxyServer; 
	MapGameKind			m_mapGameKind;
	MapGamePlayKind		m_mapGamePlayKind;
};