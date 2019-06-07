/******************************************************************
 Copyright (C),  2002-2015,  TECHNOLOGLIES CO., LTD.
 File name:		CMD_Game.h
 Version:		1.0
 Description:    游戏框架,消息定义
 Author:			osc
 Create Date:	2012-7-4
 History:
 
 ******************************************************************/
#ifndef  CMD_GAME_H_
#define  CMD_GAME_H_
#include <ctime>
#include "CTType.h"
#include "CGlobalData.h"

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif 

//玩家客户端:C
//游戏服务器:G
//CG相互发送信息: CGS

//错误定义
#define ERROR_TABLE_ID						  (-1)            //无效的桌子
#define ERROR_ROOM_ID						  (-1)            //无效的房间
#define ERROR_CHAIR_ID						  (-1)            //无效的椅子

/////////////////////////////////////////////////////////////////////////////////////
//客户端与服务器大厅的消息
#define		MSG_FRAME_MAIN						7

//C-S
#define		SUB_C2S_CREATE_PROOM				7001				//创建房间[MSG_CS_CreatePrivateRoom]
#define		SUB_C2S_ENTER_PROOM					7002				//进入房间[MSG_CS_EnterPrivateRoom]
#define		SUB_C2S_ENTER_ROOM					7003				//进入普通房间[MSG_CS_EnterGame]
#define		SUB_C2S_USER_READY					7004				//玩家准备[MSG_CS_UserReady]
#define		SUB_C2S_USER_LEFT					7005				//玩家离开[MSG_CS_UserLeft]
#define		SUB_C2S_APPLY_DISMISS_PROOM			7006				//玩家申请解散私人房间[MSG_CS_ApplyDismissPRoom]
#define		SUB_C2S_OPERATE_DISMISS_PROOM		7007				//玩家操作解散私人房间[MSG_CS_OperateDismissPRoom]
#define		SUB_C2S_VOICE_CHAT					7008				//玩家语音聊天[MSG_CS_Voice_Chat]
#define		SUB_C2S_TEXT_CHAT					7009				//玩家文字聊天[MSG_CS_Text_Chat]
#define		SUB_C2S_PULL_APPLY_PROOM			7010				//房主主动拉取申请房间信息[MSG_CS_PullApplyPRoom]
#define		SUB_C2S_OPERATE_APPLY_PROOM			7011				//房主操作玩家申请房间[MSG_CS_OperateApplyPRoom]
#define		SUB_C2S_QUERY_SCORE_ROOM_INFO		7012				//查询金币场基本信息[MSG_CS_QueryScoreRoomInfo]
#define		SUB_C2S_ENTER_ROOM_EX				7013				//进入普通房间[MSG_CS_AddWaitList 斗地主用]
#define		SUB_C2S_ADD_WAIT_LIST				7014				//加入游戏等待队列[MSG_CS_Add_WaitList]
#define		SUB_C2S_CHANGE_TABLE				7015				//换桌[MSG_CS_Change_Table]
#define 	SUB_C2S_REPORT_USER					7016				//举报玩家[MSG_CS_Report_User]

//S->C
#define		SUB_S2C_CREATE_PROOM				7101				//创建房间成功[MSG_SC_CreatePrivateRoom]
#define		SUB_S2C_CREATE_PROOM_FAIL			7102				//创建房间失败[MSG_SC_GameErrorCode]

#define		SUB_S2C_ENTER_PROOM_SUCC			7103				//进入房间成功[MSG_SC_EnterPRoomSucc]
#define		SUB_S2C_ENTER_PROOM_FAIL			7104				//进入房间失败[MSG_SC_GameErrorCode]

#define		SUB_S2C_ENTER_ROOM_SUCC				7105				//玩家进入普通房间成功[MSG_SC_EnterRoomSucc]
#define		SUB_S2C_ENTER_ROOM_FAIL				7106				//玩家进入普通房间失败[MSG_SC_GameErrorCode]

#define		SUB_S2C_USER_ENTER					7107				//玩家进入消息(包括自己的和桌子上其他玩的)[MSG_SC_UserBaseInfo， 金币场用MSG_SC_UserBaseInfo_ForScore]
#define		SUB_S2C_USER_SCORE					7108				//玩家金币信息[MSG_SC_UserScoreInfo]
#define		SUB_S2C_USER_STATUS					7109				//玩家状态信息[MSG_SC_GameUserStatus]
#define		SUB_S2C_USER_GEM					7110				//玩家钻石信息[MSG_SC_GameUserGem]

