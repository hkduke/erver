#include "BuildDBThread.h"
#include "glog_wrapper.h"
#include "ServerCfg.h"
#include "Utility.h"

const std::string record_big_score = "(\
	`id` bigint(20) NOT NULL AUTO_INCREMENT COMMENT '大局ID',\
	`gameid` int(11) DEFAULT NULL COMMENT '游戏ID',\
	`kindid` int(11) DEFAULT NULL COMMENT '游戏种类ID',\
	`roomNum` int(11) DEFAULT NULL,\
	`startTime` datetime DEFAULT NULL COMMENT '开始时间',\
	`endTime` datetime DEFAULT NULL COMMENT '结束时间',\
	PRIMARY KEY(`id`)\
	) ENGINE = InnoDB AUTO_INCREMENT = 1 DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci COMMENT = '游戏大局表';";

const std::string record_big_detail = "(\
	`id` bigint(20) DEFAULT NULL COMMENT '大局ID',\
	`userid` int(11) DEFAULT NULL COMMENT '玩家ID',\
	`nickname` varchar(32) COLLATE utf8mb4_unicode_ci DEFAULT NULL,\
	`score` int(11) DEFAULT NULL COMMENT '得分',\
	KEY `idx_record_big_detail` (`id`,`userid`)\
	) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci;";

const std::string record_small_detail = "(\
	`id` bigint(20) DEFAULT NULL COMMENT '大局ID',\
	`sid` tinyint(4) DEFAULT NULL COMMENT '小局ID',\
	`userid` int(11) DEFAULT NULL COMMENT '玩家ID',\
	`chairid` tinyint(4) DEFAULT NULL,\
	`score` int(11) DEFAULT NULL COMMENT '得分',\
	`nickname` varchar(32) COLLATE utf8mb4_unicode_ci DEFAULT NULL,\
	`fixdate` datetime DEFAULT NULL COMMENT '结束时间',\
	KEY `idx_userid` (`id`,`sid`,`userid`) USING BTREE\
	) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci COMMENT = '小局玩家总体输赢表'; ";

const std::string playbackdata = "( \
	`gameRoundId` bigint(20) NOT NULL,\
	`playId` tinyint(4) NOT NULL,\
	`data` blob,\
	`InsertTime` timestamp NULL DEFAULT CURRENT_TIMESTAMP,\
	PRIMARY KEY(`gameRoundId`,`playId`)\
	) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci;";

const std::string playbackdata2 = "( \
	`drawid` bigint(20) NOT NULL, \
	`data` blob,\
	`InsertTime` timestamp NULL DEFAULT CURRENT_TIMESTAMP,\
	 PRIMARY KEY (`drawid`)\
	) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci;";

const std::string record_score_change = "(\
	`userid` int(11) NOT NULL COMMENT '玩家ID',\
	`sourceBank` bigint(20) NOT NULL COMMENT '原有的银行金币',\
	`sourceScore` bigint(20) NOT NULL COMMENT '原有的身上金币',\
	`changeBank` bigint(20) NOT NULL COMMENT '变化的银行金币',\
	`changeScore` bigint(20) NOT NULL COMMENT '变化的身上金币',\
	`drawid` int(11) NOT NULL DEFAULT '0' COMMENT '游戏记录id',\
	`type` tinyint(4) NOT NULL COMMENT '变化类型 兑换(1), 充值(2), 赠送(3), 充值返还(4), 签到(5), 救济金(6), 红包(7), 存入金币(8), 取出金币(9),任务奖励(10),兑换退还(11), 邮件领取(12),游戏记录(20)',\
	`fixdate` int(11) NOT NULL COMMENT '时间',\
	KEY `idx_user` (`userid`,`fixdate`),\
 	KEY `idx_type` (`type`)\
	) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci COMMENT = '金币兑换记录';";

