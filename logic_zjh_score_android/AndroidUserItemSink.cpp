
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"

//////////////////////////////////////////////////////////////////////////

//辅助时间
#define TIME_LESS						2									//最少时间
#define TIME_DELAY_TIME					3									//延时时间

//游戏时间
#define TIME_USER_ADD_SCORE				5									//下注时间
#define TIME_RUSH						3									//火拼
#define TIME_OPEN_CARD					3									//开牌时间

//游戏时间
#define IDI_USER_ADD_SCORE				(101)								//下注定时器
#define IDI_RUSH						(102)								//火拼定时器
#define IDI_OPEN_CARD					(103)								//赢家开牌消息

//////////////////////////////////////////////////////////////////////////

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
{
	//游戏变量
	m_lStockScore = 0;
	m_wBankerUser = INVALID_CHAIR;
	m_wCurrentUser = INVALID_CHAIR;
	m_cbCardType = 0;

	//加注信息
	m_bMingZhu = false;
	m_lCellScore = 0;
	m_lMaxCellScore = 0;
	m_lUserMaxScore = 0;
	m_lCurrentTimes = 0;
	m_cbAddScoreCount = 1;
	memset(m_lTableScore, 0, sizeof(m_lTableScore));

	//游戏状态
	memset(m_cbPlayStatus, 0, sizeof(m_cbPlayStatus));
	memset(m_cbRealPlayer, 0, sizeof(m_cbRealPlayer));
	memset(m_cbAndroidStatus, 0, sizeof(m_cbAndroidStatus));
	memset(m_cbLookCardStatus, 0, sizeof(m_cbLookCardStatus));
	m_cbLookCardAndDouble = 0;
	m_cbChiJiStatus = 0;
	m_cbChiJiAddScoreCount = 0;

	//用户扑克
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(m_cbAllHandCardData, 0, sizeof(m_cbAllHandCardData));
	memset(m_cbHandCardType, 0, sizeof(m_cbHandCardType));
}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{
	//游戏变量
	m_lStockScore = 0;
	m_wBankerUser = INVALID_CHAIR;
	m_wCurrentUser = INVALID_CHAIR;
	m_wChairID = INVALID_CHAIR;
	m_cbCardType = 0;

	//加注信息
	m_bMingZhu = false;
	m_lCellScore = 0;
	m_lMaxCellScore = 0;
	m_lUserMaxScore = 0;
	m_lCurrentTimes = 0;
	m_cbAddScoreCount = 1;
	memset(m_lTableScore, 0, sizeof(m_lTableScore));

	//游戏状态
	memset(m_cbPlayStatus, 0, sizeof(m_cbPlayStatus));
	memset(m_cbRealPlayer, 0, sizeof(m_cbRealPlayer));
	memset(m_cbAndroidStatus, 0, sizeof(m_cbAndroidStatus));
	memset(m_cbLookCardStatus, 0, sizeof(m_cbLookCardStatus));
	m_cbLookCardAndDouble = 0;
	m_cbChiJiStatus = 0;
	m_cbChiJiAddScoreCount = 0;

	//用户扑克
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(m_cbAllHandCardData, 0, sizeof(m_cbAllHandCardData));
	memset(m_cbHandCardType, 0, sizeof(m_cbHandCardType));
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
	case IDI_USER_ADD_SCORE:
	{
		//删除定时器
		m_pAndroidUserItem->KillTimer(IDI_USER_ADD_SCORE);
		return HandleAndroidAddScore();
	}
	case IDI_RUSH:
	{
		//删除定时器
		m_pAndroidUserItem->KillTimer(IDI_RUSH);
		return HandleAndroidRush();
	}
	case IDI_OPEN_CARD:
	{
		//删除定时器
		m_pAndroidUserItem->KillTimer(IDI_OPEN_CARD);
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_OPEN_CARD, NULL, 0);
		return true;
	}
	default:
		break;
	}
	return true;
}

