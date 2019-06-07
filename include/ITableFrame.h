#pragma once

#include "CTType.h"
#include "IGameOtherData.h"
#include "GlobalStruct.h"
#include "acl_cpp/lib_acl.hpp"
#include "CMD_Dip.h"
#include "CMD_Inner.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

class ITableFrameSink;
//游戏配置数据
struct GameCfgData
{
	CT_WORD			wPlayInning;						//打多少局(达到多少局游戏结束)
	CT_WORD			wPlayHuXi;							//打多少胡息(达到多少胡息游戏结束)
};

//百人牛牛操作概率
struct tagBrGameOperateRatio
{
	CT_LONGLONG 	llTotayStock;                       //今日库存
	CT_WORD 		wChangeCardRatio;                   //换牌概率
	CT_WORD 		wSystemAllKillRatio;                //系统必杀概率
};

//百人牛牛黑名单
struct tagBrGameBlackList
{
    tagBrGameBlackList()
    :dwUserID(0)
    ,wUserLostRatio(0)
    ,dwControlScore(0)
    {

    }

	CT_DWORD 		dwUserID;							//黑名单用户ID
	CT_WORD 		wUserLostRatio;						//用户输的概率(概率为0,删除数据库记录)
	CT_DWORD    	dwControlScore;                     //控制的下注额度(对百人游戏有效)
};

//炸金花牌库概率
struct tagZjhCardDepotRatio
{
	CT_BYTE			cbDepotID;					//牌库ID
	CT_BYTE			cbCard1Ration;				//散牌概率
	CT_BYTE			cbCard2Ration;				//对子概率
	CT_BYTE			cbCard3Ration;				//顺子概率
	CT_BYTE			cbCard4Ration;				//金花概率
	CT_BYTE			cbCard5Ration;				//顺金概率
	CT_BYTE			cbCard6Ration;				//豹子概率
};

//鱼的子类型统计信息:比如一箭双雕就有两个子类型
struct subFishStatisticsInfo
{
	//鱼ID 
	CT_LONGLONG fishTypeID;
	//击中次数
	CT_LONGLONG llHitCount;
	//命中次数
	CT_LONGLONG llDeathCount;
	//历史总赔率
	CT_LONGLONG llTotalBL;
	//这条鱼的总吸分
	CT_LONGLONG llTotalWinScore;
	//这条鱼的总放分
	CT_LONGLONG llTotalLoseScore;
	//鱼种名称
	std::string strFishName;

	subFishStatisticsInfo()
	{
		ResetData();
	}

	CT_VOID ResetData()
	{
		strFishName = "";
		fishTypeID = 255;
		llHitCount = 0;
		llDeathCount = 0;
		llTotalBL = 0;
		llTotalWinScore = 0;
		llTotalLoseScore = 0;
	}
};

enum eTongJiType
{
	//统计玩家捕鱼信息
	TJ_PLAYER_SHOOT_FISH_INFO = 1,
	//统计玩家的胜率破产率信息
	TJ_PLAYER_Win_bankrupted_Pro = 2,
};

enum ePlayerWinBankupInfo
{
	//玩家进入房间
	PWB_ENTER = 1,
	//玩家退出房间时赢了
	PWB_EXIT_WIN = 2,
	//玩家退出房间不输不赢
	PWB_EXIT_NO_WIN_LOSE = 3,
	//玩家退出房间时破产了
	PWB_EXIT_Bankrup = 4,
	//玩家退出房间输了，但是没破产
	PWB_EXIT_LOSE = 5,
	//为降低破产率大幅度提高命中率之后，奖励给玩家的分数
	PWB_FORBID_POCHAN_REWARD = 6,
	//记录大鱼信息
	PWB_BIGFISH_INFO = 7
};

//大鱼信息
struct tagBigFishInfo
{
    //鱼类型
    CT_BYTE cbFishType;
    //玩家在这类鱼上付出的分数
    CT_LONGLONG llPlayerLostScore;
    //玩家在这类鱼上获取的分数
    CT_LONGLONG llPlayerGetScore;
};

