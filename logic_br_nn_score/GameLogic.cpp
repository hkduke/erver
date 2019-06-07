#include "GameLogic.h"
#include <time.h>
#include <math.h>
#include <map>
#include <random>
#include "GlobalEnum.h"

//////////////////////////////////////////////////////////////////////////

//扑克数据
const CT_BYTE CGameLogic::m_byCardDataArray[MAX_CARD_TOTAL]=
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,	//方块 A - K
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,	//梅花 A - K
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,	//红桃 A - K
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D	//黑桃 A - K
};

//////////////////////////////////////////////////////////////////////////


//构造函数
CGameLogic::CGameLogic()
{
}

//析构函数
CGameLogic::~CGameLogic()
{
}

//逻辑数值
CT_BYTE CGameLogic::GetLogicValue(CT_BYTE cbCardData)
{
	//扑克属性
	CT_BYTE bCardValue = GetCardValue(cbCardData);

	//转换数值
	return (bCardValue > 10) ? (10) : bCardValue;
}

//逻辑数值
CT_BYTE CGameLogic::GetCardLogicValue(CT_BYTE cbCardData)
{
	CT_BYTE bCardValue = GetCardValue(cbCardData);
	
	//转换数值
	return (bCardValue == 1) ? (bCardValue + 13) : bCardValue;
}

//混乱扑克
CT_VOID CGameLogic::RandCardData( CT_BYTE byCardBuffer[], CT_BYTE byBufferCount )
{
    //混乱准备
    CT_BYTE cbCardData[MAX_CARD_TOTAL] = {0};
    memcpy(cbCardData,m_byCardDataArray,MAX_CARD_TOTAL);
    
    //std::random_device r;
    //混乱扑克
    CT_BYTE bRandCount=0,bPosition=0;
    do
    {
       // std::default_random_engine e1(r());
        //std::uniform_int_distribution<int> uniform_dist(0, MAX_CARD_TOTAL-bRandCount -1);
        //bPosition = uniform_dist(e1);
        bPosition=rand()%(MAX_CARD_TOTAL-bRandCount);
        byCardBuffer[bRandCount++]=cbCardData[bPosition];
        cbCardData[bPosition]=cbCardData[MAX_CARD_TOTAL-bRandCount];
    } while (bRandCount<byBufferCount && bRandCount<MAX_CARD_TOTAL);
}

//排列扑克
CT_VOID CGameLogic::SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	//数目过虑
	if (cbCardCount == 0) return;
	//转换数值
	CT_BYTE cbSortValue[MAX_COUNT];
	for (CT_BYTE i = 0; i < cbCardCount; i++) cbSortValue[i] = GetCardValue(cbCardData[i]);

	//排序操作
	bool bSorted = true;
	CT_BYTE cbSwitchData = 0, cbLast = cbCardCount - 1;

	do
	{
		bSorted = true;
		for (CT_BYTE i = 0; i < cbLast; i++)
		{
			if ((cbSortValue[i] < cbSortValue[i + 1]) ||
				((cbSortValue[i] == cbSortValue[i + 1]) && (cbCardData[i] < cbCardData[i + 1])))
			{
				//设置标志
				bSorted = false;

				//扑克数据
				cbSwitchData = cbCardData[i];
				cbCardData[i] = cbCardData[i + 1];
				cbCardData[i + 1] = cbSwitchData;

				//排序权位
				cbSwitchData = cbSortValue[i];
				cbSortValue[i] = cbSortValue[i + 1];
				cbSortValue[i + 1] = cbSwitchData;
			}
		}
		cbLast--;
	} while (bSorted == false);
}

