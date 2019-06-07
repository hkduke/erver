//
// Created by okMan on 2018/11/9.
//

#ifndef _MSG_FXGZ_H
#define _MSG_FXGZ_H

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

#define  ICON_ROW               3       //图案行数
#define  ICON_COL               5       //图案列数
#define  ICON_COUNT             11      //图案总数量
#define  REWARD_LEAST_COL       3       //获奖最少列数
#define  PAY_CASE_COUNT         4       //返奖方案
#define  CHEAT_CASE_COUNT       4       //血控方案
#define  POINT_CHEAT_CASE_COUNT 2       //点控方案
#define  ROOM_NUM				4		//4种房间类型

#define  MAX_REWARD_ICON_COUNT  3       //最大获奖励图案

#define  JETTON_COUNT			25      //下注数目

enum en_Icon_Id
{
	en_JinLuo		= 0,	//金锣
	en_CaiShenDao	= 1,    //财神到
	en_CaiShen		= 2,    //财神
	en_MiLeFo		= 3,    //弥勒佛
	en_TaiBai		= 4,    //太白
	en_JinYu		= 5,    //金鱼
	en_FuDai		= 6,    //福袋
	en_RuYi			= 7,    //如意
	en_TaoZi		= 8,    //桃子
	en_BaoZi		= 9,    //包子
	en_TangHuLu		= 10,   //糖葫芦
};

//////////////////////////////////////////////////////////////////////////
#define GAME_PLAYER								1								//游戏人数

//////////////////////////////////////////////////////////////////////////
//游戏状态
#define GAME_STATUS_FREE						100								//空闲状态
#define GAME_STATUS_PLAY						101								//游戏状态

//场景消息
#define SC_GAMESCENE_FREE						2001							//空闲场景消息
#define SC_GAMESCENE_PLAY						2002							//游戏场景消息

//空闲状态
struct FXGZ_MSG_GS_FREE
{
	CT_DOUBLE 		dUserLeaseScore;			//用户下注需要最少金币
};

//游戏状态
struct FXGZ_MSG_GS_PLAY
{
	CT_BYTE 		cbCurrentJettonIndex;		//当前下注金额
	CT_DOUBLE		dJetton[JETTON_COUNT];		//下注金额
};

//////////////////////////////////////////////////////////////////////////
//服务命令
#define     SUB_S_MAIN_START             100          	//主游戏开始(FXGZ_CMD_S_Start)
#define 	SUB_S_FREE_START			 101		  	//免费游戏开始（10组FXGZ_CMD_S_Start）
#define 	SUB_S_MAIN_STRAT_FAIL		 102			//主游戏开始失败

struct FXGZ_CMD_S_FlashIcon
{
    FXGZ_CMD_S_FlashIcon()
    : cbIconId(0)
    , cbFlashEndCol(0)
    { }

	CT_BYTE 	cbIconId;				//头像ID
	CT_BYTE 	cbFlashEndCol;			//闪到第几列
};

//游戏开始
struct FXGZ_CMD_S_Start
{
    FXGZ_CMD_S_Start()
    : cbFreeGame(0)
    , dSourceScore(0.0f)
    , dWinScore(0.0f)
    , cbBigWinner(0)
    , cbRewardCount(0)
    {
        memset(cbResultIcon, 0 , sizeof(cbResultIcon));
    }

	CT_BYTE					cbResultIcon[ICON_ROW][ICON_COL];		//图标结果
	CT_BYTE                 cbFreeGame;                             //是否获得免费游戏
	CT_DOUBLE 				dSourceScore;							//扣掉下注额后
	CT_DOUBLE 				dWinScore;								//玩家赢分
	CT_BYTE 				cbBigWinner;							//是否大赢家
	CT_BYTE                 cbRewardCount;                          //中奖数目
    FXGZ_CMD_S_FlashIcon 	flashIcon[MAX_REWARD_ICON_COUNT];  		//中奖图标
};

//游戏开始失败
struct FXGZ_CMD_S_StartFail
{
	CT_BYTE 				cbResult;		            //1: 金币不足 2:20元余额才能开始游戏，请您充值
};

//客户端命令
#define 	FXGZ_SUB_C_START			 1				//开始游戏

struct FXGZ_CMD_C_Start : public MSG_GameMsgUpHead
{
	CT_DOUBLE 				dJettonScore;				//下注筹码
};


#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif //_MSG_FXGZ_H

