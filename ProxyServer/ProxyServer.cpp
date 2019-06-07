// ProxyServer.cpp : 定义控制台应用程序的入口点。
//

#include "stdafx.h"
#include <iostream>
#include "NetModule.h"
#include "ProxyThread.h"
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
	if (argc < 2)
	{
		printf("Please enter the Server ID!\n");
		return 1;
	}
	CT_CHAR chParamBuf[128] = { 0 };
	memcpy(&chParamBuf, argv[1], sizeof(CT_CHAR)*::strlen(argv[1]));
	std::string strPathName("./script/proxyserver_");
	strPathName.append(chParamBuf);
	strPathName.append(".cf");

#ifdef _OS_MAC_CODE
	bool use_kernel = false;
#else
	bool use_kernel = true;
#endif
	aio_handle handle(use_kernel ? ENGINE_KERNEL : ENGINE_SELECT);

	GlogWrapper gLog("log", argv[0]);
	LOG(INFO) << "正在启动代理服务器...";
	
	//pSerCfg = new CServerCfg("./script/proxyserver.cf");
	pSerCfg = new CServerCfg(strPathName.c_str());
	pSerCfg->Load();

	pNetModule = new CNetModule(&handle);
	CNetModule::getSingletonPtr()->InitNetModule<CProxyThread>(1);
	CNetModule::getSingletonPtr()->RunNetModule();
	CNetModule::getSingletonPtr()->SetServerType(PROXY_SERVER);

#ifdef _WIN32
	SetConsoleTitle(CServerCfg::m_ServerName);
#endif

	stConnectParam param;
	param.conType = CONNECT_CENTER;
	param.dwServerID = 0;
	pNetCenter = CNetModule::getSingleton().CreateConnector(CServerCfg::m_CenterAddress, false, param);

	param.conType = CONNECT_DB;
	param.dwServerID = 1;
	pNetDB = CNetModule::getSingleton().CreateConnector(CServerCfg::m_DbServerAddress, false, param);

	//param.conType = CONNECT_CUSTOMER;
	//pNetCustomer = CNetModule::getSingleton().CreateConnector(CServerCfg::m_CustomerSerAddress, param);


	bool bCreate = CNetModule::getSingleton().CreateSever(CServerCfg::m_LocalAddress, true);
	if (!bCreate)
	{
		LOG(ERROR) << "create server failed...";
	}
	CNetModule::getSingletonPtr()->StopNetModule();

    return 0;
}

