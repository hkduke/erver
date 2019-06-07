
#include "stdafx.h"
#include "ITableFrame.h"
#include "glog_wrapper.h"
#include "acl_cpp/lib_acl.hpp"
#include "Utility.h"
#include "AndroidUserItemSink.h"
#include "MSG_DDZ.h"
#include "my_ddzboot.h"
#include "BOTProces.h"
#include "GameLogic.h"
#include <algorithm>
#define TIME_1 100  //管牌
#define TIME_2 200  //不管牌
#define TIME_3 1002 //叫庄

CAndroidUserItemSink::CAndroidUserItemSink()
	: m_pTableFrame(NULL)
	, m_wChairID(INVALID_CHAIR)
	, m_pAndroidUserItem(NULL)
{
botddz=new my_ddzboot;
m_GameLogic =new CGameLogic;        
botddz->GetBotProcess()->GetCodePX = new (CGameLogic);
	//botddz->Getbotddz->GetBotProcesscess().
}

CAndroidUserItemSink::~CAndroidUserItemSink()
{

}

void CAndroidUserItemSink::RepositionSink()
{
	m_pTableFrame = NULL;
	m_wChairID = INVALID_CHAIR;
	m_pAndroidUserItem = NULL;
	memset(&m_cbHandCardData, 0, sizeof(m_cbHandCardData));
	botddz->GetBotProcess()->onCard = { 0 };
	botddz->GetBotProcess()->DownCard = { 0 };
    botddz->GetBotProcess()->ddz_max = { 0 };//大王存储
	botddz->GetBotProcess()->ddz_two = { 0 };//存储2的结构
	botddz->GetBotProcess()->ddz_lian_1 = { 0 };//存储连牌结构l
	botddz->GetBotProcess()->ddz_one = { 0 };//存储1的结构
	botddz->GetBotProcess()->ddz_twolian = { 0 };//存储双连结构
	botddz->GetBotProcess()->ddz_zd = { 0 };//炸弹结构存储
	botddz->GetBotProcess()->ddz_fj = { 0 };//飞机结构存储
	botddz->GetBotProcess()->ddz_three = { 0 };//三连结构存储
	botddz->GetBotProcess()->ddz_san = { 0 };//散牌存储
	botddz->GetBotProcess()->ddz_two_fx = { 0 };//对子存储
	botddz->GetBotProcess()->ddz_old = { 0 }; //最后出牌

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
	case TIME_1:
	{
		m_pAndroidUserItem->KillTimer(TIME_1);
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_OUT_CARD, &botddz->GetBotProcess()->mOutCard, sizeof(botddz->GetBotProcess()->mOutCard));

// botddz->GetBotProcess()->SetPX_CZ(1);
		
	//	std::cout << "自己剩余牌:" << botddz->GetBotProcess()->pssj_vh << std::endl;

	}
	break;

	case TIME_2:
	{
		m_pAndroidUserItem->KillTimer(TIME_2);
		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_PASS_CARD, NULL, 0);
		
//	////////printf("\r\n管不了 ");

		//
	}
	break;
	case TIME_3:
	{
		m_pAndroidUserItem->KillTimer(TIME_3);
		CMD_C_CallBanker callbanker;
		if (userdz==4)
		{
			if (botddz->GetBotProcess()->bot_king == 2)
			{
				callbanker.cbCallInfo = 3;
			}
		if (botddz->GetBotProcess()->bot_king == 3)
		{
			callbanker.cbCallInfo = 1; //叫一分
		}
		if (botddz->GetBotProcess()->bot_king == 1 || botddz->GetBotProcess()->bot_king == 0)
		{
			callbanker.cbCallInfo = 4;
		}
	}
else
{
	if (botddz->GetBotProcess()->bot_king == 2)
	{
		callbanker.cbCallInfo = 3;
	}
	if (botddz->GetBotProcess()->bot_king == 3)
	{
		callbanker.cbCallInfo = 4; //不叫
	}
	if (botddz->GetBotProcess()->bot_king == 1 || botddz->GetBotProcess()->bot_king == 0)
	{
		callbanker.cbCallInfo = 4;
	}
}


		m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_CALL_BANKER, &callbanker, sizeof(callbanker)); //
	
	}
	break;



	default:break;
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
	case SUB_S_CALL_BANKER: //谁叫地主
	{
		return OnSubGameCallBank(pData, wDataSize);
	}
case SUB_S_BANKER_INFO: //装家信息
	{
		return OnSubGameBank(pData, wDataSize);
	}
	case SUB_S_OUT_CARD://用户消息出牌
	{
		return OnSubSendCard(pData, wDataSize);
	}
	case SUB_S_OUT_START_START: //开始出牌
	{
		return OnSubStartCard(pData, wDataSize);
	}
	case SUB_S_PASS_CARD: //玩家弃牌
	{
		return OnSubUserPass(pData, wDataSize);
	}
	case SUB_S_CHEAT_LOOK_CARD:
	{

	 OnCard(pData, wDataSize);
	 break;
		// 看牌结果
	//	botddz->GetBotProcess()->Set_FX_JDSP(0);
	}
	case SUB_S_GAME_CONCLUDE:
	{
		OnGameOver(pData, wDataSize);
	break;
	}
	}
	return true;
}

