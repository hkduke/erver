
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
#include "MSG_DDZ.h"
#include "../GameServer/DdzAiThread.h"
#include "DataQueue.h"
#include <algorithm> 
#ifdef _OS_LINUX_CODE
#include "../DdzAi/DdzAiLib/DdzAiInterface.h"
#endif // _OS_LINUX_CODE


#define IDI_CALL_BANKER				100					//叫地主
#define IDI_ADD_DOUBLE				200					//加陪
#define IDI_OUT_CARD				300					//出牌

#define TIME_DEAL_CARD				2					//发牌时间(2)
#define TIME_CALL_BANKER			15					//叫地主时间(15)
#define TIME_ADD_DOUBLE				5					//加陪时间(5)
#define	TIME_OUT_CARD				15					//出牌时间(15)

bool complare(unsigned char a, unsigned char b)
{
	return a > b;
}

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
	, m_dwBankerUser(INVALID_CHAIR)
	, m_dwCurrentUser(INVALID_CHAIR)
	, m_cbAiCallScore(0)
	, m_cbAiOutCardType(0)
	, m_cbAiOutCardCount(0)
	, m_cbLastUserOutCount(0)
	, m_dwLastOutCardUser(INVALID_CHAIR)
	, m_dwLastOutCardTime(0)
	, m_pDdzAiThread(NULL)
{
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(m_cbAiOutCardData, 0, sizeof(m_cbAiOutCardData));
	memset(m_cbLastUserOutCardData, 0, sizeof(m_cbLastUserOutCardData));
}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{
	m_pTableFrame = NULL;
	m_wChairID = INVALID_CHAIR;
	m_pAndroidUserItem = NULL;
	m_dwBankerUser = INVALID_CHAIR;
	m_dwCurrentUser = INVALID_CHAIR;
	m_cbAiCallScore = 0;
	m_cbAiOutCardType = 0;
	m_cbAiOutCardCount = 0;
	m_cbLastUserOutCount = 0;
	m_dwLastOutCardUser = INVALID_CHAIR;
	m_dwLastOutCardTime = 0;
	memset(&m_cbAiOutCardData, 0, sizeof(m_cbAiOutCardData));
	memset(&m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(&m_cbLastUserOutCardData, 0, sizeof(m_cbLastUserOutCardData));
	m_pDdzAiThread = NULL;
}

void CAndroidUserItemSink::Initialization(ITableFrame* pTableFrame, CT_DWORD wChairID, IServerUserItem* pAndroidUserItem)
{
	m_pTableFrame = pTableFrame;
	m_wChairID = wChairID;
	m_pAndroidUserItem = pAndroidUserItem;
}

void CAndroidUserItemSink::HandleDdzAiAction(std::shared_ptr<ai_action>& aiAction)
{
#ifdef _OS_LINUX_CODE
	if (aiAction->uUserID != m_pTableFrame->GetUserID(m_dwCurrentUser))
	{
	    CT_DWORD dwUseTime = time(NULL) - aiAction->uTime;
		LOG(WARNING) << "ai action fail, not current user! event: " << (int)aiAction->event << ", use time: " << dwUseTime;

		//强制取消一次AI托管
        CMD_C_CancelTrustee cancelTrustee;
        cancelTrustee.wChairID = m_wChairID;
        m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_CANCEL_TRUSTEE, &cancelTrustee, sizeof(CMD_C_CancelTrustee));
		return;
	}

	/*if (aiAction->_type == ddz_type_i_triple_2)
	{
		char szInfoMsg[256] = { 0 };
		_snprintf_info(szInfoMsg, sizeof(szInfoMsg), "handle ai action, event=%d ----->type=%d alone1=%d alone2=%d alone3=%d, alone4=%d combo_count=%d", \
			aiAction->event, aiAction->_type, aiAction->_alone_1, aiAction->_alone_2, aiAction->_alone_3, aiAction->_alone_4, aiAction->_combo_count);
		LOG(INFO) << szInfoMsg;
		if (aiAction->_combo_count)
		{
			for (int i = 0; i < 20; i++)
			{
				if (aiAction->_combo_list[i] == 0)
					break;
				printf("%d, ", aiAction->_combo_list[i]);
			}
			printf("\n");
		}
	}*/
	
	if (aiAction->event == pai_event_play_card || aiAction->event == pai_event_play_card_passive)
	{
		m_cbAiOutCardType = aiAction->_type;

		//判断时间
		CT_DWORD dwNow = time(NULL);
		CT_DWORD Interval = dwNow - m_dwLastOutCardTime;

		//如果要出牌，则分析AI返回的数据
		if (aiAction->_type != ddz_type_i_no_move)
		{
			if (aiAction->_combo_count != 0)
			{
				std::sort(aiAction->_combo_list, aiAction->_combo_list + aiAction->_combo_count, complare);
			}
			HandleAiOutCardAction(aiAction);

			//设置出牌定时器
			//CT_BYTE cbOutCardTime = rand() % (TIME_OUT_CARD - 11) + 1;
			CT_BYTE cbOutCardTime = (Interval >= 5 ? 1 : (rand() % 3 + 1));
			m_pAndroidUserItem->SetTimer(IDI_OUT_CARD, cbOutCardTime * 1000);
		}
		else
		{
			//设置不出牌定时器
			CT_BYTE cbOutCardTime = rand() % (2) + 1;
			m_pAndroidUserItem->SetTimer(IDI_OUT_CARD, cbOutCardTime * 1000);
		}
	}
	else if (aiAction->event == pai_event_grab_landlord)
	{
		m_cbAiCallScore = aiAction->callLandScore;
		CT_BYTE cbCallBankerTime = rand() % (TIME_CALL_BANKER - 13) + 2;
		m_pAndroidUserItem->SetTimer(IDI_CALL_BANKER, cbCallBankerTime * 1000);
	}

	// 把空闲的AI回调对象放回到的空闲列表
	aiAction->reset();
	m_pDdzAiThread->InsertFreeAiActionPtr(aiAction);
#endif // _OS_LINUX_CODE
}

