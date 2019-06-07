#include "NetModule.h"
#include "MemPool.h"
#include "ObjPool.h"
#include "WorkThread.h"
#include "socket_include.h"
#include "NetConnector.h"
#include "NetServer.h"
#include "Utility.h"
#ifndef _OS_WIN32_CODE
#include <sys/types.h>  
#include <sys/stat.h>
#endif

extern CCObjPool<CMesBlock>	g_MsgBlockPool;

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

template<> CNetModule* CCSingleton<CNetModule>::ms_Singleton=NULL;

CNetModule::CNetModule(acl::aio_handle* pIoHandle):m_bIsInit(CT_FALSE), m_serverType(UNKNOW_SERVER), m_pIoHandle(pIoHandle)
{
	for (int i = 0 ; i < SYS_THREAD_MAX_NUM; ++i)
	{
		m_pWorkThread[i] = NULL;
	}
	m_wWorkThreadCount = 0;

	m_pNetWork = NULL;
	//m_pHttpService = NULL;
	m_pTimer = NULL;
	//m_ClientSocksIndex = 0;
	m_mapClientSocket.clear();
	//m_mapClientSocketIndex.clear();
	m_mapServerSocket.clear();
	m_setConnector.clear();
	m_setWaitCloseSocket.clear();
}

CNetModule::~CNetModule()
{
	if (m_bIsInit)
	{
		m_bIsInit = CT_FALSE;
		for (auto it : m_setConnector)
		{
			SAFE_DELETE(it);
		}

		for (auto it : m_setAddConnector)
		{
			SAFE_DELETE(it);
		}
	}
}

CT_VOID CNetModule::InitOther(CT_VOID)
{
	new CCMemPool();
	g_MsgBlockPool.Init(SYS_INIT_MSG_BLOCK_NUM, SYS_EXPANT_NEW_OBJ_NUM, 0);
}

//运行网络模块
CT_VOID CNetModule::RunNetModule(CT_VOID)
{
	if (m_bIsInit)
	{
		LOG(INFO) << "Start Run NetModule...";

		m_pIoHandle->set_timer(m_pTimer, TIME_LOOP, IDI_TIMER_LOOP);
		m_pIoHandle->set_timer(m_pTimer, TIME_TIMER_EVENT, IDI_TIMER_EVENT);
		m_pIoHandle->set_timer(m_pTimer, TIME_RECONNECT_SERVER, IDI_TIMER_RECONNECT_SERVER);
		//DIP和CUSTOMER不启用心跳定时器
		if (m_serverType != DIP_SERVER && m_serverType != CUSTOMER_SERVER)
		{
			m_pIoHandle->set_timer(m_pTimer, TIME_HEART_BEAT, IDI_TIMER_HEART_BEAT);
		}

		for (int i = 0; i < SYS_THREAD_MAX_NUM; ++i)
		{
			if (m_pWorkThread[i])
			{
				m_pWorkThread[i]->Start();
			}
		}

		LOG(INFO) << "NetModule Already Startover.";
	}
}

//停止网络模块
CT_VOID CNetModule::StopNetModule(CT_VOID)
{
	if (m_bIsInit)
	{
		LOG(INFO) << "Stop NetModule...";

		m_pIoHandle->del_timer(m_pTimer, IDI_TIMER_LOOP);
		m_pIoHandle->del_timer(m_pTimer, IDI_TIMER_EVENT);
		m_pIoHandle->del_timer(m_pTimer, IDI_TIMER_RECONNECT_SERVER);
		m_pIoHandle->del_timer(m_pTimer, IDI_TIMER_HEART_BEAT);

		m_pIoHandle->stop();
		StopWorkThread();

		m_bIsInit = CT_FALSE;

		LOG(INFO) << "NetModule Already Stop.";
	}
}

//停止工作线程
CT_VOID CNetModule::StopWorkThread(CT_VOID)
{
	//停止工作线程
	for (int i = 0; i < SYS_THREAD_MAX_NUM; ++i)
	{
		//_EXAM_ASSERT(NULL != m_pWorkThread[i]);
		if (m_pWorkThread[i])
		{
			m_pWorkThread[i]->Stop();

			SAFE_DELETE(m_pWorkThread[i]);
		}
	}
}

