#pragma once
#include "ITableFrameSink.h"
#include "ITableFrame.h"
#include "IGameOtherData.h"
#include "IServerUserItem.h"
#include "GlobalStruct.h"
#include "ServerUserItem.h"
#include "ReadWriteBuffer.h"


//系统定时器个数
const CT_DWORD	DW_SYS_TIMER_NUMS = 5;
//定时器结构
struct TimerInfo
{
	CT_WORD			wTimerID;				//定时器ID
	CT_INT32		iTimeInterval;			//时间间隔(以毫秒为单位)
	CT_INT32		iCountTime;				//时间计数(以毫秒为单位)
	CT_BOOL			isEnabled;				//是否有效
	CT_DWORD		dwParam;				//定时器参数
};

#ifdef _DEBUG
#define APPLY_DISMISS_PROOM_TIME			1*60   //1分钟默认解散
#else
#define APPLY_DISMISS_PROOM_TIME			5*60   //5分钟默认解散
#endif



struct stUserProxyInfo
{
	CT_DWORD		dwProxyServerID;
	CT_UINT64		uClinetNet;
};

struct stApplyVipPRoomInfo
{
	CT_BYTE			cbGender;
	CT_BYTE			cbState;					//申请VIP房间的用户0:表示未处理，1:表示同意
	CT_CHAR			szNickname[NICKNAME_LEN];
	CT_CHAR			szApplyTime[TIME_LEN];
	CT_CHAR			szHeadUrl[HEAD_URL_LEN];
};

class CTableFrame : public ITableFrame
{
public:
	CTableFrame();
	virtual ~CTableFrame();

public:
	virtual void Init(ITableFrameSink* pSink, const TableState& state, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis);
	void InitApplyVipRoomData();
	void InitDismissData();
	void Clear();

public:
	//发送数据
	//virtual CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID);
	//发送数据
	virtual CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = true);

public:
	//设置定时器
	virtual CT_BOOL SetGameTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam = 0);
	//销毁定时器
	virtual CT_BOOL	KillGameTimer(CT_WORD wTimerID);
	//判断是否有定时器
	virtual CT_BOOL	IsHasGameTimer(CT_WORD wTimerID) { return false; };

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
	virtual CT_VOID SetGameRoundPhase(enGameRoundPhase gameRoundPhase) { m_GameRoundPhase = gameRoundPhase; }
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
	virtual CT_VOID AddUserScore(CT_DWORD dwChairID, CT_LONGLONG llAddScore, CT_BOOL bBroadcast = true);
	//设置玩家状态
	virtual CT_VOID SetUserStatus(CT_DWORD dwChairID, USER_STATUS status);
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
	virtual CT_BYTE GetUserSex(CT_DWORD dwChairID);
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
    virtual GS_UserScoreData* GetUserScoreData(CT_DWORD dwChairID) { return  NULL; }
	//发送邮件
	virtual CT_VOID SendUserMail(tagUserMail* pUserMail) {}
	//获取玩家椅子号(返回INVALID_CHAIR, 说明不在线)
	virtual CT_WORD GetUserChairID(CT_DWORD dwUserID) { return  INVALID_CHAIR; }