bool CAndroidUserItemSink::OnGameMessage( CT_DWORD wSubCmdID, const CT_VOID * pData, CT_DWORD wDataSize)
{
	switch (wSubCmdID)
	{
	case SUB_S_GAME_START:		//游戏开始
	{
		return OnSubGameStart(pData, wDataSize);
	}
	case SUB_S_ADD_SCORE:		//用户下注
	{
		return OnSubAddScore(pData, wDataSize);
	}
	case SUB_S_GIVE_UP:			//用户放弃
	case SUB_S_GIVE_UP_TIME_OUT:
	{
		return OnSubGiveUp(pData, wDataSize);
	}
	case SUB_S_GAME_END:		//游戏结束
	{
		return OnSubGameEnd(pData, wDataSize);
	}
	case SUB_S_COMPARE_CARD:	//用户比牌
	{
		return OnSubCompareCard(pData, wDataSize);
	}
	case SUB_S_LOOK_CARD:		//用户看牌
	{
		return OnSubLookCard(pData, wDataSize);
	}
	case SUB_S_ALL_IN_RESULT:	//孤注一掷结果
	{
		return OnSubAllInResult(pData, wDataSize);
	}
	case SUB_S_RUSH:			//火拼
	{
		return OnSubRush(pData, wDataSize);
	}
	case SUB_S_RUSH_RESULT:		//火拼结果
	{
		return OnSubRushResult(pData, wDataSize);
	}
	case SUB_S_ANDROID_CARD:	//机器人牌消息
	{
		return OnSubAndroidCard(pData, wDataSize);
	}
	default:
		break;
	}
	return true;
}

// 机器人消息
bool CAndroidUserItemSink::OnSubAndroidCard(const void * pBuffer, CT_WORD wDataSize)
{
	//效验参数
	if (wDataSize != sizeof(CMD_S_AndroidCard)) 
		return false;

	//变量定义
	CMD_S_AndroidCard * pAndroidCard = (CMD_S_AndroidCard *)pBuffer;

	//设置变量
	m_lStockScore = pAndroidCard->lStockScore;
	memcpy(m_cbRealPlayer, pAndroidCard->cbRealPlayer, sizeof(m_cbRealPlayer));
	memcpy(m_cbAllHandCardData, pAndroidCard->cbAllHandCardData, sizeof(m_cbAllHandCardData));
	memcpy(m_cbAndroidStatus, pAndroidCard->cbAndroidStatus, sizeof(m_cbAndroidStatus));

	//计算所有的玩家的牌型
	for (CT_WORD i = 0; i < GAME_PLAYER; ++i)
	{
		if (m_cbPlayStatus[i] == 0)
			continue;

		//服务器下发的牌型已经排序，不需要再进行排序
		m_cbHandCardType[i] = m_GameLogic.GetCardType(m_cbAllHandCardData[i], MAX_COUNT);
	}

	return true;
}

//游戏开始
bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, CT_WORD wDataSize)
{
	//效验参数
	if (wDataSize != sizeof(CMD_S_GameStart)) return false;

	//变量定义
	CMD_S_GameStart * pGameStart = (CMD_S_GameStart *)pBuffer;

	//用户信息
	m_wBankerUser = pGameStart->wBankerUser;
	m_wCurrentUser = pGameStart->wCurrentUser;

	//加注信息
	m_bMingZhu = false;
	m_lUserMaxScore = 0;
	m_lCellScore = (CT_LONGLONG)pGameStart->dCellScore*TO_LL;
	m_lCurrentTimes = (CT_LONGLONG)pGameStart->dCurrentJetton*TO_LL;
	memcpy(m_cbPlayStatus, pGameStart->cbPlayStatus, sizeof(CT_BYTE)*GAME_PLAYER);
	m_lMaxCellScore = m_lCellScore * MAX_JETTON_MULTIPLE/2;			//没看牌前最大倍数

	//用户状态
	for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	{
		//获取用户
		if (m_cbPlayStatus[i] != false)
		{
			m_lTableScore[i] = m_lCellScore;
		}
	}

	//玩家处理
	if (m_pAndroidUserItem->GetChairID() == pGameStart->wCurrentUser)
	{
		//设置定时器
		CT_DWORD nElapse = rand() % TIME_USER_ADD_SCORE + TIME_LESS + 2; //开始游戏时下注给多一点时间
		m_pAndroidUserItem->SetTimer(IDI_USER_ADD_SCORE, nElapse*1000);
	}

	return true;
}

