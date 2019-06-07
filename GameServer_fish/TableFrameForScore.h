#pragma once
#include "ITableFrameSink.h"
#include "ITableFrame.h"
#include "IGameOtherData.h"
#include "IServerUserItem.h"
#include "GlobalStruct.h"
#include "GlobalEnum.h"
#include "ServerUserItem.h"
#include "ReadWriteBuffer.h"
#include "TableFrame.h"


class CTableFrameForScore : public ITableFrame
{
public:
	CTableFrameForScore();
	virtual ~CTableFrameForScore();

public:
	virtual void Init(ITableFrameSink* pSink, const TableState& state, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis);
	void Clear();

public:
	//发送数据
	//virtual CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID);
	//发送数据
	virtual CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = false);

public:
	//设置定时器
	virtual CT_BOOL SetGameTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam = 0);
	//销毁定时器
	virtual CT_BOOL	KillGameTimer(CT_WORD wTimerID);
	//判断是否有定时器
	virtual CT_BOOL	IsHasGameTimer(CT_WORD wTimerID);

public:
	//获得桌子信息
	virtual void GetTableInfo(TableState& TableInfo);
	//获取桌子ID
	virtual CT_DWORD GetTableID();
	//获得游戏配置数据
	virtual void GetGameCfgData(GameCfgData& CfgData);
	//处理输赢积分
	virtual CT_BOOL WriteUserScore(CT_DWORD dwChairID, const ScoreInfo& ScoreData);
	
	//解散游戏
	virtual CT_BOOL DismissGame(CT_BYTE cbDismissType);
	//结束游戏
	virtual CT_BOOL ConcludeGame(CT_BYTE cbGameStatus, const CT_CHAR* pDrawInfo);
	//计算税收
	virtual CT_LONGLONG CalculateRevenue(CT_DWORD dwChairID, CT_LONGLONG llScore);
	//获取私人房信息
	virtual void GetPrivateTableInfo(PrivateTableInfo& privateTableInfo);

public:
	//用户是否存在
	virtual CT_BOOL IsExistUser(CT_DWORD dwChairID);
	//是否私人场 
	virtual CT_BOOL IsPrivateRoom();
	//设置大局阶段
	virtual CT_VOID SetGameRoundPhase(enGameRoundPhase gameRoundPhase);
	//获取大局阶段
	virtual enGameRoundPhase GetGameRoundPhase() { return m_GameRoundPhase; }
	//获取私人房间房主椅子id
	virtual CT_WORD GetPRoomOwnerChairID();
	//删除玩家的连接
	virtual CT_VOID CloseUserConnect(CT_DWORD dwChairID);

public:
	//设置玩家托管状态
	virtual CT_VOID SetUserTrustee(CT_DWORD dwChairID, CT_BOOL bTrustee);
	//获得玩家托管状态
	virtual CT_BOOL GetUserTrustee(CT_DWORD dwChairID);

public:
	//获得玩家金币
	virtual CT_LONGLONG GetUserScore(CT_DWORD dwChairID);
	//设置玩家金币
	virtual CT_VOID AddUserScore(CT_DWORD dwChairID, CT_LONGLONG llAddScore, CT_BOOL bBroadcast = true);
	//设置玩家状态
	CT_VOID SetUserStatus(CT_DWORD dwChairID, USER_STATUS status);
	//玩家状态
	virtual CT_BYTE GetUserStatus(CT_DWORD dwChairID);
	//获得玩家昵称
	virtual char* GetNickName(CT_DWORD dwChairID);
	//获得玩家头像
	virtual char* GetHeadUrl(CT_DWORD dwChairID);
	//获取玩家头像索引
	virtual CT_BYTE GetHeadIndex(CT_DWORD dwChairID);
	//获得玩家VIP
	virtual CT_BYTE GetVipLevel(CT_DWORD dwChairID);
	// 获取玩家的性别
	CT_BYTE GetUserSex(CT_DWORD dwChairID);
	//获取桌子上的用户ID
	virtual CT_DWORD GetUserID(CT_DWORD dwChairID);
	//获得当前局数
	virtual CT_WORD GetCurrPlayCount();
	//设置玩家准备状态
	virtual CT_VOID SetUserReady(CT_DWORD dwChairID);
	//玩家离开
	virtual void OnUserLeft(CServerUserItem* pUser, bool bSendStateMyself, bool bForceLeave);
	//玩家离线
	virtual void OnUserOffLine(CServerUserItem* pUser);
	//获取玩家的帐号基础信息
	virtual GS_UserBaseData* GetUserBaseData(CT_DWORD dwChairID) { return NULL; }
    //获取玩家的金币信息
    virtual GS_UserScoreData* GetUserScoreData(CT_DWORD dwChairID) { return NULL; }
	//发送邮件
	virtual CT_VOID SendUserMail(tagUserMail* pUserMail) {}
	//获取玩家椅子号(返回INVALID_CHAIR, 说明不在线)
	virtual CT_WORD GetUserChairID(CT_DWORD dwUserID) { return  INVALID_CHAIR; }

