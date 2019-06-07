#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_DDZ.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>


//宏定义
//////////////////////////////////////////////////////////////////////////////////

//排序类型
#define ST_ORDER					1									//大小排序
#define ST_COUNT					2									//数目排序
#define ST_CUSTOM					3									//自定排序

#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//////////////////////////////////////////////////////////////////////////////////

//分析结构
struct tagAnalyseResult
{
	CT_BYTE 						cbBlockCount[4];					//扑克数目0:单张个数,1:对子个数,2:三张个数,3:四张个数
	CT_BYTE							cbCardData[4][MAX_COUNT];			//扑克数据(3344的存放如:cbCardData[1][3344])
};

//出牌结果
struct tagOutCardResult
{
	CT_BYTE							cbCardCount;						//扑克数目
	CT_BYTE							cbResultCard[MAX_COUNT];			//结果扑克
};

//分布信息
struct tagDistributing
{
	CT_BYTE							cbCardCount;						//扑克数目
	CT_BYTE							cbDistributing[15][6];				//分布信息(一维:牌逻辑值减1;二维:下标0-4对应方梅红黑,下标5对应当前牌总个数)
};

//搜索结果
struct tagSearchCardResult
{
	CT_BYTE							cbSearchCount;						//结果数目
	CT_BYTE							cbCardCount[MAX_COUNT];				//扑克数目
	CT_BYTE							cbResultCard[MAX_COUNT][MAX_COUNT];	//结果扑克
};

//////////////////////////////////////////////////////////////////////////////////

//游戏逻辑类
class CGameLogic
{
	//变量定义
protected:
	static const CT_BYTE				m_cbCardData[FULL_COUNT];			//扑克数据
	static const CT_BYTE				m_cbGoodCardData[GOOD_CARD_COUTN];	//好牌数据
	//函数定义
public:
	//构造函数
	CGameLogic();
	//析构函数
	virtual ~CGameLogic();

	//类型函数
public:
	//获取类型
	CT_BYTE GetCardType(const CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//获取类型
	CT_BYTE GetBackCardType(const CT_BYTE cbCardData[]);
	//获取数值
	CT_BYTE GetCardValue(CT_BYTE cbCardData) { return cbCardData&MASK_VALUE; }
	//获取花色
	CT_BYTE GetCardColor(CT_BYTE cbCardData) { return cbCardData&MASK_COLOR; }

	//控制函数
public:
	//混乱扑克
	CT_VOID RandCardList(CT_BYTE cbCardBuffer[], CT_BYTE cbBufferCount, CT_BYTE cbBomCount);
	//排列扑克
	CT_VOID SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbSortType);
	//删除扑克
	bool RemoveCardList(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//删除扑克
	bool RemoveCard(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//排列扑克
	void SortOutCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//得到好牌
	CT_VOID GetGoodCardData(CT_BYTE cbGoodCardData[], CT_BYTE cbGoodCardCount);
	//删除好牌
	CT_BOOL RemoveGoodCardData(CT_BYTE cbGoodCardData[], CT_BYTE cbGoodCardCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);

	//逻辑函数
public:
	//逻辑数值
	CT_BYTE GetCardLogicValue(CT_BYTE cbCardData);
	//对比扑克
	bool CompareCard(const CT_BYTE cbFirstCard[], const CT_BYTE cbNextCard[], CT_BYTE cbFirstCount, CT_BYTE cbNextCount);
	//出牌搜索
	CT_BYTE SearchOutCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[], CT_BYTE cbTurnCardCount,
		tagSearchCardResult *pSearchCardResult);
	//同牌搜索
	CT_BYTE SearchSameCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCardCount,
		tagSearchCardResult *pSearchCardResult);
	//连牌搜索
	CT_BYTE SearchLineCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbBlockCount, CT_BYTE cbLineCount,
		tagSearchCardResult *pSearchCardResult);
	//带牌类型搜索(三带一，四带一等)
	CT_BYTE SearchTakeCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCount, CT_BYTE cbTakeCardCount,
		tagSearchCardResult *pSearchCardResult);
	//搜索飞机
	CT_BYTE SearchThreeTwoLine(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, tagSearchCardResult *pSearchCardResult);

	//内部函数
public:
	//构造扑克
	CT_BYTE MakeCardData(CT_BYTE cbValueIndex, CT_BYTE cbColorIndex);
	//分析扑克
	CT_VOID AnalysebCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagAnalyseResult & AnalyseResult);
	//分析分布
	CT_VOID AnalysebDistributing(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagDistributing & Distributing);
};

//////////////////////////////////////////////////////////////////////////////////

#endif		 


