#ifndef		___DEF_GAME_PROCESS_H___
#define	___DEF_GAME_PROCESS_H___

#include "CTType.h"
#include "ITableFrameSink.h"
#include "DefGameLogicExport.h"
#include "MSG_LD_FPF.h"
#include "GameLogic.h"
#include "CGlobalData.h"
#include <map>

//吃牌命令
struct UserAction
{
	CT_BYTE									cbActionCode[ACTION_COUNT];								//动作代码（吃，碰，跑，喂，提，胡）
	CT_BYTE									cbActionCard;									//动作扑克
	CT_BYTE									cbChiWeaveCount;							//吃牌组合个数(吃用)
	CT_BYTE									cbChiWeaveCard[MAX_ITEM_COUNT][3];	//吃牌组合牌数据(吃用)
	CT_BYTE									cbMustIndex;									//必选索引
};

//无效用户
//const CT_DWORD INVALID_CHAIR = -1;

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
	CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = true);

	//判断
private:
	//判断出牌跑牌
	CT_BOOL EstimateOutCardPao(CT_DWORD dwOutCardUser, CT_BYTE cbOutCard, CT_DWORD& dwPaoUser);
	//判断发牌跑牌
	CT_BOOL EstimateSendCardPao(CT_DWORD dwSendCardUser, CT_BYTE cbSendCard, CT_DWORD& dwPaoUser, CT_BOOL bDelay=true);
	//判断提牌
	CT_BOOL EstimateTiCard(CT_DWORD dwCenterUser, CT_BYTE cbCard);
	//判断偎牌
	CT_BOOL EstimateWeiCard(CT_DWORD dwCenterUser, CT_BYTE cbCard);

	//判断出牌胡
	CT_BOOL EstimateOutCardHu(CT_DWORD dwChairID, CT_DWORD dwOutCardUser, CT_BYTE cbOutCard, CT_BOOL bPaoCard=false);
	//判断发牌胡
	CT_BOOL EstimateSendCardHu(CT_DWORD dwChairID, CT_BYTE cbSendCard, CT_BOOL bTiPaoWei = false);

	//判断发牌是否有跑
	CT_BOOL EstimatePaoCard(CT_DWORD dwChairID, CT_BYTE cbSendCard);
	//判断地胡
	CT_BOOL EstimateDiHu(CT_DWORD dwOutCardUser);

private:
	//清除所有定时器
	void ClearAllTimer();
	//获得游戏配置信息
	void GetGameConfigInfo();

	//功能函数
private:
	//初始化游戏数据
	void InitGameData();
	//解释私人场数据
	void ParsePrivateData(CT_CHAR *pBuf);
	//更新庄家
	void UpdateBankerUser();
	//洗牌
	void XiPai(CT_BYTE	cbCardData[GAME_PLAYER][MAX_COUNT], CT_BYTE cbCardCount[GAME_PLAYER]);
	//是否重新洗牌
	CT_BOOL IsRandCard();
	//对子个数
	CT_BYTE GetDuiZiCount(CT_BYTE cbCardIndex[MAX_INDEX]);
	//手上提牌
	CT_BYTE HandTiCard(CT_DWORD dwChairID, CT_BYTE cbTiCardList[MAX_TI_CARD], CT_BYTE& cbTiCardCount);
	//判断庄家提牌
	CT_BYTE BankerTiCard(CT_BYTE cbTiCardList[MAX_TI_CARD]);
	//玩家模牌
	CT_BOOL SendUserCard(const CT_DWORD dwChairID);
	//普通场游戏结算
	CT_VOID NormalGameEnd();
	//流局游戏结束
	CT_VOID FlowBureauEnd();
	//解散结束游戏
	CT_VOID DismissEnd();
	//获得名堂
	CT_BYTE GetMT(CT_BYTE cbMTType[MAX_MT_COUNT], CT_WORD wMTAward[MAX_MT_COUNT][2]);

	//代打
