// CenterServer.cpp : 定义控制台应用程序的入口点。
//
#include "stdafx.h"
#include <iostream>
#include "NetModule.h"
#include "CenterThread.h"
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
CNetConnector *pNetDB = NULL;
CServerCfg*	  pSerCfg = NULL;

int main(int argc, char* argv[])
{
#ifdef _OS_MAC_CODE
	bool use_kernel = false;
#else
	bool use_kernel = true;
#endif
	aio_handle handle(use_kernel ? ENGINE_KERNEL : ENGINE_SELECT);

	GlogWrapper gLog("log", argv[0]);
	LOG(INFO) << "正在启动中心服务器...";
	pSerCfg = new CServerCfg("./script/centerserver.cf");
	pSerCfg->Load();

	pNetModule = new CNetModule(&handle);
	CNetModule::getSingletonPtr()->InitNetModule<CCenterThread>(1);
	CNetModule::getSingletonPtr()->RunNetModule();
	CNetModule::getSingletonPtr()->SetServerType(CENTER_SERVER);

//#ifdef _OS_LINUX_CODE
	//CNetModule::getSingleton().InitDaemon();
	//signal(SIGPIPE, SIG_IGN);
	//signal(SIGCHLD, SIG_IGN);
//#endif // _OS_LINUX_CODE

#ifdef _OS_WIN32_CODE
	SetConsoleTitle("中心服务器");
#endif

	stConnectParam param;
	param.conType = CONNECT_DB;
	param.dwServerID = 0;
	pNetDB = CNetModule::getSingleton().AddConnector(CServerCfg::m_DbServerAddress, false, param);

	bool bCreate = CNetModule::getSingleton().CreateSever(CServerCfg::m_LocalAddress, false);
	if (!bCreate)
	{
		exit(0);
	}

	return 0;
}
