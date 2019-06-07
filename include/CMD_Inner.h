#ifndef CMD_INNER_H_
#define CMD_INNER_H_
#include "CTType.h"
#include "CGlobalData.h"
#include "GlobalStruct.h"
#include "IGameOtherData.h"
#include "CTData.h"
#include "CMD_Dip.h"
#include "GlobalEnum.h"
//#include "../GameServer/FishControl.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//24小时秒数.
#define DAY_24HOUR_SECOND       (86400)

//代理服跟登录服/游戏服绑定数据
struct PS_BindData
{
	CT_DWORD	dwProxyServerID;		//代理服务ID
	CT_DWORD	dwProxyType;			//代理服类型(0 普通类型 1 机器人代理服)
};

//更新玩家的钻石
struct MSG_UpdateUser_Gem
{
	CT_DWORD	dwUserID;
	CT_INT32	nAddGem;
	CT_BOOL		bUpdateToClient;

	MSG_UpdateUser_Gem()
		: dwUserID(0)
		, nAddGem(0)
		, bUpdateToClient(false)
	{
	}
};

//更新玩家的金币
struct MSG_UpdateUser_Score
{
	CT_DWORD			dwUserID;			//用户ID
	CT_LONGLONG			llSourceScore;		//原有金币
	CT_LONGLONG			llAddScore;			//增加金币
	CT_WORD				wRoomKind;			//房间类型
	CT_BOOL				bUpdateToClient;	//同步到客户端
	CT_BOOL				bAndroid;			//是否机器人(用于是否更新帐号库的金币)
	enScoreChangeType	enType;				//变化类型

	MSG_UpdateUser_Score()
		: dwUserID(0)
		, llSourceScore(0)
		, llAddScore(0)
		, wRoomKind(0)
		, bUpdateToClient(false)
		, bAndroid(false)
		, enType(UNKNOW_SCORE_TYPE)
	{

	}
};

//更新玩家的银行金币
struct MSG_UpdateUser_BankScore
{
	CT_DWORD			dwUserID;				//用户ID
	CT_LONGLONG			llAddBankScore;			//增加银行金币
	CT_BOOL				bUpdateToClient;		//同步到客户端

	MSG_UpdateUser_BankScore()
		:dwUserID(0)
		, llAddBankScore(0)
		, bUpdateToClient(false)
	{
		
	}
};

//金币变化记录
struct MSG_Insert_ScoreChange_Record
{
	CT_DWORD			dwUserID;
	CT_LONGLONG			llSourceScore;
	CT_LONGLONG			llSourceBankScore;
	CT_LONGLONG			llChangeScore;
	CT_LONGLONG			llChangeBankScore;
	enScoreChangeType	enType;

	MSG_Insert_ScoreChange_Record()
		:dwUserID(0)
		,llSourceScore(0)
		,llSourceBankScore(0)
		,llChangeScore(0)
		,llChangeBankScore(0)
		,enType(UNKNOW_SCORE_TYPE)
	{

	};
};

//玩家金币场信息
struct MSG_Update_ScoreInfo
{
	MSG_Update_ScoreInfo()
		: dwUserID(0)
		, dwAddRevenue(0)
		, dwAddWinCount(0)
		, dwAddLostCount(0)
		, dwAddDrawCount(0)
		, dwAddFleeCount(0)
		, dwAddPlayTime(0)
		, bIsAndroid(false)
	{
	}

	CT_DWORD		dwUserID;					//用户ID
	CT_DWORD		dwAddRevenue;				//税收
	CT_DWORD        dwAddWinCount;				//赢得次数
	CT_DWORD        dwAddLostCount;				//输的次数
	CT_DWORD        dwAddDrawCount;				//和的次数
	CT_DWORD        dwAddFleeCount;				//逃跑次数
	CT_DWORD		dwAddPlayTime;				//游戏时长
	CT_BOOL			bIsAndroid;					//是否机器人
};

//玩家显示兑换
struct MSG_Update_ShowExchange
{
	CT_DWORD		dwUserID;					//用户ID
	CT_BYTE			cbShowExchange;				//显示对换
};

//////////////////////////////////////////////////////////////////////////////////
//代理服务器和中心服通讯的消息定义
#define MSG_PCS_MAIN				1

//服务注册
#define SUB_P2CS_REGISTER				100				//注册代理
#define SUB_P2CS_CREATE_PRIVATE_ROOM	101				//创建私人房间
#define SUB_P2CS_REGISTER_HALL_USER		102				//增加大厅用户
#define SUB_P2CS_UNREGISTER_HALL_USER	103				//减少大厅用户
#define SUB_P2CS_HEART_BEAT				104				//代理服和中心服之间的玩家心跳

#define SUB_CS2P_LOGIN_INSERT			120				//登录列表
#define SUB_CS2P_LOGIN_FINISH			121				//登录完成
#define SUB_CS2P_LOGIN_REMOVE			122				//登录删除

#define SUB_CS2P_SERVER_INSERT			123				//房间列表
#define SUB_CS2P_SERVER_FINISH			124				//房间完成
#define SUB_CS2P_SERVER_REMOVE			125				//房间删除

#define SUB_CS2P_CREATE_PRIVATE_ROOM	127				//创建私人房间返回
#define SUB_CS2P_CREATE_PRIVATE_FAIL	128				//创建私人房间错误
#define SUB_CS2P_SYN_USER_ONLINE_COUNT	129				//同步在线人数
#define SUB_CS2P_UPDATE_GS_STATE		130				//更新游戏服状态

#define SUB_CS2P_CUSTOMER_INSERT		131				//客服服列表
#define SUB_CS2P_CUSTOMER_FINISH		132				//客服服完成
#define SUB_CS2P_CUSTOMER_REMOVE		133				//客服服删除

#define SUB_CS2P_ADD_GROUP_PROOM        140             //增加群房间
#define SUB_CS2P_REMOVE_GROUP_PROOM     141             //删除群房间

#define SUB_CS2P_USER_EXCHARGE			150				//玩家充值钻石


struct  CMD_ProxyServer_Info
{
	CT_DWORD	dwServerID;							//服务器ID
	CT_CHAR		szServerIP[STRING_IP_LEN];			//IP地址
	CT_CHAR		szServerName[SERVER_NAME_LEN];		//服务名称
};

//增加或者减少一个玩家
struct MSG_P2CS_IncrOrDecr_Hall_User
{
	CT_WORD		wGameID;
};

//注册一个用户
struct MSG_P2CS_Register_User
{
	CT_DWORD	dwProxyServerID;
	CT_DWORD	dwUserID;
	CT_DWORD    dwParentUserID;
	CT_DWORD	dwGem;
	//CT_WORD		wGameID;
	//CT_BYTE		cbPlayMode;
	CT_DWORD	dwChannelID;
	CT_DWORD	dwClientChannelID;
	CT_BYTE     cbPlatformId;
	CT_BYTE		cbSex;
	CT_BYTE		cbVipLevel;
	CT_BYTE		cbVipLevel2;
	CT_BYTE		cbHeadID;
	CT_BYTE		cbMachineType;
	CT_BYTE		cbShowExchange;
	CT_BYTE     cbProvinceCode;
	CT_LONGLONG llScore;
	CT_LONGLONG	llBankScore;
	CT_DWORD 	dwRecharge;
	CT_CHAR		szNickName[NICKNAME_LEN];
	//CT_CHAR		szHeadUrl[HEAD_URL_LEN];	
};

//注销一个用户
struct MSG_P2CS_UnRegister_User
{
	CT_BYTE		cbPlatformId;
	CT_DWORD	dwProxyServerID;
	CT_DWORD	dwUserID;
};

//给中心服发个心跳消息
struct MSG_P2CS_HeartBeat
{
  	CT_DWORD 	dwUserID;
};

struct MSG_CS2P_CreatePrivateRoom
{
	CT_DWORD	dwUserID;							//玩家ID
	CT_WORD		wGameID;							//游戏ID
	CT_WORD		wKindID;							//类型ID
	CT_DWORD	dwRoomNum;							//创建的房间ID
	CT_UINT64	uClientSock;						//客户端sock地址
};

struct MSG_CS2P_CreatePrivateRoomFail
{
	CT_DWORD	dwUserID;							//玩家ID
	CT_BYTE		cbErrorCode;						//错误码
	CT_UINT64	uClientSock;						//客户端sock地址
};

struct MSG_CS2P_Add_GroupPRoom
{
    CT_DWORD    dwRoomNum;                          //房间号
    CT_DWORD    dwServerID;                         //房间所在的服务器ID
};

struct MSG_CS2P_User_Recharge
{
	CT_DWORD	dwUserID;							//玩家ID
	CT_DWORD	dwRecharge;							//充值数量
};

struct MSG_CS2P_Update_GS_State
{
	CT_DWORD	dwServerID;							//游戏服务器ID
	CT_BYTE		cbState;								//游戏状态

	MSG_CS2P_Update_GS_State()
		:dwServerID(0)
		,cbState(0)
	{
	}
};

//////////////////////////////////////////////////////////////////////////////////
//游戏服务器和中心服通讯的消息定义
#define MSG_GCS_MAIN				2

