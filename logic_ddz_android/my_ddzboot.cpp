#include "stdafx.h"
#include "my_ddzboot.h"
#include "BOTProces.h"

my_ddzboot::my_ddzboot()
{
	botpro=new BOTProces;
	dxfx1 = { 0 };//牌型存储
}

bool my_ddzboot::GetBYTOINTPX(int fxpx[],ddzbootqz spx_fx)
{

	return true;
}


int my_ddzboot::GetBYTOINT(CT_BYTE cbCardData[MAX_COUNT])
{
	dxfx1 = { 0 };//牌型存储
	botpro->ddz_max = { 0 };//大王存储
	botpro->ddz_two = { 0 };//存储2的结构
	botpro->ddz_lian_1 = { 0 };//存储连牌结构l
	botpro->ddz_one = { 0 };//存储1的结构
	botpro->ddz_twolian = { 0 };//存储双连结构
	botpro->ddz_zd = { 0 };//炸弹结构存储
	botpro->ddz_fj = { 0 };//飞机结构存储
	botpro->ddz_three = { 0 };//三连结构存储
	botpro->ddz_san = { 0 };//散牌存储
	botpro->ddz_two_fx = { 0 };//对子存储
	botpro->ddz_old = { 0 }; //最后出牌
	/*
  struct ddzbootqz
{
	CT_DWORD m_cbBombCount;   //炸弹数量
	CT_DWORD BOT_THREE;//三条个数
	int BOT_MISSILE_CARD;//火箭  0为没有，1为有
	int BOT_TmpCount; //飛機 0为没有，1为有
	int BOT_ZG; //大小王 0沒有，1大王，2小王，3都有
	int BOT_two;//是否有2   0沒有 ,其他為幾個
	int BOT_one;//是否有1   0沒有 ,其他為幾個
	int BOT_K;////是否有K   0沒有 ,其他為幾個
	int BOT_Five;////是否有5連,0,沒有， 返回5連數量
	int BOT_six;////是否有6連,0,沒有， 返回6連數量
	int BOT_seven;////是否有7連,0,沒有， 返回7連數量
	int BOT_eigh;////是否有8連,0,沒有， 返回8連數量
	int BOT_nigh;////是否有9連,0,沒有， 返回9連數量
	int BOT_ten;////是否有10連,0,沒有， 返回10連數量
	int BOT_elevn;////是否有11連,0,沒有， 返回11連數量
	int BOT_twot;////是否有12連,0,沒有， 返回12連數量
	int	BOT_SAN;// 三連是否有，沒有為0	
	int	BOT_SAN_1;// 四連是否有，沒有為0
	//三連對

};*/
/*假定火箭为7分，炸弹为5分，大王4分，小王3分，一个2为2分，一个1为1分，三个K=1 三个Q =1 ，3K3Q=飞机3 其他飞机 为 2分 则当分数相加

大于等于9分时100%叫地主，如果没叫到地主，加倍, 如果分值=8分 则加倍，小于8分的不叫；*/


	dxfx1.m_cbBombCount = 0;
	dxfx1.BOT_MISSILE_CARD = 0;


	int fzfx_l=0;

	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (cbCardData[i] == 0x4E)
		{
			fzfx_l += 3;
			dxfx1.BOT_ZG1 = 1;
		//	////////////////printf("【小王】");
		

		}
		if (cbCardData[i] == 0x4F)
		{
			dxfx1.BOT_ZG2 = 1;
		//	////////////////printf("【大王】");
			fzfx_l += 4;
		}
		if (dxfx1.BOT_ZG2==1&& dxfx1.BOT_ZG1==1)
		{
			dxfx1.BOT_MISSILE_CARD = 1;
		
		}
		if (cbCardData[i] == 0x01 || cbCardData[i] == 0x11 || cbCardData[i] == 0x21 || cbCardData[i] == 0x31)
		{
			fzfx_l++;
			dxfx1.BOT_one++;
			if (dxfx1.BOT_one == 4)
			{
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}

		//	////////////////printf("【A】");
		}
		if (cbCardData[i] == 0x02|| cbCardData[i] == 0x12|| cbCardData[i] == 0x22 || cbCardData[i] == 0x32)
		{
			fzfx_l += 2;
			dxfx1.BOT_two++;
			if (dxfx1.BOT_two == 4)
			{
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
		//	////////////////printf("【2】");
		}
	
		if (cbCardData[i] == 0x0D || cbCardData[i] == 0x1D || cbCardData[i] == 0x2D || cbCardData[i] == 0x3D)
		{
		//	////////////////printf("【K】");
			dxfx1.BOT_K++;
			if (dxfx1.BOT_K == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}




		}
		if (cbCardData[i] == 0x0C || cbCardData[i] == 0x1C || cbCardData[i] == 0x2C || cbCardData[i] == 0x3C)
		{
			dxfx1.BOT_Q++;
			if (dxfx1.BOT_Q == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
		//	////////////////printf("【Q】");

		}
		if (cbCardData[i] == 0x0B || cbCardData[i] == 0x1B || cbCardData[i] == 0x2B || cbCardData[i] == 0x3B)
		{
			dxfx1.BOT_J++;
			if (dxfx1.BOT_J == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
		//	////////////////printf("【J】");

		}
		if (cbCardData[i] == 0x0A || cbCardData[i] == 0x1A || cbCardData[i] == 0x2A || cbCardData[i] == 0x3A)
		{
			dxfx1.BOT_10++;
			if (dxfx1.BOT_10 == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
	//		////////////////printf("【10】");

		}
		if (cbCardData[i] == 0x09 || cbCardData[i] == 0x19 || cbCardData[i] == 0x29 || cbCardData[i] == 0x39)
		{
			dxfx1.BOT_9++;
			if (dxfx1.BOT_9 == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
		//	////////////////printf("【9】");


		}
		if (cbCardData[i] == 0x08 || cbCardData[i] == 0x18 || cbCardData[i] == 0x28 || cbCardData[i] == 0x38)
		{
			dxfx1.BOT_8++;
			if (dxfx1.BOT_8 == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
	//		////////////////printf("【8】");

		}
		if (cbCardData[i] == 0x07 || cbCardData[i] == 0x17 || cbCardData[i] == 0x27 || cbCardData[i] == 0x37)
		{
			dxfx1.BOT_7++;
			if (dxfx1.BOT_7 == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
//			////////////////printf("【7】");

		}
		if (cbCardData[i] == 0x06 || cbCardData[i] == 0x16 || cbCardData[i] == 0x26 || cbCardData[i] == 0x36)
		{
			dxfx1.BOT_6++;
			if (dxfx1.BOT_6 == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
	//		////////////////printf("【6】");

		}
		if (cbCardData[i] == 0x05 || cbCardData[i] == 0x15 || cbCardData[i] == 0x25 || cbCardData[i] == 0x35)
		{
			dxfx1.BOT_5++;
			if (dxfx1.BOT_5 == 4)
			{
				fzfx_l += 5;
			dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
//			////////////////printf("【5】");

		}
		if (cbCardData[i] == 0x04 || cbCardData[i] == 0x14 || cbCardData[i] == 0x24 || cbCardData[i] == 0x34)
		{
			dxfx1.BOT_4++;
			if (dxfx1.BOT_4 == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
		//	////////////////printf("【4】");

		}
		if (cbCardData[i] == 0x03 || cbCardData[i] == 0x13 || cbCardData[i] == 0x23 || cbCardData[i] == 0x33)
		{
			dxfx1.BOT_3++;
			if (dxfx1.BOT_3 == 4)
			{
				fzfx_l += 5;
				dxfx1.m_cbBombCount = dxfx1.m_cbBombCount + 1;
			}
		}

	

		if (i == 16)
		{

			if(dxfx1.BOT_K==3)
				fzfx_l += 1;
			if (dxfx1.BOT_Q == 3)
				fzfx_l += 1;

			if (dxfx1.BOT_Q == 3&& dxfx1.BOT_K == 3)
				fzfx_l += 1;

			if (dxfx1.BOT_Q == 3 && dxfx1.BOT_J == 3)
				fzfx_l += 1;

			if (dxfx1.BOT_10 == 3 && dxfx1.BOT_J == 3)
				fzfx_l += 2;
			if (dxfx1.BOT_10 == 3 && dxfx1.BOT_9 == 3)
				fzfx_l += 2;
			if (dxfx1.BOT_8 == 3 && dxfx1.BOT_9 == 3)
				fzfx_l += 2;
			if (dxfx1.BOT_8 == 3 && dxfx1.BOT_7 == 3)
				fzfx_l += 2;
			if (dxfx1.BOT_6 == 3 && dxfx1.BOT_7 == 3)
				fzfx_l += 2;
			if (dxfx1.BOT_6 == 3 && dxfx1.BOT_5 == 3)
				fzfx_l += 2;
			if (dxfx1.BOT_4 == 3 && dxfx1.BOT_5 == 3)
				fzfx_l += 2;
			if (dxfx1.BOT_4 == 3 && dxfx1.BOT_3 == 3)
				fzfx_l += 2;


		}
	}





	ddz_fxl.BOT_FZPX = fzfx_l;
	if (ddz_fxl.BOT_FZPX > 9)
	{

		botpro->bot_king = 2;
		SetNMPX(cbCardData);
	}else if (ddz_fxl.BOT_FZPX== 9)
	{

		botpro->bot_king =3;
		SetNMPX(cbCardData);
	}else if (ddz_fxl.BOT_FZPX >7&& ddz_fxl.BOT_FZPX<9)
	{

		botpro->bot_king = 1;
		SetNMPX(cbCardData);
	}else if (ddz_fxl.BOT_FZPX < 8)
	{
		////////////////printf("不加倍农民\r\n 分析牌型\r\n");
		botpro->bot_king = 0;
		SetNMPX(cbCardData);
	}
	
	return CT_BYTE();
}
int  my_ddzboot::SetNMPX(CT_BYTE cbCardData1[MAX_COUNT])
{

	CT_BYTE  fxpx1[21] = { 0 };
	for (int is = 0; is <botpro->pssj_vh;is++)
	{
		fxpx1[is] = cbCardData1[is];
	}
	if (dxfx1.BOT_MISSILE_CARD == 1)
	{
	//////////////////printf("【大王】【小王】");
botpro->ddz_max.maxpxjs = 3;
		botpro->ddz_max.ps = 2;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x4F || fxpx1[is] == 0x4E)
			{
				if (fxpx1[is] == 0x4F)
				{
					botpro->ddz_max.ddzmax_1 = fxpx1[is];
				}
				else
				{
					botpro->ddz_max.ddzmax_2 = fxpx1[is];
				}
				fxpx1[is] = { 0 };

			}
		}

	}else if (dxfx1.BOT_ZG2 == 1)
	{

		botpro->ddz_max.maxpxjs = 1;
		botpro->ddz_max.ps = 1;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x4F)
			{
				botpro->ddz_max.ddzmax_1 = fxpx1[is];
				fxpx1[is] = { 0 };
			}
		}

	}else if (dxfx1.BOT_ZG1 == 1)
	{

		botpro->ddz_max.maxpxjs = 2;
		botpro->ddz_max.ps = 1;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x4E)
			{
				botpro->ddz_max.ddzmax_2 = fxpx1[is];
				fxpx1[is] = { 0 };
			}
		}


	}

	
	if (dxfx1.BOT_two > 0)
	{
		for (int is = 0; is < dxfx1.BOT_two;is++)
		{
		


		}

		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x02 || fxpx1[is] == 0x12 || fxpx1[is] == 0x22 || fxpx1[is] == 0x32)
			{
				botpro->ddz_two.ddztwo_1[botpro->ddz_two.twopxjs] = 2;
				fxpx1[is] = { 0 };
				botpro->ddz_two.twopxjs++;
				botpro->ddz_two.ps++;
			}
		}


	}

	if (dxfx1.m_cbBombCount > 0)
	{
		if (dxfx1.BOT_one == ZD_MAX)
		{

			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x01 || fxpx1[is] == 0x21 || fxpx1[is] == 0x11 || fxpx1[is] == 0x31)
				{
					fxpx1[is] = { 0 };
					botpro->ddz_one.ddzone_1[botpro->ddz_one.onepxjs] = 1;
					botpro->ddz_one.onepxjs++;
					botpro->ddz_one.ps++;
				}
				dxfx1.BOT_one = 0;
			}
			

		}
	}
	int  apt1 = 3; int  apt2 = 19; int apt3 = 35; int  apt4 = 51;

	if (dxfx1.BOT_3 > 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0 && dxfx1.BOT_K > 0 && dxfx1.BOT_10 < 4 && dxfx1.BOT_J<4 && dxfx1.BOT_Q <4 && dxfx1.BOT_K<4)
	{
		bool abd = false;
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}

		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_7 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			botpro->ddz_lian_1.lianpxjs++;
			dxfx1.BOT_3--; dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--; dxfx1.BOT_K--;
			int itp = 0;
			if (dxfx1.BOT_one == 1)
			{
				itp = ST_DZ;
				for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
				{
					if (fxpx1[iqpt] == 0x01 || fxpx1[iqpt] == 0x11 || fxpx1[iqpt] == 0x21 || fxpx1[iqpt] == 0x31)
					{
						botpro->ddz_lian_1.ddzlian_1[11] = 1;

						botpro->ddz_lian_1.ps++;
						fxpx1[iqpt] = { 0 };
					}
				}
				dxfx1.BOT_one = 0;

			}

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					fxpx1[iqpt] = { 0 };
					botpro->ddz_lian_1.ps++;
					if (apt1 == 13)
					{
						break;
					}

					apt1++;
					apt2++;
					apt3++;
					apt4++;


					iqpt = -1;
				}

			}
			if (itp == 0)
			{
				abd = true;
			}
			else
			{
				botpro->ddz_lian_1.lian_1++;
			}
			//////////////////printf("一手\r\n");
		}
	} // 判断10连

	apt1 = 3;   apt2 = 19;  apt3 = 35;   apt4 = 51;
	if (dxfx1.BOT_3 > 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0 && dxfx1.BOT_10 < 4 && dxfx1.BOT_J<4 && dxfx1.BOT_Q <4 && dxfx1.BOT_K<4)
	{
		bool abd = false;
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}

		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_7 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			botpro->ddz_lian_1.lianpxjs++;
			dxfx1.BOT_3--; dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 12)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}

		}
	}

	apt1 = 4;   apt2 = 20;  apt3 = 36;   apt4 = 52;

	if (dxfx1.BOT_K > 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0 && dxfx1.BOT_10 < 4 && dxfx1.BOT_J<4 && dxfx1.BOT_Q <4 && dxfx1.BOT_K<4)
	{
		bool abd = false;
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
	
		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_7 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			botpro->ddz_lian_1.lianpxjs++;
			int itp = 0;
			int itp1 = 0;
			dxfx1.BOT_K--; dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;
			if (dxfx1.BOT_one == 1)
			{
				itp = ST_DZ;
				for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
				{
					if (fxpx1[iqpt] == 0x01 || fxpx1[iqpt] == 0x11 || fxpx1[iqpt] == 0x21 || fxpx1[iqpt] == 0x31)
					{
						switch (botpro->ddz_lian_1.lianpxjs)
						{
						case 1:
							botpro->ddz_lian_1.ddzlian_1[10] = 1;
							itp1 = 1;
							break;

						case 2:
							botpro->ddz_lian_1.ddzlian_2[10] = 1;
							itp1 = 2;
							break;
						case 3:
							botpro->ddz_lian_1.ddzlian_3[10] = 1;
							itp1 = 3;
							break;
						case 4:
							botpro->ddz_lian_1.ddzlian_4[10] = 1;
							itp1 = 4;
							break;

						}
						botpro->ddz_lian_1.ps++;
						fxpx1[iqpt] = { 0 };
						iqpt = 99;
					}
				}
				dxfx1.BOT_one = 0;


			}

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 13)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
			if (itp == 0)
			{
				abd = true;
			}
			else if (itp == 1)
			{
				if (itp1 == 1)
					botpro->ddz_lian_1.lian_1++;
				if (itp1 == 2)
					botpro->ddz_lian_1.lian_2++;
				if (itp1 == 3)
					botpro->ddz_lian_1.lian_3++;
				if (itp1 == 4)
					botpro->ddz_lian_1.lian_4++;
			}
		}

	}

	if (dxfx1.m_cbBombCount > 0)
	{
		int pssad = 0;
		if (dxfx1.BOT_K == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_K = 0;
			pssad = 13;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
				case 2:
					botpro->ddz_zd.ddzzd_2[0] = pssad;
					botpro->ddz_zd.ddzzd_2[1] = pssad;
					botpro->ddz_zd.ddzzd_2[2] = pssad;
					botpro->ddz_zd.ddzzd_2[3] = pssad;
					break;
				case 3:
					botpro->ddz_zd.ddzzd_3[0] = pssad;
					botpro->ddz_zd.ddzzd_3[1] = pssad;
					botpro->ddz_zd.ddzzd_3[2] = pssad;
					botpro->ddz_zd.ddzzd_3[3] = pssad;
						break;
					case 4:
						botpro->ddz_zd.ddzzd_4[0] = pssad;
						botpro->ddz_zd.ddzzd_4[1] = pssad;
						botpro->ddz_zd.ddzzd_4[2] = pssad;
						botpro->ddz_zd.ddzzd_4[3] = pssad;
						break;
			}


			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x0D || fxpx1[is] == 0x1D || fxpx1[is] == 0x2D || fxpx1[is] == 0x3D)
					fxpx1[is] = { 0 };
			}
	
		}
		if (dxfx1.BOT_Q == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_Q = 0;
			pssad = 12;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}

			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x0C || fxpx1[is] == 0x1C || fxpx1[is] == 0x2C || fxpx1[is] == 0x3C)
					fxpx1[is] = { 0 };
			}
	
		}
		if (dxfx1.BOT_J == ZD_MAX)

		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_J = 0;

			pssad = 11;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x0B || fxpx1[is] == 0x1B || fxpx1[is] == 0x2B || fxpx1[is] == 0x3B)
					fxpx1[is] = { 0 };
			}
	
		}
		if (dxfx1.BOT_10 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_10 = 0;
			pssad = 10;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x0A || fxpx1[is] == 0x1A || fxpx1[is] == 0x2A || fxpx1[is] == 0x3A)
					fxpx1[is] = { 0 };
			}

		}
		if (dxfx1.BOT_9 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_9 = 0;
			pssad = 9;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x09 || fxpx1[is] == 0x19 || fxpx1[is] == 0x29 || fxpx1[is] == 0x39)
					fxpx1[is] = { 0 };
			}


		}
		if (dxfx1.BOT_8 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_8 = 0;
			pssad = 8;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x08 || fxpx1[is] == 0x18|| fxpx1[is] == 0x28 || fxpx1[is] == 0x38)
					fxpx1[is] = { 0 };
			}

		}
		if (dxfx1.BOT_7 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_7 = 0;
			pssad = 7;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x07 || fxpx1[is] == 0x17 || fxpx1[is] == 0x27 || fxpx1[is] == 0x37)
					fxpx1[is] = { 0 };
			}


		}
		if (dxfx1.BOT_6 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_6 = 0;
			pssad = 6;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x06 || fxpx1[is] == 0x16 || fxpx1[is] == 0x26 || fxpx1[is] == 0x36)
					fxpx1[is] = { 0 };
			}

		}
		if (dxfx1.BOT_5 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_5 = 0;
			pssad = 5;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x05 || fxpx1[is] == 0x15 || fxpx1[is] == 0x25|| fxpx1[is] == 0x35)
					fxpx1[is] = { 0 };
			}


		}
		if (dxfx1.BOT_4 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_4 = 0;
			pssad = 4;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}

			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x04 || fxpx1[is] == 0x14 || fxpx1[is] == 0x24 || fxpx1[is] == 0x34)
					fxpx1[is] = { 0 };
			}

		}
		if (dxfx1.BOT_3 == ZD_MAX)
		{
			botpro->ddz_zd.zdpxjs++;
			dxfx1.BOT_3 = 0;
			pssad = 3;
			switch (botpro->ddz_zd.zdpxjs)
			{
			case 1:
				botpro->ddz_zd.ddzzd_1[0] = pssad;
				botpro->ddz_zd.ddzzd_1[1] = pssad;
				botpro->ddz_zd.ddzzd_1[2] = pssad;
				botpro->ddz_zd.ddzzd_1[3] = pssad;
				break;
			case 2:
				botpro->ddz_zd.ddzzd_2[0] = pssad;
				botpro->ddz_zd.ddzzd_2[1] = pssad;
				botpro->ddz_zd.ddzzd_2[2] = pssad;
				botpro->ddz_zd.ddzzd_2[3] = pssad;
				break;
			case 3:
				botpro->ddz_zd.ddzzd_3[0] = pssad;
				botpro->ddz_zd.ddzzd_3[1] = pssad;
				botpro->ddz_zd.ddzzd_3[2] = pssad;
				botpro->ddz_zd.ddzzd_3[3] = pssad;
				break;
			case 4:
				botpro->ddz_zd.ddzzd_4[0] = pssad;
				botpro->ddz_zd.ddzzd_4[1] = pssad;
				botpro->ddz_zd.ddzzd_4[2] = pssad;
				botpro->ddz_zd.ddzzd_4[3] = pssad;
				break;
			}
			for (int is = 0; is <botpro->pssj_vh; is++)
			{
				if (fxpx1[is] == 0x03 || fxpx1[is] == 0x13|| fxpx1[is] == 0x23 || fxpx1[is] == 0x33)
					fxpx1[is] = { 0 };
			}

		}
		botpro->ddz_zd.ps = botpro->ddz_zd.zdpxjs * 4;
	}



	apt1 = 4;   apt2 = 20;  apt3 = 36;   apt4 = 52;

	if (dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0 )
	{
		bool 	abd = false;
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_7 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
	
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;
			botpro->ddz_lian_1.lianpxjs++;

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 12)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}

	apt1 = 5;   apt2 = 21;  apt3 = 37;   apt4 = 53;

	if (dxfx1.BOT_K > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0)
	{
		bool abd = false;
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_7 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_7 == 2 && dxfx1.BOT_8 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_5 != 2 && dxfx1.BOT_6 != 2 && dxfx1.BOT_7 != 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_K--;  dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;
			botpro->ddz_lian_1.lianpxjs++;
		
				for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
				{
					if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
					{
						switch (botpro->ddz_lian_1.lianpxjs)
						{
						case 1:
							botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
							botpro->ddz_lian_1.lian_1++;
							break;

						case 2:
							botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
							botpro->ddz_lian_1.lian_2++;
							break;
						case 3:
							botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
							botpro->ddz_lian_1.lian_3++;
							break;
						case 4:
							botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
							botpro->ddz_lian_1.lian_4++;
							break;

						}
						botpro->ddz_lian_1.ps++;
						fxpx1[iqpt] = { 0 };
						if (apt1 == 13)
						{
							break;
						}
						apt1++;
						apt2++;
						apt3++;
						apt4++;
						iqpt = -1;

					}

				}

		}
	}

	apt1 = 5;   apt2 = 21;  apt3 = 37;   apt4 = 53;

	if ( dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0)
	{
		bool abd = true;
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_7 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;
			botpro->ddz_lian_1.lianpxjs++;

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 12)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;



				}
			}

		}
	}
	apt1 = 6;   apt2 = 22;  apt3 = 38;   apt4 = 54;

	if (dxfx1.BOT_K > 0  && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0)
	{
		bool abd = false;
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_7 == 2 && dxfx1.BOT_8 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_7 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_K--;   dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;
			botpro->ddz_lian_1.lianpxjs++;

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 13)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}

		}

	}

	if (dxfx1.BOT_3 == 3 && dxfx1.BOT_4 == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_5 == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x03 || fxpx1[iqpt] == 0x13 || fxpx1[iqpt] == 0x23 || fxpx1[iqpt] == 0x33|| fxpx1[iqpt] == 0x04 || fxpx1[iqpt] == 0x14 || fxpx1[iqpt] == 0x24 || fxpx1[iqpt] == 0x34|| fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15 || fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
				{
				
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;

				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 3;
				botpro->ddz_fj.ddz_feiji_1[1] = 3;
				botpro->ddz_fj.ddz_feiji_1[2] = 3;
				botpro->ddz_fj.ddz_feiji_1[3] = 4;
				botpro->ddz_fj.ddz_feiji_1[4] = 4;
				botpro->ddz_fj.ddz_feiji_1[5] = 4;
				botpro->ddz_fj.ddz_feiji_1[6] = 5;
				botpro->ddz_fj.ddz_feiji_1[7] = 5;
				botpro->ddz_fj.ddz_feiji_1[8] = 5;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 3;
				botpro->ddz_fj.ddz_feiji_2[1] = 3;
				botpro->ddz_fj.ddz_feiji_2[2] = 3;
				botpro->ddz_fj.ddz_feiji_2[3] = 4;
				botpro->ddz_fj.ddz_feiji_2[4] = 4;
				botpro->ddz_fj.ddz_feiji_2[5] = 4;
				botpro->ddz_fj.ddz_feiji_1[6] = 5;
				botpro->ddz_fj.ddz_feiji_1[7] = 5;
				botpro->ddz_fj.ddz_feiji_1[8] = 5;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 3;
				botpro->ddz_fj.ddz_feiji_3[1] = 3;
				botpro->ddz_fj.ddz_feiji_3[2] = 3;
				botpro->ddz_fj.ddz_feiji_3[3] = 4;
				botpro->ddz_fj.ddz_feiji_3[4] = 4;
				botpro->ddz_fj.ddz_feiji_3[5] = 4;
				botpro->ddz_fj.ddz_feiji_1[6] = 5;
				botpro->ddz_fj.ddz_feiji_1[7] = 5;
				botpro->ddz_fj.ddz_feiji_1[8] = 5;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}
			dxfx1.BOT_3 -= 3;
			dxfx1.BOT_4 -= 3;
			dxfx1.BOT_5-= 3;

		}else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x03 || fxpx1[iqpt] == 0x13 || fxpx1[iqpt] == 0x23 || fxpx1[iqpt] == 0x33 || fxpx1[iqpt] == 0x04 || fxpx1[iqpt] == 0x14 || fxpx1[iqpt] == 0x24 || fxpx1[iqpt] == 0x34)
				{
			
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 3;
				botpro->ddz_fj.ddz_feiji_1[1] = 3;
				botpro->ddz_fj.ddz_feiji_1[2] = 3;
				botpro->ddz_fj.ddz_feiji_1[3] = 4;
				botpro->ddz_fj.ddz_feiji_1[4] = 4;
				botpro->ddz_fj.ddz_feiji_1[5] = 4;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 3;
				botpro->ddz_fj.ddz_feiji_2[1] = 3;
				botpro->ddz_fj.ddz_feiji_2[2] = 3;
				botpro->ddz_fj.ddz_feiji_2[3] = 4;
				botpro->ddz_fj.ddz_feiji_2[4] = 4;
				botpro->ddz_fj.ddz_feiji_2[5] = 4;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 3;
				botpro->ddz_fj.ddz_feiji_3[1] = 3;
				botpro->ddz_fj.ddz_feiji_3[2] = 3;
				botpro->ddz_fj.ddz_feiji_3[3] = 4;
				botpro->ddz_fj.ddz_feiji_3[4] = 4;
				botpro->ddz_fj.ddz_feiji_3[5] = 4;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_3 -= 3;
			dxfx1.BOT_4 -= 3;

		}


	}
	if (dxfx1.BOT_4 == 3 && dxfx1.BOT_5 == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_6 == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36 || fxpx1[iqpt] == 0x04 || fxpx1[iqpt] == 0x14 || fxpx1[iqpt] == 0x24 || fxpx1[iqpt] == 0x34 || fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15 || fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
				{
		
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 4;
				botpro->ddz_fj.ddz_feiji_1[1] = 4;
				botpro->ddz_fj.ddz_feiji_1[2] = 4;
				botpro->ddz_fj.ddz_feiji_1[3] = 5;
				botpro->ddz_fj.ddz_feiji_1[4] = 5;
				botpro->ddz_fj.ddz_feiji_1[5] = 5;
				botpro->ddz_fj.ddz_feiji_1[6] = 6;
				botpro->ddz_fj.ddz_feiji_1[7] = 6;
				botpro->ddz_fj.ddz_feiji_1[8] = 6;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 4;
				botpro->ddz_fj.ddz_feiji_2[1] = 4;
				botpro->ddz_fj.ddz_feiji_2[2] = 4;
				botpro->ddz_fj.ddz_feiji_2[3] = 5;
				botpro->ddz_fj.ddz_feiji_2[4] = 5;
				botpro->ddz_fj.ddz_feiji_2[5] = 5;
				botpro->ddz_fj.ddz_feiji_1[6] = 6;
				botpro->ddz_fj.ddz_feiji_1[7] = 6;
				botpro->ddz_fj.ddz_feiji_1[8] = 6;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 4;
				botpro->ddz_fj.ddz_feiji_3[1] = 4;
				botpro->ddz_fj.ddz_feiji_3[2] = 4;
				botpro->ddz_fj.ddz_feiji_3[3] = 5;
				botpro->ddz_fj.ddz_feiji_3[4] = 5;
				botpro->ddz_fj.ddz_feiji_3[5] = 5;
				botpro->ddz_fj.ddz_feiji_1[6] = 6;
				botpro->ddz_fj.ddz_feiji_1[7] = 6;
				botpro->ddz_fj.ddz_feiji_1[8] = 6;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}
			dxfx1.BOT_6 -= 3;
			dxfx1.BOT_4 -= 3;
			dxfx1.BOT_5 -= 3;

		}
		else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x04 || fxpx1[iqpt] == 0x14 || fxpx1[iqpt] == 0x24 || fxpx1[iqpt] == 0x34 || fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15 || fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
				{
					
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 4;
				botpro->ddz_fj.ddz_feiji_1[1] = 4;
				botpro->ddz_fj.ddz_feiji_1[2] = 4;
				botpro->ddz_fj.ddz_feiji_1[3] = 5;
				botpro->ddz_fj.ddz_feiji_1[4] = 5;
				botpro->ddz_fj.ddz_feiji_1[5] = 5;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 4;
				botpro->ddz_fj.ddz_feiji_2[1] = 4;
				botpro->ddz_fj.ddz_feiji_2[2] = 4;
				botpro->ddz_fj.ddz_feiji_2[3] = 5;
				botpro->ddz_fj.ddz_feiji_2[4] = 5;
				botpro->ddz_fj.ddz_feiji_2[5] = 5;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 4;
				botpro->ddz_fj.ddz_feiji_3[1] = 4;
				botpro->ddz_fj.ddz_feiji_3[2] = 4;
				botpro->ddz_fj.ddz_feiji_3[3] = 5;
				botpro->ddz_fj.ddz_feiji_3[4] = 5;
				botpro->ddz_fj.ddz_feiji_3[5] = 5;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_4 -= 3;
			dxfx1.BOT_5 -= 3;

		}


	}
	if (dxfx1.BOT_5 == 3 && dxfx1.BOT_6 == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_7 == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36 || fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37 || fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15 || fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
				{
				
					botpro->ddz_fj.ps++;
					fxpx1[iqpt] = { 0 };

				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 5;
				botpro->ddz_fj.ddz_feiji_1[1] = 5;
				botpro->ddz_fj.ddz_feiji_1[2] = 5;
				botpro->ddz_fj.ddz_feiji_1[3] = 6;
				botpro->ddz_fj.ddz_feiji_1[4] = 6;
				botpro->ddz_fj.ddz_feiji_1[5] = 6;
				botpro->ddz_fj.ddz_feiji_1[6] = 7;
				botpro->ddz_fj.ddz_feiji_1[7] = 7;
				botpro->ddz_fj.ddz_feiji_1[8] = 7;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 5;
				botpro->ddz_fj.ddz_feiji_2[1] = 5;
				botpro->ddz_fj.ddz_feiji_2[2] = 5;
				botpro->ddz_fj.ddz_feiji_2[3] = 6;
				botpro->ddz_fj.ddz_feiji_2[4] = 6;
				botpro->ddz_fj.ddz_feiji_2[5] = 6;
				botpro->ddz_fj.ddz_feiji_1[6] = 7;
				botpro->ddz_fj.ddz_feiji_1[7] = 7;
				botpro->ddz_fj.ddz_feiji_1[8] = 7;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 5;
				botpro->ddz_fj.ddz_feiji_3[1] = 5;
				botpro->ddz_fj.ddz_feiji_3[2] = 5;
				botpro->ddz_fj.ddz_feiji_3[3] = 6;
				botpro->ddz_fj.ddz_feiji_3[4] = 6;
				botpro->ddz_fj.ddz_feiji_3[5] = 6;
				botpro->ddz_fj.ddz_feiji_1[6] = 7;
				botpro->ddz_fj.ddz_feiji_1[7] = 7;
				botpro->ddz_fj.ddz_feiji_1[8] = 7;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}
			dxfx1.BOT_6 -= 3;
			dxfx1.BOT_7 -= 3;
			dxfx1.BOT_5 -= 3;

		}
		else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36 ||fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15 || fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
				{
			
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 5;
				botpro->ddz_fj.ddz_feiji_1[1] = 5;
				botpro->ddz_fj.ddz_feiji_1[2] = 5;
				botpro->ddz_fj.ddz_feiji_1[3] = 6;
				botpro->ddz_fj.ddz_feiji_1[4] = 6;
				botpro->ddz_fj.ddz_feiji_1[5] = 6;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 5;
				botpro->ddz_fj.ddz_feiji_2[1] = 5;
				botpro->ddz_fj.ddz_feiji_2[2] = 5;
				botpro->ddz_fj.ddz_feiji_2[3] = 6;
				botpro->ddz_fj.ddz_feiji_2[4] = 6;
				botpro->ddz_fj.ddz_feiji_2[5] = 6;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 5;
				botpro->ddz_fj.ddz_feiji_3[1] = 5;
				botpro->ddz_fj.ddz_feiji_3[2] = 5;
				botpro->ddz_fj.ddz_feiji_3[3] = 6;
				botpro->ddz_fj.ddz_feiji_3[4] = 6;
				botpro->ddz_fj.ddz_feiji_3[5] = 6;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_6 -= 3;
			dxfx1.BOT_5 -= 3;

		}


	}
	if (dxfx1.BOT_6 == 3 && dxfx1.BOT_7 == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_8 == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36 || fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37 || fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
				{
			
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 6;
				botpro->ddz_fj.ddz_feiji_1[1] = 6;
				botpro->ddz_fj.ddz_feiji_1[2] = 6;
				botpro->ddz_fj.ddz_feiji_1[3] = 7;
				botpro->ddz_fj.ddz_feiji_1[4] = 7;
				botpro->ddz_fj.ddz_feiji_1[5] = 7;
				botpro->ddz_fj.ddz_feiji_1[6] = 8;
				botpro->ddz_fj.ddz_feiji_1[7] = 8;
				botpro->ddz_fj.ddz_feiji_1[8] = 8;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 6;
				botpro->ddz_fj.ddz_feiji_2[1] = 6;
				botpro->ddz_fj.ddz_feiji_2[2] = 6;
				botpro->ddz_fj.ddz_feiji_2[3] = 7;
				botpro->ddz_fj.ddz_feiji_2[4] = 7;
				botpro->ddz_fj.ddz_feiji_2[5] = 7;
				botpro->ddz_fj.ddz_feiji_1[6] = 8;
				botpro->ddz_fj.ddz_feiji_1[7] = 8;
				botpro->ddz_fj.ddz_feiji_1[8] = 8;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 6;
				botpro->ddz_fj.ddz_feiji_3[1] = 6;
				botpro->ddz_fj.ddz_feiji_3[2] = 6;
				botpro->ddz_fj.ddz_feiji_3[3] = 7;
				botpro->ddz_fj.ddz_feiji_3[4] = 7;
				botpro->ddz_fj.ddz_feiji_3[5] = 7;
				botpro->ddz_fj.ddz_feiji_1[6] = 8;
				botpro->ddz_fj.ddz_feiji_1[7] = 8;
				botpro->ddz_fj.ddz_feiji_1[8] = 8;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}

			dxfx1.BOT_6 -= 3;
			dxfx1.BOT_7 -= 3;
			dxfx1.BOT_8 -= 3;

		}
		else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36 || fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37)
				{
				
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 6;
				botpro->ddz_fj.ddz_feiji_1[1] = 6;
				botpro->ddz_fj.ddz_feiji_1[2] = 6;
				botpro->ddz_fj.ddz_feiji_1[3] = 7;
				botpro->ddz_fj.ddz_feiji_1[4] = 7;
				botpro->ddz_fj.ddz_feiji_1[5] = 7;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 6;
				botpro->ddz_fj.ddz_feiji_2[1] = 6;
				botpro->ddz_fj.ddz_feiji_2[2] = 6;
				botpro->ddz_fj.ddz_feiji_2[3] = 7;
				botpro->ddz_fj.ddz_feiji_2[4] = 7;
				botpro->ddz_fj.ddz_feiji_2[5] = 7;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 6;
				botpro->ddz_fj.ddz_feiji_3[1] = 6;
				botpro->ddz_fj.ddz_feiji_3[2] = 6;
				botpro->ddz_fj.ddz_feiji_3[3] = 7;
				botpro->ddz_fj.ddz_feiji_3[4] = 7;
				botpro->ddz_fj.ddz_feiji_3[5] = 7;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_6 -= 3;
			dxfx1.BOT_7 -= 3;

		}


	}
	if (dxfx1.BOT_7 == 3 && dxfx1.BOT_8 == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_9 == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39 || fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37 || fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
				{
				
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 7;
				botpro->ddz_fj.ddz_feiji_1[1] = 7;
				botpro->ddz_fj.ddz_feiji_1[2] = 7;
				botpro->ddz_fj.ddz_feiji_1[3] = 8;
				botpro->ddz_fj.ddz_feiji_1[4] = 8;
				botpro->ddz_fj.ddz_feiji_1[5] = 8;
				botpro->ddz_fj.ddz_feiji_1[6] = 9;
				botpro->ddz_fj.ddz_feiji_1[7] = 9;
				botpro->ddz_fj.ddz_feiji_1[8] = 9;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 7;
				botpro->ddz_fj.ddz_feiji_2[1] = 7;
				botpro->ddz_fj.ddz_feiji_2[2] = 7;
				botpro->ddz_fj.ddz_feiji_2[3] = 8;
				botpro->ddz_fj.ddz_feiji_2[4] = 8;
				botpro->ddz_fj.ddz_feiji_2[5] = 8;
				botpro->ddz_fj.ddz_feiji_1[6] = 9;
				botpro->ddz_fj.ddz_feiji_1[7] = 9;
				botpro->ddz_fj.ddz_feiji_1[8] = 9;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 7;
				botpro->ddz_fj.ddz_feiji_3[1] = 7;
				botpro->ddz_fj.ddz_feiji_3[2] = 7;
				botpro->ddz_fj.ddz_feiji_3[3] = 8;
				botpro->ddz_fj.ddz_feiji_3[4] = 8;
				botpro->ddz_fj.ddz_feiji_3[5] = 8;
				botpro->ddz_fj.ddz_feiji_1[6] = 9;
				botpro->ddz_fj.ddz_feiji_1[7] = 9;
				botpro->ddz_fj.ddz_feiji_1[8] = 9;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}
			dxfx1.BOT_9 -= 3;
			dxfx1.BOT_7 -= 3;
			dxfx1.BOT_8 -= 3;

		}
		else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if ( fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37 || fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28|| fxpx1[iqpt] == 0x38)
				{
				
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 7;
				botpro->ddz_fj.ddz_feiji_1[1] = 7;
				botpro->ddz_fj.ddz_feiji_1[2] = 7;
				botpro->ddz_fj.ddz_feiji_1[3] = 8;
				botpro->ddz_fj.ddz_feiji_1[4] = 8;
				botpro->ddz_fj.ddz_feiji_1[5] = 8;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 7;
				botpro->ddz_fj.ddz_feiji_2[1] = 7;
				botpro->ddz_fj.ddz_feiji_2[2] = 7;
				botpro->ddz_fj.ddz_feiji_2[3] = 8;
				botpro->ddz_fj.ddz_feiji_2[4] = 8;
				botpro->ddz_fj.ddz_feiji_2[5] = 8;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 7;
				botpro->ddz_fj.ddz_feiji_3[1] = 7;
				botpro->ddz_fj.ddz_feiji_3[2] = 7;
				botpro->ddz_fj.ddz_feiji_3[3] = 8;
				botpro->ddz_fj.ddz_feiji_3[4] = 8;
				botpro->ddz_fj.ddz_feiji_3[5] = 8;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_8 -= 3;
			dxfx1.BOT_7 -= 3;

		}


	}
	if (dxfx1.BOT_8 == 3 && dxfx1.BOT_9 == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_10 == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39 || fxpx1[iqpt] == 0x0A || fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A|| fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
				{
			
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 8;
				botpro->ddz_fj.ddz_feiji_1[1] = 8;
				botpro->ddz_fj.ddz_feiji_1[2] = 8;
				botpro->ddz_fj.ddz_feiji_1[3] = 9;
				botpro->ddz_fj.ddz_feiji_1[4] = 9;
				botpro->ddz_fj.ddz_feiji_1[5] = 9;
				botpro->ddz_fj.ddz_feiji_1[6] = 10;
				botpro->ddz_fj.ddz_feiji_1[7] = 10;
				botpro->ddz_fj.ddz_feiji_1[8] = 10;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 8;
				botpro->ddz_fj.ddz_feiji_2[1] = 8;
				botpro->ddz_fj.ddz_feiji_2[2] = 8;
				botpro->ddz_fj.ddz_feiji_2[3] = 9;
				botpro->ddz_fj.ddz_feiji_2[4] = 9;
				botpro->ddz_fj.ddz_feiji_2[5] = 9;
				botpro->ddz_fj.ddz_feiji_1[6] = 10;
				botpro->ddz_fj.ddz_feiji_1[7] = 10;
				botpro->ddz_fj.ddz_feiji_1[8] = 10;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 8;
				botpro->ddz_fj.ddz_feiji_3[1] = 8;
				botpro->ddz_fj.ddz_feiji_3[2] = 8;
				botpro->ddz_fj.ddz_feiji_3[3] = 9;
				botpro->ddz_fj.ddz_feiji_3[4] = 9;
				botpro->ddz_fj.ddz_feiji_3[5] = 9;
				botpro->ddz_fj.ddz_feiji_1[6] = 10;
				botpro->ddz_fj.ddz_feiji_1[7] = 10;
				botpro->ddz_fj.ddz_feiji_1[8] = 10;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}
			dxfx1.BOT_9 -= 3;
			dxfx1.BOT_10 -= 3;
			dxfx1.BOT_8 -= 3;

		}
		else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39|| fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
				{
		
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 8;
				botpro->ddz_fj.ddz_feiji_1[1] = 8;
				botpro->ddz_fj.ddz_feiji_1[2] = 8;
				botpro->ddz_fj.ddz_feiji_1[3] = 9;
				botpro->ddz_fj.ddz_feiji_1[4] = 9;
				botpro->ddz_fj.ddz_feiji_1[5] = 9;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 8;
				botpro->ddz_fj.ddz_feiji_2[1] = 8;
				botpro->ddz_fj.ddz_feiji_2[2] = 8;
				botpro->ddz_fj.ddz_feiji_2[3] = 9;
				botpro->ddz_fj.ddz_feiji_2[4] = 9;
				botpro->ddz_fj.ddz_feiji_2[5] = 9;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 8;
				botpro->ddz_fj.ddz_feiji_3[1] = 8;
				botpro->ddz_fj.ddz_feiji_3[2] = 8;
				botpro->ddz_fj.ddz_feiji_3[3] = 9;
				botpro->ddz_fj.ddz_feiji_3[4] = 9;
				botpro->ddz_fj.ddz_feiji_3[5] = 9;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_8 -= 3;
			dxfx1.BOT_9 -= 3;

		}


	}
	if (dxfx1.BOT_10 == 3 && dxfx1.BOT_9 == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_J == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39 || fxpx1[iqpt] == 0x0A || fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A || fxpx1[iqpt] == 0x0B || fxpx1[iqpt] == 0x1B || fxpx1[iqpt] == 0x2B || fxpx1[iqpt] == 0x3B)
				{
				
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 9;
				botpro->ddz_fj.ddz_feiji_1[1] = 9;
				botpro->ddz_fj.ddz_feiji_1[2] = 9;
				botpro->ddz_fj.ddz_feiji_1[3] = 10;
				botpro->ddz_fj.ddz_feiji_1[4] = 10;
				botpro->ddz_fj.ddz_feiji_1[5] = 10;
				botpro->ddz_fj.ddz_feiji_1[6] = 11;
				botpro->ddz_fj.ddz_feiji_1[7] = 11;
				botpro->ddz_fj.ddz_feiji_1[8] = 11;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 9;
				botpro->ddz_fj.ddz_feiji_2[1] = 9;
				botpro->ddz_fj.ddz_feiji_2[2] = 9;
				botpro->ddz_fj.ddz_feiji_2[3] = 10;
				botpro->ddz_fj.ddz_feiji_2[4] = 10;
				botpro->ddz_fj.ddz_feiji_2[5] = 10;
				botpro->ddz_fj.ddz_feiji_1[6] = 11;
				botpro->ddz_fj.ddz_feiji_1[7] = 11;
				botpro->ddz_fj.ddz_feiji_1[8] = 11;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 9;
				botpro->ddz_fj.ddz_feiji_3[1] = 9;
				botpro->ddz_fj.ddz_feiji_3[2] = 9;
				botpro->ddz_fj.ddz_feiji_3[3] = 10;
				botpro->ddz_fj.ddz_feiji_3[4] = 10;
				botpro->ddz_fj.ddz_feiji_3[5] = 10;
				botpro->ddz_fj.ddz_feiji_1[6] = 11;
				botpro->ddz_fj.ddz_feiji_1[7] = 11;
				botpro->ddz_fj.ddz_feiji_1[8] = 11;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}
			dxfx1.BOT_9 -= 3;
			dxfx1.BOT_10 -= 3;
			dxfx1.BOT_J -= 3;
	
		}
		else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39 || fxpx1[iqpt] == 0x0A || fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A )
				{
				
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 9;
				botpro->ddz_fj.ddz_feiji_1[1] = 9;
				botpro->ddz_fj.ddz_feiji_1[2] = 9;
				botpro->ddz_fj.ddz_feiji_1[3] = 10;
				botpro->ddz_fj.ddz_feiji_1[4] = 10;
				botpro->ddz_fj.ddz_feiji_1[5] = 10;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 9;
				botpro->ddz_fj.ddz_feiji_2[1] = 9;
				botpro->ddz_fj.ddz_feiji_2[2] = 9;
				botpro->ddz_fj.ddz_feiji_2[3] = 10;
				botpro->ddz_fj.ddz_feiji_2[4] = 10;
				botpro->ddz_fj.ddz_feiji_2[5] = 10;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 9;
				botpro->ddz_fj.ddz_feiji_3[1] = 9;
				botpro->ddz_fj.ddz_feiji_3[2] = 9;
				botpro->ddz_fj.ddz_feiji_3[3] = 10;
				botpro->ddz_fj.ddz_feiji_3[4] = 10;
				botpro->ddz_fj.ddz_feiji_3[5] = 10;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_10 -= 3;
			dxfx1.BOT_9 -= 3;

		}

	}
	if (dxfx1.BOT_10 == 3 && dxfx1.BOT_J == 3)
	{
		botpro->ddz_fj.feiji_js++;
		if (dxfx1.BOT_Q == 3)
		{
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x0C || fxpx1[iqpt] == 0x1C || fxpx1[iqpt] == 0x2C || fxpx1[iqpt] == 0x3C || fxpx1[iqpt] == 0x0A || fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A || fxpx1[iqpt] == 0x0B || fxpx1[iqpt] == 0x1B || fxpx1[iqpt] == 0x2B || fxpx1[iqpt] == 0x3B)
				{
					
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 10;
				botpro->ddz_fj.ddz_feiji_1[1] = 10;
				botpro->ddz_fj.ddz_feiji_1[2] = 10;
				botpro->ddz_fj.ddz_feiji_1[3] = 11;
				botpro->ddz_fj.ddz_feiji_1[4] = 11;
				botpro->ddz_fj.ddz_feiji_1[5] = 11;
				botpro->ddz_fj.ddz_feiji_1[6] = 12;
				botpro->ddz_fj.ddz_feiji_1[7] = 12;
				botpro->ddz_fj.ddz_feiji_1[8] = 12;
				botpro->ddz_fj.feiji_1 += 9;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 10;
				botpro->ddz_fj.ddz_feiji_2[1] = 10;
				botpro->ddz_fj.ddz_feiji_2[2] = 10;
				botpro->ddz_fj.ddz_feiji_2[3] = 11;
				botpro->ddz_fj.ddz_feiji_2[4] = 11;
				botpro->ddz_fj.ddz_feiji_2[5] = 11;
				botpro->ddz_fj.ddz_feiji_1[6] = 12;
				botpro->ddz_fj.ddz_feiji_1[7] = 12;
				botpro->ddz_fj.ddz_feiji_1[8] = 12;
				botpro->ddz_fj.feiji_2 += 9;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 10;
				botpro->ddz_fj.ddz_feiji_3[1] = 10;
				botpro->ddz_fj.ddz_feiji_3[2] = 10;
				botpro->ddz_fj.ddz_feiji_3[3] = 11;
				botpro->ddz_fj.ddz_feiji_3[4] = 11;
				botpro->ddz_fj.ddz_feiji_3[5] = 11;
				botpro->ddz_fj.ddz_feiji_1[6] = 12;
				botpro->ddz_fj.ddz_feiji_1[7] = 12;
				botpro->ddz_fj.ddz_feiji_1[8] = 12;
				botpro->ddz_fj.feiji_3 += 9;
				break;

			}
			dxfx1.BOT_Q -= 3;
			dxfx1.BOT_10 -= 3;
			dxfx1.BOT_J -= 3;

		}
		else {
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if ( fxpx1[iqpt] == 0x0A || fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A || fxpx1[iqpt] == 0x0B || fxpx1[iqpt] == 0x1B || fxpx1[iqpt] == 0x2B || fxpx1[iqpt] == 0x3B)
				{
				
					fxpx1[iqpt] = { 0 };
					botpro->ddz_fj.ps++;
				}
			}
			switch (botpro->ddz_fj.feiji_js)
			{
			case 1:
				botpro->ddz_fj.ddz_feiji_1[0] = 10;
				botpro->ddz_fj.ddz_feiji_1[1] = 10;
				botpro->ddz_fj.ddz_feiji_1[2] = 10;
				botpro->ddz_fj.ddz_feiji_1[3] = 11;
				botpro->ddz_fj.ddz_feiji_1[4] = 11;
				botpro->ddz_fj.ddz_feiji_1[5] = 11;
				botpro->ddz_fj.feiji_1 += 6;
				break;
			case 2:
				botpro->ddz_fj.ddz_feiji_2[0] = 10;
				botpro->ddz_fj.ddz_feiji_2[1] = 10;
				botpro->ddz_fj.ddz_feiji_2[2] = 10;
				botpro->ddz_fj.ddz_feiji_2[3] = 11;
				botpro->ddz_fj.ddz_feiji_2[4] = 11;
				botpro->ddz_fj.ddz_feiji_2[5] = 11;
				botpro->ddz_fj.feiji_2 += 6;
				break;
			case 3:
				botpro->ddz_fj.ddz_feiji_3[0] = 10;
				botpro->ddz_fj.ddz_feiji_3[1] = 10;
				botpro->ddz_fj.ddz_feiji_3[2] = 10;
				botpro->ddz_fj.ddz_feiji_3[3] = 11;
				botpro->ddz_fj.ddz_feiji_3[4] = 11;
				botpro->ddz_fj.ddz_feiji_3[5] = 11;
				botpro->ddz_fj.feiji_3 += 6;
				break;

			}
			dxfx1.BOT_10 -= 3;
			dxfx1.BOT_J -= 3;

		}


	}
	if (dxfx1.BOT_J == 3 && dxfx1.BOT_Q == 3)
	{
		botpro->ddz_fj.feiji_js++;
	
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x0C || fxpx1[iqpt] == 0x1C || fxpx1[iqpt] == 0x2C || fxpx1[iqpt] == 0x3C || fxpx1[iqpt] == 0x0B || fxpx1[iqpt] == 0x1B || fxpx1[iqpt] == 0x2B || fxpx1[iqpt] == 0x3B)
			{
			
				fxpx1[iqpt] = { 0 };
				botpro->ddz_fj.ps++;
			}
		}
		switch (botpro->ddz_fj.feiji_js)
		{
		case 1:
			botpro->ddz_fj.ddz_feiji_1[0] = 11;
			botpro->ddz_fj.ddz_feiji_1[1] = 11;
			botpro->ddz_fj.ddz_feiji_1[2] = 11;
			botpro->ddz_fj.ddz_feiji_1[3] = 12;
			botpro->ddz_fj.ddz_feiji_1[4] = 12;
			botpro->ddz_fj.ddz_feiji_1[5] = 12;
			botpro->ddz_fj.feiji_1+=6;
			break;
		case 2:
			botpro->ddz_fj.ddz_feiji_2[0] = 11;
			botpro->ddz_fj.ddz_feiji_2[1] = 11;
			botpro->ddz_fj.ddz_feiji_2[2] = 11;
			botpro->ddz_fj.ddz_feiji_2[3] = 12;
			botpro->ddz_fj.ddz_feiji_2[4] = 12;
			botpro->ddz_fj.ddz_feiji_2[5] = 12;
			botpro->ddz_fj.feiji_2+=6;
			break;
		case 3:
			botpro->ddz_fj.ddz_feiji_3[0] = 11;
			botpro->ddz_fj.ddz_feiji_3[1] = 11;
			botpro->ddz_fj.ddz_feiji_3[2] = 11;
			botpro->ddz_fj.ddz_feiji_3[3] = 12;
			botpro->ddz_fj.ddz_feiji_3[4] = 12;
			botpro->ddz_fj.ddz_feiji_3[5] = 12;
			botpro->ddz_fj.feiji_3+=6;
			break;

		}
			dxfx1.BOT_Q -= 3;
			dxfx1.BOT_J -= 3;

	}

	apt1 = 3;   apt2 = 19;  apt3 = 35;   apt4 = 51;
	if (dxfx1.BOT_3 > 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_10 < 4 && dxfx1.BOT_J<4 )
	{
		bool abd = false;
		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}

		if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}

		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_8 == 2 && dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			botpro->ddz_lian_1.lianpxjs++;
			dxfx1.BOT_3--; dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;


			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 11)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}

	apt1 = 3;   apt2 = 19;  apt3 = 35;   apt4 = 51;
	if (dxfx1.BOT_3 > 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0  && dxfx1.BOT_10 < 4 && dxfx1.BOT_J<4)
	{
		bool abd = false;
		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
	if (dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}

		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_8 == 2 && dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			botpro->ddz_lian_1.lianpxjs++;
			dxfx1.BOT_3--; dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--;


			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 10)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}

	}
	apt1 = 4;   apt2 = 20;  apt3 = 36;   apt4 = 52;

	if (dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 )
	{
		bool abd = false;
		if (dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_3 == 2)
		{
		 abd = true;
		}
		if (dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
		abd = true;
		}
		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2)
		{
			abd = true;
		}
		if(abd ==false)
		{ 
		botpro->ddz_lian_1.lianpxjs++;
		dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;



		for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 11)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;



			}

		}

		}
	}




	apt1 = 4;   apt2 = 20;  apt3 = 36;   apt4 = 52;

	if (dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_5 != 3 && dxfx1.BOT_4 != 3 && dxfx1.BOT_9 <4 && dxfx1.BOT_10<4)
	{
		bool abd = false;
		if (dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_3 == 2 )
		{
			abd = true;
		}
		if (dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--;

			botpro->ddz_lian_1.lianpxjs++;
			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 10)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}

		
}
	apt1 = 5;   apt2 = 21;  apt3 = 37;   apt4 = 53;

	if (dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0&& dxfx1.BOT_5 != 3 && dxfx1.BOT_6 !=30 && dxfx1.BOT_10 != 3 && dxfx1.BOT_J != 3)
	{
		bool abd = false;
		if (dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_Q ==2 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			botpro->ddz_lian_1.lianpxjs++;
for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 11)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}
	apt1 = 6;   apt2 = 22;  apt3 = 38;   apt4 = 54;

	if (dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0&&dxfx1.BOT_6 != 3 && dxfx1.BOT_7 != 3 && dxfx1.BOT_10 != 3 && dxfx1.BOT_J != 3)
	{
		bool abd = false;

		if (dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2 && dxfx1.BOT_7 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_5 == 2 && dxfx1.BOT_6 == 2 && dxfx1.BOT_4 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_J == 2 && dxfx1.BOT_Q == 2 && dxfx1.BOT_K == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			botpro->ddz_lian_1.lianpxjs++;
			dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;
			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 12)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}
	apt1 = 7;   apt2 = 23;  apt3 = 39;   apt4 = 55;

	if (dxfx1.BOT_K> 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0&& dxfx1.BOT_7 != 3 && dxfx1.BOT_8 != 30 && dxfx1.BOT_Q != 3 && dxfx1.BOT_K != 3)
	{
		bool abd = false;
		if (dxfx1.BOT_8 == 2 && dxfx1.BOT_6 == 2 && dxfx1.BOT_7 == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--;
			dxfx1.BOT_Q--;
			botpro->ddz_lian_1.lianpxjs++;

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 13)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}


	if (dxfx1.BOT_3 >1 && dxfx1.BOT_4>1 && dxfx1.BOT_5 >1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_6 == 2)
		{
			dxfx1.BOT_6 -= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36)
				{
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 6;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 6;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					fxpx1[iqpt] = { 0 };
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}
		}
		dxfx1.BOT_3 -= 2;
		dxfx1.BOT_4 -= 2;
		dxfx1.BOT_5 -= 2;
		int bot1;
		bot1= 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x03 || fxpx1[iqpt] == 0x13 || fxpx1[iqpt] == 0x23 || fxpx1[iqpt] == 0x33)
			{
				fxpx1[iqpt] = { 0 };
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 3;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 3;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x04 || fxpx1[iqpt] == 0x14 || fxpx1[iqpt] == 0x24 || fxpx1[iqpt] == 0x34)
			{
				fxpx1[iqpt] = { 0 };
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 4;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 4;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15|| fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
			{
				fxpx1[iqpt] = { 0 };
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 5;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 5;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}

	}
