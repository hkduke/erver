#ifndef __STGY_PASSIVE_PLAY_CARD_H__
#define __STGY_PASSIVE_PLAY_CARD_H__

#include "ddz_move.h"
#include <vector>

class ddz_state;

class stgy_passive_play_card
{
public:
	stgy_passive_play_card();
	~stgy_passive_play_card();

	std::vector<mcts_move*> parse_pai(ddz_state* p_state, ddz_move* limit_move);

private:
	std::vector<mcts_move*> landlord_passive_playcard_prev_outcard(ddz_state* p_state, ddz_move* limit_move, int* pai_list, std::vector<mcts_move*>& move_list,
		int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* pre_pai_list, std::vector<mcts_move*>& pre_move_list);
	std::vector<mcts_move*> landlord_passive_playcard_next_outcard(ddz_state* p_state, ddz_move* limit_move, int* pai_list, std::vector<mcts_move*>& move_list,
		int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* pre_pai_list, std::vector<mcts_move*>& pre_move_list);

	std::vector<mcts_move*> farmer_passive_playcard_next_landlord_and_next_outcard(ddz_state* p_state, ddz_move* limit_move, int* pai_list, std::vector<mcts_move*>& move_list,
		int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* pre_pai_list, std::vector<mcts_move*>& pre_move_list);
	std::vector<mcts_move*> farmer_passive_playcard_next_landlord_and_prev_outcard(ddz_state* p_state, ddz_move* limit_move, int* pai_list, std::vector<mcts_move*>& move_list,
		int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* pre_pai_list, std::vector<mcts_move*>& pre_move_list);
	std::vector<mcts_move*> farmer_passive_playcard_next_farmer_and_next_outcard(ddz_state* p_state, ddz_move* limit_move, int* pai_list, std::vector<mcts_move*>& move_list,
		int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* pre_pai_list, std::vector<mcts_move*>& pre_move_list);
	std::vector<mcts_move*> farmer_passive_playcard_next_farmer_and_prev_outcard(ddz_state* p_state, ddz_move* limit_move, int* pai_list, std::vector<mcts_move*>& move_list,
		int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* pre_pai_list, std::vector<mcts_move*>& pre_move_list);

	
	bool					have_greater_than_limit_move(int* pai_list, ddz_move* limit_move);

	std::vector<mcts_move*> farmer_passive_playcard(ddz_state* p_state, ddz_move* limit_move, bool out_is_friend, int* pai_list, std::vector<mcts_move*>& move_list,
		int* next_pai_list, std::vector<mcts_move*>& next_move_list, int* pre_pai_list, std::vector<mcts_move*>& pre_move_list);

	void					is_friend_can_control_pai_and_curr_friend_out_pai(std::vector<mcts_move*>& out_list, ddz_state* p_state, ddz_move* limit_pai, int* pai_list, std::vector<mcts_move*>& move_list, std::vector<mcts_move*>& friend_move_list, int* landlord_pai);
	void					is_self_can_control_pai(std::vector<mcts_move*>& out_list, ddz_state* p_state, int* pai_list, std::vector<mcts_move*>& move_list, int* landlord_pai, ddz_move* limit_move);
	void					if_next_landlord_only_1_move(std::vector<mcts_move*>& out_list, ddz_state* p_state, ddz_move* limit_pai, int* pai_list, std::vector<mcts_move*>& move_list, std::vector<mcts_move*>& landlord_move_list);
	void					is_next_friend_only_1_move(std::vector<mcts_move*>& out_list, ddz_state* p_state, ddz_move* limit_pai, int* pai_list, std::vector<mcts_move*>& move_list, std::vector<mcts_move*>& friend_move_list);
	void					is_next_friend_can_control_pai(std::vector<mcts_move*>& out_list, ddz_state* p_state, ddz_move* limit_move, std::vector<mcts_move*>& friend_move_list, int* landlord_pai);

	// �����һ�����ȳ�����ƣ����ֻʣ����������ʱ�����ȳ�������
	void					common_max_passive_playcard(std::vector<mcts_move*>& out_list, ddz_state* p_state, ddz_move* limit_pai, int* pai_list, std::vector<mcts_move*>& move_list, int* next_pai_list, int* prev_pai_list);

};


#endif