public:
	//清理桌子上的用户
	virtual CT_VOID ClearTableUser(CT_DWORD wChairID = INVALID_CHAIR, CT_BOOL bSendState = false, CT_BOOL bSendStateMyself = false, CT_BYTE cbSendErrorCode = 0);

	virtual CT_VOID UserVoiceChat(CServerUserItem* pUser, CT_CHAR* pChatUrl);
	virtual CT_VOID UserTextChat(CServerUserItem* pUser, CT_BYTE cbType, CT_BYTE cbIndex);
	virtual CT_VOID GameRoundStartDeploy();
	virtual CT_VOID OnUserEnterAction(CServerUserItem* pUserItem, CT_BOOL bDistribute = false, CT_BOOL bReConnect = false);
	virtual CT_BOOL UserStandUp(CServerUserItem* pUserItem, CT_BOOL bSendState = true, CT_BOOL bSendStateMyself = false);
	//记录金币场基本信息
	virtual CT_BOOL RecordRawInfo(RecordScoreInfo* pRecordScoreInfo, CT_WORD wUserCount, CT_BYTE* pAreaCardType, CT_BYTE cbAreaCount,CT_DWORD dwBankerUserID,\
								  CT_LONGLONG llStock, CT_LONGLONG llAndroidStock, CT_LONGLONG llTodayStock, CT_WORD wSystemAllKillRation, CT_WORD wChangeCardRatio);
	virtual CT_BOOL RecordHongBaoSLInfo(RecordHBSLInfo* pHbInfo, RecordHBSLGrabInfo* pGrabInfo, CT_WORD wUserCount) { return false;}
	//写回放的空闲场景
	virtual CT_BOOL WriteGameSceneToRecord(CT_VOID* pData, CT_DWORD dwDataSize, CT_WORD wSubMsgID) { return true; }
	//获取桌子回调类
	virtual ITableFrameSink* GetTableFrameSink() { return m_pTableFrameSink; }
	//玩家每发一炮就通知系统赢了多少分
	virtual CT_VOID NotifySysWinScore(CT_LONGLONG llSysWinScore) {}
	//玩家每次得分就通知系统输了多少分]
	virtual CT_VOID NotifySysLoseScore(CT_LONGLONG llSysLoseScore) {}
	//获取捕鱼库存值
	virtual CT_LONGLONG GetFishKuCun() { return  0; }
	//点控玩家输钱的时候,玩家输分了
	virtual CT_VOID NotifySysWinScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llWinScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose){}
	//点控玩家赢钱的时候,玩家赢分了
	virtual CT_VOID NotifySysLoseScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llLoseScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose) {}
	//保存点控玩家数据
	virtual  CT_VOID SavePlayerDianKongData(MSG_G2DB_User_PointControl &data) {}
	//删除玩家点控数据
	virtual CT_VOID DelPlayerDianKongData(MSG_GS2DB_DelUserDianKongData &data) {}
	//保存鱼种的统计信息
	virtual CT_VOID SaveFishStatisticsInfo(FishStatisticsInfo info[], int arrSize) {}
	//捕鱼统计信息:用于测试
	virtual CT_VOID TongJi(tagTongJiInfo &info) {}
	//捕鱼统计单个玩家的输赢信息:用于测试
	virtual CT_VOID SinglePlayerMostWinLostScore(CT_DWORD dwUserID, CT_LONGLONG addScore) {}
	//玩家点控开始
	virtual CT_VOID UserDianKongStart(CT_DWORD dwUserID) {}
	//玩家点控结束
	virtual CT_VOID UserDianKongEnd(CT_DWORD dwUserID) {}
	//保存血池信息(新)
	virtual CT_VOID SaveStockControlInfo(tagStockControlInfo* pStockControlInfo) {}
	//获取捕鱼系统税收
	virtual CT_DWORD GetFishTaxPro() { return  0; }
	//设置捕鱼的炮数
	virtual CT_VOID AddFishFireCount(CT_WORD wChairID, CT_DWORD dwCount) {}
	virtual CT_VOID SetBlackListUser(tagBrGameBlackList* pBlackListUser){}
	virtual CT_VOID SaveJSFishInfo(js_fish_info_type info_type, js_fish_save_info &info){}
public:
	CT_BOOL RoomSitChair(CServerUserItem* pUserItem, CT_BOOL bDistribute = false);
public:
	//获取游戏玩家
	CServerUserItem* GetTableUserItem(CT_WORD wChairID);
	//获取游戏玩家
	CServerUserItem* GetTableUserItemByUserID(CT_DWORD dwUserID);

public:
	//时间脉冲
	void OnTimePulse();
	//游戏定时器计时
	void GameTimerMsg();

