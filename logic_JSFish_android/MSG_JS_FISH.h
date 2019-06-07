#ifndef MSG_JS_FISH
#define MSG_JS_FISH

#include "CGlobalData.h"
#include "CTType.h"
#ifdef _WIN32
#pragma pack(push,1)
#else
#pragma pack(1)
#endif

enum message
{
    SUB_S_CHANGE_SCENE=100,				//�任����
    SUB_S_SEND_BIRD = 101,					//��������Ϣ
    SUB_S_SEND_BIRD_PAUSE_LINEAR = 102,       //��ͣ(102)
    SUB_S_SEND_BIRD_ROUND = 103,				//103
    SUB_S_SEND_BIRD_LINEAR = 104,             //������������(104)
    SUB_S_CATCH_BIRD = 105,
    SUB_S_SEND_BULLET = 106,
    SUB_S_FIRE_FAILED = 107,

    SUB_C_FIRE = 108,
    SUB_C_CATCH_FISH = 109,

    STATUSFREE = 110,
    SUB_S_ENTER_ANDROID_INFO = 111
};
///////////////////////////////////////////////////////////////////////////////////////////////
class Point
{
public:
    Point():x_(0), y_(0) {}
    Point(float x, float y):x_(x), y_(y) {}
    Point(const Point &point):x_(point.x_), y_(point.y_) {}
    ~Point() {}

public:
    void offset(float x, float y) { x_+= x; y_+=y; }

    void set_point(float x, float y) { x_=x; y_=y; }

    bool operator == (const Point &point) const { return (x_==point.x_&&y_==point.y_); }
    bool operator != (const Point &point) const { return (x_!=point.x_||y_!=point.y_); }

    Point &operator = (const Point &point) { x_=point.x_; y_=point.y_; return *this; }

    void operator += (const Point &point) { x_+=point.x_; y_+=point.y_; }
    void operator -= (const Point &point) { x_-=point.x_; y_-=point.y_; }

    Point operator + (const Point &point) { return Point(x_+point.x_, y_+point.y_); }
    Point operator - (const Point &point) { return Point(x_-point.x_, y_-point.y_); }
    Point operator - () { return Point(-x_, -y_); }

    Point operator * (float multip) { return Point(x_*multip, y_*multip); }

public:
    double x_;
    double y_;
};


inline Point operator+ (const Point &lhs, const Point &rhs)
{
    return Point(lhs.x_+rhs.x_, lhs.y_+rhs.y_);
}

inline Point operator- (const Point &lhs, const Point &rhs)
{
    return Point(lhs.x_-rhs.x_, lhs.y_-rhs.y_);
}


class Size
{
public:
    Size():width_(0), height_(0)  { }
    Size(float width, float height):width_(width), height_(height) {}
    Size(const Size &size): width_(size.width_), height_(size.height_) {}
    ~Size() {}

    bool operator == (const Size &size) const { return (size.width_==width_&&size.height_==height_); }
    bool operator != (const Size &size) const { return (size.width_!=width_||size.height_!=height_); }

    Size &operator = (const Size &size) { width_=size.width_; height_=size.height_; return *this; }

    Size& operator += (const Size &size) { width_+=size.width_; height_+=size.height_; return *this; }
    Size& operator -= (const Size &size) { width_-=size.width_; height_-=size.height_; return *this; }

    void set_size(float width, float height) { width_=width; height_=height; }

    Size operator + (const Size &size) { return Size(width_+size.width_, height_+size.height_); }
    Size operator - (const Size &size) { return Size(width_-size.width_, height_-size.height_); }
    Size operator - () { return Size(-width_, -height_); }

public:
    float width_;
    float height_;
};

///////////////////////////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
#define SCORE long long
//#define KIND_ID						603									    //游戏 I D
#define GAME_PLAYER					4									    //游戏人数
//
//#define VERSION_SERVER				PROCESS_VERSION(6,0,3)					//程序版本
//#define VERSION_CLIENT				PROCESS_VERSION(6,0,3)					//程序版本
//
//#define GAME_NAME					TEXT("极速捕鱼")						//游戏名字

//////////////////////////////////////////////////////////////6/////////////
#define IDI_GAME_LOOP                 1
#define TIME_GAME_LOOP                100

#define IDI_SAVE_JSFISH_STOCK	2
#define TIME_SAVE_JSFISH_STOCK	30 * 1000

#define IDI_SAVE_JSFISH_SYSINFO 3
#define TIME_SAVE_JSFISH_SYSINFO 5 * 60 * 1000

#define IDI_ANDROID_AUTO_SHOOT 4
#define TIME_ANDROID_AUTO_SHOOT 140

