#pragma once

#include "ITableFrame.h"
#include <CMD_Dip.h>

//桌子回调接口
class ITableFrameSink
{
public:
	ITableFrameSink() {}
	virtual ~ITableFrameSink() {}

public:
	//游戏开始
	virtual void OnEventGameStart() = 0;
	//游戏结束
	virtual void OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag) = 0;
	//发送场景
	virtual void OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser) = 0;

	//事件接口
public:
	//定时器事件
	virtual void	OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam) = 0;
	//游戏消息
	virtual CT_BOOL OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize) = 0;

	//用户接口
public:
	//用户进入
	virtual void OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser) = 0;
	//用户准备
	virtual void OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser) = 0;
	//用户离开
	virtual void OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag = 0) = 0;
	//用户断线
	virtual void OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser) = 0;
	//用户重入
	virtual void OnUserConnect(CT_DWORD dwChairID, CT_BOOL bIsLookUser) = 0;
	//用户换桌
	virtual void OnUserChangeTable(CT_DWORD dwChairID, CT_BOOL bForce) = 0;
	//用户充值 llRechargeScore充值金额 单位: 分
	virtual void OnUserRecharge(CT_DWORD dwChairID, CT_LONGLONG llRechargeScore) = 0;

public:
	//设置指针
	virtual void SetTableFramePtr(ITableFrame* pTableFrame) = 0;
	//清理游戏数据
	virtual void ClearGameData() = 0;
	//私人房游戏大局是否结束
	virtual bool IsGameRoundClearing(CT_BOOL bDismissGame) = 0;
	//获取大局结算的总分
	virtual CT_INT32 GetGameRoundTotalScore(CT_DWORD dwChairID) = 0;
	//设置桌子库存(包括机器人库存值)
	virtual CT_VOID SetTableStock(CT_LONGLONG llTotalStock) = 0;
	//设置桌子库存下限
	virtual CT_VOID SetTableStockLowerLimit(CT_LONGLONG llStockLowerLimit) = 0;
	//设置机器人库存
	virtual CT_VOID SetAndroidStock(CT_LONGLONG llAndroidStock) = 0;
	//设置机器人库存下限
	virtual CT_VOID SetAndStockLowerLimit(CT_LONGLONG llAndroidStockLowerLimit) = 0;
	//设置系统通杀概率
	virtual CT_VOID SetSystemAllKillRatio(CT_WORD wSystemAllKillRatio) = 0;
	//设置今日库存值
	virtual CT_VOID SetTodayStock(CT_LONGLONG llTodayStock) = 0;
	//设置今日库存上限
	virtual CT_VOID SetTodayStockHighLimit(CT_LONGLONG llTodayHighLimit) = 0;
	//设置换牌概率
	virtual CT_VOID SetChangeCardRatio(CT_WORD wChangeCardRatio) = 0;
	//设置系统必杀和换牌概率(分不同的阶段)
	virtual CT_VOID SetSystemOprateRatio(std::vector<tagBrGameOperateRatio> &vecChangeCardRatio) = 0;
	//设置游戏黑名单
	virtual CT_VOID SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList) = 0;
	//设置基础捕获概率
	virtual CT_VOID SetBaseCapturePro(int nBloodPoolStateValue) = 0;
	//设置捕鱼点控数据
	virtual CT_BOOL SetFishDianKongData(MSG_D2CS_Set_FishDiankong *pDianKongData) = 0;
	//设置炸金花的牌库概率
	virtual CT_VOID SetZjhCardDepotRatio(tagZjhCardDepotRatio* pZjhCardDepotRatio) = 0;
	//捕鱼获取桌子上玩家的输赢值
	virtual CT_VOID GetAllPlayerAddScore(std::vector<tagPlayerAddScore> &vecAddScore) = 0;
	//设置血控系统数据(新)
	virtual CT_VOID SetStockControlInfo(std::vector<tagStockControlInfo>& vecStockControlInfo) = 0;
	//获取桌子的鱼种统计信息
	virtual CT_VOID GetAllPlayerFishStatisticInfo() = 0;
	//设置极速捕鱼数据 其他游戏桌子对这个函数不感兴趣，所以在基类中实现。
	virtual CT_VOID SetJSFishData(enJSFishDataType type, void* pData){}
};

typedef ITableFrameSink*       (*LpDllNewDesk)();                   //生成桌子
typedef void*                  (*LpDllDelDesk)(ITableFrameSink* p); //删除桌子
typedef void(*LpDllValidate)(CT_DWORD& dwLittlePlayNums, CT_DWORD& dwManyPlayNums);//确认数据

#define DLL_NEW_DESK_FN_NAME     ("GetTableInstance")               //得到桌子的实例
#define DLL_DEL_DESK_FN_NAME     ("DelTableInstance")               //删除桌子的实例
#define DLL_VALIDATE_ROOM_CONFIG ("ValidateRoomConfig")             //确认房间配置