public:
	//清理桌子上的用户
	virtual CT_VOID ClearTableUser(CT_DWORD wChairID = INVALID_CHAIR, CT_BOOL bSendState = false, CT_BOOL bSendStateMyself = false, CT_BYTE cbSendErrorCode = 0);
	//判断是否是机器人
	virtual CT_BOOL IsAndroidUser(CT_DWORD dwChairID) { return false; }
	virtual CT_BOOL RecordRawInfo(RecordScoreInfo* pRecordScoreInfo, CT_WORD wUserCount, CT_BYTE* pAreaCardType, CT_BYTE cbAreaCount,CT_DWORD dwBankerUserID,\
								  CT_LONGLONG llStock, CT_LONGLONG llAndroidStock, CT_LONGLONG llTodayStock, CT_WORD wSystemAllKillRation, CT_WORD wChangeCardRatio) { return false; }
	virtual CT_BOOL RecordHongBaoSLInfo(RecordHBSLInfo* pHbInfo, RecordHBSLGrabInfo* pGrabInfo, CT_WORD wUserCount) { return false;}
	//写回放的空闲场景
	virtual CT_BOOL WriteGameSceneToRecord(CT_VOID* pData, CT_DWORD dwDataSize, CT_WORD wSubMsgID) { return true; }
	//获取桌子回调类
	virtual ITableFrameSink* GetTableFrameSink() { return m_pTableFrameSink; }
	//玩家每发一炮就通知系统赢了多少分
	virtual CT_VOID NotifySysWinScore(CT_LONGLONG llSysWinScore) {}
	//玩家每次得分就通知系统输了多少分
	virtual CT_VOID NotifySysLoseScore(CT_LONGLONG llSysLoseScore) {}
	//获取捕鱼库存值
	virtual CT_LONGLONG GetFishKuCun() { return 0; }
	//点控玩家输钱的时候,玩家输分了
	virtual CT_VOID NotifySysWinScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llWinScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose) {}
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
	//获取游戏玩家
	CServerUserItem* GetTableUserItem(CT_WORD wChairID);
	//获取游戏玩家
	CServerUserItem* GetTableUserItemByUserID(CT_DWORD dwUserID);

public:
	//时间脉冲
	virtual void OnTimePulse();
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
	//游戏消息
	void OnGameEvent(CT_WORD wChairID, CT_DWORD dwSubID, const CT_VOID* pData, CT_WORD wDataSize);

public:
	//玩家坐下完成
	void SendUserSitdownFinish(CServerUserItem* pUser);
	//到proxy增加这个用户
	void AddGameUserToProxy(CServerUserItem* pUser);
	//到proxy删除这个玩家
	virtual void DelGameUserToProxy(CServerUserItem* pUser);

public:
	virtual CT_WORD GetPlayerCount();
	virtual CT_BOOL	CheckGameStart();
	virtual CT_VOID GameStart();

	//CT_VOID SetGameRoundPhase(enGameRoundPhase phase) {  m_GameRoundPhase = phase; }
	virtual CT_BOOL IsGameStart() { return m_bGameStart; }

	CT_VOID ApplyDismissPRoom(CServerUserItem* pUserItem);
	CT_VOID OperateDismissPRoom(CServerUserItem* pUserItem, CT_BYTE cbOperateCode);
	CT_VOID	CheckTimeoutDismissPRoom();
	CT_BOOL CheckCanDismissPRoom();
	CT_BOOL CheckCanNotDismissPRoom();
	CT_VOID TickoutUserAndClearPRoomInfo();
	CT_VOID SendDismissPRoomInfo(CServerUserItem* pUser);
	CT_VOID SendTableSameIpTip();

	//第一小局开始时的操作
	virtual CT_VOID GameRoundStartDeploy();

public:
	virtual CT_VOID UserVoiceChat(CServerUserItem* pUser, CT_CHAR* pChatUrl);
	virtual CT_VOID UserTextChat(CServerUserItem* pUser, CT_BYTE cbType, CT_BYTE cbIndex);
	//获得玩法金币限制
	virtual CT_LONGLONG GetUserEnterMinScore() { return 0; };
	//获取基础底分
	virtual CT_DWORD GetGameCellScore() { return 0; };
	//获取房间配置
	tagGameRoomKind* GetGameKindInfo() { return m_pRoomKindInfo; }
