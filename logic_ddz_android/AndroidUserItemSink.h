#pragma once

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameAndroidExport.h"
#include "MSG_DDZ.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include "IAndroidUserItemSink.h"
#include "IServerUserItem.h"
#include "BOTProces.h"
class BOTProces;
class my_ddzboot;
class CAndroidUserItemSink : public IAndroidUserItemSink
{
public:
	CAndroidUserItemSink();
	virtual ~CAndroidUserItemSink();

public:
	virtual void RepositionSink();
	//初始接口
	virtual void Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem);

	//游戏事件
public:
	//定时器事件
	virtual bool OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏事件
	virtual bool OnGameMessage(CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize);


public:
	bool OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize); 
	bool OnSubGameCallBank(const void * Buffer, CT_DWORD DataSize); //谁叫地主信息
	bool OnSubGameBank(const void * pBuffer, CT_DWORD wDataSize); //装家信息
	bool OnSubUserPass(const void * cBuffer, CT_DWORD sDataSize); //玩家弃牌
	bool OnSubSendCard(const void * SzBuffer, CT_DWORD tDataSize); //出牌信息
	bool OnSubStartCard(const void * bzBuffer, CT_DWORD eDataSize); //开始出牌信息
	bool OnGameOver(const void * sbzBuffer, CT_DWORD teDataSize); //开始出牌信息
	void SetCard(int Icount);//1为上家，2为下家
	CT_VOID OnSendCard( CT_DWORD cont); //开始出牌信息
	CT_VOID LookSendCard(CT_WORD userid); //发送看牌
	CT_VOID OnCard (const void *Buffer, CT_DWORD eDataSize); //接收看牌信息
	bool lookCard_FXJD(int onando);//显示牌信息，并拿取其他2家牌型，并分析绝对手牌  参数，0为自己出牌，1为管别人的牌
	CT_VOID SetTime(int icound);
	int LookCard_1;// 0为上家，1为下家
	int userdz;
	my_ddzboot*       botddz;
	int iqsdd = 0;
private:
	ITableFrame*		m_pTableFrame;
	CT_WORD				m_wChairID;		
	IServerUserItem*	m_pAndroidUserItem;
	CT_BYTE				m_cbHandCardData[MAX_COUNT];	//手上扑克
	CT_DWORD			m_dwBankerUser; 
	CT_BYTE				m_oldeData[MAX_COUNT];	//手上扑克

	CGameLogic*         m_GameLogic;

	///






};