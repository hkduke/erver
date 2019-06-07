#ifndef		___MESSAGE_DDZ_H_DEF___
#define		___MESSAGE_DDZ_H_DEF___

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////////////
//������
//////////////////////////////////////////////////////////////////////////////////
//��Ϸ����
#define KIND_ID						1										//��Ϸ I D
#define GAME_NAME					TEXT("�ܵÿ�")							//��Ϸ����

//�������
#define GAME_PLAYER					3										//��Ϸ����

//////////////////////////////////////////////////////////////////////////////////

//��Ŀ����
#define MAX_COUNT					16										//�����Ŀ
#define FULL_COUNT					48										//ȫ����Ŀ


//�߼���Ŀ
#define NORMAL_COUNT				16										//������Ŀ
#define DISPATCH_COUNT				48										//�ɷ���Ŀ
#define GOOD_CARD_COUTN				24										//������Ŀ
#define GOOD_CARD_MAX				10										//ÿ����������Ŀ
#define MAX_CARD_VALUE				15										//������߼���ֵ
#define MAX_CARD_COLOR				4										//�����ɫ��ֵ

//��ֵ����
#define	MASK_COLOR					0xF0									//��ɫ����
#define	MASK_VALUE					0x0F									//��ֵ����

//�߼�����
#define CT_ERROR					0									//��������
#define CT_SINGLE					1									//��������
#define CT_DOUBLE_Ex				2									//��������
#define CT_THREE_EX					3									//����
#define CT_THREE_TAKE_ONE			4									//����һ
#define CT_THREE_TAKE_TWO			5									//������
#define CT_SINGLE_LINE				6									//��������
#define CT_DOUBLE_LINE				7									//��������
#define CT_THREE_LINE				8									//����
#define CT_THREE_LINE_ONE			9									//������һ
#define CT_AIRPLANE					10									//�������� (�ɻ�)
#define CT_FOUR_TAKE_ONE			11									//4��1
#define CT_FOUR_TAKE_TWO			12									//4��2
#define CT_FOUR_TAKE_THREE			13									//4��3
#define CT_FOUR_LINE				14									//4��
#define CT_FOUR_LINE_TWO			15									//4����2
#define CT_FOUR_LINE_THREE			16									//4����3
#define CT_BOMB_CARD				17									//ը������

#define TIME_DEAL_CARD				2					                //����ʱ��(2)
#define	TIME_OUT_CARD				12					                //����ʱ��(15)
#define	TIME_NOT_OUT_CARD			(1)					                //Ҫ������ҵĹ���ʱ��(15)
#define TIME_FIRST_OUT_CARD			23					                //��һ�γ���ʱ��

//����
#define SCORE_TYPE_LOSE				1										//��������(��)
#define SCORE_TYPE_WIN				2										//��������(Ӯ)


//////////////////////////////////////////////////////////////////////////////////
//������Ϣ
#define SC_GAMESCENE_FREE					2001							//���г�����Ϣ
#define SC_GAMESCENE_PLAY					2003							//��Ϸ�г�����Ϣ
#define SC_GAMESCENE_END					2004							//��Ϸ����

//״̬����

#define GAME_SCENE_FREE						100								//�ȴ���ʼ
#define GAME_SCENE_PLAY						102								//��Ϸ����
#define GAME_SCENE_END						103								//��Ϸ����


//ʤ����Ϣ
#define GAME_WIN							0								//ʤ
#define GAME_LOSE							1								//��

//����״̬
struct CMD_S_StatusFree
{
	//��Ϸ����
	CT_DOUBLE					dCellScore;							//��������
	CT_WORD						dwCurrPlayCount;

};


//��Ϸ״̬
struct CMD_S_StatusPlay
{
    CT_WORD						    wFirstOutChairID;					    //�׳����
	//ʱ����Ϣ
	CT_BYTE							cbLeftTime;								//ʣ��ʱ��
	//��Ϸ����
	CT_DOUBLE						dCellScore;								//��Ԫ����
	CT_DWORD						dwCurrentUser;							//��ǰ���

	//������Ϣ
	CT_WORD							wTurnWiner;								//�������
	CT_BYTE							cbTurnCardCount;						//������Ŀ
	CT_BYTE							cbTurnCardData[MAX_COUNT];				//��������

	//�˿���Ϣ
	CT_BYTE							cbHandCardCount[GAME_PLAYER];			//�˿���Ŀ
	CT_BYTE							cbHandCardData[MAX_COUNT];				//�����˿�
	CT_BYTE							cbOutCount[GAME_PLAYER];				//���ƴ���
	CT_BYTE							cbOutCardListCount[GAME_PLAYER][MAX_COUNT];			//ÿ�γ�������
	CT_BYTE							cbOutCardList[GAME_PLAYER][MAX_COUNT][MAX_COUNT];	//�����б�
	//CT_BYTE							cbRemainCard[MAX_CARD_VALUE];			//ʣ��������

	CT_BYTE							cbTrustee[GAME_PLAYER];					//�Ƿ��й�
};

//��Ϸ����
struct CMD_S_StatusEND
{
	CT_DOUBLE					dCellScore;									//��������
	CT_BYTE						cbTimeLeave;								//ʣ��ʱ��
																			//�˿���Ϣ
	CT_BYTE						cbHandCardCount[GAME_PLAYER];				//�˿���Ŀ
	CT_BYTE						cbHandCardData[MAX_COUNT];					//�����˿�
};

//////////////////////////////////////////////////////////////////////////////////
//�����

#define SUB_S_GAME_START			100										//��Ϸ��ʼ
#define SUB_S_OUT_CARD				103										//�û�����
#define SUB_S_PASS_CARD				104										//�û�����
#define SUB_S_OUT_START_START		105										//��ʼ����
#define SUB_S_GAME_CONCLUDE			106										//��Ϸ����
#define SUB_S_CLEARING				107										//�ƾֽ���
#define SUB_S_TRUSTEE				108										//�й�
#define SUB_S_CHEAT_LOOK_CARD		110										//����
#define SUB_S_BOMB_SCORE			111										//ը����
//�����˿�
struct CMD_S_GameStart
{
	CT_DWORD						dwStartUser;							//�ط���λ��
	CT_DWORD				 		dwCurrentUser;							//��ǰ���
	CT_BYTE							cbCardData[NORMAL_COUNT];				//�˿��б�
	CT_BYTE							cbTimeLeft;								//ʣ��ʱ��
};

struct CMD_S_GameStartAi
{
	CT_DWORD				 		dwCurrentUser;							//��ǰ���
	CT_BYTE							cbCardData[GAME_PLAYER][NORMAL_COUNT];	//�˿��б�
	CT_BYTE							cbBankerCard[3];						//����
    CT_BYTE							cbTimeLeft;								//ʣ��ʱ��
};

//��ʼ������Ϣ
struct CMD_S_StartOutCard
{
	CT_DWORD				 		dwCurrentUser;						//��ǰ���
	CT_BYTE							cbLeftTime;							//ʣ��ʱ��
};


//�û�����
struct CMD_S_OutCard
{
	CT_BYTE							cbIsFail;							//һ�ֽ���,0�ɹ���1�ͻ��˷�����������0��2����������������������3�˿����ݿգ�4���ǵ�ǰ��ң�5���ʹ���6���Ͳ�ƥ�䣬7������ɾ���˿˴���
	CT_BYTE							cbTimeLeft;							//ʣ��ʱ��
	CT_BYTE							cbCardCount;						//������Ŀ
	CT_DWORD				 		dwCurrentUser;						//��ǰ���
	CT_DWORD						dwOutCardUser;						//�������
	CT_BYTE							cbCardData[MAX_COUNT];				//�˿��б�
};