//创建服务器socket,绑定socket.
bool CNetModule::CreateSever(const CT_CHAR* addr, bool bDataEncrypt)
{
	if (!m_bIsInit)
	{
		return false;
	}
	LOG(INFO) << "Create one server...";

	//设置事件循环过程中定时检查所有描述字状态的时间间隔为50ms
	//m_pIoHandle->set_check_inter(50000);
	// 创建监听异步流
	aio_listen_stream* sstream = new aio_listen_stream(m_pIoHandle);

	// 监听指定的地址
	if (sstream->open(addr) == false)
	{
		LOG(WARNING) << "open: " << addr << " error!";
		sstream->close();
		// XXX: 为了保证能关闭监听流，应在此处再 check 一下
		m_pIoHandle->check();

		getchar();
		return false;
	}

	// 创建回调类对象，当有新连接到达时自动调用此类对象的回调过程
	m_pNetWork = std::make_shared<CNetWork>(m_pWorkThread, m_wWorkThreadCount, bDataEncrypt);
	sstream->add_accept_callback(m_pNetWork.get());

	while (true)
	{
		// 如果返回 false 则表示不再继续，需要退出
		if (m_pIoHandle->check() == false)
		{
			LOG(WARNING) << "server stop now ...";
			break;
		}
	}

	// 关闭监听流并释放流对象
	sstream->close();

	// XXX: 为了保证能关闭监听流，应在此处再 check 一下
	m_pIoHandle->check();

	return true;
}


//创建连接socket
CNetConnector* CNetModule::CreateConnector(const CT_CHAR* addr, bool bDataEncrypt, stConnectParam& connParam)
{
	if (!m_bIsInit)
	{
		return NULL;
	}

	//std::shared_ptr<CNetConnector> pConnector = std::make_shared<CNetConnector>(m_pIoHandle, m_pWorkThread, strAddr.c_str());
	//m_vecConnector.push_back(pConnector);
	//pConnector->ConnectServer();
	//connector只使用一个线程
	CNetConnector* pConnector = new CNetConnector(m_pIoHandle, m_pWorkThread[0], addr, bDataEncrypt, connParam);
	if (pConnector == NULL)
	{
		LOG(ERROR) << "Create one connector: " << addr << " failed.";
		return NULL;
	}
	pConnector->ConnectServer();
	_AUTOMUTEX(&m_mutex);
	m_setConnector.insert(pConnector);
	return pConnector;
}

CNetConnector* CNetModule::AddConnector(const CT_CHAR* addr, bool bDataEncrypt, stConnectParam& connParam)
{
	if (!m_bIsInit)
	{
		return NULL;
	}

	//connector只使用一个线程
	CNetConnector* pConnector = new CNetConnector(m_pIoHandle, m_pWorkThread[0], addr, bDataEncrypt, connParam);
	if (pConnector == NULL)
	{
		LOG(ERROR) << "add one connector: " << addr << " failed.";
		return NULL;
	}
	_AUTOMUTEX(&m_mutex);
	m_setAddConnector.insert(pConnector);

	LOG(INFO) << "add one connector: " << addr << " succeed...";

	return pConnector;
}

CT_VOID CNetModule::AddHttpService(const char* domain, unsigned short port)
{
	/*m_pHttpService = std::make_shared<acl::http_service>();

	// 使消息服务器监听 127.0.0.1 的地址
	if (m_pHttpService->open(m_pIoHandle) == false)
	{
		LOG(ERROR) << "add http service failed.";
	}*/
}

//设置定时器
CT_BOOL CNetModule::SetTimer(CT_DWORD dwTimerID, CT_DWORD dwElapse, CThread* pThread, CT_BOOL bRepeat /*= true*/)
{
	if (m_bIsInit)
	{
		return m_pTimer->SetTimer(dwTimerID, dwElapse, pThread, bRepeat);
	}
	return CT_FALSE;
}

CT_VOID CNetModule::KillTimer(CT_DWORD dwTimerID)
{
	if (m_bIsInit)
	{
		m_pTimer->KillTimer(dwTimerID);
	}
}