//服务注册
#define SUB_G2CS_REGISTER				200				//注册游戏服
#define SUB_G2CS_UPDATE_USER			201				//更新服务服人数
#define SUB_G2CS_UPDATE_PROOM			202				//删除房间
#define SUB_G2CS_QUERY_PROOM_INFO		203				//查询房间信息返回
#define SUB_G2CS_DISMISS_PROOM			204				//解散房间返回
#define SUB_G2CS_TICK_USER				205				//剔除玩家返回
#define SUB_G2CS_RETURN_GROUPPROOM_GEM  206				//退还群主钻石
#define SUB_G2CS_UPDATE_USER_GEM		207				//更新玩家钻石
#define SUB_G2CS_UPDATE_USER_SCORE		208				//更新玩家金币
#define SUB_G2CS_UPDATE_USER_SCORE_INFO	209				//更新玩家金币相关信息（输赢率）
#define	SUB_G2CS_GROUPPROOM_CLEARING	210				//群组大局结算信息
#define SUB_G2CS_QUERY_TABLE_INFO       211             //查询桌子信息返回
#define SUB_G2CS_USER_WIN_SCORE			212				//发送玩家赢分消息
#define	SUB_G2CS_USER_PLAY_GAME			213				//玩家玩游戏
#define SUB_G2CS_FISH_FIRE_COUNT        214             //捕鱼炮数
#define SUB_G2CS_SEND_MAIL              215             //发送邮件
#define SUB_G2CS_UPDATE_GROUP_PROOM_USER 216            //更新私人房间成员信息
#define SUB_G2CS_REMOVE_GROUP_PROOM     217             //删除群组私人房间

//cs->gs
#define SUB_C2GS_USER_RECHARGE			220				//玩家充值
#define SUB_C2GS_DISMISS_PROOM			221				//解散房间(弃用)
#define SUB_C2GS_GROUP_INFO				222				//群组信息
#define	SUB_C2GS_GROUP_USRE_INFO		223				//群组成员
#define	SUB_C2GS_ADD_GROUP				224				//增加群
#define	SUB_C2GS_ADD_GROUP_USER			225				//群组增加成员
#define SUB_C2GS_ADD_GROUP_PROOM        2250            //增加公会房间列表
#define SUB_C2GS_REMOVE_GROUP_PROOM     2251            //删除公会房间列表
#define SUB_C2GS_REMOVE_GROUP			226				//删除群组
#define SUB_C2GS_REMOVE_GROUP_USER		227				//群组删除成员
#define	SUB_C2GS_QUERY_PROOM_INFO		228				//查询房间信息
#define SUB_C2GS_DISMISS_PROOM_AGENT	229				//解散房间(代理解散)
#define SUB_C2GS_TICK_USER_AGENT		230 			//剔除玩家
#define SUB_C2GS_UPDATE_GROUP_INFO		231				//更新群设置
#define SUB_C2GS_WXGROUP_INFO			232				//微信群组信息
#define	SUB_C2GS_ADD_WXGROUP_USER		233				//微信群组增加成员
#define SUB_C2GS_REMOVE_WXGROUP_USER	234				//微信群组删除成员
#define SUB_C2GS_UPDATE_USER_VIP		235				//更新用户VIP等级
#define SUB_C2GS_QUERY_TABLE_INFO       236             //查询桌子信息
#define SUB_C2GS_UPDATE_ANDROID			237				//重新加载机器人
#define SUB_C2GS_UPDATE_ANDROID_COUNT	238				//更新机器人上桌人数
#define SUB_C2GS_UPDATE_GS_STATE		239				//更新游戏服务器的状态
#define SUB_C2GS_UPDATE_GS_STOCK		240				//更新游戏服务器库存信息
#define SUB_C2GS_TICK_FISH_USER			241				//剔除捕鱼玩家
#define SUB_C2GS_BRGAME_RATIO           242             //百人游戏概率控制
#define SUB_C2GS_BRGAME_BLACKLIST		243				//百人游戏黑名单
#define SUB_C2GS_FISH_DIANKONG			244             //捕鱼点控
#define SUB_C2GS_ZJH_CARDRATIO			245				//炸金花牌库概率
#define SUB_C2GS_BENEFIT_CFG            246             //更新救济金配置
#define SUB_C2GS_BLACK_CHANNEL          247             //更新黑渠道名单

struct CMD_GameServer_Info
{
	CT_DWORD	dwServerID;								//服务器ID
	CT_WORD		wGameID;								//游戏ID
	CT_WORD		wKindID;								//游戏类型 （标识： 同一款游戏的不同打法）
	CT_WORD		wRoomKindID;							//房间类型（标识：0,房卡房; 1,初级房; 2, 中级房; 3,高级房）
	CT_WORD		wMinUserCount;							//单桌最小人数
	CT_WORD		wMaxUserCount;							//单桌最大人数
	CT_DWORD	dwOnlineCount;							//在线人数
	CT_DWORD	dwFullCount;							//满员人数
	CT_DWORD	dwEnterMinScore;						//进入最小金币
	CT_DWORD	dwEnterMaxScore;						//进入最大金币
	CT_DWORD	dwCellScore;							//基础金币
	CT_CHAR		szPRoomPlayCountCond[PROOM_PARAM_LEN];	//私人房间局数条件
	CT_CHAR		szPRoomNeedGem[PROOM_PARAM_LEN];		//私人房间需要钻石
	CT_CHAR		szPRoomOtherCond[PROOM_PARAM_LEN];		//私人房间的其他条件
	CT_CHAR		szServerIP[STRING_IP_LEN];				//IP地址
	CT_CHAR		szPublicIP[STRING_IP_LEN];				//公网IP地址
	CT_CHAR		szServerName[SERVER_NAME_LEN];			//服务名称
	CT_CHAR		szRoomName[SERVER_NAME_LEN];			//房间名称
	CT_BYTE		cbState;								//服务器状态
};

struct CMD_Update_GS_User
{
	CT_DWORD	dwServerID;							//服务器ID
	CT_WORD		wGameID;							//游戏ID
	CT_WORD		wKindID;							//游戏类型 （标识： 同一款游戏的不同打法）
	CT_WORD		wRoomKindID;						//房间类型（标识：0,房卡房; 1,初级房; 2, 中级房; 3,高级房）
	CT_WORD		wUserCount;							//总人数(包括机器人)
	CT_WORD		wRealUserCount;						//真人在线人数

	CT_DWORD	dwUserID;							//正要更新的玩家ID
	CT_BYTE		cbUpdateMode;						//0: 离开 1:进入 3:机器人进入或者离开(区分机器人不用统计在线人数)
};

struct MSG_G2CS_UpdatePRoom
{
	CT_WORD		wGameID;							//游戏ID
	CT_WORD		wKindID;							//游戏类型
	CT_DWORD	dwRoomNum;							//房间号
	CT_DWORD	dwGroupID;							//群ID
	CT_BYTE		cbGroupType;						//群类型
	CT_BOOL		bRemoveGroupRoom;					//是否删除群房间信息
	CT_BYTE		cbRoomState;						//房间状态
};

//退还自动房间的钻石
struct MSG_G2CS_Return_GroupPRoom_Gem
{
	CT_DWORD		dwGroupID;				//群ID
	CT_DWORD		dwRecordID;				//记录ID
	CT_DWORD		dwMasterID;				//群主ID
	CT_WORD			wReturnGem;				//退还gem
};

//插入群组的私人房
struct MSG_G2CS_GroupPRoom_Clearing
{
	CT_DWORD		dwGroupID;								//群ID
	CT_DWORD		dwRoomNum;								//房间号
	CT_WORD			wGameID;								//游戏ID
	CT_WORD			wKindID;								//类型ID
	CT_BYTE			cbCount;								//数量
	CT_CHAR			szNickName[MAX_PLAYER][NICKNAME_LEN];	//昵称
	CT_INT32		iScore[MAX_PLAYER];
};

//群组房间玩家更新
struct MSG_G2CS_GroupPRoom_Update
{
    MSG_G2CS_GroupPRoom_Update()
    :dwRoomNum(0)
    ,wGameID(0)
    ,wKindID(0)
    ,dwUserID(0)
    ,wChairID(0)
    ,cbHeadId(0)
    ,cbSex(0)
    {
    }

    CT_DWORD		dwRoomNum;								//房间号
    CT_WORD			wGameID;								//游戏ID
    CT_WORD			wKindID;								//类型ID
    CT_DWORD        dwUserID;                               //玩家ID
    CT_WORD         wChairID;                               //椅子ID
    CT_BYTE         cbHeadId;                               //头像ID
    CT_BYTE         cbSex;                                  //性别
    CT_BYTE         cbVip2;                                 //VIP2等级
};

//删除群组私人房间
struct MSG_G2CS_Remove_GroupPRoom
{
    MSG_G2CS_Remove_GroupPRoom()
    :dwRoomNum(0)
    ,wGameID(0)
    ,wKindID(0)
    {

    }
    CT_DWORD		dwRoomNum;								//房间号
    CT_WORD			wGameID;								//游戏ID
    CT_WORD			wKindID;								//类型ID
};

//查询桌子信息
struct MSG_G2CS_Query_Table_Info
{
    CT_CHAR		szTableInfo[SYS_NET_SENDBUF_SIZE];          //房间信息
    CT_UINT64	uWebSock;                                   //Web套接字
};

//发送玩家赢分信息
struct MSG_G2CS_WinScore_Info
{
	CT_CHAR		szNickName[NICKNAME_LEN];					//昵称
	CT_CHAR		szServerName[SERVER_NAME_LEN];				//房间昵称
  	CT_WORD 	wGameID;									//gameid *100 + kindid
  	CT_BYTE 	cbVip2;										//Vip等级
  	CT_BYTE 	cbCardType;									//牌型
	CT_LONGLONG llWinScore;									//输赢分
};

