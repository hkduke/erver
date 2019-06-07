
#include "stdafx.h"
#include <stdio.h>
#include "GameProcess.h"
#include "ITableFrame.h"
#include <time.h>
#include <vector>
#include <algorithm>
#include <math.h>
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "GlobalEnum.h"

//��ӡ��־
#define		PRINT_LOG_INFO						1
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//#define IDI_CHECK_TABLE			100					//�������
//#define IDI_CHECK_TRUSTEE			101					//�����й�

//#define IDI_AUTO_READY				200					//�Զ�׼��
#define IDI_CALL_BANKER				300					//�е���
#define IDI_ADD_DOUBLE				400					//����
#define IDI_OUT_CARD				500					//����
#define IDI_TICK_USER				600					//�޳��û�
#define IDI_CHECK_END				700					//�����Ϸ����

//#define	TIME_AUTO_READY				16					//�Զ�׼��ʱ��(15)
#define TIME_DEAL_CARD				2					//����ʱ��(2)
#define TIME_CALL_BANKER			15					//�е���ʱ��(15)
#define TIME_ADD_DOUBLE				10					//����ʱ��(5)
#define	TIME_OUT_CARD				15					//����ʱ��(15)
#define TIME_FIRST_OUT_CARD			25					//��һ�γ���ʱ��
#define TIME_TICK_USER				1					//�޳��û�ʱ��
#define TIME_CHECK_END				600					//������ӽ���

//���粹��ʱ��
#define TIME_NETWORK_COMPENSATE     2					//���粹��

//��ֵ����
#define SCORE_TIME_BOMB				2					//ը������
#define SCORE_TIME_CHUANTIAN		2					//���챶��
#define SCORE_TIME_FANCHUAN			2					//��������

#define TIME_OUT_TRUSTEE_COUNT		1					//��ʱ�����йܴ���

////////////////////////////////////////////////////////////////////////
std::map<CT_DWORD, tagBlackChannel>CGameProcess::m_mapChannelControl;
CGameProcess::CGameProcess(void) 
	: m_cbNoBankCount(1), m_dwlCellScore(1), m_pGameDeskPtr(NULL), m_pGameRoomKindInfo(NULL)
{
	m_dwFirstUser = INVALID_CHAIR;
	//������Ϸ����
	ClearGameData();
	//��ʼ������
	InitGameData();

	//CT_BYTE cbRandCard[FULL_COUNT];
	//std::vector<CT_BYTE> vecCard;
	//for (int i = 0; i < 1000000; ++i)
	//{
	//	m_GameLogic.RandCardList(cbRandCard, CountArray(cbRandCard), 0);


	//	for (int j = 0; j < FULL_COUNT; j++)
	//	{
	//		auto it = std::find(vecCard.begin(), vecCard.end(), cbRandCard[j]);
	//		if (it != vecCard.end())
	//		{
	//			LOG(ERROR) << "fuck, i want to fuck you.";
	//		}
	//		vecCard.push_back(cbRandCard[j]);
	//	}

	//	vecCard.clear();
	//}

	//CT_BYTE cbCardData[20] = { 0x3a, 0x3a, 0x2a, 0x2a,0x1a };

	//CT_BYTE cbType = m_GameLogic.GetCardType(cbCardData, 5);
	//if (cbType == CT_ERROR)
	//{
	//	LOG(WARNING) << "card type error";
	//}

}

CGameProcess::~CGameProcess(void)
{
}

//��Ϸ��ʼ
void CGameProcess::OnEventGameStart()
{
	//assert(0 != m_pGameDeskPtr);
	//��ʼ������
	InitGameData();
	//����˽�˳�����
	//ParsePrivateData(m_PrivateTableInfo.szOtherParam);
	//������Ϸ����
	//UpdateGameConfig();
	//������ж�ʱ��
	ClearAllTimer();
	//������Ϸ״̬
	m_cbGameStatus = GAME_SCENE_CALL;
	m_pGameDeskPtr->SetGameTimer(IDI_CHECK_END, TIME_CHECK_END*1000);

	//�����˿�m_cbBombCount
	CT_BYTE cbRandCard[FULL_COUNT];
	if (m_pGameRoomKindInfo->wRoomKindID == PRIMARY_ROOM)//��������%70���ʳ���ը��
	{
		/*CT_DWORD dwFirstUserID = m_pGameDeskPtr->GetUserID(0);
		int nRand = rand() % 10;
		CT_BYTE cbBomCount = 0;
		if (nRand < 7)
		{
			cbBomCount = 1;
		}*/

		m_GameLogic.RandCardList(cbRandCard, CountArray(cbRandCard), 0);
	
	}
	else
	{
		m_GameLogic.RandCardList(cbRandCard, CountArray(cbRandCard), 0);
	}

	//����ƹ���
    if (PRIMARY_ROOM == m_pGameRoomKindInfo->wRoomKindID)
    {
        MatchGoodCard(cbRandCard, CountArray(cbRandCard));
    }

    /*
	cbRandCard[0] = 0x22;
    cbRandCard[1] = 0x12;
    cbRandCard[2] = 0x31;
    cbRandCard[3] = 0x3d;
    cbRandCard[4] = 0x2d;
    cbRandCard[5] = 0x1d;
    cbRandCard[6] = 0x2c;
    cbRandCard[7] = 0x3a;
    cbRandCard[8] = 0x2a;
    cbRandCard[9] = 0x1a;
    cbRandCard[10] = 0x0a;
    cbRandCard[11] = 0x39;
    cbRandCard[12] = 0x28;
    cbRandCard[13] = 0x37;
    cbRandCard[14] = 0x17;
    cbRandCard[15] = 0x06;
    cbRandCard[16] = 0x13;

    cbRandCard[17] = 0x21;
    cbRandCard[18] = 0x01;
    cbRandCard[19] = 0x3c;
    cbRandCard[20] = 0x1c;
    cbRandCard[21] = 0x2b;
    cbRandCard[22] = 0x0b;
    cbRandCard[23] = 0x1b;
    cbRandCard[24] = 0x09;
    cbRandCard[25] = 0x08;
    cbRandCard[26] = 0x07;
    cbRandCard[27] = 0x36;
    cbRandCard[28] = 0x26;
    cbRandCard[29] = 0x35;
    cbRandCard[30] = 0x25;
    cbRandCard[31] = 0x24;
    cbRandCard[32] = 0x14;
    cbRandCard[33] = 0x04;

    cbRandCard[34] = 0x4e;
    cbRandCard[35] = 0x4f;
    cbRandCard[36] = 0x32;
    cbRandCard[37] = 0x02;
    cbRandCard[38] = 0x0d;
    cbRandCard[39] = 0x0c;
    cbRandCard[40] = 0x3b;
    cbRandCard[41] = 0x19;
    cbRandCard[42] = 0x38;
    cbRandCard[43] = 0x27;
    cbRandCard[44] = 0x16;
    cbRandCard[45] = 0x15;
    cbRandCard[46] = 0x05;
    cbRandCard[47] = 0x34;
    cbRandCard[48] = 0x33;
    cbRandCard[49] = 0x23;
    cbRandCard[50] = 0x03;

    cbRandCard[51] = 0x29;
    cbRandCard[52] = 0x11;
    cbRandCard[53] = 0x18;
	*/
	//if (m_dwFirstUser == INVALID_CHAIR)
	{
		//�����û�
		//m_dwFirstUser = m_pGameDeskPtr->GetPRoomOwnerChairID();
		//if (m_dwFirstUser == INVALID_CHAIR)
		//{
			m_dwFirstUser = rand() % GAME_PLAYER;
		//}	
	}
	m_dwCurrentUser = m_dwFirstUser;

	//���õ���
	memcpy(m_cbBankerCard, &cbRandCard[DISPATCH_COUNT], sizeof(m_cbBankerCard));

	//�û��˿�
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
		//CT_DWORD wUserIndex = (m_dwFirstUser + i) % GAME_PLAYER;
		m_cbHandCardCount[i] = NORMAL_COUNT;
		memcpy(&m_cbHandCardData[i], &cbRandCard[i*m_cbHandCardCount[i]], sizeof(CT_BYTE)*m_cbHandCardCount[i]);
	}

	//�������
	CMD_S_GameStart GameStart;
	GameStart.dwCurrentUser = m_dwCurrentUser;
	GameStart.dwStartTime = m_dwStartTime;
	GameStart.cbTimeLeft = TIME_CALL_BANKER;
	//GameStart.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();
	//=================test=begin=================

	
	//0	
	/*m_cbHandCardData[0][0]	=  0x02;
	m_cbHandCardData[0][1]	=  0x12;
	m_cbHandCardData[0][2]	=  0x22;
	m_cbHandCardData[0][3]	=  0x32;
	m_cbHandCardData[0][4]	=  0x01;
	m_cbHandCardData[0][5]	=  0x3b;
	m_cbHandCardData[0][6]	=  0x1a;
	m_cbHandCardData[0][7]	=  0x0a;
	m_cbHandCardData[0][8]	=  0x29;
	m_cbHandCardData[0][9]	=  0x38;
	m_cbHandCardData[0][10]	=  0x08;
	m_cbHandCardData[0][11] =  0x36;
	m_cbHandCardData[0][12] =  0x26;
	m_cbHandCardData[0][13] =  0x06;
	m_cbHandCardData[0][14] =  0x25;
	m_cbHandCardData[0][15] =  0x24;
	m_cbHandCardData[0][16] =  0x33;*/
			

	
	/*1*/
	/*m_cbHandCardData[1][0] = 0x31;
	m_cbHandCardData[1][1] = 0x3d;
	m_cbHandCardData[1][2] = 0x2d;
	m_cbHandCardData[1][3] = 0x0d;
	m_cbHandCardData[1][4] = 0x1b;
	m_cbHandCardData[1][5] = 0x0b;
	m_cbHandCardData[1][6] = 0x39;
	m_cbHandCardData[1][7] = 0x09;
	m_cbHandCardData[1][8] = 0x18;
	m_cbHandCardData[1][9] = 0x17;
	m_cbHandCardData[1][10] = 0x07;
	m_cbHandCardData[1][11] = 0x16;
	m_cbHandCardData[1][12] = 0x15;
	m_cbHandCardData[1][13] = 0x04;
	m_cbHandCardData[1][14] = 0x23;
	m_cbHandCardData[1][15] = 0x13;
	m_cbHandCardData[1][16] = 0x03;*/

	/*2*/
	/*m_cbHandCardData[2][0] = 0x34;
	m_cbHandCardData[2][1] = 0x14;
	m_cbHandCardData[2][2] = 0x35;
	m_cbHandCardData[2][3] = 0x05;
	m_cbHandCardData[2][4] = 0x37;
	m_cbHandCardData[2][5] = 0x27;
	m_cbHandCardData[2][6] = 0x28;
	m_cbHandCardData[2][7] = 0x19;
	m_cbHandCardData[2][8] = 0x3a;
	m_cbHandCardData[2][9] = 0x2a;
	m_cbHandCardData[2][10] = 0x3c;
	m_cbHandCardData[2][11] = 0x2c;
	m_cbHandCardData[2][12] = 0x1c;
	m_cbHandCardData[2][13] = 0x0c;
	m_cbHandCardData[2][14] = 0x1d;
	m_cbHandCardData[2][15] = 0x4e;
	m_cbHandCardData[2][16] = 0x4f;

	//���õ���
	m_cbBankerCard[0] = 0x21;
	m_cbBankerCard[1] = 0x11;
	m_cbBankerCard[2] = 0x3b;*/
	//=============test=end=====================
	
	//��Ϸ��¼�Ŀ��г�������
    CMD_S_StatusFree StatusFree;
    memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
    StatusFree.dCellScore = m_dwlCellScore*TO_DOUBLE;
    m_pGameDeskPtr->WriteGameSceneToRecord(&StatusFree, sizeof(StatusFree), SC_GAMESCENE_FREE);
    
	bool bHasAndroid = false;
	//��������
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
		//�����˿�
	//	assert(CountArray(GameStart.cbCardData) >= m_cbHandCardCount[i]);
        GameStart.dwStartUser = i; //m_dwFirstUser;
        memcpy(GameStart.cbCardData, m_cbHandCardData[i], sizeof(GameStart.cbCardData));
        //��������
        SendTableData(i, SUB_S_GAME_START, &GameStart, sizeof(GameStart), true);
        
		if (m_pGameDeskPtr->IsAndroidUser(i))
		{
            if (bHasAndroid == false)
            {
                bHasAndroid = true;
            }
		}
	}

	//�����˿�
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		m_GameLogic.SortCardList(m_cbHandCardData[i], m_cbHandCardCount[i], ST_ORDER);

		//printf("game splayer %d hand card: ", i);
		//for (int j = 0; j < NORMAL_COUNT; ++j)
		//{
		//	printf("%x, ", m_cbHandCardData[i][j]);
		//}
		//printf("\n");
	}



	//�������˷��Ϳ�ʼ����
	if (bHasAndroid)
	{
		CMD_S_GameStartAi GameStartAi;
		GameStartAi.dwStartUser = m_dwFirstUser;
		GameStartAi.dwCurrentUser = m_dwCurrentUser;
		GameStartAi.dwStartTime = m_dwStartTime;
		memcpy(GameStartAi.cbBankerCard, m_cbBankerCard, sizeof(GameStartAi.cbBankerCard));

		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			for (CT_BYTE j = 0; j != NORMAL_COUNT; ++j)
			{
				GameStartAi.cbCardData[i][j] = m_cbHandCardData[i][j];
			}
			//memcpy(GameStartAi.cbCardData[i], m_cbHandCardData[i], sizeof(GameStartAi.cbCardData));
		}

		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			if (m_pGameDeskPtr->IsAndroidUser(i))
			{
				//��������
				SendTableData(i, SUB_S_GAME_START, &GameStartAi, sizeof(GameStartAi), false);
			}
		}
	}

	//�й�
	IsTrustee();
	return;
}