CT_VOID CNetModule::Daemonize()
{
#ifndef _WIN32
	pid_t pid;
	int fd = -1;

	/* already a daemon */
	if (getppid() == 1)
		exit(1);

	pid = fork();
	if (pid < 0)
		exit(1);
	else if (pid != 0)
		exit(0);

	/* Cancel certain signals */
	signal(SIGCHLD, SIG_DFL); /* A child process dies */
	signal(SIGTSTP, SIG_IGN); /* Various TTY signals */
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGHUP, SIG_IGN); /* Ignore hangup signal */
	signal(SIGTERM, SIG_DFL); /* Die on SIGTERM */
	signal(SIGPIPE, SIG_IGN);

	/* become session leader */
	if (setsid() < 0)
		exit(1);

	/* change working directory */
	//if (chdir("/") < 0)
	//    exit(1);

	/* clear our file mode creation mask */
	umask(0);

	for (fd = getdtablesize(); fd >= 0; fd--)
		close(fd);

	/* handle standart I/O */
	fd = open("/dev/null", O_RDWR);
	fd = dup(0);
	fd = dup(0);
#endif
}

void CNetModule::InitDaemon()
{
#ifdef _OS_LINUX_CODE
	int nRet = daemon(1, 0);
	printf("daemon return: %d\n", nRet);

	// ignore signals
	signal(SIGINT, SIG_IGN);
	signal(SIGHUP, SIG_IGN);
	signal(SIGQUIT, SIG_IGN);
	signal(SIGPIPE, SIG_IGN);
	signal(SIGTTOU, SIG_IGN);
	signal(SIGTTIN, SIG_IGN);
	signal(SIGTERM, SIG_IGN);
#endif
}

GUID CNetModule::CreateGuid()
{
	GUID guid;
#ifdef WIN32
	CoCreateGuid(&guid);
#else
	uuid_generate(reinterpret_cast<unsigned char *>(&guid));
#endif
	return guid;
}

std::string CNetModule::CreateGuidString()
{
	char buf[64] = { 0 };

#if defined(_OS_LINUX_CODE) || defined(_OS_MAC_CODE)

	uuid_t id;
	uuid_generate(id);
	snprintf(buf, sizeof(buf), "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X", \
		id[0], id[1], id[2], id[3], id[4], id[5], id[6], id[7], id[8], id[9], id[10], id[11], id[12], id[13], id[14], id[15]);

	/*{
		CT_DWORD dwTick = Utility::GetTick();
		std::string strUUID;
		for (int i = 0; i < 10000; i++)
		{
			uuid_unparse(id, buf);
			strUUID = buf;
			std::string strReplace("-");
			std::string::size_type pos = 0;
			std::string::size_type nLen = strReplace.size();
			while ((pos = strUUID.find("-", pos)) != std::string::npos)
			{
				strUUID.erase(pos, 1);
				pos += 1;
			}
		}
		LOG(WARNING) << "time1:" << Utility::GetTick() - dwTick;
		LOG(WARNING) << "UUID1:" << strUUID.c_str();

	}

	{
		CT_DWORD dwTick = Utility::GetTick();
		GUID guid = CreateGuid();

		for (int i = 0; i < 10000; i++)
		{
			snprintf(
				buf,
				sizeof(buf),
				"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
				guid.Data1, guid.Data2, guid.Data3,
				guid.Data4[0], guid.Data4[1],
				guid.Data4[2], guid.Data4[3],
				guid.Data4[4], guid.Data4[5],
				guid.Data4[6], guid.Data4[7]);

			
			if (strcmp(buf, "F33360CBA9AFF90F2937466E2D00BB44")==0)
			{
				std::cout << "fuck";
			}
		}
		LOG(WARNING) << "time2:" << Utility::GetTick() - dwTick;
		LOG(WARNING) << "UUID2:" << buf;
	}
	{
		CT_DWORD dwTick = Utility::GetTick();
		char buf2[64] = { 0 };
		for (int i = 0; i < 10000; i++)
		{
			//for (int i = 0; i < 16; i++)
				//snprintf(buf2 + i * 2, sizeof(buf2), "%02X", id[i]);
				snprintf(buf2, sizeof(buf2), "%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X%02X",\
					id[0],id[1],id[2],id[3],id[4],id[5],id[6],id[7],id[8],id[9],id[10],id[11],id[12],id[13],id[14],id[15]);
		}
		LOG(WARNING) << "time3:" << Utility::GetTick() - dwTick;
		LOG(WARNING) << "UUID3:" << buf2;
	}*/

	//snprintf(
#else // MSVC
	GUID guid = CreateGuid();
	_snprintf_s(
		buf,
		sizeof(buf),
		"%08X%04X%04X%02X%02X%02X%02X%02X%02X%02X%02X",
		guid.Data1, guid.Data2, guid.Data3,
		guid.Data4[0], guid.Data4[1],
		guid.Data4[2], guid.Data4[3],
		guid.Data4[4], guid.Data4[5],
		guid.Data4[6], guid.Data4[7]);
#endif		
	return std::string(buf);
}