#define		SUB_S2C_APPLY_DISMISS_PROOM			7111				//玩家申请解散私人房间[MSG_SC_ApplyDismissPRoom]
#define		SUB_S2C_OPERATE_DISMISS_PROOM		7112				//玩家操作解散私人房间[MSG_SC_OperateDismissPRoom]
#define		SUB_S2C_DISMISS_PROOM_RESULT		7113				//玩家解散私人房间结果[MSG_SC_DismissPRoomResult]
#define		SUB_S2C_DISMISS_PROOM_SCENE			7114				//玩家解散私人房间的场景[MSG_SC_DismissPRoomScene]

#define		SUB_S2C_VOICE_CHAT					7115				//玩家语音聊天[MSG_SC_Voice_Chat]
#define		SUB_S2C_TEXT_CHAT					7116				//玩家文字聊天[MSG_SC_Text_Chat]

#define		SUB_S2C_SAME_IP_TIPS				7117				//相同IP提醒[MSG_SC_SameIp_Tips]

#define		SUB_S2C_APPLY_PROOM_SUCC			7118				//申请VIP房间成功，等待房主确认[MSG_SC_ApplyEnterVipPRoomSucc]
#define		SUB_S2C_APPLY_PROOM_TO_OWNER		7119				//向房主发送申请房间的信息[MSG_SC_Apply_VipPRoom_ToOwner]
#define		SUB_S2C_APPLY_PROOM_TO_OWNER_ALL	7120				//向房主发送申请房间的信息[MSG_SC_Apply_VipPRoom_All_ToOwner]
#define		SUB_S2C_OPERATE_APPLY_PROOM_RESULT	7121				//房主处理申请房间信息返回[MSG_SC_OperateApplyPRoom]
#define		SUB_S2C_OWNER_AGREDD_ENTER_PROOM	7122				//房主同意进入房间[MSG_SC_OwnerAgreeEnterPRoom]

#define		SUB_S2C_PROOM_GAMEROUND_REWARD		7123				//游戏大局奖励[MSG_SC_GameRoundReward]
#define		SUB_S2C_QUERY_SCORE_ROOM_INFO		7124				//查询金币场成功[MSG_SC_QueryScoreRoomInfo]

#define		SUB_S2C_GET_BENEFIT_REWARD			7125				//游戏领取救济金[MSG_SC_AutoGet_Benefit] 客户端不需要根据这条协议更新金币
#define		SUB_S2C_UPDATE_RECHARGE_VIP			7126				//更新用户充值总额和VIP等级【MSG_SC_UserVipLevel】
#define		SUB_S2C_HAS_NEW_MAIL				7127				//用户有新邮件
#define		SUB_S2C_SHOW_EXCHANGE				7128				//显示兑换按钮【MSG_SC_ShowExchange】
#define 	SUB_S2C_HAS_BENEFIT					7129				//是否有救济金领取【MSG_SC_HasBenefit】

#define		SUB_S2C_ENTER_ROOM_EX_SUCC			7130				//进入房间成功[MSG_SC_EnterRoomExSucc]
#define		SUB_S2C_ADD_WAIT_LIST				7131				//进入等待队列返回[MSG_SC_AddWaitList]
#define		SUB_S2C_MATCH_TABLE_SUCC			7132				//配桌成功[空消息]
#define		SUB_S2C_MATCH_TABLE_NEED_ANDROID	7133				//配桌需要机器人[MSG_SC_MatchTabel_NeedAndroid]
#define		SUB_S2C_CHANGE_TABLE				7134				//换桌返回[空]
#define 	SUB_S2C_REPORT_USER					7135				//举报玩家返回【MSG_SC_Report_User】

#define		SUB_S2C_IN_OTHER_ROOM				7150				//在其他游戏房间中【MSG_SC_In_OtherRoom】

//登录消息头
struct MSG_GameMsgHead
{
	CT_UINT64		uValue1;							//多功能值1.client net addr
};