//��Ϸ����
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//������ж�ʱ��
	//ClearAllTimer();
	//������Ϸ״̬-����״̬
	m_cbGameStatus = GAME_SCENE_FREE;

	switch (GETag)
	{
	case GER_NORMAL:
	{//�����˳�
		NormalGameEnd(dwChairID);
		break;
	}
	case GER_USER_LEFT:
	{//���ǿ���˳�,��Ѹ�����йܲ�����
		break;
	}
	case GER_DISMISS:
	{//��Ϸ��ɢ
		break;
	}
	default:
		break;
	}

	//���´���ŵ�ȫ���ߵ������˺�ִ��
	//��ǰʱ��
	//std::string strTime = Utility::GetTimeNowString();
	//��Ϸ����
	//m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());

	//�����
	/*if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_End)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnEventGameEnd: SendTotalClearing";

		//�����ܽ���
		SendTotalClearing(strTime);

		//���������ϵ��û�
		m_pGameDeskPtr->ClearTableUser();
	}*/
	//�й�
	//IsTrustee();
	m_cbGameStatus = GAME_SCENE_FREE;

	//����һ��ʱ���������(ֱ��t������������androidmgr�Ļ����˵�m_AndroidUserItemSittingʧЧ)
	if (m_PrivateTableInfo.dwRoomNum == 0)
    {
        m_pGameDeskPtr->SetGameTimer(IDI_TICK_USER, 1000);

        //����ֱ��T����
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            //��������Ϸ�޳��û�ʱ����Ҫ�㲥״̬���ͻ���
            if (!m_pGameDeskPtr->IsAndroidUser(i))
                m_pGameDeskPtr->ClearTableUser(i, false);
        }
    }
    else
    {
        m_pGameDeskPtr->SetGameTimer(IDI_TICK_USER, 16000);
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            m_pGameDeskPtr->SetUserStatus(i, sSit);
        }

        //��ǰʱ��
        std::string strTime = Utility::GetTimeNowString();
        //��Ϸ����
        m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());
    }
	
	return;
}

//���ͳ���
void CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID >= GAME_PLAYER)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "OnEventGameScene: dwChairID = " << (int)dwChairID << "  err";
		return;
	}

	switch (m_cbGameStatus)
	{
		case GAME_SCENE_FREE:	//����״̬
		{
			CMD_S_StatusFree StatusFree;
			memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
			StatusFree.dCellScore = m_dwlCellScore*0.01f;
			StatusFree.dwCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

			//��������
			SendTableData(dwChairID, SC_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree),false);
			break;
		}
		case GAME_SCENE_CALL:	//�е���״̬
		{
			//��������
			CMD_S_StatusCall StatusCall;
			memset(&StatusCall,0, sizeof(StatusCall));

			//��Ԫ����
			StatusCall.dCellScore = m_dwlCellScore*0.01f;

			//ʣ��ʱ�� 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusCall.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//��Ϸ��Ϣ
			StatusCall.wCurrentUser = m_dwCurrentUser;
			memcpy(StatusCall.cbCallBankerInfo, m_cbCallBankerInfo, sizeof(m_cbCallBankerInfo));

			//�����˿�
			memcpy(StatusCall.cbHandCardData, m_cbHandCardData[dwChairID], sizeof(StatusCall.cbHandCardData));
	
			//�й�״̬
			memcpy(StatusCall.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//���ͳ���
			SendTableData(dwChairID, SC_GAMESCENE_CALL, &StatusCall, sizeof(StatusCall), false);
			break;
		}
		case GAME_SCENE_DOUBLE:
		{
			CMD_S_StatusDouble StatusDouble;
			memset(&StatusDouble, 0, sizeof(StatusDouble));

			//��Ԫ����
			StatusDouble.dCellScore = m_dwlCellScore*0.01f;
			StatusDouble.cbCallScore = (CT_BYTE)m_dwStartTime;

			//ʣ��ʱ�� 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusDouble.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//��Ϸ����			 
			StatusDouble.wBankerUser = m_dwBankerUser;
			StatusDouble.wCurrTime = m_wEachTotalTime[dwChairID];
			memcpy(StatusDouble.cbDoubleInfo, m_cbAddDoubleInfo, sizeof(StatusDouble.cbDoubleInfo));

			//�˿���Ϣ
			memcpy(StatusDouble.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));
			//�����˿�
			memcpy(StatusDouble.cbHandCardCount, m_cbHandCardCount, sizeof(StatusDouble.cbHandCardCount));
			memcpy(StatusDouble.cbHandCardData, m_cbHandCardData[dwChairID], m_cbHandCardCount[dwChairID] * sizeof(CT_BYTE));
			//�й�״̬
			memcpy(StatusDouble.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//���ͳ���
			SendTableData(dwChairID, SC_GAMESCENE_DOUBLE, &StatusDouble, sizeof(StatusDouble), false);

			break;
		}
		case GAME_SCENE_PLAY:	//��Ϸ״̬
		{
			//��������
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay, 0, sizeof(StatusPlay));

			//��Ԫ����
			StatusPlay.dCellScore = m_dwlCellScore*0.01f;

			//ʣ��ʱ�� 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusPlay.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//������Ϣ
			StatusPlay.wTurnWiner = m_dwTurnWiner;
			StatusPlay.cbTurnCardCount = m_cbTurnCardCount;
			memcpy(StatusPlay.cbTurnCardData, m_cbTurnCardData, m_cbTurnCardCount * sizeof(CT_BYTE));

			//��Ϸ����			 
			StatusPlay.dwBankerUser = m_dwBankerUser;
			StatusPlay.dwCurrentUser = m_dwCurrentUser;

			StatusPlay.cbCallScore = (CT_BYTE)m_dwStartTime;
			if (dwChairID == m_dwBankerUser)
			{
				StatusPlay.dwBombTime = m_dwBombTime*2;
			}
			else
			{
				StatusPlay.dwBombTime = m_dwBombTime;
			}
		
			memcpy(StatusPlay.cbAddDoubleInfo, m_cbAddDoubleInfo, sizeof(StatusPlay.cbAddDoubleInfo));

			//�˿���Ϣ
			memcpy(StatusPlay.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));
			memcpy(StatusPlay.cbHandCardCount, m_cbHandCardCount, sizeof(m_cbHandCardCount));
			memcpy(StatusPlay.cbOutCount, m_cbOutCardCount, sizeof(StatusPlay.cbOutCount));
			memcpy(StatusPlay.cbOutCardListCount, m_cbOutCardListCount, sizeof(StatusPlay.cbOutCardListCount));
			memcpy(StatusPlay.cbOutCardList, m_cbOutCardList, sizeof(StatusPlay.cbOutCardList));

			//�����˿�
			memcpy(StatusPlay.cbHandCardData, m_cbHandCardData[dwChairID], m_cbHandCardCount[dwChairID] * sizeof(CT_BYTE));
			//�й�״̬
			memcpy(StatusPlay.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//���ͳ���
			SendTableData(dwChairID, SC_GAMESCENE_PLAY, &StatusPlay, sizeof(StatusPlay),false);
			break;
		}
		case GAME_SCENE_END://Ŀǰ�Ѿ���������������
		{
			//��������
			CMD_S_StatusEND StatusEnd;
			memset(&StatusEnd, 0, sizeof(CMD_S_StatusEND));
			//��������
			StatusEnd.dCellScore = m_dwlCellScore*0.01f;
			StatusEnd.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
			//ʣ��ʱ��
			CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
			StatusEnd.cbTimeLeave = (CT_BYTE)(m_cbOpTotalTime - std::min<CT_BYTE>(dwPassTime, m_cbOpTotalTime));
			//�˿���Ϣ
			memcpy(StatusEnd.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));
			memcpy(StatusEnd.cbHandCardCount, m_cbHandCardCount, sizeof(m_cbHandCardCount));
			//�����˿�
			memcpy(StatusEnd.cbHandCardData, m_cbHandCardData[dwChairID], m_cbHandCardCount[dwChairID] * sizeof(CT_BYTE));

			//���ͽ���״̬����
			SendTableData(dwChairID, SC_GAMESCENE_END, &StatusEnd, sizeof(CMD_S_StatusEND), false);
			
			//��Ϸ������Ϣx
			if (StatusEnd.cbTimeLeave>=5)
			{
				SendTableData(dwChairID, SUB_S_GAME_CONCLUDE, &StatusEnd, sizeof(CMD_S_StatusEND), false);
			}

			break;
		}
		default:
			break;
	}
	return ;
}