//用户放弃
bool CAndroidUserItemSink::OnSubGiveUp(const void * pBuffer, CT_WORD wDataSize)
{
	//效验数据
	if (wDataSize != sizeof(CMD_S_GiveUp)) return false;

	//处理消息
	CMD_S_GiveUp * pGiveUp = (CMD_S_GiveUp *)pBuffer;

	//设置变量
	m_cbRealPlayer[pGiveUp->wGiveUpUser] = false;
	m_cbPlayStatus[pGiveUp->wGiveUpUser] = false;
	if (pGiveUp->cbSwitchUser == 0)
		return true;

	m_cbAddScoreCount = (CT_BYTE)pGiveUp->wJettonCount;

	//判断是否到我
	if (pGiveUp->wCurrentUser != INVALID_CHAIR)
	{
		m_wCurrentUser = pGiveUp->wCurrentUser;

		//用户动作
		if (m_wChairID == m_wCurrentUser)
		{
			//设置定时器
			CT_DWORD nElapse = rand() % TIME_USER_ADD_SCORE + TIME_LESS;
			m_pAndroidUserItem->SetTimer(IDI_USER_ADD_SCORE, nElapse * 1000);
		}
	}

	return true;
}

//用户下注
bool CAndroidUserItemSink::OnSubAddScore(const void * pBuffer, CT_WORD wDataSize)
{
	//效验数据
	if (wDataSize != sizeof(CMD_S_AddScore)) return false;

	//处理消息
	CMD_S_AddScore * pAddScore = (CMD_S_AddScore *)pBuffer;

	//判断是否真人看牌并加注了
	CT_LONGLONG llCurrentTimes = (CT_LONGLONG)pAddScore->dCurrentJetton*TO_LL;
	if (!m_cbLookCardAndDouble && m_cbLookCardStatus[pAddScore->wAddJettonUser] && llCurrentTimes > m_lCurrentTimes)
	{
		if (m_cbRealPlayer[pAddScore->wAddJettonUser])
		{
			m_cbLookCardAndDouble = 1;
		}
	}

	//设置变量
	m_wCurrentUser = pAddScore->wCurrentUser;
	m_lCurrentTimes = llCurrentTimes;
	m_cbAddScoreCount = (CT_BYTE)pAddScore->wJettonCount;
	m_lTableScore[pAddScore->wAddJettonUser] += (CT_LONGLONG)pAddScore->dAddJettonCount*TO_LL;

	//用户动作
	if (m_wChairID == m_wCurrentUser)
	{
		//设置定时器
		CT_DWORD nElapse = rand() % TIME_USER_ADD_SCORE + TIME_LESS;
		m_pAndroidUserItem->SetTimer(IDI_USER_ADD_SCORE, nElapse*1000);
	}

	return true;
}

//用户看牌
bool CAndroidUserItemSink::OnSubLookCard(const void * pBuffer, CT_WORD wDataSize)
{
	//效验数据
	if (wDataSize != sizeof(CMD_S_LookCard)) return false;

	//处理消息
	CMD_S_LookCard * pLookCard = (CMD_S_LookCard *)pBuffer;

	//用户看牌
	if (pLookCard->wLookCardUser == m_wChairID)
	{
		m_bMingZhu = true;
	}

	m_cbLookCardStatus[pLookCard->wLookCardUser] = 1;

	//单注
	m_lCurrentTimes = (CT_LONGLONG)pLookCard->dCurrentJetton*TO_LL;
	//下注次数
	if (pLookCard->wCurrentUser == m_wChairID)
	{
		m_cbAddScoreCount = (CT_BYTE)pLookCard->wJettonCount;
	}

	//看牌用户
	if (m_wCurrentUser == m_wChairID)
	{
		//用户扑克
		memcpy(m_cbHandCardData, pLookCard->cbCardData, sizeof(m_cbHandCardData));

		//获取类型
		m_GameLogic.SortCardList(m_cbHandCardData, MAX_COUNT);
		m_cbCardType = m_GameLogic.GetCardType(m_cbHandCardData, MAX_COUNT);

		//如为散牌
		if (m_cbCardType < CT_DOUBLE_TYPE)
		{
			for (CT_BYTE i = 0; i < MAX_COUNT; i++)
			{
				m_cbCardType = 0;
				if (m_GameLogic.GetCardLogicValue(m_cbHandCardData[i]) > 11)
				{
					m_cbCardType = CT_SINGLE;
					break;
				}
			}
		}
		//看完牌之后，马上回去处理是否跟注或者加注
		CT_DWORD nElapse = rand() % TIME_USER_ADD_SCORE + TIME_LESS;
		m_pAndroidUserItem->SetTimer(IDI_USER_ADD_SCORE, nElapse);
	}

	return true;
}

