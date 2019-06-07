
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

//��ӡ��־
#define		PRINT_LOG_INFO						0
#define		PRINT_LOG_WARNING					1


////////////////////////////////////////////////////////////////////////
//��ʱ��
#define ID_TIME_SEND_CARD				100					//���ƶ�ʱ��
#define ID_TIME_OUT_CARD				101					//���ƶ�ʱ��
#define ID_TIME_GAME_END				102					//������ʱ��
#define ID_TIME_NEXT_OUT_CARD			103					//��һ����ҳ���
#define ID_TIME_NEW_TURN				104					//��һ�ֶ�ʱ��

//��ʱ��ʱ��
#define TIME_SEND_CARD					2500				//����ʱ��

////////////////////////////////////////////////////////////////////////


CGameProcess::CGameProcess(void)
	: m_dwGameType(0)
	, m_dwServiceCost(0)
	, m_dwlCellScore(1)
	, m_pGameDeskPtr(NULL)
{
	//������Ϸ����
	ClearGameData();
	//��ʼ������
	InitGameData();
}

CGameProcess::~CGameProcess(void)
{
}

//��Ϸ��ʼ
void CGameProcess::OnEventGameStart()
{
	assert(0 != m_pGameDeskPtr);
	//��ʼ������
	InitGameData();
	//����˽�˳�����
	ParsePrivateData(m_PrivateTableInfo.szOtherParam);
	//������Ϸ����
	UpdateGameConfig();
	//������ж�ʱ��
	ClearAllTimer();
	//������Ϸ״̬
	m_cbGameStatus = GAME_STATUS_PLAY;
	XiPai();

	//��ǰ���
	m_dwCurrentUser = m_dwFirstUser;

	//���Ϳ�ʼ��Ϣ
	for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		PDK_16_CMD_S_SendCard SendCard;
		memset(&SendCard, 0, sizeof(PDK_16_CMD_S_SendCard));
		SendCard.dwFirstUser = m_dwCurrentUser;
		SendCard.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
		SendCard.cbMustThree = IsMustOutThree(i);
		SendCard.cbCardCount = m_cbCardCount[i];
		SendCard.cbOutCardTime = m_cbOutCardTime;
		memcpy(SendCard.cbCardData, &m_cbCardData[i], MAX_COUNT);

		//��������
		SendTableData(i, PDK_16_SUB_S_SEND_CARD, &SendCard, sizeof(PDK_16_CMD_S_SendCard));
	}
}

//��Ϸ����
void CGameProcess::OnEventGameEnd(CT_DWORD dwChairID, GameEndTag GETag)
{
	//������ж�ʱ��
	ClearAllTimer();
	//������Ϸ״̬-����״̬
	m_cbGameStatus = GAME_STATUS_FREE;

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
	m_pGameDeskPtr->ConcludeGame(GAME_STATUS_FREE, strTime.c_str());

	//�����
	if (m_pGameDeskPtr->GetGameRoundPhase() == en_GameRound_End)
	{
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnEventGameEnd: SendTotalClearing";

		//�����ܽ���
		SendTotalClearing(strTime);

		//���������ϵ��û�
		m_pGameDeskPtr->ClearTableUser();
	}
}

//���ͳ���
void CGameProcess::OnEventGameScene(CT_DWORD dwChairID, CT_BOOL bIsLookUser)
{
	if (dwChairID >= GAME_PLAYER)
	{
		return;
	}

	switch (m_cbGameStatus)
	{
	case GAME_STATUS_FREE:	//����״̬
	{
		FPF_MSG_GS_FREE GameFree;
		memset(&GameFree, 0, sizeof(FPF_MSG_GS_FREE));
		GameFree.dwCellScore = m_dwlCellScore;
		GameFree.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();

		//��������
		SendTableData(dwChairID, SC_GAMESCENE_FREE, &GameFree, sizeof(FPF_MSG_GS_FREE), false);
		break;
	}
	case GAME_STATUS_PLAY:	//��Ϸ��״̬
	{
		//��������
		FPF_MSG_GS_PLAY GamePlay;
		memset(&GamePlay, 0, sizeof(FPF_MSG_GS_PLAY));
		//��������
		GamePlay.dwCellScore = m_dwlCellScore;
		GamePlay.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
		//ʣ��ʱ��
		CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
		GamePlay.cbTimeLeave = (CT_BYTE)(m_cbOutCardTime - std::min<CT_BYTE>(dwPassTime, m_cbOutCardTime));

		//��ǰ�û�(�������)
		GamePlay.dwCurrentUser = m_dwCurrentUser;

		if (m_dwTurnWiner != m_dwCurrentUser)
		{
			tagSearchCardResult SearchCardResult;
			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
			if (cbResultCount == 0)
			{//���ܳ�
				GamePlay.cbIsPass = 1;
			}
		}

		GamePlay.cbMustThree = IsMustOutThree(dwChairID);
		//��������
		memcpy(GamePlay.cbCardCount, m_cbCardCount, sizeof(m_cbCardCount));
		memcpy(GamePlay.cbCardData, &m_cbCardData[dwChairID], sizeof(GamePlay.cbCardData));
		//ʤ�����
		GamePlay.dwTurnWiner = m_dwTurnWiner;
		//��������
		GamePlay.cbTurnCardCount = m_cbTurnCardCount;
		memcpy(GamePlay.cbTurnCardData, m_cbTurnCardData, sizeof(GamePlay.cbTurnCardData));
		//�Ƿ񱨵�
		memcpy(GamePlay.cbSingle, m_cbUserSingle, sizeof(GamePlay.cbSingle));
		//ը������
		memcpy(GamePlay.cbRombCount, m_cbRombCount, sizeof(GamePlay.cbRombCount));

		//��������
		SendTableData(dwChairID, SC_GAMESCENE_PLAY, &GamePlay, sizeof(FPF_MSG_GS_PLAY), false);

		if (m_dwCurrentUser == dwChairID)
		{//֪ͨ����
			NotifyOutCard(dwChairID, GamePlay.cbTimeLeave, 1, (m_cbTurnCardCount == 0) ? true : false);
		}
		break;
	}
	case GAME_STATUS_END:
	{
		//��������
		FPF_MSG_GS_END EndStatus;
		memset(&EndStatus, 0, sizeof(FPF_MSG_GS_END));
		//��������
		EndStatus.dwCellScore = m_dwlCellScore;
		EndStatus.wCurrPlayCount = m_pGameDeskPtr->GetCurrPlayCount();
		//ʣ��ʱ��
		CT_DWORD dwPassTime = (CT_DWORD)time(NULL) - m_dwSysTime;
		EndStatus.cbTimeLeave = (CT_BYTE)(m_cbOutCardTime - std::min<CT_BYTE>(dwPassTime, m_cbOutCardTime));
		//ʣ����
		memcpy(&EndStatus.cbRemainCard, m_cbCardData, sizeof(m_cbCardData));
		memcpy(&EndStatus.cbRemainCount, m_cbCardCount, sizeof(m_cbCardCount));

		//���ͽ���״̬����
		SendTableData(dwChairID, SC_GAMESCENE_END, &EndStatus, sizeof(FPF_MSG_GS_END), false);

		//��Ϸ������Ϣ
		if (EndStatus.cbTimeLeave >= 5)
		{
			SendTableData(dwChairID, PDK_16_SUB_S_GAME_END, &m_GameEnd, sizeof(PDK_16_CMD_S_GameEnd));
		}

		break;
	}
	default:
		break;
	}
	return;
}

