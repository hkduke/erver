#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_mjxzdd.h"
#include "GameLogic.h"
#include "glog_wrapper.h"
#include <map>
#include <random>
#include "CGlobalData.h"
#include <CMD_Dip.h>
//#include <glog_linux/glog/logging.h>

/*
目前此行牌逻辑只支持两人
*/

/*
 * 1. 抓牌引发操作 操作处理 超时处理
 * 2。出牌引发操作 操作处理 超时处理
 * 3。抓牌后 和 玩家吃碰之后引发操作 操作处理 超时处理
 * 4。听牌后胡加倍的处理 和 超时处理
 */

    //游戏流程
class CGameProcess : public ITableFrameSink
{
public:
	CGameProcess(CT_VOID);
	~CGameProcess(CT_VOID);
    
public:
	//游戏开始
	virtual CT_VOID	OnEventGameStart();
	//游戏结束
	virtual CT_VOID OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag);
	//发送场景
	virtual CT_VOID OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
    
	//事件接口
public:
	//定时器事件
	virtual CT_VOID	OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏消息
	virtual CT_BOOL OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);

	//用户接口
public:
	//用户进入
	virtual CT_VOID OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//用户准备
	virtual CT_VOID OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户离开
	virtual CT_VOID OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0);
	//用户断线
	virtual CT_VOID OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户重入
	virtual CT_VOID OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//用户换桌
	virtual CT_VOID OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce);
	//用户充值 llRechargeScore充值金额 单位: 分
	virtual CT_VOID OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore) {}

public:
	//设置指针
	virtual CT_VOID SetTableFramePtr(ITableFrame* pTableFrame);
	//清理游戏数据
	virtual CT_VOID ClearGameData(){}
	//私人房游戏大局是否结束
	virtual CT_BOOL IsGameRoundClearing(CT_BOOL bDismissGame){return CT_TRUE;}
	//	获取大局结算的总分
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID){return 0;}
	//设置桌子库存
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) {  }
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) {  }
	//设置机器人库存
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) {}
	//设置机器人库存下限
	virtual CT_VOID SetAndStockLowerLimit(CT_LONGLONG llAndroidStockLowerLimit) {}
	//设置系统通杀概率
	virtual CT_VOID SetSystemAllKillRatio(CT_WORD wSystemAllKillRatio) {}
	//设置今日库存值
	virtual CT_VOID SetTodayStock(CT_LONGLONG llTodayStock) {}
	//设置今日库存上限
	virtual CT_VOID SetTodayStockHighLimit(CT_LONGLONG llTodayHighLimit) {}
	//设置换牌概率
	virtual CT_VOID SetChangeCardRatio(CT_WORD wChangeCardRatio) {}
	//设置换牌概率
	virtual CT_VOID SetSystemOprateRatio(std::vector<tagBrGameOperateRatio> &vecChangeCardRatio) {}
	//设置游戏黑名单
	virtual CT_VOID SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList);
	//设置基础捕获概率
	virtual CT_VOID SetBaseCapturePro(int nBloodPoolStateValue) {}
	//设置捕鱼点控数据
	virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData) { return true; }
	//设置炸金花的牌库概率
	virtual CT_VOID SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio){}
	//捕鱼获取桌子上玩家的输赢值
	virtual CT_VOID GetAllPlayerAddScore(std::vector<tagPlayerAddScore> &vecAddScore) {}
	//设置血控系统数据(新)
	virtual CT_VOID SetStockControlInfo(std::vector<tagStockControlInfo>& vecStockControlInfo) {}
	//获取桌子的鱼种统计信息
	virtual CT_VOID GetAllPlayerFishStatisticInfo() {}

