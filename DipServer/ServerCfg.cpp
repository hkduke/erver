#include "stdafx.h"
#include "ServerCfg.h"

//template<> CServerCfg* CCSingleton<CServerCfg>::ms_Singleton = NULL;
int CServerCfg::m_nServerID = 0;
char* CServerCfg::m_LocalAddress = NULL;
char* CServerCfg::m_CenterAddress = NULL;
char* CServerCfg::m_DbServerAddress = NULL;
char* CServerCfg::m_ServerName = NULL;

char* CServerCfg::m_RedisAddress = NULL;
char* CServerCfg::m_RedisPwd = NULL;
int	 CServerCfg::m_RedisConTimeOut = 0;
int	 CServerCfg::m_RedisRwTimeOut = 0;

acl::master_int_tbl CServerCfg::__conf_int_tab[] =
{
	{ "server_id",			1,		&m_nServerID },
	{ "rediscon_timeout",	10,		&m_RedisConTimeOut },
	{ "redisrw_timeout",	10,		&m_RedisRwTimeOut },
	{ 0, 0, 0 }
};

acl::master_str_tbl CServerCfg::__conf_str_tab[] =
{
	{ "local_addr",		"127.0.0.1:9002",		&m_LocalAddress },
	{ "center_addr",	"127.0.0.1:9001",		&m_CenterAddress },
	{ "dbserver_addr",	"127.0.0.1:6001",		&m_DbServerAddress },
	{ "server_name",	"代理服",				&m_ServerName },
	{ "redis_addr",		"192.168.1.107:6379",	&m_RedisAddress },
	{ "redis_pwd",		"123456",				&m_RedisPwd },
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