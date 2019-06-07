#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_BR_NN_SCORE.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include <map>

//游戏流程
class CGameProcess : public ITableFrameSink
{
public:
	struct tagAreaWinScore
	{
		CT_BYTE			cbAreaIndex;
		CT_LONGLONG		llWinScore;
	};
	
	struct tagAreaUserJettonScore
	{
		CT_BYTE			cbAreaIndex;
		CT_LONGLONG 	llUserJetton;
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
	virtual void OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore);

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
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) { m_llStorageControl = llTotalStock; }
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) { m_llStorageLowerLimit = llStockLowerLimit; }
	//设置机器人库存
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) { m_llAndroidStorage = llAndroidStock; }
	//设置机器人库存下限
	virtual CT_VOID SetAndStockLowerLimit(CT_LONGLONG llAndroidStockLowerLimit) { m_llAndroidStorageLowerLimit = llAndroidStockLowerLimit; }
	//设置系统通杀概率
	virtual CT_VOID SetSystemAllKillRatio(CT_WORD wSystemAllKillRatio) { m_wSystemAllKillRatio = wSystemAllKillRatio; }
	//设置今日库存值
	virtual CT_VOID SetTodayStock(CT_LONGLONG llTodayStock) { m_llTodayStorageControl = llTodayStock; }
	//设置今日库存上限
	virtual CT_VOID SetTodayStockHighLimit(CT_LONGLONG llTodayHighLimit) { m_llTodayStorageHighLimit = llTodayHighLimit; }
	//设置换牌概率
	virtual CT_VOID SetChangeCardRatio(CT_WORD wChangeCardRatio) { m_wChangeCardRatio = wChangeCardRatio; }
	//设置换牌概率
	virtual CT_VOID SetSystemOprateRatio(std::vector<tagBrGameOperateRatio> &vecChangeCardRatio);
	//设置游戏黑名单
	virtual CT_VOID SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList);
    //设置基础捕获概率
    virtual CT_VOID SetBaseCapturePro(int nBloodPoolStateValue) {}
    //设置捕鱼点控数据
    virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData) { return true;}
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
	//更新庄家
	CT_BOOL UpdateBanker();
	//获得下一个玩家
	CT_WORD GetNextUser();
    //普通场游戏结算
    CT_VOID NormalGameEnd(CT_DWORD dwChairID);
	//获取下局彩金池奖励值
	CT_LONGLONG GetNextCaiJinChiOpenRew();

    //游戏事件
private:
	//玩家下注
	CT_WORD OnUserBetScore(CT_WORD wChairID, CT_BYTE cbJettonArea, CT_WORD wJettonIndex, CT_BOOL bIsAndroid = false);
	//玩家续注
	CT_WORD OnUserBetLastScore(CT_WORD wChairID);
	//申请庄家
	CT_WORD OnUserApplyBanker(CT_WORD wChairID);
	//取消申请庄家
	CT_WORD OnUserCancelApplyBanker(CT_WORD wChairID);
	//申请下庄
	CT_WORD OnUserCancelBanker(CT_WORD wChairID);	
	//坐下
	CT_WORD OnUserSit(CT_WORD wChairID,CT_BYTE cbType,CT_BYTE cbPos);
	//查询玩家的近20局游戏数据
	CT_BOOL OnQueryUserGameHistory(CT_WORD wChairID);

private:
	//更换庄家
	CT_BOOL SendChangeBanker(CT_BOOL bChangeBanker, CT_BYTE cbReason);
	//座椅链表信息
	CT_BOOL SendUserSitList(CT_WORD wChairID);
	//庄家申请列表
	CT_BOOL SendBankerApplyList(CT_WORD wChairID);
	//更新错误信息
	CT_BOOL SendErrorCode(CT_WORD wChairID, enNNErrorCode enCodeNum);
	//上局彩金池奖励冠军信息
	CT_BOOL SendCaiJinChiChampionInfo(CT_WORD wChairID = INVALID_CHAIR);
	//更新座椅信息
	CT_VOID UpdateUserSitInfo();

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
	//增加对应区域的总下注金额
	inline CT_VOID AddAreaBetScore(CT_WORD wAreaIndex, CT_LONGLONG llAddScore, CT_BOOL bAndroid);
	//获取庄家金币
	inline CT_LONGLONG GetBankerScore();
	//获取玩家座椅位置
	inline CT_WORD GetUserSitPos(CT_WORD wChairID);
	//洗牌
	CT_VOID XiPai();
	//获取炸弹牌型
	CT_VOID GetBombCard(CT_BYTE cbBombCard[]);
	//获取五小牛牌型
	CT_VOID GetFiveSmallCard(CT_BYTE cbBombCard[]);
	//获取五花牛牌型
	CT_VOID GetGoldenBullCard(CT_BYTE cbBombCard[]);
	//是否重置/衰减彩金池
	CT_BOOL ResetCaiJinChi();

	//计算系统坐庄时，每个区域输赢
	CT_VOID CalSystemBankerAreaWinUserScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT], std::vector<tagAreaWinScore>& vecAreaWinScore, CT_LONGLONG& llSystemWinScore);
	//计算系统坐庄时输了多少分
	CT_LONGLONG CalSystemBankerWinUserScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT]);

	//计算真人坐庄时，每个区域输赢
	CT_VOID CalUserBankerAreaWinSystemScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT], std::vector<tagAreaWinScore>& vecAreaWinScore);
	//计算真人坐庄时赢系统多少分
	CT_LONGLONG CalUserBankerWinSystemScore(CT_BYTE cbTempCardArray[AREA_COUNT + 1][MAX_COUNT]);
	
	//计算换牌概率
    CT_VOID CalSystemOperateRation(CT_LONGLONG llSystemWinScore = 0);
    //计算点数为10的区域
    inline CT_VOID CalTenPointArea();

