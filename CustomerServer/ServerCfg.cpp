#include "stdafx.h"
#include "ServerCfg.h"

//template<> CServerCfg* CCSingleton<CServerCfg>::ms_Singleton = NULL;
int CServerCfg::m_nServerID = 0;
char* CServerCfg::m_LocalAddress = NULL;
char* CServerCfg::m_PublicAddress = NULL;
char* CServerCfg::m_CenterAddress = NULL;
char* CServerCfg::m_DbServerAddress = NULL;
char* CServerCfg::m_ServerName = NULL;

int CServerCfg::m_chatdbPort = 0;
char* CServerCfg::m_chatdbIP = NULL;
char* CServerCfg::m_chatdbUser = NULL;
char* CServerCfg::m_chatdbPwd = NULL;
char* CServerCfg::m_chatdbName = NULL;

char* CServerCfg::m_RedisAddress = NULL;
char* CServerCfg::m_RedisPwd = NULL;
int	 CServerCfg::m_RedisConTimeOut = 0;
int	 CServerCfg::m_RedisRwTimeOut = 0;

char* CServerCfg::m_CustomerServerAddr = NULL;
int   CServerCfg::m_CustomerServerPort = 0;

acl::master_int_tbl CServerCfg::__conf_int_tab[] =
{
	{ "server_id",			1,		&m_nServerID },
	{ "rediscon_timeout",	10,		&m_RedisConTimeOut },
	{ "redisrw_timeout",	10,		&m_RedisRwTimeOut },
	{ "oadb_port",			3306,	&m_chatdbPort },
	{ "customer_port",      80,     &m_CustomerServerPort},
	{ 0, 0, 0 }
};

acl::master_str_tbl CServerCfg::__conf_str_tab[] =
{
	{ "local_addr",		"127.0.0.1:9002",		&m_LocalAddress },
	{ "public_addr",	"127.0.0.1:9002",		&m_PublicAddress },
	{ "center_addr",	"127.0.0.1:9001",		&m_CenterAddress },
	{ "dbserver_addr",	"127.0.0.1:6001",		&m_DbServerAddress },
	{ "server_name",	"客服服",					&m_ServerName },
	{ "redis_addr",		"192.168.1.107:6379",	&m_RedisAddress },
	{ "redis_pwd",		"123456",				&m_RedisPwd },

	{ "chatdb_ip",		"192.168.1.107",		&m_chatdbIP },
	{ "chatdb_user",	"root",					&m_chatdbUser },
	{ "chatdb_pwd",		"123456",				&m_chatdbPwd },
	{ "chatdb_name",	"oa",					&m_chatdbName },

	{ "customer_addr",	"chat.xk8188.com",		&m_CustomerServerAddr },

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