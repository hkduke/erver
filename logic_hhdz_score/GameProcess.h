#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_HHDZ_SCORE.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include <vector>
#include <map>
#include <set>

//游戏流程
class CGameProcess : public ITableFrameSink
{
public:
	struct tagAreaWinScore
	{
		CT_BYTE			cbAreaIndex;
		CT_LONGLONG		llWinScore;
	};

	struct tagUserHistory
	{
		CT_WORD			wChairID;							//椅子ID
		CT_WORD			wPlayCount;							//玩的局数
		CT_BYTE			cbWinCount[USER_HISTORY_COUNT];		//赢的局数
		CT_LONGLONG		llJettonScore[USER_HISTORY_COUNT];	//加注总额
	};

	struct tagUserHistoryEx
	{
		CT_WORD			wChairID;							//椅子ID
		CT_BYTE			cbWinCount;							//赢的局数
		CT_DWORD		dwJettonScore;						//加注总额
	};

	struct tagUserFollowBet
	{
		CT_BYTE			cbFollowSitID;						//跟投的座椅ID
		CT_BYTE			cbJettonIndex;						//跟注索引
		CT_BYTE			cbTotalCount;						//跟投总次数
		CT_BYTE			cbCurrCount;						//当前跟投次数
		CT_BYTE			cbFollowThisRound;					//这局已经跟投
	};

public:
	CGameProcess(void);
	~CGameProcess(void);
    
public:
	//游戏开始
	virtual void OnEventGameStart();
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
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser);
	//用户准备
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户离开
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag=0);
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
	//设置桌子库存(包括机器人库存值)
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) { m_llStorageControl = llTotalStock; }
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) { m_llStorageLowerLimit = llStockLowerLimit; }
	//设置机器人库存
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) {}
	//设置机器人库存下限
	virtual CT_VOID SetAndStockLowerLimit(CT_LONGLONG llAndroidStockLowerLimit) {}
	//设置系统通杀概率
	virtual CT_VOID SetSystemAllKillRatio(CT_WORD wSystemAllKillRatio) { m_wSystemAllKillRatio = wSystemAllKillRatio; }
	//设置今日库存值
	virtual CT_VOID SetTodayStock(CT_LONGLONG llTodayStock) { m_llTodayStorageControl = llTodayStock; }
	//设置今日库存上限
	virtual CT_VOID SetTodayStockHighLimit(CT_LONGLONG llTodayHighLimit) { m_llTodayStorageHighLimit = llTodayHighLimit; }
	//设置换牌概率
	virtual CT_VOID SetChangeCardRatio(CT_WORD wChangeCardRatio) { }
	//设置换牌概率
	virtual CT_VOID SetSystemOprateRatio(std::vector<tagBrGameOperateRatio> &vecChangeCardRatio) {}
    //设置游戏黑名单
    virtual CT_VOID SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList);
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
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf=nullptr, CT_WORD dwLen=0, CT_BOOL bRecord = true);

    //功能函数
private:
    //初始化游戏数据
    void InitGameData();
	//更新游戏配置
	void UpdateGameConfig();
	//清除所有定时器
	void ClearAllTimer();
	//初始化游戏桌子数据
	void InitGameTableData();
    //普通场游戏结算
    CT_VOID NormalGameEnd(CT_DWORD dwChairID);
	//计算玩家的历史记录
	void CalUserHistory(CT_BOOL bUpdateToClient);
	//玩家历史记录排序
	void SortUserHistory(CT_BOOL bUpdateToClient);
	//处理玩家跟投消息
	void HandleUserFollow(CT_DWORD dwChairID, CT_BYTE cbJettonArea);
	//跟投玩家的跟投信息
	inline void SendUserFollowInfo(CT_DWORD dwChairID, const tagUserFollowBet& userFollowInfo);

    //游戏事件
private:
	//玩家下注
	CT_WORD OnUserBetScore(CT_WORD wChairID, CT_BYTE cbJettonArea, CT_WORD wJettonIndex, CT_BOOL bIsAndroid = false);
	//玩家续注
	CT_WORD OnUserBetLastScore(CT_WORD wChairID);
	//查询玩家下注排行
	CT_BOOL OnQueryUserGameHistory(CT_WORD wChairID);
	//玩家跟投
	CT_BOOL OnUserFollowBet(CT_WORD wChairID, CT_BYTE cbSitID, CT_BYTE cbJettonIndex, CT_BYTE cbFollowCount);
	//玩家取消跟投
	CT_BOOL OnUserCancelFollowBet(CT_WORD wChairID, CT_BYTE cbReason, CT_BOOL bUpdateToClient = true);

private:
	//座椅链表信息
	CT_BOOL SendUserSitList(CT_WORD wChairID);
	//更新错误信息
	CT_BOOL SendErrorCode(CT_WORD wChairID, enHHDZ_ErrorCode enCodeNum);
	//更新座椅信息
	CT_VOID UpdateUserSitInfo(CT_BOOL bUpdateToClient);
	//更新座椅的跟投信息
	CT_VOID UpdateUserSitUserFollowInfo(CT_BYTE cbSitId);

	//内部函数
