#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>
#include <sys/time.h>
#include <CMD_Dip.h>
#include "MSG_mjxzdd.h"

#define MAX_CARD_COUNT 64

//////////////////////////////////////////////////////////////////////////////////////////////////////////
//移植
//用于财神的转换，如果有牌可以代替财神本身牌使用，则设为该牌索引，否则设为MAX_INDEX. 注:如果替换牌是序数牌,将出错.
#define	INDEX_REPLACE_CARD					MAX_INDEX

//逻辑掩码
#define	MASK_COLOR					0xF0								//花色掩码
#define	MASK_VALUE					0x0F								//数值掩码

//动作标志
/*#define WIK_NULL					0x00								//没有类型
#define WIK_LEFT					0x01								//左吃类型
#define WIK_CENTER					0x02								//中吃类型
#define WIK_RIGHT					0x04								//右吃类型
#define WIK_PENG					0x08								//碰牌类型
#define WIK_GANG					0x10								//杠牌类型
#define WIK_LISTEN					0x20								//听牌类型
#define WIK_CHI_HU					0x40								//吃胡类型
#define WIK_REPLACE					0x80								//花牌替换*/

//胡牌定义
enum HUTYPES
{
	HU_TYPE_DAN_DIAO,					//单调将
	HU_TYPE_KAN_ZHANG,					//坎张
	HU_TYPE_BIAN_ZHANG,					//边张
	HU_TYPE_MING_GANG,					//明杠
	HU_TYPE_YAO_JIU_KE,					//幺九刻
	HU_TYPE_LAO_SHAO_FU,				//老少副
	HU_TYPE_LIAN_LIU,					//连六
	HU_TYPE_YI_BAN_GAO,					//一般高
	HU_TYPE_BAO_TING,					//报听
	HU_TYPE_YAO_JIU_TOU,				//幺九头
	HU_TYPE_ER_WU_BA_JIANG,				//二五八将
	HU_TYPE_DUAN_YAO,					//断幺
	HU_TYPE_AN_GANG,					//暗杠
	HU_TYPE_SHUANG_AN_KE,				//双暗刻
	HU_TYPE_SI_GUI_YI,					//四归一
	//HU_TYPE_PING_HU,					//平胡
	HU_TYPE_MEN_QIAN_QING,				//门前清
	HU_TYPE_MEN_FENG_KE,				//门风刻
	HU_TYPE_QUAN_FENG_KE,				//圈风刻
	HU_TYPE_JIAN_KE,					//箭刻
	HU_TYPE_LI_ZHI,						//立直
	//HU_TYPE_HU_JUE_ZHANG,				//胡绝张
	HU_TYPE_SHUANG_MING_GANG,			//双明杠
	HU_TYPE_BU_QIU_REN,					//不求人
	HU_TYPE_QUAN_DAI_YAO,				//全带幺
	HU_TYPE_SHUANG_JIAN_KE,				//双箭刻
	HU_TYPE_SHUANG_AN_GANG,				//双暗杠
	HU_TYPE_QUAN_QIU_REN,				//全求人
	HU_TYPE_HUN_YI_SE,					//混一色
	//HU_TYPE_PENG_PENG_HU,				//碰碰胡
	//HU_TYPE_QIANG_GANG_HU,				//抢杠胡
	//HU_TYPE_GANG_SHANG_HUA,				//杠上花
	//HU_TYPE_HAI_DI_LAO_YUE,				//海底捞月
	HU_TYPE_MIAO_SHOU_HUI_CHUN,			//妙手回春
	HU_TYPE_SAN_FENG_KE,				//三风刻
	HU_TYPE_XIAO_YU_5,					//小于5
	HU_TYPE_DA_YU_5,					//大于5
	HU_TYPE_TIAN_TING,					//天听
	HU_TYPE_SAN_AN_KE,					//三暗刻
	HU_TYPE_YI_SE_SAN_BU_GAO,			//一色三步高
	HU_TYPE_QING_LONG,					//清龙
	HU_TYPE_YI_SE_SAN_JIE_GAO,			//一色三节高
	HU_TYPE_YI_SE_SAN_TONG_SHUN,		//一色三同顺
	//HU_TYPE_QING_YI_SE,					//清一色
	//HU_TYPE_QI_DUI,						//七对
	HU_TYPE_HUN_YAO_JIU,				//混幺九
	HU_TYPE_SAN_GANG,					//三杠
	HU_TYPE_YI_SE_SI_BU_GAO,			//一色四步高
	HU_TYPE_YI_SE_SI_JIE_GAO,			//一色四节高
	HU_TYPE_YI_SE_SI_TONG_SHUN,			//一色四同顺
	HU_TYPE_YI_SE_SHUANG_LONG,			//一色双龙会
	HU_TYPE_SI_AN_KE,					//四暗刻
	HU_TYPE_ZI_YI_SE,					//字一色
	HU_TYPE_XIAO_SAN_YUAN,				//小三元
	HU_TYPE_XIAO_SI_XI,					//小四喜
	HU_TYPE_BAI_WAN_SHI,				//百万石
	HU_TYPE_REN_HU,						//人和
	//HU_TYPE_DI_HU,						//地和
	//HU_TYPE_TIAN_HU,					//天和
	HU_TYPE_LIAN_QI_DUI,				//连七对
	HU_TYPE_SI_GANG,					//四杠
	HU_TYPE_JIU_LIAN_BAO_DENG,			//九莲宝灯
	HU_TYPE_DA_SAN_YUAN,				//大三元
	HU_TYPE_DA_SI_XI,					//大四喜


