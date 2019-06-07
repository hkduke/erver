#ifndef		__DEF_MSG_BLOCK_H__
#define		__DEF_MSG_BLOCK_H__
#include "CTType.h"
#include "acl_cpp/stream/aio_socket_stream.hpp"

#ifdef SOCKETS_NAMESPACE
namespace SOCKETS_NAMESPACE {
#endif



//消息块
class CMesBlock
{
public:
	CMesBlock();
	~CMesBlock();
public:
	//申请数据
	CT_CHAR* Allocate(CT_DWORD dwSize);
	//释放数据
	CT_VOID Deallocate(CT_VOID);
public:
	CT_BYTE		m_byMsgTag;						//消息标识 
	CT_DWORD	m_dwLen;						//消息长度
	acl::aio_socket_stream*	m_pSocketStream;	//网络句柄
	CT_CHAR*	m_pBuf;							//消息指针
};

#ifdef SOCKETS_NAMESPACE
}
#endif



#endif		//__DEF_MSG_BLOCK_H__







