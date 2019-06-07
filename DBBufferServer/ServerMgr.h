#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "CMD_Inner.h"
#include "GlobalStruct.h"
#include <map>
#include <set>
#include "CMD_Plaza.h"

struct CMD_CenterServer_InfoEx : public CMD_CenterServer_Info
{
	acl::aio_socket_stream* pCenterSocket;
};

class CServerMgr : public acl::singleton<CServerMgr>
{
public:
	CServerMgr();
	~CServerMgr();

	CT_VOID		RegisterCenter(CMD_CenterServer_Info* pCenterInfo, acl::aio_socket_stream* pSocket);
	CMD_CenterServer_InfoEx* GetCenterServerInfo();

private:
	CMD_CenterServer_InfoEx		m_centerInfo;
};