#ifndef		___MESSAGE_FISH_H_DEF___
#define		___MESSAGE_FISH_H_DEF___

#include "CGlobalData.h"
#include <string.h>
#include <vector>
#include <map>
#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif


#define CountArray(A) sizeof(A)/sizeof(A[0])
#define MAKELG(a, b) ((CT_LONG)(((CT_WORD)(a)) | ((CT_DWORD)((CT_WORD)(b))) << 16))

#define LOWORD(l)           ((CT_WORD)(((CT_DWORD)(l)) & 0xffff))
#define HIWORD(l)           ((CT_WORD)((((CT_DWORD)(l)) >> 16) & 0xffff))
#define LOCB(w)           ((CT_BYTE)(((CT_DWORD)(w)) & 0xff))
#define HICB(w)           ((CT_BYTE)((((CT_DWORD)(w)) >> 8) & 0xff))



//////////////////////////////////////////////////////////////////////////////
//常量定义
#define MAX_WEAVE					4									//最大组合
#define MAX_INDEX					24									//最大索引
#define MAX_COUNT					14									//最大手牌数目
#define MAX_REPERTORY				64									//最大库存
#define MAX_HUA_CARD				8									//花牌个数
//扑克定义
#define HEAP_FULL_COUNT				36									//堆立全牌
#define MAX_RIGHT_COUNT				3									//最大权位DWORD个数
#define GAME_PLAYER                 2
#define LEN_SERVER                  64
#define LEN_ACCOUNTS                64
#define MAX_CARD_HAND               14
#define GAME_STATUS_FREE					100							//空闲状态
#define GAME_STATUS_PLAY					102							//游戏中状态
#define GAME_STATUS_END						103							//游戏结束

///////////////////////////////////////////////////////////////////////////////



#define PRINT_LOG_INFO 0

#define MIN_PLAYER_INDEX            0
#define MAX_PLAYER_INDEX            1
#define MAX_HAND_CARD_COUNT         14
#define DEALER_CARD_COUNT           14          //庄家手牌数
#define NO_DEALER_CARD_COUNT        13          //闲家手牌数
#define PLAYER_MAX_HAND_CARD_COUNT  20          //玩家最大手牌数
#define INVALID_USER_ID             0           //无效的用户ID
#define MIN_INDEX                   1           //最小牌值
#define MAX_IUNDEX                  55          //最大牌值
#define MAX_HU_CARD_COUNT           9          //最大胡牌数
#define GANG_CARD_MAX               3           //可以同时杠的牌最多三个
#define MAX_LISTEN_CARD_COUNT       14          //最大听牌数

//定时器时间间隔
#define TIME_OUT      8
#define TIMER_START_GAME            4           //开始游戏倒计时
#define TIMER_OPERATE_TIME_OUT      TIME_OUT          //玩家操作倒计时
#define TIMER_OPERATE_TIME_OUT_AFTER_ZHUACARD TIME_OUT
#define TIMER_OUT_CARD              TIME_OUT                                  //出牌倒计时
#define TIMER_AUTO_READY            10
#define TIMER_AUTO_OUT_CARD_AFTER_TING 1100
#define TIMER_AUTO_HU_CARD_AFTER_JIABEI TIME_OUT
#define TIMER_AUTO_JIESAN_DESK      15*60*1000
#define TIMER_ANDROID_OUT_CARD_MIN 2
#define TIMER_ANDROID_OUT_CARD_MAX 3

//定时器ID
#define IDI_OUT_CARD               1                                //出牌倒计时
#define IDI_START_GAME             2            //开始游戏计时器
#define IDI_OPERATE_TIME_OUT_AFTER_OUTCARD    3           //玩家操作倒计时:出牌后
#define IDI_OPERATE_TIME_OUT_AFTER_ZHUACARD   4  //玩家操作倒计时:抓牌后
#define IDI_AUTO_READY                        5  //服务器自动准备
#define IDI_AUTO_OUT_CARD_AFTER_TING          6  //听牌后自动出牌
#define IDI_AUTO_HU_CARD_AFTER_JIABEI            7           //在发送加倍消息后，启动定时器，如果玩家不超过默认胡牌
#define IDI_AUTO_JIESAN_DESK                8    //自动解散桌子，用于房间逻辑卡住后解散桌子
#define IDI_ANDROID_OUT_CARD				9	//机器人出牌

#define INVALID_USER_ID  0
#define DEALER_CARD_COUNT 14
#define NO_DEALER_CARD_COUNT 13
//#define MAX_INDEX					42									//最大索引
//#define MAX_HU_CARD_COUNT           14
#define INVALID_CARD_DATA           0
#define INVALID_CHAIR_ID            255
#define MAX_FNASHU                  64+2 //游戏最大胡牌类型,多一个自摸
#define GAME_PLAYER_MAX             2
#define MAX_CARD_HAND              14
#define MAX_ACTION                  4  //动作牌组数
#define MAX_OUT_NUM                 24

//必须保留，否则GameLogic是错的
//行为
#define WIK_NULL					0x00								//没有类型
#define WIK_LEFT					0x01								//左吃类型
#define WIK_CENTER					0x02								//中吃类型
#define WIK_RIGHT					0x04								//右吃类型
#define WIK_PENG					0x08								//碰牌类型
#define WIK_GANG					0x10								//杠牌类型
#define WIK_LISTEN					0x20								//听牌类型
#define WIK_CHI_HU					0x40								//吃胡类型
#define WIK_REPLACE					0x80								//花牌替换

//自己的行为

