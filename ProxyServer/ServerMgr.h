#pragma once
#include "CMD_Inner.h"
#include "CMD_Customer.h"
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

struct stCustomerServerInfo : public CMD_CustomerServer_Info
{
	CNetConnector* pConnector;
};


typedef std::map<CT_WORD, std::set<CT_WORD>>	MapGameKind;
typedef std::map<CT_DWORD, stGameServerInfo>	MapGameServerInfo;
typedef std::map<CT_DWORD, stLoginServerInfo>	MapLoginServerInfo;
typedef std::map<CT_DWORD, stCustomerServerInfo> MapCustomerServerInfo;
typedef std::vector<CT_DWORD>					VecLoginServerID;

class CServerMgr : public acl::singleton<CServerMgr>
{
public:
	CServerMgr() : m_SelLoginIndex(0), m_SelMainLoginIndex(0) {}
	~CServerMgr() {}

public:
	CT_VOID AddGameServerList(const CMD_GameServer_Info* pGameServer);
	CT_VOID AddLoginServerList(const CMD_LoginServer_Info* pLoginServer);
	CT_VOID AddCustomerServerList(const CMD_CustomerServer_Info* pCustomerServer);

	CT_VOID DeleteGameServer(const CMD_GameServer_Info* pGameServer);
	CT_VOID DeleteLoginServer(const CMD_LoginServer_Info* pLoginServer);
	CT_VOID DeleteCustomerServer(const CMD_CustomerServer_Info* pCustomerServer);

	acl::aio_socket_stream* SelectOneLoginServer();
	acl::aio_socket_stream* SelectMainLoginServer();
	acl::aio_socket_stream* SelectOneCustomerServer(); //其实有且只有一个customerserver

	CT_BOOL IsExistServer(CT_DWORD dwServerID);
	CT_BOOL IsExistCustomerServer(CT_DWORD dwServerID);
	CT_VOID CheckDisconnectServer(CT_DWORD dwServerID);
	CT_VOID CheckDisconnectCustomerServer();

	stGameServerInfo* FindGameServer(CT_DWORD dwServerID);
	stGameServerInfo* FindReqGameServer(CT_WORD wGameID, CT_WORD wKindID,CT_WORD wRoomKindID, CT_BYTE& cbState);
	stGameServerInfo* FindReqGameServerEx(CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID);

private:
	MapGameKind			m_mapGameKind;
	MapGameServerInfo	m_mapGameServer;

	MapLoginServerInfo	m_mapLoginServer;
	VecLoginServerID	m_vecLoginServer;
	VecLoginServerID 	m_vecMainLoginServer;
	size_t				m_SelLoginIndex;
	size_t				m_SelMainLoginIndex;

	MapCustomerServerInfo m_mapCustomerServer;
};