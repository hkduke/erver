#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "MSG_ERMJ.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"
#include "MSG_ERMJ.h"

class CAndroidUserItemSink : public IAndroidUserItemSink
{
public:
	CAndroidUserItemSink();
	virtual ~CAndroidUserItemSink();

public:
	virtual void RepositionSink();
	//初始接口
	virtual void Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem);
	//设置斗地主的AI线程
	virtual void SetDdzAiThread(CDdzAiThread* pAiThread) {}
	//设置斗地主AI处理后的数据
	virtual void HandleDdzAiAction(std::shared_ptr<ai_action>& aiAction) {}

	//游戏事件
public:
	//定时器事件
	virtual bool OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏事件
	virtual bool OnGameMessage(CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize);

	CT_BOOL OnMsg_GameStart(const CT_VOID *pData, CT_DWORD dwDataSize);
	CT_BOOL OnMsg_SendCard(const CT_VOID *pData, CT_DWORD dwDataSize);
	CT_BOOL OnMsg_OutCard(const CT_VOID *pData, CT_DWORD dwDataSize);
	CT_BOOL OnMsg_OperateTip_SendCard(const CT_VOID *pData, CT_DWORD dwDataSize);
	CT_BOOL OnMsg_OperateTip_OutCard(const CT_VOID *pData, CT_DWORD dwDataSize);
private:
	CGameLogic			m_GameLogic;								//游戏逻辑
	ITableFrame*		m_pTableFrame;								//桌子指针
	CT_WORD				m_wChairID;									//机器人椅子ID
	IServerUserItem*	m_pAndroidUserItem;							//机器人对象

	CT_DWORD			m_dwCurBanker;								//当前庄家
	CT_DWORD			m_dwSelf;									//自己
	CT_DWORD 			m_dwShengYuCount;							//剩余牌数
	CT_DOUBLE			m_dCellScore;								//单元积分
	CT_DWORD			m_dwUpdateTime;								//倒计时显示
	CT_BYTE 			m_cbHandCardData[DEALER_CARD_COUNT];							//自己的手牌数据

	CT_BYTE 			m_cbCurOutCardData;							//当前应该出的牌
	CT_BYTE 			m_cbCurZhuaCardData;						//当前应该抓的牌
};