bool CAndroidUserItemSink::OnTimerMessage(CT_DWORD dwTimerID, CT_DWORD dwParam)
{
	switch (dwTimerID)
	{
	case IDI_CALL_BANKER:
	{
		m_pAndroidUserItem->KillTimer(dwTimerID);
		CMD_C_CallBanker callBanker;
		if (m_cbAiCallScore == 0)
		{
			callBanker.cbCallInfo = CB_NO_CALL_BENKER;
		}
		else
		{
			callBanker.cbCallInfo = m_cbAiCallScore;
		}
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_CALL_BANKER, &callBanker, sizeof(callBanker));
	}
	break;

	case IDI_ADD_DOUBLE:
	{
		m_pAndroidUserItem->KillTimer(dwTimerID);
		int nRandomNum = rand() % 4;

		CMD_C_Double addDouble;
		if (nRandomNum == 0)
		{
			addDouble.cbDoubleInfo = 1;
		}
		else
		{
			addDouble.cbDoubleInfo = 2;
		}
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ADD_DOUBLE, &addDouble, sizeof(addDouble));
	}
	break;

	case IDI_OUT_CARD:
	{
		m_pAndroidUserItem->KillTimer(dwTimerID);

		if (m_cbAiOutCardType != 0)
		{
			m_GameLogic.SortCardList(m_cbAiOutCardData, m_cbAiOutCardCount, ST_ORDER);
			CMD_C_OutCard outCard;
			memset(&outCard, 0, sizeof(outCard));
			outCard.cbCardCount = m_cbAiOutCardCount;
			memcpy(outCard.cbCardData, m_cbAiOutCardData, m_cbAiOutCardCount);
			m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_OUT_CARD, &outCard, sizeof(outCard));

			//清空一下出牌数据
			m_cbAiOutCardType = 0;
			m_cbAiOutCardCount = 0;
			memset(m_cbAiOutCardData, 0, sizeof(m_cbAiOutCardData));
		}
		else
		{
			m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_PASS_CARD, NULL, 0);
		}
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
	case SUB_S_GAME_START:
	{
		return OnSubGameStart(pData, wDataSize);
	}

	case SUB_S_CALL_BANKER:
	{
		return OnSubGameCallBank(pData, wDataSize);
	}
	case SUB_S_BANKER_INFO:
	{
		return OnSubBankerInfo(pData, wDataSize);
	}
	case SUB_S_OUT_CARD:
	{
		return OnSubUserOutCard(pData, wDataSize);
	}
	case SUB_S_PASS_CARD:
	{
		return OnSubUserPass(pData, wDataSize);
	}
	case SUB_S_OUT_START_START:
	{
		return OnSubStartCard(pData, wDataSize);
	}
	case SUB_S_GAME_CONCLUDE:
	{
		return OnGameOver(pData, wDataSize);
	}
	case SUB_S_ADD_DOUBLE:
	{

	}
	default: break;
	}
	return true;
}

bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_GameStartAi))
	{
		return false;
	}

	//拷贝手牌数据
	CMD_S_GameStartAi* pGameStart = (CMD_S_GameStartAi*)pBuffer;
	if (pGameStart)
	{
		//memcpy(m_cbHandCardData, pGameStart->cbCardData, sizeof(pGameStart->cbCardData));


		//记录当前玩家
		m_dwCurrentUser = pGameStart->dwCurrentUser;
		memcpy(m_cbBankerCard, pGameStart->cbBankerCard, sizeof(m_cbBankerCard));

		for (int i = 0; i < GAME_PLAYER; ++i)
		{
			m_cbHandCardCount[i] = NORMAL_COUNT;
			memcpy(&m_cbHandCardData[i], pGameStart->cbCardData[i], NORMAL_COUNT);
		}

		//for (int i = 0; i < GAME_PLAYER; i++)
		//{
		//	printf("player %d hand card: ", i);
		//	for (int j = 0; j < NORMAL_COUNT; ++j)
		//	{
		//		printf("%x, ", m_cbHandCardData[i][j]);
		//	}
		//	printf("\n");
		//}
		//printf("\n");



#ifdef _OS_LINUX_CODE
		//如果当前玩家是本家，给AI发送数据，确定是否叫庄
		if (m_dwCurrentUser == m_wChairID)
		{
			SendCallBankerDataToAi();
		}
#endif // _OS_LINUX_CODE
	
	}
	//m_pAndroidUserItem->SetTimer(ID_TIME_CALL_BANKER, TIME_CALL_BANKER, 0);

	return true;
}

bool CAndroidUserItemSink::OnSubGameCallBank(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_CallBanker))
	{
		return false;
	}

	CMD_S_CallBanker* pCallBanker = (CMD_S_CallBanker*)pBuffer;

	m_dwCurrentUser = pCallBanker->dwCurrentUser;

#ifdef _OS_LINUX_CODE
	if (pCallBanker->dwCurrentUser == m_wChairID)
	{
		//给AI发送数据是否叫庄
		SendCallBankerDataToAi();
	}
#endif //_OS_LINUX_CODE

	return true;
}

bool CAndroidUserItemSink::OnSubBankerInfo(const void * pBuffer, CT_DWORD wDataSize)
{
	//确定庄家信息
	if (wDataSize != sizeof(CMD_S_BankerInfo))
	{
		return false;
	}

	CMD_S_BankerInfo* pBankerInfo = (CMD_S_BankerInfo*)pBuffer;
	m_dwBankerUser = pBankerInfo->dwBankerUser;
	m_dwCurrentUser = pBankerInfo->dwCurrentUser;

	//把底牌扑克给庄家
	m_cbHandCardCount[m_dwBankerUser] += 3;
	memcpy(&m_cbHandCardData[m_dwBankerUser][NORMAL_COUNT], pBankerInfo->cbBankerCard, sizeof(m_cbBankerCard));
	//排列扑克
	m_GameLogic.SortCardList(m_cbHandCardData[m_dwBankerUser], m_cbHandCardCount[m_dwBankerUser], ST_ORDER);

	//起一个定时器，加不加倍
	if (m_dwBankerUser != m_wChairID)
	{
		int nAddDoubleTime = rand() % (TIME_ADD_DOUBLE - 2) + 1;
		m_pAndroidUserItem->SetTimer(IDI_ADD_DOUBLE, nAddDoubleTime * 1000);
	}

	return true;
}

bool CAndroidUserItemSink::OnSubUserAddDouble(const void * pBuffer, CT_DWORD wDataSize)
{
	return true;
}

bool CAndroidUserItemSink::OnSubUserPass(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_PassCard))
	{
		return false;
	}

	CMD_S_PassCard* pPassCard = (CMD_S_PassCard*)pBuffer;
	m_dwCurrentUser = pPassCard->dwCurrentUser;

	if (m_dwCurrentUser == m_wChairID)
	{
		if (pPassCard->cbTurnOver)
		{
			m_cbLastUserOutCount = 0;
			memset(m_cbLastUserOutCardData, 0, sizeof(m_cbLastUserOutCardData));
			SendAiPlayCardToAi();
		}
		else
		{
			SendAiPlayCardPassiveToAi();
		}
	}

	return true;
}

bool CAndroidUserItemSink::OnSubUserOutCard(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize > sizeof(CMD_S_OutCard))
	{
		return false;
	}

	CMD_S_OutCard* pOutCard = (CMD_S_OutCard*)pBuffer;
	m_dwCurrentUser = pOutCard->dwCurrentUser;
	m_dwLastOutCardUser = pOutCard->dwOutCardUser;
	//用户出牌数据
	m_cbLastUserOutCount = pOutCard->cbCardCount;

	memset(m_cbLastUserOutCardData, 0, sizeof(m_cbLastUserOutCardData));
	memcpy(m_cbLastUserOutCardData, pOutCard->cbCardData, pOutCard->cbCardCount);
	m_GameLogic.SortCardList(m_cbLastUserOutCardData, m_cbLastUserOutCount, ST_ORDER);

	
	//删除手牌数目
	if (m_GameLogic.RemoveCardList(m_cbLastUserOutCardData, m_cbLastUserOutCount, m_cbHandCardData[m_dwLastOutCardUser], m_cbHandCardCount[m_dwLastOutCardUser]) == false)
	{
		LOG(WARNING) << "ai remove card list fail?, out card user id : " << m_pTableFrame->GetUserID(m_dwLastOutCardUser);
	}

	m_cbHandCardCount[m_dwLastOutCardUser] -= m_cbLastUserOutCount;

	//如果是当前用户
	if (m_dwCurrentUser != INVALID_CHAIR && m_dwCurrentUser == m_wChairID)
	{
		/*printf("out card data: ");
		for (int i = 0; i < m_cbLastUserOutCount; i++)
		{
			printf(" %x, ", m_cbLastUserOutCardData[i]);
		}
		printf("\n");*/
		SendAiPlayCardPassiveToAi();
		m_dwLastOutCardTime = time(NULL);
	}

	return true;
}

bool CAndroidUserItemSink::OnSubStartCard(const void * pBuffer, CT_DWORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_StartOutCard))
	{
		return false;
	}

	CMD_S_StartOutCard* pStartOutCard = (CMD_S_StartOutCard*)pBuffer;
	m_dwBankerUser = pStartOutCard->dwBankerUser;
	m_dwCurrentUser = pStartOutCard->dwCurrentUser;

	//AI主动出牌
	if (m_wChairID == m_dwCurrentUser)
	{
		SendAiPlayCardToAi();
	}

	return true;
}

bool CAndroidUserItemSink::OnGameOver(const void * pBuffer, CT_DWORD wDataSize)
{
	return true;
}