private:
	//用户出牌
	CT_BOOL OnAutoOutCard(CT_DWORD dwChairID);
	//用户操作
	CT_BOOL OnAutoOperateCard(CT_DWORD dwChairID);
	//是否有动作
	CT_BOOL IsHaveAction(const UserAction& stUserAction);

	//胡牌
private:
	//胡牌结果
	CT_BOOL GetHuCardInfo(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo, CT_BOOL bIsCheckTingCard = false);
	//吃胡分析
	CT_BOOL ChiHuAnalyse(CT_DWORD dwChairID, CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo);
	//碰胡分析
	CT_BOOL PengHuAnalyse(CT_DWORD dwChairID, CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo);
	//跑胡分析
	CT_BOOL PaoHuAnalyse(CT_DWORD dwChairID, CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, tagHuCardInfo & HuCardInfo, CT_BOOL bIsCheckTingCard = false);
	//查找最大组合
	CT_BOOL FindMaxHuCard(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray, tagHuCardInfo & HuCardInfo);

	//提取偎牌
	CT_VOID TakeWeiCard(CT_BYTE cbCardIndex[MAX_INDEX], tagHuCardInfo & HuCardInfo);
	//查找最大组合索引
	CT_INT32 FindMaxHuXiIndex(std::vector<tagAnalyseItem> AnalyseItemArray);
	//查找30卡胡索引
	CT_INT32 Find30KaHuIndex(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray);
	//查找20卡胡索引
	CT_INT32 Find20KaHuIndex(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray);
	//查找一块匾胡索引
	CT_INT32 FindYiKuaiBianHuIndex(CT_BYTE cbHuXi, std::vector<tagAnalyseItem> AnalyseItemArray, tagHuCardInfo & HuCardInfo, CT_INT32 i20KaHuIndex);
	//辅助函数
private:
	//获得下一个玩家
	CT_DWORD GetNextUser(CT_DWORD dwChairID);
	//获得胡息
	CT_BYTE GetHuXi(CT_DWORD dwChairID);

	//游戏事件
private:
	//玩家操作
	CT_INT32 OnUserOperate(CT_DWORD dwChairID, FPF_CMD_C_OperateCard* pOperateData, CT_BOOL bTimeout = CT_FALSE);
	//玩家出牌
	CT_INT32 OnUserOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCard, CT_BOOL bTimeout = CT_FALSE);
	//玩家托管
	CT_BOOL OnUserTrustee(CT_DWORD dwChairID);

private:
	//校验操作
	CT_BOOL CheckUserOperate(CT_DWORD dwChairID, FPF_CMD_C_OperateCard* pOperateData);
	//检查吃牌
	CT_BOOL CheckChiCard(CT_DWORD dwChairID, CT_BYTE cbBaiCard[MAX_ITEM_COUNT][3], CT_BYTE cbBaiCardCount, CT_BYTE cbCurCard);
	//尝试删摆牌
	CT_BOOL TryRemoveBaiCard(CT_DWORD dwChairID, CT_BYTE cbBaiCard[MAX_ITEM_COUNT][3], CT_BYTE cbBaiCardCount, CT_BYTE cbCurCard);
	//检查碰牌
	CT_BOOL CheckPengCard(CT_DWORD dwChairID, CT_BYTE cbCurCard);
	//获得最大动作(1吃，2碰，3胡， 0过)
	CT_BYTE GetMaxAction(CT_DWORD dwChairID);

	//功能函数