CMesBlock* CNetModule::MakeMesBlock(acl::aio_socket_stream* pClient, CT_WORD mainId, CT_WORD subId, const CT_VOID* pBuf, CT_DWORD len, bool bDataEncrypt)
{
	CT_WORD wHeadSize = sizeof(CMD_Command);
	CMesBlock* pBlock = g_MsgBlockPool.NewObj();
	if (pBlock == NULL)
	{
		LOG(ERROR) << "get a null message block.";
		return NULL;
	}
	CT_DWORD         needSize = len + wHeadSize;
	CT_CHAR*        pNewBuf = pBlock->Allocate(needSize);
	if (pNewBuf == NULL)
	{
		LOG(ERROR) << "Allocate message block buf fail.";
		g_MsgBlockPool.DeleteObj(pBlock);
		return NULL;
	}
	pBlock->m_byMsgTag = MSC_S_HANDLE_INFO;
	pBlock->m_pSocketStream = pClient;
	pBlock->m_dwLen = needSize;

	_EXAM_ASSERT(pNewBuf != 0);
	if (pBuf != NULL)
	{
		memset(pNewBuf, 0, needSize);
		static CT_CHAR szBuffer[SYS_NET_SENDBUF_SIZE*2] = { 0 };
		if (bDataEncrypt)
		{
			//memset(szBuffer, 0, sizeof(szBuffer));
			m_encrypt.encrypt((CT_UCHAR*)pBuf, len, (CT_UCHAR*)szBuffer);
			memcpy(pNewBuf + wHeadSize, szBuffer, len);
		}
		else
		{
			memcpy(pNewBuf + wHeadSize, pBuf, len);
		}
	}

	CMD_Command*	 pMesHead = (CMD_Command*)pNewBuf;
	memset(pMesHead, 0, wHeadSize);
	pMesHead->dwDataSize = len;
	pMesHead->dwMainID = mainId;
	pMesHead->dwSubID = subId;

	return pBlock;
}

CMesBlock* CNetModule::MakeMesBlock(acl::aio_socket_stream* pClient, const CT_VOID* pBuf, CT_DWORD len, bool bDataEncrypt)
{
	CMesBlock* pBlock = g_MsgBlockPool.NewObj();
	if (pBlock == NULL)
	{
		LOG(ERROR) << "get a null message block.";
		return NULL;
	}
	CT_CHAR*        pNewBuf = pBlock->Allocate(len + sizeof(CT_DWORD));
	if (pNewBuf == NULL)
	{
		LOG(ERROR) << "Allocate message block buf fail.";
		g_MsgBlockPool.DeleteObj(pBlock);
		return NULL;
	}
	pBlock->m_byMsgTag = MSC_S_HANDLE_INFO;
	pBlock->m_pSocketStream = pClient;
	pBlock->m_dwLen = len + sizeof(CT_DWORD);

	_EXAM_ASSERT(pNewBuf != 0);
	if (pBuf != NULL)
	{
		memset(pNewBuf, 0, len);
		static CT_CHAR szBuffer[SYS_NET_SENDBUF_SIZE * 2] = { 0 };
		if (bDataEncrypt)
		{
			//memset(szBuffer, 0, sizeof(szBuffer));
			m_encrypt.encrypt((CT_UCHAR*)pBuf, len, (CT_UCHAR*)szBuffer);
			memcpy(pNewBuf, &len, sizeof(CT_DWORD));
			memcpy(pNewBuf + sizeof(CT_DWORD), szBuffer, len);
		}
		else
		{
			memcpy(pNewBuf, &len, sizeof(CT_DWORD));
			memcpy(pNewBuf + sizeof(CT_DWORD), pBuf, len);
		}
	}

	return pBlock;
}

