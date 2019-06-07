#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_PDK.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include <map>

//游戏流程
class CGameProcess : public ITableFrameSink
{
public:
	CGameProcess(void);
	~CGameProcess(void);
    
public:
	//游戏开始
	virtual void	OnEventGameStart();
	//游戏结束
	virtual void OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag);
	//发送场景
	virtual void OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
    
	//事件接口
public:
	//定时器事件
	virtual void	OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam);
	//游戏消息
	virtual CT_BOOL OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);

	//用户接口
public:
	//用户进入
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//用户准备
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//用户离开
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0);
	//用户断线
	virtual void OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//用户重入
	virtual void OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户换桌
	virtual void OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce) {}
	//用户充值 llRechargeScore充值金额 单位: 分
	virtual void OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore) {}

public:
	//设置指针
	virtual void SetTableFramePtr(ITableFrame* pTableFrame);
	//清理游戏数据
	virtual void ClearGameData();
	//私人房游戏大局是否结束
	virtual bool IsGameRoundClearing(CT_BOOL bDismissGame);
	//获取大局结算的总分
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID);
	//设置桌子库存
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) {}
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) {}
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
    virtual CT_VOID SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList) {}
	//设置基础捕获概率
	virtual CT_VOID SetBaseCapturePro(int nBloodPoolStateValue) {}
	//设置捕鱼点控数据
	virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData) { return true; }
	//设置炸金花的牌库概率
	virtual CT_VOID SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio) {}
    //捕鱼获取桌子上玩家的输赢值
    virtual CT_VOID GetAllPlayerAddScore(std::vector<tagPlayerAddScore> &vecAddScore) {}
    //设置血控系统数据(新)
    virtual CT_VOID SetStockControlInfo(std::vector<tagStockControlInfo>& vecStockControlInfo) {}
	//获取桌子的鱼种统计信息
	virtual CT_VOID GetAllPlayerFishStatisticInfo() {}
    
private:
	//发送数据
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = false);


    //功能函数
private:
    //初始化游戏数据
    void InitGameData();
	//更新游戏配置
	void UpdateGameConfig();
	//清除所有定时器
	void ClearAllTimer();
    //普通场游戏结算
    CT_VOID NormalGameEnd(CT_DWORD dwWinChairID);
	//发送总结算
	void SendTotalClearing(std::string strTime);

	//解释私人场数据
	void ParsePrivateData(CT_CHAR *pBuf);

    //游戏事件
private:
	//玩家出牌
	CT_BOOL OnUserOutCard(CT_WORD wChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout = CT_FALSE);
	//玩家不出
	CT_BOOL OnUserPassCard(CT_WORD wChairID);
	//获得下一个玩家
	CT_DWORD GetNextUser(CT_DWORD dwChairID);
protected:
	//托管
	CT_BOOL IsTrustee(CT_DWORD dwTime = TIME_OUT_CARD);
	//设置托管
	CT_VOID SetTrustee(CT_DWORD dwChairID, CT_BYTE cbTrustee);
	//自动叫地主
	CT_BOOL OnAutoCallLand();
	//自动加倍
	CT_BOOL OnAutoDouble(CT_DWORD dwChairID);
	//自动出牌
	CT_BOOL OnAutoOutCard();
	//配好牌
	CT_BOOL MatchGoodCard(CT_BYTE cbRandCard[], CT_BYTE cbRandCount);
	CT_VOID CalculateBombScore();

private:
	CT_DWORD						m_dwSysTime;						//系统时间
	CT_BYTE							m_cbGameStatus;						//游戏状态
    //游戏信息
protected:
	CT_DWORD						m_dwBankerUser;						//红3用户
	CT_DWORD						m_dwCurrentUser;					//当前玩家
	CT_BYTE							m_cbOutCardCount[GAME_PLAYER];		//出牌次数
	CT_BYTE							m_cbWaitTime;						//等待时间
	CT_BYTE							m_cbTrustee[GAME_PLAYER];			//是否托管
	CT_BYTE							m_cbTimeOutCount[GAME_PLAYER];		//超时次数
	CT_BOOL 						m_bIsPassCard;						//是否过牌

	//倍数信息
protected:
	CT_DWORD						m_dwBombChairID;						//炸弹座位号
	CT_BYTE							m_cbEachBombCount[GAME_PLAYER];		//炸弹个数
    CT_DOUBLE						m_dEachBombScore[GAME_PLAYER];		//炸弹分数
	CT_DOUBLE						m_dEachBombRevenue[GAME_PLAYER];	//炸弹税收
	//CT_DWORD						m_dwChunTianbTime[GAME_PLAYER];					//春天倍数

	//出牌信息
protected:
	CT_DWORD						m_dwTurnWiner;						//胜利玩家
	CT_BYTE							m_cbTurnCardCount;					//出牌数目
	CT_BYTE							m_cbTurnCardData[MAX_COUNT];		//出牌数据

	CT_BYTE							m_cbOutCardListCount[GAME_PLAYER][MAX_COUNT];			//每个玩家每次出牌的
	CT_BYTE							m_cbOutCardList[GAME_PLAYER][MAX_COUNT][MAX_COUNT];		//每个玩家的出牌列表
	//CT_BYTE							m_cbRemainCard[MAX_CARD_VALUE];//当前剩余牌数据

	//扑克信息
protected:
	CT_BYTE							m_cbHandCardCount[GAME_PLAYER];		//扑克数目
	CT_BYTE							m_cbHandCardData[GAME_PLAYER][MAX_COUNT];	//手上扑克


	//组件变量
protected:
	PDK_CMD_S_GameEnd				m_GameEnd;							//游戏结算

//配置数据
private:
	//时间信息
	CT_BYTE							m_cbOpTotalTime;					//出牌时间
	CT_DWORD						m_dwOpStartTime;					//出牌时间戳

private:
	//房间信息
	//CT_DWORD						m_dwGameType;						//游戏类型.0:游戏场;1:比赛场.
	CT_DWORD						m_dwlCellScore;						//基础积分

private:
	ITableFrame*					m_pGameDeskPtr;						//游戏指针
	CGameLogic						m_GameLogic;						//游戏逻辑

	tagGameRoomKind*				m_pGameRoomKindInfo;				//房间配置
	PrivateTableInfo				m_PrivateTableInfo;					//获取私人房信息
	RecordScoreInfo					m_RecordScoreInfo[GAME_PLAYER];		//记录积分
	//CT_INT32						m_iTotalLWScore[GAME_PLAYER];		//各玩家总输赢分
	//CT_BYTE							m_cbTotalQuanGuan[GAME_PLAYER];		//各玩家全关次数
	//CT_BYTE							m_cbTotalBeiGuan[GAME_PLAYER];		//各玩家被关次数
	//CT_BYTE							m_cbTotalRomb[GAME_PLAYER];			//各玩家炸弹个数
	//CT_BYTE							m_cbTotalTongPei[GAME_PLAYER];		//各玩家通赔个数
	//CT_WORD						    m_wPlayWinCount[GAME_PLAYER];							//赢局数
	//CT_WORD						    m_wPlayLoseCount[GAME_PLAYER];							//输局数
};


#endif	//___DEF_GAME_PROCESS_H___
