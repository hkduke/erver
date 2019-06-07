#include "stdafx.h"
#include "BOTProces.h"
BOTProces::BOTProces()
{
	GetCodePX = new (CGameLogic);
 SPFX = { 0 }; //手牌分析
 JD_JL = { 0 };//绝对牌型分析
 qz = { 0 };
 onCard = {0};
 DownCard={0};
ddz_max = { 0 };//大王存储
ddz_two = { 0 };//存储2的结构
ddz_lian_1 = { 0 };//存储连牌结构l
ddz_one = { 0 };//存储1的结构
ddz_twolian = { 0 };//存储双连结构
ddz_zd = { 0 };//炸弹结构存储
ddz_fj = { 0 };//飞机结构存储
ddz_three = { 0 };//三连结构存储
ddz_san = { 0 };//散牌存储
ddz_two_fx = { 0 };//对子存储
ddz_old = { 0 }; //最后出牌
};


BOTProces::~BOTProces()
{
}


CT_BOOL BOTProces::GetCXpxPDSX() //散牌 是否还能组成连顺
{
	int bfjs;
	bfjs = ddz_san.ps;
	       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
	CT_BYTE bfjs1[25] = { 0 };

	memcpy(bfjs1, ddz_san.ddz_san_1, sizeof(ddz_san.ddz_san_1));
	int size = 0;
	int seze1 = 0;
	int si_lian[20] = { 0 };
	for (int is = 0; is < ddz_lian_1.lian_1; is++)
	{

		if (ddz_lian_1.ddzlian_1[is] == ddz_san.ddz_san_1[size])
		{

			if (si_lian[seze1 - 1] != ddz_lian_1.ddzlian_1[is])
			{
				if (is > 1)
				{
					si_lian[seze1] = ddz_lian_1.ddzlian_1[is];

					seze1++;
				}
			}


		}
		if (is == ddz_lian_1.lian_1 - 1)
		{
			is = -1;
			size++;
		}
		if (size == ddz_lian_1.lian_1 - 1)
		{
			break;
		}

	}
	int iptsi = 0;
	int psit = 0;
	int cfsl[10] = { 0 };
	if (seze1 > 0)
	{
		for (int is = 0; is < seze1; is++)
		{
			if (si_lian[iptsi] == si_lian[is + 1] - 1)
			{
				cfsl[psit] = si_lian[iptsi];
		
				for (int ipt = 0; ipt < ddz_san.ps; ipt++)
				{
					if (ddz_san.ddz_san_1[ipt] == cfsl[psit])
					{
						ddz_san.ddz_san_1[ipt] = { 0 };
						ddz_san.ddz_san_1[ipt] = ddz_san.ddz_san_1[ipt + 1];
						ddz_san.ddz_san_1[ipt + 1] = 0;
						break;

					}
				}
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				psit++;
			}
			if (is == seze1 - 1)
			{
				int aqastt = 0;
				if (iptsi > seze1)
				{
					cfsl[psit] = cfsl[psit - 1] + 1;
					for (int ipt = 0; ipt < ddz_san.ps; ipt++)
					{
						if (ddz_san.ddz_san_1[ipt] == cfsl[psit])
						{
							ddz_san.ddz_san_1[ipt] = { 0 };


						}
					}
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int ipt = 0; ipt < ddz_san.ps; ipt++)
					{
						if (ddz_san.ddz_san_1[ipt] != 0)
						{
							ddz_san.ddz_san_1[aqastt] = ddz_san.ddz_san_1[ipt];
							if (aqastt != ipt)
							{
								ddz_san.ddz_san_1[ipt] = 0;
							}

							aqastt++;
						}

					}
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;

					psit++;
					break;
				}
				iptsi++;
				is = -1;
			}


		}

	}
	for (int is = 0; is<20; is++)
	{
		if (ddz_san.ddz_san_1[is] == 0)
		{
			ddz_san.ps = is;
			break;
		}
	}
	       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;

	int sizeip = 1;
	int pd = psit;
	for (int is = 0; is < ddz_lian_1.lian_1; is++)
	{
		if (ddz_lian_1.ddzlian_1[is] == cfsl[psit - 1] + sizeip&&is>4)
		{
			sizeip++;
			is = -1;
			pd++;
		}

	}
	sizeip = 1;
	int itf = 0;
	if (pd < 5)
	{


		memcpy(ddz_san.ddz_san_1, bfjs1, sizeof(bfjs1));
		ddz_san.ps = bfjs;
	}
	       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
	if (pd >= 5)
	{
		for (int is = 0; is < ddz_lian_1.lian_1; is++)
		{
			if (ddz_lian_1.ddzlian_1[is] == cfsl[psit - 1] + sizeip&&is>4)
			{

				cfsl[psit] = ddz_lian_1.ddzlian_1[is];
				ddz_lian_1.ddzlian_1[is] = 0;
				//		ddz_lian_1.lian_1 -= 1;
				ddz_lian_1.ps -= 1;
				itf++;
				is = -1;
				psit++;


			}

		}
		if (ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1] == 1 && cfsl[psit - 1] == 13)
		{
			ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1] = 0;
			ddz_lian_1.lian_1 -= 1;
			ddz_lian_1.ps -= 1;
			cfsl[psit] = 1;
			psit++;
		}
		ddz_lian_1.lian_1 = ddz_lian_1.lian_1 - itf;
		if (ddz_lian_1.lian_2 == 0)
		{
			ddz_lian_1.lianpxjs += 1;
			for (int is = 0; is < psit; is++)
			{

				ddz_lian_1.ddzlian_2[is] = cfsl[is];
				ddz_lian_1.lian_2++;
				ddz_lian_1.ps++;

			}
			return true;
		}
		if (ddz_lian_1.lian_3 == 0)
		{
			for (int is = 0; is < psit; is++)
			{

				ddz_lian_1.ddzlian_3[is] = cfsl[is];
				ddz_lian_1.lian_3++;
				ddz_lian_1.ps++;

			}
			return true;
		}
		if (ddz_lian_1.lian_4 == 0)
		{
			ddz_lian_1.lianpxjs += 1;
			for (int is = 0; is < psit; is++)
			{

				ddz_lian_1.ddzlian_4[is] = cfsl[is];
				ddz_lian_1.lian_4++;
				ddz_lian_1.ps++;
			}

			return true;
		}

	

	
	}

	return false;
}
CT_VOID BOTProces::SetPX_CZ(int pd) //牌型显示
{
	SPFX = { 0 }; //手牌分析
	JD_JL = { 0 };//绝对牌型分析

	qz = { 0 };
	if (pd == 0)
	{
		if (ddz_san.ps > 0&&ddz_san.ps<20)
		{
			for (int itp = 0; itp < ddz_san.ps; itp++)
			{
				if (ddz_lian_1.lian_1 > 0)
				{
					if (ddz_san.ddz_san_1[itp] == ddz_lian_1.ddzlian_1[0] - 1)
					{

						ddz_lian_1.lian_1++;

						CT_BYTE linsh[35] = { 0 };
						for (int iips = 1; iips < ddz_lian_1.lian_1; iips++)
						{
							linsh[iips] = ddz_lian_1.ddzlian_1[iips - 1];


						}
						for (int iips = 1; iips < ddz_lian_1.lian_1; iips++)
						{
							ddz_lian_1.ddzlian_1[iips] = linsh[iips];


						}

						ddz_lian_1.ddzlian_1[0] = ddz_san.ddz_san_1[itp];
						ddz_san.ddz_san_1[itp] = { 0 };
					
						for (int ipst = itp; ipst < ddz_san.ps; ipst++)
						{
							ddz_san.ddz_san_1[ipst] = ddz_san.ddz_san_1[ipst + 1];
						}
						ddz_san.ps -= 1;
						ddz_lian_1.ps += 1;
						itp = -1;
					}
				}

				if (ddz_lian_1.lian_2 > 0)
				{
					if (ddz_san.ddz_san_1[itp] == ddz_lian_1.ddzlian_2[0] - 1)
					{

						ddz_lian_1.lian_2++;

						CT_BYTE linsh[35] = { 0 };
						for (int iips = 1; iips < ddz_lian_1.lian_2; iips++)
						{
							linsh[iips] = ddz_lian_1.ddzlian_2[iips - 1];


						}
						for (int iips = 1; iips < ddz_lian_1.lian_1; iips++)
						{
							ddz_lian_1.ddzlian_2[iips] = linsh[iips];


						}

						ddz_lian_1.ddzlian_2[0] = ddz_san.ddz_san_1[itp];
						ddz_san.ddz_san_1[itp] = { 0 };
						ddz_san.ps -= 1;
						for (int ipst = itp; ipst < ddz_san.ps; ipst++)
						{
							ddz_san.ddz_san_1[ipst] = ddz_san.ddz_san_1[ipst + 1];
						}
					
						ddz_lian_1.ps += 1;
						itp = -1;

					}
				}
				if (ddz_lian_1.lian_3 > 0)
				{
					if (ddz_san.ddz_san_1[itp] == ddz_lian_1.ddzlian_3[0] - 1)
					{

						ddz_lian_1.lian_3++;

						CT_BYTE linsh[35] = { 0 };
						for (int iips = 1; iips < ddz_lian_1.lian_3; iips++)
						{
							linsh[iips] = ddz_lian_1.ddzlian_3[iips - 1];


						}
						for (int iips = 1; iips < ddz_lian_1.lian_3; iips++)
						{
							ddz_lian_1.ddzlian_3[iips] = linsh[iips];


						}

						ddz_lian_1.ddzlian_3[0] = ddz_san.ddz_san_1[itp];
						ddz_san.ddz_san_1[itp] = { 0 };
						ddz_san.ps -= 1;
						for (int ipst = itp; ipst < ddz_san.ps; ipst++)
						{
							ddz_san.ddz_san_1[ipst] = ddz_san.ddz_san_1[ipst + 1];
						}
					
						ddz_lian_1.ps += 1;
						itp = -1;
					}
				}

				if (ddz_lian_1.lian_4 > 0)
				{
					if (ddz_san.ddz_san_1[itp] == ddz_lian_1.ddzlian_4[0] - 1)
					{

						ddz_lian_1.lian_4++;

						CT_BYTE linsh[35] = { 0 };
						for (int iips = 1; iips < ddz_lian_1.lian_4; iips++)
						{
							linsh[iips] = ddz_lian_1.ddzlian_4[iips - 1];


						}
						for (int iips = 1; iips < ddz_lian_1.lian_4; iips++)
						{
							ddz_lian_1.ddzlian_4[iips] = linsh[iips];


						}

						ddz_lian_1.ddzlian_4[0] = ddz_san.ddz_san_1[itp];
						ddz_san.ddz_san_1[itp] = { 0 };
						ddz_san.ps -= 1;
						for (int ipst = itp; ipst < ddz_san.ps; ipst++)
						{
							ddz_san.ddz_san_1[ipst] = ddz_san.ddz_san_1[ipst + 1];
						}
					
						ddz_lian_1.ps += 1;
						itp = -1;
					}
				}



			}

		}

	}
	   


	if (ddz_max.ddzmax_1 > 0)
	{
		//printf("【大王】");
	}
	if (ddz_max.ddzmax_2 > 0)
	{
		//printf("【小王】");
	}
	//printf(" 一手\r\n");
	if (ddz_two.ps > 0)
	{
		for (int isl = 0; isl < ddz_two.ps; isl++)
		{
			//printf("【%d】", ddz_two.ddztwo_1[isl]);
		}
		//printf(" 一手\r\n");
	}

	if (ddz_one.ps > 0)
	{

		for (int isl = 0; isl < ddz_one.ps; isl++)
		{
			//printf("【%d】", ddz_one.ddzone_1[isl]);
		}
		//printf(" 一手\r\n");
	}

	if (ddz_lian_1.ps > 0)
	{
		if (ddz_lian_1.lian_1 > 0)
		{
			for (int isl = 0; isl <ddz_lian_1.lian_1; isl++)
			{
				//printf("【%d】", ddz_lian_1.ddzlian_1[isl]);
			}
			//printf(" 连牌\r\n");
		}


		if (ddz_lian_1.lian_2 > 0)
		{
			for (int isl = 0; isl <ddz_lian_1.lian_2; isl++)
			{
				//printf("【%d】", ddz_lian_1.ddzlian_2[isl]);
			}
			//printf(" 连牌\r\n");
		}

		if (ddz_lian_1.lian_3> 0)
		{
			for (int isl = 0; isl <ddz_lian_1.lian_3; isl++)
			{
				//printf("【%d】", ddz_lian_1.ddzlian_3[isl]);
			}
			//printf(" 连牌\r\n");
		}

		if (ddz_lian_1.lian_4 > 0)
		{
			for (int isl = 0; isl <ddz_lian_1.lian_4; isl++)
			{
				//printf("【%d】", ddz_lian_1.ddzlian_4[isl]);
			}
			//printf(" 连牌\r\n");
		}

	}
	if (ddz_twolian.ps > 0)
	{
		if (ddz_twolian.lian_1 > 0)
		{
			for (int isl = 0; isl <ddz_twolian.lian_1; isl++)
			{
				//printf("【%d】", ddz_twolian.ddztwo_lian_1[isl]);
			}
			//printf(" 双连\r\n");
		}
		if (ddz_twolian.lian_2 > 0)
		{
			for (int isl = 0; isl <ddz_twolian.lian_2; isl++)
			{
				//printf("【%d】", ddz_twolian.ddztwo_lian_2[isl]);
			}
			//printf(" 双连\r\n");
		}
	}
	if (ddz_zd.ps > 0)
	{
		if (ddz_zd.ddzzd_1[0] > 0)
		{
			//printf("【%d】", ddz_zd.ddzzd_1[0]);
		}
		if (ddz_zd.ddzzd_2[0] > 0)
		{
			//printf("【%d】", ddz_zd.ddzzd_2[0]);
		}
		if (ddz_zd.ddzzd_3[0] > 0)
		{
			//printf("【%d】", ddz_zd.ddzzd_3[0]);
		}
		if (ddz_zd.ddzzd_4[0] > 0)
		{
			//printf("【%d】", ddz_zd.ddzzd_4[0]);
		}
		//printf(" 炸弹\r\n");
	}
	if (ddz_fj.ps > 0)
	{
		if (ddz_fj.feiji_1 > 0)
		{
			for (int isl = 0; isl <ddz_fj.feiji_1; isl++)
			{
				//printf("【%d】", ddz_fj.ddz_feiji_1[isl]);
			}
			//printf(" 飞机\r\n");
		}
		if (ddz_fj.feiji_2> 0)
		{
			for (int isl = 0; isl <ddz_fj.feiji_2; isl++)
			{
				//printf("【%d】", ddz_fj.ddz_feiji_2[isl]);
			}
			//printf(" 飞机\r\n");
		}
		if (ddz_fj.feiji_3 > 0)
		{
			for (int isl = 0; isl <ddz_fj.feiji_3; isl++)
			{
				//printf("【%d】", ddz_fj.ddz_feiji_3[isl]);
			}
			//printf(" 飞机\r\n");
		}
	}
	if (ddz_three.ps > 0)
	{
		if (ddz_three.ddz_three_1[0] > 0)
		{
			//printf("【%d】【%d】【%d】", ddz_three.ddz_three_1[0], ddz_three.ddz_three_1[1], ddz_three.ddz_three_1[2]);
		}
		if (ddz_three.ddz_three_2[0] > 0)
		{
			//printf("【%d】【%d】【%d】", ddz_three.ddz_three_2[0], ddz_three.ddz_three_2[1], ddz_three.ddz_three_2[2]);
		}
		if (ddz_three.ddz_three_3[0] > 0)
		{
			//printf("【%d】【%d】【%d】", ddz_three.ddz_three_3[0], ddz_three.ddz_three_3[1], ddz_three.ddz_three_3[2]);
		}
		if (ddz_three.ddz_three_4[0] > 0)
		{
			//printf("【%d】【%d】【%d】", ddz_three.ddz_three_4[0], ddz_three.ddz_three_4[1], ddz_three.ddz_three_4[2]);
		}
		if (ddz_three.ddz_three_5[0] > 0)
		{
			//printf("【%d】【%d】【%d】", ddz_three.ddz_three_5[0], ddz_three.ddz_three_5[1], ddz_three.ddz_three_5[2]);
		}
		//printf(" 三带\r\n");
	}
	if (pd==1)
	{
		switch (ddz_two_fx.two_js)
		{
		case 1:
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("对子\r\n");
			break;
		case 2:
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("对子\r\n");
			break;
		case 3:
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("对子\r\n");
			break;
		case 4:
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_4[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_4[0]);
			//printf("对子\r\n");
			break;
		case 5:
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_4[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_4[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_5[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_5[0]);
			//printf("对子\r\n");
			break;
		case 6:
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_4[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_4[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_5[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_5[0]);
			//printf("对子\r\n");
			//printf("【%d】", ddz_two_fx.ddz_two_6[0]);
			//printf("【%d】", ddz_two_fx.ddz_two_6[0]);
			//printf("对子\r\n");
			break;
		}
		for (int isl = 0; isl <ddz_san.ps; isl++)
		{
			//printf("【%d】", ddz_san.ddz_san_1[isl]);
		}
		//printf(" 散牌\r\n");
	}


}
CT_VOID BOTProces::SetSan_CZ() //并合并对子
{
	for (int i = 0; i < ddz_san.ps; i++)
	{
		if (ddz_san.ddz_san_1[i] == ddz_san.ddz_san_1[i + 1])
		{
			ddz_two_fx.two_js++;
			if (ddz_two_fx.two_js == 1)
			{
				ddz_two_fx.ddz_two_1[0] = ddz_san.ddz_san_1[i];
				ddz_two_fx.ddz_two_1[1] = ddz_san.ddz_san_1[i];
				//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
				//printf("【%d】", ddz_two_fx.ddz_two_1[0]);
			}
			if (ddz_two_fx.two_js == 2)
			{
				ddz_two_fx.ddz_two_2[0] = ddz_san.ddz_san_1[i];
				ddz_two_fx.ddz_two_2[1] = ddz_san.ddz_san_1[i];
				//printf("【%d】", ddz_two_fx.ddz_two_2[0]);
				//printf("【%d】", ddz_two_fx.ddz_two_2[0]);

			}

			if (ddz_two_fx.two_js == 3)
			{
				ddz_two_fx.ddz_two_3[0] = ddz_san.ddz_san_1[i];
				ddz_two_fx.ddz_two_3[1] = ddz_san.ddz_san_1[i];
				//printf("【%d】", ddz_two_fx.ddz_two_3[0]);
				//printf("【%d】", ddz_two_fx.ddz_two_3[0]);

			}
			if (ddz_two_fx.two_js == 4)
			{
				ddz_two_fx.ddz_two_4[0] = ddz_san.ddz_san_1[i];
				ddz_two_fx.ddz_two_4[1] = ddz_san.ddz_san_1[i];
				//printf("【%d】", ddz_two_fx.ddz_two_4[0]);
				//printf("【%d】", ddz_two_fx.ddz_two_4[0]);

			}
			if (ddz_two_fx.two_js == 5)
			{
				ddz_two_fx.ddz_two_5[0] = ddz_san.ddz_san_1[i];
				ddz_two_fx.ddz_two_5[1] = ddz_san.ddz_san_1[i];
				//printf("【%d】", ddz_two_fx.ddz_two_5[0]);
				//printf("【%d】", ddz_two_fx.ddz_two_5[0]);

			}
			if (ddz_two_fx.two_js == 6)
			{
				ddz_two_fx.ddz_two_6[0] = ddz_san.ddz_san_1[i];
				ddz_two_fx.ddz_two_6[1] = ddz_san.ddz_san_1[i];
				//printf("【%d】", ddz_two_fx.ddz_two_6[0]);
				//printf("【%d】", ddz_two_fx.ddz_two_6[0]);

			}




		//	//printf("对子\r\n");
			ddz_san.ddz_san_1[i] = { 0 };
			ddz_san.ddz_san_1[i + 1] = { 0 };
			CT_BYTE lins[20] = { 0 };
			int ipaffg = 0;
			for (int is = i + 2; is < ddz_san.ps; is++)
			{
				lins[ipaffg] = ddz_san.ddz_san_1[is];
				ipaffg++;
			}
			int sWseze = 0;
			ddz_san.ps -= 2;
			ddz_two_fx.ps += 2;
			for (int ips = i; ips < ddz_san.ps; ips++)
			{
				ddz_san.ddz_san_1[ips] = lins[sWseze];
	
				sWseze++;
			}
			ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
			ddz_san.ddz_san_1[ddz_san.ps+1] = { 0 };

		
			i = -1;


		}
	}


}
CT_VOID BOTProces::Set_ZHPX(CT_DWORD cbCardCount, CT_BYTE cbCardData[MAX_COUNT], CT_BYTE ICount) //首次出牌
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));
	CT_DWORD sz1, sz2, sz3, sz4;
	//	//printf("对子=%f,连顺= %f，双连顺= %f，散= %f，三代1= %f，飞机=  %f", qz.double_1, qz.lian_1, qz.lian_2, qz.san_1, qz.three_1, qz.three_2);
	if (SPFX.SP_SH == 1)
	{
		CT_BYTE ct[MAX_COUNT] = { 0 };
		for (int i = 0; i < MAX_COUNT; i++)
		{
			ct[i] = GetCodePX->GetCardValue(myCard[i]);
			if (ct[i] != 0)
			{

				mOutCard.cbCardData[mOutCard.cbCardCount] = ct[i];
				mOutCard.cbCardCount++;
}
		}
	SetGOCard("tps");
			return;
	}
		if (Bankxin == 1)
		{
			if (OnGameCard == 2 || DownGameCard == 2)
			{
				if (ICount == 1)
				{
					if (onCard.Cdata[2] == 0 && onCard.Cdata[0] == onCard.Cdata[1])
					{
						if (ddz_san.ps > 0&&ddz_san.ps<20)
						{
							mOutCard.cbCardCount = 1;
							mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps - 1];
							ddz_san.ddz_san_1[ddz_san.ps - 1] = { 0 };
							ddz_san.ps -= 1;
							SetGOCard("tps");
								return ;
						}
						else if (ddz_two_fx.ps > 0)
						{
							mOutCard.cbCardData[0] = ddz_two_fx.ddz_two_1[0];
							mOutCard.cbCardData[1] = ddz_two_fx.ddz_two_1[0];
							mOutCard.cbCardCount = 2;
							ddz_two_fx.ps -= 2;
							ddz_two_fx.two_js -= 1;
							ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
							ddz_san.ps += 1;
							memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
							memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
							memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
							memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
							memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
							ddz_two_fx.ddz_two_6[0] = { 0 };
							ddz_two_fx.ddz_two_6[1] = { 0 };
							SetGOCard("tps");
							       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
								return;
						}
						else if (ddz_lian_1.ps > 0)
						{
							mOutCard.cbCardCount = ddz_lian_1.lian_1;
							cbCardCount = ddz_lian_1.lian_1;
							ICount = 2;
						}
						else if (ddz_twolian.ps > 0)
						{
							mOutCard.cbCardCount = ddz_twolian.lian_1;
							cbCardCount = ddz_twolian.lian_1;
							ICount = 3;
						}
						else if (ddz_three.ps > 0&&ddz_san.ps > 0 && ddz_san.ps<20)
						{
							mOutCard.cbCardCount = 4;
							cbCardCount = 4;
							ICount = 5;
						}
						else if (ddz_fj.ps > 0&& ddz_san.ps >1 && ddz_san.ps<20)
						{
							mOutCard.cbCardCount = 8;
							cbCardCount = 8;
							ICount = 6;
						}
						else
						{
							ICount = 7;
						}
					}
					if (DownCard.Cdata[2] == 0 && DownCard.Cdata[0] == DownCard.Cdata[1])
					{
						if (ddz_san.ps > 0 && ddz_san.ps<20)
						{
							mOutCard.cbCardCount = 1;
							mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps - 1];
							ddz_san.ddz_san_1[ddz_san.ps - 1] = { 0 };
							ddz_san.ps -= 1;
							SetGOCard("tps");
							//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
							return;
						}
						else if (ddz_two_fx.ps > 0)
						{
							mOutCard.cbCardData[0] = ddz_two_fx.ddz_two_1[0];
							mOutCard.cbCardData[1] = ddz_two_fx.ddz_two_1[0];
							mOutCard.cbCardCount = 2;
							ddz_two_fx.ps -= 2;
							ddz_two_fx.two_js -= 1;
							ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
							ddz_san.ps += 1;
							memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
							memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
							memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
							memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
							memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
							ddz_two_fx.ddz_two_6[0] = { 0 };
							ddz_two_fx.ddz_two_6[1] = { 0 };
							SetGOCard("tps");
							//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
							return;
						}
						else if (ddz_lian_1.ps > 0)
						{
							ICount = 2;
						}
						else if (ddz_twolian.ps > 0)
						{
							mOutCard.cbCardCount = ddz_twolian.lian_1;
							ICount = 3;
						}
						else if (ddz_three.ps > 0&&ddz_san.ps>0&&ddz_san.ps<20)
						{
							mOutCard.cbCardCount = 4;
							ICount = 5;
						}
						else if (ddz_fj.ps > 00 && ddz_san.ps>1 && ddz_san.ps<20)
						{
							mOutCard.cbCardCount = 8;
							ICount = 6;
						}
						else
						{
							ICount = 7;
						}
					}
				}
				if (onCard.Cdata[2] == 0 && onCard.Cdata[0]!= onCard.Cdata[1])
				{
					if (ddz_two_fx.ps > 0)
					{
						mOutCard.cbCardData[0] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[1] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardCount = 2;
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
						ddz_san.ps += 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						ddz_two_fx.ddz_two_6[0] = { 0 };
						ddz_two_fx.ddz_two_6[1] = { 0 };
						SetGOCard("tps");
						//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						return;
					}
					else if (ddz_lian_1.ps > 0)
					{
						ICount = 2;
					}
					else if (ddz_twolian.ps > 0)
					{
						mOutCard.cbCardCount = ddz_twolian.lian_1;
						ICount = 3;
					}
					else if (ddz_three.ps > 0 && ddz_san.ps > 0 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 4;
						ICount = 5;
					}
					else if (ddz_fj.ps > 00 && ddz_san.ps > 1 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 8;
						ICount = 6;
					}
					else if (ddz_san.ps > 0 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps - 1];
						ddz_san.ddz_san_1[ddz_san.ps - 1] = { 0 };
						ddz_san.ps -= 1;
						SetGOCard("tps");
						return;
					}
					else
					{
						ICount = 7;
					}
				}
				if (DownCard.Cdata[2] == 0 && DownCard.Cdata[0] != DownCard.Cdata[1])
				{
					if (ddz_two_fx.ps > 0)
					{
						mOutCard.cbCardData[0] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[1] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardCount = 2;
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						ddz_two_fx.ddz_two_6[0] = { 0 };
						ddz_two_fx.ddz_two_6[1] = { 0 };
						SetGOCard("tps");
						//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						return;
					}
					else if (ddz_lian_1.ps > 0)
					{
						ICount = 2;
					}
					else if (ddz_twolian.ps > 0)
					{
						mOutCard.cbCardCount = ddz_twolian.lian_1;
						ICount = 3;
					}
					else if (ddz_three.ps > 0 && ddz_san.ps > 0 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 4;
						ICount = 5;
					}
					else if (ddz_fj.ps > 00 && ddz_san.ps > 1 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 8;
						ICount = 6;
					}
					else if (ddz_san.ps > 0 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps - 1];
						ddz_san.ddz_san_1[ddz_san.ps - 1] = { 0 };
						ddz_san.ps -= 1;
						SetGOCard("tps");
		                return;
					}
					else
					{
						ICount = 7;
					}
				}
				
			}

			if (OnGameCard == 1||DownGameCard==1)
			{
				if (ICount == 4)
				{
				if (ddz_lian_1.ps > 0)
					{
						mOutCard.cbCardCount = ddz_lian_1.lian_1;
						cbCardCount = ddz_lian_1.lian_1;
						ICount = 2;
					}
					else if (ddz_twolian.ps > 0)
					{
						mOutCard.cbCardCount = ddz_twolian.lian_1;
						cbCardCount = ddz_twolian.lian_1;
						ICount = 3;
					}
					else if (ddz_three.ps > 0&&ddz_san.ps>0&&ddz_san.ps<20)
					{
						mOutCard.cbCardCount = 4;
						cbCardCount = 4;
						ICount = 5;
					}
					else if (ddz_fj.ps > 0 && ddz_san.ps>1 && ddz_san.ps<20)
					{
						mOutCard.cbCardCount = 8;
						cbCardCount = 8;
						ICount = 6;
					}	else if (ddz_two_fx.ps > 0)
					{
						mOutCard.cbCardCount = 2;
						cbCardCount = 2;
						ICount = 1;
					}
					else if (ddz_zd.zdpxjs > 0 && ddz_san.ps > 1)
					{
					SET_FOUR_TAKE_TWO();
					SetGOCard("tps");
						return;

					}else
					{
						ICount = 7;
					}
				}
			}

		}
		if (Bankxin == 2 && OnGameCard == 2) //上家地主，还剩 2个牌
		{
			
			if (onCard.Cdata[2] == 0&&onCard.Cdata[0] == onCard.Cdata[1]&&ICount == 1)
			{
				
					if (ddz_san.ps > 0&&ddz_san.ps<20)
					{
						mOutCard.cbCardCount = 1;
						cbCardCount = 1;
						ICount = 4;
					}
					else if (ddz_lian_1.ps > 0)
					{
						mOutCard.cbCardCount = ddz_lian_1.lian_1;
						cbCardCount = ddz_lian_1.lian_1;
						ICount = 2;
					}
					else if (ddz_twolian.ps > 0)
					{
						mOutCard.cbCardCount = ddz_twolian.lian_1;
						cbCardCount= ddz_twolian.lian_1;
						ICount = 3;
					}
					else if (ddz_three.ps > 0&&ddz_san.ps>0&&ddz_san.ps<20)
					{
						mOutCard.cbCardCount = 4;
						cbCardCount = 4;
						ICount = 5;
					}
					else if (ddz_fj.ps > 0 && ddz_san.ps>1 && ddz_san.ps<20)
					{
						mOutCard.cbCardCount = 8;
						cbCardCount = 8;
						ICount = 6;
					}
					else
					{
						ICount = 7;
					}
				}
		
			if (onCard.Cdata[0] != onCard.Cdata[1]&&ICount == 4)
			{
				if (ddz_two_fx.ps > 0)
					{
						mOutCard.cbCardCount = 2;
						cbCardCount = 2;
						ICount = 1;
					}
				
			}
			
		}
		if (Bankxin == 2 && OnGameCard == 1&& ICount == 4)
		{
				if (ddz_two_fx.ps > 0)
				{
					mOutCard.cbCardCount = 2;
					cbCardCount = 2;
					ICount = 1;
				}else if (ddz_lian_1.ps > 0)
				{
					mOutCard.cbCardCount = ddz_lian_1.lian_1;
					cbCardCount = ddz_lian_1.lian_1;
					ICount = 2;
				}	else if (ddz_twolian.ps > 0)
				{
					mOutCard.cbCardCount = ddz_twolian.lian_1;
					cbCardCount = ddz_twolian.lian_1;
					ICount = 3;
				}
				else if (ddz_three.ps > 0 && ddz_san.ps > 0 && ddz_san.ps < 20)
				{
					mOutCard.cbCardCount = 4;
					cbCardCount = 4;
					ICount = 5;
				}else if (ddz_fj.ps > 0 && ddz_san.ps >1&& ddz_san.ps < 20)
				{
					mOutCard.cbCardCount = 8;
					cbCardCount = 8;
					ICount = 6;
				}
				else
				{
					ICount = 7;
				}
			}
	
		if (Bankxin == 2 && DownGameCard == 2)
		{
			if (onCard.Cdata[2] == 0 && onCard.Cdata[0] == onCard.Cdata[1])
			{
				if (ddz_two_fx.ps > 0)
				{
					mOutCard.cbCardCount = 2;
					cbCardCount = 2;
					ICount = 1;
				}
			}

		}
		if (Bankxin == 2 && DownGameCard == 1)
		{
			CT_BYTE ct[MAX_COUNT] = { 0 };
			for (int i = 0; i<20; i++)
			{
			
				ct[i] = GetCodePX->GetCardValue(myCard[i]);
			}
			for (int is = 0; is < 20; is++)
			{
				if (ct[is] < DownCard.Cdata[0]&&ct[is]!=0&& ct[is] != 1&&ct[is] != 2)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = ct[is];
					SetGOCard("tps");
						return;
				}
			}




			}
        if (Bankxin == 3 && OnGameCard == 2)
		{
			
			if (onCard.Cdata[2] == 0 && onCard.Cdata[0] == onCard.Cdata[1])
			{
				if (ddz_two_fx.ps > 0)
				{
					mOutCard.cbCardCount = 2;
					cbCardCount = 2;
					ICount = 1;
				}
			}
	


		}
		if (Bankxin == 3 && OnGameCard == 1)
		{
	
			if (ddz_san.ps > 0)
			{
				mOutCard.cbCardCount = 1;
				cbCardCount = 1;
				ICount = 4;
			}
			else if (ddz_two_fx.ps > 0)
			{
				mOutCard.cbCardData[0] = ddz_two_fx.ddz_two_1[0];
				mOutCard.cbCardCount = 1;
				cbCardCount = 1;
				ddz_two_fx.ps -= 2;
				ddz_two_fx.two_js -= 1;
				ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
				ddz_san.ps += 1;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
				memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
				ddz_two_fx.ddz_two_6[0] = { 0 };
				ddz_two_fx.ddz_two_6[1] = { 0 };
				SetGOCard("tps");
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					return;
			}
	
		}
		if (Bankxin == 3 && DownGameCard == 2)
		{
			
			if (DownCard.Cdata[0] == DownCard.Cdata[1]&&ICount == 1)
			{
					if (ddz_san.ps > 0)
					{
						mOutCard.cbCardCount = 1;
						cbCardCount = 1;
						ICount = 4;
					}
					else if (ddz_lian_1.ps > 0)
					{
						mOutCard.cbCardCount = ddz_lian_1.lian_1;
						cbCardCount = ddz_lian_1.lian_1;
						ICount = 2;
					}
					else if (ddz_twolian.ps > 0)
					{
						mOutCard.cbCardCount = ddz_twolian.lian_1;
						cbCardCount = ddz_twolian.lian_1;
						ICount = 3;
					}
					else if (ddz_three.ps > 0&&ddz_san.ps>0&&ddz_san.ps<20)
					{
						mOutCard.cbCardCount = 4;
						cbCardCount = 4;
						ICount = 5;
					}
					else if (ddz_fj.ps > 0 && ddz_san.ps>1 && ddz_san.ps<20)
					{
						mOutCard.cbCardCount = 8;
						cbCardCount = 8;
						ICount = 6;
					}
					else
					{
						ICount = 7;
					}
				
			}
			if (DownCard.Cdata[0] != DownCard.Cdata[1]&& ICount == 4)
			{
			
					if (ddz_two_fx.ps > 0)
					{
						mOutCard.cbCardCount = 2;
						ICount = 1;
					
				}
			}
		}
		if (Bankxin == 3 && DownGameCard == 1)
		{
			if (ICount == 4)
			{
				if (ddz_two_fx.ps > 0)
				{
					mOutCard.cbCardCount = 2;
					cbCardCount=2;
					ICount = 1;
				}
				else if (ddz_lian_1.ps > 0)
				{
					mOutCard.cbCardCount = ddz_lian_1.lian_1;
					cbCardCount = ddz_lian_1.lian_1;
					ICount = 2;
				}
				else if (ddz_twolian.ps > 0)
				{
					mOutCard.cbCardCount = ddz_twolian.lian_1;
					cbCardCount = ddz_twolian.lian_1;
					ICount = 3;
				}
				else if (ddz_three.ps > 0&&ddz_san.ps>0&&ddz_san.ps<20)
				{
					mOutCard.cbCardCount = 4;
					cbCardCount = 4;
					ICount = 5;
				}
				else if (ddz_fj.ps > 0 && ddz_san.ps>1 && ddz_san.ps<20)
				{
					mOutCard.cbCardCount = 8;
					cbCardCount = 8;
					ICount = 6;
				}
				else
				{
					ICount = 7;
				}
			}
		}
