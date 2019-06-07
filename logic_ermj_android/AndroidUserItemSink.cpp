
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
#include <algorithm>
#include <random>
#include <limits.h>

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
{
	//接口变量
	srand((unsigned)time(NULL));
}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{

}

void CAndroidUserItemSink::Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem)
{
	m_pTableFrame = pTableFrame;
	m_wChairID = wChairID;
	m_pAndroidUserItem = pAndroidUserItem;
}


bool CAndroidUserItemSink::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch(dwTimerID)
	{
		case IDI_ANDROID_OUT_CARD:
		{
			CMD_C_OutCard ocard;
			ocard.cbCardId = m_cbCurOutCardData;
			ocard.cbCardIndex = 0;
			ocard.dwUserID = m_pTableFrame->GetUserID(m_dwSelf);
			m_pTableFrame->OnGameEvent(m_pAndroidUserItem->GetChairID(), SUB_C_OUT_CARD, &ocard, sizeof(ocard));

			m_pAndroidUserItem->KillTimer(IDI_ANDROID_OUT_CARD);
			break;
		}
	}
	return CT_TRUE;
}

CT_BOOL CAndroidUserItemSink::OnMsg_SendCard(const CT_VOID *pData, CT_DWORD dwDataSize)
{
	if(dwDataSize != sizeof(CMD_S_SendCard))
	{
		return CT_TRUE;
	}

	CMD_S_SendCard *pcards = (CMD_S_SendCard * )pData;
	if(pcards->wSendCardUser == m_dwSelf)
	{
		m_cbCurZhuaCardData = pcards->cbCardId;
		m_cbCurOutCardData = pcards->cbCardId;
	}

	return CT_TRUE;
}

CT_BOOL CAndroidUserItemSink::OnMsg_OutCard(const CT_VOID *pData, CT_DWORD dwDataSize)
{
	if(dwDataSize != sizeof(CMD_S_OutTip))
	{
		return CT_TRUE;
	}

	CMD_S_OutTip *pouttip = (CMD_S_OutTip *)pData;
	if(pouttip->wOutCardUser == m_dwSelf)
	{
		CT_INT32  timers = (rand() % TIMER_ANDROID_OUT_CARD_MAX) + TIMER_ANDROID_OUT_CARD_MIN;
		m_pAndroidUserItem->SetTimer(IDI_ANDROID_OUT_CARD, timers * 1000);
	}

	return CT_TRUE;
}

CT_BOOL CAndroidUserItemSink::OnMsg_GameStart(const CT_VOID *pData, CT_DWORD dwDataSize)
{
	size_t s = sizeof(CMD_S_GameStart);
	if(s != dwDataSize)
	{
		return CT_FALSE;
	}

	CMD_S_GameStart *pGameStart = (CMD_S_GameStart *)pData;
	if(pGameStart)
	{
		m_dwCurBanker = pGameStart->wBankerUser;
		m_dwSelf = pGameStart->wCurrentUser;
		m_dwShengYuCount = pGameStart->wRemainNum;
		m_dCellScore = pGameStart->lCellScore;
		m_dwUpdateTime = pGameStart->wUpDateTime;
		memset(m_cbHandCardData, INVALID_CARD_DATA, sizeof(m_cbHandCardData));
		for(int i = 0; i < DEALER_CARD_COUNT; i++)
		{
			m_cbHandCardData[i] = pGameStart->cbHandCardDataIndex[i];
		}
	}

	if(m_dwCurBanker == m_dwSelf)
	{
		m_cbCurOutCardData = m_cbHandCardData[DEALER_CARD_COUNT-1];
		m_cbCurZhuaCardData = m_cbHandCardData[DEALER_CARD_COUNT-1];
	}
	else
	{
		m_cbCurOutCardData = INVALID_CARD_DATA;
		m_cbCurZhuaCardData = INVALID_CARD_DATA;
	}
	return CT_TRUE;
}

CT_BOOL CAndroidUserItemSink::OnMsg_OperateTip_SendCard(const CT_VOID *pData, CT_DWORD dwDataSize)
{
	if(sizeof(CMD_S_OperateTip_Send) != dwDataSize)
	{
		return CT_FALSE;
	}

	//CMD_S_OperateTip_Send *pSend = (CMD_S_OperateTip_Send *)pData;

	CMD_C_OperateSend opsend;
	opsend.dwUserID = m_pTableFrame->GetUserID(m_dwSelf);
	opsend.cbCardId = INVALID_CARD_DATA;
	opsend.wOperateCode = WIK_NULL_MY;
	m_pTableFrame->OnGameEvent(m_pAndroidUserItem->GetChairID(), SUB_C_OPERATE_SEND, &opsend, sizeof(CMD_C_OperateSend));

	return CT_TRUE;
}

CT_BOOL CAndroidUserItemSink::OnMsg_OperateTip_OutCard(const CT_VOID *pData, CT_DWORD dwDataSize)
{
	if(sizeof(CMD_S_OperateTip) != dwDataSize)
	{
		return CT_FALSE;
	}

	CMD_C_OperateOut out;
	out.dwUserID = m_pTableFrame->GetUserID(m_pAndroidUserItem->GetChairID());
	out.wOperateCode = WIK_NULL_MY;
	m_pTableFrame->OnGameEvent(m_pAndroidUserItem->GetChairID(), SUB_C_OPERATE_OUT, &out, sizeof(CMD_C_OperateOut));
	return CT_TRUE;
}

bool CAndroidUserItemSink::OnGameMessage(CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize)
{
	if(pData == NULL)
	{
		LOG(ERROR) << "wSubCmdID:" << wSubCmdID << " pData is equal to null, "<< "wDataSize: "<<wDataSize;
		return CT_FALSE;
	}

	switch(wSubCmdID)
	{
		case SUB_S_GAME_START:
		{
			return OnMsg_GameStart(pData, wDataSize);
			break;
		}
		case SUB_S_SEND_CARD:
		{
			return OnMsg_SendCard(pData, wDataSize);
			break;
		}
		case SUB_S_OUT_TIP:
		{
			return OnMsg_OutCard(pData, wDataSize);
			break;
		}
		case SUB_S_OPERATE_TIP_SEND:
		{
			return OnMsg_OperateTip_SendCard(pData, wDataSize);
			break;
		}
		case SUB_S_OPERATE_TIP_OUT:
		{
			return OnMsg_OperateTip_OutCard(pData, wDataSize);
			break;
		}
		default:
			//LOG(ERROR) << "未处理的消息ID: "<< wSubCmdID;
			break;
	}
	return CT_TRUE;
}

