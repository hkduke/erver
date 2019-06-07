#ifndef _FISHSERVERCFG_
#define _FISHSERVERCFG_
#include <string>
#include "acl_cpp/master/master_conf.hpp"
#include "Singleton.h"
#include "GlobalStruct.h"
#include <map>

//最大血池状态数
#define MAX_BLOODPOOL_STATE		100
//捕鱼血池状态配置数据
struct FishBloodPoolStateCfg
{
	//状态编号
	int StateNo;
	//状态类型
	int StateType;
	//状态值
	int StateValue;
	//捕获几率 万分比
	int CapturePro;
};

class CFishServerCfg
{
public:
	CFishServerCfg(std::string strScriptPath);
	~CFishServerCfg();

	CT_VOID load();
	static acl::master_int_tbl __conf_int_tab[];

	/********************************************************************************************/
	//捕鱼血池状态配置
	static int		KuCun_1_Number;
	static int		KuCun_1_StateType;
	static int		KuCun_1_StateValue;
	static int		KuCun_1_CapturePro;
	static int		KuCun_2_Number;
	static int		KuCun_2_StateType;
	static int		KuCun_2_StateValue;
	static int		KuCun_2_CapturePro;
	static int		KuCun_3_Number;
	static int		KuCun_3_StateType;
	static int		KuCun_3_StateValue;
	static int		KuCun_3_CapturePro;
	static int		KuCun_4_Number;
	static int		KuCun_4_StateType;
	static int		KuCun_4_StateValue;
	static int		KuCun_4_CapturePro;
	static int		KuCun_5_Number;
	static int		KuCun_5_StateType;
	static int		KuCun_5_StateValue;
	static int		KuCun_5_CapturePro;
	static int		KuCun_6_Number;
	static int		KuCun_6_StateType;
	static int		KuCun_6_StateValue;
	static int		KuCun_6_CapturePro;
	static int		KuCun_7_Number;
	static int		KuCun_7_StateType;
	static int		KuCun_7_StateValue;
	static int		KuCun_7_CapturePro;
	static int		KuCun_8_Number;
	static int		KuCun_8_StateType;
	static int		KuCun_8_StateValue;
	static int		KuCun_8_CapturePro;
	static int		KuCun_9_Number;
	static int		KuCun_9_StateType;
	static int		KuCun_9_StateValue;
	static int		KuCun_9_CapturePro;
	static int		KuCun_10_Number;
	static int		KuCun_10_StateType;
	static int		KuCun_10_StateValue;
	static int		KuCun_10_CapturePro;
	static int		KuCun_11_Number;
	static int		KuCun_11_StateType;
	static int		KuCun_11_StateValue;
	static int		KuCun_11_CapturePro;
	static int		KuCun_12_Number;
	static int		KuCun_12_StateType;
	static int		KuCun_12_StateValue;
	static int		KuCun_12_CapturePro;
	static int		KuCun_13_Number;
	static int		KuCun_13_StateType;
	static int		KuCun_13_StateValue;
	static int		KuCun_13_CapturePro;
	static int		KuCun_14_Number;
	static int		KuCun_14_StateType;
	static int		KuCun_14_StateValue;
	static int		KuCun_14_CapturePro;
	static int		KuCun_15_Number;
	static int		KuCun_15_StateType;
	static int		KuCun_15_StateValue;
	static int		KuCun_15_CapturePro;
	static int		KuCun_16_Number;
	static int		KuCun_16_StateType;
	static int		KuCun_16_StateValue;
	static int		KuCun_16_CapturePro;
	static int		KuCun_17_Number;
	static int		KuCun_17_StateType;
	static int		KuCun_17_StateValue;
	static int		KuCun_17_CapturePro;
	static int		KuCun_18_Number;
	static int		KuCun_18_StateType;
	static int		KuCun_18_StateValue;
	static int		KuCun_18_CapturePro;
	static int		KuCun_19_Number;
	static int		KuCun_19_StateType;
	static int		KuCun_19_StateValue;
	static int		KuCun_19_CapturePro;
	static int		KuCun_20_Number;
	static int		KuCun_20_StateType;
	static int		KuCun_20_StateValue;
	static int		KuCun_20_CapturePro;
	static int		KuCun_21_Number;
	static int		KuCun_21_StateType;
	static int		KuCun_21_StateValue;
	static int		KuCun_21_CapturePro;
	static int		KuCun_22_Number;
	static int		KuCun_22_StateType;
	static int		KuCun_22_StateValue;
	static int		KuCun_22_CapturePro;
	static int		KuCun_23_Number;
	static int		KuCun_23_StateType;
	static int		KuCun_23_StateValue;
	static int		KuCun_23_CapturePro;
	static int		KuCun_24_Number;
	static int		KuCun_24_StateType;
	static int		KuCun_24_StateValue;
	static int		KuCun_24_CapturePro;
	static int		KuCun_25_Number;
	static int		KuCun_25_StateType;
	static int		KuCun_25_StateValue;
	static int		KuCun_25_CapturePro;
	static int		KuCun_26_Number;
	static int		KuCun_26_StateType;
	static int		KuCun_26_StateValue;
	static int		KuCun_26_CapturePro;
	static int		KuCun_27_Number;
	static int		KuCun_27_StateType;
	static int		KuCun_27_StateValue;
	static int		KuCun_27_CapturePro;
	static int		KuCun_28_Number;
	static int		KuCun_28_StateType;
	static int		KuCun_28_StateValue;
	static int		KuCun_28_CapturePro;
	static int		KuCun_29_Number;
	static int		KuCun_29_StateType;
	static int		KuCun_29_StateValue;
	static int		KuCun_29_CapturePro;
	static int		KuCun_30_Number;
	static int		KuCun_30_StateType;
	static int		KuCun_30_StateValue;
	static int		KuCun_30_CapturePro;
	static int		KuCun_31_Number;
	static int		KuCun_31_StateType;
	static int		KuCun_31_StateValue;
	static int		KuCun_31_CapturePro;
	static int		KuCun_32_Number;
	static int		KuCun_32_StateType;
	static int		KuCun_32_StateValue;
	static int		KuCun_32_CapturePro;
	static int		KuCun_33_Number;
	static int		KuCun_33_StateType;
	static int		KuCun_33_StateValue;
	static int		KuCun_33_CapturePro;
	static int		KuCun_34_Number;
	static int		KuCun_34_StateType;
	static int		KuCun_34_StateValue;
	static int		KuCun_34_CapturePro;
	static int		KuCun_35_Number;
	static int		KuCun_35_StateType;
	static int		KuCun_35_StateValue;
	static int		KuCun_35_CapturePro;
	static int		KuCun_36_Number;
	static int		KuCun_36_StateType;
	static int		KuCun_36_StateValue;
	static int		KuCun_36_CapturePro;
	static int		KuCun_37_Number;
	static int		KuCun_37_StateType;
	static int		KuCun_37_StateValue;
	static int		KuCun_37_CapturePro;
	static int		KuCun_38_Number;
	static int		KuCun_38_StateType;
	static int		KuCun_38_StateValue;
	static int		KuCun_38_CapturePro;
	static int		KuCun_39_Number;
	static int		KuCun_39_StateType;
	static int		KuCun_39_StateValue;
	static int		KuCun_39_CapturePro;
	static int		KuCun_40_Number;
	static int		KuCun_40_StateType;
	static int		KuCun_40_StateValue;
	static int		KuCun_40_CapturePro;
	static int		KuCun_41_Number;
	static int		KuCun_41_StateType;
	static int		KuCun_41_StateValue;
	static int		KuCun_41_CapturePro;
	static int		KuCun_42_Number;
	static int		KuCun_42_StateType;
	static int		KuCun_42_StateValue;
	static int		KuCun_42_CapturePro;
	static int		KuCun_43_Number;
	static int		KuCun_43_StateType;
	static int		KuCun_43_StateValue;
	static int		KuCun_43_CapturePro;
	static int		KuCun_44_Number;
	static int		KuCun_44_StateType;
	static int		KuCun_44_StateValue;
	static int		KuCun_44_CapturePro;
	static int		KuCun_45_Number;
	static int		KuCun_45_StateType;
	static int		KuCun_45_StateValue;
	static int		KuCun_45_CapturePro;
	static int		KuCun_46_Number;
	static int		KuCun_46_StateType;
	static int		KuCun_46_StateValue;
	static int		KuCun_46_CapturePro;
	static int	KuCun_47_Number;
	static int	KuCun_47_StateType;
	static int	KuCun_47_StateValue;
	static int	KuCun_47_CapturePro;
	static int		KuCun_48_Number;
	static int		KuCun_48_StateType;
	static int		KuCun_48_StateValue;
	static int		KuCun_48_CapturePro;
	static int	KuCun_49_Number;
	static int	KuCun_49_StateType;
	static int	KuCun_49_StateValue;
	static int	KuCun_49_CapturePro;
	static int	KuCun_50_Number;
	static int	KuCun_50_StateType;
	static int	KuCun_50_StateValue;
	static int	KuCun_50_CapturePro;
	static int	KuCun_51_Number;
	static int	KuCun_51_StateType;
	static int	KuCun_51_StateValue;
	static int	KuCun_51_CapturePro;
	static int		KuCun_52_Number;
	static int		KuCun_52_StateType;
	static int		KuCun_52_StateValue;
	static int		KuCun_52_CapturePro;
	static int	KuCun_53_Number;
	static int	KuCun_53_StateType;
	static int	KuCun_53_StateValue;
	static int	KuCun_53_CapturePro;
	static int		KuCun_54_Number;
	static int		KuCun_54_StateType;
	static int		KuCun_54_StateValue;
	static int		KuCun_54_CapturePro;
	static int		KuCun_55_Number;
	static int		KuCun_55_StateType;
	static int		KuCun_55_StateValue;
	static int		KuCun_55_CapturePro;
	static int	KuCun_56_Number;
	static int	KuCun_56_StateType;
	static int	KuCun_56_StateValue;
	static int	KuCun_56_CapturePro;
	//捕鱼血池状态配置表
	FishBloodPoolStateCfg m_arrayFishBloodPoolState[MAX_BLOODPOOL_STATE];
	//数组大小
	int   m_arraySize;
	//如果是捕鱼GS才会调用这个函数
	void LoadFishBloodPoolStateCfg();
	static int   m_exchangeRate;
	static int DianKong_1_ZuoBiZhi;
	static int DianKong_1_BloodPoolStateValue;
	static int DianKong_1_BaseCapPro;
	static int DianKong_2_ZuoBiZhi;
	static int DianKong_2_BloodPoolStateValue;
	static int DianKong_2_BaseCapPro;
	static int DianKong_3_ZuoBiZhi;
	static int DianKong_3_BloodPoolStateValue;
	static int DianKong_3_BaseCapPro;
	static int DianKong_4_ZuoBiZhi;
	static int DianKong_4_BloodPoolStateValue;
	static int DianKong_4_BaseCapPro;
	static int DianKong_5_ZuoBiZhi;
	static int DianKong_5_BloodPoolStateValue;
	static int DianKong_5_BaseCapPro;
	static int TaxPro;
	static int Timer_SaveFishControlInfo;
	static int IsSaveCurrentKuCun;
	static int Timer_SaveCurrentKuCun;
	static int DianKongFangFen_LowerLimit;
	static int DianKongFangFen_UpperLimit;
	static int IsStatisticsFishInfo;
	static int Timer_StatisticsFishInfo;
	/********************************************************************************************/
private:
	std::string m_strScriptPath;			// 脚本位置
	acl::master_conf	m_config;
};

#endif