#define IDI_CHECK_TIMEOUT_FISH 5
#define TIMER_CHECK_TIMEOUT_FISH 5*1000

//#define INVALID_CHAIR 				  0xFFFF
#define MAX_PATH 					  128
#define JIESUAN_TIME				  10000

const int kFPS = 50;				//帧率
const float kSpeed = 2.f / kFPS;	//速度

const int kScreenWidth = 1334;
const int kScreenHeight = 750;

const int kMaxRangeCount = 10;		//库存最大数量

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
#define BIRD_TYPE_0                      0  //小黄鱼
#define BIRD_TYPE_1                      1  //小绿鱼
#define BIRD_TYPE_2                      2  //热带鱼
#define BIRD_TYPE_3                      3  //大眼鱼
#define BIRD_TYPE_4                      4  //彩红鱼
#define BIRD_TYPE_5                      5  //小丑鱼
#define BIRD_TYPE_6                      6  //海豚鱼
#define BIRD_TYPE_7                      7  //蓝条鱼
#define BIRD_TYPE_8                      8  //蓝灯笼
#define BIRD_TYPE_9                      9  //红灯笼
#define BIRD_TYPE_10                     10 //神仙鱼
#define BIRD_TYPE_11                     11 //海鬼
#define BIRD_TYPE_12                     12 //蝴蝶鱼
#define BIRD_TYPE_13                     13 //长尾鱼
#define BIRD_TYPE_14                     14 //剑雨
#define BIRD_TYPE_15                     15 //魔鬼鱼
#define BIRD_TYPE_16                     16 //金鲨
#define BIRD_TYPE_17                     17 //双头鲸
#define BIRD_TYPE_18                     18 //定海神针(金箍棒)
#define BIRD_TYPE_19                     19 //金蟾
#define BIRD_TYPE_20                     20 //蝙蝠鱼
#define BIRD_TYPE_21                     21 //美人鱼
#define BIRD_TYPE_22                     22 //凤凰
#define BIRD_TYPE_23                     23 //金龙
#define BIRD_TYPE_24                     24 //章鱼
#define BIRD_TYPE_25                     25 //孙悟空
#define BIRD_TYPE_26                     26 //蚌精
#define BIRD_TYPE_27                     27 //龙王三太子
#define BIRD_TYPE_28                     28 //炸弹
#define BIRD_TYPE_29                     29 //BOSS
#define MAX_BIRD_TYPE                    30

#define BIRD_PAUSE						 BIRD_TYPE_18	//定屏炸弹
#define BOMB_SMALL						 MAX_BIRD_TYPE	//小型炸弹
#define BOMB_LARGE						 MAX_BIRD_TYPE	//大型炸弹
#define BOMB_ULTIMATELY					 MAX_BIRD_28	//全屏炸弹
#define BOSS_FISH						 BIRD_TYPE_29	//BOSS

//特殊鱼
#define BIRD_TYPE_CHAIN					 40		//闪电鱼
#define BIRD_TYPE_RED					 41		//红鱼
#define BIRD_TYPE_INGOT					 42		//元宝鱼

//一箭多雕
#define BIRD_TYPE_ONE					 50
#define BIRD_TYPE_TWO					 51
#define BIRD_TYPE_THREE					 52
#define BIRD_TYPE_FOUR					 53
#define BIRD_TYPE_FIVE					 54

//小鱼和大鱼区分标识
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

// 鱼效果类型
//////////////////////////////////////////////////////////////////////////
#define BIRD_ITEM_NULL						0
#define BIRD_ITEM_SPECIAL_BOMB_1			1
#define BIRD_ITEM_SPECIAL_BOMB_2			2
#define BIRD_ITEM_SPECIAL_BOMB_3			3
#define BIRD_ITEM_SPECIAL_DING				4
#define BIRD_ITEM_SPECIAL_LUCKY				5
#define BIRD_ITEM_SPECIAL_GOLDX2			6
#define BIRD_ITEM_SPECIAL_TOR				7	//音效
#define BIRD_ITEM_SPECIAL_EEL				8   //音效
#define BIRD_ITEM_SPECIAL_SMALL_PURSE		9
#define BIRD_ITEM_SPECIAL_MIDDLE_PURSE		10
#define BIRD_ITEM_SPECIAL_BIG_PURSE			11

//子弹种类
#define BULLET_KIND_COUNT					10
//任务用到的鱼种最大数量
#define	TASK_BIRD_COUNT						10