//��ʱ���¼�
void CGameProcess::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch (dwTimerID)
	{
	case ID_TIME_SEND_CARD:			//���ƶ���ʱ��
	{
		//���ٶ�ʱ��
		m_pGameDeskPtr->KillGameTimer(ID_TIME_SEND_CARD);
		//֪ͨ��ҳ���
		NotifyOutCard(m_dwCurrentUser, m_cbOutCardTime, 1, 0);
		return;
	}
	case ID_TIME_OUT_CARD:			//�ȴ���ҳ���ʱ�� 
	{
		//��Ϸ״̬�ж�
		if (m_cbGameStatus != GAME_STATUS_PLAY) return;
		//���ٶ�ʱ��
		m_pGameDeskPtr->KillGameTimer(ID_TIME_OUT_CARD);

		CT_BYTE cbCardCount = 0;														//������Ŀ
		CT_BYTE cbCardData[MAX_COUNT] = { 0 };								//�˿��б�
		memset(cbCardData, 0, sizeof(cbCardData));

		CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % m_PrivateTableInfo.wUserCount;
		if (1 == m_cbTurnCardCount && m_cbUserSingle[dwNextUser])
		{
			//�������
			CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData[0]);
			if (cbMaxSingle>0)
			{
				cbCardCount = 1;
				cbCardData[0] = cbMaxSingle;
			}
		}
		else
		{
			tagSearchCardResult SearchCardResult;
			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
			if (cbResultCount>0)
			{
				cbCardCount = SearchCardResult.cbCardCount[0];
				memcpy(cbCardData, &SearchCardResult.cbResultCard[0], sizeof(CT_BYTE)*cbCardCount);
			}
		}

		//�ҵ����ʵ���
		if (cbCardCount>0)
		{
			OnUserOutCard(m_dwCurrentUser, cbCardData, cbCardCount);
		}
		else
		{//��ȡ�쳣
			OnEventGameEnd(INVALID_CHAIR, GER_DISMISS);
		}

		return;
	}
	case ID_TIME_NEXT_OUT_CARD:		//��һ����ҳ���
	{
		//���ٶ�ʱ��
		m_pGameDeskPtr->KillGameTimer(ID_TIME_NEXT_OUT_CARD);
		//֪ͨ��һ����ҳ���
		CT_INT32 iRtn = NotifyNextOutCard(m_dwResumeUser);
		LOG_IF(INFO, PRINT_LOG_INFO) << "ID_TIME_NEXT_OUT_CARD: NotifyNextOutCard iRtn=" << iRtn;
		return;
	}
	case ID_TIME_GAME_END:			//��Ϸ����(��ʾ�����ʱ��)
	{
		//��Ϸ״̬�ж�
		if (m_cbGameStatus != GAME_STATUS_END) return;
		//���ٶ�ʱ��
		m_pGameDeskPtr->KillGameTimer(ID_TIME_GAME_END);
		//���ÿ����Ϸ����
		m_pGameDeskPtr->ConcludeGame(GAME_STATUS_FREE, NULL);
		return;
	}
	}
}

//��Ϸ��Ϣ
CT_BOOL CGameProcess::OnGameMessage(CT_WORD wChairID, CT_DWORD dwSubCmdID, const CT_VOID * pDataBuffer, CT_DWORD dwDataSize)
{
	CT_INT32 iRtn = false;
	switch (dwSubCmdID)
	{
	case PDK_16_SUB_C_OUT_CARD:					//������Ϣ
	{
		//Ч����Ϣ
		if (sizeof(PDK_16_CMD_C_OutCard) != dwDataSize)
		{
			return false;
		}
		//��Ϣ����
		PDK_16_CMD_C_OutCard* pOutCard = (PDK_16_CMD_C_OutCard*)pDataBuffer;
		if (NULL == pOutCard)
		{
			return false;
		}
		iRtn = OnUserOutCard(wChairID, pOutCard->cbCardData, pOutCard->cbCardCount);
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard iRtn=" << iRtn;
		return iRtn > 0;
	}
	case PKD_16_SUB_C_PASS_CARD:
	{//��Ҳ���
		if (wChairID >= m_PrivateTableInfo.wUserCount)
		{
			LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard wChairID Err!";
			return false;
		}
		iRtn = OnUserPassCard(wChairID);
		LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard iRtn=" << iRtn;
		return iRtn > 0;
	}
	}
	return CT_TRUE;
}

//����ָ��
void CGameProcess::SetTableFramePtr(ITableFrame* pTableFrame)
{
	assert(NULL != pTableFrame);
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
	memset(m_iTotalLWScore, 0, sizeof(m_iTotalLWScore));
	//�����ȫ�ش���
	memset(m_cbTotalQuanGuan, 0, sizeof(m_cbTotalQuanGuan));
	//����ұ��ش���
	memset(m_cbTotalBeiGuan, 0, sizeof(m_cbTotalBeiGuan));
	//�����ը������
	memset(m_cbTotalRomb, 0, sizeof(m_cbTotalRomb));
	//�����ͨ�����
	memset(m_cbTotalTongPei, 0, sizeof(m_cbTotalTongPei));
	//�׳��������
	m_dwFirstUser = INVALID_CHAIR;
	//�Ƿ�Ҫ������3
	m_cbIsOutThree = 1;
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
	LOG_IF(INFO, PRINT_LOG_INFO) << "GetGameRoundTotalScore: Score=" << m_iTotalLWScore[dwChairID];
	return m_iTotalLWScore[dwChairID];
}

//��������
CT_BOOL CGameProcess::SendTableData(CT_DWORD dwChairID, CT_WORD wSubCmdID, const CT_VOID* pBuf, CT_WORD dwLen, CT_BOOL bRecord)
{
	if (NULL == m_pGameDeskPtr)
	{
		return false;
	}
	return m_pGameDeskPtr->SendTableData(dwChairID, wSubCmdID, pBuf, dwLen, bRecord);
}

//��ʼ����Ϸ����
void CGameProcess::InitGameData()
{
	//�������
	//srand((unsigned)time(NULL));
	srand(Utility::GetTick());
	//ϵͳʱ��
	m_dwSysTime = 0;
	//ʣ����Ŀ
	m_cbLeftCardCount = MAX_CARD_TOTAL;
	//����˿�
	memset(m_cbRepertoryCard, 0, sizeof(m_cbRepertoryCard));
	//��Ϸ״̬
	m_cbGameStatus = GAME_STATUS_FREE;
	//��ǰ���
	m_dwCurrentUser = INVALID_CHAIR;
	//��ԭ���
	m_dwResumeUser = INVALID_CHAIR;
	//ʤ�����
	m_dwTurnWiner = INVALID_CHAIR;
	//�ϼ�����
	m_cbTurnCardType = CT_ERROR;
	//������Ŀ
	m_cbTurnCardCount = 0;
	//��������
	memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	//ը������
	memset(m_cbRombCount, 0, sizeof(m_cbRombCount));
	//���������
	memset(m_cbCardCount, 0, sizeof(m_cbCardCount));
	//���������
	memset(m_cbCardData, 0, sizeof(m_cbCardData));
	//������Ŀ
	memset(m_cbDiscardCount, 0, sizeof(m_cbDiscardCount));
	//������¼
	memset(m_cbDiscardCard, 0, sizeof(m_cbDiscardCard));
	//���ƴ���
	memset(m_cbOutCardCount, 0, sizeof(m_cbOutCardCount));
	//����ʱ��
	m_cbOutCardTime = 8;
	//��ұ���
	memset(m_cbUserSingle, 0, sizeof(m_cbUserSingle));

	m_bMustThree = false;
	m_cbSelfCardCount = MAX_COUNT;

	//��Ϸ����
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));

	if (NULL != m_pGameDeskPtr)
	{//��ȡ˽�˷���Ϣ
		m_pGameDeskPtr->GetPrivateTableInfo(m_PrivateTableInfo);
	}
	//�Ƿ���ʾ����
	m_cbIsLookCardCount = 0;
}

//������Ϸ����
void CGameProcess::UpdateGameConfig()
{
	if (NULL == m_pGameDeskPtr)
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
	//�ȴ���ҳ��ƶ�ʱ��
	m_pGameDeskPtr->KillGameTimer(ID_TIME_OUT_CARD);
}

//��ͨ����Ϸ����
CT_VOID CGameProcess::NormalGameEnd(CT_DWORD dwWinChairID)
{
	if (dwWinChairID >= INVALID_CHAIR)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "NormalGameEnd Err!";
		return;
	}
	//�׳��������
	m_dwFirstUser = dwWinChairID;

	PDK_16_CMD_S_GameEnd GameEnd;
	memset(&GameEnd, 0, sizeof(GameEnd));
	GameEnd.wWinUser = dwWinChairID;

	CT_BYTE	cbQuanGuan[GAME_PLAYER] = { 0 };							//ȫ�ر�־
	for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		//����Ӯ��
		if (i == dwWinChairID) continue;

		//�����Ӯ����
		CT_INT32 iLWCardCount = (1 == m_cbCardCount[i]) ? 0 : m_cbCardCount[i];
		GameEnd.iLWNum[i] -= iLWCardCount;
		GameEnd.iLWNum[dwWinChairID] += iLWCardCount;
		//��Ӯ����
		CT_LONGLONG llTempScore = iLWCardCount * (CT_INT32)m_dwlCellScore;

		CT_BYTE cbMaxSelfCard = 0;
		if (m_cbSelfCardCount == 16)
		{
			cbMaxSelfCard = MAX_COUNT;
		}
		else
		{
			cbMaxSelfCard = MAX_COUNT - 1;
		}
		if (cbMaxSelfCard == iLWCardCount)
		{	//���س�2
			//cbCardCount *= 2;
			//����
			GameEnd.cbBeiGuan[i] = 1;
			//ȫ��
			++cbQuanGuan[dwWinChairID];
			//ȫ�ط�
			llTempScore *= 2;
			GameEnd.llLWScore[i] -= llTempScore;
			GameEnd.llLWScore[dwWinChairID] += llTempScore;
		}
		else
		{//û����
			GameEnd.llLWScore[i] -= llTempScore;
			GameEnd.llLWScore[dwWinChairID] += llTempScore;
		}
	}

	//����ը����
	CT_LONGLONG llRombScore[GAME_PLAYER] = { 0 };
	for (CT_BYTE i = 0; i<m_PrivateTableInfo.wUserCount; ++i)
	{
		if (0 == m_cbRombCount[i]) continue;
		//��ը�����
		CT_LONGLONG llTempScore = m_cbRombCount[i] * (CT_INT32)m_dwlCellScore * 10;
		llRombScore[i] += llTempScore * (m_PrivateTableInfo.wUserCount - 1);

		for (CT_BYTE j = 0; j<m_PrivateTableInfo.wUserCount; ++j)
		{//��ը�����
			if (j == i)continue;
			llRombScore[j] -= llTempScore;
		}
	}

	//��Ӯ����
	for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		GameEnd.llLWScore[i] += llRombScore[i];
	}

	//ʣ����
	memcpy(&GameEnd.cbRemainCard, m_cbCardData, sizeof(m_cbCardData));
	memcpy(&GameEnd.cbRemainCount, m_cbCardCount, sizeof(m_cbCardCount));
	//ը������
	memcpy(&GameEnd.cbRombCount, m_cbRombCount, sizeof(m_cbRombCount));
	//ը������
	memcpy(&GameEnd.llRombScore, llRombScore, sizeof(llRombScore));

	//��������
	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_GAME_END, &GameEnd, sizeof(PDK_16_CMD_S_GameEnd));

	//д�����
	for (CT_BYTE i = 0; i<m_PrivateTableInfo.wUserCount; ++i)
	{
		ScoreInfo  ScoreData;
		memset(&ScoreData, 0, sizeof(ScoreInfo));
		ScoreData.dwUserID = i;
		ScoreData.llScore = GameEnd.llLWScore[i];
		ScoreData.scoreKind = enScoreKind_Draw;
		if (GameEnd.iLWNum[i] != 0)
		{
			ScoreData.scoreKind = GameEnd.iLWNum[i]>0 ? enScoreKind_Win : enScoreKind_Lost;
		}
		m_pGameDeskPtr->WriteUserScore(i, ScoreData);
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: dwWinChairID=" << dwWinChairID << ", name=" << m_pGameDeskPtr->GetNickName(dwWinChairID);

	//��¼������
	for (CT_WORD i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		m_iTotalLWScore[i] += (CT_INT32)GameEnd.llLWScore[i];
		m_cbTotalQuanGuan[i] += cbQuanGuan[i];
		m_cbTotalBeiGuan[i] += GameEnd.cbBeiGuan[i];
		m_cbTotalRomb[i] += GameEnd.cbRombCount[i];

		LOG_IF(INFO, PRINT_LOG_INFO) << "NormalGameEnd: LWScore=" << GameEnd.llLWScore[i] << ", QG=" << (CT_WORD)cbQuanGuan[i] << ",BG=" << \
			(CT_WORD)GameEnd.cbBeiGuan[i] << ",Romb=" << (CT_WORD)GameEnd.cbRombCount[i];
	}

	//��¼��Ϸ��������
	memset(&m_GameEnd, 0, sizeof(m_GameEnd));
	memcpy(&m_GameEnd, &GameEnd, sizeof(PDK_16_CMD_S_GameEnd));
}