private:
	//是否碰跑
	CT_BOOL IsPengPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//是否偎跑
	CT_BOOL IsWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//是否手牌偎跑
	CT_BOOL IsHandWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//设置碰跑
	CT_BOOL SetPengPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BOOL bDelay = true);
	//设置偎跑
	CT_BOOL SetWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BOOL bDelay = true);
	//设置手牌偎跑
	CT_BOOL SetHandWeiPao(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BOOL bDelay = true);
	//是否手牌提牌
	CT_BOOL IsHandTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//是否偎提牌
	CT_BOOL IsWeiTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//设置手牌提牌
	CT_BOOL SetHandTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//设置偎提牌
	CT_BOOL SetWeiTiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//是否偎牌
	CT_BOOL IsWeiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//设置偎牌
	CT_BOOL SetWeiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//获得吃牌
	CT_BOOL GetChiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BYTE cbChiWeaveCard[MAX_ITEM_COUNT][3], CT_BYTE& cbChiWeaveCount, CT_BYTE& cbMustIndex);
	//设置吃牌组合
	CT_BOOL SetChiCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard, CT_BYTE cbChiWeaveCard[MAX_ITEM_COUNT][3], CT_BYTE cbChiWeaveCount);
	//是否碰牌
	CT_BOOL IsPengCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//设置碰牌
	CT_BOOL SetPengCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//设置跑牌
	CT_BOOL SetPaoCard(CT_DWORD dwChairID, CT_BYTE cbCurrentCard);
	//是否重跑
	CT_BOOL IsChongPao(CT_DWORD dwChairID);

private:
	//计算总输赢分
	CT_VOID CalcTotalLWScore();
	//发送总结算
	CT_VOID SendTotalClearing(std::string strTime);

	//判断是否能吃
