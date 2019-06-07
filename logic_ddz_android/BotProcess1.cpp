#include "stdafx.h"
#include "BOTProces.h"
bool BOTProces::Set_THREE_2_TWO(CT_BYTE cbCardDatas[MAX_COUNT], CT_BYTE ICount)
{
	if (ICount == 1) //飞机带单
	{
		if(ddz_old.old_Count==8|| ddz_old.old_Count == 12)
		return true;
	}
	if (ICount == 2) //飞机带单
	{
		if (ddz_old.old_Count == 10 || ddz_old.old_Count == 15)
			return true;
	
	}


	return false;
}
CT_BYTE BOTProces::Set_THREE_PX(CT_BYTE cbCardDatas[MAX_COUNT])
{
	CT_BYTE C_Toix=0;//4组飞机;
	int is=0;
	for (CT_BYTE i = 0; i < ddz_old.old_Count;i++)
	{   
		if (cbCardDatas[i] == cbCardDatas[i + 1]&& cbCardDatas[i] == cbCardDatas[i + 2])
		{
			C_Toix = cbCardDatas[i];
	
			break;
		}

	}
	return C_Toix;


}