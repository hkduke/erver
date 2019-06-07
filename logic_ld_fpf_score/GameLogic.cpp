#include "GameLogic.h"
#include <time.h>
#include "math.h"

//////////////////////////////////////////////////////////////////////////
//静态变量

//扑克数据
const CT_BYTE CGameLogic::m_byCardDataArray[MAX_CARD_TOTAL]=
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小写
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小写
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小写
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,					//小写
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大写
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大写
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大写
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,					//大写
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

//获得随机数
CT_INT32 CGameLogic::GetRandValue()
{
    return m_rand.rand();
}

//混乱扑克
CT_VOID CGameLogic::RandCardData( CT_BYTE byCardBuffer[], CT_BYTE byBufferCount )
{
    //混乱准备
    CT_BYTE cbCardData[MAX_CARD_TOTAL] = {0};
    memcpy(cbCardData,m_byCardDataArray,MAX_CARD_TOTAL);
    
    //混乱扑克
    CT_BYTE bRandCount=0,bPosition=0;
    do
    {
        bPosition=/*m_rand.*/rand()%(MAX_CARD_TOTAL-bRandCount);
        byCardBuffer[bRandCount++]=cbCardData[bPosition];
        cbCardData[bPosition]=cbCardData[MAX_CARD_TOTAL-bRandCount];
    } while (bRandCount<byBufferCount && bRandCount<MAX_CARD_TOTAL);
}

//排序,根据牌值排序
CT_BOOL CGameLogic::SortCardList(CT_BYTE byCardData[MAX_COUNT], CT_BYTE byCardCount)
{
	//数目过虑
	if (byCardCount <= 0 || byCardCount > MAX_COUNT)
		return false;

	//排序操作
	CT_BOOL bSorted = true;
	CT_BYTE bySwitchData = 0, cbLast = byCardCount - 1;
	do
	{
		bSorted = true;
		for (CT_WORD i = 0; i < cbLast; ++i)
		{
			if (byCardData[i] > byCardData[i + 1])
			{
				//设置标志
				bSorted = false;

				//扑克数据
				bySwitchData = byCardData[i];
				byCardData[i] = byCardData[i + 1];
				byCardData[i + 1] = bySwitchData;
			}
		}
		cbLast--;
	} while (bSorted == false);

	return true;
}

//删除扑克
CT_BOOL CGameLogic::RemoveCard( CT_BYTE byCardIndex[MAX_INDEX], CT_BYTE byRemoveCard )
{
    //效验扑克
    assert(IsValidCard(byRemoveCard));
    
    CT_BYTE byTempCardIndex = SwitchToCardIndex(byRemoveCard);
    assert(byCardIndex[byTempCardIndex]>0);
    
    //删除扑克
    if (byCardIndex[byTempCardIndex]>0)
    {
        byCardIndex[byTempCardIndex]--;
        return CT_TRUE;
    }
    
    //失败效验
    assert(CT_FALSE);
    
    return CT_FALSE;
}

//有效判断
CT_BOOL CGameLogic::IsValidCard( CT_BYTE byCardData )
{
    if (byCardData<=0)
        return CT_FALSE;
    
    CT_BYTE byValue=(byCardData&MASK_VALUE);
    CT_BYTE byColor=(byCardData&MASK_COLOR)>>4;
    //小字
    CT_BOOL bSmall = ((byColor==0) && (byValue>=1)&&(byValue<=10))?CT_TRUE:CT_FALSE;
    //大字
    CT_BOOL bBig = ((byColor==1) && (byValue>=1)&&(byValue<=10))?CT_TRUE:CT_FALSE;
   
    return (bSmall || bBig)?CT_TRUE:CT_FALSE;
}

//是否红牌
CT_BOOL CGameLogic::IsHongCard(CT_BYTE byCardData)
{
	CT_BYTE cbValue = GetCardValue(byCardData);
	if (2 == cbValue || 7 == cbValue || 10 == cbValue)
	{
		return true;
	}
	return false;
}

//扑克数目
CT_BYTE CGameLogic::GetCardCount( const CT_BYTE byCardIndex[MAX_INDEX] )
{
    //数目统计
    CT_BYTE byCardCount=0;
    for (CT_WORD i=0; i<MAX_INDEX; ++i)
    {
        byCardCount += byCardIndex[i];
    }
    
    return byCardCount;
}

