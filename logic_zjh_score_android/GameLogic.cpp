﻿#include "GameLogic.h"
#include <time.h>
#include "math.h"

//////////////////////////////////////////////////////////////////////////////////
//扑克数据
CT_BYTE CGameLogic::m_cbCardListData[52] =
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,	//方块 A - K
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,	//梅花 A - K
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,	//红桃 A - K
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D	//黑桃 A - K
};

//////////////////////////////////////////////////////////////////////////////////

//构造函数
CGameLogic::CGameLogic()
{
}

//析构函数
CGameLogic::~CGameLogic()
{
}

//获取类型
CT_BYTE CGameLogic::GetCardType(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	assert(cbCardCount == MAX_COUNT);

	if (cbCardCount == MAX_COUNT)
	{
		//变量定义
		bool cbSameColor = true, bLineCard = true;
		CT_BYTE cbFirstColor = GetCardColor(cbCardData[0]);
		CT_BYTE cbFirstValue = GetCardLogicValue(cbCardData[0]);

		//牌形分析
		for (CT_BYTE i = 1; i < cbCardCount; i++)
		{
			//数据分析
			if (GetCardColor(cbCardData[i]) != cbFirstColor) cbSameColor = false;
			if (cbFirstValue != (GetCardLogicValue(cbCardData[i]) + i)) bLineCard = false;

			//结束判断
			if ((cbSameColor == false) && (bLineCard == false)) break;
		}

		//特殊A32
		if (!bLineCard)
		{
			bool bOne = false, bTwo = false, bThree = false;
			for (CT_BYTE i = 0; i < MAX_COUNT; i++)
			{
				if (GetCardValue(cbCardData[i]) == 1)		bOne = true;
				else if (GetCardValue(cbCardData[i]) == 2)	bTwo = true;
				else if (GetCardValue(cbCardData[i]) == 3)	bThree = true;
			}
			if (bOne && bTwo && bThree)bLineCard = true;
		}

		//顺金类型
		if ((cbSameColor) && (bLineCard)) return CT_SHUN_JIN;

		//顺子类型
		if ((!cbSameColor) && (bLineCard)) return CT_SHUN_ZI;

		//金花类型
		if ((cbSameColor) && (!bLineCard)) return CT_JIN_HUA;

		//牌形分析
		bool bDouble = false, bPanther = true;

		//对牌分析
		for (CT_BYTE i = 0; i < cbCardCount - 1; i++)
		{
			for (CT_BYTE j = i + 1; j < cbCardCount; j++)
			{
				if (GetCardLogicValue(cbCardData[i]) == GetCardLogicValue(cbCardData[j]))
				{
					bDouble = true;
					break;
				}
			}
			if (bDouble)break;
		}

		//三条(豹子)分析
		for (CT_BYTE i = 1; i < cbCardCount; i++)
		{
			if (bPanther && cbFirstValue != GetCardLogicValue(cbCardData[i])) bPanther = false;
		}

		//对子和豹子判断
		if (bDouble == true) return (bPanther) ? CT_BAO_ZI : CT_DOUBLE_TYPE;

		//特殊235
		/*bool bTwo = false, bThree = false, bFive = false;
		for (CT_BYTE i = 0; i < cbCardCount; i++)
		{
			if (GetCardValue(cbCardData[i]) == 2)	bTwo = true;
			else if (GetCardValue(cbCardData[i]) == 3)bThree = true;
			else if (GetCardValue(cbCardData[i]) == 5)bFive = true;
		}
		if (bTwo && bThree && bFive) return CT_SPECIAL;*/
	}

	return CT_SINGLE;
}

