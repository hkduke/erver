#pragma once
#include "NetEncrypt.h"
#include "CTType.h"
#include "CTData.h"
#include "my_ddzboot.h"
#include <stdio.h>
#include "GameLogic.h"
#include "MSG_DDZ.h"
struct ddz_cp_qz  //出牌权重分配
{
	CT_DOUBLE	 san_1;//单牌权重
	CT_DOUBLE double_1;//对子权重
	CT_DOUBLE three_1; //三带权重;
	CT_DOUBLE three_2; //飞机权重
	CT_DOUBLE lian_1; //连牌权重
	CT_DOUBLE lian_2;//双连牌


};
struct ddz_JD_SP  //出牌权重分配
{
	int SP_SH; //手数
	int JD_SH; //在N手牌中敌方要不起的牌的手数总和
	int JD_DP; //绝对大牌,炸弹与火箭
};
struct ddz_JDPX  //出牌权重分配
{
	CT_BYTE JD_DPPX[20]; //绝对大牌,1炸弹，2火箭,3大王，4是小王，5是2系列，6 1系列，7 是4个2，8是4个1
	CT_BYTE JD_SHPX[20]; //绝对手牌 1大王，2是小王，3是 2，4是 1，5是连，6是双顺，7是飞机
};
struct ddz_san_px  //散牌型存储
{
	CT_BYTE ddz_san_1[MAX_COUNT + 1]; //散存储


	int ps;//牌数

};
struct ddzlian_px           //单 连牌
{
	CT_BYTE ddzlian_1[MAX_shun]; //存储连牌
	CT_BYTE ddzlian_2[MAX_shun]; //存储连牌
	CT_BYTE ddzlian_3[MAX_shun]; //存储连牌
	CT_BYTE ddzlian_4[MAX_shun]; //存储连牌
	int lianpxjs; //有几个连
	int lian_1;//第一组连计数
	int lian_2;//第一组连计数
	int lian_3;//第一组连计数
	int lian_4;//第一组连计数
	int ps;//牌数

};
struct ddzmax_px             //大小王的牌型
{
	CT_BYTE ddzmax_1; //存储大王
	CT_BYTE ddzmax_2; //存储小王
	int maxpxjs; // 0,为什么都没有，1有大王，2有小王，3是火箭都有
	int ps;//牌数
};
struct ddztwo_px           //2的牌型
{
	CT_BYTE ddztwo_1[ZD_MAX]; //存储2

	int twopxjs; // 0,有几个2
	int ps;//牌数
};
struct ddzone_px           //1的牌型
{
	CT_BYTE ddzone_1[ZD_MAX]; //存储1