	//平胡
	HU_TYPE_PING_HU,
	//清龙7对
	HU_TYPE_QING_LONG_QI_DU,
	//清七对
	HU_TYPE_QING_QI_DU,
	//清幺九
	HU_TYPE_QING_YAO_JIU,
	//清对
	HU_TYPE_QING_DUI,
	//清一色
	HU_TYPE_QING_YI_SE,
	//龙7对
	HU_TYPE_LONG_QI_DUI,
	//七对
	HU_TYPE_QI_DUI,
	//碰碰胡
	HU_TYPE_PENG_PENG_HU,
	//杠上花
	HU_TYPE_GANG_SHANG_HUA,
	//杠上炮
	HU_TYPE_GANG_SHANG_PAO,
	//天胡
	HU_TYPE_TIAN_HU,
	//地胡
	HU_TYPE_DI_HU,
	//海底捞月
	HU_TYPE_HAI_DI_LAO_YUE,
	//绝张
	HU_TYPE_HU_JUE_ZHANG,
	//大单钓
	HU_TYPE_DA_DAN_DIAO,
	//抢杠胡
	HU_TYPE_QIANG_GANG_HU,
	//带幺九
	HU_TYPE_DAI_YAO_JIU,
	//将对
	HU_TYPE_JIANG_DUI,
	//自摸
	HU_TYPE_ZI_MO,
	//根
	HU_TYPE_GEN,
};

// 标志处理
template<typename FlagEnumType>
class EnumFlag
{
public:
	explicit		EnumFlag(CT_LONGLONG data = 0):data_(data), base_(1L){}

	template<FlagEnumType flag>inline	CT_VOID	SetFlag()			{data_|=(base_<<flag);}
	template<FlagEnumType flag>inline	CT_VOID	SetFlag(CT_BOOL bset)  {bset?SetFlag<flag>():ResetFlag<flag>();}
	template<FlagEnumType flag>inline	CT_VOID	ResetFlag()			{data_&=~(base_<<flag);}
	template<FlagEnumType flag>inline	CT_BOOL	TestFlag()const		{return (0!=(data_&(base_<<flag)));}

	inline	CT_VOID	SetFlag(FlagEnumType flag)      {data_|=(base_<<flag);}
	inline	CT_VOID	ResetFlag(FlagEnumType flag)    {data_&=~(base_<<flag);}
	inline	CT_BOOL	TestFlag(FlagEnumType flag)const{return (0!=(data_&(base_<<flag)));}

	inline	CT_LONGLONG	GetData()const					{return data_;}
	inline	CT_VOID	SetData(CT_LONGLONG data)             {data_ = data;}
	inline	CT_VOID	ResetData()                     {data_ = 0;}

private:
	CT_LONGLONG	data_;
	CT_LONGLONG	base_;
};

//类型子项
struct tagKindItem
{
	CT_BYTE							cbWeaveKind;						//组合类型
	CT_BYTE							cbCenterCard;						//中心扑克
	CT_BYTE							cbValidIndex[3];					//实际扑克索引
};