#define  WIK_NULL_MY                   0                 //过
#define  WIK_CHI_MY                    1                 //吃
#define  WIK_CENTER_MY                 2          //中吃
#define  WIK_LEFT_MY                   4            //左吃
#define  WIK_RIGHT_MY                  8           //右吃
#define  WIK_PENG_MY                   16               //碰
#define  WIK_GANG_MY                   32               //杠
#define  WIK_GANG_MING_MY              64          //明杠
#define  WIK_GANG_AN_MY                128           //暗杠
#define  WIK_GANG_MING_PENG_MY         256    //碰牌后再摸的杠
#define  WIK_TING_MY                   512              //听
#define  WIK_HU_MY                     1024               //胡
#define  WIK_HU_ZI_MO_MY               2048         //胡自摸
#define  WIK_HU_FANG_PAO_MY            4096      //胡放炮
#define  WIK_JIABEI                    8192      //加倍

#define  WIK_NULL_MY_TIMEOUT           16384//888      //超时过牌


//出牌状态
#define STATE_OUT_NO         0      //不可以胡
#define STATE_OUT_NORMAL     1      //可以胡的牌
#define STATE_OUT_NUN_MAX    2      //可以胡的牌数量最多
#define STATE_OUT_FAN_MAX    3      //可以胡的牌番数最多

//服务器命令结构
enum ServerMsg
{
	SUB_S_GAME_START  = 100,                    //游戏开始
	SUB_S_OUT_TIP  = 101,                       //出牌提示
	SUB_S_OUT_CARD  = 102,                      //用户出牌
	SUB_S_OPERATE_TIP_OUT = 103,                //操作提示(出牌后)吃碰杠胡
	SUB_S_OPERATE_OUT  = 104,                   //操作执行(出牌后)
	SUB_S_SEND_CARD  = 105,                     //用户摸牌
	SUB_S_OPERATE_TIP_SEND = 106,               //操作提示(摸牌后)吃碰杠胡
	SUB_S_OPERATE_SEND  = 107,                  //操作执行(摸牌后)
	SUB_S_OPERATE_TIP_CANCEL  = 108,            //取消操作提示吃碰杠胡
	SUB_S_GAME_END = 109,                       //游戏结束
	SUB_S_MABY_HU_CARD = 110,                   //可能胡牌
	SUB_S_GAME_RELINK = 120,                    //游戏断线重连
	SUB_S_OPERATE_TING = 111,                   //听牌后操作是否胡牌
	SUB_S_SHOWCARD_TING = 112,                  //听牌后摊牌
	SUB_S_DOUBLUE_TING = 113,                   //听牌后加倍
	SUB_S_OUT_TIP_TING = 114,                   //听牌后第一次出牌提示
	SUB_S_OUT_TYPE_TIP = 115,                   //提示可以出的牌
	SUB_S_CANCEL_TING = 116,                    //取消听牌
	SUB_S_MABY_HU_CARD_TING = 117,             //可能胡牌(听牌后)
	SUB_S_GAMESCENE_FREE = 118                  //断线重连上来是空闲场景就发送空闲场景消息
};

//客户端命令结构
enum ClientMsg
{
	SUB_C_OUT_CARD = 201,                   //用户出牌
	SUB_C_READY_OUT_CARD = 202,             //准备出牌
	SUB_C_OPERATE_OUT =203,                 //出牌后操作执行
	SUB_C_OPERATE_SEND = 204,               //摸牌后操作执行
	SUB_C_OPERATE_TING = 205,               //听牌后操作执行
	SUB_C_OPERATE_TING_CANCEL = 206         //听牌后是否取消
};

//什么行为引发了抓牌行为
enum Action_Zhua
{
	Action_OutCard = 1,
	Action_PassCard = 2,
	Action_GangCard = 3,
	Action_TingCard = 4,
	Action_JiaBei = 5,
};

//服务器提示玩家行为是由什么触发的
enum TipTrigger
{
	//提示行为是由玩家出牌引发的
			Trigger_OutCard = 1,
	//提示行为是由玩家抓牌引发的
			Trigger_ZhuaCard = 2,
};

enum TingTrigger
{
	//碰吃引发听牌
			PengChiCard = 1,
	//抓牌引发听牌
			ZhuaCard = 2,
	//取消听牌引发听牌
			CancelTing = 3
};

enum CardColor
{
	Color_WAN = 0,
	Color_FENG = 3
};

//杠牌类型
enum enGangCardType
{
	INVALID_GANG_TYPE = 0,
	An_Gang = 1,
	Ming_Gang = 2,
	Ba_Gang = 3
};

//操作类型
enum enOperateType
{
	OutCard = 0,
	After_ZhuaCard = 1,
	After_OutCard = 2,
	After_TingCard = 3
};

//游戏开始
struct CMD_S_GameStart
{
	CT_DWORD              wBankerUser;                       //庄家用户
	CT_DWORD              wCurrentUser;                      //当前用户
	CT_DWORD              wRemainNum;                        //剩余牌数
	CT_DOUBLE             lCellScore;                        //倍率
	CT_DWORD              wUpDateTime;                       //倒计时时间显示
	CT_BYTE               cbHandCardDataIndex[DEALER_CARD_COUNT];      //麻将列表索引，实际上是牌值
};

struct CMD_HeapCardInfo
{
	CT_WORD	  wStartIndex;
	CT_WORD	  wRemoveCount;
};

//听牌后取消操作
struct CMD_C_OperateTing_Cancel:public MSG_GameMsgUpHead
{
	CT_DWORD   wOperateCode;  //操作代码
};

//取消听牌116
struct CMD_S_Cancel_Ting
{
	CT_DWORD wOperateCode;
};

//组合子项
struct tagWeaveItem
{
	CT_DWORD						cbWeaveKind;						//组合类型
	CT_BYTE							cbCenterCard;						//中心扑克
	CT_BYTE							cbPublicCard;						//公开标志
	CT_WORD							wProvideUser;						//供应用户
	CT_BYTE							cbCardData[4];						//

	tagWeaveItem()
	{
		clearWeaveItem();
	}

