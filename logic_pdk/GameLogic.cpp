#include "GameLogic.h"
#include <time.h>
#include "math.h"
#include "glog_wrapper.h"

//////////////////////////////////////////////////////////////////////////
//静态变量

//索引变量
const CT_BYTE cbIndexCount = 5;

//扑克数据
const CT_BYTE CGameLogic::m_byCardDataArray[MAX_CARD_TOTAL] =
{
	0x01,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,	//方块 A,3- K
	0x11,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,	//梅花 A,3- K
	0x21,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,	//红桃 A,3- K
	0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,	//黑桃 2 - K
};

const CT_BYTE CGameLogic::m_byCardDataArray15[MAX_CARD_TOTAL] =
{
	0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,			//方块 3- K
	0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,			//梅花 3- K
	0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,			//红桃 3- K
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,	//黑桃 A- Q
};

//////////////////////////////////////////////////////////////////////////


//构造函数
CGameLogic::CGameLogic()
{
	m_cbNotSplitRomb = 0;
	m_cbCanFourTakeTwo = 0;
	m_cbCanFourTakeThree = 0;
	m_cbIsOutThreeEnd = 0;
	m_cbIsThreeEnd = 0;
	m_cbIsOutFlyEnd = 0;
	m_cbIsFlyEnd = 0;
}

//析构函数
CGameLogic::~CGameLogic()
{
}

//混乱扑克
CT_VOID CGameLogic::RandCardData(CT_BYTE byCardBuffer[], CT_BYTE byBufferCount)
{
	//混乱准备
	CT_BYTE cbCardData[MAX_CARD_TOTAL] = { 0 };


	if (byBufferCount == MAX_CARD_TOTAL)
	{
		memcpy(cbCardData, m_byCardDataArray, MAX_CARD_TOTAL);
	}
	else
	{
		memcpy(cbCardData, m_byCardDataArray15, MAX_CARD_TOTAL - 3);
	}


	//混乱扑克
	CT_BYTE bRandCount = 0, bPosition = 0;
	do
	{
		bPosition = rand() % (byBufferCount - bRandCount);
		byCardBuffer[bRandCount++] = cbCardData[bPosition];
		cbCardData[bPosition] = cbCardData[byBufferCount - bRandCount];
	} while (bRandCount<byBufferCount && bRandCount<byBufferCount);
}