private:
	//库存限制操作
	CT_BOOL StorageControl();
	CT_BOOL StoreageControlForAllKill();
	CT_BOOL StoreageControlChangeAreaCard();
	CT_VOID BlackListControl();
	CT_VOID ChangeAllKillAreaCard();
	//
	//CT_VOID StorageControlForFangShui();

private:
    //游戏信息
	CT_BYTE							m_wGameStatus;										//游戏状态
	CT_BYTE							m_wPlayStatus[GAME_PLAYER];							//游戏中玩家,下注了的玩家
	CT_BYTE							m_cbCanAddLastJetton[GAME_PLAYER];					//标识是否可以续注（上一局没有下注不能续注，本局已经下注不能续注）

	CT_INT32						m_iMultiple[AREA_COUNT + 1];						//倍数
	CT_DWORD						m_dwCellScore;										//基础积分
	CT_BYTE							m_cbHistory[HISTORY_COUNT][4];						//历史记录
  	CT_BYTE							m_cbHistoryCardType[HISTORY_COUNT][4];				//历史上牌型记录
	CT_BYTE							m_cbWinRate[4];										//10局胜率
	CT_BYTE							m_cbHistoryCount;									//历史记录局数
	CT_BOOL							m_bHasUserAddJetton;								//标识是否有玩家下注

//	CT_DWORD						m_dwSystemBankerCount;								//系统坐庄次数
//	CT_BYTE							m_cbSystemAllKillCount;								//系统坐庄100次通杀次数()
	CT_BYTE							m_cbBankerWinAreaCount;								//庄家赢各区域的次数

private:
	//总下注数
	CT_LONGLONG						m_llAllJettonScore[AREA_COUNT + 1];					//全体总注
	CT_LONGLONG						m_llRealUserJettonScore[AREA_COUNT +1];				//真实玩家总注

private:
	//玩家信息
	CT_LONGLONG						m_llUserJettonScore[AREA_COUNT + 1][GAME_PLAYER];		//各区域个人总注
	CT_LONGLONG						m_llUserLastJettonScore[AREA_COUNT + 1][GAME_PLAYER];	//上一局各区域个人总注
	CT_LONGLONG						m_llUserRoundTotalJettonScore[GAME_PLAYER];				//本局个人总注
	CT_LONGLONG						m_llUserAllLWScore[GAME_PLAYER];						//当局玩家总输赢分
	CT_LONGLONG						m_llAreaTotalLWScore[AREA_COUNT + 1];					//区域总输赢分
	CT_LONGLONG						m_llUserAreaTotalLWScore[GAME_PLAYER][AREA_COUNT + 1];	//玩家区域总输赢分
	CT_DWORD						m_dwAllUserID[GAME_PLAYER];								//用户ID
	CT_LONGLONG						m_llUserSourceScore[GAME_PLAYER];						//用户原始积分

	//CT_BYTE							m_cbUserPlayCount[GAME_PLAYER];							//玩家入桌以来的总局数
	//CT_BYTE							m_cbUserWinCount[GAME_PLAYER];							//玩家入桌以来的赢局数
	//CT_LONGLONG						m_llUserTotalJettonScore[GAME_PLAYER];					//玩家入桌以来的总注
	tagUserHistory					m_UserHistory[GAME_PLAYER];								//玩家历史记录
	//CT_WORD							m_wLuckyUser;											//幸运玩家(最近20局赢的次数最多的玩家)

private:
	//座椅信息
	CT_WORD							m_wSitList[MAX_SIT_POS];								//座椅信息	

