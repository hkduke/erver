#ifndef		___MESSAGE_DDZ_H_DEF___
#define		___MESSAGE_DDZ_H_DEF___

#include "CGlobalData.h"
#include "GameLogic.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//公共宏定义

//组件属性
#define GAME_PLAYER					5										//游戏人数
#define MAX_JETTON_NUM				5										//最大筹码个数

//结束原因
#define GER_NO_PLAYER				1										//没有玩家
#define GER_COMPARECARD				2										//比牌结束
#define GER_ALLIN					3										//孤注一掷结束
#define GER_RUSH					4										//火拼(最后一个玩家有跟注)
#define GER_RUSH_GIVEUP				5										//火拼(最后一个玩家没有跟注)

#define GAME_SCENE_FREE				100										//等待开始
#define GAME_SCENE_PLAY				101										//游戏进行
#define GAME_SCENE_END				102										//游戏结束

#define GIVE_UP							2									//放弃概率
#define LOOK_CARD						4									//看牌概率
#define COMPARE_CARD					5									//比牌概率

//下注倍数和结束局数定义
//筹码最大倍数(相对于基础倍数)
#define MAX_JETTON_MULTIPLE		20
//加注筹码的倍数
#define JETTON_MULTIPLE_1		2
#define JETTON_MULTIPLE_2		4
//可以进行火拼局数
#define CAN_RUSH_JETTON_COUNT	3
//最大局数
#define MAX_JETTON_ROUND		50
//可以看牌
#define CAN_LOOKCARD_JETTON_COUNT 2

//////////////////////////////////////////////////////////////////////////
//服务器命令结构
//场景消息
#define SUB_SC_GAMESCENE_FREE			2001								//空闲场景消息
#define SUB_SC_GAMESCENE_PLAY			2002								//游戏中场景消息
#define SUB_SC_GAMESCENE_END			2003								//游戏结束

#define SUB_S_GAME_START				100									//游戏开始
#define SUB_S_ADD_SCORE					101									//加注结果
#define SUB_S_GIVE_UP					102									//放弃跟注
#define SUB_S_COMPARE_CARD				103									//比牌跟注
#define SUB_S_LOOK_CARD					104									//看牌跟注
#define SUB_S_GAME_END					105									//游戏结束
#define SUB_S_ALL_IN					106									//孤注一掷
#define SUB_S_ALL_IN_RESULT				107									//孤注一掷结果
#define SUB_S_OPEN_CARD					108									//开牌消息
#define	SUB_S_AUTO_FOLLOW				109									//跟得底【CMD_S_Auto_Follow】
#define	SUB_S_RUSH						110									//火拼【CMD_S_Rush】
#define SUB_S_RUSH_RESULT				111									//火拼结果【CMD_S_Rush_Result】
#define SUB_S_OUT_ROUND_END				112									//超过局数结束【CMD_S_OutRound_Result】

#define SUB_S_ANDROID_CARD				113									//智能消息
#define SUB_S_CHANGE_TABLE_TIPS			114									//换桌提示
#define SUB_S_GIVE_UP_TIME_OUT			115									//超时弃牌

//#define SUB_S_WAIT_COMPARE				107									//等待比牌
//#define SUB_S_CHEAT_CARD				110									//特殊命令
//#define SUB_S_ANDROID_CARD				111									//智能消息

//空闲状态
struct CMD_S_StatusFree
{
	CT_DOUBLE							dCellScore;							//基础积分
	CT_BYTE								cbCanRushCount;						//可以火拼的局数
};

//游戏状态
struct CMD_S_StatusPlay
{
	//加注信息
	CT_DOUBLE							dCellScore;							//基础积分
	CT_DOUBLE							dCurrentJetton;						//当前筹码值

	CT_DWORD							wBankerUser;						//庄家信息
	CT_DWORD							wCurrentUser;						//当前玩家
	CT_BYTE								cbPlayStatus[GAME_PLAYER];			//游戏状态
	bool								bMingZhu[GAME_PLAYER];				//看牌状态
	bool								bGiveUp[GAME_PLAYER];				//弃牌状态
	bool								bAutoFollow[GAME_PLAYER];			//跟到底

