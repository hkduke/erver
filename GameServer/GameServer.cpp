// GameServer.cpp : 定义控制台应用程序的入口点。
//
#include <iostream>
#include "NetModule.h"
#include "GameServerThread.h"
#include "acl_cpp/acl_cpp_init.hpp"
#include "acl_cpp/stdlib/log.hpp"
#include "acl_cpp/stream/aio_handle.hpp"
#include "acl_cpp/stream/aio_istream.hpp"
#include "acl_cpp/stream/aio_listen_stream.hpp"
#include "acl_cpp/stream/aio_socket_stream.hpp"
#include "glog_wrapper.h"
#include "ServerCfg.h"
#include "FishServerCfg.h"

using namespace acl;

CNetModule *pNetModule = NULL;
CNetConnector *pNetCenter = NULL;
CNetConnector *pNetDB = NULL;
CServerCfg*	  pSerCfg = NULL;
CFishServerCfg *pFishSerCfg = NULL;

int main(int argc, char* argv[])
{
	if (argc < 2)
	{
		printf("Please enter the Server ID!\n");
		return 1;
	}
	CT_CHAR chParamBuf[128] = { 0 };
	memcpy(&chParamBuf, argv[1], sizeof(CT_CHAR)*::strlen(argv[1]));
	std::string strPathName("./script/gameserver_");
	strPathName.append(chParamBuf);
	strPathName.append(".cf");

#ifdef _OS_MAC_CODE
	bool use_kernel = false;
#else
	bool use_kernel = true;
#endif
	aio_handle handle(use_kernel ? ENGINE_KERNEL : ENGINE_SELECT);

	GlogWrapper gLog("log", argv[0]);
	LOG(INFO) << "正在启动游戏服务器...";
	//pSerCfg = new CServerCfg("./script/gameserver.cf");
	pSerCfg = new CServerCfg(strPathName.c_str());
	pSerCfg->Load();

	//如果是捕鱼GameServer就读取gameserver_fish_roomid.cf
	if (CServerCfg::m_nGameID == GAME_FISH)
	{
		acl::string strFishPathName;
		strFishPathName.format("./script/gameserver_fish_%d.cf", CServerCfg::m_nRoomKindID);
		pFishSerCfg = new CFishServerCfg(strFishPathName.c_str());
		pFishSerCfg->load();
	}

	//设置一次随机种子
	srand(time(NULL));

	pNetModule = new CNetModule(&handle);
	CNetModule::getSingletonPtr()->InitNetModule<CGameServerThread>(1);
	CNetModule::getSingletonPtr()->RunNetModule();
	CNetModule::getSingletonPtr()->SetServerType(GAME_SERVER);

#ifdef _WIN32
	SetConsoleTitle(Utility::Utf82Ansi(CServerCfg::m_ServerName).c_str());
#endif

	stConnectParam param;
	param.conType = CONNECT_CENTER;
	param.dwServerID = CServerCfg::m_nServerID;
	pNetCenter = CNetModule::getSingleton().AddConnector(CServerCfg::m_CenterAddress, false, param);

	param.conType = CONNECT_DB;
	param.dwServerID = CServerCfg::m_nServerID;
	pNetDB = CNetModule::getSingleton().AddConnector(CServerCfg::m_DbServerAddress, false, param);

	bool bCreate = CNetModule::getSingleton().CreateSever(CServerCfg::m_LocalAddress, false);

	if (!bCreate)
	{
		LOG(ERROR) << "create game server fail!";
	}

	return 0;
}