//扑克转换
CT_BYTE CGameLogic::SwitchToCardData( CT_BYTE byCardIndex )
{
    assert(byCardIndex<MAX_INDEX);
    
    return ((byCardIndex/10)<<4)|(byCardIndex%10+1);
}

//扑克转换(反回牌数量)
CT_BYTE CGameLogic::SwitchToCardData( const CT_BYTE byCardIndex[MAX_INDEX], CT_BYTE byCardData[MAX_COUNT] )
{
    CT_BYTE byCardCount=GetCardCount(byCardIndex);
    //转换扑克
    CT_BYTE byPosition=0;
    for (CT_BYTE i=0; i<MAX_INDEX; ++i)
	{
		if (0 == byCardIndex[i]) continue;
		for (CT_BYTE j = 0; j < byCardIndex[i]; ++j)
		{
			assert(byPosition < MAX_COUNT);
			byCardData[byPosition++] = SwitchToCardData(i);
			//转换完
			if (byCardCount == byPosition)
				return byPosition;
		}
	}
    
    return byPosition;
}

//扑克转换 
CT_BYTE CGameLogic::SwitchToCardIndex( CT_BYTE byCardData )
{
    if (byCardData<=0)
        return 0;
    
    assert(IsValidCard(byCardData));
    
    CT_BYTE byValue=(byCardData&MASK_VALUE);
    CT_BYTE byColor=(byCardData&MASK_COLOR)>>4;
    
    return ((byColor*10)+(byValue-1));
}

//扑克转换
CT_BYTE CGameLogic::SwitchToCardIndex( const CT_BYTE byCardData[], CT_BYTE byCardCount, CT_BYTE byCardIndex[MAX_INDEX] )
{
    //设置变量
    memset(byCardIndex,0,sizeof(CT_BYTE)*MAX_INDEX);
    CT_BYTE byTempCount=0;
    //转换扑克
    for (CT_WORD i=0; i<byCardCount; ++i)
    {
        if (IsValidCard(byCardData[i]))
        {
            byCardIndex[SwitchToCardIndex(byCardData[i])]++;
            byTempCount++;
        }
    }
    
    return byTempCount;
}

//提牌判断
CT_BYTE CGameLogic::GetAcitonTiCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbTiCardIndex[5])
{
	CT_BYTE cbTiCardCount = 0;
	for (CT_BYTE i=0; i<MAX_INDEX; ++i)
	{
		if (cbCardIndex[i] == 4) cbTiCardIndex[cbTiCardCount++] = i;
	}

	return cbTiCardCount;
}

//畏牌判断
CT_BYTE CGameLogic::GetActionWeiCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbWeiCardIndex[7])
{
	CT_BYTE cbWeiCardCount = 0;
	for (CT_BYTE i=0; i<MAX_INDEX; ++i)
	{
		if (cbCardIndex[i] == 3) cbWeiCardIndex[cbWeiCardCount++] = i;
	}

	return cbWeiCardCount;
}