private:
	//发送数据
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = false);
	//每一局完成重置桌子数据
    CT_VOID ResetTableData_PerGameRound();
    //玩家抓手牌
    CT_VOID PlayerFetchHandCards();
    //玩家从牌堆中抓一张牌
    CT_BYTE PlayerFetchOneCard(CT_WORD wPlayerChairID);
    //牌局开始发牌
    CT_VOID SendCard_GameStart();
    //处理玩家出牌
    CT_BOOL OnMsg_PlayerOutCard(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);
    //处理玩家的操作(出牌引发的操作)
    CT_BOOL OnMsg_PlayerOperate_ByOutCard(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize);
    //处理玩家的操作(抓牌引发的操作)
    CT_BOOL OnMsg_PlayerOperate_ByZhuaCard(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize);
    //提示出这张牌能胡哪些番数多少
    CT_BOOL OnMsg_ReadyOutCard(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize);
    //玩家发送加倍消息
    CT_BOOL OnMsg_PlayerJiaBei(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize);
    //玩家取消听牌
    CT_BOOL OnMsg_PlayerCancenTing(CT_WORD wChairID, const CT_VOID *pDataBuffer, CT_DWORD dwDataSize);
    //开始游戏
    CT_VOID OnTimer_StartGame();
    //出牌倒计时
	CT_VOID OnTimer_PlayerOutCard();
	//自动准备
	CT_VOID OnTimer_AutoReady();
	//听牌后自动出牌
	CT_VOID OnTimer_AutoOutCardAfterTing();
	//加倍后超时自动胡牌
	CT_VOID OnTimer_AutoHuCardAfterJiaBei(CT_DWORD dwJiaBeiPlayer);
	//操作超时处理 出牌后
	CT_VOID OnTimer_PlayerOperateTimeOut_AfterOutCard(CT_DWORD dwParam);
	//操作超时处理 抓牌后
	CT_VOID OnTimer_PlayerOperateTimeOut_AfterZhuaCard(CT_DWORD dwParam);
	//自动解散桌子
	CT_VOID OnTimer_SystemJieSanDesk();
	//玩家出牌 玩家出牌分三种: 1.玩家手动出牌  2.系统超时代替玩家出牌 3.听牌后自动出牌
	CT_VOID PlayerOutCard(CT_WORD wChairID, CT_BYTE cbOutCardData, CT_BYTE cbOutCardIndex, CT_BOOL bAutoOutCard);
	//玩家抓牌  action:什么行为引发了抓牌 cbPlayer:哪个玩家做出的行为
	CT_VOID PlayerZhuaCard(Action_Zhua action, CT_BYTE cbPlayer);
	//提示玩家出牌  bStartOutCardTimer:玩家抓牌触发玩家行为的时候可以提示客户端出牌，但是不能启动出牌定时器，因为这个时候已经有IDI_OPERATE_TIME_OUT_AFTERZHUA了不能同时存在两个定时器
	CT_VOID TipPlayerOutCard(CT_BYTE cbOutCardPlayer, CT_BYTE cbOutCardData, CT_BOOL bStartOutCardTimer, CT_BOOL bFirstTip = CT_FALSE);
	//在打牌后判断其他玩家是否有行为
	CT_BOOL IsHaveAction_AfterOutCard(CT_DWORD dwOutCardPlayer, CT_BYTE cbOutCardData);
	//在巴杠后是否有抢杠胡
	CT_BOOL IsHaveQiangGangHu_AfterBaGang(CT_DWORD dwBaGangPlayer, CT_BYTE cbBaGangCardData);
	//在摸牌后判断其他玩家是否有行为
	CT_BOOL IsHaveAction_AfterZhuaCard(CT_DWORD dwZhuaCardPlayer, CT_BYTE cbZhuaCardData);
	//除了dwExpectPlayer以外，其他玩家有行为的都会通知,出牌行为触发
	CT_VOID TipPlayerAction(CT_DWORD dwExpectPlayer, CT_BYTE cbActionCardData, TipTrigger tipTrigger, CT_BOOL bStartOperateOutTimer = CT_TRUE);
	//玩家碰牌
	CT_BOOL PlayerPengCard(CT_DWORD dwPengCardPlayer, CT_BYTE cbPengCardData);
	//玩家杠牌
	CT_BOOL PlayerGangCard(CT_DWORD dWGangCardPlayer, CT_BYTE cbGangCardData, CT_DWORD dwGangType);
	//玩家吃牌
	CT_BOOL PlayerChiCard(CT_DWORD dwChiCardPlayer, CT_BYTE cbChiCardData, CT_DWORD dwChiType);
	//玩家加倍
	CT_BOOL PlayerJiaBei(CT_DWORD dwPlayeChair);
	//所有用户行为完成后下一步应该做什么 dwMaxActionPlayer:最大行为玩家   cbMaxAction:最大行为玩家完成的行为
	CT_VOID PlayerFinishAction(CT_DWORD dwMaxActionPlayer, CT_DWORD cbMaxAction);
	//玩家吃胡
	CT_BOOL PlayerChiHuCard(CT_DWORD dwChiHuPlayer, CT_BYTE cbChiChuCard, CT_BOOL bZiMo);
	//玩家听牌
	CT_BOOL PlayerTingCard(CT_DWORD dwChairID);
	//能作出的行为
	std::string CanActionStr(CT_DWORD dwAction);
	//清理最大行为相关数据:在所有判断玩家是否有行为的地方调用
	CT_VOID ClearMaxActionData();
	//设置此次操作中，最大的操作行为 和 最大操作行为的实施玩家
	CT_VOID SetMaxActionData(CT_DWORD dwMaxActionPlayer,  CT_DWORD dwMaxAction);
    CT_BYTE GetMenFeng(CT_DWORD wChairID) { return (m_dwBanker == wChairID) ? 0x31 : 0x32; }
    //断线重连处理
    CT_VOID DisReConnect(CT_DWORD dwChairID);
    //设置胡的牌型和番数
    CT_VOID SetHuTypeFanshu(CT_BYTE cbPlayerHuType[],CT_BYTE cbMaxHuTypeCount ,CT_DWORD dwChairID);
    //打牌之前判断玩家是否能听牌
    CT_BOOL CanTing(CT_DWORD dwChairID);
    //提示玩家最大番数 和 最大张数: 在发送106消息并且能听牌并且没听牌的时候发送
    CT_VOID TipPlayerMax(CT_DWORD dwChair);
    //给玩家配置手牌
    CT_VOID CfgPlayerHandCard(CT_DWORD dwPlayer);
    //清除所有定时器
    CT_VOID ClearAllTimer();
    //听牌后提示玩家可能胡牌
    //在玩家听牌后，会造成玩家能胡的牌的张数和番数发生变化的时候调用,
    //有四个时机: 1,玩家自己听牌后摊牌  2,玩家听牌后加倍 3.玩家自己先听牌，而后对面听牌并且对面手牌中有自己能胡的牌 4,玩家自己先听牌,而后对面吃碰杠的牌是玩家自己能胡的牌
    //需要处理两种特殊情况:1,听牌后两个玩家胡的某些牌一样(暂时不处理)  2,玩家B听牌的时候，玩家A已经听牌，并且玩家B手牌中某些牌是玩家A能胡的牌。
    CT_VOID TipPlayerMayBeHuCard_AfterTing(CT_DWORD dwChairID, CT_BYTE cbCardData, CT_BOOL bZiMo, CT_BOOL bFirstOutCard);
    //听牌后获取能胡的牌剩余张数
    CT_BYTE GetShengYuCountAfterTing(CT_BYTE cbPlayer ,CT_BYTE cbCardData);
    //听牌后能否杠牌
    CT_BOOL CanGangAfterTing(CT_DWORD dwChairID, CT_BYTE cbCurCardData, CT_DWORD dwGangType, CT_DWORD dwMingGangUser);
