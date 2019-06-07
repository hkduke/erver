#include "GameLogic.h"
#include <time.h>
#include <algorithm>
#include "math.h"

//////////////////////////////////////////////////////////////////////////////////
//��̬����

//��������
const CT_BYTE cbIndexCount = 5;

//�˿�����
const CT_BYTE	CGameLogic::m_cbCardData[FULL_COUNT] =
{
	0x01,0x02,0x03,0x04,0x05,0x06,0x07,0x08,0x09,0x0A,0x0B,0x0C,0x0D,	//���� A - K
	0x11,0x12,0x13,0x14,0x15,0x16,0x17,0x18,0x19,0x1A,0x1B,0x1C,0x1D,	//÷�� A - K
	0x21,0x22,0x23,0x24,0x25,0x26,0x27,0x28,0x29,0x2A,0x2B,0x2C,0x2D,	//���� A - K
	0x31,0x32,0x33,0x34,0x35,0x36,0x37,0x38,0x39,0x3A,0x3B,0x3C,0x3D,	//���� A - K
	0x4E,0x4F,
};

//////////////////////////////////////////////////////////////////////////////////

//���캯��
CGameLogic::CGameLogic()
{
}

//��������
CGameLogic::~CGameLogic()
{
}

//��ȡ����
CT_BYTE CGameLogic::GetCardType(const CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	//������
	switch (cbCardCount)
	{
	case 0:	//����
	{
		return CT_ERROR;
	}
	case 1: //����
	{
		return CT_SINGLE;
	}
	case 2:	//���ƻ��
	{
		//�����ж�
		if ((cbCardData[0] == 0x4F) && (cbCardData[1] == 0x4E)) return CT_MISSILE_CARD;
		if (GetCardLogicValue(cbCardData[0]) == GetCardLogicValue(cbCardData[1])) return CT_DOUBLE_CARD;

		return CT_ERROR;
	}
	}

	//�����˿�
	tagAnalyseResult AnalyseResult;
	AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

	//�����ж�
	if (AnalyseResult.cbBlockCount[3] > 0)
	{
		//�����ж�
		if ((AnalyseResult.cbBlockCount[3] == 1) && (cbCardCount == 4)) return CT_BOMB_CARD;
		if ((AnalyseResult.cbBlockCount[3] == 1) && (cbCardCount == 6)) return CT_FOUR_TAKE_ONE;
		if ((AnalyseResult.cbBlockCount[3] == 1) && (cbCardCount == 8) && (AnalyseResult.cbBlockCount[1] == 2)) return CT_FOUR_TAKE_TWO;

		/*if (AnalyseResult.cbBlockCount[2] > 1)
		{
			//��������
			CT_BYTE cbCardData = AnalyseResult.cbCardData[2][0];
			CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbCardData);

			//�������
			if (cbFirstLogicValue >= 15) return CT_ERROR;

			//��������
			int nLinkCardCount = 1;
			//������������
			static std::vector<int> vecLinkCardCount;
			for (CT_BYTE i = 1; i < AnalyseResult.cbBlockCount[2]; i++)
			{
				CT_BYTE cbCardData = AnalyseResult.cbCardData[2][i * 3];
				if (cbFirstLogicValue != (GetCardLogicValue(cbCardData) + i))
				{
					nLinkCardCount = 1;
				}
				++nLinkCardCount;
				vecLinkCardCount.push_back(nLinkCardCount);
			}

			//�����������
			int nMaxLinkCardCount = 0;
			for (auto& it : vecLinkCardCount)
			{
				if (it > nMaxLinkCardCount)
				{
					nMaxLinkCardCount = it;
				}
			}
			vecLinkCardCount.clear();

			//�����ж�
			if (nMaxLinkCardCount * 3 == cbCardCount) 
				return CT_THREE_LINE;

			if (nMaxLinkCardCount * 4 == cbCardCount) 
				return CT_THREE_TAKE_ONE;

			if ((nMaxLinkCardCount * 5 == cbCardCount))
				return CT_THREE_TAKE_TWO;

			if (AnalyseResult.cbBlockCount[2] == AnalyseResult.cbBlockCount[1])
				return CT_THREE_TAKE_TWO;
		}*/
		
		return CT_ERROR;
	}

	//�����ж�
	if (AnalyseResult.cbBlockCount[2] > 0)
	{
		//�����ж�
		if (AnalyseResult.cbBlockCount[2] > 1)
		{
			//��������
			/*CT_BYTE cbCardData = AnalyseResult.cbCardData[2][0];
			CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbCardData);

			//�������
			if (cbFirstLogicValue >= 15) return CT_ERROR;

			//�����ж�
			for (CT_BYTE i = 1; i < AnalyseResult.cbBlockCount[2]; i++)
			{
				CT_BYTE cbCardData = AnalyseResult.cbCardData[2][i * 3];
				if (cbFirstLogicValue != (GetCardLogicValue(cbCardData) + i)) return CT_ERROR;
			}*/


			//��������
			CT_BYTE cbCardData = AnalyseResult.cbCardData[2][0];
			CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbCardData);

			//�������
			if (cbFirstLogicValue > 15) return CT_ERROR;

			if (cbFirstLogicValue == 15)
			{
				cbFirstLogicValue = 20;
			}

			//��������
			int nLinkCardCount = 1;
			//������������
			static std::vector<int> vecLinkCardCount;
			for (CT_BYTE i = 1; i < AnalyseResult.cbBlockCount[2]; i++)
			{
				CT_BYTE cbCardData = AnalyseResult.cbCardData[2][i * 3];
				if (cbFirstLogicValue != (GetCardLogicValue(cbCardData) + i))
				{
					nLinkCardCount = 1;
					continue;
				}
				++nLinkCardCount;
				vecLinkCardCount.push_back(nLinkCardCount);
			}

			//�����������
			int nMaxLinkCardCount = 0;
			for (auto& it : vecLinkCardCount)
			{
				if (it > nMaxLinkCardCount)
				{
					nMaxLinkCardCount = it;
				}
			}
			vecLinkCardCount.clear();

			//�����ж�
			if (nMaxLinkCardCount * 3 == cbCardCount)
				return CT_THREE_LINE;

			if (nMaxLinkCardCount * 4 == cbCardCount)
				return CT_THREE_TAKE_ONE;

			if ((nMaxLinkCardCount * 5 == cbCardCount) && (AnalyseResult.cbBlockCount[1] == AnalyseResult.cbBlockCount[2]))
				return CT_THREE_TAKE_TWO;

			//if (AnalyseResult.cbBlockCount[2] == AnalyseResult.cbBlockCount[1])
			//	return CT_THREE_TAKE_TWO;

			return CT_ERROR;
		}
		else if (cbCardCount == 3) return CT_THREE;

		//�����ж�
		if (AnalyseResult.cbBlockCount[2] * 3 == cbCardCount) return CT_THREE_LINE;
		if (AnalyseResult.cbBlockCount[2] * 4 == cbCardCount) return CT_THREE_TAKE_ONE;
		if ((AnalyseResult.cbBlockCount[2] * 5 == cbCardCount) && (AnalyseResult.cbBlockCount[1] == AnalyseResult.cbBlockCount[2])) return CT_THREE_TAKE_TWO;

		/*if (AnalyseResult.cbBlockCount[2] == AnalyseResult.cbBlockCount[1] || AnalyseResult.cbBlockCount[2] == AnalyseResult.cbBlockCount[2])
		{
			return CT_THREE_TAKE_TWO;
		}*/


		return CT_ERROR;
	}

	//��������
	if (AnalyseResult.cbBlockCount[1] >= 3)
	{
		//��������
		CT_BYTE cbCardData = AnalyseResult.cbCardData[1][0];
		CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbCardData);

		//�������
		if (cbFirstLogicValue >= 15) return CT_ERROR;

		//�����ж�
		for (CT_BYTE i = 1; i < AnalyseResult.cbBlockCount[1]; i++)
		{
			CT_BYTE cbCardData = AnalyseResult.cbCardData[1][i * 2];
			if (cbFirstLogicValue != (GetCardLogicValue(cbCardData) + i)) return CT_ERROR;
		}

		//�����ж�
		if ((AnalyseResult.cbBlockCount[1] * 2) == cbCardCount) return CT_DOUBLE_LINE;

		return CT_ERROR;
	}

	//�����ж�
	if ((AnalyseResult.cbBlockCount[0] >= 5) && (AnalyseResult.cbBlockCount[0] == cbCardCount))
	{
		//��������
		CT_BYTE cbCardData = AnalyseResult.cbCardData[0][0];
		CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbCardData);

		//�������
		if (cbFirstLogicValue >= 15) return CT_ERROR;

		//�����ж�
		for (CT_BYTE i = 1; i < AnalyseResult.cbBlockCount[0]; i++)
		{
			CT_BYTE cbCardData = AnalyseResult.cbCardData[0][i];
			if (cbFirstLogicValue != (GetCardLogicValue(cbCardData) + i)) 
				return CT_ERROR;
		}

		return CT_SINGLE_LINE;
	}

	return CT_ERROR;
}