//用户比牌
bool CAndroidUserItemSink::OnSubCompareCard(const void * pBuffer, CT_WORD wDataSize)
{
	//数据效验
	if (wDataSize != sizeof(CMD_S_CompareCard)) return false;

	//处理消息
	CMD_S_CompareCard * pCompareCard = (CMD_S_CompareCard *)pBuffer;

	//设置变量
	m_wCurrentUser = pCompareCard->wCurrentUser;
	m_cbPlayStatus[pCompareCard->wLostUser] = false;

	//玩家人数
	CT_BYTE cbUserCount = 0;
	for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	{
		if (m_cbPlayStatus[i] == 1) cbUserCount++;
	}

	//判断结束
	if (cbUserCount > 1)
	{
		//当前用户
		if (m_wChairID == m_wCurrentUser)
		{
			m_cbAddScoreCount = (CT_BYTE)pCompareCard->wJettonCount;
			//设置定时器
			int nElapse = rand() % TIME_USER_ADD_SCORE + TIME_LESS;
			m_pAndroidUserItem->SetTimer(IDI_USER_ADD_SCORE, nElapse + TIME_DELAY_TIME);
		}
	}


	return true;
}

//用户开牌
bool CAndroidUserItemSink::OnSubOpenCard(const void * pBuffer, CT_WORD wDataSize)
{
	//数据效验
	if (wDataSize != sizeof(CMD_S_OpenCard)) return false;

	return true;
}

//游戏结束
bool CAndroidUserItemSink::OnSubGameEnd(const void * pBuffer, CT_WORD wDataSize)
{
	//效验参数
	if (wDataSize != sizeof(CMD_S_GameEnd)) return false;
	//CMD_S_GameEnd * pGameEnd = (CMD_S_GameEnd *)pBuffer;

	//删除定时器
	m_pAndroidUserItem->KillTimer(IDI_USER_ADD_SCORE);

	//CMD_S_GameEnd* pGameEnd = (CMD_S_GameEnd*)pBuffer;
	//赢家主动开牌消息
	//if (pGameEnd->dGameScore[m_wChairID] > 0)
	//{
	//	m_pAndroidUserItem->SetTimer(IDI_OPEN_CARD, TIME_OPEN_CARD * 1000, 0);
	//}

	////开始时间
	//if(pGameEnd->bDelayOverGame==true)
	//{
	//	UINT nElapse = rand()%(3)+TIME_LESS;
	//	m_pIAndroidUserItem->SetGameTimer(IDI_DELAY_TIME, nElapse);
	//}
	//else
	//{
	//	CT_DWORD nElapse = rand() % (10) + TIME_LESS + 3;
	//	m_pAndroidUserItem->SetTimer(IDI_START_GAME, nElapse);
	//}

	//清理变量
	m_lMaxCellScore = 0;
	m_cbAddScoreCount = 1;
	m_lCurrentTimes = 0;
	memset(m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	memset(m_cbAllHandCardData, 0, sizeof(m_cbAllHandCardData));
	memset(m_cbLookCardStatus, 0, sizeof(m_cbLookCardStatus));
	memset(m_cbHandCardType, 0, sizeof(m_cbHandCardType));
	m_cbLookCardAndDouble = 0;
	m_cbChiJiStatus = 0;
	m_cbChiJiAddScoreCount = 0;

	return true;
}

bool CAndroidUserItemSink::OnSubAllInResult(const void * pBuffer, CT_WORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_AllIn_Result))
		return false;

	CMD_S_AllIn_Result* pAllInResult = (CMD_S_AllIn_Result*)pBuffer;

	if (pAllInResult->wCurrentUser != INVALID_CHAIR)
	{
		m_wCurrentUser = pAllInResult->wCurrentUser;
		//设置变量
		m_cbRealPlayer[pAllInResult->wAllInUser] = false;
		m_cbPlayStatus[pAllInResult->wAllInUser] = false;
		m_cbAndroidStatus[pAllInResult->wAllInUser] = false;

		if (pAllInResult->wCurrentUser == m_wChairID)
		{
			m_cbAddScoreCount =  (CT_BYTE)pAllInResult->wJettonCount;

			int nElapse = rand() % TIME_USER_ADD_SCORE + TIME_LESS;
			m_pAndroidUserItem->SetTimer(IDI_USER_ADD_SCORE, nElapse + TIME_DELAY_TIME);
		}
	}

	return true;
}

//火拼结果
bool CAndroidUserItemSink::OnSubRush(const void * pBuffer, CT_WORD wDataSize)
{
	if (wDataSize != sizeof(CMD_S_Rush))
	{
		return false;
	}

	CMD_S_Rush* pRush = (CMD_S_Rush*)pBuffer;
	if (pRush->wCurrentUser == m_wChairID)
	{
		CT_DWORD nElapse = rand() % TIME_RUSH + TIME_LESS;
		m_pAndroidUserItem->SetTimer(IDI_RUSH, nElapse *1000, 0);
	}


	return true;
}

//火拼
bool CAndroidUserItemSink::OnSubRushResult(const void * pBuffer, CT_WORD wDataSize)
{
	return true;
}

//推断胜者
CT_WORD CAndroidUserItemSink::EstimateWinner()
{
	//保存扑克
	CT_BYTE cbUserCardData[GAME_PLAYER][MAX_COUNT];
	memcpy(cbUserCardData, m_cbAllHandCardData, sizeof(cbUserCardData));

	//排列扑克
	for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	{
		m_GameLogic.SortCardList(cbUserCardData[i], MAX_COUNT);
	}

	//变量定义
	CT_WORD wWinUser = INVALID_CHAIR;

	//查找数据
	for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	{
		//用户过滤
		if (m_cbPlayStatus[i] == false) continue;

		//设置用户
		if (wWinUser == INVALID_CHAIR)
		{
			wWinUser = i;
			continue;
		}

		//对比扑克
		if (m_GameLogic.CompareCard(cbUserCardData[i], cbUserCardData[wWinUser], MAX_COUNT) >= 1)
		{
			wWinUser = i;
		}
	}

	return wWinUser;
}