//��ʱ���¼�
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	if (dwTimerID == IDI_CHECK_END)
	{
		OnEventGameEnd(m_dwBankerUser, GER_NORMAL);
		return;
	}
	 
	//LOG(WARNING) << "run timer, timer id: " << dwTimerID;

	m_pGameDeskPtr->KillGameTimer(dwTimerID);
	if (m_cbGameStatus == GAME_SCENE_FREE)
	{
		//if (!m_pGameDeskPtr->IsExistUser(dwParam))
		//{
		//	return;
		//}

		switch (dwTimerID - dwParam)
		{
		//case IDI_AUTO_READY:
		//{
		//	m_pGameDeskPtr->SetUserReady(dwParam);
		//}
		//break;
		case IDI_TICK_USER:
		{
            if (m_PrivateTableInfo.dwRoomNum == 0)
            {
                for (CT_DWORD i= 0; i < GAME_PLAYER; ++i)
                {
                    if (!m_pGameDeskPtr->IsExistUser(i))
                    {
                        continue;
                    }

                    //��������Ϸ�޳��û�ʱ����Ҫ�㲥״̬���ͻ���
                    m_pGameDeskPtr->ClearTableUser(i, false);
                }
                //��ǰʱ��
                std::string strTime = Utility::GetTimeNowString();
                //��Ϸ����
                m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());
            }
            else
            {
                for (CT_DWORD i= 0; i < GAME_PLAYER; ++i)
                {
                    if (!m_pGameDeskPtr->IsExistUser(i))
                    {
                        continue;
                    }

                    //��������Ϸ�޳��û�ʱ����Ҫ�㲥״̬���ͻ���
                    if (m_pGameDeskPtr->GetUserStatus(i) != sReady)
                        m_pGameDeskPtr->ClearTableUser(i, true);
                }
            }
		}
		break;
		default:
			break;
		}
		return;
	}
	else if (m_cbGameStatus == GAME_SCENE_DOUBLE && dwTimerID == IDI_ADD_DOUBLE)
	{
		for (CT_DWORD i = 0; i != GAME_PLAYER; ++i)
		{
			if (!m_pGameDeskPtr->IsExistUser(i) || m_dwBankerUser == i)
			{
				continue;
			}

			if (m_cbAddDoubleInfo[i] == CB_NOT_ADD_DOUBLE)
			{
				OnAutoDouble(i);
			}
		}
		return;
	}

	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return;
	}
    CT_DWORD dwTime = time(NULL);
	//�е���״̬�������йܣ���Ϸ״̬��ʱ1���й�
	if (m_cbGameStatus == GAME_SCENE_PLAY &&
		m_cbTrustee[m_dwCurrentUser] != 1 && 
		(m_cbTimeOutCount[m_dwCurrentUser] >= TIME_OUT_TRUSTEE_COUNT || m_cbTurnCardCount == 0)
        && ((int)(m_cbOpTotalTime-1) <= (int)(dwTime-m_dwOpStartTime)))
	{
		SetTrustee(m_dwCurrentUser, 1);
	}

	++m_cbTimeOutCount[m_dwCurrentUser];

	CT_BOOL bSystemSucceed = true;
	switch (dwTimerID)
	{
	case IDI_CALL_BANKER:
	{
		bSystemSucceed = OnAutoCallLand();
	}
	break;
	case IDI_OUT_CARD:
	{
        //LOG(ERROR) << "lTime " << iTime << " m_dwOpStartTime " << (int)m_dwOpStartTime << " m_cbOpTotalTime " << (int)m_cbOpTotalTime;
	    if(((m_cbOpTotalTime-1) <= (int)(dwTime - m_dwOpStartTime)) || m_cbTrustee[m_dwCurrentUser])
        {
            //return;
            if (m_cbTrustee[m_dwCurrentUser])
                bSystemSucceed = OnAutoOutCard();
            else
                bSystemSucceed = OnUserPassCard(m_dwCurrentUser);
        } else{
            bSystemSucceed = false;
            //LOG(ERROR) << "IDI_OUT_CARD33 " << (int)m_cbOpTotalTime <<" " << (int)(iTime-m_dwOpStartTime);
	    }
	}
	break;
	default:
		break;
	}

	if (bSystemSucceed == false)
	{
        m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (((m_cbOpTotalTime-(dwTime-m_dwOpStartTime)) >  0) ? (m_cbOpTotalTime-(dwTime-m_dwOpStartTime)) : 1)*1000);
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "OnTimerMessage err, time id: " << dwTimerID;
	}
}

//��Ϸ��Ϣ
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	switch (dwSubCmdID)
	{
		case SUB_C_CALL_BANKER:	//�û��е���
		{
			//Ч������
			//assert(dwDataSize == sizeof(CMD_C_CallBanker));
			if (dwDataSize != sizeof(CMD_C_CallBanker)) return false;

			//״̬Ч��
			//assert(m_cbGameStatus != GAME_SCENE_CALL);
			if (m_cbGameStatus != GAME_SCENE_CALL) return true;

			//��������
			CMD_C_CallBanker * pCallBanker = (CMD_C_CallBanker *)pDataBuffer;

			//��Ϣ����
			return OnUserCallBanker(wChairID, pCallBanker->cbCallInfo);
		}
		case SUB_C_ADD_DOUBLE:
		{
			//Ч������
			if (dwDataSize != sizeof(CMD_C_Double)) return false;

			//״̬Ч��
			//ASSERT(m_cbGameStatus==GAME_SCENE_DOUBLE);
			if (m_cbGameStatus != GAME_SCENE_DOUBLE) return true;

			//��������
			CMD_C_Double * pDouble = (CMD_C_Double *)pDataBuffer;

			//��Ϣ����
			return OnUserDouble(wChairID, pDouble->cbDoubleInfo);
		}
		case SUB_C_OUT_CARD:	//�û�����
		{
			//��������
			CMD_C_OutCard * pOutCard = (CMD_C_OutCard *)pDataBuffer;
			CT_DWORD dwHeadSize = sizeof(CMD_C_OutCard) - sizeof(pOutCard->cbCardData);

			//Ч������
			/*assert((dwDataSize >= dwHeadSize) && (dwDataSize == (dwHeadSize + pOutCard->cbCardCount * sizeof(CT_BYTE))));*/
			if ((dwDataSize < dwHeadSize)/* || (dwDataSize != (dwHeadSize + pOutCard->cbCardCount * sizeof(CT_BYTE)))*/) return false;

			//״̬Ч��
			//assert(m_cbGameStatus!= GAME_SCENE_PLAY);
			if (m_cbGameStatus != GAME_SCENE_PLAY) return true;


			//��Ϣ����
			return OnUserOutCard(wChairID, pOutCard->cbCardData, pOutCard->cbCardCount);
		}
		case SUB_C_PASS_CARD:	//�û�����
		{
			//״̬Ч��
			//assert(m_cbGameStatus == GAME_SCENE_PLAY);
			if (m_cbGameStatus != GAME_SCENE_PLAY) return true;

			//��Ϣ����
			return OnUserPassCard(wChairID);
		}
		case SUB_C_TRUSTEE: //�й�
		{
			if (dwDataSize != sizeof(CMD_C_Trustee)) return false;

			//��������
			CMD_C_Trustee * pTrustee = (CMD_C_Trustee *)pDataBuffer;

			if (pTrustee->wChairID == INVALID_CHAIR) return true;
			m_cbTrustee[pTrustee->wChairID] = 1;

			//������Ϣ
			CMD_S_StatusTrustee StatusTrustee;
			memcpy(StatusTrustee.cbTrustee, m_cbTrustee,sizeof(m_cbTrustee));
			//��������
			for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
			    if (i == m_dwBankerUser)
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
                }
                else
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
                }
			}

			if (m_dwCurrentUser == wChairID)
			{
				IsTrustee(false);
			}
			return true;
		}
		case SUB_C_CANCEL_TRUSTEE: //ȡ���й�
		{
			if (dwDataSize != sizeof(CMD_C_CancelTrustee)) return false;

			//��������
			CMD_C_CancelTrustee * pCancelTrustee = (CMD_C_CancelTrustee *)pDataBuffer;

			if (pCancelTrustee->wChairID == INVALID_CHAIR) return true;

			m_cbTrustee[pCancelTrustee->wChairID] = 0;
			//������Ϣ
			CMD_S_StatusTrustee StatusTrustee;
			memcpy(StatusTrustee.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
			//��������
			for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
                if (i == m_dwBankerUser)
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
                }
                else
                {
                    SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
                }
			}
			if(pCancelTrustee->wChairID==m_dwCurrentUser)
			{
				long lTime = time(NULL);
				if((m_cbOpTotalTime - (lTime-m_dwOpStartTime)) > 1)
				{
					m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
					int iTime = m_cbOpTotalTime - (lTime-m_dwOpStartTime) + TIME_NETWORK_COMPENSATE;
					//LOG(ERROR) << "i " << iTime << " " << lTime << " " << m_dwOpStartTime;
					m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (iTime) * 1000);
				}
			}
			//IsTrustee();
			return true;
		}
		case SUB_C_CHEAT_LOOK_CARD:
		{
			if (dwDataSize != sizeof(CMD_C_CheatLookCard)) return false;

			CT_BOOL bAndroid = m_pGameDeskPtr->IsAndroidUser(wChairID);
			if (!bAndroid)
			{
				return false;
			}

			//��������
			CMD_C_CheatLookCard * pCheatLookCard = (CMD_C_CheatLookCard *)pDataBuffer;
			if (pCheatLookCard->wBeCheatChairID >= GAME_PLAYER)
			{
				return true;
			}

			CMD_S_CheatLookCard cheatCardData;
			cheatCardData.wCardUser = pCheatLookCard->wBeCheatChairID;
			cheatCardData.cbCardCount = m_cbHandCardCount[pCheatLookCard->wBeCheatChairID];
			memcpy(cheatCardData.cbCardData, m_cbHandCardData[pCheatLookCard->wBeCheatChairID], sizeof(cheatCardData.cbCardData));
			SendTableData(wChairID, SUB_S_CHEAT_LOOK_CARD, &cheatCardData, sizeof(cheatCardData), false);

			return true;
		}
	}
	return CT_TRUE;
}

//�û�����
void CGameProcess::OnUserEnter(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID == INVALID_CHAIR)
	{
		return;
	}

	//m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY + dwChairID);
	//m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER + dwChairID);

	//if (m_pGameDeskPtr->GetUserStatus(dwChairID) < sReady)
	//{
	//	m_pGameDeskPtr->SetGameTimer(IDI_AUTO_READY + dwChairID, TIME_AUTO_READY * 1000, dwChairID);
	//}
}

void CGameProcess::OnUserReady(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID == INVALID_CHAIR)
	{
		return;
	}

	//m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY + dwChairID);
	//m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER + dwChairID);
}

//�û��뿪
void CGameProcess::OnUserLeft(CT_DWORD dwChairID, CT_BOOL bIsLookUser, CT_BYTE byLeftTag)
{

}

//�û�����
void CGameProcess::OnUserOffLine(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{

}

//����ָ��
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	//assert(NULL != pTableFrame);
	m_pGameDeskPtr = pTableFrame;

	//��ʼ����Ϸ����
	InitGameData();
	//������Ϸ����
	UpdateGameConfig();
}

//������Ϸ����
void CGameProcess::ClearGameData()
{
	//���������Ӯ��
	//memset(m_iTotalLWScore, 0, sizeof(m_iTotalLWScore));
	//�����ը������
	//memset(m_cbTotalRomb, 0, sizeof(m_cbTotalRomb));
	//��Ӯ����
	//memset(m_wPlayWinCount, 0, sizeof(m_wPlayWinCount));
	//memset(m_wPlayLoseCount, 0, sizeof(m_wPlayLoseCount));
	//�׽��û�
	/*m_dwFirstUser = INVALID_CHAIR;*/

}


//˽�˷���Ϸ����Ƿ����
bool CGameProcess::IsGameRoundClearing(CT_BOOL bDismissGame)
{
	if (bDismissGame)
	{//��ɢ
		return true;
	}

	CT_WORD wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
	if (wCurrPlayCount >= m_PrivateTableInfo.wTotalPlayCount)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "IsGameRoundClearing: true";
		return true;
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "IsGameRoundClearing: false";
	return false;
}

//��ȡ��ֽ�����ܷ�
CT_INT32 CGameProcess::GetGameRoundTotalScore(CT_DWORD dwChairID)
{
	if (dwChairID >= m_PrivateTableInfo.wUserCount)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Err dwChairID=" << dwChairID;
		return 0;
	}
	//LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Score=" << m_iTotalLWScore[dwChairID];
	//return m_iTotalLWScore[dwChairID];
	return 0;
}

//������Ϸ������(��������Ϸ����Ϊ����ID��������Ӧ�ķ����Ƶĸ���)
CT_VOID CGameProcess::SetBlackList(std::vector<tagBrGameBlackList>& vecBlackList)
{
	for (auto& itVec : vecBlackList)
	{
		auto itMap = m_mapChannelControl.find(itVec.dwUserID);
		if (itMap != m_mapChannelControl.end())
		{
			itMap->second.wBadCardRation = itVec.wUserLostRatio;
		}
		else
		{
            tagBlackChannel channelControl;
			channelControl.dwChannelID = itVec.dwUserID;
			channelControl.wBadCardRation = itVec.wUserLostRatio;
			m_mapChannelControl.insert(std::make_pair(channelControl.dwChannelID, channelControl));
		}
	}

	for (auto& it : m_mapChannelControl)
    {
	    LOG(WARNING) << "channel id: " << it.first << ", ration: " << it.second.wBadCardRation;
    }
}

//��������
CT_BOOL CGameProcess::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BOOL bRecord)
{
	if (NULL == m_pGameDeskPtr)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "SendTableData: dwChairID = " << (int)dwChairID << ",wSubCmdID ="<< (int)wSubCmdID << " err";
		return false;
	}
	return m_pGameDeskPtr->SendTableData(dwChairID, wSubCmdID, pBuf, dwLen, bRecord);
}

//��ʼ����Ϸ����
void CGameProcess::InitGameData()
{
    //ϵͳʱ��
	m_dwSysTime = 0;

	//��Ϸ״̬
	m_cbGameStatus = GAME_SCENE_FREE;							
	//�׽��û�
/*	m_dwFirstUser = INVALID_CHAIR;*/
	//ׯ���û�
	m_dwBankerUser = INVALID_CHAIR;
	//��ǰ���
	m_dwCurrentUser = INVALID_CHAIR;

	//ը������
	m_cbBombCount = 0;
	m_cbMissile = 0;
	m_wMaxBombCount = 10000;
	m_cbOpTotalTime = 0;
	m_dwOpStartTime = 0;
	memset(m_cbEachBombCount, 0, sizeof(m_cbEachBombCount));
	memset(m_wEachTotalTime, 0, sizeof(m_wEachTotalTime));
	//��ʼ����
	m_dwStartTime = 1;
	//��������
	m_dwlCellScore = 1;
	
	//ը������
	m_dwBombTime = 1;
	//���챶��
	m_dwChunTianbTime = 1;
	//��������
	memset(m_cbOutCardCount, 0, sizeof(m_cbOutCardCount));

	//�е�����Ϣ
	memset(m_cbCallBankerInfo, 0, sizeof(m_cbCallBankerInfo));
	//�ӱ���Ϣ
	memset(m_cbAddDoubleInfo, 0, sizeof(m_cbAddDoubleInfo));

	//ʤ�����
	m_dwTurnWiner = INVALID_CHAIR;
	//������Ŀ
	m_cbTurnCardCount = 0;
	//��������	
	memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	memset(m_cbOutCardListCount, 0, sizeof(m_cbOutCardListCount));
	memset(m_cbOutCardList, 0, sizeof(m_cbOutCardList));

	//��ʼ��ʣ��������
	/*for (int i = 0; i < MAX_CARD_VALUE; ++i)
	{
		if (i < 13)
		{
			m_cbRemainCard[i] = 4;
		}
		else
		{
			m_cbRemainCard[i] = 1;
		}
	}*/

	//��Ϸ����
	memset(m_cbBankerCard, 0, sizeof(m_cbBankerCard));
	//�˿���Ŀ
	memset(m_cbHandCardCount, 0, sizeof(m_cbHandCardCount));
	//�����˿�
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	//�Ƿ��й�
	memset(m_cbTrustee, 0, sizeof(m_cbTrustee));
	//��ʱ����
	memset(m_cbTimeOutCount, 0, sizeof(m_cbTimeOutCount));
	
	//��Ϸ����
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));

	memset(m_RecordScoreInfo, 0, sizeof(m_RecordScoreInfo));

	if (NULL != m_pGameDeskPtr)
	{//��ȡ˽�˷���Ϣ
		m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
		m_dwlCellScore = m_pGameDeskPtr->GetGameCellScore();
		m_pGameRoomKindInfo = m_pGameDeskPtr->GetGameKindInfo();
	}
}

//������Ϸ����
void CGameProcess::UpdateGameConfig()
{
	if (NULL== m_pGameDeskPtr)
	{
		return;
	}
	GameCfgData Cfg;
	memset(&Cfg, 0, sizeof(Cfg));
	m_pGameDeskPtr->GetGameCfgData(Cfg);
}

//������ж�ʱ��
void CGameProcess::ClearAllTimer()
{
	//��ʼɾ�����ж�ʱ��
	//m_pGameDeskPtr->KillGameTimer(IDI_AUTO_READY);
	m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
	m_pGameDeskPtr->KillGameTimer(IDI_ADD_DOUBLE);
	m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
	m_pGameDeskPtr->KillGameTimer(IDI_TICK_USER);
}

//��ͨ����Ϸ����
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwWinChairID)
{
	//ɾ�����ƶ�ʱ��
	m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
	m_pGameDeskPtr->KillGameTimer(IDI_CHECK_END);

	if (dwWinChairID>=INVALID_CHAIR)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "NormalGameEnd Err!";
		return;
	}

	DDZ_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));

	//ը����Ϣ
	if (m_cbBombCount == 0)
	{
		GameEnd.cbBombTimes = 0;
	}
	else
	{
		GameEnd.cbBombTimes = 1;
		for (int i = 0; i < m_cbBombCount; i++)
		{
			GameEnd.cbBombTimes *= SCORE_TIME_BOMB;
		}	
	}
	
	GameEnd.cbMissileTimes = m_cbMissile*SCORE_TIME_BOMB;
	GameEnd.cbCallScore = (CT_BYTE)m_dwStartTime;

	//�û��˿˺ͼӱ���Ϣ
	CT_DWORD dwUserDouble[GAME_PLAYER] = { 1,1,1 };
	CT_BYTE cbDoubleCount = 0;
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		//�����˿�
		GameEnd.cbCardCount[i] = m_cbHandCardCount[i];
		memcpy(&GameEnd.cbHandCardData[i], m_cbHandCardData[i], m_cbHandCardCount[i] * sizeof(CT_BYTE));

		GameEnd.cbDoubleInfo[i] = m_cbAddDoubleInfo[i];
		if (i != m_dwBankerUser)
		{
			if (CB_ADD_DOUBLE == m_cbAddDoubleInfo[i])
			{
				++cbDoubleCount;
				dwUserDouble[i] *= 2L;
			}
		}
	}

	//�����ж�
	if (dwWinChairID == m_dwBankerUser)
	{
		//�û�����
		CT_WORD wUser1 = GetNextUser(m_dwBankerUser);
		CT_WORD wUser2 = GetNextUser(m_dwBankerUser+1);

		//�û��ж�
		if ((m_cbOutCardCount[wUser1] == 0) && (m_cbOutCardCount[wUser2] == 0))
		{
			GameEnd.bChunTian = 1;
			m_dwChunTianbTime = SCORE_TIME_CHUANTIAN;
		}
	}
	else
	{
		//�������ж�
		if (m_cbOutCardCount[m_dwBankerUser] == 1)
		{
			GameEnd.bFanChunTian = 1;
			m_dwChunTianbTime = SCORE_TIME_FANCHUAN;
		}
	}

	//���ֱ���
	tagScoreInfo ScoreInfoArray[GAME_PLAYER];
	memset(&ScoreInfoArray,0, sizeof(ScoreInfoArray));

	CT_INT32 iCellScore = m_dwlCellScore;

	if (m_wMaxBombCount != 1)
	{
		m_dwBombTime = m_dwBombTime > m_wMaxBombCount ? m_wMaxBombCount : m_dwBombTime;
	}
	
	CT_INT32 iBankerScore = iCellScore *m_dwStartTime* m_dwBombTime * m_dwChunTianbTime;

	CT_INT32 iGameScore[GAME_PLAYER] = { 0,0,0 };
	CT_BYTE bWinMaxScore[GAME_PLAYER] = { 0, 0, 0 };
	//ׯ��Ӯ
	if (m_cbHandCardCount[m_dwBankerUser] == 0)
	{
		//ׯ�����Ӯ��
		CT_LONGLONG llBankerMaxWinScore = m_pGameDeskPtr->GetUserScore(m_dwBankerUser);
		CT_INT32 iBankerWinScore = 0;
		for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
		{
			if (i != m_dwBankerUser)
			{
				//��ҷֲ����Ļ��ж��������
				//CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
				CT_LONGLONG llLostScore = iBankerScore*dwUserDouble[i];//llUserScore > iBankerScore*dwUserDouble[i] ? iBankerScore*dwUserDouble[i] : llUserScore;

				ScoreInfoArray[i].iScore -= (CT_INT32)llLostScore;
				ScoreInfoArray[i].cbType = SCORE_TYPE_LOSE;
				GameEnd.bLose[i] = GAME_LOSE;

				//���û���
				iBankerWinScore += llLostScore;
				//ScoreInfoArray[m_dwBankerUser].iScore += (CT_INT32)llLostScore;
			}
		}

		//�ж�ׯ���Ƿ�ﵽӮ�����ֵ,���˻�����Ľ��
        CT_INT32 llReturnBackPartScore = 0;
		if (iBankerWinScore > llBankerMaxWinScore)
		{
			bWinMaxScore[m_dwBankerUser] = 1;
			CT_INT32 llReturnBackScore = iBankerWinScore - (CT_INT32)llBankerMaxWinScore;
			//ScoreInfoArray[m_dwBankerUser].iScore -= llReturnBackScore;
			if (cbDoubleCount == 1)
			{
				llReturnBackPartScore = llReturnBackScore / 3;
			}
			else
			{
				llReturnBackPartScore = llReturnBackScore / 2;
			}
		}

        //���У��(�˻����ͱ�֤�мҹ��ۣ�
        for (CT_BYTE i = 0; i != GAME_PLAYER; ++i)
        {
            if (i != m_dwBankerUser)
            {
                //��Ҫ�˻����
                if (llReturnBackPartScore != 0)
                {
                    if (cbDoubleCount == 1)
                    {
                        if (CB_ADD_DOUBLE == m_cbAddDoubleInfo[i])
                        {
                            ScoreInfoArray[i].iScore += (llReturnBackPartScore * 2);
                        }
                        else
                        {
                            ScoreInfoArray[i].iScore += llReturnBackPartScore;
                        }
                    }
                    else
                    {
                        ScoreInfoArray[i].iScore += llReturnBackPartScore;
                    }
                }
                //��֤�мҹ���
                CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(i);
                if (ScoreInfoArray[i].iScore + llUserScore < 0)
                {
                    ScoreInfoArray[i].iScore = -llUserScore;
                }

                //ScoreInfoArray[i].iScore�����ֵ�Ǹ���������banker userӦ���Ǽ���ȥ���ֵ
				ScoreInfoArray[m_dwBankerUser].iScore -= ScoreInfoArray[i].iScore;
            }
        }

		ScoreInfoArray[m_dwBankerUser].cbType = SCORE_TYPE_WIN;
		GameEnd.bLose[m_dwBankerUser] = GAME_WIN;
	}
	else
	{
		//��ͳ��ׯ�������
		CT_INT32 llBankLostScore = 0;
		CT_INT32 llTrueWinScore[GAME_PLAYER] = { 0,0,0 };
		for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
		{
			if (i != m_dwBankerUser)
			{
				llBankLostScore += iBankerScore*dwUserDouble[i];
				llTrueWinScore[i] += llBankLostScore;
			}
		}

		CT_LONGLONG llBankScore = m_pGameDeskPtr->GetUserScore(m_dwBankerUser);
		if (llBankScore > llBankLostScore)
		{
			ScoreInfoArray[m_dwBankerUser].cbType = SCORE_TYPE_LOSE;
			GameEnd.bLose[m_dwBankerUser] = GAME_LOSE;
			ScoreInfoArray[m_dwBankerUser].iScore -= llBankLostScore;

			for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
			{
				if (i != m_dwBankerUser)
				{
					CT_INT32 llWinScore = iBankerScore*dwUserDouble[i];

					//�����Ƿ񳬹������
					CT_LONGLONG llMaxWinScore = m_pGameDeskPtr->GetUserScore(i);
					if (llWinScore > llMaxWinScore)
					{
						bWinMaxScore[i] = 1;
						CT_INT32 llReturnBackScore = llWinScore - (CT_INT32)llMaxWinScore;
						llWinScore -= llReturnBackScore;
						ScoreInfoArray[m_dwBankerUser].iScore += llReturnBackScore;
					}

					//д�����
					ScoreInfoArray[i].iScore += llWinScore;
					ScoreInfoArray[i].cbType = SCORE_TYPE_WIN;
					GameEnd.bLose[i] = GAME_WIN;
				}
			}
		}
		else
		{
			//���ֻ��һ����Ҽӱ�
			CT_INT32 llScaleBankScore = 0;
			if (cbDoubleCount == 1)
			{
				llScaleBankScore = (CT_INT32)llBankScore / 3;
			}
			else
			{
				llScaleBankScore = (CT_INT32)llBankScore / 2;
			}
			
			ScoreInfoArray[m_dwBankerUser].cbType = SCORE_TYPE_LOSE;
			GameEnd.bLose[m_dwBankerUser] = GAME_LOSE;
			ScoreInfoArray[m_dwBankerUser].iScore -= (CT_INT32)llBankScore;

			for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
			{
				if (i != m_dwBankerUser)
				{
					CT_INT32 llWinScore = 0;
					if (m_cbAddDoubleInfo[i] == CB_ADD_DOUBLE)
					{
						if (cbDoubleCount == 1)
						{
							llWinScore += llScaleBankScore * 2;
						}
						else
						{
							llWinScore += llScaleBankScore;
						}
					}
					else
					{
						llWinScore += llScaleBankScore;
					}

					CT_LONGLONG llWinMaxScore = m_pGameDeskPtr->GetUserScore(i);
					if (llWinScore > llWinMaxScore)
					{
						bWinMaxScore[i] = 1;
						CT_INT32 llReturnBackScore = llWinScore - (CT_INT32)llWinMaxScore;
						llWinScore -= llReturnBackScore;
						ScoreInfoArray[m_dwBankerUser].iScore += llReturnBackScore;
					}
					
					ScoreInfoArray[i].iScore = llWinScore;
					ScoreInfoArray[i].cbType = SCORE_TYPE_WIN;
					GameEnd.bLose[i] = GAME_WIN;
				}
			}
			
			//���ũ���Ƿ�������õ�һЩǮ
			if (ScoreInfoArray[m_dwBankerUser].iScore + llBankScore != 0)
			{
				for (CT_BYTE i = 0; i != GAME_PLAYER; ++i)
				{
					if (i == m_dwBankerUser)
					{
						continue;
					}

					if (bWinMaxScore[i] == 0 && llTrueWinScore[i] > ScoreInfoArray[i].iScore)
					{
						CT_INT32 iNeedMoreScore = llTrueWinScore[i] - ScoreInfoArray[i].iScore;
						CT_INT32 iBankerLeftScore = (CT_INT32)(llBankScore + ScoreInfoArray[m_dwBankerUser].iScore);
						CT_INT32 iCompensationScore = (iBankerLeftScore > iNeedMoreScore) ? iNeedMoreScore : iBankerLeftScore;

						ScoreInfoArray[i].iScore += iCompensationScore;
						ScoreInfoArray[m_dwBankerUser].iScore -= iCompensationScore;
						break;
					}
				}
			}
		}
	}

	//д�����
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		iGameScore[i] = ScoreInfoArray[i].iScore;

		m_RecordScoreInfo[i].dwUserID = m_pGameDeskPtr->GetUserID(i);
		m_RecordScoreInfo[i].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(i);
		m_RecordScoreInfo[i].cbStatus = 1;

		CT_INT32 iRevenue = 0;
		m_RecordScoreInfo[i].iScore = iGameScore[i];
		m_RecordScoreInfo[i].llSourceScore = m_pGameDeskPtr->GetUserScore(i);
		//����˰��
		if (iGameScore[i] > 0 && (m_RecordScoreInfo[i].cbIsAndroid == 0))
		{
			iRevenue = (CT_INT32)m_pGameDeskPtr->CalculateRevenue(i, iGameScore[i]);
			iGameScore[i] -= iRevenue;
			m_RecordScoreInfo[i].dwRevenue = iRevenue;
		}
		ScoreInfo  ScoreData;
		//memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = i;
		ScoreData.bBroadcast = true;
		ScoreData.llScore = iGameScore[i];
		ScoreData.llRealScore= iGameScore[i];
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);
	}

	//�Ѹ�����ҵķ���תΪdouble,�����������ȡ���й�
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		GameEnd.dGameScore[i] = iGameScore[i] * 0.01f;
	}

	//��������
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		GameEnd.bWinMaxScore = bWinMaxScore[i];
		
		if (i == m_dwBankerUser)
        {
            SendTableData(i, SUB_S_GAME_CONCLUDE, &GameEnd, sizeof(GameEnd), true);
        }
        else
        {
            SendTableData(i, SUB_S_GAME_CONCLUDE, &GameEnd, sizeof(GameEnd), false);
        }
        
		if (m_cbTrustee[i] == 1)
		{
			SetTrustee(i, 0);
		}
	}
    
    CT_DWORD dwBankerUserID = 0;
    if (m_dwBankerUser != INVALID_CHAIR)
    {
        dwBankerUserID = m_pGameDeskPtr->GetUserID(m_dwBankerUser);
    }
	
	//��¼������Ϣ
	m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, NULL, 0, dwBankerUserID, 0, 0, 0, 0, 0);

	//�л��û�
	m_dwFirstUser = dwWinChairID;
	return;
}

//�����ܽ���
void CGameProcess::SendTotalClearing(std::string strTime)
{
	/**************************************************
	DDZ_CMD_S_CLEARING ClearingData;
	memset(&ClearingData, 0, sizeof(DDZ_CMD_S_CLEARING));
	ClearingData.wRoomOwner = m_pGameDeskPtr->GetPRoomOwnerChairID();
	ClearingData.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
	//��ǰʱ��
	_snprintf_info(ClearingData.szCurTime, sizeof(ClearingData.szCurTime), "%s", strTime.c_str());
	for (CT_WORD i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		ClearingData.llTotalLWScore[i] = m_iTotalLWScore[i];
		ClearingData.cbRombCount[i] = m_cbTotalRomb[i];
		ClearingData.wPlayWinCount[i] = m_wPlayWinCount[i];
		ClearingData.wPlayLoseCount[i] = m_wPlayLoseCount[i];
	}
	for (CT_BYTE i = 0;i < GAME_PLAYER;i++)
	{
		SendTableData(i, SUB_S_CLEARING, &ClearingData, sizeof(ClearingData),(i == 0) ? true : false);
	}
	**************************************************/
}

//����˽�˳�����
void CGameProcess::ParsePrivateData(CT_CHAR *pBuf)
{
	if (strlen(pBuf)==0)
	{
		return;
	}
	//���˽��������
	acl::json jsonCond(pBuf);

	const acl::json_node* nodeCond =  jsonCond.getFirstElementByTagName("cbBombCount");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "cbBombCount error!";
		return;
	}
	CT_BYTE cbBombCount = (CT_BYTE) *nodeCond->get_int64();
	for (CT_BYTE i = 0; i < cbBombCount; i++)
	{
		m_wMaxBombCount *= SCORE_TIME_BOMB;
	} 
	
	LOG_IF(INFO, PRINT_LOG_INFO) << "ParsePrivateData: m_cbMaxBombCount = "<<(int)m_wMaxBombCount << ",cbBombCount =" << (int)cbBombCount;
}

//��ҳ���
CT_BOOL CGameProcess::OnUserOutCard(CT_WORD wChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout/*=CT_FALSE*/)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard: wChairID = " << wChairID << ",cbOutCount = " << (int)cbOutCount;
	//Ч��״̬
	//assert(wChairID == m_dwCurrentUser);
	/*printf("player %d out card: ", wChairID);
	for (int j = 0; j < cbOutCount; ++j)
	{
		printf("%x, ", cbOutCard[j]);
	}
	printf("\n");*/

	//�������Ȳ��й�
	if (m_pGameDeskPtr->IsAndroidUser(wChairID) && m_cbTrustee[wChairID] == 1)
	{
		SetTrustee(wChairID, 0);
	}

	if (cbOutCount <= 0)
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);
		return false;
	}

	//�жϳ����Ƿ�Ϸ�
	if (cbOutCount > m_cbHandCardCount[wChairID])
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);

		LOG(WARNING) << "out card count more than hand card count ?";
		return false;
	}

	for (CT_BYTE i = 0; i < cbOutCount; ++i)
	{
		if (cbOutCard[i] == 0)
		{
			if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
				m_pGameDeskPtr->CloseUserConnect(wChairID);

			LOG(WARNING) << "out card data is empty? ";
			return true;
		}
	}

	/*if (cbOutCard[0] == 0)
	{
		LOG(WARNING) << "card data is empty!";
		return false;
	}*/

	CT_BYTE cbOutCardTemp[MAX_COUNT];
	memcpy(cbOutCardTemp, cbOutCard, cbOutCount);
	m_GameLogic.SortCardList(cbOutCard, cbOutCount, ST_ORDER);

	if (wChairID != m_dwCurrentUser) 
	{
        if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
            m_pGameDeskPtr->CloseUserConnect(wChairID);
		LOG(WARNING) << "OnUserOutCard not this user! current user: " << m_dwCurrentUser << ", out card user: " << wChairID;
		return true;
	}

	//��ȡ����
	CT_BYTE cbCardType = m_GameLogic.GetCardType(cbOutCard, cbOutCount);

	//�����ж�
	if (cbCardType == CT_ERROR)
	{
		//assert(FALSE);
		acl::string strCardData;
        for (CT_BYTE i = 0;  i != cbOutCount; ++i)
        {
            //LOG(ERROR) << std::hex << (int)cbOutCard[i];
            strCardData.format_append("%x ", cbOutCard[i]);
        }
		LOG(ERROR) << "user out card, card type error, card data: " << strCardData.c_str() ;

        if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
            m_pGameDeskPtr->CloseUserConnect(wChairID);
		return true;
	}

	//�����ж�
	if (m_cbTurnCardCount != 0)
	{
		//�Ա��˿�
		if (m_GameLogic.CompareCard(m_cbTurnCardData, cbOutCard, m_cbTurnCardCount, cbOutCount) == false)
		{
			//assert(FALSE);
			LOG(WARNING) << "compare card error.";
            m_pGameDeskPtr->CloseUserConnect(wChairID);
			return false;
		}
	}

	//ɾ���˿�
	if (m_GameLogic.RemoveCardList(cbOutCard, cbOutCount, m_cbHandCardData[wChairID], m_cbHandCardCount[wChairID]) == false)
	{
		//assert(FALSE);
		LOG(WARNING) << "remove card error.";
		return false;
	}
	
	//���Ƽ�¼
	CT_BYTE cbOutCardCount = m_cbOutCardCount[wChairID];
	m_cbOutCardListCount[wChairID][cbOutCardCount] = cbOutCount;
	memcpy(&m_cbOutCardList[wChairID][cbOutCardCount], cbOutCard, cbOutCount);
	//���Ʊ���
	m_cbOutCardCount[wChairID]++;

	//���ñ���
	m_cbTurnCardCount = cbOutCount;
	m_cbHandCardCount[wChairID] -= cbOutCount;
	memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	memcpy(m_cbTurnCardData, cbOutCard, sizeof(CT_BYTE)*cbOutCount);

	//ը���ж�
	if (cbCardType == CT_BOMB_CARD)
	{
		m_cbBombCount++;
		m_dwBombTime *= SCORE_TIME_BOMB;
		m_cbEachBombCount[wChairID]++;
		
		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			m_wEachTotalTime[i] *= SCORE_TIME_BOMB;
		}
	}
	else if (cbCardType == CT_MISSILE_CARD)
	{
		m_cbMissile = 1;
		m_dwBombTime *= SCORE_TIME_BOMB;
		m_cbEachBombCount[wChairID]++;

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			m_wEachTotalTime[i] *= SCORE_TIME_BOMB;
		}
	}

	//�л��û�
	m_dwTurnWiner = wChairID;
	if (m_cbHandCardCount[wChairID] != 0)
	{
		//if (cbCardType != CT_MISSILE_CARD)
		//{
			m_dwCurrentUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
		//}
	}
	else m_dwCurrentUser = INVALID_CHAIR;

	//��������
	CMD_S_OutCard OutCard;
	OutCard.dwOutCardUser = wChairID;
	OutCard.cbCardCount = cbOutCount;
	OutCard.dwCurrentUser = m_dwCurrentUser;
	OutCard.cbTimeLeft = TIME_OUT_CARD;
	memcpy(OutCard.cbCardData, cbOutCardTemp, m_cbTurnCardCount * sizeof(CT_BYTE));
	
	/*for (CT_BYTE i = 0; i < cbOutCount; ++i)
	{
		CT_BYTE cbCardData = cbOutCard[i];
		CT_BYTE cbCardValue = m_GameLogic.GetCardValue(cbCardData);
		CT_BYTE cbIndex = cbCardValue - 1;
		if (cbIndex < 0)
		{
			continue;
		}
		--m_cbRemainCard[cbIndex];
	}*/

	//��������
	CT_DWORD wHeadSize = sizeof(OutCard) - sizeof(OutCard.cbCardData);
	CT_DWORD wSendSize = wHeadSize + OutCard.cbCardCount * sizeof(CT_BYTE);
	for (CT_BYTE i = 0;i < GAME_PLAYER;i++)
	{
		if (i == m_dwBankerUser)
		{
			OutCard.wUserTimes = m_dwBombTime * 2;
            SendTableData(i, SUB_S_OUT_CARD, &OutCard, wSendSize, true);
		}
		else
		{
			OutCard.wUserTimes = m_dwBombTime;
            SendTableData(i, SUB_S_OUT_CARD, &OutCard, wSendSize, false);
		}
	}

	//�������
	//if (cbCardType == CT_MISSILE_CARD) m_cbTurnCardCount = 0;

	//�����ж�
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		OnEventGameEnd(wChairID, GER_NORMAL);
	}
	else
	{
		//�����й�����
		IsTrustee();
	}
	
	return true;
}