	int onepxjs; // 0,有几个1
	int ps;//牌数
};
struct ddztwolian_px           //双连牌
{
	CT_BYTE ddztwo_lian_1[MAX_shun]; //存储连牌
	CT_BYTE ddztwo_lian_2[MAX_shun]; //存储连牌
	int lianpxjs; //有几个连
	int lian_1;//第一组连计数
	int lian_2;//第一组连计数
	int ps;//牌数

};
struct ddz_zhadan_px   //1，和2 都存到单独结构里，没在炸弹里
{
	CT_BYTE ddzzd_1[ZD_MAX]; //存储炸弹
	CT_BYTE ddzzd_2[ZD_MAX]; //存储炸弹
	CT_BYTE ddzzd_3[ZD_MAX]; //存储炸弹
	CT_BYTE ddzzd_4[ZD_MAX]; //存储炸弹
	int zdpxjs; //有几个炸弹
	int ps;  //牌数
};
struct ddz_feiji_px  //飞机牌型存储
{
	CT_BYTE ddz_feiji_1[MAX_COUNT]; //存储飞机
	CT_BYTE ddz_feiji_2[MAX_COUNT]; //存储飞机
	CT_BYTE ddz_feiji_3[MAX_COUNT]; //存储飞机
	int feiji_1;//第一组飞机计数
	int feiji_2;//第一组飞机计数
	int feiji_3;//第一组飞机计数
	int feiji_js; //有几个飞机
	int ps;//牌数

};
struct ddz_three_px  //三连牌型存储
{
	CT_BYTE ddz_three_1[THREE_LIAN]; //三连存储
	CT_BYTE ddz_three_2[THREE_LIAN]; //三连存储
	CT_BYTE ddz_three_3[THREE_LIAN]; //三连存储
	CT_BYTE ddz_three_4[THREE_LIAN]; //三连存储
	CT_BYTE ddz_three_5[THREE_LIAN]; //三连存储
	int three_js; //有几个三连
	int ps;//牌数

};
struct ddz_two_px  //对子型存储
{
	CT_BYTE ddz_two_1[2]; //对子存储
	CT_BYTE ddz_two_2[2]; //对子存储
	CT_BYTE ddz_two_3[2]; //对子存储
	CT_BYTE ddz_two_4[2]; //对子存储
	CT_BYTE ddz_two_5[2]; //对子存储
	CT_BYTE ddz_two_6[2]; //对子存储
	int two_js;//对子计数
	int ps;//牌数

};
struct ddz_old_card  //最后出牌信息
{
	CT_DWORD old_Count; //出牌数目
	CT_BYTE old_Card_data[MAX_COUNT]; //出牌数据
	CT_DWORD old_Card; //0，自己，1，上家，2下家


};
struct onGameCard_1  //上家手里的牌
{
	CT_BYTE Cdata[MAX_COUNT];
	CT_BYTE Count; //炸弹

};
struct DownGameCard_1  //下家手里的牌
{
	CT_BYTE Cdata[MAX_COUNT];
	CT_BYTE Count; //炸弹

};
using namespace std;
class BOTProces
{
public:
	BOTProces();
	~BOTProces();
	CT_VOID SetPX_CZ(int pd); //牌型分析显示
	CT_BOOL GetCXpxPDSX(); //审核剩余牌型
	CT_VOID SetSan_CZ(); //散牌分析，单对
	CT_VOID Set_ZHPX(CT_DWORD		cbCardCount, CT_BYTE cbCardData[MAX_COUNT], CT_BYTE ICount);//ICount 出牌型
	CT_DWORD Set_FX_QZ();//权重分析
	bool Set_SINGLE1(CT_BYTE cbCardData1[MAX_COUNT],CT_DWORD gameuserid,CT_DWORD SX_Card);//单牌管，数据,ID 上家或者下家，剩余牌
	bool Set_DOUBLE(CT_BYTE cbCardDatas[MAX_COUNT], CT_DWORD gameuserid1, CT_DWORD SX_DOUBLE_Card);//对子牌管，数据,ID 上家或者下家，剩余牌
	bool Set_CT_THREE(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_DOUBLE_Card1);//三连管牌，gameuserid 1为上家，2为下家
	bool Set_CT_THREE_1(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_SQ_Card1);//三代1管牌，gameuserid 1为上家，2为下家,剩余牌数
	bool Set_CT_THREE_2(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_SQ_Card1);//三代2管牌，gameuserid 1为上家，2为下家,剩余牌数
	bool Set_DOUBLE_LINE(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_SQ_Card1);//双连管牌
	bool Set_SINGLE_LINE(CT_BYTE CardDatas[MAX_COUNT], CT_DWORD userids, CT_DWORD SQ_Card1);//单连管牌
	bool Set_DOUBLE_1(int zf_1, CT_BYTE cbCardDatas[MAX_COUNT]);//正反序
	bool Set_JdOutCard(int XCard, int CardX);//绝对手牌出牌方案/0为自己出牌，1为 管牌,管牌的类型,1单，2对，3，三不带，4，三带1，5，三带二，6飞机，7双顺，8，连
	int FindCard(int leix, int Icont);// 1，查找类型，绝手还是还是绝大,0绝收1大牌，查找编号
	bool SetGOCard(string str);
	bool Set_outCard(int Icont); //1出绝大,2出绝手
	bool Set_THREE_2_TWO(CT_BYTE cbCardDatas[MAX_COUNT], CT_BYTE ICount);//判断是否是飞机,1是飞机带单，2是飞机带队
	CT_BYTE Set_THREE_PX(CT_BYTE cbCardDatas[MAX_COUNT]);//判断是否是飞机,1是飞机带单，2是飞机带队
	CT_VOID SET_FOUR_TAKE_TWO();
	CT_VOID Set_SHHB();
	bool Set_Card_OUT(); //判断炸弹出不出手
//	bool Set_BS_PX(CT_BYTE CardDatas);//必胜牌型
	bool jdyp; //必胜牌分析
	CT_VOID Get_zd();
	bool Set_FX_JDSP(int OUT_CARD);//分析绝对手牌,0为自己出牌，1为管别人的牌
	int bot_king; //0 不叫,1，加倍，2叫地主，和加倍
	int   pssj_vh; //扑克数量;
	int Bankxin; //1为自己地主，2为上家地主，3为下家地主
	CT_BYTE OnGameCard; //上家扑克数目
	CT_BYTE DownGameCard; //上家扑克数目
	CT_DWORD  OnGameID; //上家ID
	CT_DWORD DownGameID; //下家ID
	CT_BYTE myCard[20]; //自己扑克数目
ddz_san_px   ddz_san;//散牌存储
	ddzlian_px ddz_lian_1;//存储连牌结构l
	ddzmax_px ddz_max ;//大王存储
	ddztwo_px ddz_two ;//存储2的结构
    ddzone_px  ddz_one ;//存储1的结构
	ddztwolian_px ddz_twolian ;//存储双连结构
	ddz_zhadan_px ddz_zd ;//炸弹结构存储
	ddz_feiji_px ddz_fj ;//飞机结构存储
	ddz_three_px ddz_three ;//三连结构存储
    ddz_two_px   ddz_two_fx ;//对子存储
	ddz_old_card ddz_old ; //最后出牌
	onGameCard_1 onCard ;
	DownGameCard_1 DownCard;
	CGameLogic *GetCodePX;
	int  BotID;
	CMD_C_OutCard mOutCard; //发送打牌结构
private:
	ddz_JD_SP SPFX; //手牌分析
	ddz_JDPX  JD_JL ;//绝对牌型分析
	ddz_cp_qz  qz;
};

