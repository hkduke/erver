#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "../logic_jznc_score/MSG_jznc.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"
#include <string>
#include <iostream>
#include <fstream>
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
	bool OnStartThrowDice(const void * pBuffer, CT_DWORD wDataSize);		//游戏结束
	void ReadJsonCfg();
	bool ReadFile(std::string & file, std::string & str);
private:
	ITableFrame*		m_pTableFrame;
	CT_WORD				m_wChairID;
	IServerUserItem*	m_pAndroidUserItem;
	std::random_device 				m_rd;
	std::mt19937 					m_gen;
public:

};