	CT_DOUBLE							dTableJetton[GAME_PLAYER];			//下注数目
	CT_DOUBLE							dTotalJetton;						//总注数目

	//扑克信息
	CT_BYTE								cbHandCardData[3];					//扑克数据
	CT_BYTE								cbHandCardType;						//牌型

	//火拼信息
	CT_BYTE								cbCanRushCount;						//可以火拼的局数
	CT_BYTE								cbRushState;						//火拼状态
	CT_DOUBLE							dRushJetton;						//火拼数目
	
	//状态信息
	CT_WORD								wJettonCount;						//下注次数(大于2时可以看牌比牌)

	CT_WORD								wTimeLeft;							//剩余时间
	CT_WORD								wTotalTime;							//总时间
};

//空闲状态
struct CMD_S_StatusEnd
{
	CT_WORD								wWaitTime;							//等待时间
	CT_BYTE								cbCanRushCount;						//可以火拼的局数
};


//游戏开始
struct CMD_S_GameStart
{
	//下注信息
	CT_DOUBLE							dCellScore;							//基础积分
	CT_DOUBLE							dCurrentJetton;						//当前筹码值

	//用户信息
	CT_DWORD							wBankerUser;						//庄家信息
	CT_DWORD				 			wCurrentUser;						//当前玩家																
	CT_BYTE								cbPlayStatus[GAME_PLAYER];			//用户状态
	CT_DOUBLE							dUserScore[GAME_PLAYER];			//每个玩家当前的金币
	CT_DOUBLE							dTotalJetton;						//当前总注
	CT_WORD								wTimeLeft;							//剩余时间
};

//用户下注
struct CMD_S_AddScore
{
	CMD_S_AddScore()
		:wCurrentUser(0)
		,wJettonCount(0)
		,wAddJettonUser(0)
		,dAddJettonCount(0.0f)
		,dUserTotalJetton(0.0f)
		,dTotalJetton(0.0f)
		,dUserCurrentScore(0.0f)
		,dCurrentJetton(0.0f)
		,dRustJetton(0.0f)
		,wTimeLeft(0)
		,cbState(1)
	{
	}

	CT_DWORD							wCurrentUser;						//当前用户
	CT_WORD								wJettonCount;						//当前用户下注次数

	CT_DWORD							wAddJettonUser;						//加注用户
	CT_DOUBLE							dAddJettonCount;					//加注数目
	CT_DOUBLE							dUserTotalJetton;					//加注用户总注
	CT_DOUBLE							dTotalJetton;						//总注
	CT_DOUBLE							dUserCurrentScore;					//用户当前积分
	CT_DOUBLE							dCurrentJetton;						//当前筹码值
	CT_DOUBLE							dRustJetton;						//火拼数目
	CT_WORD								wTimeLeft;							//剩余时间
	CT_BYTE								cbState;							//状态（1:跟注 2 加注）
};

//用户放弃
struct CMD_S_GiveUp
{
	CT_DWORD							wGiveUpUser;						//放弃用户
	CT_BYTE								cbSwitchUser;						//是否需要切换用户（1：切换 0：不切换）

	CT_DWORD							wCurrentUser;						//当前用户(如果收到65535表示游戏即将结束)
	CT_WORD								wJettonCount;						//当前用户下注次数
	CT_DOUBLE							dRustJetton;						//火拼数目
	CT_WORD								wTimeLeft;							//剩余时间
};

//比牌数据包
struct CMD_S_CompareCard
{
	CMD_S_CompareCard()
	{
		memset(this, 0, sizeof(*this));
	}
	CT_DWORD							wCurrentUser;						//当前用户(如果收到65535表示游戏即将结束)
	CT_DWORD							wCompareUser[2];					//比牌用户
	CT_DWORD							wLostUser;							//输牌用户
	CT_WORD								wJettonCount;						//当前用户下注次数

	CT_DWORD							wAddJettonUser;						//加注用户(比牌用户)
	CT_DOUBLE							dAddJettonCount;					//加注数目(比牌用户下的注)
	CT_DOUBLE							dUserTotalJetton;					//加注用户总注
	CT_DOUBLE							dTotalJetton;						//总注
	CT_DOUBLE							dUserCurrentScore;					//用户当前积分
	CT_DOUBLE							dCurrentJetton;						//单注
	CT_DOUBLE							dRustJetton;						//火拼数目
	CT_WORD								wTimeLeft;							//剩余时间
};

