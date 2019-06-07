#ifndef __GAME_CONFIG_H__
#define __GAME_CONFIG_H__

///////////////////////////////////////////////////////////////////////////////////////
class Game_Table ;

///////////////////////////////////////////////////////////////////////////////////////
class Game_Config
{
public:
	Game_Config(Game_Table *table);
	~Game_Config();

	void initialise();
    void cleanup();

	void load_script();

	float get_bullet_radius() const { return bullet_radius_; }
	float get_net_radius(uint16_t cannon) const { return net_radius_[cannon]; }
	Size get_bird_size(uint8_t bird_type) const { return bird_size_[bird_type]; }
	int get_bird_price(uint8_t bird_type, uint8_t energy_cannon) const;
	int get_bullet_price(uint16_t cannon_type) const  { return bullet_price_[cannon_type]; }

	int get_bomb_radius(int bird_type) const { return bomb_radius_[bird_type]; }

	int64_t get_max_win_score() const { return max_win_score_; }
	int64_t get_max_lost_score() const { return max_lost_score_; }
	int64_t get_min_win_score() const { return min_win_score_; }
	int64_t get_min_lost_score() const { return min_lost_score_; }

	int get_probability(int cannon_type, int bird_type) ;
	int get_energy_cannon_probability()  { return energy_cannon_probability_; }

	int64_t get_table_chi_fen_threshold() { return table_chi_fen_threshold_; }

public:
	float bullet_radius_;
	//float net_radius_[MAX_CANNON_TYPE];
	float net_radius_[40];

	int bomb_radius_[MAX_BIRD_TYPE];

	int bird_price_[MAX_BIRD_TYPE];
	Size bird_size_[MAX_BIRD_TYPE];
	int bird_probability_[MAX_BIRD_TYPE];

	//int bullet_price_[MAX_CANNON_TYPE];
	int bullet_price_[40];
	//int bullet_probability_[MAX_CANNON_TYPE];
	int bullet_probability_[40];
	int energy_cannon_probability_;

	int64_t max_win_score_;                 //������Ӯ������
	int64_t max_lost_score_;				//������Ӯ������
	int64_t min_win_score_ ;                
	int64_t min_lost_score_ ; 

	//���ӳԷַ�ֵ
    uint32_t chi_fen_zhou_qi_;                
    int64_t table_chi_fen_threshold_;

	int64_t storage_start_;

    float less_lost_probability_  ;
    float less_win_probability_; 
    float more_lost_probability_; 
    float more_win_probability_	;	

	bool da_wu_gui_;
    

    Game_Table *table_;
}; 

///////////////////////////////////////////////////////////////////////////////////////

#endif