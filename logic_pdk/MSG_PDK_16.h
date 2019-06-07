#ifndef		___MESSAGE_PDK_16_H_DEF___
#define		___MESSAGE_PDK_16_H_DEF___

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

////////////////////////////////////////////////////////////////////////

#define GAME_PLAYER							3								//游戏人数

//数值定义
#define MAX_COUNT							16								//最大手牌张数

//////////////////////////////////////////////////////////////////////////
//游戏状态
#define GAME_STATUS_FREE					100								//空闲状态
#define GAME_STATUS_PLAY					102								//游戏中状态
#define GAME_STATUS_END						103								//游戏结束


//场景消息
#define SC_GAMESCENE_FREE					2001							//空闲场景消息
#define SC_GAMESCENE_PLAY					2002							//游戏中场景消息
#define SC_GAMESCENE_END					2003							//游戏结束

//空闲状态
struct FPF_MSG_GS_FREE
{
	CT_DWORD					dwCellScore;								//基础积分
	CT_WORD						wCurrPlayCount;								//当前局数
};

//游戏中状态
struct FPF_MSG_GS_PLAY 
{
	CT_DWORD					dwCellScore;								//基础积分
	CT_WORD						wCurrPlayCount;								//当前局数
	CT_BYTE						cbTimeLeave;								//剩余时间
	CT_DWORD					dwCurrentUser;								//当前用户(出牌玩家)
	CT_BYTE						cbIsPass;									//是否不出(1是不出)
	CT_BYTE						cbMustThree;								//必出黑桃3
	CT_BYTE						cbCardCount[GAME_PLAYER];					//手牌数量
	CT_BYTE						cbCardData[MAX_COUNT];						//手牌数据

	CT_DWORD					dwTurnWiner;								//胜利玩家
	CT_BYTE						cbTurnCardCount;							//出牌数目
	CT_BYTE						cbTurnCardData[MAX_COUNT];					//出牌数据
	CT_BYTE						cbSingle[GAME_PLAYER];						//是否报单
	CT_BYTE						cbRombCount[GAME_PLAYER];					//炸弹个数
};

//游戏结束
struct FPF_MSG_GS_END
{
	CT_DWORD					dwCellScore;							    //基础积分
	CT_WORD						wCurrPlayCount;								//当前局数
	CT_BYTE						cbTimeLeave;								//剩余时间
	CT_BYTE						cbRemainCount[GAME_PLAYER];					//剩余牌数量
	CT_BYTE						cbRemainCard[GAME_PLAYER][MAX_COUNT];		//剩余牌
};

//////////////////////////////////////////////////////////////////////////
//服务器命令结构
#define PDK_16_SUB_S_SEND_CARD						100						//游戏开始[PDK_16_CMD_S_SendCard]
#define PDK_16_SUB_S_NOTIFY_OUT_CARD				101						//通知出牌[PDK_16_CMD_S_NotifyOutCard]
#define PDK_16_SUB_S_OUT_CARD_RESULT				102						//出牌结果[PDK_16_CMD_S_OutCardResult]
#define PDK_16_SUB_S_GAME_END						103						//游戏结束[PDK_16_CMD_S_GameEnd]

#define PDK_16_SUB_S_CLEARING						200						//牌局结算[PDK_16_CMD_S_CLEARING]


//发送手牌
struct PDK_16_CMD_S_SendCard
{
	CT_DWORD					dwFirstUser;								//首出牌玩家	
	CT_WORD						wCurrPlayCount;								//当前局数
	CT_BYTE						cbMustThree;								//必出黑桃3
	CT_BYTE						cbCardCount;								//手牌数量
	CT_BYTE						cbCardData[MAX_COUNT];						//手牌数据
	CT_BYTE						cbOutCardTime;								//出牌时间
};

//提示出牌
struct PDK_16_CMD_S_NotifyOutCard
{
	CT_DWORD					dwOutCardUser;								//出牌玩家
	CT_BYTE						cbOutCardTime;								//出牌时间
	CT_BYTE						cbOutCardFlag;								//出牌标志0:不能出，1:能出
	CT_BYTE						cbNewTurn;									//新一轮标志1:新一轮
	CT_BYTE						cbMustThree;								//必出黑桃3
};

//出牌结果
struct PDK_16_CMD_S_OutCardResult
{
	CT_DWORD					dwOutCardUser;								//出牌玩家
	CT_BYTE						cbCardCount;								//出牌数目
	CT_BYTE						cbCardData[MAX_COUNT];						//扑克列表
	CT_BYTE						cbSingle;									//是否报单
	

	CT_DWORD					dwCurrentUser;								//当前出牌玩家
	CT_BYTE						cbIsPass;									//是否出牌(1是不出)
	CT_BYTE						cbOutCardTime;								//出牌时间
	CT_BYTE						cbNewTurn;									//新一轮标志1:新一轮
};

//游戏结算
struct PDK_16_CMD_S_GameEnd
{
	CT_LONGLONG					llLWScore[GAME_PLAYER];						//输赢分数(底分*张数)
	CT_LONGLONG					llRombScore[GAME_PLAYER];					//炸弹分数
	CT_INT32					iLWNum[GAME_PLAYER];						//输赢张数(张数)
	CT_WORD						wWinUser;									//赢家
	CT_BYTE						cbBeiGuan[GAME_PLAYER];						//被关标志
	CT_BYTE						cbRemainCount[GAME_PLAYER];					//剩余牌数量
	CT_BYTE						cbRemainCard[GAME_PLAYER][MAX_COUNT];		//剩余牌
	CT_BYTE						cbRombCount[GAME_PLAYER];					//炸弹个数
};

//牌局结算
struct PDK_16_CMD_S_CLEARING
{
	CT_LONGLONG					llTotalLWScore[GAME_PLAYER];				//各玩家总输赢分d
	CT_WORD						wRoomOwner;									//房主
	CT_BYTE						cbQuanGuanCount[GAME_PLAYER];				//各玩家全关次数
	CT_BYTE						cbBeiGuanCount[GAME_PLAYER];				//各玩家被关次数
	CT_BYTE						cbRombCount[GAME_PLAYER];					//各玩家炸弹个数
	CT_BYTE						cbTongPeiCount[GAME_PLAYER];				//各玩家通赔个数
	CT_CHAR						szCurTime[TIME_LEN];						//当前时间
};


//////////////////////////////////////////////////////////////////////////
//客户端往服务器端发消息(C->S)
#define PDK_16_SUB_C_OUT_CARD			1									//出牌消息
#define PKD_16_SUB_C_PASS_CARD			2									//不出牌

//出牌命令
struct PDK_16_CMD_C_OutCard : public MSG_GameMsgUpHead
{
	CT_BYTE						cbCardCount;								//出牌数目
	CT_BYTE						cbCardData[MAX_COUNT];						//扑克列表
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