//组合子项
/*struct tagWeaveItem
{
	CT_BYTE							cbWeaveKind;						//组合类型
	CT_BYTE							cbCenterCard;						//中心扑克
	CT_BYTE							cbPublicCard;						//公开标志
	CT_WORD							wProvideUser;						//供应用户
	CT_BYTE							cbCardData[4];						//
};*/



#define MASK_CHI_HU_RIGHT			0x0fffffff

/*
//	权位类。
//  注意，在操作仅位时最好只操作单个权位.例如
//  CChiHuRight chr;
//  chr |= (chr_zi_mo|chr_peng_peng)，这样结果是无定义的。
//  只能单个操作:
//  chr |= chr_zi_mo;
//  chr |= chr_peng_peng;
*/
class CChiHuRight
{
	//静态变量
private:
	static CT_BOOL						m_bInit;
	static CT_DWORD					m_dwRightMask[MAX_RIGHT_COUNT];

	//权位变量
private:
	CT_DWORD							m_dwRight[MAX_RIGHT_COUNT];

public:
	//构造函数
	CChiHuRight();

	//运算符重载
public:
	//赋值符
	CChiHuRight & operator = ( CT_DWORD dwRight );

	//与等于
	CChiHuRight & operator &= ( CT_DWORD dwRight );
	//或等于
	CChiHuRight & operator |= ( CT_DWORD dwRight );

	//与
	CChiHuRight operator & ( CT_DWORD dwRight );
	CChiHuRight operator & ( CT_DWORD dwRight ) const;

	//或
	CChiHuRight operator | ( CT_DWORD dwRight );
	CChiHuRight operator | ( CT_DWORD dwRight ) const;

	//相等
	CT_BOOL operator == ( CT_DWORD dwRight ) const;
	CT_BOOL operator == ( const CChiHuRight chr ) const;

	//不相等
	CT_BOOL operator != ( CT_DWORD dwRight ) const;
	CT_BOOL operator != ( const CChiHuRight chr ) const;

	//功能函数
public:
	//是否权位为空
	CT_BOOL IsEmpty();

	//设置权位为空
	CT_VOID SetEmpty();

	//获取权位数值
	CT_BYTE GetRightData( CT_DWORD dwRight[], CT_BYTE cbMaxCount );

	//设置权位数值
	CT_BOOL SetRightData( const CT_DWORD dwRight[], CT_BYTE cbRightCount );

private:
	//检查权位是否正确
	CT_BOOL IsValidRight( CT_DWORD dwRight );
};

/*
static CT_VOID NiqiuSrand()
{
#ifdef _WIN32
	LARGE_INTEGER nFrequency;
	if(QueryPerformanceFrequency(&nFrequency))
	{
		LARGE_INTEGER nStartCounter;
		QueryPerformanceCounter(&nStartCounter);
		srand((unsigned)nStartCounter.LowPart);
	}
	srand(GetTickCount()*rand());
#else
	timeval val;
	gettimeofday(&val,NULL);

	srand(val.tv_usec);
#endif
}*/
//////////////////////////////////////////////////////////////////////////////////////////////////////////


//游戏逻辑类
class CGameLogic
{
public:
	//构造函数
	CGameLogic();
	//析构函数
	virtual ~CGameLogic();

public:
	//混乱牌型
	CT_VOID RandCard(CT_BYTE cbCardData[], CT_WORD cbMaxCount);
	//从基础牌堆里面从前往后拿牌并且发给玩家
	CT_VOID GetSendCard(CT_BYTE getCard[], CT_WORD wGetCardCount);
	//获取目前剩余牌堆的牌数量
	CT_WORD GetPaiDuiCount(){return MAX_CARD_COUNT - m_wCardBeginIndex;}

public:
	//本局使用的麻将牌型
	CT_BYTE m_cbCurCardData[MAX_CARD_COUNT];
private:
    //麻将原始牌
	//const static CT_BYTE m_cbCardDataArray[MAX_CARD_COUNT];
    //m_cbCurCardData的开始索引
    CT_WORD  m_wCardBeginIndex;




