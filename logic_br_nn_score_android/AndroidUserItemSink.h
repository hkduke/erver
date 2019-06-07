#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "MSG_BR_NN_SCORE.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"
#include <random>

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
	bool OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize);
	bool OnSubStartBet(const void * pBuffer, CT_DWORD wDataSize);
	bool OnSubAddScore(const void * pBuffer, CT_DWORD wDataSize);
	bool OnSubApplyBanker(const void * pBuffer, CT_DWORD wDataSize);
	bool OnSubSitUserList(const void * pBuffer, CT_DWORD wDataSize);
	bool OnSubFlashAreaInfo(const void * pBuffer, CT_DWORD wDataSize);

public:
	bool OnSubGameEnd();

private:
	inline bool AndroidUserBet(CT_DWORD dwParam);

private:
	enum enBetPhase
	{
		en_Front,
		en_Back,
	};

	ITableFrame*		m_pTableFrame;
	CT_WORD				m_wChairID;		
	IServerUserItem*	m_pAndroidUserItem;

	CT_BYTE				m_cbBetCount;			//下注次数
	CT_BYTE				m_cbBetTotalCount;		//下注总次数

	CT_BYTE				m_cbBetPhase;			//下注阶段
	CT_BYTE				m_cbFrontBetCount;		//前半部分下注次数
	CT_BYTE				m_cbBackBetCount;		//后半部分下注次数
	CT_DWORD			m_dwFrontBetTime;		//前半部分下注时间间隔
	CT_DWORD			m_dwBackBetTime;		//后半部分下注时间间隔

	CT_BYTE				m_cbBetTotalTime;		//下注总时间
	CT_DWORD			m_dwStartBetTime;		//开始下注时间

	CT_BOOL				m_bSitList;				//是否坐在椅子上
	CT_WORD				m_wCurrBanker;			//当前庄家
	CT_LONGLONG			m_llBankerLimitScore;
	CT_LONGLONG			m_llUserLimitScore;

	std::random_device	m_randomDevice;
	std::mt19937		m_randomGen;

	CT_BYTE				m_cbFlashArea[AREA_COUNT + 1];
	static CT_LONGLONG	m_llJetton[MAX_JETTON_NUM];					//小筹码
};