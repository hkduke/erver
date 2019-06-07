#ifndef _exchange_post_h_
#define _exchange_post_h_

#include "acl_cpp/lib_acl.hpp"
#include "lib_acl.h"
#include "DataQueue.h"
#include <memory>

struct tagExchangePost
{
	int			 	nPlatformId;
	unsigned int	nExchangeId;

    tagExchangePost()
	{
		Reset();
	}

	void Reset()
	{
		nPlatformId = 0;
		nExchangeId = 0;
	}
};

class CExchangePostThread : public acl::thread
{
	friend class CLoginServerThread;
public:
	CExchangePostThread(bool bAutoDestroy);
	~CExchangePostThread();

	virtual void* run();

public:
	/************
	插入验证码数据
	*************/
	std::shared_ptr<tagExchangePost> GetFreeExchangePostQue();
	void InsertExchangePostQue(std::shared_ptr<tagExchangePost>& exchangePostPtr);

private:
	bool		 m_bAutoDestroy;
	CDataQueue<std::shared_ptr<tagExchangePost>>	m_ExchangePostQue;		//需要处理的验证码信息
	CDataQueue<std::shared_ptr<tagExchangePost>>	m_ExchangePostFreeQue;	//空闲的验证码信息
};

#endif