bool CAndroidUserItemSink::OnSubGameStart(const void * pBuffer, CT_DWORD wDataSize) //游戏开始
{
	if (wDataSize != sizeof(CMD_S_GameStart))
	{
		return false;
	}

	//拷贝手牌数据
	CMD_S_GameStart* pGameStart = (CMD_S_GameStart*)pBuffer;
	if (pGameStart)
	{
		userdz = 4;
	    LookCard_1 = 0;
		botddz->GetBotProcess()->jdyp = false;
		botddz->GetBotProcess()->OnGameCard = NORMAL_COUNT;//上家
		botddz->GetBotProcess()->DownGameCard = NORMAL_COUNT; //下家扑克数
		botddz->GetBotProcess()->pssj_vh = NORMAL_COUNT; //自己
		botddz->GetBotProcess()->bot_king = 0;
		botddz->GetBotProcess()->Bankxin = 0;
		botddz->GetBotProcess()->BotID = m_wChairID;
		memcpy(m_cbHandCardData, pGameStart->cbCardData, sizeof(pGameStart->cbCardData));
		memcpy(botddz->GetBotProcess()->myCard, pGameStart->cbCardData, sizeof(pGameStart->cbCardData));
		botddz->GetBYTOINT(m_cbHandCardData);
	   if (pGameStart->dwCurrentUser == m_wChairID)
		{
			SetTime(3);
		}
	}//nohup . / gameserver 100 &


	//m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_CALL_BANKER, &callbanker, sizeof(callbanker));
	//jia

	return true;
}
bool CAndroidUserItemSink::OnSubGameCallBank(const void * Buffer, CT_DWORD DataSize) //谁叫地主
{
	if (DataSize != sizeof(CMD_S_CallBanker))
	{
		return false;
	}
	CMD_S_CallBanker* pGameCall = (CMD_S_CallBanker*)Buffer;
	if(pGameCall->cbCallInfo!=4)
	userdz=pGameCall->cbCallInfo;

	if (pGameCall->dwCurrentUser == m_wChairID)
	{
		SetTime(3);

	}

	return true;
}
bool CAndroidUserItemSink::OnSubGameBank(const void * pBuffer, CT_DWORD wDataSize) //庄家信息
{
	if (wDataSize != sizeof(CMD_S_BankerInfo))
	{
	
		return false;
	}
	CMD_S_BankerInfo* pGameBank = (CMD_S_BankerInfo*)pBuffer;
	m_dwBankerUser = pGameBank->dwBankerUser;
	botddz->GetBotProcess()->DownGameID = (m_wChairID + 1) % GAME_PLAYER;//下家
	botddz->GetBotProcess()->OnGameID = (m_wChairID + 2) % GAME_PLAYER;


	if (pGameBank->dwBankerUser != m_wChairID)
	{
	
				CMD_C_Double add_double;

				add_double.cbDoubleInfo = 1;
				if (botddz->GetBotProcess()->bot_king ==1|| botddz->GetBotProcess()->bot_king==2)
				{
				add_double.cbDoubleInfo = ST_DZ;
				}else
				{
				add_double.cbDoubleInfo = 2;
				}
				m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_ADD_DOUBLE, &add_double, sizeof(add_double));
	}
	if (pGameBank->dwBankerUser == m_wChairID)
	{
		botddz->GetBotProcess()->OnGameCard = NORMAL_COUNT;//上家
		botddz->GetBotProcess()->DownGameCard = NORMAL_COUNT; //下家扑克数
		botddz->GetBotProcess()->pssj_vh = NORMAL_COUNT; //自己
		for (int i = 0; i < 3; i++)
		{
			m_cbHandCardData[botddz->GetBotProcess()->pssj_vh] = pGameBank->cbBankerCard[i];
			
			botddz->GetBotProcess()->pssj_vh++;
		}
		botddz->GetBotProcess()->Bankxin = ST_DZ;
		memcpy(botddz->GetBotProcess()->myCard, m_cbHandCardData, sizeof(m_cbHandCardData));
		botddz->GetBYTOINT(m_cbHandCardData);
////printf("\r\n自己地主\r\n");
	}else if (botddz->GetBotProcess()->DownGameID == m_dwBankerUser)//下家)
	{
		botddz->GetBotProcess()->OnGameCard = NORMAL_COUNT;//上家
		botddz->GetBotProcess()->DownGameCard = MAX_COUNT; //下家扑克数

		botddz->GetBotProcess()->Bankxin = 3;
	}else if(botddz->GetBotProcess()->OnGameID== m_dwBankerUser)                               //上家地主
	{
		botddz->GetBotProcess()->OnGameCard = MAX_COUNT;//上家
		botddz->GetBotProcess()->DownGameCard = NORMAL_COUNT; //下家扑克数
		botddz->GetBotProcess()->pssj_vh = NORMAL_COUNT; //自己
	//	botddz->GetBotProcess()->SetSan_CZ();
		botddz->GetBotProcess()->Bankxin = 2;
	//	////////printf("上家地主\r\n");
	}



	lookCard_FXJD(0);
	return true;


}
bool CAndroidUserItemSink::OnSubUserPass(const void * cBuffer, CT_DWORD sDataSize)
{
	if (sDataSize != sizeof(CMD_S_PassCard))
	{
	
		return false;
	}

	CMD_S_PassCard *pGamePass= (CMD_S_PassCard*)cBuffer;
	if (pGamePass->dwCurrentUser == m_wChairID)
	{
		if (botddz->GetBotProcess()->ddz_old.old_Card == 0)
		{
			SetCard(3);//自己出牌
		}
		else if (botddz->GetBotProcess()->ddz_old.old_Card != 0)
		{
			SetCard(2);//管下家
			if (botddz->GetBotProcess()->BotID == 2)
			{
				//std::cout << "触发管xia家定时器" << std::endl;
				//std::cout << "Line:" << __LINE__ << "         Function:" << __FUNCTION__ << "        PS:" << botddz->GetBotProcess()->ddz_san.ps << "           ID=" << botddz->GetBotProcess()->BotID << std::endl;
			}
		}
	}
	return false;

}