//�����ܽ���
void CGameProcess::SendTotalClearing(std::string strTime)
{
	PDK_16_CMD_S_CLEARING ClearingData;
	memset(&ClearingData, 0, sizeof(PDK_16_CMD_S_CLEARING));
	ClearingData.wRoomOwner = m_pGameDeskPtr->GetPRoomOwnerChairID();
	//��ǰʱ��
	_snprintf_info(ClearingData.szCurTime, sizeof(ClearingData.szCurTime), "%s", strTime.c_str());
	for (CT_WORD i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
	{
		ClearingData.llTotalLWScore[i] = m_iTotalLWScore[i];
		ClearingData.cbQuanGuanCount[i] = m_cbTotalQuanGuan[i];
		ClearingData.cbBeiGuanCount[i] = m_cbTotalBeiGuan[i];
		ClearingData.cbRombCount[i] = m_cbTotalRomb[i];

		LOG_IF(INFO, PRINT_LOG_INFO) << "GetTotalClearing: LW:" << ClearingData.llTotalLWScore[i] << ",QG:" << (CT_WORD)ClearingData.cbQuanGuanCount[i] << \
			",BG:" << (CT_WORD)ClearingData.cbBeiGuanCount[i] << ",Romb:" << (CT_WORD)ClearingData.cbRombCount[i];
	}

	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_CLEARING, &ClearingData, sizeof(PDK_16_CMD_S_CLEARING));
}

//����˽�˳�����
void CGameProcess::ParsePrivateData(CT_CHAR *pBuf)
{
	if (strlen(pBuf) == 0)
	{
		return;
	}
	//���˽��������
	acl::json jsonCond(pBuf);

	//�Ƿ�غ���3
	const acl::json_node* nodeCond = jsonCond.getFirstElementByTagName("isOut3");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "isOut3 error!";
		return;
	}
	//�س�����3
	m_bMustThree = *nodeCond->get_bool();
	//2���淨���ú���3�س�
	if (m_PrivateTableInfo.wUserCount == 2)
	{
		m_bMustThree = false;
	}

	//������-16�Ż���15���淨
	nodeCond = jsonCond.getFirstElementByTagName("cardCount");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "card count error!";
		return;
	}
	m_cbSelfCardCount = (CT_BYTE)*nodeCond->get_int64();

	//ը�����ɲ�  
	nodeCond = jsonCond.getFirstElementByTagName("notRomb");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is split romb error!";
		return;
	}
	m_GameLogic.m_cbNotSplitRomb = (CT_BYTE)*nodeCond->get_int64();

	//����4��2  
	nodeCond = jsonCond.getFirstElementByTagName("allow4d2");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is four line two error!";
		return;
	}
	m_GameLogic.m_cbCanFourTakeTwo = (CT_BYTE)*nodeCond->get_int64();

	//����4��3 
	nodeCond = jsonCond.getFirstElementByTagName("allow4d3");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is four line three error!";
		return;
	}
	m_GameLogic.m_cbCanFourTakeThree = (CT_BYTE)*nodeCond->get_int64();

	//���ſ��ٴ�����
	nodeCond = jsonCond.getFirstElementByTagName("outEnd3");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is out end three error!";
		return;
	}
	m_GameLogic.m_cbIsOutThreeEnd = (CT_BYTE)*nodeCond->get_int64();

	//���ſ��ٴ�����
	nodeCond = jsonCond.getFirstElementByTagName("put3End");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is end three error!";
		return;
	}
	m_GameLogic.m_cbIsThreeEnd = (CT_BYTE)*nodeCond->get_int64();

	//�ɻ����ٴ�����
	nodeCond = jsonCond.getFirstElementByTagName("outFlyEnd");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is out fly end error!";
		return;
	}
	m_GameLogic.m_cbIsOutFlyEnd = (CT_BYTE)*nodeCond->get_int64();

	//�ɻ����ٴ�����
	nodeCond = jsonCond.getFirstElementByTagName("putFlyEnd");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is fly end error!";
		return;
	}
	m_GameLogic.m_cbIsFlyEnd = (CT_BYTE)*nodeCond->get_int64();

	//�Ƿ���ʾ����
	nodeCond = jsonCond.getFirstElementByTagName("showCardCount");
	if (nodeCond == NULL)
	{
		LOG_IF(WARNING, PRINT_LOG_WARNING) << "is fly end error!";
		return;
	}
	m_cbIsLookCardCount = (CT_BYTE)*nodeCond->get_int64();

	LOG_IF(INFO, PRINT_LOG_INFO) << "ParsePrivateData: m_bMustThree=" << m_bMustThree;
}

