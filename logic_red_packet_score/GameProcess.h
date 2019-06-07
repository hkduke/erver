#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_HB.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include <map>
#include <deque>
#include <list>
#include <fstream>

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
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) {m_sllStockScore = llTotalStock;}
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) {}
	//设置机器人库存
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) {m_GameLogic.SetRealColorPool(llAndroidStock); m_sllRealColorPoolInitValue = llAndroidStock;}
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
	virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData);
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
    CT_VOID NormalGameEnd(CT_DWORD dwHBID);
	//发送总结算
	void SendTotalClearing(std::string strTime);
    //申请发红包
    CT_BOOL OpenHB(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);
    //批量发红包
    CT_BOOL ApplyBatchFaHB(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);
	//撤销发红包
	CT_BOOL RevokeFaHB(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);
	//获取被抢红包信息
	CT_BOOL GetHBGrabedInfo(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);
	//解释私人场数据
	void ParsePrivateData(CT_CHAR *pBuf);
	//读取json配置文件
	void ReadJsonCfg();
	//读文件
	bool ReadFile(std::string & file, std::string & str);
    //是否在抢红包列表内
    CT_BOOL IsInOpenHBListByUserID(CT_DWORD dwUserID);
	//是否在申请发红包列表内
    CT_BOOL IsInApplyListByUserID(CT_DWORD dwUserID);
    //在申请发红包列表内的个数
    CT_WORD GetHBCountInApplyListByUserID(CT_DWORD dwUserID);
	//删除用户战绩及排行榜
	CT_VOID DeleteUserByChairID(CT_WORD wChairID);
    CT_VOID SendRankings(CT_DWORD dwChairID = INVALID_CHAIR);
	//计算排行榜
	CT_VOID CalculateRankings(CT_BOOL bIsCalculate = true);
    //游戏结束后处理
    void GameEndReal();
    //判断是否需要点控
    CT_VOID JudgeIsPointCtrl(CT_DWORD dwHBID);
    //删除正在发的红包
    CT_VOID DeleteHBByHBID(CT_DWORD dwHBID = 0);
    //机器人发红包
    CT_VOID RobotFaHB();
    //统计机器人位置信息
    CT_VOID StatisticalRobotLocationInfo();
    //改变发红包玩家的发红包信息内的原分数
	CT_VOID ChangeFaSource(CT_DWORD dwUserID, CT_LONGLONG llChangeScore);
	//填充抢红包信息
	CT_VOID FillGrabHBInfo(CT_DWORD dwHBID, CT_DWORD dwUserID, GS_PlayerBaseData stGrabHBPlayer[]);
	//机器人立即抢红包
    CT_VOID RobotGrabHB(CT_DWORD dwHBID);
    CT_BOOL IsBlackListUserOnline();
	CT_VOID GrabLastOneHBBlackListUserDear(tagHBInfo & stHBInfo, int index, int iRobotIndex, CT_BOOL bIsSelfFaSelfGrab);
protected:
	static CT_LONGLONG 				m_sllStockScore;										//库存
	static CT_LONGLONG 				m_sllGrabWinScore;										//抢红包玩家赢的抢总和
	static CT_LONGLONG 				m_sllFaWinScore;										//发红包玩家赢的抢总和
	static CT_LONGLONG 				m_sllTotalRealRevenue;									//真人玩家总税收
    CT_LONGLONG                     m_sllRealColorPoolInitValue;

private:
	CT_DWORD						m_dwSysTime;						//系统时间
	CT_BYTE							m_cbGameStatus;						//游戏状态
    CT_DWORD 						m_dwHBIDIndex;							//红包ID索性
    CT_DWORD						m_dwOldTime;						//对配置的时间
    //游戏信息
