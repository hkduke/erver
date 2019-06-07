#ifndef __DDZ_AI_INTERFACE_H__
#define __DDZ_AI_INTERFACE_H__

#include "Communication2.h"
#include "ICommand.h"
#include <string>

enum pai_interface_enum
{
	pai_i_type_none					= 0,
	pai_i_type_3					= 3,
	pai_i_type_4					= 4,
	pai_i_type_5					= 5,
	pai_i_type_6					= 6,
	pai_i_type_7					= 7,
	pai_i_type_8					= 8,
	pai_i_type_9					= 9,
	pai_i_type_10					= 10,
	pai_i_type_J					= 11,
	pai_i_type_Q					= 12,
	pai_i_type_K					= 13,
	pai_i_type_A					= 14,
	pai_i_type_2					= 15,
	pai_i_type_blackjack			= 16,			// Ð¡Íõ
	pai_i_type_blossom				= 17,			// ´óÍõ
	pai_i_type_max					= 18,

	pai_event_play_card				= 1,
	pai_event_play_card_passive		= 2,
	pai_event_grab_landlord			= 3,
};

enum move_interface_enum
{
	ddz_type_i_no_move			= 0,			// 不出牌
	ddz_type_i_alone_1			= 1,			// 单张
	ddz_type_i_pair				= 2,			// 连对
	ddz_type_i_triple			= 3,			// 三张
	ddz_type_i_triple_1			= 4,			// 三带一
	ddz_type_i_triple_2			= 5,			// 三带二
	ddz_type_i_order			= 6,			// 顺子
	ddz_type_i_order_pair		= 7,			// 连对
	ddz_type_i_airplane			= 8,			// 飞机
	ddz_type_i_airplane_with_pai= 9,			// 飞机带牌
	ddz_type_i_bomb				= 10,			// 炸弹
	ddz_type_i_king_bomb		= 11,			// 王炸
	ddz_type_i_four_with_alone1 = 12,			// 4with2 二张单牌
	ddz_type_i_four_with_pairs  = 13,			// 4with2 二对对子
};


struct pai_interface_move
{
	unsigned char				_type;
	unsigned char				_alone_1;
	unsigned char				_alone_2;
	unsigned char				_alone_3;
	unsigned char				_alone_4;
	unsigned char				_airplane_pairs;

	unsigned char				_combo_list[20];
	unsigned char				_combo_count;

	pai_interface_move()
	{

	}

	~pai_interface_move()
	{

	}
};

typedef void (*PlayCard_Func)(int event, int handle, const pai_interface_move* move);

class ddz_ai_interface : public ICommunicationCommand
{
public:
	unsigned int					CreateAi();
	int 							AiPlayCard(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char landlord );
	int								AiPlayCardPassive(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char landlord, unsigned char outPaiIndex, const pai_interface_move* limit_card);
	int								GrabLandlord(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char landlord, unsigned char grab_pai[3]);
	int								GrabLandlord2(unsigned int handle, unsigned char pai_list1[pai_i_type_max], unsigned char pai_list2[pai_i_type_max], unsigned char pai_list3[pai_i_type_max], unsigned char grab_pai[3]);

	void							SetPlayCardInterface(PlayCard_Func func);

	void							Update();
	int								Initialize( const char* szCfg);

public:
	virtual int Execute( SwitchMsgHead * header, void* pMsg, short nMsgLength, int iConnection );
	virtual void Notify(){};

	ddz_ai_interface();
	virtual ~ddz_ai_interface();

private:
	Communication2					m_oCommunication2;
	bool							m_bCommunicationInit;

	unsigned int					m_uiThreadMissionId;
	int								m_iServerID;

	PlayCard_Func					m_pfFunc;

};




#endif