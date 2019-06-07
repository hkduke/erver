#include "ServerCfg.h"

//template<> CServerCfg* CCSingleton<CServerCfg>::ms_Singleton = NULL;
int CServerCfg::m_nServerID = 0;
int CServerCfg::m_nGameID = 0;
int CServerCfg::m_nGameKindID = 0;
int CServerCfg::m_nRoomKindID = 0;
int CServerCfg::m_nMaxCount = 0;
char* CServerCfg::m_LocalAddress = NULL;
char* CServerCfg::m_PublicAddress = NULL;
char* CServerCfg::m_CenterAddress = NULL;
char* CServerCfg::m_DbServerAddress = NULL;
char* CServerCfg::m_ServerName = NULL;
char* CServerCfg::m_ThirdServerAddress = NULL;

int CServerCfg::m_PlatformdbPort = 0;
char* CServerCfg::m_PlatformdbIP = NULL;
char* CServerCfg::m_PlatformdbUser = NULL;
char* CServerCfg::m_PlatformdbPwd = NULL;
char* CServerCfg::m_PlatformdbName = NULL;

int CServerCfg::m_AccountdbPort = 0;
char* CServerCfg::m_AccountdbIP = NULL;
char* CServerCfg::m_AccountdbUser = NULL;
char* CServerCfg::m_AccountdbPwd = NULL;
char* CServerCfg::m_AccountdbName = NULL;

int	CServerCfg::m_RecorddbPort = 0;
char* CServerCfg::m_RecorddbIP = NULL;
char* CServerCfg::m_RecorddbUser = NULL;
char* CServerCfg::m_RecorddbPwd = NULL;
char* CServerCfg::m_RecorddbName = NULL;


char* CServerCfg::m_RedisAddress = NULL;
char* CServerCfg::m_RedisPwd = NULL;
int	 CServerCfg::m_RedisConTimeOut = 0;
int	 CServerCfg::m_RedisRwTimeOut = 0;


int CServerCfg::m_nBenefitStatus = 0;
tagBenefitReward CServerCfg::m_BenefitRewardCfg;

acl::master_int_tbl CServerCfg::__conf_int_tab[] =
{
	{ "server_id",		1,		&m_nServerID },
	{ "game_id",		1,		&m_nGameID },
	{ "kind_id",		1,		&m_nGameKindID },
	{ "room_kind",		0,		&m_nRoomKindID },

	{ "max_count",			3500,	&m_nMaxCount },
	{ "platformdb_port",	3306,	&m_PlatformdbPort },
	{ "accountdb_port",		3306,	&m_AccountdbPort },
	{ "recorddb_port",		3306,	&m_RecorddbPort },
	{ "rediscon_timeout",	10,		&m_RedisConTimeOut },
	{ "redisrw_timeout",	10,		&m_RedisRwTimeOut },
	{ 0, 0, 0 }
};

acl::master_str_tbl CServerCfg::__conf_str_tab[] =
{
	{ "local_addr",			"127.0.0.1:9005",		&m_LocalAddress },
	{ "public_addr",		"127.0.0.1:9005",		&m_PublicAddress },
	{ "center_addr",		"127.0.0.1:9001",		&m_CenterAddress },
	{ "dbserver_addr",		"127.0.0.1:6001",		&m_DbServerAddress },
	{ "server_name",		"游戏服1",				&m_ServerName },
	{ "platformdb_ip",		"192.168.1.107",		&m_PlatformdbIP },
	{ "platformdb_user",	"root",					&m_PlatformdbUser },
	{ "platformdb_pwd",		"123456",				&m_PlatformdbPwd },
	{ "platformdb_name",	"platform",				&m_PlatformdbName },
	{ "thirdserver_addr",    "192.168.0.106:7105",	&m_ThirdServerAddress },

	{ "accountdb_ip",		"192.168.1.107",		&m_AccountdbIP },
	{ "accountdb_user",		"root",					&m_AccountdbUser },
	{ "accountdb_pwd",		"123456",				&m_AccountdbPwd },
	{ "accountdb_name",		"account",				&m_AccountdbName },

	{ "recorddb_ip",		"192.168.1.107",		&m_RecorddbIP },
	{ "recorddb_user",		"root",					&m_RecorddbUser },
	{ "recorddb_pwd",		"123456",				&m_RecorddbPwd },
	{ "recorddb_name",		"record",				&m_RecorddbName },

	{ "redis_addr",			"192.168.1.107:6379",	&m_RedisAddress },
	{ "redis_pwd",			"123456",				&m_RedisPwd },
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