//�Ƿ������3
CT_BYTE CGameProcess::IsMustOutThree(CT_DWORD dwChairID)
{
	if (0 == m_bMustThree)
	{
		return 0;
	}
	return (dwChairID == m_dwFirstUser && m_cbIsOutThree == 1) ? 1 : 0;
}

//��ҳ���
CT_INT32 CGameProcess::OnUserOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCard[], CT_BYTE cbOutCount, CT_BOOL bTimeout/*=CT_FALSE*/)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserOutCard: dwChairID=" << dwChairID << ",Name=" << m_pGameDeskPtr->GetNickName(dwChairID) << ",cbOutCount=" << (CT_WORD)cbOutCount << \
		",m_cbTurnCardCount=" << (CT_WORD)m_cbTurnCardCount;

	/*std::string strOutCard("OnUserOutCard: cbOutCard[");
	for (CT_WORD i=0; i<cbOutCount; ++i)
	{
	CT_CHAR	szOut[8] = {0};
	_snprintf_info(szOut, sizeof(szOut), "%x,", cbOutCard[i]);
	strOutCard.append(szOut);
	}
	strOutCard.append("]");
	LOG_IF(INFO, PRINT_LOG_INFO) << strOutCard.c_str();

	std::string strTurnCard("OnUserOutCard: m_cbTurnCardData[");
	for (CT_WORD i = 0; i < m_cbTurnCardCount; ++i)
	{
	CT_CHAR	szOut[8] = { 0 };
	_snprintf_info(szOut, sizeof(szOut), "%x,", m_cbTurnCardData[i]);
	strTurnCard.append(szOut);
	}
	strTurnCard.append("]");
	LOG_IF(INFO, PRINT_LOG_INFO) << strTurnCard.c_str();
	LOG_IF(INFO, PRINT_LOG_INFO) << "================================";*/

	if (m_bMustThree)
	{//�س�����3
		if (1 == m_cbIsOutThree && !IsHaveThree(cbOutCard, cbOutCount))
		{//�Ƿ�Ҫ������3
			return 11;
		}
	}

	//Ч��״̬
	if (dwChairID != m_dwCurrentUser)
	{
		return 1;
	}

	if (cbOutCount>0 && cbOutCount>m_cbCardCount[dwChairID])
	{//������������
		return 2;
	}

	//�ɻ�������
	CT_BYTE cbAirLineCount[1] = { 0 };

	//��ȡ����
	CT_BYTE cbCardType = m_GameLogic.GetCardType(cbOutCard, cbOutCount, (/*0 == m_cbTurnCardCount && */cbOutCount == m_cbCardCount[dwChairID]), cbAirLineCount);

	//�����ж�
	if (cbCardType == CT_ERROR)
	{
		return 3;
	}

	/*if (cbCardType!=CT_BOMB_CARD && IsHaveRomb(cbOutCard, cbOutCount))
	{
	return 4;
	}*/

	//�����ж�
	if (m_cbTurnCardCount != 0)
	{
		//�Ա��˿�
		if (m_GameLogic.CompareCard(m_cbTurnCardData, m_cbTurnCardCount, cbOutCard, cbOutCount, cbOutCount == m_cbCardCount[dwChairID]) == false)
		{
			return 6;
		}
	}
	else
	{
		//�����һ����
		if (cbOutCount == m_cbCardCount[dwChairID])
		{
			//�����Ƿ���ٴ�����
			if ((cbCardType == CT_THREE_TAKE_TWO) && m_GameLogic.m_cbIsOutThreeEnd != 1 && (cbOutCount != 5))
			{
				return 61;
			}
			//�ɻ��Ƿ���ٴ����� 
			if ((cbCardType == CT_AIRPLANE) && m_GameLogic.m_cbIsOutFlyEnd != 1)
			{
				if (cbAirLineCount != NULL && cbOutCount != (cbAirLineCount[0] * 5))
				{
					return 62;
				}
			}
		}
	}

	if (1 == cbOutCount)
	{
		CT_DWORD  dwTempUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;
		if (1 == m_cbUserSingle[dwTempUser])
		{//�¼ұ������������
			CT_BYTE cbResultCount = m_GameLogic.SearchMaxOneCard(m_cbCardData[dwChairID], m_cbCardCount[dwChairID], cbOutCard);
			//��������
			if (cbResultCount > 0) return 7;
		}
	}

	//ɾ���˿�
	if (m_GameLogic.RemoveCard(cbOutCard, cbOutCount, m_cbCardData[dwChairID], m_cbCardCount[dwChairID]) == false)
	{
		/*std::string strHandCard("OnUserOutCard: HandCard[");
		for (CT_WORD i = 0; i < m_cbCardCount[dwChairID]; ++i)
		{
		CT_CHAR	szOut[8] = { 0 };
		_snprintf_info(szOut, sizeof(szOut), "%x,", m_cbCardData[dwChairID][i]);
		strHandCard.append(szOut);
		}
		strHandCard.append("]");
		LOG_IF(INFO, PRINT_LOG_INFO) << strHandCard.c_str();*/

		return 8;
	}

	//���Ʊ���
	m_cbOutCardCount[dwChairID]++;
	//��ȥ����
	m_cbCardCount[dwChairID] -= cbOutCount;

	//���ñ���
	m_dwTurnWiner = dwChairID;
	m_cbTurnCardType = cbCardType;
	m_cbTurnCardCount = cbOutCount;
	memcpy(m_cbTurnCardData, cbOutCard, sizeof(CT_BYTE)*cbOutCount);
	//�س�����3
	m_cbIsOutThree = 0;

	//��ұ���
	if (1 == m_cbCardCount[dwChairID]) m_cbUserSingle[dwChairID] = 1;

	//��ǰ���
	m_dwCurrentUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;

	CT_BYTE cbIsPass = 0;				//�Ƿ񲻳�
	tagSearchCardResult SearchCardResult;
	memset(&SearchCardResult, 0, sizeof(SearchCardResult));
	CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
	if (cbResultCount == 0)
	{//���ܳ�
		cbIsPass = 1;
	}

	//��������
	PDK_16_CMD_S_OutCardResult OutCardResult;
	memset(&OutCardResult, 0, sizeof(OutCardResult));
	OutCardResult.dwOutCardUser = dwChairID;
	OutCardResult.cbCardCount = cbOutCount;
	OutCardResult.cbSingle = m_cbUserSingle[dwChairID];
	memcpy(OutCardResult.cbCardData, m_cbTurnCardData, m_cbTurnCardCount * sizeof(CT_BYTE));

	OutCardResult.dwCurrentUser = INVALID_CHAIR;
	if (m_cbCardCount[dwChairID]>0)
	{//û�н���
		OutCardResult.dwCurrentUser = m_dwCurrentUser;
		OutCardResult.cbOutCardTime = m_cbOutCardTime;
		OutCardResult.cbNewTurn = 0;
		OutCardResult.cbIsPass = cbIsPass;
	}

	//��������
	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_OUT_CARD_RESULT, &OutCardResult, sizeof(PDK_16_CMD_S_OutCardResult));

	if (0 == m_cbCardCount[dwChairID])
	{//����
		if (cbCardType == CT_BOMB_CARD)
		{
			++m_cbRombCount[dwChairID];
		}
		OnEventGameEnd(dwChairID, GER_NORMAL);
		return 9;
	}
	return 11;
}

