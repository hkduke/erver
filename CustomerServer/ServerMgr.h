#pragma once
#include "CMD_Inner.h"
#include "acl_cpp/lib_acl.hpp"
#include "NetConnector.h"
#include <map>
#include <set>
#include <vector>

typedef std::map<CT_DWORD, acl::aio_socket_stream*> MapProxyServer;
class CServerMgr : public acl::singleton<CServerMgr>
{
public:
	CServerMgr();
	~CServerMgr();

	CT_VOID BindProxyServer(acl::aio_socket_stream* pSocket, const PS_BindData* pBindData);
	acl::aio_socket_stream* FindProxyServer(CT_DWORD dwServerID);
	CT_DWORD FindProxyServerID(acl::aio_socket_stream* pSocket);

	CT_VOID DeleteProxyServer(acl::aio_socket_stream* pSocket);

private:
	MapProxyServer		m_mapProxyServer;
};