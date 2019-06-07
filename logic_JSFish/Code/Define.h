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

//时间管理
struct  Timer_Control
{
	float scene_elapse_;						//场景时间
	float change_difficulty_elapse_;					//难度变换时间
	float bird_move_pause_elapse_;				//定屏时间
	float energy_countdown_[GAME_PLAYER];		//玩家能量炮倒计时（未实现）
	float distribute_elapsed_[MAX_BIRD_TYPE];	//生成鱼的间隔
	float group_bird_elapse_;					//小鱼群时间
	float special_bird_elapse_;					//交叉鱼阵
	float bird_red_series_elapse_;				//红鱼
	float bird_red_bloating_elapse_;			//鼓出红鱼
	float bird_chain_elapse_;					//闪电鱼
	float bird_same_elapse_[5];					//一箭多雕
	float bird_ingot_elapse_;					//元宝鱼

	//任务
	float task_personal_elapse_[GAME_PLAYER];	//个人任务间隔
	float task_personal_time_[GAME_PLAYER];		//个人任务倒计时
	float task_everyone_elapse_;				//全场任务间隔
	float task_dragon_elapse_;					//龙王任务间隔
};

//机器人
struct Android_Working
{
	float fire_elasped_;							//开炮间隔
	int change_bullet_mulriple_;					//开多少炮后改变倍率
	float change_mulriple_time_;					//改变倍率时间（改变倍率时停止开炮）
	float rest_time_;								//机器人休息时间
	int rest_bullet_num_;
	int change_bullet_rote_;
	float bullet_rote_;
};

//多雕鱼配置
struct BirdSameConfig {
	int bird_type;						//类型
	Size bird_size;						//大小
	int bird_count;						//鱼个数
	int bird_type_max;					//最大鱼类型
	float mulriple;						//倍率
	double capture_probability[10];		//打中概率
	float speed;						//速度
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};

//特殊鱼配置 (交叉鱼群)
struct BirdSpecialConfig {
	int distribute_interval_min;
	int distribute_interval_max;
	int path_type;
};

//鱼组配置 (小鱼群)
struct BirdGroupConfig {
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};
//补充刷新鱼的配置
struct AddFishRefreshConfig
{
	int refresh_probability;				//概率
	int refresh_interval;					//刷新间隔
};
//连串红鱼
struct BirdRedSeriesConfig
{
	int bird_type_max;					//最大10
	double capture_probability[10];
	int red_probability;				//red_probability红鱼出现概率(百分比)
	float speed;
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};

//鼓出红鱼
struct BirdRedBloatingConfig
{
	int bird_type_max;					//最大10
	double capture_probability[10];
	int red_probability;				//red_probability红鱼出现概率(百分比)
	int distribute_interval_min;
	int distribute_interval_max;
	int ring_count_min;
	int ring_count_max;	
};

//闪电鱼
struct BirdChainConfig{
	int bird_type_max;					//最大10
	double capture_probability[10];
	int distribute_interval_min;
	int distribute_interval_max;
	int distribute_count_min;
	int distribute_count_max;
	int path_type;
};

//元宝鱼
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
//赠送鱼


#endif // _DEFINE_H_