public:
	//发送游戏消息
	//CT_VOID SendGameMsg(CServerUserItem* pUser, CT_WORD wSubCmdID);
	//发送游戏消息
	CT_VOID SendGameMsg(CServerUserItem* pUser, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen);

public:
	//发送用户信息
	CT_VOID SendUserMsg(CServerUserItem* pUser, MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen);
	//发送桌子数据
	CT_VOID SendAllUserData(MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen);

	//广播用户信息
	CT_VOID BroadcastUserInfo(CServerUserItem* pUser);
	//广播用户状态
	virtual CT_VOID BroadcastUserState(CServerUserItem* pUser, CT_BOOL bSendStateMyself = false);
	//广播用户金币
	CT_VOID BroadcastUserScore(CServerUserItem* pUser);
	//用户金币广播用户
	CT_VOID BroadcastUserForUser(CServerUserItem* pFromUser, CServerUserItem* pToUser);
	//发送别人的信息给用户
	CT_VOID SendOtherUserDataToUser(CServerUserItem* pUser);

public:
	CT_VOID	SetTableLock(CT_BOOL bLock) { m_TableState.bIsLock = bLock; }
	CT_BOOL GetTableisLock() { return m_TableState.bIsLock; }

public:
	//游戏消息
	void OnGameEvent(CT_WORD wChairID, CT_DWORD dwSubID, const CT_VOID* pData, CT_WORD wDataSize);

public:
	//玩家坐下完成
	void SendUserSitdownFinish(CServerUserItem* pUser, CT_BOOL bDistribute = false);
	//到proxy增加这个用户
	void AddGameUserToProxy(CServerUserItem* pUser);
	//到proxy删除这个玩家
	void DelGameUserToProxy(CServerUserItem* pUser);

	CT_VOID SetUserOnlineInfo(CT_DWORD dwUserID);
	CT_VOID DelUserOnlineInfo(CT_DWORD dwUserID);

public:
	CT_WORD	GetRealPlayerCount();
	CT_WORD	GetAndroidPlayerCount();
	CT_WORD GetPlayerCount();
	CT_WORD GetMaxPlayerCount();
	
public:
	virtual CT_BOOL	CheckGameStart();
	virtual CT_VOID GameStart();

	virtual CT_BOOL IsGameStart() { return m_bGameStart; }
	//判断是否是机器人
	virtual CT_BOOL IsAndroidUser(CT_DWORD dwChairID);
public:
	//获得玩法金币限制
	virtual CT_LONGLONG GetUserEnterMinScore();
	CT_LONGLONG GetUserEnterMaxScore();
	//获取基础底分
	virtual CT_DWORD GetGameCellScore();
	//获取房间配置
	virtual tagGameRoomKind* GetGameKindInfo();
	CT_LONGLONG GetRevenue();
	
public:
	//更新金币
	CT_VOID AddUserScore(CServerUserItem* pUserItem, CT_LONGLONG llScore);
	//更新玩家金币
	inline CT_VOID UpdateUserScoreToDB(CT_DWORD dwUserID, CT_LONGLONG llAddScore);
	inline CT_VOID UpdateUserScoreToCenterServer(CT_DWORD dwUserID, CT_LONGLONG llSourceScore, CT_LONGLONG llAddScore, enScoreChangeType enType);
	//更新玩家数据
	CT_VOID UpdateUserScoreInfo(CServerUserItem* pUserItem, const ScoreInfo& ScoreData);

	//广播玩家游戏赢分信息
	CT_VOID SendUserWinScoreInfo(CServerUserItem* pUserItem, CT_LONGLONG llWinScore);

	//发送DB信息
	inline CT_VOID SendDBMsg(CT_WORD mainId, CT_WORD subId, CT_VOID* pMsg, CT_WORD wLen);
	//发送中心服信息
	inline CT_VOID SendCenterMsg(CT_WORD mainId, CT_WORD subId, CT_VOID* pMsg, CT_WORD wLen);

	//功能函数
public:
	CT_VOID	GetBenefitReward(CServerUserItem* pUserItem);
	CT_BOOL IsLetAndroidOffline(CServerUserItem* pUserItem);
	CT_VOID AddEarnScoreInfo(CServerUserItem* pUserItem, CT_LONGLONG llAddEarnScore);

private:
	//定时器列表
	TimerInfo	            	m_SysTimer[DW_SYS_TIMER_NUMS];

private:
	ITableFrameSink*	m_pTableFrameSink;
	TableState			m_TableState;
	PrivateTableInfo	m_PrivateTableInfo;

	tagGameKind*		m_pGameKindInfo;
	tagGameRoomKind*	m_pRoomKindInfo;

	CServerUserItem**	m_UserList;
	CT_BOOL				m_bGameStart;
	enGameRoundPhase	m_GameRoundPhase;
	acl::redis*			m_pRedis;
	CT_DWORD			m_dwStartTime;
};