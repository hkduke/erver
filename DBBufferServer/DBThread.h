#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "CMD_Inner.h"
#include "CMD_Plaza.h"
#include "GlobalStruct.h"

class DBThread : public CWorkThread
{
public:
	DBThread(void);
	~DBThread(void);

public:
	//网络连接
	virtual CT_VOID OnTCPSocketLink(CNetConnector* pConnector);
	//网络连接关闭
	virtual CT_VOID OnTCPSocketShut(CNetConnector* pConnector);
	//网络应答
	virtual CT_VOID OnTCPNetworkBind(acl::aio_socket_stream* pSocket);
	//网络关闭
	virtual CT_VOID OnTCPNetworkShut(acl::aio_socket_stream* pSocket);
	//网络数据到来
	virtual CT_VOID OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pBuf, CT_DWORD wLen);
	//网络定时器
	virtual CT_VOID OnTimer(CT_DWORD dwTimerID);

	//打开
	virtual CT_VOID Open();

private:
	CT_VOID InitServer();

private:
	CT_VOID OnGameServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnProxyServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	CT_VOID OnCenterServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

private:
	CT_VOID UpdateUserCount();
	CT_BOOL IsAcrossTheDay();
	CT_VOID AcrossTheDayEvent();

private:
	//私人房相关
	CT_VOID UpdateUserGem(acl::db_handle* db, CT_DWORD dwUserID, CT_INT32 nAddGem);
	CT_VOID InsertGemConsumeRecord(acl::db_handle* db, MSG_G2DB_Gem_Consume* pConsume);
	CT_VOID InsertPRoomPlayCount(acl::db_handle* db, MSG_G2DB_PRoom_PlayCountRecord* pPlayCount);
	CT_VOID InsertPRoomGameRound(acl::db_handle* db, MSG_G2DB_PRoom_GameRoundRecord* pGameRound);
	CT_VOID InsertPlaybackRecord(acl::db_mysql* db, MSG_G2DB_PRoom_PlaybackRecord* pRecord);
	CT_VOID InsertPRoomGameRoundCount(acl::db_handle* db, MSG_G2DB_PRoom_GameRoundCount* pGameRoundCount);
	CT_VOID InsertWinLostRecord(acl::db_handle* db, MSG_G2DB_WinLostRecord* pWinLostRecord);
	CT_VOID InsertGameRoundRecord(acl::aio_socket_stream*pSocket, acl::db_handle* db, MSG_G2DB_PRoom_InsertGameRound* pWinLostRecord);
	CT_VOID QueryPRoomGameRound(acl::aio_socket_stream*pSocket, acl::db_handle* db, MSG_P2DB_Query_PRoom_GameRound* pGameRound);
	CT_VOID QueryPRoomPlayCount(acl::aio_socket_stream*pSocket, acl::db_handle* db, MSG_P2DB_Query_PRoom_PlayCount* pPlayCount);

	//在线统计
	CT_VOID InsertOnlineRecord(acl::db_handle* db, CT_BYTE cbPlatformId, CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwOnlineCount, CT_BYTE bType);
	CT_VOID InsertMachineOnlineRecord(acl::db_handle* db, CT_BYTE cbPlatformId, CT_DWORD dwOnlineCount, CT_BYTE cbType);

	//群组相关
	CT_VOID LoadGroupChatMsg();
    inline  CT_DWORD CalcDayInt(int nBeforeDays);
    CT_VOID LoadGroupInfo();
	CT_VOID LoadGroupUserRevenue();
	CT_VOID LoadGroupLevelCfg();
	CT_VOID QueryGroupPRoomConfig(acl::aio_socket_stream* pSocket, acl::db_handle* db);
	CT_VOID SetGroupPRoomState(acl::db_handle* db, MSG_CS2DB_Return_GroupPRoom_Gem* pReturnGem);
	CT_VOID ReturnGroupPRoomGem(acl::db_handle* db, MSG_CS2DB_Return_GroupPRoom_Gem* pReturnGem);
	CT_VOID InsertGroupPRoomRecord(acl::db_handle* db, MSG_G2DB_Record_GroupPRoom* pAutoPRoom);
	CT_VOID InsertGroupPRoomInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_GroupRoom_Info* pGroupRoomInfo);
	CT_VOID InsertGroup(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_CreateGroup* pGroupInfo);
	CT_VOID AddGroupUser(acl::db_handle* db, MSG_CS2DB_Add_GroupUser* pAddUser);
	CT_VOID QueryBeAddGroupUser(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_Query_BeAdd_GroupUser* pBeAddGroupUser);
	CT_VOID RemoveGroupUser(acl::db_handle* db, MSG_CS2DB_Remove_GroupUser* pRemoveUser);
	CT_VOID RemoveGroup(acl::db_handle* db, MSG_CS2DB_Remove_Group* pRemoveGroup);
	CT_VOID	RemoveGroupRoomCfg(acl::db_handle* db, MSG_CS2DB_Remove_Group* pRemoveGroup);
	CT_VOID	ModifyGroupInfo(acl::db_handle* db, MSG_CS2DB_Modify_GroupInfo* pModifyInfo);
	CT_VOID AddGroupContribution(acl::db_handle* db, MSG_CS2DB_Update_GroupContribution* pAddContribution);
	CT_VOID	DeductGroupMasterGem(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_Deduct_GroupMaster_Gem* pDeductGem);
	CT_VOID ChangeGroupMemberPosition(MSG_CS2DB_GroupPosition* pGroupPosition);
	CT_VOID UpdateGroupUserInfo(MSG_CS2DB_Update_GroupUserInfo* groupUserInfo);
	CT_VOID InsertGroupChatMsg(MSG_CS2DB_Insert_GroupChat* pGroupChat);
	CT_VOID UpdateGroupChatMsgStatus(MSG_CS2DB_Update_GroupChat* pGroupChatStatus);
	CT_VOID UpdateGroupDirIncome(MSG_CS2DB_Update_GroupIncome* pGroupIncome, CT_DWORD dwCount);
	CT_VOID UpdateGroupSubIncome(MSG_CS2DB_Update_GroupIncome* pGroupIncome, CT_DWORD dwCount);
    inline  std::string GetTimeName();
	CT_VOID InsertGroupIncomeExchange(MSG_CS2DB_Insert_Exchange_GroupIncome* pExchangeIncome);
	CT_VOID QueryGroupIncomeExchange(acl::aio_socket_stream* pSocket, MSG_CS2DB_Query_GroupIncome_Exchange* pQueryExchange);

	//微信群组相关
	CT_VOID QueryWXGroupInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db);
	CT_VOID ReturnWXGroupPRoomGem(acl::db_handle* db, MSG_G2DB_Return_WXGroupPRoom_Gem* pReturnGem);
	CT_VOID InsertWXGroupPRoomRecord(acl::db_handle* db, MSG_G2DB_Record_WXGroupPRoom* pAutoPRoom);

	//玩家金币相关
	CT_VOID UpdateUserScore(acl::db_handle* db, CT_DWORD dwUserID, CT_LONGLONG llAddScore);
	CT_VOID SetUserScore(CT_DWORD dwUserID, CT_LONGLONG llScore);
	CT_VOID UpdateUserBenefitInfo(MSG_G2GB_Update_BenefitInfo* pBenefitInfo);
	CT_VOID UpdateUserScoreInfo(acl::db_handle* db, MSG_Update_ScoreInfo* pScoreInfo); 	//弃用2018-12-15
	CT_VOID UpdateUserScoreData(MSG_G2DB_ScoreData* pScoreData);						//弃用2018-12-15
	CT_VOID QueryUserScoreInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS_Query_UserScore_Info* pUserScoreInfo);
	CT_VOID UpdateUserShowExchange(CT_DWORD dwUserID, CT_BYTE cbShowExchange);

	//红包相关
	CT_VOID LoadHongBaoInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db);
	CT_VOID InsertHongbaoInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS2DB_Insert_HongBao* pHongBaoInfo);
	CT_VOID DeleteHongbaoInfo(acl::db_handle* db, MSG_CS2DB_Delete_HongBao* pDeleteHongBao);
	CT_VOID UpdateHongbaoInfo(acl::db_handle* db, MSG_CS2DB_Update_HongBao* pHongBaoInfo);
	CT_VOID	QueryUserHongBaoInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db, MSG_CS_Query_UserHongBao_Info* pUserScoreInfo);

	//任务相关
	CT_VOID LoadTaskInfo(acl::aio_socket_stream* pSocket, acl::db_handle* db);
	CT_VOID InsertTaskInfo(acl::db_handle* db, MSG_CS2DB_Insert_Task* pTask);
	CT_VOID UpdateTaskInfo(acl::db_handle* db, MSG_CS2DB_Update_Task* pTask);
	CT_VOID ResetDailyTask();

	//活动相关(财神降临)
	CT_VOID LoadWealthGodComingCfg();
	CT_VOID LoadWealthGodComingRewardCfg();
	CT_VOID LoadWealthGodComingIndex();
	CT_VOID LoadWealthGodComingHistoryData();
	//CT_VOID LoadWealthGodComingHistoryData();
	
	CT_VOID UpdateWealthGodComingIndex(tagWealthGodComingLastIndex* pIndex);
	CT_VOID InsertWealthGodComingData(MSG_CS2DB_Insert_WGC_Data* pData);
	CT_VOID UpdateWealthGodComingData(MSG_CS2DB_Update_WGC_Data* pData);
	CT_VOID UpdateWealthGodComingUserInfo(MSG_CS2DB_Update_WGC_UserInfo* pUserInfo);

	//新年红包
	CT_VOID LoadRedPacketStatus();
	CT_VOID LoadRedPacketCfg();
	CT_VOID LoadRedPacketIndex();
	CT_VOID UpdateRedPacketStage(MSG_CS2DB_Update_RedPacket_Stage* pRedPacketStage);
	CT_VOID InsertRedPacketGrab(MSG_CS2DB_Insert_RadPacket_GrabInfo* pRedPacketGrabInfo);

	//一元夺宝相关
	CT_VOID LoadDuoBaoCfg();
	CT_VOID LoadDuoBaoDoingData();
	CT_VOID	LoadDuoBaoHistoryData(CT_WORD wID);
	CT_VOID UpdateDuoBaoData(MSG_CS2DB_Update_DuoBao_Data* pDuoBaoInfo);
	CT_VOID InsertDuoBaoUser(MSG_CS2DB_Insert_DuoBao_User* pDuoBaoUser);

	//系统相关
	CT_VOID LoadSystemStatus();
	CT_VOID SendSystemStatusToCenter();

	//排行榜
	CT_VOID LoadScoreRank();
	CT_VOID	LoadScoreRank(acl::db_handle* db);
	CT_VOID	SwitchTodayScoreRank();
	CT_VOID SwitchTodayScoreRank(acl::db_handle* db);
	CT_VOID	ResetTodayScoreRankInCache();
	CT_VOID	LoadTodayScoreRank();
	CT_VOID LoadTodayScoreRank(acl::db_handle* db);
	CT_VOID SendTodayScoreRankToCenter(std::vector<tagScoreRank>& vecScoreRank);
	//CT_VOID LoadShareScoreRank();
	//CT_VOID SendShareScoreRank(std::vector<tagShareRank>& vecShareRank);
	CT_VOID LoadTodayOnlineRank();
	CT_VOID LoadTodayTaskFinishRank();
	CT_VOID ResetTodayRank();	//包括今日在线榜和任务榜
	CT_VOID ResetTodayTaskFinishRank();

	//CT_VOID LoadSignInRank();
	CT_VOID InsertTodayEarnScore(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount);
	CT_VOID UpdateUserRankTask(CT_DWORD dwUserID);

	//最高在线人数
	CT_VOID	LoadPCUInfo();
	CT_VOID UpdatePCU(MSG_CS2DB_Update_PCU* pUpdatePCU);

	//金币场相关
	CT_VOID SaveRecordDrawInfo(acl::db_handle* db, MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount, CT_DWORD& dwDrawID);
	CT_VOID SaveBaiRenScoreInfo(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount, CT_DWORD dwDrawID);
	CT_VOID UpdateWinLostCount(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount);
	CT_VOID UpddateWinLostCountEx(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount);
	CT_VOID SaveRecordHBSLInfo(MSG_G2DB_Record_HBSL_Info* pRecordHBSLInfo, MSG_G2DB_Record_HBSL_GrabInfo* pGrabInfo, CT_WORD wUserCount);
	CT_VOID UpdateBlackListUser(MSG_GS2DB_BlackList_User* pBlackList);

	CT_VOID UpdateUserTodayOnlineTime(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount);
	CT_VOID SendMail(tagUserMail& mail);
	CT_VOID UpdateAndroidStatus(MSG_G2DB_Android_Status* pStatus);
	CT_VOID UpdateServerStockInfo(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo);
	CT_VOID InsertScoreChangeRecordForPlayGame(MSG_G2DB_Record_Draw_Info* pRecordDrawInfo, MSG_G2DB_Record_Draw_Score *pDrawScore, CT_WORD wDrawScoreCount, CT_DWORD dwDrawID);
	CT_VOID InsertScoreChangeRecord(CT_DWORD dwUserID, CT_LONGLONG llSourceBank, CT_LONGLONG llSourceScore, CT_LONGLONG llAddBank, CT_LONGLONG llAddScore, CT_BYTE cbType);
	CT_VOID InsertTaskRewardRecord(CT_DWORD dwUserID, CT_WORD wTaskID, CT_LONGLONG llReward);
	CT_VOID UpdateUserTodayTaskRewardCount(MSG_CS2DB_Insert_TaskReward* pTaskReward);
	CT_VOID InsertReportInfo(MSG_G2DB_ReportInfo* pReport, CT_WORD wReportCount);
	CT_VOID QueryGameRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_GameRecord* pQueryRecord);
	CT_VOID QueryReportRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Report_GameRecord* pQueryRecord);
	CT_VOID QueryNormalRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Normal_GameRecord* pQueryRecord);
	inline 	CT_BOOL GetGameRecord(acl::db_handle* db, MSG_CS_Query_GameRecord* pQueryRecord, std::vector<MSG_SC_Query_GameRecord>& vecGameRecord, std::string& date, CT_WORD wMaxCount);
    inline 	CT_BOOL GetGameRecord2(acl::db_handle* db, CT_DWORD dwDrawID, CT_DWORD dwQueryUserID, std::vector<MSG_SC_Query_GameRecord>& vecGameRecord, std::string& date, CT_WORD wMaxCount);
    CT_VOID InsertPlaybackRecord2(CT_VOID* pData, CT_DWORD dwDataSize, CT_DWORD dwDrawID);
    CT_VOID QueryHBSLRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Hbsl_Record* pQueryRecord);
	CT_VOID QueryHBSLFaRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Hbsl_Record* pQueryRecord);
	CT_VOID QueryHBSLWinColorPrizeRecord(acl::aio_socket_stream* pSocket, MSG_CS_Query_Hbsl_Record* pQueryRecord);
    inline	CT_VOID UpdateGoodCardInfo(MSG_G2DB_GoodCard_Info* pGoodCardInfo);

	//保存玩家点控数据
	CT_VOID SaveUserDianKongData(MSG_G2DB_User_PointControl *pDianKongData);
	//读取玩家点控数据
	CT_VOID ReadUserDianKongData(MSG_GS2DB_ReadDianKongData * pReadDianKongData, acl::aio_socket_stream* pSocket);
	//读取玩家miss数据
	CT_VOID ReadUserMissInfo(JSFish_Player_Miss_Info *pData, acl::aio_socket_stream *pSocket);
	//删除玩家点控数据
	CT_VOID DelUserDianKongData(MSG_GS2DB_DelUserDianKongData *pDelDianKongData);
	//保存当日总吸分和当日总放分
	CT_VOID SaveTotalWinLoseScore(MSG_GS2DB_FishTotalWinLoseScore *pTotalWinLoseScore);
	//保存极速捕鱼miss库
	CT_VOID SaveMissInfo(JSFish_Player_Miss_Info * pMissInfo);
	//保存玩家捕鱼信息
	CT_VOID SavePlayerFishInfo(MSG_GS2DB_PlayerFishInfo *pPlayerFishInfo);
	//保存鱼种信息
	CT_VOID SaveFishStatisticsInfo(MSG_GS2DB_FishStatisticsInfo *pInfo);
	//保存捕鱼控制数据
	CT_VOID SaveFishControlData(MSG_GS2DB_SaveFishControlData *pControlData);
	//保存当前库存
	CT_VOID SaveFishCurrentKuCun(MSG_GS2DB_SaveFishKuCun *pKuCunData);
	//充值渠道信息(是有哪些充值渠道)
	CT_VOID LoadRechargeChannelInfo(CT_BYTE cbReload);
	//充值渠道的金额配置
	CT_VOID LoadRechargeChannelAmount();
	//代理充值列表(指代理充值渠道的代理QQ,WX列表)
	CT_VOID LoadRechargeQQWXList();
	//全民代理微信号列表
	CT_VOID LoadGeneralizePromoter();
	//兑换渠道状态
	CT_VOID LoadExchangeChannelStatus();
	//兑换渠道的金额配置
	CT_VOID LoadExchangeChannelAmount();
	//加载全民代理信息
	CT_VOID LoadQuanMinChannelInfo();
	//客户端渠道主页
	CT_VOID LoadClientChannelDomain();
	//某渠道是否显示兑换按钮条件
	CT_VOID LoadChannelShowExchangeCond();
	//各个平台的充值配置项
	CT_VOID LoadPlatformRechargeCfg();

	//加载VIP2等级
	CT_VOID LoadVip2Config();
	//加载救济金配置
	CT_VOID LoadBenefitConfig();

	//加载聊天图片上传地址
	CT_VOID LoadChatUploadUrl();
	//加载公告信息
	CT_VOID LoadAnnouncementInfo();

public:
	inline acl::db_pool* GetAccountdbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_accountDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetRecorddbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_recordDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetRecordScoredbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_recordScoreDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetPlaybackdbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_playbackDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetPromotiondbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_promotionDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetGamedbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_gameDBKey.c_str(), false, true);
		return pool;
	}
	inline acl::db_pool* GetPlatformdbPool()
	{
		acl::db_pool* pool = (acl::db_pool*)m_dbManager.get(m_platformDBKey.c_str(), false, true);
		return pool;
	}

private:
	acl::mysql_manager	m_dbManager;

	acl::string			m_accountDBKey;
	acl::string			m_recordDBKey;
	acl::string			m_recordScoreDBKey;
	acl::string			m_playbackDBKey;
	acl::string			m_promotionDBKey;
	acl::string			m_gameDBKey;
	acl::string			m_platformDBKey;

	acl::redis_client	m_redisClient;
	acl::redis			m_redis;

	CT_DWORD			m_dwTodayTime;
	tagSystemStatus		m_systemStatus;
	//CT_BOOL				m_bNeedLoadNewShareRank;

	CT_BOOL				m_bInitServer;
};