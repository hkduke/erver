
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
#include "MSG_PDK.h"
#include "../GameServer/DdzAiThread.h"
#include "DataQueue.h"
#include <algorithm> 
#ifdef _OS_LINUX_CODE
#include "../DdzAi/DdzAiLib/DdzAiInterface.h"
#endif // _OS_LINUX_CODE


#define IDI_OUT_CARD				300					//出牌

#define TIME_IS_CAN_OUT_CARD		7					//是否能出牌

#define PASSCARD_TIME(A) (((A) < 2)? 2:(A))
#define OUTCARD_TIME(A) ((((m_cbHandCardCount[m_wChairID] < 5) || ((3 >= m_cbLastUserOutCount) &&(m_cbLastUserOutCount))) ? ((rand() % (4)) + 4):((rand() % ((A)-4)) + 4)))

bool complare(unsigned char a, unsigned char b)
{
	return a > b;
}

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
	, m_dwCurrentUser(INVALID_CHAIR)
	, m_cbAiOutCardType(0)
	, m_cbAiOutCardCount(0)
	, m_cbLastUserOutCount(0)
	, m_dwLastOutCardUser(INVALID_CHAIR)
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
	m_dwCurrentUser = INVALID_CHAIR;
	m_cbAiOutCardType = 0;
	m_cbAiOutCardCount = 0;
	m_cbLastUserOutCount = 0;
	m_dwLastOutCardUser = INVALID_CHAIR;
	memset(&m_cbAiOutCardData, 0, sizeof(m_cbAiOutCardData));
	memset(&m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(&m_cbLastUserOutCardData, 0, sizeof(m_cbLastUserOutCardData));
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
	case IDI_OUT_CARD:
	{
	    if (m_dwCurrentUser != m_wChairID)
        {
	        //LOG(INFO) <<"m_dwCurrentUser:"<< m_dwCurrentUser << " != " << m_wChairID <<" m_wChairID";
            return true;
        }
		m_pAndroidUserItem->KillTimer(dwTimerID);
        //LOG(INFO) << "OnTimerMessage robot m_wChairID: " << (int)m_wChairID;
		if (m_cbAiOutCardType != 0)
		{
			m_GameLogic.SortCardList(m_cbAiOutCardData, m_cbAiOutCardCount, ST_ACS);
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
	case SUB_S_OUT_CARD:
	{
		return OnSubUserOutCard(pData, wDataSize);
	}
	case SUB_S_PASS_CARD:
	{
		return OnSubUserPass(pData, wDataSize);
	}
	case SUB_S_GAME_CONCLUDE:
	{
		return OnGameOver(pData, wDataSize);
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
	if (!pGameStart)
	{
	    LOG(ERROR) << "pGameStart==NULL";
        return false;
    }

    memcpy(m_cbHandCardData, pGameStart->cbCardData, sizeof(pGameStart->cbCardData));
    memset(m_cbHandCardCount,NORMAL_COUNT, sizeof(m_cbHandCardCount));
    //记录当前玩家
    m_dwCurrentUser = pGameStart->dwCurrentUser;

    if (m_dwCurrentUser == m_wChairID)
    {
        //设置出牌定时器
        CT_BYTE cbOutCardTime = (rand() % (pGameStart->cbTimeLeft-13)) + 8;
        m_pAndroidUserItem->SetTimer(IDI_OUT_CARD, cbOutCardTime * 1000);
        //LOG(INFO) << "cbOutCardTime " << (int)cbOutCardTime;
		OnAutoOutCard();
    }
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
	CT_BYTE cbOutCardTime = 10;
	if (m_dwCurrentUser == m_wChairID)
	{
		if (pPassCard->cbTurnOver)
		{
			m_cbLastUserOutCount = 0;
			memset(m_cbLastUserOutCardData, 0, sizeof(m_cbLastUserOutCardData));
		}
		if (pPassCard->cbTimeLeft < TIME_IS_CAN_OUT_CARD)
		{
			m_cbAiOutCardType = 0;
			cbOutCardTime = PASSCARD_TIME(pPassCard->cbTimeLeft);
		}
		else {
            cbOutCardTime = OUTCARD_TIME(pPassCard->cbTimeLeft);
            //LOG(INFO) << "cbOutCardTime " << (int)cbOutCardTime;
			OnAutoOutCard();
		}
		//设置出牌定时器
		m_pAndroidUserItem->SetTimer(IDI_OUT_CARD, (cbOutCardTime * 1000));
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
	m_GameLogic.SortCardList(m_cbLastUserOutCardData, m_cbLastUserOutCount, ST_ACS);

	
	//删除手牌数目
	if (m_GameLogic.RemoveCardList(m_cbLastUserOutCardData, m_cbLastUserOutCount, m_cbHandCardData[m_dwLastOutCardUser], m_cbHandCardCount[m_dwLastOutCardUser]) == false)
	{
		LOG(WARNING) << "ai remove card list fail?, out card user id : " << m_pTableFrame->GetUserID(m_dwLastOutCardUser);
	}

	m_cbHandCardCount[m_dwLastOutCardUser] -= m_cbLastUserOutCount;

	CT_BYTE cbOutCardTime = 10;
	//LOG(ERROR) << "m_dwCurrentUser: " << m_dwCurrentUser << " m_wChairID: "<< m_wChairID;
	//如果是当前用户
	if (m_dwCurrentUser == m_wChairID)
	{
        if (pOutCard->cbTimeLeft < TIME_IS_CAN_OUT_CARD)
        {
            m_cbAiOutCardType = 0;
            cbOutCardTime = PASSCARD_TIME(pOutCard->cbTimeLeft);
        }
        else {
            cbOutCardTime = OUTCARD_TIME(pOutCard->cbTimeLeft);
            OnAutoOutCard();
        }
		//设置出牌定时器
		m_pAndroidUserItem->SetTimer(IDI_OUT_CARD, (cbOutCardTime * 1000));
        //LOG(ERROR) << "robot set IDI_OUT_CARD cbOutCardTime: " << (int)cbOutCardTime;
	}
	return true;
}

bool CAndroidUserItemSink::OnGameOver(const void * pBuffer, CT_DWORD wDataSize)
{
	return true;
}
CT_BOOL CAndroidUserItemSink::OtherPlayerCardAnalyse(OtherPlayerAnalyse & PlayerAnalyse)
{
    for (int l = 0; l < GAME_PLAYER; ++l)
    {
        if (l == m_wChairID) continue;

        if (MAX_COUNT == m_cbHandCardCount[l])
        {
            PlayerAnalyse.bIsChunTian = true;
        }
        tagSearchCardResult SearchCardResult;
        if (m_GameLogic.SearchSameCard(m_cbHandCardData[l], m_cbHandCardCount[l],0,4,SearchCardResult) > 0)
        {
            PlayerAnalyse.bIsOtherHasBomb = true;
        }
        if (PlayerAnalyse.cbMinPlayerCardCount > m_cbHandCardCount[l])
        {
            PlayerAnalyse.cbMinPlayerCardCount = m_cbHandCardCount[l];
        }
        //求出是否有玩家能两次出完牌
        if (m_cbHandCardCount[l] < 2)
        {
            PlayerAnalyse.bIsOtherPlayerSingle = true;
            PlayerAnalyse.cbOtherMaxSingle = ((m_GameLogic.GetLogicValue(PlayerAnalyse.cbOtherMaxSingle) < m_GameLogic.GetLogicValue(m_cbHandCardData[l][0]))?m_cbHandCardData[l][0]:PlayerAnalyse.cbOtherMaxSingle);
        }

        if (m_cbHandCardCount[l] < 4)
        {
            //搜索提示
            tagSearchCardResult	SearchCardResult2;					//搜索结果
            memset(&SearchCardResult2, 0, sizeof(SearchCardResult2));
            try
            {
                m_GameLogic.SearchOutCard(m_cbHandCardData[l], m_cbHandCardCount[l], m_cbLastUserOutCardData, m_cbLastUserOutCount,
                                          SearchCardResult2);
                for (int i = 0; i < SearchCardResult2.cbSearchCount; ++i)
                {
                    CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult2.cbResultCard[i], SearchCardResult2.cbCardCount[i]);
                    if ((CT_DOUBLE_Ex == cbSearchCardType))
                    {
                        memcpy(PlayerAnalyse.cbOehterDoubleCard,SearchCardResult2.cbResultCard[i],SearchCardResult2.cbCardCount[i]);
                        if (3 == m_cbHandCardCount[l])
                        {
                            CT_BYTE cbTempMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[l], m_cbHandCardCount[l],PlayerAnalyse.cbOtherMaxSingle);
                            if (cbTempMaxSingle > 0)
                            {
                                PlayerAnalyse.cbOtherMaxSingle = cbTempMaxSingle;
                            }

                            PlayerAnalyse.bIsOtherPlayerSingle = true;

                        }
                        PlayerAnalyse.bIsOtherPlayerDouble = true;

                    }
                }

            }
            catch (...)
            {
                SearchCardResult2.cbSearchCount = 0;
            }

        }
    }
    return true;
}

CT_BOOL CAndroidUserItemSink::OnAutoOutCard()
{
	if (m_dwCurrentUser == INVALID_CHAIR)
	{
		return true;
	}

	CT_BYTE cbCardData[MAX_COUNT] = { 0 };
	CT_BYTE cbCardCount = 0;

	CT_DWORD dwNextUser = (m_dwCurrentUser + 1) % GAME_PLAYER;
    tagAnalyseResult AnalyseResultl;
    memset(&AnalyseResultl, 0, sizeof(AnalyseResultl));
    CT_BYTE cbTempCardData[MAX_COUNT] = { 0 };
    memcpy(cbTempCardData,m_cbHandCardData[m_dwCurrentUser],m_cbHandCardCount[m_dwCurrentUser]* sizeof(CT_BYTE));

    m_GameLogic.SortCardList(cbTempCardData, m_cbHandCardCount[m_dwCurrentUser], ST_ACS);
    m_GameLogic.AnalysebCardData(cbTempCardData,m_cbHandCardCount[m_dwCurrentUser],AnalyseResultl);

    OtherPlayerAnalyse PlayerAnalyse;
    OtherPlayerCardAnalyse(PlayerAnalyse);
    //搜索提示
    tagSearchCardResult	SearchCardResult;					//搜索结果
    memset(&SearchCardResult, 0, sizeof(SearchCardResult));
    try
    {
        m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbLastUserOutCardData, m_cbLastUserOutCount,
                                  SearchCardResult);
    }
    catch (...)
    {
        SearchCardResult.cbSearchCount = 0;
    }

    if (SearchCardResult.cbSearchCount > 0)
    {
        CT_BYTE cbCardType = m_GameLogic.GetCardType(m_cbLastUserOutCardData, m_cbLastUserOutCount);
        CT_BYTE cbBestCardIndex = 0;
        CT_BYTE cbBombCount = 0; //拆炸弹牌的数量
        CT_BYTE cbBombDismantCount = 0; //拆炸弹牌的数量
        for (int k = 0; k < SearchCardResult.cbSearchCount; ++k)
        {
            CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[k], SearchCardResult.cbCardCount[k]);
            if (cbSearchCardType == CT_BOMB_CARD) ++cbBombCount;
        }

        for (CT_BYTE i = 0 ; i != SearchCardResult.cbSearchCount; ++i)
        {
            tagAnalyseResult AnalyseResultl2[MAX_COUNT];
            memset(AnalyseResultl2, 0, sizeof(AnalyseResultl2));
            CT_BOOL bIsRemoveLine[MAX_COUNT] = {false};
            CT_BYTE cbRemoveType = m_GameLogic.GetRemainCardType(m_cbHandCardData[m_dwCurrentUser],m_cbHandCardCount[m_dwCurrentUser],SearchCardResult.cbResultCard[i],SearchCardResult.cbCardCount[i], &AnalyseResultl2[i], &bIsRemoveLine[i]);
            CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[i], SearchCardResult.cbCardCount[i]);
            if (cbCardType != CT_ERROR)
            {

                //相同牌型，直接出牌
                if (cbSearchCardType == cbCardType || ((cbSearchCardType == CT_BOMB_CARD) && (SearchCardResult.cbSearchCount != 1)&&((SearchCardResult.cbSearchCount - cbBombCount) == cbBombDismantCount)))
                {
                    CT_BYTE cbLastBestCardIndex = cbBestCardIndex;
                    cbBestCardIndex = i;
                    CT_BOOL bFindBestIndex = true;

                    //当玩家牌小于6张时如果打的时单牌或者对子，就的随机出大牌防止下家小牌跑完了
                    int iRand = rand()%100;
                    if (((PlayerAnalyse.cbMinPlayerCardCount <= 7) && cbCardType <= CT_DOUBLE_Ex && (SearchCardResult.cbSearchCount >= 2) &&(i< SearchCardResult.cbSearchCount-1) && iRand < 25) ||
                     (m_cbHandCardCount[dwNextUser] <= 7 && cbCardType <= CT_DOUBLE_Ex && (SearchCardResult.cbSearchCount >= 2) &&(i< SearchCardResult.cbSearchCount-1) && iRand < 40))
                    {
                        if (cbCardType == CT_SINGLE && (m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][0]) >= 13 || m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][0]) < 10))
                        {
                            cbBestCardIndex = cbLastBestCardIndex;
                            if (m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][0]) < 10)
                            {
                                continue;
                            }
                        }
                        else if(cbCardType == CT_DOUBLE_Ex && (m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][0]) >= 12 || m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][0]) < 7))
                        {
                            cbBestCardIndex = cbLastBestCardIndex;
                            if (m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][0]) < 7)
                            {
                                continue;
                            }
                        }
                        else
                        {
                            continue;
                        }

                    }

                    //尽量不要拆炸弹牌型
                    if (cbSearchCardType != CT_BOMB_CARD && cbCardType != CT_BOMB_CARD)
                    {
                        CT_BOOL bBreak = false;
                        //找出炸弹
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
                    //链子出牌逻辑
                    if ((bFindBestIndex) && CT_SINGLE_LINE == cbSearchCardType && SearchCardResult.cbSearchCount > 1 && (0 == cbRemoveType) && m_cbHandCardCount[m_dwCurrentUser] > 7)
                    {
                        if (i < 1)
                        {
                            continue;
                        }
                        if (AnalyseResultl2[i].cbBlockCount[0] > AnalyseResultl2[cbLastBestCardIndex].cbBlockCount[0])
                        {
                            if (bIsRemoveLine[i] && (!bIsRemoveLine[cbLastBestCardIndex]))
                            {

                            }
                            else
                            {
                                cbBestCardIndex = cbLastBestCardIndex;
                            }
                        }
                        else if (AnalyseResultl2[i].cbBlockCount[0] < AnalyseResultl2[cbLastBestCardIndex].cbBlockCount[0])
                        {
                            if (bIsRemoveLine[cbLastBestCardIndex] && (!bIsRemoveLine[i]))
                            {
                                cbBestCardIndex = cbLastBestCardIndex;
                            }
                        }
                        else
                        {
                            cbBestCardIndex = cbLastBestCardIndex;
                        }
                        if (i < SearchCardResult.cbSearchCount - 1)
                        {
                            continue;
                        }

                    }

                    //已经找到最好的牌
                    if (bFindBestIndex)
                    {
                        break;
                    }

                }
            }
            else
            {
                //找出最长牌型
                if (SearchCardResult.cbCardCount[i] > SearchCardResult.cbCardCount[cbBestCardIndex])
                {
                    //自己不主动出弹炸
                    if (m_cbLastUserOutCount == 0 && cbSearchCardType == CT_BOMB_CARD &&
                        (((6 == m_cbHandCardCount[m_dwCurrentUser])
                          && (CT_DOUBLE_Ex > cbRemoveType))
                         ||(m_cbHandCardCount[m_dwCurrentUser] > 5)))
                    {
                        continue;
                    }
                    //如果剩余6张牌且其他玩家不是单牌
                    if (((CT_FOUR_TAKE_ONE == cbSearchCardType) || (CT_FOUR_TAKE_TWO == cbSearchCardType)) && m_cbHandCardCount[m_dwCurrentUser] <= 6)
                    {
                        continue;
                    }

                    //如果4带3中有对子，就不能4带3
                    if ((CT_FOUR_TAKE_THREE == cbSearchCardType) && (7 == m_cbHandCardCount[m_dwCurrentUser]))
                    {
                        //搜索提示
                        tagSearchCardResult	SearchCardResult2;					//搜索结果
                        memset(&SearchCardResult2, 0, sizeof(SearchCardResult2));
                        CT_BOOL bIsHasDoubleCard = false;
                        try
                        {
                            CT_BYTE cbTempCard[MAX_COUNT] = {0};
                            memcpy(cbTempCard, SearchCardResult2.cbResultCard[i+4], 3* sizeof(CT_BYTE));
                            m_GameLogic.SearchOutCard(cbTempCard, 3, m_cbLastUserOutCardData, 0, SearchCardResult2);

                            for (int i = 0; i < SearchCardResult2.cbSearchCount; ++i)
                            {
                                CT_BYTE cbSearchCardType = m_GameLogic.GetCardType(SearchCardResult2.cbResultCard[i], SearchCardResult2.cbCardCount[i]);
                                if ((CT_DOUBLE_Ex == cbSearchCardType))
                                {
                                    bIsHasDoubleCard=true;
                                    break;
                                }
                            }

                        }
                        catch (...)
                        {
                            SearchCardResult2.cbSearchCount = 0;
                        }

                        if (bIsHasDoubleCard)
                        {
                            continue;
                        }
                    }

                    //轻易不要打AAA,AA,AAAXX
                    if (((CT_THREE_TAKE_TWO==cbSearchCardType && m_cbHandCardCount[m_dwCurrentUser] > 6 && (CT_DOUBLE_Ex > cbRemoveType))
                    || (CT_THREE_TAKE_ONE == cbSearchCardType && m_cbHandCardCount[m_dwCurrentUser] > 5 && (CT_DOUBLE_Ex > cbRemoveType))
                    || (CT_THREE_EX == cbSearchCardType && m_cbHandCardCount[m_dwCurrentUser] > 4 && (CT_DOUBLE_Ex > cbRemoveType))
                    || (CT_DOUBLE_Ex == cbSearchCardType && m_cbHandCardCount[m_dwCurrentUser] > 3 && (CT_DOUBLE_Ex > cbRemoveType))
                    || (CT_SINGLE == cbSearchCardType && m_cbHandCardCount[m_dwCurrentUser] > 2 && (CT_SINGLE > cbRemoveType)&&(!PlayerAnalyse.bIsOtherPlayerSingle)))
                    && (m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][0]) >= 13))
                    {
                        continue;
                    }

                    //判断链子是否能出
                    if ((CT_SINGLE_LINE == cbSearchCardType)  && (0 == cbRemoveType)
                    && (m_cbHandCardCount[m_dwCurrentUser] - SearchCardResult.cbCardCount[i] > 0) && (!PlayerAnalyse.bIsOtherPlayerDouble) && (!PlayerAnalyse.bIsOtherPlayerSingle))
                    {
                        //不能上手就打10JQKA
                        //找最大单张
                        CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(SearchCardResult.cbResultCard[i], SearchCardResult.cbCardCount[i], m_cbLastUserOutCardData[0]);
                        if (cbMaxSingle > 0 && m_GameLogic.GetLogicValue(cbMaxSingle) >= 12 && m_cbHandCardCount[m_dwCurrentUser] > 12)
                        {
                            continue;
                        }
                        else if (((AnalyseResultl2[i].cbBlockCount[0] >= 5) || (AnalyseResultl.cbBlockCount[0] < AnalyseResultl2[i].cbBlockCount[0]))&& (!bIsRemoveLine[i]))
                        {
                            continue;
                        }
                        else if ((m_GameLogic.GetLogicValue(cbMaxSingle) == 14) && AnalyseResultl2[i].cbBlockCount[0] > 2)
                        {
                            continue;
                        }
                    }

                    //轻易不能打AAKK或AAKKQQ或AAKKQQJJ
                    if (((CT_DOUBLE_LINE==cbSearchCardType) && (SearchCardResult.cbCardCount[i] == 4)&& (0 == cbRemoveType) && (m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][3]) >= 12) && (m_cbHandCardCount[m_dwCurrentUser] - SearchCardResult.cbCardCount[i] > 0))
                    || ((CT_DOUBLE_LINE==cbSearchCardType) && (SearchCardResult.cbCardCount[i] == 6)&& (0 == cbRemoveType) &&(m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][5]) >= 12) && (m_cbHandCardCount[m_dwCurrentUser] - SearchCardResult.cbCardCount[i] > 0))
                    || ((CT_DOUBLE_LINE==cbSearchCardType) && (SearchCardResult.cbCardCount[i] == 10)&& (0 == cbRemoveType) &&(m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][9]) >= 12) && (m_cbHandCardCount[m_dwCurrentUser] - SearchCardResult.cbCardCount[i] > 0))
                    || ((CT_DOUBLE_LINE==cbSearchCardType) && (SearchCardResult.cbCardCount[i] == 8)&& (0 == cbRemoveType) &&(m_GameLogic.GetLogicValue(SearchCardResult.cbResultCard[i][7]) >= 12) && (m_cbHandCardCount[m_dwCurrentUser] - SearchCardResult.cbCardCount[i] > 0)))
                    {
                        continue;
                    }

                    CT_BYTE cbLastBestCardIndex = cbBestCardIndex;
                    cbBestCardIndex = i;

                    //如果一把能出玩，或者剩余一张，且牌大的画就不需要判断是否拆炸弹, 其他玩家没有炸弹
                    if ((CT_AIRPLANE <= cbSearchCardType) && (PlayerAnalyse.bIsChunTian) && (!PlayerAnalyse.bIsOtherHasBomb) && (0 != cbRemoveType) && (m_cbHandCardCount[m_dwCurrentUser]-SearchCardResult.cbCardCount[i] > 0))
                    {
                        continue;
                    }
                    //尽量不要拆炸弹牌型
                    if (cbSearchCardType != CT_BOMB_CARD && cbCardType != CT_BOMB_CARD)
                    {
                        CT_BOOL bBreak = false;
                        //找出炸弹
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

        //是否两把结束
        CT_BOOL bIsTwoTurnEnd = false;
        CT_BYTE cbRemainCardType = m_GameLogic.GetRemainCardType(m_cbHandCardData[m_dwCurrentUser],m_cbHandCardCount[m_dwCurrentUser],SearchCardResult.cbResultCard[cbBestCardIndex],SearchCardResult.cbCardCount[cbBestCardIndex]);
        CT_BYTE cbBestCardType = m_GameLogic.GetCardType(SearchCardResult.cbResultCard[cbBestCardIndex], SearchCardResult.cbCardCount[cbBestCardIndex]);
        if ((0 == m_cbLastUserOutCount)&&(!PlayerAnalyse.bIsOtherHasBomb)&&(m_cbHandCardCount[m_dwCurrentUser] - SearchCardResult.cbCardCount[cbBestCardIndex] == 2)&&(CT_DOUBLE_Ex != cbBestCardType)&&
        (CT_DOUBLE_Ex == cbRemainCardType))
        {
            CT_BYTE cbHandCardData[MAX_COUNT]={0};
            memcpy(cbHandCardData,m_cbHandCardData[m_dwCurrentUser],m_cbHandCardCount[m_dwCurrentUser]);
            CT_BYTE cbHandCardCount = m_cbHandCardCount[m_dwCurrentUser];
            //删除扑克
            if (m_GameLogic.RemoveCardList(SearchCardResult.cbResultCard[cbBestCardIndex], SearchCardResult.cbCardCount[cbBestCardIndex], cbHandCardData, cbHandCardCount))
            {
                cbHandCardCount -= SearchCardResult.cbCardCount[cbBestCardIndex];
                CT_BYTE cbResultCard[MAX_COUNT] = { 0 };
                CT_BOOL bIsMaxDouble = false;
                for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
                {
                    if (i == m_dwCurrentUser) continue;
                    //找最大对子
                    CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxDouble(m_cbHandCardData[i], m_cbHandCardCount[i], cbHandCardData[0], cbResultCard);
                    if ((cbMaxSingle>0) && (m_GameLogic.GetLogicValue(cbHandCardData[0]) != m_GameLogic.GetLogicValue(cbResultCard[0])))
                    {

                        bIsMaxDouble = true;
                    }
                }
                if (!bIsMaxDouble)
                {
                    bIsTwoTurnEnd = true;
                    memcpy(cbCardData, cbHandCardData, sizeof(CT_BYTE)*cbHandCardCount);
                    cbCardCount = cbHandCardCount;
                }
            }
        }
        else if ((0 == m_cbLastUserOutCount)&&(!PlayerAnalyse.bIsOtherHasBomb)&&(m_cbHandCardCount[m_dwCurrentUser] - SearchCardResult.cbCardCount[cbBestCardIndex] == 1) && (CT_SINGLE != cbBestCardType) &&
            (CT_SINGLE == cbRemainCardType))
        {
            CT_BYTE cbHandCardData[MAX_COUNT]={0};
            memcpy(cbHandCardData,m_cbHandCardData[m_dwCurrentUser],m_cbHandCardCount[m_dwCurrentUser]);
            CT_BYTE cbHandCardCount = m_cbHandCardCount[m_dwCurrentUser];
            //分析扑克
            tagAnalyseResult AnalyseResult;
            memset(&AnalyseResult, 0, sizeof(AnalyseResult));
            m_GameLogic.AnalysebCardData(cbHandCardData, cbHandCardCount, AnalyseResult);
            //找最大单张
            CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(AnalyseResult.cbCardData[0], AnalyseResult.cbBlockCount[0], m_cbLastUserOutCardData[0]);
            if (cbMaxSingle>0)
            {
                CT_BOOL bIsMaxSingle = false;
                for (CT_DWORD i = 0; i < GAME_PLAYER; ++i)
                {
                    if (i == m_dwCurrentUser) continue;
                    //找最大对子
                    CT_BYTE cbMaxSingle2 = m_GameLogic.SearchMaxSingle(m_cbHandCardData[i], m_cbHandCardCount[i], cbMaxSingle);
                    if (cbMaxSingle2>0)
                    {

                        bIsMaxSingle = true;
                    }
                }
                if (!bIsMaxSingle)
                {
                    cbCardData[0] = cbMaxSingle;
                    cbCardCount = 1;
                    bIsTwoTurnEnd = true;
                }
            }

        }
        if (!bIsTwoTurnEnd)
        {
            cbCardCount = SearchCardResult.cbCardCount[cbBestCardIndex];
            memcpy(cbCardData, &SearchCardResult.cbResultCard[cbBestCardIndex], sizeof(CT_BYTE)*cbCardCount);
        }

        CT_BYTE cbOutCardType = m_GameLogic.GetCardType(cbCardData,cbCardCount);
        if (m_cbHandCardCount[dwNextUser] == 1 && (1 == cbCardCount))
        {
            //找最大单张
            CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbLastUserOutCardData[0]);
            if (cbMaxSingle>0)
            {
                cbCardData[0] = cbMaxSingle;
                cbCardCount = 1;
            }
        }
        else if(2==m_cbHandCardCount[m_dwCurrentUser] && (1 == cbCardCount))
        {
            //找最大单张
            CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbLastUserOutCardData[0]);
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
        else if((PlayerAnalyse.bIsOtherPlayerSingle) && (0 == m_cbLastUserOutCount) && (1 == cbCardCount))
        {
            tagSearchCardResult SearchCardResult3;
            memset(&SearchCardResult3,0,sizeof(SearchCardResult3));
            //获取数值
            CT_BYTE cbValue = m_GameLogic.GetCardValue(PlayerAnalyse.cbOtherMaxSingle) - 1;
            if (2 == cbValue)
            {
                cbValue = 3;
            }
            else if (0 == cbValue)
            {
                cbValue = 13;
            }
            //获取数值
            CT_BYTE cbColor = m_GameLogic.GetCardColor(PlayerAnalyse.cbOtherMaxSingle);
            CT_BYTE cbNewCard = (cbColor | cbValue);
            CT_BYTE cbNextPlayerMaxCard = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser],PlayerAnalyse.cbOtherMaxSingle);
            //找最大单张
            CT_BYTE cbResultCount = m_GameLogic.SearchOutCard(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], &cbNewCard, 1, SearchCardResult3);
            if (cbResultCount>0 && m_GameLogic.GetLogicValue(cbNextPlayerMaxCard) < m_GameLogic.GetLogicValue(PlayerAnalyse.cbOtherMaxSingle))
            {
                for (int i = 0; i < SearchCardResult3.cbSearchCount; ++i) {
                    if (SearchCardResult3.cbCardCount[i] > 0 && SearchCardResult3.cbResultCard[i][0] > 0)
                    {
                        cbCardData[0] = SearchCardResult3.cbResultCard[i][0];
                        cbCardCount = 1;
                        break;
                    }
                }
            }

        }
        else if ((CT_DOUBLE_Ex == cbOutCardType) && (PlayerAnalyse.bIsOtherPlayerDouble) && (m_cbHandCardCount[m_dwCurrentUser] > 2)
        && (!m_GameLogic.CompareCard(PlayerAnalyse.cbOehterDoubleCard,2,cbCardData,cbCardCount)))
        {
            CT_BYTE cbResultCard[MAX_COUNT] = { 0 };
            //找最大对子
            CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxDouble(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser], m_cbLastUserOutCardData[0], cbResultCard);
            if ((cbMaxSingle>0 && (m_GameLogic.GetLogicValue(cbMaxSingle) < 13)) && (m_GameLogic.GetLogicValue(cbCardData[0]) != m_GameLogic.GetLogicValue(cbResultCard[0])))
            {
                memcpy(cbCardData,cbResultCard,2* sizeof(CT_BYTE));
                cbCardCount = 2;
            }
            //只有新的一轮才能随便改出牌的牌型
            else if ((0 == m_cbLastUserOutCount) && AnalyseResultl.cbBlockCount[0] > 0)
            {
                CT_BYTE cbMaxSingle = m_GameLogic.SearchMaxSingle(m_cbHandCardData[m_dwCurrentUser], m_cbHandCardCount[m_dwCurrentUser],0);
                if (m_GameLogic.GetLogicValue(AnalyseResultl.cbCardData[0][0]) < m_GameLogic.GetLogicValue(cbMaxSingle))
                {
                    cbCardData[0] = AnalyseResultl.cbCardData[0][0];
                    cbCardCount = 1;
                }
                else if(AnalyseResultl.cbBlockCount[1] > 0)
                {
                    cbCardData[0] = AnalyseResultl.cbCardData[1][0];
                    cbCardCount = 1;
                }

            }
            else if ((0 == m_cbLastUserOutCount) && AnalyseResultl.cbBlockCount[1] > 0)
            {
                cbCardData[0] = AnalyseResultl.cbCardData[1][0];
                cbCardCount = 1;
            }

        }
        else if ((0 == m_cbLastUserOutCount) && (CT_FOUR_TAKE_ONE == cbOutCardType || CT_FOUR_TAKE_TWO == cbOutCardType)
        && (m_cbHandCardCount[m_dwCurrentUser] <= 6) && (!PlayerAnalyse.bIsOtherHasBomb) && (!PlayerAnalyse.bIsChunTian))
        {
            if (AnalyseResultl.cbBlockCount[0] > 0)
            {
                cbCardData[0] = AnalyseResultl.cbCardData[0][0];
                cbCardCount = 1;
            }
            else if ((AnalyseResultl.cbBlockCount[1] > 0) && (m_GameLogic.GetLogicValue(AnalyseResultl.cbCardData[1][0]) != m_GameLogic.GetLogicValue(AnalyseResultl.cbCardData[3][0])))
            {
                cbCardData[0] = AnalyseResultl.cbCardData[1][0];
                cbCardCount = 1;
            }
        }


        //排列扑克
        m_GameLogic.SortCardList(cbCardData, cbCardCount, ST_DESC);
        memcpy(m_cbAiOutCardData, &cbCardData, sizeof(CT_BYTE)*cbCardCount);
        m_cbAiOutCardCount = cbCardCount;
        m_cbAiOutCardType = m_GameLogic.GetCardType(cbCardData, cbCardCount, false);
    }
    else
    {
        if (m_cbLastUserOutCount == 0)
        {
            LOG(WARNING) << "user auto pass, but last user is him?  fuck!!!";
        }
        //不出，过牌
        m_cbAiOutCardType = CT_ERROR;
    }
	return false;
}