CT_BOOL CAndroidUserItemSink:: OnSubSendCard(const void * SzBuffer, CT_DWORD tDataSize) //出牌信息
{
	if (tDataSize > sizeof(CMD_S_OutCard))
	{
		return false;
	}
	
	CMD_S_OutCard *pGameout = (CMD_S_OutCard*)SzBuffer;


	//botddz->GetBotProcess()->pssj_vh
	if (pGameout->dwOutCardUser == m_wChairID)
	{
		botddz->GetBotProcess()->ddz_old.old_Card = 0;
		botddz->GetBotProcess()->pssj_vh -= pGameout->cbCardCount;
		botddz->GetBotProcess()->ddz_old.old_Count = pGameout->cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, pGameout->cbCardData, sizeof(pGameout->cbCardData));
	
	}
	if (pGameout->dwOutCardUser == botddz->GetBotProcess()->DownGameID) //下家出牌
	{
		botddz->GetBotProcess()->ddz_old.old_Card = 2;
    	memcpy(m_oldeData, pGameout->cbCardData, sizeof(pGameout->cbCardData));
		m_GameLogic->SortCardList(pGameout->cbCardData, pGameout->cbCardCount, ST_ORDER);
		botddz->GetBotProcess()->DownGameCard -= pGameout->cbCardCount;
		botddz->GetBotProcess()->ddz_old.old_Count = pGameout->cbCardCount;
     CT_BYTE cts[MAX_COUNT] = { 0 };
		for (int i = 0; i < pGameout->cbCardCount; i++)
		{
			cts[i] = botddz->GetBotProcess()->GetCodePX->GetCardValue( pGameout->cbCardData[i]);
		}
	memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, cts, sizeof(cts));
	

}else if(pGameout->dwOutCardUser==botddz->GetBotProcess()->OnGameID)
		{
        	botddz->GetBotProcess()->ddz_old.old_Card = 1;
	       memcpy(m_oldeData, pGameout->cbCardData, sizeof(pGameout->cbCardData));
			m_GameLogic->SortCardList(pGameout->cbCardData, pGameout->cbCardCount, ST_ORDER);
			botddz->GetBotProcess()->OnGameCard= botddz->GetBotProcess()->OnGameCard - pGameout->cbCardCount;
	botddz->GetBotProcess()->ddz_old.old_Count = pGameout->cbCardCount;
	CT_BYTE cts[MAX_COUNT] = { 0 };
	for (int i = 0; i < pGameout->cbCardCount;i++)
	{
cts[i] = botddz->GetBotProcess()->GetCodePX->GetCardValue(pGameout->cbCardData[i]);
	}

      	memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, cts, sizeof(cts));
	
		}

		if (pGameout->dwCurrentUser == m_wChairID) //管牌
		{
			SetCard(1);//管上家
			if (botddz->GetBotProcess()->BotID == 2)
			{
				//std::cout << "触发管上家定时器" << std::endl;
			
			}
		}




		return true;

}


CT_BOOL CAndroidUserItemSink::OnSubStartCard(const void * bzBuffer, CT_DWORD eDataSize) //开始出牌
{
	if (eDataSize > sizeof(CMD_S_OutCard))
	{
		return false;
	}

	CMD_S_StartOutCard *pGameout = (CMD_S_StartOutCard*)bzBuffer;

	if (pGameout->dwCurrentUser == m_wChairID)
	{
		SetCard(0);
	}


	return true;


}

