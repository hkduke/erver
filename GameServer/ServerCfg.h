#pragma once
#include <string>
#include "acl_cpp/master/master_conf.hpp"
#include "Singleton.h"
#include "GlobalStruct.h"
#include <map>
#include <vector>

class CServerCfg
{
public:
	explicit CServerCfg(std::string strScriptPath);
	~CServerCfg(void);

public:
	void Load();
	static void InsertBenefitCfg(tagBenefitReward* pBenefitCfg);
	static void UpdateBenefitCfg(tagBenefitReward* pBenefitCfg);
	static tagBenefitReward* GetBenefitReward(CT_BYTE cbVip2);

public:
	static int		m_nServerID;
	static int		m_nGameID;
	static int		m_nGameKindID;
	static int		m_nRoomKindID;
	static int		m_nMaxCount;
	static char*	m_LocalAddress;
	static char*	m_PublicAddress;
	static char*	m_CenterAddress;
	static char*	m_DbServerAddress;
	static char*	m_ServerName;

	static char*	m_PlatformdbIP;
	static char*	m_PlatformdbUser;
	static char*	m_PlatformdbPwd;
	static char*	m_PlatformdbName;
	static int		m_PlatformdbPort;

	static char*	m_AccountdbIP;
	static char*	m_AccountdbUser;
	static char*	m_AccountdbPwd;
	static char*	m_AccountdbName;
	static int		m_AccountdbPort;

	static char*	m_RecorddbIP;
	static char*	m_RecorddbUser;
	static char*	m_RecorddbPwd;
	static char*	m_RecorddbName;
	static int		m_RecorddbPort;

	static char*	m_RedisAddress;
	static char*	m_RedisPwd;
	static int		m_RedisConTimeOut;
	static int		m_RedisRwTimeOut;

	static acl::master_int_tbl __conf_int_tab[];
	static acl::master_str_tbl __conf_str_tab[];

	//以下数据从数据库读取
	static int		m_nBenefitStatus;
	static tagBenefitReward m_BenefitRewardCfg;
	static std::vector<tagBenefitReward> m_vecBenefitRewardCfg;

private:
	std::string m_strScriptPath;			// 脚本位置
	acl::master_conf	m_config;				
};