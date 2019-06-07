#ifndef		__DEF_NET_MODULE_H__
#define		__DEF_NET_MODULE_H__
#include <map>
#include <set>
#include "CTType.h"
#include "Thread.h"
#include "Singleton.h"
#include "DataQueue.h"
#include "NetServer.h"
#include "NetConnector.h"
#include <memory>
#include "WorkThread.h"
#include "Timer.h"
#include "MemPool.h"
#include "glog_wrapper.h"
#include "GlobalEnum.h"
#include "NetEncrypt.h"

#if defined(WIN32) || defined(_WIN32)
#include <objbase.h>
#else
#include <uuid/uuid.h>
typedef struct _GUID
{
	unsigned long Data1;
	unsigned short Data2;
	unsigned short Data3;
	unsigned char Data4[8];
} GUID, UUID;
#endif


#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

class CNetModule : public CCSingleton<CNetModule>
{
	//线程常量
#define          SYS_THREAD_MAX_NUM             (16)                     //工作线程最大数目
#define          SYS_THREAD_MIN_NUM             (1)                      //工作线程最小数目

public:
	CNetModule(acl::aio_handle*	pIoHandle);
	~CNetModule();

public:
	//初始化网络模块
	template<class T>
	CT_VOID InitNetModule(CT_WORD wWorkThreadNums)
	{
		if (!m_bIsInit)
		{
			if (wWorkThreadNums >= SYS_THREAD_MAX_NUM)
			{
				wWorkThreadNums = SYS_THREAD_MAX_NUM;
			}

			//char szBuff[10] = { "123abc" };
			//m_encrypt.encrypt()

			InitOther();

			// 初始化ACL库(尤其是在WIN32下一定要调用此函数，在UNIX平台下可不调用)
			acl::acl_cpp_init();

			m_pTimer = new CTimer();
			_EXAM_ASSERT(m_pTimer != NULL);

			m_bIsInit = CT_TRUE;
			LOG(INFO) << "Init NetModule Finished.";

			for (int i = 0; i < wWorkThreadNums; ++i)
			{
				m_pWorkThread[i] = new T;
				_EXAM_ASSERT(m_pWorkThread[i] != NULL);
				++m_wWorkThreadCount;
				LOG(INFO) << "start work thread " << i << "...";
			}
		}
	}

	//运行网络模块
	CT_VOID RunNetModule(CT_VOID);

	//停止网络模块
	CT_VOID StopNetModule(CT_VOID);

	//创建服务器
	bool CreateSever(const CT_CHAR* addr, bool bDataEncrypt);
	//创建连接（只能在主线程调用）
	CNetConnector* CreateConnector(const CT_CHAR* addr, bool bDataEncrypt, stConnectParam& connParam);
	//增加连接
	CNetConnector* AddConnector(const CT_CHAR* addr, bool bDataEncrypt, stConnectParam& connParam);
	//增加httpclient
	CT_VOID AddHttpService(const char* domain, unsigned short port);
	//停止线程
	CT_VOID StopWorkThread(CT_VOID);

	//获得线程
	//CThread* GetWorkThread() { return m_pWorkThread; }

	//发送数据
	inline CMesBlock* MakeMesBlock(acl::aio_socket_stream* pClient, CT_WORD mainId, CT_WORD subId, const CT_VOID* pBuf, CT_DWORD len, bool bDataEncrypt);
	inline CMesBlock* MakeMesBlock(acl::aio_socket_stream* pClient, const CT_VOID* pBuf, CT_DWORD len, bool bDataEncrypt);
	CT_BOOL Send(acl::aio_socket_stream* pClient,  CT_WORD mainId, CT_WORD subId, const CT_VOID* pBuf, CT_DWORD len);
	CT_BOOL Send(acl::aio_socket_stream* pClient, CT_WORD mainId, CT_WORD subId);
	CT_BOOL Send(acl::aio_socket_stream* pClient, const CT_VOID* pBuf, CT_DWORD len);//仅仅与web通讯时使用
	CT_BOOL SendBatch(CT_WORD mainId, CT_WORD subId, const CT_VOID* pBuf, CT_DWORD len);
	CT_VOID OnEventLoop();
	CT_VOID OnEventConnectServer();
	CT_VOID OnSendHeartBeat();
	
public:
	//设置定时器
	CT_BOOL SetTimer(CT_DWORD dwTimerID, CT_DWORD dwElapse, CThread* pThread, CT_BOOL bRepeat = true);
	//销毁定时器
	CT_VOID KillTimer(CT_DWORD dwTimerID);

public:
	//设置后台运行
	CT_VOID Daemonize();
	void InitDaemon();

public:
	inline GUID CreateGuid();
	std::string CreateGuidString();