private:
	//获取玩家对应区域下注金额
	inline CT_LONGLONG GetUserAreaBetScore(CT_WORD wChairID,CT_WORD wAreaIndex);
	//增加玩家对应区域下注金额
	inline CT_VOID AddUserAreaBetScore(CT_WORD wChairID, CT_WORD wAreaIndex, CT_LONGLONG llAddScore);
	//获取玩家所有区域下注的金额
	inline CT_LONGLONG GetUserAllAreaBetScore(CT_WORD wChairID);
	//获取对应区域的总下注金额
	inline CT_LONGLONG GetAreaBetScore(CT_WORD wAreaIndex);
	//获取真实玩家对应区域的总下注金额
	inline CT_LONGLONG GetRealUserAreaBetScore(CT_WORD wAreaIndex);
	//增加对应区域的总下注金额
	inline CT_VOID AddAreaBetScore(CT_WORD wAreaIndex, CT_LONGLONG llAddScore, CT_BOOL bAndroid);
	//获取玩家座椅位置
	inline CT_WORD GetUserSitPos(CT_WORD wChairID);
	//洗牌
	CT_VOID XiPai();
	//获取一手散牌
	CT_VOID GetSanPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);
	//获取对子牌(小于9)
	CT_VOID GetDuiZiPaiLessThan9(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);
	//获取对子牌(大于等于9)
	CT_VOID GetDuiZiPaiMoreThan8(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);
	//获取顺子牌
	CT_VOID GetShunZiPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);
	//获取金花
	CT_VOID GetJinHuaPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);
	//获取顺金
	CT_VOID GetShunJinPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);
	//获取豹子
	CT_VOID GetBaoZiPai(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);
	//获取随机牌
	CT_VOID GetRandCard(CT_BYTE cbCardData[], CT_BYTE cbRepertoryCard[]);

	//大路单数据
	CT_VOID RecordBigRoad();
	//大眼仔数据
	CT_VOID RecordBigEyeRoad(std::vector<std::vector<CT_BYTE>>& vecBigEyeRoad,  std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, bool bCalPredict, CT_BYTE cbWinArea);
	//小路数据
	CT_VOID RecordSmallRoad(std::vector<std::vector<CT_BYTE>>& vecSmallRoad,  std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, bool bCalPredict, CT_BYTE cbWinArea);
	//曱甴路数据
	CT_VOID RecordCockroachRoad(std::vector<std::vector<CT_BYTE>>& vecCockroachRoad,  std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, bool bCalPredict, CT_BYTE cbWinArea);

	//生成各路数据的统一算法
	CT_VOID GenerateRoadData(std::vector<std::vector<CT_BYTE>>& vecRoad, CT_BYTE& cbLastRecord, CT_BYTE cbCurrentRecord, CT_BYTE& cbCurrRow, CT_BYTE& cbCurrCol);

	//原始记录
	inline CT_VOID GenerateOriginalRecord(std::vector<std::vector<CT_BYTE>>& vecOriginalBigHistory, CT_BYTE winArea);

private:
	//库存限制操作
	CT_BOOL StorageControl();

private:
    //游戏信息
	CT_BYTE							m_cbGameStatus;										//游戏状态
	CT_BYTE							m_cbPlayStatus[GAME_PLAYER];						//游戏中玩家,下注了的玩家
	CT_BYTE							m_cbCanAddLastJetton[GAME_PLAYER];					//标识是否可以续注（上一局没有下注不能续注，本局已经下注不能续注）
	CT_BOOL							m_bHasUserAddJetton;								//标识是否有玩家下注

	//CT_DWORD						m_dwCellScore;										//基础积分

	//历史记录
	CT_BYTE							m_cbHistory[HISTORY_COUNT];							//历史记录(1 表示黑赢 2 表示红赢)
	CT_BYTE							m_cbWinRate[CARD_AREA_COUNT];						//红黑胜率
	CT_BYTE							m_cbWinCardType[HISTORY_COUNT];						//赢牌牌型
	CT_BYTE							m_cbHistoryCount;									//历史记录局数

	//大路单
	std::vector<std::vector<CT_BYTE>> m_vecBigHistory;									//大路单数据
	CT_BYTE							m_cbLastWin;										//上次赢的记录
	CT_BYTE							m_cbBigRoadPosRow;									//大路单行记录
	CT_BYTE							m_cbBigRoadPosCol;									//大路单列记录
	std::vector<std::vector<CT_BYTE>>	m_vecOriginalBigHistory;						//大路单原始数据(用于生成大眼仔路, 小路，蟑螂路数据)

	//大眼仔路
	std::vector<std::vector<CT_BYTE>>	m_vecBigEyeRoad;								//大眼仔路数据
	//小路路单
	std::vector<std::vector<CT_BYTE>> m_vecSmallRoad;									//小路数据
	//蟑螂路单
	std::vector<std::vector<CT_BYTE>> m_vecCockroachRoad;								//曱甴路数据

    //黑方预测
    CT_BYTE							m_cbBlackPredictBigEye;						        //大路仔路预测(255表示没有预测值，下同)
    CT_BYTE							m_cbBlackPredictSmall;						        //小路预测
    CT_BYTE							m_cbBlackPredictCockroach;					        //曱甴路预测

    //红方预测
    CT_BYTE							m_cbRedPredictBigEye;							    //大路仔路预测
    CT_BYTE							m_cbRedPredictSmall;							    //小路预测
    CT_BYTE							m_cbRedPredictCockroach;						    //曱甴路预测

