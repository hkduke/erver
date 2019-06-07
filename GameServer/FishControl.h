#ifndef		_CFISHCONTROL_H_
#define		_CFISHCONTROL_H_
/*
对捕鱼桌子的控制
特别注意的地方:
血控时,不同级别的场次(炮倍不一样),因为使用了同一张“血池状态配置表”,所以不同的场次在进库库存的时候要使用不同的汇率  小资场:1比10  老板场:1比1  土豪场:1比0.1
点控时,不同级别的场次,因为使用了同一张"点控配置表",所以不同的场次在进出库存的时候要使用不同的汇率, 具体汇率和上面一致
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

	//玩家每发一炮就通知系统赢了多少分
	CT_VOID NotifySysWinScore(CT_LONGLONG llSysWinScore);
	//玩家每次得分就通知系统输了多少分
	CT_VOID NotifySysLoseScore(CT_LONGLONG llSysLoseScore);
	//关服的时候存储捕鱼控制数据
	CT_VOID SaveFishControlData(acl::mysql_manager &dbManager);
	//关服的时候统计下各种鱼的实际捕获概率 和 配置捕获概率信息
	CT_VOID StatisticFishDeathPro(acl::mysql_manager &dbManager);
	CT_VOID PrintFishControlData();
	//保存库存数据
	CT_VOID SaveKuCunData();
	//统计鱼种死亡概率
	CT_VOID StatisticFishInfo();
	//保存捕鱼的当日总吸分 和 当日总放分
	CT_VOID SaveFishTodayTotalWinLoseScore();
	//启服的时候读取捕鱼控制数据
	CT_BOOL ReadFishControlData(acl::mysql_manager &dbManager);
	//获取捕鱼库存值
	CT_LONGLONG GetFishKuCun() { return m_nAllTableKuCun; }
	//点控玩家输分的时候，玩家输分了
	CT_VOID NotifySysWinScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llWinScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose);
	//点控玩家赢分的时候，玩家赢分了
	CT_VOID NotifySysLoseScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llLoseScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose);
	CT_LONGLONG GetAllTableTotalWinScore() { return m_nAllTableTotalWinScore; }
	CT_LONGLONG GetAllTableTotalLoseScore() { return m_nAllTableTotalLoseScore; }

	CT_VOID TongJi(tagTongJiInfo &info);
	CT_VOID SinglePlayerMostWinLoseScore(CT_DWORD dwUserID, CT_LONGLONG addScore);
	//FishControl主动获取所有桌子上玩家的输赢
	CT_VOID TongJiAllPlayerAddScore();
	//玩家点控开始
	CT_VOID UserDianKongStart(CT_DWORD dwUserID);
	//玩家点控结束
	CT_VOID UserDianKongEnd(CT_DWORD dwUserID);
	//记录鱼种统计信息
	CT_VOID ReportFishStatisticsInfo(FishStatisticsInfo info[], int arrsize);
	//获取捕鱼系统税收
	CT_DWORD GetFishTaxPro() { return (CT_DWORD)CFishServerCfg::TaxPro;}
private:
	//所有桌子的总吸分,实时性数据
	CT_LONGLONG     m_nAllTableTotalWinScore;
	//所有桌子的总放分,实时性数据
	CT_LONGLONG     m_nAllTableTotalLoseScore;
	//捕鱼游戏中的临时数据，用于计算暗税																		
	CT_LONGLONG     m_nTaxTempValue;
	//库存,实时性数据
	CT_LONGLONG     m_nAllTableKuCun;
	//所有桌子的总税收,实时性数据
	CT_LONGLONG     m_nAllTableTotalTax;
	//所有桌子的当日总吸分
	CT_LONGLONG  m_llTodayTotalWinScore;
	//所有桌子的当日总放分
	CT_LONGLONG  m_llTodayTotalLoseScore;
	//所有桌子的当日库存
	CT_LONGLONG m_llTodayTotalKuCun;
	//所有桌子的当日税收
	CT_LONGLONG m_llTodayTotalTax;
	//所有桌子的当日点控输赢
	CT_LONGLONG m_llTodayDianKongWinLoseTotalScore;
	//所有桌子当日总进入人次
	CT_DWORD    m_dwTodayTotalEnterCount;
	//所有桌子当日总赢钱玩家数
	CT_DWORD    m_dwTodayTotalPlayerCount_Win;
	//所有桌子当日总破产玩家数
	CT_DWORD    m_dwTodayTotalPlayerCount_Bankrupted;
	//所有桌子当日总输但是非破产的玩家数
	CT_DWORD    m_dwTodayTotalPlayerCount_Lose;
	//所有桌子当日不输不赢的玩家数
	CT_DWORD    m_dwTodayTatalPlayerCount_NoWinLose;
	//所有桌子当日为降低破产率大幅度提高玩家命中率之后奖励给玩家的分数
	CT_DWORD    m_dwTodayForbidPoChanReward;
	//所有桌子上大鱼信息
	std::map<CT_BYTE, sBigFishInFo> m_mapTodayBigFishInfo;
	//所有桌子放分次数
	CT_DWORD  m_dwTodayRewardCount;
	//所有桌子吸分次数
	CT_DWORD  m_dwTodayXiFenCount;
	//整个服务器当日概率被设置情况
	std::map<CT_DWORD, CT_DWORD> m_mapTodayProInfo;
	//
	//当天的时间
	CT_UINT64      m_nTodayTime;
	//税率
	CT_DOUBLE     m_dTaxPro;
	//库存采样数据 采样规则:库存在目前的数值基础上向上浮动N(2000) 或者 向下浮动-N(-2000)就采样一次，所以，N值越小，采样形成的波动图越精确.
	//std::vector<sKuCunCaiYangInfo> m_vecKuCunCaiYang;
	std::list<sKuCunCaiYangInfo> m_listKuCunCaiYang;
	CT_LONGLONG m_llKuCunCaiYang;
	//FishControl是否启动了送分,启动了送分后,要送分完成才能启动下一次送分,送分完成的标识是点控玩家赢操作全部完成
	CT_BOOL m_bStartAddScore;
	//玩家当前最新的输赢数据
	std::vector<tagPlayerAddScore> m_vecUserAddScore;
	//库存的最高点 和 最低点
	CT_LONGLONG m_llKuCunMostTop;
	CT_LONGLONG m_llKuCunMostLow;
	//处于点控的玩家的点控输赢值记录:这个输赢值是针对玩家的。
	std::map<CT_DWORD, CT_LONGLONG> m_mapPlayerDianKongWinLose;
	//系统点控赢的分数:这个分数是由所有点控玩家输操作完成之后的分数总和,这是正数,没有转换汇率的值,代表系统通过点控盈利的累加值
	CT_LONGLONG m_llSysDianKongWinTotalScore;
	//系统点控输的总数:这个分数是由所有点控玩家赢操作完成之后的分数总和,这是正数，没有转换汇率的值,代表系统通过点控放分的累加值
	CT_LONGLONG m_llSysDianKongLoseTotalScore;
	//血池点控输赢 这个变量只是统计用的,实际上在放分的时候,
	CT_LONGLONG  m_llBloodPoolDianKongWinLose;
	//系统点控输赢总分:表示玩家完成了点控输赢后,系统的纯点控输赢总分,这个值为正代表系统点控玩家输后真正的盈利部分.这个值为负代表系统点控玩家赢后真正的放分部分，
	//所以这个值用于判断系统应该放多少分出去，而不能用m_llBloodPoolDianKongWinLose判断，因为这个值一直在变可能上升可能下降 。
	//m_llSysDianKongWinLoseTotalScore = m_llSysDianKongWinTotalScore - m_llSysDianKongLoseTotalScore
	CT_LONGLONG m_llSysDianKongWinLoseTotalScore;
	//目前点控赢的玩家集合
	std::vector<CT_DWORD> m_vecCurrentDianKongWinUsers;
	//所有鱼的历史记录
	FishStatisticsInfo                m_FishStatisticsInfo[MAX_FISH_TYPE];

	CT_LONGLONG m_llKuCunZhengShuCnt;
	CT_LONGLONG m_llKuCunFuShuCnt;

	CT_LONGLONG m_llTestBaseCapProDaYu10000;
	CT_LONGLONG m_llTestBaseCapProXiaoYu10000;

	//记录系统设置基础捕获概率的次数
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

	//统计所有人的税收总和
	CT_LONGLONG m_llTestAllPlayerTotalTax;

};

#endif