//排列扑克
CT_VOID CGameLogic::SortCardList2(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
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

//获取类型
CT_BYTE CGameLogic::GetCardType(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	assert(cbCardCount == MAX_COUNT);

	//是否为五小牛
	if (IsFiveSmall(cbCardData))
	{
		return OX_FIVE_SMALL;
	}
	
	//是否同花顺
	if (IsTongHuaShun(cbCardData))
	{
		return  OX_TONGHUA_SHUN;
	}

	//是否为五花牛
	if (IsGoldenBull(cbCardData))
	{
		return OX_GOLDEN_BULL;
	}

	//是否为炸弹
	if (IsBomb(cbCardData))
	{
		return OX_BOMB;
	}
	
	if (IsHuLu(cbCardData))
	{
		return OX_HULU;
	}
	
	if (IsTongHua(cbCardData))
	{
		return OX_TONGHUA;
	}
	
	if (IsShunZi(cbCardData))
	{
		return OX_SHUNZI;
	}

	//是否为银牛
	/*if (IsSilveryBull(cbCardData))
	{
		return OX_SILVERY_BULL;
	}*/

	//牛1-10
	CT_BYTE bTemp[MAX_COUNT];
	CT_BYTE bSum = 0;
	for (CT_BYTE i = 0; i < cbCardCount; ++i)
	{
		bTemp[i] = GetLogicValue(cbCardData[i]);
		bSum += bTemp[i];
	}
	for (CT_BYTE i = 0; i < cbCardCount - 1; i++)
	{
		for (CT_BYTE j = i + 1; j < cbCardCount; j++)
		{
			if ((bSum - bTemp[i] - bTemp[j]) % 10 == 0)
			{
				return ((bTemp[i] + bTemp[j]) > 10) ? (bTemp[i] + bTemp[j] - 10) : (bTemp[i] + bTemp[j]);
			}
		}
	}

	return OX_VALUE0;	//无牛
}

//对比扑克
CT_BOOL CGameLogic::CompareCard(CT_BYTE cbFirstCard[], CT_BYTE cbNextCard[])
{
	//获取点数
	CT_BYTE cbFirstType = GetCardType(cbFirstCard, MAX_COUNT);
	CT_BYTE cbNextType = GetCardType(cbNextCard, MAX_COUNT);

	if (cbFirstType!=cbNextType)
	{
	    //牌型不同
		return (cbFirstType > cbNextType) ? true : false;
	}

	//排序大到小
	CT_BYTE bFirstTemp[MAX_COUNT] = { 0 };
	CT_BYTE bNextTemp[MAX_COUNT] = { 0 };
	memcpy(bFirstTemp, cbFirstCard, MAX_COUNT);
	memcpy(bNextTemp, cbNextCard, MAX_COUNT);
	SortCardList(bFirstTemp, MAX_COUNT);
	SortCardList(bNextTemp, MAX_COUNT);

	//牌型相等
	if (cbFirstType == OX_BOMB)
	{
		return GetCardValue(bFirstTemp[MAX_COUNT / 2]) > GetCardValue(bNextTemp[MAX_COUNT / 2]);
	}

	//有牛牌型(非牛牛)比较：比牛数；牛数相同庄吃闲。
	//if (cbFirstType > OX_VALUE0 && cbFirstType < 10)
	//{
	//	return true;
	//}
	//比较数值
	CT_BYTE cbNextMaxValue = GetCardValue(bNextTemp[0]);
	CT_BYTE cbFirstMaxValue = GetCardValue(bFirstTemp[0]);
	if (cbNextMaxValue != cbFirstMaxValue)return cbFirstMaxValue > cbNextMaxValue;

	//比较颜色
	return GetCardColor(bFirstTemp[0]) > GetCardColor(bNextTemp[0]);

	return false;
}

//获取倍数
CT_BYTE CGameLogic::GetMultiple(CT_BYTE cbCardData[], CT_BYTE cbRoomKind)
{
	CT_BYTE  cbCardType = GetCardType(cbCardData, MAX_COUNT);
	if (cbRoomKind == PRIMARY_ROOM)
	{
		/*if (cbCardType <= 6)return 1;
		else if (cbCardType <= 9)return 2;
		else if (cbCardType == 10)return 3;
		//else if (cbCardType == OX_SILVERY_BULL)return 4;
		//else if (cbCardType == OX_GOLDEN_BULL)return 5;
		else if (cbCardType == OX_BOMB)return 4;
		else if (cbCardType <= OX_FIVE_SMALL)return 4;*/
		
		if (cbCardType == OX_FIVE_SMALL)
			return 5;
		else if (cbCardType == OX_TONGHUA_SHUN || cbCardType == OX_GOLDEN_BULL || cbCardType == OX_BOMB)
			return 4;
		else if (cbCardType == OX_HULU || cbCardType == OX_TONGHUA || cbCardType == OX_SHUNZI || cbCardType == 10)
			return 3;
		else if (cbCardType == 9 || cbCardType == 8)
			return 2;
		else
			return 1;
	}
	else
	{
		/*(if (cbCardType < OX_SILVERY_BULL && cbCardType != OX_VALUE0)
		{
			return cbCardType;
		}
		else if (cbCardType >= OX_SILVERY_BULL)
		{
			return 10;
		}*/

		if (cbCardType == OX_VALUE0)
			return 1;
		else if (cbCardType > OX_VALUE0 && cbCardType < OX_SHUNZI)
			return  cbCardType;
		else if (cbCardType == OX_TONGHUA || cbCardType == OX_SHUNZI)
			return  11;
		else if (cbCardType == OX_BOMB || cbCardType == OX_HULU)
			return 12;
		else if (cbCardType == OX_GOLDEN_BULL)
			return 13;
		else if (cbCardType == OX_TONGHUA_SHUN)
			return 14;
		else if (cbCardType == OX_FIVE_SMALL)
			return 15;

		/*if (cbCardType == OX_FIVE_SMALL)
			return 10;
		else if (cbCardType == OX_TONGHUA_SHUN)
			return 9;
		else if (cbCardType == OX_GOLDEN_BULL)
			return 8;
		else if (cbCardType == OX_BOMB || cbCardType == OX_HULU)
			return 7;
		else if (cbCardType == OX_TONGHUA || cbCardType == OX_SHUNZI)
			return 6;
		else if (cbCardType == 10)
			return 5;
		else if (cbCardType == 9)
			return 4;
		else if (cbCardType == 8)
			return 3;
		else if (cbCardType == 7 || cbCardType == 6 || cbCardType == 5 || cbCardType == 4 ||cbCardType == 3 || cbCardType == 2 || cbCardType == 1)
			return 2;
		else
			return 1;*/
	}

	return 1;
}

//获取牛牛牌
CT_BOOL CGameLogic::GetOxCard(CT_BYTE cbCardData[])
{
	//设置变量
	CT_BYTE cbTemp[MAX_COUNT] = { 0 };
	CT_BYTE cbTempData[MAX_COUNT] = { 0 };
	memcpy(cbTempData, cbCardData, sizeof(cbTempData));

	CT_BYTE bSum = 0;
	for (CT_BYTE i = 0; i < MAX_COUNT; ++i)
	{
		cbTemp[i] = GetLogicValue(cbCardData[i]);
		bSum += cbTemp[i];
	}

	//查找牛牛
	for (CT_BYTE i = 0; i < MAX_COUNT - 1; ++i)
	{
		for (CT_BYTE j = i + 1; j < MAX_COUNT; ++j)
		{
			if ((bSum - cbTemp[i] - cbTemp[j]) % 10 == 0)
			{
				CT_BYTE bCount = 0;
				for (CT_BYTE k = 0; k < MAX_COUNT; ++k)
				{
					if (k != i && k != j)
					{
						cbCardData[bCount++] = cbTempData[k];
					}
				}

				assert(bCount == 3);
				cbCardData[bCount++] = cbTempData[i];
				cbCardData[bCount++] = cbTempData[j];

				return true;
			}
		}
	}
	return false;
}

//是否为五小
CT_BOOL CGameLogic::IsFiveSmall(CT_BYTE cbCardData[MAX_COUNT])
{
	CT_WORD wTotalValue = 0;
	//统计牌点总合
	for (CT_WORD i = 0; i < MAX_COUNT; i++)
	{
		CT_WORD wValue = GetCardValue(cbCardData[i]);
		if (wValue > 4)
		{
			return false;
		}
		wTotalValue += wValue;
	}
	//判断是否五小
	return (wTotalValue <= 10 && wTotalValue > 0) ? true : false;
}

//是否同花顺
CT_BOOL CGameLogic::IsTongHuaShun(CT_BYTE cbCardData[MAX_COUNT])
{
	CT_BYTE cbTempCard[MAX_COUNT] = {0};
	memcpy(cbTempCard, cbCardData, sizeof(cbTempCard));
	//排序
	SortCardList2(cbTempCard, MAX_COUNT);
	
	CT_BYTE cbFirstCardData = cbTempCard[0];
	CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbFirstCardData);
	CT_BYTE cbFirstCardColor = GetCardColor(cbFirstCardData);
	for (CT_WORD i = 1; i < MAX_COUNT; ++i)
	{
		CT_BYTE cbTempCardData = cbTempCard[i];
		//判断花色
		CT_BYTE cbTempCardColor = GetCardColor(cbTempCardData);
		if (cbTempCardColor != cbFirstCardColor)
			return false;
		
		//判断牌值
		if (cbFirstLogicValue != (GetCardLogicValue(cbTempCardData) + i))
			return false;
	}
	
	return true;
}