//玩家玩游戏
struct MSG_G2CS_PlayGame
{
	//MSG_G2CS_PlayGame(CT_WORD wGameid, CT_WORD wKindid, CT_WORD	wRoomKindid)
	//	:dwUserID(0)
	//	,wGameID(wGameID)
	//	,wKindID(wKindid)
	//	,wRoomKindID(wRoomKindid)
	//{
	//}
	CT_DWORD	dwUserID;
	CT_WORD		wGameID;
	CT_WORD		wKindID;
	CT_WORD		wRoomKindID;
	CT_DWORD 	dwRevenue;
	CT_DWORD 	dwRecordTime;		//记录时间
    CT_INT32 	iWinScore;			//玩家输赢分
	CT_DWORD 	dwJettonScore;		//玩家下注数额
	CT_BYTE 	cbSex;				//玩家性别（为了增加大额度机器人进财神降临用，机器人数据中心服没有）
	CT_BYTE 	cbHeadID;			//玩家头像ID(同cbSex)
};

//用户充值
struct MSG_C2GS_User_Recharge
{
	CT_DWORD	dwUserID;							//玩家充值
	CT_LONGLONG	llRecharge;							//充值数量
	CT_DWORD 	dwRechargeTempCount;				//充值次数(指本次登录三个小时以来的充值次数)
	CT_BYTE		cbType;								//类型（1:金币 2:钻石）
};

//用户vip等级变化
struct MSG_C2GS_Update_UserVip
{
	CT_DWORD	dwUserID;							//用户ID
	CT_BYTE		cbVipLevel;							//用户VIP等级
	CT_BYTE		cbVipLevle2;						//用户VIP2等级
};

//解散房间
struct MSG_C2GS_DismissPRoom
{
	CT_DWORD	dwServerID;							//服务器ID
	CT_DWORD	dwRoomNum;							//房间号
};

//更新群信息
struct MSG_C2GS_Update_GroupInfo
{
	CT_DWORD	dwGroupID;							//群组ID
	CT_DWORD	dwBindUserPlay;						//绑定成员进入游戏
};

//删除群
struct MSG_C2GS_Remove_Group
{
	CT_DWORD	dwGroupID;
};

//删除群成员
struct MSG_C2GS_Remove_GroupUser
{
	CT_DWORD	dwGroupID;
	CT_DWORD	dwUserID;
};

//增加群组的私人房间
struct MSG_C2GS_Add_GroupPRoom
{
    CT_DWORD    dwGroupID;                          //群组ID
    CT_DWORD    dwRoomNum;                          //房间ID
};

//删除群组的私人房间
struct MSG_C2GS_Remove_GroupPRoom
{
    CT_DWORD    dwRoomNum;                          //房间ID
};

//查询房间信息
struct MSG_C2GS_Query_PRoomInfo
{
	CT_DWORD	dwRoomNum;
	CT_UINT64	uWebSock;
};

//微信群信息
struct MSG_C2GS_WXGroupInfo
{
	CT_DWORD	dwGroupID;
	CT_DWORD	dwBindUserPlay;
	CT_DWORD	dwUserCount;
};

//增加或者删除群微信群成员
struct MSG_C2GS_AddOrRemove_WXGroupUser : public MSG_D2CS_AddOrRemove_GroupUser
{
};

//解散群房间
struct MSG_C2GS_DismissPRoom_ForAgent   //给代理用的
{
	CT_DWORD	dwGroupID;
	CT_DWORD	dwRecordID;
	CT_BYTE		cbGroupType;
	CT_DWORD	dwMasterID;
	CT_DWORD	dwRoomNum;
	CT_WORD		wPlayCount;
	CT_WORD		wGameID;
	CT_WORD		wKindID;
	CT_BYTE		cbDismissType;			//(2： 管理员解散， 4：超时解散)
	CT_UINT64	uWebSock;
};

struct MSG_C2GS_TickUser_ForAgent		//给代理用
{
	CT_CHAR		szUserList[PROOM_JSON_LEN];
	CT_UINT64	uWebSock;
};

struct MSG_C2GS_DissmissPRoom_TimeOut	//超时解散房间
{
	CT_DWORD	dwGroupID;
	CT_DWORD	dwRecordID;
	CT_BYTE		cbGroupType;
	CT_DWORD	dwMasterID;
	CT_DWORD	dwRoomNum;
	CT_WORD		wPlayCount;
	CT_WORD		wGameID;
	CT_WORD		wKindID;
};

struct MSG_C2GS_Query_Table_Info	    //桌子信息查询
{
	CT_BYTE 	cbPlatformId;
    CT_UINT64	uWebSock;
};

//更新机器人上桌人数
struct MSG_C2GS_UpdateAndroid_PlayCount
{
	CT_DWORD	dwAndroidCount;			//机器人上桌人数

};

//更新游戏服务器的状态
struct MSG_C2GS_Update_GameServer_State
{
	CT_BYTE		cbState;
};

//更新服务器的库存信息
struct MSG_C2GS_Update_GameServer_Stock
{
	CT_LONGLONG		llTotalStockLowerLimit;			//总库存下限
	CT_LONGLONG		llTodayStockHighLimit;			//今日库存上限
	CT_WORD			wSystemAllKillRatio;			//系统必杀概率
	CT_WORD			wChangeCardRatio;				//百人牛牛换闪牌概率
};

//剔除捕鱼游戏服务器的玩家
struct MSG_C2GS_Tick_FishUser
{
	CT_DWORD 		dwUserID;
};

struct CMD_GameServer_OnlineCount
{
    CMD_GameServer_OnlineCount()
    :dwServerID(0)
    ,dwOnlineCount(0)
    {}

    ~CMD_GameServer_OnlineCount()
    {}

	CT_DWORD	dwServerID;								//服务器ID
	CT_DWORD	dwOnlineCount;							//在线人数
};

//////////////////////////////////////////////////////////////////////////////////
//登录服务器和中心服通讯的消息定义
#define MSG_LCS_MAIN							3

//服务注册
#define SUB_L2CS_REGISTER						300				//注册登录服
#define SUB_L2CS_ADD_USER_GEM					301				//更新用户钻石
#define SUB_L2CS_ADD_USER_SCORE					302				//更新用户金币
#define SUB_L2CS_ADD_USER_BANKSCORE				303				//更新用户银行金币
#define SUB_L2CS_ADD_GROUP_USER                 304             //添加群组成员

#define SUB_CS2L_UPDATE_SHOWEXCHANGE			350				//通知登录服某个渠道是否显示兑换按扭
#define SUB_CS2L_UPDATE_CHANNLE_PRESENT_SCORE 	351				//更新渠道注册和绑定手机赠送金币
#define SUB_CS2L_UPDATE_EXCHANGE_INFO			352				//兑换相关信息
#define SUB_CS2L_UPDATE_BENEFIT_CONFIG          353             //更新救济金配置
#define SUB_CS2L_UPDATE_SMS_PID					354				//更新短信平台

struct CMD_LoginServer_Info
{
	CT_DWORD	dwServerID;							//服务器ID
	CT_BYTE 	cbMainServer;						//是否主登陆服务器(主服用于兑换)
	CT_CHAR		szServerIP[STRING_IP_LEN];			//IP地址
	CT_CHAR		szServerPubIP[STRING_IP_LEN];		//公网IP地址
	CT_CHAR		szServerName[SERVER_NAME_LEN];		//服务名称
};

struct MSG_L2CS_AddGroupUser
{
    CT_DWORD    dwUserID;                           //用户ID
    CT_DWORD    dwParentUserID;                     //上级用户ID
    CT_BYTE     cbSex;                              //用户性别
    CT_BYTE     cbHeadId;                           //头像ID
    CT_BYTE     cbVip2;                             //vip2
};

//////////////////////////////////////////////////////////////////////////////////
//代理服务器和登录服通讯的消息定义
#define MSG_PLS_MAIN				4

#define SUB_P2LS_BIND				400				//绑定服务器
#define SUB_P2LS_USER_LOGOUT		401				//玩家登出

#define	SUB_LS2P_USER_LOGIN_SUCC	420				//玩家登录成功
#define	SUB_LS2P_KICK_USER			421				//剔除用户

struct MSG_P2LS_UserLogout
{
	CT_DWORD	dwUserID;							//玩家登出
	CT_DWORD	dwProxyServerID;					//代理服ID
};

struct MSG_LS2P_UserLoginSucc
{
	CT_DWORD	dwUserID;							//玩家ID
	CT_DWORD	dwChannelID;						//渠道ID
    CT_DWORD	dwClientChannelID;					//客户端的渠道ID
	CT_LONGLONG	llScore;							//玩家金币
	CT_LONGLONG	llBankScore;						//玩家银行金币
	CT_DWORD	dwRecharge;							//玩家总充值
	CT_CHAR		szNickName[NICKNAME_LEN];			//玩家昵称
	//CT_WORD		wGameID;							//游戏ID
	CT_BYTE 	cbPlatformId;						//平台ID
	CT_BYTE		cbSex;								//性别
	CT_BYTE		cbVipLevel;							//VIP等级
	CT_BYTE		cbVipLevel2;						//VIP2等级
	CT_BYTE		cbHeadID;							//玩家头像
	CT_BYTE	    cbMachineType;						//设备类型
	CT_BYTE		cbShowExchange;						//是否显示兑换
	CT_BYTE		cbProvinceCode;						//省份代号
	CT_UINT64	dwNetAddr;							//客户端socket
};

