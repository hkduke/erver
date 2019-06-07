#pragma once
#include "WorkThread.h"
#include "acl_cpp/lib_acl.hpp"
#include "CTType.h"
#include "ServerMgr.h"
#include "GroupMgr.h"
#include "WXGroupMgr.h"
#include "HongBaoMgr.h"
#include "RankMgr.h"
#include "GlobalStruct.h"
#include <map>
#include <set>
#include <memory>
#include <iostream>

/*struct tagGroupPRoomInfo
{
	CT_WORD		wGameID;
	CT_WORD		wKindID;
	CT_DWORD	dwRoomNum;
	CT_CHAR		szParam[PROOM_JSON_LEN];

	tagGroupPRoomInfo()
		:wGameID(0)
		,wKindID(0)
		,dwRoomNum(0)
	{
		memset(szParam, 0, sizeof(szParam));
		//std::cout << "------------------------construct tagGroupPRoomInfo-----------------------" << std::endl;
	}

	tagGroupPRoomInfofo()
	{
		//std::cout << "------------------------destruct tagGroupPRoomInfo-----------------------" << std::endl;
	}
};*/

struct stPRoomInfo
{
	CT_DWORD	dwServerID;
	CT_DWORD	dwGroupID;
	CT_DWORD	dwRecordID;
	CT_DWORD	dwMasterID;
	CT_DWORD	dwCreateTime;
	CT_WORD		wPlayCount;
	CT_BYTE		cbGroupType;
	CT_WORD		wGameID;
	CT_WORD		wKindID;
	CT_BOOL		bStart;
};

//typedef std::map<CT_WORD, std::set<CT_DWORD>>  MapGameHallUser;

//<--platformId, <--proxyserverId, --userid_count>>
typedef std::map<CT_BYTE, std::map<CT_DWORD, CT_DWORD>> MapPlatformProxyUser;
typedef std::map<CT_DWORD, CT_DWORD> MapProxyUser;

//<--platformId, <--MachineType, userid_count>>
typedef std::map<CT_BYTE, std::map<CT_BYTE, CT_DWORD>> MapPlatformMachineUser;
typedef std::map<CT_BYTE, CT_DWORD> MapMachineUser;

typedef std::map<CT_BYTE, std::map<CT_DWORD, CT_DWORD >> MapPlatformServerUser;
typedef std::map<CT_DWORD, CT_DWORD > MapServerUser;

typedef std::map<CT_DWORD, stPRoomInfo> MapPRoomInfo;
//typedef std::map<CT_DWORD, std::set<std::shared_ptr<tagGroupPRoomInfo>>> MapRoomNum;

typedef std::map<CT_BYTE, tagPcuInfo> MapPcuInfo;

class CCenterThread : public CWorkThread
{
public:
	CCenterThread(void);
	~CCenterThread(void);

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
	virtual CT_VOID OnNetEventRead(acl::aio_socket_stream* pSocket, CMD_Command* pMc, const CT_VOID* pData, CT_DWORD wLen);
	//网络定时器
	virtual CT_VOID OnTimer(CT_DWORD dwTimerID);