//关服的统计信息:测试使用
struct tagTongJiInfo
{
	/*CT_DWORD dwPlayerCnt;
	CT_LONGLONG llWinMost;
	CT_LONGLONG llLoseMost;
	CT_DWORD dwPoChanPlayerCnt;
	CT_DWORD dwWinPlayerCnt;
	//用于统计个人税收
	CT_DWORD dwTax;

    tagTongJiInfo()
    {
         dwPlayerCnt = 0;
         llWinMost = 0;
         llLoseMost = 0;
         dwPoChanPlayerCnt = 0;
         dwWinPlayerCnt = 0;
        //用于统计个人税收
         dwTax = 0;
    }*/

	eTongJiType type;
	CT_DWORD  dwUserID;
	CT_INT32 gameIndex;
    //玩家开炮总数
    CT_INT32 nTotalShootCount;
    //玩家打死鱼的总数
    CT_INT32 nTotalDeathCount;
    //每种鱼的击中数
    CT_INT32 nPerFishTypeShootCount[39];
    //每种鱼的命中数
    CT_INT32 nPerFishTypeDeathCount[39];
    //每种鱼给玩家带来的的价值总数
    CT_LONGLONG llPerFishTypeScore[39];
    //胜率破产信息
	ePlayerWinBankupInfo ePWBInfo;
	//为降低破产大幅度提高玩家命中率产生的金币
	CT_LONGLONG llPoChanReward;
    //大鱼信息
    tagBigFishInfo bigFishInfo;
    tagTongJiInfo()
    {
    	//默认统计玩家捕鱼信息
		type = TJ_PLAYER_SHOOT_FISH_INFO;
        nTotalShootCount = 0;
        nTotalDeathCount = 0;
        memset(nPerFishTypeShootCount, 0, sizeof(nPerFishTypeShootCount));
        memset(nPerFishTypeDeathCount, 0, sizeof(nPerFishTypeDeathCount));
        memset(llPerFishTypeScore, 0, sizeof(llPerFishTypeScore));
		ePWBInfo = PWB_ENTER;
		llPoChanReward = 0;
		memset(&bigFishInfo, 0, sizeof(bigFishInfo));
    }
};

//实时统计玩家的输赢
struct tagPlayerAddScore
{
	CT_DWORD dwUserID;
	CT_LONGLONG llAddScore;
	//输的比率
	CT_INT32   nLosePro;
	//玩家所在的桌子
	ITableFrameSink *pTableFrameSink;
};

//鱼种统计信息
struct FishStatisticsInfo
{
	int ServerID;
	//鱼ID 
	CT_LONGLONG fishTypeID;
	//击中次数
	CT_LONGLONG llHitCount;
	//命中次数
	CT_LONGLONG llDeathCount;
	//历史总赔率
	CT_LONGLONG llTotalBL;
	//这条鱼的总吸分
	CT_LONGLONG llTotalWinScore;
	//这条鱼的总放分
	CT_LONGLONG llTotalLoseScore;
	//鱼种名称
	std::string strFishName;
	//子鱼类型信息 目前只有30 31 32 33 35 36才用这个数据
	std::vector<subFishStatisticsInfo>  vecSubFishInfo;

	//配置死亡几率
	CT_INT32 nCfgDeathPro;
	//理论死亡几率
	CT_INT32 nLiLunDeathPro;

	FishStatisticsInfo()
	{
		ResetData();
	}

	CT_VOID ResetData()
	{
		nCfgDeathPro = 0;
		nLiLunDeathPro = 0;
		ServerID = 0;
		strFishName = "";
		fishTypeID = 255;
		llHitCount = 0;
		llDeathCount = 0;
		llTotalBL = 0;
		llTotalWinScore = 0;
		llTotalLoseScore = 0;
		vecSubFishInfo.clear();
	}
};

//血池控制信息
struct tagStockControlInfo
{
	tagStockControlInfo()
	:dwServerID(0)
	,llTotalWinScore(0)
	,llTotalLostScore(0)
	,llStock(0)
	,llBlackTax(0)
	,llDianKongWinLost(0)
	,iStockStatus(1)
	{
	}
	CT_DWORD 		dwServerID;				//服务器ID(对于荒野行动来说是serverId*100+(0-3))
	CT_LONGLONG 	llTotalWinScore;		//系统总赢钱
	CT_LONGLONG     llTotalLostScore;		//系统总输钱
	CT_LONGLONG     llStock;				//当前库存(会被衰减)
	CT_LONGLONG     llBlackTax;             //收到的总暗税
	CT_LONGLONG     llDianKongWinLost;      //点控输赢
	CT_INT32 		iStockStatus;			//库存状态
};

class CServerUserItem;
//私人房游戏阶段
enum enGameRoundPhase
{
	en_GameRound_Free,
	en_GameRound_Start,
	en_GameRound_End,
};

class ITableFrameSink;
//桌子接口
class ITableFrame
{
public:
	ITableFrame() {}
	virtual ~ITableFrame() {}

public:
	virtual void Init(ITableFrameSink* pSink, const TableState& state, tagGameKind* pGameKind, tagGameRoomKind* pRoomKind, acl::redis* pRedis) = 0;

public:
	//发送数据
	virtual CT_BOOL SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf = nullptr, CT_WORD dwLen = 0, CT_BOOL bRecord = false) = 0;

public:
	//设置定时器
	virtual CT_BOOL SetGameTimer(CT_WORD wTimerID, CT_DWORD dwTime, CT_DWORD dwParam = 0) = 0;
	//销毁定时器
	virtual CT_BOOL	KillGameTimer(CT_WORD wTimerID) = 0;
	//判断是否有定时器
	virtual CT_BOOL	IsHasGameTimer(CT_WORD wTimerID) = 0;

public:
	//获得桌子信息
	virtual void GetTableInfo(TableState& TableInfo) = 0;
	//获取桌子ID
	virtual CT_DWORD GetTableID() = 0;
	//获得游戏配置数据
	virtual void GetGameCfgData(GameCfgData& CfgData) = 0;
	//处理输赢积分
	virtual CT_BOOL WriteUserScore(CT_DWORD dwChairID, const ScoreInfo& ScoreData) = 0;
	//解散游戏
	virtual CT_BOOL DismissGame(CT_BYTE cbDismissType) = 0;
	//结束游戏
	virtual CT_BOOL ConcludeGame(CT_BYTE cbGameStatus, const CT_CHAR* pDrawInfo) = 0;
	//计算税收
	virtual CT_LONGLONG CalculateRevenue(CT_DWORD dwChairID, CT_LONGLONG llScore) = 0;
	//获取私人房信息
	virtual void GetPrivateTableInfo(PrivateTableInfo& privateTableInfo) = 0;

public:
	//用户是否存在
	virtual CT_BOOL IsExistUser(CT_DWORD dwChairID) = 0;
	//是否私人场 
	virtual CT_BOOL IsPrivateRoom() = 0;
	//设置大局阶段
	virtual CT_VOID SetGameRoundPhase(enGameRoundPhase gameRoundPhase) = 0;
	//获取大局阶段
	virtual enGameRoundPhase GetGameRoundPhase() = 0;
	//获取私人房间房主椅子id
	virtual CT_WORD GetPRoomOwnerChairID() = 0;
	//删除玩家的连接
	virtual CT_VOID CloseUserConnect(CT_DWORD dwChairID) = 0;