//��Ҳ���
CT_INT32 CGameProcess::OnUserPassCard(CT_DWORD dwChairID)
{
	LOG_IF(INFO, PRINT_LOG_INFO) << "OnUserPassCard: dwChairID=" << dwChairID << ",Name=" << m_pGameDeskPtr->GetNickName(dwChairID) << ",m_cbTurnCardCount=" << (CT_WORD)m_cbTurnCardCount;

	/*std::string strTurnCard("OnUserPassCard: m_cbTurnCardData[");
	for (CT_WORD i = 0; i < m_cbTurnCardCount; ++i)
	{
	CT_CHAR	szOut[8] = { 0 };
	_snprintf_info(szOut, sizeof(szOut), "%x,", m_cbTurnCardData[i]);
	strTurnCard.append(szOut);
	}
	strTurnCard.append("]");
	LOG_IF(INFO, PRINT_LOG_INFO) << strTurnCard.c_str();
	LOG_IF(INFO, PRINT_LOG_INFO) << "================================";*/

	//Ч��״̬
	if (dwChairID != m_dwCurrentUser)
	{//���ǵ�ǰ���
		return 1;
	}
	tagSearchCardResult SearchCardResult;
	memset(&SearchCardResult, 0, sizeof(SearchCardResult));
	CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[dwChairID], m_cbCardCount[dwChairID], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
	if (cbResultCount > 0)
	{//�п��Գ�����
		return 2;
	}

	//��ǰ���
	m_dwCurrentUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;

	CT_BYTE cbNewTurn = 0;		//�Ƿ���һ��
	CT_BYTE cbIsPass = 0;				//�Ƿ񲻳�
	if (m_dwTurnWiner == m_dwCurrentUser)
	{//��һ��
		cbNewTurn = 1;
		if (m_cbTurnCardType == CT_BOMB_CARD)
		{//��¼ը��
			++m_cbRombCount[m_dwTurnWiner];
		}
		//��һ��
		m_cbTurnCardType = CT_ERROR;
		m_cbTurnCardCount = 0;
		memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
	}
	else
	{
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		CT_BYTE cbTempResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwCurrentUser], m_cbCardCount[m_dwCurrentUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
		if (cbTempResultCount == 0)
		{//���ܳ�
			cbIsPass = 1;
		}
	}

	//��������
	PDK_16_CMD_S_OutCardResult OutCardResult;
	memset(&OutCardResult, 0, sizeof(OutCardResult));
	OutCardResult.dwOutCardUser = dwChairID;
	OutCardResult.cbCardCount = 0;
	OutCardResult.cbSingle = m_cbUserSingle[dwChairID];

	OutCardResult.dwCurrentUser = m_dwCurrentUser;
	OutCardResult.cbOutCardTime = m_cbOutCardTime;
	OutCardResult.cbNewTurn = cbNewTurn;
	OutCardResult.cbIsPass = cbIsPass;

	//��������
	SendTableData(INVALID_CHAIR, PDK_16_SUB_S_OUT_CARD_RESULT, &OutCardResult, sizeof(PDK_16_CMD_S_OutCardResult));

	return 3;
}

//֪ͨ��һ����ҳ���
CT_INT32 CGameProcess::NotifyNextOutCard(CT_DWORD dwChairID)
{
	m_dwCurrentUser = INVALID_CHAIR;
	m_dwResumeUser = (dwChairID + 1) % m_PrivateTableInfo.wUserCount;

	if (m_dwResumeUser == m_dwTurnWiner)
	{
		//��ǰ�������
		m_dwCurrentUser = m_dwTurnWiner;
		if (m_cbTurnCardType == CT_BOMB_CARD)
		{//��¼ը��
			++m_cbRombCount[m_dwTurnWiner];
		}

		//��һ��
		m_cbTurnCardType = CT_ERROR;
		m_cbTurnCardCount = 0;
		memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));

		//֪ͨ����
		NotifyOutCard(m_dwCurrentUser, m_cbOutCardTime, 1, 1);
		return 1;
	}
	else
	{
		tagSearchCardResult SearchCardResult;
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[m_dwResumeUser], m_cbCardCount[m_dwResumeUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
		//û�пɴ������
		if (0 == cbResultCount)
		{//֪ͨ���ܳ���
			NotifyOutCard(m_dwResumeUser, 1, 0, 0);
			m_pGameDeskPtr->SetGameTimer(ID_TIME_NEXT_OUT_CARD, 1000);
		}
		else if (cbResultCount > 0)
		{//�ҵ��¼�
		 //֪ͨ����
			NotifyOutCard(m_dwResumeUser, m_cbOutCardTime, 1, 0);
			//��ǰ�������
			m_dwCurrentUser = m_dwResumeUser;
			return 2;
		}
	}
	return 3;
}

