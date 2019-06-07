#pragma once
#include <string>
#include "acl_cpp/master/master_conf.hpp"
#include "Singleton.h"

class CServerCfg
{
public:
	CServerCfg(std::string strScriptPath);
	~CServerCfg(void);

public:
	void Load();

public:
	static int m_nListenPort;
	static char* m_LocalAddress;
	static acl::master_int_tbl __conf_int_tab[];
	static acl::master_str_tbl __conf_str_tab[];

private:
	std::string m_strScriptPath;			// 脚本位置
	acl::master_conf	m_config;				

public:
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

	static char*	m_RecordScoredbIP;
	static char*	m_RecordScoredbUser;
	static char*	m_RecordScoredbPwd;
	static char*	m_RecordScoredbName;
	static int		m_RecordScoredbPort;

	static char*	m_PlaybackdbIP;
	static char*	m_PlaybackdbUser;
	static char*	m_PlaybackdbPwd;
	static char*	m_PlaybackdbName;
	static int		m_PlaybackdbPort;

	static char*	m_PromotiondbIP;
	static char*	m_PromotiondbUser;
	static char*	m_PromotiondbPwd;
	static char*	m_PromotiondbName;
	static int		m_PromotiondbPort;

	static char*	m_GamedbIP;
	static char*	m_GamedbUser;
	static char*	m_GamedbPwd;
	static char*	m_GamedbName;
	static int		m_GamedbPort;

	static char*	m_PlatformdbIP;
	static char*	m_PlatformdbUser;
	static char*	m_PlatformdbPwd;
	static char*	m_PlatformdbName;
	static int		m_PlatformdbPort;

	static char*	m_RedisAddress;
	static char*	m_RedisPwd;
	static int		m_RedisConTimeOut;
	static int		m_RedisRwTimeOut;

	static int 		m_MainDBServer;
	static int 		m_WorkThreadNum;
};