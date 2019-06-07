#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_PDK.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>
#include "MSG_PDK.h"


//宏定义
//////////////////////////////////////////////////////////////////////////
//逻辑掩码
#define MASK_COLOR					0xF0								//花色掩码
#define MASK_VALUE					0x0F								//数值掩码
#define MAX_CARD_TOTAL				48									//牌总个数

//排序类型
#define ST_DESC						1									//大到小排序
#define ST_ACS						2									//小到大排序

/*
enum PDK_PlayKind
{
FEATURE = 1,		//特色玩法
CLASSIC = 2,		//经典玩法
};
*/

//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//分析结构
struct tagAnalyseResult
{
	CT_BYTE 					cbBlockCount[4];						//扑克数目0:单张个数,1:对子个数,2:三张个数,3:四张个数
	CT_BYTE						cbCardData[4][MAX_COUNT];				//扑克数据(3344的存放如:cbCardData[1][3344])
};

//分布信息
struct tagDistributing
{
	CT_BYTE						cbCardCount;							//扑克数目(总的牌数量)
	CT_BYTE						cbDistributing[15][6];					//分布信息(一维:牌逻辑值减1;二维:下标0-4对应方梅红黑,下标5对应当前牌总个数)
};

//搜索结果
struct tagSearchCardResult
{
	CT_BYTE						cbSearchCount;							//结果数目
	CT_BYTE						cbCardCount[MAX_COUNT];					//扑克数目
	CT_BYTE						cbResultCard[MAX_COUNT][MAX_COUNT];		//结果扑克
};

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
	static const CT_BYTE		m_byCardDataArray[MAX_CARD_TOTAL];		//扑克数据
	static const CT_BYTE		m_byCardDataArray15[MAX_CARD_TOTAL];	//扑克数据
	static const CT_BYTE		m_byGoodCardData[GOOD_CARD_COUTN];	    //好牌数据

public:
	//PDK_PlayKind				m_PlayKind;
	CT_BYTE						m_cbNotSplitRomb;						//炸弹不可拆  
	CT_BYTE						m_cbCanFourTakeTwo;						//允许4带2
	CT_BYTE						m_cbCanFourTakeThree;					//允许4带3
																		//最后一手牌
	CT_BYTE						m_cbIsOutThreeEnd;						//三张可少带出完 
	CT_BYTE						m_cbIsThreeEnd;							//三张可少带接完 
	CT_BYTE						m_cbIsOutFlyEnd;						//飞机可少带出完 
	CT_BYTE						m_cbIsFlyEnd;							//飞机可少带接完

																		//控制函数
public:
	//混乱扑克
	CT_VOID RandCardData(CT_BYTE byCardBuffer[], CT_BYTE byBufferCount);
	//排列扑克
	CT_VOID SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbSortType = ST_DESC);
	//删除扑克
	CT_BOOL RemoveCard(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//删除扑克
	CT_BOOL RemoveCardList(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//得到好牌
	CT_VOID GetGoodCardData(CT_BYTE cbGoodCardData[], CT_BYTE cbGoodCardCount);
	//删除好牌
	CT_BOOL RemoveGoodCardData(CT_BYTE cbGoodCardData[], CT_BYTE cbGoodCardCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//辅助函数
public:
	//获取数值
	CT_BYTE GetCardValue(CT_BYTE cbCardData) { return cbCardData&MASK_VALUE; }
	//获取花色
	CT_BYTE GetCardColor(CT_BYTE cbCardData) { return cbCardData&MASK_COLOR; }
	//逻辑数值
	CT_BYTE GetLogicValue(CT_BYTE cbCardData);
	//获取类型
	CT_BYTE GetCardType(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BOOL bLastCard = false, CT_BYTE cbAirLineCount[] = NULL);
	//删除部分扑克后剩余手牌的牌型
    CT_BYTE GetRemainCardType(const CT_BYTE cbCardData[], CT_BYTE cbCardCount,const CT_BYTE cbRemoveCardData[], CT_BYTE cbRemoveCardCount, tagAnalyseResult * pAnalyseResult = NULL, CT_BOOL * bIsLine = NULL);
	//对比扑克
	CT_BOOL CompareCard(const CT_BYTE cbFirstCard[], CT_BYTE cbFirstCount, const CT_BYTE cbNextCard[], CT_BYTE cbNextCount, CT_BOOL bLastCard = false);

	//出牌搜索
	CT_BYTE SearchOutCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[], CT_BYTE cbTurnCardCount,
		tagSearchCardResult& SearchCardResult);
	//搜索首出牌
	CT_BYTE SearchHeadOutCard(CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagSearchCardResult& SearchCardResult, CT_BYTE& cbResultCount);
	//同牌搜索
	CT_BYTE SearchSameCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCardCount,
		tagSearchCardResult& SearchCardResult);
	//连牌搜索
	CT_BYTE SearchLineCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbBlockCount, CT_BYTE cbLineCount,
		tagSearchCardResult& SearchCardResult);
	//带牌类型搜索(三带一，四带一等)
	CT_BYTE SearchTakeCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCount, CT_BYTE cbTakeCardCount,
		tagSearchCardResult& SearchCardResult);
	//搜索飞机
	CT_BYTE SearchThreeTwoLine(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbTakeCardCount, CT_BYTE cbLineCount, tagSearchCardResult& SearchCardResult, CT_BYTE cbSameCount = 3);
	//找最大单张
	CT_BYTE SearchMaxSingle(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard);
	//找最大对子
	CT_BYTE SearchMaxDouble(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE *cbResultCard);

	//拷贝第一个结果
	CT_VOID CopyFirstResult(tagSearchCardResult& SearchCardResult, CT_BYTE& cbResultCount, tagSearchCardResult TempSearchCardResult);
	//获得最少连牌个数
	CT_BYTE GetLessLineCount(CT_BYTE cbBlockCount, CT_BYTE cbLineCount);

	//搜索最大飞机
	CT_BYTE SearchMaxThreeLine(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbResultCardData[],CT_BYTE cbSameCardCount=3);

	//搜索手牌中最大的一张牌
	CT_BYTE SearchMaxOneCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[]);

	//内部函数
public:
	//构造扑克
	CT_BYTE MakeCardData(CT_BYTE cbValueIndex, CT_BYTE cbColorIndex);
	//分析扑克
	CT_VOID AnalysebCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagAnalyseResult & AnalyseResult);
	//分析分布
	CT_VOID AnalysebDistributing(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagDistributing & Distributing);

};

//////////////////////////////////////////////////////////////////////////

#endif		//GAME_LOGIC_HEAD_FILE


