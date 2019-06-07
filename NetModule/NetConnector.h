#pragma once
#include <string>
#include "CTData.h"
//#include "lib_acl.h"
#include "acl_cpp/acl_cpp_init.hpp"
#include "acl_cpp/stdlib/string.hpp"
#include "acl_cpp/stdlib/util.hpp"
#include "acl_cpp/stream/aio_handle.hpp"
#include "acl_cpp/stream/aio_socket_stream.hpp"
#include "ReadWriteBuffer.h"
#include "WorkThread.h"
#include "GlobalEnum.h"
#include "glog_wrapper.h"
#include "NetEncrypt.h"
using namespace acl;

struct stConnectParam
{
	ConnectType conType;
	CT_DWORD	dwServerID;
};


enum CONNECTSTATUS 
{
	ENULL = 1,			//无状态
	EConnecting = 2,	//正在进行连接
	EConnected = 3,		//连接成功
};

class CNetConnector;
/**
* 客户端异步连接流回调函数类
*/
class CConnector_Io_Callback : public aio_open_callback
{
	friend class CNetConnector;
	/*struct sTest
	{
		char szBuf[16];
	};*/

public:
	CConnector_Io_Callback(aio_socket_stream* pClient, CNetConnector* pConnector, bool bDataEncrypt)
		: m_readStatus(STATUS_T_HDR), m_client(pClient), m_Connector(pConnector), m_bDataEncrypt(bDataEncrypt)
	{
		m_inputStream.Init();
		m_outputStream.Init();
	}

	~CConnector_Io_Callback()
	{
	}

	/**
	* 基类虚函数, 当异步流读到所要求的数据时调用此回调函数
	* @param data {char*} 读到的数据地址
	* @param len {int｝ 读到的数据长度
	* @return {bool} 返回给调用者 true 表示继续，否则表示需要关闭异步流
	*/
	bool read_callback(char* data, int len);

	/**
	* 基类虚函数, 当异步流写成功时调用此回调函数
	* @return {bool} 返回给调用者 true 表示继续，否则表示需要关闭异步流
	*/
	bool write_callback()
	{
		return true;
	}

	/**
	* 基类虚函数, 当该异步流关闭时调用此回调函数
	*/
	void close_callback();

	/**
	* 基类虚函数，当异步流超时时调用此函数
	* @return {bool} 返回给调用者 true 表示继续，否则表示需要关闭异步流
	*/
	bool timeout_callback()
	{
		m_client->close();
		return (false);
	}

	/**
	* 基类虚函数, 当异步连接成功后调用此函数
	* @return {bool} 返回给调用者 true 表示继续，否则表示需要关闭异步流
	*/
	bool open_callback();

	/**
	* 由主线程定时器定时回调发送数据
	*/
	void SendData(CMesBlock* pBlock);

	/**
	* 由主线程定时器定时回调发送数据
	*/
	bool OnEventSend(void);

	/**
	* 把数据插入到输出流
	*/
	void DataToOutstream(void);

	/**
	* 清空缓冲数据
	*/
	void ClearData(void);

	/**
	* 设置数据是否加密
	*/
	void SetDataEncrypt(bool bEncrypt) { m_bDataEncrypt = bEncrypt; }

	/**
	* 获取数据是否加密
	*/
	bool GetDataEncrypt() { return m_bDataEncrypt; }

private:
	status_t			m_readStatus;
	aio_socket_stream*	m_client;
	CNetConnector*		m_Connector;

	// 输入流
	ReadWriteBuffer			m_inputStream;
	//输出流
	ReadWriteBuffer			m_outputStream;
	//发送数据队列
	CDataQueue<CMesBlock*>	m_SendDataList;

	Encrypt					m_encrypt;
	bool					m_bDataEncrypt;

	static	CT_DWORD		m_dwOpenCount;	//连接次数
};

struct stConnectorPtr
{
	CNetConnector *pConnector;
};

class CNetConnector
{
	friend class CConnector_Io_Callback;
public:
	CNetConnector(aio_handle* pHandle, CWorkThread* pThread, const char* addr, bool bDataEncrypt, stConnectParam conParam)
		:m_pHandle(pHandle)
		, m_pSocket(NULL)
		, m_pWorkThread(pThread)
		, m_strAddr(addr)
		, m_connectParam(conParam)
		//, m_bRunning(false)
		, m_enConnectStatus(ENULL)
		, m_bDataEncrypt(bDataEncrypt)
	{}
	~CNetConnector() {}

	/**
	* 异步连接远程服务器
	* @param addr {const char*} 远程服务器地址，格式"192.168.1.1:3306"
	*/
	bool ConnectServer();

	/**
	* 获取连接sockets
	*/
	acl::aio_socket_stream* GetSocket()
	{
		return m_pSocket;
	}

	/**
	* 连接是否正常
	*/
	bool IsRunning()
	{
		//其他状态都不是连接状态
		return (m_enConnectStatus == EConnected);
	}


	/**
	* 连接是否关闭
	*/
	bool IsClose()
	{
		//其他状态都不是连接状态
		return (m_enConnectStatus == ENULL);
	}

	/**
	* 其他链接参数
	*/
	stConnectParam& GetConnectParam()
	{
		return m_connectParam;
	}

	/**
	* 设置其他链接参数
	*/
	void SetConnectParam(stConnectParam& connectParam)
	{
		m_connectParam = connectParam;
	}

private:
	aio_handle*			m_pHandle;
	aio_socket_stream*	m_pSocket;
	CWorkThread*		m_pWorkThread;
	std::string			m_strAddr;
	stConnectParam		m_connectParam;
	//bool				m_bRunning;
	CONNECTSTATUS		m_enConnectStatus;
	bool				m_bDataEncrypt;
};