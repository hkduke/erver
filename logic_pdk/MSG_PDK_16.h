#ifndef		___MESSAGE_PDK_16_H_DEF___
#define		___MESSAGE_PDK_16_H_DEF___

#include "CGlobalData.h"
#include <string.h>

#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

////////////////////////////////////////////////////////////////////////

#define GAME_PLAYER							3								//��Ϸ����

//��ֵ����
#define MAX_COUNT							16								//�����������

//////////////////////////////////////////////////////////////////////////
//��Ϸ״̬
#define GAME_STATUS_FREE					100								//����״̬
#define GAME_STATUS_PLAY					102								//��Ϸ��״̬
#define GAME_STATUS_END						103								//��Ϸ����


//������Ϣ
#define SC_GAMESCENE_FREE					2001							//���г�����Ϣ
#define SC_GAMESCENE_PLAY					2002							//��Ϸ�г�����Ϣ
#define SC_GAMESCENE_END					2003							//��Ϸ����

//����״̬
struct FPF_MSG_GS_FREE
{
	CT_DWORD					dwCellScore;								//��������
	CT_WORD						wCurrPlayCount;								//��ǰ����
};

//��Ϸ��״̬
struct FPF_MSG_GS_PLAY 
{
	CT_DWORD					dwCellScore;								//��������
	CT_WORD						wCurrPlayCount;								//��ǰ����
	CT_BYTE						cbTimeLeave;								//ʣ��ʱ��
	CT_DWORD					dwCurrentUser;								//��ǰ�û�(�������)
	CT_BYTE						cbIsPass;									//�Ƿ񲻳�(1�ǲ���)
	CT_BYTE						cbMustThree;								//�س�����3
	CT_BYTE						cbCardCount[GAME_PLAYER];					//��������
	CT_BYTE						cbCardData[MAX_COUNT];						//��������

	CT_DWORD					dwTurnWiner;								//ʤ�����
	CT_BYTE						cbTurnCardCount;							//������Ŀ
	CT_BYTE						cbTurnCardData[MAX_COUNT];					//��������
	CT_BYTE						cbSingle[GAME_PLAYER];						//�Ƿ񱨵�
	CT_BYTE						cbRombCount[GAME_PLAYER];					//ը������
};

//��Ϸ����
struct FPF_MSG_GS_END
{
	CT_DWORD					dwCellScore;							    //��������
	CT_WORD						wCurrPlayCount;								//��ǰ����
	CT_BYTE						cbTimeLeave;								//ʣ��ʱ��
	CT_BYTE						cbRemainCount[GAME_PLAYER];					//ʣ��������
	CT_BYTE						cbRemainCard[GAME_PLAYER][MAX_COUNT];		//ʣ����
};

//////////////////////////////////////////////////////////////////////////
//����������ṹ
#define PDK_16_SUB_S_SEND_CARD						100						//��Ϸ��ʼ[PDK_16_CMD_S_SendCard]
#define PDK_16_SUB_S_NOTIFY_OUT_CARD				101						//֪ͨ����[PDK_16_CMD_S_NotifyOutCard]
#define PDK_16_SUB_S_OUT_CARD_RESULT				102						//���ƽ��[PDK_16_CMD_S_OutCardResult]
#define PDK_16_SUB_S_GAME_END						103						//��Ϸ����[PDK_16_CMD_S_GameEnd]

#define PDK_16_SUB_S_CLEARING						200						//�ƾֽ���[PDK_16_CMD_S_CLEARING]


//��������
struct PDK_16_CMD_S_SendCard
{
	CT_DWORD					dwFirstUser;								//�׳������	
	CT_WORD						wCurrPlayCount;								//��ǰ����
	CT_BYTE						cbMustThree;								//�س�����3
	CT_BYTE						cbCardCount;								//��������
	CT_BYTE						cbCardData[MAX_COUNT];						//��������
	CT_BYTE						cbOutCardTime;								//����ʱ��
};

//��ʾ����
struct PDK_16_CMD_S_NotifyOutCard
{
	CT_DWORD					dwOutCardUser;								//�������
	CT_BYTE						cbOutCardTime;								//����ʱ��
	CT_BYTE						cbOutCardFlag;								//���Ʊ�־0:���ܳ���1:�ܳ�
	CT_BYTE						cbNewTurn;									//��һ�ֱ�־1:��һ��
	CT_BYTE						cbMustThree;								//�س�����3
};

//���ƽ��
struct PDK_16_CMD_S_OutCardResult
{
	CT_DWORD					dwOutCardUser;								//�������
	CT_BYTE						cbCardCount;								//������Ŀ
	CT_BYTE						cbCardData[MAX_COUNT];						//�˿��б�
	CT_BYTE						cbSingle;									//�Ƿ񱨵�
	

	CT_DWORD					dwCurrentUser;								//��ǰ�������
	CT_BYTE						cbIsPass;									//�Ƿ����(1�ǲ���)
	CT_BYTE						cbOutCardTime;								//����ʱ��
	CT_BYTE						cbNewTurn;									//��һ�ֱ�־1:��һ��
};

//��Ϸ����
struct PDK_16_CMD_S_GameEnd
{
	CT_LONGLONG					llLWScore[GAME_PLAYER];						//��Ӯ����(�׷�*����)
	CT_LONGLONG					llRombScore[GAME_PLAYER];					//ը������
	CT_INT32					iLWNum[GAME_PLAYER];						//��Ӯ����(����)
	CT_WORD						wWinUser;									//Ӯ��
	CT_BYTE						cbBeiGuan[GAME_PLAYER];						//���ر�־
	CT_BYTE						cbRemainCount[GAME_PLAYER];					//ʣ��������
	CT_BYTE						cbRemainCard[GAME_PLAYER][MAX_COUNT];		//ʣ����
	CT_BYTE						cbRombCount[GAME_PLAYER];					//ը������
};

//�ƾֽ���
struct PDK_16_CMD_S_CLEARING
{
	CT_LONGLONG					llTotalLWScore[GAME_PLAYER];				//���������Ӯ��d
	CT_WORD						wRoomOwner;									//����
	CT_BYTE						cbQuanGuanCount[GAME_PLAYER];				//�����ȫ�ش���
	CT_BYTE						cbBeiGuanCount[GAME_PLAYER];				//����ұ��ش���
	CT_BYTE						cbRombCount[GAME_PLAYER];					//�����ը������
	CT_BYTE						cbTongPeiCount[GAME_PLAYER];				//�����ͨ�����
	CT_CHAR						szCurTime[TIME_LEN];						//��ǰʱ��
};


//////////////////////////////////////////////////////////////////////////
//�ͻ������������˷���Ϣ(C->S)
#define PDK_16_SUB_C_OUT_CARD			1									//������Ϣ
#define PKD_16_SUB_C_PASS_CARD			2									//������

//��������
struct PDK_16_CMD_C_OutCard : public MSG_GameMsgUpHead
{
	CT_BYTE						cbCardCount;								//������Ŀ
	CT_BYTE						cbCardData[MAX_COUNT];						//�˿��б�
};

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

