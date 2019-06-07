
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
#include "../logic_ddz_score/MSG_DDZ.h"

//��ӡ��־
#define		PRINT_LOG_INFO						1
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//#define IDI_CHECK_TABLE			100					//�������
//#define IDI_CHECK_TRUSTEE			101					//�����й�

//#define IDI_AUTO_READY				200					//�Զ�׼��

#define IDI_OUT_CARD				500					//����
#define IDI_TICK_USER				600					//�޳��û�
#define IDI_CHECK_END				700					//�����Ϸ����

//#define	TIME_AUTO_READY				16					//�Զ�׼��ʱ��(15)
#define TIME_TICK_USER				1					//�޳��û�ʱ��
#define TIME_CHECK_END				600					//������ӽ���
#define TIME_TRUSTEE				1					//������ӽ���
//���粹��ʱ��
#define TIME_NETWORK_COMPENSATE     1					//���粹��

//��ֵ����
#define SCORE_TIME_BOMB				5					//ը������
#define SCORE_TIME_CHUANTIAN		2					//���챶��
#define SCORE_TIME_FANCHUAN			2					//��������

#define TIME_OUT_TRUSTEE_COUNT		0					//��ʱ�����йܴ���

////////////////////////////////////////////////////////////////////////
CGameProcess::CGameProcess(void) 
	:m_dwlCellScore(1), m_pGameDeskPtr(NULL), m_pGameRoomKindInfo(NULL)
{
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

	//��¼��ʼ��Ϸÿ���˵ķ���
    for (int k = 0; k < GAME_PLAYER; ++k) {
        m_RecordScoreInfo[k].llSourceScore = m_pGameDeskPtr->GetUserScore(k);
    }

    //double dScore = m_pGameDeskPtr->GetUserScore(0)*TO_DOUBLE;
    //LOG(INFO) << "Startmoney_1: "<< m_pGameDeskPtr->GetUserScore(0) <<", 2: "<<m_pGameDeskPtr->GetUserScore(1) << ", 3: " << m_pGameDeskPtr->GetUserScore(2);
    //LOG(INFO) << "Startmoney_1: "<< dScore <<", 2: "<<(m_pGameDeskPtr->GetUserScore(1)*TO_DOUBLE) << ", 3: " << (m_pGameDeskPtr->GetUserScore(2)*TO_DOUBLE);
    //������Ϸ״̬
	m_cbGameStatus = GAME_SCENE_PLAY;
	m_pGameDeskPtr->SetGameTimer(IDI_CHECK_END, (TIME_CHECK_END)*1000);

	//�����˿�m_cbBombCount
	CT_BYTE cbRandCard[FULL_COUNT];
	if (m_pGameRoomKindInfo->wRoomKindID == PRIMARY_ROOM)//��������%70���ʳ���ը��
	{
		/*CT_DWORD dwFirstUserID = m_pGameDeskPtr->GetUserID(0);
		srand((CT_DWORD)time(NULL) + dwFirstUserID);
		int nRand = rand() % 10;
		CT_BYTE cbBomCount = 0;
		if (nRand < 7)
		{
			cbBomCount = 1;
		}*/

		m_GameLogic.RandCardData(cbRandCard, CountArray(cbRandCard));
	
	}
	else
	{
		m_GameLogic.RandCardData(cbRandCard, CountArray(cbRandCard));
	}

	if (FREE_ROOM == m_pGameRoomKindInfo->wRoomKindID)
	{
		MatchGoodCard(cbRandCard, CountArray(cbRandCard));
	}

	/*
	cbRandCard[0] = 0x21;
    cbRandCard[1] = 0x11;
    cbRandCard[2] = 0x01;
    cbRandCard[3] = 0x2d;
    cbRandCard[4] = 0x1c;
    cbRandCard[5] = 0x3b;
    cbRandCard[6] = 0x0b;
    cbRandCard[7] = 0x29;
    cbRandCard[8] = 0x38;
    cbRandCard[9] = 0x28;
    cbRandCard[10] = 0x18;
    cbRandCard[11] = 0x08;
    cbRandCard[12] = 0x27;
    cbRandCard[13] = 0x17;
    cbRandCard[14] = 0x07;
    cbRandCard[15] = 0x04;

    cbRandCard[16] = 0x1d;
    cbRandCard[17] = 0x2c;
    cbRandCard[18] = 0x0c;
    cbRandCard[19] = 0x1b;
    cbRandCard[20] = 0x1a;
    cbRandCard[21] = 0x0a;
    cbRandCard[22] = 0x09;
    cbRandCard[23] = 0x36;
    cbRandCard[24] = 0x26;
    cbRandCard[25] = 0x06;
    cbRandCard[26] = 0x35;
    cbRandCard[27] = 0x25;
    cbRandCard[28] = 0x05;
    cbRandCard[29] = 0x23;
    cbRandCard[30] = 0x13;
    cbRandCard[31] = 0x03;

    cbRandCard[32] = 0x32;
    cbRandCard[33] = 0x3d;
    cbRandCard[34] = 0x0d;
    cbRandCard[35] = 0x3c;
    cbRandCard[36] = 0x2b;
    cbRandCard[37] = 0x3a;
    cbRandCard[38] = 0x2a;
    cbRandCard[39] = 0x39;
    cbRandCard[40] = 0x19;
    cbRandCard[41] = 0x37;
    cbRandCard[42] = 0x16;
    cbRandCard[43] = 0x15;
    cbRandCard[44] = 0x34;
    cbRandCard[45] = 0x24;
    cbRandCard[46] = 0x14;
    cbRandCard[47] = 0x33;
	*/
	//�û��˿�
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
		m_cbHandCardCount[i] = NORMAL_COUNT;
		memcpy(&m_cbHandCardData[i], &cbRandCard[i*m_cbHandCardCount[i]], sizeof(CT_BYTE)*m_cbHandCardCount[i]);
        for (int j = 0; (j < NORMAL_COUNT)&&(INVALID_CHAIR == m_dwCurrentUser); ++j)
        {
            if (0x23==m_cbHandCardData[i][j])
            {
                m_dwCurrentUser = i;
            }
        }
	}
	if (m_dwCurrentUser >= GAME_PLAYER)
    {
		LOG(ERROR) <<"deal error";
        return;
    }
	m_dwBankerUser = m_dwCurrentUser;
	//�������
	CMD_S_GameStart GameStart;
	memset(&GameStart,0, sizeof(GameStart));
	GameStart.dwCurrentUser = m_dwCurrentUser;
	GameStart.cbTimeLeft = TIME_DEAL_CARD+TIME_FIRST_OUT_CARD;
	
	//��Ϸ��¼�Ŀ��г�������
    CMD_S_StatusFree StatusFree;
    memset(&StatusFree, 0, sizeof(CMD_S_StatusFree));
    StatusFree.dCellScore = m_dwlCellScore*TO_DOUBLE;
    m_pGameDeskPtr->WriteGameSceneToRecord(&StatusFree, sizeof(StatusFree), SC_GAMESCENE_FREE);
    
	bool bHasAndroid = false;
	//��������
	for (CT_DWORD i = 0; i<GAME_PLAYER; i++)
	{
        GameStart.dwStartUser = i;
		//�����˿�
	//	assert(CountArray(GameStart.cbCardData) >= m_cbHandCardCount[i]);
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
		m_GameLogic.SortCardList(m_cbHandCardData[i], m_cbHandCardCount[i], ST_DESC);

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

		GameStartAi.dwCurrentUser = m_dwCurrentUser;

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
    m_cbOpTotalTime = GameStart.cbTimeLeft + TIME_NETWORK_COMPENSATE;
    m_dwOpStartTime = (CT_DWORD)time(NULL);
	//�й�
	IsTrustee(m_cbOpTotalTime);
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

	//��ǰʱ��
	std::string strTime = Utility::GetTimeNowString();

	//��Ϸ����
	m_pGameDeskPtr->ConcludeGame(GAME_SCENE_FREE, strTime.c_str());

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
            StatusFree.dCellScore = m_dwlCellScore*TO_DOUBLE;
			StatusFree.dwCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

			//��������
			SendTableData(dwChairID, SC_GAMESCENE_FREE, &StatusFree, sizeof(StatusFree),false);
			break;
		}
		case GAME_SCENE_PLAY:	//��Ϸ״̬
		{
			//��������
			CMD_S_StatusPlay StatusPlay;
			memset(&StatusPlay, 0, sizeof(StatusPlay));
			//��Ԫ����
			StatusPlay.dCellScore = m_dwlCellScore*TO_DOUBLE;
			StatusPlay.wFirstOutChairID = (CT_WORD)m_dwBankerUser;
			//ʣ��ʱ�� 
			CT_DWORD dwNow = (CT_DWORD)time(NULL);
			CT_SHORT sLeftTime = m_cbOpTotalTime - (dwNow - m_dwOpStartTime);
			StatusPlay.cbLeftTime = sLeftTime > 0 ? sLeftTime : 0;

			//������Ϣ
			StatusPlay.wTurnWiner = m_dwTurnWiner;
			StatusPlay.cbTurnCardCount = m_cbTurnCardCount;
			memcpy(StatusPlay.cbTurnCardData, m_cbTurnCardData, m_cbTurnCardCount * sizeof(CT_BYTE));

			//��Ϸ����
			StatusPlay.dwCurrentUser = m_dwCurrentUser;



			//�˿���Ϣ
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
			StatusEnd.dCellScore = m_dwlCellScore*TO_DOUBLE;
			//StatusEnd.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
			//ʣ��ʱ��
			CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
			StatusEnd.cbTimeLeave = (CT_BYTE)(m_cbOpTotalTime - std::min<CT_BYTE>(dwPassTime, m_cbOpTotalTime));
			//�˿���Ϣ
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
		LOG(ERROR) << "Timer triggers the end of the game,";
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


	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return;
	}
    CT_DWORD dwTime = time(NULL);
	//�е���״̬�������йܣ���Ϸ״̬��ʱ1���й�
	if (m_cbGameStatus == GAME_SCENE_PLAY &&
		m_cbTrustee[m_dwCurrentUser] != 1
		&& (!m_bIsPassCard) && ((m_cbOpTotalTime-1)<= (int)(dwTime-m_dwOpStartTime)))
	{
		SetTrustee(m_dwCurrentUser, 1);
        //LOG(ERROR) << "IDI_OUT_CARD SetTrustee" ;
    }
    //LOG(WARNING) << "IDI_OUT_CARD " << (int)m_cbOpTotalTime << " " << (itime-m_dwOpStartTime)<< " m_cbTrustee[m_dwCurrentUser] " << (int)m_cbTrustee[m_dwCurrentUser] << " m_bIsPassCard " << m_bIsPassCard;
	CT_BOOL bSystemSucceed = true;
	switch (dwTimerID)
	{
	case IDI_OUT_CARD:
	{
        ++m_cbTimeOutCount[m_dwCurrentUser];
		//LOG(WARNING) << "ON IDI_OUT_CARD TIMER";
        if(m_cbTrustee[m_dwCurrentUser] || (m_bIsPassCard))
        {
            bSystemSucceed = OnAutoOutCard();
        } else{
            //LOG(ERROR) << "IDI_OUT_CARD " << (int)m_cbOpTotalTime << " "<< (itime-m_dwOpStartTime);
            bSystemSucceed = false;
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
    bool bRet = true;
	switch (dwSubCmdID)
	{
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
            bRet = OnUserOutCard(wChairID, pOutCard->cbCardData, pOutCard->cbCardCount);
            break;
		}
		case SUB_C_PASS_CARD:	//�û�����
		{
			//״̬Ч��
			//assert(m_cbGameStatus == GAME_SCENE_PLAY);
			if (m_cbGameStatus != GAME_SCENE_PLAY) return true;

			//��Ϣ����
            bRet = OnUserPassCard(wChairID);
            break;
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
			/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
				SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
			}*/
			SendTableData(INVALID_CHAIR, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
			if (m_dwCurrentUser == wChairID)
			{
				IsTrustee(0);
			}
			//LOG(ERROR) << "SUB_C_TRUSTEE" << " m_cbTrustee["<<(int)pTrustee->wChairID<<"]=" << (int)m_cbTrustee[pTrustee->wChairID];
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
			/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
			{
				SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
			}*/

            SendTableData(INVALID_CHAIR, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
            //LOG(ERROR) << "SUB_C_CANCEL_TRUSTEE" << " m_cbTrustee["<<(int)pCancelTrustee->wChairID<<"]=" << (int)m_cbTrustee[pCancelTrustee->wChairID];
            if(pCancelTrustee->wChairID==m_dwCurrentUser)
            {
                time_t tTime = time(NULL);
                if((m_cbOpTotalTime - (tTime-m_dwOpStartTime)) > 1)
                {
                    m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
                    m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (m_cbOpTotalTime - (tTime-m_dwOpStartTime)) * 1000);
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
	if (!bRet)
    {
        m_pGameDeskPtr->CloseUserConnect(wChairID);
    }
	return bRet;
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
	//�������
	srand((unsigned)time(NULL));
	//ϵͳʱ��
	m_dwSysTime = 0;

	//��Ϸ״̬
	m_cbGameStatus = GAME_SCENE_FREE;							
	//�׽��û�
/*	m_dwFirstUser = INVALID_CHAIR;*/
	//��ǰ���
	m_dwCurrentUser = INVALID_CHAIR;
	m_dwBankerUser = INVALID_CHAIR;
	//ը������
	m_dwBombChairID = INVALID_CHAIR;

	m_cbOpTotalTime = 0;
	m_dwOpStartTime = 0;

	memset(m_cbEachBombCount, 0, sizeof(m_cbEachBombCount));
	memset(m_dEachBombScore, 0, sizeof(m_dEachBombScore));
    memset(m_dEachBombRevenue, 0, sizeof(m_dEachBombRevenue));

	//��������
	m_dwlCellScore = 100;
	

	//���챶��
	//memset(m_dwChunTianbTime, 1, sizeof(m_dwChunTianbTime));
	//��������
	memset(m_cbOutCardCount, 0, sizeof(m_cbOutCardCount));


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
	m_bIsPassCard = false;
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

	PDK_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));

    GameEnd.dCellScore = m_dwlCellScore*TO_DOUBLE;
	for (CT_DWORD i = 0; i < GAME_PLAYER; i++)
	{
		//�����˿�
		GameEnd.cbCardCount[i] = m_cbHandCardCount[i];
		memcpy(&GameEnd.cbHandCardData[i], m_cbHandCardData[i], m_cbHandCardCount[i] * sizeof(CT_BYTE));

	}
    memcpy(&GameEnd.cbEachBombCount, m_cbEachBombCount, sizeof(GameEnd.cbEachBombCount));
    memcpy(&GameEnd.dEachBombScore, m_dEachBombScore, sizeof(GameEnd.dEachBombScore));


    CT_LONGLONG llGameScore[GAME_PLAYER] = { 0,0,0 };
	CT_LONGLONG llBankerMaxWinScore = m_pGameDeskPtr->GetUserScore(dwWinChairID);
	for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	{
		//����Ӯ��
		if (i == dwWinChairID) continue;

		//��Ӯ����
		CT_LONGLONG llTempScore = m_cbHandCardCount[i] * (CT_INT32)m_dwlCellScore;

		CT_BYTE cbMaxSelfCard = MAX_COUNT;

		if (cbMaxSelfCard == m_cbHandCardCount[i] && m_cbOutCardCount[i] == 0)
		{	//���س�2
			//����
			GameEnd.bChunTian[i] = true;

			//ȫ�ط�
			llTempScore *= SCORE_TIME_CHUANTIAN;

		}
		else if (i == m_dwBankerUser && 1 == m_cbOutCardCount[i])
        {
            GameEnd.bChunTian[i] = 2;
            //����
            llTempScore *= SCORE_TIME_CHUANTIAN;
        }
        llGameScore[i] -= llTempScore;
        llGameScore[dwWinChairID] += llTempScore;
	}
	//�ж��Ƿ�ﵽӮ�����ֵ,���˻�����Ľ��
	CT_LONGLONG llReturnBackPartScore[GAME_PLAYER] = {0,0,0};
	//Ӯ���ܷ�>�볡ʱ��ǰ
	if ((llGameScore[dwWinChairID] + m_dEachBombScore[dwWinChairID]) > (llBankerMaxWinScore - m_dEachBombScore[dwWinChairID]))
	{
        GameEnd.bWinMaxScore = true;
		CT_LONGLONG llTempTotal = llGameScore[dwWinChairID];
        CT_LONGLONG llReturnBackScore = (llGameScore[dwWinChairID]+ m_dEachBombScore[dwWinChairID]) - (llBankerMaxWinScore - m_dEachBombScore[dwWinChairID]);
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            if (i == dwWinChairID) continue;
            if ((llGameScore[i]+m_dEachBombScore[i]) < 0)
            {
                llReturnBackPartScore[i] = ((CT_DOUBLE)((CT_DOUBLE)(llGameScore[i]*(-1))/llTempTotal)*llReturnBackScore);
            }
        }
	}

	//���У��(�˻����ͱ�֤��ҹ��ۣ�
	for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	{
		if ((i != dwWinChairID) && (llReturnBackPartScore[i] > 0))
		{
			//��Ҫ�˻����
            llGameScore[i] += llReturnBackPartScore[i];
            llGameScore[dwWinChairID] -= llReturnBackPartScore[i];
		}
	}

	for (CT_DWORD k = 0; k < GAME_PLAYER; ++k)
	{
		if (k == dwWinChairID) continue;
		//��֤��ҹ���
		CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(k);
		if (llGameScore[k] + llUserScore < 0)
		{
            llGameScore[dwWinChairID] += (llGameScore[k] + llUserScore);
            llGameScore[k] = (-llUserScore);
		}
		GameEnd.bLose[k] = GAME_LOSE;
	}
	GameEnd.bLose[dwWinChairID] = GAME_WIN;
    //LOG(INFO) << "GameEnd:beformoney_1: "<< m_pGameDeskPtr->GetUserScore(0) <<", 2: "<<m_pGameDeskPtr->GetUserScore(1) << ", 3: " << m_pGameDeskPtr->GetUserScore(2);
    //LOG(INFO) << "GameEnd:beformoney_1: "<< (m_pGameDeskPtr->GetUserScore(0)*TO_DOUBLE) <<", 2: "<<(m_pGameDeskPtr->GetUserScore(1)*TO_DOUBLE) << ", 3: " << (m_pGameDeskPtr->GetUserScore(2)*TO_DOUBLE);
    //д�����
	for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
	{
		m_RecordScoreInfo[i].dwUserID = m_pGameDeskPtr->GetUserID(i);
		m_RecordScoreInfo[i].cbIsAndroid = m_pGameDeskPtr->IsAndroidUser(i);
		m_RecordScoreInfo[i].cbStatus = 1;

		CT_INT32 iRevenue = 0;
		m_RecordScoreInfo[i].iScore = llGameScore[i];
		//����˰��
		if (llGameScore[i] > 0)
		{
			iRevenue = (CT_INT32)m_pGameDeskPtr->CalculateRevenue(i, llGameScore[i]);
            llGameScore[i] -= iRevenue;
			m_RecordScoreInfo[i].dwRevenue = iRevenue;
		}
		ScoreInfo  ScoreData;
		memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = i;
		ScoreData.bBroadcast = true;
		ScoreData.llScore = llGameScore[i];
		ScoreData.llRealScore= llGameScore[i];
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);
        m_RecordScoreInfo[i].iScore += (m_dEachBombScore[i]+m_dEachBombRevenue[i]);
        m_RecordScoreInfo[i].dwRevenue += m_dEachBombRevenue[i];
	}

	//�Ѹ�����ҵķ���תΪdouble,�����������ȡ���й�
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		GameEnd.dGameScore[i] = llGameScore[i] * TO_DOUBLE;
        GameEnd.dEachBombScore[i] *= TO_DOUBLE;
	}
	SendTableData(INVALID_CHAIR, SUB_S_GAME_CONCLUDE, &GameEnd, sizeof(GameEnd), true);
    //LOG(INFO) << "START****************GameEnd*******************";
    //LOG(INFO) << "GameEnd:Endmoney_1: "<< m_pGameDeskPtr->GetUserScore(0) <<", 2: "<<m_pGameDeskPtr->GetUserScore(1) << ", 3: " << m_pGameDeskPtr->GetUserScore(2);
    //LOG(INFO) << "GameEnd:Endmoney_1: "<< (m_pGameDeskPtr->GetUserScore(0)*TO_DOUBLE) <<", 2: "<<(m_pGameDeskPtr->GetUserScore(1)*TO_DOUBLE) << ", 3: " << (m_pGameDeskPtr->GetUserScore(2)*TO_DOUBLE);
    //LOG(INFO) << "GameEnd:GameScore_1: "<< GameEnd.dGameScore[0] <<", 2: "<<GameEnd.dGameScore[1] << ", 3: " << GameEnd.dGameScore[2];
    //LOG(INFO) << "GameEnd:GameScore_1: "<< llGameScore[0] <<", 2: "<<llGameScore[1] << ", 3: " << llGameScore[2];
    //LOG(INFO) << "GameEnd:BombScore_1: "<< GameEnd.dEachBombScore[0] <<", 2: "<<GameEnd.dEachBombScore[1] << ", 3:" << GameEnd.dEachBombScore[2];
    //LOG(INFO) << "GameEnd:BombScore_1: "<< m_dEachBombScore[0] <<", 2: "<<m_dEachBombScore[1] << ", 3:" << m_dEachBombScore[2];
    //LOG(INFO) << "GameEnd:BombCount_1: "<< (int)GameEnd.cbEachBombCount[0] <<", 2: "<<(int)GameEnd.cbEachBombCount[1] << ", 3: " << (int)GameEnd.cbEachBombCount[2];
    //LOG(INFO) << "GameEnd:TotalScore_1: "<< GameEnd.dGameScore[0] + GameEnd.dEachBombScore[0]<<" ,2: "<<GameEnd.dGameScore[1] + GameEnd.dEachBombScore[1]<< ", 3: " << GameEnd.dGameScore[2]+ GameEnd.dEachBombScore[2];
    //LOG(INFO) << "END****************GameEnd*******************";
	//��������
	for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
		if (m_cbTrustee[i] == 1)
		{
			SetTrustee(i, 0);
		}
	}
    
   /* CT_DWORD dwBankerUserID = 0;
    if (dwWinChairID != INVALID_CHAIR)
    {
        dwBankerUserID = m_pGameDeskPtr->GetUserID(dwWinChairID);
    }*/
	
	//��¼������Ϣ
	m_pGameDeskPtr->RecordRawInfo(m_RecordScoreInfo, GAME_PLAYER, NULL, 0, 0, 0, 0, 0, 0, 0);

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
	//CT_BYTE cbBombCount = (CT_BYTE) *nodeCond->get_int64();

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
	/*if (m_pGameDeskPtr->IsAndroidUser(wChairID))
	{
		SetTrustee(wChairID, 0);
	}*/
	CMD_S_OutCard OutCard;
	memset(&OutCard, 0, sizeof(OutCard));
	if (cbOutCount <= 0)
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);
		OutCard.cbIsFail = 1;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		return false;
	}

	//�жϳ����Ƿ�Ϸ�
	if (cbOutCount > m_cbHandCardCount[wChairID])
	{
		if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
			m_pGameDeskPtr->CloseUserConnect(wChairID);

		LOG(WARNING) << "out card count more than hand card count :"   << (int)cbOutCount <<" m_cbHandCardCount:" <<(int)m_cbHandCardCount[wChairID];
		OutCard.cbIsFail = 2;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		return false;
	}

	for (CT_BYTE i = 0; i < cbOutCount; ++i)
	{
		if (cbOutCard[i] == 0)
		{
			if (!m_pGameDeskPtr->IsAndroidUser(wChairID))
				m_pGameDeskPtr->CloseUserConnect(wChairID);

			LOG(WARNING) << "out card data is empty? ";
			OutCard.cbIsFail = 3;//
			SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
			return false;
		}
	}

	/*if (cbOutCard[0] == 0)
	{
		LOG(WARNING) << "card data is empty!";
		return false;
	}*/

	CT_BYTE cbOutCardTemp[MAX_COUNT];
	memcpy(cbOutCardTemp, cbOutCard, cbOutCount);

	m_GameLogic.SortCardList(cbOutCard, cbOutCount, ST_DESC);

	if (wChairID != m_dwCurrentUser) 
	{
		OutCard.cbIsFail = 4;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		m_pGameDeskPtr->CloseUserConnect(wChairID);
		LOG(WARNING) << "OnUserOutCard not this user!";
		return false;
	}

	//��ȡ����
	CT_BYTE cbCardType = m_GameLogic.GetCardType(cbOutCard, cbOutCount);

	//�����ж�
	if (cbCardType == CT_ERROR)
	{
		//assert(FALSE);
		LOG(ERROR) << "user out card, card type error, card data: " ;
		for (CT_BYTE i = 0;  i != cbOutCount; ++i)
		{
			LOG(ERROR) << std::hex << (int)cbOutCard[i];
		}
		OutCard.cbIsFail = 5;//
		SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
		m_pGameDeskPtr->CloseUserConnect(wChairID);
		return false;
	}

	//�����ж�
	if (m_cbTurnCardCount != 0)
	{
		//�Ա��˿�
		if (m_GameLogic.CompareCard(m_cbTurnCardData, m_cbTurnCardCount, cbOutCard, cbOutCount) == false)
		{
			//assert(FALSE);
			LOG(WARNING) << "compare card error.";
			OutCard.cbIsFail = 6;//
			SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
			return false;
		}
	}

    CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
    if ((1 == cbOutCount) && (m_cbHandCardCount[m_dwCurrentUser] > 1)&& (m_cbHandCardCount[dwNextUser] == 1))
    {
        //�������
        CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
        if (m_GameLogic.GetLogicValue(cbMaxSingle) > m_GameLogic.GetLogicValue(cbOutCard[0]))
        {
            OutCard.cbIsFail = 8;//
            SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
            return false;
        }
    }

	//ɾ���˿�
    if (m_GameLogic.RemoveCardList(cbOutCard, cbOutCount, m_cbHandCardData[wChairID], m_cbHandCardCount[wChairID]) == false)
    {
        //assert(FALSE);
        LOG(WARNING) << "remove card error.";
        OutCard.cbIsFail = 7;//
        SendTableData(wChairID, SUB_S_OUT_CARD, &OutCard, sizeof(OutCard), false);
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


	OutCard.dwOutCardUser = wChairID;
	OutCard.cbCardCount = cbOutCount;
	OutCard.dwCurrentUser = m_dwCurrentUser;
	OutCard.cbTimeLeft = TIME_OUT_CARD;
	m_bIsPassCard = false;
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

	if (INVALID_CHAIR != m_dwCurrentUser)
    {
        tagSearchCardResult SearchCardResult;
        memset(&SearchCardResult, 0, sizeof(SearchCardResult));
        CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
        if (cbResultCount <= 0)
        {//�����Գ���
            OutCard.cbTimeLeft = TIME_NOT_OUT_CARD;
			m_bIsPassCard = true;
        }
    }

	//��������
	CT_DWORD wHeadSize = sizeof(OutCard) - sizeof(OutCard.cbCardData);
	CT_DWORD wSendSize = wHeadSize + OutCard.cbCardCount * sizeof(CT_BYTE);
	//for (CT_BYTE i = 0;i < GAME_PLAYER;i++)
	//{
		SendTableData(INVALID_CHAIR, SUB_S_OUT_CARD, &OutCard, wSendSize, true);
	//}

	//ը���ж�
	if (cbCardType == CT_BOMB_CARD)
	{
		m_dwBombChairID = wChairID;
	}
	//�������
	//if (cbCardType == CT_MISSILE_CARD) m_cbTurnCardCount = 0;

	//�����ж�
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
	    if(INVALID_CHAIR != m_dwBombChairID)
        {
            CalculateBombScore();
        }
		OnEventGameEnd(wChairID, GER_NORMAL);
	}
	else
	{
        m_cbOpTotalTime = OutCard.cbTimeLeft + TIME_NETWORK_COMPENSATE;
        m_dwOpStartTime = (CT_DWORD)time(NULL);
		//�����й�����
		IsTrustee(m_cbOpTotalTime);
	}
	
	return true;
}

