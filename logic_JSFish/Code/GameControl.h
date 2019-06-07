#ifndef GAME_CONTROL_H_
#define GAME_CONTROL_H_
#define MAX_CONTROL_NUM 10
#include <vector>
#include <map>
#include "Define.h"
//ǿ�ػ���
struct WinForceControl
{
	int prob;//����Ȩ��
	int addodds;//���Ӱٷ���
	SCORE ControlptsMin;//���Ʒ�ֵ����
	SCORE ControlptsMax;//���Ʒ�ֵ����
	int ControlperMin;//���Ʒ�ֵ�볡�ٷֱ�����
	int ControlperMax;//���Ʒ�ֵ�볡�ٷֱ�����
	int RecoverperMin;//�ص��ٷֱ�����
	int RecoverperMax;//�ص��ٷֱ�����
	int Recoveraddodds;//�ص����Ӱٷ���
	int NextDelayMin;//�����ӳ�����
	int NextDelayMax;//�����ӳ�����
};

//��Ӯǿ������ѡ��
struct WinForceSelectConfig
{
	int groupCount;	//��ѡ��������
	int groupIndex[MAX_CONTROL_NUM]; //��ѡ���õ��±�
	WinForceSelectConfig()
	{
		groupCount = 0;
		memset(groupIndex, 0, sizeof(groupIndex));
	}
};
//ǿ���Ᵽ�׻���
struct ForceScoreProtect
{
	int enterScorePercent; //�볡�����ٷֱ�
	int nowBulletMultiple; //��ǰ�ڱ����Եı���
	int touchInterval;	   //���δ���֮����������
	ForceScoreProtect()
	{
		enterScorePercent = nowBulletMultiple = touchInterval = 0;
	}
};

//�볡�ӳ�
struct TimesDelay
{
	int min;
	int max;
	TimesDelay()
	{
		min = max = 0;
	}
};
//���������
struct ContorlFishData
{
	int ForceChouShui;
	int ScoreContorl;
	int ForceHuiBu;
	int KindID;
	ContorlFishData()
	{
		KindID = ForceChouShui = ScoreContorl = ForceHuiBu = 0;
	}
};

//��Ӯ������
struct WinForcePondLine
{
	int lineCount;
	SCORE pondLine[MAX_CONTROL_NUM - 1];
};
//��Ϸ����
class GameControl
{
public:
	GameControl(void);
	~GameControl(void);

public:
	int		m_bForceBigFishOdd;		//���㼸��
	int		m_bForceSmallFishOdd;	//С�㼸��
	double	m_bForceMustOdd;		//ǿ������ϵ��
	int		m_bForceMulti;			//ǿ����������һ
	int		m_bForceMissCount;		//ǿ������������
	int		m_bForceUpgradeMulti;	//ǿ���ز���������

									//****************************���ֿ��ƻ���****************************
	double		m_maxWinratio;			//��󾻷�ϵ��
	double		m_minWinratio;			//��С����ϵ��
	double		m_losSratio;			//����ϵ��
	double		m_roomMaxbet;			//��������ڱ�
	double		m_fishMulitiple;		//����㱶
	double		m_wininf;				//��󾻷�ϵ������
	double		m_loseinf;				//��С����ϵ������
	int			m_Taxodd;				//ϵͳ��ˮ��ֱ�
	int			m_TaxoddSwitch;			//ϵͳ��ˮ����
	int		m_MinForceThreshold;		//��С������ֵ
	int		m_MaxForceThreshold;		//��󾻷���ֵ
	int		m_foceTaxodd;				//���ֳ�ˮ��ֱ�
	int		m_foceTaxoddWriteSwitch;	//���ֳ�ˮд�뽱�ؿ���
	int		m_cofficientKeepTimesMin;	//ȫ�ֿ��ڼ���  ����ȫ��ϵ���仯
	int		m_cofficientKeepTimesMax;	//
	int		m_currentKeepTimes;			//��ǰ����
	int		m_SystemPondTaxodd;			//��氵˰
	double	m_nowPondCoefficient;			//��ǰ����ϵ��
	double m_MinForceThresholdAndGold;	//��ҽ����������С������Ч��ֵ�ı���
	double m_MaxForceThresholdAndGold;	//��ҽ����������󾻷���Ч��ֵ�ı���
										//ȫ���ͷ�Χը���ı���
	float WinToLoseAddProb;		//�����Ӯ�ص�����
	float LoseToWinAddProb;
	bool logSwitch;						//��־����
	bool goldSwitch;					//��Ҽ�¼����
	ForceScoreProtect forceScoreProtect;//ǿ�ػ���
	TimesDelay timesDelay;				//�볡�ӳ�
	WinForceControl winForceControlArr[MAX_CONTROL_NUM];//��Ӯǿ��
	WinForcePondLine winForcePondLine;//��Ӯǿ�ؽ�����
	WinForceSelectConfig winForceSelectConfig[MAX_CONTROL_NUM];//��Ӯǿ�ؿ�ѡ������
	SCORE SafetyLine;					//���ذ�ȫ��
	SCORE MaxinvLine;					//�������
	SCORE AutoClear;					//�Զ����
	float fireInterval;					//���ڼ��
	float fireAddProb;					//���ڵ��Ӹ���
	float enterAddProb;					//�볡���Ӹ���
	int	  enterScoreCoefficient;		//�볡���ϵ��
	//��������� 
	ContorlFishData   m_arrayFishContorlRule[MAX_BIRD_TYPE];
};

struct SystemPondControl
{
	//******************************ȫ�ֽ��ؿ���***************************   2017/3/8	
	int64_t m_systemPond;						//��ǰȫ�ֽ���
	int64_t m_systemPondLineOne;				//ȫ�ֽ�����1
	int64_t m_systemPondLineTwo;				//ȫ�ֽ�����2
	int64_t m_systemPondLineThree;				//ȫ�ֽ�����3
	int64_t m_systemPondLineFour;				//ȫ�ֽ�����4
	float m_systemPondCoefficientOneMin;			//ȫ�ֽ���ϵ��1
	float m_systemPondCoefficientOneMax;			//ȫ�ֽ���ϵ��1
	float m_systemPondCoefficientTwoMin;			//ȫ�ֽ���ϵ��2
	float m_systemPondCoefficientTwoMax;			//ȫ�ֽ���ϵ��2
	float m_systemPondCoefficientThreeMin;			//ȫ�ֽ���ϵ��3
	float m_systemPondCoefficientThreeMax;			//ȫ�ֽ���ϵ��3
	float m_systemPondCoefficientFourMin;			//ȫ�ֽ���ϵ��4
	float m_systemPondCoefficientFourMax;			//ȫ�ֽ���ϵ��4
	float m_systemPondCoefficientFiveMin;			//ȫ�ֽ���ϵ��5
	float m_systemPondCoefficientFiveMax;			//ȫ�ֽ���ϵ��5
};
extern GameControl g_game_control;


#endif //#endif