//��������
struct CMD_S_PassCard
{
	CT_BYTE							cbIsFail;							//һ�ֽ���,0�ɹ���1ʧ��
	CT_BYTE							cbTurnOver;							//һ�ֽ���
	CT_DWORD				 		dwCurrentUser;						//��ǰ���
	CT_DWORD				 		dwPassCardUser;						//�������
	CT_BYTE							cbTimeLeft;
};
//ը����
struct CMD_S_BombScore
{
	CT_DOUBLE						dBombScore[GAME_PLAYER];			//��Ϸ����
};

//���׿���
struct CMD_S_CheatLookCard
{
	CT_WORD							wCardUser;							//�����û�
	CT_BYTE							cbCardCount;						//������Ŀ
	CT_BYTE							cbCardData[MAX_COUNT];				//�˿��б�
};

//��Ϸ����
struct PDK_CMD_S_GameEnd
{
	//���ֱ���
	CT_DOUBLE						dCellScore;							//��Ԫ����
	CT_DOUBLE						dGameScore[GAME_PLAYER];			//��Ϸ����
	CT_BYTE							bLose[GAME_PLAYER];					//ʤ����־
	//�����־
	CT_BYTE							bChunTian[GAME_PLAYER];				//�ر�־��1ȫ�أ�2����
	CT_BYTE							bWinMaxScore;						//�Ƿ�ﵽӮǮ�ⶥ


	//��Ϸ��Ϣ
	CT_BYTE							cbCardCount[GAME_PLAYER];				//�˿���Ŀ
	CT_BYTE							cbHandCardData[GAME_PLAYER][MAX_COUNT];	//�˿��б�

	CT_BYTE							cbEachBombCount[GAME_PLAYER];		//ը������ss
	CT_DOUBLE						dEachBombScore[GAME_PLAYER];		//ը������
};

//�ƾֽ���
struct PDK_CMD_S_CLEARING
{
	CT_LONGLONG					llTotalLWScore[GAME_PLAYER];			//���������Ӯ��d
	CT_WORD						wRoomOwner;								//����
	//CT_BYTE					cbQuanGuanCount[GAME_PLAYER];		//�����ȫ�ش���
	//CT_BYTE					cbBeiGuanCount[GAME_PLAYER];		//����ұ��ش���
	CT_BYTE						cbRombCount[GAME_PLAYER];				//�����ը������
	//CT_BYTE					cbTongPeiCount[GAME_PLAYER];		//�����ͨ�����
	CT_CHAR						szCurTime[TIME_LEN];					//��ǰʱ��
	CT_WORD						wCurrPlayCount;							//��ǰ����
	CT_WORD						wPlayWinCount[GAME_PLAYER];				//Ӯ����
	CT_WORD						wPlayLoseCount[GAME_PLAYER];			//�����
};

//������Ϣ
struct tagScoreInfo
{
	CT_INT32						iScore;								//����
	CT_BYTE							cbType;								//����
};

//��Ϸ�й�
struct CMD_S_StatusTrustee
{
	CT_BYTE						cbTrustee[GAME_PLAYER];					//�й���Ϣ
};
//////////////////////////////////////////////////////////////////////////////////
//�����

#define SUB_C_OUT_CARD				2									//�û�����
#define SUB_C_PASS_CARD				3									//�û�����
#define SUB_C_TRUSTEE				4									//�û��й�
#define SUB_C_CANCEL_TRUSTEE		5									//�û�ȡ���й�
#define SUB_C_CHEAT_LOOK_CARD		7									//���׿���	


//�û�����
struct CMD_C_OutCard : public MSG_GameMsgUpHead
{
	CT_BYTE							cbCardCount;						//������Ŀ
	CT_BYTE							cbCardData[MAX_COUNT];				//�˿�����
};

//�û��й�
struct CMD_C_Trustee : public MSG_GameMsgUpHead
{
	CT_WORD							wChairID;							//�й����id
};

//�û�ȡ���й�
struct CMD_C_CancelTrustee : public MSG_GameMsgUpHead
{
	CT_WORD							wChairID;							//ȡ���й����id
};


//�û�����
struct CMD_C_CheatLookCard : public MSG_GameMsgUpHead
{
	CT_WORD							wBeCheatChairID;					//��͵���������ID
};
//////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

