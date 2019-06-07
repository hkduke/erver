#include "ServerMgr.h"
#include "glog_wrapper.h"
#include "GlobalEnum.h"
#include "NetModule.h"

CServerMgr::CServerMgr()
{

}

CServerMgr::~CServerMgr() 
{
}

CT_VOID CServerMgr::BindProxyServer(acl::aio_socket_stream* pSocket, const PS_BindData* pBindData)
{
	if (pBindData == NULL)
	{
		return;
	}

	if (m_mapProxyServer.find(pBindData->dwProxyServerID) != m_mapProxyServer.end())
	{
		LOG(WARNING) << "repeat bind proxy server, server id :" << pBindData->dwProxyServerID;
		return;
	}

	m_mapProxyServer[pBindData->dwProxyServerID] = pSocket;

	std::string strIp = CNetModule::getSingleton().GetRemoteIp(pSocket);
	if (!strIp.empty())
	{
		LOG(INFO) << "bind one proxy server, ip:" << strIp;//pSocket->get_peer(true);
	}
}

acl::aio_socket_stream* CServerMgr::FindProxyServer(CT_DWORD dwServerID)
{
	auto it = m_mapProxyServer.find(dwServerID);
	if (it != m_mapProxyServer.end())
	{
		return (acl::aio_socket_stream*)it->second;
	}

	return NULL;
}

CT_DWORD CServerMgr::FindProxyServerID(acl::aio_socket_stream* pSocket)
{
	for (auto it = m_mapProxyServer.begin(); it != m_mapProxyServer.end(); ++it)
	{
		if (it->second == pSocket)
		{
			return it->first;
		}
	}

	return 0;
}

CT_VOID CServerMgr::DeleteProxyServer(acl::aio_socket_stream* pSocket)
{
	for (auto it = m_mapProxyServer.begin(); it != m_mapProxyServer.end(); ++it)
	{
		if (it->second == pSocket)
		{
			LOG(WARNING) << "remove one proxy server, id: " << it->first;
			m_mapProxyServer.erase(it);
			return;
		}
	}
}

CT_VOID CServerMgr::BroadcastMsgToProxy(const CT_VOID* pBuf, CT_DWORD dwLen)
{
	for (auto it = m_mapProxyServer.begin(); it != m_mapProxyServer.end(); ++it)
	{
		CNetModule::getSingleton().Send(it->second, MSG_PROXY_MAIN, SUB_PROXY_FASTSENDMSGTO_ANDROID_C, pBuf, dwLen);
	}
}