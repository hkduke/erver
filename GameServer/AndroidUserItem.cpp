#include "AndroidUserItem.h"
#include "AndroidUserMgr.h"

CAndroidUserItem::CAndroidUserItem()
	:m_pAndroidUserSink(NULL)
	,m_wPlayCount(0)
	,m_cbBigJetton(0)
{
	memset(&m_SysTimer, 0, sizeof(m_SysTimer));
}


CAndroidUserItem::~CAndroidUserItem()
{

}
//��������
void CAndroidUserItem::ResetUserItem()
{
	m_wPlayCount = 0;
	CServerUserItem::ResetUserItem();
}

//��ʱ������
void CAndroidUserItem::OnTimePulse()
{
	for (CT_WORD i = 0; i < DW_SYS_TIMER_NUMS; ++i)
	{
		if (!m_SysTimer[i].isEnabled)
		{//��Ч
			continue;
		}

		m_SysTimer[i].iCountTime += CAndroidUserMgr::get_instance().GetAndroidTimePlus();
		if (m_SysTimer[i].iCountTime >= m_SysTimer[i].iTimeInterval)
		{
			m_SysTimer[i].iCountTime = m_SysTimer[i].iCountTime - m_SysTimer[i].iTimeInterval;
			m_pAndroidUserSink->OnTimerMessage(m_SysTimer[i].wTimerID, m_SysTimer[i].dwParam);
		}
	}
}

//���ö�ʱ��
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

//���ٶ�ʱ��
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

//�ж��Ƿ��ж�ʱ��
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

//���ô��˶��پ�
void CAndroidUserItem::AddPlayCount()
{
	++m_wPlayCount;
}
//��ȡ���˶��پ�
CT_WORD CAndroidUserItem::GetPlayCount() 
{ 
	return m_wPlayCount; 
}