//----------------------------

	if (dxfx1.BOT_6 >1 && dxfx1.BOT_4>1 && dxfx1.BOT_5 >1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_7 == 2)
		{
			dxfx1.BOT_7 -= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37)
				{
					fxpx1[iqpt] = { 0 };
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 7;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 7;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}

		}
		dxfx1.BOT_6 -= 2;
		dxfx1.BOT_4 -= 2;
		dxfx1.BOT_5 -= 2;
		int bot1;
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 6;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 6;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x04 || fxpx1[iqpt] == 0x14 || fxpx1[iqpt] == 0x24 || fxpx1[iqpt] == 0x34)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 4;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 4;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15 || fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] =5;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 5;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}

	}
	if (dxfx1.BOT_6 >1 && dxfx1.BOT_7>1 && dxfx1.BOT_5 >1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_8 == 2)
		{
			dxfx1.BOT_8 -= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
				{
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 8;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 8;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					fxpx1[iqpt] = { 0 };
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}

		}
		dxfx1.BOT_6 -= 2;
		dxfx1.BOT_7 -= 2;
		dxfx1.BOT_5 -= 2;
		int bot1;
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 6;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 6;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 7;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 7;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x05 || fxpx1[iqpt] == 0x15 || fxpx1[iqpt] == 0x25 || fxpx1[iqpt] == 0x35)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] =5;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 5;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}

	}
	if (dxfx1.BOT_6 >1 && dxfx1.BOT_7>1 && dxfx1.BOT_8 >1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_9 == 2)
		{
			dxfx1.BOT_9 -= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39)
				{
					fxpx1[iqpt] = { 0 };
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 9;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 9;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}
	}
		dxfx1.BOT_6 -= 2;
		dxfx1.BOT_7 -= 2;
		dxfx1.BOT_8 -= 2;
		int bot1;
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x06 || fxpx1[iqpt] == 0x16 || fxpx1[iqpt] == 0x26 || fxpx1[iqpt] == 0x36)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 6;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 6;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 7;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 7;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 8;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 8;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}

	}
	if (dxfx1.BOT_9 >1 && dxfx1.BOT_7>1 && dxfx1.BOT_8 >1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_10 == 2)
		{
			dxfx1.BOT_10-= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x0A|| fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A)
				{
					fxpx1[iqpt] = { 0 };
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 10;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 10;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}

	}
		dxfx1.BOT_9 -= 2;
		dxfx1.BOT_7 -= 2;
		dxfx1.BOT_8 -= 2;
		int bot1;
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 9;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] =9;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x07 || fxpx1[iqpt] == 0x17 || fxpx1[iqpt] == 0x27 || fxpx1[iqpt] == 0x37)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 7;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 7;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 8;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 8;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
	}
	if (dxfx1.BOT_9 >1 && dxfx1.BOT_10>1 && dxfx1.BOT_8 >1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_J == 2)
		{
			dxfx1.BOT_J -= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x0B || fxpx1[iqpt] == 0x1B || fxpx1[iqpt] == 0x2B || fxpx1[iqpt] == 0x3B)
				{
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 11;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 11;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					fxpx1[iqpt] = { 0 };
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}
	}
		dxfx1.BOT_9 -= 2;
		dxfx1.BOT_10 -= 2;
		dxfx1.BOT_8 -= 2;
		int bot1;
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 9;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 9;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x0A|| fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 10;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 10;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x08 || fxpx1[iqpt] == 0x18 || fxpx1[iqpt] == 0x28 || fxpx1[iqpt] == 0x38)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 8;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 8;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}

	}
	if (dxfx1.BOT_9 >1 && dxfx1.BOT_10>1 && dxfx1.BOT_J>1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_Q==2)
		{
			dxfx1.BOT_Q -= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x0C || fxpx1[iqpt] == 0x1C || fxpx1[iqpt] == 0x2C || fxpx1[iqpt] == 0x3C)
				{
				
					botpro->ddz_twolian.ps++;
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 12;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 12;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					fxpx1[iqpt] = { 0 };
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}


		}
		dxfx1.BOT_9 -= 2;
		dxfx1.BOT_10 -= 2;
		dxfx1.BOT_J -= 2;
		int bot1;
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x09 || fxpx1[iqpt] == 0x19 || fxpx1[iqpt] == 0x29 || fxpx1[iqpt] == 0x39)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 9;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 9;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x0A || fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 10;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 10;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x0B || fxpx1[iqpt] == 0x1B || fxpx1[iqpt] == 0x2B || fxpx1[iqpt] == 0x3B)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 11;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 11;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}

	}
	if (dxfx1.BOT_Q >1 && dxfx1.BOT_10>1 && dxfx1.BOT_J>1)
	{
		botpro->ddz_twolian.lianpxjs++;
		if (dxfx1.BOT_K == 2)
		{
			dxfx1.BOT_K -= 2;
			int bot1;
			bot1 = 1;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x0D || fxpx1[iqpt] == 0x1D || fxpx1[iqpt] == 0x2D || fxpx1[iqpt] == 0x3D)
				{
					if (botpro->ddz_twolian.lianpxjs == 1)
					{
						botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 13;
						botpro->ddz_twolian.lian_1++;
					}
					else if (botpro->ddz_twolian.lianpxjs == 2)
					{
						botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 13;
						botpro->ddz_twolian.lian_2++;

					}
					botpro->ddz_twolian.ps++;
					fxpx1[iqpt] = { 0 };
					if (bot1 == 2)
					{
						break;
					}
					bot1++;
					iqpt = -1;
				}
			}
		}
		dxfx1.BOT_Q-= 2;
		dxfx1.BOT_10 -= 2;
		dxfx1.BOT_J -= 2;
		int bot1;
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x0C|| fxpx1[iqpt] == 0x1C || fxpx1[iqpt] == 0x2C || fxpx1[iqpt] == 0x3C)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 12;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 12;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x0A || fxpx1[iqpt] == 0x1A || fxpx1[iqpt] == 0x2A || fxpx1[iqpt] == 0x3A)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 10;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 10;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}
		bot1 = 1;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == 0x0B || fxpx1[iqpt] == 0x1B || fxpx1[iqpt] == 0x2B || fxpx1[iqpt] == 0x3B)
			{
				if (botpro->ddz_twolian.lianpxjs == 1)
				{
					botpro->ddz_twolian.ddztwo_lian_1[botpro->ddz_twolian.lian_1] = 11;
					botpro->ddz_twolian.lian_1++;
				}
				else if (botpro->ddz_twolian.lianpxjs == 2)
				{
					botpro->ddz_twolian.ddztwo_lian_2[botpro->ddz_twolian.lian_2] = 11;
					botpro->ddz_twolian.lian_2++;

				}
				botpro->ddz_twolian.ps++;
				fxpx1[iqpt] = { 0 };
				if (bot1 == 2)
				{
					break;
				}
				bot1++;
				iqpt = -1;
			}
		}

	}
	apt1 = 3;   apt2 = 19;  apt3 = 35;   apt4 = 51;

	if (dxfx1.BOT_3> 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0&&dxfx1.BOT_3!=3 &&dxfx1.BOT_8 != 3)
	{
		bool 		abd = false;
		if (dxfx1.BOT_7 == 3 && dxfx1.BOT_8 == 2 && dxfx1.BOT_9 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_8 == 2 && dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_5--; dxfx1.BOT_4--; dxfx1.BOT_3--;

			botpro->ddz_lian_1.lianpxjs++;

			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 8)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}

	apt1 = 4;   apt2 = 20;  apt3 = 36;   apt4 = 52;

	if (dxfx1.BOT_9> 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_4 != 3 && dxfx1.BOT_9 != 3)
	{
		bool abd = false;
		if (dxfx1.BOT_7 == 3 && dxfx1.BOT_8 == 2 && dxfx1.BOT_9 == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_8 == 2 && dxfx1.BOT_9 == 2 && dxfx1.BOT_10 == 2)
		{
			abd = true;
		}
		if (abd == false)
		{
			dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_5--; dxfx1.BOT_4--; dxfx1.BOT_9--;
			botpro->ddz_lian_1.lianpxjs++;
			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 9)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}

	apt1 = 5;   apt2 = 21;  apt3 = 37;   apt4 = 53;

	if (dxfx1.BOT_9> 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_5 != 3 && dxfx1.BOT_10 != 3)
	{
       bool 		abd = false;
		if (dxfx1.BOT_9 == 3 && dxfx1.BOT_10 == 2 && dxfx1.BOT_J == 2)
		{
			abd = true;
		}
		if (dxfx1.BOT_3 == 2 && dxfx1.BOT_4 == 2 && dxfx1.BOT_5 == 2)
		{
			abd = true;
		}
		dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_5--; dxfx1.BOT_9--; dxfx1.BOT_10--;
		botpro->ddz_lian_1.lianpxjs++;
		if (abd == false)
		{
			for (int iqpt = 0; iqpt < botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					if (apt1 == 10)
					{
						break;
					}
					apt1++;
					apt2++;
					apt3++;
					apt4++;
					iqpt = -1;

				}

			}
		}
	}
	apt1 = 6;   apt2 = 22;  apt3 = 38;   apt4 = 54;

	if (dxfx1.BOT_9> 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_6 != 3 && dxfx1.BOT_J != 3)
	{
		dxfx1.BOT_6--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_J--; dxfx1.BOT_9--; dxfx1.BOT_10--;

		botpro->ddz_lian_1.lianpxjs++;

		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 11)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}

	}
	apt1 = 7;   apt2 = 23;  apt3 = 39;   apt4 = 55;

	if (dxfx1.BOT_9> 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0 && dxfx1.BOT_7 > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_7 != 3 && dxfx1.BOT_Q != 3)
	{
		dxfx1.BOT_Q--; dxfx1.BOT_7--; dxfx1.BOT_8--; dxfx1.BOT_J--; dxfx1.BOT_9--; dxfx1.BOT_10--;

		botpro->ddz_lian_1.lianpxjs++;

		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 12)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}

	}
	apt1 = 8;   apt2 = 24;  apt3 = 40;   apt4 = 56;

	if (dxfx1.BOT_9> 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0 && dxfx1.BOT_K > 0 && dxfx1.BOT_8 > 0 && dxfx1.BOT_8 != 3 && dxfx1.BOT_K != 3)
	{
		dxfx1.BOT_Q--; dxfx1.BOT_K--; dxfx1.BOT_8--; dxfx1.BOT_J--; dxfx1.BOT_9--; dxfx1.BOT_10--;

		botpro->ddz_lian_1.lianpxjs++;

		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 13)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}


	}

	apt1 = 9;   apt2 = 25;  apt3 = 41;   apt4 = 57;

	if (dxfx1.BOT_9> 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0 && dxfx1.BOT_K > 0 && dxfx1.BOT_one > 0 && dxfx1.BOT_9 != 3 )
	{
		dxfx1.BOT_Q--; dxfx1.BOT_K--;  dxfx1.BOT_J--; dxfx1.BOT_9--; dxfx1.BOT_10--;
		botpro->ddz_lian_1.lianpxjs++;
		int itp = 0;
		int itp1 = 0;
		if (dxfx1.BOT_one == 1)
		{
			itp = ST_DZ;
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x01 || fxpx1[iqpt] == 0x11 || fxpx1[iqpt] == 0x21 || fxpx1[iqpt] == 0x31)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[5] = 1;
						itp1 = 1;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[5] = 1;
						itp1 = 2;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[5] = 1;
						itp1 = 3;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[5] = 1;
						itp1 = 4;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
				}
			}
		dxfx1.BOT_one = 0;


		}

		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 13)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}
		if (itp == 0)
		{
			int itp = 0;
			itp++;

		}
		else if(itp == 1)
		{
			if (itp1 == 1)
				botpro->ddz_lian_1.lian_1++;
			if (itp1 == 2)
				botpro->ddz_lian_1.lian_2++;
			if (itp1 == 3)
				botpro->ddz_lian_1.lian_3++;
			if (itp1 == 4)
				botpro->ddz_lian_1.lian_4++;
		}

	}

	apt1 = 3;   apt2 = 19;  apt3 = 35;   apt4 = 51;

	if (dxfx1.BOT_3> 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0)
	{
		dxfx1.BOT_3--; dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--; 
		botpro->ddz_lian_1.lianpxjs++;
	for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 7)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}

	}

	apt1 = 4;   apt2 = 20;  apt3 = 36;   apt4 = 52;

	if (dxfx1.BOT_8> 0 && dxfx1.BOT_4 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0)
	{
		dxfx1.BOT_8--; dxfx1.BOT_4--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--;
		botpro->ddz_lian_1.lianpxjs++;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 8)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}

	}
	apt1 = 5;   apt2 = 21;  apt3 = 37;   apt4 = 53;

	if (dxfx1.BOT_8> 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_5 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0)
	{
		botpro->ddz_lian_1.lianpxjs++;
		dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_5--; dxfx1.BOT_6--; dxfx1.BOT_7--;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				fxpx1[iqpt] = { 0 };
				botpro->ddz_lian_1.ps++;
				if (apt1 == 9)
				{
				
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}

	}
	apt1 = 6;   apt2 = 22;  apt3 = 38;   apt4 = 54;

	if (dxfx1.BOT_8> 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_6 > 0 && dxfx1.BOT_7 > 0)
	{
		dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_6--; dxfx1.BOT_7--;
		botpro->ddz_lian_1.lianpxjs++;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 10)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}


	}
	apt1 = 7;   apt2 = 23;  apt3 = 39;   apt4 = 55;

	if (dxfx1.BOT_8> 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_7 > 0)
	{
		dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--; dxfx1.BOT_7--;
		botpro->ddz_lian_1.lianpxjs++;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 11)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}


	}
	apt1 = 8;   apt2 = 24;  apt3 = 40;   apt4 = 56;

	if (dxfx1.BOT_8> 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0)
	{
		dxfx1.BOT_8--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--; dxfx1.BOT_Q--;
		botpro->ddz_lian_1.lianpxjs++;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 12)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}

	}
	apt1 = 9;   apt2 = 25;  apt3 = 41;   apt4 = 57;

	if (dxfx1.BOT_K> 0 && dxfx1.BOT_9 > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0)
	{
		dxfx1.BOT_K--; dxfx1.BOT_9--; dxfx1.BOT_10--; dxfx1.BOT_J--; dxfx1.BOT_Q--;
		botpro->ddz_lian_1.lianpxjs++;
		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{
				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 13)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}

	}
	apt1 = 10;   apt2 = 26;  apt3 = 42;   apt4 = 58;

	if (dxfx1.BOT_K> 0 && dxfx1.BOT_one > 0 && dxfx1.BOT_10 > 0 && dxfx1.BOT_J > 0 && dxfx1.BOT_Q > 0)
	{
		dxfx1.BOT_K--; dxfx1.BOT_one--;  dxfx1.BOT_10--; dxfx1.BOT_J--;
		dxfx1.BOT_Q--;
		botpro->ddz_lian_1.lianpxjs++;

		for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
		{
			if (fxpx1[iqpt] == apt1 || fxpx1[iqpt] == apt2 || fxpx1[iqpt] == apt3 || fxpx1[iqpt] == apt4)
			{

				switch (botpro->ddz_lian_1.lianpxjs)
				{
				case 1:
					botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] = apt1;
					botpro->ddz_lian_1.lian_1++;
					break;

				case 2:
					botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = apt1;
					botpro->ddz_lian_1.lian_2++;
					break;
				case 3:
					botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = apt1;
					botpro->ddz_lian_1.lian_3++;
					break;
				case 4:
					botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = apt1;
					botpro->ddz_lian_1.lian_4++;
					break;

				}
				botpro->ddz_lian_1.ps++;
				fxpx1[iqpt] = { 0 };
				if (apt1 == 13)
				{
					break;
				}
				apt1++;
				apt2++;
				apt3++;
				apt4++;
				iqpt = -1;

			}

		}
			for (int iqpt = 0; iqpt <botpro->pssj_vh; iqpt++)
			{
				if (fxpx1[iqpt] == 0x01 || fxpx1[iqpt] == 0x11 || fxpx1[iqpt] == 0x21 || fxpx1[iqpt] == 0x31)
				{
					switch (botpro->ddz_lian_1.lianpxjs)
					{
					case 1:
						botpro->ddz_lian_1.ddzlian_1[botpro->ddz_lian_1.lian_1] =1;
						botpro->ddz_lian_1.lian_1++;
						break;

					case 2:
						botpro->ddz_lian_1.ddzlian_2[botpro->ddz_lian_1.lian_2] = 1;
						botpro->ddz_lian_1.lian_2++;
						break;
					case 3:
						botpro->ddz_lian_1.ddzlian_3[botpro->ddz_lian_1.lian_3] = 1;
						botpro->ddz_lian_1.lian_3++;
						break;
					case 4:
						botpro->ddz_lian_1.ddzlian_4[botpro->ddz_lian_1.lian_4] = 1;
						botpro->ddz_lian_1.lian_4++;
						break;

					}
					botpro->ddz_lian_1.ps++;
					fxpx1[iqpt] = { 0 };
					iqpt=99;
				}
			}
		
	}

	int ppassde = 0;
	if (dxfx1.BOT_3 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 3;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x03 || fxpx1[is] == 0x13 || fxpx1[is] == 0x23 || fxpx1[is] == 0x33)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}

	}
	if (dxfx1.BOT_4 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 4;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x04 || fxpx1[is] == 0x14 || fxpx1[is] == 0x24 || fxpx1[is] == 0x34)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}

	}
	if (dxfx1.BOT_5 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 5;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x05 || fxpx1[is] == 0x15 || fxpx1[is] == 0x25 || fxpx1[is] == 0x35)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}

	}
	if (dxfx1.BOT_6 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 6;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x06 || fxpx1[is] == 0x16 || fxpx1[is] == 0x26 || fxpx1[is] == 0x36)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}

	}
	if (dxfx1.BOT_7 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 7;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x07 || fxpx1[is] == 0x17 || fxpx1[is] == 0x27 || fxpx1[is] == 0x37)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}


	}
	if (dxfx1.BOT_8 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 8;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x08 || fxpx1[is] == 0x18 || fxpx1[is] == 0x28 || fxpx1[is] == 0x38)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}

	}
	if (dxfx1.BOT_9 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 9;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x09 || fxpx1[is] == 0x19 || fxpx1[is] == 0x29 || fxpx1[is] == 0x39)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}

	}
	if (dxfx1.BOT_10 == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 10;

		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x0A || fxpx1[is] == 0x1A || fxpx1[is] == 0x2A || fxpx1[is] == 0x3A)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}
	}
	if (dxfx1.BOT_J == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 11;

		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x0B || fxpx1[is] == 0x1B || fxpx1[is] == 0x2B || fxpx1[is] == 0x3B)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}
	}
	if (dxfx1.BOT_Q == ST_THREE)
	{
		botpro->ddz_three.ps += 3;
		botpro->ddz_three.three_js++;
		ppassde = 12;

		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x0C || fxpx1[is] == 0x1C || fxpx1[is] == 0x2C || fxpx1[is] == 0x3C)
				fxpx1[is] = { 0 };
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}
	}
	if (dxfx1.BOT_K == ST_THREE)
	{
		botpro->ddz_three.three_js++;
		botpro->ddz_three.ps += 3;
		ppassde = 13;
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x0D || fxpx1[is] == 0x1D || fxpx1[is] == 0x2D || fxpx1[is] == 0x3D)
			{
			
				fxpx1[is] = { 0 };
			}
		}
		switch (botpro->ddz_three.three_js)
		{
		case 1:
			botpro->ddz_three.ddz_three_1[0] = ppassde;
			botpro->ddz_three.ddz_three_1[1] = ppassde;
			botpro->ddz_three.ddz_three_1[2] = ppassde;
			break;

		case 2:
			botpro->ddz_three.ddz_three_2[0] = ppassde;
			botpro->ddz_three.ddz_three_2[1] = ppassde;
			botpro->ddz_three.ddz_three_2[2] = ppassde;
			break;
		case 3:
			botpro->ddz_three.ddz_three_3[0] = ppassde;
			botpro->ddz_three.ddz_three_3[1] = ppassde;
			botpro->ddz_three.ddz_three_3[2] = ppassde;
			break;
		case 4:
			botpro->ddz_three.ddz_three_4[0] = ppassde;
			botpro->ddz_three.ddz_three_4[1] = ppassde;
			botpro->ddz_three.ddz_three_4[2] = ppassde;
			break;
		case 5:
			botpro->ddz_three.ddz_three_5[0] = ppassde;
			botpro->ddz_three.ddz_three_5[1] = ppassde;
			botpro->ddz_three.ddz_three_5[2] = ppassde;
			break;
		}

	
	}









	if (dxfx1.BOT_one > 0)
	{
	
		for (int is = 0; is < dxfx1.BOT_one; is++)
		{
			botpro->ddz_one.ddzone_1[botpro->ddz_one.onepxjs] = 1;
			botpro->ddz_one.onepxjs++;
			botpro->ddz_one.ps++;

		}
		for (int is = 0; is <botpro->pssj_vh; is++)
		{
			if (fxpx1[is] == 0x01 || fxpx1[is] == 0x11 || fxpx1[is] == 0x21 || fxpx1[is] == 0x31)
				fxpx1[is] = { 0 };
		}


	}

//	--------------------------
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x03 || fxpx1[i] == 0x13 || fxpx1[i] == 0x23 || fxpx1[i] == 0x33)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 3;
			botpro->ddz_san.ps++;


		}
}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x04 || fxpx1[i] == 0x14 || fxpx1[i] == 0x24 || fxpx1[i] == 0x34)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 4;
			botpro->ddz_san.ps++;

		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x05 || fxpx1[i] == 0x15 || fxpx1[i] == 0x25 || fxpx1[i] == 0x35)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 5;
			botpro->ddz_san.ps++;


		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x06 || fxpx1[i] == 0x16 || fxpx1[i] == 0x26 || fxpx1[i] == 0x36)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 6;
			botpro->ddz_san.ps++;


		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x07 || fxpx1[i] == 0x17 || fxpx1[i] == 0x27 || fxpx1[i] == 0x37)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] =7;
			botpro->ddz_san.ps++;


		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x08 || fxpx1[i] == 0x18 || fxpx1[i] == 0x28 || fxpx1[i] == 0x38)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 8;
			botpro->ddz_san.ps++;

		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x09 || fxpx1[i] == 0x19 || fxpx1[i] == 0x29 || fxpx1[i] == 0x39)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 9;
			botpro->ddz_san.ps++;
	}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x0A || fxpx1[i] == 0x1A || fxpx1[i] == 0x2A || fxpx1[i] == 0x3A)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 10;
			botpro->ddz_san.ps++;
		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x0B || fxpx1[i] == 0x1B || fxpx1[i] == 0x2B || fxpx1[i] == 0x3B)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 11;
			botpro->ddz_san.ps++;

		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x0C || fxpx1[i] == 0x1C || fxpx1[i] == 0x2C || fxpx1[i] == 0x3C)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] =12;
			botpro->ddz_san.ps++;

		}
	}
	for (int i = 0; i <botpro->pssj_vh; i++)
	{
		if (fxpx1[i] == 0x0D || fxpx1[i] == 0x1D || fxpx1[i] == 0x2D || fxpx1[i] == 0x3D)
		{
			botpro->ddz_san.ddz_san_1[botpro->ddz_san.ps] = 13;
			botpro->ddz_san.ps++;


		}
	}
	////////////////printf("\r\n");
	botpro->SetPX_CZ(0);

	if (botpro->ddz_lian_1.lianpxjs > 0)
	{
	

		if (botpro->ddz_lian_1.lian_1 > 5 || botpro->ddz_lian_1.lian_2 > 5 || botpro->ddz_lian_1.lian_3 > 5 || botpro->ddz_lian_1.lian_4 > 5)
		{
			bool hscz = false;
			hscz=botpro->GetCXpxPDSX();
			if (hscz == true)
			{
			////////////////printf("\r\n连顺需要重组结果\r\n");
			
				botpro->SetPX_CZ(0);
			}
		}

	}
	botpro->SetSan_CZ();


	return 0;
}