//排列扑克
CT_VOID CGameLogic::SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbSortType)
{
	//数目过虑
	if (cbCardCount == 0) return;
	//转换数值
	CT_BYTE cbSortValue[MAX_COUNT];
	for (CT_BYTE i = 0; i < cbCardCount; i++) cbSortValue[i] = GetLogicValue(cbCardData[i]);

	//排序操作
	bool bSorted = true;
	CT_BYTE cbSwitchData = 0, cbLast = cbCardCount - 1;

	if (cbSortType == ST_DESC)
	{//大到小排序
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
	else
	{
		do
		{
			bSorted = true;
			for (CT_BYTE i = 0; i < cbLast; i++)
			{
				if ((cbSortValue[i] > cbSortValue[i + 1]) ||
					((cbSortValue[i] == cbSortValue[i + 1]) && (cbCardData[i] > cbCardData[i + 1])))
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
}

//删除扑克
CT_BOOL CGameLogic::RemoveCard(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	//检验数据
	assert(cbRemoveCount <= cbCardCount);

	//定义变量
	CT_BYTE cbDeleteCount = 0;
	CT_BYTE cbTempCardData[MAX_COUNT] = { 0 };
	if (cbCardCount > CountArray(cbTempCardData)) return false;
	memcpy(cbTempCardData, cbCardData, cbCardCount * sizeof(cbCardData[0]));

	//置零扑克
	for (CT_BYTE i = 0; i < cbRemoveCount; i++)
	{
		for (CT_BYTE j = 0; j < cbCardCount; j++)
		{
			if (cbRemoveCard[i] == cbTempCardData[j])
			{
				cbDeleteCount++;
				cbTempCardData[j] = 0;
				break;
			}
		}
	}
	if (cbDeleteCount != cbRemoveCount) return false;

	//清理扑克
	CT_BYTE cbCardPos = 0;
	for (CT_BYTE i = 0; i < cbCardCount; i++)
	{
		if (cbTempCardData[i] != 0) cbCardData[cbCardPos++] = cbTempCardData[i];
	}

	return true;
}

//逻辑数值
CT_BYTE CGameLogic::GetLogicValue(CT_BYTE cbCardData)
{
	//扑克属性
	CT_BYTE cbCardColor = GetCardColor(cbCardData);
	CT_BYTE cbCardValue = GetCardValue(cbCardData);

	//转换数值
	if (cbCardColor == 0x40) return cbCardValue + 2;
	return (cbCardValue <= 2) ? (cbCardValue + 13) : cbCardValue;
}

//获取类型
CT_BYTE CGameLogic::GetCardType(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BOOL bLastCard /*= false*/, CT_BYTE cbAirLineCount[] /*= NULL*/)
{
	//空牌
	if (0 == cbCardCount) return CT_ERROR;
	//单牌
	if (1 == cbCardCount && cbCardData[0] > 0) return CT_SINGLE;
	//对牌
	if (2 == cbCardCount && (GetLogicValue(cbCardData[0]) == GetLogicValue(cbCardData[1]))) return CT_DOUBLE_Ex;

	//拷贝牌
	CT_BYTE cbTempCard[MAX_COUNT] = { 0 };
	memset(cbTempCard, 0, sizeof(cbTempCard));
	memcpy(cbTempCard, cbCardData, sizeof(CT_BYTE)*cbCardCount);

	//排列扑克(小到大排序)
	SortCardList(cbTempCard, cbCardCount, ST_ACS);

	//分析扑克
	tagAnalyseResult AnalyseResult;
	memset(&AnalyseResult, 0, sizeof(AnalyseResult));
	AnalysebCardData(cbTempCard, cbCardCount, AnalyseResult);

	//四牌判断
	if (AnalyseResult.cbBlockCount[3] > 0)
	{
		//炸弹
		if ((AnalyseResult.cbBlockCount[3] == 1) && (cbCardCount == 4)) return CT_BOMB_CARD;
		//三带二   
		if (m_cbNotSplitRomb != 1 && (AnalyseResult.cbBlockCount[3] == 1) && (cbCardCount == 5)) return CT_THREE_TAKE_TWO;
		//四带二
		if (m_cbCanFourTakeTwo == 1 && (AnalyseResult.cbBlockCount[3] == 1) && (cbCardCount == 6)) return CT_FOUR_TAKE_TWO;
		//四带三
		if (m_cbCanFourTakeThree == 1 && (AnalyseResult.cbBlockCount[3] == 1) && (cbCardCount == 7)) return CT_FOUR_TAKE_THREE;
		//飞机,拆炸弹组成飞机
		if (m_cbNotSplitRomb != 1 && (AnalyseResult.cbBlockCount[3] >= 1) && (cbCardCount >= 10))
		{
			CT_BYTE cbLineCard[5] = { 0 };//三张以上牌数据
			CT_BYTE cbLineCount = 0;
			for (CT_BYTE i = 0; i < AnalyseResult.cbBlockCount[2]; ++i)
			{
				cbLineCard[cbLineCount] = AnalyseResult.cbCardData[2][i * 3];
				cbLineCount++;
			}
			for (CT_BYTE i = 0; i < AnalyseResult.cbBlockCount[3]; ++i)
			{
				cbLineCard[cbLineCount] = AnalyseResult.cbCardData[3][i * 4];
				cbLineCount++;
			}
			if (cbLineCount >= 2)
			{
				SortCardList(cbLineCard, cbLineCount, ST_ACS);
				CT_BOOL bAirPlaneFalg = false;
				if (cbCardCount == 10)
				{
					CT_BYTE cbLogicValue1 = GetLogicValue(cbLineCard[0]);
					CT_BYTE cbLogicValue2 = GetLogicValue(cbLineCard[1]);
					CT_BYTE cbLogicValue3 = GetLogicValue(cbLineCard[2]);
					if ((cbLogicValue2 - cbLogicValue1) == 1 || (cbLogicValue3 - cbLogicValue2) == 1)
					{
						bAirPlaneFalg = true;
					}
				}
				else
				{
					for (CT_BYTE i = 0; i < cbLineCount; ++i)
					{
						if ((i + 2) < cbLineCount)
						{
							CT_BYTE cbLogicValue1 = GetLogicValue(cbLineCard[i]);
							CT_BYTE cbLogicValue2 = GetLogicValue(cbLineCard[i + 1]);
							CT_BYTE cbLogicValue3 = GetLogicValue(cbLineCard[i + 2]);
							if ((cbLogicValue2 - cbLogicValue1) == 1 && (cbLogicValue3 - cbLogicValue2) == 1)
							{
								bAirPlaneFalg = true;
							}
						}
					}
				}

				if (cbAirLineCount != NULL)
				{
					cbAirLineCount[0] = cbLineCount;
				}

				if (bAirPlaneFalg)
				{
					//拆开炸弹作飞机
					return CT_AIRPLANE;
				}
			}
		}
		return CT_ERROR;
	}

	//三牌判断
	if (AnalyseResult.cbBlockCount[2]>0)
	{
		if (m_cbNotSplitRomb == 1 && AnalyseResult.cbBlockCount[3] > 0)
		{//不能拆炸弹
			return CT_ERROR;
		}
		//连牌判断
		if (AnalyseResult.cbBlockCount[2] >= 2)
		{
			CT_BYTE cbPos = 0;
			for (CT_BYTE n = 0; n < 10; ++n)
			{
				//变量定义
				CT_BYTE cbCardData = AnalyseResult.cbCardData[2][cbPos * 3];
				CT_BYTE cbFirstLogicValue = GetLogicValue(cbCardData);

				//错误过虑(不能大于K)
				if (cbFirstLogicValue >= 14) return CT_ERROR;

				//连牌判断
				CT_BYTE cbCount = 1;
				CT_BYTE i = cbPos + 1;
				for (/*CT_BYTE i = cbPos+1*/; i < AnalyseResult.cbBlockCount[2]; i++)
				{
					cbPos = i;
					CT_BYTE cbCardData = AnalyseResult.cbCardData[2][i * 3];
					CT_BYTE cbNextLogicValue = GetLogicValue(cbCardData);
					if (cbFirstLogicValue != (cbNextLogicValue - 1))
					{
						break;
					}
					else
					{
						++cbCount;
						cbFirstLogicValue = cbNextLogicValue;
					}
				}

				if (cbAirLineCount != NULL)
				{
					cbAirLineCount[0] = cbCount;
				}

				if (cbCount * 5 == cbCardCount) return CT_AIRPLANE;
				if (bLastCard && cbCount * 3 == cbCardCount)  return CT_AIRPLANE;  //最后一手牌可以不带
				if (bLastCard && cbCount * 4 == cbCardCount)  return CT_AIRPLANE;  //最后一手牌可以不带
				if (bLastCard && cbCount * 5 > cbCardCount)  return CT_AIRPLANE;  //最后一手牌可以不带

				if ((cbCardCount % 5) == 0 && cbCount > 2)
				{
					return CT_AIRPLANE;
				}
				if (cbCount < 2 && i >= AnalyseResult.cbBlockCount[2])
				{
					break;
				}
			}
			return CT_ERROR;
		}
		else
		{
			if (5 == cbCardCount) return CT_THREE_TAKE_TWO;
			if (bLastCard && 3 == cbCardCount)  return CT_THREE_TAKE_TWO;  //最后一手牌可以不带
			if (bLastCard && 4 == cbCardCount)  return CT_THREE_TAKE_TWO;  //最后一手牌可以带一张
		}
		return CT_ERROR;
	}

	//两张类型
	if (AnalyseResult.cbBlockCount[1] >= 2)
	{
		//变量定义
		CT_BYTE cbCardData = AnalyseResult.cbCardData[1][0];
		CT_BYTE cbFirstLogicValue = GetLogicValue(cbCardData);

		//错误过虑(不能大于k) 
		if (cbFirstLogicValue >= 14) return CT_ERROR;

		//连牌判断
		for (CT_BYTE i = 1; i<AnalyseResult.cbBlockCount[1]; i++)
		{
			CT_BYTE cbCardData = AnalyseResult.cbCardData[1][i * 2];
			if (cbFirstLogicValue != (GetLogicValue(cbCardData) - i)) return CT_ERROR;
		}

		//二连判断
		if ((AnalyseResult.cbBlockCount[1] * 2) == cbCardCount) return CT_DOUBLE_LINE;

		return CT_ERROR;
	}

	//单张判断
	if ((AnalyseResult.cbBlockCount[0] >= 5) && (AnalyseResult.cbBlockCount[0] == cbCardCount))
	{
		//变量定义
		CT_BYTE cbCardData = AnalyseResult.cbCardData[0][0];
		CT_BYTE cbFirstLogicValue = GetLogicValue(cbCardData);

		//错误过虑(不能大于10) 
		if (cbFirstLogicValue >= 11) return CT_ERROR;

		//连牌判断
		for (CT_BYTE i = 1; i<AnalyseResult.cbBlockCount[0]; i++)
		{
			CT_BYTE cbCurCard = AnalyseResult.cbCardData[0][i];
			if (cbFirstLogicValue != (GetLogicValue(cbCurCard) - i))
			{
				return CT_ERROR;
			}
		}

		return CT_SINGLE_LINE;
	}

	return CT_ERROR;
}

//对比扑克
CT_BOOL CGameLogic::CompareCard(const CT_BYTE cbFirstCard[], CT_BYTE cbFirstCount, const CT_BYTE cbNextCard[], CT_BYTE cbNextCount, CT_BOOL bLastCard/* = false*/)
{
	//获取类型
	CT_BYTE cbFirstType = GetCardType(cbFirstCard, cbFirstCount);
	CT_BYTE cbNextType = GetCardType(cbNextCard, cbNextCount, bLastCard);

	//类型判断
	if (cbNextType == CT_ERROR) return false;

	//炸弹判断
	if ((cbFirstType != CT_BOMB_CARD) && (cbNextType == CT_BOMB_CARD)) return true;
	if ((cbFirstType == CT_BOMB_CARD) && (cbNextType != CT_BOMB_CARD)) return false;

	//规则判断
	if (bLastCard)
	{
		if (m_cbIsThreeEnd == 1 && cbNextType == CT_THREE_TAKE_TWO)
		{
			if (cbFirstType != cbNextType)
				return false;
		}
		else if (m_cbIsFlyEnd == 1 && cbNextType == CT_AIRPLANE)
		{
			if (cbFirstType != cbNextType)
				return false;
		}
		else
		{
			if ((cbFirstType != cbNextType) || (cbFirstCount != cbNextCount))
				return false;
		}
	}
	else
	{
		if ((cbFirstType != cbNextType) || (cbFirstCount != cbNextCount))
			return false;
	}

	//拷贝牌
	CT_BYTE cbTempFirstCard[MAX_COUNT] = { 0 };
	memset(cbTempFirstCard, 0, sizeof(cbTempFirstCard));
	memcpy(cbTempFirstCard, cbFirstCard, sizeof(CT_BYTE)*cbFirstCount);
	CT_BYTE cbTempNextCard[MAX_COUNT] = { 0 };
	memset(cbTempNextCard, 0, sizeof(cbTempNextCard));
	memcpy(cbTempNextCard, cbNextCard, sizeof(CT_BYTE)*cbNextCount);
	//排列扑克(小到大排序)
	SortCardList(cbTempFirstCard, cbFirstCount, ST_ACS);
	SortCardList(cbTempNextCard, cbNextCount, ST_ACS);

	//开始对比
	switch (cbNextType)
	{
	case CT_SINGLE:					//单牌类型	
	case CT_DOUBLE_Ex:				//对牌类型
	case CT_BOMB_CARD:				//炸弹类型
	case CT_SINGLE_LINE:			//单连类型
	case CT_DOUBLE_LINE:			//对连类型
	{
		//获取数值
		CT_BYTE cbFirstLogicValue = GetLogicValue(cbTempFirstCard[0]);
		CT_BYTE cbNextLogicValue = GetLogicValue(cbTempNextCard[0]);

		//对比扑克
		return cbNextLogicValue > cbFirstLogicValue;
	}
	case CT_FOUR_TAKE_TWO:			//四带二   
	case CT_FOUR_TAKE_THREE:		//四带三
	case CT_THREE_TAKE_TWO:			//三带二   
	{
		CT_BYTE cbCount = 3;
		if (cbNextType != CT_THREE_TAKE_TWO)
		{
			cbCount = 4;
		}
		//分析扑克
		tagDistributing FirstDistributing = {};
		AnalysebDistributing(cbTempFirstCard, cbFirstCount, FirstDistributing);
		tagDistributing NextDistributing = {};
		AnalysebDistributing(cbTempNextCard, cbNextCount, NextDistributing);
		CT_BYTE cbFirstLogicValue = 0;
		CT_BYTE cbNextLogicValue = 0;
		for (CT_BYTE cbIndex = 0; cbIndex < 15; ++cbIndex)
		{
			if (FirstDistributing.cbDistributing[cbIndex][cbIndexCount] >= cbCount)
			{
				cbFirstLogicValue = cbIndex + 1;
			}
			if (NextDistributing.cbDistributing[cbIndex][cbIndexCount] >= cbCount)
			{
				cbNextLogicValue = cbIndex + 1;
			}
			if (cbFirstLogicValue>0 && cbNextLogicValue>0)
			{//两个都找到了
				break;
			}
		}

		//对比扑克
		return cbNextLogicValue > cbFirstLogicValue;
	}
	case CT_AIRPLANE:
	{
		//搜索最大飞机
		CT_BYTE cbFirstResultCard[MAX_COUNT] = { 0 };
		SearchMaxThreeLine(cbTempFirstCard, cbFirstCount, cbFirstResultCard);
		CT_BYTE cbNextResultCard[MAX_COUNT] = { 0 };
		SearchMaxThreeLine(cbTempNextCard, cbNextCount, cbNextResultCard);

		//获取数值
		CT_BYTE cbFirstLogicValue = GetLogicValue(cbFirstResultCard[0]);
		CT_BYTE cbNextLogicValue = GetLogicValue(cbNextResultCard[0]);

		//对比扑克
		return cbNextLogicValue > cbFirstLogicValue;
	}
	}

	return false;
}

//出牌搜索
CT_BYTE CGameLogic::SearchOutCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[], CT_BYTE cbTurnCardCount, tagSearchCardResult& SearchCardResult)
{
	//设置结果
	memset(&SearchCardResult, 0, sizeof(tagSearchCardResult));

	//变量定义
	CT_BYTE cbResultCount = 0;

	//构造扑克
	CT_BYTE cbCardData[MAX_COUNT];
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//获取类型
	CT_BYTE cbTurnOutType = GetCardType(cbTurnCardData, cbTurnCardCount);

	//出牌分析
	switch (cbTurnOutType)
	{
	case CT_ERROR:						//错误类型
	{
		//搜索首出牌(提取各种牌型一组)
		SearchHeadOutCard(cbCardData, cbCardCount, SearchCardResult, cbResultCount);
		return cbResultCount;
	}
	case CT_SINGLE:						//单牌类型
	case CT_DOUBLE_Ex:					//对牌类型
	case CT_BOMB_CARD:				    //炸弹类型
	{
		//变量定义
		CT_BYTE cbReferCard = cbTurnCardData[0];
		CT_BYTE cbSameCount = 1;
		if (cbTurnOutType == CT_DOUBLE_Ex) cbSameCount = 2;
		else if (cbTurnOutType == CT_BOMB_CARD) cbSameCount = 4;

		//搜索相同牌
		cbResultCount = SearchSameCard(cbCardData, cbCardCount, cbReferCard, cbSameCount, SearchCardResult);

		break;
	}
	case CT_SINGLE_LINE:				//单连类型
	case CT_DOUBLE_LINE:			//对连类型
	{
		//效验牌数
		if (cbCardCount < cbTurnCardCount) break;

		//变量定义
		CT_BYTE cbBlockCount = 1;
		if (cbTurnOutType == CT_DOUBLE_LINE) cbBlockCount = 2;
		CT_BYTE cbLineCount = cbTurnCardCount / cbBlockCount;

		//构造扑克
		CT_BYTE cbTempCardData[MAX_COUNT] = { 0 };
		memcpy(cbTempCardData, cbTurnCardData, sizeof(CT_BYTE)*cbTurnCardCount);
		//排列扑克(小到大排序)
		SortCardList(cbTempCardData, cbTurnCardCount, ST_ACS);
		CT_BYTE cbReferCard = cbTempCardData[0];

		//搜索边牌
		cbResultCount = SearchLineCardType(cbCardData, cbCardCount, cbReferCard, cbBlockCount, cbLineCount, SearchCardResult);

		break;
	}
	case CT_THREE_TAKE_TWO:	//三带二
	{
		//效验牌数
		if (m_cbIsThreeEnd != 1 && cbCardCount < cbTurnCardCount) break;

		//分析扑克
		tagAnalyseResult AnalyseResult;
		memset(&AnalyseResult, 0, sizeof(AnalyseResult));
		AnalysebCardData(cbTurnCardData, cbTurnCardCount, AnalyseResult);
		if (0 == AnalyseResult.cbBlockCount[2] && (0 == AnalyseResult.cbBlockCount[3]))
		{//没有三张的牌
			break;
		}
		//三张的牌
		CT_BYTE cbReferCard = AnalyseResult.cbCardData[2][0];
		if (cbReferCard == 0)
		{
			cbReferCard = AnalyseResult.cbCardData[3][0];
		}

		//搜索三带牌型
		if (m_cbIsThreeEnd == 1 && cbTurnCardCount > cbCardCount)
		{
			cbResultCount = SearchSameCard(cbCardData, cbCardCount, cbReferCard, 3, SearchCardResult);
		}
		else
		{
			cbResultCount = SearchTakeCardType(cbCardData, cbCardCount, cbReferCard, 3, 2, SearchCardResult);
		}
		break;
	}
	case CT_AIRPLANE:  //飞机带单或带双
	{
		//变量定义
		/*CT_BYTE cbLineCount = cbTurnCardCount / (cbTurnOutType == CT_THREE_LINE_ONE ? 4 : 5);
		CT_BYTE cbTakeCardCount = (cbTurnOutType == CT_THREE_LINE_ONE) ? 1 : 2;*/

		if (m_cbIsFlyEnd != 1 && cbCardCount < cbTurnCardCount) break;

		CT_BYTE cbLineCount = cbTurnCardCount / 5;
		CT_BYTE cbTakeCardCount = 2;

		//构造扑克
		CT_BYTE cbTempCardData[MAX_COUNT] = { 0 };
		memcpy(cbTempCardData, cbTurnCardData, sizeof(CT_BYTE)*cbTurnCardCount);
		//排列扑克(小到大排序)
		SortCardList(cbTempCardData, cbTurnCardCount, ST_ACS);

		//分析扑克
		tagAnalyseResult AnalyseResult;
		memset(&AnalyseResult, 0, sizeof(AnalyseResult));
		AnalysebCardData(cbTempCardData, cbTurnCardCount, AnalyseResult);

		CT_BYTE cbBlockCount = AnalyseResult.cbBlockCount[2];
		CT_BYTE cbReferCard = AnalyseResult.cbCardData[2][0];
		if (cbBlockCount > 2)
		{
			//找出飞机连牌
			CT_BYTE cbLogicValue_1 = GetLogicValue(AnalyseResult.cbCardData[2][0]);
			CT_BYTE cbLogicValue_2 = GetLogicValue(AnalyseResult.cbCardData[2][3]);
			if ((cbLogicValue_2 - cbLogicValue_1) != 1)
			{
				cbReferCard = AnalyseResult.cbCardData[2][3];
			}
		}
		if (m_cbIsFlyEnd == 1 && cbTurnCardCount > cbCardCount)
		{
			cbResultCount = SearchLineCardType(cbHandCardData, cbHandCardCount, cbReferCard, 3, cbLineCount, SearchCardResult);
		}
		else
		{
			//搜索飞机
			cbResultCount = SearchThreeTwoLine(cbCardData, cbCardCount, cbReferCard, cbTakeCardCount, cbLineCount, SearchCardResult);
		}
		break;
	}
	default:
		break;
	}

	//搜索炸弹
	if ((cbCardCount >= 4) && (cbTurnOutType != CT_BOMB_CARD))
	{
		//搜索炸弹
		tagSearchCardResult TempSearchResult;
		memset(&TempSearchResult, 0, sizeof(tagSearchCardResult));
		CT_BYTE cbTmpResultCount = SearchSameCard(cbCardData, cbCardCount, 0, 4, TempSearchResult);
		for (CT_BYTE i = 0; i < cbTmpResultCount; i++)
		{
			SearchCardResult.cbCardCount[cbResultCount] = TempSearchResult.cbCardCount[i];
			memcpy(SearchCardResult.cbResultCard[cbResultCount], TempSearchResult.cbResultCard[i], sizeof(CT_BYTE)*TempSearchResult.cbCardCount[i]);
			cbResultCount++;
		}
	}
	SearchCardResult.cbSearchCount = cbResultCount;
	return cbResultCount;
}

//搜索首出牌(提取各种牌型一组)
CT_BYTE CGameLogic::SearchHeadOutCard(CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagSearchCardResult& SearchCardResult, CT_BYTE& cbResultCount)
{
	//是否一手出完
	if (GetCardType(cbCardData, cbCardCount, true) != CT_ERROR)
	{
		SearchCardResult.cbCardCount[cbResultCount] = cbCardCount;
		memcpy(SearchCardResult.cbResultCard[cbResultCount], cbCardData, sizeof(CT_BYTE)*cbCardCount);
		cbResultCount++;
	}

	//排列扑克(小到大排序)
	SortCardList(cbCardData, cbCardCount, ST_ACS);

	////如果最小牌不是单牌，则提取
	//CT_BYTE cbSameCount = 0;
	//do 
	//{
	//	//取第一张
	//	SearchCardResult.cbResultCard[cbResultCount][cbSameCount] = cbCardData[cbSameCount];
	//	cbSameCount++;
	//	//取与后面相同
	//} while (GetCardValue(cbCardData[0]) == GetCardValue(cbCardData[cbSameCount]));
	//SearchCardResult.cbCardCount[cbResultCount] = cbSameCount;
	//cbResultCount++;


	//临时结果
	tagSearchCardResult tmpSearchCardResult;
	memset(&tmpSearchCardResult, 0, sizeof(tmpSearchCardResult));

	//单牌
	//CT_BYTE cbTmpCount = 0;
	//if (cbSameCount != 1)
	{
		if (SearchSameCard(cbCardData, cbCardCount, 0, 1, tmpSearchCardResult) > 0)
		{//拷贝第一个结果
			CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
		}
	}

	//对牌
	//if (cbSameCount != 2)
	{
		memset(&tmpSearchCardResult, 0, sizeof(tmpSearchCardResult));
		if (SearchSameCard(cbCardData, cbCardCount, 0, 2, tmpSearchCardResult) > 0)
		{//拷贝第一个结果
			CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
		}
	}

	//三带二
	memset(&tmpSearchCardResult, 0, sizeof(tmpSearchCardResult));
	if (SearchTakeCardType(cbCardData, cbCardCount, 0, 3, 2, tmpSearchCardResult) > 0)
	{//拷贝第一个结果
		CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
	}

	//单连
	memset(&tmpSearchCardResult, 0, sizeof(tmpSearchCardResult));
	if (SearchLineCardType(cbCardData, cbCardCount, 0, 1, 0, tmpSearchCardResult) > 0)
	{//拷贝第一个结果
		CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
	}

	//连对
	memset(&tmpSearchCardResult, 0, sizeof(tmpSearchCardResult));
	if (SearchLineCardType(cbCardData, cbCardCount, 0, 2, 0, tmpSearchCardResult) > 0)
	{//拷贝第一个结果
		CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
	}

	//飞机带双
	memset(&tmpSearchCardResult, 0, sizeof(tmpSearchCardResult));
	if (SearchThreeTwoLine(cbCardData, cbCardCount, 0, 2, 0, tmpSearchCardResult) > 0)
	{//拷贝第一个结果
		CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
	}
	else
	{//飞机带单
		memset(&tmpSearchCardResult, 0, sizeof(tmpSearchCardResult));
		if (SearchThreeTwoLine(cbCardData, cbCardCount, 0, 1, 0, tmpSearchCardResult) > 0)
		{//拷贝第一个结果
			CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
		}
	}

	//炸弹
	//if (cbSameCount != 4)
	{
		if (SearchSameCard(cbCardData, cbCardCount, 0, 4, tmpSearchCardResult) > 0)
		{//拷贝第一个结果
			CopyFirstResult(SearchCardResult, cbResultCount, tmpSearchCardResult);
		}
	}

	SearchCardResult.cbSearchCount = cbResultCount;
	return cbResultCount;
}

//同牌搜索
CT_BYTE CGameLogic::SearchSameCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCardCount, tagSearchCardResult& SearchCardResult)
{
	//设置结果
	memset(&SearchCardResult, 0, sizeof(tagSearchCardResult));
	//结果个数
	CT_BYTE cbResultCount = 0;

	//构造扑克
	CT_BYTE cbCardData[MAX_COUNT] = { 0 };
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//排列扑克(小到大排序)
	SortCardList(cbCardData, cbCardCount, ST_ACS);

	//分析扑克
	tagAnalyseResult AnalyseResult = {};
	AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

	//参考值
	CT_BYTE cbReferValue = (cbReferCard == 0) ? 0 : GetLogicValue(cbReferCard);
	//张数索引
	CT_BYTE cbBlockIndex = cbSameCardCount - 1;
	do
	{
		for (CT_BYTE i = 0; i < AnalyseResult.cbBlockCount[cbBlockIndex]; i++)
		{
			//CT_BYTE cbIndex = (AnalyseResult.cbBlockCount[cbBlockIndex] - i - 1)*(cbBlockIndex + 1);
			CT_BYTE cbIndex = i*(cbBlockIndex + 1);
			if (GetLogicValue(AnalyseResult.cbCardData[cbBlockIndex][cbIndex]) > cbReferValue)
			{
				assert(cbResultCount < CountArray(SearchCardResult.cbCardCount));

				//复制扑克
				memcpy(SearchCardResult.cbResultCard[cbResultCount], &AnalyseResult.cbCardData[cbBlockIndex][cbIndex], cbSameCardCount * sizeof(CT_BYTE));
				SearchCardResult.cbCardCount[cbResultCount] = cbSameCardCount;

				cbResultCount++;
			}
		}

		cbBlockIndex++;
	} while (cbBlockIndex < CountArray(AnalyseResult.cbBlockCount));

	SearchCardResult.cbSearchCount = cbResultCount;
	return cbResultCount;
}

//连牌搜索
CT_BYTE CGameLogic::SearchLineCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbBlockCount, CT_BYTE cbLineCount, tagSearchCardResult& SearchCardResult)
{
	//设置结果
	memset(&SearchCardResult, 0, sizeof(tagSearchCardResult));
	//结果个数
	CT_BYTE cbResultCount = 0;

	//最少连牌个数
	CT_BYTE cbLessLineCount = GetLessLineCount(cbBlockCount, cbLineCount);

	//从3开始找(索引=3-1)
	CT_BYTE cbReferIndex = 2;
	if (cbReferCard != 0)
	{
		cbReferIndex = GetLogicValue(cbReferCard);
	}
	//超过A
	if (cbReferIndex + cbLessLineCount > 14) return cbResultCount;

	//长度判断
	if (cbHandCardCount < cbLessLineCount*cbBlockCount) return cbResultCount;

	//构造扑克
	CT_BYTE cbCardData[MAX_COUNT] = { 0 };
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//分析扑克
	tagDistributing Distributing = {};
	AnalysebDistributing(cbCardData, cbCardCount, Distributing);

	//搜索顺子
	CT_BYTE cbTmpLinkCount = 0;
	for (CT_BYTE cbValueIndex = cbReferIndex; cbValueIndex < 14; cbValueIndex++)
	{
		//继续判断
		if (Distributing.cbDistributing[cbValueIndex][cbIndexCount] < cbBlockCount)
		{
			if (cbTmpLinkCount < cbLessLineCount)
			{
				cbTmpLinkCount = 0;
				continue;
			}
			else
			{//退回一个位置
				cbValueIndex--;
			}
		}
		else
		{
			cbTmpLinkCount++;
			//寻找最长连
			if (cbLineCount == 0 && cbTmpLinkCount<12) continue;
		}

		if (cbTmpLinkCount >= cbLessLineCount)
		{
			assert(cbResultCount < CountArray(SearchCardResult.cbCardCount));

			//复制扑克
			CT_BYTE cbCount = 0;
			for (CT_BYTE cbIndex = cbValueIndex + 1 - cbTmpLinkCount; cbIndex <= cbValueIndex; cbIndex++)
			{
				CT_BYTE cbTmpCount = 0;
				for (CT_BYTE cbColorIndex = 0; cbColorIndex < 4; cbColorIndex++)
				{
					for (CT_BYTE cbColorCount = 0; cbColorCount < Distributing.cbDistributing[cbIndex][3 - cbColorIndex]; cbColorCount++)
					{
						SearchCardResult.cbResultCard[cbResultCount][cbCount++] = MakeCardData(cbIndex, 3 - cbColorIndex);

						if (++cbTmpCount == cbBlockCount) break;
					}
					if (cbTmpCount == cbBlockCount) break;
				}
			}

			//设置变量
			SearchCardResult.cbCardCount[cbResultCount] = cbCount;
			cbResultCount++;

			if (cbLineCount != 0)
			{
				cbTmpLinkCount--;
			}
			else
			{
				cbTmpLinkCount = 0;
			}
		}
	}
	SearchCardResult.cbSearchCount = cbResultCount;
	return cbResultCount;
}

//带牌类型搜索(三带二)
CT_BYTE CGameLogic::SearchTakeCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCount, CT_BYTE cbTakeCardCount, tagSearchCardResult& SearchCardResult)
{
	//设置结果
	memset(&SearchCardResult, 0, sizeof(tagSearchCardResult));
	//结果个数
	CT_BYTE cbResultCount = 0;

	//效验
	if (cbSameCount != 3) return cbResultCount;
	if (cbTakeCardCount != 2) return cbResultCount;

	//长度判断
	if (cbHandCardCount < cbSameCount + cbTakeCardCount)
	{
		return cbResultCount;
	}

	//构造扑克
	CT_BYTE cbCardData[MAX_COUNT] = { 0 };
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//排列扑克
	SortCardList(cbCardData, cbCardCount, ST_ACS);

	//搜索同张
	tagSearchCardResult SameCardResult = {};
	memset(&SameCardResult, 0, sizeof(SameCardResult));
	CT_BYTE cbSameCardResultCount = SearchSameCard(cbCardData, cbCardCount, cbReferCard, cbSameCount, SameCardResult);
	if (0 == cbSameCardResultCount)
	{//没有同张
		return cbResultCount;
	}

	//分析扑克
	tagAnalyseResult AnalyseResult;
	AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

	//需要牌数
	CT_BYTE cbNeedCount = cbSameCount + cbTakeCardCount;

	//提取带牌
	for (CT_BYTE i = 0; i < cbSameCardResultCount; i++)
	{
		bool bMerge = false;

		for (CT_BYTE j = 0; j < CountArray(AnalyseResult.cbBlockCount); j++)
		{
			for (CT_BYTE k = 0; k < AnalyseResult.cbBlockCount[j]; k++)
			{
				//从小到大
				CT_BYTE cbIndex = (AnalyseResult.cbBlockCount[j] - k - 1)*(j + 1);

				//过滤相同牌
				if (GetCardValue(SameCardResult.cbResultCard[i][0]) == GetCardValue(AnalyseResult.cbCardData[j][cbIndex]))
				{
					continue;
				}

				//复制带牌
				CT_BYTE cbCount = SameCardResult.cbCardCount[i];
				memcpy(&SameCardResult.cbResultCard[i][cbCount], &AnalyseResult.cbCardData[j][cbIndex], sizeof(CT_BYTE)*cbTakeCardCount);
				SameCardResult.cbCardCount[i] += cbTakeCardCount;

				if (SameCardResult.cbCardCount[i] < cbNeedCount) continue;

				//复制结果
				memcpy(SearchCardResult.cbResultCard[cbResultCount], SameCardResult.cbResultCard[i], sizeof(CT_BYTE)*SameCardResult.cbCardCount[i]);
				SearchCardResult.cbCardCount[cbResultCount] = SameCardResult.cbCardCount[i];
				cbResultCount++;

				bMerge = true;

				//下一组合
				break;
			}

			if (bMerge) break;
		}
	}

	SearchCardResult.cbSearchCount = cbResultCount;
	return cbResultCount;
}

//搜索飞机
CT_BYTE CGameLogic::SearchThreeTwoLine(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbTakeCardCount, CT_BYTE cbLineCount, tagSearchCardResult& SearchCardResult)
{
	//设置结果
	memset(&SearchCardResult, 0, sizeof(tagSearchCardResult));
	//结果数量
	CT_BYTE cbResultCount = 0;
	//变量定义
	tagSearchCardResult tmpSearchResult = {};
	memset(&tmpSearchResult, 0, sizeof(tagSearchCardResult));
	//搜索连牌
	CT_BYTE cbTmpResultCount = SearchLineCardType(cbHandCardData, cbHandCardCount, cbReferCard, 3, cbLineCount, tmpSearchResult);
	if (0 == cbTmpResultCount)
	{
		return cbResultCount;
	}

	//提取带牌
	for (CT_BYTE i = 0; i < cbTmpResultCount; i++)
	{
		//变量定义
		CT_BYTE cbTmpCardData[MAX_COUNT] = { 0 };
		CT_BYTE cbTmpCardCount = cbHandCardCount;
		//连牌个数
		CT_INT32 iTempLineCount = tmpSearchResult.cbCardCount[i] / 3;
		//带牌个数
		CT_INT32 iTempTakeCount = iTempLineCount*cbTakeCardCount;
		//除去顺子剩余多少
		CT_BYTE cbRemainCount = cbHandCardCount - iTempLineCount * 3;

		//顺子对
		if (iTempLineCount < 2) continue;

		//不够牌
		if (cbRemainCount < iTempTakeCount)
		{
			CT_BYTE cbCount = 0;
			CT_INT32 iTemp = 0;
			do
			{
				--iTempLineCount;
				++cbCount;
				iTempTakeCount = iTempLineCount*cbTakeCardCount;
				cbRemainCount = cbHandCardCount - iTempLineCount * 3;

				++iTemp;
				if (iTemp>50)
				{
					LOG(WARNING) << "SearchThreeTwoLine ! ";
					std::string strOutCard("GetCardType: cbCardData[");
					for (CT_WORD i = 0; i < cbHandCardCount; ++i)
					{
						CT_CHAR	szOut[8] = { 0 };
						_snprintf_info(szOut, sizeof(szOut), "%x,", cbHandCardData[i]);
						strOutCard.append(szOut);
					}
					strOutCard.append("]");
					LOG(WARNING) << strOutCard.c_str();
					return cbResultCount;
				}
			} while (cbRemainCount < iTempTakeCount);

			if (cbRemainCount < iTempTakeCount)
			{//不够连牌
				continue;
			}

			//拆分连牌
			RemoveCard(tmpSearchResult.cbResultCard[i], (cbCount * 3), tmpSearchResult.cbResultCard[i], tmpSearchResult.cbCardCount[i]);
			tmpSearchResult.cbCardCount[i] -= (cbCount * 3);
		}

		//删除连牌
		memcpy(cbTmpCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);
		RemoveCard(tmpSearchResult.cbResultCard[i], tmpSearchResult.cbCardCount[i], cbTmpCardData, cbTmpCardCount);
		cbTmpCardCount -= tmpSearchResult.cbCardCount[i];

		//分析牌
		tagAnalyseResult  TmpResult = {};
		memset(&TmpResult, 0, sizeof(tagAnalyseResult));
		AnalysebCardData(cbTmpCardData, cbTmpCardCount, TmpResult);

		//提取翅膀
		CT_BYTE cbDistillCard[MAX_COUNT] = { 0 };
		CT_BYTE cbDistillCount = 0;
		for (CT_BYTE j = 0; j < CountArray(TmpResult.cbBlockCount); ++j)
		{
			if (0 == TmpResult.cbBlockCount[j]) continue;
			//当前 单张/对子/三张/四张的 牌张数
			CT_BYTE cbCurCount = (j + 1) * TmpResult.cbBlockCount[j];
			CT_INT32 iGetCount = iTempTakeCount - cbDistillCount;

			CT_BYTE cbCopyCount = (cbCurCount >= iGetCount) ? iGetCount : cbCurCount;
			memcpy(&cbDistillCard[cbDistillCount], &TmpResult.cbCardData[j][0], sizeof(CT_BYTE)*cbCopyCount);

			cbDistillCount += cbCopyCount;

			if (cbDistillCount == iTempTakeCount)
			{
				break;
			}
		}

		if (cbDistillCount == iTempTakeCount)
		{//找到足够的牌
			memcpy(SearchCardResult.cbResultCard[cbResultCount], tmpSearchResult.cbResultCard[i], sizeof(CT_BYTE)*tmpSearchResult.cbCardCount[i]);
			memcpy(&SearchCardResult.cbResultCard[cbResultCount][tmpSearchResult.cbCardCount[i]], cbDistillCard, sizeof(CT_BYTE)*cbDistillCount);
			SearchCardResult.cbCardCount[cbResultCount] = tmpSearchResult.cbCardCount[i] + cbDistillCount;

			cbResultCount++;
		}
	}
	SearchCardResult.cbSearchCount = cbResultCount;

	return cbResultCount;
}

