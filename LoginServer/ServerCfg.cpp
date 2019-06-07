#include "ServerCfg.h"

//template<> CServerCfg* CCSingleton<CServerCfg>::ms_Singleton = NULL;
int CServerCfg::m_nServerID = 0;
char* CServerCfg::m_LocalAddress = NULL;
char* CServerCfg::m_PublicAddress = NULL;
char* CServerCfg::m_CenterAddress = NULL;
char* CServerCfg::m_ServerName = NULL;

int CServerCfg::m_AccountdbPort = 0;
char* CServerCfg::m_AccountdbIP = NULL;
char* CServerCfg::m_AccountdbUser = NULL;
char* CServerCfg::m_AccountdbPwd = NULL;
char* CServerCfg::m_AccountdbName = NULL;

int CServerCfg::m_PlatformdbPort = 0;
char* CServerCfg::m_PlatformdbIP = NULL;
char* CServerCfg::m_PlatformdbUser = NULL;
char* CServerCfg::m_PlatformdbPwd = NULL;
char* CServerCfg::m_PlatformdbName = NULL;

int	CServerCfg::m_RecorddbPort = 0;
char* CServerCfg::m_RecorddbIP = NULL;
char* CServerCfg::m_RecorddbUser = NULL;
char* CServerCfg::m_RecorddbPwd = NULL;
char* CServerCfg::m_RecorddbName = NULL;

int CServerCfg::m_GamedbPort = 0;
char* CServerCfg::m_GamedbIP = NULL;
char* CServerCfg::m_GamedbUser = NULL;
char* CServerCfg::m_GamedbPwd = NULL;
char* CServerCfg::m_GamedbName = NULL;

char* CServerCfg::m_RedisAddress = NULL;
char* CServerCfg::m_RedisPwd = NULL;
int	 CServerCfg::m_RedisConTimeOut = 0;
int	 CServerCfg::m_RedisRwTimeOut = 0;

char* CServerCfg::m_RedisAddress2 = NULL;
char* CServerCfg::m_RedisPwd2 = NULL;
int	 CServerCfg::m_RedisConTimeOut2 = 0;
int	 CServerCfg::m_RedisRwTimeOut2 = 0;

char* CServerCfg::m_SmsAccount = NULL;
char* CServerCfg::m_SmsPwd = NULL;
char* CServerCfg::m_SmsAccount1 = NULL;
char* CServerCfg::m_SmsPwd1 = NULL;
char* CServerCfg::m_RechargePostAddress = NULL;

int	CServerCfg::m_MainLoginServer = 1;
int	CServerCfg::m_WorkThreadNum = 1;

acl::master_int_tbl CServerCfg::__conf_int_tab[] =
{
	{ "server_id",			1,		&m_nServerID },

	{ "accountdb_port",		3306,	&m_AccountdbPort },
	{ "platformdb_port",	3306,	&m_PlatformdbPort },
	{ "recorddb_port",		3306,	&m_RecorddbPort },
	{ "gamedb_port",		3306,	&m_GamedbPort },

	{ "rediscon_timeout",	10,		&m_RedisConTimeOut },
	{ "redisrw_timeout",	10,		&m_RedisRwTimeOut },

	{ "rediscon_timeout2",	10,		&m_RedisConTimeOut2 },
	{ "redisrw_timeout2",	10,		&m_RedisRwTimeOut2 },

	{ "main_loginserver",	1,		&m_MainLoginServer },
	{ "workthread_num",		1,		&m_WorkThreadNum },

	{ 0, 0, 0 }
};

acl::master_str_tbl CServerCfg::__conf_str_tab[] =
{
	{ "local_addr",			"127.0.0.1:9002",		&m_LocalAddress },
	{ "public_addr",		"127.0.0.1:9002",		&m_PublicAddress },
	{ "center_addr",		"127.0.0.1:9001",		&m_CenterAddress },
	{ "server_name",		"登录服1",				&m_ServerName },

	{ "accountdb_ip",		"192.168.1.107",		&m_AccountdbIP },
	{ "accountdb_user",		"root",					&m_AccountdbUser },
	{ "accountdb_pwd",		"123456",				&m_AccountdbPwd },
	{ "accountdb_name",		"account",				&m_AccountdbName },

	{ "platformdb_ip",		"192.168.1.107",		&m_PlatformdbIP },
	{ "platformdb_user",	"root",					&m_PlatformdbUser },
	{ "platformdb_pwd",		"123456",				&m_PlatformdbPwd },
	{ "platformdb_name",	"platform",				&m_PlatformdbName },

	{ "recorddb_ip",		"192.168.1.107",		&m_RecorddbIP },
	{ "recorddb_user",		"root",					&m_RecorddbUser },
	{ "recorddb_pwd",		"123456",				&m_RecorddbPwd },
	{ "recorddb_name",		"record",				&m_RecorddbName },

	{ "gamedb_ip",			"192.168.1.107",		&m_GamedbIP },
	{ "gamedb_user",		"root",					&m_GamedbUser },
	{ "gamedb_pwd",			"123456",				&m_GamedbPwd },
	{ "gamedb_name",		"game",					&m_GamedbName },

	{ "redis_addr",			"192.168.1.107:6379",	&m_RedisAddress },
	{ "redis_pwd",			"123456",				&m_RedisPwd },

	{ "redis_addr2",		"192.168.1.107:6379",	&m_RedisAddress2 },
	{ "redis_pwd2",			"123456",				&m_RedisPwd2 },

	{ "sms_account",		"N5264141",				&m_SmsAccount },
	{ "sms_pwd",			"R8oDvVIZpG14c0",		&m_SmsPwd },
	{ "sms_account1",		"N5264141",				&m_SmsAccount1 },
	{ "sms_pwd1",			"R8oDvVIZpG14c0",		&m_SmsPwd1 },
	{ "exchange_addr",		"192.168.0.107:6396",	&m_RechargePostAddress },

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