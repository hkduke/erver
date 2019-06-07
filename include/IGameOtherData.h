//
//  igameotherdata.h
//  GameService
//
//  Created by oushuncai on 16/4/6.
//  Copyright (c) 2016年 oushuncai. All rights reserved.
//

#ifndef GameService_igameotherdata_h
#define GameService_igameotherdata_h


#include "CTType.h"
#include "CGlobalData.h"
#include <memory.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif


//24小时秒数.
#define DAY_24HOUR_SECOND       (86400)


//游戏结束标志
enum GameEndTag
{
	GER_NORMAL = 0,			//正常
	GER_USER_LEFT,			//强退
	GER_DISMISS,			//解散
    GER_FLOW_BUREAU,		//流局
};

//分数类型
enum ScoreKind
{
	enScoreKind_Win,		//胜
	enScoreKind_Lost,		//输
	enScoreKind_Draw,		//和
	enScoreKind_Flee		//逃
};

//积分信息
struct ScoreInfo
{
	ScoreInfo()
		:dwUserID(0)
		,llScore(0)
		,llRealScore(0)
  		,cbCardType(255)
		,bBroadcast(true)
	{

	}

	CT_DWORD				dwUserID;			//用户id
	CT_LONGLONG				llScore;			//游戏积分
	CT_LONGLONG				llRealScore;		//实际得分
	//ScoreKind				scoreKind;			//分数类型
  	CT_BYTE 				cbCardType;			//牌型
	CT_BOOL					bBroadcast;			//是否广播
};

//玩家基础数据
typedef struct STR_GAME_UserBaseData
{
	STR_GAME_UserBaseData()
	{
		memset(this, 0, sizeof(*this));
	}

	CT_DWORD		dwUserID;							//用户id
	CT_DWORD		dwGem;								//宝石
	CT_DWORD		dwPlatformID;						//平台ID
	CT_DWORD 		dwchannelID;						//渠道ID
	CT_BYTE         cbGender;							//性别
	CT_BYTE			cbVipLevel;							//VIP等级
	CT_BYTE			cbVipLevel2;						//VIP2等级
	CT_BYTE			cbHeadIndex;						//头像索引
	CT_DOUBLE		dLongitude;							//经度
	CT_DOUBLE		dLatitude;							//纬度
	CT_CHAR			szNickName[NICKNAME_LEN];			//昵称
	//CT_CHAR			szHeadUrl[HEAD_URL_LEN];			//头像URL
	CT_CHAR			szIP[IP_LEN];						//IP
	CT_CHAR			szLocation[USER_LOCATE_LEN];		//所在地
	CT_LONGLONG		llScore;							//金币
	CT_LONGLONG 	llBankScore;						//银行金币
	CT_BYTE         cbBenefitCount;                     //救济金次数
	CT_BYTE 		cbNotifyBenefit;					//是否通知救济金
	CT_DWORD        dwBenefitLastTime;                  //救济金最后时间
	//CT_DWORD 		dwZjhCount;							//炸金花局数
	//CT_BYTE		cbZjhParam;							//炸金花好牌参数
	//CT_DWORD 		dwQznnCount;						//抢庄牛牛局数
	//CT_BYTE 		cbQznnParam;						//抢庄牛牛好牌局数
	//CT_DWORD 		dwGoodCardTime;						//好牌参数更新时间
	CT_DWORD		dwTotalRecharge;					//玩家总充值
	CT_DWORD 		dwRechargeTempCount;				//本次充值次数
	CT_DWORD        dwTodayPlayCount;                   //今日游戏局数
	CT_LONGLONG 	llTotalWinScore;					//本游戏总赢分
}GS_UserBaseData;

//输赢信息
//玩家游戏数据
struct GS_UserScoreData
{
    GS_UserScoreData()
    : dwUserID(0)
    , llWinScore(0)
    , llLostScore(0)
    , dwRevenue(0)
    , dwWinCount(0)
    , dwLostCount(0)
    , dwEnterTime(0)
    , dwPlayCount(0)
    , dwChangeTableCount(0)
    { }

    CT_DWORD        dwUserID;                           //用户id
    //增量数据
	CT_LONGLONG 	llAddScore;							//总输赢金币(老捕鱼用到)
    CT_LONGLONG		llWinScore;					        //赢金币
    CT_LONGLONG     llLostScore;                        //输金币
    CT_DWORD		dwRevenue;							//税收
    CT_DWORD        dwWinCount;                         //赢得次数
    CT_DWORD        dwLostCount;                        //输的次数
    CT_DWORD		dwEnterTime;						//进入游戏时间
    CT_DWORD 		dwPlayCount;						//当前桌的游戏局数(对炸金花强制换桌有效)
    CT_DWORD 		dwChangeTableCount;					//当前游戏换桌局数(对炸金花强制换桌有效)
};

//记录信息
struct RecordScoreInfo
{
	RecordScoreInfo()
	: dwUserID(0)
	, llSourceScore(0)
	, iScore(0)
	, dwRevenue(0)
	, cbIsAndroid(0)
	, cbStatus(0)
	{
		for (int i = 0; i < 4; ++i)
		{
			iAreaJetton[i] = 0;
		}
	}

	CT_DWORD				dwUserID;			//用户ID
	CT_LONGLONG				llSourceScore;		//原有积分
	CT_INT32				iScore;				//游戏输赢分
	CT_INT32				iAreaJetton[4];		//各区域下注数目
	CT_DWORD				dwRevenue;			//游戏税收
	CT_DWORD 				llJettonScore;		//游戏下注总数
	CT_BYTE					cbIsAndroid;		//是否机器人
	CT_BYTE					cbStatus;			//标识数据是否有效(0无效 1有效)
};

//红包扫雷详细信息
struct RecordHBSLInfo
{
	RecordHBSLInfo()
	:dwSendUserID(0)
	,dwHongbaoID(0)
	,dwAmount(0)
	,wAllotCount(0)
	,dMultiple(0)
	,cbThunderNO(0)
	,dwSendTime(0)
	,dwWinColorPrize(0)
	{
	}
	CT_DWORD 				dwSendUserID;		//发送者ID
	CT_DWORD                dwHongbaoID;        //红包ID
	CT_DWORD                dwAmount;           //红包金额(单位:分)
	CT_WORD                 wAllotCount;		//分包个数
	CT_DOUBLE               dMultiple;         //红包赔率
	CT_BYTE                 cbThunderNO;        //雷号
	CT_DWORD                dwSendTime;         //发送红包的时间戳
	CT_DWORD                dwWinColorPrize;         //中彩金额
};

//红包扫雷的抢红包信息
struct RecordHBSLGrabInfo
{
    RecordHBSLGrabInfo()
    : dwUserID(0)
    , iScore(0)
    , cbIsThunder(0)
    , dwWinColorPrize(0)
    {

    }
	CT_DWORD                dwUserID;           //用户ID
	CT_INT32                iScore;            //抢到的红包金额(单位:分)
	CT_BYTE                 cbIsThunder;        //是否中雷
    CT_DWORD                dwWinColorPrize;         //中彩金额
};

//桌子状态
struct TableState
{
	CT_DWORD		dwTableID;				//桌子ID
	CT_BOOL			bIsLock;				//是否锁定
	//CT_BOOL			bIsLookOn;				//是否允许旁观
};


//私人桌子的信息
struct PrivateTableInfo
{
    PrivateTableInfo()
    :dwRoomNum(0)
    ,dwOwnerUserID(0)
    ,wCurrPlayCount(0)
    ,wTotalPlayCount(0)
    ,wUserCount(0)
    ,cbIsVip(0)
    ,dwGroupID(0)
    ,dwRecordID(0)
    ,cbGroupType(0)
    {
        szOtherParam[0] = '\0';
    }
    void reset()
    {
        dwRoomNum = 0;
        dwOwnerUserID = 0;
        wCurrPlayCount = 0;
        wTotalPlayCount = 0;
        wUserCount = 0;
        cbIsVip = 0;
        dwGroupID = 0;
        dwRecordID = 0;
        cbGroupType = 0;
        szOtherParam[0] = '\0';
    }

	CT_DWORD		dwRoomNum;			//私人房间ID
	CT_DWORD		dwOwnerUserID;		//房主用户ID
	CT_WORD			wCurrPlayCount;		//当前局数
	CT_WORD			wTotalPlayCount;	//总局数
	CT_WORD			wUserCount;			//人数
	//CT_WORD			wEndLimit;			//结束最大值 （如达到总胡息...)
	//CT_WORD			wWinLimit;			//输赢最大值
	CT_BYTE			cbIsVip;			//是否VIP房
	//CT_BYTE			cbAuto;				//是否自动开房
	CT_DWORD		dwGroupID;			//群ID
	CT_DWORD		dwRecordID;			//记录ID
	CT_BYTE			cbGroupType;		//群组类型
	CT_CHAR			szOtherParam[PROOM_PARAM_LEN]; //游戏其他参数
};


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif



#endif