private:
	//库存控制
	//CT_LONGLONG						m_llCaiJinChiCount;									//库存数值-彩金池(后台控制初始值) 
	//CT_LONGLONG						m_llCaiJinChiMax;									//库存封顶-彩金池(后台控制)
	//CT_WORD							m_wCaiJinChiMaxPercent;								//库存封顶比例-彩金池(后台控制，千分比)
	//CT_BOOL							m_bIsExtraRew;										//是否额外给予彩金池奖励
	//CT_LONGLONG						m_llOpenRewScore;									//彩金池开奖金额
	//CT_LONGLONG						m_llRevenueScore;									//税收金额
	CT_LONGLONG						m_llStorageControl;									//库存控制值,控牌对应值
	CT_LONGLONG						m_llStorageLowerLimit;								//库存控制下限值
	CT_LONGLONG						m_llAndroidStorage;									//机器人库存控制值
	CT_LONGLONG						m_llAndroidStorageLowerLimit;						//机器人库存下限
	CT_LONGLONG						m_llTodayStorageControl;							//今日库存
	CT_LONGLONG						m_llTodayStorageHighLimit;							//今日库存上限制
	CT_WORD							m_wSystemAllKillRatio;								//系统通杀概率(百分比)
	CT_WORD							m_wChangeCardRatio;									//闪牌的换牌概率
    std::vector<tagBrGameOperateRatio> m_vecSystemOperateRatio;                         	//闪牌的换牌概率
	std::vector<tagBrGameBlackList>	   m_vecBlackList;									//黑名单
	CT_DWORD						m_dwTodayTime;										//今日时间
	CT_BOOL 						m_bCheatAllKill;									//系统通杀
	CT_BOOL 						m_bCheatBlackList;									//杀黑名单
	CT_BOOL							m_bChangeAreaCard;									//换某个区域的牌
	//CT_BOOL							m_bResetCaiJinChi;								//是否衰减彩金池-零点衰减80%

private:
	//时间控制
	CT_DWORD						m_dwFreeTime;										//空闲时间
	CT_DWORD						m_dwSendCardTime;									//发牌时间
	CT_DWORD						m_dwBetTime;										//下注时间
	CT_DWORD						m_dwEndTime;										//结束时间
	CT_BYTE							m_cbEndTimeInterval;								//结束时间间隔

private:
	//扑克信息
	CT_BYTE							m_cbTableCardArray[AREA_COUNT + 1][MAX_COUNT];		//桌面扑克
	CT_BYTE							m_cbSendCardCount; 									//剩余数目
	CT_BYTE							m_cbRepertoryCard[MAX_CARD_TOTAL];					//库存扑克
private:
	//庄家信息
	std::vector<CT_WORD>			m_ApplyUserArray;									//申请玩家组合信息
	CT_LONGLONG						m_llApplyBankerCondition;							//申请庄家条件
	CT_WORD							m_wBankerUser;										//当前庄家
	CT_WORD							m_wBankerCount;										//做庄次数
	CT_LONGLONG						m_llBankerWinScore;									//庄家赢分
	CT_LONGLONG						m_llBankerSourceScore;								//庄家结算前的金币
	CT_WORD							m_wNextBankerUser;									//下局庄家
	CT_BOOL							m_bIsApplyCancelBanker;								//是否申请了下庄
	CT_LONGLONG						m_llBankerTotalWinScore;							//庄家总的输赢分

private:
	//游戏数据
	static CT_LONGLONG				m_llSmallJetton[MAX_JETTON_NUM];					//小筹码
	CT_BOOL							m_bIsTableRuning;									//当前桌子是否启动了
	CT_WORD							m_wMaxCardMulTiple;									//最大牌型倍数		

private:
	//上局彩金池冠军信息
	//CT_CHAR							m_szChampionNickName[NICKNAME_LEN];					//彩金池冠军名字
	//CT_CHAR							m_szChampionHeadUrl[HEAD_URL_LEN];					//彩金池冠军头像URL
	//CT_BYTE							m_cbChampionVipLevel;								//彩金池冠军VIP等级
	//CT_LONGLONG						m_llChampionRew;									//彩金池冠军获得奖励
	//CT_DWORD							m_dwUserID;											//彩金池冠军id
private:
	ITableFrame*					m_pGameDeskPtr;										//游戏指针
	CGameLogic						m_GameLogic;										//游戏逻辑
	RecordScoreInfo					m_RecordScoreInfo[GAME_PLAYER];						//记录积分
	NN_CMD_S_GameEnd				m_GameEnd;
	NN_CMD_S_Game_FlashArea			m_FlashArea;										//闪牌区域
	CT_DWORD						m_dwPingTime;

public:
	struct tagBlackListWinScore
	{
		CT_WORD 	wChairID;
		CT_WORD 	wLostRatio;
		CT_BYTE     cbMaxJettonArea;
		CT_LONGLONG llWinScore;
	};
	
	struct tagAreaRealBet
    {
        CT_BYTE     cbAreaID;
        CT_LONGLONG llRealBetScore;
    };
};


#endif