//	//printf("对子=%f,连顺= %f，双连顺= %f，散= %f，三代1= %f，飞机=  %f", qz.double_1, qz.lian_1, qz.lian_2, qz.san_1, qz.three_1, qz.three_2	//----------------------------------------------------
		switch (ICount)
		{
		case 1:
		mOutCard.cbCardCount = cbCardCount;
			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
			memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			ddz_two_fx.ddz_two_6[0] = { 0 };
			ddz_two_fx.ddz_two_6[1] = { 0 };
			break;
		case 2:
			if (ddz_lian_1.ps > 0)
			{
				mOutCard.cbCardCount = cbCardCount;
				memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
				ddz_lian_1.lianpxjs -= 1;
				ddz_lian_1.ps -= cbCardCount;
				ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
				ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
				ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
				ddz_lian_1.lian_4 = 0;
				memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
				memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_3));
				memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_4));
			}
			break;
		case 3:
			mOutCard.cbCardCount = cbCardCount;
			memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
			memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
			memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
			ddz_twolian.lianpxjs -= 1;
			ddz_twolian.ps -= ddz_twolian.lian_1;
			ddz_twolian.lian_1 = ddz_twolian.lian_2;
			break;
		case 4:
	
				if (Bankxin == 3)
				{
					ddz_san.ps -= 1;
					mOutCard.cbCardCount = cbCardCount;
					mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps];
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 }; 
					break;
				}
				else if (Bankxin != 3)
				{
					mOutCard.cbCardCount = cbCardCount;
					mOutCard.cbCardData[0] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = { 0 };
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 }; //散牌位移
				}
			
			break;
		case 5:
			if (ddz_san.ps>0&&ddz_san.ps<20)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
				mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
				ddz_three.three_js -= 1;
				ddz_three.ps -= 3;
				memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
				memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
				memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
				memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
				ddz_three.ddz_three_5[0] = { 0 };
				ddz_three.ddz_three_5[1] = { 0 };
				ddz_three.ddz_three_5[2] = { 0 };
				ddz_san.ddz_san_1[0] = { 0 };
				ddz_san.ps -= 1;
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 }; //散牌位移
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			}
			else if (ddz_two_fx.two_js>0)
			{
				memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
				mOutCard.cbCardCount = 5;
				ddz_three.ps -= 3;
				ddz_three.three_js -= 1;
				memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
				memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
				memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
				memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
				memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
				mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
				mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
				ddz_two_fx.ps -= 2;
				ddz_two_fx.two_js -= 1;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
				memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
				memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			}
			break;
		case 6:
			if (ddz_san.ps > 1 && ddz_fj.feiji_1 == 6)
			{
				memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
				mOutCard.cbCardCount = 8;
				ddz_fj.feiji_js -= 1;
				ddz_fj.ps -= ddz_fj.feiji_1;
				ddz_fj.feiji_1 = ddz_fj.feiji_2;
				ddz_fj.feiji_2 = ddz_fj.feiji_2;
				ddz_fj.feiji_3 = 0;
				memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
				memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
				memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
				ddz_san.ps -= 1;
				mOutCard.cbCardData[6] = ddz_san.ddz_san_1[0];
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				ddz_san.ps -= 1;
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				mOutCard.cbCardData[7] = ddz_san.ddz_san_1[0];
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;


			}
			if (ddz_two_fx.two_js > 1 && ddz_fj.feiji_1 == 6)
			{
				memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
				mOutCard.cbCardData[ddz_fj.feiji_1] = ddz_two_fx.ddz_two_1[0];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 1] = ddz_two_fx.ddz_two_1[1];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 2] = ddz_two_fx.ddz_two_2[0];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 3] = ddz_two_fx.ddz_two_2[1];
				mOutCard.cbCardCount = 10;
				ddz_fj.feiji_js -= 1;
				ddz_fj.ps -= ddz_fj.feiji_1;
				ddz_fj.feiji_1 = ddz_fj.feiji_2;
				ddz_fj.feiji_2 = ddz_fj.feiji_2;
				ddz_fj.feiji_3 = 0;
				memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
				memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
				memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));

				ddz_two_fx.ps -= 4;
				ddz_two_fx.two_js -= 2;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
				memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
				ddz_two_fx.ddz_two_5[0] = { 0 };
				ddz_two_fx.ddz_two_5[1] = { 0 };
				ddz_two_fx.ddz_two_6[0] = { 0 };
				ddz_two_fx.ddz_two_6[0] = { 0 }; //对子位移




			}
			if (ddz_two_fx.two_js == 1 && ddz_fj.feiji_1 == 6)
			{
				memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
				mOutCard.cbCardCount = 8;
				ddz_fj.feiji_js -= 1;
				ddz_fj.ps -= ddz_fj.feiji_1;
				ddz_fj.feiji_1 = ddz_fj.feiji_2;
				ddz_fj.feiji_2 = ddz_fj.feiji_2;
				ddz_fj.feiji_3 = 0;
				memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
				memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
				memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
				mOutCard.cbCardData[6] = ddz_two_fx.ddz_two_1[0];
				mOutCard.cbCardData[7] = ddz_two_fx.ddz_two_1[1];
				ddz_two_fx.ps -= 2;
				ddz_two_fx.two_js = 0;
				memset(ddz_two_fx.ddz_two_1, 0, sizeof(ddz_two_fx.ddz_two_1));

			}
			if (ddz_san.ps > 2 && ddz_fj.feiji_1 ==9)
			{
				memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
				mOutCard.cbCardCount = 12;
				ddz_fj.feiji_js -= 1;
				ddz_fj.ps -= ddz_fj.feiji_1;
				ddz_fj.feiji_1 = ddz_fj.feiji_2;
				ddz_fj.feiji_2 = ddz_fj.feiji_2;
				ddz_fj.feiji_3 = 0;
				memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
				memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
				memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
				ddz_san.ps -= 1;
				mOutCard.cbCardData[9] = ddz_san.ddz_san_1[0];
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				ddz_san.ps -= 1;
	
				mOutCard.cbCardData[10] = ddz_san.ddz_san_1[0];
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				ddz_san.ps -= 1;

				mOutCard.cbCardData[11] = ddz_san.ddz_san_1[0];
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
		


			}
			if (ddz_two_fx.two_js > 2 && ddz_fj.feiji_1 == 9)
			{
				memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
				mOutCard.cbCardData[ddz_fj.feiji_1] = ddz_two_fx.ddz_two_1[0];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 1] = ddz_two_fx.ddz_two_1[1];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 2] = ddz_two_fx.ddz_two_2[0];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 3] = ddz_two_fx.ddz_two_2[1];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 4] = ddz_two_fx.ddz_two_3[0];
				mOutCard.cbCardData[ddz_fj.feiji_1 + 5] = ddz_two_fx.ddz_two_3[1];
				mOutCard.cbCardCount = 15;
				ddz_fj.feiji_js -= 1;
				ddz_fj.ps -= ddz_fj.feiji_1;
				ddz_fj.feiji_1 = ddz_fj.feiji_2;
				ddz_fj.feiji_2 = ddz_fj.feiji_2;
				ddz_fj.feiji_3 = 0;
				memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
				memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
				memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));

				ddz_two_fx.ps -= 6;
				ddz_two_fx.two_js -= 3;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_5));
				ddz_two_fx.ddz_two_4[0] = { 0 };
				ddz_two_fx.ddz_two_4[1] = { 0 };
				ddz_two_fx.ddz_two_5[0] = { 0 };
				ddz_two_fx.ddz_two_5[1] = { 0 };
				ddz_two_fx.ddz_two_6[0] = { 0 };
				ddz_two_fx.ddz_two_6[0] = { 0 }; //对子位移




			}
			break;
		case 7:

			if (ddz_one.ps > 0 && ddz_one.ps != 4)
			{
				cbCardCount = ddz_one.ps;
				mOutCard.cbCardCount = ddz_one.ps;
				ddz_one.ps = 0;
				ddz_one.onepxjs = 0;
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				break;
			}
			if (ddz_two.ps > 0 && ddz_two.ps != 4)
			{
				cbCardCount = ddz_two.ps;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				mOutCard.cbCardCount = ddz_two.ps;
				ddz_two.ps = 0;
				ddz_two.twopxjs = 0;
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				break;
			}

			if (ddz_max.maxpxjs == 2)
			{
				cbCardCount = 1;
				ddz_max.maxpxjs = 0;
				ddz_max.ps = 0;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				mOutCard.cbCardData[0] = 0x4e;
				mOutCard.cbCardCount = cbCardCount;
	
				return;
	
			}
			if (ddz_max.maxpxjs == 1)
			{
				cbCardCount = 1;
				ddz_max.maxpxjs = 0;
				ddz_max.ps = 0;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				mOutCard.cbCardData[0] = 0x4f;
				mOutCard.cbCardCount = cbCardCount;
		
				return;
	
			}
			if (ddz_zd.zdpxjs > 0)
			{
				if (DownGameCard == 1 || OnGameCard == 1)
				{
					if (ddz_san.ps > 1)
					{
						cbCardCount = 6;
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						mOutCard.cbCardData[4] = ddz_san.ddz_san_1[0];
						mOutCard.cbCardData[5] = ddz_san.ddz_san_1[1];
						ddz_san.ddz_san_1[0] = 0;
						ddz_san.ps -= 1;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

						}
						ddz_san.ps -= 1;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

						}
						mOutCard.cbCardCount = 6;
						break;
					}
				}


				cbCardCount = 4;
				ddz_zd.zdpxjs -= 1;
				ddz_zd.ps -= 4;
				memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
				memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
				memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
				memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
      memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));

				mOutCard.cbCardCount = 4;
			
				break;
			}
			if (ddz_max.maxpxjs == 3)
			{
				cbCardCount = 2;
				ddz_max.maxpxjs = 0;
				ddz_max.ps = 0;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				mOutCard.cbCardData[0] = 0x4e;
				mOutCard.cbCardData[1] = 0x4f;
				mOutCard.cbCardCount = cbCardCount;
		
				return;

			}
			if (ddz_san.ps > 0&&ddz_san.ps<20)
			{
				mOutCard.cbCardCount = 1;
				ddz_san.ps -= 1;
				mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps];
				ddz_san.ddz_san_1[ddz_san.ps] = 0;
				      
				break;
			}
			if (ddz_two_fx.ps > 0)
			{
				mOutCard.cbCardCount = 2;
				memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
				ddz_two_fx.ps -= 2;
				ddz_two_fx.two_js -= 1;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
				memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
				ddz_two_fx.ddz_two_6[0] = { 0 };
				ddz_two_fx.ddz_two_6[1] = { 0 };
				break;
			}
			break;
		}

	
	
		for (int ist = 0; ist <  mOutCard.cbCardCount; ist++)
			{
				sz1 = mOutCard.cbCardData[ist];
				sz2 = sz1 + 16;
				sz3 = sz2 + 16;
				sz4 = sz3 + 16;
				for (int itp = 0; itp < MAX_COUNT; itp++)
				{
					if (myCard[itp] == sz1 || myCard[itp] == sz2 || myCard[itp] == sz3 || myCard[itp] == sz4)
					{
		mOutCard.cbCardData[ist] = myCard[itp];
						myCard[itp] = { 0 };
					
						break;
					}
				}
			}

	
}
CT_DWORD BOTProces::Set_FX_QZ() //权重分析
{
	qz = { 0 };
	if (ddz_max.ps>0)
	{
		if (ddz_max.maxpxjs == 1)
		{
			qz.san_1 += 1.5;
		}
		else if (ddz_max.maxpxjs == 2)
		{
			qz.san_1 += 1.0;
		}
		else if (ddz_max.maxpxjs == 3)
		{
			qz.san_1 += 0.8;
		}

	}
	if (ddz_two.ps == 1)
	{
		qz.san_1 += 1.0;
	}
	if (ddz_two.ps == 3)
	{
		qz.san_1 += 1.5;
	}
	if (ddz_two.ps == 2)
	{
		qz.double_1 += 1.5;
	}
	if (ddz_one.ps == 1)
	{
		qz.san_1 += 1.0;
	}
	if (ddz_one.ps == 2)
	{
		qz.san_1 += 0.5;
	}
	if (ddz_one.ps == 2)
	{
		qz.double_1 += 1.2;
	}
	if (ddz_one.ps == 3)
	{
		qz.san_1 += 1.0;
		qz.three_1 += 2.5;
	}
	if (ddz_two_fx.two_js== 3)
	{
	
			qz.double_1 += 2.5;
	}
	if (ddz_two_fx.two_js > 3)
	{

		qz.double_1 += 3.5;
	}
	if (ddz_two_fx.two_js== 2)
	{

		qz.double_1 += 1.7;
	}
	if (ddz_two_fx.two_js == 1)
	{

		qz.double_1 += 0.7;
	}
	if (ddz_two.ps == 4 && ddz_one.ps == 2)
	{
		qz.double_1 += 1.5;
	}
	if (ddz_two.ps == 4 && ddz_one.ps >= 2 && ddz_max.ps>0)
	{
		qz.double_1 += 1.5;
		qz.san_1 += 1.5;
	}
	if (ddz_two.ps == 2 && ddz_max.maxpxjs == 2 && ddz_one.ps == 3)
	{
		qz.san_1 += 1;
	}
	if (ddz_two.ps == 2 && ddz_max.ps >0 && ddz_one.ps == 2)
	{
		qz.double_1 += 1;
	}
	if (ddz_lian_1.lianpxjs > 1)
	{
		qz.lian_1 += 3;
	}
	if (ddz_lian_1.lianpxjs ==1)
	{
		qz.lian_1 += 2.2;
	}
	if (ddz_lian_1.ps > 8)
	{
		qz.lian_1 += 2.5;
	}
	if (ddz_twolian.lianpxjs == 1)
	{
		qz.lian_2 += 2;
	}
	if (ddz_twolian.lianpxjs == 2)
	{
		qz.lian_2 += 3;
	}
	if (ddz_three.three_js == 1)
	{
		qz.three_1 += 1.5;
	}
	if (ddz_three.three_js == 2)
	{
		qz.three_1 += 4.5;
	}
	if (ddz_three.three_js>2)
	{
		qz.three_1 += 6.0;
	}
	if (ddz_three.three_js == 2 && ddz_three.ddz_three_1[0]>9)
	{
		qz.three_1 += 1.5;
	}

	if (ddz_fj.feiji_js>1)
	{
		qz.three_2 += 7;

	}
	if (ddz_fj.feiji_js==1)
	{
		qz.three_2 += 6.5;

	}
	if (ddz_san.ps != 0)
	{
		CT_DWORD xNcpxd = 0;
		if (ddz_fj.ps > 0 || ddz_three.ps > 0)
		{
			xNcpxd = ddz_fj.feiji_js + ddz_fj.feiji_js;
			xNcpxd += ddz_three.three_js;
			xNcpxd = ddz_san.ps - xNcpxd;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;

			if (xNcpxd > 2)
			{
				qz.san_1 += 1.5;
			}
			if (xNcpxd == 2)
			{
				qz.san_1 -= 0.5;
			}
			if (xNcpxd == 1)
			{
				qz.san_1 -= 2;
			}
		}
		else
		{ if(ddz_san.ps>4)qz.san_1 += 2.5; if (ddz_san.ps==4)qz.san_1 += 2; if (ddz_san.ps == 3)qz.san_1 +=1.5; if (ddz_san.ps == 2)qz.san_1 += 1.0; if (ddz_san.ps == 1)qz.san_1 += 0.7;
		}
	}
	else
	{
		qz.san_1 = 0;
	}

	if(ddz_two_fx.ps==0)
		qz.double_1 = 0;
	if (ddz_three.ps == 0)
		qz.three_1 = 0;
	if (ddz_fj.ps == 0)
		qz.three_2 = 0;
	if (ddz_lian_1.ps == 0)
		qz.lian_1 = 0;
	if (ddz_twolian.ps == 0)
		qz.lian_2 = 0;

	if (qz.double_1 == 0 && qz.lian_1 == 0 && qz.lian_2 == 0 && qz.san_1 == 0 && qz.three_1 == 0 && qz.three_2 == 0)
	{
		return 7;
	}
	////printf("\r\n对子=%f,连顺= %f，双连顺= %f，散= %f，三代1= %f，飞机=  %f \r\n", qz.double_1, qz.lian_1, qz.lian_2, qz.san_1, qz.three_1, qz.three_2);
	if (qz.double_1 >= qz.lian_1&&qz.double_1 >= qz.lian_2&&qz.double_1 >= qz.san_1&&qz.double_1 >= qz.three_1&&qz.double_1 >= qz.three_2)
		return 1;
	if (qz.lian_1 >= qz.double_1&&qz.lian_1 >= qz.lian_2&&qz.lian_1 >= qz.san_1&&qz.lian_1 >= qz.three_1&&qz.lian_1 >= qz.three_2)
		return 2;
	if (qz.lian_2 >= qz.lian_1&&qz.lian_2 >= qz.double_1&&qz.lian_2 >= qz.san_1&&qz.lian_2 >= qz.three_1&&qz.lian_2 >= qz.three_2)
		return 3;
	if (qz.san_1 >= qz.lian_1&&qz.san_1 >= qz.double_1&&qz.san_1 >= qz.lian_2&&qz.san_1 >= qz.three_1&&qz.san_1 >= qz.three_2)
		return 4;
	if (qz.three_1 >= qz.lian_1&&qz.three_1 >= qz.double_1&&qz.three_1 >= qz.lian_2&&qz.three_1 >= qz.san_1&&qz.three_1 >= qz.three_2)
		return 5;
	if (qz.three_2 >= qz.lian_1&&qz.three_2 >= qz.double_1&&qz.three_2 >= qz.lian_2&&qz.three_2 >= qz.san_1&&qz.three_2 >= qz.three_1)
		return 6;


	return 7;
}
bool  BOTProces::Set_FX_JDSP(int OUT_CARD)//分析绝对手牌
{

	SPFX = { 0 };
	JD_JL = { 0 };
	//cheatCardData = { 0 };
	int dpjs = 0;
	int spjs = 0;
Get_zd();
if (Bankxin == 1)
{
	if (ddz_max.ps > 0)
	{
		SPFX.SP_SH += 1;
		if (ddz_max.maxpxjs == 3)
		{
			SPFX.JD_DP += 1;
			
			JD_JL.JD_DPPX[dpjs] = 2;
			dpjs++;
		}
		if (ddz_max.maxpxjs == 1)
		{
			if (onCard.Count == 0 && DownCard.Count == 0)
			{
				SPFX.JD_DP += 1;
				JD_JL.JD_DPPX[dpjs] = 3;
				dpjs++;
			}
			else
			{
				SPFX.JD_SH += 1;
				JD_JL.JD_SHPX[spjs] = 1;
				spjs++;
			}
		}


		if (ddz_max.maxpxjs == 2)
		{
			int is = 0;
			for (int i = 0; i < 20; i++)
			{
				if (onCard.Cdata[i] == 15 || DownCard.Cdata[i] == 15)
				{
					is = 1;
				}
			}
			if (is == 0)
			{
				if (onCard.Count == 0 && DownCard.Count == 0)
				{
					SPFX.JD_DP += 1;
					JD_JL.JD_DPPX[dpjs] = 4;
					dpjs++;
				}
				else
				{
					SPFX.JD_SH += 1;
					JD_JL.JD_SHPX[spjs] = 2;
					spjs++;
				}
			}
		}


	}


	if (ddz_two.ps > 0)
	{
		if(ddz_two.ps!=4)
			SPFX.SP_SH += 1;
		if (ddz_two.ps == 4)
		{
			SPFX.SP_SH += 1;
			SPFX.JD_DP += 1;
			JD_JL.JD_DPPX[dpjs] = 7;
			dpjs++;
		
		}else if (ddz_two.ps > 0 && onCard.Count == 0 && DownCard.Count == 0&&ddz_two.ps!=4)
		{
			int is = 0;
			for (int i = 0; i < 20; i++)
			{
				if (onCard.Cdata[i] == 14 || onCard.Cdata[i] ==2 || onCard.Cdata[i] == 15 || DownCard.Cdata[i] == 15 || DownCard.Cdata[i] == 2 || DownCard.Cdata[i] == 14)
				{
					is = 1;
				}
			}
			if (is == 0)
			{
				JD_JL.JD_DPPX[dpjs] = 5;
				dpjs++;
				SPFX.JD_DP += 1;
			}
			else
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (onCard.Cdata[i] == 14 || onCard.Cdata[i] == 15)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					JD_JL.JD_DPPX[dpjs] = 5;
					dpjs++;
					SPFX.JD_DP += 1;
				}
			}
		}

	}

	if (ddz_one.ps > 0)
	{

		if (ddz_one.ps == 4)
		{
			SPFX.JD_DP += 1;
			SPFX.SP_SH += 1;
			JD_JL.JD_DPPX[dpjs] = 8;
			dpjs++;

		}
		else
		{
			SPFX.SP_SH += 1;
			int onC2 = 0;
			int downC = 0;
			int onC1 = 0;
			int down1 = 0;
			for (int i = 0; i < 20; i++)
			{
				if (onCard.Cdata[i] == 2)
				{
					onC2++;

				}
				if (onCard.Cdata[i] == 1)
				{
					onC1++;

				}
				if (DownCard.Cdata[i] == 2)
				{
					downC++;

				}
				if (DownCard.Cdata[i] == 1)
				{
					down1++;

				}
			}

			if (downC == 0 && onC2 == 0 && down1 == 0 && onC1 == 0 && onCard.Count == 0 && DownCard.Count == 0)
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (onCard.Cdata[i] == 14 || onCard.Cdata[i] == 15 || DownCard.Cdata[i] == 15 || DownCard.Cdata[i] == 14)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					SPFX.JD_DP += 1;
					JD_JL.JD_DPPX[dpjs] = 6;
					dpjs++;
				}

			}
			else	if (downC < ddz_one.ps&&onC2 < ddz_one.ps&& onCard.Count == 0 && DownCard.Count == 0)
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (onCard.Cdata[i] == 14 || onCard.Cdata[i] == 15 || DownCard.Cdata[i] == 15 || DownCard.Cdata[i] == 14)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					SPFX.JD_SH += 1;
					JD_JL.JD_SHPX[spjs] = 4;
					spjs++;
				}

			}


		}
	}
	if (ddz_twolian.lianpxjs > 0 && onCard.Count == 0 && DownCard.Count == 0)
	{
		SPFX.SP_SH += ddz_twolian.lianpxjs;
		SPFX.JD_SH += ddz_twolian.lianpxjs;
		JD_JL.JD_SHPX[spjs] = 6;
		spjs++;
	}else
	{
		SPFX.SP_SH += ddz_twolian.lianpxjs;
	}
	if (ddz_zd.zdpxjs > 0)
	{
		SPFX.SP_SH += ddz_zd.zdpxjs;
		SPFX.JD_DP += ddz_zd.zdpxjs;
		JD_JL.JD_DPPX[dpjs] = 1;
		dpjs++;
	}
	if (ddz_lian_1.lian_1 >5 && onCard.Count == 0 && DownCard.Count == 0)
	{
		SPFX.SP_SH += ddz_lian_1.lianpxjs;
		SPFX.JD_SH += 1;
		JD_JL.JD_SHPX[spjs] = 5;
		spjs++;
	}else if(ddz_lian_1.lian_1>0)
           	{
		if (ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1] == 1)
		{
			SPFX.JD_SH += 1;
			JD_JL.JD_SHPX[spjs] = 5;
			spjs++;
		}

		if (ddz_lian_1.ddzlian_2[ddz_lian_1.lian_2 - 1] == 1)
		{
			SPFX.JD_SH += 1;
			JD_JL.JD_SHPX[spjs] = 5;
			spjs++;
		}
		SPFX.SP_SH += ddz_lian_1.lianpxjs;
	}



	if (ddz_san.ps > 0) SPFX.SP_SH += ddz_san.ps;
	if (ddz_two_fx.two_js > 0) SPFX.SP_SH += ddz_two_fx.two_js;


   if (ddz_san.ps==0&&ddz_three.three_js>0&& ddz_two_fx.ps == 0)
	{
		 SPFX.SP_SH+=ddz_three.three_js;
   }
   if (ddz_one.ps ==3 &&pssj_vh>3)
   {
	   SPFX.SP_SH-=1;
   }
   if (ddz_two.ps == 3 && pssj_vh > 3)
   {
	   SPFX.SP_SH -= 1;
   }
   Set_SHHB();
   if (SPFX.SP_SH == -1 || SPFX.SP_SH > 20)
   {
	   SPFX.SP_SH = 0;
	   return false;
   }
   if (ddz_fj.feiji_js > 0 && onCard.Count == 0 && DownCard.Count == 0&&SPFX.SP_SH!=1)
   {

		   SPFX.SP_SH -= ddz_fj.feiji_js;
		  SPFX.JD_SH += ddz_fj.feiji_js;
	     JD_JL.JD_SHPX[spjs] = 7;
	     spjs++;
   }
	//printf("\r\n剩余->%d手,绝对大牌->%d,绝对手牌->%d\r\n", SPFX.SP_SH, SPFX.JD_DP, SPFX.JD_SH);
	int ipsx = 0;
	ipsx = SPFX.JD_DP * 2;
	if (SPFX.JD_SH > 0)
	{
		if (SPFX.JD_SH > 1)
		{
			ipsx += SPFX.JD_SH + 1;
		}
		else
		{
			ipsx += SPFX.JD_SH;
		}
	}
	if (SPFX.JD_DP > 0)
	{
		if (OUT_CARD == 0)
		{
			ipsx += 1;
		}
	}
	if (SPFX.SP_SH == ipsx || ipsx > SPFX.SP_SH)  //该我出牌
	{

		//printf("\r\n__________________________地主机器人绝对赢牌______________________________\r\n");

		return true;
	}
	if (SPFX.SP_SH == 1&& OUT_CARD==0)
	{

		//printf("\r\n__________________________地主机器人绝对赢牌_____________________________\r\n");
	
		return true;
	}

}
if (Bankxin == 2)//上家地主
{
	if (ddz_max.ps > 0)
	{
		SPFX.SP_SH += 1;
		if (ddz_max.maxpxjs == 3)
		{

			SPFX.JD_DP += 1;
			JD_JL.JD_DPPX[dpjs] = 2;
			dpjs++;
		}
		if (ddz_max.maxpxjs == 1)
		{
			if (onCard.Count == 0 )
			{
				SPFX.JD_DP += 1;
				JD_JL.JD_DPPX[dpjs] = 3;
				dpjs++;
			}
			else
			{
				SPFX.JD_SH += 1;
				JD_JL.JD_SHPX[spjs] = 1;
				spjs++;
			}
		}


		if (ddz_max.maxpxjs == 2)
		{
			int is = 0;
			for (int i = 0; i < 20; i++)
			{
				if (onCard.Cdata[i] == 15 )
				{
					is = 1;
				}
			}
			if (is == 0)
			{
				if (onCard.Count == 0)
				{
					SPFX.JD_DP += 1;
					JD_JL.JD_DPPX[dpjs] = 4;
					dpjs++;
				}
				else
				{
					SPFX.JD_SH += 1;
					JD_JL.JD_SHPX[spjs] = 2;
					spjs++;
				}
			}
		}


	}
	if (ddz_two.ps > 0)
	{
		SPFX.SP_SH += 1;
		if (ddz_two.ps == 4)
		{
			SPFX.JD_DP += 1;
	JD_JL.JD_DPPX[dpjs] = 7;
			dpjs++;
		
		}else if (ddz_two.ps > 0 && onCard.Count == 0)
		{
			int is = 0;
			for (int i = 0; i < 20; i++)
			{
				if (onCard.Cdata[i]==14||onCard.Cdata[i]==15||onCard.Cdata[i]== 2)
				{
					is = 1;
				}
			}
			if (is == 0)
			{
				JD_JL.JD_DPPX[dpjs] = 5;
				dpjs++;
				SPFX.JD_DP += 1;
			}else
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (onCard.Cdata[i] == 14 || onCard.Cdata[i] == 15 )
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					JD_JL.JD_DPPX[dpjs] = 5;
					dpjs++;
					SPFX.JD_DP += 1;
				}
			
			}
		}

	}

	if (ddz_one.ps > 0)
	{
		SPFX.SP_SH += 1;
		if (ddz_one.ps == 4)
		{
			SPFX.JD_DP += 1;
			JD_JL.JD_DPPX[dpjs] = 8;
			dpjs++;
			
		}
		else
		{
			int onC2 = 0;
			int onC1 = 0;
			for (int i = 0; i < 20; i++)
			{
				if (onCard.Cdata[i] == 2)
				{
					onC2++;

				}
				if (onCard.Cdata[i] == 1)
				{
					onC1++;

				}
			}

			if (onC2 == 0 && onC1 == 0 && onCard.Count == 0)
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (onCard.Cdata[i] == 14 || onCard.Cdata[i] == 15)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					SPFX.JD_DP += 1;
					JD_JL.JD_DPPX[dpjs] = 6;
					dpjs++;
				}

			}
			else	if (onC2 < ddz_one.ps&& onCard.Count == 0)
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (onCard.Cdata[i] == 14 || onCard.Cdata[i] == 15)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					SPFX.JD_SH += 1;
					JD_JL.JD_SHPX[spjs] = 4;
					spjs++;
				}

			}
		}

	}
	if (ddz_one.ps == 1 && OnGameCard == 1 && onCard.Cdata[0] == 1)
	{
		SPFX.JD_SH += 1;
		JD_JL.JD_SHPX[spjs] = 4;
		spjs++;
	}


	if (ddz_twolian.lianpxjs > 0 && onCard.Count == 0 )
	{
		SPFX.SP_SH += ddz_twolian.lianpxjs;
		SPFX.JD_SH += ddz_twolian.lianpxjs;
		JD_JL.JD_SHPX[spjs] = 6;
		spjs++;
	}else
	{
		SPFX.SP_SH += ddz_twolian.lianpxjs;
	}
	if (ddz_zd.zdpxjs > 0)
	{
		SPFX.SP_SH += ddz_zd.zdpxjs;
		SPFX.JD_DP += ddz_zd.zdpxjs;
		JD_JL.JD_DPPX[dpjs] = 1;
		dpjs++;
	}
	if (ddz_lian_1.lian_1 > 5 && onCard.Count == 0 )
	{
		SPFX.SP_SH += ddz_lian_1.lianpxjs;
		SPFX.JD_SH += 1;
		JD_JL.JD_SHPX[spjs] = 5;
		spjs++;
	}else if (ddz_lian_1.lian_1>0)
	{
		if (ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1] == 1)
		{
			SPFX.JD_SH += 1;
			JD_JL.JD_SHPX[spjs] = 5;
			spjs++;
		}

		if (ddz_lian_1.ddzlian_2[ddz_lian_1.lian_2 - 1] == 1)
		{
			SPFX.JD_SH += 1;
			JD_JL.JD_SHPX[spjs] = 5;
			spjs++;
		}
		SPFX.SP_SH += ddz_lian_1.lianpxjs;
	}
	if (ddz_san.ps > 0) SPFX.SP_SH += ddz_san.ps;
	if (ddz_two_fx.two_js > 0) SPFX.SP_SH += ddz_two_fx.two_js;

	if (ddz_san.ps == 0 && ddz_three.three_js>0 && ddz_two_fx.ps == 0)
	{
		SPFX.SP_SH += ddz_three.three_js;
	}
	if (ddz_three.three_js > 0)
	{
	//	SPFX.SP_SH += ddz_three.three_js - (ddz_san.ps + ddz_two_fx.two_js);
	}
	if (ddz_one.ps == 3 && pssj_vh>3)
	{
		SPFX.SP_SH -= 1;
	}
	if (ddz_two.ps == 3 && pssj_vh > 3)
	{
		SPFX.SP_SH -= 1;
	}
	Set_SHHB();
	if (SPFX.SP_SH == -1 || SPFX.SP_SH > 20)
	{
		SPFX.SP_SH = 0;
		return false;
	}
	if (ddz_fj.feiji_js > 0 && onCard.Count == 0&&SPFX.SP_SH!=1)
	{
		SPFX.SP_SH -= ddz_fj.feiji_js;
		SPFX.JD_SH += ddz_fj.feiji_js;
		JD_JL.JD_SHPX[spjs] = 7;
		spjs++;
	}
	else
	{
		SPFX.SP_SH -= ddz_fj.feiji_js;
	}
	//printf("\r\n剩余->%d手,绝对大牌->%d,绝对手牌->%d\r\n", SPFX.SP_SH, SPFX.JD_DP, SPFX.JD_SH);
	int ipsx = 0;
	ipsx = SPFX.JD_DP * 2;
	if (SPFX.JD_SH > 0)
	{
		ipsx += SPFX.JD_SH + 1;
	}
	if (SPFX.JD_DP > 0)
	{
		if (OUT_CARD == 0)
		{
			ipsx += 1;
		}
	}
	if (SPFX.SP_SH == ipsx || ipsx > SPFX.SP_SH)  //该我出牌
	{
	    if(BotID==1)
		//printf("\r\n__________________________农民机器人[1号]绝对赢牌绝对赢牌______________________________\r\n");

		if (BotID == 2)
		//printf("\r\n__________________________农民机器人[2号]绝对赢牌绝对赢牌______________________________\r\n");
		return true;
	}
	if (SPFX.SP_SH == 1 && OUT_CARD == 0)
	{

		if (BotID == 1)
			//printf("\r\n__________________________农民机器人[1号]绝对赢牌绝对赢牌______________________________\r\n");

		if (BotID == 2)
		//printf("\r\n__________________________农民机器人[2号]绝对赢牌绝对赢牌______________________________\r\n");

		return true;
	}
}
if (Bankxin == 3)//下家地主
{
	if (ddz_max.ps > 0)
	{
		SPFX.SP_SH += 1;
		if (ddz_max.maxpxjs == 3)
		{
	
			SPFX.JD_DP += 1;
			JD_JL.JD_DPPX[dpjs] = 2;
			dpjs++;
		}
		if (ddz_max.maxpxjs == 1)
		{
			if (DownCard.Count == 0)
			{
				SPFX.JD_DP += 1;
				JD_JL.JD_DPPX[dpjs] = 3;
				dpjs++;
			}
			else
			{
				SPFX.JD_SH += 1;
				JD_JL.JD_SHPX[spjs] = 1;
				spjs++;
			}
		}


		if (ddz_max.maxpxjs == 2)
		{
			int is = 0;
			for (int i = 0; i < 20; i++)
			{
				if (DownCard.Cdata[i] == 15)
				{
					is = 1;
				}
			}
			if (is == 0)
			{
				if ( DownCard.Count == 0)
				{
					SPFX.JD_DP += 1;
					JD_JL.JD_DPPX[dpjs] = 4;
					dpjs++;
				}
				else
				{
					SPFX.JD_SH += 1;
					JD_JL.JD_SHPX[spjs] = 2;
					spjs++;
				}
			}
		}


	}


	if (ddz_two.ps > 0)
	{
		SPFX.SP_SH += 1;
		if (ddz_two.ps == 4)
		{
			SPFX.JD_DP += 1;
JD_JL.JD_DPPX[dpjs] = 7;
			dpjs++;
	
		}	else if (ddz_two.ps > 0 && DownCard.Count == 0)
		{
			int is = 0;
			for (int i = 0; i < 20; i++)
			{
				if ( DownCard.Cdata[i] == 15 || DownCard.Cdata[i] == 14||DownCard.Cdata[i]==2)
				{
					is = 1;
				}
			}
			if (is == 0)
			{
				JD_JL.JD_DPPX[dpjs] = 5;
				dpjs++;
				SPFX.JD_DP += 1;
			}
			else
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (onCard.Cdata[i] == 14 || onCard.Cdata[i] == 15)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					JD_JL.JD_DPPX[dpjs] = 5;
					dpjs++;
					SPFX.JD_DP += 1;
				}
			}
		}

	}

	if (ddz_one.ps > 0)
	{
		SPFX.SP_SH += 1;
		if (ddz_one.ps == 4)
		{
			SPFX.JD_DP += 1;
			JD_JL.JD_DPPX[dpjs] = 8;
			dpjs++;
		
		}
		else
		{
			int downC = 0;
			int down1 = 0;
			for (int i = 0; i < 20; i++)
			{

				if (DownCard.Cdata[i] == 2)
				{
					downC++;
				}
				if (DownCard.Cdata[i] == 1)
				{
					downC++;
				}
			}

			if (downC == 0 && down1 == 0 && DownCard.Count == 0)
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (DownCard.Cdata[i] == 15 || DownCard.Cdata[i] == 14)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					SPFX.JD_DP += 1;
					JD_JL.JD_DPPX[dpjs] = 6;
					dpjs++;
				}

			}
			else	if (downC < ddz_one.ps&& DownCard.Count == 0)
			{
				int is = 0;
				for (int i = 0; i < 20; i++)
				{
					if (DownCard.Cdata[i] == 15 || DownCard.Cdata[i] == 14)
					{
						is = 1;
					}
				}
				if (is == 0)
				{
					SPFX.JD_SH += 1;
					JD_JL.JD_SHPX[spjs] = 4;
					spjs++;
				}

			}

		}
	}
	if (ddz_one.ps == 1 && DownGameCard == 1 && DownCard.Cdata[0] == 1)
	{
		SPFX.JD_SH += 1;
		JD_JL.JD_SHPX[spjs] = 4;
		spjs++;
	}

	if (ddz_twolian.lianpxjs > 0 &&DownCard.Count == 0)
	{
		SPFX.SP_SH += ddz_twolian.lianpxjs;
		SPFX.JD_SH += ddz_twolian.lianpxjs;
		JD_JL.JD_SHPX[spjs] = 6;
		spjs++;
	}else
	{
		SPFX.SP_SH += ddz_twolian.lianpxjs;
	}
	if (ddz_zd.zdpxjs > 0)
	{
		SPFX.SP_SH += ddz_zd.zdpxjs;
		SPFX.JD_DP += ddz_zd.zdpxjs;
		JD_JL.JD_DPPX[dpjs] = 1;
		dpjs++;
	}
	if (ddz_lian_1.lian_1 > 5&& DownCard.Count == 0)
	{
		SPFX.SP_SH += ddz_lian_1.lianpxjs;
		SPFX.JD_SH += 1;
		JD_JL.JD_SHPX[spjs] = 5;
		spjs++;
	}else if (ddz_lian_1.lian_1>0)
	{
		if (ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1] == 1)
		{
			SPFX.JD_SH += 1;
			JD_JL.JD_SHPX[spjs] = 5;
			spjs++;
		}

		if (ddz_lian_1.ddzlian_2[ddz_lian_1.lian_2 - 1] == 1)
		{
			SPFX.JD_SH += 1;
			JD_JL.JD_SHPX[spjs] = 5;
			spjs++;
		}
		SPFX.SP_SH += ddz_lian_1.lianpxjs;
	}
	if (ddz_san.ps > 0) SPFX.SP_SH += ddz_san.ps;
	if (ddz_two_fx.two_js > 0) SPFX.SP_SH += ddz_two_fx.two_js;

	if (ddz_san.ps == 0 && ddz_three.three_js>0 && ddz_two_fx.ps == 0)
	{
		SPFX.SP_SH += ddz_three.three_js;
	}
	if (ddz_one.ps == 3 && pssj_vh > 3)
	{
		SPFX.SP_SH -= 1;
	}
	if (ddz_two.ps == 3 && pssj_vh > 3)
	{
		SPFX.SP_SH -= 1;
	}
	Set_SHHB();
	if (SPFX.SP_SH == -1 ||SPFX.SP_SH > 20)
	{
		SPFX.SP_SH = 0;
		return false;
	}
	if (ddz_fj.feiji_js > 0 && DownCard.Count == 0 && SPFX.SP_SH != 1)
	{
		SPFX.SP_SH -= ddz_fj.feiji_js;
		SPFX.JD_SH += ddz_fj.feiji_js;
		JD_JL.JD_SHPX[spjs] = 7;
		spjs++;
	}
	else
	{
		SPFX.SP_SH -= ddz_fj.feiji_js;
	}
	//printf("\r\n剩余->%d手,绝对大牌->%d,绝对手牌->%d\r\n", SPFX.SP_SH, SPFX.JD_DP, SPFX.JD_SH);
	int ipsx = 0;
	ipsx = SPFX.JD_DP * 2;
	if (SPFX.JD_SH > 0)
	{
		ipsx += SPFX.JD_SH + 1;
	}
	if (SPFX.JD_DP > 0)
	{
		if (OUT_CARD == 0)
		{
			ipsx += 1;
		}
	}
	if (SPFX.SP_SH == ipsx || ipsx > SPFX.SP_SH)  //该我出牌
	{

		if (BotID == 1)
			//printf("\r\n__________________________农民机器人[1号]绝对赢牌绝对赢牌______________________________\r\n");

		if (BotID == 2)
			//printf("\r\n__________________________农民机器人[2号]绝对赢牌绝对赢牌______________________________\r\n");

		return true;
	}
	if (SPFX.SP_SH == 1 && OUT_CARD == 0)
	{

		if (BotID == 1)
			//printf("\r\n__________________________农民机器人[1号]绝对赢牌绝对赢牌______________________________\r\n");

		if (BotID == 2)
			//printf("\r\n__________________________农民机器人[2号]绝对赢牌绝对赢牌______________________________\r\n");

		return true;
	}
}
	return false ;
}
bool BOTProces::Set_SINGLE1(CT_BYTE cbCardData1[MAX_COUNT], CT_DWORD gameuserid, CT_DWORD SX_Card)
{ 
	//gameuserid,1为我上家，2为我下家
	//	int Bankxin; //1为自己地主，2为上家地主，3为下家地主
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));
	bool icont = false;
	CT_BYTE cond_l = 0;
	if (Bankxin == 1)
	{

//------------------------------------
		if (OnGameCard == 1 ||DownGameCard == 1)
		{
			if (DownGameCard == 1 && ddz_san.ps > 0 && ddz_san.ddz_san_1[ddz_san.ps - 1] > DownCard.Cdata[0]&& DownCard.Cdata[0]!=1&& DownCard.Cdata[0]!=2&& ddz_san.ddz_san_1[ddz_san.ps - 1]>ddz_old.old_Card_data[0]&&ddz_old.old_Card_data[0]!=2&&ddz_old.old_Card_data[0] != 1)
			{
	
     ddz_san.ps -= 1;
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps];
				ddz_san.ddz_san_1[ddz_san.ps] = 0;
				return true;
			}
			else if (OnGameCard == 1 && ddz_san.ps > 0 && ddz_san.ddz_san_1[ddz_san.ps - 1] > onCard.Cdata[0] && onCard.Cdata[0] != 1 && onCard.Cdata[0] != 2&& ddz_san.ddz_san_1[ddz_san.ps - 1]>ddz_old.old_Card_data[0] && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 1)
			{
				ddz_san.ps -= 1;
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps];
				ddz_san.ddz_san_1[ddz_san.ps] = 0;
				return true;
			}
			if (ddz_two.ps > 0 && ddz_two.ps != 4)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
				{
			
					ddz_two.ps -= 1;
					ddz_two.ddztwo_1[ddz_two.ps] = 0;
					ddz_two.twopxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 2;
					icont = true;
					return true;
				}
		    }
			if (ddz_max.maxpxjs==2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
			{

					ddz_max.ddzmax_2 = 0;
					ddz_max.ps -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 14;
	         	ddz_max.maxpxjs = 0;
			return true;
	}
		  if(ddz_max.maxpxjs==1)
		{
		     ddz_max.ddzmax_1 = 0;
				ddz_max.ps -= 1;
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = 15;
				ddz_max.maxpxjs = 0;
				return true;

			
		}
			if (ddz_one.ps > 0 && ddz_one.ps != 4)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
				{
					cond_l = 1;
					ddz_one.ps -= 1;
					ddz_one.ddzone_1[ddz_one.ps] = 0;
					ddz_one.onepxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}
	
		}
		if (cbCardData1[0] != 1 && cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
		{
			if (ddz_san.ps > 0)
			{
				for (int i = 0; i < 20; i++)
				{
					if (ddz_san.ddz_san_1[i] > cbCardData1[0])
					{
						cond_l = ddz_san.ddz_san_1[i];
						ddz_san.ddz_san_1[i] = 0;
						ddz_san.ps -= 1;
						       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						for (int is = i; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

						}
						icont = true;
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = cond_l;
					

						return true;
					}
				}
			}
		}

		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd=	Set_JdOutCard(1, 1);
			if (abdd == true)
			{
				return true;
			}
	
		}
		if (icont == false)
		{
			if (ddz_two.ps == 1)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
				{
					cond_l = 2;
					ddz_two.ps -= 1;
					ddz_two.ddztwo_1[0] = 0;
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					ddz_two.twopxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}
			if (ddz_one.ps == 1)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
				{
					cond_l = 1;
					ddz_one.ps -= 1;
					ddz_one.ddzone_1[ddz_one.ps] = 0;
					ddz_one.onepxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}
		}
		if (OnGameCard < 10 || DownGameCard < 10)
		{
			if (ddz_one.ps == 3)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
				{
					cond_l = 1;
					ddz_one.ps -= 1;
					ddz_one.ddzone_1[ddz_one.ps] = 0;
					ddz_one.onepxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}
			if (ddz_two.ps == 3)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
				{
					cond_l = 2;
					ddz_two.ps -= 1;
					ddz_two.ddztwo_1[ddz_two.ps] = 0;
					ddz_two.twopxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}

			}
			if (ddz_two_fx.two_js <4)
			{
				if (ddz_one.ps == 2)
				{
					if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
					{
						cond_l = 1;
						ddz_one.ps -= 1;
						ddz_one.ddzone_1[ddz_one.ps] = 0;
						ddz_one.onepxjs -= 1;
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = cond_l;
						icont = true;
						return true;
					}
				}
			}

			if (ddz_two_fx.two_js <4)
			{
				if (ddz_two.ps == 2)
				{
					if (ddz_max.maxpxjs != 3 && ddz_max.ddzmax_2 > 0)
					{
						if (cbCardData1[0] != 15 && cbCardData1[0] != 14)
						{
							cond_l = 14;
							ddz_max.ddzmax_2 = 0;
							ddz_max.ps -= 1;
							ddz_max.maxpxjs = 0;
							mOutCard.cbCardCount = 1;
							mOutCard.cbCardData[0] = cond_l;
							icont = true;
							return true;
						}
					}
					if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
					{
						cond_l = 2;
						ddz_two.ps -= 1;
						ddz_two.ddztwo_1[ddz_two.ps] = 0;
						ddz_two.twopxjs -= 1;
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = cond_l;
						icont = true;
						return true;
					}
				}
			}
		}
		if (OnGameCard < 7 || DownGameCard < 7)
		{
	
				if (ddz_one.ps == 2)
				{
					if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
					{
						cond_l = 1;
						ddz_one.ps -= 1;
						ddz_one.ddzone_1[ddz_one.ps] = 0;
						ddz_one.onepxjs -= 1;
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = cond_l;
						icont = true;
						return true;
					}
				
			}

			if (ddz_two_fx.two_js <4)
			{
				if (ddz_two.ps == 2)
				{
					if (ddz_max.maxpxjs != 3 && ddz_max.ddzmax_2 > 0)
					{
						if (cbCardData1[0] != 15 && cbCardData1[0] != 14)
						{
							cond_l = 14;
							ddz_max.ddzmax_2 = 0;
							ddz_max.ps -= 1;
							ddz_max.maxpxjs = 0;
							mOutCard.cbCardCount = 1;
							mOutCard.cbCardData[0] = cond_l;
							icont = true;
							return true;
						}
					}
					if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
					{
						cond_l = 2;
						ddz_two.ps -= 1;
						ddz_two.ddztwo_1[ddz_two.ps] = 0;
						ddz_two.twopxjs -= 1;
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = cond_l;
						icont = true;
						return true;
					}
				}
		
			}
			if (ddz_max.maxpxjs != 3 && ddz_max.ddzmax_2 > 0)
			{
				if (cbCardData1[0] != 15 && cbCardData1[0] != 14)
				{
					cond_l = 14;
					ddz_max.ddzmax_2 = 0;
					ddz_max.ps -= 1;
					ddz_max.maxpxjs = 0;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}


		}
		if (OnGameCard <6 || DownGameCard < 6)
		{
			if (ddz_two.ps == 2)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
				{
					cond_l = 2;
					ddz_two.ps -= 1;
					ddz_two.ddztwo_1[ddz_two.ps] = 0;
					ddz_two.twopxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}
			if (ddz_one.ps == 4)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
				{
					cond_l = 1;
					ddz_one.ps -= 1;
					ddz_one.ddzone_1[ddz_one.ps] = 0;
					ddz_one.onepxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}
			if (ddz_two.ps == 4)
			{
				if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
				{
					cond_l = 2;
					ddz_two.ps -= 1;
					ddz_two.ddztwo_1[ddz_two.ps] = 0;
					ddz_two.twopxjs -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
					icont = true;
					return true;
				}
			}
		}
		if (OnGameCard < 4 || DownGameCard < 4)
		{
			if (ddz_max.ddzmax_1 > 0 && ddz_max.maxpxjs != 3)
			{

				cond_l = 15;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ps -= 1;
				ddz_max.maxpxjs = 0;
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = cond_l;
				icont = true;
				return true;
			}

		}

		if (OnGameCard < 3 || DownGameCard < 3)
		{
			if (ddz_zd.zdpxjs > 0)
			{
				bool ad = Set_Card_OUT();
				if (!ad)
				{
					return false;
				}
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
				mOutCard.cbCardCount = 4;
				ddz_zd.ps -= 4;
				ddz_zd.zdpxjs -= 1;
				memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
				memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
				memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
				memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
				return true;
			}
		}
}//------------------