CT_VOID CAndroidUserItemSink::OnSendCard(CT_DWORD cont)
{
	if (cont == CT_SINGLE) //单张
	{

	/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 1)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e)
					{
						botddz->GetBotProcess()->myCard[i] = 0;
						break;
					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				SetTime(1);
				return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;
						break;
					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4f;
				SetTime(1);
			return;
			}
		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3])
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			int iqssdd=0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
	
						itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14&& botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e|| botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;
			
					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
				return;
			}
		}
		int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
		int sz2 = sz1 + 16;
		int sz3 = sz2 + 16;
		int sz4 = sz3 + 16;
		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
	botddz->GetBotProcess()->mOutCard.cbCardData[0] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				SetTime(1);
		break;
			}

		}

	return;
	}
	if (cont == CT_DOUBLE_CARD) //对子
	{
	
	/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
         	int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			int iqssdd = 0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
				itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
		SetTime(1);
				return;
			}
		}
		int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
		int sz2 = sz1 + 16;
		int sz3 = sz2 + 16;
		int sz4 = sz3 + 16;
		int qpssd = 0;
	for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
	botddz->GetBotProcess()->mOutCard.cbCardData[qpssd] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				qpssd++;
				if (qpssd == 2)
				{
					SetTime(1);
				itp = 99;
				}
			}

		}

	
		return;
	}

	if (cont == CT_THREE) //三连
	{


		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			int iqssdd = 0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
						itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
				return;
			}
		}
		int iql = 0;
		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[iql];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
	botddz->GetBotProcess()->mOutCard.cbCardData[iql] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				iql++;
				if (iql == 3)
				{
					SetTime(1);
					break;
				}
			
	


			}

		}

		return;
	}
	if (cont == CT_THREE_TAKE_ONE) //三代1
	{
	
		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[2] == botddz->GetBotProcess()->mOutCard.cbCardData[3])
			{
				int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
				int sz2 = sz1 + 16;
				int sz3 = sz2 + 16;
				int sz4 = sz3 + 16;
		int iqssdd = 0;
				for (int itp = 0; itp < 20; itp++)
				{
					if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
					{
						botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
						botddz->GetBotProcess()->myCard[itp] = { 0 };
						iqssdd++;
						if (iqssdd == 4)
						{
							SetTime(1);
							itp = 99;
						}
					}
				}
				return;
			}
		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
		}
	int iql = 0;

		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[iql];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
	botddz->GetBotProcess()->mOutCard.cbCardData[iql] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				iql++;
				itp=-1;
				if (iql == botddz->GetBotProcess()->mOutCard.cbCardCount)
				{
					SetTime(1);
					break;
				}
		}

		}

		return ;
	}
	if (cont == CT_THREE_TAKE_TWO) //三代2
	{
		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
int iqssdd = 0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
						itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
		}
       int iql = 0;
		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[iql];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
		botddz->GetBotProcess()->mOutCard.cbCardData[iql] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				iql++;
				itp = -1;
				if (iql == 5)
				{
					SetTime(1);
					break;
				}
			}

		}

		return;
}

	if (cont == CT_DOUBLE_LINE)
	{
		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
		
	int iqssdd = 0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
						itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);

				return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);

				return;
			}
		}
	int iql = 0;
		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[iql];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
		botddz->GetBotProcess()->mOutCard.cbCardData[iql] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				iql++;
				itp = -1;
				if (iql == botddz->GetBotProcess()->mOutCard.cbCardCount)
				{
				SetTime(1);
					break;
				}
			}

		}

		return;
	}
	if (cont == CT_SINGLE_LINE) //单连
	{
		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			int iqssdd = 0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
						itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);

				return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
		}
		int iql = 0;
		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[iql];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
				botddz->GetBotProcess()->mOutCard.cbCardData[iql] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				iql++;
				itp = -1;
				if (iql == botddz->GetBotProcess()->mOutCard.cbCardCount)
				{
					SetTime(1);
					break;
				}
			}

		}

		return;
	}
	if (cont == CT_THREE_LINE)
	{
	
		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			int iqssdd = 0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
						itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);

				return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
		}
		int iql = 0;
		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[iql];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
				botddz->GetBotProcess()->mOutCard.cbCardData[iql] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				iql++;
				itp = -1;
				if (iql == botddz->GetBotProcess()->mOutCard.cbCardCount)
				{
					SetTime(1);
					break;
				}
			}

		}
		return;
	}
	if (cont == CT_FOUR_TAKE_ONE|| cont == CT_FOUR_TAKE_TWO||cont== CT_BOMB_CARD)
	{
		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 4 && botddz->GetBotProcess()->mOutCard.cbCardData[0] == botddz->GetBotProcess()->mOutCard.cbCardData[3]) //炸弹
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[0];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			int iqssdd = 0;
			for (int itp = 0; itp < 20; itp++)
			{
				if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
				{
					botddz->GetBotProcess()->mOutCard.cbCardData[iqssdd] = botddz->GetBotProcess()->myCard[itp];
					botddz->GetBotProcess()->myCard[itp] = { 0 };
					iqssdd++;
					if (iqssdd == 4)
					{
						SetTime(1);
						itp = 99;
					}
				}
			}
			return;

		}
		if (botddz->GetBotProcess()->mOutCard.cbCardCount == 2)
		{
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 14 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 15)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
				SetTime(1);
return;
			}
			if (botddz->GetBotProcess()->mOutCard.cbCardData[0] == 15 && botddz->GetBotProcess()->mOutCard.cbCardData[1] == 14)
			{
				for (int i = 0; i < 20; i++)
				{
					if (botddz->GetBotProcess()->myCard[i] == 0x4e || botddz->GetBotProcess()->myCard[i] == 0x4f)
					{
						botddz->GetBotProcess()->myCard[i] = 0;

					}

				}
				botddz->GetBotProcess()->mOutCard.cbCardData[0] = 0x4e;
				botddz->GetBotProcess()->mOutCard.cbCardData[1] = 0x4f;
		SetTime(1);

				return;
			}
		}
		int iql = 0;
		for (int itp = 0; itp < MAX_COUNT; itp++)
		{
			int sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[iql];
			int sz2 = sz1 + 16;
			int sz3 = sz2 + 16;
			int sz4 = sz3 + 16;
			if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
			{
			botddz->GetBotProcess()->mOutCard.cbCardData[iql] = botddz->GetBotProcess()->myCard[itp];
				botddz->GetBotProcess()->myCard[itp] = { 0 };
				iql++;
				itp = -1;
				if (iql == botddz->GetBotProcess()->mOutCard.cbCardCount)
				{
					SetTime(1);
					break;
				}
			}

		}
		return;
	}
}
CT_VOID CAndroidUserItemSink::LookSendCard(CT_WORD userid) //发送看牌
{
	CMD_C_CheatLookCard lookCard;
if (userid == botddz->GetBotProcess()->OnGameID)
	{
		LookCard_1 = 0;
	}
	else if (userid == botddz->GetBotProcess()->DownGameID)
	{
		LookCard_1 = 1;
	}

	lookCard.wBeCheatChairID = userid;
	m_pTableFrame->OnGameEvent(m_wChairID, SUB_C_CHEAT_LOOK_CARD, &lookCard, sizeof(lookCard));
}

CT_VOID CAndroidUserItemSink ::OnCard(const void *Buffer, CT_DWORD eDataSize) //接收看牌信息n
{
	CMD_S_CheatLookCard* LookCard = (CMD_S_CheatLookCard*)Buffer;
	if (botddz->GetBotProcess()->DownGameCard>20&& botddz->GetBotProcess()->OnGameCard>20)
	{
		////////printf("越界\r\n");
		return;
	}
	if (LookCard_1==0)
	{
		CT_BYTE ct[20] = {0};

		
		for (int i = 0; i < botddz->GetBotProcess()->OnGameCard; i++)
		{
			ct[i] = botddz->GetBotProcess()->GetCodePX->GetCardValue(LookCard->cbCardData[i]);

	
		}
		memcpy(botddz->GetBotProcess()->onCard.Cdata, ct, sizeof(ct)); //储存上家

	}
	else if (LookCard_1== 1)
	{
		CT_BYTE ct[20] = { 0 };


		for (int i = 0; i <botddz->GetBotProcess()->DownGameCard; i++)
		{
			ct[i] = botddz->GetBotProcess()->GetCodePX->GetCardValue(LookCard->cbCardData[i]);
	}


		memcpy(botddz->GetBotProcess()->DownCard.Cdata, ct, sizeof(ct));
	
	}

}
bool CAndroidUserItemSink::lookCard_FXJD(int onando)
{

	botddz->GetBotProcess()->onCard = { 0 };
	botddz->GetBotProcess()->DownCard = { 0 };
	LookSendCard(botddz->GetBotProcess()->OnGameID);
	LookSendCard(botddz->GetBotProcess()->DownGameID);
	botddz->GetBotProcess()->jdyp=botddz->GetBotProcess()->Set_FX_JDSP(onando); //分析绝对手牌
	if (botddz->GetBotProcess()->jdyp == true)
		return true;
	
	return false;
}
bool CAndroidUserItemSink::OnGameOver(const void * sbzBuffer, CT_DWORD teDataSize)
{
	m_pAndroidUserItem->KillTimer(TIME_1);
	m_pAndroidUserItem->KillTimer(TIME_2);
	m_pAndroidUserItem->KillTimer(TIME_3);


	return true;
}