//////////////////////////////////////////////////////////////////////////
// 鱼特效
//////////////////////////////////////////////////////////////////////////
#define BIRD_EFFECT_NULL					0
#define BIRD_EFFECT_GOLDX2					1
#define BIRD_EFFECT_LUCKY					2
#define BIRD_EFFECT_DING					3
#define BIRD_EFFECT_EEL						4
#define BIRD_EFFECT_TOR						5

//鱼阵类型
#define YUZHEN_NORMAL						0
#define YUZHEN_LINEAR						1
#define YUZHEN_ROUND						2
#define YUZHEN_PAUSE_LINEAR					3


//////////////////////////////////////////////////////////////////////////
//*****************服务端消息

#define SUB_S_CATCH_BIRD_GROUP              113		//抓住鱼

//////////////////////////////////////////////////////////////////////////
//子弹类型
#define BULLET_PENETRATE		0x01		//穿透
#define BULLET_DOUBLE			0x02		//双炮
#define BULLET_FURY				0x04		//狂暴

//捕到鱼
struct CMD_C_Catch_Fish:public MSG_GameMsgUpHead
{
    unsigned short 					chair_id;			//椅子ID
    unsigned int 					fish_id_;			//鱼ID
    unsigned char 					bullet_kind;		//子弹类型
    unsigned int 					bullet_multiple;	//子弹倍数
	unsigned int            		bullet_id;
};
struct Role_Net_Object
{
    unsigned short chair_id_;
	double catch_gold_;
    unsigned short cannon_type_;
	double cannon_mulriple_;
};

//子弹配置
struct BulletConfig{
	double speed;				//速度
	double fire_interval;		//开炮间隔
};

struct S_Enter_Android_info
{
	double score;
	unsigned  int chair_id;
};


//////////////////////////////////////////////////////////////////////////
struct CMD_S_StatusFree
{
    unsigned char scene_;										//场景
	double cannon_mulriple_[MAX_CANNON_TYPE];
    unsigned int mulriple_count_;
    unsigned short scene_special_time_;						//
	Role_Net_Object role_objects_[GAME_PLAYER];			//玩家信息
	BulletConfig bullet_config_[BULLET_KIND_COUNT];		//子弹配置
};

struct CMD_S_TimeCheck
{
    unsigned short chair_id_;
    unsigned int client_time_;
    unsigned int server_time_;
};

struct CMD_C_TimeCheck
{
    unsigned short chair_id_;
    unsigned int time_;
	bool is_game_status_;
};

struct CMD_S_OrderTimeCheck
{
    unsigned short chair_id_;
};

struct CMD_S_Send_Bird
{
    unsigned int id_;
    unsigned char type_;
    unsigned char  item_;			//鱼特效；如果是闪电鱼 红鱼 此项代表鱼类型
    unsigned short path_id_;
    unsigned char path_type_;
	double path_delay_;
	Point path_offset_;
	double elapsed_;
	double speed_;
    unsigned int time_;

    CMD_S_Send_Bird& operator= (CMD_S_Send_Bird &bird)
	{
		id_ = bird.id_;
		type_ = bird.type_;
		item_ = bird.item_;
		path_id_ = bird.path_id_;
		path_type_ = bird.path_type_;
		path_delay_ = bird.path_delay_;
		path_offset_.y_ = bird.path_offset_.y_;
		path_offset_.x_ = bird.path_offset_.x_;
		elapsed_ = bird.elapsed_;
		speed_ = bird.speed_;
		time_ = bird.time_;

		return *this;
	}
};

struct CMD_S_Send_Bird_Linear
{
    unsigned int id_;
    unsigned char type_;
    unsigned char item_;
	double path_delay_;
	double elapsed_;
	double speed_;

	Point start_;
	Point end_;

    unsigned int time_;
};

struct CMD_S_Send_Bird_Pause_Linear
{
    unsigned int id_;
    unsigned char type_;
    unsigned char item_;
	double path_delay_;
	double elapsed_;
	double speed_;
	double start_angle_;

	Point start_;
	Point pause_;
	Point end_;
	double pause_time_;

    unsigned int time_;
};

struct CMD_S_Send_Bird_Round
{
    unsigned int id_;
    unsigned char  type_;
    unsigned char item_;
	double path_delay_;
	double elapsed_;
	double speed_;

	Point center_;
	double radius_;
	double rotate_duration_;
	double start_angle_;
	double rotate_angle_;
	double move_duration_;

    unsigned int time_;
};