CT_BOOL CNetModule::Send(acl::aio_socket_stream* pClient, CT_WORD mainId, CT_WORD subId, const CT_VOID* pBuf, CT_DWORD len)
{
	if (pClient == NULL)
	{
		return false;
	}

	_AUTOMUTEX(&m_mutex);
	auto itClient = m_mapClientSocket.find(pClient);
	if (itClient != m_mapClientSocket.end())
	{
		CNetWork_Io_callback* pNetworkCallback = itClient->second;
		if (pNetworkCallback)
		{
			CMesBlock* pBlock = MakeMesBlock(pClient, mainId, subId, pBuf, len, pNetworkCallback->GetDataEncrypt());
			if (pBlock)
				pNetworkCallback->SendData(pBlock);
			return true;
		}
	}
	else
	{
		auto itServer = m_mapServerSocket.find(pClient);
		if (itServer != m_mapServerSocket.end())
		{
			CConnector_Io_Callback* pConnectorCallback = itServer->second;
			if (pConnectorCallback)
			{
				CMesBlock* pBlock = MakeMesBlock(pClient, mainId, subId, pBuf, len, pConnectorCallback->GetDataEncrypt());
				if (pBlock)
					pConnectorCallback->SendData(pBlock);
				return true;
			}
		}
	}

	//std::cout << "can not find client ... " << std::endl;

	//m_SendDataList.PushBack(pBlock);

	return false;
}

CT_BOOL CNetModule::Send(acl::aio_socket_stream* pClient, CT_WORD mainId, CT_WORD subId)
{
	if (pClient == NULL)
	{
		return false;
	}

	_AUTOMUTEX(&m_mutex);
	auto itClient = m_mapClientSocket.find(pClient);
	if (itClient != m_mapClientSocket.end())
	{
		CNetWork_Io_callback* pNetworkCallback = itClient->second;
		if (pNetworkCallback)
		{
			CMesBlock* pBlock = MakeMesBlock(pClient, mainId, subId, NULL, 0, pNetworkCallback->GetDataEncrypt());
			if (pBlock)
				pNetworkCallback->SendData(pBlock);
			return true;
		}
	}
	else
	{
		auto itServer = m_mapServerSocket.find(pClient);
		if (itServer != m_mapServerSocket.end())
		{
			CConnector_Io_Callback* pConnectorCallback = itServer->second;
			if (pConnectorCallback)
			{
				CMesBlock* pBlock = MakeMesBlock(pClient, mainId, subId, NULL, 0, pConnectorCallback->GetDataEncrypt());
				if (pBlock)
					pConnectorCallback->SendData(pBlock);
				return true;
			}
		}
	}

	//std::cout << "can not find client ... " << std::endl;

	//m_SendDataList.PushBack(pBlock);

	return true;
}

//仅仅与web通讯时使用
CT_BOOL CNetModule::Send(acl::aio_socket_stream* pClient, const CT_VOID* pBuf, CT_DWORD len)
{
	if (pClient == NULL)
	{
		return false;
	}

	std::string strUrlEncodeJson = Utility::urlEncodeCPP(Utility::Ansi2Utf8((const CT_CHAR*)pBuf));
	auto dwLen = strUrlEncodeJson.length();
	//LOG(WARNING) << strUrlEncodeJson;

	_AUTOMUTEX(&m_mutex);
	auto itClient = m_mapClientSocket.find(pClient);
	if (itClient != m_mapClientSocket.end())
	{
		CNetWork_Io_callback* pNetworkCallback = itClient->second;
		if (pNetworkCallback)
		{
			CMesBlock* pBlock = MakeMesBlock(pClient, strUrlEncodeJson.c_str(), (CT_DWORD)dwLen, pNetworkCallback->GetDataEncrypt());
			if (pBlock)
				pNetworkCallback->SendData(pBlock);
			return true;
		}
	}
	else
	{
		auto itServer = m_mapServerSocket.find(pClient);
		if (itServer != m_mapServerSocket.end())
		{
			CConnector_Io_Callback* pConnectorCallback = itServer->second;
			if (pConnectorCallback)
			{
				CMesBlock* pBlock = MakeMesBlock(pClient, strUrlEncodeJson.c_str(), (CT_DWORD)dwLen, pConnectorCallback->GetDataEncrypt());
				if (pBlock)
					pConnectorCallback->SendData(pBlock);
				return true;
			}
		}
	}

	std::cout << "can not find client ... " << std::endl;
	return true;
}


