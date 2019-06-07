#ifndef GAME_LOGIC_HEAD_FILE
#define GAME_LOGIC_HEAD_FILE

#include "MSG_PDK_16.h"
#include "CTType.h"
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <memory.h>
#include <vector>


//�궨��
//////////////////////////////////////////////////////////////////////////
//�߼�����
#define MASK_COLOR					0xF0								//��ɫ����
#define MASK_VALUE					0x0F								//��ֵ����
#define MAX_CARD_TOTAL				48									//���ܸ���

//��������
#define ST_DESC						1									//��С����
#define ST_ACS						2									//С��������

//�߼�����
#define CT_ERROR					0									//��������
#define CT_SINGLE					1									//��������
#define CT_DOUBLE_Ex				2									//��������
#define CT_THREE_TAKE_TWO			3									//������   
#define CT_SINGLE_LINE				4									//��������
#define CT_DOUBLE_LINE				5									//��������
#define CT_THREE_LINE_ONE			6									//������һ
#define CT_AIRPLANE					7									//�������� (�ɻ�)
#define CT_FOUR_TAKE_TWO			8									//4��2
#define CT_FOUR_TAKE_THREE			9									//4��3
#define CT_BOMB_CARD				10									//ը������

/*
enum PDK_PlayKind
{
FEATURE = 1,		//��ɫ�淨
CLASSIC = 2,		//�����淨
};
*/

//����ά��
#define CountArray(Array) (sizeof(Array)/sizeof(Array[0]))

//�����ṹ
struct tagAnalyseResult
{
	CT_BYTE 					cbBlockCount[4];						//�˿���Ŀ0:���Ÿ���,1:���Ӹ���,2:���Ÿ���,3:���Ÿ���
	CT_BYTE						cbCardData[4][MAX_COUNT];				//�˿�����(3344�Ĵ����:cbCardData[1][3344])
};

//�ֲ���Ϣ
struct tagDistributing
{
	CT_BYTE						cbCardCount;							//�˿���Ŀ(�ܵ�������)
	CT_BYTE						cbDistributing[15][6];					//�ֲ���Ϣ(һά:���߼�ֵ��1;��ά:�±�0-4��Ӧ��÷���,�±�5��Ӧ��ǰ���ܸ���)
};

//�������
struct tagSearchCardResult
{
	CT_BYTE						cbSearchCount;							//�����Ŀ
	CT_BYTE						cbCardCount[MAX_COUNT];					//�˿���Ŀ
	CT_BYTE						cbResultCard[MAX_COUNT][MAX_COUNT];		//����˿�
};

//��Ϸ�߼���
class CGameLogic
{
	//��������
public:
	//���캯��
	CGameLogic();
	//��������
	virtual ~CGameLogic();

	//��������
protected:
	static const CT_BYTE		m_byCardDataArray[MAX_CARD_TOTAL];		//�˿�����
	static const CT_BYTE		m_byCardDataArray15[MAX_CARD_TOTAL];	//�˿�����

public:
	//PDK_PlayKind				m_PlayKind;
	CT_BYTE						m_cbNotSplitRomb;						//ը�����ɲ�  
	CT_BYTE						m_cbCanFourTakeTwo;						//����4��2
	CT_BYTE						m_cbCanFourTakeThree;					//����4��3
																		//���һ����
	CT_BYTE						m_cbIsOutThreeEnd;						//���ſ��ٴ����� 
	CT_BYTE						m_cbIsThreeEnd;							//���ſ��ٴ����� 
	CT_BYTE						m_cbIsOutFlyEnd;						//�ɻ����ٴ����� 
	CT_BYTE						m_cbIsFlyEnd;							//�ɻ����ٴ�����

																		//���ƺ���
public:
	//�����˿�
	CT_VOID RandCardData(CT_BYTE byCardBuffer[], CT_BYTE byBufferCount);
	//�����˿�
	CT_VOID SortCardList(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbSortType = ST_DESC);
	//ɾ���˿�
	CT_BOOL RemoveCard(const CT_BYTE cbRemoveCard[], CT_BYTE cbRemoveCount, CT_BYTE cbCardData[], CT_BYTE cbCardCount);

	//��������
public:
	//��ȡ��ֵ
	CT_BYTE GetCardValue(CT_BYTE cbCardData) { return cbCardData&MASK_VALUE; }
	//��ȡ��ɫ
	CT_BYTE GetCardColor(CT_BYTE cbCardData) { return cbCardData&MASK_COLOR; }
	//�߼���ֵ
	CT_BYTE GetLogicValue(CT_BYTE cbCardData);
	//��ȡ����
	CT_BYTE GetCardType(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BOOL bLastCard = false, CT_BYTE cbAirLineCount[] = NULL);
	//�Ա��˿�
	CT_BOOL CompareCard(const CT_BYTE cbFirstCard[], CT_BYTE cbFirstCount, const CT_BYTE cbNextCard[], CT_BYTE cbNextCount, CT_BOOL bLastCard = false);

	//��������
	CT_BYTE SearchOutCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[], CT_BYTE cbTurnCardCount,
		tagSearchCardResult& SearchCardResult);
	//�����׳���
	CT_BYTE SearchHeadOutCard(CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagSearchCardResult& SearchCardResult, CT_BYTE& cbResultCount);
	//ͬ������
	CT_BYTE SearchSameCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCardCount,
		tagSearchCardResult& SearchCardResult);
	//��������
	CT_BYTE SearchLineCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbBlockCount, CT_BYTE cbLineCount,
		tagSearchCardResult& SearchCardResult);
	//������������(����һ���Ĵ�һ��)
	CT_BYTE SearchTakeCardType(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbSameCount, CT_BYTE cbTakeCardCount,
		tagSearchCardResult& SearchCardResult);
	//�����ɻ�
	CT_BYTE SearchThreeTwoLine(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard, CT_BYTE cbTakeCardCount, CT_BYTE cbLineCount, tagSearchCardResult& SearchCardResult);
	//�������
	CT_BYTE SearchMaxSingle(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, CT_BYTE cbReferCard);

	//������һ�����
	CT_VOID CopyFirstResult(tagSearchCardResult& SearchCardResult, CT_BYTE& cbResultCount, tagSearchCardResult TempSearchCardResult);
	//����������Ƹ���
	CT_BYTE GetLessLineCount(CT_BYTE cbBlockCount, CT_BYTE cbLineCount);

	//�������ɻ�
	CT_BYTE SearchMaxThreeLine(CT_BYTE cbCardData[], CT_BYTE cbCardCount, CT_BYTE cbResultCardData[]);

	//��������������һ����
	CT_BYTE SearchMaxOneCard(const CT_BYTE cbHandCardData[], CT_BYTE cbHandCardCount, const CT_BYTE cbTurnCardData[]);

	//�ڲ�����
public:
	//�����˿�
	CT_BYTE MakeCardData(CT_BYTE cbValueIndex, CT_BYTE cbColorIndex);
	//�����˿�
	CT_VOID AnalysebCardData(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagAnalyseResult & AnalyseResult);
	//�����ֲ�
	CT_VOID AnalysebDistributing(const CT_BYTE cbCardData[], CT_BYTE cbCardCount, tagDistributing & Distributing);

};

//////////////////////////////////////////////////////////////////////////

#endif		//GAME_LOGIC_HEAD_FILE


