#ifndef	___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_FXGZ.h"
#include "CGlobalData.h"
#include <map>
#include <random>
#include <memory>

//游戏流程
class CGameProcess : public ITableFrameSink
{
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
	//获取大局结算的总分
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID);
	//设置桌子库存
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) { /*m_llStock = llTotalStock;*/ }
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) { }
	//设置机器人库存
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) {}
	//设置机器人库存下限
	virtual CT_VOID SetAndStockLowerLimit(CT_LONGLONG llAndroidStockLowerLimit) {}
	//设置系统通杀概率
	virtual CT_VOID SetSystemAllKillRatio(CT_WORD wSystemAllKillRatio) {}
	//设置今日库存值
	virtual CT_VOID SetTodayStock(CT_LONGLONG llTodayStock) { /*m_llTodayStock = llTodayStock;*/ }
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
	virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData);
	//设置炸金花的牌库概率
	virtual CT_VOID SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio) {}
	//捕鱼获取桌子上玩家的输赢值
	virtual CT_VOID GetAllPlayerAddScore(std::vector<tagPlayerAddScore> &vecAddScore) {}
    //设置血控系统数据(新)
    virtual CT_VOID SetStockControlInfo(std::vector<tagStockControlInfo>& vecStockControlInfo);
	//获取桌子的鱼种统计信息
	virtual CT_VOID GetAllPlayerFishStatisticInfo() {}

private:
	//发送数据
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf=nullptr, CT_WORD dwLen=0, CT_BOOL bRecord = false);

    //功能函数
private:
    //初始化游戏数据
    void InitGameData();
    //游戏开始
    bool GameStart(CT_DWORD dwChairID, FXGZ_CMD_C_Start* pStart);
	//生成游戏数据
	void GenerateMainGameIcon(CT_DWORD dwUserID, CT_BYTE cbIcon[ICON_ROW][ICON_COL], int nRoomIndex,
	                            std::map<int, std::vector<int>> &mapIcon, unsigned int &nTotalMult);
	//生成免费游戏数据
	void GenerateFreeGameIcon(CT_BYTE cbIcon[ICON_ROW][ICON_COL], std::map<int, std::vector<int>> &mapIcon,
							  unsigned int &nTotalMult, unsigned int& nFreeGameTotalMult);
	//计算血池状态
	inline void CalcStockStatus(int nRoomIndex);
	//计算游戏是属于哪一个场次
	inline int GetJettonRoomIndex(CT_LONGLONG llJettonScore);

private:
	//点控数据
	struct tagDianKongData
	{
		CT_DWORD		dwUserID;			//用户ID
		CT_INT32 		iDianKongZhi;		//点控作弊值
		CT_LONGLONG 	llDianKongFen;		//点控分数
		CT_LONGLONG     llCurDianKongFen;   //当前点控分数
	};

	//血池状态
	enum en_StockStatus
    {
	    en_EatScore = 1,                    //吃分
        en_OutScore = -1,                   //放分
    };

private:
    //游戏信息
	CT_WORD					m_wGameStatus;										//游戏状态

private:
    //房间信息
    CT_DWORD				m_dwCellScore;										//基础积分
    CT_UINT64 				m_uLastSpinTime;									//上一次转轴时间
    CT_LONGLONG 			m_llUserWinScore;									//用户输赢分
    //CT_WORD 				m_wUnLuckyCount;									//没中奖次数
    //CT_LONGLONG 			m_llLastJetton;										//上一次压注数目

private:
	ITableFrame*			m_pGameDeskPtr;										//游戏指针

private:
	//static	CT_DWORD 		m_dwJetton[JETTON_COUNT];								//下注金额
	//static  CT_LONGLONG 	m_llStock;													//累计库存
	//static  CT_LONGLONG     m_llTodayStock;                                     		//今日库存值
    //static  CT_DWORD        m_dwTodayTime;                                      		//当前时间
    static  tagStockControlInfo 				m_stockControlInfo[ROOM_NUM];           //血池系统(新)信息
    static  CT_LONGLONG 						m_llStockTaxTemp[ROOM_NUM];			    //血池收税临时变量
	static std::map<CT_DWORD, tagDianKongData> 	m_mapUserPointControl;					//用户点控数据
	//static en_StockStatus                       m_enStockStatus;                      //血池状态
	static int                                  m_nStockCheatVal[ROOM_NUM];             //血池作弊率
};


#endif	//___DEF_GAME_PROCESS_H___