struct CMD_C_Fire:public MSG_GameMsgUpHead
{
    unsigned short chair_id_;
	double rote_;				//< 开火, 角度
	double bullet_mulriple_;	//< 子弹倍数
	int lock_bird_id_;			//锁定鱼id
    unsigned char bullet_type_;		//< 子弹类型  穿透 双倍  狂暴
    unsigned  int bullet_id;   //子弹id
};

struct CMD_S_Fire
{
    unsigned short chair_id_;
	double rote_;				//< 开火, 角度
    unsigned int bullet_mulriple_;	//< 子弹倍数
	int lock_bird_id_;			//锁定鱼id
    unsigned char bullet_type_;		//< 子弹类型  穿透 双倍  狂暴
	SCORE cur_gold_;
};

struct CMD_S_Send_Bullet
{
	double rotation_;					//角度
    unsigned short chair_id_;					//座位号
	double bullet_mulriple_;			//子弹倍数
	int lock_bird_id_;					//锁定鱼ID
	double cur_gold_;					//当前金币
    unsigned char bullet_kind;				//子弹类型
	unsigned int bullet_id;                 //子弹ID
};

struct CMD_S_Fire_Failed
{
    unsigned short chair_id_;
	double nowGlod;
};

struct CMD_S_Catch_Bird
{
    unsigned short chair_id_;
    unsigned short cannon_type_;
    unsigned int catch_gold_;
    unsigned int bird_id_;
    unsigned short award_type_;
    unsigned short awarad_count_;
    unsigned int medal_;
};

struct Catch_Bird
{
    unsigned short chair_id_;
	double catch_gold_;		//分数
    unsigned int bird_id_;			//当前鱼id
	double now_money;			//当前金币数
	bool isDouble;				//是否暴击
    unsigned char  is_die;			//是否死亡
};
//红鱼和其它鱼使用的一样，未加新消息
struct CMD_S_Catch_Bird_Group
{
    unsigned char count_; //对齐
    unsigned char chair_id_;

	Catch_Bird catch_bird_[MAX_BIRD_CATCH];
    unsigned int bullet_id_;
};


struct CMD_S_Change_Scene
{
    unsigned char scene_;
    unsigned short special_time_;   //特殊鱼阵时间
};

//查询
struct CMD_C_Admin_Query
{
    unsigned char operate_;
};

//<基本配置
struct CMD_CS_Admin_Basic
{
	int exchange_ratio_userscore_;					//兑换比率
	int exchange_ratio_birdscore_;
	int exchange_count_;							//每次兑换数量
	float scene_swith_time_;						//场景切换时间
    unsigned long long  bullet_valid_count_;					//有效子弹数量
	float total_return_rate_;						//总体返还百分比
	int64_t game_revenue_;							//总税收值
	int level_difficulty_;							//当前难度
	int change_level_interval_;						//难度切换时间
    unsigned int cannon_mulriple_[MAX_CANNON_TYPE];		//大炮倍率
    unsigned int mulriple_count_;						//倍率个数
	BulletConfig bullet_config_[BULLET_KIND_COUNT]; //子弹配置
};




//<鱼配置
//普通鱼配置
struct BirdConfig {
	int mulriple_min;								//最小倍数
	int mulriple_extend[4];							//扩展倍数
	int mulriple_max;								//最大倍数
	int over_zero_prob;								//净分>0暴击概率
	int under_zero_prob;							//净分<0暴击概率
	Size bird_size;									//大小
	float speed;									//速度
	double capture_probability;						//捕获概率
	int distribute_interval_min;					//最小生成间隔
	int distribute_interval_max;					//最大生成间隔
	int distribute_count_min;						//最小生成数量
	int distribute_count_max;						//最大生成数量
	int path_type;									//路径类型
	char fish_name[20];							//鱼名字
};

struct SendBird
{
    unsigned long long  id_;
    unsigned char  type_;
    unsigned char  item_;
    unsigned short path_id_;
    unsigned char  path_type_;
	double    path_delay;
	double    path_offset_x_;
	double    path_offset_y_;
	int      elapsed_;
	double    speed_;
	int64_t  time_;
};

struct S_ChangeScene
{
    unsigned char scene_;
	double   scene_special_time_;
};

struct chair_info
{
    unsigned int user_id;
	int64_t  user_enter_score;
	int64_t  user_current_score;
	//玩家实际输赢
	int64_t  user_win_lose_score;
	int64_t  user_miss_stock;
	//玩家流水
	int64_t  user_stream;
	//玩家提供的税收
	int64_t  user_tax;
	//paoshu
    unsigned int user_send_pao_count;
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
	}
};

//////////////////////////////////////////////////////////////////////////

#ifdef _WIN32
#pragma pack(pop)
#else
#pragma pack()
#endif

#endif