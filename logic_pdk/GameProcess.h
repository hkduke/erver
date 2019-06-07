#ifndef		___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_PDK_16.h"
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
	virtual CT_BOOL OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID* pDataBuffer, CT_DWORD dwDataSize);

	//用户接口
public:
	//用户进入
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户准备
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户离开
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0) {};
	//用户断线
	virtual void OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户重入
	virtual void OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};

public:
	//设置指针
	virtual void SetTableFramePtr(ITableFrame* pTableFrame);
	//清理游戏数据
	virtual void ClearGameData();
	//私人房游戏大局是否结束
	virtual bool IsGameRoundClearing(CT_BOOL bDismissGame);
	//获取大局结算的总分
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID);

private:
	//发送数据
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = true);


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
	//是否出黑桃3
	CT_BYTE IsMustOutThree(CT_DWORD dwChairID);

	//游戏事件
private:
	//玩家出牌
	CT_INT32 OnUserOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout = CT_FALSE);
	//玩家不出
	CT_INT32	OnUserPassCard(CT_DWORD dwChairID);
	//通知下一个玩家出牌
	CT_INT32 NotifyNextOutCard(CT_DWORD dwChairID);
	//查找下一个出牌玩家
	CT_DWORD FindNextOutCardUser(CT_DWORD dwChairID);
	//通知出牌
	CT_BOOL NotifyOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCardTime, CT_BYTE cbOutCardFlag, CT_BYTE cbNewTurn);
	//是否有炸弹
	CT_BOOL IsHaveRomb(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//是否有黑桃3
	CT_BOOL IsHaveThree(CT_BYTE cbCardData[], CT_BYTE cbCardCount);

	//洗牌
	void XiPai();

private:
	//游戏信息
	CT_DWORD				m_dwSysTime;										//系统时间
	CT_BYTE					m_cbGameStatus;										//游戏状态

																				//玩家标识
	CT_DWORD				m_dwFirstUser;										//首出牌牌玩家
	CT_DWORD				m_dwCurrentUser;									//当前出牌玩家
	CT_DWORD				m_dwResumeUser;										//还原用户
	CT_DWORD				m_dwTurnWiner;										//胜利玩家
	CT_BYTE					m_cbTurnCardType;									//上家牌型
	CT_BYTE					m_cbTurnCardCount;									//出牌数目
	CT_BYTE					m_cbTurnCardData[MAX_COUNT];						//出牌数据
	CT_BYTE					m_cbRombCount[GAME_PLAYER];							//炸弹个数
	CT_BOOL					m_bMustThree;										//必出黑桃3
	CT_BYTE					m_cbIsOutThree;										//是否要出黑桃3
	CT_BYTE					m_cbSelfCardCount;

	//扑克信息
	CT_BYTE					m_cbLeftCardCount;									//剩余数目
	CT_BYTE					m_cbRepertoryCard[MAX_CARD_TOTAL];					//库存扑克

	CT_BYTE					m_cbCardCount[GAME_PLAYER];							//玩家牌数量
	CT_BYTE					m_cbCardData[GAME_PLAYER][MAX_COUNT];				//玩家牌数据

	CT_BYTE					m_cbDiscardCount[GAME_PLAYER];						//丢弃数目
	CT_BYTE					m_cbDiscardCard[GAME_PLAYER][30];					//丢弃记录

	CT_BYTE					m_cbOutCardCount[GAME_PLAYER];						//出牌次数

	CT_BYTE					m_cbUserSingle[GAME_PLAYER];						//玩家报单
	CT_BYTE					m_cbUserBaoPei[GAME_PLAYER];						//玩家包赔

	PDK_16_CMD_S_GameEnd	m_GameEnd;											//游戏结算

																				//配置数据
private:
	//时间信息
	CT_BYTE					m_cbOutCardTime;									//出牌时间

private:
    //房间信息
    CT_DWORD				m_dwGameType;										//游戏类型.0:游戏场;1:比赛场.
    CT_DWORD				m_dwServiceCost;									//服务费用.(%).
    CT_DWORD				m_dwlCellScore;										//基础积分

private:
	ITableFrame*			m_pGameDeskPtr;										//游戏指针
	CGameLogic				m_GameLogic;										//游戏逻辑

	PrivateTableInfo		m_PrivateTableInfo;									//获取私人房信息
	CT_INT32				m_iTotalLWScore[GAME_PLAYER];						//各玩家总输赢分
	CT_BYTE					m_cbTotalQuanGuan[GAME_PLAYER];						//各玩家全关次数
	CT_BYTE					m_cbTotalBeiGuan[GAME_PLAYER];						//各玩家被关次数
	CT_BYTE					m_cbTotalRomb[GAME_PLAYER];							//各玩家炸弹个数
	CT_BYTE					m_cbTotalTongPei[GAME_PLAYER];						//各玩家通赔个数
private:
	CT_BYTE					m_cbIsLookCardCount;								//是否显示牌数
};

#endif	 