#pragma once
#include <memory>

//机器人回调接口
class ITableFrame;
class IServerUserItem;
class CDdzAiThread;
struct ai_action;
class IAndroidUserItemSink
{
public:
	IAndroidUserItemSink() {}
	virtual ~IAndroidUserItemSink() {}

public:
	//重置接口
	virtual void RepositionSink() = 0;
	//初始接口
	virtual void Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pUserItem) = 0;
	//设置斗地主的AI线程
	virtual void SetDdzAiThread(CDdzAiThread* pAiThread) = 0;
	//设置斗地主AI处理后的数据
	virtual void HandleDdzAiAction(std::shared_ptr<ai_action>& aiAction) = 0;

	//游戏事件
public:
	//定时器事件
	virtual bool OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam) = 0;
	//游戏事件
	virtual bool OnGameMessage(CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize) = 0;
};

typedef IAndroidUserItemSink*		(*LpDllNewAndroid)();							//生成机器人
typedef void*						(*LpDllDelAndroid)(IAndroidUserItemSink* p);	//删除机器人

#define DLL_NEW_ANDROID_FN_NAME		("GetAndroidInstance")							//得到桌子的实例
#define DLL_DEL_ANDROID_FN_NAME		("DelAndroidInstance")							//删除桌子的实例