//创建私人房
struct MSG_CS_CreatePrivateRoom
{
	CT_DWORD	 dwUserID;							//玩家ID
	CT_WORD		 wGameID;							//游戏ID
	CT_WORD		 wKindID;							//游戏玩法
	CT_WORD		 wUserCount;						//游戏人数
	CT_WORD		 wMaxHuxi;							//胡息
	CT_BYTE		 cbPlayCount;						//游戏局数
	CT_BYTE		 cbIsVip;							//是否VIP (0: 普通房卡房; 1 : VIP房卡房)
	CT_CHAR		 szOtherParam[PROOM_PARAM_LEN];		//其他参数
	CT_CHAR		 szUUID[UUID_LEN];					//session.
};

struct MSG_PS_CreatePrivateRoom 
{
	CT_DWORD	 dwUserID;							//玩家ID
	CT_WORD		 wGameID;							//游戏ID
	CT_WORD		 wKindID;							//游戏玩法
	CT_WORD		 wUserCount;						//游戏人数
	CT_WORD		 wMaxHuxi;							//胡息
	CT_BYTE		 cbPlayCount;						//游戏局数
	CT_BYTE		 cbIsVip;							//是否VIP (0: 普通房卡房; 1 : VIP房卡房)
	CT_CHAR		 szOtherParam[PROOM_PARAM_LEN];		//其他参数
	CT_INT64	 uClientSock;						//客户端sock地址
};

struct MSG_SC_CreatePrivateRoom
{
	CT_WORD		wGameID;							//游戏ID
	CT_WORD		wKindID;							//游戏类型ID
	CT_DWORD	dwRoomNum;							//创建的房间ID
};

struct MSG_CS_EnterPrivateRoom
{
	CT_DWORD	dwUserID;							//玩家ID
    CT_DWORD	dwRoomNum;							//房间号
    CT_DWORD    dwServerID;                         //房间所在的服务器ID
    //CT_WORD		wCurGameID;							//当前游戏ID
	//CT_WORD		wCurKindID;							//当前游戏玩家ID
	//CT_DOUBLE	dLongitude;							//经度
	//CT_DOUBLE	dLatitude;							//纬度
	//CT_CHAR		szLocation[USER_LOCATE_LEN];		//用户所在地
	//CT_CHAR		szUUID[UUID_LEN];					//session.
};

struct MSG_CS_EnterGame
{
	CT_DWORD	dwUserID;							//玩家ID
	CT_WORD		wGameID;							//游戏ID
	CT_WORD		wKindID;							//玩法ID
	CT_WORD		wRoomKindID;						//房间类型（房卡房0，初级房1，中级房2，高级房3）
	CT_CHAR		szUUID[UUID_LEN];					//session
};

struct MSG_CS_UserReady								//用户准备
{
	CT_DWORD	dwUserID;
};

struct MSG_CS_UserLeft								//用户离开
{
	CT_DWORD	dwUserID;
};

//用户申请解散私人房间
struct MSG_CS_ApplyDismissPRoom					
{
	CT_DWORD	dwUserID;							//用户ID		
};

//用户操作解散私人房间
struct MSG_CS_OperateDismissPRoom
{
	CT_DWORD	dwUserID;							//用户ID
	CT_BYTE		cbOperate;							//操作(0:不同意, 1:同意)
};

//用户聊天
struct MSG_CS_Voice_Chat
{
	CT_DWORD	dwUserID;							//用户ID
	CT_CHAR		szChatUrl[GAME_VOICE_LEN];			//语音URL
};

//用户文字聊天
struct MSG_CS_Text_Chat
{
	CT_DWORD	dwUserID;							//用户ID
	CT_BYTE		cbType;								//聊天类型（1：文字 2：表情？）
	CT_BYTE		cbIndex;							//序号（文字的序号或者表情的序号）
};

//房主拉取申请进房的信息
struct  MSG_CS_PullApplyPRoom
{
	CT_DWORD	dwUserID;							//房主自己的用户ID
}; 

//房主处理申请进房的信息
struct MSG_CS_OperateApplyPRoom
{
	CT_DWORD	dwOwnerUserID;						//房主自己的用户ID
	CT_DWORD	dwUserID;							//用户ID
	CT_BYTE		cbOperate;							//操作码（1：同意; 2：不同意）
};

//查询金币场信息
struct MSG_CS_QueryScoreRoomInfo
{
	CT_DWORD	dwUserID;							//玩家ID
	CT_WORD		wGameID;							//游戏ID
	CT_WORD		wKindID;							//玩法ID
	CT_CHAR		szUUID[UUID_LEN];					//session
};

