#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_ZJH.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include <map>
//#include <sstream>
#include <random>

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
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户准备
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户离开
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0);
	//用户断线
	virtual void OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户重入
	virtual void OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser) {};
	//用户换桌
	virtual void OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce);
	//用户充值 llRechargeScore充值金额 单位: 分
	virtual void OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore) {}

public:
	//设置指针
	virtual void SetTableFramePtr(ITableFrame* pTableFrame);
	//清理游戏数据
	virtual void ClearGameData();
	//私人房游戏大局是否结束
	virtual bool IsGameRoundClearing(CT_BOOL bDismissGame);
	//	获取大局结算的总分
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID);
	//设置桌子库存
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) { m_lStockScore = llTotalStock; }
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) { m_lStockLowLimit = llStockLowerLimit; }
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
	virtual CT_VOID SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio);
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
	inline void ClearAllTimer();
    //普通场游戏结算
    CT_VOID NormalGameEnd(CT_DWORD dwWinChairID);
	//发送总结算
	void SendTotalClearing(std::string strTime);
	//解释私人场数据
	void ParsePrivateData(CT_CHAR *pBuf);
	//获得下一个玩家
	inline CT_WORD GetNextUser(CT_WORD wChairID);
	//获取至少金币数
	inline CT_LONGLONG GetLeastUserScore();
	//获取是否可以火拼
	inline CT_BOOL CheckCanRush();
	//随机发牌
	CT_VOID SendCardRand();
	//控制发牌
	CT_VOID SendCardControl();

    //游戏事件
private:
	//放弃事件
	bool OnUserGiveUp(CT_WORD wChairID, bool bExit = false);
	//看牌事件
	bool OnUserLookCard(CT_WORD wChairID);
	//比牌事件
	bool OnUserCompareCard(CT_WORD wFirstChairID, CT_WORD wNextChairID);
	//开牌事件
	bool OnUserOpenCard(CT_WORD wUserID);
	//加注事件
	bool OnUserAddScore(CT_WORD wChairID, CT_LONGLONG lScore, bool bCompareUser);
	//孤注一掷
	bool OnUserAllIn(CT_WORD wChairID);
	//处理跟到底
	bool OnUserAutoFollow(CT_WORD wChairID);
	//取消跟到底
	bool OnUserCancelAutoFollow(CT_WORD wChairID);
	//火拼
	bool OnUserRush(CT_WORD wChairID);

	//获得下一个玩家
	//CT_DWORD GetNextUser(CT_DWORD dwChairID);
	//所有玩家进入准备
	void OnAllUserReady();
	//处理all消息
	//void HandleAllIn(bool bGiveUp);
	void HandleAllIn();
	//处理火拼消息
	void HandleRush(bool bGiveUp);
	//处理auto follow消息
	void HandleAutoFollow(CT_WORD wChairID);
	//处理超过局数比牌
	bool HandleMorethanRoundEnd();

	//功能函数
protected:
	//扑克分析
	void AnalyseStartCard();
	void AnalyseBlackListCard();

	//获取一手散牌
	CT_BOOL GetSanPai(CT_BYTE cbCardData[]);
	//获取对子牌
	CT_BOOL GetDuiZiPai(CT_BYTE cbCardData[]);
	//获取顺子牌
	CT_BOOL GetShunZiPai(CT_BYTE cbCardData[]);
	//获取金花
	CT_BOOL GetJinHuaPai(CT_BYTE cbCardData[]);
	//获取顺金
	CT_BOOL GetShunJinPai(CT_BYTE cbCardData[]);
	//获取豹子
	CT_BOOL GetBaoZiPai(CT_BYTE cbCardData[]);
	//获取随机牌
	CT_BOOL GetRandCard(CT_BYTE cbCardData[]);
	//1号牌库
	CT_BOOL GetCardFromDepot1(CT_BYTE cbCardData[]);
	//2号牌库
	CT_BOOL GetCardFromDepot2(CT_BYTE cbCardData[]);
	//更新玩家好牌信息
	inline CT_VOID UpdateGoodCardInfo(CT_WORD wChairID, CT_BOOL bWin);
	//更新玩家的游戏局数
	inline CT_VOID UpdateUserScoreInfo(CT_WORD wChairID, CT_LONGLONG llWinScore, CT_DWORD dwRevenue);

protected:
	CT_WORD							m_wCurrentUser;							//当前用户
	CT_WORD							m_wBankerUser;							//庄家用户
	CT_WORD							m_wWinnerUser;							//赢家
	//CT_WORD							m_wFlashUser[GAME_PLAYER];				//动画用户

	//用户状态
