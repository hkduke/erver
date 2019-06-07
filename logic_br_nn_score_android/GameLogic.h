#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_BR_NN_SCORE.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>


//宏定义
//////////////////////////////////////////////////////////////////////////
//逻辑掩码
#define MASK_COLOR					0xF0								//花色掩码
#define MASK_VALUE					0x0F								//数值掩码
#define MAX_CARD_TOTAL				52									//牌总个数

//扑克类型
#define OX_VALUE0					0									//混合牌型
//#define OX_SILVERY_BULL				11									//银牛
#define OX_SHUNZI					11									//顺子
#define OX_TONGHUA					12									//同花
#define OX_HULU						13									//葫芦牛
#define OX_BOMB						14									//炸弹（四梅）
#define OX_GOLDEN_BULL				15									//五花牛（金牛）
#define OX_TONGHUA_SHUN				16									//同花顺
#define OX_FIVE_SMALL				17									//五小牛

//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//游戏逻辑类
class CGameLogic
{
    //函数定义
public:
    //构造函数
    CGameLogic();
    //析构函数
    virtual ~CGameLogic();
    
    //变量定义
protected:
    static const CT_BYTE		m_byCardDataArray[MAX_CARD_TOTAL];	//扑克数据
    
public:
	//获取数值
	CT_BYTE GetCardValue(CT_BYTE cbCardData) { return cbCardData&MASK_VALUE; }
	//获取花色
	CT_BYTE GetCardColor(CT_BYTE cbCardData) { return cbCardData&MASK_COLOR; }
	//逻辑数值
	CT_BYTE GetLogicValue(CT_BYTE cbCardData);
	//逻辑数值(用于判断顺子)
	CT_BYTE GetCardLogicValue(CT_BYTE cbCardData);

public:
    //混乱扑克
    CT_VOID RandCardData(CT_BYTE byCardBuffer[], CT_BYTE byBufferCount);
	//排列扑克
	CT_VOID SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//排列扑克
	CT_VOID SortCardList2(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//获取类型
	CT_BYTE GetCardType(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//对比扑克
	CT_BOOL CompareCard(CT_BYTE cbFirstCard[], CT_BYTE cbNextCard[]);
	//获取倍数
	CT_BYTE GetMultiple(CT_BYTE cbCardData[], CT_BYTE cbRoomKind);
	//获取牛牛牌
	CT_BOOL GetOxCard(CT_BYTE cbCardData[]);

public:
	//是否为五小
	CT_BOOL IsFiveSmall(CT_BYTE cbCardData[MAX_COUNT]);
	//是否同花顺
	CT_BOOL IsTongHuaShun(CT_BYTE cbCardData[MAX_COUNT]);
	//是否为炸弹
	CT_BOOL IsBomb(CT_BYTE cbCardData[MAX_COUNT]);
	//是否为金牛(五花牛)
	CT_BOOL IsGoldenBull(CT_BYTE cbCardData[MAX_COUNT]);
	//是否葫芦牛
	CT_BOOL IsHuLu(CT_BYTE cbCardData[MAX_COUNT]);
	//是否同花
	CT_BOOL IsTongHua(CT_BYTE cbCardData[MAX_COUNT]);
	//是否顺子
	CT_BOOL IsShunZi(CT_BYTE cbCardData[MAX_COUNT]);
	//是否为银牛
	//CT_BOOL IsSilveryBull(CT_BYTE cbCardData[MAX_COUNT]);
};

//////////////////////////////////////////////////////////////////////////

#endif		//GAME_LOGIC_HEAD_FILE


