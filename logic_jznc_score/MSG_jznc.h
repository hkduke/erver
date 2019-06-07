#ifndef		___MESSAGE_DDZ_H_DEF___
#define		___MESSAGE_DDZ_H_DEF___

#include "CGlobalData.h"
#include <string.h>
#include <vector>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//服务定义
//////////////////////////////////////////////////////////////////////////////////
//游戏属性
#define KIND_ID						10										//游戏 I D
#define GAME_NAME					TEXT("金猪纳财")							//游戏名字


//////////////////////////////////////////////////////////////////////////////////

//结算
#define SCORE_TYPE_LOSE				1										//积分类型(输)
#define SCORE_TYPE_WIN				2										//积分类型(赢)
#define TIME_ANIMATION				2					                    //动画时间
#define TIME_ANIMATION_DICE			1					                    //动画时间-骰子开始时间
//////////////////////////////////////////////////////////////////////////////////
//场景消息
#define SC_GAMESCENE_FREE					2001							//空闲场景消息
#define SC_GAMESCENE_BET					2002							//游戏中下注场景消息
#define SC_GAMESCENE_THROWDICE				2003							//游戏中投骰场景消息
#define SC_GAMESCENE_END					2004							//游戏结束(暂时没有)

//状态定义
#define GAME_SCENE_FREE						101								//等待开始
#define GAME_SCENE_BET						102								//游戏下注
#define GAME_SCENE_THROWDICE				103								//游戏投骰
#define GAME_SCENE_END						104								//游戏结束(暂时没有)


#define GAME_PLAYER								8								//游戏人数
#define MAX_LEVEL							    6								//最大场次
#define TO_SCORE                                100
#define RAND(a,b,c) 							(((rand() + (c))% ((b)-(a)))+ (a))      //随机一个数在a,b范围内的数，c 是防止循环内调用每次随机的值都是相同的
#define RAND_NUM(a)                             (rand()+(a))                    //随机数
#define MAX_CONFIG                              4                               //配置项最大
////////////////////////////////////////////////////////////////////////////////////////////
//玩家控制方式
enum en_PlayCtrlType
{
    en_NoCtrl   = 0,                            //不点控
    en_PlayerEatScore = -1,                    //玩家吃分
    en_PlayerOutScore = 1,                   //玩家放分
};
//血池状态
enum en_StockStatus
{
	en_EatScore = 1,                    //吃分
	en_OutScore = -1,                   //放分
};
/////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////
//空闲状态
struct CMD_S_StatusFree
{
	//游戏属性
    CT_DOUBLE				    dCellScore;						            //基础积分
};
//下注状态
struct CMD_S_StatusBet
{
    //游戏属性
    CT_DOUBLE				    dCellScore;						            //基础积分
    CT_WORD						wTimeLeft;								    //下注剩余时间
    CT_BYTE                     cbBet[GAME_PLAYER];                         //是否下注 0未下注，1下注
};
//下注状态
struct CMD_S_StatusThrowDice
{
    //游戏属性
    CT_DOUBLE				    dCellScore;						            //基础积分
    CT_WORD						wTimeLeft;								    //投骰剩余时间
    CT_BYTE                     cbThrowDice[GAME_PLAYER];                   //是否投骰   0未投骰，1投骰
};
//游戏结束
struct CMD_S_StatusEND
{
    //游戏属性
    CT_DOUBLE				    dCellScore;						            //基础积分
};


//////////////////////////////////////////////////////////////////////////////////
//命令定义

#define SUB_S_START_GAME			100										//游戏开始
#define SUB_S_BET					101									    //用户下注
#define SUB_S_START_THROW_DICE		102										//开始投骰
#define SUB_S_THROW_DICE		    103										//用户投骰
#define SUB_S_GAME_CONCLUDE			104									    //游戏结算

//广播游戏开始
struct CMD_S_StartGame
{
	CT_WORD							wTimeLeft;								//下注剩余时间
};
//广播用户下注结果
struct CMD_S_Bet
{
    CT_BYTE                         cbIsFail;                           //是否失败
    CT_WORD							wChairID;							//玩家id
};
//开始投骰
struct CMD_S_StartThrowDice
{
    CT_WORD							wTimeLeft;								//投骰剩余时间
};
//投骰结果
struct CMD_S_ThrowDice
{
    CT_WORD							wChairID;							//玩家id
    CT_BYTE							cbDice;								//投骰
};
//广播游戏结算
struct CMD_S_GameEnd
{
    CT_DOUBLE						dWinScore[GAME_PLAYER];							//飘分（包含本金）
};

