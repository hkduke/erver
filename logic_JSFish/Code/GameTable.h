#ifndef TABLE_FRAME_SINK_HEAD_FILE
#define TABLE_FRAME_SINK_HEAD_FILE

///////////////////////////////////////////////////////////////////////////////////////////
#include <Mutex.h>
#include "Define.h"
#include "Prereqs.h"

#include "EntityManager.h"
#include "Bullet_Manager.h"

#include "Bird.h"
#include "Bullet.h"

#include "ActionInterval.h"
#include "ActionInstant.h"
#include "ActionCombine.h"
#include "ActionCustom.h"
#include "ActionManager.h"
#include "PathManager.h"
#include "Role.h"
#include "Timer.h"
#include "GameConfig.h"
#include "Game_Config_Xml.h"
#include "BirdFactory.h"
#include "GameControl.h"
class CGameProcess;
class BYStockConfig;
///////////////////////////////////////////////////////////////////////////////////////////
//桌子类
class GameTableLogic 
{
public:
	typedef std::vector<uint32_t> Entity_Ids;

	//函数定义
public:
	//构造函数
	GameTableLogic(CGameProcess* pGameRoom);
	//析构函数
	virtual ~GameTableLogic();

	//基础接口
public:
	//初始化接口
	void Initialization();
	//清理
	void CleanUp();

	//游戏事件
public:
	//游戏开始
	void OnEventGameStart();
	//游戏结束 没有调用屏蔽
	//void OnEventGameConclude(unsigned short wChairID, GamePlayer * pGamePlayer, unsigned char cbReason);
	//发送场景
	void OnEventSendGameScene(unsigned short wChiarID, unsigned char bGameStatus, bool bSendSecret);

	//事件接口
public:
	//时间事件
	void OnTimerMessage(unsigned int wTimerID, /*WPARAM*/unsigned int wBindParam);
	//游戏消息
	//void OnGameMessage(int nProtocol,SVar &s , GamePlayer * pGamePlayer);
	void OnGameMessage(unsigned short wChairID, unsigned int dwSubCmdID,  void * pDataBuffer, unsigned int dwDataSize);
	//框架消息 没有调用屏蔽
	//void OnFrameMessage(unsigned short wSubCmdID, const void * pDataBuffer, unsigned short wDataSize, GamePlayer * pGamePlayer);
	
	//用户事件
public:
	//用户坐下 没有调用 屏蔽
	//void OnActionUserSitDown(unsigned short wChairID, bool bLookonUser);
	void InitRoleInfo(unsigned short wChairID, bool bLookonUser);
	//用户起立
	void OnActionUserStandUp(unsigned short wChairID, bool bLookonUser);
	void UnInitRoleInfo(unsigned short wChairID, bool bLookonUser);
	//用户断线 没有调用屏蔽
	//void OnActionUserOffLine(unsigned short wChairID, GamePlayer * pGamePlayer);
	
	//消息处理函数
protected:
	//玩家开火
	void OnSubPlayerFire(unsigned short chair_id, CMD_C_Fire *pFire,  bool bLoop = false);
	//处理打中的鱼
	void OnCatchFish(unsigned short chair_id, CMD_C_Catch_Fish *cathFish);

private:
	//计算分数 没有调用 屏蔽
	/*void CalGameScore(unsigned short wChairID, GamePlayer* pGamePlayer);*/
	//切换场景
	void ChangeScene();

private:
	//定时分发鱼
	void GameLoop(float dt);	
	//处理捕获鱼
	void ProcessCatchFish(unsigned short chair_id, uint8_t type, uint32_t nBulletMul, uint32_t bullet_id, bool isAddFireProb = false);

	//发送鱼
private:
	//发送到客户端
	void SendFish();
	//发送特殊鱼
	void SendSpecialFish();
	//生成单条鱼
	void SendSingleFish(int bird_type);
	//发送组合
	void SendGroupFish();
	//红鱼鼓出鱼阵
	void SendRedBloating();
	//鱼阵
public:
	void send_bird_linear();
	void send_bird_round();
	void send_bird_pause_linear();
	void send_scene_bloating();
	void send_scene_double_bloating();
	bool round_linear(Entity *target, void* data);
	bool build_scene_bird(Bird bird, Action_Bird_Move* action);
	bool build_bird_common(Bird &bird, Action_Bird_Move *action);

