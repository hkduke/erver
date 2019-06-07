#pragma once
#include <iostream>
#include "CTData.h"
//#include "lib_acl.h"
#include "acl_cpp/acl_cpp_init.hpp"
#include "acl_cpp/stdlib/log.hpp"
#include "acl_cpp/stream/aio_handle.hpp"
#include "acl_cpp/stream/aio_istream.hpp"
#include "acl_cpp/stream/aio_listen_stream.hpp"
#include "acl_cpp/stream/aio_socket_stream.hpp"
#include "ReadWriteBuffer.h"
#include "sysdata.h"
#include "ObjPool.h"
#include "MesBlock.h"
#include "WorkThread.h"
#include "Utility.h"
#include "NetEncrypt.h"

/**
* 异步监听流的回调类的子类
*/
class CNetWork : public acl::aio_accept_callback
{
public:
	CNetWork(CWorkThread** pWorkThread, CT_WORD wWorkThreadNum, bool bDataEncrypt)
	: m_pWorkThread(pWorkThread)
	, m_wWorkThreadNum(wWorkThreadNum)
	, m_bDataEncrypt(bDataEncrypt)
	{

	}

	~CNetWork()
	{
		std::cout << "network over!" << std::endl;
	}

	/**
	* 基类虚函数，当有新连接到达后调用此回调过程
	* @param client {aio_socket_stream*} 异步客户端流
	* @return {bool} 返回 true 以通知监听流继续监听
	*/
	bool accept_callback(acl::aio_socket_stream* client);

private:
	CWorkThread**	m_pWorkThread;
	CT_WORD 		m_wWorkThreadNum;
	bool			m_bDataEncrypt;
};

class CNetWork_Io_callback : public acl::aio_callback
{
public:
	CNetWork_Io_callback(acl::aio_socket_stream* client, CWorkThread** pWorkThread, CT_WORD wWorkThreadNum, bool bDataEncrypt)
		: m_readStatus(STATUS_T_HDR)
		, m_client(client)
		, m_pWorkThread(pWorkThread)
		, m_wWorkThreadNum(wWorkThreadNum)
		, m_wCurrentWorkThread(0)
		, m_dwRecvTickCount(0)
		, m_bDataEncrypt(bDataEncrypt)
	{
		m_inputStream.Init();
		m_outputStream.Init();
		m_dwRecvTickCount = Utility::GetTick();
	}

	~CNetWork_Io_callback()
	{
		//std::cout << "delete network io_callback now ..." << std::endl;
	}

	/**
	* 实现父类中的虚函数，客户端流的读成功回调过程
	* @param data {char*} 读到的数据地址
	* @param len {int} 读到的数据长度
	* @return {bool} 返回 true 表示继续，否则希望关闭该异步流
	*/
	virtual bool read_callback(char* data, int len);

	/**
	* 实现父类中的虚函数，客户端流的写成功回调过程
	* @return {bool} 返回 true 表示继续，否则希望关闭该异步流
	*/
	virtual bool write_callback()
	{
		return true;
	}

	/**
	* 实现父类中的虚函数，客户端流的关闭回调过程
	*/
	virtual void close_callback();

	/**
	* 实现父类中的虚函数，客户端流的超时回调过程
	* @return {bool} 返回 true 表示继续，否则希望关闭该异步流
	*/
	virtual bool timeout_callback()
	{
		std::cout << "Timeout, delete it ..." << std::endl;
		return false;
	}

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
	* 获取心跳时间 
	*/
	CT_DWORD GetHeartBeatTime() { return m_dwRecvTickCount; }

	/**
	* 获取数据是否加密
	*/
	bool GetDataEncrypt() { return m_bDataEncrypt; }

private:
	status_t m_readStatus;
	acl::aio_socket_stream* m_client;
	CWorkThread**			m_pWorkThread;
	CT_WORD 				m_wWorkThreadNum;
	int 					m_wCurrentWorkThread;

	// 输入流
	ReadWriteBuffer			m_inputStream;
	//输出流
	ReadWriteBuffer			m_outputStream;
	//发送数据队列
	CDataQueue<CMesBlock*>	m_SendDataList;

	CT_DWORD				m_dwRecvTickCount;
	Encrypt					m_encrypt;
	bool					m_bDataEncrypt;
};

struct stNetWorkPtr
{
	acl::aio_socket_stream* pSocket;
};