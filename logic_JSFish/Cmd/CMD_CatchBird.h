#ifndef CMD_CATCH_BIRD_HEAD_FILE
#define CMD_CATCH_BIRD_HEAD_FILE

#include "../Code/Prereqs.h"
#include "CGlobalData.h"
#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

//////////////////////////////////////////////////////////////////////////
#define SCORE long long
//#define KIND_ID						603									    //��Ϸ I D
#define GAME_PLAYER					4									    //��Ϸ����
//
//#define VERSION_SERVER				PROCESS_VERSION(6,0,3)					//����汾
//#define VERSION_CLIENT				PROCESS_VERSION(6,0,3)					//����汾
//
//#define GAME_NAME					TEXT("���ٲ���")						//��Ϸ����

//////////////////////////////////////////////////////////////6/////////////
#define IDI_GAME_LOOP                 1
#define TIME_GAME_LOOP                200//100

#define IDI_SAVE_JSFISH_STOCK	2
#define TIME_SAVE_JSFISH_STOCK	30 * 1000

#define IDI_SAVE_JSFISH_SYSINFO 3
#define TIME_SAVE_JSFISH_SYSINFO 5 * 60 * 1000

#define IDI_SAVE_TODAY_INFO 4
#define TIME_SAVE_TODAY_INFO 5 * 60 * 1000

//#define INVALID_CHAIR 				  0xFFFF
#define MAX_PATH 					  128
#define JIESUAN_TIME				  10000

const int kFPS = 50;				//֡��
const float kSpeed = 2.f / kFPS;	//�ٶ�

const int kScreenWidth = 1334;
const int kScreenHeight = 750;

const int kMaxRangeCount = 10;		//����������

//1420*800
const int CANNON_POSITION[][2] = {
	{ 537, 78 },{ 830, 78 },
	{ 537,652 },{ 830, 652 }
};
const int CANNON_POSITION_1[][2] = {
	{ 233, 63 },{ 935, 63 },
	{ 233, 702 },{ 935, 702 }
};
//////////////////////////////////////////////////////////////////////////
#define MAX_BIRD_IN_MANAGER              2048
#define MAX_BULLET_IN_MANAGER            1024
#define SCENE_TIME_END                   600
#define MAX_SMALL_PATH				     208
#define MAX_BIG_PATH					 130
#define MAX_HUGE_PATH					 62
#define MAX_SPECIAL_PATH				 24
#define MAX_SCENE_PATH                    4
#define MAX_SCENE_SHAPE                   6
#define MAX_BIRD_CATCH                    6
#define MAX_BIRD_CATCH_CHAIN			  6
#define MAX_DIFFERENCE                    2000
#define MAX_BULLET_COUNT				  40

#define SCENE_TYPE_0                      0
#define SCENE_TYPE_1                      1
#define SCENE_TYPE_2                      2
#define SCENE_TYPE_3                      3
#define SCENE_TYPE_4                      4
#define SCENE_TYPE_NULL                   -1
#define MAX_SCENE_TYPE                    5

#define BIRD_TYPE_NULL                   -1
#define BIRD_TYPE_0                      0  //С����
#define BIRD_TYPE_1                      1  //С����
#define BIRD_TYPE_2                      2  //�ȴ���
#define BIRD_TYPE_3                      3  //������
#define BIRD_TYPE_4                      4  //�ʺ���
#define BIRD_TYPE_5                      5  //С����
#define BIRD_TYPE_6                      6  //������
#define BIRD_TYPE_7                      7  //������
#define BIRD_TYPE_8                      8  //������
#define BIRD_TYPE_9                      9  //�����
#define BIRD_TYPE_10                     10 //������
#define BIRD_TYPE_11                     11 //����
#define BIRD_TYPE_12                     12 //������
#define BIRD_TYPE_13                     13 //��β��
#define BIRD_TYPE_14                     14 //����
#define BIRD_TYPE_15                     15 //ħ����
#define BIRD_TYPE_16                     16 //����
#define BIRD_TYPE_17                     17 //˫ͷ��
#define BIRD_TYPE_18                     18 //��������(�𹿰�)
#define BIRD_TYPE_19                     19 //���
#define BIRD_TYPE_20                     20 //������
#define BIRD_TYPE_21                     21 //������
#define BIRD_TYPE_22                     22 //���
#define BIRD_TYPE_23                     23 //����
#define BIRD_TYPE_24                     24 //����
#define BIRD_TYPE_25                     25 //�����
#define BIRD_TYPE_26                     26 //����
#define BIRD_TYPE_27                     27 //������̫��
#define BIRD_TYPE_28                     28 //ը��
#define BIRD_TYPE_29                     29 //BOSS
#define MAX_BIRD_TYPE                    30

