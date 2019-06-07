// ProxyServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "NetModule.h"
#include "MaintainThread.h"
#include "acl_cpp/acl_cpp_init.hpp"
#include "acl_cpp/stdlib/log.hpp"
#include "acl_cpp/stream/aio_handle.hpp"
#include "acl_cpp/stream/aio_istream.hpp"
#include "acl_cpp/stream/aio_listen_stream.hpp"
#include "acl_cpp/stream/aio_socket_stream.hpp"
#include "glog_wrapper.h"
#include "ServerCfg.h"

using namespace acl;

CNetModule *pNetModule = NULL;
CNetConnector *pNetCenter = NULL;
CNetConnector *pNetDB = NULL;
CServerCfg*	  pSerCfg = NULL;

int main(int argc, char* argv[])
{
	std::string strPathName("./script/maintainserver.cf");

	bool use_kernel = false;
	aio_handle handle(use_kernel ? ENGINE_KERNEL : ENGINE_SELECT);

	GlogWrapper gLog("log", argv[0]);
	LOG(INFO) << "正在启动维服工具...";
	
	pSerCfg = new CServerCfg(strPathName.c_str());
	pSerCfg->Load();

	pNetModule = new CNetModule(&handle);
	CNetModule::getSingletonPtr()->InitNetModule<CMaintainThread>(1);
	CNetModule::getSingletonPtr()->RunNetModule();
	CNetModule::getSingletonPtr()->SetServerType(UNKNOW_SERVER);

#ifdef _WIN32
	SetConsoleTitle(CServerCfg::m_ServerName);
#endif

	stConnectParam param;
	param.conType = CONNECT_CENTER;
	pNetCenter = CNetModule::getSingleton().CreateConnector(CServerCfg::m_CenterAddress, false, param);

	bool bCreate = CNetModule::getSingleton().CreateSever(CServerCfg::m_LocalAddress, false);
	if (!bCreate)
	{
		LOG(ERROR) << "create server failed...";
	}
	CNetModule::getSingletonPtr()->StopNetModule();

    return 0;
}