void CAndroidUserItemSink::HandleAiOutCardAction(std::shared_ptr<ai_action>& aiAction)
{
#ifdef _OS_LINUX_CODE
	switch (aiAction->_type)
	{
	case ddz_type_i_alone_1:
	{
		m_cbAiOutCardCount = 1;
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[0] = cbCardValue;
				break;
			}
		}
	}
	break;
	case ddz_type_i_pair:
	{
		m_cbAiOutCardCount = 2;
		CT_BYTE cbOutCardCount = 0;
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 2)
					break;
			}
		}
	}
	break;
	case ddz_type_i_triple:
	{
		m_cbAiOutCardCount = 3;
		CT_BYTE cbOutCardCount = 0;
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 3)
					break;
			}
		}

	}
	break;
	case ddz_type_i_triple_1:
	{
		m_cbAiOutCardCount = 4;
		CT_BYTE cbOutCardCount = 0;

		//三张牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 3)
					break;
			}
		}
		//单牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_2)
			{
				m_cbAiOutCardData[cbOutCardCount] = cbCardValue;
				break;
			}
		}
	}
	break;
	case ddz_type_i_triple_2:
	{
		m_cbAiOutCardCount = 5;
		CT_BYTE cbOutCardCount = 0;

		//
		if (aiAction->_alone_1 == aiAction->_alone_2)
		{
			m_cbAiOutCardType = ddz_type_i_no_move;
			LOG(ERROR) << "ai return triple_2 error, _alone_1 equal _alone_2.";
			break;
		}

		//三张牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 3)
					break;
			}
		}

		//对牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_2)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 5)
					break;
			}
		}
	}
	break;
	case ddz_type_i_order:
	{
		m_cbAiOutCardCount = aiAction->_combo_count;

		CT_BYTE cbOutCardCount = 0;
		//顺子牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_combo_list[cbOutCardCount])
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == aiAction->_combo_count)
					break;
			}
		}
	}
	break;
	case ddz_type_i_order_pair:
	{
		m_cbAiOutCardCount = aiAction->_combo_count;

		CT_BYTE cbOutCardCount = 0;
		//连对牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_combo_list[cbOutCardCount])
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == aiAction->_combo_count)
					break;
			}
		}
	}
	break;
	case ddz_type_i_airplane:
	{
		m_cbAiOutCardCount = aiAction->_combo_count;

		CT_BYTE cbOutCardCount = 0;
		//飞机牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_combo_list[cbOutCardCount])
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == aiAction->_combo_count)
					break;
			}
		}
	}
	break;
	case ddz_type_i_airplane_with_pai:
	{
		m_cbAiOutCardCount = aiAction->_combo_count;

		std::vector<CT_BYTE> vecWingCard;

		CT_BYTE cbOutCardCount = 0;
		//飞机牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_combo_list[cbOutCardCount])
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				//翅膀不能包括飞机牌
				vecWingCard.push_back(cbCardValue);
				if (cbOutCardCount == aiAction->_combo_count)
					break;
			}
		}

		//翅膀1
		if (aiAction->_alone_1 != 0)
		{
			CT_BYTE cbWingCardCount = 0;
			for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
			{
				CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
				//这张牌已经找到了
				auto it = std::find(vecWingCard.begin(), vecWingCard.end(), cbCardValue);
				if (it != vecWingCard.end())
					continue;

				CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
				if (cbLogicValue == aiAction->_alone_1)
				{
					m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
					m_cbAiOutCardCount++;
					cbWingCardCount++;
					vecWingCard.push_back(cbCardValue);
					if (aiAction->_airplane_pairs == 0)
					{
						break;
					}
					else
					{
						if (cbWingCardCount >= 2)
						{
							break;
						}
					}
				}
			}
		}

		//翅膀2
		if (aiAction->_alone_2 != 0)
		{
			CT_BYTE cbWingCardCount = 0;
			for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
			{
				CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
				//这张牌已经找到了
				auto it = std::find(vecWingCard.begin(), vecWingCard.end(), cbCardValue);
				if (it != vecWingCard.end())
					continue;
				CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
				if (cbLogicValue == aiAction->_alone_2)
				{
					m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
					m_cbAiOutCardCount++;
					cbWingCardCount++;
					vecWingCard.push_back(cbCardValue);
					if (aiAction->_airplane_pairs == 0)
					{
						break;
					}
					else
					{
						if (cbWingCardCount >= 2)
						{
							break;
						}
					}
				}
			}
		}

		//翅膀3
		if (aiAction->_alone_3 != 0)
		{
			CT_BYTE cbWingCardCount = 0;
			for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
			{
				CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
				//这张牌已经找到了
				auto it = std::find(vecWingCard.begin(), vecWingCard.end(), cbCardValue);
				if (it != vecWingCard.end())
					continue;
				CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
				if (cbLogicValue == aiAction->_alone_3)
				{
					m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
					m_cbAiOutCardCount++;
					cbWingCardCount++;
					vecWingCard.push_back(cbCardValue);
					if (aiAction->_airplane_pairs == 0)
					{
						break;
					}
					else
					{
						if (cbWingCardCount >= 2)
						{
							break;
						}
					}
				}
			}
		}

		//翅膀4
		if (aiAction->_alone_4 != 0)
		{
			CT_BYTE cbWingCardCount = 0;
			for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
			{
				CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
				//这张牌已经找到了
				auto it = std::find(vecWingCard.begin(), vecWingCard.end(), cbCardValue);
				if (it != vecWingCard.end())
					continue;
				CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
				if (cbLogicValue == aiAction->_alone_4)
				{
					m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
					m_cbAiOutCardCount++;
					cbWingCardCount++;
					vecWingCard.push_back(cbCardValue);
					if (aiAction->_airplane_pairs == 0)
					{
						break;
					}
					else
					{
						if (cbWingCardCount >= 2)
						{
							break;
						}
					}
				}
			}
		}


	}
	break;

	case ddz_type_i_bomb:
	{
		m_cbAiOutCardCount = 4;

		CT_BYTE cbOutCardCount = 0;
		//顺子牌
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 4)
					break;
			}
		}
	}
	break;

	case ddz_type_i_king_bomb:
	{
		m_cbAiOutCardCount = 2;
		m_cbAiOutCardData[0] = 0x4e;
		m_cbAiOutCardData[1] = 0x4f;

	}
	break;
	
	case ddz_type_i_four_with_alone1:
	{
		m_cbAiOutCardCount = 6;

		CT_BYTE cbOutCardCount = 0;
		//炸弹
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 4)
					break;
			}
		}

		//翅膀1
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_2)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				break;
			}
		}

		//翅膀2
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_3)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				break;
			}
		}
	}
	break;
	case ddz_type_i_four_with_pairs:
	{
		m_cbAiOutCardCount = 8;

		CT_BYTE cbOutCardCount = 0;
		//炸弹
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_1)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				if (cbOutCardCount == 4)
					break;
			}
		}

		//翅膀1
		CT_BYTE cbWingCount = 0;
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_2)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				cbWingCount++;
				if (cbWingCount >= 2)
					break;
			}
		}

		//翅膀2
		cbWingCount = 0;
		for (CT_BYTE i = 0; i < m_cbHandCardCount[m_dwCurrentUser]; ++i)
		{
			CT_BYTE cbCardValue = m_cbHandCardData[m_dwCurrentUser][i];
			CT_BYTE cbLogicValue = m_GameLogic.GetCardLogicValue(cbCardValue);
			if (cbLogicValue == aiAction->_alone_3)
			{
				m_cbAiOutCardData[cbOutCardCount++] = cbCardValue;
				cbWingCount++;
				if (cbWingCount >= 2)
					break;
			}
		}
	}
	break;
	default:
	{
		m_cbAiOutCardType = ddz_type_i_no_move;
		break;
	}
	}