//������һ���������
CT_DWORD CGameProcess::FindNextOutCardUser(CT_DWORD dwChairID)
{
	for (CT_BYTE i = 1; i <= m_PrivateTableInfo.wUserCount; ++i)
	{
		CT_DWORD dwTempUser = (dwChairID + i) % m_PrivateTableInfo.wUserCount;
		if (dwTempUser == dwChairID)
		{
			if (m_cbTurnCardType == CT_BOMB_CARD)
			{//��¼ը��
				++m_cbRombCount[m_dwTurnWiner];
			}

			//��һ��
			m_cbTurnCardType = CT_ERROR;
			m_cbTurnCardCount = 0;
			memset(m_cbTurnCardData, 0, sizeof(m_cbTurnCardData));
			return dwTempUser;
		}

		tagSearchCardResult SearchCardResult;
		memset(&SearchCardResult, 0, sizeof(SearchCardResult));
		CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbCardData[dwTempUser], m_cbCardCount[dwTempUser], m_cbTurnCardData, m_cbTurnCardCount, SearchCardResult);
		if (cbResultCount>0)
		{//�ҵ��д�ù������
			return dwTempUser;
		}
	}
	LOG_IF(INFO, PRINT_LOG_INFO) << "Find User Err!";
	return INVALID_CHAIR;
}

//֪ͨ����
CT_BOOL CGameProcess::NotifyOutCard(CT_DWORD dwChairID, CT_BYTE cbOutCardTime, CT_BYTE cbOutCardFlag, CT_BYTE cbNewTurn)
{
	PDK_16_CMD_S_NotifyOutCard OutCard;
	memset(&OutCard, 0, sizeof(OutCard));
	OutCard.dwOutCardUser = dwChairID;
	OutCard.cbOutCardTime = cbOutCardTime;
	OutCard.cbOutCardFlag = cbOutCardFlag;
	OutCard.cbNewTurn = cbNewTurn;
	OutCard.cbMustThree = 0;
	if (m_bMustThree)
	{//�س�����3
		OutCard.cbMustThree = IsMustOutThree(dwChairID);
	}

	//��������
	return SendTableData(INVALID_CHAIR, PDK_16_SUB_S_NOTIFY_OUT_CARD, &OutCard, sizeof(PDK_16_CMD_S_NotifyOutCard));
}

//�Ƿ���ը��
CT_BOOL CGameProcess::IsHaveRomb(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	tagAnalyseResult AnalyseResult = { 0 };
	m_GameLogic.AnalysebCardData(cbCardData, cbCardCount, AnalyseResult);

	return (AnalyseResult.cbBlockCount[3] > 0);
}

//�Ƿ��к���3
CT_BOOL CGameProcess::IsHaveThree(CT_BYTE cbCardData[], CT_BYTE cbCardCount)
{
	for (CT_WORD i = 0; i < cbCardCount&&i < MAX_COUNT; ++i)
	{
		if (cbCardData[i] == 0x33)
		{//����3
			return true;
		}
	}
	return false;
}

void CGameProcess::XiPai()
{
	//�ַ��˿�
	CT_DWORD	dwFirstUser = m_dwFirstUser;
	for (CT_BYTE i = 0; i<100; ++i)
	{
		m_dwFirstUser = dwFirstUser;
		memset(m_cbCardData, 0, sizeof(m_cbCardData));

		CT_BYTE cbMaxCard = 0;
		CT_BYTE cbMaxSelfCard = 0;
		if (m_cbSelfCardCount == 16)
		{
			cbMaxCard = MAX_CARD_TOTAL;
			cbMaxSelfCard = MAX_COUNT;
		}
		else
		{
			cbMaxCard = MAX_CARD_TOTAL - 3;
			cbMaxSelfCard = MAX_COUNT - 1;
		}

		m_cbLeftCardCount = cbMaxCard;
		m_GameLogic.RandCardData(m_cbRepertoryCard, cbMaxCard);

		///////////////////////////////////////////////////////	
		//���һ�������ܴ���
		/*m_cbRepertoryCard[47] = 0x03;
		m_cbRepertoryCard[46] = 0x13;
		m_cbRepertoryCard[45] = 0x33;
		m_cbRepertoryCard[44] = 0x04;
		m_cbRepertoryCard[43] = 0x14;
		m_cbRepertoryCard[42] = 0x24;
		m_cbRepertoryCard[41] = 0x34;
		m_cbRepertoryCard[40] = 0x23;
		m_cbRepertoryCard[39] = 0x08;
		m_cbRepertoryCard[38] = 0x2C;
		m_cbRepertoryCard[37] = 0x3C;
		m_cbRepertoryCard[36] = 0x0D;
		m_cbRepertoryCard[35] = 0x1D;
		m_cbRepertoryCard[34] = 0x01;
		m_cbRepertoryCard[33] = 0x11;
		m_cbRepertoryCard[32] = 0x21;

		m_cbRepertoryCard[31] = 0x05;
		m_cbRepertoryCard[30] = 0x15;
		m_cbRepertoryCard[29] = 0x25;
		m_cbRepertoryCard[28] = 0x35;
		m_cbRepertoryCard[27] = 0x06;
		m_cbRepertoryCard[26] = 0x16;
		m_cbRepertoryCard[25] = 0x26;
		m_cbRepertoryCard[24] = 0x36;
		m_cbRepertoryCard[23] = 0x07;
		m_cbRepertoryCard[22] = 0x17;
		m_cbRepertoryCard[21] = 0x27;
		m_cbRepertoryCard[20] = 0x37;
		m_cbRepertoryCard[19] = 0x18;
		m_cbRepertoryCard[18] = 0x1C;
		m_cbRepertoryCard[17] = 0x2D;
		m_cbRepertoryCard[16] = 0x3D;

		m_cbRepertoryCard[15] = 0x09;
		m_cbRepertoryCard[14] = 0x19;
		m_cbRepertoryCard[13] = 0x29;
		m_cbRepertoryCard[12] = 0x39;
		m_cbRepertoryCard[11] = 0x0A;
		m_cbRepertoryCard[10] = 0x1A;
		m_cbRepertoryCard[9]  = 0x2A;
		m_cbRepertoryCard[8]  = 0x3A;
		m_cbRepertoryCard[7]  = 0x0B;
		m_cbRepertoryCard[6]  = 0x1B;
		m_cbRepertoryCard[5]  = 0x2B;
		m_cbRepertoryCard[4]  = 0x3B;
		m_cbRepertoryCard[3]  = 0x28;
		m_cbRepertoryCard[2]  = 0x38;
		m_cbRepertoryCard[1]  = 0x32;
		m_cbRepertoryCard[0]  = 0x0C;
		m_dwFirstUser = 0;
		m_dwCurrentUser = 0;*/

		/*m_cbRepertoryCard[47] = 0x33;
		m_cbRepertoryCard[46] = 0x23;
		m_cbRepertoryCard[45] = 0x13;
		m_cbRepertoryCard[44] = 0x34;
		m_cbRepertoryCard[43] = 0x24;
		m_cbRepertoryCard[42] = 0x14;
		m_cbRepertoryCard[41] = 0x35;
		m_cbRepertoryCard[40] = 0x36;
		m_cbRepertoryCard[39] = 0x3C;
		m_cbRepertoryCard[38] = 0x3D;
		m_cbRepertoryCard[37] = 0x3A;
		m_cbRepertoryCard[36] = 0x2A;
		m_cbRepertoryCard[35] = 0x1A;
		m_cbRepertoryCard[34] = 0x3B;
		m_cbRepertoryCard[33] = 0x2B;
		m_cbRepertoryCard[32] = 0x1B;

		m_cbRepertoryCard[31] = 0x38;
		m_cbRepertoryCard[30] = 0x28;
		m_cbRepertoryCard[29] = 0x18;
		m_cbRepertoryCard[28] = 0x39;
		m_cbRepertoryCard[27] = 0x29;
		m_cbRepertoryCard[26] = 0x19;
		m_cbRepertoryCard[25] = 0x2C;
		m_cbRepertoryCard[24] = 0x1C;
		m_cbRepertoryCard[23] = 0x2D;
		m_cbRepertoryCard[22] = 0x21;
		m_cbRepertoryCard[21] = 0x11;
		m_cbRepertoryCard[20] = 0x32;
		m_cbRepertoryCard[19] = 0x25;
		m_cbRepertoryCard[18] = 0x26;
		m_cbRepertoryCard[17] = 0x37;
		m_cbRepertoryCard[16] = 0x27;

		m_cbRepertoryCard[15] = 0x03;
		m_cbRepertoryCard[14] = 0x04;
		m_cbRepertoryCard[13] = 0x08;
		m_cbRepertoryCard[12] = 0x09;
		m_cbRepertoryCard[11] = 0x05;
		m_cbRepertoryCard[10] = 0x15;
		m_cbRepertoryCard[9] = 0x06;
		m_cbRepertoryCard[8] = 0x16;
		m_cbRepertoryCard[7] = 0x07;
		m_cbRepertoryCard[6] = 0x17;
		m_cbRepertoryCard[5] = 0x0A;
		m_cbRepertoryCard[4] = 0x0B;
		m_cbRepertoryCard[3] = 0x0C;
		m_cbRepertoryCard[2] = 0x1D;
		m_cbRepertoryCard[1] = 0x0D;
		m_cbRepertoryCard[0] = 0x01;*/
		///////////////////////////////////////////////////////
		CT_BYTE cbSingle = 0;
		CT_BYTE cbDouble = 0;
		for (CT_BYTE i = 0; i < m_PrivateTableInfo.wUserCount; ++i)
		{
			m_cbLeftCardCount -= cbMaxSelfCard;
			m_cbCardCount[i] = cbMaxSelfCard;
			memcpy(m_cbCardData[i], &m_cbRepertoryCard[m_cbLeftCardCount], sizeof(CT_BYTE)*cbMaxSelfCard);

			if (m_dwFirstUser != INVALID_CHAIR) continue;

			if (m_PrivateTableInfo.wUserCount == GAME_PLAYER)
			{
				for (CT_BYTE n = 0; n < cbMaxSelfCard; ++n)
				{
					if (0x33 == m_cbCardData[i][n])
					{//��¼����3���
						m_dwFirstUser = i;
					}
				}
			}
			else
			{
				m_dwFirstUser = rand() % m_PrivateTableInfo.wUserCount;
			}

			tagSearchCardResult SearchCardResult;
			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			if (m_GameLogic.SearchLineCardType(m_cbCardData[i], m_cbCardCount[i], 0, 1, 0, SearchCardResult))
			{//��˳
				for (CT_BYTE i = 0; i < SearchCardResult.cbSearchCount; ++i)
				{
					if (SearchCardResult.cbCardCount[i] >= 12)
					{
						cbSingle = 1;
						break;
					}
				}
			}

			memset(&SearchCardResult, 0, sizeof(SearchCardResult));
			if (m_GameLogic.SearchLineCardType(m_cbCardData[i], m_cbCardCount[i], 0, 2, 0, SearchCardResult))
			{//˫˳
				for (CT_BYTE i = 0; i < SearchCardResult.cbSearchCount; ++i)
				{
					if (SearchCardResult.cbCardCount[i] >= 7 * 2)
					{
						cbDouble = 1;
						break;
					}
				}
			}
		}
		if (0 == cbSingle && 0 == cbDouble)
		{//û�е�˳��˫˳
			break;
		}
	}
}