//________________________________________________________________________________________

	
//------------------------------------------------------------------------------------------ 上家地主,朋友出牌
if (Bankxin == 2 && ddz_old.old_Card == 2)
{
	if (cbCardData1[0] != 1 && cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
	{
		if (ddz_san.ps > 0)
		{
			for (int i = 0; i < 20; i++)
			{
				if (ddz_san.ddz_san_1[i] > cbCardData1[0])
				{
					cond_l = ddz_san.ddz_san_1[i];
					ddz_san.ddz_san_1[i] = 0;
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int is = i; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					icont = true;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
				

					return true;
				}
			}
		}

	}
	if (jdyp == true)
	{
		//printf("必胜牌打法\r\n");
		bool abdd = Set_JdOutCard(1, 1);
		if (abdd == true)
		{
			return true;
		}
	

	}
	return false;
}

//------------------------------------------------------------------------上家地主 上家出牌
if (Bankxin == 2 && ddz_old.old_Card == 1)
{
	if (cbCardData1[0] != 1 && cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
	{
		if (ddz_san.ps > 0)
		{
			for (int i = 0; i < ddz_san.ps; i++)
			{
				if (ddz_san.ddz_san_1[i] > cbCardData1[0])
				{
					cond_l = ddz_san.ddz_san_1[i];
					ddz_san.ddz_san_1[i] = 0;
					ddz_san.ps -= 1;
					      //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int is = i; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					ddz_san.ddz_san_1[ddz_san.ps] = 0;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
				

					return true;
				}
			}
		}
	
	}
	if (jdyp == true)
	{
		//printf("必胜牌打法\r\n");
		bool abdd = Set_JdOutCard(1, 1);
		if (abdd == true)
		{
			return true;
		}
	
	}
	if (ddz_two.ps != 4&&ddz_two.ps>0)
	{
		if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
		{
	
			ddz_two.ps -= 1;
			ddz_two.ddztwo_1[ddz_two.ps] = 0;
			ddz_two.twopxjs -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = 2;
			return true;
		}
	}
	if (ddz_one.ps != 4&&ddz_one.ps>0)
	{
		if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
		{
			cond_l = 1;
			ddz_one.ps -= 1;
			ddz_one.ddzone_1[ddz_one.ps] = 0;
			ddz_one.onepxjs -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = cond_l;
			icont = true;
			return true;
		}
	}
	if (ddz_max.maxpxjs != 3 && ddz_max.ddzmax_2 > 0)
	{
		if (cbCardData1[0] != 15 && cbCardData1[0] != 14)
		{
			cond_l = 14;
			ddz_max.ddzmax_2 = 0;
			ddz_max.ps -= 1;
			ddz_max.maxpxjs = 0;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = cond_l;
			icont = true;
			return true;
		}
	}
	if (ddz_max.ddzmax_1 > 0 && ddz_max.maxpxjs != 3)
	{

		cond_l = 15;
		ddz_max.ddzmax_1 = 0;
		ddz_max.ps -= 1;
		ddz_max.maxpxjs = 0;
		mOutCard.cbCardCount = 1;
		mOutCard.cbCardData[0] = cond_l;
		icont = true;
		return true;
	}
	if (OnGameCard == 1&&onCard.Cdata[0]!=14&& onCard.Cdata[0] != 15 && onCard.Cdata[0]!=2 && onCard.Cdata[0] != 1)
	{
		bool as = Set_DOUBLE_1(8, cbCardData1);
		if (as == true)
			return true;


	}

}
//------------------------------------------------------------------------下家地主 下家出牌
//	CT_DWORD old_Card; //0，自己，1，上家，2下家
if (Bankxin == 3 && ddz_old.old_Card == 2)
{
	if (jdyp == true)
	{
		//printf("必胜牌打法\r\n");
		bool abdd = Set_JdOutCard(1, 1);
		if (abdd == true)
		{
			return true;
		}

	}
	if (ddz_two.ps != 4 && ddz_two.ps>0)
	{
		if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
		{

			ddz_two.ps -= 1;
			ddz_two.ddztwo_1[ddz_two.ps] = 0;
			ddz_two.twopxjs -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = 2;
			return true;
		}
	}

	if (ddz_one.ps != 4&&ddz_one.ps>0)
	{
		if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
		{
			cond_l = 1;
			ddz_one.ps -= 1;
			ddz_one.ddzone_1[ddz_one.ps] = 0;
			ddz_one.onepxjs -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = cond_l;
			icont = true;
			return true;
		}
	}
	if (cbCardData1[0] != 1 && cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
	{
		if (ddz_san.ps > 0)
		{
			for (int i = 0; i < 20; i++)
			{
				if (ddz_san.ddz_san_1[i] > cbCardData1[0])
				{
					cond_l = ddz_san.ddz_san_1[i];
					ddz_san.ddz_san_1[i] = 0;
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int is = i; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = cond_l;
				

					return true;
				}
			}
		}
	}
	if (ddz_max.maxpxjs != 3 && ddz_max.ddzmax_2 > 0)
	{
		if (cbCardData1[0] != 15 && cbCardData1[0] != 14)
		{
			cond_l = 14;
			ddz_max.ddzmax_2 = 0;
			ddz_max.ps -= 1;
			ddz_max.maxpxjs = 0;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = cond_l;
			icont = true;
			return true;
		}
	}
	if (ddz_max.ddzmax_1 > 0 && ddz_max.maxpxjs != 3)
	{

		cond_l = 15;
		ddz_max.ddzmax_1 = 0;
		ddz_max.ps -= 1;
		ddz_max.maxpxjs = 0;
		mOutCard.cbCardCount = 1;
		mOutCard.cbCardData[0] = cond_l;
		icont = true;
		return true;
	}
	if (DownGameCard == 1 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 15 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 1)
	{
		bool as = Set_DOUBLE_1(8, cbCardData1);
		if (as == true)
			return true;

	}


}
//------------------------------------------------------------------------下家地主 上家出牌
if (Bankxin == 3 && ddz_old.old_Card == 1)
{
	if (DownGameCard == 1)
	{
		if (ddz_san.ddz_san_1[ddz_san.ps - 1] > DownCard.Cdata[0]&& DownCard.Cdata[0]!=1&& DownCard.Cdata[0]!=2)
		{
			mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps - 1];
			ddz_san.ddz_san_1[ddz_san.ps - 1] = { 0 };
			mOutCard.cbCardCount = 1;
			ddz_san.ps -= 1;
			//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			return true;
		}
		if (ddz_one.ps != 4 && ddz_one.ps > 0 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 15)
		{
			if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
			{
				cond_l = 1;
				ddz_one.ps -= 1;
				ddz_one.ddzone_1[ddz_one.ps] = 0;
				ddz_one.onepxjs -= 1;
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = cond_l;
				icont = true;
				return true;
			}
		}
		if (ddz_two.ps != 4 && ddz_two.ps > 0 && DownCard.Cdata[0] != 14&& DownCard.Cdata[0] != 15)
		{
			if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15&& cbCardData1[0] != 1)
			{

				ddz_two.ps -= 1;
				ddz_two.ddztwo_1[ddz_two.ps] = 0;
				ddz_two.twopxjs -= 1;
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = 2;
				return true;
			}
		}
		if (ddz_max.maxpxjs != 3 && ddz_old.old_Card_data[0] != 15 && ddz_max.maxpxjs == 2)
		{
			ddz_max.ddzmax_2 = 0;
			ddz_max.maxpxjs = 0;
			ddz_max.ps -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = 14;

			return true;
		}
		if (ddz_max.maxpxjs != 3 && ddz_max.maxpxjs == 1)
		{
			ddz_max.ddzmax_1 = 0;
			ddz_max.maxpxjs = 0;
			ddz_max.ps -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = 15;

			return true;
		}

	}

	if (cbCardData1[0] != 1 && cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
	{
		if (ddz_san.ps>0&&ddz_san.ps<20)
		{
			if (ddz_san.ddz_san_1[ddz_san.ps - 1] > cbCardData1[0])
			{
				mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps - 1];
				ddz_san.ddz_san_1[ddz_san.ps - 1] = { 0 };
				mOutCard.cbCardCount = 1;
				ddz_san.ps -= 1;
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				return true;
			}
		}
	}
	if (jdyp == true)
	{
		//printf("必胜牌打法\r\n");
		bool abdd = Set_JdOutCard(1, 1);
		if (abdd == true)
		{
			return true;
		}
	

	}
	if (ddz_one.ps != 4&& ddz_one.ps>0)
	{
		if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15 && cbCardData1[0] != 1)
		{
			cond_l = 1;
			ddz_one.ps -= 1;
			ddz_one.ddzone_1[ddz_one.ps] = 0;
			ddz_one.onepxjs -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = cond_l;
			icont = true;
			return true;
		}
	}
	if (ddz_two.ps != 4&&ddz_two.ps>0)
	{
		if (cbCardData1[0] != 2 && cbCardData1[0] != 14 && cbCardData1[0] != 15)
		{

			ddz_two.ps -= 1;
			ddz_two.ddztwo_1[ddz_two.ps] = 0;
			ddz_two.twopxjs -= 1;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = 2;
			return true;
		}
	}
	return false;
}