public:
	CT_VOID SetPrivateTableInfo(const PrivateTableInfo& privateInfo);
	PrivateTableInfo& GetPrivateTableInfo() { return m_PrivateTableInfo; }
	CT_DWORD GetPRoomOwner() { return m_PrivateTableInfo.dwOwnerUserID; }
	CT_BOOL IsPRoomOwner(CT_DWORD dwUserID) { return dwUserID == m_PrivateTableInfo.dwOwnerUserID; }
	CT_BOOL IsPRoomVipMode() { return m_PrivateTableInfo.cbIsVip == 1 ? true : false; }

	CT_BYTE AddApplyVipPRoom(CT_DWORD dwUserID, const GS_UserBaseData& userBaseData);
	CT_BYTE GetVipPRoomApplyState(CT_DWORD dwUserID);
	CT_VOID SendVipPRoomApplyToOwner(CT_DWORD dwUserID);
	CT_VOID SendAllVipPRoomApplyToOwner();
	CT_VOID RemoveVipPRoomApplyWhenLogout(CT_DWORD dwUserID);

	CT_VOID OwnerOperateApply(CT_DWORD dwOwnerID, CT_DWORD dwUserID, CT_BYTE cbOperate);
	CT_VOID SendVipPRoomApplyOperateToOwner(CT_DWORD dwUserID, CT_BYTE cbOperate);

	CT_VOID UpdatePRoomGameRoundReward();
	CT_VOID SendGameRoundReward(CServerUserItem* pUserItem, CT_DWORD dwGameRoundCount, CT_DWORD dwRewardGem);

public:
	CT_VOID	SetTableLock(CT_BOOL bLock) { m_TableState.bIsLock = bLock; }
	CT_BOOL GetTableisLock() { return m_TableState.bIsLock;  }

public:
	CT_BOOL PrivateRoomSitChair(CServerUserItem* pUserItem);
	virtual CT_BOOL UserStandUp(CServerUserItem* pUserItem, CT_BOOL bSendState = true, CT_BOOL bSendStateMyself = false);
	virtual CT_VOID OnUserEnterAction(CServerUserItem* pUserItem, CT_BOOL bDistribute = false, CT_BOOL bReConnect = false);
	CT_VOID SendForceLeaveTableMsg(CServerUserItem* pUserItem);

//redis相关操作
public:
	CT_VOID SetAllUserOnlineInfo();
	CT_VOID SetUserOnlineInfo(CT_DWORD dwUserID, CT_DWORD dwRoomNum);
	CT_VOID SetOtherUserPRoomNum();
	CT_VOID SetUserPRoomNum(CT_DWORD dwUserID);
	CT_VOID DelUserPRoomNum(CT_DWORD dwUserID);
	CT_VOID DelUserOnlineInfo(CT_DWORD dwUserID);
	CT_VOID IncrPRoomUserCount();
	CT_VOID DecrPRoomUserCount();
	CT_VOID DelPRoomInfo();

	CT_VOID DeductUserGem(CServerUserItem* pUserItem);

	CT_BOOL CheckUserIsOnline(CT_DWORD dwUserID);
	CT_BOOL CheckUserHasRoom(CT_DWORD dwUserID);
	CT_BOOL GetUserProxyInfo(CT_DWORD dwUserID, stUserProxyInfo& proxyInfo);

	CT_VOID UpdateUserPRoomGameRoundCount(CT_DWORD dwUserID, CT_DWORD dwGameRoundCount);

public:
	CT_VOID UpdateUserGemToClient(CServerUserItem* pUserItem, CT_DWORD dwGem);
	CT_VOID AddUserGem(CServerUserItem* pUserItem, CT_INT32 nAddGem);