#endif // _OS_LINUX_CODE
}


void CAndroidUserItemSink::ChangeCardDataForAi(CT_DWORD wChairID, unsigned char cardList[])
{
	for (CT_BYTE cbCardIndex = 0; cbCardIndex < m_cbHandCardCount[wChairID]; ++cbCardIndex)
	{
		CT_BYTE cbCardData = m_cbHandCardData[wChairID][cbCardIndex];
		CT_BYTE cbCardLogicValue = m_GameLogic.GetCardLogicValue(cbCardData);
		//扑克牌逻辑
		cardList[cbCardLogicValue]++;
	}
}

void CAndroidUserItemSink::SendCallBankerDataToAi()
{
#ifdef _OS_LINUX_CODE
	std::shared_ptr<ai_event> aiEvent = m_pDdzAiThread->GetFreeAiEeventPtr();
	aiEvent->uUserID = m_pTableFrame->GetUserID(m_wChairID);
	aiEvent->event = pai_event_grab_landlord;

	//三家的扑克
	ChangeCardDataForAi(m_dwCurrentUser, aiEvent->pai_list);

	CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
	ChangeCardDataForAi(dwNextUser, aiEvent->pai_list2);

	dwNextUser = (dwNextUser + 1) % GAME_PLAYER;
	ChangeCardDataForAi(dwNextUser, aiEvent->pai_list3);

	//底牌
	for (CT_BYTE i = 0; i < 3; ++i)
	{
		aiEvent->grab_pai[i] = m_GameLogic.GetCardLogicValue(m_cbBankerCard[i]);
	}

	//插入队列，等待处理
	m_pDdzAiThread->InsertAiEvent(aiEvent); 
	//LOG(WARNING) << "send call banker data to ai. current user: " << m_dwCurrentUser;

#endif // #ifdef _OS_LINUX_CODE
}

void CAndroidUserItemSink::SendAiPlayCardToAi()
{
#ifdef _OS_LINUX_CODE
	std::shared_ptr<ai_event> aiEvent = m_pDdzAiThread->GetFreeAiEeventPtr();
	aiEvent->uUserID = m_pTableFrame->GetUserID(m_wChairID);
	aiEvent->event = pai_event_play_card;
	aiEvent->uTime = time(NULL);

	//三家的扑克
	ChangeCardDataForAi(m_dwCurrentUser, aiEvent->pai_list);
	if (m_dwBankerUser == m_dwCurrentUser)
	{
		aiEvent->landlord = 0;
	}

	CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
	ChangeCardDataForAi(dwNextUser, aiEvent->pai_list2);
	if (m_dwBankerUser == dwNextUser)
	{
		aiEvent->landlord = 1;
	}

	dwNextUser = (dwNextUser + 1) % GAME_PLAYER;
	ChangeCardDataForAi(dwNextUser, aiEvent->pai_list3);
	if (m_dwBankerUser == dwNextUser)
	{
		aiEvent->landlord = 2;
	}

	//插入队列，等待处理
	m_pDdzAiThread->InsertAiEvent(aiEvent);

#endif // #ifdef _OS_LINUX_CODE
}

void CAndroidUserItemSink::SendAiPlayCardPassiveToAi()
{
#ifdef _OS_LINUX_CODE
	std::shared_ptr<ai_event> aiEvent = m_pDdzAiThread->GetFreeAiEeventPtr();
	aiEvent->uUserID = m_pTableFrame->GetUserID(m_wChairID);
	aiEvent->event = pai_event_play_card_passive;
	aiEvent->uTime = time(NULL);

	//三家的扑克
	ChangeCardDataForAi(m_dwCurrentUser, aiEvent->pai_list);
	if (m_dwBankerUser == m_dwCurrentUser)
	{
		aiEvent->landlord = 0;
	}

	CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
	ChangeCardDataForAi(dwNextUser, aiEvent->pai_list2);
	if (m_dwBankerUser == dwNextUser)
	{
		aiEvent->landlord = 1;
	}
	if (dwNextUser == m_dwLastOutCardUser)
	{
		aiEvent->outPaiIndex = 1;
	}

	dwNextUser = (dwNextUser + 1) % GAME_PLAYER;
	ChangeCardDataForAi(dwNextUser, aiEvent->pai_list3);
	if (m_dwBankerUser == dwNextUser)
	{
		aiEvent->landlord = 2;
	}
	if (dwNextUser == m_dwLastOutCardUser)
	{
		aiEvent->outPaiIndex = 2;
	}

	//分析中已经出的牌
	CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbLastUserOutCardData, m_cbLastUserOutCount);
	if (cbCardType == CT_ERROR)
	{

		LOG(WARNING) << "ai get last out card type error. ";
		return;
	}


	pai_interface_move& moveCard = aiEvent->limit_card;
	switch (cbCardType)
	{
	case CT_SINGLE:
	{
		moveCard._type = ddz_type_i_alone_1;
		moveCard._alone_1 = m_GameLogic.GetCardLogicValue(m_cbLastUserOutCardData[0]);
	}
	break;
	case CT_DOUBLE_CARD:
	{
		moveCard._type = ddz_type_i_pair;
		moveCard._alone_1 = m_GameLogic.GetCardLogicValue(m_cbLastUserOutCardData[0]);
	}
	break;
	case CT_THREE:
	{
		moveCard._type = ddz_type_i_triple;
		moveCard._alone_1 = m_GameLogic.GetCardLogicValue(m_cbLastUserOutCardData[0]);
	}
	break;
	case CT_SINGLE_LINE:
	{
		moveCard._type = ddz_type_i_order;
		moveCard._combo_count = m_cbLastUserOutCount;

		unsigned char comboCount = 0;
		for (int i = (m_cbLastUserOutCount-1); i >=0; --i)
		{
			moveCard._combo_list[comboCount++] =   m_GameLogic.GetCardLogicValue(m_cbLastUserOutCardData[i]);
		}
	}
	break;
	case CT_DOUBLE_LINE:
	{
		moveCard._type = ddz_type_i_order_pair;
		moveCard._combo_count = m_cbLastUserOutCount;

		unsigned char comboCount = 0;
		for (int i = (m_cbLastUserOutCount - 1); i >= 0; --i)
		{
			moveCard._combo_list[comboCount++] = m_GameLogic.GetCardLogicValue(m_cbLastUserOutCardData[i]);
		}
	}
	break;
	case CT_THREE_LINE:
	{
		moveCard._type = ddz_type_i_airplane;
		moveCard._combo_count = m_cbLastUserOutCount;

		unsigned char comboCount = 0;
		for (int i = (m_cbLastUserOutCount - 1); i >= 0; --i)
		{
			moveCard._combo_list[comboCount++] = m_GameLogic.GetCardLogicValue(m_cbLastUserOutCardData[i]);
		}

	}
	break;
	case CT_THREE_TAKE_ONE:
	{
		tagAnalyseResult AnalyseResult;
		m_GameLogic.AnalysebCardData(m_cbLastUserOutCardData, m_cbLastUserOutCount, AnalyseResult);

		if (m_cbLastUserOutCount == 4)
		{
			moveCard._type = ddz_type_i_triple_1;
			//分析扑克
			moveCard._alone_1 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][0]);
			moveCard._alone_2 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][0]);
		}
		else
		{
			moveCard._type = ddz_type_i_airplane_with_pai;
			moveCard._airplane_pairs = 0;

			moveCard._combo_count = AnalyseResult.cbBlockCount[2] * 3;
	
			//飞机的数据
			unsigned char comboCount = 0;
			for (int i = (moveCard._combo_count - 1); i >= 0; --i)
			{
				moveCard._combo_list[comboCount++] = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][i]);
			}

			//翅膀数据
			if (AnalyseResult.cbCardData[0][0] != 0)
			{
				moveCard._alone_1 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][0]);
			}
			if (AnalyseResult.cbCardData[0][1] != 0)
			{
				moveCard._alone_2 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][1]);
			}
			if (AnalyseResult.cbCardData[0][2] != 0)
			{
				moveCard._alone_3 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][2]);
			}
			if (AnalyseResult.cbCardData[0][3] != 0)
			{
				moveCard._alone_4 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][3]);
			}
		}

	}
	break;
	case CT_THREE_TAKE_TWO:	
	{
		tagAnalyseResult AnalyseResult;
		m_GameLogic.AnalysebCardData(m_cbLastUserOutCardData, m_cbLastUserOutCount, AnalyseResult);

		if (m_cbLastUserOutCount == 5)
		{
			moveCard._type = ddz_type_i_triple_2;
			//分析扑克
			moveCard._alone_1 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][0]);
			moveCard._alone_2 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[1][0]);
		}
		else
		{
			moveCard._type = ddz_type_i_airplane_with_pai;
			moveCard._airplane_pairs = 1;

			moveCard._combo_count = AnalyseResult.cbBlockCount[2] * 3;
			
			//飞机的数据
			unsigned char comboCount = 0;
			for (int i = (moveCard._combo_count - 1); i >= 0; --i)
			{
				moveCard._combo_list[comboCount++] = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[2][i]);
			}

			//翅膀数据
			if (AnalyseResult.cbCardData[1][0] != 0)
			{
				moveCard._alone_1 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[1][0]);
			}
			if (AnalyseResult.cbCardData[1][2] != 0)
			{
				moveCard._alone_2 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[1][2]);
			}
			if (AnalyseResult.cbCardData[1][4] != 0)
			{
				moveCard._alone_3 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[1][4]);
			}
			if (AnalyseResult.cbCardData[1][6] != 0)
			{
				moveCard._alone_4 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[1][6]);
			}
		}
	}
	break;
	case CT_FOUR_TAKE_ONE:
	{
		tagAnalyseResult AnalyseResult;
		m_GameLogic.AnalysebCardData(m_cbLastUserOutCardData, m_cbLastUserOutCount, AnalyseResult);

		moveCard._type = ddz_type_i_four_with_alone1;
		moveCard._alone_1 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[3][0]);
		moveCard._alone_2 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][0]);
	}
	break;
	case CT_FOUR_TAKE_TWO:
	{
		tagAnalyseResult AnalyseResult;
		m_GameLogic.AnalysebCardData(m_cbLastUserOutCardData, m_cbLastUserOutCount, AnalyseResult);

		moveCard._type = ddz_type_i_four_with_alone1;
		moveCard._alone_1 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[3][0]);
		moveCard._alone_2 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][0]);
		moveCard._alone_3 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[0][2]);
	}
	break;
	case CT_BOMB_CARD:
	{
		tagAnalyseResult AnalyseResult;
		m_GameLogic.AnalysebCardData(m_cbLastUserOutCardData, m_cbLastUserOutCount, AnalyseResult);

		moveCard._type = ddz_type_i_bomb;
		moveCard._alone_1 = m_GameLogic.GetCardLogicValue(AnalyseResult.cbCardData[3][0]);

	}
	break;
	case CT_MISSILE_CARD:
	{
		moveCard._type = ddz_type_i_king_bomb;
	}
	break;
	default:
		break;
	}

	//插入队列，等待处理
	m_pDdzAiThread->InsertAiEvent(aiEvent);

#endif // #ifdef _OS_LINUX_CODE
}