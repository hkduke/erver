#include "DdzAiInterface.h"
#include "ConfigIniFile.h"
#include "../DdzAiPublic/include/MsgBodyLogic.h"
#include "Log.h"
#include <vector>
#include "stgy_split_card.h"
#include "ddz_move.h"

/* ----------------------------------------------------------------------------------------------------------*/
ddz_ai_interface::ddz_ai_interface(): m_oCommunication2(Communication2::CT_Client) , m_uiThreadMissionId(0), m_bCommunicationInit(false), m_pfFunc(0)
{
}

/* ----------------------------------------------------------------------------------------------------------*/
ddz_ai_interface::~ddz_ai_interface()
{
}

/* ----------------------------------------------------------------------------------------------------------*/
unsigned int ddz_ai_interface::CreateAi()
{
	return ++m_uiThreadMissionId;
}

/* ----------------------------------------------------------------------------------------------------------*/
int ddz_ai_interface::Initialize(const char* szCfg)
{
	ConfigIniFile oIniFile;
	if ( oIniFile.OpenFile( szCfg ) < 0 )
	{
		return -1;
	}
	
	oIniFile.GetIntValue( "AiLib", "ServerID", m_iServerID, 1 );

	char szConfigName[256] = {0};
	oIniFile.GetStringValue( "AiLib", "CommunicationConfig", szConfigName, sizeof(szConfigName));
	
	if (!m_bCommunicationInit)
	{
		int iRet = m_oCommunication2.Initialize( Cst_DBServer, m_iServerID, szConfigName, this );
		if ( iRet < 0 )
		{
			return -2;
		}

		m_bCommunicationInit = true;
	}

	return 0;
}

/* ----------------------------------------------------------------------------------------------------------*/
void ddz_ai_interface::Update()
{
	m_oCommunication2.Update(this);
}

/*---------------------------------------------------------------------------------------------------------*/
int ddz_ai_interface::Execute( SwitchMsgHead * header, void* pMsg, short nMsgLength, int iConnection )
{
	if ( !pMsg )
	{
		return -1;
	}

	NetMsg * p_msg = (NetMsg*)pMsg;

	int iRet = 0;
	switch( p_msg->m_oHead.m_sMsgID )
	{
	case MC_S2C_Play_Card_Result:
		if (m_pfFunc != 0)
		{
			S2C_Play_Card_Result* pResult = (S2C_Play_Card_Result*)p_msg->m_pMsgBody;

			pai_interface_move move;
			move._type = pResult->m_oDdzMove._type;
			move._alone_1 = pResult->m_oDdzMove._alone_1;
			move._alone_2 = pResult->m_oDdzMove._alone_2;
			move._alone_3 = pResult->m_oDdzMove._alone_3;
			move._alone_4 = pResult->m_oDdzMove._alone_4;
			move._airplane_pairs = pResult->m_oDdzMove._airplane_pairs;
			move._combo_count = pResult->m_oDdzMove._combo_count;
			memcpy(move._combo_list, pResult->m_oDdzMove._combo_list, 20*sizeof(unsigned char));

			(*m_pfFunc)(pai_event_play_card, pResult->m_uiAiHandle, &move);
		}
		break;

	case MC_S2C_Passive_Play_Card_Result:
		if (m_pfFunc != 0)
		{
			S2C_Play_Card_Result* pResult = (S2C_Play_Card_Result*)p_msg->m_pMsgBody;

			pai_interface_move move;
			move._type = pResult->m_oDdzMove._type;
			move._alone_1 = pResult->m_oDdzMove._alone_1;
			move._alone_2 = pResult->m_oDdzMove._alone_2;
			move._alone_3 = pResult->m_oDdzMove._alone_3;
			move._alone_4 = pResult->m_oDdzMove._alone_4;
			move._airplane_pairs = pResult->m_oDdzMove._airplane_pairs;
			move._combo_count = pResult->m_oDdzMove._combo_count;
			memcpy(move._combo_list, pResult->m_oDdzMove._combo_list, 20*sizeof(unsigned char));

			(*m_pfFunc)(pai_event_play_card_passive, pResult->m_uiAiHandle, &move);
		}
		break;

	case MC_S2C_Grab_Landlord_Result:
		{
			if (m_pfFunc != 0)
			{
				S2C_Grab_Landlord_Result* pResult = (S2C_Grab_Landlord_Result*)p_msg->m_pMsgBody;

				pai_interface_move move;
				move._type = 0;
				move._alone_1 = (unsigned char)(10*pResult->m_uiScore);
				move._alone_2 = 0;
				move._alone_3 = 0;
				move._alone_4 =0;
				move._airplane_pairs = false;
				move._combo_count = 0;
				memset(move._combo_list, 20, 20*sizeof(unsigned char));

				(*m_pfFunc)(pai_event_grab_landlord, pResult->m_uiAiHandle, &move);
			}
		}
		break;
	}
	
	return iRet;
}

