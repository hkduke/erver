#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_jznc.h"
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
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) {m_sdStockScore = (CT_DOUBLE)llTotalStock;}
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
	//解释私人场数据
	void ParsePrivateData(CT_CHAR *pBuf);
	//读取json配置文件
	void ReadJsonCfg();
	void AnalysisJsonAutoCtrl(acl::json & jsonTmp, const char * pValue, std::vector<tagPlayerAutoCtrlCfg> & WinLossCtrl);
	//读文件
	bool ReadFile(std::string & file, std::string & str);
    CT_BOOL OnUserBet(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);
    CT_BOOL OnUserThrowDice(CT_WORD wChairID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize);
	int GetCtrlEffectKey(CT_WORD wChairID);
	//删除用户战绩及排行榜
	CT_VOID DeleteUserByChairID(CT_WORD wChairID);
	CT_VOID DelayWriteScore();
protected:
	static CT_DOUBLE 				m_sdStockScore;										//库存

private:
	CT_DWORD						m_dwSysTime;						//系统时间
	CT_BYTE							m_cbGameStatus;						//游戏状态
    CT_DWORD						m_dwOldTime;						//对配置的时间

    //游戏信息
protected:
	static std::map<CT_DWORD, MSG_G2DB_User_PointControl> 	    m_mapUserPointControl;					//userId，点控用户
    static std::map<CT_DWORD, tagJZNCGameBlackList>				m_mapBlackList;							//userId，黑名单用户
    static std::map<CT_DWORD, GS_SimpleUserInfo>				m_mapUserBaseInfo;						//userId，用户信息

protected:
	std::vector<CT_WORD>            m_vRobotChairID;                    //机器人的座位号
    std::vector<CT_WORD>            m_vEffectRobotChairID;                    //机器人的座位号
	//倍数信息
protected:
    tagPlayerBet                    m_PlayerBet;
    tagPlayerDice                   m_PlayerDice;
    CT_BYTE                         m_cbThrowDice[GAME_PLAYER];
    CT_WORD                         m_wPlayerCount;
    CT_WORD                         m_wEffectRobotCount;                //已经参与特效控制的机器人数量
private:
	//房间信息
	CT_DWORD						m_dwGameType;						//游戏类型.0:点数房;1:大小房.
	CT_DWORD						m_dwlCellScore;						//基础积分
	CT_DWORD						m_dwRandIndex;						//随机数索引
private:
	ITableFrame*					m_pGameDeskPtr;						//游戏指针
	CGameLogic						m_GameLogic;						//游戏逻辑

private:
	tagGameRoomKind*				m_pGameRoomKindInfo;				//房间配置
	PrivateTableInfo				m_PrivateTableInfo;					//获取私人房信息
	RecordScoreInfo					m_RecordScoreInfo[GAME_PLAYER];					//记录积分
    CT_LONGLONG                     m_llScore[GAME_PLAYER];
    CT_DOUBLE                       m_llWinLoseScore[GAME_PLAYER];
    CT_DWORD                        m_dwRevenue[GAME_PLAYER];
    //统计类变量
private:
    static CT_LONGLONG              m_sllStatisticsPointCount[6];
    static CT_LONGLONG              m_sllRealWinTotalScore;
    static CT_LONGLONG              m_sllRealLoseTotalScore;
    static CT_LONGLONG              m_sllRealTotalRevenue;
    static CT_LONGLONG              m_sllRealWinTotalCount;
    static CT_LONGLONG              m_sllRealLoseTotalCount;
    static CT_LONGLONG              m_sllRealPlayTotalCount;
	//配置数据
private:
	std::map<CT_DWORD, std::vector<tagAIStockCtrlCfg>>			    m_mAIStockCtrlCfg;	//库存控制配置，房间roomkindID，配置
	std::vector<tagPlayerAutoCtrlCfg>				                m_vWinCtrlCfg;		//玩家赢的控制
	std::vector<tagPlayerAutoCtrlCfg>				                m_vLoseCtrlCfg;		//玩家输的控制
    std::vector<tagPointCtrlCfg>				                    m_vPointtrlCfg;		//点控配置
	CT_BOOL                                                         m_bIsAllowSame;     //是否允许出现个玩家点数相同
	CT_DWORD                                                        m_dwRollDice;       //投币等待结束,提示掷骰的最大时间,超时自动掷骰
	CT_DWORD                                                        m_dwBetCoin;        //投币最大时间,超时自动投币
    CT_DWORD                                                        m_dwReadyMatch;     //投币最大时间,超时自动投币
private:
	std::random_device 				m_rd;
	std::mt19937 					m_gen;
    std::uniform_int_distribution<unsigned int>			 		    m_iProba;
};


#endif	//___DEF_GAME_PROCESS_H___
