//库存配置类
#pragma once

#include <inttypes.h>
#include <time.h>
struct StockInfo
{
	int64_t valueA;
	int64_t valueB;
	int64_t valueC;
	int64_t valueD;
	int64_t curStockValue;
	int 	nRevenueCount;

};

//记录游戏库存、配置
class BYStockConfig
{

public:
	//构造函数
	BYStockConfig();
	//析构函数
	~BYStockConfig();
    void init();
public:
	//设置库存
	//void SetStockValue(SVar &s);
	//获取库存
	void GetStockValue(StockInfo &si);
	//接口函数
public:
	//设置/获取底值(必杀)
	void  SetValueA(int64_t lValue) { m_lValueA = lValue; }
	int64_t GetValueA(){ return m_lValueA; }
	
	//设置/获取顶值（概率杀分）
	void  SetValueB(int64_t lValue) { m_lValueB = lValue; }
	int64_t GetValueB() { return m_lValueB; }

	//设置/获取底值（概率放水）
	void  SetValueC(int64_t lValue) { m_lValueC = lValue; }
	int64_t GetValueC(){ return m_lValueC; }
	
	//设置/获取顶值（必放）
	void  SetValueD(int64_t lValue) { m_lValueD = lValue; }
	int64_t GetValueD() { return m_lValueD; }
	//设置/获取当前库存
	void SetCurValue(int64_t lCurValue) {   m_lCurValue += lCurValue;}
	int64_t GetCurValue(){ return m_lCurValue; }
	void InitCurValue(int64_t llCurValue) {m_lCurValue = llCurValue;}

	//税收相关
public:
	//获得税收计数
	int   GetRevenueSwitch() { return m_nRevenueSwitch; }
	//设置税收计数
	void  SetRevenueSwitch() { m_nRevenueSwitch++; }
	//充值税收计数
	void  ResetRevenueSwitch() { m_nRevenueSwitch = 0; }
	//收税炮数
	int   GetRevenueCount() { return m_nRevenueCount; }
	//设置炮数
	void  SetRevenueCount(int nPaoCount) {m_nRevenueCount = nPaoCount;}
    //设置税收
	void  SetCureentRevenue(int64_t lRevenue) {m_lRevenue+=lRevenue;}
	void  InitCurrentRevenue(int64_t llRevenue) { m_lRevenue = llRevenue;}
	int64_t GetCurrentRevenue(){return m_lRevenue;}

	void InitSysWinScore(int64_t llSysWinScore) {m_llSystem_win_score = llSysWinScore;}
	void AddSysWinScore(int64_t llAdd) {m_llSystem_win_score += llAdd;}
	int64_t GetSysWinScore(){return m_llSystem_win_score;}

	void InitSysLoseScore(int64_t llSysLoseScore) {m_llSystem_lose_score = llSysLoseScore;}
	void AddSysLoseScore(int64_t llAdd) {m_llSystem_lose_score+=llAdd;}
	int64_t GetSysLoseScore(){return m_llSystem_lose_score;}

public:
    //当日数据
    //所有桌子的当日总吸分
    long long  m_llTodayTotalWinScore;
    //所有桌子的当日总放分
    long long  m_llTodayTotalLoseScore;
    //所有桌子的当日库存
    long long  m_llTodayTotalKuCun;
    //所有桌子的当日税收
    long long  m_llTodayTotalTax;
    //所有桌子的当日点控输赢
    long long  m_llTodayDianKongWinLoseTotalScore;
    //所有桌子当日总进入人次
    long long  m_dwTodayTotalEnterCount;
    //所有桌子当日总赢钱玩家数
    unsigned int m_dwTodayTotalPlayerCount_Win;
    //所有桌子当日总破产玩家数
    unsigned int m_dwTodayTotalPlayerCount_Bankrupted;
    //所有桌子当日总输但是非破产的玩家数
    unsigned int m_dwTodayTotalPlayerCount_Lose;
    //所有桌子当日不输不赢的玩家数
    unsigned int m_dwTodayTatalPlayerCount_NoWinLose;
    //所有桌子放分次数
    unsigned int  m_dwTodayRewardCount;
    //所有桌子吸分次数
    unsigned int  m_dwTodayXiFenCount;
    time_t  m_today_time;
    void ResetTodayInfo();
	//void SetSysCurTax(int64_t llCurTax) {m_llSystem_cur_tax = llCurTax;}
	//int64_t GetSysCurTax(){return m_llSystem_cur_tax;}
	//内部函数
private:

	//定时器函数
private:
	//删除定时器
	void				KillTimer(uint64_t dTimerID);
	//定时函数
	void				OnTimerFun(uint64_t id, void *pData);
	//获取配置
	void                OnTimeGetStockConfig(uint64_t id, void *pData);
	//库存配置信息
private:
	int64_t  m_lValueA;     //底值(越界，必杀)
	int64_t  m_lValueB;     //底值(越界，概率杀分)
	int64_t  m_lValueC;     //底值(越界，概率放水)
	int64_t  m_lValueD;     //底值(越界，必放)
	int64_t  m_lCurValue;     //当前库存

	//税收总额
	int64_t m_lRevenue;
	//税收计数
	int m_nRevenueSwitch;
	//收税炮数(例如设置为100,100炮收一炮税)
	int m_nRevenueCount;

	//时间定义
private:
	//时间标识
	uint64_t	m_dTimerID;

	//时间标识 没有调用屏蔽
	//uint64_t	m_dStockTimerID;

	int64_t m_llSystem_win_score;
	int64_t m_llSystem_lose_score;
	//int64_t m_llSystem_cur_tax;
	//int64_t m_llSystem_diankong;
};