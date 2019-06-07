//
// Created by okMan on 2018/11/9.
//

#ifndef _MSG_DCDF_H
#define _MSG_DCDF_H

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

#define  GAME_ID						        82								//游戏 I D
#define  GAME_NAME					TEXT("多财多福/dancing-drums")							//游戏名字

#define  ICON_MAX_ROW           6       //图案行数
#define  ICON_ROW               3       //图案行数
#define  ICON_COL               5       //图案列数
#define  ICON_COUNT             13      //图案总数量
#define  ROOM_NUM				4		//4种房间类型
#define  JACK_POT				4		//4种将池
#define  MAX_STAKE_TYPE                          15                              //下注等级

//游戏图标ID
enum en_Icon_Id
{
	en_Scatter		    = 0,	//鼓：SCATTER，免费游戏图标，前3列连续出现将触发免费游戏。
	en_Wild	            = 1,    //福：WILD，百搭图标，可代替其他普通图标进行赔付。
	en_GoldenLion		= 2,    //金狮
	en_GoldenBoat		= 3,    //金船
	en_GoldenTree		= 4,    //金摇钱树
	en_GoldenIngots	    = 5,    //金元宝
	en_Gold		        = 6,    //金币
	en_A			    = 7,    //A
	en_K		        = 8,    //K
	en_Q		        = 9,    //Q
	en_J		        = 10,   //J
    en_10		        = 11,   //10
    en_9		        = 12,   //9
};
//免费游戏类型
enum en_Free_Game_Type
{
    en_Error_Type       = 0,    //错误类型
    en_3X5_15		    = 1,    //15次[3×5]免费游戏
    en_4X5_10		    = 2,    //10次[4×5]免费游戏
    en_5X5_5		    = 3,    //5次[5×5]免费游戏
    en_6X5_3		    = 4,    //3次[6×5]免费游戏
    en_Sys_Rand         = 5,    //神秘精选（系统随机选中前4种组合的免费次数和框格大小，再组合成免费奖励）
};
//////////////////////////////////////////////////////////////////////////
//空闲状态
struct tagFreeGameInfo
{
    CT_BYTE                 cbJettonIndex;              //下注筹码索引
    CT_DWORD                dwGetFreeTime;              //获得免费游戏时间
    CT_BYTE 				cbFreeGameType;				//免费游戏类型
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
struct MSG_GS_FREE
{
	CT_DOUBLE 		dUserLeaseScore;			    //用户下注需要最少金币
    CT_DOUBLE       dStakeJetton[MAX_STAKE_TYPE];   //下注分数等级--配置信息
};


//////////////////////////////////////////////////////////////////////////
//服务命令
#define     SUB_S_MAIN_START             100          	//主游戏开始(FXGZ_CMD_S_Start)
#define 	SUB_S_FREE_START			 101		  	//免费游戏开始（10组FXGZ_CMD_S_Start）
#define 	SUB_S_MAIN_STRAT_FAIL		 102			//主游戏开始失败
#define 	SUB_S_FREE_GAME_FAIL		 103			//免费游戏类型
#define 	SUB_S_FREE_GAME_TYPE		 104			//免费游戏类型
#define 	SUB_S_JACK_POT			     105			//获取将池信息

struct CMD_S_FlashIcon
{
    CMD_S_FlashIcon()
    : cbIconId(0)
    , cbFlashEndCol(0)
    { }

	CT_BYTE 	cbIconId;				//头像ID
	CT_BYTE 	cbFlashEndCol;			//闪到第几列
};
struct CMD_S_Jackpot
{
    CT_BYTE                 cbJettonIndex;              //下注筹码索引
    CT_DOUBLE               dJackpot[JACK_POT];                //将池信息
};
//游戏开始
struct CMD_S_Start
{
    CMD_S_Start()
    : cbFreeGame(0)
    , dSourceScore(0.0f)
    , dWinScore(0.0f)
    , cbBigWinner(0)
    , cbRewardCount(0)
    , cbWildCount(0)
    {
        memset(cbResultIcon, 0 , sizeof(cbResultIcon));
		memset(flashIcon, 0 , sizeof(flashIcon));
        memset(&stJackPotInfo, 0 , sizeof(stJackPotInfo));
    }

	CT_BYTE					cbResultIcon[ICON_ROW][ICON_COL];		//图标结果
	CT_BYTE                 cbFreeGame;                             //免费游戏--0没有免费游戏，1-4为有免费游戏，及其不点的话默认的免费游戏类型
	CT_DOUBLE 				dSourceScore;							//扣掉下注额后
	CT_DOUBLE 				dWinScore;								//玩家赢分
	CT_BYTE 				cbBigWinner;							//是否大赢家
	CT_BYTE                 cbRewardCount;                          //中奖数目
	CT_BYTE                 cbWildCount;                            //福的数量
    CMD_S_FlashIcon 		flashIcon[ICON_ROW];  		        //中奖图标
    CMD_S_Jackpot           stJackPotInfo;                          //此下注对应的彩金池的最新信息
};
//免费游戏开始
struct CMD_S_FreeGameStart
{
    CMD_S_FreeGameStart()
            : cbFreeGame(0)
            , dSourceScore(0.0f)
            , dWinScore(0.0f)
            , cbBigWinner(0)
            , cbRewardCount(0)
            , cbWildCount(0)
    {
        memset(cbResultIcon, 0 , sizeof(cbResultIcon));
		memset(flashIcon, 0 , sizeof(flashIcon));
        memset(&stJackPotInfo, 0 , sizeof(stJackPotInfo));
    }
    CT_BYTE					cbResultIcon[ICON_MAX_ROW][ICON_COL];	//图标结果
    CT_BYTE                 cbFreeGame;                             //是否获得免费游戏
    CT_DOUBLE 				dSourceScore;							//扣掉下注额后
    CT_DOUBLE 				dWinScore;								//玩家赢分
    CT_BYTE 				cbBigWinner;							//是否大赢家
    CT_BYTE                 cbRewardCount;                          //中奖数目
    CT_BYTE                 cbWildCount;                            //福的数量
    CMD_S_FlashIcon 		flashIcon[ICON_MAX_ROW];  		    //中奖图标
    CMD_S_Jackpot           stJackPotInfo;                          //此下注对应的彩金池的最新信息
};
//游戏开始失败
struct CMD_S_StartFail
{
    CT_BYTE 				cbResult;		            //1: 金币不足 2:20元余额才能开始游戏，请您充值
};
//游戏开始失败
struct CMD_S_FreeGameFail
{
    CT_BYTE 				cbResult;		            //1: 没有你的免费游戏 2:类型错误
};
//免费游戏类型
struct CMD_S_FreeGameType
{
    CT_BYTE                 cbRows;                                 //行数
    CT_BYTE                 cbCount;                                //次数
};
//客户端命令
#define 	SUB_C_START			        1				//开始游戏
#define 	SUB_C_FREE_GAME_TYPE			2				//免费游戏类型
#define 	SUB_C_JACK_POT			        3				//获取将池信息

struct CMD_C_Start : public MSG_GameMsgUpHead
{
    CT_BYTE                 cbJettonIndex;              //下注筹码索引
};

struct CMD_C_FreeGame : public MSG_GameMsgUpHead
{
    CT_BYTE 				cbFreeGameType;				//免费游戏类型
};

struct CMD_C_GetJackpot : public MSG_GameMsgUpHead
{
    CT_BYTE                 cbJettonIndex;              //下注筹码索引
};
#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif //_MSG_DCDF_H

