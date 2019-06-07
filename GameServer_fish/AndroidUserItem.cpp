#include "AndroidUserItem.h"

CAndroidUserItem::CAndroidUserItem()
	:m_pAndroidUserSink(NULL)
	,m_wPlayCount(0)
{

}


CAndroidUserItem::~CAndroidUserItem()
{

}
//重置数据
void CAndroidUserItem::ResetUserItem()
{
	m_wPlayCount = 0;
	CServerUserItem::ResetUserItem();
}

//定时器脉冲
void CAndroidUserItem::OnTimePulse()
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (!m_SysTimer[i].isEnabled)
		{//无效
			continue;
		}
		m_SysTimer[i].iCountTime += 1000;
		if (m_SysTimer[i].iCountTime >= m_SysTimer[i].iTimeInterval)
		{
			m_SysTimer[i].iCountTime = m_SysTimer[i].iCountTime - m_SysTimer[i].iTimeInterval;
			m_pAndroidUserSink->OnTimerMessage(m_SysTimer[i].wTimerID, m_SysTimer[i].dwParam);
		}
	}
}

//设置定时器
bool CAndroidUserItem::SetTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam/* = 0*/)
{
	if (dwTime <= 0)
	{
		return false;
	}
	CT_WORD iSaveFirstIndex = DW_SYS_TIMER_NUMS;
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (m_SysTimer[i].wTimerID == wTimerID)
		{
			m_SysTimer[i].iTimeInterval = dwTime;
			m_SysTimer[i].iCountTime = 0;
			m_SysTimer[i].isEnabled = CT_TRUE;
			m_SysTimer[i].dwParam = dwParam;
			return true;
		}
		else if (!m_SysTimer[i].isEnabled && DW_SYS_TIMER_NUMS == iSaveFirstIndex)
		{
			iSaveFirstIndex = i;
		}
	}
	if (iSaveFirstIndex < DW_SYS_TIMER_NUMS)
	{
		m_SysTimer[iSaveFirstIndex].wTimerID = wTimerID;
		m_SysTimer[iSaveFirstIndex].iTimeInterval = dwTime;
		m_SysTimer[iSaveFirstIndex].iCountTime = 0;
		m_SysTimer[iSaveFirstIndex].isEnabled = true;
		m_SysTimer[iSaveFirstIndex].dwParam = dwParam;
		return true;
	}
	return false;
}

//销毁定时器
bool CAndroidUserItem::KillTimer(CT_WORD wTimerID)
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (m_SysTimer[i].wTimerID == wTimerID)
		{
			m_SysTimer[i].wTimerID = -1;
			m_SysTimer[i].iTimeInterval = 0;
			m_SysTimer[i].iCountTime = 0;
			m_SysTimer[i].isEnabled = false;
			m_SysTimer[i].dwParam = 0;
			return true;
		}
	}
	return false;
}

//判断是否有定时器
bool CAndroidUserItem::IsHasTimer(CT_WORD wTimerID)
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (m_SysTimer[i].wTimerID == wTimerID)
		{
			return true;
		}
	}
	return false;
}

void CAndroidUserItem::SendUserMessage(CT_DWORD mainID, CT_DWORD subID)
{

}

void CAndroidUserItem::SendUserMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len)
{
	if (m_pAndroidUserSink)
	{
		m_pAndroidUserSink->OnGameMessage(subID, pBuf, len);
	}
}

void CAndroidUserItem::SetAndroidUserItemSink(IAndroidUserItemSink* pSink)
{
	m_pAndroidUserSink = pSink;
}

//设置打了多少局
void CAndroidUserItem::AddPlayCount()
{
	++m_wPlayCount;
}
//获取打了多少局
CT_WORD CAndroidUserItem::GetPlayCount() 
{ 
	return m_wPlayCount; 
}