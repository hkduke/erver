#ifndef GAME_CONFIG_XML_H_
#define GAME_CONFIG_XML_H_

#include "Define.h"
#include "BYStockConfig.h"


class Game_Config_Xml
{
public:
	Game_Config_Xml(void);
	~Game_Config_Xml(void);

	//��ȡ����
	bool load_game_config();

	float get_bullet_radius() const { return bullet_radius_; }
	int get_bomb_radius(int bird_type) const { return bomb_radius_[bird_type]; }

//private:
	//unsigned short server_id_;								//������id

public:
	int level;										//���������εȼ�
	float scene_swith_time_;						//�����л�ʱ��
	int fish_multiple_control[4];					//������������
	char m_horn[2][255];							//����
	char m_birdName[10][10];
	int hornBirdNumber;
	BirdConfig bird_config_[MAX_BIRD_TYPE];
	BirdSameConfig bird_same_config_[BIRD_TYPE_FIVE - BIRD_TYPE_ONE + 1];
	BirdSpecialConfig bird_special_config_;
	BirdGroupConfig bird_group_config_;
	BirdRedSeriesConfig bird_red_series_config_;
	BirdRedBloatingConfig bird_red_bloating_config_;
	BirdChainConfig bird_chain_config_;
	BirdIngotConfig bird_ingot_config_;
	BulletConfig bullet_config_[BULLET_KIND_COUNT];
	AddFishRefreshConfig add_fish_refresh_config_[MAX_BIRD_TYPE];	//����ˢ�µ���
	//���ڱ���
	uint32_t cannon_mulriple_[MAX_CANNON_TYPE];
	uint32_t mulriple_count_;

	/*StockInfo m_low_stockInfo;
	StockInfo m_middle_stockInfo;
	StockInfo m_high_stockInfo;*/

	float bullet_radius_;	//�ӵ��뾶
	int bomb_radius_[MAX_BIRD_TYPE];	//�뾶

	//bool is_open_task_;									//�Ƿ�������
	//TaskConfig task_personal_config_;					//������������
	//TaskConfig task_everyone_config_;					//ȫ����������
	//TaskDragonConfig task_dragon_config_;				//��������

private:
	static bool load_success_;
};

extern Game_Config_Xml g_game_config_xml;


#endif //GAME_CONFIG_XML_H_
