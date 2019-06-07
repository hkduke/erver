#ifndef		___MESSAGE_TEXAS_H_DEF___
#define		___MESSAGE_TEXAS_H_DEF___

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
#define GAME_NAME					TEXT("�����˿�")							//��Ϸ����

//�������
#define GAME_PLAYER					9										//��Ϸ����

//////////////////////////////////////////////////////////////////////////////////

enum CardType
{
	//�˿�����
	CT_SINGLE					= 1,								//��������
	CT_ONE_LONG					= 2,								//��������
	CT_TWO_LONG					= 3,								//��������
	CT_THREE_TIAO				= 4,								//��������
	CT_SHUN_ZI					= 5,								//˳������
	CT_TONG_HUA					= 6,								//ͬ������
	CT_HU_LU					= 7,								//��«����
	CT_TIE_ZHI					= 8,								//��֧����
	CT_TONG_HUA_SHUN			= 9,								//ͬ��˳��
	CT_KING_TONG_HUA_SHUN		= 10								//�ʼ�ͬ��˳
};
//����������
enum RobotCardType
{
	//����Ȧ���� HC��������
	RCT_OTHER					= 20,								//��������
	RCT_SINGLE_HC_2ToJ			= 21,								//����(����2-J)
	RCT_SINGLE_HC_QToA			= 22,								//����(����Q-A)
	RCT_ONE_PAIR_1HC			= 23,								//һ��(1������)
	RCT_ONE_PAIR_2HC			= 24,								//һ��(2������)
	RCT_TWO_PAIR_HC_PAIR		= 25,								//����(�ֶ�)
	RCT_TWO_PAIR_1HC			= 26,								//����(1������)
	RCT_TWO_PAIR_2HC			= 27,								//����(2������ ���ֶ�)
	RCT_THREE_KIND_1HC			= 28,								//����(1������)
	RCT_STRAIGHT_1HC			= 29,								//˳��(1������)
	RCT_FLUSH_1HC_2TO7			= 30,								//ͬ��(1������2-7)
	RCT_FLUSH_1HC_8TOT			= 31,								//ͬ��(1������8-T)
	RCT_THREE_KIND_2HC			= 32,								//����(2������)
	RCT_STRAIGHT_2HC			= 33,								//˳��(2������)
	RCT_FLUSH_1HC_JTOA			= 34,								//ͬ��(1������J-A)
	RCT_FLUSH_2HC_2TO7			= 35,								//ͬ��(2������2-7)
	RCT_FLUSH_2HC_8TOT			= 36,								//ͬ��(2������8-T)
	RCT_FLUSH_2HC_JTOA			= 37,								//ͬ��(2������J-A)
	RCT_FULL_HOUSE				= 38,								//��«
	RCT_FOUR_KIND				= 39,								//���(����)


	//����Ȧ���� -- R0
	RCT_R0_OTHER				= 67,								//��������
	RCT_R0_7_9ToT				= 68,								//7(9-T)
	RCT_R0_6_8ToT				= 69,								//6(8-T)
	RCT_R0_5_7ToT				= 70,								//5(7-T)
	RCT_R0_4_6ToT				= 71,								//4(6-T)
	RCT_R0_3_5ToT				= 72,								//3(5-T)
	RCT_R0_2_4ToT				= 73,								//2(4-T)
	RCT_R0_A_5ToT				= 74,								//��ͬ��A(5-T)
	RCT_R0_J_8ToT				= 75,								//��ͬ��J(8-T)
	RCT_R0_Q_9ToJ				= 76,								//��ͬ��Q(9-J)
	RCT_R0_K_TToQ				= 77,								//��ͬ��K(T-Q)
	RCT_R0_A_2To4				= 78,								//��ͬ��A(2-4)
	RCT_R0_A_JToK				= 79,								//��ͬ��A(J-K)
	RCT_R0_S_J_2To7				= 80,								//ͬ��J(2-7)
	RCT_R0_S_Q_2To8				= 81,								//ͬ��Q(2-8)
	RCT_R0_S_K_2To9				= 82,								//ͬ��K(2-9)
	RCT_R0_S_A_5ToT				= 83,								//ͬ��A(5-T)
	RCT_R0_S_3_2				= 84,								//ͬ��32
	RCT_R0_S_4_3				= 85,								//ͬ��43
	RCT_R0_S_5_4				= 86,								//ͬ��54
	RCT_R0_S_6_5				= 87,								//ͬ��65
	RCT_R0_S_7_6				= 88,								//ͬ��76
	RCT_R0_S_8_7				= 89,								//ͬ��87
	RCT_R0_S_9_8				= 90,								//ͬ��98
	RCT_R0_S_T_9				= 91,								//ͬ��T9
	RCT_R0_S_J_8ToT				= 92,								//ͬ��J(8-T)
	RCT_R0_S_Q_9ToJ				= 93,								//ͬ��Q(9-J)
	RCT_R0_S_K_TToQ				= 94,								//ͬ��K(T-Q)
	RCT_R0_S_A_2To4				= 95,								//ͬ��A(2-4)
	RCT_R0_S_A_JToK				= 96,								//ͬ��A(J-K)
	RCT_R0_22To55				= 97,								//22-55
	RCT_R0_66ToTT				= 98,								//66-TT
	RCT_R0_JJToKK				= 99,								//JJ?KK
	RCT_R0_AA					= 100,								//AA
};
enum TableStatus
{
	TABLE_STATUS_READY = 1,				//׼��״̬�����ӵĳ�ʼ״̬
	TABLE_STATUS_START = 2,				//��Ϸ��ʼ״̬��Ҳָ�շ���״̬
	TABLE_STATUS_BET = 3,				//��ע״̬
	TABLE_STATUS_END = 4,				//��Ϸ����״̬
};
enum UserOperate
{
	USE_OPERATE_FLOD = 1,				//����
	USE_OPERATE_CHECK = 2,				//����
	USE_OPERATE_CALL = 3,				//��ע
	USE_OPERATE_BET = 4,				//��ע
	USE_OPERATE_ALLIN = 5,				//ȫ��
};
enum UserPreOperate
{
	USE_PRE_OPERATE_CANCEL = 0,				//ȡ��Ԥ����
	USE_PRE_OPERATE_FLOD_CHECK = 1,			//���ƹ���
	USE_PRE_OPERATE_CHECK = 2,				//����
	USE_PRE_PERATE_CALL = 3,				//���κ�ע
};
enum UserSitResult
{
	SIT_RESULT_SUCCESS = 0,				//�ɹ�
	SIT_RESULT_CHIP_INSUFFICIENT = 1,	//���벻������ʧ��
	SIT_RESULT_HAS_OTHER = 2,			//��λ����������ʧ��
	SIT_RESULT_ALREADY_SIT = 3,			//���ܻ�λ��
	SIT_RESULT_MONEY_INSUFFICIENT = 4,	//�����Ǯ֮�Ͳ���
};



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



//////////////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif


#endif		//___MESSAGE_H_DEF___