    //////////////////////////////////////////////////////////////////////////////////////////////////////////
    //移植
	//变量定义
protected:
	static const CT_BYTE				m_cbCardDataArray[MAX_REPERTORY];	//扑克数据
	CT_BYTE							m_cbMagicIndex;						//钻牌索引

	//random_engine					m_random;							//随机生成器
public:
	//混乱扑克
	CT_VOID RandCardData(CT_BYTE cbCardData[], CT_BYTE cbMaxCount);
	//混乱扑克
	CT_VOID RandCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbRandData[]);
	//检查扑克
	CT_BOOL CheckCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbRemoveCard);
	//删除扑克
	CT_BOOL RemoveCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbRemoveCard);
	//添加扑克
	CT_BOOL AddCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbAddCard);
	//删除扑克
	CT_BOOL RemoveCard(CT_BYTE cbCardIndex[MAX_INDEX], const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount);
	//删除扑克
	CT_BOOL RemoveCard(CT_BYTE cbCardData[], CT_BYTE cbCardCount, const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount);
	//设置财神
	CT_VOID SetMagicIndex( CT_BYTE cbMagicIndex ) { m_cbMagicIndex = cbMagicIndex; }
	//财神判断
	CT_BOOL IsMagicCard( CT_BYTE cbCardData );
	//花牌判断
	CT_BOOL IsHuaCard( CT_BYTE cbCardData );
	//获得牌的花色
	CT_BYTE GetCardColor(CT_BYTE cbcardData);

	//辅助函数
public:
	//有效判断
	CT_BOOL IsValidCard(CT_BYTE cbCardData);
	//扑克数目
	CT_BYTE GetCardCount(const CT_BYTE cbCardIndex[MAX_INDEX]);
	//组合扑克
	CT_BYTE GetWeaveCard(CT_BYTE cbWeaveKind, CT_BYTE cbCenterCard, CT_BYTE cbCardBuffer[4]);

	//获取手牌中某牌的张数
	CT_BYTE GetCardCount(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbOneCardIndex);
	//获取手牌中某张牌的张数
	CT_BYTE GetCardCountByCardData(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbOneCardData);
	//等级函数
public:
	//动作等级
	CT_BYTE GetUserActionRank(CT_BYTE cbUserAction);
	//胡牌等级
	CT_WORD GetChiHuActionRank(const CChiHuRight & ChiHuRight);

	//动作判断
public:
	//吃牌判断
	CT_BYTE EstimateEatCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard);
	//碰牌判断
	CT_BYTE EstimatePengCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard);
	//杠牌判断
	CT_BYTE EstimateGangCard(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard);

	//动作判断
public:
	//杠牌分析
	CT_BYTE AnalyseGangCard(const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, tagGangCardResult & GangCardResult);
	//吃胡分析
	CT_BYTE AnalyseChiHuCard(const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, CT_BYTE cbCurrentCard, CChiHuRight &ChiHuRight, CAnalyseItemArray& AnalyseItemArray);
	//听牌分析/*const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount*/
	CT_BYTE AnalyseTingCard(CT_WORD wChairID, CT_WORD wBankerUser, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX],
						 const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], CT_BYTE cbOutCardCount,
						 const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER],
						 const CT_BYTE cbListenStatus[GAME_PLAYER], CT_BOOL bIsTianTing[GAME_PLAYER], CT_BYTE cbQuanFeng, CT_BYTE cbMenFeng,
						 CT_BYTE cbLeftCardCount, CT_BOOL bGangStatus, const CT_CHAR szGameRoomName[LEN_SERVER],
						 CT_BYTE cbListenCardData[MAX_COUNT][2+MAX_INDEX*4], CT_BYTE& cbListenCardCount, CT_BOOL bOnce = false);

	CT_VOID GetHuCardData(CT_WORD wChairID, CT_WORD wBankerUser, CT_WORD wOutCardUser, CT_WORD wProvideUser, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX],
					   const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], CT_BYTE cbOutCardCount, const CAnalyseItemArray& AnalyseItemArray, const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER],
					   const CT_BYTE cbListenStatus[GAME_PLAYER], CT_BOOL bIsTianTing[GAME_PLAYER], CT_BYTE cbQuanFeng, CT_BYTE cbMenFeng, CT_BYTE cbLeftCardCount, CT_BOOL bGangStatus, const CT_CHAR szGameRoomName[LEN_SERVER],
					   CT_BYTE cbListenCardData[2+MAX_INDEX*4], CT_BYTE cbCard);

	//13张牌听牌分析
	CT_BYTE AnalyseTingCard13( const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount );

	CT_BYTE AnalyseTingCard13Ex( const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, CT_BYTE cbCurCardIndex );

	//分析好牌
	CT_BYTE AnalyseGoodCard( const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE& cbFengCardCount );

	//转换函数