//是否为炸弹
CT_BOOL CGameLogic::IsBomb(CT_BYTE cbCardData[MAX_COUNT])
{
	//炸弹牌型
	CT_BYTE cbSameCount = 0;
	CT_BYTE cbTempCard[MAX_COUNT] = {0};
	memcpy(cbTempCard, cbCardData, sizeof(cbTempCard));
	//排序
	SortCardList(cbTempCard, MAX_COUNT);

	CT_BYTE bSecondValue = GetCardValue(cbTempCard[MAX_COUNT / 2]);
	for (CT_BYTE i = 0; i < MAX_COUNT; ++i)
	{
		if (bSecondValue == GetCardValue(cbTempCard[i]))
		{
			cbSameCount++;
		}
	}

	return  (4 == cbSameCount) ? true : false;
}

//是否为金牛(五花牛)
CT_BOOL CGameLogic::IsGoldenBull(CT_BYTE cbCardData[MAX_COUNT])
{
	CT_BYTE cbCount = 0;
	for (CT_BYTE i = 0; i < MAX_COUNT; ++i)
	{
		if (GetCardValue(cbCardData[i]) > 10)
		{//大于10
			++cbCount;
		}
		else
		{
			return false;
		}
	}
	return (5 == cbCount) ? true : false;
}