return false;
}
bool BOTProces::Set_DOUBLE(CT_BYTE cbCardDatas[MAX_COUNT], CT_DWORD gameuserid1, CT_DWORD SX_DOUBLE_Card)//对子牌管，数据,ID 上家或者下家，剩余牌
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));
	CT_BYTE cond_l[6];
	memset(cond_l, 0, sizeof(cond_l));

	if (Bankxin == 1)
	{
//--------------------------------------------------------------------------------- //

		if (cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
		{
			if (ddz_two_fx.two_js > 0)
			{
			 bool as=	Set_DOUBLE_1(1, cbCardDatas);
			 if(as == true)
				 return true;
			}
		}
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 2);
			if (abdd == true)
			{
				return true;
			}

		}
		if (OnGameCard < 14 && ddz_old.old_Card == 1)
		{
			if (ddz_one.ps == 2 && cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
			{
				if (ddz_two_fx.two_js < 3)
				{
					memcpy(cond_l, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					mOutCard.cbCardCount = 2;
					ddz_one.onepxjs = 0;
					ddz_one.ps = 0;
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				}
				return true;
			}
		}
		if (OnGameCard < 10 && ddz_old.old_Card == 1)
		{
			if (ddz_one.ps == 2 && cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
			{

				memcpy(cond_l, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				mOutCard.cbCardCount = 2;
				ddz_one.onepxjs = 0;
				ddz_one.ps = 0;
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				return true;
			}

			if (ddz_two.ps == 2  && cbCardDatas[0] != 2)
			{

				memcpy(cond_l, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				mOutCard.cbCardCount = 2;
				ddz_two.twopxjs = 0;
				ddz_two.ps = 0;
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				return true;
			}

		}
		if (DownGameCard < 10 && ddz_old.old_Card == 2)
		{
			if (ddz_one.ps == 2 && cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
			{

				memcpy(cond_l, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				mOutCard.cbCardCount = 2;
				ddz_one.onepxjs = 0;
				ddz_one.ps = 0;
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				return true;
			}

			if (ddz_two.ps == 2 && cbCardDatas[0] != 2)
			{

				memcpy(cond_l, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				mOutCard.cbCardCount = 2;
				ddz_two.twopxjs = 0;
				ddz_two.ps = 0;
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				return true;
			}

		}

		if (OnGameCard < 6 && ddz_old.old_Card == 1)
		{
			if (ddz_one.ps == 3 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
			{

				memcpy(cond_l, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				ddz_one.ddzone_1[2] = { 0 };
				ddz_one.ddzone_1[1] = { 0 };
				mOutCard.cbCardCount = 2;
				ddz_one.onepxjs -= 2;
				ddz_one.ps -= 2;

				return true;
			}

			if (ddz_two.ps == 3 && cbCardDatas[0] != 2)
			{

				memcpy(cond_l, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				ddz_two.ddztwo_1[2] = { 0 };
				ddz_two.ddztwo_1[1] = { 0 };
				mOutCard.cbCardCount = 2;
				ddz_two.twopxjs -= 2;
				ddz_two.ps -= 2;

				return true;
			}
		}
		if (DownGameCard < 6 && ddz_old.old_Card == 2)
		{
			if (ddz_one.ps == 3 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
			{

				memcpy(cond_l, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				ddz_one.ddzone_1[2] = { 0 };
				ddz_one.ddzone_1[1] = { 0 };
				mOutCard.cbCardCount = 2;
				ddz_one.onepxjs -= 2;
				ddz_one.ps -= 2;

				return true;
			}

			if (ddz_two.ps == 3 && cbCardDatas[0] != 2)
			{

				memcpy(cond_l, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				ddz_two.ddztwo_1[2] = { 0 };
				ddz_two.ddztwo_1[1] = { 0 };
				mOutCard.cbCardCount = 2;
				ddz_two.twopxjs -= 2;
				ddz_two.ps -= 2;

				return true;
			}

		}

		if (OnGameCard < 4 || DownGameCard < 4)
		{
			if (ddz_one.ps == 4 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
			{
				memcpy(cond_l, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				ddz_one.ddzone_1[3] = { 0 };
				ddz_one.ddzone_1[2] = { 0 };
				mOutCard.cbCardCount = 2;
				ddz_one.onepxjs -= 2;
				ddz_one.ps -= 2;
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				return true;
			}

			if (ddz_two.ps == 4 && cbCardDatas[0] != 2)
			{

				memcpy(cond_l, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				ddz_two.ddztwo_1[3] = { 0 };
				ddz_two.ddztwo_1[2] = { 0 };
				mOutCard.cbCardCount = 2;
				ddz_two.twopxjs -= 2;
				ddz_two.ps -= 2;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				return true;
			}

			if (DownGameCard < 4 && ddz_old.old_Card == 2 && ddz_san.ps<4)
			{
				bool ad= Set_Card_OUT();
				if (!ad)
				{
					return false;
				}
				if (ddz_zd.zdpxjs > 0)
				{
					memcpy(cond_l, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					mOutCard.cbCardCount = 4;
					ddz_zd.ps -= 4;
					ddz_zd.zdpxjs -= 1;
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					return true;
				}
			}
			if (OnGameCard < 4 && ddz_old.old_Card == 1&&ddz_san.ps<4)
			{//0，自己，1，上家，2下家
				if (ddz_zd.zdpxjs > 0)
				{
					bool ad = Set_Card_OUT();
					if (!ad)
					{
						return false;
					}
					memcpy(cond_l, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					mOutCard.cbCardCount = 4;
					ddz_zd.ps -= 4;
					ddz_zd.zdpxjs -= 1;
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					return true;
				}
			}


		}


		return false;
	}
//	int Bankxin; //1为自己地主，2为上家地主，3为下家地主 	CT_DWORD old_Card; //0，自己，1，上家，2下家
	//------------------------------------------------------------------------------------------ 上家地主,朋友出牌
	if (Bankxin == 2 && ddz_old.old_Card == 2)  //上家地主下家出牌
	{
		if (cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
		{
			if (ddz_two_fx.two_js > 0)
			{
				bool as = Set_DOUBLE_1(1, cbCardDatas);
				if (as == true)
					return true;
			}
		}
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 2);
			if (abdd == true)
			{
				return true;
			}
	
		}
		return false;
	}
	//	int Bankxin; //1为自己地主，2为上家地主，3为下家地主 	CT_DWORD old_Card; //0，自己，1，上家，2下家
	//------------------------------------------------------------------------------------------ 上家地主,地主出牌
	if (Bankxin == 2 && ddz_old.old_Card == 1)
	{
		if (cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
		{
			if (ddz_two_fx.two_js > 0)
			{
				bool as = Set_DOUBLE_1(1, cbCardDatas);
				if (as == true)
					return true;
			}
		}
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 2);
			if (abdd == true)
			{
				return true;
			}
	
		}
		if (ddz_one.ps == 3 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
		{
			mOutCard.cbCardData[0] = 1;
			mOutCard.cbCardData[1] = 1;
			mOutCard.cbCardCount = 2;
			ddz_one.ddzone_1[2] = 0;
			ddz_one.ddzone_1[1] = 0;
			ddz_one.onepxjs -= 2;
			ddz_one.ps-= 2;
			return true;
		}
		if (ddz_one.ps == 2 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
		{
			mOutCard.cbCardData[0] = 1;
			mOutCard.cbCardData[1] = 1;
			mOutCard.cbCardCount = 2;
			ddz_one.ddzone_1[0] = 0;
			ddz_one.ddzone_1[1] = 0;
			ddz_one.onepxjs -= 2;
			ddz_one.ps -= 2;
			return true;
		}

		if (ddz_two.ps == 3 && cbCardDatas[0] != 2)
		{
			mOutCard.cbCardData[0] = 2;
			mOutCard.cbCardData[1] = 2;
			mOutCard.cbCardCount = 2;
			ddz_two.ddztwo_1[2] = 0;
			ddz_two.ddztwo_1[1] = 0;
			ddz_two.twopxjs -= 2;
			ddz_two.ps -= 2;
			return true;
		}
		if (ddz_two.ps == 2 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
		{
			mOutCard.cbCardData[0] = 2;
			mOutCard.cbCardData[1] = 2;
			mOutCard.cbCardCount = 2;
			ddz_two.ddztwo_1[0] = 0;
			ddz_two.ddztwo_1[1] = 0;
			ddz_two.twopxjs -= 2;
			ddz_two.ps -= 2;
			return true;
		}
		if (OnGameCard == 1 && onCard.Cdata[0] != 14 && onCard.Cdata[0] != 15 && onCard.Cdata[0] != 2 && onCard.Cdata[0] != 1)
		{
			bool as = Set_DOUBLE_1(8, cbCardDatas);
			if (as == true)
				return true;


		}
		return false;
	}

	//--------------------------------------------------------------------------------------------- 下家地主，下家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 2)
	{
		if (cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
		{
			if (ddz_two_fx.two_js > 0)
			{
				bool as = Set_DOUBLE_1(0, cbCardDatas);
				if (as == true)
					return true;
			}
		}
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 2);
			if (abdd == true)
			{
				return true;
			}

		}
		if (ddz_one.ps == 3 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
		{
			mOutCard.cbCardData[0] = 1;
			mOutCard.cbCardData[1] = 1;
			mOutCard.cbCardCount = 2;
			ddz_one.ddzone_1[2] = 0;
			ddz_one.ddzone_1[1] = 0;
			ddz_one.onepxjs -= 2;
			ddz_one.ps -= 2;
			return true;
		}
		if (ddz_one.ps == 2 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
		{
			mOutCard.cbCardData[0] = 1;
			mOutCard.cbCardData[1] = 1;
			mOutCard.cbCardCount = 2;
			ddz_one.ddzone_1[0] = 0;
			ddz_one.ddzone_1[1] = 0;
			ddz_one.onepxjs -= 2;
			ddz_one.ps -= 2;
			return true;
		}

		if (ddz_two.ps == 3 && cbCardDatas[0] != 2)
		{
			mOutCard.cbCardData[0] = 2;
			mOutCard.cbCardData[1] = 2;
			mOutCard.cbCardCount = 2;
			ddz_two.ddztwo_1[2] = 0;
			ddz_two.ddztwo_1[1] = 0;
			ddz_two.twopxjs -= 2;
			ddz_two.ps -= 2;
			return true;
		}
		if (ddz_two.ps == 2 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
		{
			mOutCard.cbCardData[0] = 2;
			mOutCard.cbCardData[1] = 2;
			mOutCard.cbCardCount = 2;
			ddz_two.ddztwo_1[0] = 0;
			ddz_two.ddztwo_1[1] = 0;
			ddz_two.twopxjs -= 2;
			ddz_two.ps -= 2;
			return true;
		}
		if (DownGameCard == 1 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 15 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 1)
		{
			bool as = Set_DOUBLE_1(8, cbCardDatas);
			if (as == true)
				return true;

		}
		return false;
	}
	//--------------------------------------------------------------------------------------------- 下家地主，上家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 1)
	{
		if (cbCardDatas[0] != 1 && cbCardDatas[0] != 2)
		{
			if (ddz_two_fx.two_js > 0)
			{
				bool as = Set_DOUBLE_1(0, cbCardDatas);
				if (as == true)
					return true;
			}
		}
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 2);
			if (abdd == true)
			{
				return true;
			}
	
		}
		return false;
	}
	return false;
}
bool BOTProces::Set_CT_THREE(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_DOUBLE_Card1)//三代管牌，
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));
CT_BYTE cond_l[6];
	memset(cond_l, 0, sizeof(cond_l));
if (Bankxin == 1) //自己地主
	{
	if (qbCardDatas[0] != 1 && qbCardDatas[0] != 2)
	{
		if (ddz_three.three_js > 0)
		{
			bool as = Set_DOUBLE_1(3, qbCardDatas);
			if (as == true)
				return true;
		}
	}
	if (jdyp == true)
	{
		//printf("必胜牌打法\r\n");
		bool abdd = Set_JdOutCard(1, 3);
		if (abdd == true)
		{
			return true;
		}

	}

	if (OnGameCard < 4 || DownGameCard < 4)
	{

		if (ddz_one.ps == 3 && qbCardDatas[0] != 2 && qbCardDatas[0] != 1)
		{
			mOutCard.cbCardData[0] = 1;
			mOutCard.cbCardData[1] = 1;
			mOutCard.cbCardData[2] = 1;
			mOutCard.cbCardCount = 3;
			ddz_one.ddzone_1[2] = 0;
			ddz_one.ddzone_1[1] = 0;
			ddz_one.ddzone_1[0] = 0;
			ddz_one.onepxjs -= 3;
			ddz_one.ps -=3;
			return true;
		}
		if (ddz_two.ps == 3 && qbCardDatas[0] != 2 )
		{
			mOutCard.cbCardData[0] = 2;
			mOutCard.cbCardData[1] = 2;
			mOutCard.cbCardData[2] = 2;
			mOutCard.cbCardCount = 3;
			ddz_two.ddztwo_1[2] = 0;
			ddz_two.ddztwo_1[1] = 0;
			ddz_two.ddztwo_1[0] = 0;
			ddz_two.twopxjs -= 3;
			ddz_two.ps -= 3;
			return true;
		}
		if (ddz_zd.zdpxjs > 0)
		{
			bool ad = Set_Card_OUT();
			if (!ad)
			{
				return false;
			}
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
			mOutCard.cbCardCount = 4;
			ddz_zd.ps -= 4;
			ddz_zd.zdpxjs -= 1;
			memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
			memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
			memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
			memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
			return true;
		}
	}

	return false;
	}
//------------------------------------------------------------------------------------------ 上家地主,朋友出牌
if (Bankxin == 2 && ddz_old.old_Card==2)
{
	return false;
}
//---------------------------------------------------------- 上家地主，上家出牌
if (Bankxin == 2 && ddz_old.old_Card == 1)
{
	if (qbCardDatas[0] != 1 && qbCardDatas[0] != 2)
	{
		if (ddz_three.three_js > 0)
		{
			bool as = Set_DOUBLE_1(3, qbCardDatas);
			if (as == true)
				return true;
		}
	}
	if (jdyp == true)
	{
		//printf("必胜牌打法\r\n");
		bool abdd = Set_JdOutCard(1, 3);
		if (abdd == true)
		{
			return true;
		}

	}
	if (ddz_one.ps == 3 && qbCardDatas[0] != 2 && qbCardDatas[0] != 1)
	{
		mOutCard.cbCardData[0] = 1;
		mOutCard.cbCardData[1] = 1;
		mOutCard.cbCardData[2] = 1;
		mOutCard.cbCardCount = 3;
		ddz_one.ddzone_1[2] = 0;
		ddz_one.ddzone_1[1] = 0;
		ddz_one.ddzone_1[0] = 0;
		ddz_one.onepxjs -= 3;
		ddz_one.ps -= 3;
		return true;
	}
	if (ddz_two.ps == 3 && qbCardDatas[0] != 2)
	{
		mOutCard.cbCardData[0] = 2;
		mOutCard.cbCardData[1] = 2;
		mOutCard.cbCardData[2] = 2;
		mOutCard.cbCardCount = 3;
		ddz_two.ddztwo_1[2] = 0;
		ddz_two.ddztwo_1[1] = 0;
		ddz_two.ddztwo_1[0] = 0;
		ddz_two.twopxjs -= 3;
		ddz_two.ps -= 3;
		return true;
	}
	if (OnGameCard == 2 || OnGameCard == 1)
	{
		if (ddz_zd.zdpxjs > 0)
		{
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
			mOutCard.cbCardCount = 4;
			ddz_zd.ps -= 4;
			ddz_zd.zdpxjs -= 1;
			memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
			memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
			memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
			memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
			return true;
		}
	}
	if (OnGameCard == 1 && onCard.Cdata[0] != 14 && onCard.Cdata[0] != 15 && onCard.Cdata[0] != 2 && onCard.Cdata[0] != 1)
	{
		bool as = Set_DOUBLE_1(8, qbCardDatas);
		if (as == true)
			return true;


	}

	return false;
}
//---------------------------------------------下家地主，下家出牌
if (Bankxin == 3 && ddz_old.old_Card == 2)
{
	if (qbCardDatas[0] != 1 && qbCardDatas[0] != 2)
	{
		if (ddz_three.three_js > 0)
		{
			bool as = Set_DOUBLE_1(3, qbCardDatas);
			if (as == true)
				return true;
		}
	}
	if (jdyp == true)
	{
		//printf("必胜牌打法\r\n");
		bool abdd = Set_JdOutCard(1, 3);
		if (abdd == true)
		{
			return true;
		}

	}
	if (ddz_one.ps == 3 && qbCardDatas[0] != 2 && qbCardDatas[0] != 1)
	{
		mOutCard.cbCardData[0] = 1;
		mOutCard.cbCardData[1] = 1;
		mOutCard.cbCardData[2] = 1;
		mOutCard.cbCardCount = 3;
		ddz_one.ddzone_1[2] = 0;
		ddz_one.ddzone_1[1] = 0;
		ddz_one.ddzone_1[0] = 0;
		ddz_one.onepxjs -= 3;
		ddz_one.ps -= 3;
		return true;
	}
	if (ddz_two.ps == 3 && qbCardDatas[0] != 2)
	{
		mOutCard.cbCardData[0] = 2;
		mOutCard.cbCardData[1] = 2;
		mOutCard.cbCardData[2] = 2;
		mOutCard.cbCardCount = 3;
		ddz_two.ddztwo_1[2] = 0;
		ddz_two.ddztwo_1[1] = 0;
		ddz_two.ddztwo_1[0] = 0;
		ddz_two.twopxjs -= 3;
		ddz_two.ps -= 3;
		return true;
	}
	if (OnGameCard == 2 || OnGameCard == 1)
	{
		if (ddz_zd.zdpxjs > 0)
		{
			
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
			mOutCard.cbCardCount = 4;
			ddz_zd.ps -= 4;
			ddz_zd.zdpxjs -= 1;
			memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
			memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
			memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
			memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
			return true;
		}
	}
	if (DownGameCard == 1 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 15 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 1)
	{
		bool as = Set_DOUBLE_1(8, qbCardDatas);
		if (as == true)
			return true;

	}
	return false;
}

//----------------------------------------------- 下家地主 朋友出牌
if (Bankxin == 3 && ddz_old.old_Card ==1) 
{	bool as = Set_DOUBLE_1(4, qbCardDatas);
				if (as == true)
					return true;
	return false;
}
	return false;
}
bool BOTProces::Set_CT_THREE_1(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_SQ_Card1)//三代1管牌，gameuserid 1为上家，2为下家,剩余牌数
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));


	if (Bankxin == 1)
	{
		if (qbCardDatas[0] != 1 && qbCardDatas[2] != 2)
		{
			if (Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
			{
				bool as = Set_DOUBLE_1(9, qbCardDatas);//飞机带1，
				if (as)
					return true;
			}else if (!Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
			{
				if (ddz_three.three_js > 0)
				{
					bool as = Set_DOUBLE_1(4, qbCardDatas);
					if (as == true)
						return true;
				}


			}

		}

		if (!Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
		{
			if (jdyp == true)
			{
				//printf("必胜牌打法\r\n");
				bool abdd = Set_JdOutCard(1, 4);
				if (abdd == true)
				{
					return true;
				}

			}
			if (OnGameCard < 5 || DownGameCard < 5)
			{
				if (ddz_san.ps > 0 && ddz_san.ps < 20)
				{
					if (ddz_one.ps == 3 && qbCardDatas[0] != 2 && qbCardDatas[0] != 1)
					{
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						mOutCard.cbCardData[2] = 1;
						mOutCard.cbCardCount = 4;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.ddzone_1[1] = 0;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.onepxjs -= 3;
						ddz_one.ps -= 3;
						mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
						ddz_san.ddz_san_1[0] = 0;
						ddz_san.ps -= 1;
						//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

						}

						return true;
					}
					if (ddz_two.ps == 3 && qbCardDatas[0] != 2)
					{
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						mOutCard.cbCardData[2] = 2;
						mOutCard.cbCardCount = 4;
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.twopxjs -= 3;
						ddz_two.ps -= 3;
						ddz_san.ddz_san_1[0] = 0;
						ddz_san.ps -= 1;
						//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

						}

						return true;
					}
				}

			}
		}else
		{
			if (jdyp == true)
			{
				//printf("必胜牌打法\r\n");
				bool abdd = Set_JdOutCard(1,9);
				if (abdd == true)
				{
					return true;
				}

			}
		}
	return false;
	}
	//------------------------------------------------ 上家地主，上家出牌
	if (Bankxin == 2&&ddz_old.old_Card==1)
	{
		if (qbCardDatas[0] != 1 && qbCardDatas[2] != 2)
		{
			if (Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
			{
				bool as = Set_DOUBLE_1(9, qbCardDatas);//飞机带1，
				if (as)
					return true;
				if (jdyp == true)
				{
					//printf("必胜牌打法\r\n");
					bool abdd = Set_JdOutCard(1, 9);
					if (abdd == true)
					{
						return true;
					}

				}

			}else if (!Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
			{
				if (ddz_three.three_js > 0)
				{
					bool as = Set_DOUBLE_1(4, qbCardDatas);
					if (as == true)
						return true;
				}
				if (jdyp == true)
				{
					//printf("必胜牌打法\r\n");
					bool abdd = Set_JdOutCard(1, 4);
					if (abdd == true)
					{
						return true;
					}

				}
			}
		}
		if (!Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
		{
			if (ddz_san.ps > 0)
			{
				if (ddz_one.ps == 3 && qbCardDatas[0] != 2 && qbCardDatas[0] != 1)
				{
					mOutCard.cbCardData[0] = 1;
					mOutCard.cbCardData[1] = 1;
					mOutCard.cbCardData[2] = 1;
					mOutCard.cbCardCount = 4;
					ddz_one.ddzone_1[2] = 0;
					ddz_one.ddzone_1[1] = 0;
					ddz_one.ddzone_1[0] = 0;
					ddz_one.onepxjs -= 3;
					ddz_one.ps -= 3;
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = 0;
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}

					return true;
				}
				if (ddz_two.ps == 3 && qbCardDatas[0] != 2)
				{
					mOutCard.cbCardData[0] = 2;
					mOutCard.cbCardData[1] = 2;
					mOutCard.cbCardData[2] = 2;
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					mOutCard.cbCardCount = 4;
					ddz_two.ddztwo_1[2] = 0;
					ddz_two.ddztwo_1[1] = 0;
					ddz_two.ddztwo_1[0] = 0;
					ddz_two.twopxjs -= 3;
					ddz_two.ps -= 3;
					ddz_san.ddz_san_1[0] = 0;
					ddz_san.ps -= 1;
					//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}

					return true;
				}
			}
			if (OnGameCard == 1 && onCard.Cdata[0] != 14 && onCard.Cdata[0] != 15 && onCard.Cdata[0] != 2 && onCard.Cdata[0] != 1)
			{
				bool ad = Set_Card_OUT();
				if (!ad)
				{
					return false;
				}
				bool as = Set_DOUBLE_1(8, qbCardDatas);
				if (as == true)
					return true;


			}
		}
		return false;
	}
	//------------------------------------------------- 上家地主，下家出牌
	if (Bankxin == 2 && ddz_old.old_Card == 2)
	{
	
		return false;
	}
//------------------------------------------ 下家地主，上家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 1)
	{
		return false;
	}
//----------------------------------------------------下家地主，下家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 2)
	{
		if (qbCardDatas[0] != 1 && qbCardDatas[2] != 2)
		{
			if (Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
			{
				bool as = Set_DOUBLE_1(9, qbCardDatas);//飞机带1，
				if (as)
					return true;

				if (jdyp == true)
				{
					//printf("必胜牌打法\r\n");
					bool abdd = Set_JdOutCard(1,9);
					if (abdd == true)
					{
						return true;
					}

				}
			}
			else if (!Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
			{
				if (ddz_three.three_js > 0)
				{
					bool as = Set_DOUBLE_1(4, qbCardDatas);
					if (as == true)
						return true;
				}

			}
		}
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			if (!Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
			{
				bool abdd = Set_JdOutCard(1, 4);
				if (abdd == true)
				{
					return true;
				}
			}
		}
		if (!Set_THREE_2_TWO(ddz_old.old_Card_data, 1))
		{
			if (ddz_san.ps > 0)
			{
				if (ddz_one.ps == 3 && qbCardDatas[0] != 2 && qbCardDatas[0] != 1)
				{
					mOutCard.cbCardData[0] = 1;
					mOutCard.cbCardData[1] = 1;
					mOutCard.cbCardData[2] = 1;
					mOutCard.cbCardCount = 4;
					ddz_one.ddzone_1[2] = 0;
					ddz_one.ddzone_1[1] = 0;
					ddz_one.ddzone_1[0] = 0;
					ddz_one.onepxjs -= 3;
					ddz_one.ps -= 3;
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = 0;
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}

					return true;
				}
				if (ddz_two.ps == 3 && qbCardDatas[0] != 2)
				{
					mOutCard.cbCardData[0] = 2;
					mOutCard.cbCardData[1] = 2;
					mOutCard.cbCardData[2] = 2;
					mOutCard.cbCardCount = 4;
					ddz_two.ddztwo_1[2] = 0;
					ddz_two.ddztwo_1[1] = 0;
					ddz_two.ddztwo_1[0] = 0;
					ddz_two.twopxjs -= 3;
					ddz_two.ps -= 3;
					ddz_san.ddz_san_1[0] = 0;
					ddz_san.ps -= 1;
					//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}

					return true;
				}
			}
			if (DownGameCard == 1 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 15 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 1)
			{
				bool as = Set_DOUBLE_1(8, qbCardDatas);
				if (as == true)
					return true;

			}
		}
		return false;
	}
	return false;
}
bool BOTProces::Set_CT_THREE_2(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_SQ_Card1)//三代1管牌，gameuserid 1为上家，2为下家,剩余牌数
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));

	
	if (jdyp == false)
	{
		if (ddz_old.old_Count >9)
		{
			return false;
		}
	}
	else if(jdyp==true)
	{
		if (ddz_old.old_Count > 9)
		{
			bool abdd = Set_JdOutCard(1,9);
			if (abdd)
				return true;
			if (!abdd)
				return false;
		}
	}

	if (Bankxin == 1) //自己地主
	{

			bool as = Set_DOUBLE_1(5, qbCardDatas);
			if (as == true)
				return true;
	
			if (jdyp == true)
			{
				//printf("必胜牌打法\r\n");
				bool abdd = Set_JdOutCard(1, 5);
				if (abdd == true)
					return true;
			}
		return false;
	}
//------------------------- 上家地主，上家出牌
	if (Bankxin == 2&&ddz_old.old_Card==1) 
	{
		bool as = Set_DOUBLE_1(5, qbCardDatas);
		if (as == true)
			return true;
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 5);
			if (abdd == true)
			{
				return true;
			}
	
		}
		if (OnGameCard == 1 && onCard.Cdata[0] != 14 && onCard.Cdata[0] != 15 && onCard.Cdata[0] != 2 && onCard.Cdata[0] != 1)
		{
			bool ad = Set_Card_OUT();
			if (!ad)
			{
				return false;
			}
			bool as = Set_DOUBLE_1(8, qbCardDatas);
			if (as == true)
				return true;


		}


	}
//--------------------------------上家地主，下家出牌
	if (Bankxin == 2 && ddz_old.old_Card == 2)
	{
		return false;
	}
//-----------------------------------下家地主，上家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 1)
	{
		return false;
	}
	//--------------------------------
	if (Bankxin == 3 && ddz_old.old_Card == 2)//下家地主，下家出牌
	{
		bool as = Set_DOUBLE_1(5, qbCardDatas);
		if (as == true)
			return true;
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 5);
			if (abdd == true)
			{
				return true;
			}

		}
		if (DownGameCard == 1 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 15 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 1)
		{
			bool as = Set_DOUBLE_1(8, qbCardDatas);
			if (as == true)
				return true;

		}
		return false;
	}


	return false;
}
bool BOTProces::Set_DOUBLE_LINE(CT_BYTE qbCardDatas[MAX_COUNT], CT_DWORD gameuserids, CT_DWORD SX_SQ_Card1)
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));
	if (Bankxin == 1) //自己地主
	{
		bool as = Set_DOUBLE_1(6, qbCardDatas);
		if (as == true)
			return true;

		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 7);
			if (abdd == true)
			{
				return true;
			}
	
		}
		return false;
	}
//----------------------- 上家地主，上家出牌
	if (Bankxin == 2 && ddz_old.old_Card == 1)
	{
		bool as = Set_DOUBLE_1(6, qbCardDatas);
		if (as == true)
			return true;
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 7);
			if (abdd == true)
			{
				return true;
			}

		}
		if (OnGameCard == 1 && onCard.Cdata[0] != 14 && onCard.Cdata[0] != 15 && onCard.Cdata[0] != 2 && onCard.Cdata[0] != 1)
		{
			bool as = Set_DOUBLE_1(8, qbCardDatas);
			if (as == true)
				return true;


		}
		return false;
	}
//----------------------- 上家地主，下家出牌
	if (Bankxin == 2 && ddz_old.old_Card == 2)
	{
	
		return false ;
	}
//----------------------- 下家地主，下家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 2)
	{
		bool as = Set_DOUBLE_1(6, qbCardDatas);
		if (as == true)
			return true;
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 7);
			if (abdd == true)
			{
				return true;
			}
	
		}
		if (DownGameCard == 1 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 15 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 1)
		{
			bool as = Set_DOUBLE_1(8, qbCardDatas);
			if (as == true)
				return true;

		}
		return false;
	}

//----------------------- 下家地主，上家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 1)
	{

		return false;
	}
	return false;
}
bool BOTProces::Set_SINGLE_LINE(CT_BYTE CardDatas[MAX_COUNT], CT_DWORD userids, CT_DWORD SQ_Card1)//单连管牌
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));
	if (Bankxin == 1) //自己地主
	{
		bool as = Set_DOUBLE_1(7, CardDatas);
		if (as == true)
			return true;
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 8);
			if (abdd == true)
			{
				return true;
			}

		}
		return false;
	}
	//----------------------- 上家地主，上家出牌
	if (Bankxin == 2 && ddz_old.old_Card == 1)
	{
		bool as = Set_DOUBLE_1(7, CardDatas);
		if (as == true)
			return true;
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 8);
			if (abdd == true)
			{
				return true;
			}
	
		}
		if (OnGameCard == 1 && onCard.Cdata[0] != 14 && onCard.Cdata[0] != 15 && onCard.Cdata[0] != 2 && onCard.Cdata[0] != 1)
		{
			bool as = Set_DOUBLE_1(8, CardDatas);
			if (as == true)
				return true;


		}
		return false;
	}
	//----------------------- 上家地主，下家出牌
	if (Bankxin == 2 && ddz_old.old_Card == 2)
	{

		return false;
	}
	//----------------------- 下家地主，下家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 2)
	{
		bool as = Set_DOUBLE_1(7, CardDatas);
		if (as == true)
			return true;
		if (jdyp == true)
		{
			//printf("必胜牌打法\r\n");
			bool abdd = Set_JdOutCard(1, 8);
			if (abdd == true)
			{
				return true;
			}

		}
		if (DownGameCard == 1 && DownCard.Cdata[0] != 14 && DownCard.Cdata[0] != 15 && DownCard.Cdata[0] != 2 && DownCard.Cdata[0] != 1)
		{
			bool as = Set_DOUBLE_1(8, CardDatas);
			if (as == true)
				return true;

		}
		return false;
	}

	//----------------------- 下家地主，上家出牌
	if (Bankxin == 3 && ddz_old.old_Card == 1)
	{

		return false;
	}
	return false;
}
CT_VOID BOTProces::Get_zd()
{
	int ipt = 1;
	int iqs = 0;
	onCard.Count = 0;
	DownCard.Count = 0;
	for (int i = 0; i < OnGameCard; i++)
	{
		if (onCard.Cdata[i] == ipt)
		{
			iqs++;
			if (iqs == 4)
			{
				//printf("上家有炸弹\r\n");
				onCard.Count += 1;
			}

		}

		if (i == OnGameCard - 1)
		{
			ipt++;
			iqs = 0;
			i = -1;

		}
		if (ipt == 14)
			break;

	}

	ipt = 1;
	iqs = 0;
	for (int i = 0; i < DownGameCard; i++)
	{
		if (DownCard.Cdata[i] == ipt)
		{
			iqs++;
			if (iqs == 4)
			{
				//printf("下家有炸弹\r\n");
				DownCard.Count += 1;
			}

			
		}
		if (i == DownGameCard - 1)
		{
			ipt++;
			iqs = 0;
			i = -1;

		}
		if (ipt == 14)
			break;

	}
}
bool  BOTProces::Set_DOUBLE_1(int zfx, CT_BYTE cbCardDatas[MAX_COUNT])
{
	if (zfx == 1) //正序
	{
		if (ddz_two_fx.ddz_two_1[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
			memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_2[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;

		}
		if (ddz_two_fx.ddz_two_3[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_4[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_5[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_6[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
	} //对子
	if (zfx == 0) //反序
	{
		if (ddz_two_fx.ddz_two_6[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_5[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_4[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_3[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_two_fx.ddz_two_2[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;

		}
		if (ddz_two_fx.ddz_two_1[0] > cbCardDatas[0])
		{

			memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
			memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
	} 
	if (zfx == 3)
	{
		if (ddz_three.ddz_three_1[0] > cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
			mOutCard.cbCardCount = 3;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
			memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
	
			return true;
		}
		if (ddz_three.ddz_three_2[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));  //还差带1单
			mOutCard.cbCardCount = 3;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
		
			return true;
		}

		if (ddz_three.ddz_three_3[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));  //还差带1单
			mOutCard.cbCardCount = 3;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
		
			return true;
		}

		if (ddz_three.ddz_three_4[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));  //还差带1单
			mOutCard.cbCardCount = 3;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
		
			return true;
		}
		if (ddz_three.ddz_three_5[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));  //还差带1单
			mOutCard.cbCardCount = 3;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
			
			return true;
		}

	}
	if (zfx == 4) //三代1
	{
	
		if (ddz_san.ps > 0&&ddz_san.ps<20)
		{
		if (ddz_three.ddz_three_1[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
			mOutCard.cbCardCount = 4;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
			memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));

			mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
			ddz_san.ddz_san_1[0] = 0;
			ddz_san.ps -= 1;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			for (int is = 0; is < ddz_san.ps; is++)
			{
				ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

			}
			
			return true;
		}

		if (ddz_three.ddz_three_2[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));  //还差带1单
			mOutCard.cbCardCount = 4;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
        	mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
			ddz_san.ddz_san_1[0] = 0;
			ddz_san.ps -= 1;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			for (int is = 0; is < ddz_san.ps; is++)
			{
				ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

			}
		
			return true;
		}

		if (ddz_three.ddz_three_3[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));  //还差带1单
			mOutCard.cbCardCount = 4;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));

			mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
			ddz_san.ddz_san_1[0] = 0;
			ddz_san.ps -= 1;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			for (int is = 0; is < ddz_san.ps; is++)
			{
				ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

			}
		
			return true;
		}

		if (ddz_three.ddz_three_4[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));  //还差带1单
			mOutCard.cbCardCount = 4;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));

			mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
			ddz_san.ddz_san_1[0] = 0;
			ddz_san.ps -= 1;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			for (int is = 0; is < ddz_san.ps; is++)
			{
				ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

			}
		
			return true;
		}
		if (ddz_three.ddz_three_5[0]>cbCardDatas[0])
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));  //还差带1单
			mOutCard.cbCardCount = 4;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));

			mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
			ddz_san.ddz_san_1[0] = 0;
			ddz_san.ps -= 1;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			for (int is = 0; is < ddz_san.ps; is++)
			{
				ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

			}
			
			return true;
		}

	
		}

		return false;
	}
	if (zfx == 5)
	{
		if (cbCardDatas[0] != 1 && cbCardDatas[2] != 2)
		{
			if (ddz_three.three_js > 0)
			{
				if (ddz_three.ddz_three_1[0] > cbCardDatas[0])
				{
					if (ddz_two_fx.ps > 0)
					{
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
						mOutCard.cbCardCount = 5;
						ddz_three.ps -= 3;
						ddz_three.three_js -= 1;
						memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
						memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
						memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));

						return true;
					}
				}

				if (ddz_three.ddz_three_2[0] > cbCardDatas[0])
				{
					if (ddz_two_fx.ps > 0)
					{
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));  //还差带1单
						mOutCard.cbCardCount = 5;
						ddz_three.ps -= 3;
						ddz_three.three_js -= 1;
						memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
						memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));

						return true;
					}
				}

				if (ddz_three.ddz_three_3[0] > cbCardDatas[0])
				{
					if (ddz_two_fx.ps > 0)
					{
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));  //还差带1单
						mOutCard.cbCardCount = 5;
						ddz_three.ps -= 3;
						ddz_three.three_js -= 1;
						memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));

						return true;
					}
				}

				if (ddz_three.ddz_three_4[0] > cbCardDatas[0])
				{
					if (ddz_two_fx.ps > 0)
					{
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));  //还差带1单
						mOutCard.cbCardCount = 5;
						ddz_three.ps -= 3;
						ddz_three.three_js -= 1;
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));

						return true;
					}
				}
				if (ddz_three.ddz_three_5[0] > cbCardDatas[0])
				{
					if (ddz_two_fx.ps > 0)
					{
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));  //还差带1单
						mOutCard.cbCardCount = 5;
						ddz_three.ps -= 3;
						ddz_three.three_js -= 1;
						memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));

						return true;
					}
				}

			}

		}
		if (OnGameCard < 5 ||DownGameCard < 5)
		{
			if (ddz_two_fx.ps > 0)
			{
				if (ddz_one.ps == 3 && cbCardDatas[0] != 2 && cbCardDatas[0] != 1)
				{
					mOutCard.cbCardData[0] = 1;
					mOutCard.cbCardData[1] = 1;
					mOutCard.cbCardData[2] = 1;
					mOutCard.cbCardCount = 5;
					ddz_one.ddzone_1[2] = 0;
					ddz_one.ddzone_1[1] = 0;
					ddz_one.ddzone_1[0] = 0;
					ddz_one.onepxjs -= 3;
					ddz_one.ps -= 3;
					mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}

				if (ddz_two.ps == 3 && cbCardDatas[0] != 2 )
				{
					mOutCard.cbCardData[0] = 2;
					mOutCard.cbCardData[1] = 2;
					mOutCard.cbCardData[2] = 2;
					mOutCard.cbCardCount = 5;
					ddz_two.ddztwo_1[2] = 0;
					ddz_two.ddztwo_1[1] = 0;
					ddz_two.ddztwo_1[0] = 0;
					ddz_two.twopxjs -= 3;
					ddz_two.ps -= 3;
					mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}



			}
			
		}

		return false;
	}
	if (zfx == 6)
	{
		if (ddz_twolian.lian_1 == (int)ddz_old.old_Count || ddz_twolian.lian_2 == (int)ddz_old.old_Count)
		{
			for (int i = 0; i < ddz_twolian.lian_1; i++)
			{
				if (ddz_twolian.ddztwo_lian_1[i]>cbCardDatas[0])
				{
					mOutCard.cbCardCount = ddz_twolian.lian_1;
					ddz_twolian.lianpxjs -= 1;
					ddz_twolian.ps -= ddz_twolian.lian_1;
					ddz_twolian.lian_1 = ddz_twolian.lian_2;
					ddz_twolian.lian_2 = 0;
					memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
					memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
					memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
					return true;
				}

			}

			for (int i = 0; i < ddz_twolian.lian_2; i++)
			{
				if (ddz_twolian.ddztwo_lian_2[i] > cbCardDatas[0])
				{
					mOutCard.cbCardCount = ddz_twolian.lian_2;
					ddz_twolian.lianpxjs -= 1;
					ddz_twolian.ps -= ddz_twolian.lian_2;
					ddz_twolian.lian_2 = 0;
					memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
					memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
				
					return true;
				}
			}




		}


		return false;

	}
	if (zfx == 7)
	{
		if (ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1] >cbCardDatas[0] && ddz_lian_1.lian_1 == (int)ddz_old.old_Count&& cbCardDatas[0]!=1)
		{
			mOutCard.cbCardCount = ddz_lian_1.lian_1;
			ddz_lian_1.lianpxjs -= 1;
			ddz_lian_1.ps -= ddz_lian_1.lian_1;
			ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
			ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
			ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
			ddz_lian_1.lian_4 = 0;
			memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
			memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
			memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_3, sizeof(ddz_lian_1.ddzlian_3));
			memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));
			memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
			return true;

		}
		if (ddz_lian_1.ddzlian_2[ddz_lian_1.lian_2 - 1] > cbCardDatas[0] && ddz_lian_1.lian_2 == (int)ddz_old.old_Count&& cbCardDatas[0] != 1)
		{
			mOutCard.cbCardCount = ddz_lian_1.lian_2;
			ddz_lian_1.lianpxjs -= 1;
			ddz_lian_1.ps -= ddz_lian_1.lian_2;
			ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
			ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
			ddz_lian_1.lian_4 = 0;
			memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
			memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_3, sizeof(ddz_lian_1.ddzlian_3));
			memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));
			memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
			return true;
		}
		if (ddz_lian_1.ddzlian_3[ddz_lian_1.lian_3 - 1] > cbCardDatas[0] && ddz_lian_1.lian_3 == (int)ddz_old.old_Count&& cbCardDatas[0] != 1)
		{
			mOutCard.cbCardCount = ddz_lian_1.lian_3;
			ddz_lian_1.lianpxjs -= 1;
			ddz_lian_1.ps -= ddz_lian_1.lian_3;
			ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
			ddz_lian_1.lian_4 = 0;
			memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_3, sizeof(ddz_lian_1.ddzlian_3));
			memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));
			memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
			return true;
		}
		if (ddz_lian_1.ddzlian_4[ddz_lian_1.lian_4 - 1] > cbCardDatas[0] && ddz_lian_1.lian_4 == (int)ddz_old.old_Count&& cbCardDatas[0] != 1)
		{
			mOutCard.cbCardCount = ddz_lian_1.lian_4;
			ddz_lian_1.lianpxjs -= 1;
			ddz_lian_1.ps -= ddz_lian_1.lian_4;
			ddz_lian_1.lian_4 = 0;
			memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));
			memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
			return true;
		}

		return false;
	}
	if (zfx == 8)//炸弹
	{
		if (ddz_zd.ps > 0)
		{
			mOutCard.cbCardCount = 4;
			ddz_zd.ps -= 4;
			ddz_zd.zdpxjs -= 1;
			memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
			memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
			memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
			memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
			memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
			return true;
		}
		if (ddz_one.ps == 4)
		{
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
			memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
			ddz_one.onepxjs = 0;
			ddz_one.ps = 0;
			return true;
		}
		if (ddz_two.ps == 4)
		{
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
			memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
			ddz_two.ps = 0;
			ddz_two.twopxjs = 0;
			return true;
		}
	}
	if (zfx == 9)//飞机带单
	{
		CT_BYTE qs = Set_THREE_PX(cbCardDatas);
		if (ddz_fj.feiji_js > 0)
		{
			if (ddz_old.old_Count == 8 && ddz_fj.feiji_1 == 6&&ddz_san.ps>1)
			{
				if (ddz_fj.ddz_feiji_1[0] > qs|| ddz_fj.ddz_feiji_1[3] > qs)
				{
					mOutCard.cbCardCount = 8;
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));  //还差带1单
			   ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_js -= 1;
                    memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_3;
					mOutCard.cbCardData[6] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = 0;
					ddz_san.ps -= 1;
				for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					ddz_san.ps -= 1;
					mOutCard.cbCardData[7] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = 0;
			for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					return true;
				}
			}
			if (ddz_old.old_Count ==12 && ddz_fj.feiji_1 == 9 && ddz_san.ps>2)
			{
				if (ddz_fj.ddz_feiji_1[0] > qs || ddz_fj.ddz_feiji_1[3] > qs || ddz_fj.ddz_feiji_1[6] > qs)
				{
					mOutCard.cbCardCount = 12;
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));  //还差带1单
					ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_js -= 1;
					memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_3;
					mOutCard.cbCardData[9] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = 0;
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					ddz_san.ps -= 1;
					mOutCard.cbCardData[10] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = 0;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					ddz_san.ps -= 1;
					mOutCard.cbCardData[11] = ddz_san.ddz_san_1[0];
					ddz_san.ddz_san_1[0] = 0;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];

					}
					return true;
				}
			}


		}

	}
	if (zfx == 10)//飞机带对
	{

	}


	return false ;
}
bool BOTProces::Set_JdOutCard(int XCard, int CardX)//绝对手牌出牌方案/0为自己出牌，1为 管牌
{
	mOutCard.cbCardCount = 0;
	memset(mOutCard.cbCardData, 0, sizeof(mOutCard.cbCardData));
	if (XCard == 0) //自己出牌
	{
		//绝对手牌 1大王，2是小王，3是 2，4是 1，5是连，6是双顺，7是飞机

		if (SPFX.SP_SH == 1)
		{
			CT_BYTE ct[MAX_COUNT] = { 0 };
			for (int i = 0; i < MAX_COUNT; i++)
			{
				ct[i] = GetCodePX->GetCardValue(myCard[i]);
				if (ct[i] != 0)
				{

					mOutCard.cbCardData[mOutCard.cbCardCount] = ct[i];
					mOutCard.cbCardCount++;


				}
			}
			return true;
		}
		if (SPFX.SP_SH == 2)
		{
			//绝对手牌 1大王，2是小王，3是 2，4是 1，5是连，6是双顺，7是飞机
			int is;
          is = FindCard(0, 6);
			if (is != -1)
			{
				JD_JL.JD_SHPX[is] = 0;
				mOutCard.cbCardCount = ddz_twolian.lian_1;
				ddz_twolian.ps -= ddz_twolian.lian_1;
				ddz_twolian.lianpxjs -= 1;
				ddz_twolian.lian_1 = ddz_twolian.lian_2;
				memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
				memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
				memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
				return true;
			}
			is = FindCard(0, 5);
			if (is != -1)
			{
				JD_JL.JD_SHPX[is] = 0;
				mOutCard.cbCardCount = ddz_lian_1.lian_1;
				ddz_lian_1.ps -= ddz_lian_1.lian_1;
				ddz_lian_1.lianpxjs -= 1;
				ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
				ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
				ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
				ddz_lian_1.lian_4 = 0;
				ddz_lian_1.lianpxjs -= 1;
				memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
				memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
				memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_3, sizeof(ddz_lian_1.ddzlian_3));
				memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));
				memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
				return true;
			}
			is = FindCard(0, 7);
			if (is != -1)
			{

				if (ddz_san.ps > 1 && ddz_fj.feiji_1 == 6)
				{
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
					mOutCard.cbCardCount = 8;
					JD_JL.JD_SHPX[is] = 0;
					ddz_fj.feiji_js -= 1;
					ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_2;
					ddz_fj.feiji_3 = 0;
					memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					mOutCard.cbCardData[6] = ddz_san.ddz_san_1[0];
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					mOutCard.cbCardData[7] = ddz_san.ddz_san_1[0];
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };

					return true;
				}
				if (ddz_two_fx.two_js > 1 && ddz_fj.feiji_1 == 6)
				{
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
					mOutCard.cbCardData[ddz_fj.feiji_1] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[ddz_fj.feiji_1 + 1] = ddz_two_fx.ddz_two_1[1];
					mOutCard.cbCardData[ddz_fj.feiji_1 + 2] = ddz_two_fx.ddz_two_2[0];
					mOutCard.cbCardData[ddz_fj.feiji_1 + 3] = ddz_two_fx.ddz_two_2[1];
					mOutCard.cbCardCount = 10;
					JD_JL.JD_SHPX[is] = 0;
					ddz_fj.feiji_js -= 1;
					ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_2;
					ddz_fj.feiji_3 = 0;
					memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));

					ddz_two_fx.ps -= 4;
					ddz_two_fx.two_js -= 2;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					ddz_two_fx.ddz_two_5[0] = { 0 };
					ddz_two_fx.ddz_two_5[1] = { 0 };
					ddz_two_fx.ddz_two_6[0] = { 0 };
					ddz_two_fx.ddz_two_6[0] = { 0 }; //对子位移



					return true;
				}
				if (ddz_two_fx.two_js == 1 && ddz_fj.feiji_1 == 6)
				{
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
					mOutCard.cbCardCount = 8;
					JD_JL.JD_SHPX[is] = 0;
					ddz_fj.feiji_js -= 1;
					ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_2;
					ddz_fj.feiji_3 = 0;
					memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
					mOutCard.cbCardData[6] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[7] = ddz_two_fx.ddz_two_1[1];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js = 0;
					memset(ddz_two_fx.ddz_two_1, 0, sizeof(ddz_two_fx.ddz_two_1));
					return true;
				}


			}
        	is = FindCard(0, 4);
			if (is != -1)
			{

				mOutCard.cbCardCount = ddz_one.ps;
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				ddz_one.ps = 0;
				ddz_one.onepxjs = 0;
				JD_JL.JD_SHPX[is] = 0;
				return true;


			}
        	is = FindCard(0, 3);
            if (is != -1)
			{

				mOutCard.cbCardCount = ddz_two.ps;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.ps = 0;
				ddz_two.twopxjs = 0;
				JD_JL.JD_SHPX[is] = 0;
				return true;
			}
			is = FindCard(0, 2);
			if (is != -1)
			{

				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = 14;
				ddz_max.ddzmax_2 = 0;
				ddz_max.maxpxjs = 0;
		
				JD_JL.JD_SHPX[is] = 0;
				return true;
			}
			is = FindCard(0, 1);
            if (is != -1)
			{

				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = 15;
				ddz_max.ddzmax_1 = 0;
				ddz_max.maxpxjs = 0;

				JD_JL.JD_SHPX[is] = 0;
				return true;
			}

			if (OnGameCard != 1 && DownGameCard != 1 && ddz_san.ps > 0)
			{
				if (ddz_san.ps > 0 && ddz_san.ps < 20)
				{
					ddz_san.ps -= 1;
					//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps];
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;
				}
			}
			if (Bankxin == 1 && DownGameCard == 2 && DownCard.Cdata[2] == 0 && DownCard.Cdata[0] != DownCard.Cdata[1])
			{
				if (ddz_two_fx.ps > 0)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
			}
			if (Bankxin == 1 && OnGameCard == 2 && onCard.Cdata[2] == 0 && onCard.Cdata[0] != onCard.Cdata[1])
			{
				if (ddz_two_fx.ps > 0)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
			}
			if (ddz_lian_1.lianpxjs > 0)
			{
				memcpy(mOutCard.cbCardData, &ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
				mOutCard.cbCardCount = ddz_lian_1.lian_1;
				ddz_lian_1.lianpxjs -= 1;
				ddz_lian_1.ps -= ddz_lian_1.lian_1;
				ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
				ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
				ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
				ddz_lian_1.lian_4 = 0;
				memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
				memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_3, sizeof(ddz_lian_1.ddzlian_3));
				memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));

				memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
				return true;
			}
			if (ddz_san.ps > 0 && ddz_three.ps > 0)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
				mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
				ddz_three.three_js -= 1;
				ddz_three.ps -= 3;
				memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
				memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
				memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
				memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
				ddz_three.ddz_three_5[0] = { 0 };
				ddz_three.ddz_three_5[1] = { 0 };
				ddz_three.ddz_three_5[2] = { 0 };
				ddz_san.ddz_san_1[0] = { 0 };
				ddz_san.ps -= 1;
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				return true;

			}
			if (ddz_two_fx.ps > 0 && ddz_three.ps > 0)
			{
				memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
				mOutCard.cbCardCount = 5;
				ddz_three.ps -= 3;
				ddz_three.three_js -= 1;
				memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
				memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
				memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
				memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
				memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
				mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
				mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
				ddz_two_fx.ps -= 2;
				ddz_two_fx.two_js -= 1;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
				memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
				memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
				return true;
			}
			if (ddz_one.ps > 0)
			{
				mOutCard.cbCardCount = ddz_one.ps;
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				ddz_one.ps = 0;
				ddz_one.onepxjs = 0;
				return true;
			}
			if (ddz_two.ps > 0)
			{
				mOutCard.cbCardCount = ddz_two.ps;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.ps = 0;
				ddz_two.twopxjs = 0;
	
				return true;
			}
			is = FindCard(1, 1);
			if (is != -1)
			{
				if (ddz_zd.zdpxjs > 0)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
			}
			is = FindCard(1, 8);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				ddz_one.onepxjs -= 4;
				ddz_one.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;

			}
			is = FindCard(1, 7);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.twopxjs -= 4;
				ddz_two.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 2);
			if (is != -1)
			{
				mOutCard.cbCardCount = 2;
				mOutCard.cbCardData[0] = 14;
				mOutCard.cbCardData[1] = 15;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				ddz_max.maxpxjs = 0;
				ddz_max.ps -= 2;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 5);
			if (is != -1)
			{
				if (ddz_three.three_js > 1)
				{
					if (ddz_san.ps > 0 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
						mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
						ddz_three.three_js -= 1;
						ddz_three.ps -= 3;
						memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
						memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
						memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						ddz_three.ddz_three_5[0] = { 0 };
						ddz_three.ddz_three_5[1] = { 0 };
						ddz_three.ddz_three_5[2] = { 0 };
						ddz_san.ddz_san_1[0] = { 0 };
						ddz_san.ps -= 1;
						//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
						}
						ddz_san.ddz_san_1[ddz_san.ps] = { 0 }; //散牌位移
						return true;

					}
					if (ddz_two_fx.two_js > 0)
					{
						mOutCard.cbCardCount = 5;
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_three.three_js -= 1;
						ddz_three.ps -= 3;
						memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
						memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
						memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						ddz_three.ddz_three_5[0] = { 0 };
						ddz_three.ddz_three_5[1] = { 0 };
						ddz_three.ddz_three_5[2] = { 0 };
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;

						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
						return true;
					}

				}
				mOutCard.cbCardCount = ddz_two.ps;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.ps = 0;
				ddz_two.twopxjs = 0;
				JD_JL.JD_DPPX[is] = 0;
				return true;

			}
			is = FindCard(1, 6);
			if (is != -1)
			{
				if (ddz_three.three_js > 1)
				{
					if (ddz_san.ps > 0 && ddz_san.ps < 20)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
						mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
						ddz_three.three_js -= 1;
						ddz_three.ps -= 3;
						memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
						memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
						memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						ddz_three.ddz_three_5[0] = { 0 };
						ddz_three.ddz_three_5[1] = { 0 };
						ddz_three.ddz_three_5[2] = { 0 };
						ddz_san.ddz_san_1[0] = { 0 };
						ddz_san.ps -= 1;
						//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
						}
						ddz_san.ddz_san_1[ddz_san.ps] = { 0 }; //散牌位移
						return true;

					}
					if (ddz_two_fx.two_js > 0)
					{
						mOutCard.cbCardCount = 5;
						memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_three.three_js -= 1;
						ddz_three.ps -= 3;
						memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
						memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
						memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
						memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
						ddz_three.ddz_three_5[0] = { 0 };
						ddz_three.ddz_three_5[1] = { 0 };
						ddz_three.ddz_three_5[2] = { 0 };
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;

						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
						return true;
					}

				}


				mOutCard.cbCardCount = ddz_one.ps;
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				ddz_one.ps = 0;
				ddz_one.onepxjs = 0;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}

		}
		if (Bankxin == 1)
		{
			if (OnGameCard == 1 || DownGameCard == 1)
			{
				if (ddz_two_fx.ps > 0)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_san.ps == 1)
				{
					bool SetToJD = Set_outCard(1);
					if (SetToJD == true)
						return true;
				}
				if (ddz_san.ps > 1)
				{
					bool SetToJD = Set_outCard(3);
					if (SetToJD == true)
						return true;
				}

			}
		}
		if (Bankxin == 3)
		{
			if (DownGameCard == 2&&DownCard.Cdata[0]!= DownCard.Cdata[1])
			{
			
				if (ddz_lian_1.ps > 0)
				{
					mOutCard.cbCardCount = ddz_lian_1.lian_1;
					memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
					ddz_lian_1.lianpxjs -= 1;
					ddz_lian_1.ps -= ddz_lian_1.lian_1;
					ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
					ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
					ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
					ddz_lian_1.lian_4 = 0;
					memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
					memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_3));
					memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_4));
					return true;
				}
				if (ddz_twolian.lianpxjs > 0)
				{
					mOutCard.cbCardCount = ddz_twolian.lian_1;
					memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
					memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
					memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
					ddz_twolian.lianpxjs -= 1;
					ddz_twolian.ps -= ddz_twolian.lian_1;
					ddz_twolian.lian_1 = ddz_twolian.lian_2;
					return true;
				}
				if (ddz_san.ps > 0 && ddz_three.ps > 0)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_three.three_js -= 1;
					ddz_three.ps -= 3;
					memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
					memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
					memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
					memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
					ddz_three.ddz_three_5[0] = { 0 };
					ddz_three.ddz_three_5[1] = { 0 };
					ddz_three.ddz_three_5[2] = { 0 };
					ddz_san.ddz_san_1[0] = { 0 };
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;

				}
				if (ddz_two_fx.ps > 0 && ddz_three.ps > 0)
				{
					memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
					mOutCard.cbCardCount = 5;
					ddz_three.ps -= 3;
					ddz_three.three_js -= 1;
					memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
					memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
					memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
					memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
					memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
					mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_san.ps > 0 && ddz_one.ps == 3 && pssj_vh <10&&ddz_two.ps==2)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_one.onepxjs = 0;
					ddz_one.ps = 0;
					ddz_san.ddz_san_1[0] = { 0 };
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;

				}
				if (ddz_two_fx.ps > 0)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}

				if (ddz_san.ps == 1)
				{
					bool SetToJD = Set_outCard(1);
					if (SetToJD == true)
						return true;
				}
				if (ddz_san.ps > 1)
				{
					bool SetToJD = Set_outCard(3);
					if (SetToJD == true)
						return true;
				}

			}

			if (DownGameCard == 1)
			{
				if (ddz_two_fx.ps > 0)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_lian_1.ps > 0)
				{
					mOutCard.cbCardCount = ddz_lian_1.lian_1;
					memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
					ddz_lian_1.lianpxjs -= 1;
					ddz_lian_1.ps -= ddz_lian_1.lian_1;
					ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
					ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
					ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
					ddz_lian_1.lian_4 = 0;
					memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
					memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_3));
					memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_4));
					return true;
				}
				if (ddz_twolian.lianpxjs > 0)
				{
					mOutCard.cbCardCount = ddz_twolian.lian_1;
					memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
					memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
					memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
					ddz_twolian.lianpxjs -= 1;
					ddz_twolian.ps -= ddz_twolian.lian_1;
					ddz_twolian.lian_1 = ddz_twolian.lian_2;
					return true;
				}
				if (ddz_san.ps > 0 && ddz_three.ps >0)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_three.three_js -= 1;
					ddz_three.ps -= 3;
					memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
					memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
					memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
					memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
					ddz_three.ddz_three_5[0] = { 0 };
					ddz_three.ddz_three_5[1] = { 0 };
					ddz_three.ddz_three_5[2] = { 0 };
					ddz_san.ddz_san_1[0] = { 0 };
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;
				
}
				if (ddz_two_fx.ps > 0 && ddz_three.ps>0)
				{
					memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
					mOutCard.cbCardCount = 5;
					ddz_three.ps -= 3;
					ddz_three.three_js -= 1;
					memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
					memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
					memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
					memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
					memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
					mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
            	if (ddz_san.ps == 1)
				{
					bool SetToJD = Set_outCard(1);
					if (SetToJD == true)
						return true;
				}
				if (ddz_san.ps > 1)
				{
					bool SetToJD = Set_outCard(3);
					if (SetToJD == true)
						return true;
				}

			}
		}
		if (Bankxin == 2)
		{
			if (OnGameCard == 2 && onCard.Cdata[0]!= onCard.Cdata[1])
			{
		
				if (ddz_lian_1.ps > 0)
				{
					mOutCard.cbCardCount = ddz_lian_1.lian_1;
					memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
					ddz_lian_1.lianpxjs -= 1;
					ddz_lian_1.ps -= ddz_lian_1.lian_1;
					ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
					ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
					ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
					ddz_lian_1.lian_4 = 0;
					memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
					memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_3));
					memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_4));
					return true;
				}
				if (ddz_twolian.lianpxjs > 0)
				{
					mOutCard.cbCardCount = ddz_twolian.lian_1;
					memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
					memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
					memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
					ddz_twolian.lianpxjs -= 1;
					ddz_twolian.ps -= ddz_twolian.lian_1;
					ddz_twolian.lian_1 = ddz_twolian.lian_2;
					return true;
				}
				if (ddz_san.ps > 0 && ddz_three.ps > 0)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_three.three_js -= 1;
					ddz_three.ps -= 3;
					memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
					memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
					memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
					memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
					ddz_three.ddz_three_5[0] = { 0 };
					ddz_three.ddz_three_5[1] = { 0 };
					ddz_three.ddz_three_5[2] = { 0 };
					ddz_san.ddz_san_1[0] = { 0 };
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;

				}
				if (ddz_two_fx.ps > 0 && ddz_three.ps > 0)
				{
					memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
					mOutCard.cbCardCount = 5;
					ddz_three.ps -= 3;
					ddz_three.three_js -= 1;
					memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
					memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
					memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
					memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
					memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
					mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_san.ps > 0 && ddz_one.ps == 3 && pssj_vh < 10 && ddz_two.ps == 2)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_one.onepxjs = 0;
					ddz_one.ps = 0;
					ddz_san.ddz_san_1[0] = { 0 };
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;

				}
				if (ddz_two_fx.ps > 0)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_san.ps > 0 && ddz_one.ps==3&&pssj_vh<10)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData,ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_one.onepxjs = 0;
					ddz_one.ps = 0;
					ddz_san.ddz_san_1[0] = { 0 };
					ddz_san.ps -= 1;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;

				}
				if (ddz_san.ps == 1)
				{
					bool SetToJD = Set_outCard(1);
					if (SetToJD == true)
						return true;
				}
				if (ddz_san.ps > 1)
				{
					bool SetToJD = Set_outCard(3);
					if (SetToJD == true)
						return true;
				}

			}
			if (OnGameCard == 1)
			{
				if (ddz_two_fx.ps > 0)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_san.ps == 1)
				{
					bool SetToJD = Set_outCard(1);
					if (SetToJD == true)
						return true;
				}
				if (ddz_san.ps > 1)
				{
					bool SetToJD = Set_outCard(3);
					if (SetToJD == true)
						return true;
				}

			}
		}
		if (SPFX.JD_SH > 0)
		{
			int i = -1;
			i = FindCard(0, 6);
			if (i != -1)
			{
				JD_JL.JD_SHPX[i] = 0;
				mOutCard.cbCardCount = ddz_twolian.lian_1;
				ddz_twolian.ps -= ddz_twolian.lian_1;
				ddz_twolian.lianpxjs -= 1;
				ddz_twolian.lian_1 = ddz_twolian.lian_2;
				memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
				memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
				memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
				return true;
			}
			i = FindCard(0, 5);
			if (i != -1)
			{
				JD_JL.JD_SHPX[i] = 0;
				mOutCard.cbCardCount = ddz_lian_1.lian_1;
				ddz_lian_1.ps -= ddz_lian_1.lian_1;
				ddz_lian_1.lianpxjs -= 1;
				ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
				ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
				ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
				ddz_lian_1.lian_4 = 0;
				ddz_lian_1.lianpxjs -= 1;
				memcpy(mOutCard.cbCardData, ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
				memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
				memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_3, sizeof(ddz_lian_1.ddzlian_3));
				memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));
				memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
				return true;
			}
			i = FindCard(0, 7);
			if (i != -1)
			{

				if (ddz_san.ps > 1 && ddz_fj.feiji_1 == 6)
				{
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
					mOutCard.cbCardCount = 8;
					JD_JL.JD_SHPX[i] = 0;
					ddz_fj.feiji_js -= 1;
					ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_2;
					ddz_fj.feiji_3 = 0;
					memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					mOutCard.cbCardData[6] = ddz_san.ddz_san_1[0];
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					mOutCard.cbCardData[7] = ddz_san.ddz_san_1[0];
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };

					return true;
				}
				if (ddz_two_fx.two_js > 1 && ddz_fj.feiji_1 == 6)
				{
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
					mOutCard.cbCardData[ddz_fj.feiji_1] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[ddz_fj.feiji_1 + 1] = ddz_two_fx.ddz_two_1[1];
					mOutCard.cbCardData[ddz_fj.feiji_1 + 2] = ddz_two_fx.ddz_two_2[0];
					mOutCard.cbCardData[ddz_fj.feiji_1 + 3] = ddz_two_fx.ddz_two_2[1];
					mOutCard.cbCardCount = 10;
					JD_JL.JD_SHPX[i] = 0;
					ddz_fj.feiji_js -= 1;
					ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_2;
					ddz_fj.feiji_3 = 0;
					memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));

					ddz_two_fx.ps -= 4;
					ddz_two_fx.two_js -= 2;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					ddz_two_fx.ddz_two_5[0] = { 0 };
					ddz_two_fx.ddz_two_5[1] = { 0 };
					ddz_two_fx.ddz_two_6[0] = { 0 };
					ddz_two_fx.ddz_two_6[0] = { 0 }; //对子位移



					return true;
				}
				if (ddz_two_fx.two_js == 1 && ddz_fj.feiji_1 == 6)
				{
					memcpy(mOutCard.cbCardData, ddz_fj.ddz_feiji_1, sizeof(ddz_fj.ddz_feiji_1));
					mOutCard.cbCardCount = 8;
					JD_JL.JD_SHPX[i] = 0;
					ddz_fj.feiji_js -= 1;
					ddz_fj.ps -= ddz_fj.feiji_1;
					ddz_fj.feiji_1 = ddz_fj.feiji_2;
					ddz_fj.feiji_2 = ddz_fj.feiji_2;
					ddz_fj.feiji_3 = 0;
					memcpy(ddz_fj.ddz_feiji_1, ddz_fj.ddz_feiji_2, sizeof(ddz_fj.ddz_feiji_2));
					memcpy(ddz_fj.ddz_feiji_2, ddz_fj.ddz_feiji_3, sizeof(ddz_fj.ddz_feiji_3));
					memset(ddz_fj.ddz_feiji_3, 0, sizeof(ddz_fj.ddz_feiji_3));
					mOutCard.cbCardData[6] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[7] = ddz_two_fx.ddz_two_1[1];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js = 0;
					memset(ddz_two_fx.ddz_two_1, 0, sizeof(ddz_two_fx.ddz_two_1));
					return true;
				}


			}

			i = FindCard(0, 4);
			if (i != -1)
			{
				if (ddz_two_fx.two_js > 0 && ddz_one.ps == 2)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					JD_JL.JD_SHPX[i] = 0;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_san.ps > 0 && ddz_one.ps == 1&&ddz_san.ps<20)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = ddz_san.ddz_san_1[0];
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					JD_JL.JD_SHPX[i] = 0;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;
				}

				if (ddz_san.ps > 0 && ddz_one.ps == 3&&ddz_san.ps<20)
				{
					memcpy(mOutCard.cbCardData, &ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs = 0;
					ddz_one.ps = 0;
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					JD_JL.JD_SHPX[i] = 0;
					mOutCard.cbCardCount = 4;
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;
				}

				if (ddz_two_fx.two_js > 0 && ddz_one.ps == 3)
				{
					memcpy(mOutCard.cbCardData, &ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs = 0;
					ddz_one.ps = 0;
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					JD_JL.JD_SHPX[i] = 0;
					mOutCard.cbCardCount = 5;
					mOutCard.cbCardData[3] = ddz_two.ddztwo_1[0];
					mOutCard.cbCardData[4] = ddz_two.ddztwo_1[1];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
			}

			i = FindCard(0, 3);
			if (i != -1)
			{

				if (ddz_two_fx.two_js > 0 && ddz_two.ps == 2)
				{
					memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
					mOutCard.cbCardCount = 2;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
				if (ddz_san.ps > 0 && ddz_two.ps == 3&&ddz_san.ps<20)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = ddz_san.ddz_san_1[0];
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					JD_JL.JD_SHPX[i] = 0;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;
				}
				if (ddz_san.ps > 0 && ddz_two.ps == 3&&ddz_san.ps<20)
				{
					memcpy(mOutCard.cbCardData, &ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
					ddz_two.ps = 0;
					ddz_two.twopxjs = 0;
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					JD_JL.JD_SHPX[i] = 0;
					mOutCard.cbCardCount = 4;
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					for (int is = 0; is < ddz_san.ps; is++)
					{
						ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
					}
					ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
					return true;
				}

				if (ddz_two_fx.two_js > 0 && ddz_two.ps == 3)
				{
					memcpy(mOutCard.cbCardData, &ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
					ddz_two.twopxjs = 0;
					ddz_two.ps = 0;
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					JD_JL.JD_SHPX[i] = 0;
					mOutCard.cbCardCount = 5;
					mOutCard.cbCardData[3] = ddz_two.ddztwo_1[0];
					mOutCard.cbCardData[4] = ddz_two.ddztwo_1[1];
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
			}

		}
	
		if (OnGameCard != 1 && DownGameCard != 1 && ddz_san.ps > 0)
		{
			if (ddz_san.ps > 0 && ddz_san.ps < 20)
			{
				ddz_san.ps -= 1;
				//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps];
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				return true;
			}
		}
		if (ddz_two_fx.ps > 0)
		{
			memcpy(mOutCard.cbCardData, &ddz_two_fx.ddz_two_1, sizeof(ddz_two_fx.ddz_two_1));
			mOutCard.cbCardCount = 2;
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
			memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_lian_1.lianpxjs > 0)
		{
			memcpy(mOutCard.cbCardData, &ddz_lian_1.ddzlian_1, sizeof(ddz_lian_1.ddzlian_1));
			mOutCard.cbCardCount = ddz_lian_1.lian_1;
			ddz_lian_1.lianpxjs -= 1;
			ddz_lian_1.ps -= ddz_lian_1.lian_1;
			ddz_lian_1.lian_1 = ddz_lian_1.lian_2;
			ddz_lian_1.lian_2 = ddz_lian_1.lian_3;
			ddz_lian_1.lian_3 = ddz_lian_1.lian_4;
			ddz_lian_1.lian_4 = 0;
			memcpy(ddz_lian_1.ddzlian_1, ddz_lian_1.ddzlian_2, sizeof(ddz_lian_1.ddzlian_2));
			memcpy(ddz_lian_1.ddzlian_2, ddz_lian_1.ddzlian_3, sizeof(ddz_lian_1.ddzlian_3));
			memcpy(ddz_lian_1.ddzlian_3, ddz_lian_1.ddzlian_4, sizeof(ddz_lian_1.ddzlian_4));

			memset(ddz_lian_1.ddzlian_4, 0, sizeof(ddz_lian_1.ddzlian_4));
			return true;
		}
		if (ddz_twolian.ps > 0)
		{
			mOutCard.cbCardCount = ddz_twolian.lian_1;
			memcpy(mOutCard.cbCardData, ddz_twolian.ddztwo_lian_1, sizeof(ddz_twolian.ddztwo_lian_1));
			memcpy(ddz_twolian.ddztwo_lian_1, ddz_twolian.ddztwo_lian_2, sizeof(ddz_twolian.ddztwo_lian_2));
			memset(ddz_twolian.ddztwo_lian_2, 0, sizeof(ddz_twolian.ddztwo_lian_2));
			ddz_twolian.lianpxjs -= 1;
			ddz_twolian.ps -= ddz_twolian.lian_1;
			ddz_twolian.lian_1 = ddz_twolian.lian_2;
			return true;
		}
		if (ddz_san.ps > 0 && ddz_three.ps > 0)
		{
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));
			mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
			ddz_three.three_js -= 1;
			ddz_three.ps -= 3;
			memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
			memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			ddz_three.ddz_three_5[0] = { 0 };
			ddz_three.ddz_three_5[1] = { 0 };
			ddz_three.ddz_three_5[2] = { 0 };
			ddz_san.ddz_san_1[0] = { 0 };
			ddz_san.ps -= 1;
			for (int is = 0; is < ddz_san.ps; is++)
			{
				ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
			}
			ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
			return true;

		}
		if (ddz_two_fx.ps > 0 && ddz_three.ps > 0)
		{
			memcpy(mOutCard.cbCardData, ddz_three.ddz_three_1, sizeof(ddz_three.ddz_three_1));  //还差带1单
			mOutCard.cbCardCount = 5;
			ddz_three.ps -= 3;
			ddz_three.three_js -= 1;
			memcpy(ddz_three.ddz_three_1, ddz_three.ddz_three_2, sizeof(ddz_three.ddz_three_2));
			memcpy(ddz_three.ddz_three_2, ddz_three.ddz_three_3, sizeof(ddz_three.ddz_three_3));
			memcpy(ddz_three.ddz_three_3, ddz_three.ddz_three_4, sizeof(ddz_three.ddz_three_4));
			memcpy(ddz_three.ddz_three_4, ddz_three.ddz_three_5, sizeof(ddz_three.ddz_three_5));
			memset(ddz_three.ddz_three_5, 0, sizeof(ddz_three.ddz_three_5));
			mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
			mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
			ddz_two_fx.ps -= 2;
			ddz_two_fx.two_js -= 1;
			memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
			memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
			memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
			memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
			memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
			memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
			return true;
		}
		if (ddz_san.ps > 0 && ddz_san.ps < 20)
		{
			ddz_san.ps -= 1;
			//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = ddz_san.ddz_san_1[ddz_san.ps];
			ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
			return true;
		}

		if (SPFX.JD_DP > 0)
		{
			int is;
			is = FindCard(1, 6);
			if (is != -1)
			{
				
					mOutCard.cbCardCount = ddz_one.ps;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					ddz_one.ps = 0;
					ddz_one.onepxjs = 0;
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					JD_JL.JD_DPPX[is] = 0;
					return true;
			
			}
			is = FindCard(1, 5);
			if (is != -1)
			{
			
				mOutCard.cbCardCount = ddz_one.ps;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				ddz_two.ps = 0;
				ddz_two.twopxjs = 0;
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				JD_JL.JD_DPPX[is] = 0;
				return true;
			
			}
			is = FindCard(1, 4);
			if (is != -1)
			{
				mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 14;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 1;
					ddz_max.ddzmax_2 = 0;
					JD_JL.JD_DPPX[is] = 0;
					return true;
			
			}
			is = FindCard(1, 3);
			if (is != -1)
			{
				mOutCard.cbCardCount = 1;
				mOutCard.cbCardData[0] = 15;
				ddz_max.maxpxjs = 0;
				ddz_max.ps -= 1;
				ddz_max.ddzmax_1 = 0;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 1);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
				memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
				memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
				memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
				memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
				ddz_zd.zdpxjs -= 1;
				ddz_zd.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 8);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				ddz_one.onepxjs -= 4;
				ddz_one.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 7);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.twopxjs -= 4;
				ddz_two.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 2);
			if (is != -1)
			{
				mOutCard.cbCardCount = 2;
				mOutCard.cbCardData[0] = 14;
				mOutCard.cbCardData[1] = 15;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				ddz_max.maxpxjs = 0;
				ddz_max.ps -= 2;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
		}

	}
	if (XCard == 1) //管人家牌1单，2对，3，三不带，4，三带1，5，三带二，6飞机，7双顺，8，连,9 四带2个单，或者带2对,0为炸弹
	{
		if (Bankxin == 3 && ddz_old.old_Card == 1)
		{
			if (ddz_old.old_Card_data[0] == 2 || ddz_old.old_Card_data[0] == 14 || ddz_old.old_Card_data[0] == 15)
				return false;
		}
	
		if (SPFX.SP_SH == 2)
		{
			int is;
			is = FindCard(1, 1);
			if (is != -1)
			{
				if (ddz_old.old_Card_data[0] == ddz_old.old_Card_data[3] && ddz_old.old_Count == 4)
				{
					if (ddz_zd.ddzzd_1[0] > ddz_old.old_Card_data[0] && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}else
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}


			}
			is = FindCard(1, 8);
			if (is != -1)
			{
				if (ddz_old.old_Card_data[0] == ddz_old.old_Card_data[3] && ddz_old.old_Count == 4)
				{
					if (ddz_old.old_Card_data[0] != 2)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
						memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
						ddz_one.onepxjs -= 4;
						ddz_one.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				else
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}

			}
			is = FindCard(1, 7);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.twopxjs -= 4;
				ddz_two.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 2);
			if (is != -1)
			{
				mOutCard.cbCardCount = 2;
				mOutCard.cbCardData[0] = 14;
				mOutCard.cbCardData[1] = 15;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				ddz_max.maxpxjs = 0;
				ddz_max.ps -= 2;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
		}
		if (SPFX.SP_SH == 3 && SPFX.JD_DP > 1)
		{
			int is;
			is = FindCard(1, 1);
			if (is != -1)
			{
				if (ddz_old.old_Card_data[0] == ddz_old.old_Card_data[3] && ddz_old.old_Count == 4)
				{
					if (ddz_zd.ddzzd_1[0] > ddz_old.old_Card_data[0] && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				else
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
			}
			is = FindCard(1, 8);
			if (is != -1)
			{
				if (ddz_old.old_Card_data[0] != 2)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
			}
			is = FindCard(1, 7);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.twopxjs -= 4;
				ddz_two.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1,2);
			if (is != -1)
			{
				mOutCard.cbCardCount = 2;
				mOutCard.cbCardData[0] = 14;
				mOutCard.cbCardData[1] = 15;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				ddz_max.maxpxjs = 0;
				ddz_max.ps -= 2;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
		}
		if (SPFX.SP_SH == 3 && SPFX.JD_DP > 0 && SPFX.JD_SH > 0)
		{
			int is;
			is = FindCard(1, 1);
			if (is != -1)
			{
				if (ddz_old.old_Card_data[0] == ddz_old.old_Card_data[3] && ddz_old.old_Count == 4)
				{
					if (ddz_zd.ddzzd_1[0] > ddz_old.old_Card_data[0] && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				else
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
			}
			is = FindCard(1, 8);
			if (is != -1)
			{
				if (ddz_old.old_Card_data[0] != 2)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
			}
			is = FindCard(1, 7);
			if (is != -1)
			{
				mOutCard.cbCardCount = 4;
				memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.twopxjs -= 4;
				ddz_two.ps -= 4;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
			is = FindCard(1, 2);
			if (is != -1)
			{
				mOutCard.cbCardCount = 2;
				mOutCard.cbCardData[0] = 14;
				mOutCard.cbCardData[1] = 15;
				ddz_max.ddzmax_1 = 0;
				ddz_max.ddzmax_2 = 0;
				ddz_max.maxpxjs = 0;
				ddz_max.ps -= 2;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
		}


	
		//绝对手牌 1大王，2是小王，3是 2，4是 1，5是连，6是双顺，7是飞机
		if (SPFX.JD_SH > 0)
		{
			int i;
			switch (CardX)
			{
			case 1:
				i = FindCard(0, 4);
				if (i != -1)
				{
					if (ddz_one.ps == 1&&ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 1;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.onepxjs -= 1;
						ddz_one.ps -= 1;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
					if (ddz_one.ps == 3 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 1;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.onepxjs -= 1;
						ddz_one.ps -= 1;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				i = FindCard(0, 3);
				if (i != -1)
				{
					if (ddz_two.ps == 1 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 2;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ps -= 1;
						ddz_two.twopxjs -= 1;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
					if (ddz_two.ps == 3 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14&& ddz_old.old_Card_data[0] != 15)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 2;
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ps -= 1;
						ddz_two.twopxjs -= 1;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				i = FindCard(0, 2);
				if (i != -1)
				{
					if (ddz_max.maxpxjs != 3&&ddz_old.old_Card_data[0] != 15)
					{
						ddz_max.ddzmax_2 = 0;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 1;
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 14;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				i = FindCard(0, 1);
				if (i != -1)
				{
					if (ddz_max.maxpxjs != 3)
					{
						ddz_max.ddzmax_1 = 0;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 1;
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 15;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				break;
			case 2:
				i = FindCard(0, 4);
				if (i != -1)
				{
					if (ddz_one.ps == 2&&ddz_old.old_Card_data[0]!=1&& ddz_old.old_Card_data[0]!=2)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.ddzone_1[1] = 0;
						ddz_one.onepxjs -= 2;
						ddz_one.ps -= 2;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
	
				}
				i = FindCard(0, 3);
				if (i != -1)
				{
					if (ddz_two.ps == 2 && ddz_old.old_Card_data[0] != 2)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ps -= 2;
						ddz_two.twopxjs -= 2;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				i = FindCard(0, 4);
				if (i != -1)
				{
				
					if (ddz_one.ps == 3 && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						ddz_one.ddzone_1[1] = 0;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.onepxjs -= 2;
						ddz_one.ps -= 2;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				i = FindCard(0, 3);
				if (i != -1)
				{
				
					if (ddz_two.ps == 3 && ddz_old.old_Card_data[0] != 2)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ps -= 2;
						ddz_two.twopxjs -= 2;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				break;
			case 3:
				i = FindCard(0, 4);
				if (i != -1)
				{

					if (ddz_one.ps == 3&& ddz_old.old_Card_data[1] != 2)
					{
						mOutCard.cbCardCount = 3;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						mOutCard.cbCardData[2] = 1;
						ddz_one.ddzone_1[1] = 0;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.onepxjs -= 3;
						ddz_one.ps -= 3;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				i = FindCard(0, 3);
				if (i != -1)
				{

					if (ddz_two.ps == 3 &&ddz_old.old_Card_data[1] != 2)
					{
						mOutCard.cbCardCount = 3;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						mOutCard.cbCardData[2] = 2;
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ps -= 3;
						ddz_two.twopxjs -= 3;
						JD_JL.JD_SHPX[i] = 0;
						return true;
					}
				}
				break;
			case 4:
				i = FindCard(0, 4);
				if (i != -1)
				{
					if (ddz_one.ps == 3 && ddz_san.ps > 0 && ddz_old.old_Card_data[1] !=1 && ddz_old.old_Card_data[1] != 2)
					{
						mOutCard.cbCardCount = 4;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						mOutCard.cbCardData[2] = 1;
						mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
						ddz_one.ddzone_1[1] = 0;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.onepxjs -= 3;
						ddz_one.ps -= 3;
						JD_JL.JD_SHPX[i] = 0;
						ddz_san.ps -= 1;
						       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
						}
						ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
						return true;
					}
				}
				i = FindCard(0, 3);
				if (i != -1)
				{
					if (ddz_two.ps == 3 && ddz_san.ps > 0 && ddz_old.old_Card_data[1] != 1 && ddz_old.old_Card_data[1] != 2)
					{
						mOutCard.cbCardCount = 4;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						mOutCard.cbCardData[2] = 2;
						mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ps -= 3;
						ddz_two.twopxjs -= 3;
						ddz_san.ps -= 1;
						       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						JD_JL.JD_SHPX[i] = 0;
						for (int is = 0; is < ddz_san.ps; is++)
						{
							ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
						}
						ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
						return true;
					}
				}
				i = FindCard(0, 4);
				if (i != -1)
				{
					if (ddz_one.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[1] != 1 && ddz_old.old_Card_data[1] != 2)
					{
						mOutCard.cbCardCount = 4;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						mOutCard.cbCardData[2] = 1;
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
						ddz_one.ddzone_1[1] = 0;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.onepxjs -= 3;
						ddz_one.ps -= 3;
						ddz_san.ps += 1;
						       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
						return true;
					}
				}
				i = FindCard(0, 3);
				if (i != -1)
				{
					if (ddz_two.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[1] != 2)
					{
						mOutCard.cbCardCount = 4;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						mOutCard.cbCardData[2] = 2;
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ps -= 3;
						ddz_two.twopxjs -= 3;
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
						ddz_san.ps += 1;
						       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
						return true;
					}
				}
				break;
			case 5:
				i = FindCard(0, 4);
				if (i != -1)
				{
					if (ddz_one.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[2] != 1 && ddz_old.old_Card_data[2] != 2)
					{
						mOutCard.cbCardCount = 5;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						mOutCard.cbCardData[2] = 1;
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_one.ddzone_1[1] = 0;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.onepxjs -= 3;
						ddz_one.ps -= 3;
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
						return true;
					}
				}
				i = FindCard(0, 3);
				if (i != -1)
				{
					if (ddz_two.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[2] != 1 && ddz_old.old_Card_data[2] != 2)
					{
						mOutCard.cbCardCount = 5;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						mOutCard.cbCardData[2] = 2;
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ps -= 3;
						ddz_two.twopxjs -= 3;
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
						return true;
					}
				}
				break;


			}
		}
		switch (CardX)
		{
		case 1:

				if (ddz_one.ps == 1 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 1;
					ddz_one.ddzone_1[0] = 0;
					ddz_one.onepxjs -= 1;
					ddz_one.ps -= 1;
			
					return true;
				}
				if (ddz_one.ps == 3 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 1;
					ddz_one.ddzone_1[2] = 0;
					ddz_one.onepxjs -= 1;
					ddz_one.ps -= 1;
			return true;
				}
		if (ddz_two.ps == 1 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 2;
					ddz_two.ddztwo_1[0] = 0;
					ddz_two.ps -= 1;
					ddz_two.twopxjs -= 1;
				
					return true;
				}
				if (ddz_two.ps == 3 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 2;
					ddz_two.ddztwo_1[2] = 0;
					ddz_two.ps -= 1;
					ddz_two.twopxjs -= 1;
			
					return true;
				}
		if (ddz_max.maxpxjs != 3 && ddz_old.old_Card_data[0] != 15&& ddz_max.maxpxjs == 2)
				{
					ddz_max.ddzmax_2 = 0;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 14;
				
					return true;
				}
		if (ddz_max.maxpxjs != 3 && ddz_max.maxpxjs==1)
				{
					ddz_max.ddzmax_1 = 0;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 1;
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 15;
			
					return true;
				}
		
			break;
		case 2:
	
				if (ddz_one.ps == 2 && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 1;
					mOutCard.cbCardData[1] = 1;
					ddz_one.ddzone_1[0] = 0;
					ddz_one.ddzone_1[1] = 0;
					ddz_one.onepxjs -= 2;
					ddz_one.ps -= 2;
				
					return true;
				}
				if (ddz_one.ps == 3 && ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 1;
					mOutCard.cbCardData[1] = 1;
					ddz_one.ddzone_1[1] = 0;
					ddz_one.ddzone_1[2] = 0;
					ddz_one.onepxjs -= 2;
					ddz_one.ps -= 2;
			return true;
				}
	if (ddz_two.ps == 2 && ddz_old.old_Card_data[0] != 2)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 2;
					mOutCard.cbCardData[1] = 2;
					ddz_two.ddztwo_1[0] = 0;
					ddz_two.ddztwo_1[1] = 0;
					ddz_two.ps -= 2;
					ddz_two.twopxjs -= 2;
			
					return true;
				}
				if (ddz_two.ps == 3 && ddz_old.old_Card_data[0] != 2)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 2;
					mOutCard.cbCardData[1] = 2;
					ddz_two.ddztwo_1[2] = 0;
					ddz_two.ddztwo_1[1] = 0;
					ddz_two.ps -= 2;
					ddz_two.twopxjs -= 2;
			
					return true;
				}
	
			break;
		case 3:

			if (ddz_one.ps == 3 && ddz_old.old_Card_data[1] != 2)
			{
				mOutCard.cbCardCount = 3;
				mOutCard.cbCardData[0] = 1;
				mOutCard.cbCardData[1] = 1;
				mOutCard.cbCardData[2] = 1;
				ddz_one.ddzone_1[1] = 0;
				ddz_one.ddzone_1[2] = 0;
				ddz_one.ddzone_1[0] = 0;
				ddz_one.onepxjs -= 3;
				ddz_one.ps -= 3;

				return true;
			}
			if (ddz_two.ps == 3 && ddz_old.old_Card_data[1] != 2)
			{
				mOutCard.cbCardCount = 3;
				mOutCard.cbCardData[0] = 2;
				mOutCard.cbCardData[1] = 2;
				mOutCard.cbCardData[2] = 2;
				ddz_two.ddztwo_1[2] = 0;
				ddz_two.ddztwo_1[1] = 0;
				ddz_two.ddztwo_1[0] = 0;
				ddz_two.ps -= 3;
				ddz_two.twopxjs -= 3;

				return true;

			}
	
			break;
		case 4:
	
			if (ddz_one.ps == 3 && ddz_san.ps > 0 && ddz_old.old_Card_data[1] != 1 && ddz_old.old_Card_data[1] != 2)
			{
				mOutCard.cbCardCount = 4;
				mOutCard.cbCardData[0] = 1;
				mOutCard.cbCardData[1] = 1;
				mOutCard.cbCardData[2] = 1;
				mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
				ddz_one.ddzone_1[1] = 0;
				ddz_one.ddzone_1[2] = 0;
				ddz_one.ddzone_1[0] = 0;
				ddz_one.onepxjs -= 3;
				ddz_one.ps -= 3;
				ddz_san.ps -= 1;
				//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				return true;
			}
			if (ddz_two.ps == 3 && ddz_san.ps > 0 && ddz_old.old_Card_data[1] != 1 && ddz_old.old_Card_data[1] != 2)
			{
				mOutCard.cbCardCount = 4;
				mOutCard.cbCardData[0] = 2;
				mOutCard.cbCardData[1] = 2;
				mOutCard.cbCardData[2] = 2;
				mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
				ddz_two.ddztwo_1[2] = 0;
				ddz_two.ddztwo_1[1] = 0;
				ddz_two.ddztwo_1[0] = 0;
				ddz_two.ps -= 3;
				ddz_two.twopxjs -= 3;
				ddz_san.ps -= 1;
				for (int is = 0; is < ddz_san.ps; is++)
				{
					ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
				}
				ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
				return true;
			}
			if (ddz_one.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[1] != 1 && ddz_old.old_Card_data[1] != 2)
			{
				mOutCard.cbCardCount = 4;
				mOutCard.cbCardData[0] = 1;
				mOutCard.cbCardData[1] = 1;
				mOutCard.cbCardData[2] = 1;
				mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
				ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
				ddz_one.ddzone_1[1] = 0;
				ddz_one.ddzone_1[2] = 0;
				ddz_one.ddzone_1[0] = 0;
				ddz_one.onepxjs -= 3;
				ddz_one.ps -= 3;
				ddz_san.ps += 1;
				//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				ddz_two_fx.ps -= 2;
				ddz_two_fx.two_js -= 1;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
				memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
				memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
				return true;
			}
			if (ddz_two.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[1] != 2)
			{
				mOutCard.cbCardCount = 4;
				mOutCard.cbCardData[0] = 2;
				mOutCard.cbCardData[1] = 2;
				mOutCard.cbCardData[2] = 2;
				mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
				ddz_two.ddztwo_1[2] = 0;
				ddz_two.ddztwo_1[1] = 0;
				ddz_two.ddztwo_1[0] = 0;
				ddz_two.ps -= 3;
				ddz_two.twopxjs -= 3;
				ddz_two_fx.ps -= 2;
				ddz_two_fx.two_js -= 1;
				ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
				ddz_san.ps += 1;
				//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
				memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
				memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
				memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
				memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
				memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
				return true;
			}
		
			break;
		case 5:
	if (ddz_one.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[2] != 1 && ddz_old.old_Card_data[2] != 2)
				{
					mOutCard.cbCardCount = 5;
					mOutCard.cbCardData[0] = 1;
					mOutCard.cbCardData[1] = 1;
					mOutCard.cbCardData[2] = 1;
					mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
					ddz_one.ddzone_1[1] = 0;
					ddz_one.ddzone_1[2] = 0;
					ddz_one.ddzone_1[0] = 0;
					ddz_one.onepxjs -= 3;
					ddz_one.ps -= 3;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
	if (ddz_two.ps == 3 && ddz_two_fx.ps > 0 && ddz_old.old_Card_data[2] != 1 && ddz_old.old_Card_data[2] != 2)
				{
					mOutCard.cbCardCount = 5;
					mOutCard.cbCardData[0] = 2;
					mOutCard.cbCardData[1] = 2;
					mOutCard.cbCardData[2] = 2;
					mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
					mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
					ddz_two.ddztwo_1[2] = 0;
					ddz_two.ddztwo_1[1] = 0;
					ddz_two.ddztwo_1[0] = 0;
					ddz_two.ps -= 3;
					ddz_two.twopxjs -= 3;
					ddz_two_fx.ps -= 2;
					ddz_two_fx.two_js -= 1;
					memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
					memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
					memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
					memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
					memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
					memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
					return true;
				}
			break;
		}
		if (SPFX.JD_DP > 0)
		{////绝对大牌,1炸弹，2火箭,3大王，4是小王，5是2系列，6 1系列，7 是4个2，8是4个1
			int is;
			switch (CardX)
			{
			case 1:
				is = FindCard(1, 6);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 1;
						ddz_one.ps -= 1;
						ddz_one.onepxjs -= 1;
						ddz_one.ddzone_1[ddz_one.ps] = 0;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 5);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 2;
						ddz_two.ps -= 1;
						ddz_two.twopxjs -= 1;
						ddz_two.ddztwo_1[ddz_two.ps] = 0;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 4);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[0] != 15)
					{
						mOutCard.cbCardCount = 1;
						mOutCard.cbCardData[0] = 14;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 1;
						ddz_max.ddzmax_2 = 0;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 3);
				if (is != -1)
				{
					mOutCard.cbCardCount = 1;
					mOutCard.cbCardData[0] = 15;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 1;
					ddz_max.ddzmax_1 = 0;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 1);
				if (is != -1)
				{
		
							mOutCard.cbCardCount = 4;
							memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
							memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
							memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
							memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
							memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
							ddz_zd.zdpxjs -= 1;
							ddz_zd.ps -= 4;
							JD_JL.JD_DPPX[is] = 0;
						return true;
				
				}
				is = FindCard(1, 8);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 7);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					ddz_two.twopxjs -= 4;
					ddz_two.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 2);
				if (is != -1)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 14;
					mOutCard.cbCardData[1] = 15;
					ddz_max.ddzmax_1 = 0;
					ddz_max.ddzmax_2 = 0;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 2;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				break;
			case 2:
				is = FindCard(1, 6);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15 && ddz_one.ps > 1)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						ddz_one.ddzone_1[ddz_one.ps - 1] = 0;
						ddz_one.ddzone_1[ddz_one.ps - 2] = 0;
						ddz_one.ps -= 2;
						ddz_one.onepxjs -= 2;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 5);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15 && ddz_two.ps > 1)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						ddz_two.ddztwo_1[ddz_two.ps - 1] = 0;
						ddz_two.ddztwo_1[ddz_two.ps - 2] = 0;
						ddz_two.ps -= 1;
						ddz_two.twopxjs -= 1;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 1);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 8);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 7);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					ddz_two.twopxjs -= 4;
					ddz_two.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 2);
				if (is != -1)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 14;
					mOutCard.cbCardData[1] = 15;
					ddz_max.ddzmax_1 = 0;
					ddz_max.ddzmax_2 = 0;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 2;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				break;
			case 3:
				is = FindCard(1, 6);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[0] != 1 && ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15 && ddz_one.ps == 3)
					{
						mOutCard.cbCardCount = 3;
						mOutCard.cbCardData[0] = 1;
						mOutCard.cbCardData[1] = 1;
						mOutCard.cbCardData[2] = 1;
						ddz_one.ddzone_1[0] = 0;
						ddz_one.ddzone_1[1] = 0;
						ddz_one.ddzone_1[2] = 0;
						ddz_one.ps -= 3;
						ddz_one.onepxjs -= 3;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 5);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[0] != 2 && ddz_old.old_Card_data[0] != 14 && ddz_old.old_Card_data[0] != 15 && ddz_two.ps == 3)
					{
						mOutCard.cbCardCount = 3;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						mOutCard.cbCardData[2] = 2;
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ps -= 3;
						ddz_two.twopxjs -= 3;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 1);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 8);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 7);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					ddz_two.twopxjs -= 4;
					ddz_two.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 2);
				if (is != -1)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 14;
					mOutCard.cbCardData[1] = 15;
					ddz_max.ddzmax_1 = 0;
					ddz_max.ddzmax_2 = 0;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 2;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				break;
			case 4:
				is = FindCard(1, 6);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[1] != 1 && ddz_old.old_Card_data[1] != 2 && ddz_one.ps == 3&&ddz_san.ps<20)
					{
						if (ddz_san.ps > 0)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 1;
							mOutCard.cbCardData[1] = 1;
							mOutCard.cbCardData[2] = 1;
							mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
							ddz_one.ddzone_1[0] = 0;
							ddz_one.ddzone_1[1] = 0;
							ddz_one.ddzone_1[2] = 0;
							ddz_one.ps -= 3;
							ddz_one.onepxjs -= 3;
							ddz_san.ps -= 1;
							       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
							for (int is = 0; is < ddz_san.ps; is++)
							{
								ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
							}
							ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
						if (ddz_two_fx.ps > 0)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 1;
							mOutCard.cbCardData[1] = 1;
							mOutCard.cbCardData[2] = 1;
							mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
							ddz_one.ddzone_1[0] = 0;
							ddz_one.ddzone_1[1] = 0;
							ddz_one.ddzone_1[2] = 0;
							ddz_one.ps -= 3;
							ddz_one.onepxjs -= 3;
							ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
							ddz_san.ps += 1;
							       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
							ddz_two_fx.ps -= 2;
							ddz_two_fx.two_js -= 1;
							memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
							memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
							memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
							memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
							memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
							memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
						if (ddz_lian_1.lian_1 > 5)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 1;
							mOutCard.cbCardData[1] = 1;
							mOutCard.cbCardData[2] = 1;
							mOutCard.cbCardData[3] = ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1];
							ddz_lian_1.lian_1 -= 1;
							ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1] = 0;
							ddz_one.ddzone_1[0] = 0;
							ddz_one.ddzone_1[1] = 0;
							ddz_one.ddzone_1[2] = 0;
							ddz_one.ps -= 3;
							ddz_one.onepxjs -= 3;
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
						if (ddz_two.ps == 1)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 1;
							mOutCard.cbCardData[1] = 1;
							mOutCard.cbCardData[2] = 1;
							mOutCard.cbCardData[3] = 2;
							ddz_one.ddzone_1[0] = 0;
							ddz_one.ddzone_1[1] = 0;
							ddz_one.ddzone_1[2] = 0;
							ddz_one.ps -= 3;
							ddz_one.onepxjs -= 3;
							ddz_two.ps = 0;
							ddz_two.twopxjs = 0;
							ddz_two.ddztwo_1[0] = 0;
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
					}
				}
				is = FindCard(1, 5);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[1] != 2 && ddz_two.ps == 3)
					{
						if (ddz_san.ps > 0&&ddz_san.ps<20)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 2;
							mOutCard.cbCardData[1] = 2;
							mOutCard.cbCardData[2] = 2;
							mOutCard.cbCardData[3] = ddz_san.ddz_san_1[0];
							ddz_two.ddztwo_1[0] = 0;
							ddz_two.ddztwo_1[1] = 0;
							ddz_two.ddztwo_1[2] = 0;
							ddz_two.ps -= 3;
							ddz_two.twopxjs -= 3;
							JD_JL.JD_DPPX[is] = 0;
							ddz_san.ps -= 1;
							       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
							for (int is = 0; is < ddz_san.ps; is++)
							{
								ddz_san.ddz_san_1[is] = ddz_san.ddz_san_1[is + 1];
							}
							ddz_san.ddz_san_1[ddz_san.ps] = { 0 };
							return true;
						}
						if (ddz_two_fx.ps > 0)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 2;
							mOutCard.cbCardData[1] = 2;
							mOutCard.cbCardData[2] = 2;
							mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
							ddz_two.ddztwo_1[0] = 0;
							ddz_two.ddztwo_1[1] = 0;
							ddz_two.ddztwo_1[2] = 0;
							ddz_two.ps -= 3;
							ddz_two.twopxjs -= 3;
							ddz_san.ddz_san_1[ddz_san.ps] = ddz_two_fx.ddz_two_1[0];
							ddz_san.ps += 1;
							       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
							ddz_two_fx.ps -= 2;
							ddz_two_fx.two_js -= 1;
							memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
							memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
							memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
							memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
							memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
							memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
						if (ddz_lian_1.lian_1 > 5)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 2;
							mOutCard.cbCardData[1] = 2;
							mOutCard.cbCardData[2] = 2;
							mOutCard.cbCardData[3] = ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1 - 1];
							ddz_lian_1.lian_1 -= 1;
							ddz_lian_1.ddzlian_1[ddz_lian_1.lian_1] = 0;
							ddz_two.ddztwo_1[0] = 0;
							ddz_two.ddztwo_1[1] = 0;
							ddz_two.ddztwo_1[2] = 0;
							ddz_two.ps -= 3;
							ddz_two.twopxjs -= 3;
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
						if (ddz_one.ps == 1)
						{
							mOutCard.cbCardCount = 4;
							mOutCard.cbCardData[0] = 2;
							mOutCard.cbCardData[1] = 2;
							mOutCard.cbCardData[2] = 2;
							mOutCard.cbCardData[3] = 1;
							ddz_two.ddztwo_1[0] = 0;
							ddz_two.ddztwo_1[1] = 0;
							ddz_two.ddztwo_1[2] = 0;
							ddz_two.ps -= 3;
							ddz_two.twopxjs -= 3;
							ddz_one.ddzone_1[0] = 0;
							ddz_one.onepxjs -= 1;
							ddz_one.ps -= 1;
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
					}
				}
				is = FindCard(1, 1);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 8);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 7);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					ddz_two.twopxjs -= 4;
					ddz_two.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 2);
				if (is != -1)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 14;
					mOutCard.cbCardData[1] = 15;
					ddz_max.ddzmax_1 = 0;
					ddz_max.ddzmax_2 = 0;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 2;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				break;
			case 5:
				is = FindCard(1, 6);
				if (is != -1)
				{
					if (ddz_old.old_Card_data[2] != 2 && ddz_one.ps == 3)
					{
						if (ddz_two_fx.ps > 0)
						{
							mOutCard.cbCardCount = 5;
							mOutCard.cbCardData[0] = 1;
							mOutCard.cbCardData[1] = 1;
							mOutCard.cbCardData[2] = 1;
							mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
							mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
							ddz_one.ddzone_1[0] = 0;
							ddz_one.ddzone_1[1] = 0;
							ddz_one.ddzone_1[2] = 0;
							ddz_one.ps -= 3;
							ddz_one.onepxjs -= 3;
							ddz_two_fx.ps -= 2;
							ddz_two_fx.two_js -= 1;
							memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
							memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
							memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
							memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
							memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
							memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
					}
				}
				is = FindCard(1, 5);
				if (is != 1)
				{
					if (ddz_two.ps == 3)
					{
						mOutCard.cbCardCount = 5;
						mOutCard.cbCardData[0] = 2;
						mOutCard.cbCardData[1] = 2;
						mOutCard.cbCardData[2] = 2;
						mOutCard.cbCardData[3] = ddz_two_fx.ddz_two_1[0];
						mOutCard.cbCardData[4] = ddz_two_fx.ddz_two_1[0];
						ddz_two.ddztwo_1[0] = 0;
						ddz_two.ddztwo_1[1] = 0;
						ddz_two.ddztwo_1[2] = 0;
						ddz_two.ps -= 3;
						ddz_two.twopxjs -= 3;
						ddz_two_fx.ps -= 2;
						ddz_two_fx.two_js -= 1;
						memcpy(ddz_two_fx.ddz_two_1, ddz_two_fx.ddz_two_2, sizeof(ddz_two_fx.ddz_two_2));
						memcpy(ddz_two_fx.ddz_two_2, ddz_two_fx.ddz_two_3, sizeof(ddz_two_fx.ddz_two_3));
						memcpy(ddz_two_fx.ddz_two_3, ddz_two_fx.ddz_two_4, sizeof(ddz_two_fx.ddz_two_4));
						memcpy(ddz_two_fx.ddz_two_4, ddz_two_fx.ddz_two_5, sizeof(ddz_two_fx.ddz_two_5));
						memcpy(ddz_two_fx.ddz_two_5, ddz_two_fx.ddz_two_6, sizeof(ddz_two_fx.ddz_two_6));
						memset(ddz_two_fx.ddz_two_6, 0, sizeof(ddz_two_fx.ddz_two_6));
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				}
				is = FindCard(1, 1);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
					memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
					memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
					memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
					memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
					ddz_zd.zdpxjs -= 1;
					ddz_zd.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 8);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.onepxjs -= 4;
					ddz_one.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 7);
				if (is != -1)
				{
					mOutCard.cbCardCount = 4;
					memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
					memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
					ddz_two.twopxjs -= 4;
					ddz_two.ps -= 4;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				is = FindCard(1, 2);
				if (is != -1)
				{
					mOutCard.cbCardCount = 2;
					mOutCard.cbCardData[0] = 14;
					mOutCard.cbCardData[1] = 15;
					ddz_max.ddzmax_1 = 0;
					ddz_max.ddzmax_2 = 0;
					ddz_max.maxpxjs = 0;
					ddz_max.ps -= 2;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
				break;
				case 6:
					is = FindCard(1, 1);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 8);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
						memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
						ddz_one.onepxjs -= 4;
						ddz_one.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 7);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
						memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
						ddz_two.twopxjs -= 4;
						ddz_two.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 2);
					if (is != -1)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 14;
						mOutCard.cbCardData[1] = 15;
						ddz_max.ddzmax_1 = 0;
						ddz_max.ddzmax_2 = 0;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 2;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					break;
				case 7:
					is = FindCard(1, 1);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 8);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
						memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
						ddz_one.onepxjs -= 4;
						ddz_one.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 7);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
						memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
						ddz_two.twopxjs -= 4;
						ddz_two.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 2);
					if (is != -1)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 14;
						mOutCard.cbCardData[1] = 15;
						ddz_max.ddzmax_1 = 0;
						ddz_max.ddzmax_2 = 0;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 2;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
				case 8:
					is = FindCard(1, 1);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 8);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
						memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
						ddz_one.onepxjs -= 4;
						ddz_one.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 7);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
						memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
						ddz_two.twopxjs -= 4;
						ddz_two.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 2);
					if (is != -1)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 14;
						mOutCard.cbCardData[1] = 15;
						ddz_max.ddzmax_1 = 0;
						ddz_max.ddzmax_2 = 0;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 2;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					break;
				case 9:
					is = FindCard(1, 1);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
						memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
						memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
						memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
						memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
						ddz_zd.zdpxjs -= 1;
						ddz_zd.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 8);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
						memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
						ddz_one.onepxjs -= 4;
						ddz_one.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 7);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
						memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
						ddz_two.twopxjs -= 4;
						ddz_two.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 1);
					if (is != -1)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 14;
						mOutCard.cbCardData[1] = 15;
						ddz_max.ddzmax_1 = 0;
						ddz_max.ddzmax_2 = 0;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 2;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					break;
				case 0:
					is = FindCard(1, 1);
					if (is != -1)
					{
						if (ddz_zd.ddzzd_1[0] > ddz_old.old_Card_data[0])
						{
							mOutCard.cbCardCount = 4;
							memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
							memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
							memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
							memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
							memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
							ddz_zd.zdpxjs -= 1;
							ddz_zd.ps -= 4;
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
					}
					is = FindCard(1, 8);
					if (is != -1)
					{
						if (ddz_old.old_Card_data[0] != 2)
						{
							mOutCard.cbCardCount = 4;
							memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
							memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
							ddz_one.onepxjs -= 4;
							ddz_one.ps -= 4;
							JD_JL.JD_DPPX[is] = 0;
							return true;
						}
					}
					is = FindCard(1, 7);
					if (is != -1)
					{
						mOutCard.cbCardCount = 4;
						memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
						memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
						ddz_two.twopxjs -= 4;
						ddz_two.ps -= 4;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					is = FindCard(1, 2);
					if (is != -1)
					{
						mOutCard.cbCardCount = 2;
						mOutCard.cbCardData[0] = 14;
						mOutCard.cbCardData[1] = 15;
						ddz_max.ddzmax_1 = 0;
						ddz_max.ddzmax_2 = 0;
						ddz_max.maxpxjs = 0;
						ddz_max.ps -= 2;
						JD_JL.JD_DPPX[is] = 0;
						return true;
					}
					break;
			}

		}


	}
	return false;
}
int BOTProces::FindCard(int leix, int Icont)
{
	if (leix == 0) //绝手
	{
		for (int i = 0; i < SPFX.JD_SH; i++) //出双顺，飞机连
		{
			if (JD_JL.JD_SHPX[i] == Icont)
			{
				return i;
			}
		}
	}
	else if (leix == 1)//绝大
	{
		for (int i = 0; i < SPFX.JD_DP; i++) 
		{
			if (JD_JL.JD_DPPX[i] == Icont)
			{
				return i;
			}
		}
		
	}

	return -1;
}