#define BIRD_PAUSE						 BIRD_TYPE_18	//����ը��
#define BOMB_SMALL						 MAX_BIRD_TYPE	//С��ը��
#define BOMB_LARGE						 MAX_BIRD_TYPE	//����ը��
#define BOMB_ULTIMATELY					 MAX_BIRD_28	//ȫ��ը��
#define BOSS_FISH						 BIRD_TYPE_29	//BOSS

//������
#define BIRD_TYPE_CHAIN					 40		//������
#define BIRD_TYPE_RED					 41		//����
#define BIRD_TYPE_INGOT					 42		//Ԫ����

//һ�����
#define BIRD_TYPE_ONE					 50
#define BIRD_TYPE_TWO					 51
#define BIRD_TYPE_THREE					 52
#define BIRD_TYPE_FOUR					 53
#define BIRD_TYPE_FIVE					 54

//С��ʹ������ֱ�ʶ
#define SMALL_BIRD_FLAG                  BIRD_TYPE_18

#define ITEM_TYPE_NULL                   -1

#define CANNON_TYPE_NULL                   -1
#define CANNON_TYPE_0                      0
#define CANNON_TYPE_1                      1
#define CANNON_TYPE_2                      2
#define CANNON_TYPE_3                      3
#define CANNON_TYPE_4                      4
#define CANNON_TYPE_5                      5
#define CANNON_TYPE_6                      6
#define CANNON_TYPE_7                      7
#define CANNON_TYPE_8                      8
#define CANNON_TYPE_9                      9
#define MAX_CANNON_TYPE                    10

#define PATH_TYPE_SMALL                    0
#define PATH_TYPE_BIG                      1
#define PATH_TYPE_HUGE                     2
#define PATH_TYPE_SPECIAL                  3
#define PATH_TYPE_SCENE                    4

#define MAX_BOMB                           3
#define ENERGY_CANNON_CREATE               12

// ��Ч������
//////////////////////////////////////////////////////////////////////////
#define BIRD_ITEM_NULL						0
#define BIRD_ITEM_SPECIAL_BOMB_1			1
#define BIRD_ITEM_SPECIAL_BOMB_2			2
#define BIRD_ITEM_SPECIAL_BOMB_3			3
#define BIRD_ITEM_SPECIAL_DING				4
#define BIRD_ITEM_SPECIAL_LUCKY				5
#define BIRD_ITEM_SPECIAL_GOLDX2			6
#define BIRD_ITEM_SPECIAL_TOR				7	//��Ч
#define BIRD_ITEM_SPECIAL_EEL				8   //��Ч
#define BIRD_ITEM_SPECIAL_SMALL_PURSE		9
#define BIRD_ITEM_SPECIAL_MIDDLE_PURSE		10
#define BIRD_ITEM_SPECIAL_BIG_PURSE			11

//�ӵ�����
#define BULLET_KIND_COUNT					10
//�����õ��������������
#define	TASK_BIRD_COUNT						10

//////////////////////////////////////////////////////////////////////////
// ����Ч
//////////////////////////////////////////////////////////////////////////
#define BIRD_EFFECT_NULL					0
#define BIRD_EFFECT_GOLDX2					1
#define BIRD_EFFECT_LUCKY					2
#define BIRD_EFFECT_DING					3
#define BIRD_EFFECT_EEL						4
#define BIRD_EFFECT_TOR						5

//��������
#define YUZHEN_NORMAL						0
#define YUZHEN_LINEAR						1
#define YUZHEN_ROUND						2
#define YUZHEN_PAUSE_LINEAR					3


//////////////////////////////////////////////////////////////////////////
//*****************�������Ϣ

