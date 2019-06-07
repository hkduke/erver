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
//������
class GameTableLogic 
{
public:
	typedef std::vector<uint32_t> Entity_Ids;

	//��������
public:
	//���캯��
	GameTableLogic(CGameProcess* pGameRoom);
	//��������
	virtual ~GameTableLogic();

	//�����ӿ�
public:
	//��ʼ���ӿ�
	void Initialization();
	//����
	void CleanUp();

	//��Ϸ�¼�
public:
	//��Ϸ��ʼ
	void OnEventGameStart();
	//��Ϸ���� û�е�������
	//void OnEventGameConclude(unsigned short wChairID, GamePlayer * pGamePlayer, unsigned char cbReason);
	//���ͳ���
	void OnEventSendGameScene(unsigned short wChiarID, unsigned char bGameStatus, bool bSendSecret);

	//�¼��ӿ�
public:
	//ʱ���¼�
	void OnTimerMessage(unsigned int wTimerID, /*WPARAM*/unsigned int wBindParam);
	//��Ϸ��Ϣ
	//void OnGameMessage(int nProtocol,SVar &s , GamePlayer * pGamePlayer);
	void OnGameMessage(unsigned short wChairID, unsigned int dwSubCmdID,  void * pDataBuffer, unsigned int dwDataSize);
	//�����Ϣ û�е�������
	//void OnFrameMessage(unsigned short wSubCmdID, const void * pDataBuffer, unsigned short wDataSize, GamePlayer * pGamePlayer);
	
	//�û��¼�
public:
	//�û����� û�е��� ����
	//void OnActionUserSitDown(unsigned short wChairID, bool bLookonUser);
	void InitRoleInfo(unsigned short wChairID, bool bLookonUser);
	//�û�����
	void OnActionUserStandUp(unsigned short wChairID, bool bLookonUser);
	void UnInitRoleInfo(unsigned short wChairID, bool bLookonUser);
	//�û����� û�е�������
	//void OnActionUserOffLine(unsigned short wChairID, GamePlayer * pGamePlayer);
	
	//��Ϣ������
protected:
	//��ҿ���
	void OnSubPlayerFire(unsigned short chair_id, CMD_C_Fire *pFire,  bool bLoop = false);
	//������е���
	void OnCatchFish(unsigned short chair_id, CMD_C_Catch_Fish *cathFish);

private:
	//������� û�е��� ����
	/*void CalGameScore(unsigned short wChairID, GamePlayer* pGamePlayer);*/
	//�л�����
	void ChangeScene();

private:
	//��ʱ�ַ���
	void GameLoop(float dt);	
	//��������
	void ProcessCatchFish(unsigned short chair_id, uint8_t type, uint32_t nBulletMul, uint32_t bullet_id, bool isAddFireProb = false);

	//������
private:
	//���͵��ͻ���
	void SendFish();
	//����������
	void SendSpecialFish();
	//���ɵ�����
	void SendSingleFish(int bird_type);
	//�������
	void SendGroupFish();
	//����ĳ�����
	void SendRedBloating();
	//����
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
	//���ڼ��� û�е��� ����
	//void CalculateFire(unsigned short wChairId, uint32_t multiple, GamePlayer* pGamePlayer);
	//���Ϳ���ʧ��
	void SendFireFailed(unsigned short chairID);
	//�ۻ����ڴ��� �ж��Ƿ������Ӯǿ��  û�е�������
	/*void AccumulateFireCount(unsigned short chairID, int bulletMultiple, GamePlayer* pGamePlayer);*/

	//�ж����Ƿ��ܹ�������
	bool ComputeProbability(unsigned short chair_id, int nBulletMul, int nFishKind, int nFishMul, int nChairID, Bird* pFish, bool & bIsDouble);
	//�������ж� û�е�������
	//bool CheckBaoJi(GamePlayer*pGamePlayer, int nFishKind, int64_t lCatchScore);
	//��������������
	void AddFishRefresh(int nFishType);

	//������
public:
	//������������
	int GetFishBaseMul(int nFishType);
	//����Ѫ��
	void SetFishScore(Bird *pFish, int nFishType);
	//�����㸽��Ѫ�� û�е�������
	//void SetFishPlusScore(GamePlayer *pGamePlayer, Bird *pFish, int nBulletMul,int nFishMul, int nFishType);
	//עѪ���� û�е�������
	//void RecovePlusScore(Bird *pFish, int nFishType);
	
public:
	//���������
	void ClearFish(float dt);
	//�ַ���
	void DistributeFish(float dt);
	//���������ϵĳ�ʱ�ӵ�
	void check_outtime_bullet();

	//�������
private:
	//���ر仯
	void PondChange();

public:
	//��ҵ�� û�е��� ����
	/*bool CheckPlayer(GamePlayer *pGamePlayer);*/

public:
	//����ʱ�����ѡ����ĸ���
	void selectFishByTime(int &fishKind, int &fishMultiple);

public:
	//������Ч�ӵ�
	void DeleteInvalidBullet(unsigned short wChairID);

	//�ڲ�����
protected:
    bool compute_collision(float bird_x, float bird_y, float bird_width, float bird_height,
		                   float bird_rotation,float bullet_x, float bullet_y, float bullet_radius);

	bool compute_energy_collision(float bird_x, float bird_y, float bird_width, float bird_height, float bird_rotation, 
							float bullet_x, float bullet_y, float bullet_width, float bullet_height, float bullet_rotation);

	int compute_distance(float x1, float y1, float x2, float y2);

	void bomb_check(Bullet* bullet, Bird *bomb_bird);		//ը��
	//void chain_bird_event(Bullet* bullet, Bird* bird);		//������
	void red_bird_event(Bullet* bullet, Bird* bird, unsigned short charid);		//����

public:
	//void task_update(float dt);					//����
	void config_update();
	void user_fire_bullet();
	void bird_speed_up();						//�����ζ�
	bool send_scene_yuzhen(Entity *target, uint32_t data);	//��������
	void test_energy(uint16_t bird_type, unsigned short chair_id);	//���������
	void load_conf();

	//У����Ҵ��������ӵ�ID�Ƿ�Ϸ�
	bool is_valid_bullet_id(int chair_id, uint32_t bullet_id);
public:
	//ԭ�������� δʹ��
	//void send_normal_bird();
	//void send_small_bird();
	//void send_big_bird();
	//void send_huge_bird();
	//void send_small_clean_sweep_bird();
	//void send_clean_sweep_bird();
	//void send_boss_bird();
	void send_bird_small_clean_sweep();
	void send_bird_shape();

	//��������
	void send_scene_left_bird();
	void send_scene_right_bird();

	//������
	/*void send_scene_entity(GamePlayer * pGamePlayer);*/
	bool func_bird_end(Entity *target, uint32_t data);

	
	//ֱ�� ���¶���
	void send_scene_array_four();
	bool up_down_linear(Entity *target, uint32_t data);

	//2015��5��15��17:20:14
	

	//������
	void android_update(float delta_time);
	void change_bullite_mulriple(unsigned short chair_id);
	//���������
	

	//*****������� ���� ���� ����******
	//��λ�ü��
	bool inside_screen(Bird* bird);
	//ȡ������
	bool cancel_bullet_lock(Entity * target, uint32_t data);
	bool cancel_bullet_Temp(Entity * target, uint32_t data);
	//�ӵ���͸ ʱ�����
	float bullet_move_duration(const Point& start, const Point end, float bullet_speed);
	//����ȫ�ֽ���ϵ�� û�е�������
	void calculatorPondCoefficient();
	//���㵱ǰ��ҵľ�����ʵ��ֵ û�е��� ����
	/*void calculatorForceThreshold(unsigned short wChairID, GamePlayer * pGamePlayer);*/
	//�����־
	void debugLog(char *logStr, unsigned int userID);
	
	//��ʱ��¼��ҽ����־
	void timeWriteScore();

	
	//���ݿ��ȷ��ǿ������
	//WinForceControl selectConfigByPond(GamePlayer* userItem);

	
	
	//������������
	int fish_multiple_control(int fishKind);
	//Բ��
	int RoundDesk(int count, int weightArr[]);
public:
	Entity_Manager<Bird> *get_bird_manager() { return &bird_manager_; }
	Path_Manager *get_path_manager() { return &path_manager_; }

	//�����ؿ�ʼ

    //�����ؽ���
	//��ҽ�ɫ
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

	//�򵽵���
	std::vector<CMD_C_Catch_Fish> catch_fishs;	//���е���
	Entity_Ids red_birds_;				//����
	Entity_Ids chain_birds_;			//������
	Entity_Ids remove_birds_;			//������
		
	//��ײ���ӵ�
	Entity_Ids remove_bullet_;

	//��������
	//�����
	Entity_Manager<Bird> bird_manager_;
	//�ӵ�����
	Bullet_Manager<Bullet> bullet_manager_;
	Entity_Manager<Entity> entity_manager_;

	//�������
public:
	//���ؿ����Ϣ Ŀǰ�Ǵ������ж�ȡ
	//void load_stock_info(StockInfo &info);
	uint8_t scene_;
    uint32_t scene_start_time_;	
	//��������ʱ��(>0, ����ʱ��)
	float scene_special_time_;						//��������ʱ�䣨�л�����ʱ������������

	//�л�����ʱ����������
	bool scene_left_sended_;						
    bool scene_right_sended_;

	Timer_Control timer_control_;			//ʱ��
	//**********************************************************


	bool bird_move_pause_is_;							//����

	Timer timer_;
	time_t old_time_;
	time_t write_old_time;
	Bird_Factory birds_factory_;						//���������
	static Path_Manager path_manager_;					//·��

	//2015��5��15��17:02:44

	//��Ч�ӵ�
	float bullet_valid_check_elasped_[GAME_PLAYER];	

	//����ˢ����ļ�� �ж��Ƿ���ˢ��
	time_t  add_fish_interval[MAX_BIRD_TYPE];			

	//��ʵ��ҿ���
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
	int64_t m_userMinForceThreshold[GAME_PLAYER];	//��Ҿ������ֵ ��ϵͳ��ֵ�Ƚ� ȡ���з��Ϲ����һ��
	int64_t m_userMaxForceThreshold[GAME_PLAYER];
	int				m_iUserFireCount[GAME_PLAYER];   //��������δ���еĴ���
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

	//��ʱ�ӵ������
	float m_check_timeou_bullet_interval;
};

///////////////////////////////////////////////////////////////////////////////////////////

#endif