protected:
    CT_DWORD 									m_dwHBID;
	std::map<CT_DWORD, tagHBInfo> 				m_FaingHB;                        //红包ID,正在发的红包申请者及红包信息
	std::list<tagHBInfo>							m_ApplyList;						//申请发红包列表

	std::map<CT_DWORD, CMD_S_GameEnd>					m_mGameEnd;			//红包ID,红包结算信息
    std::vector<CT_WORD>            m_vRobotChairID;                    //机器人的座位号
    std::map<CT_DWORD, GS_OnlinePlayerExploits>               	m_mOnlinePlayerExploits;           //在线玩家抢红包战绩统计UserID
    std::map<CT_DWORD, CT_DOUBLE >                              m_mOnlinePlayerTotalExploits;       //在线玩家抢发红包战绩统计UserID
	std::vector<GS_OnlinePlayerExploits>						m_PlayerCountsRanking;			//在线玩家战绩排行榜
	std::vector<GS_OnlinePlayerExploits>						m_PlayerWinScoresRanking;			//在线玩家战绩排行榜
	std::map<CT_DWORD ,GS_SimpleUserInfo>                       m_mUserBaseData;					//userId，玩家信息
    static std::map<CT_DWORD, MSG_G2DB_User_PointControl> 	    m_mapUserPointControl;					//用户点控数据
    static std::map<CT_DWORD, tagHBGameBlackList>				m_mapBlackList;						//userId，将只抢最后一个红包的玩家加入黑名单
	//倍数信息
protected:
	//出牌信息
protected:
	CT_BYTE 											m_cbLastRealPlayerCount;
	std::map<CT_DWORD,CT_BYTE>							m_mRealPlayerCount;			//上局抢红包的真实玩家数量

//配置数据
private:
	//时间信息
    std::map<CT_DWORD,CT_BYTE>						m_mOpTotalTime;					//出牌时间(红包ID，总时间)
    std::map<CT_DWORD,CT_DWORD>						m_mOpStartTime;					//出牌时间戳(红包ID，开始时间)

private:
	//房间信息
	//CT_DWORD						m_dwGameType;						//游戏类型.0:游戏场;1:比赛场.
	CT_DWORD						m_dwlCellScore;						//基础积分
	CT_DWORD						m_dwRandIndex;							//随机数索引
private:
	ITableFrame*					m_pGameDeskPtr;						//游戏指针
	CGameLogic						m_GameLogic;						//游戏逻辑

private:
	tagGameRoomKind*				m_pGameRoomKindInfo;				//房间配置
	PrivateTableInfo				m_PrivateTableInfo;					//获取私人房信息
	std::map<CT_DWORD ,std::vector<RecordScoreInfo>>					m_mRecordScoreInfo;		//红包ID,记录积分
	std::vector<GS_HBRoomBaseCfg>	m_vHBRoomConfig;					//红包房间配置
	std::vector<GS_RobotGrabHBCfg>  m_vRobotGrabHBCfg;					//机器人抢红包配置
	std::vector<GS_RobotApplyFaHBCfg>	m_vRobotApplyFaHBCfg;			//机器人申请发红包配置
	GS_RobotQuitCfg 					m_RobotQuitCfg;					//机器人退出配置
	GS_StockKillScoreCfg	            m_StockKillScoreCfg;			//机器人库存杀分配置
	std::vector<GS_BatchHBCountVIPCfg>  m_vFaHBCountVIPCfg;              //批量发红包数量与VIP配置
	GS_PointCtrlCfg	    				m_PointCtrlCfg;					//点控配置
    GS_HBRoomBaseCfg*					m_pHBRoomConfig;					//红包房间配置
    std::vector<GS_RobotFaHBAllotCfg>   m_vRobotFaHBAllotCfg;
	std::vector<GS_ColorPoolCfg>   		m_vColorPoolCfg;						//彩池配置
	CT_BOOL                             m_bCtrlMasterSwitch;                   //是否控制总开关
	CT_INT32 							m_iGrabLastOneHBProba;					//机器人抢最后一个红包的概率
    GS_BlackListCfg                     m_BlackListCfg;                  //用户黑名单配置
    GS_HBButtonCfg                      m_HBButtonCfg;                   //红包按钮配置
private:
	std::random_device m_rd;
	std::mt19937 m_gen;
    std::uniform_int_distribution<unsigned int> m_iProba;
};


#endif	//___DEF_GAME_PROCESS_H___
