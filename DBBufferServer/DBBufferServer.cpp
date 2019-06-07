// DBBufferServer.cpp : 定义控制台应用程序的入口点。
//
// CenterServer.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include "NetModule.h"
#include "DBThread.h"
#include "acl_cpp/acl_cpp_init.hpp"
#include "acl_cpp/stdlib/log.hpp"
#include "acl_cpp/stream/aio_handle.hpp"
#include "acl_cpp/stream/aio_istream.hpp"
#include "acl_cpp/stream/aio_listen_stream.hpp"
#include "acl_cpp/stream/aio_socket_stream.hpp"
#include "glog_wrapper.h"
#include "ServerCfg.h"
#include "BuildDBThread.h"

using namespace acl;

CNetModule *pNetModule = NULL;
CServerCfg*	  pSerCfg = NULL;

static void BuildDBThread(void)
{
	CBuildDBThread* thr = new CBuildDBThread();

	thr->set_detachable(true);
	if (thr->start() == false)
		LOG(ERROR) << "start build db thread failed";

	LOG(INFO) << "start build db thread...";
}

int main(int argc, char* argv[])
{
    if (argc < 2)
    {
        printf("Please enter the Server ID!\n");
        return 1;
    }
    CT_CHAR chParamBuf[128] = { 0 };
    memcpy(&chParamBuf, argv[1], sizeof(CT_CHAR)*::strlen(argv[1]));
    std::string strPathName("./script/dbserver_");
    strPathName.append(chParamBuf);
    strPathName.append(".cf");

#ifdef _OS_MAC_CODE
	bool use_kernel = false;
#else 
	bool use_kernel = true;
#endif
	aio_handle handle(use_kernel ? ENGINE_KERNEL : ENGINE_SELECT);

	GlogWrapper gLog("log", argv[0]);
	LOG(INFO) << "正在启动DB服务器...";
    pSerCfg = new CServerCfg(strPathName.c_str());
	pSerCfg->Load();
	if (CServerCfg::m_MainDBServer)
    {
        BuildDBThread();
    }

	pNetModule = new CNetModule(&handle);
	CNetModule::getSingletonPtr()->InitNetModule<DBThread>(CServerCfg::m_WorkThreadNum);
	CNetModule::getSingletonPtr()->RunNetModule();
	CNetModule::getSingletonPtr()->SetServerType(DB_SERVER);

#ifdef _WIN32
	SetConsoleTitle("DB前置机");
#endif

	bool bCreate = CNetModule::getSingleton().CreateSever(CServerCfg::m_LocalAddress, false);
	if (!bCreate)
	{
		LOG(ERROR) << "create db server fail...";
	}

	return 0;
}
