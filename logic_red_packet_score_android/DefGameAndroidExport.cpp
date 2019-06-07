#include "DefGameAndroidExport.h"
#include "AndroidUserItemSink.h"


//得到桌子实例
extern "C" IAndroidUserItemSink* GetAndroidInstance()
{
	IAndroidUserItemSink* pSink = new CAndroidUserItemSink();
	return pSink;
}

//删除桌子实例
extern "C" void DelAndroidInstance(IAndroidUserItemSink* pIAndroidUserItemSink)
{
	if (nullptr != pIAndroidUserItemSink)
	{
		delete pIAndroidUserItemSink;
	}
	pIAndroidUserItemSink = nullptr;
}