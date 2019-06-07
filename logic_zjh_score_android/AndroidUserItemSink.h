#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "MSG_ZJH.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"

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

	//消息处理
protected:
	//机器人消息
	bool OnSubAndroidCard(const void * pBuffer, CT_WORD wDataSize);
	//游戏开始
	bool OnSubGameStart(const void * pBuffer, CT_WORD wDataSize);
	//用户放弃
	bool OnSubGiveUp(const void * pBuffer, CT_WORD wDataSize);
	//用户加注
	bool OnSubAddScore(const void * pBuffer, CT_WORD wDataSize);
	//用户看牌
	bool OnSubLookCard(const void * pBuffer, CT_WORD wDataSize);
	//用户比牌
	bool OnSubCompareCard(const void * pBuffer, CT_WORD wDataSize);
	//用户开牌
	bool OnSubOpenCard(const void * pBuffer, CT_WORD wDataSize);
	//游戏结束
	bool OnSubGameEnd(const void * pBuffer, CT_WORD wDataSize);
	//孤注一掷
	bool OnSubAllInResult(const void * pBuffer, CT_WORD wDataSize);
	//火拼结果
	bool OnSubRush(const void * pBuffer, CT_WORD wDataSize);
	//火拼
	bool OnSubRushResult(const void * pBuffer, CT_WORD wDataSize);

	//逻辑辅助
protected:
	//推断胜者
	CT_WORD EstimateWinner();
	bool HandleAndroidAddScore();
	bool HandleAndroidRush();

public:
	bool ClearAllTimer();

private:
	CGameLogic			m_GameLogic;								//游戏逻辑
	ITableFrame*		m_pTableFrame;								//桌子指针
	CT_WORD				m_wChairID;									//机器人椅子ID
	IServerUserItem*	m_pAndroidUserItem;							//机器人对象

	//游戏变量
protected:
	CT_LONGLONG			m_lStockScore;								//当前库存
	CT_WORD				m_wBankerUser;								//庄家用户
	CT_WORD				m_wCurrentUser;								//当前用户
	CT_BYTE				m_cbCardType;								//用户牌型

	//加注信息
protected:
	bool				m_bMingZhu;									//看牌动作
	CT_LONGLONG			m_lCellScore;								//单元下注
	CT_LONGLONG			m_lMaxCellScore;							//最大下注
	CT_LONGLONG			m_lUserMaxScore;							//最大分数
	CT_LONGLONG			m_lCurrentTimes;							//当前倍数(其实就是当前的下注额度)
	CT_LONGLONG			m_lTableScore[GAME_PLAYER];					//下注数目
	CT_BYTE				m_cbAddScoreCount;							//下注次数
	CT_BYTE				m_cbLookCardAndDouble;						//标志已经有人看牌并且加注了
	CT_BYTE				m_cbChiJiStatus;							//进入吃鸡状态
	CT_BYTE				m_cbChiJiAddScoreCount;						//进入吃鸡状态后加注次数

	//用户状态
protected:
	CT_BYTE				m_cbPlayStatus[GAME_PLAYER];				//游戏状态
	CT_BYTE				m_cbRealPlayer[GAME_PLAYER];				//真人玩家
	CT_BYTE				m_cbAndroidStatus[GAME_PLAYER];				//机器玩家
	CT_BYTE				m_cbLookCardStatus[GAME_PLAYER];			//看牌状态

	//用户扑克
protected:
	CT_BYTE				m_cbHandCardData[MAX_COUNT];				//用户数据
	CT_BYTE				m_cbAllHandCardData[GAME_PLAYER][MAX_COUNT];//桌面扑克
	CT_BYTE				m_cbHandCardType[GAME_PLAYER];				//玩家的牌型
};