//////////////////////////////////////////////////////////////////////////////////
//命令定义
#define SUB_C_BET					        1									//用户下注
#define SUB_C_THROW_DICE					2									//投掷骰子

//用户下注
struct CMD_C_UserBet : public MSG_GameMsgUpHead
{
    CT_WORD							wChairID;							//玩家id
};
//投掷骰子
struct CMD_C_ThrowDice : public MSG_GameMsgUpHead
{
    CT_WORD							wChairID;							//玩家id
};

//////////////////////////////////////////////////////////////////////////////////
//黑名单
struct tagJZNCGameBlackList
{
    CT_DWORD 		dwUserID;							//黑名单用户ID
    CT_WORD 		wUserLostRatio;						//用户输的概率
    CT_LONGLONG    	llControlScore;                     //控制的下注额度
    CT_LONGLONG    	llCurrControlScore;                 //当前控制的下注额度
};
struct tagPlayerBet
{
    CT_BYTE                         cbCount;
    CT_BYTE                         cbIsBet[GAME_PLAYER];
};
struct tagPlayerDice
{
    CT_BYTE                         cbCount;
    CT_BYTE                         cbIsDice[GAME_PLAYER];
};
//////////////////////////////////////////////////////////////////////////////////
//库存控制
struct tagAIStockCtrlCfg
{
    CT_LONGLONG     llStockLowerScore;                  //房间库存存量值,单位:分,库存下限
    CT_LONGLONG     llStockUpperScore;                  //房间库存存量值,单位:分,库存上线
    CT_DWORD        dwActiveProb;                       //系统存量控制机器人作弊概率
    CT_INT32        iUnitCtrlStep;                      //控制时机器人单位控制步长,根据游戏匹配中机器人数量来计算最终控制个数,向下取整,但最终控制不得低于iCtrlMinCnt
    CT_INT32        iCtrlMinCnt;                        //控制时机器人最低控制数量,与单位步长计算值取最大值
    CT_INT32        iCtrlEffect;                        //控制作弊的机器人控制效果
};
//玩家自动控制
struct tagPlayerAutoCtrlCfg
{
    CT_INT32        iContWinLossLowerCnt;               //玩家连续输赢的次数下线。
    CT_INT32        iContWinLossUpperCnt;               //玩家连续输赢的次数上线
    CT_DWORD        iCtrlProb;                          //控制玩家赢的概率,万分比
    CT_INT32        iCtrlEffect;                        //成功控制时返奖效果,未成功则默认为0
};
//玩家自动控制
struct tagPointCtrlCfg
{
    CT_INT32        iCheatRatioLower;                   //下限，玩家点控作弊率为负,则点控为赢,根据作弊率反向匹配小等于,若满足则判定是否执行?若大于最低档则采用最低档
    CT_INT32        iCheatRatioUpper;                   //上线，玩家点控作弊率为正,则点控为输,根据作弊率反向匹配大等于,若满足则判定是否执行?若小于最低档则采用最低档
    CT_DWORD        iCtrlProb;                          //点控概率
    CT_INT32        iExeEffect;                         //执行方案
};
//玩家基础数据
typedef struct GAME_SimpleUserInfo
{
    GAME_SimpleUserInfo()
    {
        dwUserID = 0;							//用户id
        cbGender = 0;							//性别
        cbVipLevel = 0;							//VIP等级
        cbHeadIndex = 0;						//头像索引
        bRobot = false;
        llWinLoseScore = 0;
        dwFirstTime = 0;
        dwPointCtlEndTime = 0;
        dwContinueWinCount = 0;
        dwContinueLoseCount = 0;
    }
    CT_DWORD		dwUserID;							//用户id
    CT_BYTE         cbGender;							//性别
    CT_BYTE			cbVipLevel;							//VIP等级
    CT_BYTE			cbHeadIndex;						//头像索引
    CT_BOOL 		bRobot;								//机器人
    CT_LONGLONG		llWinLoseScore;                     //累计输赢
    CT_DWORD        dwFirstTime;                        //首次加入时间
    CT_DWORD        dwPointCtlEndTime;                  //点控结束时间
    CT_DWORD		dwContinueWinCount;			        //连续赢的次数
    CT_DWORD		dwContinueLoseCount;			    //连续输的次数
}GS_SimpleUserInfo;
//////////////////////////////////////////////////////////////////////////////////
#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

