#pragma once
#include "CTType.h"
#include "CGlobalData.h"
#include <string>
#include <time.h>
#include <memory.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

struct UserAccountsBaseData
{
	CT_DWORD							dwUserID;						//用户ID
	CT_DWORD							dwChannelID;					//代理ID(绑定渠道来源ID)
	CT_DWORD							dwClientChannelID;				//客户端的渠道ID
	CT_BYTE								cbNewAccount;					//新帐号(0:不是新账号，1:默认新手引导，2:斗地主引导)
	CT_BYTE								cbShowExchange;					//显示兑换
	//CT_DWORD							dwGem;							//宝石
	CT_BYTE								cbPlatformID;					//平台ID
	CT_BYTE								cbGender;						//性别
	CT_BYTE								cbHeadId;						//头像ID
	CT_BYTE								cbVipLevel;						//VIP等级(用于客户端连接proxy)
	CT_BYTE								cbVipLevel2;					//VIP等级(用于客户端VIP系统)
	CT_DWORD							dwTotalRecharge;				//总充值
	CT_LONGLONG							llScore;						//金币
	CT_LONGLONG							llBankScore;					//银行金币
	//CT_CHAR								szHeadUrl[HEAD_URL_LEN];		//头像Url
	CT_CHAR								szNickName[NICKNAME_LEN];		//昵称
	CT_CHAR								szIP[IP_LEN];					//客户端IP
	//CT_CHAR								szUUID[UUID_LEN];				//session.
	CT_CHAR								szLocation[USER_LOCATE_LEN];	//所在地
    CT_CHAR								szProvince[PROVINCE_LEN];	    //省份
	CT_CHAR								szPassword[PASS_LEN];			//密码
	CT_CHAR								szBankPassword[PASS_LEN];		//银行密码
	CT_CHAR								szMobileNum[MOBILE_NUM_LEN];	//手机号码
	CT_CHAR								szMachineType[MACHINETYPE_LEN]; //手机类型

	//支付宝和银行卡信息
	CT_CHAR								szAliPayAccout[ALIPAY_ACCOUNT_LEN];	//支付宝帐号
	CT_CHAR								szAliPayName[REAL_NAME_LEN];		//支付宝姓名
	CT_CHAR								szBankCardNum[BANK_CARD_LEN];		//银行卡号
	CT_CHAR								szBankCardName[REAL_NAME_LEN];		//银行卡姓名

	//好牌参数
	//CT_DWORD                            dwZjhCount;						//炸金花总局数
	//CT_BYTE                             cbZjhGoodCardParam;				//炸金花好牌参数
	//CT_DWORD 							dwQznnCount;					//抢庄牛牛总局数
	//CT_BYTE 							cbQznnGoodCardParam;			//抢庄牛牛好牌参数
	//CT_DWORD 							dwGoodCardTime;					//好牌调整时间

	CT_BYTE								cbStatus;							//帐号状态(状态：1.启用 2.冻结 3.删除)
};

struct UserScoreData
{
	CT_DWORD		dwRevenue;											//税收
	CT_DWORD        dwWinCount;											//赢得次数
	CT_DWORD        dwLostCount;										//输的次数
	CT_DWORD        dwDrawCount;										//和的次数
	CT_DWORD        dwFleeCount;										//逃跑次数
	CT_DWORD		dwPlayTime;											//游戏时长
};

struct PrivateRoomInfo
{
	CT_WORD			wGameID;				//游戏ID
	CT_WORD			wKindID;				//类型ID
	CT_WORD			wPlayCount;				//游戏局数
	CT_WORD			wUserCount;				//用户人数
	CT_WORD			dwRoomNum;				//私人房间ID
	CT_DWORD		dwServerID;				//房间ID
	CT_DWORD		dwOwerUserID;			//房主ID
	//CT_WORD			wWinLimit;			//输赢上限
	//CT_WORD			wEndLimit;			//结束上限
	CT_BYTE			cbIsVip;				//是否VIP房
	//CT_BYTE			cbIsAuto;			//是否自动开房
	CT_DWORD		dwGroupID;				//群ID
	CT_DWORD		dwRecordID;				//记录ID
	CT_BYTE			cbGroupType;			//群组类型
	CT_CHAR			szOtherParam[PROOM_PARAM_LEN]; //其他游戏参数
};

struct tagSystemStatus
{
	tagSystemStatus()
		:cbPrivateMode(0)
		, cbScoreMode(0)
	{
	}

	CT_BYTE			cbPrivateMode;			//私人房间模式（0：关闭 1：打开）
	CT_BYTE			cbScoreMode;			//金币房间模式（0：关闭 1：打开）
};

struct tagGameKind
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_WORD			wSortID;
	CT_CHAR			szKindName[KINDNAME_LEN];
	CT_CHAR			szServerDll[PROCESSNAME_STR_LEN];
};

struct tagGameRoomKind
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_WORD			wRoomKindID;
	//CT_WORD			wSortID;
	CT_WORD			wEnterVipLevel;
	CT_WORD			wStartMinPlayer;
	CT_WORD			wStartMaxPlayer;
	CT_WORD			wTableCount;
	CT_DWORD		dwCellScore;
	CT_DWORD		dwEnterMinScore;
	CT_DWORD		dwEnterMaxScore;
	CT_DWORD		dwRevenue;
	CT_INT32		iBroadcastScore;
	/*CT_BYTE		cbCreateRoomPlayCount1;
	CT_WORD			wCreateRoomNeedGem1;
	CT_WORD			wCreateRoomHuxi1;
	CT_BYTE			cbCreateRoomPlayCount2;
	CT_WORD			wCreateRoomNeedGem2;
	CT_WORD			wCreateRoomHuxi2;*/
	//CT_CHAR			szPRoomPlayCountCond[PROOM_PARAM_LEN];
	//CT_CHAR			szPRoomNeedGem[PROOM_PARAM_LEN];
	CT_CHAR			szPRoomCond[PROOM_PARAM_LEN];
	CT_CHAR			szRoomKindName[KINDNAME_LEN];
	CT_DWORD		dwMaxUserCount;//服务器承载最大人数
	CT_BYTE			cbState;		//服务器状态
};

//群组基本信息
struct tagGroupInfo
{
	tagGroupInfo()
	:dwGroupID(0)
	,wUserCount(0)
	,dwMasterID(0)
	//,dwDeputyID1(0)
	//,dwDeputyID2(0)
	,dwContribution(0)
	,cbIncomeRate(0)
	,cbSettleDays(0)
	,cbLevel(0)
	,cbIcon(0)
	{
        szGroupName[0] = '\0';
        szNotice[0] = '\0';
	}

	CT_DWORD		dwGroupID;						//群组ID
	CT_WORD			wUserCount;						//成员数量
	CT_DWORD		dwMasterID;						//群主ID
	//CT_DWORD        dwDeputyID1;                    //副群主ID1
	//CT_DWORD        dwDeputyID2;                    //副群主ID2
	CT_DWORD    	dwContribution;					//贡献值
	CT_BYTE         cbIncomeRate;                    //收入比例
    CT_BYTE         cbSettleDays;                   //结算天数
    CT_BYTE 		cbLevel;						//等级
	CT_BYTE 		cbIcon;							//图标
	//CT_DWORD		dwKindID;						//默认的游戏ID(gameid*100+kindid)
	//CT_WORD		wBindUserPlay;					//是否成员才能玩游戏
	//CT_CHAR		szMasterName[NICKNAME_LEN];		//群主昵称
	CT_CHAR			szGroupName[GROUP_NAME_LEN];	//群名称
	CT_CHAR			szNotice[GROUP_NOTICE_LEN];		//群公告
};


struct tagGSGroupInfo
{
    tagGSGroupInfo()
    :dwGroupID(0)
    ,dwMasterID(0)
    {

    }

    CT_DWORD		dwGroupID;						//群组ID
    CT_DWORD		dwMasterID;						//群主ID
};

//群组成员信息
struct tagGroupUserInfo
{
	tagGroupUserInfo()
	:dwUserID(0)
	,dwParentUserId(0)
	,cbSex(0)
	,cbHeadId(0)
	,cbVip2(0)
	,cbMemLevel(0)
	,cbInComeRate(0)
	,wDeductRate(0)
	,wDeductTop(0)
	,wDeductIncre(0)
	,dwDeductStart(0)
	,dwLastTakeIncomeDate(0)
	,dwLastIncomeDate(0)
	,wLinkIncomeDays(0)
	,llTotalIncome(0)
	,dwRecharge(0)
	,dwRegDate(0)
	,dwLastDate(0)
	,bOnline(false)
	{ szRemarks[0] = '\0';}

	CT_DWORD		dwUserID;					    //群成员ID
	CT_DWORD        dwParentUserId;                 //上级成员ID(群主的上级成员为0)
	CT_BYTE 		cbSex;						    //性别
	CT_BYTE 		cbHeadId;					    //头像
	CT_BYTE 		cbVip2;						    //成员Vip2
	CT_BYTE         cbMemLevel;                     //群成员等级(群主是总代等级为0)
	CT_BYTE         cbInComeRate;                   //收入比例
    CT_WORD         wDeductRate;                    //扣量比例(千分比)
    CT_WORD         wDeductTop;                     //扣量封顶(千分比)
    CT_WORD         wDeductIncre;                   //扣量递增(千分比)
    CT_DWORD        dwDeductStart;                  //扣量起征点(单位:分)
    CT_DWORD        dwLastTakeIncomeDate;           //最后提取收入时间
    CT_DWORD        dwLastIncomeDate;               //最后产生收入的时间
    CT_WORD         wLinkIncomeDays;                //连续产生收益的
    CT_LONGLONG 	llTotalIncome;				    //成员总收入
	CT_DWORD        dwRecharge;                     //群成员充值
	CT_DWORD        dwRegDate;                      //注册时间
	CT_DWORD        dwLastDate;					    //最后登录时间
	CT_BOOL         bOnline;                        //玩家是否在线
	CT_CHAR         szRemarks[GROUP_REMARKS_LEN];   //上级给他的备注
	//CT_CHAR			szNickName[NICKNAME_LEN];	//昵称
	//CT_CHAR			szHeadUrl[HEAD_URL_LEN];	//头像URL
	bool operator < (const tagGroupUserInfo& oDR) const
	{
		return dwUserID < oDR.dwUserID;
	}
};

struct tagGSGroupUserInfo
{
    tagGSGroupUserInfo()
    :dwUserID(0)
    ,dwParentUserId(0)
    {
    }

    CT_DWORD		dwUserID;					    //群成员ID
    CT_DWORD        dwParentUserId;                 //上级成员ID(群主的上级成员为0)
};


//成员税收
struct tagGroupUserRevenue
{
    tagGroupUserRevenue()
    :dwUserID(0)
    ,dwRevenue(0)
    {

    }
    CT_DWORD        dwUserID;                       //用户ID
    CT_DWORD        dwRevenue;                      //税收
};

//成员日收入
struct tagGroupUserDateIncome
{
    tagGroupUserDateIncome()
    :wDeductRate(0)
    ,dwOriginalIncome(0)
    ,dwDeductIncome(0)
    {}

    CT_WORD         wDeductRate;                    //扣量比例(千分比)
    CT_DWORD        dwOriginalIncome;               //原始收入
    CT_DWORD        dwDeductIncome;                 //扣量后的收入
};

//成员收入详细
struct tagGroupUserDateIncomeDetail
{
    tagGroupUserDateIncomeDetail()
    :dwUserID(0)
    ,dwSubUserID(0)
    ,dwDateInt(0)
    ,wDeductRate(0)
    ,dwOriginalIncome(0)
    ,dwDeductIncome(0)
    {

    }
    CT_DWORD        dwUserID;                       //玩家ID
    CT_DWORD        dwSubUserID;                    //附属玩家ID
    CT_DWORD        dwDateInt;                      //时间Int
    CT_WORD         wDeductRate;                    //扣量比例(千分比)
    CT_DWORD        dwOriginalIncome;               //原始收入
    CT_DWORD        dwDeductIncome;                 //扣量后的收入
};

//成员总收入
struct tagGroupUserTotalIncome
{
    tagGroupUserTotalIncome()
    :dwUserID(0)
    ,llTotalIncome(0)
    { }

    CT_DWORD        dwUserID;
    CT_DWORD        dwSubUserID;
    CT_LONGLONG     llTotalIncome;
};

struct tagGroupChat
{
    tagGroupChat()
    :dwMsgId(0)
    ,dwFromUserID(0)
    ,dwToUserID(0)
    ,dwSendTime(0)
    {
        szChat[0] = '\0';
    }
    CT_DWORD        dwMsgId;                    //消息ID
    CT_DWORD        dwFromUserID;               //发送者ID
    CT_DWORD        dwToUserID;                 //接收者ID
    CT_DWORD        dwSendTime;                 //发送时间
    CT_CHAR         szChat[GROUP_CHAT_LEN];     //聊天内容
};

struct tagGroupPRoomUserInfo
{
    tagGroupPRoomUserInfo()
        :dwUserID(0)
        ,cbHeadId(1)
        ,cbSex(1)
        ,cbVip2(0)
    {
    }
    CT_DWORD    dwUserID;
    CT_BYTE     cbHeadId;
    CT_BYTE     cbSex;
    CT_BYTE     cbVip2;
};

struct tagGroupPRoomInfo
{
    CT_WORD		wGameID;
    CT_WORD		wKindID;
    CT_WORD     wRoomKindID;
    CT_WORD     wSitUserCount;
    CT_DWORD	dwRoomNum;
    CT_DWORD    dwServerID;
    CT_DWORD    dwOwnerID;
    CT_DWORD    dwEmptyTime;
    CT_BYTE     cbLock;
    tagGroupPRoomUserInfo userInfo[MAX_PLAYER];

    tagGroupPRoomInfo()
        :wGameID(0)
        ,wKindID(0)
        ,wRoomKindID(0)
        ,wSitUserCount(0)
        ,dwRoomNum(0)
        ,dwServerID(0)
        ,dwOwnerID(0)
        ,dwEmptyTime(time(NULL))
        ,cbLock(0)
    {
        //memset(szParam, 0, sizeof(szParam));
        //std::cout << "------------------------construct tagGroupPRoomInfo-----------------------" << std::endl;
    }

    ~tagGroupPRoomInfo()
    {
        //std::cout << "------------------------destruct tagGroupPRoomInfo-----------------------" << std::endl;
    }
};

struct tagGSGroupPRoomInfo
{
    CT_DWORD     dwRoomNum;
};

struct tagGroupLevelCfg
{
	CT_BYTE 		cbLevel;                   //等级
	CT_BYTE         cbSettleDays;              //结算天数
	CT_WORD         wIncomeRate;               //本等级默认收益比例(百分比)
	CT_WORD         wDeductRate;               //扣量比例(千分比)
	CT_WORD         wDeductTop;                //扣量封顶(千分比)
	CT_WORD         wDeductIncre;              //扣量递增(千分比)
	CT_DWORD        dwDeductStart;             //扣量起征点(单位:分)
	CT_DWORD 		dwMaxContribution;         //本等级最大贡献值(下一等级开始值)
	CT_DWORD        dwSettleMinIncome;         //结算最小收益
	CT_DWORD        dwSettleMaxIncome;         //结算最大收益
	CT_CHAR         szDomain[GROUP_PROMOTION_DOMIAN]; //推广链接
};

struct tagGroupUserGameData
{
	tagGroupUserGameData()
	:wGameID(0)
	,wKindID(0)
	,wRoomKindID(0)
	,dwRoomCount(0)
	,llIncome(0)
	{}

	CT_WORD 		wGameID;
	CT_WORD 		wKindID;
	CT_WORD 		wRoomKindID;
	CT_DWORD 		dwRoomCount;                //开房间次数
	CT_LONGLONG 	llIncome;                   //开房收益
};

struct tagDefaultRoomConfig
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_CHAR			szConfig[PROOM_PARAM_LEN];
};

struct tagGroupRoomConfig
{
	CT_DWORD		dwGroupID;
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_CHAR			szConfig[PROOM_PARAM_LEN];
};

struct tagGlobalUserInfo
{
	CT_DWORD		dwUserID;
	CT_DWORD		dwProxyServerID;
	CT_DWORD		dwGameServerID;
	CT_DWORD		dwGem;
	CT_DWORD 		dwLastHeartBeatTime;
	CT_DWORD        dwLastRedPacketTime;
	CT_LONGLONG		llScore;
	CT_LONGLONG		llBankScore;
	CT_DWORD    	dwTotalRecharge; //充值单位:元
	CT_DWORD		dwChannelID;
	CT_DWORD		dwClientChannelID;
	CT_DWORD        dwGroupID;
	CT_BYTE			cbPlatformId;
	CT_BYTE			cbSex;
	CT_BYTE			cbVip;
	CT_BYTE			cbVip2;
	CT_BYTE			cbHeadID;
	CT_BYTE			cbMachineType;
	CT_BYTE			cbShowExchange;
	CT_BYTE         cbProvinceCode;
	CT_BOOL			bOnline;
	CT_CHAR			szNickName[NICKNAME_LEN];
	CT_CHAR			szHeadUrl[HEAD_URL_LEN];
	tagGlobalUserInfo()
		: dwUserID(0)
		, dwProxyServerID(0)
		, dwGameServerID(0)
		, dwGem(0)
		, dwLastHeartBeatTime(0)
		, dwLastRedPacketTime(0)
		, llScore(0)
		, llBankScore(0)
		, dwTotalRecharge(0)
		, dwChannelID(0)
		, dwClientChannelID(0)
		, dwGroupID(0)
		, cbPlatformId(0)
		, cbSex(0)
		, cbVip(0)
		, cbHeadID(0)
		, cbMachineType(0)
		, cbShowExchange(0)
		, cbProvinceCode(0)
		, bOnline(false)
	{
		szNickName[0] = '\0';
		szHeadUrl[0] = '\0';
	}

	~tagGlobalUserInfo()
	{
		//std::cout << "remove online user info!";
	}
};

struct tagApplyAddGroupInfo
{
	CT_DWORD		dwApplyUserID;						//申请者ID
	CT_BYTE 		cbSex;								//申请者性别
	CT_BYTE 		cbHeadId;							//申请者头像ID
	CT_BYTE         cbVip2;                             //申请者等级
    CT_DWORD        dwApplyTime;                        //申请时间
    CT_CHAR			szNickName[NICKNAME_LEN];			//申请者昵称
	//CT_CHAR			szHeadUrl[HEAD_URL_LEN];			//用户头像
	//CT_CHAR			szApplyTime[TIME_LEN];				//申请时间
};

//机器人参数
struct tagAndroidUserParameter
{
	CT_DWORD		dwUserID;							//用户ID
	CT_DWORD		dwEnterTime;						//进入时间
	CT_DWORD		dwLeaveTime;						//离开时间
	CT_LONGLONG		llTakeMinScore;						//至少带入金币
	CT_LONGLONG		llTakeMaxScore;						//至多带入金币
	CT_DWORD		dwGem;								//钻石
	CT_BYTE			cbBigJetton;						//大额度玩家
	CT_BYTE			cbSex;								//性别
	CT_BYTE			cbHeadId;							//头像ID
	CT_BYTE			cbVip2;								//VIP2等级
	CT_CHAR			szNickName[NICKNAME_LEN];			//昵称
};

//救济金信息
struct tagBenefitData
{
	CT_BYTE 		cbVip2;							//玩家VIP2等级
	CT_BYTE			cbCurrCount;					//当前救济金次数
	CT_DWORD		dwLastTime;						//最后领取时间

	tagBenefitData()
	{
		Reset();
	}

	void Reset()
	{
        cbVip2 = 0;
		cbCurrCount = 0;
		dwLastTime = 0;
	}
};

//救济金奖励
struct tagBenefitReward
{
	CT_BYTE 		cbVip2;							//vip2等级
	CT_BYTE			cbRewardCount;					//奖励次数
	CT_DWORD		dwRewardScore;					//奖励金币
	CT_DWORD		dwLessScore;					//开始领救济金的至少金币
};

//钻石兑换金币配置表
struct tagExchangeScoreConfig
{
	CT_WORD			wGem;							//需要钻石
	CT_DWORD		dwExchangeScore;				//兑换金币
	CT_DWORD		dwExtraPresentScore;			//额外赠送金币
};

//红包信息
struct tagHongBaoInfo
{
	CT_DWORD		dwID;							//红包ID
	CT_DWORD		dwHongBaoScore;					//红包金币
	CT_DWORD		dwSenderID;						//发送者
	CT_DWORD		dwSendTime;						//发送时间
	CT_CHAR			szSenderNickName[NICKNAME_LEN];	//发送者名称
	CT_CHAR			szSenderHeadUrl[HEAD_URL_LEN];	//发送者头像
	CT_CHAR			szHongBaoName[HONGBAO_NAME_LEN];//红包名称
	CT_DWORD		dwGuessUserID;					//猜中红包的用户ID
	CT_CHAR			szGuessNickName[NICKNAME_LEN];	//猜中者名称
	CT_WORD			wGuessCount;					//竞猜次数
};

//金币排行榜
struct tagScoreRank
{
	CT_BYTE			cbRank;							//排名
	CT_DWORD		dwUserID;						//用户ID
	CT_DOUBLE		dScore;							//玩家金币
	CT_BYTE			cbVip2;							//VIP2等级
	CT_BYTE			cbGender;						//性别
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//发送者名称
	//CT_CHAR			szHeadUrl[HEAD_URL_LEN];		//发送者头像
};

//分享排行榜
struct tagShareRank
{
	CT_BYTE			cbRank;							//排名
	CT_BYTE			cbSendReward;					//是否发送奖励
	CT_DWORD		dwUserID;						//用户ID
	CT_DWORD		dwShareCount;					//分享次数
	CT_DOUBLE		dRewardScore;					//奖励金币
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//用户昵称
};

//签到排行榜
struct tagSignInRank
{
	CT_BYTE			cbRank;							//排名
	CT_DWORD		dwUserID;						//用户ID
	CT_WORD			wSignInCount;					//连续签到次数
	CT_BYTE			cbVipLevel;						//VIP等级
	CT_CHAR			szNickName[NICKNAME_LEN];		//发送者名称
	CT_CHAR			szHeadUrl[HEAD_URL_LEN];		//用户昵称
};

//今日在线排行榜
struct tagTodayOnlineRank
{
	CT_DWORD 		dwUserID;						//用户ID
	CT_DWORD 		dwOnlineTime;					//在线时长
	CT_BYTE			cbVip2;							//VIP2等级
	CT_BYTE			cbRank;							//排名
	CT_BYTE			cbGender;						//性别
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//用户昵称
};

//今日任务完成榜
struct tagTodayTaskFinishRank
{
	CT_DWORD 		dwUserID;						//用户ID
	CT_WORD 		wFinishCount;					//任务完成次数
	CT_DOUBLE 		dRewardScore;					//奖励金币
	CT_BYTE			cbVip2;							//VIP2等级
	CT_BYTE			cbGender;						//性别
	CT_BYTE			cbRank;							//排名
	CT_BYTE			cbHeadID;						//头像ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//用户昵称
};

//任务信息
struct tagTaskModel
{
	CT_WORD			wTaskID;						//任务ID
	CT_WORD			wTaskType;						//任务类型
	CT_WORD			wCompleteType;					//任务完成类型
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//游戏子ID
	CT_WORD			wRoomKindID;					//房间类型
	CT_DWORD		dwTotalProgress;				//任务要完成的总进度
	CT_DWORD		dwReward;						//任务奖励
	//CT_CHAR		szTaskDesc[TASK_DESC_LEN];		//任务描述
};

//用户任务
struct tagUserTask
{
	CT_WORD		wTaskID;
	CT_DWORD	dwCurrProgress;
	CT_DWORD	dwReceiveTime;
	CT_BOOL		bFinish;

	tagUserTask()
	:wTaskID(0)
	,dwCurrProgress(0)
	,dwReceiveTime((CT_DWORD)time(NULL))
	,bFinish(false)
	{
	}

	tagUserTask(CT_WORD taskID)
	:wTaskID(taskID)
	,dwCurrProgress(0)
	,dwReceiveTime((CT_DWORD)time(NULL))
	,bFinish(false)
	{	
	}

	void Reset()
	{
		dwCurrProgress = 0;
		dwReceiveTime = (CT_DWORD)time(NULL);
		bFinish = false;
	}
};

//财神降临活动
struct tagWealthGodComingCfg
{
    tagWealthGodComingCfg()
    {
        memset(this, 0, sizeof(*this));
    }
    CT_WORD     wID;                        //活动ID
    CT_WORD     wGameID;                    //游戏ID
    CT_WORD     wKindID;                    //类型ID
    CT_WORD     wRoomKindID;                //房间类型ID
	CT_WORD 	wRewardRatio;				//奖励比例
	CT_WORD     wCompleteCount;				//完成次数
	CT_WORD     wSendRewardMinUser;         //发奖最低人数
	CT_BYTE     cbJudgeStander;				//判断标准(1、输赢判定 2、下注总额判定)
    CT_BYTE     cbStartHour;                //开始时间
    CT_BYTE     cbStartMin;                 //开始时间
    CT_BYTE     cbEndHour;                  //结束时间
    CT_BYTE     cbEndMin;                   //结束时间
    CT_BYTE     cbState;                    //状态(1、启用 2、停用)
};

//财神降临奖励
struct tagWealthGodComingReward
{
    
    CT_WORD     wGameID;                    //游戏ID
    CT_WORD     wKindID;                    //类型ID
    CT_WORD     wRoomKindID;                //房间类型ID
	CT_BYTE 	cbRank;						//名次
	CT_BYTE		cbRewardRate;				//奖励比例(如果按照比例奖励)
	CT_DWORD 	dwRewardScore;				//奖励金币(如果按照固定金额奖励)
};

//正在进行的财神降临数据
struct tagWealthGodComingData
{
    tagWealthGodComingData()
    {
        memset(this, 0, sizeof(*this));
    }
    CT_WORD     wID;                        //活动ID
    CT_DWORD    dwIndex;                    //场次
	CT_LONGLONG llRevenue;					//总税收
	CT_CHAR		szDate[DATE_LEN];			//活动日期
	CT_CHAR 	szTime[TIME_LEN_3];			//活动时间
};

//财神降临的最后一个场次ID
struct tagWealthGodComingLastIndex
{
    tagWealthGodComingLastIndex()
    {
        memset(this, 0, sizeof(*this));
    }
    CT_DWORD    dwLastIndex;                //最后一个场次
    CT_WORD     wGameID;                    //游戏ID
    CT_WORD     wKindID;                    //游戏类型
    CT_WORD     wRoomKindID;                //房间类型
};

//财神降临的用户信息
struct tagWealthGodComingUserInfo
{
	tagWealthGodComingUserInfo()
	{
		memset(this, 0, sizeof(*this));
	}
	
	CT_DWORD 	dwIndex;					//场次
	CT_DWORD 	dwUserID;					//用户ID
	CT_BYTE		cbRank;						//名次
	CT_BYTE		cbGender;					//用户昵称
	CT_BYTE		cbHeadIndex;				//头像索引
	CT_BYTE		cbVip2;						//玩家VIP2
	CT_WORD 	wCompleteCount;				//完成次数
	CT_DWORD 	dwRewardScore;				//奖励金币
	CT_DWORD 	dwScore;					//玩家总成绩（下注数或者输赢分）
	CT_DWORD 	dwRevenue;					//总税收
	CT_CHAR 	szNickName[NICKNAME_LEN];	//用户昵称
};

//财神降临的用户信息
struct tagWealthGodComingUserInfoEx : public tagWealthGodComingUserInfo
{
	tagWealthGodComingUserInfoEx()
	{
		memset(this, 0 , sizeof(*this));
	}
	
	CT_WORD     wGameID;                    //游戏ID
	CT_WORD     wKindID;                    //类型ID
	CT_WORD     wRoomKindID;                //房间类型ID
};

//红包状态
struct tagRedPacketStatus
{
    tagRedPacketStatus()
    {
        cbStatus = 0;
        szDes[0] = '\0';
    }
    CT_BYTE     cbStatus;                   //红包状态(0:未开放 1:开放弹窗口 2:开放不弹窗口)
    CT_CHAR     szDes[RED_PACKET_DES];      //红包描述
};

//新年红包的房间配置
struct tagRedPacketRoomCfg
{
    tagRedPacketRoomCfg()
    : dwGameIndex(0)
    , dwRedPacketValue(0)
    { }

    CT_DWORD    dwGameIndex;                //游戏房间(gameid*100+kindi*10+roomkindid)
    CT_DWORD 	dwRedPacketValue;           //红包参数
};

//新年红包场次
struct tagRedPacketIndex
{
    tagRedPacketIndex()
    :dwIndex(0)
    //,startDate(0)
    ,cbReadyHour(0)
	,cbReadyMin(0)
    ,cbStartHour(0)
    ,cbStartMin(0)
    ,cbEndHour(0)
    ,cbEndMin(0)
    ,dwRedPacketScore(0)
    ,dwRedRacketGrabbedScore(0)
    ,wRedPacketCount(0)
    ,cbStage(0)
    ,cbState(1)
    {
        for (int i = 0; i < RED_PACKET_ROOM_COUNT; ++i)
        {
            dwGameIndex[i] = 0;
        }
        szStartDate[0] = '\0';
    }

	CT_DWORD 	dwIndex;                            //红包期数
	CT_DWORD 	dwGameIndex[RED_PACKET_ROOM_COUNT];	//活动配置ID(指是哪个房间)
    CT_CHAR 	szStartDate[DATE_LEN];              //开始日期
    CT_BYTE     cbReadyHour;                        //准备时间
    CT_BYTE     cbReadyMin;                         //准备时间
    CT_BYTE     cbStartHour;                        //开始时间
    CT_BYTE     cbStartMin;                         //开始时间
    CT_BYTE     cbEndHour;                          //结束时间
    CT_BYTE     cbEndMin;                           //结束时间
    CT_DWORD 	dwRedPacketScore;			        //红包金币
    CT_DWORD    dwRedRacketGrabbedScore;            //红包被抢红包
    CT_WORD 	wRedPacketCount;			        //红包个数
    CT_BYTE 	cbStage;                            //活动阶段(0: 未开始 1: 活动准备阶段 2: 活动进行中 3: 活动结束未发奖 4:活动结束已发奖)
    CT_BYTE     cbState;                            //状态(1、启用 2、停用)
};

//新年红包的玩家数据
struct tagRedPacketUserGrabData
{
    tagRedPacketUserGrabData()
    :dwIndex(0)
    ,wGameID(0)
    ,wKindID(0)
    ,wRoomKindID(0)
    ,dwLuckyMoney(0)
    ,dwUserID(0)
    { }

	CT_DWORD 	dwIndex;							//红包期数
    CT_WORD     wGameID;                            //游戏ID
    CT_WORD     wKindID;                            //游戏类型
    CT_WORD     wRoomKindID;                        //房间类型
	CT_DWORD    dwLuckyMoney;                       //红包金额
	CT_DWORD    dwUserID;                           //玩家ID
};

//上一期红包大赢家
struct tagRedPacketBigWinner
{
    tagRedPacketBigWinner()
    {
        dwIndex = 0;
        for (int i = 0; i < RED_PACKET_BIGWINNER; ++i)
        {
            wGameID[i] = 0;
            wKindID[i] = 0;
            wRoomKindID[i] = 0;
			cbBigSex[i] = 1;
			cbBigHeadId[i] = 1;
            cbVip2[i] = 0;
            dwBigWiner[i] = 0;
			dwBigScore[i] = 0;
        }
    }

    void Reset()
    {
        dwIndex = 0;
        for (int i = 0; i < RED_PACKET_BIGWINNER; ++i)
        {
            wGameID[i] = 0;
            wKindID[i] = 0;
            wRoomKindID[i] = 0;
			cbBigSex[i] = 1;
			cbBigHeadId[i] = 1;
            cbVip2[i] = 0;
            dwBigWiner[i] = 0;
            dwBigScore[i] = 0;
        }
    }

    CT_DWORD    dwIndex;							    //上期ID
	CT_WORD     wGameID[RED_PACKET_BIGWINNER];          //游戏ID
	CT_WORD     wKindID[RED_PACKET_BIGWINNER];          //游戏类型
	CT_WORD     wRoomKindID[RED_PACKET_BIGWINNER];      //房间类型
	CT_BYTE     cbBigSex[RED_PACKET_BIGWINNER];         //上一期大赢家性别
	CT_BYTE     cbBigHeadId[RED_PACKET_BIGWINNER];      //上一期大赢头像
    CT_BYTE     cbVip2[RED_PACKET_BIGWINNER];           //上一期大赢VIP2
    CT_DWORD	dwBigWiner[RED_PACKET_BIGWINNER];       //上期大赢家
    CT_DWORD    dwBigScore[RED_PACKET_BIGWINNER];       //上期大赢家金币
};

//一元夺宝活动
struct tagDuoBaoCfg
{
	tagDuoBaoCfg()
	{
		memset(this, 0, sizeof(*this));
	}

	CT_WORD 	wID;						//活动ID
	CT_WORD 	wSingleScore;               //单注金额(单位元)
	CT_DWORD    dwUserLeaseScore;           //玩家购买至少金额(单位元)
	CT_WORD     wlimitCount;                //个人限制次数
	CT_DWORD    dwTotalCount;               //活动开奖总次数
	CT_DWORD    dwTimeLeft;                 //剩余时间
	CT_DWORD    dwReward;                   //奖励金币(单位元)
	CT_WORD     wRevenueRation;             //税收比例(百分比)
	CT_DWORD    dwIndex;                    //第几期
	CT_BYTE 	cbState;					//状态(0: 未开启 1: 开启)
    CT_CHAR     szTitle[DUOBAO_TITLE];      //夺宝标题
};

//一元夺宝的活动信息
struct tagDuoBaoData
{
    tagDuoBaoData()
    {
        memset(this, 0, sizeof(*this));
    }

    CT_WORD     wID;                        //活动ID
    CT_BYTE     cbState;                    //状态(0表示没有开奖 1表示正在开奖 2表示已经开奖 3表示过期不开奖)
    CT_DWORD    dwIndex;                    //第几期
	CT_DWORD 	dwLuckyUserID;              //得奖者ID
	CT_DWORD    dwLuckyNum;                 //幸运码
	CT_DWORD 	dwRewardScore;				//奖励金币
    CT_DWORD    dwStartTime;                //开始时间
    CT_DWORD    dwEndTime;                  //结束时间
};

//一元夺宝的个人信息
struct tagDuoBaoUserInfo
{
    tagDuoBaoUserInfo()
    {
        memset(this, 0, sizeof(*this));
    }

	CT_WORD 	wID;						//活动
	CT_DWORD    dwIndex;                    //第几期
	CT_DWORD    dwUserID;                   //用户ID
	CT_DWORD    dwLuckyNum;                 //幸运码
	CT_BYTE     cbSex;                      //性别
	CT_BYTE     cbHeadIndex;                //头像索引
};

//用户邮件
struct tagUserMail
{
    tagUserMail()
    :dwUserID(0)
    ,cbMailType(2)
    ,cbState(0)
    ,llScore(0)
    ,cbScoreChangeType(0)
    ,dwSendTime(0)
    ,dwExpiresTime(0)
    {
        szTitle[0] = '\0';
        szContent[0] = '\0';
    }

	CT_DWORD	dwUserID;					//用户ID
	CT_BYTE		cbMailType;					//邮件类型(1:表示阅后即毁普通邮件, 2:表示阅后过期才销毁)
	CT_BYTE		cbState;					//邮件状态(0:表示未读, 1:表示已读, 2:登录弹框)
	CT_CHAR		szTitle[MAIL_TITLE];		//邮件标题
	CT_CHAR		szContent[MAIL_CONTENT];	//邮件内容
	CT_LONGLONG	llScore;					//奖励积分
	CT_BYTE 	cbScoreChangeType;			//金币变化类型
	CT_DWORD	dwSendTime;					//邮件时间
	CT_DWORD	dwExpiresTime;				//邮件过期时间
};

//最高在线信息
struct tagPcuInfo
{
	tagPcuInfo()
	:dwPCU(0)
	,dwPCUiOS(0)
	,dwPCUAndroid(0)
	,cbPlatformId(0) { }

	~tagPcuInfo() {}

	CT_DWORD	dwPCU;						//最高在线
	CT_DWORD	dwPCUiOS;					//iOS最高在线
	CT_DWORD	dwPCUAndroid;				//Android最高在线
	CT_BYTE     cbPlatformId;               //平台ID
};

//充值渠道信息
struct tagRechargeChannelInfo
{
	CT_DWORD	dwChannelID;								//渠道ID
	CT_BYTE		cbRechargeChannelID[RECHARGE_COUNT];		//充值渠道ID
	CT_BYTE		cbState[RECHARGE_COUNT];					//渠道状态
	CT_BYTE		cbReload;									//是否重新加载
};

//充值渠道的充值金额
struct tagRechargeChannelAmount
{
    CT_BYTE     cbChannelID;
    CT_DWORD    dwAmount[7];                //0-5是充值金额 6是充值最大金额
};

//兑换渠道信息
struct tagExchangeChannelStatus
{
	CT_BYTE		cbChannelID[2];				//兑换渠道ID(2支付宝 3银联)
	CT_BYTE		cbStatus[2];				//渠道状态
};

//兑换渠道的兑换金额
struct tagExchangeChannelAmount
{
    CT_BYTE     cbChannelID;
    CT_DWORD    dwAmount[5];                //0-4是兑换金额
	CT_DWORD 	dwMinAmount;				//兑换最小金额
	CT_DWORD 	dwMaxAmount;				//兑换最大金额
	CT_BYTE		cbDayCountLimite;			//每天兑换次数限制
	CT_WORD 	wHandlingFee;				//单笔手续费(千分比)
};

//推广代理(月入百万)
struct tagGeneralizePromoterInfo
{
	CT_CHAR		szAccount[3][NICKNAME_LEN];
	CT_CHAR		szNickName[3][NICKNAME_LEN];
	CT_BYTE		cbType[3];					//1 表示微信 2 表示QQ
	CT_BYTE 	cbPlatformId;				//平台Id
	CT_BYTE		cbAccountCount;             //账号数量
};

//渠道主页
struct tagClientChannelDomain
{
	CT_DWORD	dwChannelID;					//渠道ID
	CT_BYTE		cbLock;							//渠道是否锁定
	CT_CHAR		szDomain[CHANNEL_DOMAIN_LEN];	//渠道主页
	CT_CHAR     szDomain2[CHANNEL_DOMAIN_LEN];  //渠道主页2
};

//渠道显示兑换的条件
struct tagChannelShowExchangeCond
{
	CT_DWORD	dwChannelID;			    //充值渠道
	CT_DWORD	dwTotalRecharge;			//总充值
};

//全民代理的信息
struct tagQuanMinChannel
{
	CT_DWORD	dwChannelID;				//全民代理ID
	CT_BYTE		cbStatus;					//状态
};

//各平台的充值配置
struct tagPlatformRechargeCfg
{
	CT_BYTE 	cbPlatformId;								                //平台Id
	CT_BYTE		cbPromoterRechargeType;						                //代理充值类型
	CT_CHAR     szRechargeDomain[RECHARGE_DOMAIN_LEN];		                //充值域名
	CT_CHAR 	szPromoterRechargeUrl[PROMOTER_RECHARGE_URL_LEN];           //代理充值Url
	CT_CHAR 	szPromoterRechargeUpload[PROMOTER_RECHARGE_URL_LEN];        //代理充值上传图片Url
	CT_CHAR     szPromoterRechargeMerchant[PROMOTER_RECHARGE_MERCHANT_LEN]; //代理充值商户号
	CT_CHAR     szPromoterRechargeToken[PROMOTER_RECHARGE_TOKEN_LEN];       //代理充值令牌
};

//VIP等级
struct tagVip2Config
{
	CT_BYTE			cbVip2;				//VIP等级
	CT_DWORD		dwRecharge;			//需要充值数量
};

//渠道注册和绑定手机赠送金币
struct tagChannelPresentScore
{
  	CT_DWORD 		dwChannelID;		//渠道ID
  	CT_DWORD 		dwRegisterScore;	//注册赠送金币
  	CT_DWORD 		dwBindMobileScore;	//绑定手机赠送金币
};

//聊天上传图片链接
struct tagChatUpLoadUrl
{
	CT_CHAR 		szUploadUrl[CHAT_UPLOAD_URL];  //上传图片地址
};

//公告信息
struct tagAnnouncementInfo
{
	CT_BYTE			cbAnnouncementID;                       //公告ID
	CT_BYTE         cbSortID;                               //排序ID
	CT_CHAR         szName[ANNOUNTCEMENT_TITLE_LEN];        //公告标题
	CT_CHAR         szContent[ANNOUNTCEMENT_CONTENT_LEN];   //公告内容
	CT_BYTE 		cbStatus;								//状态(1:正常 2:删除)
};

//斗地主黑渠道信息
struct tagBlackChannel
{
	CT_DWORD 	dwChannelID;		//渠道ID
	CT_WORD 	wBadCardRation;		//发差牌的概率
};

enum enJSFishDataType
{
	//系统信息类型 比如：系统总赢分 系统税收等
	en_SysInfo_type = 1,
	//系统控制信息  比如: 杀分线 放分线值 ，抽水炮数等。
	en_ControlInfo_type = 2,
	//关服的时候保存库存等系统信息
	en_SaveSysInfo_type = 3,
	//玩家上线设置玩家的miss库
	en_SetPlayerMiss_type = 4
};

//极速捕鱼系统信息
struct tagJSFishSystemInfo
{
	CT_INT64 llSystemWinScore;
    CT_INT64 llSystemLoseScore;
    CT_INT64 llSystemTax;
    CT_INT64 llSystemStock;
    CT_INT64 llSystemDiankongValue;
};

//极速捕鱼控制信息
struct tagJSFishControlInfo
{
    CT_INT64 llMustKillValue;
    CT_INT64 llProKillValue;
    CT_INT64 llProAwardValue;
    CT_INT64 llMustAwardValue;
	CT_INT32 nRevenueCount;
};

struct tagBlackChannelEx : public tagBlackChannel
{
	CT_DWORD 	dwGameIndex;		//游戏索引
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif