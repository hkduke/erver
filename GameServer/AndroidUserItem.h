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
	//��������
	virtual void ResetUserItem();
	//��ʱ������
	virtual void OnTimePulse();
	//���ö�ʱ��
	virtual bool SetTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam = 0);
	//���ٶ�ʱ��
	virtual bool KillTimer(CT_WORD wTimerID);
	//�ж��Ƿ��ж�ʱ��
	virtual bool IsHasTimer(CT_WORD wTimerID);

	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID);
	virtual void SendUserMessage(CT_DWORD mainID, CT_DWORD subID, const CT_VOID* pBuf, CT_DWORD len);

	//���ô��˶��پ�
	virtual void AddPlayCount();
	//��ȡ���˶��پ�
	virtual CT_WORD GetPlayCount();

	//���ô��Ȼ�����
	virtual CT_VOID SetBigJettionAndroid(CT_BYTE cbBigJetton) { m_cbBigJetton = cbBigJetton; }
	//����Ƿ���Ȼ�����
	virtual CT_BYTE GetBigJettonAndroid() { return m_cbBigJetton; }

public:
	void SetAndroidUserItemSink(IAndroidUserItemSink* pSink);
	
private:
	IAndroidUserItemSink*	m_pAndroidUserSink;
	CT_WORD					m_wPlayCount;
	CT_BYTE 				m_cbBigJetton;

	//��ʱ���б�
	TimerInfo	            m_SysTimer[DW_SYS_TIMER_NUMS];
};