//排列扑克
void CGameLogic::SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	//转换数值
	CT_BYTE cbLogicValue[MAX_COUNT];
	for (CT_BYTE i = 0; i < cbCardCount; i++)
		cbLogicValue[i] = GetCardLogicValue(cbCardData[i]);

	//排序操作
	bool bSorted = true;
	CT_BYTE cbTempData, bLast = cbCardCount - 1;
	do
	{
		bSorted = true;
		for (CT_BYTE i = 0; i < bLast; i++)
		{
			if ((cbLogicValue[i] < cbLogicValue[i + 1]) ||
				((cbLogicValue[i] == cbLogicValue[i + 1]) && (cbCardData[i] < cbCardData[i + 1])))
			{
				//交换位置
				cbTempData = cbCardData[i];
				cbCardData[i] = cbCardData[i + 1];
				cbCardData[i + 1] = cbTempData;
				cbTempData = cbLogicValue[i];
				cbLogicValue[i] = cbLogicValue[i + 1];
				cbLogicValue[i + 1] = cbTempData;
				bSorted = false;
			}
		}
		bLast--;
	} while (bSorted == false);

	return;
}

//混乱扑克 (洗牌)
void CGameLogic::RandCardList(CT_BYTE cbCardBuffer[], CT_BYTE cbBufferCount)
{
	//CopyMemory(cbCardBuffer,m_cbCardListData,cbBufferCount);

	//混乱准备
	CT_BYTE cbCardData[CountArray(m_cbCardListData)];
	memcpy(cbCardData, m_cbCardListData, sizeof(m_cbCardListData));  // cbCardData 整付牌，去掉大小鬼

	 //混乱扑克
	CT_BYTE bRandCount = 0, bPosition = 0;
	do
	{
		bPosition = rand() % (CountArray(m_cbCardListData) - bRandCount);
		cbCardBuffer[bRandCount++] = cbCardData[bPosition];
		cbCardData[bPosition] = cbCardData[CountArray(m_cbCardListData) - bRandCount];
	} while (bRandCount < cbBufferCount);

	/*cbCardBuffer[0] = 0x04;
	cbCardBuffer[1] = 0x03;
	cbCardBuffer[2] = 0x22;

	cbCardBuffer[3] = 0x14;
	cbCardBuffer[4] = 0x23;
	cbCardBuffer[5] = 0x32;*/

	return;
}

//逻辑数值
CT_BYTE CGameLogic::GetCardLogicValue(CT_BYTE cbCardData)
{
	//扑克属性
	//CT_BYTE bCardColor = GetCardColor(cbCardData);
	CT_BYTE bCardValue = GetCardValue(cbCardData);

	//转换数值
	return (bCardValue == 1) ? (bCardValue + 13) : bCardValue;
}

