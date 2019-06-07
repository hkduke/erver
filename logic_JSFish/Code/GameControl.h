#ifndef GAME_CONTROL_H_
#define GAME_CONTROL_H_
#define MAX_CONTROL_NUM 10
#include <vector>
#include <map>
#include "Define.h"
//强控机制
struct WinForceControl
{
	int prob;//触发权重
	int addodds;//叠加百分率
	SCORE ControlptsMin;//控制分值下限
	SCORE ControlptsMax;//控制分值上限
	int ControlperMin;//控制分值入场百分比下限
	int ControlperMax;//控制分值入场百分比上限
	int RecoverperMin;//回弹百分比下限
	int RecoverperMax;//回弹百分比上限
	int Recoveraddodds;//回弹叠加百分率
	int NextDelayMin;//结束延迟下限
	int NextDelayMax;//结束延迟上限
};

//输赢强控配置选择
struct WinForceSelectConfig
{
	int groupCount;	//可选配置组数
	int groupIndex[MAX_CONTROL_NUM]; //可选配置的下标
	WinForceSelectConfig()
	{
		groupCount = 0;
		memset(groupIndex, 0, sizeof(groupIndex));
	}
};
//强发库保底机制
struct ForceScoreProtect
{
	int enterScorePercent; //入场分数百分比
	int nowBulletMultiple; //当前炮倍乘以的倍数
	int touchInterval;	   //两次触发之间间隔的炮数
	ForceScoreProtect()
	{
		enterScorePercent = nowBulletMultiple = touchInterval = 0;
	}
};

//入场延迟
struct TimesDelay
{
	int min;
	int max;
	TimesDelay()
	{
		min = max = 0;
	}
};
//控制鱼规则
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

//输赢奖池线
struct WinForcePondLine
{
	int lineCount;
	SCORE pondLine[MAX_CONTROL_NUM - 1];
};
//游戏控制
class GameControl
{
public:
	GameControl(void);
	~GameControl(void);

public:
	int		m_bForceBigFishOdd;		//大鱼几率
	int		m_bForceSmallFishOdd;	//小鱼几率
	double	m_bForceMustOdd;		//强发过低系数
	int		m_bForceMulti;			//强发触发条件一
	int		m_bForceMissCount;		//强发触发条件二
	int		m_bForceUpgradeMulti;	//强发回补提升倍数

									//****************************净分控制机制****************************
	double		m_maxWinratio;			//最大净分系数
	double		m_minWinratio;			//最小净分系数
	double		m_losSratio;			//亏损系数
	double		m_roomMaxbet;			//房间最大炮倍
	double		m_fishMulitiple;		//最大鱼倍
	double		m_wininf;				//最大净分系数参数
	double		m_loseinf;				//最小净分系数参数
	int			m_Taxodd;				//系统抽水万分比
	int			m_TaxoddSwitch;			//系统抽水开关
	int		m_MinForceThreshold;		//最小净分阈值
	int		m_MaxForceThreshold;		//最大净分阈值
	int		m_foceTaxodd;				//净分抽水万分比
	int		m_foceTaxoddWriteSwitch;	//净分抽水写入奖池开关
	int		m_cofficientKeepTimesMin;	//全局开炮计数  决定全局系数变化
	int		m_cofficientKeepTimesMax;	//
	int		m_currentKeepTimes;			//当前计数
	int		m_SystemPondTaxodd;			//库存暗税
	double	m_nowPondCoefficient;			//当前奖池系数
	double m_MinForceThresholdAndGold;	//玩家进场金币与最小净分生效阈值的倍率
	double m_MaxForceThresholdAndGold;	//玩家进场金币与最大净分生效阈值的倍率
										//全屏和范围炸弹的倍数
	float WinToLoseAddProb;		//玩家输赢回弹控制
	float LoseToWinAddProb;
	bool logSwitch;						//日志开关
	bool goldSwitch;					//金币记录开关
	ForceScoreProtect forceScoreProtect;//强控机制
	TimesDelay timesDelay;				//入场延迟
	WinForceControl winForceControlArr[MAX_CONTROL_NUM];//输赢强控
	WinForcePondLine winForcePondLine;//输赢强控奖池线
	WinForceSelectConfig winForceSelectConfig[MAX_CONTROL_NUM];//输赢强控可选配置组
	SCORE SafetyLine;					//奖池安全线
	SCORE MaxinvLine;					//最大库存线
	SCORE AutoClear;					//自动清除
	float fireInterval;					//开炮间隔
	float fireAddProb;					//开炮叠加概率
	float enterAddProb;					//入场叠加概率
	int	  enterScoreCoefficient;		//入场金币系数
	//控制鱼规则 
	ContorlFishData   m_arrayFishContorlRule[MAX_BIRD_TYPE];
};

struct SystemPondControl
{
	//******************************全局奖池控制***************************   2017/3/8	
	int64_t m_systemPond;						//当前全局奖池
	int64_t m_systemPondLineOne;				//全局奖池线1
	int64_t m_systemPondLineTwo;				//全局奖池线2
	int64_t m_systemPondLineThree;				//全局奖池线3
	int64_t m_systemPondLineFour;				//全局奖池线4
	float m_systemPondCoefficientOneMin;			//全局奖池系数1
	float m_systemPondCoefficientOneMax;			//全局奖池系数1
	float m_systemPondCoefficientTwoMin;			//全局奖池系数2
	float m_systemPondCoefficientTwoMax;			//全局奖池系数2
	float m_systemPondCoefficientThreeMin;			//全局奖池系数3
	float m_systemPondCoefficientThreeMax;			//全局奖池系数3
	float m_systemPondCoefficientFourMin;			//全局奖池系数4
	float m_systemPondCoefficientFourMax;			//全局奖池系数4
	float m_systemPondCoefficientFiveMin;			//全局奖池系数5
	float m_systemPondCoefficientFiveMax;			//全局奖池系数5
};
extern GameControl g_game_control;


#endif //#endif
