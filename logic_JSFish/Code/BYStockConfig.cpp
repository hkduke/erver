
#include "stdafx.h"
#include "BYStockConfig.h"
#define TIME_PLUSE 2 * 60 * 1000
#define TIME_GET_STOCK_CONFIG  1000
//���캯��

BYStockConfig g_bystockconfig;



BYStockConfig::BYStockConfig()
{   
 	init();
    ResetTodayInfo();
}

//��������
BYStockConfig::~BYStockConfig()
{
	KillTimer(m_dTimerID);
}
void BYStockConfig::init()
{
	//��ֵ(Խ�磬��ɱ)
	m_lValueA=-10000;   
	//��ֵ(Խ�磬����ɱ��)
	m_lValueB=-5000;   
	//��ֵ(Խ�磬���ʷ�ˮ)
	m_lValueC=10000;    
	//��ֵ(Խ�磬�ط�)
	m_lValueD=20000;    
	//��ǰ���
	m_lCurValue    = 0L;
	//˰���ܶ�
	m_lRevenue=0;
	//˰�ռ���
    m_nRevenueSwitch=0;
	//��˰����(��������Ϊ100,100����һ��˰)
	m_nRevenueCount=0;
}

void BYStockConfig::ResetTodayInfo()
{
    //�������ӵĵ���������
    m_llTodayTotalWinScore = 0;
    //�������ӵĵ����ܷŷ�
    m_llTodayTotalLoseScore = 0;
    //�������ӵĵ��տ��
    m_llTodayTotalKuCun = 0;
    //�������ӵĵ���˰��
    m_llTodayTotalTax = 0;
    //�������ӵĵ��յ����Ӯ
    m_llTodayDianKongWinLoseTotalScore = 0;
    //�������ӵ����ܽ����˴�
    m_dwTodayTotalEnterCount = 0;
    //�������ӵ�����ӮǮ�����
    m_dwTodayTotalPlayerCount_Win = 0;
    //�������ӵ������Ʋ������
    m_dwTodayTotalPlayerCount_Bankrupted = 0;
    //�������ӵ������䵫�Ƿ��Ʋ��������
    m_dwTodayTotalPlayerCount_Lose = 0;
    //�������ӵ��ղ��䲻Ӯ�������
    m_dwTodayTatalPlayerCount_NoWinLose = 0;
    //�������ӷŷִ���
    m_dwTodayRewardCount = 0;
    //�����������ִ���
    m_dwTodayXiFenCount = 0;

    m_today_time = time(NULL);
}

//û�е�������
//���ÿ����Ϣ
/*void  BYStockConfig:: SetStockValue(SVar &s)
{
	m_lValueA=s["StockValueA"].ToNumber<INT64>();           //��Ϳ���
	m_lValueB=s["StockValueB"].ToNumber<INT64>();           //ɱ�ִ�����
	m_lValueC=s["StockValueC"].ToNumber<INT64>();           //�ŷִ�����
	m_lValueD=s["StockValueD"].ToNumber<INT64>();           //��߿���
	m_lCurValue=s["CurStockValue"].ToNumber<INT64>();       //��ǰ���ֵ
    m_nRevenueCount=s["DarkTax"].ToNumber<DWORD>();             //��ǰ���ֵ
	if(m_lValueA==0&&m_lValueB==0&&m_lValueC==0&&m_lValueD==0&&m_lCurValue==0)
	{ 
        m_dStockTimerID= INSTANCE(STimer)->AddTimer(TIME_GET_STOCK_CONFIG, this, &BYStockConfig::OnTimeGetStockConfig);
	}
	else
	{
       	m_dTimerID = INSTANCE(STimer)->AddTimer(TIME_PLUSE, this, &BYStockConfig::OnTimerFun);
	}
	SHOW("���õ�ǰ���m_lStockValueA:%I64d m_lStockValueB:%I64d m_lStockValueC:%I64d m_lStockValueD:%I64d m_lCurStockValue::%I64d",m_lValueA,m_lValueB,m_lValueC,m_lValueD,m_lCurValue);
}*/
//��ȡ�����Ϣ
void  BYStockConfig::GetStockValue(StockInfo &si)
{
	si.valueA = m_lValueA;
	si.valueB = m_lValueB;
	si.valueC = m_lValueC;
	si.valueD = m_lValueD;
	si.curStockValue = m_lCurValue;
	si.nRevenueCount = m_nRevenueCount;

	/*s["StockValueA"]=m_lValueA;          //��Ϳ���
	s["StockValueB"]=m_lValueB;          //ɱ�ִ�����
	s["StockValueC"]=m_lValueC;          //�ŷִ�����
	s["StockValueD"]=m_lValueD;          //��߿���
	s["CurStockValue"]=m_lCurValue;      //��ǰ���ֵ
	s["DarkTax"]=(int)m_nRevenueCount;   //��˰*/
	
}
//ɾ����ʱ��
void BYStockConfig::KillTimer(uint64_t dTimerID)
{
	if (dTimerID <= 0) return;
	//û�е��� ����
	//INSTANCE(STimer)->DelTimerEx(dTimerID);
}

//��ʱ����
void BYStockConfig::OnTimerFun(uint64_t id, void *pData)
{
	KillTimer(m_dTimerID);
	//ÿ2���ӱ���
	//SVar s;
	StockInfo si;
    GetStockValue(si);

    //����û�е��� ����
	//INSTANCE(Client)->Send((emFunction_Game<<16)+XS_UPDATE_STOCK_INFO_P,&s);
	//m_dTimerID = INSTANCE(STimer)->AddTimer(TIME_PLUSE, this, &BYStockConfig::OnTimerFun);
}
void BYStockConfig:: OnTimeGetStockConfig(uint64_t id, void *pData)
{
	//û�е��� ����
	/*KillTimer(m_dStockTimerID);
	SVar s;
	INSTANCE(Client)->Send((emFunction_Game<<16)+XS_REGISTER_P,&s);*/
}