#define SUB_S_CATCH_BIRD_GROUP              113		//ץס��

//////////////////////////////////////////////////////////////////////////
//�ӵ�����
#define BULLET_PENETRATE		0x01		//��͸
#define BULLET_DOUBLE			0x02		//˫��
#define BULLET_FURY				0x04		//��

//������
struct CMD_C_Catch_Fish: public MSG_GameMsgUpHead
{
	uint16_t 					chair_id;			//����ID
	uint32_t 					fish_id_;			//��ID
	uint8_t 					bullet_kind;		//�ӵ�����
	uint32_t 					bullet_multiple;	//�ӵ�����
	uint32_t  					bullet_id;
};
struct Role_Net_Object
{
	uint16_t chair_id_;
	double catch_gold_;
	uint16_t cannon_type_;
	double cannon_mulriple_;
};

//�ӵ�����
struct BulletConfig{
    double speed;				//�ٶ�
    double fire_interval;		//���ڼ��
};

//////////////////////////////////////////////////////////////////////////
struct CMD_S_StatusFree
{
	uint8_t scene_;										//����
    double cannon_mulriple_[MAX_CANNON_TYPE];
	uint32_t mulriple_count_;
	uint16_t scene_special_time_;						//
	Role_Net_Object role_objects_[GAME_PLAYER];			//�����Ϣ
	BulletConfig bullet_config_[BULLET_KIND_COUNT];		//�ӵ�����
};

struct CMD_S_TimeCheck
{
	uint16_t chair_id_;
	uint32_t client_time_;
	uint32_t server_time_;
};

struct CMD_C_TimeCheck
{
	uint16_t chair_id_;
	uint32_t time_;
	bool is_game_status_;
};

struct CMD_S_OrderTimeCheck
{
	uint16_t chair_id_;
};

struct CMD_S_Send_Bird
{
	uint32_t id_;
	uint8_t type_;
	uint8_t item_;			//����Ч������������� ���� �������������
	uint16_t path_id_;
	uint8_t path_type_;
	double path_delay_;
	Point path_offset_;
    double elapsed_;
    double speed_;

	uint32_t time_;
};

struct CMD_S_Send_Bird_Linear
{
	uint32_t id_;
	uint8_t type_;
	uint8_t item_;
    double path_delay_;
    double elapsed_;
    double speed_;

	Point start_;
	Point end_;

	uint32_t time_;
};

struct CMD_S_Send_Bird_Pause_Linear
{
	uint32_t id_;
	uint8_t type_;
	uint8_t item_;
    double path_delay_;
    double elapsed_;
    double speed_;
    double start_angle_;

	Point start_;
	Point pause_;
	Point end_;
    double pause_time_;

	uint32_t time_;
};

struct CMD_S_Send_Bird_Round
{
	uint32_t id_;
	uint8_t type_;
	uint8_t item_;
    double path_delay_;
    double elapsed_;
    double speed_;

	Point center_;
    double radius_;
    double rotate_duration_;
    double start_angle_;
    double rotate_angle_;
    double move_duration_;

	uint32_t time_;
};



struct CMD_C_Fire:public MSG_GameMsgUpHead
{
	uint16_t chair_id_;
    double rote_;				//< ����, �Ƕ�
	double bullet_mulriple_;	//< �ӵ�����
	int lock_bird_id_;			//������id
	uint8_t bullet_type_;		//< �ӵ�����  ��͸ ˫��  ��
	uint32_t  bullet_id; 		//�ӵ�ID
	uint32_t  bullet_idTwo;		//�ڶ����ڹܵ��ӵ�id

	CMD_C_Fire()
	{
		memset(this, 0, sizeof(CMD_C_Fire));
	}
};

struct CMD_S_Fire
{
	uint16_t chair_id_;
    double rote_;				//< ����, �Ƕ�
	uint32_t bullet_mulriple_;	//< �ӵ�����
	int lock_bird_id_;			//������id
	uint8_t bullet_type_;		//< �ӵ�����  ��͸ ˫��  ��
	SCORE cur_gold_;
};

struct CMD_S_Send_Bullet
{
    double rotation_;					//�Ƕ�
	uint16_t chair_id_;					//��λ��
	double bullet_mulriple_;			//�ӵ�����
	int lock_bird_id_;					//������ID
	double cur_gold_;					//��ǰ���
	uint8_t bullet_kind;				//�ӵ�����
	uint32_t bullet_id;                 //�ӵ�ID
};

struct CMD_S_Fire_Failed
{
	uint16_t chair_id_;
    double nowGlod;
};

struct CMD_S_Catch_Bird
{
	uint16_t chair_id_;
	uint16_t cannon_type_;
	uint32_t catch_gold_;
	uint32_t bird_id_;
	uint16_t award_type_;
	uint16_t awarad_count_;
	uint32_t medal_;
};

struct Catch_Bird
{
	uint16_t chair_id_;
    double catch_gold_;		//����
	uint32_t bird_id_;			//��ǰ��id
    double now_money;			//��ǰ�����
	bool isDouble;				//�Ƿ񱩻�
	uint8_t  is_die;			//�Ƿ�����	
};
//�����������ʹ�õ�һ����δ������Ϣ
struct CMD_S_Catch_Bird_Group
{
	uint8_t count_; //����
	uint8_t chair_id_;

	Catch_Bird catch_bird_[MAX_BIRD_CATCH];
	uint32_t bullet_id_;
};








struct CMD_S_Change_Scene
{
	uint8_t scene_;
	uint16_t special_time_;   //��������ʱ��
};










//��ѯ
struct CMD_C_Admin_Query
{
	uint8_t operate_;
};

//<��������
struct CMD_CS_Admin_Basic
{
	int exchange_ratio_userscore_;					//�һ�����
	int exchange_ratio_birdscore_;
	int exchange_count_;							//ÿ�ζһ�����
	float scene_swith_time_;						//�����л�ʱ��
	uint64_t bullet_valid_count_;					//��Ч�ӵ�����
	float total_return_rate_;						//���巵���ٷֱ�
	int64_t game_revenue_;							//��˰��ֵ
	int level_difficulty_;							//��ǰ�Ѷ�
	int change_level_interval_;						//�Ѷ��л�ʱ��
	uint32_t cannon_mulriple_[MAX_CANNON_TYPE];		//���ڱ���
	uint32_t mulriple_count_;						//���ʸ���
	BulletConfig bullet_config_[BULLET_KIND_COUNT]; //�ӵ�����
};




//<������
//��ͨ������
struct BirdConfig {
	int mulriple_min;								//��С����
	int mulriple_extend[4];							//��չ����
	int mulriple_max;								//�����
	int over_zero_prob;								//����>0��������
	int under_zero_prob;							//����<0��������
	Size bird_size;									//��С
	float speed;									//�ٶ�
	double capture_probability;						//�������
	int distribute_interval_min;					//��С���ɼ��
	int distribute_interval_max;					//������ɼ��
	int distribute_count_min;						//��С��������
	int distribute_count_max;						//�����������
	int path_type;									//·������
	char fish_name[20];							//������
};

struct SendBird
{
    uint64_t id_;
    uint8_t  type_;
    uint8_t  item_;
    uint16_t path_id_;
    uint8_t  path_type_;
    double    path_delay;
    double    path_offset_x_;
    double    path_offset_y_;
    int      elapsed_;
    double    speed_;
    int64_t  time_;
};

struct S_Enter_Android_info
{
	double score;
	unsigned  int chair_id;
};

struct S_ChangeScene
{
    uint8_t scene_;
    double   scene_special_time_;
};

struct chair_info
{
	uint32_t user_id;
	int64_t  user_enter_score;
	int64_t  user_current_score;
	//���ʵ����Ӯ
	int64_t  user_win_lose_score;
	int64_t  user_miss_stock;
	//�����ˮ
	int64_t  user_stream;
	//����ṩ��˰��
	int64_t  user_tax;
    //paoshu
    uint32_t user_send_pao_count;
    time_t   user_last_fire_time;
	chair_info()
	{
		user_id = 0;
		user_enter_score = 0;
		user_current_score = 0;
		user_win_lose_score = 0;
		user_miss_stock = 0;
		user_stream = 0;
        user_tax = 0;
        user_send_pao_count = 0;
		user_last_fire_time = 0;
	}
};

//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif