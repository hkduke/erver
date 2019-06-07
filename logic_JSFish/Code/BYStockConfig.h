//���������
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

//��¼��Ϸ��桢����
class BYStockConfig
{

public:
	//���캯��
	BYStockConfig();
	//��������
	~BYStockConfig();
    void init();
public:
	//���ÿ��
	//void SetStockValue(SVar &s);
	//��ȡ���
	void GetStockValue(StockInfo &si);
	//�ӿں���
public:
	//����/��ȡ��ֵ(��ɱ)
	void  SetValueA(int64_t lValue) { m_lValueA = lValue; }
	int64_t GetValueA(){ return m_lValueA; }
	
	//����/��ȡ��ֵ������ɱ�֣�
	void  SetValueB(int64_t lValue) { m_lValueB = lValue; }
	int64_t GetValueB() { return m_lValueB; }

	//����/��ȡ��ֵ�����ʷ�ˮ��
	void  SetValueC(int64_t lValue) { m_lValueC = lValue; }
	int64_t GetValueC(){ return m_lValueC; }
	
	//����/��ȡ��ֵ���طţ�
	void  SetValueD(int64_t lValue) { m_lValueD = lValue; }
	int64_t GetValueD() { return m_lValueD; }
	//����/��ȡ��ǰ���
	void SetCurValue(int64_t lCurValue) {   m_lCurValue += lCurValue;}
	int64_t GetCurValue(){ return m_lCurValue; }
	void InitCurValue(int64_t llCurValue) {m_lCurValue = llCurValue;}

	//˰�����
public:
	//���˰�ռ���
	int   GetRevenueSwitch() { return m_nRevenueSwitch; }
	//����˰�ռ���
	void  SetRevenueSwitch() { m_nRevenueSwitch++; }
	//��ֵ˰�ռ���
	void  ResetRevenueSwitch() { m_nRevenueSwitch = 0; }
	//��˰����
	int   GetRevenueCount() { return m_nRevenueCount; }
	//��������
	void  SetRevenueCount(int nPaoCount) {m_nRevenueCount = nPaoCount;}
    //����˰��
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
    //��������
    //�������ӵĵ���������
    long long  m_llTodayTotalWinScore;
    //�������ӵĵ����ܷŷ�
    long long  m_llTodayTotalLoseScore;
    //�������ӵĵ��տ��
    long long  m_llTodayTotalKuCun;
    //�������ӵĵ���˰��
    long long  m_llTodayTotalTax;
    //�������ӵĵ��յ����Ӯ
    long long  m_llTodayDianKongWinLoseTotalScore;
    //�������ӵ����ܽ����˴�
    long long  m_dwTodayTotalEnterCount;
    //�������ӵ�����ӮǮ�����
    unsigned int m_dwTodayTotalPlayerCount_Win;
    //�������ӵ������Ʋ������
    unsigned int m_dwTodayTotalPlayerCount_Bankrupted;
    //�������ӵ������䵫�Ƿ��Ʋ��������
    unsigned int m_dwTodayTotalPlayerCount_Lose;
    //�������ӵ��ղ��䲻Ӯ�������
    unsigned int m_dwTodayTatalPlayerCount_NoWinLose;
    //�������ӷŷִ���
    unsigned int  m_dwTodayRewardCount;
    //�����������ִ���
    unsigned int  m_dwTodayXiFenCount;
    time_t  m_today_time;
    void ResetTodayInfo();
	//void SetSysCurTax(int64_t llCurTax) {m_llSystem_cur_tax = llCurTax;}
	//int64_t GetSysCurTax(){return m_llSystem_cur_tax;}
	//�ڲ�����
private:

	//��ʱ������
private:
	//ɾ����ʱ��
	void				KillTimer(uint64_t dTimerID);
	//��ʱ����
	void				OnTimerFun(uint64_t id, void *pData);
	//��ȡ����
	void                OnTimeGetStockConfig(uint64_t id, void *pData);
	//���������Ϣ
private:
	int64_t  m_lValueA;     //��ֵ(Խ�磬��ɱ)
	int64_t  m_lValueB;     //��ֵ(Խ�磬����ɱ��)
	int64_t  m_lValueC;     //��ֵ(Խ�磬���ʷ�ˮ)
	int64_t  m_lValueD;     //��ֵ(Խ�磬�ط�)
	int64_t  m_lCurValue;     //��ǰ���

	//˰���ܶ�
	int64_t m_lRevenue;
	//˰�ռ���
	int m_nRevenueSwitch;
	//��˰����(��������Ϊ100,100����һ��˰)
	int m_nRevenueCount;

	//ʱ�䶨��
private:
	//ʱ���ʶ
	uint64_t	m_dTimerID;

	//ʱ���ʶ û�е�������
	//uint64_t	m_dStockTimerID;

	int64_t m_llSystem_win_score;
	int64_t m_llSystem_lose_score;
	//int64_t m_llSystem_cur_tax;
	//int64_t m_llSystem_diankong;
};