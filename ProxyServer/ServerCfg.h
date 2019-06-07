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
	static int m_nProxyType;
	static int m_nUsePublicAddress;
	static char* m_LocalAddress;
	static char* m_CenterAddress;
	static char* m_DbServerAddress;
	//static char* m_CustomerSerAddress;
	static char* m_ServerName;
	static char* m_RedisAddress;
	static char* m_RedisPwd;
	static int	 m_RedisConTimeOut;
	static int	 m_RedisRwTimeOut;
	static acl::master_int_tbl __conf_int_tab[];
	static acl::master_str_tbl __conf_str_tab[];

private:
	std::string m_strScriptPath;			// 脚本位置
	acl::master_conf	m_config;				
};