//找最大单张
CT_BYTE CGameLogic::SearchMaxSingle(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard)
{
	//拷贝牌
	CT_BYTE cbTempCard[MAX_COUNT] = { 0 };
	memset(cbTempCard, 0, sizeof(cbTempCard));
	memcpy(cbTempCard, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);
	//排列扑克(大到小排序)
	SortCardList(cbTempCard, cbHandCardCount, ST_DESC);

	if (GetLogicValue(cbTempCard[0]) > GetLogicValue(cbReferCard))
	{
		return cbTempCard[0];
	}
	return 0;
}

//拷贝第一个结果
CT_VOID CGameLogic::CopyFirstResult(tagSearchCardResult& SearchCardResult, CT_BYTE& cbResultCount, tagSearchCardResult TempSearchCardResult)
{
	SearchCardResult.cbCardCount[cbResultCount] = TempSearchCardResult.cbCardCount[0];
	memcpy(SearchCardResult.cbResultCard[cbResultCount], TempSearchCardResult.cbResultCard[0], sizeof(CT_BYTE)*TempSearchCardResult.cbCardCount[0]);
	cbResultCount++;
}

//获得最少连牌个数
CT_BYTE CGameLogic::GetLessLineCount(CT_BYTE cbBlockCount, CT_BYTE cbLineCount)
{
	if (cbLineCount>0)
	{//有指定个数
		return cbLineCount;
	}
	if (cbBlockCount == 1)
	{//单顺最少5个
		return  5;
	}
	else if (cbBlockCount == 2)
	{//双顺最少2个
		return 2;
	}
	//3顺是少2个
	return 2;
}

//搜索最大飞机
CT_BYTE CGameLogic::SearchMaxThreeLine(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbResultCardData[])
{
	//排列扑克(小到大排序)
	SortCardList(cbCardData, cbCardCount, ST_ACS);

	//分析扑克
	tagAnalyseResult AnalyseResult;
	memset(&AnalyseResult, 0, sizeof(AnalyseResult));
	AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

	/*struct  AnalyseThreeLine
	{
	CT_BYTE cbCount;
	CT_BYTE cbCardData[MAX_COUNT];
	};

	AnalyseThreeLine ThreeLine[10] = { 0 };
	memset(&ThreeLine, 0, sizeof(ThreeLine));

	//变量定义
	CT_BYTE cbFirstLogicValue = GetLogicValue(AnalyseResult.cbCardData[2][0]);
	//错误过虑(不能大于K)
	if (cbFirstLogicValue >= 14) return CT_ERROR;

	CT_BYTE cbPos = 0;
	CT_BYTE cbIndex = 0/ *ThreeLine[cbPos].cbCount* /;
	for (CT_BYTE j = 0; j < 3; ++j)
	{
	ThreeLine[cbPos].cbCardData[cbIndex * 3 + j] = AnalyseResult.cbCardData[2][0 * 3 + j];
	}
	ThreeLine[cbPos].cbCount = 1;

	for (CT_BYTE i=1; i<AnalyseResult.cbBlockCount[2]; ++i)
	{

	CT_BYTE cbNextCard = AnalyseResult.cbCardData[2][i * 3];
	CT_BYTE cbNextLogicValue = GetLogicValue(cbNextCard);
	if (cbFirstLogicValue != (cbNextLogicValue - 1))
	{
	++cbPos;
	CT_BYTE cbIndex = 0/ *ThreeLine[cbPos].cbCount* /;
	for (CT_BYTE j = 0; j < 3; ++j)
	{
	ThreeLine[cbPos].cbCardData[cbIndex * 3 + j] = AnalyseResult.cbCardData[2][i * 3 + j];
	}
	ThreeLine[cbPos].cbCount = 1;
	}
	else
	{
	CT_BYTE cbIndex = ThreeLine[cbPos].cbCount;
	for (CT_BYTE j = 0; j < 3; ++j)
	{
	ThreeLine[cbPos].cbCardData[cbIndex * 3 + j] = AnalyseResult.cbCardData[2][i * 3 + j];
	}
	++ThreeLine[cbPos].cbCount;
	}
	cbFirstLogicValue = cbNextLogicValue;
	}

	CT_BYTE cbMaxCount = 1;
	CT_INT32 iMaxIndex = -1;
	for (CT_BYTE i=0; i<10; ++i)
	{//找三连的个数最多的
	if (ThreeLine[i].cbCount<2)
	{
	continue;
	}
	if (ThreeLine[i].cbCount>cbMaxCount)
	{
	cbMaxCount = ThreeLine[i].cbCount;
	iMaxIndex = i;
	}
	}

	if (iMaxIndex<0)
	{//没有三连
	return 0;
	}

	CT_BYTE cbTempCount = ThreeLine[iMaxIndex].cbCount * 3;
	for (CT_BYTE i=0; i<cbTempCount; ++i)
	{
	cbResultCardData[i] = ThreeLine[iMaxIndex].cbCardData[i];
	}
	return ThreeLine[iMaxIndex].cbCount;*/

	/////////////////////////////////////////////
	CT_BYTE cbMaxLogicCard = 0;
	CT_BYTE cbLineCard[5] = { 0 };//三张以上牌数据
	CT_BYTE cbLineCount = 0;
	for (CT_BYTE i = 0; i < AnalyseResult.cbBlockCount[2]; ++i)
	{
		cbLineCard[cbLineCount] = AnalyseResult.cbCardData[2][i * 3];
		cbLineCount++;
	}

	if (m_cbNotSplitRomb != 1)
	{
		for (CT_BYTE i = 0; i < AnalyseResult.cbBlockCount[3]; ++i)
		{
			cbLineCard[cbLineCount] = AnalyseResult.cbCardData[3][i * 4];
			cbLineCount++;
		}
	}

	if (cbLineCount >= 2)
	{
		SortCardList(cbLineCard, cbLineCount, ST_ACS);
		if (cbCardCount == 10)
		{
			CT_BYTE cbLogicValue1 = GetLogicValue(cbLineCard[0]);
			CT_BYTE cbLogicValue2 = GetLogicValue(cbLineCard[1]);
			CT_BYTE cbLogicValue3 = GetLogicValue(cbLineCard[2]);
			if ((cbLogicValue2 - cbLogicValue1) == 1 || (cbLogicValue3 - cbLogicValue2) == 1)
			{
				cbMaxLogicCard = ((cbLogicValue2 - cbLogicValue1) == 1) ? cbLineCard[1] : cbLineCard[2];
			}
		}
		else
		{
			for (CT_BYTE i = 0; i < cbLineCount; ++i)
			{
				if ((i + 2) < cbLineCount)
				{
					CT_BYTE cbLogicValue1 = GetLogicValue(cbLineCard[i]);
					CT_BYTE cbLogicValue2 = GetLogicValue(cbLineCard[i + 1]);
					CT_BYTE cbLogicValue3 = GetLogicValue(cbLineCard[i + 2]);
					if ((cbLogicValue2 - cbLogicValue1) == 1 && (cbLogicValue3 - cbLogicValue2) == 1)
					{
						cbMaxLogicCard = cbLineCard[i + 2];
					}
				}
			}
		}
	}
	cbResultCardData[0] = cbMaxLogicCard;
	return cbMaxLogicCard;
}

CT_BYTE CGameLogic::SearchMaxOneCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[])
{
	//变量定义
	CT_BYTE cbResultCount = 0;

	//构造扑克
	CT_BYTE cbCardData[MAX_COUNT];
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);
	//排列扑克(大到小排序)
	SortCardList(cbCardData, cbCardCount, ST_DESC);
	if (GetLogicValue(cbTurnCardData[0]) < GetLogicValue(cbCardData[0]))
	{
		cbResultCount++;
	}

	return cbResultCount;
}

//构造扑克
CT_BYTE CGameLogic::MakeCardData(CT_BYTE cbValueIndex, CT_BYTE cbColorIndex)
{
	if (cbValueIndex >= 13)
	{
		cbValueIndex -= 13;
	}
	return (cbColorIndex << 4) | (cbValueIndex + 1);
}

//分析扑克
CT_VOID CGameLogic::AnalysebCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagAnalyseResult & AnalyseResult)
{
	//设置结果
	memset(&AnalyseResult, 0, sizeof(AnalyseResult));

	//扑克分析
	for (CT_BYTE i = 0; i < cbCardCount; ++i)
	{
		if (0 == cbCardData[i]) continue;

		//变量定义
		CT_BYTE cbSameCount = 1/*, cbCardValueTemp = 0*/;
		CT_BYTE cbLogicValue = GetLogicValue(cbCardData[i]);

		//搜索同牌
		for (CT_BYTE j = i + 1; j < cbCardCount; ++j)
		{
			//获取扑克
			if (GetLogicValue(cbCardData[j]) != cbLogicValue) break;

			//设置变量
			cbSameCount++;
		}

		//设置结果
		CT_BYTE cbIndex = AnalyseResult.cbBlockCount[cbSameCount - 1]++;
		for (CT_BYTE j = 0; j < cbSameCount; ++j)
		{
			AnalyseResult.cbCardData[cbSameCount - 1][cbIndex*cbSameCount + j] = cbCardData[i + j];
		}

		//设置索引
		i += cbSameCount - 1;
	}

	return;
}

//分析分布
CT_VOID CGameLogic::AnalysebDistributing(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagDistributing & Distributing)
{
	//设置变量
	memset(&Distributing, 0, sizeof(Distributing));

	//设置变量
	for (CT_BYTE i = 0; i < cbCardCount; ++i)
	{
		if (cbCardData[i] == 0) continue;

		//获取属性
		CT_BYTE cbCardColor = GetCardColor(cbCardData[i]);
		CT_BYTE cbCardValue = GetLogicValue(cbCardData[i]);
		//过滤王
		if (cbCardValue>15) continue;

		//分布信息
		Distributing.cbCardCount++;
		Distributing.cbDistributing[cbCardValue - 1][cbIndexCount]++;
		Distributing.cbDistributing[cbCardValue - 1][cbCardColor >> 4]++;
	}

	return;
}
