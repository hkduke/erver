#ifndef _DEFINE_H_
#define _DEFINE_H_

#include "Point.h"
#include "../Cmd/CMD_CatchBird.h"


struct round_bird_linear
{
	uint8_t  bird_type;
	uint32_t bird_count;
	Point    pt_center;
	bool     is_red;
};

//ʱ�����
struct  Timer_Control
{
	float scene_elapse_;						//����ʱ��
	float change_difficulty_elapse_;					//�Ѷȱ任ʱ��
	float bird_move_pause_elapse_;				//����ʱ��
	float energy_countdown_[GAME_PLAYER];		//��������ڵ���ʱ��δʵ�֣�
	float distribute_elapsed_[MAX_BIRD_TYPE];	//������ļ��
	float group_bird_elapse_;					//С��Ⱥʱ��
	float special_bird_elapse_;					//��������
	float bird_red_series_elapse_;				//����
	float bird_red_bloating_elapse_;			//�ĳ�����
	float bird_chain_elapse_;					//������
	float bird_same_elapse_[5];					//һ�����
	float bird_ingot_elapse_;					//Ԫ����

	//����
	float task_personal_elapse_[GAME_PLAYER];	//����������
	float task_personal_time_[GAME_PLAYER];		//�������񵹼�ʱ
	float task_everyone_elapse_;				//ȫ��������
	float task_dragon_elapse_;					//����������
};

//������
struct Android_Working
{
	float fire_elasped_;							//���ڼ��
	int change_bullet_mulriple_;					//�������ں�ı䱶��
	float change_mulriple_time_;					//�ı䱶��ʱ�䣨�ı䱶��ʱֹͣ���ڣ�
	float rest_time_;								//��������Ϣʱ��
	int rest_bullet_num_;
	int change_bullet_rote_;
	float bullet_rote_;
};

//���������
struct BirdSameConfig {
	int bird_type;						//����
	Size bird_size;						//��С
	int bird_count;						//�����
	int bird_type_max;					//���������
	float mulriple;						//����
	double capture_probability[10];		//���и���
	float speed;						//�ٶ�
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};

//���������� (������Ⱥ)
struct BirdSpecialConfig {
	int distribute_interval_min;
	int distribute_interval_max;
	int path_type;
};

//�������� (С��Ⱥ)
struct BirdGroupConfig {
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};
//����ˢ���������
struct AddFishRefreshConfig
{
	int refresh_probability;				//����
	int refresh_interval;					//ˢ�¼��
};
//��������
struct BirdRedSeriesConfig
{
	int bird_type_max;					//���10
	double capture_probability[10];
	int red_probability;				//red_probability������ָ���(�ٷֱ�)
	float speed;
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};

//�ĳ�����
struct BirdRedBloatingConfig
{
	int bird_type_max;					//���10
	double capture_probability[10];
	int red_probability;				//red_probability������ָ���(�ٷֱ�)
	int distribute_interval_min;
	int distribute_interval_max;
	int ring_count_min;
	int ring_count_max;	
};

//������
struct BirdChainConfig{
	int bird_type_max;					//���10
	double capture_probability[10];
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};

//Ԫ����
struct BirdIngotConfig{
	int bird_type_max;
	double capture_probability[10];
	unsigned int reward[10];
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};

typedef std::map<unsigned int, SCORE> ListLimitMap;
typedef std::map<unsigned int, SCORE> UserWinScoreMap;
typedef std::map<unsigned int, uint8_t> DifficultyMap;
//������


#endif // _DEFINE_H_