CT_BOOL CNetModule::SendBatch(CT_WORD mainId, CT_WORD subId, const CT_VOID* pBuf, CT_DWORD len)
{
	_AUTOMUTEX(&m_mutex);
	auto it = m_mapClientSocket.begin();
	while (it != m_mapClientSocket.end())
	{
		CNetWork_Io_callback* pNetworkCallback = it->second;
		if (pNetworkCallback)
		{
			CMesBlock* pBlock = MakeMesBlock(it->first, mainId, subId, pBuf, len, pNetworkCallback->GetDataEncrypt());
			if (pBlock)
				pNetworkCallback->SendData(pBlock);
		}
		++it;
	}
	return true;
}

CT_VOID CNetModule::OnEventLoop()
{
	//while (CMesBlock* pBlock = m_SendDataList.GetHead())
	//{
	//	_EXAM_ASSERT(NULL != pBlock);
	//	
	//	pBlock->m_pSocketStream->write(pBlock->m_pBuf, pBlock->m_wLen);
	//	//释放
	//	pBlock->Deallocate();
	//	g_MsgBlockPool.DeleteObj(pBlock);
	//}

	_AUTOMUTEX(&m_mutex);
	for (auto& it: m_mapClientSocket)
	{
		it.second->OnEventSend();
	}

	for (auto& it : m_mapServerSocket)
	{
		it.second->OnEventSend();
	}

	/*auto itAddConnector = m_setAddConnector.begin();
	while (itAddConnector != m_setAddConnector.end())
	{
		CNetConnector* pConnector = *itAddConnector;
		pConnector->ConnectServer();
		m_setConnector.insert(pConnector);
		itAddConnector = m_setAddConnector.erase(itAddConnector);
	}*/
	
	if (!m_setWaitCloseSocket.empty())
	{
		auto itClose = m_setWaitCloseSocket.begin();
		while (itClose != m_setWaitCloseSocket.end())
		{
			acl::aio_socket_stream* pSocket = *itClose;
			itClose = m_setWaitCloseSocket.erase(itClose);
			pSocket->close();
		}
	}

	/*auto itReconnect = m_setDisconnector.begin();
	while (itReconnect != m_setDisconnector.end())
	{
		(*itReconnect)->ConnectServer();
		itReconnect = m_setDisconnector.erase(itReconnect);
	}*/
}

CT_VOID CNetModule::OnEventConnectServer()
{
	_AUTOMUTEX(&m_mutex);
	if (!m_setAddConnector.empty())
	{
		for (auto& it : m_setAddConnector)
		{
			CNetConnector* pConnector = it;
			pConnector->ConnectServer();
			m_setConnector.insert(pConnector);
		}
		m_setAddConnector.clear();
	}

	if (!m_setDisconnector.empty())
	{
		for (auto& it : m_setDisconnector)
		{
			it->ConnectServer();
			LOG(WARNING) << "ready reconnect server, server id: " << it->GetConnectParam().dwServerID;
		}
		m_setDisconnector.clear();
	}
	
	/*auto itClose = m_setWaitCloseSocket.begin();
	while (itClose != m_setWaitCloseSocket.end())
	{
		acl::aio_socket_stream* pSocket = *itClose;
		itClose = m_setWaitCloseSocket.erase(itClose);
		pSocket->close();
	}*/

	/*auto itReconnect = m_setDisconnector.begin();
	while (itReconnect != m_setDisconnector.end())
	{
		(*itReconnect)->ConnectServer();
		itReconnect = m_setDisconnector.erase(itReconnect);
	}*/
}

CT_VOID CNetModule::OnSendHeartBeat()
{
	_AUTOMUTEX(&m_mutex);
	for (auto& it : m_mapClientSocket)
	{
		acl::aio_socket_stream* pClient = it.first;
		CNetWork_Io_callback* pNetWorkCallback = it.second;

		if (pClient == NULL || pNetWorkCallback == NULL)
		{
			LOG(WARNING) << "find a null client in map client!";
			continue;
		}

		CMD_Command heartBeat;
		heartBeat.dwDataSize = 0;
		heartBeat.dwMainID = MSG_HEART_BEAT_MAIN;
		heartBeat.dwSubID = SUB_HEART_BEAT;
		pClient->write(&heartBeat, sizeof(heartBeat));
#ifndef _DEBUG	//Debug不主动关闭连接
		if (Utility::GetTick() -  pNetWorkCallback->GetHeartBeatTime() > 33000)
		{
			//pClient->close();
			CloseSocket(pClient);
		}
#endif
	}
}

#ifdef SOCKETS_NAMESPACE
}
#endif