//DB相关的操作
private:
	CT_VOID WriteVideoCommonPackage(MSG_GameMsgDownHead* pDownData, CT_DWORD dwLen);
	CT_VOID WriteEnterSuccMsg();
	CT_VOID WriteUserEnterMsg();
	CT_VOID WriteFreeGameScene();

	//发送DB信息
	CT_VOID SendDBMsg(CT_WORD mainId, CT_WORD subId, CT_VOID* pMsg, CT_WORD wLen);
	//更新玩家钻石
	CT_VOID UpdateUserGemToDB(CT_DWORD dwUserID, CT_INT32 nAddGem);
	CT_VOID UpdateUserGemToCenterServer(CT_DWORD dwUserID, CT_INT32 nAddGem);
	//玩家钻石消耗记录
	CT_VOID InsertGemConsumeRecord(CT_DWORD dwOldGem, CT_DWORD dwNewGem);
	//保存录像回放文件
	CT_VOID SaveVideoToDB();
	//插入私人场大局信息
	CT_VOID InsertGameRoundRecord();
	CT_VOID InsertGameRoundRecord(acl::db_handle* db);
	//插入大局信息
	CT_VOID InsertGameRoundRecordEx();
	//保存私人场的小局信息
	CT_VOID SavePlayCountRecord(CT_DWORD dwUserID, CT_CHAR* pNickName, CT_WORD wChairID, CT_LONGLONG llScore);
	//保存私人场的大局结算信息
	CT_VOID SaveGameRoundRecord(const CT_CHAR* pEndTime);
	//保存输赢记录
	CT_VOID SaveWinLostCount(const CT_CHAR* pEndTime);
	//保存大局信息
	CT_VOID SaveGameRoundCount(CT_DWORD dwUserID, CT_DWORD dwGameRoundCount);
	//记录自动开房信息（针对微信群组和搞两把群组）
	CT_VOID RecordGroupRoomInfo();

public:
	//DB结果
	CT_VOID InsertGameRoundResult(CT_UINT64 uGameRoundID);

//与中心服相关操作
public:
	CT_VOID	SendGroupPRoomClearing();

private:
	//定时器列表
	TimerInfo	            	m_SysTimer[DW_SYS_TIMER_NUMS];

public:
	enum enDismissPRoom
	{
		en_UnDismiss_PRoom,			//不解散
		en_Dismiss_PRoom,			//解散
		en_Dismiss_PRoom_ByAdmin,	//管理员解散房间
		en_Force_LeavePRoom_ByAdmin,//被T出房间
		en_Timeout_LeavePRoom,		//时间超时，房间自动解散
		en_Dismiss_Max,
	};

	enum enVipPRoomApplyState
	{
		en_PRoomApply_Undisposed,	//已申请未处理
		en_PRoomApply_Agree,		//房主同意
		en_PRoomApply_Disagree,		//房主不同意

		en_PRoomApply_InOtherRoom,	//在其他房间
		en_PRoomApply_Unapply,		//未申请
	};

	enum enAddVipPRoomApply
	{
		en_Add_Succ,				//增加成功
		en_Add_Repeat,				//重复申请
		en_Add_Max,					//已经达到最大申请人数
	};

	enum enVipPRoomApplyMax
	{
		en_PRoomApplyMax = 10,
	};

private:
	ITableFrameSink*	m_pTableFrameSink;
	TableState			m_TableState;
	PrivateTableInfo	m_PrivateTableInfo;

	tagGameKind*		m_pGameKindInfo;
	tagGameRoomKind*	m_pRoomKindInfo;

	CServerUserItem**	m_UserList;
	CT_BOOL				m_bGameStart;
	enGameRoundPhase	m_GameRoundPhase;
	CT_UINT64			m_GameRoundID;
	CT_DWORD			m_dwPlayTime;

	std::string			m_strPlayCountTime;			//小局结算时间

	std::vector<enDismissPRoom> m_vecDismissPRoom;
	CT_DWORD			m_dwApplyDismissTime;
	CT_DWORD			m_dwApplyDismissUserID;
	CT_BOOL				m_bDismissPRoom;

	acl::redis*			m_pRedis;

	std::map<CT_DWORD, stApplyVipPRoomInfo> m_mapApplyVipRoom;

	//录像文件数据
	ReadWriteBuffer				m_GameMsg;
};