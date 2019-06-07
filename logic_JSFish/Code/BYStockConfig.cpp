
#include "stdafx.h"
#include "BYStockConfig.h"
#define TIME_PLUSE 2 * 60 * 1000
#define TIME_GET_STOCK_CONFIG  1000
//构造函数

BYStockConfig g_bystockconfig;



BYStockConfig::BYStockConfig()
{   
 	init();
    ResetTodayInfo();
}

//析构函数
BYStockConfig::~BYStockConfig()
{
	KillTimer(m_dTimerID);
}
void BYStockConfig::init()
{
	//底值(越界，必杀)
	m_lValueA=-10000;   
	//底值(越界，概率杀分)
	m_lValueB=-5000;   
	//底值(越界，概率放水)
	m_lValueC=10000;    
	//底值(越界，必放)
	m_lValueD=20000;    
	//当前库存
	m_lCurValue    = 0L;
	//税收总额
	m_lRevenue=0;
	//税收计数
    m_nRevenueSwitch=0;
	//收税炮数(例如设置为100,100炮收一炮税)
	m_nRevenueCount=0;
}

void BYStockConfig::ResetTodayInfo()
{
    //所有桌子的当日总吸分
    m_llTodayTotalWinScore = 0;
    //所有桌子的当日总放分
    m_llTodayTotalLoseScore = 0;
    //所有桌子的当日库存
    m_llTodayTotalKuCun = 0;
    //所有桌子的当日税收
    m_llTodayTotalTax = 0;
    //所有桌子的当日点控输赢
    m_llTodayDianKongWinLoseTotalScore = 0;
    //所有桌子当日总进入人次
    m_dwTodayTotalEnterCount = 0;
    //所有桌子当日总赢钱玩家数
    m_dwTodayTotalPlayerCount_Win = 0;
    //所有桌子当日总破产玩家数
    m_dwTodayTotalPlayerCount_Bankrupted = 0;
    //所有桌子当日总输但是非破产的玩家数
    m_dwTodayTotalPlayerCount_Lose = 0;
    //所有桌子当日不输不赢的玩家数
    m_dwTodayTatalPlayerCount_NoWinLose = 0;
    //所有桌子放分次数
    m_dwTodayRewardCount = 0;
    //所有桌子吸分次数
    m_dwTodayXiFenCount = 0;

    m_today_time = time(NULL);
}

//没有调用屏蔽
//设置库存信息
/*void  BYStockConfig:: SetStockValue(SVar &s)
{
	m_lValueA=s["StockValueA"].ToNumber<INT64>();           //最低库存点
	m_lValueB=s["StockValueB"].ToNumber<INT64>();           //杀分触发点
	m_lValueC=s["StockValueC"].ToNumber<INT64>();           //放分触发点
	m_lValueD=s["StockValueD"].ToNumber<INT64>();           //最高库存点
	m_lCurValue=s["CurStockValue"].ToNumber<INT64>();       //当前库存值
    m_nRevenueCount=s["DarkTax"].ToNumber<DWORD>();             //当前库存值
	if(m_lValueA==0&&m_lValueB==0&&m_lValueC==0&&m_lValueD==0&&m_lCurValue==0)
	{ 
        m_dStockTimerID= INSTANCE(STimer)->AddTimer(TIME_GET_STOCK_CONFIG, this, &BYStockConfig::OnTimeGetStockConfig);
	}
	else
	{
       	m_dTimerID = INSTANCE(STimer)->AddTimer(TIME_PLUSE, this, &BYStockConfig::OnTimerFun);
	}
	SHOW("设置当前库存m_lStockValueA:%I64d m_lStockValueB:%I64d m_lStockValueC:%I64d m_lStockValueD:%I64d m_lCurStockValue::%I64d",m_lValueA,m_lValueB,m_lValueC,m_lValueD,m_lCurValue);
}*/
//获取库存信息
void  BYStockConfig::GetStockValue(StockInfo &si)
{
	si.valueA = m_lValueA;
	si.valueB = m_lValueB;
	si.valueC = m_lValueC;
	si.valueD = m_lValueD;
	si.curStockValue = m_lCurValue;
	si.nRevenueCount = m_nRevenueCount;

	/*s["StockValueA"]=m_lValueA;          //最低库存点
	s["StockValueB"]=m_lValueB;          //杀分触发点
	s["StockValueC"]=m_lValueC;          //放分触发点
	s["StockValueD"]=m_lValueD;          //最高库存点
	s["CurStockValue"]=m_lCurValue;      //当前库存值
	s["DarkTax"]=(int)m_nRevenueCount;   //抽税*/
	
}
//删除定时器
void BYStockConfig::KillTimer(uint64_t dTimerID)
{
	if (dTimerID <= 0) return;
	//没有调用 屏蔽
	//INSTANCE(STimer)->DelTimerEx(dTimerID);
}

//定时函数
void BYStockConfig::OnTimerFun(uint64_t id, void *pData)
{
	KillTimer(m_dTimerID);
	//每2分钟保存
	//SVar s;
	StockInfo si;
    GetStockValue(si);

    //函数没有调用 屏蔽
	//INSTANCE(Client)->Send((emFunction_Game<<16)+XS_UPDATE_STOCK_INFO_P,&s);
	//m_dTimerID = INSTANCE(STimer)->AddTimer(TIME_PLUSE, this, &BYStockConfig::OnTimerFun);
}
void BYStockConfig:: OnTimeGetStockConfig(uint64_t id, void *pData)
{
	//没有调用 屏蔽
	/*KillTimer(m_dStockTimerID);
	SVar s;
	INSTANCE(Client)->Send((emFunction_Game<<16)+XS_REGISTER_P,&s);*/
}