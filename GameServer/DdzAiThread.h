#pragma once

//#ifdef _OS_LINUX_CODE
#include "acl_cpp/lib_acl.hpp"
#include "DataQueue.h"
#ifdef _OS_LINUX_CODE
#include "../DdzAi/DdzAiLib/Log.h"
#include "DdzAiStruct.h"
#include <memory>
#endif // _OS_LINUX_CODE


//void get_result(int event, int handle, const pai_interface_move* move);
class CDdzAiThread : public acl::thread
{
public:
	CDdzAiThread();
	~CDdzAiThread();

protected:
	virtual void* run();

#ifdef _OS_LINUX_CODE
public:
	void	InsertAiEvent(std::shared_ptr<ai_event>& aiEvent);
	std::shared_ptr<ai_event> GetFreeAiEeventPtr();

	void	InsertAiAction(std::shared_ptr<ai_action>& aiAction);
	std::shared_ptr<ai_action> GetOneAiActionPtr();

	void	InsertFreeAiActionPtr(std::shared_ptr<ai_action>& aiAction);
	std::shared_ptr<ai_action> GetFreeAiActionPtr();

	std::map<unsigned int, ai_user>& GetHandleForUserMap() { return m_mapAiHandleForUser; }

private:
	//消息块
	CDataQueue<std::shared_ptr<ai_event>>	m_AiEventQue;		//需要处理AI事件
	CDataQueue<std::shared_ptr<ai_event>>	m_AiEventFreeQue;	//空闲的AI事件

	CDataQueue<std::shared_ptr<ai_action>>	m_AiActionQue;			//需要处理的AI回调事件
	CDataQueue<std::shared_ptr<ai_action>>  m_AiActionFreeQue;		//空闲的AI回调事件
	std::map<unsigned int, ai_user>	        m_mapAiHandleForUser;	//handle对应的userid
#endif
};

//#endif