private:
	//获得能摆组合
	CT_BYTE GetBaiCardWeave(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3], CT_BYTE cbChiCount, CT_BYTE cbCurCard, CT_BYTE WeaveCard[15][MAX_ITEM_COUNT][3]);
	//判断摆牌后能不能出牌
	CT_BOOL EstimateBaiCanOutCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurCard, CT_BYTE cbWeaveCard[100][MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCount, \
		CT_BYTE cbSuccWeave[100][MAX_ITEM_COUNT][3], CT_BYTE& cbSuccCount);
	//删除指定牌
	CT_BOOL RemoveDesCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbChiWeave[3]);
	//删除指定组合牌
	CT_BOOL RemoveDesWeaveCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3]);
	//是否有相同组合
	CT_BOOL IsHaveSameWeave(CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCard[3]);
	//是否有吃相同组合
	CT_BOOL IsHaveChiSameSevae(CT_BYTE cbChiWeaveCard[100][MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCount, CT_BYTE cbWeaveCard[3]);

	//偎后能否出牌
	CT_BOOL IsWeiCanOutCard(CT_DWORD dwChairID);
	//提跑后能否出牌
	CT_BOOL IsTiPaoCanOutCard(CT_DWORD dwChairID);
private:
	//判断听牌
	CT_VOID EstimateTingCard(CT_DWORD dwChairID);


	//测试牌
	CT_VOID TestCard(CT_BYTE cbCardData[GAME_PLAYER][MAX_COUNT]);
private:
	//游戏信息
	CT_DWORD				m_dwSysTime;											//系统时间
	CT_CHAR					m_chUserNickName[GAME_PLAYER][NICKNAME_LEN];			//玩家昵称
	CT_BYTE					m_cbSice[2];											//骰子点数
	CT_BYTE					m_cbGameStatus;											//游戏状态

	CT_BOOL					m_bResponse[GAME_PLAYER];								//响应标志
	UserAction				m_stUserAction[GAME_PLAYER];							//玩家动作
	CT_DWORD				m_dwActionNumber;										//动作流水号
	FPF_CMD_C_OperateCard	m_stUserOperate[GAME_PLAYER];							//玩家操作
	CT_BYTE					m_cbUserQiHu[GAME_PLAYER];								//玩家弃胡

	//用户标识
	CT_DWORD				m_dwBankerUser;											//庄家
	CT_DWORD				m_dwCurrentUser;										//当前玩家
	CT_DWORD				m_dwResumeUser;											//还原用户
	CT_DWORD				m_dwProvideUser;										//供应玩家
	CT_BYTE					m_cbProvideCard;										//供应牌

	//扑克信息
	CT_BYTE					m_cbLeftCardCount;										//剩余数目
	CT_BYTE					m_cbRepertoryCard[MAX_CARD_TOTAL];						//库存扑克
	CT_BYTE					m_cbCardIndex[GAME_PLAYER][MAX_INDEX];					//玩家手牌数据

	CT_BYTE					m_cbDiscardCount[GAME_PLAYER];							//丢弃数目
	CT_BYTE					m_cbDiscardCard[GAME_PLAYER][MAX_COUNT];				//丢弃记录(没有被吃碰的牌)

	CT_BOOL					m_bGuoPengCard[GAME_PLAYER][MAX_INDEX];					//过牌(打出的牌，忍碰的牌)——不能碰
	CT_BOOL					m_bGuoChiCard[GAME_PLAYER][MAX_INDEX];					//过吃的牌(打出的牌，忍吃的牌)——不能吃

	CT_BYTE					m_cbActionStatus;										//动作状态1等待发牌动作,2等待出牌动作
	CT_BYTE					m_cbTiPaoAction;										//提跑动作1提2跑
	CT_BYTE					m_cbWeiAction;											//偎动作(1有偎)

private:
	//特殊番判断标志
	CT_BYTE					m_cbOutCardCount[GAME_PLAYER];							//庄闲出牌次数
	CT_BOOL					m_bTianHuFlag;											//天胡标志
	CT_BOOL					m_bDiHuFlag;											//地胡标志
	CT_BOOL					m_bWeiCannotHu[GAME_PLAYER];							//偎牌后不能胡牌
	CT_BOOL					m_bOutOtherWeiPao[GAME_PLAYER];							//出了别人有偎跑的牌
	CT_BOOL					m_bSendCard;											//是否摸过牌了(判断地胡）
	CT_BOOL					m_bTiPaoCannotOutCard[GAME_PLAYER];						//提跑牌后不能出牌

private:
	//组合扑克
	CT_BYTE					m_cbWeaveCount[GAME_PLAYER];							//组合数目
	tagWeaveItem			m_WeaveArray[GAME_PLAYER][MAX_WEAVE];					//组合扑克

private:
	//结束信息
	CT_DWORD				m_dwHuCardUser;											//胡牌玩家
	CT_BYTE					m_cbHuCard;												//胡的扑克
	CT_BYTE					m_cbHuCardType;											//胡牌类型(0平胡,1放炮,2自模)
	tagHuCardInfo			m_HuCardInfo[GAME_PLAYER];								//胡牌信息
	CT_WORD					m_wMaxHuXi;												//最大胡息

//配置数据
private:
	//时间信息
	CT_BYTE					m_cbOutCardTime;										//出牌时间
	CT_BYTE					m_cbOperateTime;										//操作时间

private:
	//房间信息
	//CT_DWORD				m_dwGameType;											//游戏类型.0:游戏场;1:比赛场.
	//CT_DWORD				m_dwServiceCost;										//服务费用.(%).
	CT_DWORD				m_dwlCellScore;											//基础积分

private:
	ITableFrame*			m_pGameDeskPtr;											//游戏指针
	CGameLogic				m_GameLogic;											//游戏逻辑

	PrivateTableInfo		m_PrivateTableInfo;										//获取私人房信息
	CT_WORD					m_wHuCount[GAME_PLAYER];								//各玩家胡牌次数
	CT_WORD					m_wFPCount[GAME_PLAYER];								//各玩家放炮次数
	CT_WORD					m_wHZCount[GAME_PLAYER];								//各玩家荒庄次数	
	CT_INT32 				m_iTotalHuXi[GAME_PLAYER];								//各玩家累计胡息
	CT_INT32				m_iTotalLWScore[GAME_PLAYER];							//各玩家总输赢分
	RecordScoreInfo			m_RecordScoreInfo[GAME_PLAYER];							//记录积分
};


#endif	//___DEF_GAME_PROCESS_H___