bool CAndroidUserItemSink::HandleAndroidAddScore()
{
	if (m_wChairID != m_wCurrentUser)
		return false;

	//定义变量
	bool bGiveUp = false;
	CT_BYTE cbProbability = 0;
	CT_LONGLONG lCurrentScore = 0;
	CT_LONGLONG lAndroidMaxScore = m_pTableFrame->GetUserScore(m_wChairID);

	//最大下注
	m_lUserMaxScore = lAndroidMaxScore;

	//玩家人数
	CT_BYTE cbPlayerCount = 0;
	for (CT_WORD i = 0; i < GAME_PLAYER; i++)
	{
		if (m_cbPlayStatus[i] == 1)
			cbPlayerCount++;
	}

	//是否真人
	CT_WORD wEstimateWinner = EstimateWinner();
	if (m_cbRealPlayer[wEstimateWinner] == 1)
	{
		CT_LONGLONG lTableScore = 0;
		for (CT_WORD i = 0; i < GAME_PLAYER; i++)
		{
			if (m_cbAndroidStatus[i] == 1)
			{
				lTableScore += m_lTableScore[i];
			}
		}

		//是否输到底限
		if (lTableScore + m_lMaxCellScore >= m_lStockScore)
			bGiveUp = true;
	}

	//首次操作
	if (m_lTableScore[m_wChairID] == m_lCellScore)
	{
		//随机概率
		cbProbability = rand() % (10);

		//首轮加注时，如果有玩家加注到4倍后，有几率弃牌
		if (m_lCurrentTimes >= m_lCellScore * JETTON_MULTIPLE_2)
		{
			if (wEstimateWinner != m_wChairID)
			{
				//不是赢家的话，有20%的几率首轮弃牌
				if (cbProbability < 2)
				{
					m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
					return true;
				}
			}
		}
		
		lCurrentScore = m_lCurrentTimes;
		if (lCurrentScore > m_lMaxCellScore)
		{
			lCurrentScore = m_lMaxCellScore;
		}
		//发送消息
		CMD_C_AddScore AddScore;
		AddScore.dScore = lCurrentScore*TO_DOUBLE;
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ADD_SCORE, &AddScore, sizeof(AddScore));

		return true;
	}
	else
	{
		//是否看牌
		if (m_bMingZhu == false)
		{
			//智能放弃
			if (bGiveUp == true)
			{
				m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);

				return true;
			}

			//随机概率
			cbProbability = rand() % (10);

			//看牌概率
			if (cbProbability < LOOK_CARD)
			{
				m_bMingZhu = true;
				m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_LOOK_CARD, NULL, 0);

				return true;
			}

			//判断真人看牌并且加注的情况下,大概率看牌
			if (m_cbLookCardAndDouble)
			{
				if (cbProbability < 8)
				{
					m_bMingZhu = true;
					m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_LOOK_CARD, NULL, 0);

					return true;
				}
			}

			//用户加注
			if (m_lTableScore[m_wChairID] < 2 * m_lMaxCellScore && (4 * cbPlayerCount * m_lMaxCellScore + m_lCellScore + m_lTableScore[m_wChairID]) < lAndroidMaxScore)
			{
				//加注积分
				lCurrentScore = m_lCurrentTimes;

				//发送消息
				CMD_C_AddScore AddScore;
				AddScore.dScore = m_lCurrentTimes*TO_DOUBLE;

				m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ADD_SCORE, &AddScore, sizeof(AddScore));
				return true;
			}
			//比牌概率
			else
			{
				if (lAndroidMaxScore < m_lCurrentTimes || m_cbAddScoreCount <= CAN_LOOKCARD_JETTON_COUNT)
				{
					if (m_wChairID == wEstimateWinner && lAndroidMaxScore < m_lCurrentTimes)
					{
						//如果当前玩家是赢家，发起孤注一掷
						m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ALL_IN, NULL, 0);
					}
					else
					{
						m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
					}
				}
				else
				{
					//构造消息
					CMD_C_CompareCard CompareCard;
					memset(&CompareCard, 0, sizeof(CompareCard));

					//查找玩家
					for (CT_WORD i = 0; i < GAME_PLAYER; i++)
					{
						if (m_cbPlayStatus[i] == 1 && i != m_wChairID)
						{
							CompareCard.wCompareUser = i;
							break;
						}
					}
					m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_COMPARE_CARD, &CompareCard, sizeof(CompareCard));
				}
				
				return true;
			}
		}
		else
		{
			//智能放弃
			if (bGiveUp == true)
			{
				m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);

				return true;
			}

			//随机概率
			cbProbability = rand() % (100);

			//散牌放弃
			if ((m_cbCardType == 0 || m_cbCardType == CT_SINGLE) && m_wChairID != wEstimateWinner)
			{
				//散牌有10的概率吃鸡, 并且还没有到达最大倍数
				if (cbProbability < 10 && m_cbChiJiStatus == 0/* && m_lCurrentTimes < (m_lMaxCellScore * 2)*/)
				{
					lCurrentScore = m_lCurrentTimes * JETTON_MULTIPLE_2;
					if (lCurrentScore >  m_lMaxCellScore * 2)
					{
						lCurrentScore = m_lMaxCellScore * 2;
					}

					CMD_C_AddScore AddScore;
					AddScore.dScore = lCurrentScore*TO_DOUBLE;
					m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ADD_SCORE, &AddScore, sizeof(AddScore));

					m_cbChiJiStatus = 1;
					m_cbChiJiAddScoreCount = 1;
					//LOG(WARNING) << "enter chi ji status.";

					return true;
				}
				else if (m_cbChiJiStatus == 1)
				{
					if (m_cbChiJiAddScoreCount == 1)
					{
						//30的概率再跟一次
						if (cbProbability < 30)
						{
							CMD_C_AddScore AddScore;
							AddScore.dScore = m_lCurrentTimes*TO_DOUBLE;
							m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ADD_SCORE, &AddScore, sizeof(AddScore));
							m_cbChiJiAddScoreCount = 2;
							return true;
						}
						else
						{
							m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
							return true;
						}
					}
					else
					{
						m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
						return true;
					}
				}
				else if (m_cbCardType == 0)
				{
					m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
					return true;
				}
				else
				{
					if (cbProbability < 20)
					{
						m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
						return true;
					}
				}
			}
			/*else if (m_cbCardType == CT_SINGLE && m_wChairID != wEstimateWinner)
			{
				//放弃概率
				if (cbProbability < GIVE_UP)
				{
					m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
					return true;
				}
			}*/

			//加注概率
			if (m_cbAddScoreCount <= CAN_LOOKCARD_JETTON_COUNT
				|| (((m_cbCardType < CT_JIN_HUA && m_lTableScore[m_wChairID] < (1 * 2 * m_lMaxCellScore * m_cbCardType))
				|| (m_cbCardType == CT_JIN_HUA && m_lTableScore[m_wChairID] < (3 * 2 * m_lMaxCellScore * m_cbCardType))
				|| (m_cbCardType > CT_JIN_HUA && m_lTableScore[m_wChairID] < (5 * 2 * m_lMaxCellScore * m_cbCardType)))
				&& (4 * cbPlayerCount * m_lMaxCellScore + m_lCellScore + m_lTableScore[m_wChairID]) < lAndroidMaxScore))
			{
				//下注积分
				lCurrentScore = m_lCurrentTimes;

				//如果对子则超过5轮，弃牌
				if (m_cbCardType == CT_DOUBLE_TYPE && m_wChairID != wEstimateWinner)
				{
					if (m_cbChiJiAddScoreCount > 5)
					{
						m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
						return true;
					}
				}

				//如果是大牌的话，25%概率加注
				if (m_wChairID == wEstimateWinner)
				{
					int nRandom = rand() % 100;
					if (nRandom < 25)
					{
						lCurrentScore *= 2;
						if (lCurrentScore > m_lMaxCellScore * 2)
							lCurrentScore = m_lMaxCellScore * 2;
					}
				}

				//发送消息
				CMD_C_AddScore AddScore;
				AddScore.dScore = lCurrentScore*TO_DOUBLE;
				m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ADD_SCORE, &AddScore, sizeof(AddScore));

				return true;
			}
			//用户比牌
			else
			{
				if (lAndroidMaxScore < m_lCurrentTimes || m_cbAddScoreCount <= CAN_LOOKCARD_JETTON_COUNT)
				{
					if (m_wChairID == wEstimateWinner && lAndroidMaxScore < m_lCurrentTimes)
					{
						//如果当前玩家是赢家，发起孤注一掷
						m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ALL_IN, NULL, 0);
					}
					else
					{
						m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
					}
				}
				else
				{
					//构造消息
					CMD_C_CompareCard CompareCard;
					memset(&CompareCard, 0, sizeof(CompareCard));

					//查找玩家
					for (CT_WORD i = 0; i < GAME_PLAYER; i++)
					{
						if (m_cbPlayStatus[i] == 1 && i != m_wChairID)
						{
							CompareCard.wCompareUser = i;
							break;
						}
					}
					m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_COMPARE_CARD, &CompareCard, sizeof(CompareCard));
				}

				return true;
			}
		}
	}

	return true;
}

bool CAndroidUserItemSink::HandleAndroidRush()
{
	if (m_wChairID != EstimateWinner())
	{
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_GIVE_UP, NULL, 0);
		return true;
	}
	else
	{
		//如果是大赢家进行火拼
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_RUSH, NULL, 0);
		return true;
	}

	return true;
}

bool CAndroidUserItemSink::ClearAllTimer()
{
	
	return true;
}
