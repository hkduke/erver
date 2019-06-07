
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
//#include <random>

#define ID_TIME_FREE					100					//空闲定时器
#define ID_TIME_BET_SCORE				101					//下注定时器 + 人数
#define ID_TIME_DELAY_BET_SCORE			102					//延迟下注定时器

#define TIME_BET_SCORE					1000				//下注时间
#define TIME_DELAY_BET_SCORE			2000				//延迟下注时间
#define TIME_BET_SCORE1					2000				//下注时间2

CT_LONGLONG CAndroidUserItemSink::m_llJetton[MAX_JETTON_NUM] = { 100,1000,5000,10000,100000 };

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
	, m_cbBetCount(0)
	, m_cbBetTotalCount(0)
	, m_cbBetPhase(en_Front)
	, m_cbFrontBetCount(0)
	, m_cbBackBetCount(0)
	, m_dwFrontBetTime(0)
	, m_dwBackBetTime(0)
	, m_cbBetTotalTime(0)
	, m_dwStartBetTime(0)
	, m_bSitList(false)
	, m_wCurrBanker(INVALID_CHAIR)
	, m_llBankerLimitScore(0)
	, m_llUserLimitScore(0)
	, m_randomGen(m_randomDevice())
{
	memset(&m_cbFlashArea, 0, sizeof(m_cbFlashArea));
}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{
	m_pTableFrame = NULL;
	m_wChairID = INVALID_CHAIR;
	m_pAndroidUserItem = NULL;
	m_cbBetCount = 0;
	m_cbBetTotalCount = 0;
	m_cbBetPhase = en_Front;
	m_cbFrontBetCount = 0;
	m_cbBackBetCount = 0;
	m_dwFrontBetTime = 0;
	m_dwBackBetTime = 0;
	m_cbBetTotalTime = 0;
	m_dwStartBetTime = 0;
	m_bSitList = false;

	memset(&m_cbFlashArea, 0, sizeof(m_cbFlashArea));
}

void CAndroidUserItemSink::Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem)
{
	m_pTableFrame = pTableFrame;
	m_wChairID = wChairID;
	m_pAndroidUserItem = pAndroidUserItem;
}

bool CAndroidUserItemSink::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch (dwTimerID - dwParam)
	{
	case ID_TIME_FREE:
	{
		return true;
	}
	break;
	case ID_TIME_DELAY_BET_SCORE:
	{
		m_pAndroidUserItem->KillTimer(ID_TIME_DELAY_BET_SCORE);


		//前半部分的总时间
		CT_DWORD nHalfTime = (CT_DWORD)round(m_cbBetTotalTime / 2) * 1000;
		std::uniform_int_distribution<> disBetTime(200, 600);
		CT_DWORD nRandBetTime = disBetTime(m_randomGen);

		m_cbBetPhase = en_Front;
		CT_DWORD nRandFrontQuickBet = rand() % 2;
		if (nRandFrontQuickBet == 1)
		{
			std::uniform_int_distribution<> disBackBetTime(800, 2000);
			m_dwBackBetTime = disBackBetTime(m_randomGen);

			m_cbFrontBetCount = nHalfTime / nRandBetTime;
			m_cbBackBetCount = nHalfTime / m_dwBackBetTime;
			m_dwFrontBetTime = nRandBetTime;
		}
		else
		{
			std::uniform_int_distribution<> disFrontBetTime(800, 2000);
			m_dwFrontBetTime = disFrontBetTime(m_randomGen);

			m_cbFrontBetCount = nHalfTime / m_dwFrontBetTime;
			m_cbBackBetCount = nHalfTime / nRandBetTime;
			m_dwBackBetTime = nRandBetTime;
		}

		//LOG(WARNING) << "android m_cbFrontBetCount: " << (int)m_cbFrontBetCount << ", m_cbBackBetCount: " << (int)m_cbBackBetCount;
		m_pAndroidUserItem->SetTimer(ID_TIME_BET_SCORE + m_wChairID, m_dwFrontBetTime, m_wChairID);
		m_dwStartBetTime = (CT_DWORD)time(NULL);


		//随机下注时间
		/*CT_DWORD dwTime = 0;
		if (m_bSitList || m_wCurrBanker != INVALID_CHAIR)
		{
			//std::default_random_engine e((unsigned int)time(NULL));
			//std::uniform_int_distribution<unsigned> u(3, 7);
			//dwTime = u(e);
			//r = rand()%(n - m + 1) + m;
			dwTime = rand() % (6 - 1 + 1) + 1;
		}
		else
		{
			//std::default_random_engine e;
			//std::uniform_int_distribution<unsigned> u(1, 2);
			dwTime = 1;
		}
		m_cbBetTotalCount = m_cbBetTotalTime / dwTime;
		m_pAndroidUserItem->SetTimer(ID_TIME_BET_SCORE + m_wChairID, dwTime * 1000, m_wChairID);*/
	}
	break;
	case ID_TIME_BET_SCORE:
	{
		//下注
		if (m_cbBetPhase == en_Front)
		{
			bool bBet = AndroidUserBet(dwParam);

			if (bBet)
			{
				CT_DWORD dwNow = (CT_DWORD)time(NULL);
				//时间超过
				if (dwNow - m_dwStartBetTime >= (CT_DWORD)(m_cbBetTotalTime / 2))
				{
					m_cbBetPhase = en_Back;
					m_pAndroidUserItem->KillTimer(ID_TIME_BET_SCORE + dwParam);
					m_pAndroidUserItem->SetTimer(ID_TIME_BET_SCORE + m_wChairID, m_dwBackBetTime, m_wChairID);
				}
			}
			else
			{
				m_pAndroidUserItem->KillTimer(ID_TIME_BET_SCORE + dwParam);
			}

			return bBet;
		}
		else
		{
			if (m_cbBetCount < (m_cbFrontBetCount + m_cbBackBetCount))
			{
				bool bBet = AndroidUserBet(dwParam);
				if (!bBet)
				{
					m_pAndroidUserItem->KillTimer(ID_TIME_BET_SCORE + dwParam);
				}
				return bBet;
			}
		}

		m_pAndroidUserItem->KillTimer(ID_TIME_BET_SCORE + dwParam);
		return true;
	}
	break;
	default:
		break;
	}
	return true;
}

