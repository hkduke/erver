#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"
#include "../logic_hyxd_score/MSG_FXGZ.h"

class CAndroidUserItemSink : public IAndroidUserItemSink
{
public:
	CAndroidUserItemSink();
	virtual ~CAndroidUserItemSink();

public:
	virtual void RepositionSink();
	//初始接口
	virtual void Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem);

	virtual void SetDdzAiThread(CDdzAiThread* pAiThread) {}

	//设置斗地主AI处理后的数据
	virtual void HandleDdzAiAction(std::shared_ptr<ai_action>& aiAction) {}

	//游戏事件
public:
	//定时器事件
	virtual bool OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏事件
	virtual bool OnGameMessage(CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize);

public:
    bool OnFreeGameScene(const void * pBuffer, CT_DWORD wDataSize);
	bool OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize);


private:
	ITableFrame*		m_pTableFrame;
	CT_WORD				m_wChairID;		
	IServerUserItem*	m_pAndroidUserItem;

	static CT_DWORD m_dwJetton[JETTON_COUNT];
};