//��Ҳ���
CT_BOOL CGameProcess::OnUserPassCard(CT_WORD wChairID)
{
	//LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard :wChairID = " << wChairID ;
	//Ч��״̬
	//assert((wChairID == m_dwCurrentUser) && (m_cbTurnCardCount != 0));
	if ((wChairID != m_dwCurrentUser) || (m_cbTurnCardCount == 0)) return false;

	//������Ϣ
	CMD_S_PassCard PassCard;
	memset(&PassCard,0,sizeof(PassCard));
	tagSearchCardResult SearchCardResult;
	memset(&SearchCardResult, 0, sizeof(SearchCardResult));
	CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbHandCardData[wChairID], m_cbHandCardCount[wChairID], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
	if (cbResultCount > 0)
	{//�п��Գ�����
		PassCard.cbIsFail = true;
		LOG(ERROR)<<"OnUserPassCard FAIL";
		SendTableData(wChairID, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), false);
		return false;
	}
    PassCard.cbTurnOver = 0;

	//���ñ���
	m_dwCurrentUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
	if (m_dwCurrentUser == m_dwTurnWiner)
	{
        memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
        m_cbTurnCardCount = 0;
        PassCard.cbTurnOver = 1;
    }

	PassCard.dwPassCardUser = wChairID;
	PassCard.dwCurrentUser = m_dwCurrentUser;

	PassCard.cbTimeLeft = TIME_OUT_CARD;
	m_bIsPassCard = false;
	//PassCard.wCurrPlayCount = 0;//m_pGameDeskPtr->GetCurrPlayCount();

	if (!PassCard.cbTurnOver)
    {
        memset(&SearchCardResult, 0, sizeof(SearchCardResult));
        cbResultCount = m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
        if (cbResultCount <= 0)
        {//Ҫ�������ʱ��
            PassCard.cbTimeLeft = TIME_NOT_OUT_CARD;
			m_bIsPassCard = true;
        }
    }

	//��������
	/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), true);
        }
        else
        {
            SendTableData(i, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), false);
        }
	}*/
    SendTableData(INVALID_CHAIR, SUB_S_PASS_CARD, &PassCard, sizeof(PassCard), true);

    CalculateBombScore();
    m_cbOpTotalTime = PassCard.cbTimeLeft + TIME_NETWORK_COMPENSATE;
    m_dwOpStartTime = (CT_DWORD)time(NULL);
	//�й�
	IsTrustee(m_cbOpTotalTime);
	return true;
}

//�����һ�����
CT_DWORD CGameProcess::GetNextUser(CT_DWORD dwChairID)
{
	return (dwChairID + 1) % GAME_PLAYER;
}

CT_BOOL CGameProcess::IsTrustee(CT_DWORD dwTime)
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
	case GAME_SCENE_PLAY:
	{
	    if (dwTime)
		{
			m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
		}

		if ((m_cbOpTotalTime-1)<=(time(NULL)-m_dwOpStartTime))
        {
            m_pGameDeskPtr->KillGameTimer(IDI_OUT_CARD);
            m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, ((TIME_NETWORK_COMPENSATE) * 1000));
        }
		else if (m_cbTrustee[m_dwCurrentUser])
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, ((TIME_TRUSTEE > dwTime && dwTime != 0) ? dwTime: TIME_TRUSTEE)*1000 + 500);
		}
		else
		{
			m_pGameDeskPtr->SetGameTimer(IDI_OUT_CARD, (dwTime * 1000));
			//LOG(WARNING) << "SET IDI_OUT_CARD TIMER m_dwCurrentUser: "<< m_dwCurrentUser;
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
	/*for (CT_BYTE i = 0; i < GAME_PLAYER; i++)
	{
	    if (i == m_dwCurrentUser)
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
        }
        else
        {
            SendTableData(i, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), false);
        }
	}*/
    SendTableData(INVALID_CHAIR, SUB_S_TRUSTEE, &StatusTrustee, sizeof(StatusTrustee), true);
    //LOG(ERROR) << "SendTableData";
}
//�����
CT_BOOL CGameProcess::MatchGoodCard(CT_BYTE cbRandCard[], CT_BYTE cbRandCount)
{
	CT_BOOL bConfigCard[GAME_PLAYER] = {false};
	CT_BYTE cbConfigCount = 0;

	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if ((!m_pGameDeskPtr->IsAndroidUser(i)) && (m_pGameDeskPtr->GetUserBaseData(i)) && (m_pGameDeskPtr->GetUserBaseData(i)->dwTodayPlayCount < 3))
		{
			bConfigCard[i] = true;
			++cbConfigCount;
		}
	}

	if(cbConfigCount > 3)
    {
	    LOG(ERROR) << "cbConfigCount > 3 error!";
        return false;
    }

	//ֻ�е�Ԫ���ֵ���1�ĳ��βŻ������
	if (cbConfigCount > 0)
	{
		CT_BYTE cbTempCard[FULL_COUNT]={0};
		memcpy(cbTempCard, cbRandCard,CountArray(cbTempCard));
		memset(cbRandCard,0,cbRandCount);
		CT_BYTE cbGoodCard[GOOD_CARD_COUTN];
		memset(cbGoodCard,0,CountArray(cbGoodCard));
		m_GameLogic.GetGoodCardData(cbGoodCard, GOOD_CARD_COUTN);
		//��ȡ����
		m_GameLogic.RemoveGoodCardData(cbGoodCard, GOOD_CARD_COUTN, cbTempCard, FULL_COUNT);
		int iGoodIndex = 0;
		int iBadIndex = 0;
		CT_BYTE cbGoodCount = (cbConfigCount==2) ? (GOOD_CARD_MAX-1): GOOD_CARD_MAX;

		for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
		{
			int iNeedGoodCount = 0;
			//����������
			if(3 == cbConfigCount)
			{
                iNeedGoodCount = cbGoodCount-2;
			}
			else if (bConfigCard[i])
			{
				//�ַ��˿�
				iNeedGoodCount = cbGoodCount;

			}
			else if (2 == cbConfigCount)
			{
			    //һ��������
				iNeedGoodCount = (GOOD_CARD_COUTN-2*cbGoodCount);
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
	}

	return true;
}

CT_BOOL CGameProcess::OnAutoOutCard()
{
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return true;
	}

    CT_BYTE cbCardData[MAX_COUNT] = { 0 };
    CT_BYTE cbCardCount = 0;

    CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
    if (1 == m_cbTurnCardCount && m_cbHandCardCount[dwNextUser] == 1)
    {
        //�������
        CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
        if (cbMaxSingle>0)
        {
            cbCardData[0] = cbMaxSingle;
            cbCardCount = 1;
            return OnUserOutCard(m_dwCurrentUser, cbCardData, cbCardCount);
        }
    }
    else
	/*if (0 == m_cbTurnCardCount)
	{//��һ������
		CT_BYTE cbCardCount = m_cbHandCardCount[m_dwCurrentUser];
		m_GameLogic.SearchHeadOutCard();
		CT_BYTE cbCard[MAX_COUNT] = { 0 };
		cbCard[0] = m_cbHandCardData[m_dwCurrentUser][cbCardCount - 1];
		return OnUserOutCard(m_dwCurrentUser, cbCard, 1) ;
	}
	else*/
	{
		//����
		//return OnUserPassCard(m_dwCurrentUser) ;

		//���ը��ֱ�ӹ���
		/*if (m_cbTurnCardCount == 2)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			if (cbCardType == CT_MISSILE_CARD)
			{
				return OnUserPassCard(m_dwCurrentUser);
			}
		}*/

		//������ʾ
		tagSearchCardResult	SearchCardResult;					//�������
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		try
		{
			m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount,
				SearchCardResult);
		}
		catch (...)
		{
			SearchCardResult.cbSearchCount = 0;
		}

		if (SearchCardResult.cbSearchCount > 0)
		{
			CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbTurnCardData, m_cbTurnCardCount);
			CT_BYTE cbBestCardIndex = 0;
            CT_BYTE cbBombCount = 0; //��ը���Ƶ�����
            CT_BYTE cbBombDismantCount = 0; //��ը���Ƶ�����
            for (int k = 0; k < SearchCardResult.cbSearchCount; ++k)
            {
                CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[k], SearchCardResult.cbCardCount[k]);
                if (cbSearchCardType == CT_BOMB_CARD) ++cbBombCount;
            }

			for (CT_BYTE i = 0 ; i != SearchCardResult.cbSearchCount; ++i)
			{
				CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[i], SearchCardResult.cbCardCount[i]);
				if (cbCardType != CT_ERROR)
				{

					//��ͬ���ͣ�ֱ�ӳ���
					if (cbSearchCardType == cbCardType || ((cbSearchCardType == CT_BOMB_CARD) && (SearchCardResult.cbSearchCount != 1)&&((SearchCardResult.cbSearchCount - cbBombCount) == cbBombDismantCount)))
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
                                            ++cbBombDismantCount;
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
			cbCardCount = SearchCardResult.cbCardCount[cbBestCardIndex];
			memcpy(cbCardData, &SearchCardResult.cbResultCard[cbBestCardIndex], sizeof(CT_BYTE)*cbCardCount);
            if (m_cbHandCardCount[dwNextUser] == 1 && (1 == cbCardCount))
            {
                //�������
                CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
                if (cbMaxSingle>0)
                {
                    cbCardData[0] = cbMaxSingle;
                    cbCardCount = 1;
                }
            }
            //����������2�ҵ�ǰ�����ǵ��ƣ��ж��Լ�������������Ƿ��������������ģ�������ȳ�����
            else if((2==m_cbHandCardCount[m_dwCurrentUser]) && (1 == cbCardCount))
            {
                //�������
                CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
                if (cbMaxSingle>0)
                {
                    CT_BYTE cbTempCardData[MAX_COUNT] = { 0 };
                    CT_BYTE cbCount = 0;
                    cbTempCardData[0] = cbMaxSingle;
                    for (CT_DWORD k = 0; k < GAME_PLAYER; ++k)
                    {
                        if (m_dwCurrentUser == k) continue;
                        cbCount += m_GameLogic.SearchMaxOneCard(m_cbHandCardData[k], m_cbHandCardCount[k],cbTempCardData);
                    }
                    if (0 == cbCount)
                    {
                        cbCardData[0] = cbMaxSingle;
                        cbCardCount = 1;
                    }
                }

            }
            else if ((CT_DOUBLE_Ex == m_GameLogic.GetCardType(cbCardData,cbCardCount))
                     && (m_cbHandCardCount[dwNextUser] == 2)
                     && (CT_DOUBLE_Ex == m_GameLogic.GetCardType(m_cbHandCardData[dwNextUser],m_cbHandCardCount[dwNextUser])))
            {
                CT_BYTE cbResultCard[MAX_COUNT] = { 0 };
                //��������
                CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxDouble(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbTurnCardData[0], cbResultCard);
                if ((cbMaxSingle>0) && (m_GameLogic.GetLogicValue(cbCardData[0]) != m_GameLogic.GetLogicValue(cbResultCard[0])))
                {

                    memcpy(cbCardData,cbResultCard,2* sizeof(CT_BYTE));
                    cbCardCount = 2;
                }
            }
			//�����˿�
			m_GameLogic.SortCardList(cbCardData, cbCardCount, ST_DESC);
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
    return OnUserPassCard(m_dwCurrentUser);
}
CT_VOID CGameProcess::CalculateBombScore()
{
    if (((m_dwBombChairID != INVALID_CHAIR) && (m_dwTurnWiner == m_dwBombChairID) && (m_dwCurrentUser == m_dwBombChairID)) || ((m_dwBombChairID != INVALID_CHAIR) && (0 == m_cbHandCardCount[m_dwBombChairID])))
    {
        //ը������
        m_cbEachBombCount[m_dwBombChairID]++;
        CMD_S_BombScore stBombScore;
        memset(&stBombScore,0, sizeof(stBombScore));

        CT_LONGLONG llTempScore[GAME_PLAYER] = {0,0,0};
        //����ը���÷�
        for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
        {
            if (i != m_dwBombChairID)
            {
                CT_LONGLONG llTempScore1 = (SCORE_TIME_BOMB*m_dwlCellScore);
                llTempScore[i] -= llTempScore1;
                llTempScore[m_dwBombChairID] += llTempScore1;
            }
        }

        CT_LONGLONG llBankerMaxWinScore = m_pGameDeskPtr->GetUserScore(m_dwBombChairID);
        //�ж��Ƿ�ﵽӮ�����ֵ,���˻�����Ľ��
        CT_INT32 llReturnBackPartScore[GAME_PLAYER] = {0,0,0};
        //Ӯ���ܶ�>�볡ʱ��Ǯ
        if (llTempScore[m_dwBombChairID] > (llBankerMaxWinScore - m_dEachBombScore[m_dwBombChairID]))
        {
            //���㷢�ֽ��
            CT_LONGLONG llTempTotal = llTempScore[m_dwBombChairID]+ m_dEachBombScore[m_dwBombChairID]+m_dEachBombRevenue[m_dwBombChairID];
            CT_LONGLONG llReturnBackScore = (llTempScore[m_dwBombChairID]+ m_dEachBombScore[m_dwBombChairID]) - (llBankerMaxWinScore - m_dEachBombScore[m_dwBombChairID]);
            for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
            {
                if (i == m_dwBombChairID) continue;
                if ((llTempScore[i]+m_dEachBombScore[i]) < 0)
                {
                    llReturnBackPartScore[i] = ((CT_DOUBLE)((CT_DOUBLE)((llTempScore[i]+m_dEachBombScore[i])*(-1))/llTempTotal)*llReturnBackScore);
                }
            }

        }
        //���У��(�˻���
        for (CT_DWORD i = 0; (i < GAME_PLAYER) ; ++i)
        {
            if ((i != m_dwBombChairID) && (llReturnBackPartScore[i] > 0))
            {
                //��Ҫ�˻����
                llTempScore[i] += llReturnBackPartScore[i];
                llTempScore[m_dwBombChairID] -= llReturnBackPartScore[i];
            }
        }

        //�ж�����Ƿ񹻿ۣ���֤��ҹ��ۣ�
        for (CT_DWORD k = 0; k < GAME_PLAYER; ++k)
        {
            if (k == m_dwBombChairID) continue;
            //��֤�мҹ���
            CT_LONGLONG llUserScore = m_pGameDeskPtr->GetUserScore(k);
            if (llTempScore[k] + llUserScore < 0)
            {
                llTempScore[m_dwBombChairID] += (llTempScore[k] + llUserScore);
                llTempScore[k] = (-llUserScore);
            }
            stBombScore.dBombScore[k] = llTempScore[k]*TO_DOUBLE;
            m_pGameDeskPtr->AddUserScore(k,llTempScore[k], true);
            m_dEachBombScore[k] += llTempScore[k];
        }

        CT_INT32 iRevenue = (CT_INT32)m_pGameDeskPtr->CalculateRevenue(m_dwBombChairID, llTempScore[m_dwBombChairID]);
        m_dEachBombRevenue[m_dwBombChairID] += iRevenue;
        m_dEachBombScore[m_dwBombChairID] += (llTempScore[m_dwBombChairID]-iRevenue);

        stBombScore.dBombScore[m_dwBombChairID] = (llTempScore[m_dwBombChairID]-iRevenue)*TO_DOUBLE;

        m_pGameDeskPtr->AddUserScore(m_dwBombChairID,(llTempScore[m_dwBombChairID]-iRevenue),true);
        //LOG(INFO) << "BombScore: " << (llTempScore[m_dwBombChairID]-iRevenue);
        m_dwBombChairID = INVALID_CHAIR;
        SendTableData(INVALID_CHAIR, SUB_S_BOMB_SCORE, &stBombScore, sizeof(stBombScore), true);
    }
}