	//如果tagWeaveItem用于玩家身上那么就用这个清除函数
	CT_VOID clearWeaveItem()
	{
		cbWeaveKind = WIK_NULL_MY;
		cbCenterCard = INVALID_CARD_DATA;
		cbPublicCard = CT_FALSE;
		wProvideUser = INVALID_CHAIR_ID;
		memset(cbCardData, INVALID_CARD_DATA, sizeof(cbCardData));
	}

	//如果tagWeaveItem用于GameLogic判断吃碰杠胡就用这个清除函数
	CT_VOID clearWeaveItem1()
	{
		cbWeaveKind = WIK_NULL;
		cbCenterCard = INVALID_CARD_DATA;
		cbPublicCard = CT_FALSE;
		wProvideUser = INVALID_CHAIR_ID;
		memset(cbCardData, INVALID_CARD_DATA, sizeof(cbCardData));
	}

};

//空闲状态
struct CMD_S_StatusFree
{
	//游戏属性
	CT_DOUBLE					dCellScore;							//基础积分
	CT_WORD						dwCurrPlayCount;
};

//可以出牌提示115
struct CMD_S_OutType_Tip
{
	CT_BYTE bOutCardData[MAX_CARD_HAND];
	CT_BYTE bCanHuType[MAX_CARD_HAND];

	CMD_S_OutType_Tip()
	{
		memset(bOutCardData, INVALID_CARD_DATA, sizeof(bOutCardData));
		memset(bCanHuType, STATE_OUT_NO, sizeof(bCanHuType));
	}
};

struct tagHuCardData
{
	CT_BYTE cbListenCard;
	CT_BYTE cbRemainCount;
	CT_WORD wMultipleCount;

	tagHuCardData()
	{
		clearHuCardData();
	}

	CT_VOID clearHuCardData()
	{
		cbListenCard = INVALID_CARD_DATA;
		cbRemainCount = 0;
		wMultipleCount = 0;
	}
};

struct tagListenCardData
{
	CT_BYTE  cbOutCard;
	CT_BYTE  cbHuCardCount;
	CT_BYTE  cbHuCardData[MAX_INDEX * sizeof(tagHuCardData)];

	tagListenCardData()
	{
		cbOutCard = INVALID_CARD_DATA;
		cbHuCardCount = 0;
		memset(cbHuCardData, INVALID_CARD_DATA, sizeof(CT_BYTE) * MAX_INDEX * sizeof(tagHuCardData));
	}
};

//牌的数量
struct tagCardShow
{
	CT_BYTE               cbCardId;           //操作牌的ID
	CT_BYTE               cbCardCount;        //操作牌的数量
	CT_BYTE               cbFanCount;         //胡该牌的翻数
};

//可以胡的牌117
struct CMD_S_HaveHuCard_Ting
{
	CT_DWORD              wCurrentUser;                //当前玩家
	CT_DWORD               cbBei;
	tagCardShow           cbHuCard[MAX_HU_CARD_COUNT];

	CMD_S_HaveHuCard_Ting()
	{
		wCurrentUser = INVALID_CHAIR_ID;
		cbBei = 0;
		memset(cbHuCard, INVALID_CARD_DATA, sizeof(cbHuCard));
	}
};

//可以胡的牌110
struct CMD_S_HaveHuCard
{
	CT_BYTE               cbHuCount;        //胡牌的数量
	tagCardShow           cbHuCard[MAX_HU_CARD_COUNT];

	CMD_S_HaveHuCard()
	{
		cbHuCount = 0;
		memset(cbHuCard, INVALID_CARD_DATA, sizeof(cbHuCard));
	}
};

struct CMD_C_Ready_OutCard:public MSG_GameMsgUpHead
{
	CT_BYTE              cbCardId;  //出牌ID
};

struct CMD_S_OperateTip_Cancel
{
	CT_DWORD              wOperateCode;     //操作代码
};

//105
struct CMD_S_SendCard
{
	CT_DWORD              wSendCardUser;        //摸牌玩家
	CT_BYTE               cbCardId;             //麻将ID(只有摸牌玩家的椅子号为自己时才有值,否则值为0)
	CT_DWORD              wRemainNum;           //剩余牌数
};

//动作牌结构
struct tagActionGroup
{
	CT_BYTE              cbCardId;               //动作的牌ID
	CT_DWORD              wOperateCode;         //动作类型
	CT_DWORD              wProvideUser;         //提供玩家
};

//游戏状态
#define STATE_DEFAUT                  0                   //默认状态
#define STATE_WAIT_OUT                1                   //等待出牌阶段
#define STATE_WAIT_OUT_OPERATE        2                   //等待出牌后操作阶段
#define STATE_WAIT_SEND_OPERATE       3                   //等待摸牌后操作阶段
#define STATE_JIABEI                  4                   //加倍操作状态
//#define STATE_TING                    5                   //听牌后的状态

#define CLIENT_MAX_INDEX              14 //麻将最大索引
//游戏状态120
struct CMD_S_StatusPlay
{
	CT_DWORD             wBankerUser;                  //庄家用户
	CT_DWORD             wCurrentUser;                 //当前玩家
	CT_DWORD             wCurOutUser;                  //当前牌局中的最近一次的出牌玩家
	// CT_DWORD             wCode;                      //操作代码
	CT_DWORD             wRemainNum;                   //剩余牌数
	CT_BYTE              bCurOutCard;                  //当前牌局中的最近一次出的牌
	CT_BYTE              bCurSendCard;                 //当前牌局中的最近一次摸的牌
/*   CT_BYTE              bState;                     //当前游戏状态
   CT_BOOL              bOperateFlag;                 //仅用于出牌后操作回执*/
	CT_DOUBLE            cbCellScore;                    //倍率
	CT_BYTE              cbTotalTimes;
	CT_BYTE              cbShengYuTimes;
	CT_BOOL              bIsTing[GAME_PLAYER_MAX];
	CT_DWORD             cbJiaBei[GAME_PLAYER_MAX];
	CT_BYTE              cbOtherPlayerHandCard[MAX_HAND_CARD_COUNT];
	CT_BYTE              cbHandCardDataIndex[MAX_HAND_CARD_COUNT];            //手上扑克索引
	CT_BYTE              cbCurOutCardData[GAME_PLAYER_MAX][MAX_OUT_NUM];//出牌列表
	CT_BYTE              cbCurOutCardNum[GAME_PLAYER_MAX];          //当前牌局每个玩家出的牌数量
	tagActionGroup       cbCurActionCardData[GAME_PLAYER_MAX][MAX_ACTION];     //每个玩家的动作牌最多四组
	CT_BYTE              cbCurActionGroupNum[GAME_PLAYER_MAX];        //每个玩家动作牌组数量
	CT_BYTE              cbCurHandCardDataNum[GAME_PLAYER_MAX];       //当前牌局每个玩家手牌数量
	CMD_S_StatusPlay()
	{
		memset(this, 0 , sizeof(CMD_S_StatusPlay));
	}
};

//出牌提示101
struct CMD_S_OutTip
{
	CT_DWORD    wOutCardUser;               //可以出牌玩家
	CT_DWORD    wUpDateTime;                //倒计时时间
	CT_BOOL     bZhuangFirstOutCard;        //庄家是否第一次出牌

	CMD_S_OutTip()
	{
		wOutCardUser = INVALID_CHAIR_ID;
		wUpDateTime = TIME_OUT;
		bZhuangFirstOutCard = CT_FALSE;
	}
};

//出牌数据包
struct CMD_C_OutCard:public MSG_GameMsgUpHead
{
	CT_BYTE              cbCardId;      //出牌ID
	CT_BYTE              cbCardIndex;  //出牌索引,客户端使用
};


//出牌后操作执行104
struct CMD_S_OperateOut
{
	CT_DWORD              wOperateCode;           //操作代码
	CT_DWORD              wOperateUser;           //操作用户
	CT_DWORD              wProvideUser;           //提供用户
	CT_BYTE               cbCardId;               //操作牌的ID
	CT_BYTE               isQiangGang;            //是否是抢杠胡
};

struct tagGangCard
{
	CT_BYTE               cbCardId;           //操作牌的ID
	CT_DWORD              wGangOperateCode;           //操作代码
};

//摸牌后操作提示106
struct CMD_S_OperateTip_Send
{
	CT_DWORD              wOperateCode;           //操作代码
	CT_BYTE               bGangCount;             //可以杠的数量
	tagGangCard            cbCardTB[GANG_CARD_MAX];//操作牌的ID
};

//摸牌后操作执行107
struct CMD_S_OperateSend
{
	CT_DWORD              wOperateCode;           //操作代码
	CT_DWORD              wOperateUser;           //操作用户
	CT_BYTE               cbCardId;           //操作牌的ID
};

//出牌后操作执行 203
struct CMD_C_OperateOut:public MSG_GameMsgUpHead
{
	CT_DWORD              wOperateCode;  //操作代码
};

//用户出牌102
struct CMD_S_OutCard
{
	CT_DWORD              wOutCardUser;         //出牌玩家
	CT_BYTE               cbCardId;             //麻将ID
	CT_BYTE               cbCardIndex;           //出牌索引
	CT_BOOL               bIsAutoOutCard;        //是否自动出牌
};

//出牌后操作提示(吃碰杠胡)103
struct CMD_S_OperateTip
{
	CT_DWORD              wOperateCode;     //操作代码
	CT_BYTE              cbCardId;
};


//听牌后操作是否胡牌--111
struct CMD_S_OperateTip_Ting
{
	CT_DWORD  wOperateCode;
};


//听牌后摊牌--112
struct CMD_S_ShowCard_Ting
{
	CT_DWORD  wOperateUser;
	CT_BYTE   bCurHandCardData[MAX_CARD_HAND];
};


//听牌后加倍--113
// 113
struct CMD_S_Double_Ting
{
	CT_DWORD  wOperateUser;
	CT_DWORD  wOperateCode;
	CT_DWORD  cbBei;

	CMD_S_Double_Ting()
	{
		wOperateUser = 0;
		wOperateCode = 0;
		cbBei = 0;
	}
};


//出牌提示114
struct CMD_S_OutTip_Ting
{
	CT_BYTE bOutCardData[MAX_CARD_HAND];

	CMD_S_OutTip_Ting()
	{
		memset(bOutCardData, 0, sizeof(bOutCardData));
	}
};


//SUB_C_OPERATE_TING = 205    //听牌后操作执行
//听牌后操作执行
struct CMD_C_OperateTing:public MSG_GameMsgUpHead
{
	CT_DWORD              wOperateCode;  //操作代码
};

//摸牌后操作执行 204
struct CMD_C_OperateSend:public MSG_GameMsgUpHead
{
	CT_DWORD              wOperateCode;  //操作代码
	CT_BYTE               cbCardId; //杠牌ID
};

enum eWeaveSource
{
	InvalidZone = 0,
	HandCardZone = 1,
	PengChiGangZone = 2
};

//分析子项
struct tagAnalyseItem
{
	CT_BYTE							cbCardEye;							//牌眼扑克
	CT_BOOL                         bMagicEye;                          //牌眼是否是王霸
	CT_BYTE							cbWeaveKind[MAX_WEAVE];				//组合类型
	CT_BYTE							cbCenterCard[MAX_WEAVE];			//中心扑克
	CT_BYTE                         cbCardData[MAX_WEAVE][4];           //实际扑克
	CT_BYTE							cbIsQiDui;							//特殊牌:七对
	CT_BYTE							cbPublicCard[MAX_WEAVE];			//特殊牌:七对
	eWeaveSource                    cbWeaveSource[MAX_WEAVE];          //用于描述组合牌的来源 1:手牌 2:碰杠吃牌区 0:不知道来源

	tagAnalyseItem()
	{
		memset(cbWeaveSource, InvalidZone, sizeof(cbWeaveSource));
	}
};


//动作牌结构
/* struct tagActionGroup
{
    CT_BYTE              bCardId;               //动作的牌ID
    CT_DWORD              wOperateCode;         //动作类型
    CT_DWORD              wProvideUser;         //提供玩家
 };*/

//游戏小结算109
struct CMD_S_GameEnd
{
	CT_BOOL               bIsNoWin;              //是否留局
	CT_DWORD              wBanker;             //庄家（值为INVALID_CHAIR表示没有放炮玩家）
	CT_DWORD              wWinner;              //胡牌玩家
	CT_BYTE               dCardType[MAX_FNASHU]; //胡牌牌型类型
	CT_BYTE               bJiangId;             //将牌ID
	CT_BYTE               bHuCardId;            //胡牌ID
	CT_BYTE               bCurHandCardData[GAME_PLAYER_MAX][MAX_CARD_HAND];  //当前牌局每个玩家剩余手牌
	tagActionGroup        cbCurActionCardData[GAME_PLAYER_MAX][MAX_ACTION];     //每个玩家的动作牌最多四组
	CT_DOUBLE             lScore[GAME_PLAYER_MAX];             //积分信息
	CT_BYTE               cbWinLoseMax;
	CMD_S_GameEnd()
	{
		cbWinLoseMax = 0;
		memset(this, 0, sizeof(CMD_S_GameEnd));
	}
};


//数组模板类
template <class TYPE, class ARG_TYPE = const TYPE &> class CWHArray
{
	//变量定义
protected:
	TYPE *								m_pData;							//数组指针
	CT_INT64							m_nMaxCount;						//缓冲数目
	CT_INT64							m_nGrowCount;						//增长数目
	CT_INT64							m_nElementCount;					//元素数目

	//函数定义
public:
	//构造函数
	CWHArray();
	//析构函数
	virtual ~CWHArray();

	//信息函数
public:
	//是否空组
	CT_BOOL IsEmpty() const;
	//获取数目
	CT_INT64 GetCount() const;

	//功能函数
public:
	//获取缓冲
	TYPE * GetData();
	//获取缓冲
	const TYPE * GetData() const;
	//增加元素
	CT_INT64 Add(ARG_TYPE newElement);
	//拷贝数组
	CT_VOID Copy(const CWHArray & Src);
	//追加数组
	CT_INT64 Append(const CWHArray & Src);
	//获取元素
	TYPE & GetAt(CT_INT64 nIndex);
	//获取元素
	const TYPE & GetAt(CT_INT64 nIndex) const;
	//获取元素
	TYPE & ElementAt(CT_INT64 nIndex);
	//获取元素
	const TYPE & ElementAt(CT_INT64 nIndex) const;

	//操作函数
public:
	//设置大小
	CT_VOID SetSize(CT_INT64 nNewSize);
	//设置元素
	CT_VOID SetAt(CT_INT64 nIndex, ARG_TYPE newElement);
	//设置元素
	CT_VOID SetAtGrow(CT_INT64 nIndex, ARG_TYPE newElement);
	//插入数据
	CT_VOID InsertAt(CT_INT64 nIndex, const CWHArray & Src);
	//插入数据
	CT_VOID InsertAt(CT_INT64 nIndex, ARG_TYPE newElement, CT_INT64 nCount = 1);
	//删除数据
	CT_VOID RemoveAt(CT_INT64 nIndex, CT_INT64 nCount = 1);
	//删除元素
	CT_VOID RemoveAll();

	//操作重载
public:
	//操作重载
	TYPE & operator[](CT_INT64 nIndex);
	//操作重载
	const TYPE & operator[](CT_INT64 nIndex) const;

	//内存函数
public:
	//释放内存
	CT_VOID FreeMemory();
	//申请内存
	CT_VOID AllocMemory(CT_INT64 nNewCount);
};



//是否空组
template<class TYPE, class ARG_TYPE>
inline CT_BOOL CWHArray<TYPE, ARG_TYPE>::IsEmpty() const
{
	return (m_nElementCount == 0);
}

//获取数目
template<class TYPE, class ARG_TYPE>
inline CT_INT64 CWHArray<TYPE, ARG_TYPE>::GetCount() const
{
	return m_nElementCount;
}

//增加元素
template<class TYPE, class ARG_TYPE>
inline CT_INT64 CWHArray<TYPE, ARG_TYPE>::Add(ARG_TYPE newElement)
{
	CT_INT64 nIndex = m_nElementCount;
	SetAtGrow(nIndex, newElement);
	return nIndex;
}

//操作重载
template<class TYPE, class ARG_TYPE>
inline TYPE & CWHArray<TYPE, ARG_TYPE>::operator[](CT_INT64 nIndex)
{
	return ElementAt(nIndex);
}

//操作重载
template<class TYPE, class ARG_TYPE>
inline const TYPE & CWHArray<TYPE, ARG_TYPE>::operator[](CT_INT64 nIndex) const
{
	return GetAt(nIndex);
}

//////////////////////////////////////////////////////////////////////////////////
//构造函数
template<class TYPE, class ARG_TYPE>
CWHArray<TYPE, ARG_TYPE>::CWHArray()
{
	m_pData = NULL;
	m_nMaxCount = 0;
	m_nGrowCount = 0;
	m_nElementCount = 0;

	return;
}

//构造函数
template<class TYPE, class ARG_TYPE>
CWHArray<TYPE, ARG_TYPE>::~CWHArray()
{
	if (m_pData != NULL)
	{
		for (CT_INT64 i = 0; i < m_nElementCount; i++)	(m_pData + i)->~TYPE();
		delete[](CT_BYTE *)m_pData;
		m_pData = NULL;
	}

	return;
}

//获取缓冲
template<class TYPE, class ARG_TYPE>
TYPE * CWHArray<TYPE, ARG_TYPE>::GetData()
{
	return m_pData;
}

//获取缓冲
template<class TYPE, class ARG_TYPE>
const TYPE * CWHArray<TYPE, ARG_TYPE>::GetData() const
{
	return m_pData;
}

//拷贝数组
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::Copy(const CWHArray & Src)
{
	//效验参数
	assert(this != &Src);
	if (this == &Src) return;

	//拷贝数组
	AllocMemory(Src.m_nElementCount);
	if (m_nElementCount > 0)
	{
		for (CT_INT64 i = 0; i < m_nElementCount; i++) (m_pData + i)->~TYPE();
		memset(m_pData, 0, m_nElementCount*sizeof(TYPE));
	}
	for (CT_INT64 i = 0; i < Src.m_nElementCount; i++)	m_pData[i] = Src.m_pData[i];
	m_nElementCount = Src.m_nElementCount;

	return;
}

//追加数组
template<class TYPE, class ARG_TYPE>
CT_INT64 CWHArray<TYPE, ARG_TYPE>::Append(const CWHArray & Src)
{
	//效验参数
	assert(this != &Src);
	if (this == &Src)return 0;


	//拷贝数组
	if (Src.m_nElementCount > 0)
	{
		//CT_INT64 nOldCount = m_nElementCount;
		AllocMemory(m_nElementCount + Src.m_nElementCount);
		for (CT_INT64 i = 0; i < Src.m_nElementCount; i++)	m_pData[m_nElementCount + i] = Src.m_pData[i];
		m_nElementCount += Src.m_nElementCount;
	}

	return m_nElementCount;
}

//获取元素
template<class TYPE, class ARG_TYPE>
TYPE & CWHArray<TYPE, ARG_TYPE>::GetAt(CT_INT64 nIndex)
{
	assert((nIndex >= 0) && (nIndex < m_nElementCount));
	if ((nIndex < 0) || (nIndex >= m_nElementCount)){

	}


	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE>
const TYPE & CWHArray<TYPE, ARG_TYPE>::GetAt(CT_INT64 nIndex) const
{
	assert((nIndex >= 0) && (nIndex < m_nElementCount));
	if ((nIndex < 0) || (nIndex >= m_nElementCount))
	{
		//DbgBreakPoint
		//LOG(ERROR) << "nIndex :"<<nIndex << " m_nElementCount: "<< m_nElementCount;
	}


	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE>
TYPE & CWHArray<TYPE, ARG_TYPE>::ElementAt(CT_INT64 nIndex)
{
	assert((nIndex >= 0) && (nIndex < m_nElementCount));
	if ((nIndex < 0) && (nIndex >= m_nElementCount))
	{
		//DbgBreakPoint
	}


	return m_pData[nIndex];
}

//获取元素
template<class TYPE, class ARG_TYPE>
const TYPE & CWHArray<TYPE, ARG_TYPE>::ElementAt(CT_INT64 nIndex) const
{
	assert((nIndex >= 0) && (nIndex < m_nElementCount));
	if ((nIndex < 0) && (nIndex >= m_nElementCount))return NULL;


	return m_pData[nIndex];
}

//设置大小
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::SetSize(CT_INT64 nNewSize)
{
	//效验参数
	assert(nNewSize >= 0);
	if (nNewSize < 0)return;


	//设置大小
	AllocMemory(nNewSize);
	if (nNewSize > m_nElementCount)
	{
		for (CT_INT64 i = m_nElementCount; i < nNewSize; i++) new ((CT_VOID *)(m_pData + i)) TYPE;
	}
	else if (nNewSize < m_nElementCount)
	{
		for (CT_INT64 i = nNewSize; i < m_nElementCount; i++) (m_pData + i)->~TYPE();
		memset(m_pData + nNewSize, 0, (m_nElementCount - nNewSize)*sizeof(TYPE));
	}
	m_nElementCount = nNewSize;

	return;
}

//设置元素
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::SetAt(CT_INT64 nIndex, ARG_TYPE newElement)
{
	assert((nIndex >= 0) && (nIndex < m_nElementCount));
	if ((nIndex >= 0) && (nIndex < m_nElementCount)) m_pData[nIndex] = newElement;
	else return ;


	return;
}

//设置元素
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::SetAtGrow(CT_INT64 nIndex, ARG_TYPE newElement)
{
	//效验参数
	assert(nIndex >= 0);
	if (nIndex < 0)return ;

	//设置元素
	if (nIndex >= m_nElementCount) SetSize(m_nElementCount + 1);
	m_pData[nIndex] = newElement;

	return;
}

//插入数据
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::InsertAt(CT_INT64 nIndex, const CWHArray & Src)
{
	//效验参数
	//assert(nStartIndex >= 0);
	//if (nStartIndex < 0)return ;
/*
    assert(nIndex >= 0);
    if (nIndex < 0)return ;


    if (Src.m_nElementCount>0)
    {
        //申请数组
        if (nIndex < m_nElementCount)
        {
            CT_INT64 nOldCount = m_nElementCount;
            SetSize(m_nElementCount + Src.m_nElementCount);
            for (CT_INT64 i = 0; i < nCount; i++) (m_pData + nOldCount + i)->~TYPE();
            memmove(m_pData + nIndex + nCount, m_pData + nIndex, (nOldCount - nIndex)*sizeof(TYPE));
            memset(m_pData + nIndex, 0, Src.m_nElementCount*sizeof(TYPE));
            for (CT_INT64 i = 0; i < Src.m_nElementCount; i++) new (m_pData + nIndex + i) TYPE();
        }
        else SetSize(nIndex + nCount);

        //拷贝数组
        assert((nIndex + Src.m_nElementCount) <= m_nElementCount);
        while (nCount--) m_pData[nIndex++] = newElement;
    }
*/

	return;
}

//插入数据
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::InsertAt(CT_INT64 nIndex, ARG_TYPE newElement, CT_INT64 nCount)
{
	//效验参数
	assert(nIndex >= 0);
	assert(nCount > 0);
	if ((nIndex < 0) || (nCount <= 0))return ;


	//申请数组
	if (nIndex < m_nElementCount)
	{
		CT_INT64 nOldCount = m_nElementCount;
		SetSize(m_nElementCount + nCount);
		for (CT_INT64 i = 0; i < nCount; i++) (m_pData + nOldCount + i)->~TYPE();
		memmove(m_pData + nIndex + nCount, m_pData + nIndex, (nOldCount - nIndex)*sizeof(TYPE));
		memset(m_pData + nIndex, 0, nCount*sizeof(TYPE));
		for (CT_INT64 i = 0; i < nCount; i++)::new (m_pData + nIndex + i) TYPE();
	}
	else SetSize(nIndex + nCount);

	//拷贝数组
	assert((nIndex + nCount) <= m_nElementCount);
	while (nCount--) m_pData[nIndex++] = newElement;

	return;
}

//删除数据
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::RemoveAt(CT_INT64 nIndex, CT_INT64 nCount)
{
	//效验参数
	assert(nIndex >= 0);
	assert(nCount >= 0);
	assert(nIndex + nCount <= m_nElementCount);
	if ((nIndex < 0) || (nCount<0) || ((nIndex + nCount>m_nElementCount)))return ;


	//删除数据
	CT_INT64 nMoveCount = m_nElementCount - (nIndex + nCount);
	for (CT_INT64 i = 0; i < nCount; i++) (m_pData + nIndex + i)->~TYPE();
	if (nMoveCount > 0) memmove(m_pData + nIndex, m_pData + nIndex + nCount, nMoveCount*sizeof(TYPE));
	m_nElementCount -= nCount;

	return;
}

//删除元素
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::RemoveAll()
{
	if (m_nElementCount > 0)
	{
		for (CT_INT64 i = 0; i < m_nElementCount; i++) (m_pData + i)->~TYPE();
		memset(m_pData, 0, m_nElementCount*sizeof(TYPE));
		m_nElementCount = 0;
	}

	return;
}

//释放内存
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::FreeMemory()
{
	if (m_nElementCount != m_nMaxCount)
	{
		TYPE * pNewData = NULL;
		if (m_nElementCount != 0)
		{
			pNewData = (TYPE *) new CT_BYTE[m_nElementCount*sizeof(TYPE)];
			memcpy(pNewData, m_pData, m_nElementCount*sizeof(TYPE));
		}
		delete[](CT_BYTE *)m_pData;
		m_pData = pNewData;
		m_nMaxCount = m_nElementCount;
	}

	return;
}

//申请内存
template<class TYPE, class ARG_TYPE>
CT_VOID CWHArray<TYPE, ARG_TYPE>::AllocMemory(CT_INT64 nNewCount)
{
	//效验参数
	assert(nNewCount >= 0);

	if (nNewCount > m_nMaxCount)
	{
		//计算数目
		CT_INT64 nGrowCount = m_nGrowCount;
		if (nGrowCount == 0)
		{
			nGrowCount = m_nElementCount / 8;
			nGrowCount = (nGrowCount < 4) ? 4 : ((nGrowCount>1024) ? 1024 : nGrowCount);
		}
		nNewCount += nGrowCount;

		//申请内存
		TYPE * pNewData = (TYPE *) new CT_BYTE[nNewCount*sizeof(TYPE)];
		memcpy(pNewData, m_pData, m_nElementCount*sizeof(TYPE));
		memset(pNewData + m_nElementCount, 0, (nNewCount - m_nElementCount)*sizeof(TYPE));
		delete[](CT_BYTE *)m_pData;

		//设置变量
		m_pData = pNewData;
		m_nMaxCount = nNewCount;
	}

	return;
}

//数组说明
typedef CWHArray<tagAnalyseItem,tagAnalyseItem &> CAnalyseItemArray;


//杠牌结果
struct tagGangCardResult
{
	CT_BYTE							cbCardCount;						//扑克数目
	CT_BYTE							cbCardData[MAX_WEAVE];				//扑克数据
	CT_DWORD                        dwGangCardType[MAX_WEAVE];          //杠牌类型

	tagGangCardResult()
	{
		clear();
	}

	CT_VOID clear()
	{
		cbCardCount = 0;
		memset(cbCardData, INVALID_CARD_DATA, sizeof(cbCardData));
		memset(dwGangCardType, INVALID_GANG_TYPE, sizeof(dwGangCardType));
	}
};

enum eMaxInfo
{
	//
			Max_Normal = 0,
	//最大番数
			Max_Fan = 1,
	//最大胡牌数
			Max_HuCardCount = 2
};

//玩家最大番数 和 最多胡牌数信息
struct tagPlayerMaxInfo
{
	//打出的牌
	CT_BYTE cbCardData;
	//默认为nomal
	eMaxInfo maxInfo;
	//打一张牌能胡的牌剩余总张数
	CT_WORD  wTotalShengYuCount;
	//打一张牌出去能胡的最大番数
	CT_WORD  wHuMaxFan;
	//打一张牌出去能胡的所有牌中最大番数对应的剩余张数
	CT_WORD  wShengYuCount_wHuMaxFan;

	tagPlayerMaxInfo()
	{
		cbCardData = INVALID_CARD_DATA;
		maxInfo = Max_Normal;
		wTotalShengYuCount = 0;
		wHuMaxFan = 0;
		wShengYuCount_wHuMaxFan = 0;
	}
};

//玩家位置信息
struct tagChairInfo
{
	CT_DWORD  dwUserID;
	//每个玩家的手牌值转换成索引后的值对应的个数  比如:cbHandCardIndex[10] = 4;表示某一牌值对应的索引为10，牌张数为4张
	CT_BYTE   cbHandCardIndex[MAX_INDEX];
	//玩家初始手牌
	std::vector<CT_BYTE> cbInitHandCardData;
	//玩家能拥有的行为
	CT_DWORD cbPlayerAction;
	//如果玩家有行为,判断玩家行为是否完成
	CT_BOOL  bFinishAction;
	//如果玩家有行为,玩家行为的牌
	CT_BYTE  cbPlayerActionCardData;
	//如果玩家有行为，行为的引发者是谁
	CT_DWORD  dwActionProvider;
	//玩家的吃碰杠组合牌
	tagWeaveItem weaveItem[MAX_WEAVE];
	//胡牌组合
	CAnalyseItemArray m_AnalyseItemArray;
	//抓牌的时候，玩家能杠的牌
	tagGangCardResult  canGangCard;
	//是否托管
	CT_BOOL   bIsTuoGuan;
	//是否听牌
	CT_BOOL   bIsTing;
	//丢弃数目
	CT_BYTE	  cbDiscardCount;
	//丢弃记录
	CT_BYTE	  cbDiscardCard[MAX_OUT_NUM];
	//出牌数目
	CT_BYTE   cbOutCardCount;
	//胡的牌
	CT_BYTE  cbHuCard;
	//玩家有胡牌行为时,能胡的牌型
	CT_LONGLONG llHuCardType;
	//玩家有胡牌行为时,能胡的最大番数
	CT_WORD  wHuCardFan;
	//是否胡牌了
	CT_BOOL bHu;
	//是否只自摸胡
	CT_BOOL bZiMo;
	//玩家能听的牌数据
	CT_BYTE cbListenCardData[MAX_COUNT][2+MAX_INDEX*4];
	//玩家能听的牌数量
	CT_BYTE cbListenCardCount;
	//听牌之后是否打了第一张牌
	CT_BOOL bIsOutFirstCard_AfterTing;
	//胡牌判断是否要加自摸的番
	CT_BOOL bIsSuanZiMo;
	//玩家加倍
	CT_WORD wJiaBei;
	//加倍是什么引发的 自己摸牌引发加倍: 1     别人出牌引发加倍：2
	CT_BYTE cbJiaBeiSource;
	//是否是天听
	CT_BOOL bTianTing;
	//玩家初始分数:进入房间时的分数
	CT_LONGLONG llInitScore;
	//玩家当前分数
	CT_LONGLONG llCurSocre;
	//玩家抽税总额
	CT_LONGLONG llTotalTax;
	//是否能抢杠胡
	CT_BOOL bCanQiangGangHu;
	//用于断线重连通知玩家能胡哪些牌,番数 和 剩余张数，这个数据是变化的.
	tagCardShow  cbHuCard_DisReconect[MAX_HU_CARD_COUNT];
	//听牌后能胡哪些牌,玩家听后摊牌就固定了
	CT_BYTE cbCanHuCard_AfterTing[MAX_HU_CARD_COUNT];
	//玩家点击听按钮后只能出的牌
	CT_BYTE cbMustOutCard_AfterClickTing[MAX_CARD_HAND];

	tagChairInfo()
	{
		//
		dwUserID = INVALID_USER_ID;
		clearAllHandCard();
		clearPlayerAction();
		clearPlayerOtherInfo();
	}

	CT_VOID clearPerGameRound()
	{
		//用于每一局开始清除玩家数据
		clearAllHandCard();
		clearPlayerAction();
		clearPlayerOtherInfo();

	}

	CT_VOID clearAllHandCard()
	{
		//清除玩家手牌相关
		memset(cbHandCardIndex, 0, sizeof(cbHandCardIndex));
		cbInitHandCardData.clear();
	}

	CT_VOID clearPlayerAction()
	{
		//清除玩家行为相关
		cbPlayerAction = WIK_NULL_MY;
		bFinishAction = CT_FALSE;
		cbPlayerActionCardData = INVALID_CARD_DATA;
		dwActionProvider = INVALID_CHAIR_ID;
		bCanQiangGangHu = CT_FALSE;
	}

	CT_VOID clearPlayerOtherInfo()
	{
		//清除玩家其他信息
		bIsTuoGuan = CT_FALSE;
		for(int i = 0; i < MAX_WEAVE; i++)
		{
			weaveItem[i].clearWeaveItem();
		}
		m_AnalyseItemArray.RemoveAll();
		canGangCard.clear();
		bIsTing = CT_FALSE;
		cbDiscardCount = 0;
		memset(cbDiscardCard, INVALID_CARD_DATA, sizeof(cbDiscardCard));
		cbOutCardCount = 0;
		cbHuCard = INVALID_CARD_DATA;
		bHu = CT_FALSE;
		llHuCardType = -1;
		wHuCardFan = 0;
		bZiMo = CT_FALSE;
		memset(cbListenCardData, 0, sizeof(cbListenCardData));
		cbListenCardCount = 0;
		bIsOutFirstCard_AfterTing = CT_TRUE;
		bIsSuanZiMo = CT_FALSE;
		wJiaBei = 0;
		cbJiaBeiSource = 1;
		bTianTing = CT_FALSE;
		memset(cbHuCard_DisReconect, INVALID_CARD_DATA, sizeof(cbHuCard_DisReconect));
		memset(cbCanHuCard_AfterTing, INVALID_CARD_DATA, sizeof(cbCanHuCard_AfterTing));
		memset(cbMustOutCard_AfterClickTing, INVALID_CARD_DATA, sizeof(cbMustOutCard_AfterClickTing));
	}
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