private:
	//总下注数
	CT_LONGLONG						m_llAllJettonScore[AREA_COUNT];						//全体总注(包括机器人)
	CT_LONGLONG						m_llRealUserJettonScore[AREA_COUNT];				//真实玩家总注

private:
	//玩家信息
	CT_LONGLONG						m_llUserJettonScore[AREA_COUNT][GAME_PLAYER];		//各区域个人总注
	CT_LONGLONG						m_llUserLastJettonScore[AREA_COUNT][GAME_PLAYER];	//上一局各区域个人总注
	CT_LONGLONG						m_llUserTotalJettonScore[GAME_PLAYER];				//个人总注
	CT_LONGLONG						m_llUserAllLWScore[GAME_PLAYER];					//当局玩家总输赢分
	CT_LONGLONG						m_llAreaTotalLWScore[AREA_COUNT];					//区域总输赢分
	CT_LONGLONG						m_llUserAreaTotalLWScore[AREA_COUNT][GAME_PLAYER];	//玩家区域总输赢分
	CT_DWORD						m_dwAllUserID[GAME_PLAYER];							//用户ID
	CT_LONGLONG						m_llUserSourceScore[GAME_PLAYER];					//用户原始积分
	CT_LONGLONG 					m_llAreaTimedRefreshScore[AREA_COUNT];				//各个区域定时刷新金币的数额
	std::map<CT_DWORD, CT_LONGLONG> m_mapUserLWScore;									//玩家输赢分

	//玩家下注记录
	tagUserHistory					m_UserHistory[GAME_PLAYER];							//玩家历史记录
	tagUserHistoryEx				m_LuckyUserHistory;									//幸运玩家(最近20局赢的次数最多的玩家)	
	std::vector<tagUserHistoryEx>	m_vecUserHistory;									//历史下注排行榜

	//玩家跟投记录
	std::map<CT_BYTE, std::set<CT_WORD>>	m_mapBeFollowBetInfo;						//被跟投信息<-sitid, -set<chairid>>
	std::map<CT_WORD, tagUserFollowBet>		m_mapFollowBetInfo;							//玩家跟投信息<-chairid, -followinfo>

private:
	//座椅信息
	CT_WORD							m_wSitList[MAX_SIT_POS];							//座椅信息	

private:
	//库存控制
	CT_LONGLONG						m_llStorageControl;									//库存控制值,控牌对应值
	CT_LONGLONG						m_llStorageLowerLimit;								//库存下限
	CT_LONGLONG						m_llTodayStorageControl;							//今日库存
	CT_LONGLONG						m_llTodayStorageHighLimit;							//今日库存上限制
	CT_WORD 						m_wSystemAllKillRatio;								//系统必杀概率
	CT_DWORD						m_dwTodayTime;										//今日时间
	std::vector<tagBrGameBlackList>	m_vecBlackList;										//黑名单
	
private:
	//时间控制
	CT_DWORD						m_dwBetTime;										//下注时间
	CT_DWORD						m_dwEndTime;										//结束时间
	//CT_BYTE							m_cbEndTimeInterval;								//结束时间间隔

private:
	//扑克信息
	CT_BYTE							m_cbTableCardArray[CARD_AREA_COUNT][MAX_COUNT];		//桌面扑克
	CT_BYTE							m_cbLeftCardCount; 									//剩余数目
	CT_BYTE							m_cbRepertoryCard[MAX_CARD_TOTAL];					//库存扑克

private:
	//游戏数据
	static CT_LONGLONG				m_llSmallJetton[MAX_JETTON_NUM];					//小筹码
	CT_BOOL							m_bIsTableRuning;									//当前桌子是否启动了

private:
	ITableFrame*					m_pGameDeskPtr;										//游戏指针
	CGameLogic						m_GameLogic;										//游戏逻辑
	RecordScoreInfo					m_RecordScoreInfo[GAME_PLAYER];						//记录积分
	HHDZ_CMD_S_GameEnd				m_GameEnd;

public:
	struct tagBlackListWinScore
	{
		CT_WORD 	wChairID;
		CT_WORD 	wLostRatio;
		CT_BYTE     cbMaxJettonArea;
		CT_LONGLONG llWinScore;
	};
};

#endif