	CT_VOID SetServerType(ServerType serverType) { m_serverType = serverType; }
	ServerType GetServerType() { return m_serverType; }

public:
	void InsertClientSocket(acl::aio_socket_stream* pClient, CNetWork_Io_callback* ioCallback)
	{
		_AUTOMUTEX(&m_mutex);
		m_mapClientSocket[pClient] = ioCallback;
	}

	void DeleteClientSocket(acl::aio_socket_stream* pClient)
	{
		_AUTOMUTEX(&m_mutex);
		m_mapClientSocket.erase(pClient);
		if (m_setWaitCloseSocket.find(pClient) != m_setWaitCloseSocket.end())
		{
			m_setWaitCloseSocket.erase(pClient);
		}
	}

	void InsertServerSocket(acl::aio_socket_stream* pClient, CConnector_Io_Callback* ioCallback)
	{
		_AUTOMUTEX(&m_mutex);
		m_mapServerSocket[pClient] = ioCallback;
	}

	void DeleteServerSocket(acl::aio_socket_stream* pClient)
	{
		_AUTOMUTEX(&m_mutex);
		auto it = m_mapServerSocket.find(pClient);
		if (it != m_mapServerSocket.end())
		{
			m_mapServerSocket.erase(it);
		}
		
		if (m_setWaitCloseSocket.find(pClient) != m_setWaitCloseSocket.end())
		{
			m_setWaitCloseSocket.erase(pClient);
		}
	}

	void InsertReconnector(CNetConnector* pConnector)
	{
		_AUTOMUTEX(&m_mutex);
		m_setDisconnector.insert(pConnector);
	}

	void DeleteConnector(CNetConnector* pConnector)
	{
		_AUTOMUTEX(&m_mutex);
		auto it = m_setConnector.find(pConnector);
		if (it != m_setConnector.end())
		{
			m_setConnector.erase(it);
		}

		auto itAdd = m_setAddConnector.find(pConnector);
		if (itAdd != m_setAddConnector.end())
		{
			m_setAddConnector.erase(itAdd);
		}
		
		auto itDisconnect = m_setDisconnector.find(pConnector);
		if (itDisconnect != m_setDisconnector.end())
		{
			m_setDisconnector.erase(itDisconnect);
		}
	}

	void CloseSocket(acl::aio_socket_stream* pSocket)
	{
		if (pSocket != NULL)
		{
			_AUTOMUTEX(&m_mutex);
			auto itClient = m_mapClientSocket.find(pSocket);
			if (itClient != m_mapClientSocket.end())
			{
				m_setWaitCloseSocket.insert(pSocket);
				return;
			}

			auto itServer = m_mapServerSocket.find(pSocket);
			if (itServer != m_mapServerSocket.end())
			{
				m_setWaitCloseSocket.insert(pSocket);
				return;
			}
		}
	}

	std::string GetRemoteIp(acl::aio_socket_stream* pSocket, bool bFull = true)
	{
		_AUTOMUTEX(&m_mutex);
		auto it = m_mapClientSocket.find(pSocket);
		if (it != m_mapClientSocket.end())
		{
			return std::string(pSocket->get_peer(bFull));
		}

		return std::string("");
	}

private:
	//初始化其它
	CT_VOID InitOther(CT_VOID);

private:
	CWorkThread*			m_pWorkThread[SYS_THREAD_MAX_NUM];
	CT_WORD 				m_wWorkThreadCount;
	bool					m_bIsInit;
	CTMutex					m_mutex;
	Encrypt					m_encrypt;
	ServerType				m_serverType;

	std::shared_ptr<CNetWork>	m_pNetWork;
	std::set<CNetConnector*>	m_setConnector;				// 已经连接了conn
	std::set<CNetConnector*>	m_setAddConnector;			// 有待增加的conn						
	std::set<CNetConnector*>	m_setDisconnector;			// 等待重连的connector
	std::set<acl::aio_socket_stream*> m_setWaitCloseSocket; // 等待关闭的socket

	//std::shared_ptr<acl::http_service> m_pHttpService;
	//std::map<std::string, std::string> m_mapHttpRequest;

	acl::aio_handle*			m_pIoHandle;
	CTimer*						m_pTimer;

	//输出流
	//ReadWriteBuffer				m_outputStream;
	//发送数据队列
	//CDataQueue<CMesBlock*>		m_SendDataList;

	// 所有的客户端连接
	//CT_UINT64	m_ClientSocksIndex;
	std::map<acl::aio_socket_stream*, CNetWork_Io_callback*>	m_mapClientSocket;
	//std::map<CT_UINT64, acl::aio_socket_stream*>				m_mapClientSocketIndex;
	// 所有的服务器连接
	std::map<acl::aio_socket_stream*, CConnector_Io_Callback*>	m_mapServerSocket;
};

#ifdef SOCKETS_NAMESPACE
}
#endif

#endif		//__DEF_NET_MODULE_H__













