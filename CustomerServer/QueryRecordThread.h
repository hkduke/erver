#ifndef _CHAT_QUERY_RECORD_THREAH_H_
#define _CHAT_QUERY_RECORD_THREAH_H_

#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "DataQueue.h"
#include "CGlobalData.h"
#include <memory>
#include "glog_wrapper.h"

struct tagChatUser
{
	CT_DWORD	dwUserID;
	CT_BYTE     cbMsgType;
	acl::aio_socket_stream* pSocket;

	tagChatUser()
	{
		Reset();
		//LOG(WARNING) << "tagUserLoginInfo()";
	}

	~tagChatUser()
	{
		//LOG(WARNING) << "~tagUserLoginInfo()";
	}

	void Reset()
	{
		dwUserID = 0;
        cbMsgType = 0;
		pSocket = NULL;
	}
};

class CQueryRecordThread : public acl::thread
{
public:
	CQueryRecordThread(bool bAutoDestroy);
	~CQueryRecordThread();

	virtual void* run();

public:
	/************
	插入登录数据
	*************/
	std::shared_ptr<tagChatUser> GetFreeChatUserQue();
	void InsertQueryChatRecord(std::shared_ptr<tagChatUser>& chatUserPtr);
	//发送消息给客户端
	CT_VOID SendMsgToUser(acl::aio_socket_stream* pSocket, CT_DWORD dwUserID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BYTE cbType);

private:
	bool		 m_bAutoDestroy;
	CDataQueue<std::shared_ptr<tagChatUser>>	m_ChatUserQue;		//需要处理登录信息
	CDataQueue<std::shared_ptr<tagChatUser>>	m_ChatUserFreeQue;	//空闲的登录信息
};

#endif