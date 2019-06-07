#pragma once
#include "CMD_Inner.h"
#include "acl_cpp/lib_acl.hpp"
#include "NetConnector.h"
#include <map>
#include <set>
#include <vector>


struct stGameServerInfo : public CMD_GameServer_Info
{
	CNetConnector* pConnector;
};

struct stLoginServerInfo : public CMD_LoginServer_Info
{
	CNetConnector* pConnector;
};

typedef std::map<CT_WORD, std::set<CT_WORD>>	MapGameKind;
typedef std::map<CT_DWORD, stGameServerInfo>	MapGameServerInfo;
typedef std::map<CT_DWORD, stLoginServerInfo>	MapLoginServerInfo;
typedef std::vector<CT_DWORD>					VecLoginServerID;

class CServerMgr : public acl::singleton<CServerMgr>
{
public:
	CServerMgr() : m_SelLoginIndex(0) {}
	~CServerMgr() {}

public:
	CT_VOID AddGameServerList(const CMD_GameServer_Info* pGameServer);
	CT_VOID AddLoginServerList(const CMD_LoginServer_Info* pLoginServer);

	CT_VOID DeleteGameServer(const CMD_GameServer_Info* pGameServer);
	CT_VOID DeleteLoginServer(const CMD_LoginServer_Info* pLoginServer);

	acl::aio_socket_stream* SelectOneLoginServer();

	CT_BOOL IsExistServer(CT_DWORD dwServerID);
	CT_VOID CheckDisconnectServer();

	stGameServerInfo* FindGameServer(CT_DWORD dwServerID);

private:
	MapGameKind			m_mapGameKind;
	MapGameServerInfo	m_mapGameServer;

	MapLoginServerInfo	m_mapLoginServer;
	VecLoginServerID	m_vecLoginServer;
	size_t				m_SelLoginIndex;
};