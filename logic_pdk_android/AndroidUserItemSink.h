#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "MSG_PDK.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"

struct OtherPlayerAnalyse
{
    OtherPlayerAnalyse(){
        reSet();
    }
    void reSet()
    {
        bIsOtherPlayerSingle = false;
        bIsOtherPlayerDouble = false;
        bIsChunTian = false;
        bIsOtherHasBomb = false;    //其他玩家是否有炸弹
        memset(cbOehterDoubleCard, 0,sizeof(cbOehterDoubleCard));
        cbOtherMaxSingle = 0;
        cbMinPlayerCardCount = 16;
    }
    CT_BOOL bIsOtherPlayerSingle;
    CT_BOOL bIsOtherPlayerDouble;
    CT_BOOL bIsChunTian;
    CT_BOOL bIsOtherHasBomb ;    //其他玩家是否有炸弹
    CT_BYTE cbOehterDoubleCard[MAX_COUNT];
    CT_BYTE cbOtherMaxSingle;
    CT_BYTE cbMinPlayerCardCount;
};

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
    virtual void SetDdzAiThread(CDdzAiThread* pAiThread) {}
    //设置斗地主AI处理后的数据
    virtual void HandleDdzAiAction(std::shared_ptr<ai_action>& aiAction){};

	//游戏事件
public:
	//定时器事件
	virtual bool OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏事件
	virtual bool OnGameMessage(CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize);


public:
	bool OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize);		//游戏开始
	bool OnSubUserPass(const void * pBuffer, CT_DWORD wDataSize);		//玩家弃牌
	bool OnSubUserOutCard(const void * pBuffer, CT_DWORD wDataSize);	//出牌信息
	bool OnGameOver(const void * pBuffer, CT_DWORD wDataSize);		//游戏结束
public:
	CT_BOOL OnAutoOutCard();
	CT_BOOL OtherPlayerCardAnalyse(OtherPlayerAnalyse & PlayerAnalyse);
private:
	ITableFrame*		m_pTableFrame;
	CT_WORD				m_wChairID;		
	IServerUserItem*	m_pAndroidUserItem;
	CT_BYTE				m_cbHandCardData[GAME_PLAYER][MAX_COUNT];	//手上扑克
	CT_BYTE				m_cbHandCardCount[GAME_PLAYER];				//手上扑克数目
	CT_DWORD			m_dwCurrentUser;
	CGameLogic	        m_GameLogic;

	//AI返回的一些数据
	CT_BYTE				m_cbAiOutCardType;							//出牌类型
	CT_BYTE				m_cbAiOutCardCount;							//出牌数目
	CT_BYTE				m_cbAiOutCardData[MAX_COUNT];				//出牌数据

	//用户出牌
	CT_BYTE				m_cbLastUserOutCount;						//上一手用户出牌数目
	CT_BYTE				m_cbLastUserOutCardData[MAX_COUNT];			//上一手用户出牌数据
	CT_DWORD			m_dwLastOutCardUser;						//上一手出牌的用户

};