//看牌数据包
struct CMD_S_LookCard
{
	CT_DWORD							wLookCardUser;						//看牌用户
	CT_DWORD							wCurrentUser;						//当前用户
	CT_DOUBLE							dCurrentJetton;						//单注
	CT_BYTE								cbTimeLeft;							//剩余时间
	CT_BYTE								cbTotalTime;						//总时间
	CT_WORD								wJettonCount;						//当前用户下注次数
	CT_DOUBLE							dRustJetton;						//火拼数目
	CT_BYTE								cbCardType;							//牌型
	CT_BYTE								cbCardData[MAX_COUNT];				//用户扑克
};

//孤注一掷
struct CMD_S_AllIn
{
	CMD_S_AllIn()
	{
		memset(this, 0, sizeof(*this));
	}
	CT_DWORD							wCurrentUser;						//当前用户
	CT_WORD								wJettonCount;						//当前用户下注次数
	CT_DOUBLE							dCurrentJetton;						//当前用户单注

	CT_DWORD							wAllInUser;							//allIn用户
	CT_DOUBLE							dAllInJettonCount;					//allIn数目
	CT_DOUBLE							dAllInUserTotalJetton;				//allIn用户总注
	CT_DOUBLE							dTotalJetton;						//总注
	CT_DOUBLE							dAllInUserCurrentScore;				//allIn用户金币
	CT_BYTE								cbAllInUserGiveUp;					//用户是否弃牌

	CT_WORD								wTimeLeft;							//剩余时间
};

//孤注一掷比牌结果
struct CMD_S_AllIn_Result
{
	CT_DWORD							wCurrentUser;						//当前用户(65535表示游戏即将结束)
	CT_WORD								wJettonCount;						//当前用户下注次数
	CT_DOUBLE							dCurrentJetton;						//当前用户单注
	CT_DOUBLE							dRustJetton;						//火拼数目

	CT_DWORD							wAllInUser;							//allIn用户
	CT_DOUBLE							dAllInJettonCount;					//allIn数目
	CT_DOUBLE							dAllInUserTotalJetton;				//allIn用户总注
	CT_DOUBLE							dTotalJetton;						//总注
	CT_DOUBLE							dAllInUserCurrentScore;				//allIn用户金币
	//CT_BYTE								cbAllInUserGiveUp;					//用户是否弃牌

	//比牌结果
	CT_DWORD							wCompareUser[GAME_PLAYER];			//比牌用户[不参与比牌用户为65535]
	//CT_DWORD								wStartAllInUser;					//发起AllIn用户
	CT_BYTE								cbStartAllInUserWin;				//发起AllIn用户是否赢（0:输 1:赢）

	CT_WORD								wTimeLeft;							//剩余时间
};


//火拼
struct CMD_S_Rush
{
	CMD_S_Rush()
	{
		memset(this, 0, sizeof(*this));
	}
	CT_DWORD							wCurrentUser;						//当前用户
	CT_WORD								wJettonCount;						//当前用户下注次数
	CT_DOUBLE							dCurrentJetton;						//当前用户单注

	CT_DWORD							wRushUser;							//火拼用户
	CT_DOUBLE							dRushJettonCount;					//火拼数目
	CT_DOUBLE							dRushUserTotalJetton;				//火拼用户总注
	CT_DOUBLE							dTotalJetton;						//总注
	CT_DOUBLE							dRushUserCurrentScore;				//火拼用户金币
	CT_BYTE								cbRushUserGiveUp;					//用户是否弃牌

	CT_WORD								wTimeLeft;							//剩余时间
};

struct CMD_S_Rush_Result
{
	CMD_S_Rush_Result()
	{
		memset(this, 0, sizeof(*this));
	}

	//CT_DWORD							wCurrentUser;						//当前用户(65535表示游戏即将结束)
	//CT_WORD								wJettonCount;						//当前用户下注次数
	//CT_DOUBLE							dCurrentJetton;						//当前用户单注

	CT_DWORD							wRushUser;							//火拼用户
	CT_DOUBLE							dRushJettonCount;					//火拼金额
	CT_DOUBLE							dRushUserTotalJetton;				//火拼用户总注
	CT_DOUBLE							dTotalJetton;						//总注
	CT_DOUBLE							dRushUserCurrentScore;				//火拼用户金币
	CT_BYTE								cbRushUserGiveUp;					//用户是否弃牌

	//比牌结果
	CT_DWORD							wCompareUser[GAME_PLAYER];			//参与比牌用户[不参与比牌用户为65535]
	CT_DWORD							wRushWinUser;						//火拼赢用户
	//CT_DWORD							wStartRushUser;						//发起火拼用户
	//CT_BYTE								cbStartRushUserWin;					//发起火拼用户是否赢（0:输 1:赢）
};

struct CMD_S_OutRound_Result
{
	CMD_S_OutRound_Result()
	{
		for (int i = 0; i < GAME_PLAYER; ++i)
		{
			wCompareUser[i] = INVALID_CHAIR;
		}
		wRushWinUser = INVALID_CHAIR;
	}

	CT_DWORD							wCompareUser[GAME_PLAYER];			//参与比牌用户[不参与比牌用户为65535]
	CT_DWORD							wRushWinUser;						//火拼赢用户
};



//开牌数据包
struct CMD_S_OpenCard
{
	CT_DWORD							wWinner;							//胜利用户
	CT_BYTE								cbCardData[3];						//用户扑克
	CT_BYTE								cbCardType;							//牌型
};

//跟到底
struct CMD_S_Auto_Follow
{
	CT_DWORD							wAutoFollowUser;					//跟到底用户
	CT_DWORD							wCurrentUser;						//当前用户
	CT_WORD								wJettonCount;						//当前用户下注次数
	CT_BYTE								cbRushState;						//是否火拼
	CT_BYTE								cbState;							//状态（0：取消跟到底 1：跟到底）
};

//游戏结束
struct CMD_S_GameEnd
{
	CT_BYTE								cbGameState[GAME_PLAYER];			//游戏状态
	CT_DOUBLE							dGameScore[GAME_PLAYER];			//游戏得分
	CT_DOUBLE							dTotalScore[GAME_PLAYER];			//游戏总分
	CT_BYTE								cbCardData[GAME_PLAYER][3];			//用户扑克
	CT_BYTE								cbCardType[GAME_PLAYER];			//牌型

	CT_BYTE								wCompareUser[GAME_PLAYER];			//比牌用户
	CT_WORD								wTimeLeft;							//等待下一局开始时间
	CT_BYTE								cbEndState;							//结束状态(0表示比牌结束 1表示弃牌结束)
};

//机器人扑克
struct CMD_S_AndroidCard
{
	CT_BYTE								cbRealPlayer[GAME_PLAYER];				//真人玩家
	CT_BYTE								cbAndroidStatus[GAME_PLAYER];			//机器数目
	CT_BYTE								cbAllHandCardData[GAME_PLAYER][MAX_COUNT];//手上扑克
	CT_LONGLONG							lStockScore;							//当前库存
};


enum en_ChangeTable
{
	en_ReadyChangeTable = 1,			//下一局准备换桌
	en_FinishChangeTable = 2,			//换桌完成
};
//换桌提示
struct CMD_S_ChangeTable_Tips
{
	CT_BYTE 	cbResult;				//见en_ChangeTable
};


//客户端命令结构
#define SUB_C_ADD_SCORE					1									//用户加注
#define SUB_C_GIVE_UP					2									//放弃消息
#define SUB_C_COMPARE_CARD				3									//比牌消息
#define SUB_C_LOOK_CARD					4									//看牌消息
#define SUB_C_ALL_IN					5									//孤注一掷
#define SUB_C_OPEN_CARD					6									//开牌消息
#define SUB_C_AUTO_FOLLOW				7									//跟到底
#define SUB_C_CANCEL_AUTO_FOLLOW		8									//取消跟到底
#define SUB_C_RUSH						9									//火拼

//用户加注
struct CMD_C_AddScore : public MSG_GameMsgUpHead
{
	CT_DOUBLE							dScore;								//加注数目
};

//比牌数据包
struct CMD_C_CompareCard : public MSG_GameMsgUpHead
{
	CT_WORD								wCompareUser;						//比牌用户
};

//////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

