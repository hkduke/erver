#ifndef		__DEF_WORK_THREAD_DEF_H__
#define		__DEF_WORK_THREAD_DEF_H__

#include "Thread.h"
#include "CTType.h"
#include "DataQueue.h"
#include "CTData.h"
#include "acl_cpp/stream/aio_socket_stream.hpp"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif

class CMesBlock;
class CNetConnector;

//工作线程
class CWorkThread : public CThread
{
public:
	CWorkThread();
	~CWorkThread();

public:
	//打开
	virtual CT_VOID Open();
	//运行
	virtual CT_VOID Run();

public:
	//网络连接
	virtual CT_VOID OnTCPSocketLink(CNetConnector* pConnector) = 0;
	//网络连接关闭
	virtual CT_VOID OnTCPSocketShut(CNetConnector* pConnector) = 0;
	//网络应答
	virtual CT_VOID OnTCPNetworkBind(acl::aio_socket_stream* pSocket) = 0;
	//网络关闭
	virtual CT_VOID OnTCPNetworkShut(acl::aio_socket_stream* pSocket) = 0;
	//网络数据到来
	virtual CT_VOID OnNetEventRead(acl::aio_socket_stream* pStream, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen) = 0;
	//网络定时器
	virtual CT_VOID OnTimer(CT_DWORD dwTimerID) = 0;

public:
	//插入消息块
	CT_VOID InsertMseBlock(CMesBlock* pBlock);

private:
	//消息块
	CDataQueue<CMesBlock*>	m_MesBlockQue;

};





#ifdef SOCKETS_NAMESPACE
}
#endif


#endif			//__DEF_WORK_THREAD_DEF_H__