private:
	ITableFrame*					m_pGameDeskPtr;							//游戏指针
	CGameLogic						m_GameLogic;							//游戏逻辑
	typedef std::map<CT_DWORD,tagChairInfo>   MapChairInfo;
	MapChairInfo 					m_mapChairInfo;

	CT_BYTE							m_cbLeftCardCount;						//剩余数目
	CT_BYTE							m_cbRepertoryCard[MAX_REPERTORY];		//牌堆
	CT_DWORD 						m_dwBanker;								//庄家位置
	CT_DWORD 						m_dwWinerChaifID;						//赢家位置
    CT_DWORD                        m_dwCurRepertoryCardIndex;              //牌堆当前有效位置
    CT_DWORD  						m_dwCurOutCardPlayer;					//当前已经出牌玩家
    CT_DWORD 						m_dwShouldOutCardPlayer;				//当前应该出牌的玩家
    CT_BOOL							m_bCurOutCardPlayerFinish;				//当前出牌玩家是否已经完成了出牌
    CT_BYTE							m_cbCurOutCardData;						//最近一次出的牌
    CT_BYTE							m_cbZhuaCardData;						//抓牌的玩家抓的牌,用于玩家超时不打牌时,系统自动自动出牌
    CT_DWORD  						m_dwCurZhuaCardPlayer;					//当前抓牌的玩家
    CT_DWORD 						m_dwAutoOutCardPlayer;					//因为超时不出牌，定时器自动选择这个玩家出牌
    CT_DWORD  						m_dWAutoOutCardData;					//因为超时不出牌，定时器自动选择出的牌
    CT_DWORD  						m_dwMaxAction;							//此次操作玩家作出的最大行为
    CT_DWORD 						m_dwMaxActionPlayer;					//此次操作作出最大行为的玩家
	CT_BOOL							m_bGangStatus;							//当前是否是杠的状态
	CT_DWORD 						m_dwCurGangPlayer;						//最近一次杠的玩家
	//CT_BYTE							m_cbHuCardData[2+MAX_INDEX*4];			//胡牌数据 数组长度预设参照tagListenCardData
	CT_BYTE 						m_cbCurState;							//当前状态
	CT_BYTE                         m_cbAllPlayerOutCardCount;              //所有玩家的出牌数目
	CT_BYTE 						m_cbAllPlayerZhuaCardCount;				//所有玩家的抓牌数目
	CT_UINT32 						m_startOperateTime;						//服务器命令客户端发起某种操作的开始时间
	CT_DWORD  						m_dwCurOperatePlayer;					//当前操作玩家
	CT_DWORD 						m_dwCurFinishOutCardPlayer;				//最后一次完成出牌的玩家
	CT_BOOL                         m_bCanTianTingBanker;                   //庄家是否能天听
	CT_BYTE							m_cbCancelTing_OutTime_OutCard;			//玩家取消听牌后,超时不出牌，系统自动替玩家出的牌
	TipTrigger                      m_eCanTingTrigger;                      //听牌的触发器是什么:Trigger_OutCard or Trigger_ZhuaCard
	CT_BYTE 						m_cbCanTingTrigger_ActionCardData;		//触发听牌时的活动牌
	TingTrigger  					m_eTingReason;
	CT_BOOL                         m_bXianPlayerFirstZhuaCard;             //闲家是否是第一次抓牌
	CT_BOOL                         m_bCanTianTingXian;                     //闲家是否能天听
	CT_BYTE                         m_cbLastZhuaCardData;                   //玩家抓的最后一张牌，出牌后清除
	CT_BOOL                         m_bZhuangFirstOutCard;                  //庄家是否是第一次出牌
};
#endif	//___DEF_GAME_PROCESS_H___
