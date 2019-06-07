#pragma once
#include "ServerUserItem.h"
#include "IAndroidUserItemSink.h"
#include "TableFrameForScore.h"

class CAndroidUserItem : public CServerUserItem
{
public:
	CAndroidUserItem();
	~CAndroidUserItem();

public:
	virtual bool IsAndroid() { return true; }
	virtual IAndroidUserItemSink* GetAndroidUserItemSink() { return m_pAndroidUserSink; }
	//重置数据
	virtual void ResetUserItem();
	//定时器脉冲
	virtual void OnTimePulse();
	//设置定时器
	virtual bool SetTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam = 0);
	//销毁定时器
	virtual bool KillTimer(CT_WORD wTimerID);
	//判断是否有定时器
	virtual bool IsHasTimer(CT_WORD wTimerID);

	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID);
	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len);

	//设置打了多少局
	virtual void AddPlayCount();
	//获取打了多少局
	virtual CT_WORD GetPlayCount();

	//设置大额度机器人
	virtual CT_VOID SetBigJettionAndroid(CT_BYTE cbBigJetton) { m_cbBigJetton = cbBigJetton; }
	//获得是否大额度机器人
	virtual CT_BYTE GetBigJettonAndroid() { return m_cbBigJetton; }

public:
	void SetAndroidUserItemSink(IAndroidUserItemSink* pSink);
	
private:
	IAndroidUserItemSink*	m_pAndroidUserSink;
	CT_WORD					m_wPlayCount;
	CT_BYTE 				m_cbBigJetton;

	//定时器列表
	TimerInfo	            m_SysTimer[DW_SYS_TIMER_NUMS];
};