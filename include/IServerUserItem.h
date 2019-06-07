#pragma once

class IAndroidUserItemSink;
class IServerUserItem
{
public:
	IServerUserItem() {}
	virtual ~IServerUserItem() {}

public:
	//是否机器人
	virtual bool IsAndroid() = 0;
	//获取用户回调对象
	virtual IAndroidUserItemSink* GetAndroidUserItemSink() = 0;
	//重置数据
	virtual void ResetUserItem() = 0;
	//定时器脉冲
	virtual void OnTimePulse() = 0;
	//设置定时器
	virtual bool SetTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam = 0) = 0;
	//销毁定时器
	virtual bool KillTimer(CT_WORD wTimerID) = 0;
	//判断是否有定时器
	virtual bool IsHasTimer(CT_WORD wTimerID) = 0;
	//发送用户信息
	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID) = 0;
	//发送用户信息
	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len) = 0;
	//设置打了多少局
	virtual void AddPlayCount() = 0;
	//获取打了多少局
	virtual CT_WORD GetPlayCount() = 0;
	//得到椅子id
	virtual CT_WORD GetChairID() = 0;
	//设置大额度机器人
	virtual CT_VOID SetBigJettionAndroid(CT_BYTE cbBigJetton) = 0;
	//获得是否大额度机器人
	virtual CT_BYTE GetBigJettonAndroid() = 0;
};