	//打开
	virtual CT_VOID Open();

public:
	CT_VOID AcrossTheDayEvent();
	inline CT_VOID CheckUserOffLine();

public:
	//代理服消息
	CT_VOID OnProxyServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//游戏服消息
	CT_VOID OnGameServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//登录服消息
	CT_VOID OnLoginServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//客服服消息
	CT_VOID OnCustomerServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//DIP服消息
	CT_VOID OnDipServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//维服工具
	CT_VOID OnMaintainMsg(CT_WORD wSubCmdID, const CT_VOID* pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//db服消息
	CT_VOID OnDBServerMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

	//群组消息（由proxy转发）
	CT_VOID OnGroupMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//红包消息
	CT_VOID OnHongBaoMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//任务消息
	CT_VOID OnTaskMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//排行榜
	CT_VOID OnRankMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//充值提醒消息
	CT_VOID OnConfigOptionMsg(CT_WORD wSubCmdID, const CT_VOID *pData, CT_DWORD wDataSize, acl::aio_socket_stream *pSocket);
	//财神降临活动消息
	CT_VOID OnWealthGodComingMsg(CT_WORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);

	//俱乐部的消息
private:
    CT_VOID OnGroupSubCreateGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubApplyAddGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubOpApplyAddGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQuitGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubTickUser(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubCreatePRoom(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubDimissGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
    CT_VOID OnGroupSubModifyGroupName(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubModifyGroupNotice(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubModifyGroupIcon(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubUpgradeGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubModifyGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubModifyGroupUserRemarks(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubUpgradeGroupUserIncomeRate(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryDirGroupUserList(const CT_VOID *pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubChat(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupQueryGroupDirInfo(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupQueryGroupSubInfoList(const CT_VOID *pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryGroupIncome(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryGroupYesterDayIncome(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryGroupIncomeDetail(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryGroupSettleRecord(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryGroupSettleInfo(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubSettleIncome(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubCreatPRoom(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubOpenGameUI(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubCloseGameUI(const CT_VOID * pData, CT_DWORD wDataSize);

    CT_VOID OnLoginServerAddGroupUser(MSG_L2CS_AddGroupUser* addGroupUser);

    CT_VOID OnGroupSubPullUserIntoGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubRecommendAddGroup(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnGroupSubQueryUserInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
    CT_VOID OnGroupSubChangePosition(const CT_VOID * pData, CT_DWORD wDataSize);

private:
	//查询游戏服务器等待用户数量
	CT_VOID OnSubDip2csQueryWaitListUser(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//获取ServerID等待玩家数量
	CT_DWORD GetWaitListUserByServerId(CT_DWORD dwServerID, CT_WORD wGameID, CT_WORD wRoomKindID);
	//查询指定游戏服务器桌子信息数据
	CT_VOID OnSubDip2csQueryTableMsg(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//重新更新机器人
	CT_VOID OnSubUpdateAndroid(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//更新机器人同时上桌人数
	CT_VOID OnSubUpdateAndroidpPlayCount(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//更新服务器的库存信息
	CT_VOID OnSubUpdateServerStockInfo(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//剔除捕鱼玩家
	CT_VOID OnSubDipTickFishUser(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//百人游戏概率控制
	CT_VOID OnSubBRGameRationControl(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//百人游戏黑名单
	CT_VOID OnSubBRGameBlackList(const CT_VOID * pData, CT_DWORD wDataSize, acl::aio_socket_stream* pSocket);
	//更新或者插入财神降临活动内容
	CT_VOID OnSubDipUpdateWealthGodComing(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新或者插入财神降临活动的奖励
	CT_VOID OnSubDipUpdateWealthGodComingRank(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新夺宝配置
	CT_VOID OnSubDipUpdateDuoBaoCfg(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新救济金配置
	CT_VOID OnSubDipUpdateBenefitCfg(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新平台充值配置信息
	CT_VOID OnSubDipUpdatePlatformRechargeCfg(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新短信平台的ID
	CT_VOID OnSubDipUpdateSMSPid(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新新年红包的状态
	CT_VOID OnSubDipUpdateRedPacketStatus(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新红包的房间配置
	CT_VOID OnSubDipUpdateRedPacketRoomCfg(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新红包的期数配置
	CT_VOID OnSubDipUpdateRedPacketIndex(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新聊天上传图片
	CT_VOID OnSubDipUpdateChatUploadUrl(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新公告类型
	CT_VOID OnSubDipUpdateAnnouncement(const CT_VOID * pData, CT_DWORD wDataSize);
	//更新黑渠道信息
	CT_VOID OnSubDipUpdateBlackChannel(const CT_VOID * pData, CT_DWORD wDataSize);

	CT_VOID OnSubDipUpdateRechargePromoterList(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateExchangeChannelStatus(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateUserStatus(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateGeneralizePromoter(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateClientChannelDomain(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateQuanMinChannel(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateChannelPresentScore(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateRechargeAmount(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubUpdateExchangeAmount(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnDipSubSetFishDianKong(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubDipUpdateZjhCardRatio(const CT_VOID * pData, CT_DWORD wDataSize);

	//DB返回的消息
	CT_VOID OnSubLoadGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadGroupUser(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadGroupUserRevenue(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadGroupUserDirIncome(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnSubLoadGroupUserSubIncome(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnSubLoadGroupUserTotalIncome(const CT_VOID * pData, CT_DWORD wDataSize);
    CT_VOID OnSubLoadGroupChatMsgId(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadGroupUnreadChatMsg(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubQueryGroupExchange(const CT_VOID * pData, CT_DWORD wDataSize);

	CT_VOID OnSubLoadQueryBeAddGroupUser(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadDefaultPRoomConfig(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadPRoomConfig(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadGroupInfoFinish(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubDBCreateGroup(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubDBDeductGroupMaterGem(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubDBGroupRoomInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubDBWXGroupInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubDBWxGroupFinish(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadGroupLevelCfg(const CT_VOID * pData, CT_DWORD wDataSize);

	CT_VOID OnSubLoadHongBaoInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubInsertHongBao(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadScoreRank(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadYesterdayScoreRank(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadTodayOnlineRank(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadTaskRankFinish(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadSigninRank(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadShareRank(const CT_VOID * pData, CT_DWORD);
	CT_VOID OnSubLoadTaskInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadPCUInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadRechargeChannelInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadRechargePromoterInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadExchangeChannelStatus(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadGeneralizePromoterInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadClientChannelDomain(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadQuanMinChannelInfo(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadVip2Config(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadChannelShowExchange(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadRechargeChannelAmount(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadExchangeChannelAmount(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadBenefitConfig(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID	OnSubLoadPlatformRechargeCfg(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadWealthGodConfig(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadWealthGodReward(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadWealthGodIndex(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadWealthGodHistoryData(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadWealthGodHistoryUser(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadDuoBaoConfig(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadDuoBaoDoingData(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadDuoBaoDoingUser(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadDuoBaoHistoryData(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadDuoBaoHistoryUser(const CT_VOID * pData, CT_DWORD wDataSize);
	CT_VOID	OnSubLoadRedPacketStatus(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadRedPacketRoomCfg(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadRedPacketIndex(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadRedPacketGrabInfo(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadChatUploadUrl(const CT_VOID* pData, CT_DWORD wDataSize);
	CT_VOID OnSubLoadAnnouncementInfo(const CT_VOID* pData, CT_DWORD wDataSize);

private:
	CT_BOOL	 CheckRoomNumIsExist(CT_DWORD dwRoomNum);
	CT_BOOL	 GetPRoomInfo(CT_DWORD dwRoomNum, PrivateRoomInfo& roomInfo);
	CT_DWORD CreateRoomNum(CT_DWORD dwServerID);
	CT_BOOL	 SetRoomNumToCache(CT_DWORD dwUserID, CT_DWORD dwServerID, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wUserCount, \
								CT_WORD wPlayCount,/* CT_WORD wWinLimit, CT_WORD wEndLimit,*/ CT_BYTE bIsVip, CT_CHAR* pOtherParam, \
								CT_DWORD dwRoomNum, /*CT_BYTE bIsAuto = 0,*/ CT_DWORD dwGroupID = 0, CT_DWORD dwRecordID = 0, CT_BYTE cbGroupType = 0);
	CT_BOOL	 CheckUserHasRoom(CT_DWORD dwUserID);
	CT_DWORD GetUserGem(CT_DWORD dwUserID);
	CT_BOOL	 AddUserGemToCache(CT_DWORD dwUserID, int iAddGem, long long& dwGemResult, CT_BYTE cbType = 0);
	CT_VOID	 UpdateUserGem(CT_DWORD dwUserID, int iAddGem, long long& dwGemResult, CT_BYTE cbType = 0);
	CT_BOOL	 AddUserScoreToCache(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_DWORD dwTotalCash,  CT_LONGLONG& llScoreResult, CT_LONGLONG& llRechargeTempCount); 	//充值专用
	CT_BOOL  UpdateUserScore(CT_DWORD dwUserID, CT_LONGLONG llAddScore, CT_DWORD dwTotalCash);
	CT_BOOL  AddUserBankScoreToCache(CT_DWORD dwUserID, long long llAddBankScore, long long& llBankResult);	//充值专用
	CT_VOID  UpdateUserBankScore(CT_DWORD dwUserID, long long llAddBankScore, long long& llBankResult);
	CT_VOID  SetGroupPRoomRecordID(CT_DWORD dwRoomNum, CT_DWORD dwRecordID);
	CT_VOID	 SetUserPromoterID(CT_DWORD dwUserID, CT_DWORD dwPromoterID);

	//inline CT_VOID  UpdateServerUser(CT_BYTE cbPlatformId, CT_WORD wGameID, CT_WORD wKindID, CT_WORD wRoomKindID);
	//inline CT_VOID  UpdateHallUser(CT_BYTE cbPlatformId, CT_DWORD dwProxyServerID, CT_BYTE cbMachineType, CT_BOOL bUpdatePcu);
	//inline CT_VOID  IncreaseHallUser(CT_BYTE cbPlatformId, CT_DWORD dwProxyServerID, CT_BYTE cbMachineType, CT_DWORD dwUserID);
	//inline CT_VOID  DecreaseHallUser(CT_BYTE cbPlatformId, CT_DWORD dwProxyServerID, CT_DWORD dwUserID);
	inline CT_VOID  CalcOnlineUser();

	CT_VOID  SetPCUInfoToCache(tagPcuInfo& pcuInfo);
	CT_VOID  AcrossTheDayUpdatePCU();

	inline CT_VOID  UpdatePCUToDB(CT_BYTE cbPlatformId, CT_BYTE cbPCUType, CT_DWORD dwPCU);

	CT_DWORD GetUserGameServerID(CT_DWORD dwUserID);
	CT_DWORD GetUserRoomNum(CT_DWORD dwUserID);
	CT_VOID  GetUserProxyServerID(CT_DWORD dwUserID, CT_DWORD& dwProxyServerID, CT_UINT64& uClientAddr);
	CT_VOID  RemoveUserPRoomNum(CT_DWORD dwUserID);
	CT_VOID  GetUserHeadUrl(CT_DWORD dwUserID, CT_CHAR* szHeadUrl);
	CT_VOID  GetUserNickName(CT_DWORD dwUserID, CT_CHAR* szNickName);
	CT_BOOL  GetAccountInfo(CT_DWORD dwUserID, UserAccountsBaseData& accountData);

	CT_VOID	 AddPRoomNum(CT_DWORD dwGroupID, std::shared_ptr<tagGroupPRoomInfo> pRoomInfo);
	CT_VOID  RemovePRoomNum(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID, CT_DWORD dwRoomNum);
	CT_VOID	 RemoveTimeOutPRoom();
	CT_BOOL	 FindPRoomInfo(CT_DWORD dwGroupID, CT_WORD wGameID, CT_WORD wKindID, /*CT_CHAR szParam[PROOM_JSON_LEN],*/ CT_DWORD& dwRoomNum, CT_WORD& userCount);
	CT_BOOL	 CreatePRoomForGroup(const CT_CHAR* pJson, CT_DWORD dwGroupID, CT_DWORD& dwRoomNum, CT_WORD& wCurrUserCount);

private:
	CT_VOID	GoDBServerRegister();
	CT_VOID LoadAllGroupInfo();
	CT_VOID LoadHongBaoInfo();
	CT_VOID LoadTaskInfo();

private:
	//MapGameHallUser		m_mapHallUser;
	//在线人数统计相关
	MapPlatformProxyUser	m_mapPlatformProxyUser;
    MapPlatformMachineUser	m_mapPlatformMachineUser;
    MapPlatformServerUser   m_mapPlatformServerUser;
	CWXGroupMgr				m_wxGroupMgr;
	CT_DWORD				m_updateUserCountTick;
	//CT_DWORD				m_updateServerUserTick;
	MapPcuInfo				m_mapPcuInfo;

	//私人房间
	MapGroupPRoomNum		m_mapWxRoomNum;
	MapPRoomInfo			m_mapPRoomInfo;

	//redis
	acl::redis_client		m_redisClient;
	acl::redis				m_redis;

	//时间相关
	CT_BOOL					m_bInitServer;
	CT_INT64				m_iTodayTime;
	CT_INT64				m_iWeekTime;

	tagSystemStatus			m_SystemStatus;
	bool					m_bRechargeJudge;
};