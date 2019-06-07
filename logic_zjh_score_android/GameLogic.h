#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////////
//宏定义
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

#define MAX_COUNT					3									//最大数目
#define	DRAW						2									//和局类型

//数值掩码
#define	LOGIC_MASK_COLOR			0xF0								//花色掩码
#define	LOGIC_MASK_VALUE			0x0F								//数值掩码

//扑克类型
#define CT_SINGLE					1									//单牌类型
#define CT_DOUBLE_TYPE				2									//对子类型
#define	CT_SHUN_ZI					3									//顺子类型
#define CT_JIN_HUA					4									//金花类型
#define	CT_SHUN_JIN					5									//顺金类型
#define	CT_BAO_ZI					6									//豹子类型
#define CT_SPECIAL					7									//特殊类型

//////////////////////////////////////////////////////////////////////////////////
//游戏逻辑类
class CGameLogic
{
	//变量定义
private:
	static CT_BYTE						m_cbCardListData[52];			//扑克定义

																		//函数定义
public:
	//构造函数
	CGameLogic();
	//析构函数
	virtual ~CGameLogic();

	//类型函数
public:
	//获取类型
	CT_BYTE GetCardType(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//获取数值
	CT_BYTE GetCardValue(CT_BYTE cbCardData) { return cbCardData&LOGIC_MASK_VALUE; }
	//获取花色
	CT_BYTE GetCardColor(CT_BYTE cbCardData) { return cbCardData&LOGIC_MASK_COLOR; }

	//控制函数
public:
	//排列扑克
	void SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//混乱扑克
	void RandCardList(CT_BYTE cbCardBuffer[], CT_BYTE cbBufferCount);

	//功能函数
public:
	//逻辑数值
	CT_BYTE GetCardLogicValue(CT_BYTE cbCardData);
	//对比扑克
	CT_BYTE CompareCard(CT_BYTE cbFirstData[], CT_BYTE cbNextData[], CT_BYTE cbCardCount, CT_BYTE bCompareColor = 0);
};

//////////////////////////////////////////////////////////////////////////
#endif		 


