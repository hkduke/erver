#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "MSG_DDZ.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"

class CDdzAiThread;
class CAndroidUserItemSink : public IAndroidUserItemSink
{
public:
	CAndroidUserItemSink();
	virtual ~CAndroidUserItemSink();

public:
	virtual void RepositionSink();
	//初始接口
	virtual void Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem);
	//设置斗地主AI线程
	virtual void SetDdzAiThread(CDdzAiThread* pAiThread) { m_pDdzAiThread = pAiThread; }
	//设置斗地主AI处理后的数据
	virtual void HandleDdzAiAction(std::shared_ptr<ai_action>& aiAction);

	//游戏事件
public:
	//定时器事件
	virtual bool OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏事件
	virtual bool OnGameMessage(CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize);


public:
	bool OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize);		//游戏开始
	bool OnSubGameCallBank(const void * Buffer, CT_DWORD DataSize);		//叫地主信息
	bool OnSubBankerInfo(const void * pBuffer, CT_DWORD wDataSize);		//装家信息
	bool OnSubUserAddDouble(const void * pBuffer, CT_DWORD wDataSize);	//玩家加倍
	bool OnSubUserPass(const void * pBuffer, CT_DWORD wDataSize);		//玩家弃牌
	bool OnSubUserOutCard(const void * pBuffer, CT_DWORD wDataSize);	//出牌信息
	bool OnSubStartCard(const void * pBuffer, CT_DWORD wDataSize);		//开始出牌信息
	bool OnGameOver(const void * pBuffer, CT_DWORD wDataSize);		//游戏结束

private:
	void HandleAiOutCardAction(std::shared_ptr<ai_action>& aiAction);

	void ChangeCardDataForAi(CT_DWORD wChairID, unsigned char cardList[]);
	void SendCallBankerDataToAi();

	void SendAiPlayCardToAi();
	void SendAiPlayCardPassiveToAi();

private:
	ITableFrame*		m_pTableFrame;
	CT_WORD				m_wChairID;		
	IServerUserItem*	m_pAndroidUserItem;
	CT_BYTE				m_cbHandCardData[GAME_PLAYER][MAX_COUNT];	//手上扑克
	CT_BYTE				m_cbHandCardCount[GAME_PLAYER];				//手上扑克数目
	CT_BYTE				m_cbBankerCard[3];							//底牌
	CT_DWORD			m_dwBankerUser; 
	CT_DWORD			m_dwCurrentUser;
	CGameLogic	        m_GameLogic;

	//AI返回的一些数据
	CT_BYTE				m_cbAiCallScore;							//AI返回的叫分
	CT_BYTE				m_cbAiOutCardType;							//出牌类型
	CT_BYTE				m_cbAiOutCardCount;							//出牌数目
	CT_BYTE				m_cbAiOutCardData[MAX_COUNT];				//出牌数据

	//用户出牌
	CT_BYTE				m_cbLastUserOutCount;						//上一手用户出牌数目
	CT_BYTE				m_cbLastUserOutCardData[MAX_COUNT];			//上一手用户出牌数据
	CT_DWORD			m_dwLastOutCardUser;						//上一手出牌的用户
	CT_DWORD 			m_dwLastOutCardTime;						//上一手出牌时间

	//斗地主
	CDdzAiThread*		m_pDdzAiThread;
};