const std::string record_draw_info = "(\
	`drawid` int(11) NOT NULL AUTO_INCREMENT COMMENT '局数标识',\
	`gameid` int(11) NOT NULL COMMENT '游戏ID',\
	`kindid` int(11) NOT NULL COMMENT '游戏类型ID',\
	`roomKindid` int(11) NOT NULL COMMENT '游戏对战类型ID',\
	`tableid` int(11) NOT NULL COMMENT '桌子号码',\
	`userCount` smallint(6) NOT NULL COMMENT '用户数目',\
	`androidCount` tinyint(4) NOT NULL COMMENT '机器数目',\
	`bankUserid` int(11) NOT NULL DEFAULT '0' COMMENT '百人游戏庄家ID',\
	`stock` bigint(20) NOT NULL DEFAULT '0' COMMENT '百人游戏当前剩余库存',\
	`playTime` int(11) NOT NULL COMMENT '游戏时长',\
	`fixdate` int(11) NOT NULL COMMENT '记录时间',\
	PRIMARY KEY(`drawid`),\
	KEY `idx_gameid_kindid` (`gameid`,`kindid`,`roomKindid`) USING BTREE,\
	KEY `idx_bankUserId` (`bankUserid`) \
	) ENGINE = InnoDB AUTO_INCREMENT = 1 DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci COMMENT = '游戏数据概览表';";

const std::string record_draw_score = "(\
	`drawid` int(11) NOT NULL COMMENT '局数标识',\
	`userid` int(11) NOT NULL COMMENT '用户标识',\
	`chairid` int(11) NOT NULL COMMENT '椅子号码',\
	`score` int(11) NOT NULL COMMENT '用户成绩',\
	`revenue` int(11) NOT NULL COMMENT '税收数目',\
	`fixdate` int(11) NOT NULL COMMENT '记录时间',\
	`isbot` tinyint(4) NOT NULL COMMENT '是否机器人',\
	`report` tinyint(4) NOT NULL DEFAULT '0' COMMENT '是否举报本次局游戏',\
	`location` varchar(33) COLLATE utf8mb4_unicode_ci DEFAULT NULL,\
	 KEY `idx_drawid` (`drawid`) USING BTREE, \
	 KEY `idx_fixdate` (`fixdate`) USING BTREE,\
	 KEY `idx_userid` (`userid`) USING BTREE,\
	 KEY `idx_score` (`isbot`,`score`) USING BTREE \
	) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci COMMENT = '游戏数据详细记录表';";

const std::string record_score_br = "(\
	`drawid` int(11) NOT NULL COMMENT '局数标识',\
	`userid` int(11) NOT NULL COMMENT '用户标识',\
	`area1Jetton` int(11) NOT NULL COMMENT '用户区域成绩',\
	`area2Jetton` int(11) NOT NULL COMMENT '用户区域成绩',\
	`area3Jetton` int(11) NOT NULL COMMENT '用户区域成绩',\
	`area4Jetton` int(11) NOT NULL COMMENT '用户区域成绩',\
	`area0CardType` tinyint(4) NOT NULL COMMENT '庄家区域牌型',\
	`area1CardType` tinyint(4) NOT NULL COMMENT '区域1牌型',\
	`area2CardType` tinyint(4) NOT NULL COMMENT '区域2牌型',\
	`area3CardType` tinyint(4) NOT NULL COMMENT '区域3牌型',\
	`area4CardType` tinyint(4) NOT NULL COMMENT '区域4牌型',\
	`sourceScore` bigint(20) NOT NULL,\
	`winScore` int(11) NOT NULL,\
	`fixdate` int(11) NOT NULL COMMENT '记录时间',\
	KEY `idx_drawid` (`drawid`) USING BTREE,\
	KEY `idx_fixdate` (`fixdate`) \
	) ENGINE = InnoDB DEFAULT CHARSET = utf8mb4 COLLATE = utf8mb4_unicode_ci COMMENT = '游戏数据详细记录表';";

const std::string record_login = "("
	 "`id` int(11) NOT NULL AUTO_INCREMENT COMMENT '自增列',\n"
	 "  `userid` int(11) NOT NULL COMMENT '帐号ID',\n"
	 "  `fixdate` int(11) NOT NULL COMMENT '登录时间',\n"
	 "  `loginSerial` varchar(64) NOT NULL COMMENT '登录机器码',\n"
	 "  `loginMachine` varchar(64) NOT NULL COMMENT '登录手机型号',\n"
	 "  `ip` varchar(15) NOT NULL COMMENT '进入IP',\n"
	 "  PRIMARY KEY (`id`),\n"
	 "  KEY `idx_fixdate` (`fixdate`) USING BTREE,\n"
     "  KEY `idx_userid` (`userid`) USING BTREE"
	 ") ENGINE = InnoDB DEFAULT CHARSET = utf8 COMMENT = '用户登录记录'; ";
	
