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
#define KIND_ID						1										//游戏 I D
#define GAME_NAME					TEXT("跑得快")							//游戏名字

//组件属性
#define GAME_PLAYER					3										//游戏人数

//////////////////////////////////////////////////////////////////////////////////

//数目定义
#define MAX_COUNT					16										//最大数目
#define FULL_COUNT					48										//全牌数目


//逻辑数目
#define NORMAL_COUNT				16										//常规数目
#define DISPATCH_COUNT				48										//派发数目
#define GOOD_CARD_COUTN				24										//好牌数目
#define GOOD_CARD_MAX				10										//每人最多好牌数目
#define MAX_CARD_VALUE				15										//牌最大逻辑数值
#define MAX_CARD_COLOR				4										//牌最大花色数值

//数值掩码
#define	MASK_COLOR					0xF0									//花色掩码
#define	MASK_VALUE					0x0F									//数值掩码

//逻辑类型
#define CT_ERROR					0									//错误类型
#define CT_SINGLE					1									//单牌类型
#define CT_DOUBLE_Ex				2									//对牌类型
#define CT_THREE_EX					3									//三张
#define CT_THREE_TAKE_ONE			4									//三带一
#define CT_THREE_TAKE_TWO			5									//三带二
#define CT_SINGLE_LINE				6									//单连类型
#define CT_DOUBLE_LINE				7									//对连类型
#define CT_THREE_LINE				8									//三连
#define CT_THREE_LINE_ONE			9									//三连带一
#define CT_AIRPLANE					10									//三连带二 (飞机)
#define CT_FOUR_TAKE_ONE			11									//4带1
#define CT_FOUR_TAKE_TWO			12									//4带2
#define CT_FOUR_TAKE_THREE			13									//4带3
#define CT_FOUR_LINE				14									//4连
#define CT_FOUR_LINE_TWO			15									//4连带2
#define CT_FOUR_LINE_THREE			16									//4连带3
#define CT_BOMB_CARD				17									//炸弹类型

#define TIME_DEAL_CARD				2					                //发牌时间(2)
#define	TIME_OUT_CARD				12					                //出牌时间(15)
#define	TIME_NOT_OUT_CARD			(1)					                //要不起玩家的过牌时间(15)
#define TIME_FIRST_OUT_CARD			23					                //第一次出牌时间

//结算
#define SCORE_TYPE_LOSE				1										//积分类型(输)
#define SCORE_TYPE_WIN				2										//积分类型(赢)


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