public:
	//扑克转换
	CT_BYTE SwitchToCardData(CT_BYTE cbCardIndex);
	//扑克转换
	CT_BYTE SwitchToCardIndex(CT_BYTE cbCardData);
	//扑克转换
	CT_BYTE SwitchToCardData(const CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCardData[MAX_COUNT]);
	//扑克转换
	CT_BYTE SwitchToCardIndex(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbCardIndex[MAX_INDEX]);

	//内部函数
private:
	//分析扑克
	CT_BOOL AnalyseCard(const CT_BYTE cbCardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, CAnalyseItemArray & AnalyseItemArray);
	//排序,根据牌值排序
	CT_BOOL SortCardList( CT_BYTE cbCardData[MAX_COUNT], CT_BYTE cbCardCount );

	//检查牌型函数
public:
	CT_DWORD GetGenCount(const CT_BYTE cbHandIndex[MAX_INDEX]);
	//取得胡牌番数及牌型
	CT_LONGLONG GetHuType(CT_WORD wChairID, CT_WORD wBankerUser, CT_WORD wOutCardUser, CT_WORD wProvideUser, CT_BYTE cbProvideCard, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX],
					   const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], CT_BYTE cbOutCardCount, const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER],
					   const CT_BYTE cbListenStatus[GAME_PLAYER], CT_BOOL bIsTianTing[GAME_PLAYER], CT_BYTE cbQuanFeng, CT_BYTE cbMenFeng, CT_BYTE cbLeftCardCount, CT_BOOL bGangStatus, const CT_CHAR szGameRoomName[LEN_SERVER],
					   const CAnalyseItemArray& AnalyseItemArray, CT_WORD& wHuMultiple, tagAnalyseItem& LastAnalyseItem, CT_BOOL& cbIsZiMo,CT_BOOL bCheckTing = false);
	//取得手牌索引
	CT_BYTE GetHandcardIndex(const tagAnalyseItem& AnalyseItem, CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//是否清一色
	CT_BOOL IsQingYiSe(const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//大四喜
	CT_BOOL CheckDaSiXi(const tagAnalyseItem& AnalyseItem);
	//大三元
	CT_BOOL CheckDaSanYuan(const tagAnalyseItem& AnalyseItem);
	//九莲宝灯
	CT_BOOL CheckJiuLianBaoDeng(const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//四杠
	CT_BOOL CheckSiGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//连七对
	CT_BOOL CheckLianQiDui(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//天和
	CT_BOOL CheckTianHu(const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//百万石
	CT_BOOL CheckBaiWanShi(const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//小四喜
	CT_BOOL CheckXiaoSiXi(const tagAnalyseItem& AnalyseItem);
	//小三元
	CT_BOOL CheckXiaoSanYuan(const tagAnalyseItem& AnalyseItem);
	//字一色
	CT_BOOL CheckZiYiSe(const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//四暗刻
	CT_BOOL CheckSiAnKe(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const tagAnalyseItem& AnalyseItem);
	//一色双龙会
	CT_BOOL CheckYiSeShuangLong(const tagAnalyseItem& AnalyseItem);
	//一色四同顺
	CT_BOOL CheckYiSeSiTongShun(const tagAnalyseItem& AnalyseItem);
	//一色四节高
	CT_BOOL CheckYiSeSiJieGao(const tagAnalyseItem& AnalyseItem);
	//一色四步高
	CT_BOOL CheckYiSeSiBuGao(const tagAnalyseItem& AnalyseItem);
	//三杠
	CT_BOOL CheckSanGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//混幺九
	CT_BOOL CheckHunYaoJiu(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//七对
	CT_BOOL CheckQiDui(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//一色三同顺
	CT_BOOL CheckYiSeSanTongShun(const tagAnalyseItem& AnalyseItem);
	//一色三节高
	CT_BOOL CheckYiSeSanJieGao(const tagAnalyseItem& AnalyseItem);
	//清龙
	CT_BOOL CheckQingLong(const tagAnalyseItem& AnalyseItem);
	//一色四步高
	CT_BOOL CheckYiSeSanBuGao(const tagAnalyseItem& AnalyseItem);
	//三暗刻
	CT_BOOL CheckSanAnKe(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const tagAnalyseItem& AnalyseItem);
	//大于5
	CT_BOOL CheckDaYu5(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//小于5
	CT_BOOL CheckXiaoYu5(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//三风刻
	CT_BOOL CheckSanFengKe(const tagAnalyseItem& AnalyseItem);
	//碰碰胡
	CT_BOOL CheckPengPengHu(const tagAnalyseItem& AnalyseItem);
	//全求人
	CT_BOOL CheckQuanQiuRen(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//双暗杠
	CT_BOOL CheckShuangAnGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//双箭刻
	CT_BOOL CheckShuangJianKe(const tagAnalyseItem& AnalyseItem);
	//全带幺
	CT_BOOL CheckQuanDaiYao(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//不求人
	CT_BOOL CheckBuQiuRen(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//双明杠
	CT_BOOL CheckShuangMingGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//取牌剩余张数
	CT_BYTE GetCardRemain(CT_BYTE cbChairID, const CT_BYTE cbHandCardIndex[GAME_PLAYER][MAX_INDEX], const CT_BYTE bTing[GAME_PLAYER],const CT_BYTE cbDiscardCard[GAME_PLAYER][60], const CT_BYTE cbDiscardCount[GAME_PLAYER], const tagWeaveItem WeaveItemArray[GAME_PLAYER][MAX_WEAVE], const CT_BYTE cbWeaveItemCount[GAME_PLAYER], CT_BYTE cbCard);
	//门前清
	CT_BOOL CheckMenQianQing(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//箭刻
	CT_BOOL CheckJianKe(const tagAnalyseItem& AnalyseItem);
	//圈风刻
	CT_BOOL CheckQuanFengKe(const tagAnalyseItem& AnalyseItem, CT_BYTE cbQuanFendIndex);
	//门风刻
	CT_BOOL CheckMenFengKe(const tagAnalyseItem& AnalyseItem, CT_BYTE cbCard);
	//平胡
	CT_BOOL CheckPingHu(const tagAnalyseItem& AnalyseItem);
	//四归一
	CT_BOOL CheckSiGuiYi(const CT_BYTE cbHandcardIndex[MAX_INDEX], const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//双暗刻
	CT_BOOL CheckShuangAnKe(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount, const tagAnalyseItem& AnalyseItem);
	//暗杠
	CT_BOOL CheckAnGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//断幺
	CT_BOOL CheckDuanYao(const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//258将
	CT_BOOL Check258Jiang(const CAnalyseItemArray& AnalyseItemArray);
	//幺九头
	CT_BOOL CheckYaoJiuTou(const tagAnalyseItem& AnalyseItem);
	//一般高
	CT_BOOL CheckYiBanGao(const tagAnalyseItem& AnalyseItem);
	//连六
	CT_BOOL CheckLianLiu(const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX]);
	//老少副
	CT_BOOL CheckLaoShaoFu(const tagAnalyseItem& AnalyseItem);
	//幺九刻
	CT_BOOL CheckYaoJiuKe(const tagAnalyseItem& AnalyseItem);
	//明杠
	CT_BOOL CheckMingGang(const tagWeaveItem WeaveItem[], CT_BYTE cbItemCount);
	//边张
	CT_BOOL CheckBianZhang(const tagAnalyseItem& AnalyseItem, CT_BYTE cbCard);
	//坎张
	CT_BOOL CheckKanZhang(const tagAnalyseItem& AnalyseItem, CT_BYTE cbCard);
	//单调将
	CT_BOOL CheckDanDiaoJiang(const tagWeaveItem WeaveItem[], CT_BYTE cbWeaveCount, const tagAnalyseItem& AnalyseItem, const CT_BYTE cbHandcardIndex[MAX_INDEX], CT_BYTE cbCard);
	//////////////////////////////////////////////////////////////////////////////////////////////////////////

};

//////////////////////////////////////////////////////////////////////////
#endif		 