struct MSG_LS2P_KickUser
{
	CT_DWORD	dwUserID;							//被踢玩家ID
	//CT_WORD		wGameID;							//游戏ID
	CT_UINT64	dwNetAddr;							//客户端socket
    CT_BOOL     bSendToClient;                      //是否发送消息给客户端
};

//////////////////////////////////////////////////////////////////////////////////
//代理服务器和游戏服通讯的消息定义
#define MSG_PGS_MAIN				5

#define SUB_P2GS_BIND				500				//绑定服务器
#define SUB_P2GS_USER_OFFLINE		501				//通知GS玩家断开连接

#define SUB_GS2P_ADD_USER			510				//游戏服务器往PS添加玩家
#define	SUB_GS2P_DEL_USER			511				//游戏服务器往PS删除玩家
#define SUB_GS2P_CLOSE_USER_CONNECT 512				//游戏服务器主动关闭玩家连接


struct MSG_P2GS_UserOffLine
{
	CT_DWORD	dwUserID;
};

//往proxy添加玩家
struct MSG_GS2P_AddGameUserInfo
{
	CT_DWORD	dwUserID;
	CT_DWORD	dwServerID;
	CT_UINT64	uClientNetAddr;
};

//往proxy删除玩家
struct MSG_GS2P_DelGameUser
{
	CT_DWORD	dwUserID;
};

//关闭玩家连接
struct MSG_GS2P_CloseConnect
{
	CT_DWORD	dwUserID;
};

//////////////////////////////////////////////////////////////////////////////////
//代理服务器和所有服务器通讯的消息定义
#define MSG_PROXY_MAIN						10
#define SUB_PROXY_FASTSENDMSGTOC			1001
#define SUB_PROXY_FASTSENDMSGTOC_NODATA		1002
#define SUB_PROXY_CLOSEUSERSOCKET			1003
#define SUB_PROXY_FASTSENDMSGTOC_BY_USERID	1004
#define SUB_PROXY_FASTSENDMSGTO_ANDROID_C	1005

//关闭玩家socket.
struct Proxy_CloseUserSocket
{
	CT_UINT64	uUserSocketAddr;					//socket地址.
};

//////////////////////////////////////////////////////////////////////////////////
//游戏服务器和数据库服务器
#define MSG_GDB_MAIN						    11
#define SUB_G2DB_UPDATE_USER_GEM			    1101		//更新玩家的钻石
#define SUB_G2DB_PROOM_PALYCOUNT_RECORD		    1102		//保存私人房的每小局信息
#define SUB_G2DB_PROOM_GAMEROUND_RECORD		    1103		//保存私人房的大局信息
#define SUB_G2DB_PROOM_PLAYBACK_RECORD		    1104		//保存回放二进制流
#define SUB_G2DB_PROOM_GAMEROUND_COUNT		    1105		//保存大局数量
#define SUB_G2DB_PROOM_GAME_WINLOST			    1106		//保存输赢记录
#define SUB_G2DB_PROOM_INSERT_GAMEROUND		    1107		//插入私人房大局信息
#define SUB_G2DB_GEM_CONSUME				    1108		//钻石消耗记录
#define	SUB_G2DB_RETURN_GROUPPROOM_GEM		    1109		//退还群组开房的钻石
#define SUB_G2DB_GROUP_PROOM_RECORD			    1110		//群组开房记录
#define	SUB_G2DB_RETURN_WXGROUPPROOM_GEM	    1111		//退还微信群组开房的钻石
#define SUB_G2DB_WXGROUP_PROOM_RECORD		    1112		//微信群组开房记录
#define SUB_G2DB_UPDATE_USER_SCORE			    1113		//更新玩家的金币
#define	SUB_G2DB_UPDATE_BENEFIT_INFO		    1114		//更新玩家救济金信息
#define SUB_G2DB_UPDATE_USER_SCORE_INFO		    1115		//更新玩家金币场信息
#define SUB_G2DB_RECORD_DRAW_INFO			    1116		//记录金币场基本信息
#define SUB_G2DB_SET_USER_SCORE				    1117		//设置玩家的金币
#define SUB_G2DB_USER_ENTER_ROOM			    1118		//用户进入房间记录
#define SUB_G2DB_USER_LEAVE_ROOM			    1119		//用户离开房间记录
#define SUB_G2DB_ANDROID_STATUE				    1120		//机器人上桌或者下桌状态
#define SUB_G2DB_INSERT_REPORT				    1121		//插入举报信息
#define SUB_G2DB_UPDATE_GOODCARD_INFO		    1122		//更新好牌数据
#define SUB_G2DB_SAVE_USER_DIANKONG_DATA   	    1123   		//保存用户点控数据
#define SUB_G2DB_READ_USER_DIANKONG_DATA   	    1124		//读取用户点控数据
#define SUB_G2DB_DEL_USER_DIANKONG_DATA         1125    	//删除玩家点控数据
#define SUB_G2DB_SAVE_FISH_TOTALWINLOSESCORE    1126 		//保存捕鱼服务器的当日总吸分 和 当日总放分
#define SUB_G2DB_SAVE_FISH_STATISTICSINFO	    1127 		//保存鱼种信息
#define SUB_G2DB_SAVE_FISH_CONTROL_DATA         1128 		//保存捕鱼控制数据
#define SUB_G2DB_SAVE_FISH_CURRENT_KUCUN        1129		//保存捕鱼当前库存值
#define SUB_G2DB_UPDATE_USER_SCORE_DATA         1130        //更新玩家的金币汇总信息
#define SUB_G2DB_SAVE_PLAYER_FISH_INFO          1131        //保存玩家捕鱼信息
#define SUB_G2DB_RECORD_HBSL_INFO				1132		//记录红包扫雷信息
#define SUB_G2DB_BLACK_LIST_USER                1133        //黑名单操作
#define SUB_G2DB_SAVE_MISS_INFO                 1134        //保存极速捕鱼的miss信息
#define SUB_G2DB_READ_USER_MISS_INFO			1135 		//读取极速捕鱼玩家的Miss信息

#define SUB_DB2G_PROOM_INSERT_GAMEROUND		    1150		//插入私人房大局信息返回
#define SUB_DB2G_USER_DIANKONG_DATA			    1151		//DB把读取到的用户点控数据返回给GS
#define SUB_DB2G_USER_MISS_DATA					1152        //DB把读取到的 MISS数据返给GS

//更新玩家的钻石
/*struct MSG_G2DB_UpdateUser_Gem
{
	CT_DWORD	dwUserID;
	CT_INT32	nAddGem;
};

//更新玩家的金币
struct MSG_G2DB_UpdateUser_Score
{
	CT_DWORD	dwUserID;
	CT_LONGLONG	dwScore;
};*/

//插入玩家消耗钻石记录
struct MSG_G2DB_Gem_Consume
{
	CT_WORD		wGameID;
	CT_WORD		wKindID;
	CT_DWORD	dwRoomNum;
	CT_DWORD	dwUserID;
	CT_DWORD	dwConsumeGem;
	CT_DWORD	dwRemainGem;
};

//保存私人房的小局消息
struct MSG_G2DB_PRoom_PlayCountRecord
{
	CT_UINT64	uGameRoundID;					//大局ID
	CT_WORD		wCurrPlayCount;					//小局ID
	CT_DWORD	dwUserID;						//用户ID
	CT_WORD		wChairID;						//椅子ID
	CT_LONGLONG llScore;						//用户分
	CT_CHAR		szNickName[NICKNAME_LEN];		//用户昵称
	CT_CHAR		szEndTime[TIME_LEN];			//结果时间	
};

//保存私人房间的大局消息
struct MSG_G2DB_PRoom_GameRoundRecord
{
	CT_UINT64		uGameRoundID;				//大局ID
	CT_BYTE			uCount;						//用户数量
	CT_DWORD		dwArrUserID[10];			//用户ID
	CT_LONGLONG		llArrScore[10];				//用户分数
	CT_CHAR			szNickname[10][NICKNAME_LEN]; //用户昵称
 	CT_CHAR			szEndTime[TIME_LEN];		//结束时间
};

//保存玩家的输赢记录
struct MSG_G2DB_WinLostRecord
{
	CT_WORD			wGameID;					//游戏ID
	CT_BYTE			uCount;						//用户数量
	CT_DWORD		dwArrUserID[10];			//用户ID
	CT_BYTE			cbWinCount[10];				//用户赢局数
	CT_BYTE			cbLostCount[10];			//用户输局数
	CT_BYTE			cbDrawCount[10];			//用户平局数
	CT_DWORD		dwPlayTime;					//游戏时间
	CT_CHAR			szEndTime[TIME_LEN];		//结束时间
};

//录像的二进制流
struct MSG_G2DB_PRoom_PlaybackRecord
{
	CT_UINT64		uGameRoundID;				//大局ID
	CT_WORD			wPlayCount;					//小局ID
	CT_UINT32		uBinaryLen;					//数据长度
	CT_CHAR			szEndTime[TIME_LEN];		//结束时间
	CT_CHAR			szBinary[SYS_NET_SENDBUF_SIZE];		//先按10K保存
};

//保存大局数量
struct MSG_G2DB_PRoom_GameRoundCount
{
	CT_DWORD 		dwUserID;
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_DWORD		dwGameRoundCount;
};

//插入大局信息
struct MSG_G2DB_PRoom_InsertGameRound
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_DWORD		dwRoomNum;
	CT_DWORD		dwUserID;
	CT_DWORD 		dwTableID;
	CT_CHAR			szParam[PROOM_PARAM_LEN];
};

//退还自动房间的钻石
struct MSG_G2DB_Return_GroupPRoom_Gem
{
	CT_DWORD		dwGroupID;				//群ID
	CT_DWORD		dwRecordID;				//记录ID
	CT_DWORD		dwMasterID;				//群主ID
	CT_WORD			wReturnGem;				//退还gem
};

//自动房间信息记录
struct MSG_G2DB_Record_GroupPRoom
{
	CT_DWORD		dwGroupID;				//群ID
	CT_DWORD		dwRecordID;				//记录ID
	CT_UINT64		uGameRoundID;			//大局ID
};

//退还微信群组房间的钻石
struct MSG_G2DB_Return_WXGroupPRoom_Gem
{
	CT_DWORD		dwGroupID;				//群ID
	CT_DWORD		dwRecordID;				//记录ID
};

//微信群组房间信息记录
struct MSG_G2DB_Record_WXGroupPRoom
{
	CT_DWORD		dwGroupID;				//群ID
	CT_DWORD		dwRecordID;				//记录ID
	CT_UINT64		uGameRoundID;			//大局ID
};

//更新救济金信息
struct MSG_G2GB_Update_BenefitInfo
{
	CT_DWORD		dwUserID;				//玩家ID
	CT_BYTE			cbRewardCount;			//领取次数
	//CT_DWORD		dwSourceGem;			//原有钻石(作记录用)
	CT_LONGLONG		llSourceScore;			//原有金币（作记录用）
	CT_DWORD		dwRewardScore;			//奖励金币（作记录用）
	CT_DWORD        dwLastTime;             //最后领取时间
};

//插入大局信息返回
struct MSG_DB2G_PRoom_InsertGameRound
{
	CT_DWORD 		dwTableID;
	CT_UINT64		uGameRoundID;
};

//记录金币场玩家金币信息
struct MSG_G2DB_Record_Draw_Score
{ 
	CT_DWORD		dwUserID;					//玩家id
	CT_WORD			wChairID;					//玩家椅子id
	CT_LONGLONG		llSourceScore;				//源金币
	CT_INT32		iScore;						//总输赢金币
	CT_INT32		iAreaJetton[4];				//区域下注数
	CT_DWORD		dwRevenue;					//税收金额
	CT_BYTE			cbIsbot;					//是否是机器人
	CT_CHAR			szLocation[USER_LOCATE_LEN];//位置信息
};

//记录金币场基本信息
struct MSG_G2DB_Record_Draw_Info
{
	CT_WORD			wGameID;			//游戏ID
	CT_WORD			wKindID;			//游戏子ID
	CT_WORD			wRoomKindID;		//游戏
	CT_WORD			wTableID;			//桌子ID
	CT_WORD			wUserCount;			//游戏参与人数
	CT_WORD			wAndroidCount;		//机器人数
	CT_DWORD		dwBankerUserID;		//庄家ID
	CT_DWORD		dwServerID;			//服务器ID
	CT_LONGLONG		llStock;			//库存值(包括机器人库存值)
	CT_LONGLONG		llTodayStock;		//今日库存值
	CT_LONGLONG		llAndroidStock;		//机器人库存值
	CT_WORD 		wSystemAllKillRatio;//系统必杀概率(针对百人牛牛)
	CT_WORD 		wChangeCardRatio;	//换牌概率(针对百人牛牛)
	CT_BYTE			cbAreaCardType[5];	//区域牌型
	CT_DWORD		dwPlayTime;			//游戏时长
	CT_DWORD 		dwRecordTime;		//记录时间
    CT_DWORD        dwRecordLen;        //回放记录长度
};

//记录红包信息
struct  MSG_G2DB_Record_HBSL_Info : public RecordHBSLInfo
{
    MSG_G2DB_Record_HBSL_Info()
    : wUserCount(0), wRoomKindID(0) {}

    CT_WORD         wUserCount;
    CT_WORD 		wRoomKindID;
};

//记录抢红包信息
struct MSG_G2DB_Record_HBSL_GrabInfo : public RecordHBSLGrabInfo
{

};

//玩家进入房间记录
struct MSG_G2DB_User_EnterRoom
{
	CT_DWORD		dwUserID;
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_WORD			wRoomKindID;
	CT_DWORD		dwServerID;
};

//机器人状态
struct MSG_G2DB_Android_Status
{
	CT_DWORD	dwAndroidUserID;		//机器人ID
	CT_BYTE		cbStatus;				//机器人状态
};

//举报信息
struct MSG_G2DB_ReportInfo
{
	CT_DWORD 	dwUserID;				//举报人id
	CT_DWORD 	dwBeReportUserID;		//被举报人id
};

//好牌数据
struct MSG_G2DB_GoodCard_Info
{
    CT_DWORD    dwUserID;               //用户ID
	CT_WORD 	wGameID;				//游戏ID
	CT_WORD 	wKindID;				//类型ID
	CT_DWORD 	dwPlayCount;			//某个游戏的局数
	CT_BYTE		cbGoodCardParam;		//某个游戏的好牌参数
};

//玩家金币数据量
struct MSG_G2DB_ScoreData
{
    CT_DWORD    dwUserID;               //用户ID
    CT_WORD 	wGameID;				//游戏ID
    CT_WORD 	wKindID;				//类型ID
    CT_WORD     wRoomKindID;            //房间类型
    CT_DWORD 	dwWinCount;				//赢的次数
    CT_DWORD 	dwLostCount;			//输的次数
    CT_LONGLONG llWinScore;             //赢分
    CT_LONGLONG llLostScore;            //输分
    CT_DWORD    dwRevenue;              //税收
};

//读取玩家的点控数据
struct MSG_GS2DB_ReadDianKongData
{
	CT_DWORD dwUserID;
	CT_DWORD dwGameIndex;

    MSG_GS2DB_ReadDianKongData()
    {
        dwUserID = 0;
        dwGameIndex = 0;
    }
};

//删除玩家的点控数据
struct MSG_GS2DB_DelUserDianKongData
{
	CT_DWORD dwUserID;
};

//玩家的黑名单操作
struct MSG_GS2DB_BlackList_User
{
    CT_DWORD        dwServerID;                         //服务器ID
    CT_DWORD 		dwUserID;							//黑名单用户ID
    CT_WORD 		wUserLostRatio;						//用户输的概率
    CT_DWORD    	dwControlScore;                     //控制的下注额度(对百人游戏有效)
};

//23-30 35-39 目前只统计这13种大鱼
#define TONG_JI_BIG_FISH_COUNT 11
#define BASE_PRO_MAX_COUNT 60

struct sBigFishInFo
{
    CT_LONGLONG llPlayerGetScore;
    CT_LONGLONG llPlayerLostScore;
};

struct tagBigInfo
{
    CT_BYTE cbFishType;
    sBigFishInFo bfInfo;
};

struct tagBaseProInfo
{
    CT_DWORD dwBasePro;
    CT_DWORD dwCheckCount;
};

//记录捕鱼的当日总吸分和当日总放分
struct MSG_GS2DB_FishTotalWinLoseScore
{
	int serverID;
	CT_LONGLONG llTotalWinScore;
	CT_LONGLONG llTotalLoseScore;
	int nYear;
	int nMonth;
	int nDay;
	CT_LONGLONG llTodayTotalKuCun;
	CT_LONGLONG llTodayTotalTax;
	CT_LONGLONG llTodayDianKongWinLoseTotalScore;
	CT_DWORD  dwTotalEnterCount;
	CT_DWORD  dwTotalWinCount;
	CT_DWORD  dwTotalPoChanCount;
	CT_DWORD  dwTotalRewardCount;
	CT_DWORD  dwTotalXiFenCount;
    tagBigInfo bfInfo[TONG_JI_BIG_FISH_COUNT];
    CT_BYTE    cbbfInfoCount;
    tagBaseProInfo proInfo[BASE_PRO_MAX_COUNT];
    CT_BYTE    cbproInfoCount;

	MSG_GS2DB_FishTotalWinLoseScore()
	{
		memset(bfInfo, 0, sizeof(bfInfo));
		memset(proInfo, 0, sizeof(proInfo));
		cbbfInfoCount = 0;
		cbproInfoCount = 0;
	}
};

//记录玩家的捕鱼信息
struct MSG_GS2DB_PlayerFishInfo
{
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
};

//鱼种统计信息
struct MSG_GS2DB_FishStatisticsInfo
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
	//鱼的名称
	CT_CHAR        strFishName[128];
	//配置死亡几率
	CT_INT32        nCfgDeathPro;
	//理论死亡几率
	CT_INT32        nLiLunDeathPro;

	MSG_GS2DB_FishStatisticsInfo()
	{
		memset(strFishName, 0, 128);
	}
};

struct MSG_GS2DB_SaveFishControlData
{
	CT_LONGLONG llTotalTax;
	CT_LONGLONG llTotalKuCun;
	CT_LONGLONG llTotalLoseScore;
	CT_LONGLONG llTotalWinScore;
	CT_LONGLONG llBloodPoolDianKongWinLose;
	int 		nServerID;
	int 		nKunCunStatus;
};