bool BOTProces::Set_outCard(int Icont) //1出绝大,2出绝手
{
	if (Icont == 1)
	{
		int is;
		is = FindCard(1, 6);
		if (is != -1)
		{
			if (ddz_one.ps == 3)
			{
				if (ddz_san.ps > 0 && ddz_san.ps < 20)
				{
					mOutCard.cbCardCount = 4;
					mOutCard.cbCardData[0] = 1;
					mOutCard.cbCardData[1] = 1;
					mOutCard.cbCardData[2] = 1;
					ddz_san.ps -= 1;
					       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
					mOutCard.cbCardData[3] = ddz_san.ddz_san_1[ddz_san.ps];
					ddz_san.ddz_san_1[ddz_san.ps] = 0;
					memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
					ddz_one.ps = 0;
					ddz_one.onepxjs = 0;
					JD_JL.JD_DPPX[is] = 0;
					return true;
				}
			}
			mOutCard.cbCardCount = ddz_one.ps;;
			memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
			memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
			ddz_one.ps = 0;
			ddz_one.onepxjs = 0;
			JD_JL.JD_DPPX[is] = 0;
			return true;

		}
		is = FindCard(1, 5);
		if (is != -1)
		{

			mOutCard.cbCardCount = ddz_two.ps;;
			memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
			memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
			ddz_two.ps = 0;
			ddz_two.twopxjs = 0;
			JD_JL.JD_DPPX[is] = 0;
			return true;

		}
		is = FindCard(1, 4);
		if (is != -1)
		{

			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = 14;
			ddz_max.maxpxjs = 0;
			ddz_max.ps -= 1;
			ddz_max.ddzmax_2 = 0;
			JD_JL.JD_DPPX[is] = 0;
			return true;

		}
		is = FindCard(1, 3);
		if (is != -1)
		{
			mOutCard.cbCardCount = 1;
			mOutCard.cbCardData[0] = 15;
			ddz_max.maxpxjs = 0;
			ddz_max.ps -= 1;
			ddz_max.ddzmax_1 = 0;
			JD_JL.JD_DPPX[is] = 0;
			return true;
		}
		is = FindCard(1, 1);
		if (is != -1)
		{
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_zd.ddzzd_1, sizeof(ddz_zd.ddzzd_1));
			memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
			memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
			memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
			memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
			ddz_zd.zdpxjs -= 1;
			ddz_zd.ps -= 4;
			JD_JL.JD_DPPX[is] = 0;
			return true;
		}
		is = FindCard(1, 8);
		if (is != -1)
		{
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_one.ddzone_1, sizeof(ddz_one.ddzone_1));
			memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
			ddz_one.onepxjs -= 4;
			ddz_one.ps -= 4;
			JD_JL.JD_DPPX[is] = 0;
			return true;
		}
		is = FindCard(1, 7);
		if (is != -1)
		{
			mOutCard.cbCardCount = 4;
			memcpy(mOutCard.cbCardData, ddz_two.ddztwo_1, sizeof(ddz_two.ddztwo_1));
			memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
			ddz_two.twopxjs -= 4;
			ddz_two.ps -= 4;
			JD_JL.JD_DPPX[is] = 0;
			return true;
		}
		is = FindCard(1, 2);
		if (is != -1)
		{
			mOutCard.cbCardCount = 2;
			mOutCard.cbCardData[0] = 14;
			mOutCard.cbCardData[1] = 15;
			ddz_max.ddzmax_1 = 0;
			ddz_max.ddzmax_2 = 0;
			ddz_max.maxpxjs = 0;
			ddz_max.ps -= 2;
			JD_JL.JD_DPPX[is] = 0;
			return true;
		}
	}

	if (Icont == 3)
	{
		int is;
		is = FindCard(1, 6);
		if (is != -1)
		{
			if (ddz_one.ps == 3&&ddz_san.ps>0&&ddz_san.ps<20)
			{
				mOutCard.cbCardCount = 4;
				mOutCard.cbCardData[0] = 1;
				mOutCard.cbCardData[1] = 1;
				mOutCard.cbCardData[2] = 1;
				ddz_san.ps -= 1;
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;//
				mOutCard.cbCardData[3] = ddz_san.ddz_san_1[ddz_san.ps];
				ddz_san.ddz_san_1[ddz_san.ps] = 0;
				memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
				ddz_one.ps = 0;
				ddz_one.onepxjs = 0;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
		}
		is = FindCard(1, 5);
		if (is != -1)
		{

			if (ddz_two.ps == 3 && ddz_san.ps>0 && ddz_san.ps<20)
			{
				mOutCard.cbCardCount = 4;
				mOutCard.cbCardData[0] = 2;
				mOutCard.cbCardData[1] =2;
				mOutCard.cbCardData[2] = 2;
				ddz_san.ps -= 1;
				       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
				mOutCard.cbCardData[3] = ddz_san.ddz_san_1[ddz_san.ps];
				ddz_san.ddz_san_1[ddz_san.ps] = 0;
				memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
				ddz_two.ps = 0;
				ddz_two.twopxjs = 0;
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
	

		}
    	is = FindCard(1, 1);
		if (is != -1)
		{
			if (ddz_san.ps > 1 && ddz_san.ps < 20)
			{
				SET_FOUR_TAKE_TWO();
				JD_JL.JD_DPPX[is] = 0;
				return true;
			}
		}
		is = FindCard(1, 8);
		if (is != -1)
		{
			mOutCard.cbCardCount = 6;
			mOutCard.cbCardData[0] = 1;
			mOutCard.cbCardData[1] = 1;
			mOutCard.cbCardData[2] = 1;
			mOutCard.cbCardData[3] = 1;
			mOutCard.cbCardData[4] = ddz_san.ddz_san_1[ddz_san.ps - 1];
			mOutCard.cbCardData[5] = ddz_san.ddz_san_1[ddz_san.ps - 2];
			memset(ddz_one.ddzone_1, 0, sizeof(ddz_one.ddzone_1));
			ddz_san.ddz_san_1[ddz_san.ps - 1] = 0;
			ddz_san.ddz_san_1[ddz_san.ps - 2] = 0;
			ddz_san.ps -= 2;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			ddz_one.onepxjs = 0;
			ddz_one.ps= 0;
			JD_JL.JD_DPPX[is] = 0;
			return true;
		}
		is = FindCard(1, 7);
		if (is != -1)
		{
			mOutCard.cbCardCount = 6;
			mOutCard.cbCardData[0] =2;
			mOutCard.cbCardData[1] = 2;
			mOutCard.cbCardData[2] =2;
			mOutCard.cbCardData[3] = 2;
			mOutCard.cbCardData[4] = ddz_san.ddz_san_1[ddz_san.ps - 1];
			mOutCard.cbCardData[5] = ddz_san.ddz_san_1[ddz_san.ps - 2];
			memset(ddz_two.ddztwo_1, 0, sizeof(ddz_two.ddztwo_1));
			ddz_san.ddz_san_1[ddz_san.ps - 1] = 0;
			ddz_san.ddz_san_1[ddz_san.ps - 2] = 0;
			ddz_san.ps -= 2;
			       //std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
			ddz_two.ps= 0;
			ddz_two.twopxjs = 0;
			JD_JL.JD_DPPX[is] = 0;
			return true;
		}

	}
	return false;
}
bool BOTProces::SetGOCard(string str)
{
	CT_DWORD sz1, sz2, sz3, sz4;
	if (str== "tps")
	{

for (int ist = 0; ist < mOutCard.cbCardCount; ist++)
		{
			sz1 = mOutCard.cbCardData[ist];
			sz2 = sz1 + 16;
			sz3 = sz2 + 16;
			sz4 = sz3 + 16;
			if (sz1 == 15)
				sz1 = 79;
			if (sz1 == 14)
				sz1 = 78;
			for (int itp = 0; itp < MAX_COUNT; itp++)
			{
				if (myCard[itp] == sz1 || myCard[itp] == sz2 || myCard[itp] == sz3 || myCard[itp] == sz4)
				{
					if (sz1 == 78)
					{
						mOutCard.cbCardData[ist] = 0x4e;
						myCard[itp] = { 0 };

						break;
					}
					if (sz1 == 79)
					{
						mOutCard.cbCardData[ist] = 0x4f;
						myCard[itp] = { 0 };

						break;
					}
					mOutCard.cbCardData[ist] = myCard[itp];
					myCard[itp] = { 0 };

					break;
				}
			}
		}
	}

	return true;
}
CT_VOID BOTProces::Set_SHHB()
{
	if (SPFX.SP_SH == 2 && ddz_two.ps == 3 && ddz_san.ps == 1 && pssj_vh == 4) //三2 一个单
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_two.ps == 3 && ddz_two_fx.two_js == 1 && pssj_vh == 4)//三2 一个对
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_one.ps == 3 && ddz_san.ps == 1 && pssj_vh == 4)//三1 一个单
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_one.ps == 3 && ddz_two_fx.two_js == 1 && pssj_vh == 5)//三1 一个对
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_one.ps == 3 && ddz_max.ps == 1 && pssj_vh == 4)//三1 一个一个王
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_one.ps == 3 && ddz_two.ps == 1 && pssj_vh == 4)//三1 一个2
	{

		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_two.ps == 3 && ddz_max.ps == 1 && pssj_vh == 4)//三2 一个wang
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_three.ps == 3 && ddz_two.ps == 1 && pssj_vh == 4)
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_three.ps == 3 && ddz_one.ps == 1 && pssj_vh == 4)
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_three.ps == 3 && ddz_max.ps == 1 && pssj_vh == 4)
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_three.ps == 3 && ddz_one.ps == 2 && pssj_vh == 5)
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2 && ddz_three.ps == 3 && ddz_two.ps == 2 && pssj_vh == 5)
	{
		SPFX.SP_SH -= 1;
	}
	else if (SPFX.SP_SH == 2&&ddz_three.ps > 1 && ddz_two.ps == 0 && ddz_two_fx.two_js == 0 && ddz_one.ps == 0 && ddz_san.ps == 1 && ddz_max.maxpxjs== 0)
	{
		SPFX.SP_SH += ddz_three.three_js - ddz_san.ps;
	}
	else if (SPFX.SP_SH == 2 && ddz_three.ps > 1 && ddz_two.ps == 1 && ddz_two_fx.two_js == 0 && ddz_one.ps == 0 && ddz_san.ps == 0 && ddz_max.maxpxjs == 0)
	{
		SPFX.SP_SH += ddz_three.three_js - ddz_two_fx.two_js;
	}

	if (ddz_two.ps == 0 && ddz_one.ps == 0 && ddz_max.ps == 0 && ddz_three.three_js==3&& SPFX.SP_SH == 1&&ddz_two_fx.two_js==1)
	{
		SPFX.SP_SH = 3;
	}



}
CT_VOID BOTProces:: SET_FOUR_TAKE_TWO()
{
	mOutCard.cbCardCount = 6;
	mOutCard.cbCardData[0] = ddz_zd.ddzzd_1[0];
	mOutCard.cbCardData[1] = ddz_zd.ddzzd_1[1];
	mOutCard.cbCardData[2] = ddz_zd.ddzzd_1[2];
	mOutCard.cbCardData[3] = ddz_zd.ddzzd_1[3];
	mOutCard.cbCardData[4] = ddz_san.ddz_san_1[ddz_san.ps - 1];
	mOutCard.cbCardData[5] = ddz_san.ddz_san_1[ddz_san.ps - 2];
	memcpy(ddz_zd.ddzzd_1, ddz_zd.ddzzd_2, sizeof(ddz_zd.ddzzd_2));
	memcpy(ddz_zd.ddzzd_2, ddz_zd.ddzzd_3, sizeof(ddz_zd.ddzzd_3));
	memcpy(ddz_zd.ddzzd_3, ddz_zd.ddzzd_4, sizeof(ddz_zd.ddzzd_4));
	memset(ddz_zd.ddzzd_4, 0, sizeof(ddz_zd.ddzzd_4));
	ddz_san.ddz_san_1[ddz_san.ps - 1] = 0;
	ddz_san.ddz_san_1[ddz_san.ps - 2] = 0;
	ddz_san.ps -= 2;
	//std::cout << "Line:" <<__LINE__ << "         Function:"<< __FUNCTION__<<"        PS:"<< ddz_san.ps<<"           ID="<< BotID<<std::endl;
	ddz_zd.zdpxjs -= 1;
	ddz_zd.ps -= 4;

	return;
}
bool BOTProces::Set_Card_OUT()//判断炸弹是否炸
{
	if (DownGameCard == 1 || OnGameCard == 1)
	{
		if (ddz_san.ps > 0)
		{
			int pdlx;
			pdlx = 0;
			pdlx = ddz_san.ps - ddz_three.ps;
			pdlx = ddz_san.ps - (ddz_fj.feiji_js * 2);
			if (pdlx > 1)
			{
				return false;
			}
		}
	
	}
	return true;
}