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
#define KIND_ID						26										//��Ϸ I D
#define GAME_NAME					TEXT("������")							//��Ϸ����

//�������
#define GAME_PLAYER					3										//��Ϸ����

//////////////////////////////////////////////////////////////////////////////////

//��Ŀ����
#define MAX_COUNT					20										//�����Ŀ
#define FULL_COUNT					54										//ȫ����Ŀ
#define MAX_NO_CALL_BANKER			3										//���ֻ�����ֲ��е���


//�߼���Ŀ
#define NORMAL_COUNT				17										//������Ŀ
#define DISPATCH_COUNT				51										//�ɷ���Ŀ
#define GOOD_CARD_COUTN				38										//������Ŀ
#define MAX_CARD_VALUE				15										//������߼���ֵ
#define MAX_CARD_COLOR				4										//�����ɫ��ֵ

//��ֵ����
#define	MASK_COLOR					0xF0									//��ɫ����
#define	MASK_VALUE					0x0F									//��ֵ����

//�߼�����
#define CT_ERROR					0										//��������
#define CT_SINGLE					1										//��������
#define CT_DOUBLE_CARD				2										//��������
#define CT_THREE					3										//��������
#define CT_SINGLE_LINE				4										//��������
#define CT_DOUBLE_LINE				5										//��������
#define CT_THREE_LINE				6										//��������
#define CT_THREE_TAKE_ONE			7										//����һ��
#define CT_THREE_TAKE_TWO			8										//����һ��
#define CT_FOUR_TAKE_ONE			9										//�Ĵ�����
#define CT_FOUR_TAKE_TWO			10										//�Ĵ�����
#define CT_BOMB_CARD				11										//ը������
#define CT_MISSILE_CARD				12										//�������


//��������
#define BCT_GENERAL					0										//��ͨ����
#define BCT_FULSH					1										//˳������
#define BCT_STRAIGHT				2										//ͬ������
#define BCT_STRAIGHT_FULSH			3										//ͬ��˳����
#define BCT_SINGLE_MISSILE			4										//��������
#define BCT_DOUBLE_MISSILE			5										//��������
#define BCT_THREE					6										//��������

//����
#define SCORE_TYPE_LOSE				1										//��������(��)
#define SCORE_TYPE_WIN				2										//��������(Ӯ)


//////////////////////////////////////////////////////////////////////////////////
//������Ϣ
#define SC_GAMESCENE_FREE					2001							//���г�����Ϣ
#define SC_GAMESCENE_CALL					2002							//�е���״̬
#define SC_GAMESCENE_PLAY					2003							//��Ϸ�г�����Ϣ
#define SC_GAMESCENE_END					2004							//��Ϸ����
#define SC_GAMESCENE_DOUBLE					2005							//�ӱ�����

//״̬����

#define GAME_SCENE_FREE						100								//�ȴ���ʼ
#define GAME_SCENE_CALL						101								//�е���״̬
#define GAME_SCENE_PLAY						102								//��Ϸ����
#define GAME_SCENE_END						103								//��Ϸ����
#define GAME_SCENE_DOUBLE					104								//�û��ӱ�

//�е���
#define CB_NOT_CALL							0								//û�е���
#define CB_CALL_BENKER_1					1								//�е���1��
#define CB_CALL_BENKER_2					2								//�е���2��
#define CB_CALL_BENKER_3					3								//�е���3��
#define CB_NO_CALL_BENKER					4								//���е���

//�ӱ���Ϣ
#define CB_NOT_ADD_DOUBLE					0								//û�ӱ�
#define CB_ADD_DOUBLE						1								//�ӱ���־
#define CB_NO_ADD_DOUBLE					2								//���ӱ�

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

//�е���״̬
struct CMD_S_StatusCall
{
	//ʱ����Ϣ
	CT_BYTE							cbLeftTime;							//ʣ��ʱ��

	//��Ϸ��Ϣ
	CT_DOUBLE						dCellScore;							//��Ԫ����
	CT_WORD							wCurrentUser;						//��ǰ���
	CT_WORD							cbCallBankerInfo[GAME_PLAYER];		//�е�����Ϣ	
	CT_BYTE							cbHandCardData[NORMAL_COUNT];		//�����˿�
	CT_BYTE							cbTrustee[GAME_PLAYER];				//�Ƿ��й�
};

//�ӱ�״̬
struct CMD_S_StatusDouble
{
	//��Ϸ��Ϣ
	CT_BYTE							cbLeftTime;							//ʣ��ʱ��
	CT_BYTE							cbCallScore;						//�з�
	CT_DOUBLE						dCellScore;							//��Ԫ����
	CT_WORD							wBankerUser;						//ׯ���û�
	CT_WORD							wCurrTime;							//��ǰ����
	CT_BYTE							cbDoubleInfo[GAME_PLAYER];			//�ӱ���Ϣ
	CT_BYTE							cbBankerCard[3];					//��Ϸ����
	CT_BYTE							cbHandCardCount[GAME_PLAYER];		//�˿���Ŀ
	CT_BYTE							cbHandCardData[MAX_COUNT];			//�����˿�
	CT_BYTE							cbTrustee[GAME_PLAYER];				//�Ƿ��й�
};

//��Ϸ״̬
struct CMD_S_StatusPlay
{
	//ʱ����Ϣ
	CT_BYTE							cbLeftTime;								//ʣ��ʱ��
	//��Ϸ����
	CT_DOUBLE						dCellScore;								//��Ԫ����
	CT_DWORD						dwBankerUser;							//ׯ���û�
	CT_DWORD						dwCurrentUser;							//��ǰ���

	CT_BYTE							cbCallScore;							//�з�
	CT_DWORD						dwBombTime;								//ը������(����*2)
	CT_BYTE							cbAddDoubleInfo[GAME_PLAYER];			//�ӱ���Ϣ

	//������Ϣ
	CT_WORD							wTurnWiner;								//�������
	CT_BYTE							cbTurnCardCount;						//������Ŀ
	CT_BYTE							cbTurnCardData[MAX_COUNT];				//��������

	//�˿���Ϣ
	CT_BYTE							cbBankerCard[3];						//��Ϸ����
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
	CT_WORD						wCurrPlayCount;								//��ǰ����
	CT_BYTE						cbTimeLeave;								//ʣ��ʱ��
																			//�˿���Ϣ
	CT_BYTE						cbBankerCard[3];							//��Ϸ����
	CT_BYTE						cbHandCardCount[GAME_PLAYER];				//�˿���Ŀ
	CT_BYTE						cbHandCardData[MAX_COUNT];					//�����˿�
};

//////////////////////////////////////////////////////////////////////////////////
//�����

#define SUB_S_GAME_START			100										//��Ϸ��ʼ
#define SUB_S_CALL_BANKER			101										//�е���
#define SUB_S_BANKER_INFO			102										//ׯ����Ϣ
#define SUB_S_OUT_CARD				103										//�û�����
#define SUB_S_PASS_CARD				104										//�û�����
#define SUB_S_OUT_START_START		105										//��ʼ����
#define SUB_S_GAME_CONCLUDE			106										//��Ϸ����
#define SUB_S_CLEARING				107										//�ƾֽ���
#define SUB_S_TRUSTEE				108										//�й�
#define SUB_S_ADD_DOUBLE			109										//�ӱ�[CMD_S_Double]
#define SUB_S_CHEAT_LOOK_CARD		110										//����

//�����˿�
struct CMD_S_GameStart
{
	CT_DWORD						dwStartUser;							//��ʼ���
	CT_DWORD				 		dwCurrentUser;							//��ǰ���
	CT_DWORD						dwStartTime;							//��ʼ����
	CT_BYTE							cbCardData[NORMAL_COUNT];				//�˿��б�
	CT_BYTE							cbTimeLeft;								//ʣ��ʱ��
};

struct CMD_S_GameStartAi
{
	CT_DWORD						dwStartUser;							//��ʼ���
	CT_DWORD				 		dwCurrentUser;							//��ǰ���
	CT_DWORD						dwStartTime;							//��ʼ����
	CT_BYTE							cbCardData[GAME_PLAYER][NORMAL_COUNT];	//�˿��б�
	CT_BYTE							cbBankerCard[3];						//����
};

//�û��е���
struct CMD_S_CallBanker
{
	CT_DWORD				 		dwCurrentUser;							//��ǰ���
	CT_DWORD				 		dwBankerUser;							//ׯ�����
	CT_DWORD						dwLastUser;								//��һ���е������
	CT_WORD							cbCallInfo;								//�е�������
	CT_BYTE							cbTimeLeft;								//ʣ��ʱ��
};

//�ӱ���Ϣ
struct CMD_S_Double
{
	CT_DWORD						dwCurrentUser;						//��ǰ�û�
	CT_BYTE							cbDouble;							//�Ƿ�ӱ�(0 δ���� 1 �ӱ� 2 ���ӱ�)
	CT_WORD							wUserTimes;							//�û�����
};

