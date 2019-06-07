#ifndef		_CFISHCONTROL_H_
#define		_CFISHCONTROL_H_
/*
�Բ������ӵĿ���
�ر�ע��ĵط�:
Ѫ��ʱ,��ͬ����ĳ���(�ڱ���һ��),��Ϊʹ����ͬһ�š�Ѫ��״̬���ñ�,���Բ�ͬ�ĳ����ڽ������ʱ��Ҫʹ�ò�ͬ�Ļ���  С�ʳ�:1��10  �ϰ峡:1��1  ������:1��0.1
���ʱ,��ͬ����ĳ���,��Ϊʹ����ͬһ��"������ñ�",���Բ�ͬ�ĳ����ڽ�������ʱ��Ҫʹ�ò�ͬ�Ļ���, ������ʺ�����һ��
*/

#include <CTType.h>
#include <ITableFrame.h>
#include "acl_cpp/lib_acl.hpp"
#include "FishServerCfg.h"

struct sKuCunCaiYangInfo
{
	CT_LONGLONG llKuCuCaiYang;
	CT_UINT32 nCaiYangTime;
};

#define Hight_Point 1000
#define Low_Point -1000
#define MAX_FISH_TYPE 40

class CFishControl : public acl::singleton<CFishControl>
{
public:
	CFishControl();
	~CFishControl();

public:
	CT_VOID CheckBaseCapturePro();
	CT_VOID Init();

	//���ÿ��һ�ھ�֪ͨϵͳӮ�˶��ٷ�
	CT_VOID NotifySysWinScore(CT_LONGLONG llSysWinScore);
	//���ÿ�ε÷־�֪ͨϵͳ���˶��ٷ�
	CT_VOID NotifySysLoseScore(CT_LONGLONG llSysLoseScore);
	//�ط���ʱ��洢�����������
	CT_VOID SaveFishControlData(acl::mysql_manager &dbManager);
	//�ط���ʱ��ͳ���¸������ʵ�ʲ������ �� ���ò��������Ϣ
	CT_VOID StatisticFishDeathPro(acl::mysql_manager &dbManager);
	CT_VOID PrintFishControlData();
	//����������
	CT_VOID SaveKuCunData();
	//ͳ��������������
	CT_VOID StatisticFishInfo();
	//���沶��ĵ��������� �� �����ܷŷ�
	CT_VOID SaveFishTodayTotalWinLoseScore();
	//������ʱ���ȡ�����������
	CT_BOOL ReadFishControlData(acl::mysql_manager &dbManager);
	//��ȡ������ֵ
	CT_LONGLONG GetFishKuCun() { return m_nAllTableKuCun; }
	//��������ֵ�ʱ����������
	CT_VOID NotifySysWinScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llWinScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose);
	//������Ӯ�ֵ�ʱ�����Ӯ����
	CT_VOID NotifySysLoseScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llLoseScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose);
	CT_LONGLONG GetAllTableTotalWinScore() { return m_nAllTableTotalWinScore; }
	CT_LONGLONG GetAllTableTotalLoseScore() { return m_nAllTableTotalLoseScore; }

	CT_VOID TongJi(tagTongJiInfo &info);
	CT_VOID SinglePlayerMostWinLoseScore(CT_DWORD dwUserID, CT_LONGLONG addScore);
	//FishControl������ȡ������������ҵ���Ӯ
	CT_VOID TongJiAllPlayerAddScore();
	//��ҵ�ؿ�ʼ
	CT_VOID UserDianKongStart(CT_DWORD dwUserID);
	//��ҵ�ؽ���
	CT_VOID UserDianKongEnd(CT_DWORD dwUserID);
	//��¼����ͳ����Ϣ
	CT_VOID ReportFishStatisticsInfo(FishStatisticsInfo info[], int arrsize);
	//��ȡ����ϵͳ˰��
	CT_DWORD GetFishTaxPro() { return (CT_DWORD)CFishServerCfg::TaxPro;}
private:
	//�������ӵ�������,ʵʱ������
	CT_LONGLONG     m_nAllTableTotalWinScore;
	//�������ӵ��ܷŷ�,ʵʱ������
	CT_LONGLONG     m_nAllTableTotalLoseScore;
	//������Ϸ�е���ʱ���ݣ����ڼ��㰵˰																		
	CT_LONGLONG     m_nTaxTempValue;
	//���,ʵʱ������
	CT_LONGLONG     m_nAllTableKuCun;
	//�������ӵ���˰��,ʵʱ������
	CT_LONGLONG     m_nAllTableTotalTax;
	//�������ӵĵ���������
	CT_LONGLONG  m_llTodayTotalWinScore;
	//�������ӵĵ����ܷŷ�
	CT_LONGLONG  m_llTodayTotalLoseScore;
	//�������ӵĵ��տ��
	CT_LONGLONG m_llTodayTotalKuCun;
	//�������ӵĵ���˰��
	CT_LONGLONG m_llTodayTotalTax;
	//�������ӵĵ��յ����Ӯ
	CT_LONGLONG m_llTodayDianKongWinLoseTotalScore;
	//�������ӵ����ܽ����˴�
	CT_DWORD    m_dwTodayTotalEnterCount;
	//�������ӵ�����ӮǮ�����
	CT_DWORD    m_dwTodayTotalPlayerCount_Win;
	//�������ӵ������Ʋ������
	CT_DWORD    m_dwTodayTotalPlayerCount_Bankrupted;
	//�������ӵ������䵫�Ƿ��Ʋ��������
	CT_DWORD    m_dwTodayTotalPlayerCount_Lose;
	//�������ӵ��ղ��䲻Ӯ�������
	CT_DWORD    m_dwTodayTatalPlayerCount_NoWinLose;
	//�������ӵ���Ϊ�����Ʋ��ʴ����������������֮��������ҵķ���
	CT_DWORD    m_dwTodayForbidPoChanReward;
	//���������ϴ�����Ϣ
	std::map<CT_BYTE, sBigFishInFo> m_mapTodayBigFishInfo;
	//�������ӷŷִ���
	CT_DWORD  m_dwTodayRewardCount;
	//�����������ִ���
	CT_DWORD  m_dwTodayXiFenCount;
	//�������������ո��ʱ��������
	std::map<CT_DWORD, CT_DWORD> m_mapTodayProInfo;
	//
	//�����ʱ��
	CT_UINT64      m_nTodayTime;
	//˰��
	CT_DOUBLE     m_dTaxPro;
	//���������� ��������:�����Ŀǰ����ֵ���������ϸ���N(2000) ���� ���¸���-N(-2000)�Ͳ���һ�Σ����ԣ�NֵԽС�������γɵĲ���ͼԽ��ȷ.
	//std::vector<sKuCunCaiYangInfo> m_vecKuCunCaiYang;
	std::list<sKuCunCaiYangInfo> m_listKuCunCaiYang;
	CT_LONGLONG m_llKuCunCaiYang;
	//FishControl�Ƿ��������ͷ�,�������ͷֺ�,Ҫ�ͷ���ɲ���������һ���ͷ�,�ͷ���ɵı�ʶ�ǵ�����Ӯ����ȫ�����
	CT_BOOL m_bStartAddScore;
	//��ҵ�ǰ���µ���Ӯ����
	std::vector<tagPlayerAddScore> m_vecUserAddScore;
	//������ߵ� �� ��͵�
	CT_LONGLONG m_llKuCunMostTop;
	CT_LONGLONG m_llKuCunMostLow;
	//���ڵ�ص���ҵĵ����Ӯֵ��¼:�����Ӯֵ�������ҵġ�
	std::map<CT_DWORD, CT_LONGLONG> m_mapPlayerDianKongWinLose;
	//ϵͳ���Ӯ�ķ���:��������������е�������������֮��ķ����ܺ�,��������,û��ת�����ʵ�ֵ,����ϵͳͨ�����ӯ�����ۼ�ֵ
	CT_LONGLONG m_llSysDianKongWinTotalScore;
	//ϵͳ����������:��������������е�����Ӯ�������֮��ķ����ܺ�,����������û��ת�����ʵ�ֵ,����ϵͳͨ����طŷֵ��ۼ�ֵ
	CT_LONGLONG m_llSysDianKongLoseTotalScore;
	//Ѫ�ص����Ӯ �������ֻ��ͳ���õ�,ʵ�����ڷŷֵ�ʱ��,
	CT_LONGLONG  m_llBloodPoolDianKongWinLose;
	//ϵͳ�����Ӯ�ܷ�:��ʾ�������˵����Ӯ��,ϵͳ�Ĵ������Ӯ�ܷ�,���ֵΪ������ϵͳ���������������ӯ������.���ֵΪ������ϵͳ������Ӯ�������ķŷֲ��֣�
	//�������ֵ�����ж�ϵͳӦ�÷Ŷ��ٷֳ�ȥ����������m_llBloodPoolDianKongWinLose�жϣ���Ϊ���ֵһֱ�ڱ�������������½� ��
	//m_llSysDianKongWinLoseTotalScore = m_llSysDianKongWinTotalScore - m_llSysDianKongLoseTotalScore
	CT_LONGLONG m_llSysDianKongWinLoseTotalScore;
	//Ŀǰ���Ӯ����Ҽ���
	std::vector<CT_DWORD> m_vecCurrentDianKongWinUsers;
	//���������ʷ��¼
	FishStatisticsInfo                m_FishStatisticsInfo[MAX_FISH_TYPE];

	CT_LONGLONG m_llKuCunZhengShuCnt;
	CT_LONGLONG m_llKuCunFuShuCnt;

	CT_LONGLONG m_llTestBaseCapProDaYu10000;
	CT_LONGLONG m_llTestBaseCapProXiaoYu10000;

	//��¼ϵͳ���û���������ʵĴ���
	std::map<int, CT_UINT32> m_mapTestCapProCnt;

	int m_nTestDengYu1650Cnt;
	int m_nTestDaYu1650Cnt;


	CT_DWORD m_dwPlayerCnt;
	CT_LONGLONG m_llWinMost;
	CT_LONGLONG m_llLoseMost;
	CT_DWORD m_dwPoChanPlayerCnt;
	CT_DWORD m_dwWinPlayerCnt;

	time_t m_nTestBeginTime;

	CT_LONGLONG m_llTestZhengTotal;
	CT_LONGLONG m_llTestZhengCnt;
	CT_LONGLONG m_llTestFuTotal;
	CT_LONGLONG m_llTestFuCnt;

	std::map<CT_DWORD, CT_LONGLONG> m_mapTestPlayerMostWinLoseScore;

	//ͳ�������˵�˰���ܺ�
	CT_LONGLONG m_llTestAllPlayerTotalTax;

};

#endif