//��Ҳ���
CT_BOOL CGameProcess::OnUserPassCard(CT_WORD wChairID)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard :wChairID = " << wChairID ;
	//Ч��״̬
	//assert((wChairID == m_dwCurrentUser) && (m_cbTurnCardCount != 0));
	if ((wChairID != m_dwCurrentUser) || (m_cbTurnCardCount == 0))
	    return false;

	//���ñ���
	m_dwCurrentUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
	if (m_dwCurrentUser == m_dwTurnWiner) m_cbTurnCardCount = 0;

	//������Ϣ
	CMD_S_PassCard PassCard;
	PassCard.dwPassCardUser = wChairID;
	PassCard.dwCurrentUser = m_dwCurrentUser;
	PassCard.cbTurnOver = (m_cbTurnCardCount == 0) ? 1 : 0;
	PassCard.cbTimeLeft = TIME_OUT_CARD;
	//PassCard.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();
	//��������
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), true);
        }
        else
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), false);
        }
	}
	//�й�
	IsTrustee();
	return true;
}

CT_BOOL CGameProcess::OnBankerInfo()
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << " OnBankerInfo: m_dwBankerUser = " << m_dwBankerUser;
	//����״̬
	//m_cbGameStatus = GAME_SCENE_PLAY;
	m_cbGameStatus = GAME_SCENE_DOUBLE;

	//���͵���
	m_cbHandCardCount[m_dwBankerUser] += CountArray(m_cbBankerCard);
	memcpy(&m_cbHandCardData[m_dwBankerUser][NORMAL_COUNT], m_cbBankerCard, sizeof(m_cbBankerCard));

	//�����˿�
	m_GameLogic.SortCardList(m_cbHandCardData[m_dwBankerUser], m_cbHandCardCount[m_dwBankerUser], ST_ORDER);

	//������Ϣ
	CMD_S_BankerInfo BankerInfo;
	BankerInfo.dwBankerUser = m_dwBankerUser;
	BankerInfo.dwCurrentUser = m_dwCurrentUser;
	BankerInfo.cbTimeLeft = TIME_ADD_DOUBLE;
	memcpy(BankerInfo.cbBankerCard, m_cbBankerCard, sizeof(m_cbBankerCard));

	//������Ϣ��ȷ��ÿ����ҵ�ǰ����
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_dwBankerUser)
		{
			m_wEachTotalTime[i] = 2;
            SendTableData(i, SUB_S_BANKER_INFO, &BankerInfo, sizeof(BankerInfo), true);
		}
		else
		{
			m_wEachTotalTime[i] = 1;
            SendTableData(i, SUB_S_BANKER_INFO, &BankerInfo, sizeof(BankerInfo), false);
		}
	}
	return true;
}

CT_BOOL CGameProcess::OnUserCallBanker(CT_WORD wChairID, CT_WORD cbCallInfo)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserCallBanker: wChairID = " << wChairID << ",cbCallInfo = " << cbCallInfo;
	//Ч��״̬
	//assert(wChairID == m_dwCurrentUser);
	if (wChairID != m_dwCurrentUser) 
	{ 
		LOG(WARNING) << "OnUserCallBanker, is not current user. chair id: " << wChairID << ", current user: " << m_dwCurrentUser;
		return false;
	}
		

	//Ч��״̬
	if (CB_NOT_CALL < m_cbCallBankerInfo[wChairID])
	{
		LOG(WARNING) << "OnUserCallBanker, has call banker? banker info: " << (CT_WORD)m_cbCallBankerInfo[wChairID];
		return false;
	}

	//Ч�����
	//assert(cbCallInfo >= CB_NOT_CALL && cbCallInfo <= CB_NO_CALL_BENKER);
	if (cbCallInfo< CB_NOT_CALL || cbCallInfo > CB_NO_CALL_BENKER)
		cbCallInfo = CB_NO_CALL_BENKER;

	CT_DWORD dwMaxCallBankerUser = 0; //��ǰ�ѽе��������������
	CT_WORD cbMaxBankerScore = 0;//��ǰ�ѽе�����������
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_cbCallBankerInfo[i] >= CB_CALL_BENKER_1 && m_cbCallBankerInfo[i] <= CB_CALL_BENKER_3)
		{
			if (cbMaxBankerScore < m_cbCallBankerInfo[i])
			{
				cbMaxBankerScore = m_cbCallBankerInfo[i];
				dwMaxCallBankerUser = i;
			}
		}
	}

	//�ж��Ƿ��Ѿ����������һ����û�не���,������Ĭ�Ͻ�1��
	if (m_cbNoBankCount >= MAX_NO_CALL_BANKER && cbMaxBankerScore == 0 && cbCallInfo == CB_NO_CALL_BENKER && m_dwFirstUser == GetNextUser(wChairID))
	{
		cbCallInfo = CB_CALL_BENKER_1;
	}

	//���з�
	if (cbCallInfo != CB_NO_CALL_BENKER && cbCallInfo <= cbMaxBankerScore)
	{
		LOG(WARNING) << "call banker bigger than mac banker score! call info: " << (CT_WORD)cbCallInfo << ", max banker info: " << (CT_WORD)cbMaxBankerScore;
		return false;
	}

	if (cbCallInfo != CB_NO_CALL_BENKER)
	{
		cbMaxBankerScore = cbCallInfo;
		dwMaxCallBankerUser = wChairID;
	}
	m_cbCallBankerInfo[wChairID] = cbCallInfo;

	//�����û�
	if ((cbCallInfo == CB_CALL_BENKER_3) || (m_dwFirstUser == GetNextUser(wChairID)))
	{
		m_dwCurrentUser = INVALID_CHAIR;
	}
	else
	{
		m_dwCurrentUser = GetNextUser(wChairID);
	}
	m_dwStartTime = cbMaxBankerScore;
	//�������
	CMD_S_CallBanker CallBanker;
	
	CallBanker.cbCallInfo = cbCallInfo;
	//CallBanker.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();
	CallBanker.dwLastUser = wChairID;
	CallBanker.dwBankerUser = INVALID_CHAIR;
	CallBanker.dwCurrentUser = m_dwCurrentUser;
	CallBanker.cbTimeLeft = TIME_CALL_BANKER;

	if ((cbCallInfo == CB_CALL_BENKER_3) || (m_dwFirstUser == GetNextUser(wChairID)))
	{
		//���Ҷ���������,���·��ƣ�������һ�����Ƚе���
		if ((cbMaxBankerScore == 0) && (m_dwFirstUser ==  GetNextUser(wChairID)))
		{
			for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
			{
			    if (m_dwCurrentUser == i)
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), true);
                }
                else
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), false);
			    }
			}

			m_dwFirstUser = GetNextUser(m_dwFirstUser);
			++m_cbNoBankCount;
			OnEventGameStart();	
			return true;
		}
		else
		{
			//����״̬
			m_cbGameStatus = GAME_SCENE_DOUBLE;
			m_dwBankerUser = dwMaxCallBankerUser;
			m_dwCurrentUser = m_dwBankerUser;

			CallBanker.dwBankerUser = m_dwBankerUser;
			CallBanker.dwCurrentUser = m_dwCurrentUser;
			//������Ϣ
			for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
			{
                if (m_dwCurrentUser == i)
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), true);
                }
                else
                {
                    SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), false);
                }
			}

			//����ȷ��
			//LOG(WARNING) << "call isTrustee2!";
			//���ý�ׯ����
			m_cbNoBankCount = 1;
			m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
			IsTrustee();
			return OnBankerInfo();
		}
	}

	//������Ϣ
	for (CT_BYTE i = 0; i < GAME_PLAYER; ++i)
	{
        if (m_dwCurrentUser == i)
        {
            SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), true);
        }
        else
        {
            SendTableData(i, SUB_S_CALL_BANKER, &CallBanker, sizeof(CallBanker), false);
        }
	}
	//�й�
	//LOG(WARNING) << "call isTrustee!";
	m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
	IsTrustee();
	return true;
}

