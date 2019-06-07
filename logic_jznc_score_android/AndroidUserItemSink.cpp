
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
#include "DataQueue.h"
#include <algorithm>


#define IDI_TIME_BET					400					//下注定时器
#define IDI_TIME_THROW_DICE				401					//投掷骰子

#define PASSCARD_TIME(A) (((A) < 2)? 2:(A))

bool complare(unsigned char a, unsigned char b)
{
	return a > b;
}

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
	, m_gen(m_rd())
{
	srand(time(NULL));
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
    ReadJsonCfg();
}

bool CAndroidUserItemSink::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{

    m_pAndroidUserItem->KillTimer(dwTimerID);
    dwTimerID -= (dwParam%300);
	switch (dwTimerID)
	{
	case IDI_TIME_BET:
	{
		CMD_C_UserBet UserBet;
		UserBet.wChairID = m_wChairID;
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_BET, &UserBet, sizeof(UserBet));
		//LOG(INFO) << "ontimer IDI_ROBOT_OPEN_HB time";
	}
	break;
	case IDI_TIME_THROW_DICE:
	{
		CMD_C_ThrowDice ThrowDice;
		ThrowDice.wChairID = m_wChairID;
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_THROW_DICE, &ThrowDice, sizeof(ThrowDice));
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
	case SUB_S_START_GAME:
	{
		return OnSubGameStart(pData, wDataSize);
	}
	case SUB_S_BET:
	{
		return true;
	}
	case SUB_S_START_THROW_DICE:
	{
		return OnStartThrowDice(pData, wDataSize);
	}
	case SUB_S_THROW_DICE:
	{
		return true;
	}
	default: break;
	}
	return true;
}

bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_StartGame))
	{
		return false;
	}

	//拷贝手牌数据
	CMD_S_StartGame* pNetInfo = (CMD_S_StartGame*)pBuffer;
	if (!pNetInfo)
	{
		LOG(ERROR) << "pNetInfo==NULL";
		return false;
	}
	if(pNetInfo->wTimeLeft <= 2)
	{
		pNetInfo->wTimeLeft = 3;
	}
	std::uniform_int_distribution<unsigned int> disTime((TIME_ANIMATION* 1000) + 500, ((pNetInfo->wTimeLeft+TIME_ANIMATION)* 1000) -1000);
	m_pAndroidUserItem->SetTimer(IDI_TIME_BET,  disTime(m_gen));
	//LOG(INFO) << "set IDI_ROBOT_OPEN_HB time: "<< pGameStart->wGrabTime * 1000;

	return true;
}
bool CAndroidUserItemSink::OnStartThrowDice(const void * pBuffer, CT_DWORD wDataSize)
{

	if (wDataSize != sizeof(CMD_S_StartThrowDice))
	{
		return false;
	}

	//拷贝手牌数据
	CMD_S_StartThrowDice* pNetInfo = (CMD_S_StartThrowDice*)pBuffer;
	if (!pNetInfo)
	{
		LOG(ERROR) << "pNetInfo==NULL";
		return false;
	}
	if(pNetInfo->wTimeLeft <= 2)
	{
		pNetInfo->wTimeLeft = 3;
	}
	std::uniform_int_distribution<unsigned int> disTime((TIME_ANIMATION_DICE*1000)+500, ((pNetInfo->wTimeLeft+TIME_ANIMATION_DICE)*1000)-1000);
	m_pAndroidUserItem->SetTimer(IDI_TIME_THROW_DICE, disTime(m_gen));
	//LOG(INFO) << "set IDI_ROBOT_OPEN_HB time: "<< pGameStart->wGrabTime * 1000;

	return true;
}

bool CAndroidUserItemSink::ReadFile(std::string & file, std::string & str)
{
	std::ifstream in;
	std::string line;

	in.open(file, std::ifstream::in);
	if (!in.is_open())
		return false;
	while (getline(in, line)) {
		str.append(line + "\n");
	}
	in.close();
	return true;
}
void CAndroidUserItemSink::ReadJsonCfg()
{


	return;
}