public:
	//设置玩家托管状态
	virtual CT_VOID SetUserTrustee(CT_DWORD dwChairID, CT_BOOL bTrustee) = 0;
	//获得玩家托管状态
	virtual CT_BOOL GetUserTrustee(CT_DWORD dwChairID) = 0;
	//获得玩家昵称
	virtual char* GetNickName(CT_DWORD dwChairID) = 0;
	//获得玩家头像
	virtual char* GetHeadUrl(CT_DWORD dwChairID) = 0;
	//获取玩家头像索引
	virtual CT_BYTE GetHeadIndex(CT_DWORD dwChairID) = 0;
	//获得玩家VIP
	virtual CT_BYTE GetVipLevel(CT_DWORD dwChairID) = 0;
	//获取玩家的性别
	virtual CT_BYTE GetUserSex(CT_DWORD dwChairID) = 0;
	//获取桌子上的用户ID
	virtual CT_DWORD GetUserID(CT_DWORD dwChairID) = 0;
	//获得当前局数
	virtual CT_WORD GetCurrPlayCount() = 0;	
	//设置玩家准备状态
	virtual CT_VOID SetUserReady(CT_DWORD dwChairID) = 0;
	//玩家离开
	virtual void OnUserLeft(CServerUserItem* pUser, bool bSendStateMyself, bool bForceLeave) = 0;
	//玩家离线
	virtual void OnUserOffLine(CServerUserItem* pUser) = 0;

public:
	//获得玩家金币
	virtual CT_LONGLONG GetUserScore(CT_DWORD dwChairID) = 0;
	//设置玩家金币
	virtual CT_VOID AddUserScore(CT_DWORD dwChairID, CT_LONGLONG llAddScore, CT_BOOL bBroadcast = true) = 0;
	//设置玩家状态
	virtual CT_VOID SetUserStatus(CT_DWORD dwChairID, USER_STATUS status) = 0;
	//玩家状态
	virtual CT_BYTE GetUserStatus(CT_DWORD dwChairID) = 0;	
	//获取玩家进入限制
	virtual CT_LONGLONG GetUserEnterMinScore() = 0;
	//获取基础底分
	virtual CT_DWORD GetGameCellScore() = 0;
	//获取玩家的帐号基础信息
	virtual GS_UserBaseData* GetUserBaseData(CT_DWORD dwChairID) = 0;
	//获取玩家的金币信息
	virtual GS_UserScoreData* GetUserScoreData(CT_DWORD dwChairID) = 0;
	//获取房间配置
	virtual tagGameRoomKind* GetGameKindInfo() = 0;
	//发送邮件
	virtual CT_VOID SendUserMail(tagUserMail* pUserMail) = 0;
	//获取玩家椅子号(返回INVALID_CHAIR, 说明不在线)
	virtual CT_WORD GetUserChairID(CT_DWORD dwUserID) = 0;

public:
	//清理桌子上的用户
	virtual CT_VOID ClearTableUser(CT_DWORD wChairID = INVALID_CHAIR, CT_BOOL bSendState = false, CT_BOOL bSendStateMyself = false, CT_BYTE cbSendErrorCode = 0) = 0;

	//时间脉冲
	virtual void OnTimePulse() = 0;

	//游戏消息
	virtual void OnGameEvent(CT_WORD wChairID, CT_DWORD dwSubID, const CT_VOID* pData, CT_WORD wDataSiz) = 0;

public:
	virtual CT_VOID UserVoiceChat(CServerUserItem* pUser, CT_CHAR* pChatUrl) = 0;
	virtual CT_VOID UserTextChat(CServerUserItem* pUser, CT_BYTE cbType, CT_BYTE cbIndex) = 0;
	virtual CT_BOOL	CheckGameStart() = 0;
	virtual CT_VOID GameRoundStartDeploy() = 0;
	virtual CT_VOID GameStart() = 0;
	//广播用户状态bSendStateMyself表示是否广播自己的状态给自己
	virtual CT_VOID BroadcastUserState(CServerUserItem* pUser, CT_BOOL bSendStateMyself = false) = 0;
	virtual CT_BOOL IsGameStart() =  0;
	virtual CT_VOID OnUserEnterAction(CServerUserItem* pUserItem, CT_BOOL bDistribute = false, CT_BOOL bReConnect = false) = 0;
	virtual void DelGameUserToProxy(CServerUserItem* pUser) = 0;
	//玩家起立bSendStateMyself表示是否广播自己的状态给自己
	virtual CT_BOOL UserStandUp(CServerUserItem* pUserItem, CT_BOOL bSendState = true, CT_BOOL bSendStateMyself = false) = 0;
	virtual CT_BOOL RecordRawInfo(RecordScoreInfo* pRecordScoreInfo, CT_WORD wUserCount, CT_BYTE* pAreaCardType, CT_BYTE cbAreaCount,CT_DWORD dwBankerUserID,\
								  CT_LONGLONG llStock, CT_LONGLONG llAndroidStock, CT_LONGLONG llTodayStock, CT_WORD wSystemAllKillRation, CT_WORD wChangeCardRatio) = 0;
	//记录红包扫雷的信息
	virtual CT_BOOL RecordHongBaoSLInfo(RecordHBSLInfo* pHbInfo, RecordHBSLGrabInfo* pGrabInfo, CT_WORD wUserCount) = 0;
	virtual CT_BOOL WriteGameSceneToRecord(CT_VOID* pData, CT_DWORD dwDataSize, CT_WORD wSubMsgID) = 0;

	//玩家每发一炮就通知系统赢了多少分
	virtual CT_VOID NotifySysWinScore(CT_LONGLONG llSysWinScore) = 0;
	//玩家每次得分就通知系统输了多少分
	virtual CT_VOID NotifySysLoseScore(CT_LONGLONG llSysLoseScore) = 0;
	//获取捕鱼库存值
	virtual CT_LONGLONG GetFishKuCun() = 0;
	//点控玩家输钱的时候,玩家输分了 
	virtual CT_VOID NotifySysWinScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llWinScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose) = 0;
	//点控玩家赢钱的时候,玩家赢分了
	virtual CT_VOID NotifySysLoseScore_DianKong(CT_DWORD dwUserID, CT_LONGLONG llLoseScore, CT_LONGLONG llKuCun, CT_LONGLONG llBloodPoolDianKongWinLose) = 0;
	//保存点控玩家数据
	virtual  CT_VOID SavePlayerDianKongData(MSG_G2DB_User_PointControl &data) = 0;
	//删除玩家点控数据
	virtual CT_VOID DelPlayerDianKongData(MSG_GS2DB_DelUserDianKongData &data) = 0;
	//保存鱼种的统计信息
	virtual CT_VOID SaveFishStatisticsInfo(FishStatisticsInfo info[], int arrSize) = 0;
	//捕鱼统计所有信息:用于测试
	virtual CT_VOID TongJi(tagTongJiInfo &info) = 0;
	//捕鱼统计单个玩家的输赢信息:用于测试
	virtual CT_VOID SinglePlayerMostWinLostScore(CT_DWORD dwUserID, CT_LONGLONG addScore) = 0;
	//玩家点控开始
	virtual CT_VOID UserDianKongStart(CT_DWORD dwUserID) = 0;
	//玩家点控结束
	virtual CT_VOID UserDianKongEnd(CT_DWORD dwUserID) = 0;
	//保存血池信息(新)
	virtual CT_VOID SaveStockControlInfo(tagStockControlInfo* pStockControlInfo) = 0;
	//获取捕鱼系统税收
	virtual CT_DWORD GetFishTaxPro() = 0;
	//设置捕鱼的炮数
	virtual CT_VOID AddFishFireCount(CT_WORD wChairID, CT_DWORD dwCount) = 0;
    //设置黑名单
    virtual CT_VOID SetBlackListUser(tagBrGameBlackList* pBlackListUser) = 0;
    virtual CT_VOID SaveJSFishInfo(js_fish_info_type info_type, js_fish_save_info &info) = 0;
public:
	//判断是否是机器人
	virtual CT_BOOL IsAndroidUser(CT_DWORD dwChairID) = 0; 
	//获取桌子回调类
	virtual ITableFrameSink* GetTableFrameSink() = 0;
};


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif