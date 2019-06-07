#include "ServerMgr.h"
#include "glog_wrapper.h"
#include "GlobalEnum.h"

CServerMgr::CServerMgr()
{
	memset(&m_centerInfo, 0, sizeof(m_centerInfo));
}

CServerMgr::~CServerMgr() 
{
}


CT_VOID	CServerMgr::RegisterCenter(CMD_CenterServer_Info* pCenterInfo, acl::aio_socket_stream* pSocket)
{
	memcpy(&m_centerInfo, pCenterInfo, sizeof(CMD_CenterServer_Info));
	m_centerInfo.pCenterSocket = pSocket;
}

CMD_CenterServer_InfoEx* CServerMgr::GetCenterServerInfo()
{
	return &m_centerInfo;
}