//是否葫芦牛
CT_BOOL CGameLogic::IsHuLu(CT_BYTE cbCardData[MAX_COUNT])
{
	CT_BYTE cbTempCard[MAX_COUNT] = {0};
	memcpy(cbTempCard, cbCardData, sizeof(cbTempCard));
	
	std::map<CT_BYTE, CT_BYTE> mapPoint;
	for (CT_WORD i = 0; i < MAX_COUNT; ++i)
	{
		CT_BYTE cbTempCardData = cbTempCard[i];
		CT_BYTE cbLogicValue = GetCardLogicValue(cbTempCardData);
		auto it = mapPoint.find(cbLogicValue);
		if (it == mapPoint.end())
		{
			mapPoint.insert(std::make_pair(cbLogicValue, 1));
		}
		else
		{
			it->second++;
		}
	}
	
	if (mapPoint.size() == 2)
		return true;
	
	return false;
}

//是否同花
CT_BOOL CGameLogic::IsTongHua(CT_BYTE cbCardData[MAX_COUNT])
{
	CT_BYTE cbTempCard[MAX_COUNT] = {0};
	memcpy(cbTempCard, cbCardData, sizeof(cbTempCard));
	//排序
	SortCardList2(cbTempCard, MAX_COUNT);
	
	CT_BYTE cbFirstCardData = cbTempCard[0];
	CT_BYTE cbFirstCardColor = GetCardColor(cbFirstCardData);
	for (CT_WORD i = 1; i < MAX_COUNT; ++i)
	{
		CT_BYTE cbTempCardData = cbTempCard[i];
		//判断花色
		CT_BYTE cbTempCardColor = GetCardColor(cbTempCardData);
		if (cbTempCardColor != cbFirstCardColor)
			return false;
	}
	
	return true;
}

//是否顺子
CT_BOOL CGameLogic::IsShunZi(CT_BYTE cbCardData[MAX_COUNT])
{
	CT_BYTE cbTempCard[MAX_COUNT] = {0};
	memcpy(cbTempCard, cbCardData, sizeof(cbTempCard));
	//排序
	SortCardList2(cbTempCard, MAX_COUNT);
	
	CT_BYTE cbFirstCardData = cbTempCard[0];
	CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbFirstCardData);
	for (CT_WORD i = 1; i < MAX_COUNT; ++i)
	{
		CT_BYTE cbTempCardData = cbTempCard[i];
		//判断牌值
		if (cbFirstLogicValue != (GetCardLogicValue(cbTempCardData) + i))
			return false;
	}
	
	return true;
}

//是否为银牛
/*CT_BOOL CGameLogic::IsSilveryBull(CT_BYTE cbCardData[MAX_COUNT])
{
	CT_BYTE cbCount = 0;
	for (CT_BYTE i = 0; i < MAX_COUNT; ++i)
	{
		if (GetCardValue(cbCardData[i]) >= 10)
		{//大于等于10
			++cbCount;
		}
		else
		{
			return false;
		}
	}
	return (5 == cbCount) ? true : false;
}*/


