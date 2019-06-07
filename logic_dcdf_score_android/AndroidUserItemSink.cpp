
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"

#define ID_TIME_GAME_START				100					//开始定时器(2S)
#define TIME_GAME_START                 3                   //多久开始一次

//定时器时间
#define TIME_BASE						1000				//基础定时器

CT_DWORD CAndroidUserItemSink::m_dwJetton[JETTON_COUNT] = { 50, 100, 200, 300, 400, 500, 600, 700, 800, 900,
													1000, 2000, 3000, 4000, 5000, 6000, 7000, 8000,
													9000, 10000, 50000, 100000, 150000, 200000, 300000};

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
{
}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{
	m_pTableFrame = NULL;
	m_wChairID = INVALID_CHAIR;
	m_pAndroidUserItem = NULL;
}

void CAndroidUserItemSink::Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem)
{
	m_pTableFrame = pTableFrame;
	m_wChairID = wChairID;
	m_pAndroidUserItem = pAndroidUserItem;
}

bool CAndroidUserItemSink::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch (dwTimerID)
	{
	case ID_TIME_GAME_START:
	{
		//m_pAndroidUserItem->KillTimer(ID_TIME_OPEN_CARD);
		FXGZ_CMD_C_Start start;
		start.dwUserID = m_pTableFrame->GetUserID(m_wChairID);
		int nJettonIndex = rand() % JETTON_COUNT;
		start.dJettonScore = m_dwJetton[nJettonIndex]*TO_DOUBLE;

		m_pTableFrame->OnGameEvent(m_wChairID, FXGZ_SUB_C_START, &start, sizeof(start));
	}
	break;
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
	case SC_GAMESCENE_FREE:
	{
		return OnFreeGameScene(pData, wDataSize);
	}
	case SUB_S_MAIN_START:
	{
		return OnSubGameStart(pData, wDataSize);
	}
	case SUB_S_FREE_START:
	{
		return true;
	}
    case SUB_S_MAIN_STRAT_FAIL:
    {
        m_pAndroidUserItem->KillTimer(ID_TIME_GAME_START);
        return true;
    }
	default:
        break;
	}
	return true;
}

bool CAndroidUserItemSink::OnFreeGameScene(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(FXGZ_MSG_GS_FREE))
	{
		return false;
	}

	//FXGZ_MSG_GS_FREE* pGSFree = (FXGZ_MSG_GS_FREE*)pBuffer;
	int nStartTime = 1 + rand() % TIME_GAME_START;
	m_pAndroidUserItem->SetTimer(ID_TIME_GAME_START, nStartTime*TIME_BASE);
	
	return true;
}

bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize)
{
	return true;
}