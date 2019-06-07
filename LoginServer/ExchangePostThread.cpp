#include "ExchangePostThread.h"
#include "CTType.h"
#include "Utility.h"
#include "glog_wrapper.h"
#include "ServerCfg.h"
#include "httplib.h"

CExchangePostThread::CExchangePostThread(bool bAutoDestroy)
	: m_bAutoDestroy(bAutoDestroy)
{

}

CExchangePostThread::~CExchangePostThread()
{

}


std::shared_ptr<tagExchangePost> CExchangePostThread::GetFreeExchangePostQue()
{
	if (m_ExchangePostFreeQue.GetSize() == 0)
	{
		return std::make_shared<tagExchangePost>();
	}

	return m_ExchangePostFreeQue.GetHead();
}


void CExchangePostThread::InsertExchangePostQue(std::shared_ptr<tagExchangePost> &exchangePostPtr)
{
    m_ExchangePostQue.PushBack(exchangePostPtr);
}

void* CExchangePostThread::run()
{
	while (true)
	{
		if (m_ExchangePostQue.GetSize() == 0)
		{
			Utility::Sleep(100);
			continue;
		}

		std::shared_ptr<tagExchangePost> exchangePostPtr = m_ExchangePostQue.GetHead();

        httplib::Client cli(CServerCfg::m_RechargePostAddress);

        char szParam[64] = {0};
        _snprintf_info(szParam, sizeof(szParam), "id=%d&platform=%u", exchangePostPtr->nExchangeId, exchangePostPtr->nPlatformId);
        std::string strParam(szParam);
        auto res = cli.Post("/chat/setOrderAdmin", strParam, "application/x-www-form-urlencoded");
        if (res)
        {
            if (res->body.find("\"code\":0") != std::string::npos)
            {
                //插入空闲队列
                exchangePostPtr->Reset();
                m_ExchangePostFreeQue.PushBack(exchangePostPtr);
                continue;
            }
        }

        m_ExchangePostQue.PushBack(exchangePostPtr);
	}

	if (m_bAutoDestroy)
		delete this;

	return NULL;
}