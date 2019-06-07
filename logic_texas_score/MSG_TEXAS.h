#ifndef		___MESSAGE_TEXAS_H_DEF___
#define		___MESSAGE_TEXAS_H_DEF___

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//服务定义
//////////////////////////////////////////////////////////////////////////////////
//游戏属性
#define KIND_ID						1										//游戏 I D
#define GAME_NAME					TEXT("德州扑克")							//游戏名字

//组件属性
#define GAME_PLAYER					9										//游戏人数

//////////////////////////////////////////////////////////////////////////////////

enum CardType
{
	//扑克类型
	CT_SINGLE					= 1,								//单牌类型
	CT_ONE_LONG					= 2,								//对子类型
	CT_TWO_LONG					= 3,								//两对类型
	CT_THREE_TIAO				= 4,								//三条类型
	CT_SHUN_ZI					= 5,								//顺子类型
	CT_TONG_HUA					= 6,								//同花类型
	CT_HU_LU					= 7,								//葫芦类型
	CT_TIE_ZHI					= 8,								//铁支类型
	CT_TONG_HUA_SHUN			= 9,								//同花顺型
	CT_KING_TONG_HUA_SHUN		= 10								//皇家同花顺
};
//机器人牌型
enum RobotCardType
{
	//其他圈牌型 HC代表手牌
	RCT_OTHER					= 20,								//其他牌型
	RCT_SINGLE_HC_2ToJ			= 21,								//高牌(手牌2-J)
	RCT_SINGLE_HC_QToA			= 22,								//高牌(手牌Q-A)
	RCT_ONE_PAIR_1HC			= 23,								//一对(1张手牌)
	RCT_ONE_PAIR_2HC			= 24,								//一对(2张手牌)
	RCT_TWO_PAIR_HC_PAIR		= 25,								//两对(手对)
	RCT_TWO_PAIR_1HC			= 26,								//两对(1张手牌)
	RCT_TWO_PAIR_2HC			= 27,								//两对(2张手牌 无手对)
	RCT_THREE_KIND_1HC			= 28,								//三条(1张手牌)
	RCT_STRAIGHT_1HC			= 29,								//顺子(1张手牌)
	RCT_FLUSH_1HC_2TO7			= 30,								//同花(1张手牌2-7)
	RCT_FLUSH_1HC_8TOT			= 31,								//同花(1张手牌8-T)
	RCT_THREE_KIND_2HC			= 32,								//三条(2张手牌)
	RCT_STRAIGHT_2HC			= 33,								//顺子(2张手牌)
	RCT_FLUSH_1HC_JTOA			= 34,								//同花(1张手牌J-A)
	RCT_FLUSH_2HC_2TO7			= 35,								//同花(2张手牌2-7)
	RCT_FLUSH_2HC_8TOT			= 36,								//同花(2张手牌8-T)
	RCT_FLUSH_2HC_JTOA			= 37,								//同花(2张手牌J-A)
	RCT_FULL_HOUSE				= 38,								//葫芦
	RCT_FOUR_KIND				= 39,								//金刚(以上)


	//底牌圈牌型 -- R0
	RCT_R0_OTHER				= 67,								//其他牌型
	RCT_R0_7_9ToT				= 68,								//7(9-T)
	RCT_R0_6_8ToT				= 69,								//6(8-T)
	RCT_R0_5_7ToT				= 70,								//5(7-T)
	RCT_R0_4_6ToT				= 71,								//4(6-T)
	RCT_R0_3_5ToT				= 72,								//3(5-T)
	RCT_R0_2_4ToT				= 73,								//2(4-T)
	RCT_R0_A_5ToT				= 74,								//不同花A(5-T)
	RCT_R0_J_8ToT				= 75,								//不同花J(8-T)
	RCT_R0_Q_9ToJ				= 76,								//不同花Q(9-J)
	RCT_R0_K_TToQ				= 77,								//不同花K(T-Q)
	RCT_R0_A_2To4				= 78,								//不同花A(2-4)
	RCT_R0_A_JToK				= 79,								//不同花A(J-K)
	RCT_R0_S_J_2To7				= 80,								//同花J(2-7)
	RCT_R0_S_Q_2To8				= 81,								//同花Q(2-8)
	RCT_R0_S_K_2To9				= 82,								//同花K(2-9)
	RCT_R0_S_A_5ToT				= 83,								//同花A(5-T)
	RCT_R0_S_3_2				= 84,								//同花32
	RCT_R0_S_4_3				= 85,								//同花43
	RCT_R0_S_5_4				= 86,								//同花54
	RCT_R0_S_6_5				= 87,								//同花65
	RCT_R0_S_7_6				= 88,								//同花76
	RCT_R0_S_8_7				= 89,								//同花87
	RCT_R0_S_9_8				= 90,								//同花98
	RCT_R0_S_T_9				= 91,								//同花T9
	RCT_R0_S_J_8ToT				= 92,								//同花J(8-T)
	RCT_R0_S_Q_9ToJ				= 93,								//同花Q(9-J)
	RCT_R0_S_K_TToQ				= 94,								//同花K(T-Q)
	RCT_R0_S_A_2To4				= 95,								//同花A(2-4)
	RCT_R0_S_A_JToK				= 96,								//同花A(J-K)
	RCT_R0_22To55				= 97,								//22-55
	RCT_R0_66ToTT				= 98,								//66-TT
	RCT_R0_JJToKK				= 99,								//JJ?KK
	RCT_R0_AA					= 100,								//AA
};
enum TableStatus
{
	TABLE_STATUS_READY = 1,				//准备状态，桌子的初始状态
	TABLE_STATUS_START = 2,				//游戏开始状态，也指刚发牌状态
	TABLE_STATUS_BET = 3,				//下注状态
	TABLE_STATUS_END = 4,				//游戏结算状态
};
enum UserOperate
{
	USE_OPERATE_FLOD = 1,				//弃牌
	USE_OPERATE_CHECK = 2,				//过牌
	USE_OPERATE_CALL = 3,				//跟注
	USE_OPERATE_BET = 4,				//加注
	USE_OPERATE_ALLIN = 5,				//全下
};
enum UserPreOperate
{
	USE_PRE_OPERATE_CANCEL = 0,				//取消预操作
	USE_PRE_OPERATE_FLOD_CHECK = 1,			//弃牌过牌
	USE_PRE_OPERATE_CHECK = 2,				//过牌
	USE_PRE_PERATE_CALL = 3,				//跟任何注
};
enum UserSitResult
{
	SIT_RESULT_SUCCESS = 0,				//成功
	SIT_RESULT_CHIP_INSUFFICIENT = 1,	//筹码不足坐下失败
	SIT_RESULT_HAS_OTHER = 2,			//此位置有人坐下失败
	SIT_RESULT_ALREADY_SIT = 3,			//不能换位置
	SIT_RESULT_MONEY_INSUFFICIENT = 4,	//筹码和钱之和不够
};



//////////////////////////////////////////////////////////////////////////////////
//场景消息
#define SC_GAMESCENE_FREE					2001							//空闲场景消息
#define SC_GAMESCENE_PLAY					2003							//游戏中场景消息
#define SC_GAMESCENE_END					2004							//游戏结束

//状态定义

#define GAME_SCENE_FREE						100								//等待开始
#define GAME_SCENE_PLAY						102								//游戏进行
#define GAME_SCENE_END						103								//游戏结束


//胜负信息
#define GAME_WIN							0								//胜
#define GAME_LOSE							1								//负

//空闲状态
struct CMD_S_StatusFree
{
	//游戏属性
	CT_DOUBLE					dCellScore;							//基础积分
	CT_WORD						dwCurrPlayCount;

};


//游戏状态
struct CMD_S_StatusPlay
{
    CT_WORD						    wFirstOutChairID;					    //首出玩家
	//时间信息
	CT_BYTE							cbLeftTime;								//剩余时间
	//游戏变量
	CT_DOUBLE						dCellScore;								//单元积分
	CT_DWORD						dwCurrentUser;							//当前玩家

	//出牌信息
	CT_WORD							wTurnWiner;								//出牌玩家
	CT_BYTE							cbTurnCardCount;						//出牌数目
	CT_BYTE							cbTurnCardData[MAX_COUNT];				//出牌数据

	//扑克信息
	CT_BYTE							cbHandCardCount[GAME_PLAYER];			//扑克数目
	CT_BYTE							cbHandCardData[MAX_COUNT];				//手上扑克
	CT_BYTE							cbOutCount[GAME_PLAYER];				//出牌次数
	CT_BYTE							cbOutCardListCount[GAME_PLAYER][MAX_COUNT];			//每次出牌张数
	CT_BYTE							cbOutCardList[GAME_PLAYER][MAX_COUNT][MAX_COUNT];	//出牌列表
	//CT_BYTE							cbRemainCard[MAX_CARD_VALUE];			//剩余牌数据

	CT_BYTE							cbTrustee[GAME_PLAYER];					//是否托管
};

//游戏结束
struct CMD_S_StatusEND
{
	CT_DOUBLE					dCellScore;									//基础积分
	CT_BYTE						cbTimeLeave;								//剩余时间
																			//扑克信息
	CT_BYTE						cbHandCardCount[GAME_PLAYER];				//扑克数目
	CT_BYTE						cbHandCardData[MAX_COUNT];					//手上扑克
};

//////////////////////////////////////////////////////////////////////////////////
//命令定义

#define SUB_S_GAME_START			100										//游戏开始
#define SUB_S_OUT_CARD				103										//用户出牌
#define SUB_S_PASS_CARD				104										//用户放弃
#define SUB_S_OUT_START_START		105										//开始出牌
#define SUB_S_GAME_CONCLUDE			106										//游戏结束
#define SUB_S_CLEARING				107										//牌局结算
#define SUB_S_TRUSTEE				108										//托管
#define SUB_S_CHEAT_LOOK_CARD		110										//看牌
#define SUB_S_BOMB_SCORE			111										//炸弹分
//发送扑克
struct CMD_S_GameStart
{
	CT_DWORD						dwStartUser;							//回放座位号
	CT_DWORD				 		dwCurrentUser;							//当前玩家
	CT_BYTE							cbCardData[NORMAL_COUNT];				//扑克列表
	CT_BYTE							cbTimeLeft;								//剩余时间
};

struct CMD_S_GameStartAi
{
	CT_DWORD				 		dwCurrentUser;							//当前玩家
	CT_BYTE							cbCardData[GAME_PLAYER][NORMAL_COUNT];	//扑克列表
	CT_BYTE							cbBankerCard[3];						//底牌
    CT_BYTE							cbTimeLeft;								//剩余时间
};

//开始出牌信息
struct CMD_S_StartOutCard
{
	CT_DWORD				 		dwCurrentUser;						//当前玩家
	CT_BYTE							cbLeftTime;							//剩余时间
};


//用户出牌
struct CMD_S_OutCard
{
	CT_BYTE							cbIsFail;							//一轮结束,0成功，1客户端发送牌数量是0，2出牌数量大于手牌数量，3扑克数据空，4不是当前玩家，5牌型错误，6牌型不匹配，7服务器删除扑克错误
	CT_BYTE							cbTimeLeft;							//剩余时间
	CT_BYTE							cbCardCount;						//出牌数目
	CT_DWORD				 		dwCurrentUser;						//当前玩家
	CT_DWORD						dwOutCardUser;						//出牌玩家
	CT_BYTE							cbCardData[MAX_COUNT];				//扑克列表
};

//放弃出牌
struct CMD_S_PassCard
{
	CT_BYTE							cbIsFail;							//一轮结束,0成功，1失败
	CT_BYTE							cbTurnOver;							//一轮结束
	CT_DWORD				 		dwCurrentUser;						//当前玩家
	CT_DWORD				 		dwPassCardUser;						//放弃玩家
	CT_BYTE							cbTimeLeft;
};
//炸弹分
struct CMD_S_BombScore
{
	CT_DOUBLE						dBombScore[GAME_PLAYER];			//游戏积分
};

//作弊看牌
struct CMD_S_CheatLookCard
{
	CT_WORD							wCardUser;							//看牌用户
	CT_BYTE							cbCardCount;						//出牌数目
	CT_BYTE							cbCardData[MAX_COUNT];				//扑克列表
};

//游戏结束
struct PDK_CMD_S_GameEnd
{
	//积分变量
	CT_DOUBLE						dCellScore;							//单元积分
	CT_DOUBLE						dGameScore[GAME_PLAYER];			//游戏积分
	CT_BYTE							bLose[GAME_PLAYER];					//胜负标志
	//春天标志
	CT_BYTE							bChunTian[GAME_PLAYER];				//关标志，1全关，2反关
	CT_BYTE							bWinMaxScore;						//是否达到赢钱封顶


	//游戏信息
	CT_BYTE							cbCardCount[GAME_PLAYER];				//扑克数目
	CT_BYTE							cbHandCardData[GAME_PLAYER][MAX_COUNT];	//扑克列表

	CT_BYTE							cbEachBombCount[GAME_PLAYER];		//炸弹个数ss
	CT_DOUBLE						dEachBombScore[GAME_PLAYER];		//炸弹分数
};

//牌局结算
struct PDK_CMD_S_CLEARING
{
	CT_LONGLONG					llTotalLWScore[GAME_PLAYER];			//各玩家总输赢分d
	CT_WORD						wRoomOwner;								//房主
	//CT_BYTE					cbQuanGuanCount[GAME_PLAYER];		//各玩家全关次数
	//CT_BYTE					cbBeiGuanCount[GAME_PLAYER];		//各玩家被关次数
	CT_BYTE						cbRombCount[GAME_PLAYER];				//各玩家炸弹个数
	//CT_BYTE					cbTongPeiCount[GAME_PLAYER];		//各玩家通赔个数
	CT_CHAR						szCurTime[TIME_LEN];					//当前时间
	CT_WORD						wCurrPlayCount;							//当前局数
	CT_WORD						wPlayWinCount[GAME_PLAYER];				//赢局数
	CT_WORD						wPlayLoseCount[GAME_PLAYER];			//输局数
};

//积分信息
struct tagScoreInfo
{
	CT_INT32						iScore;								//积分
	CT_BYTE							cbType;								//类型
};

//游戏托管
struct CMD_S_StatusTrustee
{
	CT_BYTE						cbTrustee[GAME_PLAYER];					//托管信息
};
//////////////////////////////////////////////////////////////////////////////////
//命令定义

#define SUB_C_OUT_CARD				2									//用户出牌
#define SUB_C_PASS_CARD				3									//用户放弃
#define SUB_C_TRUSTEE				4									//用户托管
#define SUB_C_CANCEL_TRUSTEE		5									//用户取消托管
#define SUB_C_CHEAT_LOOK_CARD		7									//作弊看牌	



//////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

