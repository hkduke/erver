#include "ServerCfg.h"

//template<> CServerCfg* CCSingleton<CServerCfg>::ms_Singleton = NULL;
int CServerCfg::m_nListenPort = 0;
char* CServerCfg::m_LocalAddress = NULL;

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

int	CServerCfg::m_RecordScoredbPort = 0;
char* CServerCfg::m_RecordScoredbIP = NULL;
char* CServerCfg::m_RecordScoredbUser = NULL;
char* CServerCfg::m_RecordScoredbPwd = NULL;
char* CServerCfg::m_RecordScoredbName = NULL;

int	CServerCfg::m_PlaybackdbPort = 0;
char* CServerCfg::m_PlaybackdbIP = NULL;
char* CServerCfg::m_PlaybackdbUser = NULL;
char* CServerCfg::m_PlaybackdbPwd = NULL;
char* CServerCfg::m_PlaybackdbName = NULL;

int	CServerCfg::m_PromotiondbPort = 0;
char* CServerCfg::m_PromotiondbIP = NULL;
char* CServerCfg::m_PromotiondbUser = NULL;
char* CServerCfg::m_PromotiondbPwd = NULL;
char* CServerCfg::m_PromotiondbName = NULL;

int CServerCfg::m_GamedbPort = 0;
char* CServerCfg::m_GamedbIP = NULL;
char* CServerCfg::m_GamedbUser = NULL;
char* CServerCfg::m_GamedbPwd = NULL;
char* CServerCfg::m_GamedbName = NULL;

int CServerCfg::m_PlatformdbPort = 0;
char* CServerCfg::m_PlatformdbIP = NULL;
char* CServerCfg::m_PlatformdbUser = NULL;
char* CServerCfg::m_PlatformdbPwd = NULL;
char* CServerCfg::m_PlatformdbName = NULL;

char* CServerCfg::m_RedisAddress = NULL;
char* CServerCfg::m_RedisPwd = NULL;
int	 CServerCfg::m_RedisConTimeOut = 0;
int	 CServerCfg::m_RedisRwTimeOut = 0;

int CServerCfg::m_MainDBServer = 0;
int CServerCfg::m_WorkThreadNum = 0;

acl::master_int_tbl CServerCfg::__conf_int_tab[] =
{
	{ "listen_port",		9004,	&m_nListenPort },
	{ "accountdb_port",		3306,	&m_AccountdbPort },
	{ "recorddb_port",		3306,	&m_RecorddbPort },
	{ "recordscoredb_port",	3306,	&m_RecordScoredbPort },
	{ "playbackdb_port",	3306,	&m_PlaybackdbPort },
	{ "promotiondb_port",	3306,	&m_PromotiondbPort },
	{ "gamedb_port",		3306,	&m_GamedbPort },
	{ "platformdb_port",	3306,	&m_PlatformdbPort },

	{ "rediscon_timeout",	10,		&m_RedisConTimeOut },
	{ "redisrw_timeout",	10,		&m_RedisRwTimeOut },

	{ "main_dbserver",		1,		&m_MainDBServer },
	{ "workthread_num",		1,		&m_WorkThreadNum },

	{ 0, 0, 0 }
};

acl::master_str_tbl CServerCfg::__conf_str_tab[] =
{
	{ "local_addr",			"127.0.0.1:9004",		&m_LocalAddress },

	{ "accountdb_ip",		"192.168.1.107",		&m_AccountdbIP },
	{ "accountdb_user",		"root",					&m_AccountdbUser },
	{ "accountdb_pwd",		"123456",				&m_AccountdbPwd },
	{ "accountdb_name",		"account",				&m_AccountdbName },

	{ "recorddb_ip",		"192.168.1.107",		&m_RecorddbIP },
	{ "recorddb_user",		"root",					&m_RecorddbUser },
	{ "recorddb_pwd",		"123456",				&m_RecorddbPwd },
	{ "recorddb_name",		"record",				&m_RecorddbName },

	{ "recordscoredb_ip",	"192.168.1.107",		&m_RecordScoredbIP },
	{ "recordscoredb_user",	"root",					&m_RecordScoredbUser },
	{ "recordscoredb_pwd",	"123456",				&m_RecordScoredbPwd },
	{ "recordscoredb_name",	"record_score",			&m_RecordScoredbName },

	{ "playbackdb_ip",		"192.168.1.107",		&m_PlaybackdbIP },
	{ "playbackdb_user",	"root",					&m_PlaybackdbUser },
	{ "playbackdb_pwd",		"123456",				&m_PlaybackdbPwd },
	{ "playbackdb_name",	"playback",				&m_PlaybackdbName },

	{ "promotiondb_ip",		"192.168.1.107",		&m_PromotiondbIP },
	{ "promotiondb_user",	"root",					&m_PromotiondbUser },
	{ "promotiondb_pwd",	"123456",				&m_PromotiondbPwd },
	{ "promotiondb_name",	"promotion",			&m_PromotiondbName },

	{ "gamedb_ip",			"192.168.1.107",		&m_GamedbIP },
	{ "gamedb_user",		"root",					&m_GamedbUser },
	{ "gamedb_pwd",			"123456",				&m_GamedbPwd },
	{ "gamedb_name",		"game",					&m_GamedbName },

	{ "platformdb_ip",		"192.168.1.107",		&m_PlatformdbIP },
	{ "platformdb_user",	"root",					&m_PlatformdbUser },
	{ "platformdb_pwd",		"123456",				&m_PlatformdbPwd },
	{ "platformdb_name",	"platform",				&m_PlatformdbName },

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