//加入游戏等待队列
struct MSG_CS_Add_WaitList
{
	CT_DWORD	dwUserID;							//玩家ID
	CT_WORD		wGameID;							//游戏ID
	CT_WORD		wKindID;							//玩法ID
	CT_WORD		wRoomKindID;						//房间类型（房卡房0，初级房1，中级房2，高级房3）
};

//换桌
struct MSG_CS_Change_Table
{
	CT_DWORD	dwUserID;							//用户换桌
};

//举报玩家
struct MSG_CS_Report_User
{
	CT_DWORD 	dwUserID;							//用户ID
	CT_DWORD 	dwBeReportUserID;					//被举报玩家
};

struct MSG_PG_EnterPrivateRoom : public MSG_GameMsgHead
{
	CT_DWORD		dwUserID;				//用户ID
	CT_DOUBLE		dLongitude;				//经度
	CT_DOUBLE		dLatitude;				//纬度
	CT_CHAR			szLocation[USER_LOCATE_LEN];		//用户所在地
	CT_DWORD		dwOwnerUserID;			//房主ID
	CT_DWORD		dwRoomNum;				//私人房间ID
	CT_WORD			wGameID;				//游戏ID
	CT_WORD			wKindID;				//类型ID
	CT_WORD			wUserCount;				//桌子人数
	CT_WORD			wPlayCount;				//游戏局数
	//CT_WORD			wWinLimit;				//输赢上限
	//CT_WORD			wEndLimit;				//结束上限
	CT_BYTE			cbIsVip;				//是否VIP房
	//CT_BYTE			cbIsAuto;				//是否自动开房
	CT_DWORD		dwGroupID;				//自动开房的群ID
	CT_DWORD		dwRecordID;				//自动开房间的记录ID
	CT_BYTE			cbGroupType;			//群组类型
	CT_CHAR			szOtherParam[PROOM_PARAM_LEN]; //其他游戏参数
};

struct MSG_PG_EnterGroupPRoom : public MSG_GameMsgHead
{
    CT_DWORD        dwUserID;               //用户ID
    CT_DWORD        dwRoomNum;              //房间号
};

struct MSG_PG_EnterRoom : public MSG_GameMsgHead
{
	CT_DWORD		dwUserID;				//用户ID
};

struct MSG_PG_QueryScoreRoomInfo : public MSG_GameMsgHead
{
	CT_DWORD		dwUserID;				//用户ID
	CT_DWORD		dwPlayerCount[3];		//人数0初级场，1中级场，2高级场
	CT_DWORD		dwEnterMinScore[3];		//最低进入条件
	CT_DWORD		dwEnterMaxScore[3];		//最高进入条件
	CT_DWORD		dwBaseScore[3];			//底分
};

//已经有其他类型房间不能再进入
struct MSG_SC_In_OtherRoom : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;				// 游戏ID
	CT_WORD			wKindID;				// 游戏子类型
	CT_WORD			wRoomKindID;			// 房间类型
};

//举报玩家返回
struct MSG_SC_Report_User : public MSG_GameMsgDownHead
{
	CT_BYTE         cbResult;               //举报结果（0：成功 1：已经举报过了）
};

//直接给前端发的错误码
struct MSG_SC_GameErrorCodeDirect
{
	CT_INT32		iErrorCode;
};

//游戏服务器给前端发的错误码(客户端收不到MSG_GameMsgDownHead段)
struct MSG_SC_GameErrorCode : public MSG_GameMsgDownHead
{
	CT_INT32		iErrorCode;
};

//进入房间成功
struct MSG_SC_EnterPRoomSucc : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;
	CT_WORD			wKindID;
	CT_WORD			wRoomKindID;		
	CT_DWORD		dwRoomNum;			//房间号
	CT_DWORD		dwRoomOwnerID;		//房主ID
	CT_DWORD		dwCellScore;		//底分
	CT_WORD			wUserCount;			//游戏人数
	CT_WORD			wPlayCount;			//游戏局数
	CT_BYTE			cbIsVip;			//是否VIP房间
	CT_CHAR			szOtherParam[PROOM_PARAM_LEN];
};