struct MSG_GS2DB_SaveFishKuCun
{
	int serverID;
	CT_LONGLONG llCurKuCun;
	CT_INT64 llCurTime;
};

struct FishTodayInfo
{
    CT_LONGLONG llTotalWinScore;
    CT_LONGLONG llTotalLoseScore;
    CT_LONGLONG llTodayTotalKuCun;
    CT_LONGLONG llTodayTotalTax;
    CT_LONGLONG llTodayDianKongWinLoseTotalScore;
    CT_DWORD  dwTotalEnterCount;
    CT_DWORD  dwTotalWinCount;
    CT_DWORD  dwTotalPoChanCount;
    CT_DWORD  dwTotalRewardCount;
    CT_DWORD  dwTotalXiFenCount;
};

//极速捕鱼信息类型
typedef enum
{
    //库存信息
    stock_info = 1,
    //系统信息:
    sys_info = 2,
    //当日系统信息
    today_info = 3,
    //miss库信息
    miss_info = 4

}js_fish_info_type;

struct JSFish_Player_Miss_Info
{
    CT_DWORD dwUserID;
    CT_LONGLONG miss;
};

union js_fish_save_info
{
	MSG_GS2DB_SaveFishKuCun save_stock;
	tagJSFishSystemInfo sys_info;
    FishTodayInfo todayinfo;
    JSFish_Player_Miss_Info missInfo;
};

//保存捕鱼玩家当日输赢
struct MSG_GS2DB_SaveFishUserTodayWinLoseScore
{
	CT_DWORD dwUserID;
	CT_INT32 serverID;
	CT_CHAR todayDate[64];
	CT_LONGLONG llWinLoseScore;

	MSG_GS2DB_SaveFishUserTodayWinLoseScore()
	{
		dwUserID = 0;
		serverID = 0;
		memset(todayDate, 0, 64);
		llWinLoseScore = 0;
	}
};

//玩家点控数据
struct MSG_G2DB_User_PointControl
{
	CT_DWORD 		dwGameIndex;        //游戏索引(gameid*10000+kindid*100+roomkindid)
	CT_DWORD 		dwUserID;           //用户id
	CT_INT32		iDianKongZhi;       //点控作弊值
	CT_LONGLONG  	llDianKongFen;      //点控分数
	CT_LONGLONG     llCurrDianKongFen;  //当前点控分
};

//////////////////////////////////////////////////////////////////////////////////
//代理服务器和数据库服务器
#define MSG_PDB_MAIN						12
#define SUB_P2DB_QUERY_PROOM_GAMEROUND		1201		//更新玩家的钻石
#define SUB_P2DB_QUERY_PROOM_PALYCOUNT		1202		//保存私人房的每小局信息
#define SUB_P2DB_QUERY_USER_SCORE_INFO		1203		//查询用户金币信息
#define SUB_P2DB_QUERY_USER_HONGBAO_INFO	1204		//查询玩家红包信息
#define SUB_P2DB_QUERY_GAME_RECORD			1205		//查询游戏记录
#define SUB_P2DB_QUERY_REPORT_GAME_RECORD	1206		//查询举报游戏记录
#define SUB_P2DB_QUERY_NORMAL_GAME_RECORD	1207		//查询正常游戏记录
#define SUB_P2DB_QUERY_HBSL_RECORD          1208        //查询红包扫雷记录
#define SUB_P2DB_QUERY_HBSL_FA_RECORD       1209        //查询红包扫雷发红包记录
#define SUB_P2DB_QUERY_HBSL_WIN_COLOR_PRIZE_RECORD  1210  //查询红包扫雷中彩金记录

//玩家查询大局信息(返回10)
struct MSG_P2DB_Query_PRoom_GameRound
{
	CT_DWORD		dwUserID;						//玩家ID
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//
	CT_UINT64		uClientAddr;					//客户端地址
};

//查询每个大局中每小局的信息
struct MSG_P2DB_Query_PRoom_PlayCount
{
	CT_UINT64		uGameRoundID;					//大局ID
	CT_UINT64		uClientAddr;					//客户端地址
	CT_DWORD		dwUserID;						//查询的用户ID
};


//////////////////////////////////////////////////////////////////////////////////
//中心服务器和数据库服务器
#define MSG_CSDB_MAIN						13
#define	MSG_CS2DB_REGISTER					1300		//注册服务
#define SUB_CS2DB_LOAD_GROUP_INFO			1301		//加载群信息
#define SUB_CS2DB_CREATE_GROUP				1302		//创建群组
#define SUB_CS2DB_ADD_GROUP_USER			1303        //增加群成员
#define SUB_CS2DB_QUERY_BE_ADD_GROUP_USER	13030		//查询被增加成员信息	
#define SUB_CS2DB_DEL_GROUP_USER			1304		//删除群成员
#define SUB_CS2DB_DEL_GROUP					1305		//删除群
#define	SUB_CS2DB_MODIFY_GROUP_INFO			1306		//修改群信息
#define SUB_CS2DB_DEDUCT_GROUP_MASTER_GEM	1307		//扣除群主钻石
#define SUB_CS2DB_GROUP_ROOM_INFO			1308		//群组房间信息
#define SUB_CS2DB_GROUP_CONTRIBUTION		13080		//更新群组的贡献值(以及群组等级)
#define SUB_CS2DB_GROUP_POSITION            13081       //更新群组的职位
#define SUB_CS2DB_GROUP_USER_INFO           13082       //更新群成员信息
#define SUB_CS2DB_GROUP_CHAT                13083       //插入群聊天记录
#define SUB_CS2DB_UPDATE_GROUP_CHAT         13084       //更新群聊天的状态
#define SUB_CS2DB_UPDATE_GROUP_DIR_INCOME   13085       //更新直属成员收入
#define SUB_CS2DB_UPDATE_GROUP_SUB_INCOME   13086       //更新附属成员收入
#define SUB_CS2DB_INSERT_GROUP_EXCHANGE     13087       //插入兑换信息
#define SUB_CS2DB_QUERY_GROUP_EXCHANGE      13088       //查询兑换记录

#define SUB_CS2DB_RETURN_GROUPPROOM_GEM		1309		//返还群主钻石
#define SUB_CS2DB_LOAD_HONGBAO_INFO			1310		//加载红包信息
#define SUB_CS2DB_INSERT_HONGBAO			1311		//插入红包
#define SUB_CS2DB_DELETE_HONGBAO			1312		//删除红包
#define SUB_CS2DB_UPDATE_HONGBAO			1313		//更新红包
#define SUB_CS2DB_LOAD_TASK					1314		//加载任务信息
#define SUB_CS2DB_INSERT_TASK				1315		//插入任务
#define SUB_CS2DB_UPDATE_TASK				1316		//更新任务
#define	SUB_CS2DB_RESET_TASK				1317		//重置日常任务
#define SUB_CS2DB_UPDATE_PCU				1325		//更新最高在线
#define SUB_CS2DB_INSERT_SCORE_CHANGE		1326		//插入金币变化记录
#define SUB_CS2DB_INSERT_TASK_REWARD		1327		//插入任务奖励记录
#define SUB_CS2DB_RELOAD_RECHARGE_CHANNEL	1328		//重新加载充值渠道

#define SUB_CS2DB_UPDATE_WGC_INDEX			1335		//更新财神降临的索引值
#define SUB_CS2DB_INSERT_WGC_DATA			1336		//插入或者更新一场财神降临活动
#define SUB_CS2DB_UPDATE_WGC_DATA			1337		//更新一场财神降临活动
#define SUB_CS2DB_UPDATE_WGC_USERINFO		1338		//插入或者更新财神降临活动的用户信息
#define SUB_CS2DB_UPDATE_WGC_ALLUSERINFO    1339        //更新财神降临活动的用户排名信息
#define SUB_CS2DB_UPADATE_DUOBAO_DATA		1340		//插入或者更新一场一元夺宝
#define SUB_CS2DB_INSERT_DUOBAO_USERINFO    1341        //插入一个用户夺宝信息

#define SUB_CS2DB_UPDATE_REDPACKET_STAGE	1342		//更新红包阶段
#define SUB_CS2DB_REDPACKET_GRAB_INFO 		1343		//抢红包信息

#define SUB_CS2DB_SEND_MAIL                 1345        //发送邮件
#define SUB_CS2DB_UPDAET_USER_SHOWEXCHANGE  1346		//更新用户显示兑换按钮
#define SUB_CS2DB_ADD_USER_GEM				1347		//更新用户钻石
#define SUB_CS2DB_ADD_USER_SCORE			1348		//更新用户金币


