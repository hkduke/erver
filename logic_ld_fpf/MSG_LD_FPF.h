#ifndef		___MESSAGE_LD_FPF_H_DEF___
#define	___MESSAGE_LD_FPF_H_DEF___

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

////////////////////////////////////////////////////////////////////////
#define GAME_ID						        1								//游戏 I D
#define GAME_PLAYER							3								//游戏人数

//数值定义
#define MAX_WEAVE							7								//最大组合
#define MAX_WEAVE_CARD						4								//最大组合牌张数
#define MAX_INDEX							20								//最大索引
#define MAX_COUNT							21								//最大手牌张数
#define MAX_TI_CARD							5								//最大提牌个数
#define MAX_ITEM_COUNT						10								//最大吃牌组合个数
#define MAX_MT_COUNT						11								//最大名堂个数
#define START_HU_XI							15								//起始胡息

//动作定义
#define ACTION_COUNT						9								//动作个数(在数组中零下标不使用)
#define ACK_NULL							0								//空
#define ACK_CHI								1								//吃
#define ACK_CHI_EX							2								//绞牌
#define ACK_PENG							3								//碰
#define ACK_PAO								4								//跑
#define ACK_WEI								5								//偎
#define ACK_TI								6								//提
#define ACK_CHIHU							7								//胡
#define ACK_JIANG							8								//将牌

//全名堂定义
#define FPF_MT_ZI_MO						0								//自　摸(x2)
#define FPF_MT_YI_DIAN_HONG					1								//一点红(x2)
#define FPF_MT_YI_KUAI_BIAN					2								//一块扁(x2)
#define FPF_MT_SHI_HONG						3								//十红(x2)
#define FPF_MT_SHI_SAN_HONG					4								//十三红(+100胡息)
#define FPF_MT_WU_HU						5								//乌胡(+100胡息)
#define FPF_MT_HAI_DI_HU					6								//海底胡(x2)
#define FPF_MT_20_KA_HU						7								//20卡胡(x2)
#define FPF_MT_30_KA_HU						8								//30卡胡(+100胡息)
#define FPF_MT_TIAN_HU						9								//天胡(+100胡息)
#define FPF_MT_DI_HU						10								//地胡(+100胡息)

//////////////////////////////////////////////////////////////////////////
//游戏状态
#define GAME_STATUS_FREE					100							//空闲状态
#define GAME_STATUS_PLAY					102							//游戏中状态
#define GAME_STATUS_END						103							//游戏结束

//场景消息
#define SC_GAMESCENE_FREE					2001						//空闲场景消息
#define SC_GAMESCENE_PLAY					2002						//游戏中场景消息
#define SC_GAMESCENE_END					2003						//游戏结束

//组合子项
struct tagWeaveItem
{
	CT_BYTE							cbWeaveKind;						//组合类型
	CT_BYTE							cbWeaveHuXi;						//组合胡息
	CT_BYTE							cbCardCount;						//扑克数目
	CT_BYTE							cbCenterCard;						//中心扑克
	CT_BYTE							cbShowCenter;						//显示中心牌
	CT_BYTE							cbCardList[MAX_WEAVE_CARD];			//扑克列表
};

//吃牌信息
struct tagChiCardInfo
{
	CT_BYTE							cbChiKind;							//吃牌类型
	CT_BYTE							cbCenterCard;						//中心扑克
	CT_BYTE							cbResultCount;						//结果数目
	CT_BYTE							cbCardData[3][3];					//吃牌组合
};

//胡牌信息
struct tagHuCardInfo
{
	CT_BYTE							cbCardEye;							//牌眼扑克
	CT_BYTE							cbHuXiCount;						//胡息数目
	CT_BYTE							cbWeaveCount;						//组合数目
	tagWeaveItem					WeaveItemArray[MAX_WEAVE];			//组合扑克
};

//名堂数据
struct  MT_Data
{
	CT_WORD						wType;									//名堂类型
	CT_WORD						wAward[2];								//0:下标加胡息，1:下标剩倍数
};

//空闲状态
struct FPF_MSG_GS_FREE
{
	CT_DWORD						dwCellScore;						//基础积分
	CT_WORD						wCurrPlayCount;							//当前局数
};

//游戏中状态
struct FPF_MSG_GS_PLAY
{
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_DWORD						dwCellScore;								//基础积分
	CT_WORD							wCurrPlayCount;								//当前局数
	CT_DWORD						dwBankerUser;								//庄家用户
	CT_DWORD						dwCurrentUser;								//当前用户(出牌玩家)

	CT_BYTE							cbActionStatus;								//动作状态(1摸牌,2出牌)
	CT_DWORD						dwProvideUser;								//供应玩家(cbActionStatuso为1: 摸牌玩家; 为2:出牌玩家)
	CT_BYTE							cbProvideCard;								//供应牌(cbActionStatuso为1: 摸的牌; 为2:出的牌)

	CT_BYTE							cbActionCode[ACTION_COUNT];					//动作代码（吃，碰，跑, 胡）
	CT_BYTE							cbMustIndex;								//必须吃索引(默认255)
	CT_DWORD						dwActionNumber;								//动作流水号
	CT_BYTE							cbChiWeaveCount;							//吃牌组合个数(吃用)
	CT_BYTE							cbChiWeaveCard[MAX_ITEM_COUNT][3];			//吃牌组合牌数据(吃用)

	CT_BYTE							cblastCardCount;							//牌堆剩余牌数

	CT_BYTE							cbDiscardCardCount[GAME_PLAYER];			//出牌数
	CT_BYTE							cbDiscardCard[GAME_PLAYER][MAX_COUNT];		//出牌记录 

	CT_BYTE							cbUserCardCount;							//牌张数
	CT_BYTE							cbUserCard[MAX_COUNT];						//牌数据
	CT_BYTE							cbWeaveItemCount[GAME_PLAYER];				//组合数目
	tagWeaveItem					WeaveItem[GAME_PLAYER][MAX_WEAVE];			//组合数据
	CT_BYTE							cbUserQiHu[GAME_PLAYER];					//玩家弃胡
	CT_WORD						wCurHuXi[GAME_PLAYER];							//当前胡息
};

//游戏结束状态
struct FPF_MSG_GS_END
{
	CT_BYTE							cbTimeLeave;								//剩余时间
	CT_DWORD						dwCellScore;								//基础积分
	CT_WORD						wCurrPlayCount;									//当前局数
	CT_DWORD						dwBankerUser;								//庄家用户
		
	CT_BYTE							cblastCardCount;							//牌堆剩余牌数

	CT_BYTE							cbDiscardCardCount[GAME_PLAYER];			//出牌数
	CT_BYTE							cbDiscardCard[GAME_PLAYER][MAX_COUNT];		//出牌记录 

	CT_BYTE							cbUserCardCount;							//牌张数
	CT_BYTE							cbUserCard[MAX_COUNT];						//牌数据
	CT_BYTE							cbWeaveItemCount[GAME_PLAYER];				//组合数目
	tagWeaveItem					WeaveItem[GAME_PLAYER][MAX_WEAVE];			//组合数据
	CT_BYTE							cbUserQiHu[GAME_PLAYER];					//玩家弃胡
};


//////////////////////////////////////////////////////////////////////////
//服务器命令结构
#define FPF_SUB_S_GAME_START						100							//游戏开始[FPF_CMD_S_GameStart]
#define FPF_SUB_S_OUT_CARD							101							//玩家出牌[FPF_CMD_S_OutCard]
#define FPF_SUB_S_SEND_CARD							102							//发牌命令[FPF_CMD_S_SendCard]
#define FPF_SUB_S_OPERATE_RESULT					103							//玩家操作结果[FPF_CMD_S_OperateResult]
#define FPF_SUB_S_GAME_END							104							//游戏结束[FPF_CMD_S_GameEnd]
#define FPF_SUB_S_USER_QI_HU						105							//玩家弃胡[FPF_CMD_S_UserQiHu]
#define FPF_SUB_S_TING_CARD							106							//听牌数据[FPF_CMD_S_TingCard]

#define FPF_SUB_S_CLEARING							200							//牌局结算[FPF_CMD_S_CLEARING]


//游戏开始(发手牌)
struct FPF_CMD_S_GameStart
{
	CT_DWORD								dwBankerUser;						//庄家玩家
	CT_DWORD								dwCurrentUser;						//当前玩家（可不用）							
	CT_BYTE									cbCardData[MAX_COUNT];				//手牌数据
	CT_BYTE									cbCardCount;						//手牌数量
	CT_BYTE									cbLastCard;							//庄家最后摸的牌
	CT_WORD									wCurrPlayCount;						//当前局数

	CT_BYTE									cbTiCardList[MAX_TI_CARD];			//庄家提牌扑克列表
	CT_BYTE									cbTiCardCount;						//庄家提牌扑克个数
	CT_BYTE									cbOutCardTime;						//出牌时间
	CT_DWORD								dwCurOutCardUser;					//当前出牌玩家
	CT_WORD									wCurHuXi[GAME_PLAYER];				//当前胡息
};

//用户出牌
struct FPF_CMD_S_OutCard
{
	CT_DWORD								dwOutCardUser;									//出牌玩家
	CT_BYTE									cbOutCard;										//出牌扑克

	CT_BYTE									cbTiCardList[GAME_PLAYER][MAX_TI_CARD];			//庄家提牌扑克列表
	CT_BYTE									cbTiCardCount[GAME_PLAYER];						//庄家提牌扑克个数

	CT_DWORD								dwActionNumber;									//动作流水号
	CT_DWORD								dwPaoCardUser;									//跑牌玩家
	CT_BYTE									cbActionTime;									//动作时间
	CT_BYTE									cbActionCode[ACTION_COUNT];						//动作代码（吃，碰，跑，喂，提，胡）
	CT_BYTE									cbActionCard;									//动作扑克

	CT_BYTE									cbMustIndex;									//必须吃索引(默认255)
	CT_BYTE									cbChiWeaveCount;								//吃牌组合个数(吃用)
	CT_BYTE									cbChiWeaveCard[MAX_ITEM_COUNT][3];				//吃牌组合牌数据(吃用)

	//有跑牌才有可能
	CT_BYTE									cbOutCardTime;									//出牌时间
	CT_DWORD								dwCurOutCardUser;								//当前出牌玩家
	CT_WORD									wCurHuXi[GAME_PLAYER];							//当前胡息
};

//发牌命令
struct FPF_CMD_S_SendCard
{
	CT_DWORD								dwSendCardUser;									//发牌玩家
	CT_BYTE									cbSendCard;										//发牌扑克
	CT_BYTE									cbShow;											//是否显示

	CT_DWORD								dwActionNumber;									//动作流水号
	CT_DWORD								dwWeiTiPaoUser;									//偎提跑玩家

	CT_BYTE									cbActionTime;									//动作时间
	CT_BYTE									cbActionCode[ACTION_COUNT];						//动作代码（吃，碰，跑，喂，提，胡）
	CT_BYTE									cbActionCard;									//动作扑克

	CT_BYTE									cbMustIndex;									//必须吃索引(默认255)
	CT_BYTE									cbChiWeaveCount;								//吃牌组合个数(吃用)
	CT_BYTE									cbChiWeaveCard[MAX_ITEM_COUNT][3];				//吃牌组合牌数据(吃用)

	//有提偎跑牌才有可能
	CT_BYTE									cbOutCardTime;									//出牌时间
	CT_DWORD								dwCurOutCardUser;								//当前出牌玩家
	CT_WORD									wCurHuXi[GAME_PLAYER];							//当前胡息
};

//操作结果
struct FPF_CMD_S_OperateResult
{
	CT_DWORD								dwOperateUser;									//操作玩家
	CT_DWORD								dwOperateCode;									//操作代码
	CT_BYTE									cbOperateCard;									//操作牌
	CT_BYTE									cbBaiCardCount;									//摆牌个数(吃用)
	CT_BYTE									cbBaiCard[MAX_ITEM_COUNT][3];					//摆牌数据(吃用)

	CT_BYTE									cbOutCardTime;									//出牌时间
	CT_DWORD								dwCurOutCardUser;								//当前出牌玩家
	CT_WORD									wCurHuXi[GAME_PLAYER];							//当前胡息
};

//游戏结束
struct  FPF_CMD_S_GameEnd
{
	CT_BYTE									cbReason;										//结束原因(0荒庄,1正常结束)
	CT_BYTE									cbHuCard;										//胡牌扑克
	CT_BYTE									cbHuCardType;									//胡牌类型(0平胡,1放炮,2自模)
	CT_DWORD								dwHuCardUser;									//胡牌玩家
	CT_DWORD								dwProvideUser;									//放跑玩家

	CT_BYTE									cbMTType[MAX_MT_COUNT];							//名堂类型
	CT_WORD									wMTAward[MAX_MT_COUNT][2];						//0:下标加胡息，1:下标剩倍数

	CT_WORD									wTotalMultiple;									//总倍数
	CT_WORD									wBaseHuXi;										//基础胡息
	CT_WORD									wTotalHuXi;										//总胡息
	CT_INT32								iLWHuXi[GAME_PLAYER];							//当局输赢胡息		
	//CT_INT32								iTotalLWHuXi[GAME_PLAYER];						//累计输赢胡息
	//CT_INT32								iTotalHZScore[GAME_PLAYER];						//总荒庄分

	CT_BYTE									cblastCardCount;								//剩余牌数
	CT_BYTE									cbLastCard[MAX_COUNT];							//剩余牌

	tagWeaveItem							HuWeaveItem[MAX_WEAVE];							//胡牌玩家组合数据(胡牌为七个组合)
	CT_BYTE									cbUserWeaveCount[GAME_PLAYER];					//组合数目
	tagWeaveItem							UserWeaveArray[GAME_PLAYER][MAX_WEAVE];			//组合扑克
	CT_BYTE									cbRemainCardCount[GAME_PLAYER];					//剩余牌数量
	CT_BYTE									cbRemainCard[GAME_PLAYER][MAX_COUNT];			//剩余牌

    CT_DOUBLE						        dCellScore;							            //单元积分
    CT_DOUBLE						        dGameScore[GAME_PLAYER];			            //游戏积分
};

//听牌数据
struct FPF_CMD_S_TingCard
{
	CT_BYTE									cbCardCount;									//听牌个数
	CT_BYTE									cbCardList[MAX_INDEX];							//听的牌
};

//玩家弃胡
struct FPF_CMD_S_UserQiHu
{
	CT_DWORD								dwQiHuUser;										//弃胡玩家
	CT_BYTE									cbTiCardList[GAME_PLAYER][MAX_TI_CARD];			//庄家提牌扑克列表
	CT_BYTE									cbTiCardCount[GAME_PLAYER];						//庄家提牌扑克个数
	CT_WORD									wCurHuXi[GAME_PLAYER];							//当前胡息
};

//牌局结算
struct FPF_CMD_S_CLEARING
{
	CT_WORD									wHuCount[GAME_PLAYER];							//各玩家胡牌次数
	CT_WORD									wFPCount[GAME_PLAYER];							//各玩家放炮次数
	CT_WORD									wHZCount[GAME_PLAYER];							//各玩荒庄次数
	CT_INT32								iTotalHuXi[GAME_PLAYER];						//各玩家累计胡息
	CT_INT32								iTotalLWScore[GAME_PLAYER];						//各玩家总输赢分
	CT_CHAR									szCurTime[TIME_LEN];							//当前时间
};


//////////////////////////////////////////////////////////////////////////
//客户端往服务器端发消息(C->S)
#define FPF_SUB_C_OUT_CARD					1												//出牌消息
#define FPF_SUB_C_OPERATE					2												//操作消息
#define FPF_SUB_C_QI_HU						3												//弃胡消息[空消息]

//出牌命令
struct FPF_CMD_C_OutCard : public MSG_GameMsgUpHead
{
	CT_BYTE									cbOutCard;										//扑克数据
};

//操作命令
struct FPF_CMD_C_OperateCard : public MSG_GameMsgUpHead
{
	CT_DWORD								dwOperateCode;									//操作代码(1吃,2碰,4胡,0过)
	CT_BYTE									cbOperateCard;									//操作牌
	CT_BYTE									cbBaiCardCount;									//摆牌个数(吃用)
	CT_BYTE									cbBaiCard[MAX_ITEM_COUNT][3];					//摆牌数据(吃用)
	CT_DWORD								dwActionNumber;									//动作流水号
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