//��ȡ����
CT_BYTE CGameLogic::GetBackCardType(const CT_BYTE cbCardData[])
{
	CT_BYTE	cbCardDataTemp[3];
	memcpy(cbCardDataTemp, cbCardData, sizeof(cbCardDataTemp));
	SortCardList(cbCardDataTemp, 3, ST_ORDER);
	//�����˿�
	tagAnalyseResult AnalyseResult;
	AnalysebCardData(cbCardDataTemp, 3, AnalyseResult);

	if (AnalyseResult.cbBlockCount[2] > 0)
	{
		//����
		return BCT_THREE;
	}
	else if (3 == AnalyseResult.cbBlockCount[0])
	{
		if (0x4F == AnalyseResult.cbCardData[0][0] && 0x4E == AnalyseResult.cbCardData[0][1])
		{
			//˫��
			return BCT_DOUBLE_MISSILE;
		}
		else if (0x4F == AnalyseResult.cbCardData[0][0] || 0x4E == AnalyseResult.cbCardData[0][0]
			|| 0x4F == AnalyseResult.cbCardData[0][1] || 0x4E == AnalyseResult.cbCardData[0][1])
		{
			//����
			return BCT_SINGLE_MISSILE;
		}
		bool bStraight = true;
		bool bFlush = true;

		for (int i = 1; i < 3; i++)
		{
			if (GetCardColor(AnalyseResult.cbCardData[0][0]) != GetCardColor(AnalyseResult.cbCardData[0][i]))
			{
				bStraight = false;
				break;
			}
		}
		if (2 != GetCardValue(AnalyseResult.cbCardData[0][0]) - GetCardValue(AnalyseResult.cbCardData[0][2]))
		{
			bFlush = false;
		}
		if (bFlush && bStraight)
		{
			//ͬ��˳
			return BCT_STRAIGHT_FULSH;
		}
		else if (bFlush)
		{
			//˳��
			return BCT_FULSH;
		}
		else if (bStraight)
		{
			//ͬ��
			return BCT_STRAIGHT;
		}
	}

	return BCT_GENERAL;
}

//�����˿�
CT_VOID CGameLogic::SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbSortType)
{
	//��Ŀ����
	if (cbCardCount == 0) return;
	if (cbSortType == ST_CUSTOM) return;

	//ת����ֵ
	CT_BYTE cbSortValue[MAX_COUNT];
	for (CT_BYTE i = 0; i < cbCardCount; i++) cbSortValue[i] = GetCardLogicValue(cbCardData[i]);

	//�������
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
				//���ñ�־
				bSorted = false;

				//�˿�����
				cbSwitchData = cbCardData[i];
				cbCardData[i] = cbCardData[i + 1];
				cbCardData[i + 1] = cbSwitchData;

				//����Ȩλ
				cbSwitchData = cbSortValue[i];
				cbSortValue[i] = cbSortValue[i + 1];
				cbSortValue[i + 1] = cbSwitchData;
			}
		}
		cbLast--;
	} while (bSorted == false);

	//��Ŀ����
	if (cbSortType == ST_COUNT)
	{
		//��������
		CT_BYTE cbCardIndex = 0;

		//�����˿�
		tagAnalyseResult AnalyseResult;
		AnalysebCardData(&cbCardData[cbCardIndex], cbCardCount - cbCardIndex, AnalyseResult);

		//��ȡ�˿�
		for (CT_BYTE i = 0; i < CountArray(AnalyseResult.cbBlockCount); i++)
		{
			//�����˿�
			CT_BYTE cbIndex = CountArray(AnalyseResult.cbBlockCount) - i - 1;
			memcpy(&cbCardData[cbCardIndex], AnalyseResult.cbCardData[cbIndex], AnalyseResult.cbBlockCount[cbIndex] * (cbIndex + 1) * sizeof(CT_BYTE));

			//��������
			cbCardIndex += AnalyseResult.cbBlockCount[cbIndex] * (cbIndex + 1) * sizeof(CT_BYTE);
		}
	}

	return;
}

//�����˿�
CT_VOID CGameLogic::RandCardList(CT_BYTE cbCardBuffer[], CT_BYTE cbBufferCount, CT_BYTE cbBomCount)
{
	//����׼��
	CT_BYTE cbCardData[FULL_COUNT] = { 0 };
	memcpy(cbCardData, m_cbCardData, sizeof(m_cbCardData));

	//�����˿�
	if (cbBomCount == 0)
	{
		CT_BYTE cbRandCount = 0, cbPosition = 0;
		do
		{
			cbPosition = rand() % (cbBufferCount - cbRandCount);
			cbCardBuffer[cbRandCount++] = cbCardData[cbPosition];
			cbCardData[cbPosition] = cbCardData[cbBufferCount - cbRandCount];
		} while (cbRandCount < cbBufferCount);

		return;
	}

	//ģ����ʵϴ�ƣ�ϴ�ƹ���1���ֿ������ƣ�AB����2��A�ȷ�1-3�ţ�B�ٷ�1-3�ţ�3����1��2�Ĳ����ظ�4�Σ���Ϊ�������ϴ5��
	CT_BYTE ucACount = 0;
	CT_BYTE ucBCount = 0;
	CT_BYTE ucAData[FULL_COUNT] = { 0 };
	CT_BYTE ucBData[FULL_COUNT] = { 0 };
	CT_BYTE ucRandTimes = 4;
	CT_BYTE ucTmp = 0;
	CT_BYTE ucRand = 0;

	//�������ȡ����ը��
#define BOM_COUNT 10		//���10��ը��
	CT_BYTE ucBomTmpCount = rand() % (cbBomCount)+1;	//1-cbBomCount��ը��	
	std::vector<int> vecIndex;	vecIndex.clear();
	CT_BYTE ucTmpIndex = 0;
	CT_BYTE ucBomData[BOM_COUNT][4] = { 0 };	//����ը������	
	CT_BYTE ucLeftCardCount = FULL_COUNT;
	CT_BYTE cbLeftCardData[CountArray(cbCardData)];
	int iIndex = 0;
	if (0 < ucBomTmpCount)
	{
		ucLeftCardCount = 0;
		for (CT_BYTE i = 0; i != ucBomTmpCount; ++i)
		{
			ucTmpIndex = rand() % 13;	//��A-K
			std::vector<int>::iterator iter = std::find(vecIndex.begin(), vecIndex.end(), ucTmpIndex);
			if (iter != vecIndex.end())
			{
				continue;
			}
			vecIndex.push_back(ucTmpIndex);

			ucBomData[iIndex][0] = cbCardData[ucTmpIndex];
			cbCardData[ucTmpIndex] = 0xFF;

			ucTmpIndex += 13;
			ucBomData[iIndex][1] = cbCardData[ucTmpIndex];
			cbCardData[ucTmpIndex] = 0xFF;

			ucTmpIndex += 13;
			ucBomData[iIndex][2] = cbCardData[ucTmpIndex];
			cbCardData[ucTmpIndex] = 0xFF;

			ucTmpIndex += 13;
			ucBomData[iIndex][3] = cbCardData[ucTmpIndex];
			cbCardData[ucTmpIndex] = 0xFF;
			++iIndex;
		}

		//����ʣ�µ��˿���
		for (int i = 0; i != FULL_COUNT; ++i)
		{
			if (0xFF != cbCardData[i])
			{
				cbLeftCardData[ucLeftCardCount++] = cbCardData[i];
			}
		}
	}
	else
	{
		memcpy(cbLeftCardData, cbCardData, sizeof(cbCardData));
	}


	for (int i = 0; i != ucRandTimes; ++i)
	{
		ucACount = ucLeftCardCount / 2;
		ucBCount = ucLeftCardCount - ucACount;
		memcpy(ucAData, cbLeftCardData, ucACount);
		memcpy(ucBData, cbLeftCardData + ucACount, ucBCount);
		//��һ�ε�ʱ��B���Ƶ����������ţ���ը�����������AB�����У���Ȼ�����������ը
		if (0 == i)
		{
			ucRand = rand() % ucACount;
			ucTmp = ucAData[ucRand];	ucAData[ucRand] = ucBData[ucBCount - 2];		ucBData[ucBCount - 2] = ucTmp;
			ucRand = rand() % ucBCount;
			ucTmp = ucBData[ucRand];		ucBData[ucRand] = ucBData[ucBCount - 1];		ucBData[ucBCount - 1] = ucTmp;
		}

		CT_BYTE ucATmp = 0;
		CT_BYTE ucBTmp = 0;
		CT_BYTE ucAIndex = 0;
		CT_BYTE ucBIndex = 0;
		CT_BYTE ucIndex = 0;
		while (ucACount > 0 || ucBCount > 0)
		{
			if (ucACount > 0)
			{
				ucATmp = ucBCount > 0 ? (rand() % 2 + 1) : ucACount;
				if (ucATmp > ucACount)
				{
					ucATmp = ucACount;
				}
				memcpy(cbLeftCardData + ucIndex, ucAData + ucAIndex, ucATmp);
				ucAIndex += ucATmp;
				ucIndex += ucATmp;
				ucACount -= ucATmp;
			}
			if (ucBCount > 0)
			{
				ucBTmp = ucACount > 0 ? (rand() % 2 + 1) : ucBCount;
				if (ucBTmp > ucBCount)
				{
					ucBTmp = ucBCount;
				}
				memcpy(cbLeftCardData + ucIndex, ucBData + ucBIndex, ucBTmp);
				ucBIndex += ucBTmp;
				ucIndex += ucBTmp;
				ucBCount -= ucBTmp;
			}
		}
	}
	//��ը����ʣ���˿����
	CT_BYTE ucIndex = 0;
	CT_BYTE ucData = 0;
	//���ֵ��Ʋ���
	ucLeftCardCount -= 3;
	CT_BYTE cbBankerCard[3] = { 0 };
	memcpy(cbBankerCard, cbLeftCardData + ucLeftCardCount, sizeof(cbBankerCard));
	for (size_t  i = 0; i != vecIndex.size(); ++i)
	{
		CT_BYTE ucRand = rand() % 13;
		if (ucRand > ucLeftCardCount || i == (vecIndex.size() - 1))
		{
			ucRand = ucLeftCardCount;
		}
		memcpy(cbCardData + ucIndex, cbLeftCardData + ucData, ucRand * sizeof(cbCardData[0]));
		ucLeftCardCount -= ucRand;
		ucData += ucRand;
		ucIndex += ucRand;
		memcpy(cbCardData + ucIndex, ucBomData[i], 4 * sizeof(cbCardData[0]));
		ucIndex += 4 * sizeof(cbCardData[0]);
	}
	memcpy(cbCardData + ucIndex, cbBankerCard, sizeof(cbBankerCard));
	memcpy(cbCardBuffer, cbCardData, sizeof(cbCardData));
	return;
}

//ɾ���˿�
bool CGameLogic::RemoveCardList(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	//��������
	assert(cbRemoveCount <= cbCardCount);

	//�������
	CT_BYTE cbDeleteCount = 0, cbTempCardData[MAX_COUNT];
	if (cbCardCount > CountArray(cbTempCardData)) return false;
	memcpy(cbTempCardData, cbCardData, cbCardCount * sizeof(cbCardData[0]));

	//�����˿�
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

	//�����˿�
	CT_BYTE cbCardPos = 0;
	for (CT_BYTE i = 0; i < cbCardCount; i++)
	{
		if (cbTempCardData[i] != 0) cbCardData[cbCardPos++] = cbTempCardData[i];
	}

	return true;
}


//ɾ���˿�
bool CGameLogic::RemoveCard(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	//��������
	assert(cbRemoveCount <= cbCardCount);

	//�������
	CT_BYTE cbDeleteCount = 0, cbTempCardData[MAX_COUNT];
	if (cbCardCount > CountArray(cbTempCardData)) return false;
	memcpy(cbTempCardData, cbCardData, cbCardCount * sizeof(cbCardData[0]));

	//�����˿�
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

	//�����˿�
	CT_BYTE cbCardPos = 0;
	for (CT_BYTE i = 0; i < cbCardCount; i++)
	{
		if (cbTempCardData[i] != 0) cbCardData[cbCardPos++] = cbTempCardData[i];
	}

	return true;
}

//�����˿�
void CGameLogic::SortOutCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	//��ȡ����
	CT_BYTE cbCardType = GetCardType(cbCardData, cbCardCount);

	if (cbCardType == CT_THREE_TAKE_ONE || cbCardType == CT_THREE_TAKE_TWO)
	{
		//������
		tagAnalyseResult AnalyseResult = {};
		AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

		cbCardCount = AnalyseResult.cbBlockCount[2] * 3;
		memcpy(cbCardData, AnalyseResult.cbCardData[2], sizeof(CT_BYTE)*cbCardCount);
		for (int i = CountArray(AnalyseResult.cbBlockCount) - 1; i >= 0; i--)
		{
			if (i == 2) continue;

			if (AnalyseResult.cbBlockCount[i] > 0)
			{
				memcpy(&cbCardData[cbCardCount], AnalyseResult.cbCardData[i],
					sizeof(CT_BYTE)*(i + 1)*AnalyseResult.cbBlockCount[i]);
				cbCardCount += (i + 1)*AnalyseResult.cbBlockCount[i];
			}
		}
	}
	else if (cbCardType == CT_FOUR_TAKE_ONE || cbCardType == CT_FOUR_TAKE_TWO)
	{
		//������
		tagAnalyseResult AnalyseResult = {};
		AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

		cbCardCount = AnalyseResult.cbBlockCount[3] * 4;
		memcpy(cbCardData, AnalyseResult.cbCardData[3], sizeof(CT_BYTE)*cbCardCount);
		for (int i = CountArray(AnalyseResult.cbBlockCount) - 1; i >= 0; i--)
		{
			if (i == 3) continue;

			if (AnalyseResult.cbBlockCount[i] > 0)
			{
				memcpy(&cbCardData[cbCardCount], AnalyseResult.cbCardData[i],
					sizeof(CT_BYTE)*(i + 1)*AnalyseResult.cbBlockCount[i]);
				cbCardCount += (i + 1)*AnalyseResult.cbBlockCount[i];
			}
		}
	}

	return;
}

//�߼���ֵ
CT_BYTE CGameLogic::GetCardLogicValue(CT_BYTE cbCardData)
{
	//�˿�����
	CT_BYTE cbCardColor = GetCardColor(cbCardData);
	CT_BYTE cbCardValue = GetCardValue(cbCardData);

	//ת����ֵ
	if (cbCardColor == 0x40) return cbCardValue + 2;
	return (cbCardValue <= 2) ? (cbCardValue + 13) : cbCardValue;
}

//�Ա��˿�
bool CGameLogic::CompareCard(const CT_BYTE cbFirstCard[], const CT_BYTE cbNextCard[], CT_BYTE cbFirstCount, CT_BYTE cbNextCount)
{
	//��ȡ����
	CT_BYTE cbNextType = GetCardType(cbNextCard, cbNextCount);
	CT_BYTE cbFirstType = GetCardType(cbFirstCard, cbFirstCount);

	//�����ж�
	if (cbNextType == CT_ERROR) return false;
	if (cbNextType == CT_MISSILE_CARD) return true;

	//ը���ж�
	if ((cbFirstType != CT_BOMB_CARD) && (cbNextType == CT_BOMB_CARD)) return true;
	if ((cbFirstType == CT_BOMB_CARD) && (cbNextType != CT_BOMB_CARD)) return false;

	//�����ж�
	if ((cbFirstType != cbNextType) || (cbFirstCount != cbNextCount)) return false;

	//��ʼ�Ա�
	switch (cbNextType)
	{
	case CT_SINGLE:
	case CT_DOUBLE_CARD:
	case CT_THREE:
	case CT_SINGLE_LINE:
	case CT_DOUBLE_LINE:
	case CT_THREE_LINE:
	case CT_BOMB_CARD:
	{
		//��ȡ��ֵ
		CT_BYTE cbNextLogicValue = GetCardLogicValue(cbNextCard[0]);
		CT_BYTE cbFirstLogicValue = GetCardLogicValue(cbFirstCard[0]);

		//�Ա��˿�
		return cbNextLogicValue > cbFirstLogicValue;
	}
	case CT_THREE_TAKE_ONE:
	case CT_THREE_TAKE_TWO:
	{
		//�����˿�
		tagAnalyseResult NextResult;
		tagAnalyseResult FirstResult;
		AnalysebCardData(cbNextCard, cbNextCount, NextResult);
		AnalysebCardData(cbFirstCard, cbFirstCount, FirstResult);

		//��ȡ��ֵ
		CT_BYTE cbNextLogicValue = GetCardLogicValue(NextResult.cbCardData[2][0]);
		CT_BYTE cbFirstLogicValue = GetCardLogicValue(FirstResult.cbCardData[2][0]);

		//�Ա��˿�
		return cbNextLogicValue > cbFirstLogicValue;
	}
	case CT_FOUR_TAKE_ONE:
	case CT_FOUR_TAKE_TWO:
	{
		//�����˿�
		tagAnalyseResult NextResult;
		tagAnalyseResult FirstResult;
		AnalysebCardData(cbNextCard, cbNextCount, NextResult);
		AnalysebCardData(cbFirstCard, cbFirstCount, FirstResult);

		//��ȡ��ֵ
		CT_BYTE cbNextLogicValue = GetCardLogicValue(NextResult.cbCardData[3][0]);
		CT_BYTE cbFirstLogicValue = GetCardLogicValue(FirstResult.cbCardData[3][0]);

		//�Ա��˿�
		return cbNextLogicValue > cbFirstLogicValue;
	}
	}

	return false;
}

//�����˿�
CT_BYTE CGameLogic::MakeCardData(CT_BYTE cbValueIndex, CT_BYTE cbColorIndex)
{
	return (cbColorIndex << 4) | (cbValueIndex + 1);
}

//�����˿�
CT_VOID CGameLogic::AnalysebCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagAnalyseResult & AnalyseResult)
{
	//���ý��
	memset(&AnalyseResult,0, sizeof(AnalyseResult));

	//�˿˷���
	for (CT_BYTE i = 0; i < cbCardCount; i++)
	{
		//��������
		CT_BYTE cbSameCount = 1;//cbCardValueTemp = 0;
		CT_BYTE cbLogicValue = GetCardLogicValue(cbCardData[i]);

		//����ͬ��
		for (CT_BYTE j = i + 1; j < cbCardCount; j++)
		{
			//��ȡ�˿�
			if (GetCardLogicValue(cbCardData[j]) != cbLogicValue) break;

			//���ñ���
			cbSameCount++;
		}

		//���ý��(4����Ҳ������)
		/*if (cbSameCount == 4)
		{
			CT_BYTE cbIndex = AnalyseResult.cbBlockCount[cbSameCount - 1]++;
			for (CT_BYTE j = 0; j < cbSameCount; j++)
				AnalyseResult.cbCardData[cbSameCount - 1][cbIndex*cbSameCount + j] = cbCardData[i + j];

			cbIndex = AnalyseResult.cbBlockCount[cbSameCount - 2]++;
			for (CT_BYTE j = 0; j < cbSameCount - 1; j++)
				AnalyseResult.cbCardData[cbSameCount - 2][cbIndex*(cbSameCount-1) + j] = cbCardData[i + j];
		}
		else*/
		{
			CT_BYTE cbIndex = AnalyseResult.cbBlockCount[cbSameCount - 1]++;
			for (CT_BYTE j = 0; j < cbSameCount; j++) 
				AnalyseResult.cbCardData[cbSameCount - 1][cbIndex*cbSameCount + j] = cbCardData[i + j];
		}
		

		//��������
		i += cbSameCount - 1;
	}

	return;
}

//�����ֲ�
CT_VOID CGameLogic::AnalysebDistributing(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagDistributing & Distributing)
{
	//���ñ���
	memset(&Distributing,0, sizeof(Distributing));

	//���ñ���
	for (CT_BYTE i = 0; i < cbCardCount; i++)
	{
		if (cbCardData[i] == 0) continue;

		//��ȡ����
		CT_BYTE cbCardColor = GetCardColor(cbCardData[i]);
		CT_BYTE cbCardValue = GetCardValue(cbCardData[i]);

		//�ֲ���Ϣ
		Distributing.cbCardCount++;
		Distributing.cbDistributing[cbCardValue - 1][cbIndexCount]++;
		Distributing.cbDistributing[cbCardValue - 1][cbCardColor >> 4]++;
	}

	return;
}

//��������
CT_BYTE CGameLogic::SearchOutCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[], CT_BYTE cbTurnCardCount,
	tagSearchCardResult *pSearchCardResult)
{
	//���ý��
	assert(pSearchCardResult != NULL);
	if (pSearchCardResult == NULL) return 0;

	memset(pSearchCardResult, 0, sizeof(tagSearchCardResult));

	//��������
	CT_BYTE cbResultCount = 0;
	tagSearchCardResult tmpSearchCardResult = {};

	//�����˿�
	CT_BYTE cbCardData[MAX_COUNT];
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//�����˿�
	SortCardList(cbCardData, cbCardCount, ST_ORDER);

	//��ȡ����
	CT_BYTE cbTurnOutType = GetCardType(cbTurnCardData, cbTurnCardCount);

	//���Ʒ���
	switch (cbTurnOutType)
	{
	case CT_ERROR:					//��������
	{
		//��ȡ��������һ��
		assert(pSearchCardResult);
		if (!pSearchCardResult) return 0;

		//�Ƿ�һ�ֳ���
		if (GetCardType(cbCardData, cbCardCount) != CT_ERROR)
		{
			pSearchCardResult->cbCardCount[cbResultCount] = cbCardCount;
			memcpy(pSearchCardResult->cbResultCard[cbResultCount], cbCardData,
				sizeof(CT_BYTE)*cbCardCount);
			cbResultCount++;
		}

		//�����С�Ʋ��ǵ��ƣ�����ȡ
		CT_BYTE cbSameCount = 0;
		if (cbCardCount > 1 && GetCardValue(cbCardData[cbCardCount - 1]) == GetCardValue(cbCardData[cbCardCount - 2]))
		{
			cbSameCount = 1;
			pSearchCardResult->cbResultCard[cbResultCount][0] = cbCardData[cbCardCount - 1];
			CT_BYTE cbCardValue = GetCardValue(cbCardData[cbCardCount - 1]);
			for (int i = cbCardCount - 2; i >= 0; i--)
			{
				if (GetCardValue(cbCardData[i]) == cbCardValue)
				{
					pSearchCardResult->cbResultCard[cbResultCount][cbSameCount++] = cbCardData[i];
				}
				else break;
			}

			pSearchCardResult->cbCardCount[cbResultCount] = cbSameCount;
			cbResultCount++;
		}

		//����
		CT_BYTE cbTmpCount = 0;
		if (cbSameCount != 1)
		{
			cbTmpCount = SearchSameCard(cbCardData, cbCardCount, 0, 1, &tmpSearchCardResult);
			if (cbTmpCount > 0)
			{
				pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
				memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
					sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
				cbResultCount++;
			}
		}

		//����
		if (cbSameCount != 2)
		{
			cbTmpCount = SearchSameCard(cbCardData, cbCardCount, 0, 2, &tmpSearchCardResult);
			if (cbTmpCount > 0)
			{
				pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
				memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
					sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
				cbResultCount++;
			}
		}

		//����
		if (cbSameCount != 3)
		{
			cbTmpCount = SearchSameCard(cbCardData, cbCardCount, 0, 3, &tmpSearchCardResult);
			if (cbTmpCount > 0)
			{
				pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
				memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
					sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
				cbResultCount++;
			}
		}

		//����һ��
		cbTmpCount = SearchTakeCardType(cbCardData, cbCardCount, 0, 3, 1, &tmpSearchCardResult);
		if (cbTmpCount > 0)
		{
			pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
				sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
			cbResultCount++;
		}

		//����һ��
		cbTmpCount = SearchTakeCardType(cbCardData, cbCardCount, 0, 3, 2, &tmpSearchCardResult);
		if (cbTmpCount > 0)
		{
			pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
				sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
			cbResultCount++;
		}

		//����
		cbTmpCount = SearchLineCardType(cbCardData, cbCardCount, 0, 1, 0, &tmpSearchCardResult);
		if (cbTmpCount > 0)
		{
			pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
				sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
			cbResultCount++;
		}

		//����
		cbTmpCount = SearchLineCardType(cbCardData, cbCardCount, 0, 2, 0, &tmpSearchCardResult);
		if (cbTmpCount > 0)
		{
			pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
				sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
			cbResultCount++;
		}

		//����
		cbTmpCount = SearchLineCardType(cbCardData, cbCardCount, 0, 3, 0, &tmpSearchCardResult);
		if (cbTmpCount > 0)
		{
			pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
				sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
			cbResultCount++;
		}

		////�ɻ�
		//cbTmpCount = SearchThreeTwoLine( cbCardData,cbCardCount,&tmpSearchCardResult );
		//if( cbTmpCount > 0 )
		//{
		//	pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
		//	CopyMemory( pSearchCardResult->cbResultCard[cbResultCount],tmpSearchCardResult.cbResultCard[0],
		//		sizeof(BYTE)*tmpSearchCardResult.cbCardCount[0] );
		//	cbResultCount++;
		//}

		//ը��
		if (cbSameCount != 4)
		{
			cbTmpCount = SearchSameCard(cbCardData, cbCardCount, 0, 4, &tmpSearchCardResult);
			if (cbTmpCount > 0)
			{
				pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[0];
				memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[0],
					sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[0]);
				cbResultCount++;
			}
		}

		//�������
		if ((cbCardCount >= 2) && (cbCardData[0] == 0x4F) && (cbCardData[1] == 0x4E))
		{
			//���ý��
			pSearchCardResult->cbCardCount[cbResultCount] = 2;
			pSearchCardResult->cbResultCard[cbResultCount][0] = cbCardData[0];
			pSearchCardResult->cbResultCard[cbResultCount][1] = cbCardData[1];

			cbResultCount++;
		}

		pSearchCardResult->cbSearchCount = cbResultCount;
		return cbResultCount;
	}
	case CT_SINGLE:					//��������
	case CT_DOUBLE_CARD:					//��������
	case CT_THREE:					//��������
	{
		//��������
		CT_BYTE cbReferCard = cbTurnCardData[0];
		CT_BYTE cbSameCount = 1;
		if (cbTurnOutType == CT_DOUBLE_CARD) cbSameCount = 2;
		else if (cbTurnOutType == CT_THREE) cbSameCount = 3;

		//������ͬ��
		cbResultCount = SearchSameCard(cbCardData, cbCardCount, cbReferCard, cbSameCount, pSearchCardResult);

		break;
	}
	case CT_SINGLE_LINE:		//��������
	case CT_DOUBLE_LINE:		//��������
	case CT_THREE_LINE:				//��������
	{
		//��������
		CT_BYTE cbBlockCount = 1;
		if (cbTurnOutType == CT_DOUBLE_LINE) cbBlockCount = 2;
		else if (cbTurnOutType == CT_THREE_LINE) cbBlockCount = 3;

		CT_BYTE cbLineCount = cbTurnCardCount / cbBlockCount;

		//��������
		cbResultCount = SearchLineCardType(cbCardData, cbCardCount, cbTurnCardData[0], cbBlockCount, cbLineCount, pSearchCardResult);

		break;
	}
	case CT_THREE_TAKE_ONE:	//����һ��
	case CT_THREE_TAKE_TWO:	//����һ��
	{
		//Ч������
		if (cbCardCount < cbTurnCardCount) break;

		//���������һ��������
		if (cbTurnCardCount == 4 || cbTurnCardCount == 5)
		{
			CT_BYTE cbTakeCardCount = cbTurnOutType == CT_THREE_TAKE_ONE ? 1 : 2;

			//������������
			cbResultCount = SearchTakeCardType(cbCardData, cbCardCount, cbTurnCardData[2], 3, cbTakeCardCount, pSearchCardResult);
		}
		else
		{
			//��������
			CT_BYTE cbBlockCount = 3;
			CT_BYTE cbLineCount = cbTurnCardCount / (cbTurnOutType == CT_THREE_TAKE_ONE ? 4 : 5);
			CT_BYTE cbTakeCardCount = cbTurnOutType == CT_THREE_TAKE_ONE ? 1 : 2;

			//��������
			CT_BYTE cbTmpTurnCard[MAX_COUNT] = {};
			memcpy(cbTmpTurnCard, cbTurnCardData, sizeof(CT_BYTE)*cbTurnCardCount);
			SortOutCardList(cbTmpTurnCard, cbTurnCardCount);
			cbResultCount = SearchLineCardType(cbCardData, cbCardCount, cbTmpTurnCard[0], cbBlockCount, cbLineCount, pSearchCardResult);

			//��ȡ����
			bool bAllDistill = true;
			for (CT_BYTE i = 0; i < cbResultCount; i++)
			{
				CT_BYTE cbResultIndex = cbResultCount - i - 1;

				//��������
				CT_BYTE cbTmpCardData[MAX_COUNT];
				CT_BYTE cbTmpCardCount = cbCardCount;

				//ɾ������
				memcpy(cbTmpCardData, cbCardData, sizeof(CT_BYTE)*cbCardCount);
				assert(RemoveCard(pSearchCardResult->cbResultCard[cbResultIndex], pSearchCardResult->cbCardCount[cbResultIndex],
					cbTmpCardData, cbTmpCardCount));
				cbTmpCardCount -= pSearchCardResult->cbCardCount[cbResultIndex];

				//������
				tagAnalyseResult  TmpResult = {};
				AnalysebCardData(cbTmpCardData, cbTmpCardCount, TmpResult);

				//��ȡ��
				CT_BYTE cbDistillCard[MAX_COUNT] = {};
				CT_BYTE cbDistillCount = 0;
				for (CT_BYTE j = cbTakeCardCount - 1; j < CountArray(TmpResult.cbBlockCount); j++)
				{
					if (TmpResult.cbBlockCount[j] > 0)
					{
						if (j + 1 == cbTakeCardCount && TmpResult.cbBlockCount[j] >= cbLineCount)
						{
							CT_BYTE cbTmpBlockCount = TmpResult.cbBlockCount[j];
							memcpy(cbDistillCard, &TmpResult.cbCardData[j][(cbTmpBlockCount - cbLineCount)*(j + 1)],
								sizeof(CT_BYTE)*(j + 1)*cbLineCount);
							cbDistillCount = (j + 1)*cbLineCount;
							break;
						}
						else
						{
							for (CT_BYTE k = 0; k < TmpResult.cbBlockCount[j]; k++)
							{
								CT_BYTE cbTmpBlockCount = TmpResult.cbBlockCount[j];
								memcpy(&cbDistillCard[cbDistillCount], &TmpResult.cbCardData[j][(cbTmpBlockCount - k - 1)*(j + 1)],
									sizeof(CT_BYTE)*cbTakeCardCount);
								cbDistillCount += cbTakeCardCount;

								//��ȡ���
								if (cbDistillCount == cbTakeCardCount*cbLineCount) break;
							}
						}
					}

					//��ȡ���
					if (cbDistillCount == cbTakeCardCount*cbLineCount) break;
				}

				//��ȡ���
				if (cbDistillCount == cbTakeCardCount*cbLineCount)
				{
					//���ƴ���
					CT_BYTE cbCount = pSearchCardResult->cbCardCount[cbResultIndex];
					memcpy(&pSearchCardResult->cbResultCard[cbResultIndex][cbCount], cbDistillCard,
						sizeof(CT_BYTE)*cbDistillCount);
					pSearchCardResult->cbCardCount[cbResultIndex] += cbDistillCount;
				}
				//����ɾ������
				else
				{
					bAllDistill = false;
					pSearchCardResult->cbCardCount[cbResultIndex] = 0;
				}
			}

			//�������
			if (!bAllDistill)
			{
				pSearchCardResult->cbSearchCount = cbResultCount;
				cbResultCount = 0;
				for (CT_BYTE i = 0; i < pSearchCardResult->cbSearchCount; i++)
				{
					if (pSearchCardResult->cbCardCount[i] != 0)
					{
						tmpSearchCardResult.cbCardCount[cbResultCount] = pSearchCardResult->cbCardCount[i];
						memcpy(tmpSearchCardResult.cbResultCard[cbResultCount], pSearchCardResult->cbResultCard[i],
							sizeof(CT_BYTE)*pSearchCardResult->cbCardCount[i]);
						cbResultCount++;
					}
				}
				tmpSearchCardResult.cbSearchCount = cbResultCount;
				memcpy(pSearchCardResult, &tmpSearchCardResult, sizeof(tagSearchCardResult));
			}
		}

		break;
	}
	case CT_FOUR_TAKE_ONE:		//�Ĵ�����
	case CT_FOUR_TAKE_TWO:		//�Ĵ���˫
	{
		CT_BYTE cbTakeCount = cbTurnOutType == CT_FOUR_TAKE_ONE ? 1 : 2;

		CT_BYTE cbTmpTurnCard[MAX_COUNT] = {};
		memcpy(cbTmpTurnCard, cbTurnCardData, sizeof(CT_BYTE)*cbTurnCardCount);
		SortOutCardList(cbTmpTurnCard, cbTurnCardCount);

		//��������
		cbResultCount = SearchTakeCardType(cbCardData, cbCardCount, cbTmpTurnCard[0], 4, cbTakeCount, pSearchCardResult);

		break;
	}
	}

	//����ը��
	if ((cbCardCount >= 4) && (cbTurnOutType != CT_MISSILE_CARD))
	{
		//��������
		CT_BYTE cbReferCard = 0;
		if (cbTurnOutType == CT_BOMB_CARD) cbReferCard = cbTurnCardData[0];

		//����ը��
		CT_BYTE cbTmpResultCount = SearchSameCard(cbCardData, cbCardCount, cbReferCard, 4, &tmpSearchCardResult);
		for (CT_BYTE i = 0; i < cbTmpResultCount; i++)
		{
			pSearchCardResult->cbCardCount[cbResultCount] = tmpSearchCardResult.cbCardCount[i];
			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSearchCardResult.cbResultCard[i],
				sizeof(CT_BYTE)*tmpSearchCardResult.cbCardCount[i]);
			cbResultCount++;
		}
	}

	//�������
	if (cbTurnOutType != CT_MISSILE_CARD && (cbCardCount >= 2) && (cbCardData[0] == 0x4F) && (cbCardData[1] == 0x4E))
	{
		//���ý��
		pSearchCardResult->cbCardCount[cbResultCount] = 2;
		pSearchCardResult->cbResultCard[cbResultCount][0] = cbCardData[0];
		pSearchCardResult->cbResultCard[cbResultCount][1] = cbCardData[1];

		cbResultCount++;
	}

	pSearchCardResult->cbSearchCount = cbResultCount;
	return cbResultCount;
}

//ͬ������
CT_BYTE CGameLogic::SearchSameCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCardCount,
	tagSearchCardResult *pSearchCardResult)
{
	//���ý��
	if (pSearchCardResult)
		memset(pSearchCardResult, 0, sizeof(tagSearchCardResult));
	CT_BYTE cbResultCount = 0;

	//�����˿�
	CT_BYTE cbCardData[MAX_COUNT];
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//�����˿�
	SortCardList(cbCardData, cbCardCount, ST_ORDER);

	//�����˿�
	tagAnalyseResult AnalyseResult = {};
	AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

	CT_BYTE cbReferLogicValue = cbReferCard == 0 ? 0 : GetCardLogicValue(cbReferCard);
	CT_BYTE cbBlockIndex = cbSameCardCount - 1;
	do
	{
		for (CT_BYTE i = 0; i < AnalyseResult.cbBlockCount[cbBlockIndex]; i++)
		{
			CT_BYTE cbIndex = (AnalyseResult.cbBlockCount[cbBlockIndex] - i - 1)*(cbBlockIndex + 1);
			if (GetCardLogicValue(AnalyseResult.cbCardData[cbBlockIndex][cbIndex]) > cbReferLogicValue)
			{
				if (pSearchCardResult == NULL) return 1;

				assert(cbResultCount < CountArray(pSearchCardResult->cbCardCount));

				//�����˿�
				memcpy(pSearchCardResult->cbResultCard[cbResultCount], &AnalyseResult.cbCardData[cbBlockIndex][cbIndex],
					cbSameCardCount * sizeof(CT_BYTE));
				pSearchCardResult->cbCardCount[cbResultCount] = cbSameCardCount;

				cbResultCount++;
			}
		}

		cbBlockIndex++;
	} while (cbBlockIndex < CountArray(AnalyseResult.cbBlockCount));

	if (pSearchCardResult)
		pSearchCardResult->cbSearchCount = cbResultCount;
	return cbResultCount;
}

//��������
CT_BYTE CGameLogic::SearchLineCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbBlockCount, CT_BYTE cbLineCount,
	tagSearchCardResult *pSearchCardResult)
{
	//���ý��
	if (pSearchCardResult)
		memset(pSearchCardResult, 0, sizeof(tagSearchCardResult));
	CT_BYTE cbResultCount = 0;

	//�������
	CT_BYTE cbLessLineCount = 0;
	if (cbLineCount == 0)
	{
		if (cbBlockCount == 1)
			cbLessLineCount = 5;
		else if (cbBlockCount == 2)
			cbLessLineCount = 3;
		else cbLessLineCount = 2;
	}
	else cbLessLineCount = cbLineCount;

	CT_BYTE cbReferIndex = 2;
	if (cbReferCard != 0)
	{
		assert(GetCardLogicValue(cbReferCard) - cbLessLineCount >= 2);
		cbReferIndex = GetCardLogicValue(cbReferCard) - cbLessLineCount + 1;
	}
	//����A
	if (cbReferIndex + cbLessLineCount > 14) return cbResultCount;

	//�����ж�
	if (cbHandCardCount < cbLessLineCount*cbBlockCount) return cbResultCount;

	//�����˿�
	CT_BYTE cbCardData[MAX_COUNT];
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//�����˿�
	SortCardList(cbCardData, cbCardCount, ST_ORDER);

	//�����˿�
	tagDistributing Distributing = {};
	AnalysebDistributing(cbCardData, cbCardCount, Distributing);

	//����˳��
	CT_BYTE cbTmpLinkCount = 0;
	CT_BYTE cbValueIndex = 0;
	for (cbValueIndex = cbReferIndex; cbValueIndex < 13; cbValueIndex++)
	{
		//�����ж�
		if (Distributing.cbDistributing[cbValueIndex][cbIndexCount] < cbBlockCount)
		{
			if (cbTmpLinkCount < cbLessLineCount)
			{
				cbTmpLinkCount = 0;
				continue;
			}
			else cbValueIndex--;
		}
		else
		{
			cbTmpLinkCount++;
			//Ѱ�����
			if (cbLineCount == 0) continue;
		}

		if (cbTmpLinkCount >= cbLessLineCount)
		{
			if (pSearchCardResult == NULL) return 1;

			assert(cbResultCount < CountArray(pSearchCardResult->cbCardCount));

			//�����˿�
			CT_BYTE cbCount = 0;
			for (CT_BYTE cbIndex = cbValueIndex + 1 - cbTmpLinkCount; cbIndex <= cbValueIndex; cbIndex++)
			{
				CT_BYTE cbTmpCount = 0;
				for (CT_BYTE cbColorIndex = 0; cbColorIndex < 4; cbColorIndex++)
				{
					for (CT_BYTE cbColorCount = 0; cbColorCount < Distributing.cbDistributing[cbIndex][3 - cbColorIndex]; cbColorCount++)
					{
						pSearchCardResult->cbResultCard[cbResultCount][cbCount++] = MakeCardData(cbIndex, 3 - cbColorIndex);

						if (++cbTmpCount == cbBlockCount) break;
					}
					if (cbTmpCount == cbBlockCount) break;
				}
			}

			//���ñ���
			pSearchCardResult->cbCardCount[cbResultCount] = cbCount;
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

	//����˳��
	if (cbTmpLinkCount >= cbLessLineCount - 1 && cbValueIndex == 13)
	{
		if (Distributing.cbDistributing[0][cbIndexCount] >= cbBlockCount ||
			cbTmpLinkCount >= cbLessLineCount)
		{
			if (pSearchCardResult == NULL) return 1;

			assert(cbResultCount < CountArray(pSearchCardResult->cbCardCount));

			//�����˿�
			CT_BYTE cbCount = 0;
			CT_BYTE cbTmpCount = 0;
			for (CT_BYTE cbIndex = cbValueIndex - cbTmpLinkCount; cbIndex < 13; cbIndex++)
			{
				cbTmpCount = 0;
				for (CT_BYTE cbColorIndex = 0; cbColorIndex < 4; cbColorIndex++)
				{
					for (CT_BYTE cbColorCount = 0; cbColorCount < Distributing.cbDistributing[cbIndex][3 - cbColorIndex]; cbColorCount++)
					{
						pSearchCardResult->cbResultCard[cbResultCount][cbCount++] = MakeCardData(cbIndex, 3 - cbColorIndex);

						if (++cbTmpCount == cbBlockCount) break;
					}
					if (cbTmpCount == cbBlockCount) break;
				}
			}
			//����A
			if (Distributing.cbDistributing[0][cbIndexCount] >= cbBlockCount)
			{
				cbTmpCount = 0;
				for (CT_BYTE cbColorIndex = 0; cbColorIndex < 4; cbColorIndex++)
				{
					for (CT_BYTE cbColorCount = 0; cbColorCount < Distributing.cbDistributing[0][3 - cbColorIndex]; cbColorCount++)
					{
						pSearchCardResult->cbResultCard[cbResultCount][cbCount++] = MakeCardData(0, 3 - cbColorIndex);

						if (++cbTmpCount == cbBlockCount) break;
					}
					if (cbTmpCount == cbBlockCount) break;
				}
			}

			//���ñ���
			pSearchCardResult->cbCardCount[cbResultCount] = cbCount;
			cbResultCount++;
		}
	}

	if (pSearchCardResult)
		pSearchCardResult->cbSearchCount = cbResultCount;
	return cbResultCount;
}

// ������������(����һ���Ĵ�һ��)
CT_BYTE CGameLogic::SearchTakeCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCount, CT_BYTE cbTakeCardCount,
	tagSearchCardResult *pSearchCardResult)
{
	//���ý��
	if (pSearchCardResult)
		memset(pSearchCardResult, 0, sizeof(tagSearchCardResult));
	CT_BYTE cbResultCount = 0;

	//Ч��
	assert(cbSameCount == 3 || cbSameCount == 4);
	assert(cbTakeCardCount == 1 || cbTakeCardCount == 2);
	if (cbSameCount != 3 && cbSameCount != 4)
		return cbResultCount;
	if (cbTakeCardCount != 1 && cbTakeCardCount != 2)
		return cbResultCount;

	//�����ж�
	if ((cbSameCount == 4 && cbHandCardCount < cbSameCount + cbTakeCardCount * 2) ||
		cbHandCardCount < cbSameCount + cbTakeCardCount)
		return cbResultCount;

	//�����˿�
	CT_BYTE cbCardData[MAX_COUNT];
	CT_BYTE cbCardCount = cbHandCardCount;
	memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);

	//�����˿�
	SortCardList(cbCardData, cbCardCount, ST_ORDER);

	//����ͬ��
	tagSearchCardResult SameCardResult = {};
	CT_BYTE cbSameCardResultCount = SearchSameCard(cbCardData, cbCardCount, cbReferCard, cbSameCount, &SameCardResult);

	if (cbSameCardResultCount > 0)
	{
		//�����˿�
		tagAnalyseResult AnalyseResult;
		AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

		//��Ҫ����
		CT_BYTE cbNeedCount = cbSameCount + cbTakeCardCount;
		if (cbSameCount == 4) cbNeedCount += cbTakeCardCount;

		//��ȡ����
		for (CT_BYTE i = 0; i < cbSameCardResultCount; i++)
		{
			bool bMerge = false;

			for (CT_BYTE j = cbTakeCardCount - 1; j < CountArray(AnalyseResult.cbBlockCount); j++)
			{
				for (CT_BYTE k = 0; k < AnalyseResult.cbBlockCount[j]; k++)
				{
					//��С����
					CT_BYTE cbIndex = (AnalyseResult.cbBlockCount[j] - k - 1)*(j + 1);

					//������ͬ��
					if (GetCardValue(SameCardResult.cbResultCard[i][0]) ==
						GetCardValue(AnalyseResult.cbCardData[j][cbIndex]))
						continue;

					//���ƴ���
					CT_BYTE cbCount = SameCardResult.cbCardCount[i];
					memcpy(&SameCardResult.cbResultCard[i][cbCount], &AnalyseResult.cbCardData[j][cbIndex],
						sizeof(CT_BYTE)*cbTakeCardCount);
					SameCardResult.cbCardCount[i] += cbTakeCardCount;

					if (SameCardResult.cbCardCount[i] < cbNeedCount) continue;

					if (pSearchCardResult == NULL) return 1;

					//���ƽ��
					memcpy(pSearchCardResult->cbResultCard[cbResultCount], SameCardResult.cbResultCard[i],
						sizeof(CT_BYTE)*SameCardResult.cbCardCount[i]);
					pSearchCardResult->cbCardCount[cbResultCount] = SameCardResult.cbCardCount[i];
					cbResultCount++;

					bMerge = true;

					//��һ���
					break;
				}

				if (bMerge) break;
			}
		}
	}

	if (pSearchCardResult)
		pSearchCardResult->cbSearchCount = cbResultCount;
	return cbResultCount;
}

//�����ɻ�
CT_BYTE CGameLogic::SearchThreeTwoLine(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, tagSearchCardResult *pSearchCardResult)
{
	//���ý��
	if (pSearchCardResult)
		memset(pSearchCardResult, 0, sizeof(tagSearchCardResult));

	//��������
	tagSearchCardResult tmpSearchResult = {};
	tagSearchCardResult tmpSingleWing = {};
	tagSearchCardResult tmpDoubleWing = {};
	CT_BYTE cbTmpResultCount = 0;

	//��������
	cbTmpResultCount = SearchLineCardType(cbHandCardData, cbHandCardCount, 0, 3, 0, &tmpSearchResult);

	if (cbTmpResultCount > 0)
	{
		//��ȡ����
		for (CT_BYTE i = 0; i < cbTmpResultCount; i++)
		{
			//��������
			CT_BYTE cbTmpCardData[MAX_COUNT];
			CT_BYTE cbTmpCardCount = cbHandCardCount;

			//������
			if (cbHandCardCount - tmpSearchResult.cbCardCount[i] < tmpSearchResult.cbCardCount[i] / 3)
			{
				CT_BYTE cbNeedDelCount = 3;
				while (cbHandCardCount + cbNeedDelCount - tmpSearchResult.cbCardCount[i] < (tmpSearchResult.cbCardCount[i] - cbNeedDelCount) / 3)
					cbNeedDelCount += 3;
				//��������
				if ((tmpSearchResult.cbCardCount[i] - cbNeedDelCount) / 3 < 2)
				{
					//�ϳ�����
					continue;
				}

				//�������
				RemoveCard(tmpSearchResult.cbResultCard[i], cbNeedDelCount, tmpSearchResult.cbResultCard[i],
					tmpSearchResult.cbCardCount[i]);
				tmpSearchResult.cbCardCount[i] -= cbNeedDelCount;
			}

			if (pSearchCardResult == NULL) return 1;

			//ɾ������
			memcpy(cbTmpCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);
			assert(RemoveCard(tmpSearchResult.cbResultCard[i], tmpSearchResult.cbCardCount[i],
				cbTmpCardData, cbTmpCardCount));
			cbTmpCardCount -= tmpSearchResult.cbCardCount[i];

			//��Ϸɻ�
			CT_BYTE cbNeedCount = tmpSearchResult.cbCardCount[i] / 3;
			assert(cbNeedCount <= cbTmpCardCount);

			CT_BYTE cbResultCount = tmpSingleWing.cbSearchCount++;
			memcpy(tmpSingleWing.cbResultCard[cbResultCount], tmpSearchResult.cbResultCard[i],
				sizeof(CT_BYTE)*tmpSearchResult.cbCardCount[i]);
			memcpy(&tmpSingleWing.cbResultCard[cbResultCount][tmpSearchResult.cbCardCount[i]],
				&cbTmpCardData[cbTmpCardCount - cbNeedCount], sizeof(CT_BYTE)*cbNeedCount);
			tmpSingleWing.cbCardCount[i] = tmpSearchResult.cbCardCount[i] + cbNeedCount;

			//���������
			if (cbTmpCardCount < tmpSearchResult.cbCardCount[i] / 3 * 2)
			{
				CT_BYTE cbNeedDelCount = 3;
				while (cbTmpCardCount + cbNeedDelCount - tmpSearchResult.cbCardCount[i] < (tmpSearchResult.cbCardCount[i] - cbNeedDelCount) / 3 * 2)
					cbNeedDelCount += 3;
				//��������
				if ((tmpSearchResult.cbCardCount[i] - cbNeedDelCount) / 3 < 2)
				{
					//�ϳ�����
					continue;
				}

				//�������
				RemoveCard(tmpSearchResult.cbResultCard[i], cbNeedDelCount, tmpSearchResult.cbResultCard[i],
					tmpSearchResult.cbCardCount[i]);
				tmpSearchResult.cbCardCount[i] -= cbNeedDelCount;

				//����ɾ������
				memcpy(cbTmpCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);
				assert(RemoveCard(tmpSearchResult.cbResultCard[i], tmpSearchResult.cbCardCount[i],
					cbTmpCardData, cbTmpCardCount));
				cbTmpCardCount = cbHandCardCount - tmpSearchResult.cbCardCount[i];
			}

			//������
			tagAnalyseResult  TmpResult = {};
			AnalysebCardData(cbTmpCardData, cbTmpCardCount, TmpResult);

			//��ȡ���
			CT_BYTE cbDistillCard[MAX_COUNT] = {};
			CT_BYTE cbDistillCount = 0;
			CT_BYTE cbLineCount = tmpSearchResult.cbCardCount[i] / 3;
			for (CT_BYTE j = 1; j < CountArray(TmpResult.cbBlockCount); j++)
			{
				if (TmpResult.cbBlockCount[j] > 0)
				{
					if (j + 1 == 2 && TmpResult.cbBlockCount[j] >= cbLineCount)
					{
						CT_BYTE cbTmpBlockCount = TmpResult.cbBlockCount[j];
						memcpy(cbDistillCard, &TmpResult.cbCardData[j][(cbTmpBlockCount - cbLineCount)*(j + 1)],
							sizeof(CT_BYTE)*(j + 1)*cbLineCount);
						cbDistillCount = (j + 1)*cbLineCount;
						break;
					}
					else
					{
						for (CT_BYTE k = 0; k < TmpResult.cbBlockCount[j]; k++)
						{
							CT_BYTE cbTmpBlockCount = TmpResult.cbBlockCount[j];
							memcpy(&cbDistillCard[cbDistillCount], &TmpResult.cbCardData[j][(cbTmpBlockCount - k - 1)*(j + 1)],
								sizeof(CT_BYTE) * 2);
							cbDistillCount += 2;

							//��ȡ���
							if (cbDistillCount == 2 * cbLineCount) break;
						}
					}
				}

				//��ȡ���
				if (cbDistillCount == 2 * cbLineCount) break;
			}

			//��ȡ���
			if (cbDistillCount == 2 * cbLineCount)
			{
				//���Ƴ��
				cbResultCount = tmpDoubleWing.cbSearchCount++;
				memcpy(tmpDoubleWing.cbResultCard[cbResultCount], tmpSearchResult.cbResultCard[i],
					sizeof(CT_BYTE)*tmpSearchResult.cbCardCount[i]);
				memcpy(&tmpDoubleWing.cbResultCard[cbResultCount][tmpSearchResult.cbCardCount[i]],
					cbDistillCard, sizeof(CT_BYTE)*cbDistillCount);
				tmpDoubleWing.cbCardCount[i] = tmpSearchResult.cbCardCount[i] + cbDistillCount;
			}
		}

		//���ƽ��
		for (CT_BYTE i = 0; i < tmpDoubleWing.cbSearchCount; i++)
		{
			CT_BYTE cbResultCount = pSearchCardResult->cbSearchCount++;

			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpDoubleWing.cbResultCard[i],
				sizeof(CT_BYTE)*tmpDoubleWing.cbCardCount[i]);
			pSearchCardResult->cbCardCount[cbResultCount] = tmpDoubleWing.cbCardCount[i];
		}
		for (CT_BYTE i = 0; i < tmpSingleWing.cbSearchCount; i++)
		{
			CT_BYTE cbResultCount = pSearchCardResult->cbSearchCount++;

			memcpy(pSearchCardResult->cbResultCard[cbResultCount], tmpSingleWing.cbResultCard[i],
				sizeof(CT_BYTE)*tmpSingleWing.cbCardCount[i]);
			pSearchCardResult->cbCardCount[cbResultCount] = tmpSingleWing.cbCardCount[i];
		}
	}

	return pSearchCardResult == NULL ? 0 : pSearchCardResult->cbSearchCount;
}
//////////////////////////////////////////////////////////////////////////////////