	void user_enter(unsigned int chair_id);
	void user_left(unsigned int chair_id);

	void send_special_birds(unsigned int birds_no);
	void send_special_boss(unsigned short chair_id, Bird &boss);
	void send_special_birds_ex(unsigned short chair_id);

private:
	//开炮计算 没有调用 屏蔽
	//void CalculateFire(unsigned short wChairId, uint32_t multiple, GamePlayer* pGamePlayer);
	//发送开炮失败
	void SendFireFailed(unsigned short chairID);
	//累积开炮次数 判断是否进入输赢强控  没有调用屏蔽
	/*void AccumulateFireCount(unsigned short chairID, int bulletMultiple, GamePlayer* pGamePlayer);*/

	//判断鱼是否能够被命中
	bool ComputeProbability(unsigned short chair_id, int nBulletMul, int nFishKind, int nFishMul, int nChairID, Bird* pFish, bool & bIsDouble);
	//暴击鱼判断 没有调用屏蔽
	//bool CheckBaoJi(GamePlayer*pGamePlayer, int nFishKind, int64_t lCatchScore);
	//根据死鱼增加鱼
	void AddFishRefresh(int nFishType);

	//库存相关
public:
	//库存鱼基础概率
	int GetFishBaseMul(int nFishType);
	//设置血量
	void SetFishScore(Bird *pFish, int nFishType);
	//设置鱼附加血量 没有调用屏蔽
	//void SetFishPlusScore(GamePlayer *pGamePlayer, Bird *pFish, int nBulletMul,int nFishMul, int nFishType);
	//注血回收 没有调用屏蔽
	//void RecovePlusScore(Bird *pFish, int nFishType);
	
public:
	//清理过期鱼
	void ClearFish(float dt);
	//分发鱼
	void DistributeFish(float dt);
	//检查该桌子上的超时子弹
	void check_outtime_bullet();

	//奖池相关
private:
	//奖池变化
	void PondChange();

public:
	//玩家点控 没有调用 屏蔽
	/*bool CheckPlayer(GamePlayer *pGamePlayer);*/

public:
	//根据时间随机选择鱼的概率
	void selectFishByTime(int &fishKind, int &fishMultiple);

public:
	//回收无效子弹
	void DeleteInvalidBullet(unsigned short wChairID);

	//内部函数
protected:
    bool compute_collision(float bird_x, float bird_y, float bird_width, float bird_height,
		                   float bird_rotation,float bullet_x, float bullet_y, float bullet_radius);

	bool compute_energy_collision(float bird_x, float bird_y, float bird_width, float bird_height, float bird_rotation, 
							float bullet_x, float bullet_y, float bullet_width, float bullet_height, float bullet_rotation);

	int compute_distance(float x1, float y1, float x2, float y2);

	void bomb_check(Bullet* bullet, Bird *bomb_bird);		//炸弹
	//void chain_bird_event(Bullet* bullet, Bird* bird);		//闪电鱼
	void red_bird_event(Bullet* bullet, Bird* bird, unsigned short charid);		//红鱼

public:
	//void task_update(float dt);					//任务
	void config_update();
	void user_fire_bullet();
	void bird_speed_up();						//加速游动
	bool send_scene_yuzhen(Entity *target, uint32_t data);	//发送鱼阵
	void test_energy(uint16_t bird_type, unsigned short chair_id);	//检测能量炮
	void load_conf();

	//校验玩家传上来的子弹ID是否合法
	bool is_valid_bullet_id(int chair_id, uint32_t bullet_id);
public:
	//原版生成鱼 未使用
	//void send_normal_bird();
	//void send_small_bird();
	//void send_big_bird();
	//void send_huge_bird();
	//void send_small_clean_sweep_bird();
	//void send_clean_sweep_bird();
	//void send_boss_bird();
	void send_bird_small_clean_sweep();
	void send_bird_shape();

	//左右鱼阵
	void send_scene_left_bird();
	void send_scene_right_bird();

	//生成鱼
	/*void send_scene_entity(GamePlayer * pGamePlayer);*/
	bool func_bird_end(Entity *target, uint32_t data);

	
	//直线 上下对游
	void send_scene_array_four();
	bool up_down_linear(Entity *target, uint32_t data);

	//2015年5月15日17:20:14
	

	//机器人
	void android_update(float delta_time);
	void change_bullite_mulriple(unsigned short chair_id);
	//标记锁定鱼
	

