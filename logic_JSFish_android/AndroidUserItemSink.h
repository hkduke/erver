#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "MSG_JS_FISH.h"
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

public:
	bool ClearAllTimer();
	CT_INT32 NewBulletID();
private:
	CGameLogic			m_GameLogic;								//游戏逻辑
	ITableFrame*		m_pTableFrame;								//桌子指针
	CT_WORD				m_wChairID;									//机器人椅子ID
	IServerUserItem*	m_pAndroidUserItem;							//机器人对象


	double cannon_mulriple_[MAX_CANNON_TYPE];
	unsigned  short  m_self_chair_id;
	double   m_self_score;

	std::map<unsigned int ,CMD_S_Send_Bird> m_mapAliveBird;
	std::map<unsigned int , time_t> m_mapBirdCreatetime;
	unsigned  int m_nLockBirdID;
	unsigned  int m_nLockTimers;

	unsigned  int m_bullet_id_start;
	unsigned  int m_bullet_id_cur;
	unsigned  int m_bullet_id_end;
};