#ifndef		___MESSAGE_DDZ_H_DEF___
#define		___MESSAGE_DDZ_H_DEF___

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
#define KIND_ID						26										//游戏 I D
#define GAME_NAME					TEXT("斗地主")							//游戏名字

//组件属性
#define GAME_PLAYER					3										//游戏人数

//////////////////////////////////////////////////////////////////////////////////

//数目定义
#define MAX_COUNT					20										//最大数目
#define FULL_COUNT					54										//全牌数目
#define MAX_NO_CALL_BANKER			3										//最多只能三局不叫地主


//逻辑数目
#define NORMAL_COUNT				17										//常规数目
#define DISPATCH_COUNT				51										//派发数目
#define GOOD_CARD_COUTN				38										//好牌数目
#define MAX_CARD_VALUE				15										//牌最大逻辑数值
#define MAX_CARD_COLOR				4										//牌最大花色数值

//数值掩码
#define	MASK_COLOR					0xF0									//花色掩码
#define	MASK_VALUE					0x0F									//数值掩码

//逻辑类型
#define CT_ERROR					0										//错误类型
#define CT_SINGLE					1										//单牌类型
#define CT_DOUBLE_CARD				2										//对牌类型
#define CT_THREE					3										//三条类型
#define CT_SINGLE_LINE				4										//单连类型
#define CT_DOUBLE_LINE				5										//对连类型
#define CT_THREE_LINE				6										//三连类型
#define CT_THREE_TAKE_ONE			7										//三带一单
#define CT_THREE_TAKE_TWO			8										//三带一对
#define CT_FOUR_TAKE_ONE			9										//四带两单
#define CT_FOUR_TAKE_TWO			10										//四带两对
#define CT_BOMB_CARD				11										//炸弹类型
#define CT_MISSILE_CARD				12										//火箭类型


//底牌类型
#define BCT_GENERAL					0										//普通类型
#define BCT_FULSH					1										//顺子类型
#define BCT_STRAIGHT				2										//同花类型
#define BCT_STRAIGHT_FULSH			3										//同花顺类型
#define BCT_SINGLE_MISSILE			4										//单王类型
#define BCT_DOUBLE_MISSILE			5										//对王类型
#define BCT_THREE					6										//三条类型

//结算
#define SCORE_TYPE_LOSE				1										//积分类型(输)
#define SCORE_TYPE_WIN				2										//积分类型(赢)


//////////////////////////////////////////////////////////////////////////////////
//场景消息
#define SC_GAMESCENE_FREE					2001							//空闲场景消息
#define SC_GAMESCENE_CALL					2002							//叫地主状态
#define SC_GAMESCENE_PLAY					2003							//游戏中场景消息
#define SC_GAMESCENE_END					2004							//游戏结束
#define SC_GAMESCENE_DOUBLE					2005							//加倍场景

//状态定义

#define GAME_SCENE_FREE						100								//等待开始
#define GAME_SCENE_CALL						101								//叫地主状态
#define GAME_SCENE_PLAY						102								//游戏进行
#define GAME_SCENE_END						103								//游戏结束
#define GAME_SCENE_DOUBLE					104								//用户加倍

//叫地主
#define CB_NOT_CALL							0								//没叫地主
#define CB_CALL_BENKER_1					1								//叫地主1分
#define CB_CALL_BENKER_2					2								//叫地主2分
#define CB_CALL_BENKER_3					3								//叫地主3分
#define CB_NO_CALL_BENKER					4								//不叫地主

//加倍信息
#define CB_NOT_ADD_DOUBLE					0								//没加倍
#define CB_ADD_DOUBLE						1								//加倍标志
#define CB_NO_ADD_DOUBLE					2								//不加倍

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

//叫地主状态
struct CMD_S_StatusCall
{
	//时间信息
	CT_BYTE							cbLeftTime;							//剩余时间

	//游戏信息
	CT_DOUBLE						dCellScore;							//单元积分
	CT_WORD							wCurrentUser;						//当前玩家
	CT_WORD							cbCallBankerInfo[GAME_PLAYER];		//叫地主信息	
	CT_BYTE							cbHandCardData[NORMAL_COUNT];		//手上扑克
	CT_BYTE							cbTrustee[GAME_PLAYER];				//是否托管
};