	//*****功能添加 锁定 闪电 红鱼******
	//鱼位置检测
	bool inside_screen(Bird* bird);
	//取消锁定
	bool cancel_bullet_lock(Entity * target, uint32_t data);
	bool cancel_bullet_Temp(Entity * target, uint32_t data);
	//子弹穿透 时间计算
	float bullet_move_duration(const Point& start, const Point end, float bullet_speed);
	//计算全局奖池系数 没有调用屏蔽
	void calculatorPondCoefficient();
	//计算当前玩家的净分真实阈值 没有调用 屏蔽
	/*void calculatorForceThreshold(unsigned short wChairID, GamePlayer * pGamePlayer);*/
	//打出日志
	void debugLog(char *logStr, unsigned int userID);
	
	//定时记录玩家金币日志
	void timeWriteScore();

	
	//根据库存确定强控配置
	//WinForceControl selectConfigByPond(GamePlayer* userItem);

	
	
	//浮动倍数控制
	int fish_multiple_control(int fishKind);
	//圆桌
	int RoundDesk(int count, int weightArr[]);
public:
	Entity_Manager<Bird> *get_bird_manager() { return &bird_manager_; }
	Path_Manager *get_path_manager() { return &path_manager_; }

	//库存相关开始

    //库存相关结束
	//玩家角色
	Role role_[GAME_PLAYER];
private:
	std::random_device m_rd;
	std::mt19937 m_gen;
	//CRITICAL_SECTION lockSection;
	CTMutex m_mutex;
    //GameRoom* table_;
	//GameRoom *m_pGameRoom;

	CGameProcess *m_pGame_Process;

	Action_Manager action_manager_;

	//打到的鱼
	std::vector<CMD_C_Catch_Fish> catch_fishs;	//打中的鱼
	Entity_Ids red_birds_;				//红鱼
	Entity_Ids chain_birds_;			//闪电鱼
	Entity_Ids remove_birds_;			//常规鱼
		
	//碰撞的子弹
	Entity_Ids remove_bullet_;

	//动作管理
	//鱼管理
	Entity_Manager<Bird> bird_manager_;
	//子弹管理
	Bullet_Manager<Bullet> bullet_manager_;
	Entity_Manager<Entity> entity_manager_;

	//场景相关
public:
	//加载库存信息 目前是从配置中读取
	//void load_stock_info(StockInfo &info);
	uint8_t scene_;
    uint32_t scene_start_time_;	
	//特殊鱼阵时间(>0, 鱼阵时间)
	float scene_special_time_;						//特殊鱼阵时间（切换场景时生成特殊鱼阵）

	//切换场景时的左右鱼阵
	bool scene_left_sended_;						
    bool scene_right_sended_;

	Timer_Control timer_control_;			//时间
	//**********************************************************


	bool bird_move_pause_is_;							//定屏

	Timer timer_;
	time_t old_time_;
	time_t write_old_time;
	Bird_Factory birds_factory_;						//生成与管理
	static Path_Manager path_manager_;					//路径

	//2015年5月15日17:02:44

	//有效子弹
	float bullet_valid_check_elasped_[GAME_PLAYER];	

	//增加刷新鱼的间隔 判断是否能刷新
	time_t  add_fish_interval[MAX_BIRD_TYPE];			

	//真实玩家开炮
	struct UserFire
	{
		bool bUserFire;
		CMD_C_Fire userFire;

		UserFire()
		{
			//bool bUserFire = false;
		}
	};

	UserFire user_fire[6];
	int64_t m_userMinForceThreshold[GAME_PLAYER];	//玩家具体的阈值 和系统阈值比较 取其中符合规则的一个
	int64_t m_userMaxForceThreshold[GAME_PLAYER];
	int				m_iUserFireCount[GAME_PLAYER];   //连续开炮未命中的次数
	bool yuzhen;
	float initScene_Time;
	int64_t        m_lCount;
	//BYStockConfig *m_pByStockConfig;

	unsigned int m_curtime;
	unsigned int m_oldtime;

	float m_small_fish_interval_time;
	float m_big_fish_interval_time;

	bool m_b_send_boss[4];
	Bird m_cur_Boss;
	bool m_b_once;
	bool m_b_boss_die;

	//超时子弹检查间隔
	float m_check_timeou_bullet_interval;
};

///////////////////////////////////////////////////////////////////////////////////////////

#endif