//进入房间成功
struct MSG_SC_EnterRoomSucc : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//游戏类型
	CT_WORD			wRoomKindID;					//房间类型
	CT_DWORD        dwRoomNum;                      //房间号
	CT_DOUBLE		dCellScore;						//游戏底分
};

//进入房间成功
struct MSG_SC_EnterRoomExSucc : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//游戏类型
	CT_WORD			wRoomKindID;					//房间类型
	CT_DWORD        dwRoomNum;                      //房间号
	CT_DOUBLE		dCellScore;						//游戏底分
};

//进入等待队列返回
struct MSG_SC_AddWaitList : public MSG_GameMsgDownHead
{
	CT_BYTE			cbResult;						//加入结果（0代表成功 1加入失败）
	CT_BYTE			cbHeadIndex;					//头像索引
};

//进入SUB_S2C_MATCH_TABLE_NEED_ANDROID
struct MSG_SC_MatchTabel_NeedAndroid : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//游戏类型
	CT_WORD			wRoomKindID;					//房间类型
};

//金币场信息
struct MSG_SC_QueryScoreRoomInfo
{
	CT_WORD				wGameID;			//游戏ID
	CT_WORD				wKindID;			//游戏类型ID
	CT_BYTE				cbRoomKindCount;	//房间类型数目
	CT_BYTE				cbRoomKindID[4];	//房间类型ID
	CT_BYTE				cbRoomStatus[4];	//房间状态(1:良好 2: 拥挤 3: 爆满)
	CT_DWORD			dwPlayerCount[4];	//人数0初级场，1中级场，2高级场 3免费场
	CT_DOUBLE			dEnterMinScore[4];	//最低进入条件
	CT_DOUBLE			dEnterMaxScore[4];	//最高进入条件
	CT_DOUBLE			dBaseScore[4];		//底分

	MSG_SC_QueryScoreRoomInfo()
		:wGameID(0)
		,wKindID(0)
		,cbRoomKindCount(0)
	{
		for (int i = 0; i < 4; i++)
		{
			cbRoomKindID[i] = 0;
			cbRoomStatus[i] = 1;
			dwPlayerCount[i] = 0;
			dEnterMinScore[i] = 0.0f;
			dEnterMaxScore[i] = 0.0f;
			dBaseScore[i] = 0.0f;
		}
	}
};

//领取救济金
struct MSG_SC_AutoGet_Benefit : public MSG_GameMsgDownHead
{
	CT_BYTE				cbCurrBenefitCount;		// 领取次数
	CT_BYTE 			cbRemainBenefitCount;	// 剩余次数
	CT_DOUBLE			dRewardScore;			// 奖励金币
};

//玩家基本信息
struct MSG_SC_UserBaseInfo : public MSG_GameMsgDownHead
{
	CT_DWORD        dwUserID;									//ID
	CT_CHAR			szNickName[NICKNAME_LEN];					//昵称
	CT_CHAR			szHeadUrl[HEAD_URL_LEN];					//头像URL
	CT_CHAR			szIP[IP_LEN];								//客户端IP
	CT_DOUBLE		dLongitude;									//经度
	CT_DOUBLE		dLatitude;									//纬度
	CT_CHAR			szLocation[USER_LOCATE_LEN];				//用户所在地
	CT_BYTE			cbGender;									//性别
	CT_DWORD		dwGem;										//玩家钻石
	CT_LONGLONG		llScore;									//玩家金币

	CT_WORD			wTableID;									//桌子id
	CT_WORD			wChairID;									//椅子id
	USER_STATUS     usStatus;									//用户状态
};

//玩家基本信息(对于龙虎斗这种百人游戏而言)
struct MSG_SC_UserBaseInfo_ForScoreEx
{
    MSG_SC_UserBaseInfo_ForScoreEx()
        :dwUserID(0)
        ,cbHeadIndex(0)
        ,cbGender(0)
        ,cbVipLevel(0)
        ,cbVipLevel2(0)
        ,dwGem(0)
        ,dScore(0.0f)
        ,wTableID(INVALID_CHAIR)
        ,wChairID(INVALID_CHAIR)
        ,usStatus(sFree)
    {
        szNickName[0] = '\0';
    }