//加倍状态
struct CMD_S_StatusDouble
{
	//游戏信息
	CT_BYTE							cbLeftTime;							//剩余时间
	CT_BYTE							cbCallScore;						//叫分
	CT_DOUBLE						dCellScore;							//单元积分
	CT_WORD							wBankerUser;						//庄家用户
	CT_WORD							wCurrTime;							//当前倍数
	CT_BYTE							cbDoubleInfo[GAME_PLAYER];			//加倍信息
	CT_BYTE							cbBankerCard[3];					//游戏底牌
	CT_BYTE							cbHandCardCount[GAME_PLAYER];		//扑克数目
	CT_BYTE							cbHandCardData[MAX_COUNT];			//手上扑克
	CT_BYTE							cbTrustee[GAME_PLAYER];				//是否托管
};

//游戏状态
struct CMD_S_StatusPlay
{
	//时间信息
	CT_BYTE							cbLeftTime;								//剩余时间
	//游戏变量
	CT_DOUBLE						dCellScore;								//单元积分
	CT_DWORD						dwBankerUser;							//庄家用户
	CT_DWORD						dwCurrentUser;							//当前玩家

	CT_BYTE							cbCallScore;							//叫分
	CT_DWORD						dwBombTime;								//炸弹倍数(地主*2)
	CT_BYTE							cbAddDoubleInfo[GAME_PLAYER];			//加倍信息

	//出牌信息
	CT_WORD							wTurnWiner;								//出牌玩家
	CT_BYTE							cbTurnCardCount;						//出牌数目
	CT_BYTE							cbTurnCardData[MAX_COUNT];				//出牌数据

	//扑克信息
	CT_BYTE							cbBankerCard[3];						//游戏底牌
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
	CT_WORD						wCurrPlayCount;								//当前局数
	CT_BYTE						cbTimeLeave;								//剩余时间
																			//扑克信息
	CT_BYTE						cbBankerCard[3];							//游戏底牌
	CT_BYTE						cbHandCardCount[GAME_PLAYER];				//扑克数目
	CT_BYTE						cbHandCardData[MAX_COUNT];					//手上扑克
};

//////////////////////////////////////////////////////////////////////////////////
//命令定义

#define SUB_S_GAME_START			100										//游戏开始
#define SUB_S_CALL_BANKER			101										//叫地主
#define SUB_S_BANKER_INFO			102										//庄家信息
#define SUB_S_OUT_CARD				103										//用户出牌
#define SUB_S_PASS_CARD				104										//用户放弃
#define SUB_S_OUT_START_START		105										//开始出牌
#define SUB_S_GAME_CONCLUDE			106										//游戏结束
#define SUB_S_CLEARING				107										//牌局结算
#define SUB_S_TRUSTEE				108										//托管
#define SUB_S_ADD_DOUBLE			109										//加倍[CMD_S_Double]
#define SUB_S_CHEAT_LOOK_CARD		110										//看牌

//发送扑克
struct CMD_S_GameStart
{
	CT_DWORD						dwStartUser;							//开始玩家
	CT_DWORD				 		dwCurrentUser;							//当前玩家
	CT_DWORD						dwStartTime;							//初始倍数
	CT_BYTE							cbCardData[NORMAL_COUNT];				//扑克列表
	CT_BYTE							cbTimeLeft;								//剩余时间
};

struct CMD_S_GameStartAi
{
	CT_DWORD						dwStartUser;							//开始玩家
	CT_DWORD				 		dwCurrentUser;							//当前玩家
	CT_DWORD						dwStartTime;							//初始倍数
	CT_BYTE							cbCardData[GAME_PLAYER][NORMAL_COUNT];	//扑克列表
	CT_BYTE							cbBankerCard[3];						//底牌
};

//用户叫地主
struct CMD_S_CallBanker
{
	CT_DWORD				 		dwCurrentUser;							//当前玩家
	CT_DWORD				 		dwBankerUser;							//庄家玩家
	CT_DWORD						dwLastUser;								//下一个叫地主玩家
	CT_WORD							cbCallInfo;								//叫地主分数
	CT_BYTE							cbTimeLeft;								//剩余时间
};

//加倍信息
struct CMD_S_Double
{
	CT_DWORD						dwCurrentUser;						//当前用户
	CT_BYTE							cbDouble;							//是否加倍(0 未操作 1 加倍 2 不加倍)
	CT_WORD							wUserTimes;							//用户倍数
};

//开始出牌信息
struct CMD_S_StartOutCard
{
	CT_DWORD				 		dwBankerUser;						//庄家玩家
	CT_DWORD				 		dwCurrentUser;						//当前玩家
	CT_BYTE							cbLeftTime;							//剩余时间
};

//庄家信息
struct CMD_S_BankerInfo
{
	CT_DWORD				 		dwBankerUser;						//庄家玩家
	CT_DWORD				 		dwCurrentUser;						//当前玩家
	CT_BYTE							cbBankerCard[3];					//庄家扑克
	CT_BYTE							cbTimeLeft;							//剩余时间
};

//用户出牌
struct CMD_S_OutCard
{
	CT_BYTE							cbTimeLeft;							//剩余时间
	CT_BYTE							cbCardCount;						//出牌数目
	CT_DWORD				 		dwCurrentUser;						//当前玩家
	CT_DWORD						dwOutCardUser;						//出牌玩家
	CT_WORD							wUserTimes;							//用户倍数
	CT_BYTE							cbCardData[MAX_COUNT];				//扑克列表
};

//放弃出牌
struct CMD_S_PassCard
{
	CT_BYTE							cbTurnOver;							//一轮结束
	CT_DWORD				 		dwCurrentUser;						//当前玩家
	CT_DWORD				 		dwPassCardUser;						//放弃玩家
	CT_BYTE							cbTimeLeft;
	//CT_WORD							wCurrPlayCount;						//当前局数
};

//作弊看牌
struct CMD_S_CheatLookCard
{
	CT_WORD							wCardUser;							//看牌用户
	CT_BYTE							cbCardCount;						//出牌数目
	CT_BYTE							cbCardData[MAX_COUNT];				//扑克列表
};

//游戏结束
struct DDZ_CMD_S_GameEnd
{
	//积分变量
	//CT_DOUBLE						dCellScore;							//单元积分
	CT_BYTE							cbCallScore;						//叫分
	CT_DOUBLE						dGameScore[GAME_PLAYER];			//游戏积分
	CT_BYTE							bLose[GAME_PLAYER];					//胜负标志							

	//春天标志
	CT_BYTE							bChunTian;							//春天标志
	CT_BYTE							bFanChunTian;						//春天标志
	CT_BYTE							bWinMaxScore;						//是否达到赢钱封顶

	//炸弹信息
	CT_WORD							cbBombTimes;						//炸弹倍数(不包括火箭)
	CT_BYTE							cbMissileTimes;						//火箭倍数
	CT_BYTE							cbDoubleInfo[GAME_PLAYER];			//加倍信息

	//游戏信息
	CT_BYTE							cbCardCount[GAME_PLAYER];				//扑克数目
	CT_BYTE							cbHandCardData[GAME_PLAYER][MAX_COUNT];	//扑克列表
};

//牌局结算
struct DDZ_CMD_S_CLEARING
{
	CT_LONGLONG					llTotalLWScore[GAME_PLAYER];			//各玩家总输赢分d
	CT_WORD						wRoomOwner;								//房主
	//CT_BYTE						cbQuanGuanCount[GAME_PLAYER];		//各玩家全关次数
	//CT_BYTE						cbBeiGuanCount[GAME_PLAYER];		//各玩家被关次数
	CT_BYTE						cbRombCount[GAME_PLAYER];				//各玩家炸弹个数
	//CT_BYTE						cbTongPeiCount[GAME_PLAYER];		//各玩家通赔个数
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

#define SUB_C_CALL_BANKER			1									//用户叫地主
#define SUB_C_OUT_CARD				2									//用户出牌
#define SUB_C_PASS_CARD				3									//用户放弃
#define SUB_C_TRUSTEE				4									//用户托管
#define SUB_C_CANCEL_TRUSTEE		5									//用户取消托管
#define SUB_C_ADD_DOUBLE			6									//用户加倍[CMD_C_Double]
#define SUB_C_CHEAT_LOOK_CARD		7									//作弊看牌	

//用户地主
struct CMD_C_CallBanker : public MSG_GameMsgUpHead
{
	CT_WORD							cbCallInfo;							//叫地主
};

//用户加倍
struct CMD_C_Double : public MSG_GameMsgUpHead
{
	CT_BYTE							cbDoubleInfo;						//加倍信息(1：加倍 2：不加倍)
};

//用户出牌
struct CMD_C_OutCard : public MSG_GameMsgUpHead
{
	CT_BYTE							cbCardCount;						//出牌数目
	CT_BYTE							cbCardData[MAX_COUNT];				//扑克数据
};

//用户托管
struct CMD_C_Trustee : public MSG_GameMsgUpHead
{
	CT_WORD							wChairID;							//托管玩家id
};

//用户取消托管
struct CMD_C_CancelTrustee : public MSG_GameMsgUpHead
{
	CT_WORD							wChairID;							//取消托管玩家id
};


//用户看牌
struct CMD_C_CheatLookCard : public MSG_GameMsgUpHead
{
	CT_WORD							wBeCheatChairID;					//被偷看玩家椅子ID
};
//////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

