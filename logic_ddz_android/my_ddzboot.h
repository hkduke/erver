#pragma once
//排序类型
#define ST_DZ					1									//地主
#define ST_NM					2									//农民
#define MAX_COUNT				20										//最大数目
#define FULL_COUNT				54										//全牌数目
#define NORMAL_COUNT			17										//常规数目
#define DDZ_MAX			        -1									//大王
#define ZD_MAX			        4									//炸弹数目
#define ST_THREE			       3									//三条数目
#define MAX_shun			       12									//最大连数
#define THREE_LIAN			       3									//三连个数

#include "AndroidUserItemSink.h"
#include "CTType.h"

struct ddzbootqz
{
	CT_DWORD m_cbBombCount;   //炸弹数量
							  //CT_DWORD BOT_THREE;//三条个数
	int BOT_MISSILE_CARD;//火箭  0为没有，1为有

	int BOT_ZG1; //小王 有为1
	int BOT_ZG2; //大王 有为1
	int BOT_two;//是否有2   0沒有 ,其他為幾個
	int BOT_one;//是否有1   0沒有 ,其他為幾個
	int BOT_K;////是否有K   0沒有 ,其他為幾個
	int BOT_Q;////是否有K   0沒有 ,其他為幾個
	int BOT_J;////是否有K   0沒有 ,其他為幾個
	int BOT_10;////是否有K   0沒有 ,其他為幾個
	int BOT_9;////是否有K   0沒有 ,其他為幾個
	int BOT_8;////是否有K   0沒有 ,其他為幾個
	int BOT_7;////是否有K   0沒有 ,其他為幾個
	int BOT_6;////是否有K   0沒有 ,其他為幾個
	int BOT_5;////是否有K   0沒有 ,其他為幾個
	int BOT_4;////是否有K   0沒有 ,其他為幾個
	int BOT_3;////是否有K   0沒有 ,其他為幾個

			  //三連對

};





  struct ddzbootpxfx_1
  {
	  int BOT_FZPX;  //   

  };


  class BOTProces;
class my_ddzboot
{
public:
	my_ddzboot();
    int GetBYTOINT(CT_BYTE cbCardData[NORMAL_COUNT]);
	bool GetBYTOINTPX(int fxpx[], ddzbootqz spx_fx);
	int  SetNMPX(CT_BYTE cbCardData1[NORMAL_COUNT]); //农民分析，几手可以出去
    int cxpx[NORMAL_COUNT];
	int cxpx1[NORMAL_COUNT];
	BOTProces* GetBotProcess() { return botpro; }
	BOTProces*       botpro;
private :
	ddzbootpxfx_1 ddz_fxl;
	ddzbootqz dxfx1;
};

