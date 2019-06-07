#pragma once
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "Thread.h"
#include <map>
#include "Mutex.h"

struct TimerEvent
{
	CT_DWORD	id;
	CT_DWORD	elapse;
	CT_DWORD	lastTime;
	CT_BOOL		bRepeats; 
	CThread		*pThread;

	TimerEvent()
	{
		memset(this, 0, sizeof(*this));
	}
}; 

#define IDI_TIMER_LOOP				0
#define IDI_TIMER_EVENT				1
#define IDI_TIMER_RECONNECT_SERVER	2
#define IDI_TIMER_HEART_BEAT		3

#ifdef _OS_MAC_CODE
#define TIME_LOOP					20000
#define TIME_TIMER_EVENT			100000
#define TIME_RECONNECT_SERVER		500000
#define TIME_HEART_BEAT				10000000
#else
#define TIME_LOOP					20000
#define TIME_TIMER_EVENT			100000
#define TIME_RECONNECT_SERVER		500000
#define TIME_HEART_BEAT				10000000
#endif

class CTimer : public acl::aio_timer_callback
{
public:
	CTimer() : aio_timer_callback(true) {}
	~CTimer() {}
protected:
	// 基类纯虚函数
	virtual void timer_callback(unsigned int id);

	virtual void destroy(void)
	{
		delete this;
	}

public:
	CT_BOOL	SetTimer(CT_DWORD timerId, CT_DWORD elapse, CThread* pThread, CT_BOOL bRepeat);
	CT_VOID KillTimer(CT_DWORD timerId);

private:
	typedef std::map<CT_DWORD, TimerEvent>	TIMEITEMMAP;
	TIMEITEMMAP		m_TimeMap;					//正在执行的定时器
	TIMEITEMMAP		m_waitAddTimeMap;			//等待添加的定时器
	CTMutex			m_mutex;
};