void CBuildDBThread::AddRecordScoreStuffTable()
{
	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_RecordScoredbIP, CServerCfg::m_RecordScoredbPort);
	acl::db_mysql db(dbaddr, CServerCfg::m_RecordScoredbName, CServerCfg::m_RecordScoredbUser, CServerCfg::m_RecordScoredbPwd);

	if (db.open() == false)
	{
		LOG(WARNING) << "open :" << dbaddr.c_str() << "@mysql error!";
		return;
	}

	std::string date = Utility::GetTimeNowString("%Y%m%d");
	std::string thisMonth = date.substr(0, 6);
	std::string nextMonth = Utility::GetNextMonth(date);
	//std::string lastMonth = Utility::GetLastYearMonth(Utility::GetTimeNowString());

	//std::string gameRoundTable = "record.record_big_score_" + thisMonth;
	//std::string gameRoundTableNext = "record.record_big_score_" + nextMonth;

	//std::string gameRoundDetailTable = "record.record_big_detail_" + thisMonth;
	//std::string gameRoundDetailTableNext = "record.record_big_detail_" + nextMonth;
	//std::string gameRoundDetailTableLast = "record.record_big_detail_" + lastMonth;

	//std::string gameRoundSmallDetailTable  = "record.record_small_detail_" + thisMonth;
	//std::string gameRoundSmallDetailTableNext = "record.record_small_detail_" + nextMonth;

	std::string recordDrawInfoTable = "record_score.record_draw_info_" + thisMonth;
	std::string recordDrawInfoTableNext = "record_score.record_draw_info_" + nextMonth;

	std::string recordDrawScoreTable = "record_score.record_draw_score_" + thisMonth;
	std::string recordDrawScoreTableNext = "record_score.record_draw_score_" + nextMonth;

	std::string recordBRScoreTable = "record_score.record_score_br_" + thisMonth;
	std::string recordBRScoreTableNext = "record_score.record_score_br_" + nextMonth;
	
	std::string playbackTable = "playback.playbackdata_" + thisMonth;
	std::string playbackTableNext = "playback.playbackdata_" + nextMonth;

	std::vector<std::string> vecSql;
	//std::string sql = "CREATE TABLE IF NOT EXISTS " + gameRoundTable + record_big_score;
	//vecSql.push_back(sql);
	//std::string sql = "CREATE TABLE IF NOT EXISTS " + gameRoundTableNext + record_big_score;
	//vecSql.push_back(sql);
	//std::string sql = "CREATE TABLE IF NOT EXISTS " + gameRoundDetailTable + record_big_detail;
	//vecSql.push_back(sql);
	//sql = "CREATE TABLE IF NOT EXISTS " + gameRoundDetailTableNext + record_big_detail;
	//vecSql.push_back(sql);
	//sql = "CREATE TABLE IF NOT EXISTS " + gameRoundDetailTableLast + record_big_detail;
	//vecSql.push_back(sql);
	//sql = "CREATE TABLE IF NOT EXISTS " + gameRoundSmallDetailTable + record_small_detail;
	//vecSql.push_back(sql);
	//sql = "CREATE TABLE IF NOT EXISTS " + gameRoundSmallDetailTableNext + record_small_detail;
	
	std::string sql = "CREATE TABLE IF NOT EXISTS " + recordDrawInfoTable + record_draw_info;
	vecSql.push_back(sql);
	sql = "CREATE TABLE IF NOT EXISTS " + recordDrawInfoTableNext + record_draw_info;
	vecSql.push_back(sql);

	sql = "CREATE TABLE IF NOT EXISTS " + recordDrawScoreTable + record_draw_score;
	vecSql.push_back(sql);
	sql = "CREATE TABLE IF NOT EXISTS " + recordDrawScoreTableNext + record_draw_score;
	vecSql.push_back(sql);

	sql = "CREATE TABLE IF NOT EXISTS " + recordBRScoreTable + record_score_br;
	vecSql.push_back(sql);
	sql = "CREATE TABLE IF NOT EXISTS " + recordBRScoreTableNext + record_score_br;
	vecSql.push_back(sql);
	
	sql = "CREATE TABLE IF NOT EXISTS " + playbackTable + playbackdata2;
	vecSql.push_back(sql);
	sql = "CREATE TABLE IF NOT EXISTS " + playbackTableNext + playbackdata2;
	vecSql.push_back(sql);

	for (auto& it : vecSql)
	{
		if (db.sql_update(it.c_str()) == false)
		{
			LOG(WARNING) << "sql error: " << it.c_str() << ", error: " << db.get_error();
			return;
		}
	}
}

void CBuildDBThread::AddRecordStuffTable()
{
	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_RecorddbIP, CServerCfg::m_RecorddbPort);
	acl::db_mysql db(dbaddr, CServerCfg::m_RecorddbName, CServerCfg::m_RecorddbUser, CServerCfg::m_RecorddbPwd);

	if (db.open() == false)
	{
		LOG(WARNING) << "open :" << dbaddr.c_str() << "@mysql error!";
		return;
	}

	std::string date = Utility::GetTimeNowString("%Y%m%d");
	std::string thisMonth = date.substr(0, 6);
	std::string nextMonth = Utility::GetNextMonth(date);
	
	std::string recordChangeTable = "record.record_score_change_" + thisMonth;
	std::string recordChangeTableNext = "record.record_score_change_" + nextMonth;

	std::string recordLoginTable = "record_login_" + thisMonth;
	std::string recordLoginTableNext = "record_login_" + nextMonth;

	std::vector<std::string> vecSql;

	std::string sql = "CREATE TABLE IF NOT EXISTS " + recordChangeTable + record_score_change;
	vecSql.push_back(sql);
	sql = "CREATE TABLE IF NOT EXISTS " + recordChangeTableNext + record_score_change;
	vecSql.push_back(sql);

	sql = "CREATE TABLE IF NOT EXISTS " + recordLoginTable + record_login;
	vecSql.push_back(sql);
	sql = "CREATE TABLE IF NOT EXISTS " + recordLoginTableNext + record_login;
	vecSql.push_back(sql);

	for (auto& it : vecSql)
	{
		if (db.sql_update(it.c_str()) == false)
		{
			LOG(WARNING) << "sql error: " << it.c_str() << ", errno: " << db.get_errno() << ", error: " << db.get_error();
			return;
		}
	}
}

void CBuildDBThread::AddPRoomPlaybackTable()
{
	acl::string dbaddr;
	dbaddr.format("%s:%d", CServerCfg::m_PlaybackdbIP, CServerCfg::m_PlaybackdbPort);
	acl::db_mysql db(dbaddr, CServerCfg::m_PlaybackdbName, CServerCfg::m_PlaybackdbUser, CServerCfg::m_PlaybackdbPwd);

	if (db.open() == false)
	{
		LOG(WARNING) << "open :" << dbaddr.c_str() << "@mysql error!";
		return;
	}

	std::string date = Utility::GetTimeNowString("%Y%m%d");
	std::string thisMonth = date.substr(0, 6);
	std::string nextMonth = Utility::GetNextMonth(date);

	std::string playbackTable = "playback.playbackdata_" + thisMonth;
	std::string playbackTableNext = "playback.playbackdata_" + nextMonth;

	std::vector<std::string> vecSql;
	std::string sql = "CREATE TABLE IF NOT EXISTS " + playbackTable + playbackdata;
	vecSql.push_back(sql);
	sql = "CREATE TABLE IF NOT EXISTS " + playbackTableNext + playbackdata;
	vecSql.push_back(sql);

	for (auto& it : vecSql)
	{
		if (db.sql_update(it.c_str()) == false)
		{
			LOG(WARNING) << "sql error: " << it.c_str();
			return;
		}
	}
}

void* CBuildDBThread::run()
{
	do 
	{
		try
		{
			LOG(WARNING) << "CBuildDBThread run, begin build database ..";
			Utility::Sleep(2000);
			AddRecordScoreStuffTable();
			AddRecordStuffTable();
			//AddPRoomPlaybackTable();
		}
		catch (const std::exception& e)
		{
			LOG(ERROR) << "CBuildDBThread run catch a std exception: " << e.what();
		}
		catch (...)
		{
			LOG(ERROR) << "CBuildDBThread run catch a unkown exception!";
		}
		Utility::Sleep(1000 * 3600);
	} while (true);

	delete this;
	return NULL;
}