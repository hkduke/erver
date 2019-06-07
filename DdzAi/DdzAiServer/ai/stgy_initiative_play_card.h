#ifndef __STGY_INITIATIVE_PLAY_CARD_H__
#define __STGY_INITIATIVE_PLAY_CARD_H__

#include "mcts_move.h"
#include <vector>

class ddz_state;
class ddz_move;

class stgy_initiative_play_card
{
public:
	stgy_initiative_play_card();
	~stgy_initiative_play_card();

	std::vector<mcts_move*>				parse_pai(ddz_state* p_state);

private:

	std::vector<mcts_move*>				landlord_initiative_playcard(ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* prev_pai_list, std::vector<mcts_move*>& prev_move_list);
	std::vector<mcts_move*>				farmer_initiative_playcard_next_is_landlord(ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* prev_pai_list, std::vector<mcts_move*>& prev_move_list);
	std::vector<mcts_move*>				farmer_initiative_playcard_next_is_farmer(ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* prev_pai_list, std::vector<mcts_move*>& prev_move_list);

	// �����һ�����ȳ�����ƣ����ֻʣ����������ʱ�����ȳ�������
	void								common_max_initiative_playcard(std::vector<mcts_move*>& out_list, ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, int* next_pai_list, int* prev_pai_list, std::vector<mcts_move*>* next_move_list, std::vector<mcts_move*>* prev_move_list);

	// ����Է�ũ��ֻ��һ���ƣ����ȳ��Է�ũ���ܹ�����
	void								if_friend_only_1_move(std::vector<mcts_move*>& out_list, ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, std::vector<mcts_move*>& friend_move_list);
	void								is_friend_can_control_pai(std::vector<mcts_move*>& out_list, ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, std::vector<mcts_move*>& friend_move_list, int* landlord_pai);
	void								is_self_can_control_pai(std::vector<mcts_move*>& out_list, ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, int* landlord_pai);

	// ����ֻʣ����,����û�д���,���Ʋ���
	void								landlord_no_maxpai_only_2_move(std::vector<mcts_move*>& out_list, std::vector<mcts_move*>& move_list);
	void								landlord_no_maxpai_only_3_move(std::vector<mcts_move*>& out_list, std::vector<mcts_move*>& move_list);

	void								farmer_initiative_playcard_next_farmer_only_1_move(std::vector<mcts_move*>& out_list, std::vector<mcts_move*>& move_list, int* pai_list, std::vector<mcts_move*>& next_move_list);
	void								farmer_initiative_playcard_next_landlord_only_1_move(std::vector<mcts_move*>& out_list, int* pai_list,  std::vector<mcts_move*>& self_list, std::vector<mcts_move*>& next_move_list);

	bool								decide_self_is_master(std::vector<mcts_move*>& self_move_list, std::vector<mcts_move*>& other_move_list);

	void								common_landlord_playcard(std::vector<mcts_move*>& out_list, std::vector<mcts_move*>& move_list);
	void								sendout_from_max_to_min_playcard(ddz_state* p_state, std::vector<mcts_move*>& out_list, std::vector<mcts_move*>& move_list, ddz_move* landlord_move);
};



#endif