#define SUB_DB2CS_SYSTEM_STATUS				1349		//系统参数
#define SUB_DB2CS_GROUP_INFO				1350		//群信息返回
#define SUB_DB2CS_GROUP_USER				1351		//群成员信息返回
#define SUB_DB2CS_QUERY_BE_ADD_GROUP_USER	13510		//查询被增加成员信息返回
#define SUB_DB2CS_DEFAULT_PROOM_CONFIG		1352		//群组开房默认配置
#define SUB_DB2CS_GROUP_PROOM_CONFIG		1353		//群组开房配置
#define SUB_DB2CS_GROUP_INFO_FINISH			1354		//群信息返回完成
#define SUB_DB2CS_CREATE_GROUP				1355		//创建群组返回
#define SUB_DB2CS_DEDUCT_GROUP_MASTER_GEM	1356		//扣除群主钻石返回
#define	SUB_DB2CS_GROUP_ROOM_INFO			1357		//群组房间信息返回
#define SUB_DB2CS_GROUP_LEVEL_CFG           13570       //群组等级配置
#define SUB_DB2CS_GROUP_CHAT_ID             13571       //加载群聊天消息的
#define SUB_DB2CS_GROUP_UNREAD_CHAT         13572       //加载未读聊天消息
#define SUB_DB2CS_GROUP_USER_REVENUE        13573       //加载成员今天总税收
#define SUB_DB2CS_GROUP_USER_DIR_INCOME     13574       //加载成员未结算收入
#define SUB_DB2CS_GROUP_USER_SUB_INCOME     13575       //加载成员未结算收入
#define SUB_DB2CS_GROUP_USER_TOTAL_INCOME   13576       //加载成员总收入
#define SUB_DB2CS_GROUP_EXCHANGE            13577       //查询成员结算记录

#define SUB_DB2CS_WXGROUP_INFO				1358		//微信群组信息返回
#define SUB_DB2CS_WXGROUP_INFO_FINISH		1359		//微信群信息完成

#define SUB_DB2CS_HONGBAO_INFO				1360		//红包信息
#define SUB_DB2CS_INSERT_HONGBAO			1361		//插入红包

#define SUB_DB2CS_SCORE_RANK				1362		//金币排行榜
#define SUB_DB2CS_YESTERDAY_SCORE_RANK		1363		//昨日赚金榜
#define SUB_DB2CS_TODAY_ONLINE_RANK			1364		//今日在线榜
#define SUB_DB2CS_CLEAR_ONLINE_RANK			1365		//清空今日在线榜
#define SUB_DB2CS_TASK_FINISH_RANK			1366		//今日任务完成榜
#define SUB_DB2CS_CLEAR_TASK_FINISH_RANK	1367		//清空今日任务完成榜

#define SUB_DB2CS_TASK_INFO					1370		//任务列表
#define SUB_DB2CS_WEALTH_GOD_CONFIG			1371		//财神降临配置
#define SUB_DB2CS_WEALTH_GOD_REWARD			1372		//财神降临奖励配置
#define SUB_DB2CS_WEALTH_GOD_INDEX          1373        //财神降临场次ID
#define SUB_DB2CS_WEALTH_GOD_HISTORY_DATA   1374        //财神降临的历史记录
#define SUB_DB2CS_WEALTH_GOD_HISTORY_USER   1375        //财神降临的玩家历史记录

#define SUB_DB2CS_DUOBAO_CONFIG				13700		//一元夺宝配置
#define SUB_DB2CS_DUOBAO_DOING_DATA         13701       //一元夺宝正在进行的记录
#define SUB_DB2CS_DUOBAO_DOING_USER         13702       //一元夺宝正在进行的玩家资料
#define SUB_DB2CS_DUOBAO_HISTORY_DATA		13703		//一元夺宝的历史记录
#define SUB_DB2CS_DUOBAO_HISTORY_USER		13704		//一元夺宝的玩家历史记录
#define SUB_DB2CS_DUOBAO_FINISH             13705       //一元夺宝加载完成

#define SUB_DB2CS_REDPACKET_STATUS          13710       //新年红包的状态
#define SUB_DB2CS_REDPACKET_ROOM_CFG		13711		//新年红包房间配置
#define SUB_DB2CS_REDPACKET_INDEX			13712		//新年红包期数
#define SUB_DB2CS_REDPACKET_GRAB_INFO       13713       //新年抢红包信息

#define SUB_DB2CS_PCU_INFO					1379		//最高在线信息
#define SUB_DB2CS_RECHARGE_CHANNEL_INFO		1380		//充值渠道信息
#define	SUB_DB2CS_RECHARGE_PROMOTER_INFO	1381		//代理充值信息（代理的QQ号微信号等）
#define SUB_DB2CS_EXCHANGE_CHANNEL_STATUS	1382		//兑换渠道状态
#define SUB_DB2CS_GENERALIZE_PROMOTER_INFO	1383		//推广渠道信息
#define SUB_DB2CS_CHANNEL_DOMAIN			1384		//客户端渠道的主页信息
#define SUB_DB2CS_QUANMIN_CHANNEL			1385		//全民代理信息
#define SUB_DB2CS_VIP2_CONFIG				1386		//VIP2配置信息
#define SUB_DB2CS_CHANNEL_SHOW_EXCHANGE		1387		//渠道是否兑换
#define SUB_DB2CS_RECHANGE_AMOUNT			1388		//充值渠道金额
#define SUB_DB2CS_EXCHANGE_AMOUNT			1389		//兑换渠道金额
#define SUB_DB2CS_BENEFIT_CONFIG            1390        //救济金配置
#define SUB_DB2CS_PLATFORM_RECHARGE_CFG		1391		//平台的充值信息
#define SUB_DB2CS_CHAT_UPLOAD_URL			1392		//聊天图片上传地址
#define SUB_DB2CS_ANNOUNCEMENT_INFO			1393		//加载公告的信息

struct  CMD_CenterServer_Info
{
	CT_DWORD	dwServerID;							//服务器ID
	CT_CHAR		szServerIP[STRING_IP_LEN];			//IP地址
	CT_CHAR		szServerName[SERVER_NAME_LEN];		//服务名称
};

struct MSG_DB2CS_WXGroupInfo
{
	CT_DWORD	dwGroupID;
	CT_DWORD	dwBindUserPlay;
	CT_DWORD	dwUserCount;
};

struct MSG_CS2DB_CreateGroup
{
	CT_DWORD	dwMasterID;								//群主ID
	CT_BYTE     cbPlatformID;                           //平台ID
	CT_BYTE     cbLevel;                                //公会等级
	CT_BYTE     cbIcon;                                 //公会图标
	CT_BYTE     cbSettleDays;                           //结算天数
	CT_WORD     cbIncomeRate;                           //收益比例
	//CT_CHAR		szNickName[NICKNAME_LEN];				//群主
	CT_CHAR		szGroupName[GROUP_NAME_LEN];			//群名称
	CT_CHAR		szNotice[GROUP_NOTICE_LEN];				//群公告
};

struct MSG_CS2DB_Add_GroupUser
{
	CT_DWORD	dwGroupID;								//群组ID
	CT_DWORD	dwUserID;								//成员ID
	CT_DWORD    dwParentUserID;                         //上级成员ID
	CT_BYTE     cbIncomeRate;                           //收入比例
    CT_WORD     wDeductRate;                            //扣量比例(千分比)
    CT_WORD     wDeductTop;                             //扣量封顶(千分比)
    CT_WORD     wDeductIncre;                           //扣量递增(千分比)
    CT_DWORD    dwDeductStart;                          //扣量起征点(单位:分)
	CT_LONGLONG llIncome;								//玩家收入
	CT_BYTE     cbMemLevel;                             //成员等级
};

struct MSG_CS2DB_Query_BeAdd_GroupUser
{
	CT_DWORD	dwGroupID;								//群组ID
	CT_DWORD	dwUserID;								//玩家ID
	CT_DWORD	dwOperateUserID;						//操作者玩家ID
	CT_BYTE		cbType;									//操作类型
};

struct MSG_CS2DB_Remove_GroupUser
{
	CT_DWORD	dwGroupID;								//群组ID
	CT_DWORD	dwUserID;								//成员ID
};

struct MSG_CS2DB_Remove_Group
{
	CT_DWORD	dwGroupID;								//群组ID
};

struct MSG_CS2DB_Modify_GroupInfo
{
	CT_DWORD	dwGroupID;								//群组ID
	CT_BYTE     cbLevel;                                //群等级
	CT_BYTE     cbIcon;                                 //群图标
	CT_BYTE     cbSettleDays;                           //结算天数
	CT_BYTE     cbIncomeRate;                           //公会红利
	CT_CHAR		szGroupName[GROUP_NAME_LEN];			//群名称
	CT_CHAR		szNotice[GROUP_NOTICE_LEN];				//群公告
};

struct MSG_CS2DB_Update_GroupContribution
{
    CT_DWORD    dwGroupID;                              //群组ID
    CT_DWORD    dwAddContribution;                      //贡献值
};

struct MSG_CS2DB_Update_GroupUserInfo
{
    CT_DWORD    dwGroupID;                              //群ID
    CT_DWORD    dwUserID;                               //用户ID
    CT_BYTE     cbIncomeRate;                           //收入比例
    CT_DWORD    dwLastTakeIncomeDate;                   //最后兑换时间
    CT_DWORD    dwLastIncomeDate;                       //最后产生收入的时间
    CT_WORD     wLinkIncomeDays;                        //连续产生收益的
    CT_LONGLONG llTotalIncome;				            //成员总收入
    CT_CHAR     szRemarks[GROUP_REMARKS_LEN];           //备注
};

//插入玩家的聊天记录
struct MSG_CS2DB_Insert_GroupChat : public tagGroupChat
{
    CT_DWORD    dwGroupID;                              //群ID
    CT_BYTE     cbStatus;                               //状态
};

//更新群主玩家聊天记录的状态
struct MSG_CS2DB_Update_GroupChat
{
    CT_DWORD    dwMsgID;                                //消息ID
    CT_BYTE     cbStatus;                               //状态
};

