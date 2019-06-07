#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_LD_FPF.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>
#include "CRand.h"


//宏定义
//////////////////////////////////////////////////////////////////////////
//逻辑掩码
#define MASK_COLOR					0xF0								//花色掩码
#define MASK_VALUE						0x0F								//数值掩码

#define MAX_CARD_TOTAL			80									//牌总个数

//////////////////////////////////////////////////////////////////////////
////动作定义
//#define ACK_NULL							0x00								//空
//#define ACK_TI								0x01								//提
//#define ACK_WEI								0x02								//偎
//#define ACK_CHI								0x04								//吃
//#define ACK_CHI_EX						0x08								//绞牌
//#define ACK_PENG							0x10								//碰
//#define ACK_PAO							0x20								//跑
//#define ACK_CHIHU						0x40								//胡
//#define ACK_JIANG							0x80								//将牌


//吃牌类型
#define CK_NULL								0x00								//无效类型
#define CK_XXD								0x01								//小小大搭
#define CK_XDD								0x02								//小大大搭
#define CK_EQS								0x04								//二七十吃
#define CK_LEFT								0x10								//靠左对齐
#define CK_CENTER						0x20								//居中对齐
#define CK_RIGHT							0x40								//靠右对齐

//数组维数
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//分析子项
struct tagAnalyseItem
{
	CT_BYTE							cbCardEye;								//牌眼扑克
	CT_BYTE							cbHuXiCount;							//胡息数目
	CT_BYTE							cbWeaveCount;						//组合数目
	tagWeaveItem					WeaveItemArray[MAX_WEAVE];			//组合扑克
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
    static const CT_BYTE		m_byCardDataArray[MAX_CARD_TOTAL];	//扑克数据
    CCRand							m_rand;								//随机数
    
    //控制函数
public:
    //获得随机数
    CT_INT32 GetRandValue();
    //混乱扑克
    CT_VOID RandCardData(CT_BYTE byCardBuffer[], CT_BYTE byBufferCount);
	//排序,根据牌值排序
	CT_BOOL SortCardList(CT_BYTE byCardData[MAX_COUNT], CT_BYTE byCardCount);
    //删除扑克
    CT_BOOL RemoveCard(CT_BYTE byCardIndex[MAX_INDEX], CT_BYTE byRemoveCard);
    
    //辅助函数
public:
    //有效判断
    CT_BOOL IsValidCard(CT_BYTE byCardData);
	//是否红牌
	CT_BOOL IsHongCard(CT_BYTE byCardData);
    //扑克数目
    CT_BYTE GetCardCount(const CT_BYTE byCardIndex[MAX_INDEX]);
	//获取数值
	CT_BYTE GetCardValue(CT_BYTE cbCardData) { return cbCardData&MASK_VALUE; }
	//获取花色
	CT_BYTE GetCardColor(CT_BYTE cbCardData) { return cbCardData&MASK_COLOR; }
    
    //转换函数
public:
    //扑克转换
    CT_BYTE SwitchToCardData(CT_BYTE byCardIndex);
    //扑克转换
    CT_BYTE SwitchToCardIndex(CT_BYTE byCardData);
    //扑克转换
    CT_BYTE SwitchToCardData(const CT_BYTE byCardIndex[MAX_INDEX], CT_BYTE byCardData[MAX_COUNT]);
    //扑克转换
    CT_BYTE SwitchToCardIndex(const CT_BYTE byCardData[], CT_BYTE byCardCount, CT_BYTE byCardIndex[MAX_INDEX]);
    
	//逻辑函数
public:
	//提牌判断
	CT_BYTE GetAcitonTiCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbTiCardIndex[5]);
	//畏牌判断
	CT_BYTE GetActionWeiCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbWeiCardIndex[7]);

    //动作判断
public:
	//获得吃牌组合
	CT_BYTE GetChiWeave(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3]);
	//查出指定牌数量
	CT_BYTE FindCardCount(CT_BYTE cbFindCard, CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCount);
	//是否相同组合
	CT_BOOL IsSameWeave(CT_BYTE cbWeaveOne[3], CT_BYTE cbWeaveTwo[3]);

	//名堂判断
public:
	//是否乌胡
	CT_BOOL IsWuHu(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount);
	//十三红
	CT_BOOL IsShiSanHong(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount);
	//十红
	CT_BOOL IsShiHong(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount);
	//一块扁
	CT_BOOL IsYiKuaiBian(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount);
	//一点红
	CT_BOOL IsYiDianHong(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount);

    //内部函数
public:
	//胡牌结果
	CT_BOOL GetHuCardInfo(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, CT_BYTE cbHuXiWeave, tagHuCardInfo & HuCardInfo);
	//分析扑克
	CT_BOOL AnalyseCard(const CT_BYTE cbCardIndex[MAX_INDEX], std::vector<tagAnalyseItem>& AnalyseItemArray);
	//获取胡息
	CT_BYTE GetWeaveHuXi(const tagWeaveItem & WeaveItem);
	//分析单钓
	CT_BOOL AnalyDanDiao(const CT_BYTE cbCardIndex[MAX_INDEX], std::vector<tagAnalyseItem>& AnalyseItemArray);
	//分析组合
	CT_BYTE AnalyseWeaveItem(const CT_BYTE cbCardIndex[MAX_INDEX], tagWeaveItem WeaveItem[76]);
	//是否偎碰
	CT_BOOL IsWeiPengCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard);
};

//////////////////////////////////////////////////////////////////////////

#endif		//GAME_LOGIC_HEAD_FILE


