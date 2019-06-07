#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "../logic_red_packet_score/MSG_HB.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"
#include <string>
#include <iostream>
#include <fstream>

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
	bool OnSubFaHB(const void * pBuffer, CT_DWORD wDataSize);
	bool OnGameOver(const void * pBuffer, CT_DWORD wDataSize);		//游戏结束
	void ReadJsonCfg();
	bool ReadFile(std::string & file, std::string & str);
private:
	ITableFrame*		m_pTableFrame;
	CT_WORD				m_wChairID;
	IServerUserItem*	m_pAndroidUserItem;
	CT_DWORD 			m_dwRandIndex;
public:
	std::vector<GS_RobotFaHBAmountCfg>	m_vRobotFaHBAmountCfg;			//机器人发红包金额配置
};