//对比扑克
CT_BYTE CGameLogic::CompareCard(CT_BYTE cbFirstData[], CT_BYTE cbNextData[], CT_BYTE cbCardCount, CT_BYTE bCompareColor/* = 0*/)
{
	//设置变量
	CT_BYTE FirstData[MAX_COUNT], NextData[MAX_COUNT];
	memcpy(FirstData, cbFirstData, sizeof(FirstData));
	memcpy(NextData, cbNextData, sizeof(NextData));

	//大小排序
	SortCardList(FirstData, cbCardCount);
	SortCardList(NextData, cbCardCount);

	//获取类型
	CT_BYTE cbNextType = GetCardType(NextData, cbCardCount);
	CT_BYTE cbFirstType = GetCardType(FirstData, cbCardCount);

	//特殊情况分析
	if ((cbNextType + cbFirstType) == (CT_SPECIAL + CT_BAO_ZI))return (CT_BYTE)(cbFirstType > cbNextType);

	//还原单牌类型
	if (cbNextType == CT_SPECIAL)cbNextType = CT_SINGLE;
	if (cbFirstType == CT_SPECIAL)cbFirstType = CT_SINGLE;

	//类型判断
	if (cbFirstType != cbNextType) return (cbFirstType > cbNextType) ? 1 : 0;

	//简单类型
	switch (cbFirstType)
	{
	case CT_BAO_ZI:			//豹子
	case CT_SINGLE:			//单牌
	case CT_JIN_HUA:		//金花
	{
		//对比数值
		for (CT_BYTE i = 0; i < cbCardCount; i++)
		{
			CT_BYTE cbNextValue = GetCardLogicValue(NextData[i]);
			CT_BYTE cbFirstValue = GetCardLogicValue(FirstData[i]);
			if (cbFirstValue != cbNextValue) return (cbFirstValue > cbNextValue) ? 1 : 0;
		}

		//如果需要比较颜色
		if (bCompareColor == 1)
		{
			for (CT_BYTE i = 0; i< cbCardCount; i++)
			{
				CT_BYTE cbNextColor = GetCardColor(NextData[i]);
				CT_BYTE cbFirstColor = GetCardColor(FirstData[i]);
				if (cbFirstColor != cbNextColor) return (cbFirstColor > cbNextColor) ? 1 : 0;
			}
		}
		return DRAW;
	}
	case CT_SHUN_ZI:		//顺子
	case CT_SHUN_JIN:		//顺金 432>A32
	{
		CT_BYTE cbNextValue = GetCardLogicValue(NextData[0]);
		CT_BYTE cbFirstValue = GetCardLogicValue(FirstData[0]);

		//特殊A32
		if (cbNextValue == 14 && GetCardLogicValue(NextData[cbCardCount - 1]) == 2)
		{
			cbNextValue = 3;
		}
		if (cbFirstValue == 14 && GetCardLogicValue(FirstData[cbCardCount - 1]) == 2)
		{
			cbFirstValue = 3;
		}

		//对比数值
		if (cbFirstValue != cbNextValue) return (cbFirstValue > cbNextValue) ? 1 : 0;;

		//如果需要比较颜色
		if (bCompareColor == 1)
		{
			for (CT_BYTE i = 0; i < cbCardCount; i++)
			{
				CT_BYTE cbNextColor = GetCardColor(NextData[i]);
				CT_BYTE cbFirstColor = GetCardColor(FirstData[i]);
				if (cbFirstColor != cbNextColor) return (cbFirstColor > cbNextColor) ? 1 : 0;
			}
		}

		return DRAW;
	}
	case CT_DOUBLE_TYPE:			//对子
	{
		CT_BYTE cbNextValue = GetCardLogicValue(NextData[0]);
		CT_BYTE cbFirstValue = GetCardLogicValue(FirstData[0]);

		//查找对子/单牌
		CT_BYTE bNextDouble = 0, bNextSingle = 0;
		CT_BYTE bFirstDouble = 0, bFirstSingle = 0;
		if (cbNextValue == GetCardLogicValue(NextData[1]))
		{
			bNextDouble = cbNextValue;
			bNextSingle = GetCardLogicValue(NextData[cbCardCount - 1]);
		}
		else
		{
			bNextDouble = GetCardLogicValue(NextData[cbCardCount - 1]);
			bNextSingle = cbNextValue;
		}
		if (cbFirstValue == GetCardLogicValue(FirstData[1]))
		{
			bFirstDouble = cbFirstValue;
			bFirstSingle = GetCardLogicValue(FirstData[cbCardCount - 1]);
		}
		else
		{
			bFirstDouble = GetCardLogicValue(FirstData[cbCardCount - 1]);
			bFirstSingle = cbFirstValue;
		}

		if (bNextDouble != bFirstDouble)return (bFirstDouble > bNextDouble) ? 1 : 0;
		if (bNextSingle != bFirstSingle)return (bFirstSingle > bNextSingle) ? 1 : 0;

		//如果需要比较颜色
		if (bCompareColor == 1)
		{
			for (CT_BYTE i = 0; i < cbCardCount; i++)
			{
				CT_BYTE cbNextColor = GetCardColor(NextData[i]);
				CT_BYTE cbFirstColor = GetCardColor(FirstData[i]);
				if (cbFirstColor != cbNextColor) return (cbFirstColor > cbNextColor) ? 1 : 0;
			}
		}

		return DRAW;
	}
	}

	return DRAW;
}

//////////////////////////////////////////////////////////////////////////////////