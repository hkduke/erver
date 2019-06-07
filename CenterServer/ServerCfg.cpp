#include "stdafx.h"
#include "ServerCfg.h"

//template<> CServerCfg* CCSingleton<CServerCfg>::ms_Singleton = NULL;
int CServerCfg::m_nListenPort = 0;
char* CServerCfg::m_LocalAddress = NULL;

char* CServerCfg::m_RedisAddress = NULL;
char* CServerCfg::m_RedisPwd = NULL;
int	 CServerCfg::m_RedisConTimeOut = 0;
int	 CServerCfg::m_RedisRwTimeOut = 0;

char* CServerCfg::m_DbServerAddress = NULL;

acl::master_int_tbl CServerCfg::__conf_int_tab[] =
{
	{ "listen_port",	9001,	&m_nListenPort },

	{ "rediscon_timeout",	10,		&m_RedisConTimeOut },
	{ "redisrw_timeout",	10,		&m_RedisRwTimeOut },

	{ 0, 0, 0 }
};

acl::master_str_tbl CServerCfg::__conf_str_tab[] =
{
	{ "local_addr",		"127.0.0.1:9001",		&m_LocalAddress },
	{ "redis_addr",		"192.168.1.107:6379",	&m_RedisAddress },
	{ "redis_pwd",		"123456",				&m_RedisPwd },
	{ "dbserver_addr",	"127.0.0.1:6001",		&m_DbServerAddress },
	{ 0, 0, 0 }
};

CServerCfg::CServerCfg(std::string strScriptPath)
	: m_strScriptPath(strScriptPath)
{
	m_config.set_cfg_int(__conf_int_tab);
	m_config.set_cfg_str(__conf_str_tab);
}

CServerCfg::~CServerCfg()
{
}

void CServerCfg::Load()
{
	m_config.load(m_strScriptPath.c_str());
}