CT_VOID  CAndroidUserItemSink::SetTime(int icound)
{
	if (icound == 1)
	{
		int nRand =rand() % 7 + 3;
	//	////////printf("SetTimer OK-----------\r\n-----------\r\n");
		m_pAndroidUserItem->SetTimer(TIME_1, nRand * 500, 0);

	}
	if (icound == 2)
	{
		int nRand = rand() %7 + 3; 
		//////////printf("SetTimer OK-----------\r\n-----------\r\n");
		m_pAndroidUserItem->SetTimer(TIME_2, nRand * 500, 0);
	}
	if (icound == 3)
	{
		int nRand = rand() % 5 + 3; 

		m_pAndroidUserItem->SetTimer(TIME_3, nRand * 1000, 0);
	}

}
void CAndroidUserItemSink::SetCard(int Icount)
{
	if (Icount == 1) //管上家
	{
		m_GameLogic->SortCardList(m_oldeData, botddz->GetBotProcess()->ddz_old.old_Count, ST_ORDER);
		CT_BYTE rs = botddz->GetBotProcess()->GetCodePX->GetCardType(m_oldeData, botddz->GetBotProcess()->ddz_old.old_Count); //管上家出牌
		lookCard_FXJD(1);//显示牌信息，并拿取其他2家牌型，并分析绝对手牌  参数，0为自己出牌，1为管别人的牌
		switch (rs)
		{
		case CT_SINGLE://单
			bool asd;
			asd = botddz->GetBotProcess()->Set_SINGLE1(botddz->GetBotProcess()->ddz_old.old_Card_data, 1, botddz->GetBotProcess()->OnGameCard);
			if (asd == false)
			{
				SetTime(2);

			}
			else if (asd == true)
			{

				OnSendCard(CT_SINGLE);
	
			}
			break;
		case CT_DOUBLE_CARD://对
			bool asdt;
			asdt = botddz->GetBotProcess()->Set_DOUBLE(botddz->GetBotProcess()->ddz_old.old_Card_data, 1, botddz->GetBotProcess()->OnGameCard);
			if (asdt == false)
			{

				SetTime(2);
			}
			else if (asdt == true)
			{
	OnSendCard(CT_DOUBLE_CARD);
			}
			break;
		case CT_THREE:// 三条啥也不带，属于神经病
			bool asd1;
			asd1 = botddz->GetBotProcess()->Set_CT_THREE(botddz->GetBotProcess()->ddz_old.old_Card_data, 1, botddz->GetBotProcess()->OnGameCard);
			if (asd1 == false)
			{
				SetTime(2);
			}
			else if (asd1 == true)
			{
	OnSendCard(CT_THREE);
	
			}
			break;
		case CT_SINGLE_LINE://单连
			bool asdz;
			asdz = botddz->GetBotProcess()->Set_SINGLE_LINE(botddz->GetBotProcess()->ddz_old.old_Card_data, 1, botddz->GetBotProcess()->DownGameCard);
			if (asdz == false)
			{

				SetTime(2);
			}
			else if (asdz == true)
			{

				OnSendCard(CT_SINGLE_LINE);
			}
			break;
		case CT_DOUBLE_LINE://双连
			bool asdp;
			asdp = botddz->GetBotProcess()->Set_DOUBLE_LINE(botddz->GetBotProcess()->ddz_old.old_Card_data, 1, botddz->GetBotProcess()->OnGameCard);
			if (asdp == false)
			{

				SetTime(2);

			}
			else if (asdp == true)
			{
				OnSendCard(CT_DOUBLE_LINE);
		
			}
			break;
		case CT_THREE_LINE://三连
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdp1;
				asdp1 = botddz->GetBotProcess()->Set_JdOutCard(1, 7);
				if (asdp1 == false)
				{
					SetTime(2);

				}
				else if (asdp1 == true)
				{

					OnSendCard(CT_THREE_LINE);
			break;
				}
			}
			else if (botddz->GetBotProcess()->jdyp == false)
			{
				SetTime(2);
			}
			break;
		case CT_THREE_TAKE_ONE://三带1
			bool asd2;
			asd2 = botddz->GetBotProcess()->Set_CT_THREE_1(botddz->GetBotProcess()->ddz_old.old_Card_data, 1, botddz->GetBotProcess()->OnGameCard);
			if (asd2 == false)
			{
				SetTime(2);
			}
			else if (asd2 == true)
			{

			
				OnSendCard(CT_THREE_TAKE_ONE);
		
			}
			break;
		case CT_THREE_TAKE_TWO://三代对
			bool asfs;
			asfs = botddz->GetBotProcess()->Set_CT_THREE_2(botddz->GetBotProcess()->ddz_old.old_Card_data, 1, botddz->GetBotProcess()->DownGameCard);
			if (asfs == false)
			{
				SetTime(2);

			}
			else if (asfs == true)
			{

				OnSendCard(CT_THREE_TAKE_TWO);
			}
			break;
		case CT_FOUR_TAKE_ONE://4带2单
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdpf;
				asdpf = botddz->GetBotProcess()->Set_JdOutCard(1, 9);
				if (asdpf == false)
				{
					SetTime(2);

				}
				else if (asdpf == true)
				{
					OnSendCard(CT_FOUR_TAKE_ONE);
				break;
				}
			}
			else if (botddz->GetBotProcess()->jdyp == false)
			{
				SetTime(2);
			}
			break;
		case CT_FOUR_TAKE_TWO://4带 2对
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdpg;
				asdpg = botddz->GetBotProcess()->Set_JdOutCard(1, 9);
				if (asdpg == false)
				{
					SetTime(2);

				}
				else if (asdpg == true)
				{
					OnSendCard(CT_FOUR_TAKE_TWO);
		
					break;
				}
			}
			else 	if (botddz->GetBotProcess()->jdyp == false)
			{
				SetTime(2);
			}
			break;
		case CT_BOMB_CARD://炸弹
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdpz;
				asdpz = botddz->GetBotProcess()->Set_JdOutCard(1, 0);
				if (asdpz == false)
				{
					SetTime(2);

				}
				else if (asdpz == true)
				{
					OnSendCard(CT_BOMB_CARD);
			
					break;
				}
			}
			else 	if (botddz->GetBotProcess()->jdyp == false)
			{

				SetTime(2);
			}
			break;
		case CT_MISSILE_CARD://火箭
			SetTime(2);
			break;
		}

	}//管
	if (Icount == 2)//管下家,
	{
	m_GameLogic->SortCardList(m_oldeData, botddz->GetBotProcess()->ddz_old.old_Count, ST_ORDER); //排序
		CT_BYTE rs = botddz->GetBotProcess()->GetCodePX->GetCardType(m_oldeData, botddz->GetBotProcess()->ddz_old.old_Count);//管下家出牌
		lookCard_FXJD(1);//显示牌信息，并拿取其他2家牌型，并分析绝对手牌  参数，0为自己出牌，1为管别人的牌
	switch (rs)
		{
		case CT_SINGLE://单
			bool asd;
			asd = botddz->GetBotProcess()->Set_SINGLE1(botddz->GetBotProcess()->ddz_old.old_Card_data, 2, botddz->GetBotProcess()->DownGameCard);
			if (asd == false)
			{
				SetTime(2);
				//	////////printf("\r\n管不了 过 剩余%d\r\n", botddz->GetBotProcess()->pssj_vh);

			}
			else 	if (asd == true)
			{
				OnSendCard(CT_SINGLE);
	
			}
			break;
		case CT_DOUBLE_CARD://对
			bool asdt;
			asdt = botddz->GetBotProcess()->Set_DOUBLE(botddz->GetBotProcess()->ddz_old.old_Card_data, 2, botddz->GetBotProcess()->DownGameCard);
			if (asdt == false)
			{

				SetTime(2);
			}
			else 	if (asdt == true)
			{

				OnSendCard(CT_DOUBLE_CARD);
	
			}
			break;
		case CT_THREE:// 三条啥也不带，属于神经病
			bool asd1;
			asd1 = botddz->GetBotProcess()->Set_CT_THREE(botddz->GetBotProcess()->ddz_old.old_Card_data, 2, botddz->GetBotProcess()->DownGameCard);
			if (asd1 == false)
			{
				SetTime(2);
			}
			else 	if (asd1 == true)
			{

				OnSendCard(CT_THREE);
	
			}
			break;
		case CT_SINGLE_LINE://单连
			bool asdz;
			asdz = botddz->GetBotProcess()->Set_SINGLE_LINE(botddz->GetBotProcess()->ddz_old.old_Card_data, 2, botddz->GetBotProcess()->DownGameCard);
			if (asdz == false)
			{

				SetTime(2);
			}
			else 	if (asdz == true)
			{

				OnSendCard(CT_SINGLE_LINE);
	
			}
			break;
		case CT_DOUBLE_LINE://双连
			bool asd9;
			asd9 = botddz->GetBotProcess()->Set_DOUBLE_LINE(botddz->GetBotProcess()->ddz_old.old_Card_data, 2, botddz->GetBotProcess()->DownGameCard);
			if (asd9 == false)
			{
				SetTime(2);
			}
			else if (asd9 == true)
			{

				OnSendCard(CT_DOUBLE_LINE);
	
			}
			break;
		case CT_THREE_LINE://三不带
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdp1;
				asdp1 = botddz->GetBotProcess()->Set_JdOutCard(1, 7);
				if (asdp1 == false)
				{
					SetTime(2);

				}
				else 	if (asdp1 == true)
				{
					OnSendCard(CT_THREE_LINE);
	break;
				}
			}
			else 	if (botddz->GetBotProcess()->jdyp == false)
			{
				SetTime(2);

			}
			break;
		case CT_THREE_TAKE_ONE://三带1
			bool asds;
			asds = botddz->GetBotProcess()->Set_CT_THREE_1(botddz->GetBotProcess()->ddz_old.old_Card_data, 2, botddz->GetBotProcess()->DownGameCard);
			if (asds == false)
			{
				SetTime(2);
			}
			else 	if (asds == true)
			{
				OnSendCard(CT_THREE_TAKE_ONE);
	
			}
			break;
		case CT_THREE_TAKE_TWO://三代对
			bool asfs;
			asfs = botddz->GetBotProcess()->Set_CT_THREE_2(botddz->GetBotProcess()->ddz_old.old_Card_data, 2, botddz->GetBotProcess()->DownGameCard);
			if (asfs == false)
			{
				SetTime(2);
			}
			else if (asfs == true)
			{
				OnSendCard(CT_THREE_TAKE_TWO);
	
			}
			break;
		case CT_FOUR_TAKE_ONE://4带2单
		{
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdpf;
				asdpf = botddz->GetBotProcess()->Set_JdOutCard(1, 9);
				if (asdpf == false)
				{
					SetTime(2);

				}
				else 	if (asdpf == true)
				{
					OnSendCard(CT_FOUR_TAKE_ONE);
			break;
				}
			}
			else 	if (botddz->GetBotProcess()->jdyp == false)
			{
				SetTime(2);
			}
		}
		break;
		case CT_FOUR_TAKE_TWO://4带 2对
		{
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdpg;
				asdpg = botddz->GetBotProcess()->Set_JdOutCard(1, 9);
				if (asdpg == false)
				{
					SetTime(2);

				}
				else if(asdpg == true)
				{
					OnSendCard(CT_FOUR_TAKE_TWO);
		break;
				}
			}
			else  	if (botddz->GetBotProcess()->jdyp == false)
			{
				SetTime(2);
			}
		}

		break;
		case CT_BOMB_CARD://炸弹
		{
			if (botddz->GetBotProcess()->jdyp == true)
			{
				bool asdpz;
				asdpz = botddz->GetBotProcess()->Set_JdOutCard(1, 0);
				if (asdpz == false)
				{
					SetTime(2);

				}
				else 	if (asdpz == true)
				{

					OnSendCard(CT_BOMB_CARD);
		
					break;
				}
			}
			else 	if (botddz->GetBotProcess()->jdyp == false)
			{

				SetTime(2);
			}

		}

		break;
		case CT_MISSILE_CARD://火箭
		{
			SetTime(2);
		}

		break;
		}


	}
	if (Icount == 3)
	{
	
			//	////////printf("自己出牌\r\n");
			bool abc = lookCard_FXJD(0);
			if (abc == true)
			{
				bool outCard = botddz->GetBotProcess()->Set_JdOutCard(0, 0);
				if (outCard == true)
				{

					CT_DWORD sz1, sz2, sz3, sz4;
					for (int ist = 0; ist <botddz->GetBotProcess()->mOutCard.cbCardCount; ist++)
					{
						sz1 = botddz->GetBotProcess()->mOutCard.cbCardData[ist];
						sz2 = sz1 + 16;
						sz3 = sz2 + 16;
						sz4 = sz3 + 16;
						if (sz1 == 15)
							sz1 = 79;
						if (sz1 == 14)
							sz1 = 78;
						for (int itp = 0; itp < MAX_COUNT; itp++)
						{
							if (botddz->GetBotProcess()->myCard[itp] == sz1 || botddz->GetBotProcess()->myCard[itp] == sz2 || botddz->GetBotProcess()->myCard[itp] == sz3 || botddz->GetBotProcess()->myCard[itp] == sz4)
							{
								if (sz1 == 78)
								{
									botddz->GetBotProcess()->mOutCard.cbCardData[ist] = 0x4e;
									botddz->GetBotProcess()->myCard[itp] = { 0 };

									break;
								}
								if (sz1 == 79)
								{
									botddz->GetBotProcess()->mOutCard.cbCardData[ist] = 0x4f;
									botddz->GetBotProcess()->myCard[itp] = { 0 };

									break;
								}
								botddz->GetBotProcess()->mOutCard.cbCardData[ist] = botddz->GetBotProcess()->myCard[itp];
								botddz->GetBotProcess()->myCard[itp] = { 0 };
								break;
							}
						}
					}
					/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
					memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
					botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
					SetTime(1);
					return;
				}

			}
			CT_DWORD xCntld = botddz->GetBotProcess()->Set_FX_QZ();
			switch (xCntld)
			{
			case 1:
				botddz->GetBotProcess()->Set_ZHPX(2, botddz->GetBotProcess()->ddz_two_fx.ddz_two_1, 1);//出对子
				break;
			case 2:
				botddz->GetBotProcess()->Set_ZHPX(botddz->GetBotProcess()->ddz_lian_1.lian_1, botddz->GetBotProcess()->ddz_lian_1.ddzlian_1, 2);//连顺
				break;
			case 3:
				botddz->GetBotProcess()->Set_ZHPX(botddz->GetBotProcess()->ddz_twolian.lian_1, botddz->GetBotProcess()->ddz_twolian.ddztwo_lian_1, 3);//出对连顺
				break;
			case 4:
				botddz->GetBotProcess()->Set_ZHPX(1, botddz->GetBotProcess()->ddz_san.ddz_san_1, 4);//出散牌
				break;
			case 5:
				botddz->GetBotProcess()->Set_ZHPX(4, botddz->GetBotProcess()->ddz_three.ddz_three_1, 5);//出三带1
				break;
			case 6:
				botddz->GetBotProcess()->Set_ZHPX(8, botddz->GetBotProcess()->ddz_fj.ddz_feiji_1, 6);//飞机 带 
				break;
			case 7:
				botddz->GetBotProcess()->Set_ZHPX(0, botddz->GetBotProcess()->ddz_fj.ddz_feiji_1, 7);//全没有//还没写代码
				break;

			}

			/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
			memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
			botddz->GetBotProcess()->ddz_old.old_Card = 0;*/

		//	std::cout << "Line:" << __LINE__ << "         Function:" << __FUNCTION__ << "        PS:" << botddz->GetBotProcess()->ddz_san.ps << "           ID=" << botddz->GetBotProcess()->BotID << std::endl;
			SetTime(1);
			return;
		}
	if (Icount == 0) //自己开始出牌，做地主
	{
		CT_DWORD xCntld = botddz->GetBotProcess()->Set_FX_QZ();
		switch (xCntld)
		{
		case 1:
			botddz->GetBotProcess()->Set_ZHPX(2, botddz->GetBotProcess()->ddz_two_fx.ddz_two_1, 1);//出对子
			break;
		case 2:
			botddz->GetBotProcess()->Set_ZHPX(botddz->GetBotProcess()->ddz_lian_1.lian_1, botddz->GetBotProcess()->ddz_lian_1.ddzlian_1, 2);//连顺
			break;
		case 3:
			botddz->GetBotProcess()->Set_ZHPX(botddz->GetBotProcess()->ddz_twolian.lian_1, botddz->GetBotProcess()->ddz_twolian.ddztwo_lian_1, 3);//出对连顺
			break;
		case 4:
			botddz->GetBotProcess()->Set_ZHPX(1, botddz->GetBotProcess()->ddz_san.ddz_san_1, 4);//出散牌
			break;
		case 5:
			botddz->GetBotProcess()->Set_ZHPX(4, botddz->GetBotProcess()->ddz_three.ddz_three_1, 5);//出三带
			break;
		case 6:
			botddz->GetBotProcess()->Set_ZHPX(8, botddz->GetBotProcess()->ddz_fj.ddz_feiji_1, 6);
			break;

		}
		/*	botddz->GetBotProcess()->ddz_old.old_Count = botddz->GetBotProcess()->mOutCard.cbCardCount;
		memcpy(botddz->GetBotProcess()->ddz_old.old_Card_data, botddz->GetBotProcess()->mOutCard.cbCardData, sizeof(botddz->GetBotProcess()->mOutCard.cbCardData));
		botddz->GetBotProcess()->ddz_old.old_Card = 0;*/
		SetTime(1);

	}

	return;
}