protected:
	CT_BYTE							m_cbGiveUp[GAME_PLAYER];				//别人不能看到弃牌玩家的牌,游戏结束的时候用
	CT_BYTE							m_cbSystemGiveUp[GAME_PLAYER];			//系统弃牌
	CT_BOOL							m_cbPlayStatus[GAME_PLAYER];			//游戏状态[表示还在玩]
	CT_BYTE							m_bGameStatus[GAME_PLAYER];				//游戏状态[表示参加过游戏，比牌弃牌和比牌失败也算，新开游戏时清0]
	CT_BYTE							m_bGameStatus2[GAME_PLAYER];			//游戏状态[[表示参加过游戏，比牌弃牌和比牌失败也算，离开桌子时清0]]
	CT_BOOL							m_bAutoFollow[GAME_PLAYER];				//是否跟到底
	CT_WORD							m_wAddScoreCount[GAME_PLAYER];			//下注次数
	//CT_LONGLONG						m_lCompareCount;						//比牌状态
	bool							m_bGameEnd;								//结束状态
	CMD_S_GameEnd					m_StGameEnd;							//结束数据
	CT_DWORD						m_dwPlayUserID[GAME_PLAYER];			//参加游戏的玩家ID
	CT_BYTE							m_cbRealPlayer[GAME_PLAYER];			//真人玩家
	CT_BYTE							m_cbAndroidStatus[GAME_PLAYER];			//机器状态

	//扑克变量
protected:
	//扑克信息
	CT_BYTE							m_cbSendCardCount; 						//剩余数目
	CT_BYTE							m_cbRepertoryCard[MAX_CARD_TOTAL];		//库存扑克
	CT_BYTE							m_cbHandCardData[GAME_PLAYER][MAX_COUNT];//桌面扑克
	CT_BYTE							m_cbHandCardType[GAME_PLAYER];			//牌型
	std::vector<CT_WORD>			m_wCompardUser[GAME_PLAYER];			//扑克数组
	CT_BYTE 						m_cbCardDepotRatio[2][CT_BAO_ZI];		//牌型概率

	//下注信息
protected:
	CT_LONGLONG						m_llUserSourceScore[GAME_PLAYER];		//玩家原始金币
	CT_LONGLONG						m_lTableScore[GAME_PLAYER];				//下注数目
	//CT_LONGLONG						m_lUserMaxScore[GAME_PLAYER];			//最大下注
	//CT_LONGLONG						m_lMaxCellScore;						//单元上限
	CT_LONGLONG						m_lCellScore;							//单元下注
	CT_LONGLONG						m_lCurrentJetton;						//当前筹码
	bool							m_bMingZhu[GAME_PLAYER];				//看明下注.
	//CT_LONGLONG						m_lMaxScore;							//封顶值
	CT_DWORD						m_dwOpEndTime;							//操作结束时间
	CT_WORD							m_wTotalOpTime;							//操作总时间
	bool							m_bRushState;							//火拼状态
	CT_WORD							m_wStartRushUser;						//发起火拼用户
	CT_LONGLONG						m_lRushJetton;							//火拼金额
	//std::stringstream				m_strRecord;							//日志记录
				
	//库存变量
protected:
	static CT_LONGLONG				m_lStockScore;							//总输赢分
	static CT_LONGLONG				m_lStockLowLimit;						//库存下限
	std::vector<tagBrGameBlackList>	m_vecBlackList;							//黑名单

	//CT_LONGLONG						m_lStorageDeduct;						//回扣变量
	//CT_LONGLONG						m_lStorageMax;							//库存封顶
	//int								m_nStorageMul;							//系统输钱比例
	//CT_LONGLONG						m_lBeginStorage;						//服务启动时读取

	//属性变量
protected:
	static const CT_WORD			m_wPlayerCount;							//游戏人数
	//static CT_WORD					m_wLargeJetton[MAX_JETTON_NUM];			//大筹码
	//static CT_WORD					m_wMediumJetton[MAX_JETTON_NUM];		//中筹码
	//static CT_WORD					m_wSmallJetton[MAX_JETTON_NUM];			//小筹码
	//CT_WORD							m_wCurJetton[MAX_JETTON_NUM];			//当前筹码
	CT_LONGLONG						m_llMaxJettonScore;						//最大下注筹码

private:
	ITableFrame*					m_pGameDeskPtr;							//游戏指针
	CGameLogic						m_GameLogic;							//游戏逻辑
	CT_BYTE							m_cbGameStatus;							//游戏状态

	std::random_device 				m_random_device;
	std::mt19937 					m_random_gen;

	struct tagGoodCardParam
	{
		tagGoodCardParam()
			:wChairID(INVALID_CHAIR)
			,cbParam(0)
		{
			//std::cout<< "tagGoodCardParam()" << std::endl;
		}

		~tagGoodCardParam()
		{
			//std::cout<< "~tagGoodCardParam()" << std::endl;
		}
		CT_WORD     wChairID;
		CT_BYTE     cbParam;
	};
	//PrivateTableInfo				m_PrivateTableInfo;						//获取私人房信息
};
#endif	//___DEF_GAME_PROCESS_H___