//获得吃牌组合
CT_BYTE CGameLogic::GetChiWeave(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3])
{
	CT_BYTE cbWeaveCount = 0;
	CT_BYTE cbTempWeave[MAX_ITEM_COUNT][3];
	memset(cbTempWeave, 0, sizeof(cbTempWeave));

	if (!IsValidCard(cbCurrentCard))
	{//无效牌
		return cbWeaveCount;
	}

	//变量定义
	CT_BYTE cbCurrentIndex = SwitchToCardIndex(cbCurrentCard);

	//三牌判断
	if (cbCardIndex[cbCurrentIndex] >= 3)
	{
		return cbWeaveCount;
	}

	//大小搭吃
	CT_BYTE cbReverseIndex = (cbCurrentIndex + 10) % MAX_INDEX;
	if ((cbCardIndex[cbCurrentIndex] >= 1) && (cbCardIndex[cbReverseIndex] >= 1) && (cbCardIndex[cbReverseIndex] != 3))
	{
		cbTempWeave[cbWeaveCount][0] = cbCurrentCard;
		cbTempWeave[cbWeaveCount][1] = cbCurrentCard;
		cbTempWeave[cbWeaveCount][2] = SwitchToCardData(cbReverseIndex);
		++cbWeaveCount;
	}

	//大小搭吃
	if (cbCardIndex[cbReverseIndex] == 2)
	{
		cbTempWeave[cbWeaveCount][0] = SwitchToCardData(cbReverseIndex);
		cbTempWeave[cbWeaveCount][1] = SwitchToCardData(cbReverseIndex);
		cbTempWeave[cbWeaveCount][2] = cbCurrentCard;
		++cbWeaveCount;
	}

	//二七十吃
	CT_BYTE bCardValue = GetCardValue(cbCurrentCard);
	if ((bCardValue == 0x02) || (bCardValue == 0x07) || (bCardValue == 0x0A))
	{
		//变量定义 
		CT_BYTE cbExcursion[3] = { 1,6,9 };
		CT_BYTE cbInceptIndex = (GetCardColor(cbCurrentCard) == 0) ? 0 : 10;

		//类型判断
		bool bFlag = true;
		for (CT_WORD i = 0; i < CountArray(cbExcursion); ++i)
		{
			CT_BYTE cbIndex = cbInceptIndex + cbExcursion[i];
			if (((cbCardIndex[cbIndex] == 0) && cbIndex != cbCurrentIndex) || (cbCardIndex[cbIndex] == 3))
			{
				bFlag = false;
			}
		}

		if (bFlag)
		{//有组合
			cbTempWeave[cbWeaveCount][0] = SwitchToCardData(cbInceptIndex + cbExcursion[0]);
			cbTempWeave[cbWeaveCount][1] = SwitchToCardData(cbInceptIndex + cbExcursion[1]);
			cbTempWeave[cbWeaveCount][2] = SwitchToCardData(cbInceptIndex + cbExcursion[2]);
			++cbWeaveCount;
		}
	}

	//顺子判断
	CT_BYTE cbExcursion[3] = { 0,1,2 };
	for (CT_WORD i = 0; i < CountArray(cbExcursion); ++i)
	{
		CT_BYTE cbValueIndex = cbCurrentIndex % 10;
		if ((cbValueIndex >= cbExcursion[i]) && ((cbValueIndex - cbExcursion[i]) <= 7))
		{
			//吃牌判断
			CT_BYTE cbFirstIndex = cbCurrentIndex - cbExcursion[i];
			//if ((cbCardIndex[cbFirstIndex] == 0) || (cbCardIndex[cbFirstIndex] == 3)) continue;
			//if ((cbCardIndex[cbFirstIndex + 1] == 0) || (cbCardIndex[cbFirstIndex + 1] == 3)) continue;
			//if ((cbCardIndex[cbFirstIndex + 2] == 0) || (cbCardIndex[cbFirstIndex + 2] == 3)) continue;
			if (((cbCardIndex[cbFirstIndex] == 0) && (cbFirstIndex+0) != cbCurrentIndex) || (cbCardIndex[cbFirstIndex] == 3)) continue;
			if (((cbCardIndex[cbFirstIndex + 1] == 0) && (cbFirstIndex + 1 != cbCurrentIndex)) || (cbCardIndex[cbFirstIndex + 1] == 3)) continue;
			if (((cbCardIndex[cbFirstIndex + 2] == 0) && (cbFirstIndex + 2) != cbCurrentIndex) || (cbCardIndex[cbFirstIndex + 2] == 3)) continue;

			cbTempWeave[cbWeaveCount][0] = SwitchToCardData(cbFirstIndex);
			cbTempWeave[cbWeaveCount][1] = SwitchToCardData(cbFirstIndex + 1);
			cbTempWeave[cbWeaveCount][2] = SwitchToCardData(cbFirstIndex + 2);
			++cbWeaveCount;
		}
	}

	CT_BYTE cbTempCount = cbWeaveCount;
	//看重复组合
	for (CT_WORD i=0; i<cbTempCount; ++i)
	{
		CT_BYTE cbTempIndex[MAX_INDEX] = { 0 };
		memset(cbTempIndex, 0, sizeof(cbTempIndex));
		memcpy(cbTempIndex, cbCardIndex, sizeof(cbTempIndex));
		++cbTempIndex[cbCurrentIndex];
		CT_BYTE cbWeaveIndex0 = SwitchToCardIndex(cbTempWeave[i][0]);
		CT_BYTE cbWeaveIndex1 = SwitchToCardIndex(cbTempWeave[i][1]);
		CT_BYTE cbWeaveIndex2 = SwitchToCardIndex(cbTempWeave[i][2]);
		if (cbWeaveIndex0== cbWeaveIndex1 || cbWeaveIndex1== cbWeaveIndex2 || cbWeaveIndex0 == cbWeaveIndex2)
		{//过滤绞牌
			continue;
		}

		if ((cbTempIndex[cbWeaveIndex0] >= 2) && (cbTempIndex[cbWeaveIndex1] >= 2) && (cbTempIndex[cbWeaveIndex2] >= 2))
		{
			cbTempWeave[cbWeaveCount][0] = cbTempWeave[i][0];
			cbTempWeave[cbWeaveCount][1] = cbTempWeave[i][1];
			cbTempWeave[cbWeaveCount][2] = cbTempWeave[i][2];
			++cbWeaveCount;
		}
	}

	//统计摆牌个数
	CT_BYTE cbCardCount = FindCardCount(cbCurrentCard, cbTempWeave, cbWeaveCount);
	//吃牌总张数
	CT_BYTE cbCurCardTotal = (cbCardIndex[cbCurrentIndex] + 1);
	if (cbCardCount < cbCurCardTotal)
	{//摆不完
		return 0;
	}

	//记录组合
	memcpy(cbChiWeave, cbTempWeave, sizeof(cbTempWeave));
	return cbWeaveCount;
}

//查出指定牌数量
CT_BYTE CGameLogic::FindCardCount(CT_BYTE cbFindCard, CT_BYTE cbChiWeave[MAX_ITEM_COUNT][3], CT_BYTE cbWeaveCount)
{
	CT_BYTE cbFindIndex = SwitchToCardIndex(cbFindCard);

	CT_BYTE cbCardCount = 0;
	for (CT_WORD i = 0; i < cbWeaveCount; ++i)
	{
		CT_BYTE cbIndex[MAX_INDEX] = { 0 };
		memset(cbIndex, 0, sizeof(cbIndex));

		SwitchToCardIndex(cbChiWeave[i], 3, cbIndex);

		cbCardCount += cbIndex[cbFindIndex];
	}

	return cbCardCount;
}

//是否相同组合
CT_BOOL CGameLogic::IsSameWeave(CT_BYTE cbWeaveOne[3], CT_BYTE cbWeaveTwo[3])
{
	CT_BYTE cbOneIndex[MAX_INDEX] = { 0 };
	memset(cbOneIndex, 0, sizeof(CT_BYTE)*MAX_INDEX);
	SwitchToCardIndex(cbWeaveOne, 3, cbOneIndex);

	CT_BYTE cbTwoIndex[MAX_INDEX] = { 0 };
	memset(cbTwoIndex, 0, sizeof(CT_BYTE)*MAX_INDEX);
	SwitchToCardIndex(cbWeaveTwo, 3, cbTwoIndex);

	for (CT_WORD i = 0; i < MAX_INDEX; ++i)
	{
		if (cbOneIndex[i] != cbTwoIndex[i])
		{
			return false;
		}
	}
	return true;
}

//是否乌胡
CT_BOOL CGameLogic::IsWuHu(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount)
{//没有大小 2,7,10
	CT_BYTE cbCount = 0;
	for (CT_WORD i=0; i<cbWeaveCount; ++i)
	{
		for (CT_WORD j=0; j<WeaveItemArray[i].cbCardCount; ++j)
		{
			if (IsHongCard(WeaveItemArray[i].cbCardList[j]))
			{//牌红
				return false;
			}
			cbCount++;
		}
	}

	return (cbCount>0);
}

//十三红
CT_BOOL CGameLogic::IsShiSanHong(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount)
{
	CT_BYTE cbCount = 0;
	for (CT_WORD i = 0; i < cbWeaveCount; ++i)
	{
		for (CT_WORD j = 0; j < WeaveItemArray[i].cbCardCount; ++j)
		{
			if (IsHongCard(WeaveItemArray[i].cbCardList[j]))
			{//牌红
				++cbCount;
			}
		}
	}
	return (cbCount>=13)?true:false;
}

//十红
CT_BOOL CGameLogic::IsShiHong(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount)
{
	CT_BYTE cbCount = 0;
	for (CT_WORD i = 0; i < cbWeaveCount; ++i)
	{
		for (CT_WORD j = 0; j < WeaveItemArray[i].cbCardCount; ++j)
		{
			if (IsHongCard(WeaveItemArray[i].cbCardList[j]))
			{//牌红
				++cbCount;
			}
		}
	}
	return (cbCount >= 10) ? true : false;
}

//一块扁
CT_BOOL CGameLogic::IsYiKuaiBian(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount)
{//一块扁：红字一句话、红字一坎、红字一提，胡息x2。
	CT_BYTE cbNum = 0;
	for (CT_WORD i = 0; i < cbWeaveCount; ++i)
	{
		CT_BYTE cbCount = 0;
		for (CT_WORD j = 0; j < WeaveItemArray[i].cbCardCount; ++j)
		{
			if (IsHongCard(WeaveItemArray[i].cbCardList[j]))
			{//牌红
				++cbCount;
			}
		}
		if (cbCount == WeaveItemArray[i].cbCardCount)
		{
			++cbNum;
			if (cbNum > 1)
			{
				return false;
			}
		}
		else if (cbCount>0)
		{//有一个组合 不满足
			return false;
		}
	}
	return (cbNum == 1) ? true : false;
}

//一点红
CT_BOOL CGameLogic::IsYiDianHong(tagWeaveItem WeaveItemArray[MAX_WEAVE], CT_BYTE cbWeaveCount)
{
	CT_BYTE cbCount = 0;
	for (CT_WORD i = 0; i < cbWeaveCount; ++i)
	{
		for (CT_WORD j = 0; j < WeaveItemArray[i].cbCardCount; ++j)
		{
			if (IsHongCard(WeaveItemArray[i].cbCardList[j]))
			{//牌红
				++cbCount;
				if (cbCount>1) 
				{
					return false;
				}
			}
		}
	}
	return (cbCount == 1) ? true : false;
}

//胡牌结果
CT_BOOL CGameLogic::GetHuCardInfo(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard, CT_BYTE cbHuXiWeave, tagHuCardInfo & HuCardInfo)
{
	std::vector<tagAnalyseItem> AnalyseItemArray;
	AnalyseItemArray.clear();

	//构造扑克
	CT_BYTE cbCardIndexTemp[MAX_INDEX];
	memcpy(cbCardIndexTemp, cbCardIndex, sizeof(cbCardIndexTemp));
	//设置结果
	memset(&HuCardInfo, 0, sizeof(HuCardInfo));

	//提取三牌
	for (CT_BYTE i=0; i<MAX_INDEX; ++i)
	{
		if (cbCardIndexTemp[i] == 3)
		{
			//设置扑克
			cbCardIndexTemp[i] = 0;

			//设置组合
			CT_BYTE cbCardData = SwitchToCardData(i);
			CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardCount = 3;
			HuCardInfo.WeaveItemArray[cbIndex].cbWeaveKind = ACK_WEI;
			HuCardInfo.WeaveItemArray[cbIndex].cbCenterCard = cbCardData;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[0] = cbCardData;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[1] = cbCardData;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[2] = cbCardData;
			//设置胡息
			HuCardInfo.cbHuXiCount += GetWeaveHuXi(HuCardInfo.WeaveItemArray[cbIndex]);
		}
	}

	//提取碰牌
	if (0!= cbCurrentCard && IsValidCard(cbCurrentCard))
	{
		if (IsWeiPengCard(cbCardIndexTemp, cbCurrentCard))
		{
			CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardCount = 3;
			HuCardInfo.WeaveItemArray[cbIndex].cbWeaveKind = ACK_PENG;
			HuCardInfo.WeaveItemArray[cbIndex].cbCenterCard = cbCurrentCard;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[0] = cbCurrentCard;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[1] = cbCurrentCard;
			HuCardInfo.WeaveItemArray[cbIndex].cbCardList[2] = cbCurrentCard;
			//删除扑克
			cbCardIndexTemp[SwitchToCardIndex(cbCurrentCard)] = 0;
			//设置胡息
			HuCardInfo.cbHuXiCount += GetWeaveHuXi(HuCardInfo.WeaveItemArray[cbIndex]);
		}
		else
		{//加入到手牌里
			++cbCardIndexTemp[SwitchToCardIndex(cbCurrentCard)];
		}
	}

	if (!AnalyseCard(cbCardIndexTemp, AnalyseItemArray))
	{//没有可胡的牌
		return false;
	}

	//寻找最优
	CT_BYTE cbHuXiCard = 0;
	CT_INT32 nBestItem = -1;
	for (CT_INT32 i=0; i != (CT_INT32)AnalyseItemArray.size(); ++i)
	{
		//获取子项
		tagAnalyseItem * pAnalyseItem = &AnalyseItemArray[i];

		//胡息分析
		if (pAnalyseItem->cbHuXiCount >= cbHuXiCard)
		{//最大胡息
			nBestItem = i;
			cbHuXiCard = pAnalyseItem->cbHuXiCount;
		}
	}

	//设置结果
	HuCardInfo.cbHuXiCount += cbHuXiCard;
	if (nBestItem >= 0)
	{
		//获取子项
		tagAnalyseItem * pAnalyseItem = &AnalyseItemArray[nBestItem];

		//牌眼
		HuCardInfo.cbCardEye = pAnalyseItem->cbCardEye;

		//设置组合
		for (CT_WORD i=0; i<pAnalyseItem->cbWeaveCount; i++)
		{
			CT_BYTE cbIndex = HuCardInfo.cbWeaveCount++;
			HuCardInfo.WeaveItemArray[cbIndex] = pAnalyseItem->WeaveItemArray[i];
		}
	}

	return ((HuCardInfo.cbHuXiCount + cbHuXiWeave) >= 15);
}

//分析扑克
CT_BOOL CGameLogic::AnalyseCard(const CT_BYTE cbCardIndex[MAX_INDEX], std::vector<tagAnalyseItem>& AnalyseItemArray)
{
	//变量定义
	CT_BYTE cbWeaveItemCount = 0;
	tagWeaveItem WeaveItem[76];
	memset(WeaveItem,0,sizeof(WeaveItem));

	//数目统计
	CT_BYTE cbCardCount = GetCardCount(cbCardIndex);

	//数目判断
	if (cbCardCount == 0) return true;
	if ((cbCardCount % 3 != 0) && ((cbCardCount + 1) % 3 != 0)) return false;

	//需求计算
	CT_BYTE bLessWeavItem = cbCardCount / 3;
	bool bNeedCardEye = ((cbCardCount + 1) % 3 == 0);

	//单吊判断
	if ((bLessWeavItem == 0) && (bNeedCardEye == true))
	{
		CT_BOOL bDanDiao = AnalyDanDiao(cbCardIndex, AnalyseItemArray);
		return bDanDiao;
	}

	//分析组合
	cbWeaveItemCount = AnalyseWeaveItem(cbCardIndex, WeaveItem);

	//组合分析
	if (cbWeaveItemCount >= bLessWeavItem)
	{
		//变量定义
		CT_BYTE cbCardIndexTemp[MAX_INDEX];
		memset(cbCardIndexTemp, 0, sizeof(cbCardIndexTemp));

		//变量定义
		CT_BYTE cbIndex[7] = { 0,1,2,3,4,5,6 };
		tagWeaveItem * pWeaveItem[CountArray(cbIndex)];

		//开始组合
		do
		{
			//设置变量
			memcpy(cbCardIndexTemp, cbCardIndex, sizeof(cbCardIndexTemp));
			for (CT_WORD i = 0; i < bLessWeavItem; i++) pWeaveItem[i] = &WeaveItem[cbIndex[i]];

			//数量判断
			bool bEnoughCard = true;
			for (CT_WORD i = 0; i < bLessWeavItem * 3; i++)
			{
				//存在判断
				CT_BYTE cbCardIndex = SwitchToCardIndex(pWeaveItem[i / 3]->cbCardList[i % 3]);
				if (cbCardIndexTemp[cbCardIndex] == 0)
				{
					bEnoughCard = false;
					break;
				}
				else
				{
					cbCardIndexTemp[cbCardIndex]--;
				}
			}

			//胡牌判断
			if (bEnoughCard == true)
			{
				//牌眼判断
				CT_BYTE cbCardEye = 0;
				if (bNeedCardEye == true)
				{
					for (CT_BYTE i=0; i<MAX_INDEX; i++)
					{
						if (cbCardIndexTemp[i] == 2)
						{
							cbCardEye = SwitchToCardData(i);
							break;
						}
					}
				}

				//组合类型
				if ((bNeedCardEye == false) || (cbCardEye != 0))
				{
					//变量定义
					tagAnalyseItem AnalyseItem;
					memset(&AnalyseItem,0, sizeof(AnalyseItem));

					//设置结果
					AnalyseItem.cbHuXiCount = 0;
					AnalyseItem.cbCardEye = cbCardEye;
					AnalyseItem.cbWeaveCount = bLessWeavItem;

					//设置组合
					for (CT_WORD i = 0; i < bLessWeavItem; i++)
					{
						AnalyseItem.WeaveItemArray[i] = *pWeaveItem[i];
						AnalyseItem.WeaveItemArray[i].cbWeaveHuXi = GetWeaveHuXi(AnalyseItem.WeaveItemArray[i]);
						AnalyseItem.cbHuXiCount += AnalyseItem.WeaveItemArray[i].cbWeaveHuXi;
					}

					//插入结果
					AnalyseItemArray.push_back(AnalyseItem);
				}
			}

			//设置索引
			if (cbIndex[bLessWeavItem - 1] == (cbWeaveItemCount - 1))
			{
				CT_BYTE i = bLessWeavItem - 1;
				for (; i > 0; i--)
				{
					if ((cbIndex[i - 1] + 1) != cbIndex[i])
					{
						CT_BYTE cbNewIndex = cbIndex[i - 1];
						for (CT_WORD j = (i - 1); j < bLessWeavItem; j++) cbIndex[j] = cbNewIndex + j - i + 2;
						break;
					}
				}
				if (i == 0) break;
			}
			else
			{
				cbIndex[bLessWeavItem - 1]++;
			}
		} while (true);

		return (AnalyseItemArray.size() > 0);
	}

	return false;
}

//获取胡息
CT_BYTE CGameLogic::GetWeaveHuXi(const tagWeaveItem & WeaveItem)
{
	//计算胡息
	switch (WeaveItem.cbWeaveKind)
	{
		case ACK_TI:		//提
		{
			return ((WeaveItem.cbCardList[0] & MASK_COLOR) == 0x10) ? 12 : 9;
		}
		case ACK_PAO:	//跑
		{
			return ((WeaveItem.cbCardList[0] & MASK_COLOR) == 0x10) ? 9 : 6;
		}
		case ACK_WEI:		//偎
		{
			return ((WeaveItem.cbCardList[0] & MASK_COLOR) == 0x10) ? 6 : 3;
		}
		case ACK_PENG:	//碰
		{
			return ((WeaveItem.cbCardList[0] & MASK_COLOR) == 0x10) ? 3 : 1;
		}
		case ACK_CHI:		//吃
		{
			//获取数值
			CT_BYTE cbValue1 = WeaveItem.cbCardList[0] & MASK_VALUE;
			CT_BYTE cbValue2 = WeaveItem.cbCardList[1] & MASK_VALUE;
			CT_BYTE cbValue3 = WeaveItem.cbCardList[2] & MASK_VALUE;

			//一二三吃
			if ((cbValue1 == 1) && (cbValue2 == 2) && (cbValue3 == 3)) return ((WeaveItem.cbCardList[0] & MASK_COLOR) == 0x10) ? 6 : 3;

			//二七十吃
			if ((cbValue1 == 2) && (cbValue2 == 7) && (cbValue3 == 10)) return ((WeaveItem.cbCardList[0] & MASK_COLOR) == 0x10) ? 6 : 3;

			return 0;
		}
	}
	return 0;
}

//分析单钓
CT_BOOL CGameLogic::AnalyDanDiao(const CT_BYTE cbCardIndex[MAX_INDEX], std::vector<tagAnalyseItem>& AnalyseItemArray)
{
	//牌眼判断
	for (CT_BYTE i = 0; i < MAX_INDEX; ++i)
	{
		if (cbCardIndex[i] != 2) { continue; }

		//变量定义
		tagAnalyseItem AnalyseItem;
		memset(&AnalyseItem, 0, sizeof(AnalyseItem));
		//设置结果
		AnalyseItem.cbHuXiCount = 0;
		AnalyseItem.cbWeaveCount = 0;
		AnalyseItem.cbCardEye = SwitchToCardData(i);

		//插入结果
		AnalyseItemArray.push_back(AnalyseItem);

		return true;
	}
	return false;
}

//分析组合
CT_BYTE CGameLogic::AnalyseWeaveItem(const CT_BYTE cbCardIndex[MAX_INDEX], tagWeaveItem WeaveItem[76])
{
	CT_BYTE cbWeaveItemCount = 0;
	//memset(WeaveItem, 0,sizeof(WeaveItem));

	//拆分处理
	for (CT_BYTE i=0; i<MAX_INDEX; ++i)
	{
		//分析过虑
		if (cbCardIndex[i] == 0) continue;

		//变量定义
		CT_BYTE cbCardData = SwitchToCardData(i);

		CT_BYTE cbReverseIndex = (i + 10) % MAX_INDEX;
		//大小搭吃
		if ((cbCardIndex[i] >= 2) && (cbCardIndex[cbReverseIndex] >= 1))
		{
			WeaveItem[cbWeaveItemCount].cbCardCount = 3;
			WeaveItem[cbWeaveItemCount].cbWeaveKind = ACK_CHI;
			WeaveItem[cbWeaveItemCount].cbCenterCard = cbCardData;
			WeaveItem[cbWeaveItemCount].cbCardList[0] = cbCardData;
			WeaveItem[cbWeaveItemCount].cbCardList[1] = cbCardData;
			WeaveItem[cbWeaveItemCount].cbCardList[2] = SwitchToCardData(cbReverseIndex);
			++cbWeaveItemCount;
		}

		//大小搭吃
		if ((cbCardIndex[i] >= 1) && (cbCardIndex[cbReverseIndex] >= 2))
		{
			WeaveItem[cbWeaveItemCount].cbCardCount = 3;
			WeaveItem[cbWeaveItemCount].cbWeaveKind = ACK_CHI;
			WeaveItem[cbWeaveItemCount].cbCenterCard = cbCardData;
			WeaveItem[cbWeaveItemCount].cbCardList[0] = cbCardData;
			WeaveItem[cbWeaveItemCount].cbCardList[1] = SwitchToCardData(cbReverseIndex);
			WeaveItem[cbWeaveItemCount].cbCardList[2] = SwitchToCardData(cbReverseIndex);
			++cbWeaveItemCount;
		}

		//二七十吃
		if (2 == GetCardValue(cbCardData))
		{
			for (CT_WORD j = 1; j <= cbCardIndex[i]; ++j)
			{
				if ((cbCardIndex[i + 5] >= j) && (cbCardIndex[i + 8] >= j))
				{
					WeaveItem[cbWeaveItemCount].cbCardCount = 3;
					WeaveItem[cbWeaveItemCount].cbWeaveKind = ACK_CHI;
					WeaveItem[cbWeaveItemCount].cbCenterCard = cbCardData;
					WeaveItem[cbWeaveItemCount].cbCardList[0] = cbCardData;
					WeaveItem[cbWeaveItemCount].cbCardList[1] = cbCardData + 5;
					WeaveItem[cbWeaveItemCount].cbCardList[2] = cbCardData + 8;
					++cbWeaveItemCount;
				}
			}
		}

		//顺子判断
		if ((i < (MAX_INDEX - 2)) && ((i % 10) <= 7) && (cbCardIndex[i] > 0))
		{
			for (CT_WORD j = 1; j <= cbCardIndex[i]; ++j)
			{
				if ((cbCardIndex[i + 1] >= j) && (cbCardIndex[i + 2] >= j))
				{
					WeaveItem[cbWeaveItemCount].cbCardCount = 3;
					WeaveItem[cbWeaveItemCount].cbWeaveKind = ACK_CHI;
					WeaveItem[cbWeaveItemCount].cbCenterCard = cbCardData;
					WeaveItem[cbWeaveItemCount].cbCardList[0] = cbCardData;
					WeaveItem[cbWeaveItemCount].cbCardList[1] = cbCardData + 1;
					WeaveItem[cbWeaveItemCount].cbCardList[2] = cbCardData + 2;
					++cbWeaveItemCount;
				}
			}
		}
	}

	return cbWeaveItemCount;
}

//是否偎碰
CT_BOOL CGameLogic::IsWeiPengCard(CT_BYTE cbCardIndex[MAX_INDEX], CT_BYTE cbCurrentCard)
{
	//效验扑克
	assert(cbCurrentCard != 0);
	if (cbCurrentCard == 0) return false;

	//转换索引
	CT_BYTE cbCurrentIndex = SwitchToCardIndex(cbCurrentCard);

	//跑偎判断
	return (cbCardIndex[cbCurrentIndex] == 2) ? true : false;
}