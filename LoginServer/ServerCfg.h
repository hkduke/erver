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
	static int m_nServerID;
	static char* m_LocalAddress;
	static char* m_PublicAddress;
	static char* m_CenterAddress;
	static char* m_ServerName;

	static char* m_AccountdbIP;
	static char* m_AccountdbUser;
	static char* m_AccountdbPwd;
	static char* m_AccountdbName;
	static int   m_AccountdbPort;

	static char* m_PlatformdbIP;
	static char* m_PlatformdbUser;
	static char* m_PlatformdbPwd;
	static char* m_PlatformdbName;
	static int   m_PlatformdbPort;

	static char* m_RecorddbIP;
	static char* m_RecorddbUser;
	static char* m_RecorddbPwd;
	static char* m_RecorddbName;
	static int	 m_RecorddbPort;

	static char* m_GamedbIP;
	static char* m_GamedbUser;
	static char* m_GamedbPwd;
	static char* m_GamedbName;
	static int   m_GamedbPort;

	static char* m_RedisAddress;
	static char* m_RedisPwd;
	static int	 m_RedisConTimeOut;
	static int	 m_RedisRwTimeOut;

	static char* m_RedisAddress2;
	static char* m_RedisPwd2;
	static int	 m_RedisConTimeOut2;
	static int	 m_RedisRwTimeOut2;

	static char* m_SmsAccount;
	static char* m_SmsPwd;
	static char* m_SmsAccount1;
	static char* m_SmsPwd1;
	static char* m_RechargePostAddress;


	static int 	 m_MainLoginServer;
	static int 	 m_WorkThreadNum;

	static acl::master_int_tbl __conf_int_tab[];
	static acl::master_str_tbl __conf_str_tab[];

private:
	std::string m_strScriptPath;			// 脚本位置
	acl::master_conf	m_config;
};