CT_BOOL CGameProcess::OnUserDouble(CT_WORD wChairID, CT_BYTE cbDoubleInfo)
{
	//Ч��״̬
	if (CB_NOT_ADD_DOUBLE != m_cbAddDoubleInfo[wChairID])
	{
		return false;
	}

	if (CB_NOT_ADD_DOUBLE == cbDoubleInfo)
		cbDoubleInfo = CB_NO_ADD_DOUBLE;

	m_cbAddDoubleInfo[wChairID] = cbDoubleInfo;

	//ȷ���������������������ӱ���
	/*if (m_cbAddDoubleInfo[wChairID] == CB_ADD_DOUBLE)
	{
		m_wEachTotalTime[wChairID] += 1;
		m_wEachTotalTime[m_dwBankerUser] += 1;
	}*/

	CMD_S_Double DoubleInfo;
	DoubleInfo.dwCurrentUser = wChairID;
	DoubleInfo.cbDouble = cbDoubleInfo;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		//���������ָ���ӱ�����ı���
		if (i == m_dwBankerUser)
		{
			DoubleInfo.wUserTimes = 2;
            SendTableData(i, SUB_S_ADD_DOUBLE, &DoubleInfo, sizeof(DoubleInfo), true);
		}
		else
		{
			DoubleInfo.wUserTimes = 1;
            SendTableData(i, SUB_S_ADD_DOUBLE, &DoubleInfo, sizeof(DoubleInfo), false);
		}
		//DoubleInfo.wUserTimes = m_wEachTotalTime[i];
		//������Ϣ
	}

	//�Ƿ�ʼ
	bool bAddEnd = true;
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_dwBankerUser)
		{
			continue;
		}

		if (CB_NOT_ADD_DOUBLE == m_cbAddDoubleInfo[i])
		{
			bAddEnd = false;
			break;
		}
	}

	if (bAddEnd)
	{
		OnSubOutCardStart();
	}

	return true;
}

//���ƿ�ʼ
CT_BOOL CGameProcess::OnSubOutCardStart()
{
	m_cbGameStatus = GAME_SCENE_PLAY;

	//������Ϣ
	CMD_S_StartOutCard startOutCard;
	startOutCard.dwBankerUser = m_dwBankerUser;
	startOutCard.dwCurrentUser = m_dwCurrentUser;
	startOutCard.cbLeftTime = TIME_FIRST_OUT_CARD;
	
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		if (i == m_dwBankerUser)
			SendTableData(i, SUB_S_OUT_START_START, &startOutCard, sizeof(startOutCard), true);
		else
			SendTableData(i, SUB_S_OUT_START_START, &startOutCard, sizeof(startOutCard), false);
	}

	//ɾ���Զ��ӱ���ʱ��
	m_pGameDeskPtr->KillGameTimer(IDI_ADD_DOUBLE);

	//�趨һ��4S����ƵĶ�ʱ������Ϊ�ͻ���Ҫ��ʾȷ��ׯ����Ϣ��
	if (m_cbTrustee[m_dwCurrentUser])
	{
		m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, 2000);
	}
	else
	{
		m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (TIME_FIRST_OUT_CARD + TIME_NETWORK_COMPENSATE) * 1000);
	}

	return true;
}

CT_WORD CGameProcess::GetUserCallBanker()
{
	CT_WORD cbMaxBenkerScore = 0;
	CT_WORD wBenker = 0;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_cbCallBankerInfo[i] >= CB_CALL_BENKER_1 && m_cbCallBankerInfo[i] <= CB_CALL_BENKER_3)
		{
			if (cbMaxBenkerScore < m_cbCallBankerInfo[i])
			{
				cbMaxBenkerScore = m_cbCallBankerInfo[i];
				wBenker = i + 1;
			}
		}
	}
	return wBenker;
}

//�����һ�����
CT_DWORD CGameProcess::GetNextUser(CT_DWORD dwChairID)
{
	return (dwChairID + 1) % GAME_PLAYER;
}

CT_BOOL CGameProcess::IsTrustee(bool bIsNewTurn/* = true*/)
{
	//�ǳ�����λ
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		LOG(WARNING) << "set trustee, but current user is invalid!";
		return false;
	}

	//���������
	//if (m_cbTrustee[m_dwCurrentUser] != 1) return false;


	//ɾ����ʱ��ʱ��
	//m_pGameDeskPtr->KillGameTimer(IDI_CHECK_TRUSTEE);

	//CT_DWORD dwTimerTime = 1;
	//m_pGameDeskPtr->SetGameTimer(IDI_CHECK_TRUSTEE, dwTimerTime * 1000);

	//���ݲ�ͬ״̬���ò�ͬ�Ķ�ʱ��
	switch (m_cbGameStatus)
	{
	case GAME_SCENE_DOUBLE:
	{
		if (!m_pGameDeskPtr->IsHasGameTimer(IDI_ADD_DOUBLE))
		{
			m_pGameDeskPtr->SetGameTimer(IDI_ADD_DOUBLE, (TIME_ADD_DOUBLE + TIME_NETWORK_COMPENSATE) * 1000);
			m_cbOpTotalTime = TIME_ADD_DOUBLE;
			m_dwOpStartTime = (CT_DWORD)time(NULL);
		}
	}
	break;
	case GAME_SCENE_CALL:
	{
		//LOG(WARNING) << "set IDI_CALL_BANKER timer!";
		m_pGameDeskPtr->KillGameTimer(IDI_CALL_BANKER);
		if (m_cbTrustee[m_dwCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_CALL_BANKER, 1000);
		}
		else
		{
			if (m_dwCurrentUser == m_dwFirstUser)
			{
				m_pGameDeskPtr->SetGameTimer(IDI_CALL_BANKER, (TIME_CALL_BANKER + TIME_DEAL_CARD + TIME_NETWORK_COMPENSATE) * 1000);
			}
			else
			{
				m_pGameDeskPtr->SetGameTimer(IDI_CALL_BANKER, (TIME_CALL_BANKER + TIME_NETWORK_COMPENSATE) * 1000);
			}
		}
		m_cbOpTotalTime = TIME_CALL_BANKER;
		m_dwOpStartTime = (CT_DWORD)time(NULL);
	}
	break;
	case GAME_SCENE_PLAY:
	{
		m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
		if (m_cbTrustee[m_dwCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, 2000);
		}
		else
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (TIME_OUT_CARD + TIME_NETWORK_COMPENSATE) * 1000);
		}
		if(bIsNewTurn)
        {
            m_cbOpTotalTime = TIME_OUT_CARD;
            m_dwOpStartTime = (CT_DWORD)time(NULL);
        }
	}
	break;
	default:
		break;
	}

	return true;
}

CT_VOID CGameProcess::SetTrustee(CT_DWORD dwChairID, CT_BYTE cbTrustee)
{
	if (dwChairID == INVALID_CHAIR)
	{
		return;
	}

	//������Ϣ
	m_cbTrustee[dwChairID] = cbTrustee;
	/*//��ʱ���λ����˵��й��·�
	if (m_pGameDeskPtr->IsAndroidUser(dwChairID))
	{
		return;
	}*/

	CMD_S_StatusTrustee StatusTrustee;
	memcpy(StatusTrustee.cbTrustee, m_cbTrustee, sizeof(m_cbTrustee));
	//��������
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
        }
        else
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
        }
	}
}

CT_BOOL CGameProcess::OnAutoCallLand()
{
	//��ǰ�ѽе�����������
	/*CT_WORD cbMaxBenkerScore = 0;
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_cbCallBankerInfo[i] >= CB_CALL_BENKER_1 && m_cbCallBankerInfo[i] <= CB_CALL_BENKER_3)
		{
			if (cbMaxBenkerScore < m_cbCallBankerInfo[i])
			{
				cbMaxBenkerScore = m_cbCallBankerInfo[i];
			}
		}
	}
	//�ж��Ƿ�е���
	int bCallBanker = rand() % 2;
	if (bCallBanker)
	{
		return OnUserCallBanker(m_dwCurrentUser, cbMaxBenkerScore + 1);
	}
	else
	{
		return OnUserCallBanker(m_dwCurrentUser, CB_NO_CALL_BENKER);
	}*/

	//if (m_cbNoBankCount >= 2 && m_dwCurrentUser == m_dwFirstUser)
		//return OnUserCallBanker(m_dwCurrentUser, CB_CALL_BENKER_1);
	//else
	return OnUserCallBanker(m_dwCurrentUser, CB_NO_CALL_BENKER);
}

CT_BOOL CGameProcess::OnAutoDouble(CT_DWORD dwChairID)
{
	//Ĭ�ϲ���
	return OnUserDouble(dwChairID, CB_NO_ADD_DOUBLE);
}

CT_BOOL CGameProcess::OnAutoOutCard()
{
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return true;
	}

	//if (0 == m_cbTurnCardCount)
	//{//��һ������
	//	CT_BYTE cbCardCount = m_cbHandCardCount[m_dwCurrentUser];
	//	CT_BYTE cbCard[MAX_COUNT] = { 0 };
	//	cbCard[0] = m_cbHandCardData[m_dwCurrentUser][cbCardCount - 1];
	//	return OnUserOutCard(m_dwCurrentUser, cbCard, 1) ;
	//}
	//else
	{
		//����
		//return OnUserPassCard(m_dwCurrentUser) ;

		//���ը��ֱ�ӹ���
		if (m_cbTurnCardCount == 2)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			if (cbCardType == CT_MISSILE_CARD)
			{
				return OnUserPassCard(m_dwCurrentUser);
			}
		}

		//������ʾ
		tagSearchCardResult	SearchCardResult;					//�������
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		try
		{
			m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount,
				&SearchCardResult);
		}
		catch (...)
		{
			SearchCardResult.cbSearchCount = 0;
		}

		if (SearchCardResult.cbSearchCount > 0)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			CT_BYTE cbBestCardIndex = 0;
			for (CT_BYTE i = 0 ; i != SearchCardResult.cbSearchCount; ++i)
			{
				CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[i], SearchCardResult.cbCardCount[i]);
				if (cbCardType != CT_ERROR)
				{
					//��ͬ���ͣ�ֱ�ӳ���
					if (cbSearchCardType == cbCardType)
					{
						CT_BYTE cbLastBestCardIndex = cbBestCardIndex;
						cbBestCardIndex = i;
						CT_BOOL bFindBestIndex = true;

						//������Ҫ��ը������
						if (cbSearchCardType != CT_BOMB_CARD && cbCardType != CT_BOMB_CARD)
						{
							CT_BOOL bBreak = false;
							//�ҳ�ը��
							for (CT_BYTE j = 0; j < SearchCardResult.cbSearchCount; ++j)
							{
								if (j == i || SearchCardResult.cbCardCount[j] != 4)
								{
									continue;
								}

								CT_BYTE cbCardTypeEx = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[j], SearchCardResult.cbCardCount[j]);
								if (cbCardTypeEx == CT_BOMB_CARD)
								{
									CT_BYTE cbCardValue = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[j][0]);
									for (CT_BYTE m = 0; m < SearchCardResult.cbCardCount[i]; ++m)
									{
										CT_BYTE cbCardValueEx = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[i][m]);
										if (cbCardValue == cbCardValueEx)
										{
											cbBestCardIndex = cbLastBestCardIndex;
											bBreak = true;
											bFindBestIndex = false;
											break;
										}
									}
								}

								if (bBreak)
								{
									break;
								}
							}
						}

						//�Ѿ��ҵ���õ���
						if (bFindBestIndex)
						{
							break;
						}
						
					}
				}
				else
				{
					//�ҳ������
					if (SearchCardResult.cbCardCount[i] > SearchCardResult.cbCardCount[cbBestCardIndex])
					{
						//�Լ�����������ը
						if (m_cbTurnCardCount == 0 && cbSearchCardType == CT_BOMB_CARD && m_cbHandCardCount[m_dwCurrentUser] != 4)
						{
							continue;
						}

						CT_BYTE cbLastBestCardIndex = cbBestCardIndex;
						cbBestCardIndex = i;

						//������Ҫ��ը������
						if (cbSearchCardType != CT_BOMB_CARD && cbCardType != CT_BOMB_CARD)
						{
							CT_BOOL bBreak = false;
							//�ҳ�ը��
							for (CT_BYTE j = 0; j < SearchCardResult.cbSearchCount; ++j)
							{
								if (j == i || SearchCardResult.cbCardCount[j] != 4)
								{
									continue;
								}

								CT_BYTE cbCardTypeEx = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[j], SearchCardResult.cbCardCount[j]);
								if (cbCardTypeEx == CT_BOMB_CARD)
								{
									CT_BYTE cbCardValue = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[j][0]);
									for (CT_BYTE m = 0; m < SearchCardResult.cbCardCount[i]; ++m)
									{
										CT_BYTE cbCardValueEx = m_GameLogic.GetCardValue(SearchCardResult.cbResultCard[i][m]);
										if (cbCardValue == cbCardValueEx)
										{
											cbBestCardIndex = cbLastBestCardIndex;
											bBreak = true;
											break;
										}
									}
								}

								if (bBreak)
								{
									break;
								}
							}
						}
					}	
				}
			}
			CT_BYTE cbCardData[MAX_COUNT] = { 0 };
			CT_BYTE cbCardCount = SearchCardResult.cbCardCount[cbBestCardIndex];
			memcpy(cbCardData, &SearchCardResult.cbResultCard[cbBestCardIndex], sizeof(CT_BYTE)*cbCardCount);
			//�����˿�
			m_GameLogic.SortCardList(cbCardData, cbCardCount, ST_ORDER);
			return OnUserOutCard(m_dwCurrentUser, cbCardData, cbCardCount);
		}
		else
		{
			if (m_cbTurnCardCount == 0)
			{
				LOG(WARNING) << "user auto pass, but last user is him?  fuck!!!";
			}
			return OnUserPassCard(m_dwCurrentUser);
		}
	}
}

//�����
CT_BOOL CGameProcess::MatchGoodCard(CT_BYTE cbRandCard[], CT_BYTE cbRandCount)
{
    //CT_BOOL bConfigCard[GAME_PLAYER] = {false};
    //CT_BYTE cbConfigCount = 0;
    CT_WORD wChairID = INVALID_CHAIR;
    //�����������ӮǮ����7�����������
	CT_INT32 iRand = rand()%100;
	for (int j = 0; j < GAME_PLAYER; ++j)
	{
		if(m_pGameDeskPtr->IsAndroidUser(j)) continue;
		GS_UserBaseData* pUserBaseData = m_pGameDeskPtr->GetUserBaseData(j);
		if(NULL == pUserBaseData) continue;
		auto it = m_mapChannelControl.find(pUserBaseData->dwchannelID);
		if((it != m_mapChannelControl.end()) && (iRand < it->second.wBadCardRation))
		{
			wChairID = j;
		}
		else if(((pUserBaseData->llTotalWinScore >= 700) && (iRand < 30))|| (pUserBaseData->llTotalWinScore >= 900))
		{
			wChairID = j;
		}
	}
    CT_BYTE cbTempCard[FULL_COUNT]={0};
    memcpy(cbTempCard, cbRandCard,CountArray(cbTempCard));

	/*if(INVALID_CHAIR != wChairID)
    {
        for (CT_WORD j = 0; j < GAME_PLAYER; ++j)
        {
            if(wChairID != j)
            {
                bConfigCard[j] = true;
                cbConfigCount++;
            }
        }
    }
    //ֻ�е�Ԫ���ֵ���1�ĳ��βŻ������
    if (cbConfigCount > 0)
    {
		memset(cbRandCard,0,cbRandCount);
		CT_BYTE cbGoodCard[GOOD_CARD_COUTN];
		memset(cbGoodCard,0,CountArray(cbGoodCard));
		m_GameLogic.GetGoodCardData(cbGoodCard, GOOD_CARD_COUTN);
		//��ȡ����
		m_GameLogic.RemoveGoodCardData(cbGoodCard, GOOD_CARD_COUTN, cbTempCard, FULL_COUNT);
		int iGoodIndex = 0;
		int iBadIndex = 0;

		CT_BYTE cbGoodCount = ((2 == cbConfigCount) ? (GOOD_CARD_MAX-1) : GOOD_CARD_MAX);

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			int iNeedGoodCount = 0;
			//����������
			if(3 == cbConfigCount)
			{
				iNeedGoodCount = (GOOD_CARD_MAX-1);
			}
			else if (bConfigCard[i])
			{
				//�ַ��˿�
				iNeedGoodCount = cbGoodCount;

			}
			else if (2 == cbConfigCount)
			{
				//һ��������
				iNeedGoodCount = (GOOD_CARD_COUTN-2*cbGoodCount)-1;
			}
			else if(1 == cbConfigCount)
			{
				//2��������
				//�ַ��˿�
				iNeedGoodCount = (GOOD_CARD_COUTN-cbGoodCount)/2;
			}
			//�ַ��˿�
			memcpy(&cbRandCard[i*NORMAL_COUNT], &cbGoodCard[iGoodIndex], iNeedGoodCount);
			iGoodIndex += iNeedGoodCount;
			memcpy(&cbRandCard[i*NORMAL_COUNT + iNeedGoodCount], &cbTempCard[iBadIndex], NORMAL_COUNT-iNeedGoodCount);
			iBadIndex += (NORMAL_COUNT-iNeedGoodCount);
		}
        //����
        if((GOOD_CARD_COUTN-iGoodIndex) > 0)
        {
            memcpy(&cbRandCard[3*NORMAL_COUNT], &cbGoodCard[iGoodIndex], GOOD_CARD_COUTN-iGoodIndex);
        }

        memcpy(&cbRandCard[3*NORMAL_COUNT+(GOOD_CARD_COUTN-iGoodIndex)], &cbTempCard[iBadIndex], FULL_COUNT-GOOD_CARD_COUTN-iBadIndex);

    }*/
    if(INVALID_CHAIR != wChairID)
    {
        memcpy(cbTempCard, &cbRandCard[wChairID*NORMAL_COUNT], NORMAL_COUNT);
        tagAnalyseResult AnalyseResult;
        memset(&AnalyseResult, 0, sizeof(AnalyseResult));
        m_GameLogic.SortCardList(cbTempCard, NORMAL_COUNT, ST_ORDER);
        m_GameLogic.AnalysebCardData(cbTempCard,NORMAL_COUNT,AnalyseResult);
		CT_BYTE cbIndex = 0;
        if(AnalyseResult.cbBlockCount[3] > 0)
        {

            int iCount = 0;
            for (int i = 0; i < NORMAL_COUNT && iCount < 2; ++i)
            {
                CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
                if(m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[3][0]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && (cbIndex < NORMAL_COUNT))
                {

                    CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                    cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex];
                    cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex] = cbTemp;
                    iCount++;
					cbIndex++;
                }
            }
        }
        for (int k = 0; k < AnalyseResult.cbBlockCount[2]; ++k)
        {
            CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
            for (int i = 0; i < NORMAL_COUNT; ++i)
            {
                if((m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) > 7) && m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i])&& (cbIndex < NORMAL_COUNT))
                {
                    CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                    cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex];
                    cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex] = cbTemp;
					cbIndex++;
                }
            }
        }
        int i2Count = 0;
        int iWangCount = 0;
        int iACount = 0;
        for (int j = 0; j < NORMAL_COUNT; ++j)
        {
            if(m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+j]) == m_GameLogic.GetCardLogicValue(0x02))
            {
                i2Count++;
            }
            if(m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+j]) == m_GameLogic.GetCardLogicValue(0x01))
            {
                iACount++;
            }
            //0x4E,0x4F,
            if(cbRandCard[wChairID*NORMAL_COUNT+j] == 0x4E || cbRandCard[wChairID*NORMAL_COUNT+j]==0x4F)
            {
                iWangCount++;
            }
        }

		for (int i = 0; (i < NORMAL_COUNT) && (iACount > 2); ++i)
		{
			CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
			if(iACount > 2 && (m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) == m_GameLogic.GetCardLogicValue(0x01))&& (cbIndex < NORMAL_COUNT))
			{
				CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
				cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex];
				cbRandCard[wTmpChaird*NORMAL_COUNT+cbIndex] = cbTemp;
				iACount--;
				cbIndex++;
			}
		}
		for (int i = 0; (i < NORMAL_COUNT) && (i2Count > 2); ++i)
		{
			CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
			if(i2Count > 2 && (m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) == m_GameLogic.GetCardLogicValue(0x02))&& (cbIndex < NORMAL_COUNT))
			{
                for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT && (i2Count > 2); ++j)
                {
                    if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                    {
                        CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                        cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                        cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                        i2Count--;
                        cbIndex++;
                        break;
                    }
                }
			}

		}
		for (int i = 0; (i < NORMAL_COUNT) && (iWangCount > 0); ++i)
		{
			CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
			if(iWangCount > 0 && (cbRandCard[wChairID*NORMAL_COUNT+i]==0x4F||cbRandCard[wChairID*NORMAL_COUNT+i] == 0x4E)&& (cbIndex < NORMAL_COUNT))
			{
                for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT && (iWangCount > 0); ++j)
                {
                    if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                    {
                        CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                        cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                        cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                        iWangCount--;
                        cbIndex++;
                        break;
                    }
                }

			}
		}
        //����
        tagSearchCardResult tmpSearchCardResult;
        CT_BYTE cbTmpCount = m_GameLogic.SearchLineCardType(cbTempCard, NORMAL_COUNT, 0, 1, 0, &tmpSearchCardResult);
        if (cbTmpCount > 0)
        {
            CT_BYTE cbCard = tmpSearchCardResult.cbResultCard[0][2];
            for (int i = 0; i < NORMAL_COUNT; ++i)
            {
                CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
                if(m_GameLogic.GetCardLogicValue(cbCard) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && (cbIndex < NORMAL_COUNT))
                {
                    for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT; ++j)
                    {
                        if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                        {
                            CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                            cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                            cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                            cbIndex++;
                            break;
                        }
                    }
                }
            }
        }

        memcpy(cbTempCard, &cbRandCard[wChairID*NORMAL_COUNT], NORMAL_COUNT);
        memset(&AnalyseResult, 0, sizeof(AnalyseResult));
        m_GameLogic.SortCardList(cbTempCard, NORMAL_COUNT, ST_ORDER);
        m_GameLogic.AnalysebCardData(cbTempCard,NORMAL_COUNT,AnalyseResult);
        if(AnalyseResult.cbBlockCount[3] > 0)
        {

            int iCount = 0;
            for (int i = 0; i < NORMAL_COUNT && iCount < 2; ++i)
            {
                CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
                if(m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[3][0]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && (cbIndex < NORMAL_COUNT))
                {
                    for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT; ++j)
                    {
                        if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                        {
                            CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                            cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                            cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                            cbIndex++;
                            break;
                        }
                    }
                }
            }
        }
        for (int k = 0; k < AnalyseResult.cbBlockCount[2]-1; ++k)
        {
            CT_WORD wTmpChaird = (wChairID+((rand()%2)+1))%GAME_PLAYER;
            for (int i = 0; i < NORMAL_COUNT; ++i)
            {
                if((m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) > 7) && m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][k*3]) == m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i])&& (cbIndex < NORMAL_COUNT))
                {
                    for (int j = cbIndex; j < NORMAL_COUNT && cbIndex < NORMAL_COUNT; ++j)
                    {
                        if(m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) != m_GameLogic.GetCardLogicValue(cbRandCard[wChairID*NORMAL_COUNT+i]) && m_GameLogic.GetCardLogicValue(cbRandCard[wTmpChaird*NORMAL_COUNT+j]) < 13)
                        {
                            CT_BYTE cbTemp = cbRandCard[wChairID*NORMAL_COUNT+i];
                            cbRandCard[wChairID*NORMAL_COUNT+i] = cbRandCard[wTmpChaird*NORMAL_COUNT+j];
                            cbRandCard[wTmpChaird*NORMAL_COUNT+j] = cbTemp;
                            cbIndex++;
                            break;
                        }
                    }
                }
            }
        }
    }
    return true;
}