//��ʼ������Ϣ
struct CMD_S_StartOutCard
{
	CT_DWORD				 		dwBankerUser;						//ׯ�����
	CT_DWORD				 		dwCurrentUser;						//��ǰ���
	CT_BYTE							cbLeftTime;							//ʣ��ʱ��
};

//ׯ����Ϣ
struct CMD_S_BankerInfo
{
	CT_DWORD				 		dwBankerUser;						//ׯ�����
	CT_DWORD				 		dwCurrentUser;						//��ǰ���
	CT_BYTE							cbBankerCard[3];					//ׯ���˿�
	CT_BYTE							cbTimeLeft;							//ʣ��ʱ��
};

//�û�����
struct CMD_S_OutCard
{
	CT_BYTE							cbTimeLeft;							//ʣ��ʱ��
	CT_BYTE							cbCardCount;						//������Ŀ
	CT_DWORD				 		dwCurrentUser;						//��ǰ���
	CT_DWORD						dwOutCardUser;						//�������
	CT_WORD							wUserTimes;							//�û�����
	CT_BYTE							cbCardData[MAX_COUNT];				//�˿��б�
};

//��������
struct CMD_S_PassCard
{
	CT_BYTE							cbTurnOver;							//һ�ֽ���
	CT_DWORD				 		dwCurrentUser;						//��ǰ���
	CT_DWORD				 		dwPassCardUser;						//�������
	CT_BYTE							cbTimeLeft;
	//CT_WORD							wCurrPlayCount;						//��ǰ����
};

//���׿���
struct CMD_S_CheatLookCard
{
	CT_WORD							wCardUser;							//�����û�
	CT_BYTE							cbCardCount;						//������Ŀ
	CT_BYTE							cbCardData[MAX_COUNT];				//�˿��б�
};

//��Ϸ����
struct DDZ_CMD_S_GameEnd
{
	//���ֱ���
	//CT_DOUBLE						dCellScore;							//��Ԫ����
	CT_BYTE							cbCallScore;						//�з�
	CT_DOUBLE						dGameScore[GAME_PLAYER];			//��Ϸ����
	CT_BYTE							bLose[GAME_PLAYER];					//ʤ����־							

	//�����־
	CT_BYTE							bChunTian;							//�����־
	CT_BYTE							bFanChunTian;						//�����־
	CT_BYTE							bWinMaxScore;						//�Ƿ�ﵽӮǮ�ⶥ

	//ը����Ϣ
	CT_WORD							cbBombTimes;						//ը������(���������)
	CT_BYTE							cbMissileTimes;						//�������
	CT_BYTE							cbDoubleInfo[GAME_PLAYER];			//�ӱ���Ϣ

	//��Ϸ��Ϣ
	CT_BYTE							cbCardCount[GAME_PLAYER];				//�˿���Ŀ
	CT_BYTE							cbHandCardData[GAME_PLAYER][MAX_COUNT];	//�˿��б�
};

//�ƾֽ���
struct DDZ_CMD_S_CLEARING
{
	CT_LONGLONG					llTotalLWScore[GAME_PLAYER];			//���������Ӯ��d
	CT_WORD						wRoomOwner;								//����
	//CT_BYTE						cbQuanGuanCount[GAME_PLAYER];		//�����ȫ�ش���
	//CT_BYTE						cbBeiGuanCount[GAME_PLAYER];		//����ұ��ش���
	CT_BYTE						cbRombCount[GAME_PLAYER];				//�����ը������
	//CT_BYTE						cbTongPeiCount[GAME_PLAYER];		//�����ͨ�����
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

#define SUB_C_CALL_BANKER			1									//�û��е���
#define SUB_C_OUT_CARD				2									//�û�����
#define SUB_C_PASS_CARD				3									//�û�����
#define SUB_C_TRUSTEE				4									//�û��й�
#define SUB_C_CANCEL_TRUSTEE		5									//�û�ȡ���й�
#define SUB_C_ADD_DOUBLE			6									//�û��ӱ�[CMD_C_Double]
#define SUB_C_CHEAT_LOOK_CARD		7									//���׿���	

//�û�����
struct CMD_C_CallBanker : public MSG_GameMsgUpHead
{
	CT_WORD							cbCallInfo;							//�е���
};

//�û��ӱ�
struct CMD_C_Double : public MSG_GameMsgUpHead
{
	CT_BYTE							cbDoubleInfo;						//�ӱ���Ϣ(1���ӱ� 2�����ӱ�)
};

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