bool CAndroidUserItemSink::OnGameMessage( CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize)
{
	switch (wSubCmdID)
	{
	case NN_SUB_S_GAME_START:
	{
		return OnSubGameStart(pData, wDataSize);
	}
	break;
	case NN_SUB_S_GAME_START_BET:
	{
		return OnSubStartBet(pData, wDataSize);
	}
	break;
	case NN_SUB_S_BET_SCORE_RESULT:
	{
		return OnSubAddScore(pData, wDataSize);
	}
	break;
	case NN_SUB_S_GAME_END:
	{
		return OnSubGameEnd();
	}
	break;
	case NN_SUB_S_USER_SIT_LIST:
	{
		return OnSubSitUserList(pData, wDataSize);
	}
	case NN_SUB_S_FLASH_AREA:
	{
		return OnSubFlashAreaInfo(pData, wDataSize);
	}
	default: break;
	}
	return true;
}

bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(NN_CMD_S_GameStart))
	{
		return false;
	}
	NN_CMD_S_GameStart* pGameStart = (NN_CMD_S_GameStart*)pBuffer;
	if (m_wCurrBanker != pGameStart->wBankerUser)
	{
		m_wCurrBanker = pGameStart->wBankerUser;//设置庄家
	}
	m_llBankerLimitScore = (CT_LONGLONG)(pGameStart->dBankerScore*TO_LL);
	m_llUserLimitScore = m_pTableFrame->GetUserScore(m_wChairID);

	return true;
}

bool CAndroidUserItemSink::OnSubStartBet(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(NN_CMD_S_GameBet))
	{
		return false;
	}

	NN_CMD_S_GameBet* pStartBet = (NN_CMD_S_GameBet*)pBuffer;
	m_cbBetTotalTime = pStartBet->cbTimeLeave;

	m_pAndroidUserItem->SetTimer(ID_TIME_DELAY_BET_SCORE , TIME_DELAY_BET_SCORE, 0);

	return true;
}
bool CAndroidUserItemSink::OnSubAddScore(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(NN_CMD_S_BetResult))
	{
		return false;
	}

	NN_CMD_S_BetResult* pBetResult = (NN_CMD_S_BetResult*)pBuffer;
	if (pBetResult)
	{
		m_llBankerLimitScore =(CT_LONGLONG)(pBetResult->dBankerLimitScore*TO_LL);
		m_llUserLimitScore = (CT_LONGLONG)(pBetResult->dUserLimitScore*TO_LL);
	}
	return true;
}

bool CAndroidUserItemSink::OnSubApplyBanker(const void * pBuffer, CT_DWORD wDataSize)
{
	return true;
}

bool CAndroidUserItemSink::OnSubSitUserList(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(NN_CMD_S_UserSitList))
	{
		return false;
	}

	NN_CMD_S_UserSitList* pUserSitList = (NN_CMD_S_UserSitList*)pBuffer;

	for (CT_WORD pos = 0; pos < MAX_SIT_POS; ++pos)
	{
		if (pUserSitList->wSitList[pos] == m_wChairID)
		{
			m_bSitList = true;
			return true;
		}
	}

	m_bSitList = false;
	return true;
}

bool CAndroidUserItemSink::OnSubFlashAreaInfo(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(NN_CMD_S_Game_FlashArea))
		return false;

	NN_CMD_S_Game_FlashArea* pFlashArea = (NN_CMD_S_Game_FlashArea*)pBuffer;
	for (CT_BYTE cbArea = 0; cbArea < AREA_COUNT + 1 ; ++cbArea)
	{
		m_cbFlashArea[cbArea] = pFlashArea->cbFlashArea[cbArea];
	}

	return true;
}

bool CAndroidUserItemSink::OnSubGameEnd()
{
	m_pAndroidUserItem->KillTimer(ID_TIME_BET_SCORE + m_wChairID);
	m_cbBetCount = 0;
	return true;
}

bool  CAndroidUserItemSink::AndroidUserBet(CT_DWORD dwParam)
{
	NN_CMD_C_Bet betScore;

	betScore.cbJettonArea = 1 + rand() % AREA_COUNT;//筹码区域

	if (m_cbFlashArea[betScore.cbJettonArea])
	{
		int nRandNum = rand();
		//筹码编号（0-4）
		if (nRandNum % 100 >= 90)
		{
			//百分之一的机会下大筹码（2-4）
			betScore.wJettonIndex = nRandNum % 3 + 1;
		}
		else
		{
			betScore.wJettonIndex = nRandNum % 2;
		}
	}
	else
	{
		int nRandNum = rand();
		//筹码编号（0-4）
		if (nRandNum % 100 >= 97)
		{
			//百分之一的机会下大筹码（2-4）
			betScore.wJettonIndex = nRandNum % 3 + 1;
		}
		else
		{
			betScore.wJettonIndex = nRandNum % 2;
		}
	}
	
	if (m_llJetton[betScore.wJettonIndex] > m_llBankerLimitScore || m_llJetton[betScore.wJettonIndex] > m_llUserLimitScore)
	{
		return false;
	}
	m_pTableFrame->OnGameEvent(m_wChairID, NN_SUB_C_BET, &betScore, sizeof(betScore));
	return true;
}