    CT_DWORD        dwUserID;									//ID
    CT_CHAR			szNickName[NICKNAME_LEN];					//昵称
    CT_BYTE			cbHeadIndex;								//头像索引
    CT_BYTE			cbGender;									//性别
    CT_BYTE			cbVipLevel;									//VIP等级
    CT_BYTE			cbVipLevel2;								//VIP等级2
    CT_DWORD		dwGem;										//玩家钻石
    CT_DOUBLE		dScore;										//玩家金币

    CT_WORD			wTableID;									//桌子id
    CT_WORD			wChairID;									//椅子id
    USER_STATUS     usStatus;									//用户状态
};

//玩家基本信息
struct MSG_SC_UserBaseInfo_ForScore : public MSG_GameMsgDownHead
{
	MSG_SC_UserBaseInfo_ForScore()
		:dwUserID(0)
		,cbHeadIndex(0)
		,cbGender(0)
		,cbVipLevel(0)
		,cbVipLevel2(0)
		,dwGem(0)
		,dScore(0.0f)
		,wTableID(INVALID_CHAIR)
		,wChairID(INVALID_CHAIR)
		,usStatus(sFree)
	{
		szNickName[0] = '\0';
		//szHeadUrl[0] = '\0';
		//szLocation[0] = '\0';
	}

	CT_DWORD        dwUserID;									//ID
	CT_CHAR			szNickName[NICKNAME_LEN];					//昵称
	//CT_CHAR			szHeadUrl[HEAD_URL_LEN];					//头像URL
	//CT_CHAR			szLocation[USER_LOCATE_LEN];				//用户所在地
	CT_BYTE			cbHeadIndex;								//头像索引
	CT_BYTE			cbGender;									//性别
	CT_BYTE			cbVipLevel;									//VIP等级
	CT_BYTE			cbVipLevel2;								//VIP等级2
	CT_DWORD		dwGem;										//玩家钻石
	CT_DOUBLE		dScore;										//玩家金币

	CT_WORD			wTableID;									//桌子id
	CT_WORD			wChairID;									//椅子id
	USER_STATUS     usStatus;									//用户状态
};

//用户状态
struct MSG_SC_GameUserStatus : public MSG_GameMsgDownHead
{
	CT_DWORD        dwUserID;									//用户id
	CT_WORD			wChairID;									//椅子id
	CT_WORD			wTableID;									//桌子id
	USER_STATUS     usStatus;									//用户状态
};

//用户状态（用户状态打包，用于百人游戏）
struct MSG_SC_GameUserStatusEx
{
    CT_DWORD        dwUserID;									//用户id
    CT_WORD			wChairID;									//椅子id
    CT_WORD			wTableID;									//桌子id
    USER_STATUS     usStatus;									//用户状态
};

//用户钻石
struct MSG_SC_GameUserGem : public MSG_GameMsgDownHead
{
	CT_DWORD        dwUserID;									//用户id
	CT_DWORD		dwGem;										//钻石
};

//房间用户积分信息
struct MSG_SC_UserScoreInfo : public MSG_GameMsgDownHead
{
	CT_DWORD        dwUserID;									//用户ID
	CT_WORD			wChairID;									//椅子id
	CT_WORD			wTableID;									//桌子id
	CT_DOUBLE		dScore;										//玩家金币
};

//用户VIP信息
struct MSG_SC_UserVipLevel : public MSG_GameMsgDownHead
{
	CT_DWORD        dwUserID;									//用户ID
	CT_DWORD		dwTotalRecharge;							//充值总额
	CT_BYTE			cbVipLevel;									//vip等级
	CT_BYTE			cbVipLevel2;								//VIP2等级
	CT_BYTE			cbNextVipLevel2;							//下一个VIP2等级
	CT_DWORD		dwNextLevel2NeedRecharge;					//下一等级需要的充值
};

//更新用户显示兑换按钮
struct MSG_SC_ShowExchange: public MSG_GameMsgDownHead
{
	CT_BYTE			cbShowExchange;								//是否显示对换按钮(0不显示 1显示 2显示并显示红点)
};

//是否有救济金领取
struct MSG_SC_HasBenefit : public MSG_GameMsgDownHead
{
    CT_BYTE         cbState;                                    //是否能领取救济金(0不能领取 1能领取)
};

//申请解散私人房间
struct MSG_SC_ApplyDismissPRoom : public MSG_GameMsgDownHead
{
	CT_DWORD        dwUserID;									//申请解散的用户ID
	CT_BYTE			cbWaitOperate;								//等待操作的用户个数
	CT_DWORD		arrWaitUserID[10];							//等待操作的用户ID
};

//操作解散私人房间
struct MSG_SC_OperateDismissPRoom : public MSG_GameMsgDownHead
{
	CT_DWORD        dwUserID;									//申请解散的用户ID
	CT_BYTE			cbOperate;									//操作码（0: 不同意, 1: 同意）
};

//解散私人房间结果
struct MSG_SC_DismissPRoomResult : public MSG_GameMsgDownHead
{
	CT_BYTE			cbOperateResult;							//操作码(0: 不解散，1: 解散)
	CT_BYTE			cbOperateCount;								//操作的用户数
	CT_DWORD		arrUserID[10];								//操作的用户ID
};

struct MSG_SC_DismissPRoomScene : public MSG_GameMsgDownHead
{
	CT_DWORD		dwApplyUserID;								//申请解散的玩家ID
	CT_DWORD		dwDismissTimeLeft;							//离解散的剩余时间（秒）
	CT_BYTE			cbCount;									//操作的用户数
	CT_DWORD		arrOperate[10];								//操作码（0: 不同意, 1: 同意, 2: 未操作）
	CT_DWORD		arrUserID[10];								//操作的用户ID
};

//用户聊天
struct MSG_SC_Voice_Chat : public MSG_GameMsgDownHead
{
	CT_DWORD	dwUserID;							//用户ID
	CT_CHAR		szChatUrl[GAME_VOICE_LEN];			//语音URL
};

//用户文字聊天
struct MSG_SC_Text_Chat : public MSG_GameMsgDownHead
{
	CT_DWORD	dwUserID;							//用户ID
	CT_BYTE		cbType;								//聊天类型（1：文字 2：表情？）
	CT_BYTE		cbIndex;							//序号（文字的序号或者表情的序号）
};

//相同IP提醒
struct MSG_SC_SameIp_Tips : public MSG_GameMsgDownHead
{
	CT_BYTE		cbSameCount;
	CT_DWORD	arrUserID[10];
};

//申请进入房间成功，等待房主确认
struct MSG_SC_ApplyEnterVipPRoomSucc : public MSG_GameMsgDownHead
{
	CT_DWORD		dwRoomNum;						//房间号
};

//向房主发送申请信息
struct MSG_SC_Apply_VipPRoom_ToOwner : public MSG_GameMsgDownHead
{
	CT_DWORD	dwUserID;
	CT_BYTE		cbGender;
	CT_BYTE		cbState;
	CT_CHAR		szNickname[NICKNAME_LEN];
	CT_CHAR		szHeadUrl[HEAD_URL_LEN];
	CT_CHAR		szApplyTime[TIME_LEN];
};

//向房主发送申请信息
struct MSG_SC_Apply_VipPRoom_All_ToOwner : public MSG_GameMsgDownHead
{
	CT_BYTE		cbCount;
	CT_DWORD	arrUserID[10];
	CT_CHAR		szNickname[10][NICKNAME_LEN];
	CT_CHAR		szApplyTime[10][TIME_LEN];
	CT_CHAR		szHeadUrl[10][HEAD_URL_LEN];
	CT_BYTE		cbGender[10];
	CT_BYTE		arrState[10];						//申请状态（0：未同意; 1：已同意 ）
};

//房主处理申请进房的信息
struct MSG_SC_OperateApplyPRoom : public MSG_GameMsgDownHead
{
	CT_DWORD	dwUserID;							//用户ID
	CT_BYTE		cbOperateResult;					//操作码（1：同意成功; 2：不同意成功; 3：玩家已经在其他游戏中）
};

//房主同意进入房间，前端收到此消息再发一次进入消息
struct MSG_SC_OwnerAgreeEnterPRoom : public MSG_GameMsgDownHead
{
	CT_WORD			wGameID;						//游戏ID
	CT_WORD			wKindID;						//玩法ID
	CT_DWORD		dwRoomNum;						//房间号
};

//游戏大局奖励
struct MSG_SC_GameRoundReward : public MSG_GameMsgDownHead
{
	CT_DWORD		dwGameRoundCount;				//当前游戏大局数
	CT_DWORD		dwRewardGem;					//奖励的宝石
};

//客户端与服务器的游戏消息
#define		MSG_GAME_MAIN										8

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif