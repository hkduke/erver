#ifndef CMD_DIP_H_
#define CMD_DIP_H_
#include "CTType.h"
#include "CGlobalData.h"
#include "IGameOtherData.h"
#include "CTData.h"


#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//web服务器和dip服务服通讯的消息定义
#define MSG_WDIP_MAIN						50

//服务注册
#define SUB_W2D_RECHARGE					5000			//玩家充值(玩家的金币和银行修改都走这里)
#define SUB_W2D_WX_CREATE_PROOM				5001			//创建私人房间
#define SUB_W2D_WX_ADD_GROUP_USER			5002			//添加群成员
#define SUB_W2D_WX_REMOVE_GROUP_USER		5003			//删除群成员
#define SUB_W2D_QUERY_PROOM_INFO			5004			//查询房间信息
#define SUB_W2D_DISMISS_PROOM				5005			//解散房间
#define SUB_W2D_TICK_USER					5006			//踢除玩家

#define SUB_W2D_UPDATE_GROUP_OPTIONS		5007			//更新群设置
#define SUB_W2D_UPDATE_GROUP_ROOM_CFG		5008			//更新群组开房设置
#define SUB_W2D_REGISTER_PROMOTER			5009			//注册代理

#define SUB_W2D_APPLY_ADD_GROUP				5010			//申请加入群组

#define SUB_W2D_QUERY_ONLINE_USER			5020			//查询在线玩家
#define SUB_W2D_QUERY_WAITLLIST_USER        5021        	//查询等待玩家
#define SUB_W2D_QUERY_SERVERID_TABLE_MSG    5022       		//查询指定ServerID桌子信息
#define SUB_W2D_QUERY_USER_IS_ONLINE		5023			//查询玩家是否在线
#define SUB_W2D_QUERY_ONLINE_INFO			5024			//查询在线人数(pcu和各个游戏的实时人数)
#define SUB_W2D_UPDATE_ANDROID_CFG			5025			//更新机器人
#define SUB_W2D_ANDROID_PLAY_COUNT			5026			//更新机器人同时游戏人数
#define SUB_W2D_RELOAD_RECHARGE_CHANNEL		5027			//重新加载充值渠道
#define SUB_W2D_NEW_MAIL					5028			//有新邮件
#define SUB_W2D_SEND_SYSMSG					5029			//发送系统消息
#define SUB_W2D_PROMOTER_LIST				5030			//代理充值列表(qq,weixin号列表)
#define SUB_W2D_ACCOUNT_STATUS				5031			//帐号状态
#define SUB_W2D_EXCHANGE_ACCOUNT_STATUS		5032			//兑换帐号状态
#define SUB_W2D_PROMOTER_GENERALIZE			5033			//代理推广信息
#define SUB_W2D_CLIENT_CHANNEL_DOMAIN		5034			//客户端渠道的主页
#define SUB_W2D_QUANMIN_PROMOTER			5035			//全民代理信息
#define SUB_W2D_CHANNEL_PRESENT_SCORE		5036			//渠道赠送金币
#define SUB_W2D_UPDATE_STOCK_CONTROL		5037			//更新库存控制
#define SUB_W2D_REMOVE_SYSMSG				5038			//删除系统消息
#define SUB_W2D_TICK_USER_FOR_FISH			5039			//剔除捕鱼用户
#define SUB_W2D_RATIO_CONTROL_FOR_BRNN		5040			//百人牛牛概率控制
#define SUB_W2D_BRNN_BLACK_LIST         	5041            //百人牛牛黑名单
#define SUB_W2D_WEALTHGODCOMING_CFG			5042			//财神降临配置
#define SUB_W2D_WEALTHGODCOMING_REWARD		5043			//财神降临奖励
#define SUB_W2D_RECHARGE_AMOUNT				5044			//充值金额配置
#define SUB_W2D_EXCHANGE_AMOUNT				5045			//兑换金额配置
#define SUB_W2D_DIANKONG					5046			//玩家点控
#define SUB_W2D_ZJH_CARDRATIO				5047			//炸金花牌库概率
#define SUB_W2D_DUOBAO_CFG					5048			//一元夺宝配置
#define SUB_W2D_BENEFIT_CFG					5049			//更新救济金配置
#define SUB_W2D_PLATFORM_RECHARGE_CFG		5050			//平台充值配置
#define SUB_W2D_SMS_PLATFORM_ID             5051            //短信平台id
#define SUB_W2D_REDPACKET_STATUS            5052            //红包活动状态
#define SUB_W2D_REDPACKET_ROOM_CFG          5053            //红包活动的房间配置
#define SUB_W2D_REDPACKET_INDEX             5054            //红包活动的期数配置
#define SUB_W2D_CHAT_UPLOAD_URL				5055			//更新图片上传链接
#define SUB_W2D_UPDATE_ANNOUNCEMENT         5056            //更新公告信息
#define SUB_W2D_UPDATE_BLACK_CHANNEL		5057			//更新黑渠道信息(针对斗地主游戏)
#define SUB_W2D_UPDATE_GROUP_INFO           5058            //更新群组的信息
#define SUB_W2D_UPDATE_GROUP_USER_INFO      5059            //更新群组成员信息
#define SUB_W2D_UPDATE_GROUP_LEVEL_CFG      5060            //更新群组的等级配置信息
#define SUB_W2D_USER_BIND_GROUP             5061            //绑定玩家到公会

//#define SUB_D2W_CREATE_PROOM_SUCC			5050			//创建房间返回
//#define SUB_D2W_CREATE_PROOM_FAIL			5051			//创建房间失败
//#define SUB_D2W_QUERY_PROOM_INFO			5052			//查询房间信息返回
//#define SUB_D2W_DISMISS_PROOM				5053			//解散房间返回
//#define SUB_D2W_TICK_USER					5054			//剔除玩家返回

struct MSG_W2D_User_Recharge
{
	CT_DWORD		dwUserID;						//玩家ID
	CT_DWORD		dwRecharge;						//充值数量
};

//////////////////////////////////////////////////////////////////////////////////
//中心服务器和dip服务服通讯的消息定义
#define MSG_DIPCS_MAIN							51

//C->S
#define SUB_DIP2CS_REGSTER						5199		//注册DIP
#define SUB_DIP2CS_RECHARGE						5100		//玩家充值
#define SUB_DIP2CS_WX_CREATE_PROOM				5101		//自动开房
#define SUB_DIP2CS_WX_ADD_GROUP_USER			5102		//添加群成员
#define SUB_DIP2CS_WX_REMOVE_GROUP_USER			5103		//删除群成员
#define SUB_DIP2CS_UPDATE_GROUP_OPTIONS			5104		//修改群设置
#define SUB_DIP2CS_UPDATE_GROUP_ROOM_CFG		5105		//修改群默认游戏
#define	SUB_DIP2CS_REGSTER_PROMOTER				5106		//注册代理
#define SUB_DIP2CS_QUERY_PROOM_INFO				5107		//查询房间信息
#define SUB_DIP2CS_DISMISS_PROOM				5108		//解散房间
#define SUB_DIP2CS_TICK_USER					5109		//剔除玩家
#define SUB_DIP2CS_APPLY_ADD_GROUP				5110		//申请加入群

#define SUB_DIP2CS_QUERY_ONLINE_USER			5120		//查询在线用户
#define SUB_DIP2CS_QUERY_WAITLLIST_USER     	5121        //查询等待玩家
#define SUB_DIP2CS_QUERY_SERVERID_TABLE_MSG 	5122        //查询指定ServerID桌子信息
#define SUB_DIP2CS_UPDATE_ANDROID				5123		//更新机器人
#define SUB_DIP2CS_UPDATE_ANDROID_PLAY_COUNT	5124		//更新机器人上桌人数
#define SUB_DIP2CS_RELOAD_RECHARGE_CHANNEL		5125		//重新加载充值渠道
#define SUB_DIP2CS_NOTIFY_NEWMAIL				5126		//通知有新邮件
#define SUB_DIP2CS_SEND_SYSMSG					5127		//发送新系统消息
#define SUB_DIP2CS_PROMOTER_LIST				5128		//充值代理列表
#define SUB_DIP2CS_EXCHANGE_CHANNEL_STATUS  	5129		//兑换渠道状态
#define SUB_DIP2CS_UPDATE_USER_STATUS			5130		//更新玩家状态
#define SUB_DIP2CS_GENERALIZE_PROMOTER			5131		//更新推广代理信息
#define SUB_DIP2CS_CLIENTCHANNEL_DOMAIN			5132		//客户端渠道的主页
#define SUB_DIP2CS_QUANMIN_INFO					5133		//全民代理信息
#define SUB_DIP2CS_CHANNEL_PRESENTSCORE			5134		//渠道注册赠送金币
#define SUB_DIP2CS_UPDATE_SERVER_STOCK			5135		//更新库存信息
#define SUB_DIP2CS_REMOVE_SYSMSG				5136		//删除系统消息
#define SUB_DIP2CS_TICK_FISH_USER				5137		//剔除捕鱼用户
#define SUB_DIP2CS_RATIO_CONTROL_FOR_BRNN   	5138        //百人游戏概率控制
#define SUB_DIP2CS_BRGAME_BLACKLIST         	5139        //百人游戏黑名单
#define SUB_DIP2CS_WEALTHGODCOMING_CFG			5140		//财神降临配置
#define SUB_DIP2CS_WEALTHGODCOMING_REWARD   	5141		//财神降临奖励配置
#define SUB_DIP2CS_RECHARGE_AMOUNT				5142		//充值金额配置
#define SUB_DIP2CS_EXCHANGE_AMOUNT				5143		//兑换金额配置
#define SUB_DIP2CS_FISH_DIANKONG				5144		//捕鱼点控
#define SUB_DIP2CS_ZJH_CARDRATIO				5145		//炸金花出牌概率
#define SUB_DIP2CS_DUOBAO_CFG					5146		//夺宝配置
#define SUB_DIP2CS_UPDATE_BENEFIT_CFG       	5147		//更新救济金配置
#define	SUB_DIP2CS_UPDATE_PLATFORM_RECHARGE_CFG 5148        //平台充值配置
#define SUB_DIP2CS_UPDATE_SMS_PID               5149        //更新短信平台
#define SUB_DIP2CS_REDPACKET_STATUS             51500       //红包活动状态
#define SUB_DIP2CS_REDPACKET_ROOM_CFG           51501       //红包活动的房间配置
#define SUB_DIP2CS_REDPACKET_INDEX              51502       //红包活动的期数配置
#define SUB_DIP2CS_CHAT_UPLOAD_URL				51503		//上传聊天图片链接
#define SUB_DIP2CS_UPDATE_ANNOUNCEMENT			51504		//更新公告
#define SUB_DIP2CS_UPDATE_BLACK_CHANNEL         51505       //更新黑渠道名单
#define SUB_DIP2CS_UPDATE_GROUP_INFO            51506       //更新群信息
#define SUB_DIP2CS_UPDATE_GROUP_USER_INFO       51507       //更新群成员信息
#define SUB_DIP2CS_UPDATE_GROUP_LEVEL_CFG       51508       //更新群等级配置信息
#define SUB_DIP2CS_ADD_GROUP_USER               51509       //增加群组成员

//S->C
#define SUB_CS2DIP_CREATE_PROOM_SUCC			5150		//创建房间成功返回
#define SUB_CS2DIP_CREATE_PROOM_FAIL			5151		//创建房间失败返回
#define SUB_CS2DIP_QUERY_PROOM_INFO				5152		//查询房间信息返回
#define SUB_CS2DIP_DISMISS_PROOM				5153		//解散房间返回
#define SUB_CS2DIP_TICK_USER					5154		//剔除玩家返回
#define SUB_CS2DIP_APPLY_ADD_GROUP				5155		//申请入加群返回

#define SUB_CS2DIP_QUERY_ONLINE_USER			5170		//查询用户返回
#define SUB_CS2DIP_QUERY_WAITLLIST_USER     	5171        //查询等待返回
#define SUB_CS2DIP_QUERY_SERVERID_TABLE_MSG 	5172        //查询指定ServerID桌子返回

enum enCreatePRoom_ErrorCode
{
	EN_GAMESERVER_NOENOUGH	= 101,					//没有找到空闲的服务器
	EN_ERROR_PARAM			= 102,					//参数错误
};

struct CMD_DipServer_Info
{
	CT_DWORD	dwServerID;							//服务器ID
	CT_CHAR		szServerIP[STRING_IP_LEN];			//IP地址
	CT_CHAR		szServerName[SERVER_NAME_LEN];		//服务名称
};

struct MSG_D2CS_User_Recharge
{
	CT_DWORD		dwUserID;						//玩家ID
	CT_LONGLONG		llRecharge;						//充值数量
	CT_DWORD		dwTotalCash;					//总充值金额(单位：元)
	CT_WORD			wCurrCash;						//本次金额（单位：元）
	CT_BYTE			cbVipLevel;						//当前VIP等级
	CT_BYTE			cbVipLevel2;					//当前VIP2等级
	CT_BYTE			cbType;							//充值类型（0:充值失败 1：金币 2：钻石, 3:银行）
};

#define PROOM_JSON_LEN					256
struct MSG_D2CS_AutoCreate_PRoom
{
	CT_CHAR			szJsonParam[PROOM_JSON_LEN];	//json数据
	CT_UINT64		uWebSock;						//web端连接
};

struct MSG_D2CS_AddOrRemove_GroupUser
{
	CT_DWORD		dwGroupID;						//群ID
	CT_DWORD		dwBindUserPlay;					//是否绑定玩家
	CT_WORD			wUserCount;						//玩家数量
};

struct MSG_D2CS_Update_GroupOptions
{
	CT_DWORD		dwGroupID;						//群ID
	CT_DWORD		dwDefaultKindID;				//默认游戏
	CT_WORD			wBindUserPlay;					//是否绑定成员进入游戏
};

struct MSG_D2CS_Update_GroupRoom_Cfg
{
	CT_DWORD		dwGroupID;						//群ID
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//类型ID
	CT_CHAR			szPRoomCfg[PROOM_JSON_LEN];		//群ID
};

struct MSG_D2CS_Regster_Promoter
{
	CT_DWORD		dwUserID;						//用户ID
	CT_DWORD		dwPromoterID;					//代理ID
};

struct MSG_D2CS_Query_PRoomInfo
{
	CT_CHAR			szGroupIDList[PROOM_JSON_LEN];  //群ID
	CT_DWORD		dwRoomNum;						//房间号
	CT_UINT64		uWebSock;						//web端连接
};

struct MSG_D2CS_Dismiss_PRoom
{
	CT_CHAR			szGroupIDList[PROOM_JSON_LEN];  //群ID
	CT_DWORD		dwRoomNum;						//房间号
	CT_UINT64		uWebSock;						//web端连接
};

struct MSG_D2CS_Tick_User
{
	CT_CHAR			szGroupIDList[PROOM_JSON_LEN];  //群ID
	CT_CHAR			szUserList[PROOM_JSON_LEN];		//玩家ID
	CT_UINT64		uWebSock;						//web端连接
};

struct MSG_D2CS_Apply_Add_Group
{
	CT_DWORD		dwGroupID;						//群组ID
	CT_DWORD		dwUserID;						//用户ID
	CT_UINT64		uWebSock;						//web端连接
};

struct MSG_D2CS_Query_OnlineUser
{
	CT_BYTE			cbType;							//类型
	CT_BYTE 		cbPlatformId;					//平台ID
	CT_UINT64		uWebSock;						//web端连接
};

struct MSG_D2CS_Query_WaitList
{
    CT_UINT64       uWebSock;                       //web端连接
};

struct MSG_D2CS_Query_TableInfo
{
    CT_DWORD        nServerID;                      //游戏服务器ID
    CT_BYTE 		cbPlatformId;					//平台Id
    CT_UINT64       uWebSock;                       //web端连接
};

struct MSG_D2CS_UpdateAndroid
{
	CT_DWORD		dwServerID;						//游戏服务器ID
};

struct MSG_D2CS_UpdateAndroid_PlayGameCount
{
	CT_DWORD		dwServerID;						//游戏服务器ID
	CT_DWORD		dwAndroidCount;					//机器人数量
};

struct MSG_D2CS_Notify_NewMail
{
	CT_DWORD		dwUserID;						//用户ID
};

struct MSG_D2CS_SendSysMsg
{
	CT_DWORD 		dwMsgID;						//消息ID
	CT_CHAR			szMessage[512];					//消息内容
	CT_DWORD		dwInterval;						//时间间隔
	CT_DWORD		dwValidTime;					//截至时间
	CT_BYTE 		cbPlatformId;					//平台ID
};

struct MSG_D2CS_RemoveSysMsg
{
	CT_DWORD 		dwMsgID;						//消息ID
	CT_BYTE         cbPlatformId;                   //平台ID
};

struct MSG_D2CS_UserStatus
{
	CT_DWORD		dwUserID;						//用户ID
	CT_BYTE			cbStatus;						//状态
};

struct MSG_D2CS_ExchangeStatus
{
	CT_BYTE			cbExchangeType;					//兑换类型(2:支付宝 3:银联)
	CT_BYTE			cbStatus;						//状态
};

struct MSG_D2CS_ClientChannel_Domain
{
	CT_DWORD		dwChannelID;					//渠道ID
	CT_BYTE			cbLocked;						//是否锁定
	CT_DWORD		dwShowExchangeCond;				//显示兑换的条件
	CT_CHAR			szDomain[CHANNEL_DOMAIN_LEN];	//主页
	CT_CHAR 		szDomain2[CHANNEL_DOMAIN_LEN];	//主页2
};

struct MSG_D2CS_Update_StockInfo
{
	CT_DWORD		dwServerID;						//服务器ID
	CT_LONGLONG		llTotalStockLowerLimit;			//总库存下限
	CT_LONGLONG		llTodayStockHighLimit;			//今日库存上限
	CT_WORD			wSystemAllKillRatio;			//系统必杀概率
	CT_WORD			wChangeCardRatio;				//百人牛牛换闪牌概率
};

struct MSG_D2CS_Tick_FishUser
{
	CT_DWORD 		dwUserID;						//玩家ID
};

struct MSG_D2CS_Reload_BrGameRatio
{
    CT_DWORD        dwServerID;
};

struct MSG_D2CS_Reload_BrGameBlackList
{
    CT_DWORD        dwServerID;
};

struct MSG_D2CS_Set_FishDiankong
{
    MSG_D2CS_Set_FishDiankong()
    : dwGameIndex(0)
    , dwUserID(0)
    , iDianKongZhi(0)
    , llDianKongFen(0)
    , llCurrDianKongFen(0)
    , nBloodPoolStateValue(0)
    , nBaseCapPro(0)
    , bySource(0) { }
	//游戏gameid*10000+kindid*100+roomkindid
	CT_DWORD 		dwGameIndex;
	CT_DWORD		dwUserID;
	//点控作弊值,由后台提供
	CT_INT32			iDianKongZhi;
	//点控分数,由后台提供
	CT_LONGLONG  llDianKongFen;
	//当前点控分
	CT_LONGLONG   llCurrDianKongFen;
	//血池状态值,由GameServer根据点控作弊值在点控配置表中匹配得到
	CT_INT32			nBloodPoolStateValue;
	//基础捕获概率,由GameServer根据点控作弊值在点控配置表中匹配得到
	CT_INT32          nBaseCapPro;
	//数据来源  1:来源于后台 2:来源于数据库 3:来源于游戏服务器自动点控
	CT_BYTE			bySource;
};

struct MSG_D2CS_Reload_ZjhDepot_Ratio
{
	CT_BYTE			cbDepotID;					//牌库ID
	CT_BYTE			cbCard1Ration;				//散牌概率
	CT_BYTE			cbCard2Ration;				//对子概率
	CT_BYTE			cbCard3Ration;				//顺子概率
	CT_BYTE			cbCard4Ration;				//金花概率
	CT_BYTE			cbCard5Ration;				//顺金概率
	CT_BYTE			cbCard6Ration;				//豹子概率
};

//CS->DIP
struct MSG_CS2D_CreatePRoom_PROOM_Succ
{
	CT_DWORD		dwRoomNum;						//房间号
	CT_WORD			wNeedGem;						//需要钻石
	CT_WORD			wCurrUserCount;					//当前人数
	CT_UINT64		uWebSock;						//web端连接
};
struct MSG_CS2D_CreatePRoom_Failed
{
	CT_BYTE			cbErrorCode;					//错误码
	CT_UINT64		uWebSock;						//web端连接
};
struct MSG_CS2D_QueryPRoom_Info
{
	CT_BYTE			cbResult;						//0表示查询成功,1房间号不存在，2房间号不属于当前代理
	CT_BYTE			cbUserCount;
	CT_DWORD		dwUserID[MAX_PLAYER];
	CT_CHAR			szNickName[MAX_PLAYER][NICKNAME_LEN];
	CT_BYTE			cbState[MAX_PLAYER];
	CT_UINT64		uWebSock;
};
struct MSG_CS2D_Dismiss_PRoom
{
	CT_BYTE			cbResult;						//0表示查询成功,1房间号不存在，2房间号不属于当前代理, 3游戏已经开始不能解散
	CT_UINT64		uWebSock;
};
struct MSG_CS2D_Tick_User
{
	CT_BYTE			cbResult;						//0表示查询成功,1玩家不在游戏中，2玩家所在的房间不属于当前代理，3游戏已经开始不能剔人
	CT_UINT64		uWebSock;
};

struct MSG_CS2D_Apply_Add_Group
{
	CT_BYTE			cbResult;						//0表示操作成功，1表示没有找到这个群，2表示已经是群成员， 3已经提交过申请，请等待群主通过， 4.群已经满员
	CT_UINT64		uWebSock;
};

struct MSG_CS2D_Query_OnlineUser
{
	CT_CHAR			szJsonUserList[SYS_NET_SENDBUF_SIZE];
	CT_UINT64		uWebSock;
};

struct MSG_CS2D_Query_WaitList
{
    CT_CHAR			szJsonUserList[2048];
    CT_UINT64		uWebSock;
};

struct MSG_CS2D_Query_TableInfo
{
    CT_CHAR			szJsonUserList[SYS_NET_SENDBUF_SIZE];
    CT_UINT64		uWebSock;
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif