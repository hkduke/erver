#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_DDZ.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>


//�궨��
//////////////////////////////////////////////////////////////////////////////////

//��������
#define ST_ORDER					1									//��С����
#define ST_COUNT					2									//��Ŀ����
#define ST_CUSTOM					3									//�Զ�����

#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//////////////////////////////////////////////////////////////////////////////////

//�����ṹ
struct tagAnalyseResult
{
	CT_BYTE 						cbBlockCount[4];					//�˿���Ŀ0:���Ÿ���,1:���Ӹ���,2:���Ÿ���,3:���Ÿ���
	CT_BYTE							cbCardData[4][MAX_COUNT];			//�˿�����(3344�Ĵ����:cbCardData[1][3344])
};

//���ƽ��
struct tagOutCardResult
{
	CT_BYTE							cbCardCount;						//�˿���Ŀ
	CT_BYTE							cbResultCard[MAX_COUNT];			//����˿�
};

//�ֲ���Ϣ
struct tagDistributing
{
	CT_BYTE							cbCardCount;						//�˿���Ŀ
	CT_BYTE							cbDistributing[15][6];				//�ֲ���Ϣ(һά:���߼�ֵ��1;��ά:�±�0-4��Ӧ��÷���,�±�5��Ӧ��ǰ���ܸ���)
};

//�������
struct tagSearchCardResult
{
	CT_BYTE							cbSearchCount;						//�����Ŀ
	CT_BYTE							cbCardCount[MAX_COUNT];				//�˿���Ŀ
	CT_BYTE							cbResultCard[MAX_COUNT][MAX_COUNT];	//����˿�
};

//////////////////////////////////////////////////////////////////////////////////

//��Ϸ�߼���
class CGameLogic
{
	//��������
protected:
	static const CT_BYTE				m_cbCardData[FULL_COUNT];			//�˿�����
	static const CT_BYTE				m_cbGoodCardData[GOOD_CARD_COUTN];	//��������
	//��������
public:
	//���캯��
	CGameLogic();
	//��������
	virtual ~CGameLogic();

	//���ͺ���
public:
	//��ȡ����
	CT_BYTE GetCardType(const CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//��ȡ����
	CT_BYTE GetBackCardType(const CT_BYTE cbCardData[]);
	//��ȡ��ֵ
	CT_BYTE GetCardValue(CT_BYTE cbCardData) { return cbCardData&MASK_VALUE; }
	//��ȡ��ɫ
	CT_BYTE GetCardColor(CT_BYTE cbCardData) { return cbCardData&MASK_COLOR; }

	//���ƺ���
public:
	//�����˿�
	CT_VOID RandCardList(CT_BYTE cbCardBuffer[], CT_BYTE cbBufferCount, CT_BYTE cbBomCount);
	//�����˿�
	CT_VOID SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbSortType);
	//ɾ���˿�
	bool RemoveCardList(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//ɾ���˿�
	bool RemoveCard(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//�����˿�
	void SortOutCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount);
	//�õ�����
	CT_VOID GetGoodCardData(CT_BYTE cbGoodCardData[], CT_BYTE cbGoodCardCount);
	//ɾ������
	CT_BOOL RemoveGoodCardData(CT_BYTE cbGoodCardData[], CT_BYTE cbGoodCardCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);

	//�߼�����
public:
	//�߼���ֵ
	CT_BYTE GetCardLogicValue(CT_BYTE cbCardData);
	//�Ա��˿�
	bool CompareCard(const CT_BYTE cbFirstCard[], const CT_BYTE cbNextCard[], CT_BYTE cbFirstCount, CT_BYTE cbNextCount);
	//��������
	CT_BYTE SearchOutCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[], CT_BYTE cbTurnCardCount,
		tagSearchCardResult *pSearchCardResult);
	//ͬ������
	CT_BYTE SearchSameCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCardCount,
		tagSearchCardResult *pSearchCardResult);
	//��������
	CT_BYTE SearchLineCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbBlockCount, CT_BYTE cbLineCount,
		tagSearchCardResult *pSearchCardResult);
	//������������(����һ���Ĵ�һ��)
	CT_BYTE SearchTakeCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCount, CT_BYTE cbTakeCardCount,
		tagSearchCardResult *pSearchCardResult);
	//�����ɻ�
	CT_BYTE SearchThreeTwoLine(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, tagSearchCardResult *pSearchCardResult);

	//�ڲ�����
public:
	//�����˿�
	CT_BYTE MakeCardData(CT_BYTE cbValueIndex, CT_BYTE cbColorIndex);
	//�����˿�
	CT_VOID AnalysebCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagAnalyseResult & AnalyseResult);
	//�����ֲ�
	CT_VOID AnalysebDistributing(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagDistributing & Distributing);
};

//////////////////////////////////////////////////////////////////////////////////

#endif		 