/*---------------------------------------------------------------------------------------------------------*/
void ddz_ai_interface::SetPlayCardInterface(PlayCard_Func func)
{
	m_pfFunc = func;
}

/*---------------------------------------------------------------------------------------------------------*/
int ddz_ai_interface::AiPlayCard(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char landlord )
{
	for(int i = pai_type_3; i <= pai_type_2; ++i)
	{
		if (pai_list1[i] > 4 || pai_list2[i] > 4 || pai_list3[i] > 4)
			return -1;
	}

	if (pai_list1[pai_type_blackjack] > 1 || pai_list2[pai_type_blackjack] > 1 || pai_list2[pai_type_blackjack] > 1 || 
		pai_list1[pai_type_blossom] > 1 || pai_list2[pai_type_blossom] > 1 || pai_list2[pai_type_blossom] > 1 )
		return -1;

	if (landlord > 2)
		return -1;

	C2S_Play_Card c2s_PlayCard;
	c2s_PlayCard.m_uiAiHandle = handle;
	for(int i = 0; i < pai_type_max; ++i)
	{
		c2s_PlayCard.m_vPaiList1[i] = pai_list1[i];
		c2s_PlayCard.m_vPaiList2[i] = pai_list2[i];
		c2s_PlayCard.m_vPaiList3[i] = pai_list3[i];
	}
	c2s_PlayCard.m_ucLandlordIndex = landlord;

	m_oCommunication2.SendMsgByUID(&c2s_PlayCard, Cst_DBServer, m_iServerID, handle);
	return 0;
}


/*---------------------------------------------------------------------------------------------------------*/
int ddz_ai_interface::GrabLandlord2(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char grab_pai[3])
{
	// 检查数据是否合法	
	for(int i = pai_type_3; i <= pai_type_2; ++i)
	{
		if (pai_list1[i] > 4 || pai_list2[i] > 4 || pai_list3[i] > 4)
			return -1;
	}

	if (pai_list1[pai_type_blackjack] > 1 || pai_list2[pai_type_blackjack] > 1 || pai_list2[pai_type_blackjack] > 1 || 
		pai_list1[pai_type_blossom] > 1 || pai_list2[pai_type_blossom] > 1 || pai_list2[pai_type_blossom] > 1 )
		return -1;

	C2S_Grab_Landlord c2s_grabLandlord;
	c2s_grabLandlord.m_uiAiHandle = handle;
	for(int i = 0; i < pai_type_max; ++i)
	{
		c2s_grabLandlord.m_vPaiList1[i] = pai_list1[i];
		c2s_grabLandlord.m_vPaiList2[i] = pai_list2[i];
		c2s_grabLandlord.m_vPaiList3[i] = pai_list3[i];
	}
	for(int i = 0; i < 3; ++i)
	{
		c2s_grabLandlord.m_vthreePaiList[i] = grab_pai[i];
	}

	m_oCommunication2.SendMsgByUID(&c2s_grabLandlord, Cst_DBServer, m_iServerID, handle);

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
int ddz_ai_interface::AiPlayCardPassive(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char landlord,  unsigned char outPaiIndex, const pai_interface_move* limit_card)
{
	// 检查数据是否合法	
	for(int i = pai_type_3; i <= pai_type_2; ++i)
	{
		if (pai_list1[i] > 4 || pai_list2[i] > 4 || pai_list3[i] > 4)
			return -1;
	}

	if (pai_list1[pai_type_blackjack] > 1 || pai_list2[pai_type_blackjack] > 1 || pai_list2[pai_type_blackjack] > 1 || 
		pai_list1[pai_type_blossom] > 1 || pai_list2[pai_type_blossom] > 1 || pai_list2[pai_type_blossom] > 1 )
		return -1;

	if (landlord > 2 || outPaiIndex > 2 || outPaiIndex == 0)
		return -1;

	if (!limit_card)
		return -2;

	C2S_Passive_Play_Card c2s_passPlayCard;
	c2s_passPlayCard.m_uiAiHandle = handle;
	for(int i = 0; i < pai_type_max; ++i)
	{
		c2s_passPlayCard.m_vPaiList1[i] = pai_list1[i];
		c2s_passPlayCard.m_vPaiList2[i] = pai_list2[i];
		c2s_passPlayCard.m_vPaiList3[i] = pai_list3[i];
	}
	c2s_passPlayCard.m_ucLandlordIndex = landlord;
	c2s_passPlayCard.m_ucOutPaiIndex = outPaiIndex;
	c2s_passPlayCard.m_oLimitPai._type = limit_card->_type;
	c2s_passPlayCard.m_oLimitPai._alone_1 = limit_card->_alone_1;
	c2s_passPlayCard.m_oLimitPai._alone_2 = limit_card->_alone_2;
	c2s_passPlayCard.m_oLimitPai._alone_3 = limit_card->_alone_3;
	c2s_passPlayCard.m_oLimitPai._alone_4 = limit_card->_alone_4;
	c2s_passPlayCard.m_oLimitPai._airplane_pairs = limit_card->_airplane_pairs;
	c2s_passPlayCard.m_oLimitPai._combo_count = limit_card->_combo_count;
	memcpy(c2s_passPlayCard.m_oLimitPai._combo_list, limit_card->_combo_list, 20*sizeof(unsigned char));

	m_oCommunication2.SendMsgByUID(&c2s_passPlayCard, Cst_DBServer, m_iServerID, handle);

	return 0;
}

/*---------------------------------------------------------------------------------------------------------*/
struct help_grab_landlord
{
	int need_round;
	int sum_value;
};

static void calc_handcard_value(std::vector<mcts_move*>& move_list, help_grab_landlord& help_value)
{
	help_value.need_round = move_list.size();
	help_value.sum_value = 0;

	int three_count = 0;
	int airplane_count = 0;
	int alone_1_count = 0;
	int pairs_count = 0;
	for(int i = 0; i < (int)move_list.size(); ++i)
	{
		ddz_move* move = dynamic_cast<ddz_move*>(move_list[i]);
		help_value.sum_value += move->_type;
		if (move->_type == ddz_value_order)
			help_value.sum_value += (move->_combo_count - 5);
		if (move->_type == ddz_value_order_pairs)
			help_value.sum_value += (move->_combo_count / 2 - 3)*2;
		if (move->_type == ddz_value_order_airplane)
		{
			help_value.sum_value += (move->_combo_count / 3 - 2)*3;
			airplane_count += move->_combo_count / 3;
		}
		if (move->_type == ddz_value_three)
			three_count++;
		if (move->_type == ddz_value_alone_1)
			alone_1_count++;
		if (move->_type == ddz_value_pairs)
			pairs_count++;
	}

	if (three_count != 0)
	{
		int sub_value = three_count;
		if ((alone_1_count+pairs_count) < three_count)
			sub_value = alone_1_count+pairs_count;
		help_value.need_round -= sub_value;
	}

	if (airplane_count >= 2)
	{
		int sub_value = airplane_count;
		if (alone_1_count < sub_value && pairs_count == 0)
			sub_value = 0;
		if (alone_1_count == 0 && pairs_count < airplane_count)
			sub_value = 0;
		if (alone_1_count + pairs_count*2 < airplane_count)
			sub_value = 0;

		help_value.need_round -= sub_value;
	}
}

int ddz_ai_interface::GrabLandlord(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char landlord, unsigned char grab_pai[3])
{
	bool can_grab = false;

	stgy_split_card _stgy_split_card;
	help_grab_landlord value[3];
	memset((void*)&value[0], 0, sizeof(help_grab_landlord)*3);

	int pai_list[pai_type_max];

	memset(pai_list, 0, sizeof(int)*pai_type_max);
	for(int i = pai_type_3; i < pai_type_max; ++i)
	{
		pai_list[i] = pai_list1[i];
	}
	for(int i = 0; i < 3; ++i)
	{
		if (grab_pai[i] >= pai_type_3 && grab_pai[i] < pai_type_max)
		{
			pai_list[grab_pai[i]] += 1;
		}
	}
	std::vector<mcts_move*> player_move1;
	_stgy_split_card.generate_out_pai_move(pai_list, player_move1);
	calc_handcard_value(player_move1, value[0]);

	memset(pai_list, 0, sizeof(int)*pai_type_max);
	for(int i = pai_type_3; i < pai_type_max; ++i)
	{
		pai_list[i] = pai_list2[i];
	}
	for(int i = 0; i < 3; ++i)
	{
		if (grab_pai[i] >= pai_type_3 && grab_pai[i] < pai_type_max)
		{
			pai_list[grab_pai[i]] += 1;
		}
	}
	std::vector<mcts_move*> player_move2;
	_stgy_split_card.generate_out_pai_move(pai_list, player_move2);
	calc_handcard_value(player_move2, value[1]);

	memset(pai_list, 0, sizeof(int)*pai_type_max);
	for(int i = pai_type_3; i < pai_type_max; ++i)
	{
		pai_list[i] = pai_list3[i];
	}
	for(int i = 0; i < 3; ++i)
	{
		if (grab_pai[i] >= pai_type_3 && grab_pai[i] < pai_type_max)
		{
			pai_list[grab_pai[i]] += 1;
		}
	}
	std::vector<mcts_move*> player_move3;
	_stgy_split_card.generate_out_pai_move(pai_list, player_move3);
	calc_handcard_value(player_move3, value[2]);

	for(int i = 0; i < player_move1.size(); ++i)
		delete player_move1[i];

	for(int i = 0; i < player_move2.size(); ++i)
		delete player_move2[i];

	for(int i = 0; i < player_move3.size(); ++i)
		delete player_move3[i];

	int grab_score = 0;

	if (value[0].need_round <= value[1].need_round && value[0].need_round <= value[2].need_round &&
		value[0].sum_value > value[1].sum_value && value[0].sum_value > value[2].sum_value)
		grab_score = 3;

	memset(pai_list, 0, sizeof(int)*pai_type_max);
	for(int i = pai_type_3; i < pai_type_max; ++i)
	{
		pai_list[i] = pai_list1[i];
	}
	for(int i = 0; i < 3; ++i)
	{
		if (grab_pai[i] >= pai_type_3 && grab_pai[i] < pai_type_max)
		{
			pai_list[grab_pai[i]] += 1;
		}
	}

	if (grab_score == 0)
	{
		if (pai_list[pai_type_blackjack] > 0 && pai_list[pai_type_blossom] > 0 && pai_list[pai_type_2] > 1)
			grab_score = 3;
		else if (pai_list[pai_type_blackjack] > 0 && pai_list[pai_type_blossom] > 0 && pai_list[pai_type_2] == 1)
		{
			if (pai_list[pai_type_A] > 1)
				grab_score = 3;
			else
				grab_score = 2;
		}

		if (grab_score == 0)
		{
			if (pai_list[pai_type_blossom] > 0 && pai_list[pai_type_2] > 2)
				grab_score = 3;
			else if (pai_list[pai_type_blossom] > 0 && pai_list[pai_type_2] == 2)
			{
				if (pai_list[pai_type_A] > 1)
					grab_score = 3;
				else
					grab_score = 2;
			}
		}

		if (grab_score == 0)
		{
			if (pai_list[pai_type_blackjack] > 0 && pai_list[pai_type_2] > 2)
				grab_score = 3;
			else if (pai_list[pai_type_blackjack] > 0 && pai_list[pai_type_2] == 2 && pai_list[pai_type_A] > 2)
				grab_score = 3;
		}
	}


	return grab_score;
}