//更新玩家的收入
struct MSG_CS2DB_Update_GroupIncome
{
    CT_DWORD    dwDateInt;                              //日期
    CT_DWORD    dwUserID;                               //玩家ID
    CT_DWORD    dwParentUserID;                         //父级玩家ID
    CT_DWORD    dwSubUserID;                            //下级玩家ID
    CT_DWORD    dwGroupID;                              //群组ID
    CT_DWORD    dwOriginalIncome;                       //原始收入
    CT_DWORD    dwDeductIncome;                         //扣量后的收入
    CT_WORD     wDeductRate;                            //扣量比例(千分比)
    CT_BYTE     cbCrossDays;                            //是否跨天
};

//插入玩家的群级收入结算记录
struct MSG_CS2DB_Insert_Exchange_GroupIncome
{
    CT_DWORD    dwUserID;                               //用户ID
    CT_DWORD    dwGroupID;                              //群ID
    CT_DWORD    dwTime;                                 //兑换时间
    CT_LONGLONG llIncome;                               //兑换金额
};

//查询结算记录
struct MSG_CS2DB_Query_GroupIncome_Exchange
{
    CT_DWORD    dwUserID;
};

struct MSG_CS2DB_Deduct_GroupMaster_Gem
{
	CT_DWORD	dwGroupID;								//群组ID
	CT_DWORD	dwMasterID;								//群主ID
	CT_DWORD	dwUserID;								//玩家ID
	CT_WORD		wNeedGem;								//扣除钻石
	CT_WORD		wGameID;								//游戏ID
	CT_WORD		wKindID;								//类型ID
};

struct MSG_CS2DB_Return_GroupPRoom_Gem
{
	CT_DWORD		dwGroupID;							//群ID
	CT_DWORD		dwRecordID;							//记录ID
	CT_DWORD		dwMasterID;							//群主ID
	CT_WORD			wReturnGem;							//退还gem
};

struct MSG_CS2DB_GroupRoom_Info
{
	CT_DWORD	dwGroupID;								//群组ID
	CT_DWORD	dwRoomNum;								//房间ID
	CT_WORD		wNeedGem;								//需要钻石
	CT_WORD		wGameID;								//游戏ID
	CT_WORD		wKindID;								//类型ID
};

//更新群组的贡献值
struct MSG_CS2DB_GroupContribution
{
    CT_DWORD    dwGroupID;
    CT_DWORD    dwContribution;
    CT_BYTE     cbLevel;
};

//更新群组的职位
struct MSG_CS2DB_GroupPosition
{
    CT_DWORD    dwGroupID;
    CT_DWORD    dwDeputyID1;
    CT_DWORD    dwDeputyID2;
};

struct MSG_CS2DB_Insert_HongBao
{
	CT_DWORD	dwUserID;								//用户ID
	CT_DWORD	dwHongBaoScore;							//红包金币
	CT_DWORD	dwSendTime;								//发送时间
	CT_CHAR		szHongBaoName[HONGBAO_NAME_LEN];		//红包名称
};

struct MSG_CS2DB_Delete_HongBao
{
	CT_DWORD	dwHongBaoID;							//红包ID
	CT_DWORD	dwTakeUserID;							//领取人ID
};

struct MSG_CS2DB_Update_HongBao
{
	CT_DWORD	dwHongBaoID;							//红包ID
	CT_DWORD	dwGuessUserID;							//猜中红包的用户ID
};

struct MSG_CS2DB_Insert_Task
{
	CT_DWORD	dwUserID;								//用户ID
	CT_WORD		wTaskID;								//任务ID
};

struct MSG_CS2DB_Update_Task
{
	CT_DWORD	dwUserID;								//用户ID
	CT_WORD		wTaskID;								//任务ID
	CT_DWORD	dwCurrProgress;							//当前进度
	CT_DWORD	dwReceiveTime;							//接受任务进度
	CT_BOOL		bFinish;								//是否完成
};

struct MSG_CS2DB_Insert_TaskReward
{
	CT_DWORD		dwUserID;							//用户ID
	CT_WORD			wTaskID;							//任务ID
	CT_LONGLONG		llReward;							//任务奖励
};

struct MSG_CS2DB_Update_PCU
{
    CT_BYTE     cbPlatformId;                           //平台ID
	CT_BYTE		cbType;									//PCU类型(0:hall 1:ios 2:android)
	CT_DWORD	dwPCU;									//PCU值
};

struct MSG_CS2DB_Insert_WGC_Data
{
	CT_WORD 	wGameID;								//游戏ID
	CT_WORD 	wKindID;								//类型ID
	CT_WORD 	wRoomKind;								//房间类型ID
	CT_DWORD 	dwIndex;								//活动索引
	CT_WORD		wActivityID;							//活动ID
	CT_LONGLONG llRevenue;								//税收
    CT_CHAR     szStartTime[TIME_LEN];                  //开始时间
    CT_CHAR     szEndTime[TIME_LEN];                    //结束时间
};

struct MSG_CS2DB_Update_WGC_Data
{
    CT_WORD 	wGameID;								//游戏ID
    CT_WORD 	wKindID;								//类型ID
    CT_WORD 	wRoomKind;								//房间类型ID
    CT_DWORD 	dwIndex;								//活动索引
    CT_LONGLONG llRevenue;								//税收
};

struct MSG_CS2DB_Update_WGC_UserInfo
{
	MSG_CS2DB_Update_WGC_UserInfo()
	{
		memset(this, 0, sizeof(*this));
	}
	CT_WORD 	wGameID;								//游戏ID
	CT_WORD 	wKindID;								//类型ID
	CT_WORD 	wRoomKind;								//房间类型ID
	CT_DWORD 	dwIndex;								//活动索引
    CT_DWORD    dwUserID;                               //用户ID
    CT_DWORD 	dwRevenue;                              //用户税收
	CT_DWORD 	dwScore;								//玩家总成绩（下注数或者输赢分）
	CT_DWORD 	dwRewardScore;							//奖励金币
    CT_WORD     wCompleteCount;                         //完成次数
    CT_BYTE     cbRank;                                 //用户排名
};

//夺宝资料
struct MSG_CS2DB_Update_DuoBao_Data
{
	CT_WORD     wID;                        			//活动ID
	CT_BYTE     cbState;                    			//状态(0表示没有开奖 1表示正在开奖 2表示已经开奖 3表示过期不开奖)
	CT_DWORD    dwIndex;                    			//第几期
	CT_DWORD 	dwLuckyUserID;              			//得奖者ID
	CT_DWORD    dwLuckyNum;                 			//幸运码
	CT_DWORD    dwRewardScore;                          //奖励金币
	CT_DWORD    dwStartTime;                			//开始时间
	CT_DWORD    dwEndTime;                  			//结束时间
};

//夺宝用户
struct MSG_CS2DB_Insert_DuoBao_User
{
	CT_WORD 	wID;									//活动
	CT_DWORD    dwIndex;                    			//第几期
	CT_DWORD    dwUserID;                   			//用户ID
	CT_DWORD    dwLuckyNum;                 			//幸运码
};

//更新红包的已抢金币数
struct MSG_CS2DB_Update_RedPacket_GrabScore
{
	CT_DWORD 	dwIndex;								//红包期数
    CT_DWORD    dwGrabScore;                            //红包已抢金币
};

//更新红包活动的阶段信息
struct MSG_CS2DB_Update_RedPacket_Stage
{
	CT_DWORD 	dwIndex;								//红包期数
	CT_BYTE		cbStage;                                //本期状态
};

//插入抢到的红包信息
struct MSG_CS2DB_Insert_RadPacket_GrabInfo
{
    CT_DWORD    dwIndex;                                //红包期数
    CT_WORD     wGameID;                                //游戏ID
    CT_WORD     wKindID;                                //游戏类型
    CT_WORD     wRoomKindID;                            //房间类型
    CT_DWORD    dwUserID;								//用户ID
    CT_DWORD 	dwLuckyMoney;                           //红包金额
    CT_BYTE     cbIsRobot;                              //是否机器人
};

struct MSG_DB2CS_System_Status
{
	CT_BYTE		cbPrivateMode;							//是否启用私人房
	CT_BYTE		cbScoreMode;							//是否启用金币场
};

struct MSG_DB2CS_Deduct_GroupMaster_Gem
{
	CT_BYTE		cbResult;								//结果(0失败，1成功)
	CT_DWORD	dwGroupID;								//群组ID
	CT_DWORD	dwMasterID;								//群主ID
	CT_DWORD	dwUserID;								//玩家ID
	CT_WORD		wNeedGem;								//扣除钻石
	CT_WORD		wGameID;								//游戏ID
	CT_WORD		wKindID;								//类型ID
};

struct MSG_DB2CS_GroupRoom_Info
{
	CT_DWORD	dwRoomNum;								//房间ID
	CT_DWORD	dwRecordID;								//记录ID
};

struct MSG_DB2CS_BeAdd_Group_User
{
	CT_BYTE			cbResult;						//结果
	CT_BYTE			cbType;							//操作类型
	CT_DWORD		dwGroupID;						//群组ID
	CT_DWORD		dwUserID;						//被操作用户ID
	CT_DWORD		dwOperateUserID;				//操作用户ID
	CT_CHAR			szNickName[NICKNAME_LEN];		//昵称
	CT_CHAR			szHeadUrl[HEAD_URL_LEN];		//头像URL
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif