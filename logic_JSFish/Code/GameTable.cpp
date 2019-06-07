#include "stdafx.h"
#include "Exception.h"
#include "GameControl.h"
#include "GameTable.h"
#include "lockArray.h"
#include "../F_JsBu.h"
#include <time.h>
//#include "../GameRoom.h"
//#include "../GamePlayer.h"
#include "Random.h"
#include "GameLog.h"
#include "glog_wrapper.h"
#include "../GameProcess.h"
#include "BYStockConfig.h"
namespace 
{
	//û�е��� ����
	/*float calc_distance(float x1, float y1, float x2, float y2)
	{
		return std::sqrt((x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2));
	}*/

	//�Ƕȼ��� û�е��� ����
	/*float calc_angle(float x1, float y1, float x2, float y2)
	{
		float distance = calc_distance(x1, y1, x2, y2);
		if (distance == 0.f)
			return 0.f;
		float sin_value = (x1 - x2) / distance;
		float angle = std::acos(sin_value);
		if (y1 < y2) angle= 2 * M_PI - angle;
		angle += M_PI_2;
		return angle;
	}*/

	//û�е��� ����
	/*float calc_rotate(uint16_t chair_id, const Point pt_bow, const Point pt_offset)
 	{
		float rorate = 0;
		Point pt_rorate = pt_offset - pt_bow;

		if (chair_id == 0 || chair_id == 1 || chair_id == 2)
		{
			pt_rorate = pt_offset - pt_bow;

			if (pt_rorate.y_ >= 0)
			{
				rorate = std::atan2(pt_rorate.y_, pt_rorate.x_) + M_PI_2;
			}
			else
			{
				if (pt_rorate.x_ >= 0)
					rorate = (float)M_PI_2;
				else
					rorate = (float)(M_PI_2 + M_PI);
			}
		}
		else if (chair_id == 3 || chair_id == 4 || chair_id == 5)
		{
			pt_rorate = pt_offset - pt_bow;

			if (pt_rorate.y_ <= 0)
			{
				rorate = std::atan2(pt_rorate.y_, pt_rorate.x_) + M_PI_2;
			}
			else
			{
				if (pt_rorate.x_ >= 0)
					rorate = (float)M_PI_2;
				else
					rorate = (float)(M_PI_2 + M_PI);
			}
		}

		return rorate;
 	}*/
}

///////////////////////////////////////////////////////////////////////////
Path_Manager GameTableLogic::path_manager_;
Game_Config_Xml g_game_config_xml;
SystemPondControl pond_control;						//ȫ�ֽ��ؿ���
extern BYStockConfig g_bystockconfig;
//GameControl g_game_control; //���ƶ�
///////////////////////////////////////////////////////////////////////////
GameTableLogic::GameTableLogic(CGameProcess* pGameProcess):m_gen(m_rd()) //: birds_factory_(this)//, win_lost_manager_(this), game_config_(this)
{
	//InitializeCriticalSection(&lockSection);
	m_pGame_Process = pGameProcess;
	//m_pByStockConfig = new BYStockConfig();
	scene_ = SCENE_TYPE_0;

	scene_start_time_ = 0;		
	scene_special_time_ = 0;
	old_time_ = 0;
	write_old_time = 0;
	scene_right_sended_ = false;
	scene_left_sended_ = false;
	bird_move_pause_is_ = false;

	memset(bullet_valid_check_elasped_,0,sizeof(bullet_valid_check_elasped_));
	
	timer_control_.scene_elapse_ = 0.f;
	timer_control_.bird_move_pause_elapse_ = 0.f;
	memset(timer_control_.distribute_elapsed_, 0.f, sizeof(timer_control_.distribute_elapsed_));
	timer_control_.group_bird_elapse_ = 0.f;
	timer_control_.special_bird_elapse_ = 0.f;
	timer_control_.bird_red_series_elapse_ = 0.f;
	timer_control_.bird_red_bloating_elapse_ = 0.f;
	timer_control_.bird_chain_elapse_ = 0.f;
	timer_control_.bird_ingot_elapse_ = 0.f;
	timer_control_.change_difficulty_elapse_ = 0.f;

	memset(timer_control_.bird_same_elapse_, 0.f, sizeof(timer_control_.bird_same_elapse_));
	yuzhen = true;
	initScene_Time = 0;

	for (int i = 0; i < GAME_PLAYER; i++)
		role_->cleanup(); 
	//srand(GetTickCount());
	srand(Utility::GetTick());
	Randfun::InitRealRandom();
	m_lCount=0;
	m_check_timeou_bullet_interval = 5.0f;

	return;
}

GameTableLogic::~GameTableLogic()
{   Randfun::ReleaseRealRandom();
	CleanUp();

   // if(m_pByStockConfig)
   // {
   //     delete m_pByStockConfig;
   // }
}

void GameTableLogic::Initialization()
{
	//��ǰ����
	scene_ = SCENE_TYPE_0;
	scene_start_time_ = 0;					
	old_time_ = 0;
	write_old_time = 0;
	//�л�����ʱ����������
	scene_right_sended_ = false;
	scene_left_sended_ = false;
	timer_.reset();

	//������Ϸ����
	//g_game_config_xml.load_game_config();
	//����·�������ļ�
	//path_manager_.initialise();
	//��Ҿ������ֵ ��ϵͳ��ֵ�Ƚ� ȡ���з��Ϲ����һ��
	memset(m_userMinForceThreshold, 0, sizeof(m_userMinForceThreshold));
	memset(m_userMaxForceThreshold, 0, sizeof(m_userMaxForceThreshold));
	//��������δ���еĴ���
	memset(m_iUserFireCount, 0, sizeof(m_iUserFireCount));
	//����ˢ����ļ�� �ж��Ƿ���ˢ��
	memset(add_fish_interval, 0, sizeof(add_fish_interval));
	
	//��Ϸ����
	//��ǰ����
	g_game_control.m_currentKeepTimes   = 0;
	//��ǰ����ϵ��
	g_game_control.m_nowPondCoefficient = 1;
}

void GameTableLogic::load_conf()
{
	path_manager_.initialise();
}

void GameTableLogic::CleanUp()
{
	scene_ = SCENE_TYPE_0;

	scene_special_time_ = 0;
	old_time_ = 0;
	write_old_time = 0;
	scene_right_sended_ = false;
	scene_left_sended_ = false;
	bird_move_pause_is_ = false;
	timer_.reset();
	action_manager_.cleanup();
	birds_factory_.cleanup();
	bird_manager_.cleanup();
	bullet_manager_.cleanup();
	entity_manager_.cleanup();
	chain_birds_.clear();
	remove_birds_.clear();
	remove_bullet_.clear();

	timer_control_.scene_elapse_ = 0.f;
	timer_control_.bird_move_pause_elapse_ = 0.f;
	memset(timer_control_.distribute_elapsed_, 0.f, sizeof(timer_control_.distribute_elapsed_));
	timer_control_.group_bird_elapse_ = 0.f;
	timer_control_.special_bird_elapse_ = 0.f;
	timer_control_.bird_red_series_elapse_ = 0.f;
	timer_control_.bird_red_bloating_elapse_ = 0.f;
	timer_control_.bird_chain_elapse_ = 0.f;
	timer_control_.bird_ingot_elapse_ = 0.f;
	timer_control_.change_difficulty_elapse_ = 0.f;
	memset(timer_control_.bird_same_elapse_, 0.f, sizeof(timer_control_.bird_same_elapse_));
	memset(add_fish_interval, 0, sizeof(add_fish_interval));
}

//��Ϸ��ʼ
void GameTableLogic::OnEventGameStart()
{
	//table_->m_pITableFrame->SetGameStatus(GAME_STATUS_FREE);
}

//��Ϸ���� û�е�������
/*void GameTableLogic::OnEventGameConclude(unsigned short wChairID, GamePlayer * pGamePlayer, unsigned char cbReason)
{
	//����û�б����ã�����
	*//*for (unsigned short i = 0; i < GAME_PLAYER; ++i)
	{
		GamePlayer* pGamePlayer = m_pGameRoom->GetPlayer(i);
		if (pGamePlayer == NULL)
		{
			continue;
		}

		CalGameScore(i, pGamePlayer);
	}

	scene_ = 0;
	return;*//*
}*/

//���ͳ���
void GameTableLogic::OnEventSendGameScene(unsigned short wChiarID, unsigned char bGameStatus, bool bSendSecret)
{
	//switch (bGameStatus)
	//{
	//case GAME_STATUS_FREE:		//����״̬
	//case GAME_STATUS_PLAY:		//��Ϸ״̬
	//	{
	if (scene_special_time_ < 0)
	{
		scene_special_time_ = 0;
	}

	//SVar sSend;
	CMD_S_StatusFree StatusFree;
	//ZeroMemory(&StatusFree, sizeof(StatusFree));
	memset(&StatusFree, 0, sizeof(StatusFree));
	StatusFree.scene_              = scene_; 
	StatusFree.scene_special_time_ = scene_special_time_;
	//SHOW("OnEventSendGameScene %d", scene_start_time_);
	//��ǰ����
	//sSend["scene_"]                = scene_;
	//sSend["scene_special_time_"]   = scene_special_time_;
	//�ڵ�����
	memset(StatusFree.cannon_mulriple_, 0, sizeof(StatusFree.cannon_mulriple_));
	//������
	StatusFree.mulriple_count_     = g_game_config_xml.mulriple_count_;
	//sSend["mulriple_count_"]       = g_game_config_xml.mulriple_count_;
	//�����ڵı���
	for (uint32_t i = 0; i < g_game_config_xml.mulriple_count_; i++)
	{
		//sSend["cannon_mulriple_"][i + 1] = g_game_config_xml.cannon_mulriple_[i];
		StatusFree.cannon_mulriple_[i]   = (double)g_game_config_xml.cannon_mulriple_[i] / 100.0f;
	}
	//��ǰ�����Ϣ
	for (int i = 0; i < GAME_PLAYER; i++)
	{
		StatusFree.role_objects_[i].chair_id_        = role_[i].get_id();
		StatusFree.role_objects_[i].catch_gold_      = (double)role_[i].get_gold() / 100.0f;
		StatusFree.role_objects_[i].cannon_type_     = role_[i].get_cannon();
		StatusFree.role_objects_[i].cannon_mulriple_ = (double)role_[i].get_cannon_mulriple() / 100.0f;

		//sSend["role_objects_"][i+1]["chair_id_"]        = role_[i].get_id();
		//sSend["role_objects_"][i+1]["catch_gold_"]      = role_[i].get_gold();
		//sSend["role_objects_"][i+1]["cannon_type_"]     = role_[i].get_cannon();
		//sSend["role_objects_"][i+1]["cannon_mulriple_"] = role_[i].get_cannon_mulriple();
	}
	//�״ν��룬������ʱ��
	if (scene_start_time_  == 0)
	{
		scene_ = 0;
		scene_start_time_ = (unsigned int)time(NULL);
		timer_.reset();
		//SHOW("������ʱ��");
		//m_pGameRoom->SetTimer(TIME_GAME_LOOP);

		m_curtime = m_oldtime = Utility::GetTick();
		m_pGame_Process->set_timer(IDI_GAME_LOOP, TIME_GAME_LOOP);
		//m_pGameRoom->SetTimer(1);
		//m_pGameRoom->m_pITableFrame->SetGameTimer(IDI_GAME_LOOP,TIME_GAME_LOOP,TIMES_INFINITY,0L);
		action_manager_.initialise();
		bird_manager_.initialise(MAX_BIRD_IN_MANAGER);
		bullet_manager_.initialise(MAX_BULLET_IN_MANAGER);
		birds_factory_.initialise(this);
		entity_manager_.initialise(1024);
	}
	//�ӵ�����
	memcpy(StatusFree.bullet_config_, g_game_config_xml.bullet_config_, sizeof(StatusFree.bullet_config_));
/*	for (int i = 0;i < BULLET_KIND_COUNT; i++)
	{
		sSend["bullet_config_"][i + 1]["fire_interval"] = g_game_config_xml.bullet_config_[i].fire_interval;
		sSend["bullet_config_"][i + 1]["speed"]         = g_game_config_xml.bullet_config_[i].speed;
	}*/
	//pGamePlayer->Send(STATUSFREE,&sSend);
	m_pGame_Process->send_player_data(wChiarID, STATUSFREE, &StatusFree, sizeof(CMD_S_StatusFree));
}

//ʱ���¼�
void GameTableLogic::OnTimerMessage(unsigned int wTimerID, unsigned int wBindParam)
{
	//lockArray m_meLock(&lockSection);
	//�˴�����Ҫ��������
	//m_mutex.Lock();
	switch (wTimerID)
	{
	case IDI_GAME_LOOP:
		{
			/*time_t now    = time(NULL);
			time_t elapse = now - old_time_;
			old_time_     = now;*/

			/*m_curtime = Utility::GetTick();
			LOG(ERROR) <<  "time:"  << m_curtime - m_oldtime;
			m_oldtime = m_curtime;*/
			GameLoop(TIME_GAME_LOOP / 1000.0);
			break;
		}
	}
	//m_mutex.Unlock();
	//���ö�ʱ��
	//�������ã�һֱ�����У���������
	//m_pGameRoom->SetTimer(TIME_GAME_LOOP);
	//m_pGame_Process->set_timer(IDI_GAME_LOOP, TIME_GAME_LOOP);
}

//��Ϸ��Ϣ
void GameTableLogic::OnGameMessage(unsigned short wChairID, unsigned int dwSubCmdID, void * pDataBuffer, unsigned int dwDataSize)
{
	if(pDataBuffer == NULL || dwDataSize == 0)
	{
	   // LOG(ERROR) << "GameTableLogic::OnGameMessage ����." << "dwDataSize: " << dwDataSize << " wChairID: "<< wChairID << " dwSubCmdID: "<< dwSubCmdID;
		return;
	}

	switch (dwSubCmdID)
	{
		//�û�����
		case SUB_C_FIRE:
		{
			CMD_C_Fire *pData = static_cast<CMD_C_Fire*>(pDataBuffer);

			CMD_C_Fire PlayerFire;
			//ZeroMemory(&PlayerFire, sizeof(PlayerFire));
			memset(&PlayerFire, 0, sizeof(PlayerFire));

			/*PlayerFire.bullet_mulriple_  = s["bullet_mulriple_"].ToNumber<uint32_t>();
			PlayerFire.bullet_type_      = s["bullet_type_"].ToNumber<uint8_t>();
			PlayerFire.chair_id_         = s["chair_id_"].ToNumber<uint16_t>();
			PlayerFire.lock_bird_id_     = s["lock_bird_id_"].ToNumber<int>();
			PlayerFire.rote_             = s["rote_"].ToNumber<float>();*/

			PlayerFire.bullet_mulriple_  = pData->bullet_mulriple_;
			PlayerFire.bullet_type_      = pData->bullet_type_;
			PlayerFire.chair_id_         = pData->chair_id_;
			PlayerFire.lock_bird_id_     = pData->lock_bird_id_;
			PlayerFire.rote_             = pData->rote_;
			PlayerFire.bullet_id  		 = pData->bullet_id;
			PlayerFire.bullet_idTwo      = pData->bullet_idTwo;
			//m_mutex.Unlock();
			//�����û�����
			return OnSubPlayerFire(wChairID, &PlayerFire);
		}
		//������
		case SUB_C_CATCH_FISH:
		{
			CMD_C_Catch_Fish *pData = static_cast<CMD_C_Catch_Fish *>(pDataBuffer);
			CMD_C_Catch_Fish CatchFish;
			//ZeroMemory(&CatchFish, sizeof(CatchFish));
			memset(&CatchFish, 0, sizeof(CatchFish));

			/*CatchFish.bullet_kind     = s["bullet_kind"].ToNumber<uint8_t>();
			CatchFish.bullet_multiple = s["bullet_multiple"].ToNumber<uint32_t>();
			CatchFish.chair_id        = s["chair_id"].ToNumber<uint16_t>();
			CatchFish.fish_id_        = s["fish_id_"].ToNumber<uint32_t>();*/

			CatchFish.bullet_kind     = pData->bullet_kind;
			CatchFish.bullet_multiple = pData->bullet_multiple;
			CatchFish.chair_id        = pData->chair_id;
			CatchFish.fish_id_        = pData->fish_id_;
			CatchFish.bullet_id       = pData->bullet_id;
			//m_mutex.Unlock();
			//��������
			return OnCatchFish(wChairID, &CatchFish);
		}
	}

	//m_mutex.Unlock();
}

//�����Ϣ û�е�������
/*void GameTableLogic::OnFrameMessage(unsigned short wSubCmdID, const void * pDataBuffer, unsigned short wDataSize, GamePlayer * pGamePlayer)
{
	return ;
}*/

//�û����� û�е��� ����
/*void GameTableLogic::OnActionUserSitDown(unsigned short wChairID, bool bLookonUser)
{
	role_[wChairID].initialise(wChairID);
	bullet_manager_.delete_chair(wChairID);

	*//*if (m_pGameRoom->m_pITableFrame->GetGameStatus() == GAME_STATUS_FREE)
	{
		m_pGameRoom->m_pITableFrame->StartGame();
		m_pGameRoom->m_pITableFrame->SetGameStatus(GAME_STATUS_PLAY);
	}*//*
	//ֱ�Ӱѽ�һ���
	*//*role_[wChairID].set_gold(pGamePlayer->GetCoin());
	role_[wChairID].set_exchange_gold(pGamePlayer->GetCoin());
	role_[wChairID].StartScore     = pGamePlayer->GetCoin();
	role_[wChairID].maxScore       = pGamePlayer->GetCoin();
	role_[wChairID].minScore       = pGamePlayer->GetCoin();
	role_[wChairID].FoceSwitch     = false;
	role_[wChairID].userEnterScore = pGamePlayer->GetCoin();*//*

	role_[wChairID].set_gold(m_pGame_Process->get_user_score(wChairID));
	role_[wChairID].set_exchange_gold(m_pGame_Process->get_user_score(wChairID));
	role_[wChairID].StartScore     = m_pGame_Process->get_user_score(wChairID);
	role_[wChairID].maxScore       = m_pGame_Process->get_user_score(wChairID);
	role_[wChairID].minScore       = m_pGame_Process->get_user_score(wChairID);
	role_[wChairID].FoceSwitch     = false;
	role_[wChairID].userEnterScore = m_pGame_Process->get_user_score(wChairID);

	//������ҵ������С��ֵ
	//calculatorForceThreshold(wChairID, pGamePlayer);
	*//*if (role_[wChairID].userEnterScore < m_pGameRoom->GetLimitInMoney() * g_game_control.enterScoreCoefficient)
	{
		role_[wChairID].EnterScoreAddProb = g_game_control.enterAddProb;
	}*//*
	if (role_[wChairID].userEnterScore < m_pGame_Process->get_room_enter_limit() * g_game_control.enterScoreCoefficient)
	{
		role_[wChairID].EnterScoreAddProb = g_game_control.enterAddProb;
	}
}*/

void GameTableLogic::InitRoleInfo(unsigned short wChairID, bool bLookonUser)
{
	role_[wChairID].initialise(wChairID);
	bullet_manager_.delete_chair(wChairID);

	role_[wChairID].set_gold(m_pGame_Process->get_user_score(wChairID));
	role_[wChairID].set_exchange_gold(m_pGame_Process->get_user_score(wChairID));
	role_[wChairID].StartScore     = m_pGame_Process->get_user_score(wChairID);
	role_[wChairID].maxScore       = m_pGame_Process->get_user_score(wChairID);
	role_[wChairID].minScore       = m_pGame_Process->get_user_score(wChairID);
	role_[wChairID].FoceSwitch     = false;
	role_[wChairID].userEnterScore = m_pGame_Process->get_user_score(wChairID);

	if (role_[wChairID].userEnterScore < m_pGame_Process->get_room_enter_limit() * g_game_control.enterScoreCoefficient)
	{
		role_[wChairID].EnterScoreAddProb = g_game_control.enterAddProb;
	}
}

void GameTableLogic::UnInitRoleInfo(unsigned short wChairID, bool bLookonUser)
{
	unsigned short user_count = 0;
	user_count = m_pGame_Process->get_total_user_count();
	if (user_count == 0)
	{
		scene_ = 0;
	}

	/*int iBulletCount=bullet_manager_.get_count(wChairID);
	for(int i=0;i<iBulletCount;i++)
	{
		Bullet *bullet=  bullet_manager_.get_entity_Bullet(wChairID);
		if(bullet!=NULL)
		{
			int iMulriple =bullet->bullet_mulriple();
			m_pGame_Process->set_user_miss_stock(wChairID, iMulriple);
		}
	}*/

	uint32_t bullet_mulriple = bullet_manager_.get_player_all_bullet_mulriple(wChairID);
	m_pGame_Process->set_user_miss_stock(wChairID, bullet_mulriple);

	role_[wChairID].cleanup();
	bullet_manager_.delete_chair(wChairID);
}

//�û�����
void GameTableLogic::OnActionUserStandUp(unsigned short wChairID, bool bLookonUser)
{
	unsigned short user_count = 0;
	//unsigned short player_count = 0;
	/*for (unsigned short i = 0; i < GAME_PLAYER; ++i)
	{
		GamePlayer* user_item = m_pGameRoom->GetPlayer(i);
		if (user_item)
		{
			++user_count;
		}
	}*/
	user_count = m_pGame_Process->get_total_user_count();

 	if (user_count == 0)
	{
		//m_pGameRoom->m_pITableFrame->ConcludeGame(GAME_STATUS_FREE);
		scene_ = 0;
 	}
    
	/*int iBulletCount=bullet_manager_.get_count(wChairID);
	for(int i=0;i<iBulletCount;i++)
	{
      Bullet *bullet=  bullet_manager_.get_entity_Bullet(wChairID);
	  if(bullet!=NULL)
	  {
		  int iMulriple =bullet->bullet_mulriple();
	      //pGamePlayer->SetPlayerMissStock(iMulriple);
	      m_pGame_Process->set_user_miss_stock(wChairID, iMulriple);
	  }
	}*/

	uint32_t bullet_mulriple = bullet_manager_.get_player_all_bullet_mulriple(wChairID);
	m_pGame_Process->set_user_miss_stock(wChairID, bullet_mulriple);

	role_[wChairID].cleanup();
	bullet_manager_.delete_chair(wChairID);
	return;
}

//�û����� û�е�������
/*void GameTableLogic::OnActionUserOffLine(unsigned short wChairID, GamePlayer * pGamePlayer)
{

}*/

//ץ����
void GameTableLogic::OnCatchFish(unsigned short chair_id, CMD_C_Catch_Fish *cathFish)
{
	int chaid_id = chair_id;//pGamePlayer->GetSeat();

	//У������
	if (chaid_id < 0 || chaid_id >= GAME_PLAYER)
	{
		return;
	}

	//У����
	if (cathFish->fish_id_ < 0 || cathFish->fish_id_ > MAX_BIRD_IN_MANAGER)
	{
		return;
	}

	time_t nowTime     = Utility::GetTick();//GetTickCount();
	bool isAddFireProb = false;

	//���ڼ��Ĭ��Ϊ200.
	if (g_game_control.fireInterval > 10 && nowTime - role_[chaid_id].LastFireTime > g_game_control.fireInterval)
	{
		isAddFireProb = true;
	}
	
	//��¼���һ�ο���ʱ��
	role_[chaid_id].LastFireTime = nowTime;
	//ץ����
	role_[chaid_id].isCatchFish_ = true;
	//ץ�����ID
	role_[chaid_id].fish_id_     = cathFish->fish_id_; 

	//��������
	ProcessCatchFish(chair_id, cathFish->bullet_kind, cathFish->bullet_multiple, cathFish->bullet_id ,isAddFireProb);
}

void GameTableLogic::timeWriteScore()
{
	//����û��������
	/*for (int i = 0; i < GAME_PLAYER; i++)
	{
		GamePlayer* user_item = m_pGameRoom->GetPlayer(i);
		if (user_item != NULL)
		{
			char szFilename[256];
//			sprintf(szFilename, "E:\\yunding\\dntg\\%d_%d_%S.txt", m_pGameRoom->m_pGameServiceOption->wServerID, user_item->GetUserID(), CTime::GetCurrentTime().Format("%d"));
			FILE *fp = fopen(szFilename, "a");
			if (NULL == fp)
			{
				return;
			}
			fprintf(fp, "[%S]--%I64d\n", CTime::GetCurrentTime().Format("%H:%M:%S"), role_[i].get_gold());
			fclose(fp);
		}
	}*/
}


//�л�����
void GameTableLogic::ChangeScene()
{
	//��ʼ������
	timer_control_.scene_elapse_             = 0.f;
	timer_control_.bird_move_pause_elapse_   = 0.f;
	memset(timer_control_.distribute_elapsed_, 0.f, sizeof(timer_control_.distribute_elapsed_));
	timer_control_.group_bird_elapse_        = 0.f;
	timer_control_.special_bird_elapse_      = 0.f;
	timer_control_.bird_red_series_elapse_   = 0.f;
	timer_control_.bird_red_bloating_elapse_ = 0.f;
	timer_control_.bird_chain_elapse_        = 0.f;
	timer_control_.bird_ingot_elapse_        = 0.f;
	timer_control_.change_difficulty_elapse_ = 0.f;
	memset(timer_control_.bird_same_elapse_, 0.f, sizeof(timer_control_.bird_same_elapse_));

	scene_left_sended_ = false;
	scene_right_sended_ = false;

	scene_ = scene_ % MAX_SCENE_TYPE;

	scene_special_time_ = 40;
	if (scene_ % 2 == 0)
	{
		scene_special_time_ -= 5;
	}

	//bird_speed_up();

	//����עѪ
	//for (uint32_t i = 0; i < bird_manager_.get_count(); i++)
	//{
	//	Bird *pFish = bird_manager_.get_index_entity(i);
	//	if (pFish != NULL)
	//	{
	//		//����עѪ
	//		//RecovePlusScore(pFish, pFish->get_type());
	//	}
	//}
	//�����һ������
	bird_manager_.delete_all();
	//�л�����������ӵ��ˡ�
	//bullet_manager_.delete_all();
	birds_factory_.bird_clear();
	action_manager_.cleanup();
	birds_factory_.cleanup();
	action_manager_.initialise();

	S_ChangeScene cs;
	cs.scene_ = scene_;
	cs.scene_special_time_ = scene_special_time_;
	m_pGame_Process->broadcast_all(SUB_S_CHANGE_SCENE, &cs, sizeof(cs));

	/*SVar sSend;
	sSend["scene_"] = scene_;
	sSend["special_time_"] = scene_special_time_;
	m_pGameRoom->Bordcast_All(SUB_S_CHANGE_SCENE,&sSend);*/
}

void GameTableLogic::bird_speed_up()
{
	//�����ζ�
 	Bird* bird = NULL;
 	for (unsigned int i = 0; i < bird_manager_.get_count(); i++)
 	{
 		bird = bird_manager_.get_index_entity(i);
 		if (!bird || bird->get_index() == INVALID_ID)
		{
			//DBGPRINT("TableID=%d ** bird_speed_up() !bird_manager_.get_index_entity", m_pGameRoom->m_pITableFrame->GetTableID());
			continue;
		}
		Action_Interval * bird_act = ((Action_Interval*)action_manager_.get_action(bird->action_id()));
 		bird->set_speed(20);
 		bird_act->set_speed(20);
 	}
}

//��������
bool GameTableLogic::send_scene_yuzhen(Entity *target, uint32_t data)
{
	//uint16_t n =rand() % 11;
	//ż����BOSS����(�������ݲ�ʹ��)
	//if (scene_ % 2 == 0||n==3||n==7)
	//{
		uint16_t n = 11;
	//}
	switch (n)
	{
	case 0:
		{
			//����Բ����
			birds_factory_.create_scene_double_round();
			break;
		}
	//case 1:
	//	birds_factory_.create_scene_double_bloating();
	//	break;
	case 2:
		{
			//һ�������С��
			birds_factory_.create_scene_array_one();
			break;
		}
	case 3:
		{
			//һ�������С��
			birds_factory_.create_scene_array_one(false);
			break;
		}
	case 4:
		{
			//�����������뻷 ÿ����һ������
			birds_factory_.create_scene_array_two();
			break;
		}
	case 5:
		{
			//�����������뻷 ÿ����һ������
			birds_factory_.create_scene_array_two(false);
			break;
		}
	 case 6:
		{
			//�໷��
			birds_factory_.create_scene_array_three();
			break;
		}
	  case 7:
	    {
			//�໷��
			birds_factory_.create_scene_array_three(false);
			break;
		}
		
	//case 8:
	//	birds_factory_.create_scene_array_four();
	//	break;
	//case 9:
	//	birds_factory_.create_scene_bloating();
	//	break;
 	case 10:
		{
			//Բ����
 			birds_factory_.create_scene_round();
 	     	break;
		}
	default:
		{
			 //BOSS����
			birds_factory_.create_scene_boss_one();
			break;
		}
		
// 	case 10:
// 		birds_factory_.create_scene_array_five();
// 		break;
// 	case 11:
// 		birds_factory_.create_scene_array_six();
// 		break;
// 	case 12:
// 		birds_factory_.create_scene_array_seven();
// 		break;
	}

	//action_manager_.pause_target(target);
	//action_manager_.remove_all_action(target);
	//entity_manager_.delete_entity(target->get_id());
	scene_++;
	if (scene_ == MAX_SCENE_TYPE)
		scene_ = 0;
	return true;
}

void GameTableLogic::test_energy(uint16_t bird_type, unsigned short chair_id)
{

}

void GameTableLogic::send_scene_left_bird()
{
	birds_factory_.create_scene_left_bird();
}

void GameTableLogic::send_scene_right_bird()
{
	birds_factory_.create_scene_right_bird();
}

//void GameTableLogic::send_normal_bird()
//{
//	birds_factory_.create_normal_bird();
//}

//void GameTableLogic::send_small_bird()
//{
//	birds_factory_.create_small_bird();
//}

//void GameTableLogic::send_big_bird()
//{
//	birds_factory_.create_big_bird();
//}

//void GameTableLogic::send_small_clean_sweep_bird()
//{
//	birds_factory_.create_small_clean_sweep_bird();
//}

//void GameTableLogic::send_clean_sweep_bird()
//{
//	birds_factory_.create_clean_sweep_bird();
//}

//void GameTableLogic::send_huge_bird()
//{
//	birds_factory_.create_huge_bird();
//}

//����boss��
//void GameTableLogic::send_boss_bird()
//{
//	birds_factory_.create_boss_bird();
//}

//���������
void GameTableLogic::SendGroupFish()
{
	birds_factory_.create_group_bird();
}

//����������
void GameTableLogic::SendSpecialFish()
{
	birds_factory_.create_special_bird();
}

//���͵�����
void GameTableLogic::SendSingleFish(int bird_type)
{
	birds_factory_.create_onetype_bird(bird_type);
}

//�ĳ�����
void GameTableLogic::SendRedBloating()
{
	round_bird_linear* birds;

	BirdRedBloatingConfig bird_config = g_game_config_xml.bird_red_bloating_config_;
	//int count = Random(bird_config.ring_count_min, bird_config.ring_count_max);

	int count;
	if(bird_config.ring_count_min <= bird_config.ring_count_max)
	{
		std::uniform_int_distribution<> uid(bird_config.ring_count_min, bird_config.ring_count_max);
		count = uid(m_gen);
	}
	else
	{
		count = bird_config.ring_count_min;
	}

	int type = rand()%bird_config.bird_type_max;
//	bool is_create_red = bird_config.red_probability > rand()%100;
	Point center;
	center.x_ = rand()%(kScreenWidth-400) + 200;
	center.y_ = rand()%(kScreenHeight-400) + 200;

	Entity* entity;
	for (int i = 0; i < count; i++)
	{
		birds = new round_bird_linear;
		birds->bird_type = type;
		birds->bird_count = 30;
		birds->pt_center = center;
		birds->is_red = false;

		entity = entity_manager_.new_entity();

		if (entity)
		{
			Action* act = new Action_Sequence(new Action_Delay(2.5f * (i + 1)), new Action_FuncNDD(fastdelegate::MakeDelegate(this, &GameTableLogic::round_linear), birds), NULL);
			action_manager_.add_action(act, entity, false);
		}
	}
}

/*void GameTableLogic::load_stock_info(StockInfo &info)
{
	g_bystockconfig.SetCurValue(info.curStockValue);
	g_bystockconfig.SetValueA(info.valueA);
	g_bystockconfig.SetValueB(info.valueB);
	g_bystockconfig.SetValueC(info.valueC);
	g_bystockconfig.SetValueD(info.valueD);
    g_bystockconfig.SetRevenueCount(info.nRevenueCount);
}*/

//������
void GameTableLogic::SendFish()
{
	Bird bird;
	Bird *bird_in_manager;
	CMD_S_Send_Bird send_bird;

	if (birds_factory_.get_bird(&bird))
	{
		bird_in_manager = bird_manager_.new_entity();

		if (!bird_in_manager)
		{
			//SHOW("û���µ����������");
			//LOG(ERROR) << "û���µ����������";
			return;
		}

		bird_in_manager->set_item(bird.get_item());
		bird_in_manager->set_type(bird.get_type());
		bird_in_manager->set_path_id(bird.get_path_id());
		bird_in_manager->set_path_type(bird.get_path_type());
		bird_in_manager->set_path_delay(bird.get_path_delay());
		bird_in_manager->set_path_offset(bird.get_path_offset());
		bird_in_manager->set_speed(bird.get_speed());

		uint8_t type = bird.get_type();
		if (type >= MAX_BIRD_TYPE)
		{
			type = bird.get_item();
		}

		//bird_in_manager->set_mulriple(Random(g_game_config_xml.bird_config_[type].mulriple_min, g_game_config_xml.bird_config_[type].mulriple_max));

		uint32_t mulriple_random;
		if(g_game_config_xml.bird_config_[type].mulriple_min <= g_game_config_xml.bird_config_[type].mulriple_max)
		{
			std::uniform_int_distribution<> uid(g_game_config_xml.bird_config_[type].mulriple_min, g_game_config_xml.bird_config_[type].mulriple_max);
			mulriple_random = uid(m_gen);
		}
		else
		{
			mulriple_random = g_game_config_xml.bird_config_[type].mulriple_min;
		}

		bird_in_manager->set_mulriple(mulriple_random);
		bird_in_manager->set_position(Point(-500, -500));
        
        /*int64_t lAllStock=INSTANCE(CBYStockConfig)->GetCurrentAllStock();
		int64_t lStockA=INSTANCE(CBYStockConfig)->GetValueA();
		int64_t lStockB=INSTANCE(CBYStockConfig)->GetValueB();
		int64_t lStockC=INSTANCE(CBYStockConfig)->GetValueC();
		int64_t lStockD=INSTANCE(CBYStockConfig)->GetValueD();*/

		int64_t lAllStock=g_bystockconfig.GetCurValue();
		int64_t lStockA=g_bystockconfig.GetValueA();
		int64_t lStockB=g_bystockconfig.GetValueB();
		int64_t lStockC=g_bystockconfig.GetValueC();
		int64_t lStockD=g_bystockconfig.GetValueD();

        //���㵱ǰ����ϵ��
		int iFishDieRatio=bird_in_manager->get_mulriple();

		//ɱ��
        if(lAllStock<=lStockB)
		{
          iFishDieRatio= iFishDieRatio + ((lStockB-lAllStock)*iFishDieRatio)/ (lStockB-lStockA);
		  //SHOW("����ɱ�֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d  iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());

		  //����
		  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

		  if(0 <= iFishDieRatio-1)
		  {
			  std::uniform_int_distribution<> uid(0,iFishDieRatio-1);
			  int nRandNumber = uid(m_gen);
			  bird_in_manager->m_lRandNumber = nRandNumber;
		  }
		  else
		  {
			  bird_in_manager->m_lRandNumber = 0;
		  }

		  {
		  		//check
		  }
		}
		//�ŷ�
		else if(lAllStock>=lStockC)
		{
          iFishDieRatio= iFishDieRatio -((lAllStock-lStockC)*iFishDieRatio)/ (lStockD-lStockC);
		  //SHOW("�����ŷ֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
		  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

		  if(0 <= iFishDieRatio-1)
		  {
			  std::uniform_int_distribution<> uid(0,iFishDieRatio-1);
			  int nRandNumber = uid(m_gen);
			  bird_in_manager->m_lRandNumber = nRandNumber;
		  }
		  else
		  {
			  bird_in_manager->m_lRandNumber = 0;
		  }

		  {
		      //check
		      /*if(bird_in_manager->m_lRandNumber >= iFishDieRatio)
              {
                  LOG(ERROR) << "�ŷ�ʱ: " <<"fish_type: " << bird_in_manager->get_type()
                  << " m_lRandNumber: "<< bird_in_manager->m_lRandNumber << " m_iFishDieRatio: "<< iFishDieRatio;
              }
              if(bird_in_manager->m_lRandNumber >= bird_in_manager->get_mulriple())
			  {
				  LOG(ERROR) << "�ŷ�ʱ: " <<"fish_type: " << bird_in_manager->get_type()
							 << " m_lRandNumber: "<< bird_in_manager->m_lRandNumber << " mulriple_max: "<< bird_in_manager->get_mulriple();
			  }
			  if(iFishDieRatio > bird_in_manager->get_mulriple())
			  {
				  LOG(ERROR) << "�ŷ�ʱ: " <<"fish_type: " << bird_in_manager->get_type()
							 << " iFishDieRatio: "<< iFishDieRatio << " mulriple_max: "<< bird_in_manager->get_mulriple();
			  }*/
		  }
		}
		else
		{
/*			//check
			if(bird_in_manager->m_lRandNumber >= iFishDieRatio)
			{
				LOG(ERROR) << "fish_type: " << bird_in_manager->get_type() << " m_lRandNumber: "<< bird_in_manager->m_lRandNumber << " m_iFishDieRatio: "<< iFishDieRatio;
			}
			if(bird_in_manager->m_lRandNumber >= bird_in_manager->get_mulriple())
			{
				LOG(ERROR) << "fish_type: " << bird_in_manager->get_type() << "m_lRandNumber: " << bird_in_manager->m_lRandNumber << " mulriple_max:"<<bird_in_manager->get_mulriple();
			}*/
		}

		//���ò���ϵ��
        bird_in_manager->SetFishDieRatio(iFishDieRatio);

		if(iFishDieRatio > g_game_config_xml.bird_config_[type].mulriple_max)
		{
			g_bystockconfig.m_dwTodayXiFenCount++;
		}
		else if(iFishDieRatio < g_game_config_xml.bird_config_[type].mulriple_max)
		{
			g_bystockconfig.m_dwTodayRewardCount++;
		}

		//�������Ѫ��
		SetFishScore(bird_in_manager, type);

		Move_Points &move_points = path_manager_.get_paths(bird.get_path_id(), bird.get_path_type());
		Action *act = new Action_Sequence(new Action_Delay(bird.get_path_delay()), new Action_Move_Point(1.6, move_points, bird.get_path_offset()),
			new Action_Func1(fastdelegate::MakeDelegate(this, &GameTableLogic::func_bird_end), 0), 0);
		act->set_speed(bird.get_speed());
		action_manager_.add_action(act, bird_in_manager, false);
		bird_in_manager->set_action_id(act->get_tag());

		send_bird.id_          = bird_in_manager->get_id();
		send_bird.type_        = bird_in_manager->get_type();
		send_bird.item_        = bird_in_manager->get_item();
		send_bird.path_id_     = bird_in_manager->get_path_id();
		send_bird.path_type_   = bird_in_manager->get_path_type();
		send_bird.path_delay_  = bird_in_manager->get_path_delay();
		send_bird.path_offset_ = bird_in_manager->get_path_offset();
		send_bird.elapsed_     = 0;
		send_bird.speed_       = bird_in_manager->get_speed();
		send_bird.time_        = Utility::GetTick(); //GetTickCount();

		m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD, &send_bird, sizeof(send_bird));

		/*SVar sSend;
		sSend["id_"]           = send_bird.id_;
		sSend["type_"]         = send_bird.type_;
		sSend["item_"]         = send_bird.item_;
		sSend["path_id_"]      = send_bird.path_id_;
		sSend["path_type_"]    = send_bird.path_type_;
		sSend["path_delay_"]   = send_bird.path_delay_;
		sSend["path_offset_x"] = send_bird.path_offset_.x_;
		sSend["path_offset_y"] = send_bird.path_offset_.y_;
		sSend["elapsed_"]      = send_bird.elapsed_;
		sSend["speed_"]        = send_bird.speed_;
		m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD,&sSend);*/
	}
}

void GameTableLogic::send_bird_shape()
{
//	int packLen = 8*1024;
	Bird bird;
	Bird *bird_in_manager;
//	int index = 0;
	while(birds_factory_.get_shape_bird(&bird))
	{
 		bird_in_manager = bird_manager_.new_entity();
 
 		if (!bird_in_manager)
		{
			//SHOW("TableID=%d ** send_bird_shape() !bird_in_manager", m_pGameRoom->m_nRoomID);
			continue;
		}
 
 		bird_in_manager->set_item(bird.get_item());
 		bird_in_manager->set_type(bird.get_type());
 		bird_in_manager->set_path_id(bird.get_path_id());
 		bird_in_manager->set_path_type(bird.get_path_type());
 		bird_in_manager->set_path_delay(bird.get_path_delay());
 		bird_in_manager->set_path_offset(bird.get_path_offset());
 		bird_in_manager->set_speed(bird.get_speed());

		uint8_t type = bird.get_type();
		if (type >= MAX_BIRD_TYPE)
		{
			type = bird.get_item();
		}

		//bird_in_manager->set_mulriple(Random(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max));

		uint32_t  ml;
		if(g_game_config_xml.bird_config_[type].mulriple_min <= g_game_config_xml.bird_config_[type].mulriple_max)
		{
			std::uniform_int_distribution<> uid(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max);
			ml = uid(m_gen);
		}
		else
		{
			ml = g_game_config_xml.bird_config_[type].mulriple_min;
		}

		bird_in_manager->set_mulriple(ml);
 		bird_in_manager->set_position(Point(-500,-500));

        /*int64_t lAllStock=INSTANCE(CBYStockConfig)->GetCurrentAllStock();
		int64_t lStockA=INSTANCE(CBYStockConfig)->GetValueA();
		int64_t lStockB=INSTANCE(CBYStockConfig)->GetValueB();
		int64_t lStockC=INSTANCE(CBYStockConfig)->GetValueC();
		int64_t lStockD=INSTANCE(CBYStockConfig)->GetValueD();*/

		int64_t lAllStock=g_bystockconfig.GetCurValue();
		int64_t lStockA=g_bystockconfig.GetValueA();
		int64_t lStockB=g_bystockconfig.GetValueB();
		int64_t lStockC=g_bystockconfig.GetValueC();
		int64_t lStockD=g_bystockconfig.GetValueD();

        //���㵱ǰ����ϵ��
		int iFishDieRatio=bird_in_manager->get_mulriple();

		//ɱ��
        if(lAllStock<=lStockB)
		{
          iFishDieRatio= iFishDieRatio + ((lStockB-lAllStock)*iFishDieRatio)/ (lStockB-lStockA);
		  //SHOW("����ɱ�֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d  iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
          //����
		  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

		  if(0 <= iFishDieRatio-1)
		  {
			  std::uniform_int_distribution<> uid(0,iFishDieRatio-1);
			  bird_in_manager->m_lRandNumber = uid(m_gen);
		  }
		  else
		  {
			  bird_in_manager->m_lRandNumber = 0;
		  }
		}
		//�ŷ�
		else if(lAllStock>=lStockC)
		{
          iFishDieRatio= iFishDieRatio -((lAllStock-lStockC)*iFishDieRatio)/ (lStockD-lStockC);
		  //SHOW("�����ŷ֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
		  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

		  if(0 <= iFishDieRatio-1)
		  {
			  std::uniform_int_distribution<> uid(0,iFishDieRatio-1);
			  bird_in_manager->m_lRandNumber = uid(m_gen);
		  }
		  else
		  {
			  bird_in_manager->m_lRandNumber = 0;
		  }
		}
		//���ò���ϵ��
        bird_in_manager->SetFishDieRatio(iFishDieRatio);
		//�������Ѫ��
		SetFishScore(bird_in_manager, type);

 		Move_Points &move_points = path_manager_.get_paths(bird.get_path_id(), bird.get_path_type());
 		Action *act = new Action_Sequence(new Action_Delay(bird.get_path_delay()), new Action_Move_Point(0.26, move_points, bird.get_path_offset()),
 			new Action_Func1(fastdelegate::MakeDelegate(this, &GameTableLogic::func_bird_end), 0), 0);
 		act->set_speed(bird.get_speed());
 		action_manager_.add_action(act, bird_in_manager, false);
  		bird_in_manager->set_action_id(act->get_tag());

  		CMD_S_Send_Bird cssb;
  		cssb.id_ = bird_in_manager->get_id();
  		cssb.type_ = bird_in_manager->get_type();
  		cssb.item_ = bird_in_manager->get_item();
  		cssb.path_id_ = bird_in_manager->get_path_id();
  		cssb.path_type_ = bird_in_manager->get_path_type();
  		cssb.path_delay_ = bird_in_manager->get_path_delay();
  		cssb.path_offset_.x_ = bird_in_manager->get_path_offset().x_;
  		cssb.path_offset_.y_ = bird_in_manager->get_path_offset().y_;
  		cssb.elapsed_ = 0;
  		cssb.speed_ = bird_in_manager->get_speed();
  		cssb.time_ = Utility::GetTick();
  		m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD, &cssb, sizeof(CMD_S_Send_Bird));

		/*SVar sSend;
		sSend["id_"]            = (int64_t)bird_in_manager->get_id();
		sSend["type_"]          = bird_in_manager->get_type();
		sSend["item_"]          = bird_in_manager->get_item();
		sSend["path_id_"]       = bird_in_manager->get_path_id();
		sSend["path_type_"]     = bird_in_manager->get_path_type();
		sSend["path_delay_"]    = bird_in_manager->get_path_delay();
		sSend["path_offset_x"]  = bird_in_manager->get_path_offset().x_;
		sSend["path_offset_y"]  = bird_in_manager->get_path_offset().y_;
		sSend["elapsed_"]       = 0;
		sSend["speed_"]         = bird_in_manager->get_speed();
		sSend["time_"]          = (int64_t)GetTickCount();
		m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD,&sSend);*/
	}
}

void GameTableLogic::user_enter(unsigned int chair_id)
{
    m_b_send_boss[chair_id] = false;
	/*if(m_pGame_Process->get_user_id(chair_id) == 85514214)
	{
		LOG(ERROR) << "player enter:bird.size():" << birds_factory_.get_bird_count();
	}*/
}

void GameTableLogic::user_left(unsigned int chair_id)
{
	m_b_send_boss[chair_id] = false;
//	if(m_pGame_Process->get_user_id(chair_id) == 85514214)
//	{
//		LOG(ERROR) << "player left:bird.size():" << birds_factory_.get_bird_count();
//	}
}

// ����
void GameTableLogic::send_bird_linear()
{
	Bird bird;
//	uint16_t send_size = 0;

	while (birds_factory_.get_special_bird(&bird))
	{
        CMD_S_Send_Bird_Linear cssbl;
        cssbl.id_ = bird.get_id();
        cssbl.type_ = bird.get_type();
        cssbl.item_ = bird.get_item();
        cssbl.path_delay_ = bird.get_path_delay();
        cssbl.elapsed_ = 0;
        cssbl.speed_ = bird.get_speed();
        cssbl.start_.x_ = bird.get_start_pt().x_;
        cssbl.start_.y_ = bird.get_start_pt().y_;
        cssbl.end_.x_ = bird.get_linear_end_pt().x_;
        cssbl.end_.y_ = bird.get_linear_end_pt().y_;
        cssbl.time_ = Utility::GetTick();
        m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_LINEAR, &cssbl, sizeof(CMD_S_Send_Bird_Linear));

		/*SVar sSend;
		sSend["id_"]         = (int64_t)bird.get_id();
		sSend["type_"]       = bird.get_type();
		sSend["item_"]       = bird.get_item();
		sSend["path_delay_"] = bird.get_path_delay();
		sSend["elapsed_"]    = 0;
		sSend["speed_"]      = bird.get_speed();
		sSend["start_x"]     = bird.get_start_pt().x_;
		sSend["start_y"]     = bird.get_start_pt().y_;
		sSend["end_x"]       = bird.get_linear_end_pt().x_;
		sSend["end_y"]       = bird.get_linear_end_pt().y_;
		sSend["time_"]       = (int64_t)GetTickCount();
		m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD_LINEAR,&sSend);*/
	}
}

void GameTableLogic::send_special_birds_ex(unsigned short chair_id)
{
	Bird_Factory::Birds birds = birds_factory_.get_all_bird_1();
	Bird bird;
	for(Bird_Factory::Birds::iterator it = birds.begin(); it != birds.end(); it++)
	{
	    bird = *it;
		CMD_S_Send_Bird_Pause_Linear cssbpl;
		cssbpl.id_ = bird.get_id();
		cssbpl.type_ = bird.get_type();
		cssbpl.item_ = bird.get_item();
		cssbpl.path_delay_ = bird.get_path_delay();
		cssbpl.elapsed_ = 0;
		cssbpl.speed_ = bird.get_speed();
		cssbpl.pause_time_ = bird.get_pause_time();
		cssbpl.start_.x_ = bird.get_start_pt().x_;
		cssbpl.start_.y_ = bird.get_start_pt().y_;
		cssbpl.pause_.x_ = bird.get_pause_pt().x_;
		cssbpl.pause_.y_ = bird.get_pause_pt().y_;
		cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
		cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
		cssbpl.start_angle_ = bird.get_round_start_angle();
		cssbpl.time_ = Utility::GetTick();
		//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
		m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR,&cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	}

    birds = birds_factory_.get_all_bird_2();
    for(Bird_Factory::Birds::iterator it = birds.begin(); it != birds.end(); it++)
	{
        bird = *it;
		CMD_S_Send_Bird_Pause_Linear cssbpl;
		cssbpl.id_ = bird.get_id();
		cssbpl.type_ = bird.get_type();
		cssbpl.item_ = bird.get_item();
		cssbpl.path_delay_ = bird.get_path_delay();
		cssbpl.elapsed_ = 0;
		cssbpl.speed_ = bird.get_speed();
		cssbpl.pause_time_ = bird.get_pause_time();
		cssbpl.start_.x_ = bird.get_start_pt().x_;
		cssbpl.start_.y_ = bird.get_start_pt().y_;
		cssbpl.pause_.x_ = bird.get_pause_pt().x_;
		cssbpl.pause_.y_ = bird.get_pause_pt().y_;
		cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
		cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
		cssbpl.start_angle_ = bird.get_round_start_angle();
		cssbpl.time_ = Utility::GetTick();
		//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
		m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR,&cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	}

    birds = birds_factory_.get_all_bird_3();
    for(Bird_Factory::Birds::iterator it = birds.begin(); it != birds.end(); it++)
	{
        bird = *it;
		CMD_S_Send_Bird_Pause_Linear cssbpl;
		cssbpl.id_ = bird.get_id();
		cssbpl.type_ = bird.get_type();
		cssbpl.item_ = bird.get_item();
		cssbpl.path_delay_ = bird.get_path_delay();
		cssbpl.elapsed_ = 0;
		cssbpl.speed_ = bird.get_speed();
		cssbpl.pause_time_ = bird.get_pause_time();
		cssbpl.start_.x_ = bird.get_start_pt().x_;
		cssbpl.start_.y_ = bird.get_start_pt().y_;
		cssbpl.pause_.x_ = bird.get_pause_pt().x_;
		cssbpl.pause_.y_ = bird.get_pause_pt().y_;
		cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
		cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
		cssbpl.start_angle_ = bird.get_round_start_angle();
		cssbpl.time_ = Utility::GetTick();
		//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
		m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR,&cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	}

    birds = birds_factory_.get_all_bird_4();
    for(Bird_Factory::Birds::iterator it = birds.begin(); it != birds.end(); it++)
	{
        bird = *it;
		CMD_S_Send_Bird_Pause_Linear cssbpl;
		cssbpl.id_ = bird.get_id();
		cssbpl.type_ = bird.get_type();
		cssbpl.item_ = bird.get_item();
		cssbpl.path_delay_ = bird.get_path_delay();
		cssbpl.elapsed_ = 0;
		cssbpl.speed_ = bird.get_speed();
		cssbpl.pause_time_ = bird.get_pause_time();
		cssbpl.start_.x_ = bird.get_start_pt().x_;
		cssbpl.start_.y_ = bird.get_start_pt().y_;
		cssbpl.pause_.x_ = bird.get_pause_pt().x_;
		cssbpl.pause_.y_ = bird.get_pause_pt().y_;
		cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
		cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
		cssbpl.start_angle_ = bird.get_round_start_angle();
		cssbpl.time_ = Utility::GetTick();
		//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
		m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR,&cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	}

    birds = birds_factory_.get_all_bird_5();
    for(Bird_Factory::Birds::iterator it = birds.begin(); it != birds.end(); it++)
    {
        bird = *it;
		CMD_S_Send_Bird_Pause_Linear cssbpl;
		cssbpl.id_ = bird.get_id();
		cssbpl.type_ = bird.get_type();
		cssbpl.item_ = bird.get_item();
		cssbpl.path_delay_ = bird.get_path_delay();
		cssbpl.elapsed_ = 0;
		cssbpl.speed_ = bird.get_speed();
		cssbpl.pause_time_ = bird.get_pause_time();
		cssbpl.start_.x_ = bird.get_start_pt().x_;
		cssbpl.start_.y_ = bird.get_start_pt().y_;
		cssbpl.pause_.x_ = bird.get_pause_pt().x_;
		cssbpl.pause_.y_ = bird.get_pause_pt().y_;
		cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
		cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
		cssbpl.start_angle_ = bird.get_round_start_angle();
		cssbpl.time_ = Utility::GetTick();
		//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
		m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR,&cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	}

    birds = birds_factory_.get_all_bird_6();
    for(Bird_Factory::Birds::iterator it = birds.begin(); it != birds.end(); it++)
    {
        bird = *it;
		CMD_S_Send_Bird_Pause_Linear cssbpl;
		cssbpl.id_ = bird.get_id();
		cssbpl.type_ = bird.get_type();
		cssbpl.item_ = bird.get_item();
		cssbpl.path_delay_ = bird.get_path_delay();
		cssbpl.elapsed_ = 0;
		cssbpl.speed_ = bird.get_speed();
		cssbpl.pause_time_ = bird.get_pause_time();
		cssbpl.start_.x_ = bird.get_start_pt().x_;
		cssbpl.start_.y_ = bird.get_start_pt().y_;
		cssbpl.pause_.x_ = bird.get_pause_pt().x_;
		cssbpl.pause_.y_ = bird.get_pause_pt().y_;
		cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
		cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
		cssbpl.start_angle_ = bird.get_round_start_angle();
		cssbpl.time_ = Utility::GetTick();
		//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
		m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR,&cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	}

    birds = birds_factory_.get_all_bird_7();
    for(Bird_Factory::Birds::iterator it = birds.begin(); it != birds.end(); it++)
    {
        bird = *it;
		CMD_S_Send_Bird_Pause_Linear cssbpl;
		cssbpl.id_ = bird.get_id();
		cssbpl.type_ = bird.get_type();
		cssbpl.item_ = bird.get_item();
		cssbpl.path_delay_ = bird.get_path_delay();
		cssbpl.elapsed_ = 0;
		cssbpl.speed_ = bird.get_speed();
		cssbpl.pause_time_ = bird.get_pause_time();
		cssbpl.start_.x_ = bird.get_start_pt().x_;
		cssbpl.start_.y_ = bird.get_start_pt().y_;
		cssbpl.pause_.x_ = bird.get_pause_pt().x_;
		cssbpl.pause_.y_ = bird.get_pause_pt().y_;
		cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
		cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
		cssbpl.start_angle_ = bird.get_round_start_angle();
		cssbpl.time_ = Utility::GetTick();
		//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
		m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR,&cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	}
}

void GameTableLogic::send_special_boss(unsigned short chair_id, Bird &bird)
{
	CMD_S_Send_Bird_Pause_Linear cssbpl;
	cssbpl.id_ = bird.get_id();
	cssbpl.type_ = bird.get_type();
	cssbpl.item_ = bird.get_item();
	cssbpl.path_delay_ = bird.get_path_delay();
	cssbpl.elapsed_ = 0;
	cssbpl.speed_ = bird.get_speed();
	cssbpl.pause_time_ = bird.get_pause_time();
	cssbpl.start_.x_ = bird.get_start_pt().x_;
	cssbpl.start_.y_ = bird.get_start_pt().y_;
	cssbpl.pause_.x_ = bird.get_pause_pt().x_;
	cssbpl.pause_.y_ = bird.get_pause_pt().y_;
	cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
	cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
	cssbpl.start_angle_ = bird.get_round_start_angle();
	cssbpl.time_ = Utility::GetTick();
	//m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
	m_pGame_Process->send_player_data(chair_id,SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
}

void GameTableLogic::send_special_birds(unsigned int birds_no)
{
	Bird bird;
	if(birds_no == 1)
	{
		birds_factory_.get_special_bird_1(&bird);
	}
	else if(birds_no == 2)
	{
		birds_factory_.get_special_bird_2(&bird);
	}
	else if(birds_no == 3)
	{
		birds_factory_.get_special_bird_3(&bird);
	}
	else if(birds_no == 4)
	{
		birds_factory_.get_special_bird_4(&bird);
	}
	else if(birds_no == 5)
	{
		birds_factory_.get_special_bird_5(&bird);
	}
	else if(birds_no == 6)
	{
		birds_factory_.get_special_bird_6(&bird);
	}
	else if(birds_no == 7)
    {
        birds_factory_.get_special_birds_boss(&bird);
    }
	else
	{
		LOG(ERROR) << "birds_no: "<<birds_no <<" error";
		return;
	}

	CMD_S_Send_Bird_Pause_Linear cssbpl;
	cssbpl.id_ = bird.get_id();
	cssbpl.type_ = bird.get_type();
	cssbpl.item_ = bird.get_item();
	cssbpl.path_delay_ = bird.get_path_delay();
	cssbpl.elapsed_ = 0;
	cssbpl.speed_ = bird.get_speed();
	cssbpl.pause_time_ = bird.get_pause_time();
	cssbpl.start_.x_ = bird.get_start_pt().x_;
	cssbpl.start_.y_ = bird.get_start_pt().y_;
	cssbpl.pause_.x_ = bird.get_pause_pt().x_;
	cssbpl.pause_.y_ = bird.get_pause_pt().y_;
	cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
	cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
	cssbpl.start_angle_ = bird.get_round_start_angle();
	cssbpl.time_ = Utility::GetTick();
	m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));
}

void GameTableLogic::send_bird_pause_linear()
{
	Bird bird;
	while (birds_factory_.get_special_bird(&bird))
	{
        CMD_S_Send_Bird_Pause_Linear cssbpl;
        cssbpl.id_ = bird.get_id();
        cssbpl.type_ = bird.get_type();
        cssbpl.item_ = bird.get_item();
        cssbpl.path_delay_ = bird.get_path_delay();
        cssbpl.elapsed_ = 0;
        cssbpl.speed_ = bird.get_speed();
        cssbpl.pause_time_ = bird.get_pause_time();
        cssbpl.start_.x_ = bird.get_start_pt().x_;
        cssbpl.start_.y_ = bird.get_start_pt().y_;
        cssbpl.pause_.x_ = bird.get_pause_pt().x_;
        cssbpl.pause_.y_ = bird.get_pause_pt().y_;
        cssbpl.end_.x_ = bird.get_linear_end_pt().x_;
        cssbpl.end_.y_ = bird.get_linear_end_pt().y_;
        cssbpl.start_angle_ = bird.get_round_start_angle();
        cssbpl.time_ = Utility::GetTick();
        m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_PAUSE_LINEAR, &cssbpl, sizeof(CMD_S_Send_Bird_Pause_Linear));

		/*SVar sSend;
		sSend["id_"]          = (int64_t)bird.get_id();
		sSend["type_"]        = bird.get_type();
		sSend["item_"]        = bird.get_item();
		sSend["path_delay_"]  = bird.get_path_delay();
		sSend["elapsed_"]     = 0;
		sSend["speed_"]       = bird.get_speed();
		sSend["pause_time_"]  = bird.get_pause_time();
		sSend["start_x"]      = bird.get_start_pt().x_;
		sSend["start_y"]      = bird.get_start_pt().y_;
		sSend["pause_x"]      = bird.get_pause_pt().x_;
		sSend["pause_y"]      = bird.get_pause_pt().y_;
		sSend["end_x"]        = bird.get_linear_end_pt().x_;
		sSend["end_y"]        = bird.get_linear_end_pt().y_;
		sSend["start_angle_"] = bird.get_round_start_angle();
		sSend["time_"]        =(int64_t)GetTickCount();
		m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD_PAUSE_LINEAR,&sSend);*/
	}
}

void GameTableLogic::send_bird_round()
{
	Bird bird;
//	uint16_t send_size = 0;

	while (birds_factory_.get_special_bird(&bird))
	{
		CMD_S_Send_Bird_Round cssbr;
		cssbr.id_ = bird.get_id();
		cssbr.type_ = bird.get_type();
		cssbr.item_ = bird.get_item();
		cssbr.path_delay_ = bird.get_path_delay();
		cssbr.elapsed_ = 0;
		cssbr.speed_ = bird.get_speed();
		cssbr.center_.x_ = bird.get_start_pt().x_;
		cssbr.center_.y_ = bird.get_start_pt().y_;
		cssbr.radius_ = bird.get_round_radius();
		cssbr.rotate_duration_ = bird.get_round_rotate_duration();
		cssbr.start_angle_ = bird.get_round_start_angle();
		cssbr.rotate_angle_ = bird.get_round_rotate_angle();
		cssbr.move_duration_ = bird.get_round_move_duration();
		cssbr.time_ = Utility::GetTick();
		m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_ROUND, &cssbr, sizeof(CMD_S_Send_Bird_Round));

		/*SVar sSend;
		sSend["id_"] = (int64_t)bird.get_id();
		sSend["type_"] = bird.get_type();
		sSend["item_"] = bird.get_item();
		sSend["path_delay_"] = bird.get_path_delay();
		sSend["elapsed_"] = 0;
		sSend["speed_"] = bird.get_speed();
		sSend["center_x"] = bird.get_start_pt().x_;
		sSend["center_y"] = bird.get_start_pt().y_;
		sSend["radius_"] = bird.get_round_radius();
		sSend["rotate_duration_"] = bird.get_round_rotate_duration();
		sSend["start_angle_"] = bird.get_round_start_angle();
		sSend["rotate_angle_"] = bird.get_round_rotate_angle();
		sSend["move_duration_"] = bird.get_round_move_duration();
		sSend["time_"] =(int64_t)GetTickCount();
		m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD_ROUND,&sSend);*/
	}
}

void GameTableLogic::send_scene_bloating()
{
	round_bird_linear* birds;
	const int kBirdCount[14] = { 70, 65, 60, 55, 50, 50, 40, 45, 45, 40, 30, 30, 25, 20 };
	const Point center(kScreenWidth / 2.0f, kScreenHeight / 2.f);
	Entity* entity;
	
	//int nRed = rand()%10;
	for (int i = 0; i < 14; i++)
	{
		birds = new round_bird_linear;
		birds->bird_type = BIRD_TYPE_0 + i;
		birds->bird_count = kBirdCount[i];
		birds->pt_center = center;
		birds->is_red = false;

		entity = entity_manager_.new_entity();

		if(entity)
		{
			Action* act = new Action_Sequence(new Action_Delay(2.5f * (i + 1)), new Action_FuncNDD(fastdelegate::MakeDelegate(this, &GameTableLogic::round_linear), birds), NULL);
			action_manager_.add_action(act, entity, false);
		}
	}
}

void GameTableLogic::send_scene_double_bloating()
{
	round_bird_linear* birds;
	const int kBirdCount[14] = { 35, 32, 30, 27, 25, 25, 20, 22, 22, 20, 15, 15, 22, 10 };
	const Point center_left(kScreenWidth / 4.0f, kScreenHeight / 2.f);
	const Point center_right(kScreenWidth / 4.0f * 3, kScreenHeight / 2.f);
	Entity* entity;

	for (int i = 0; i < 14; i++)
	{
		birds = new round_bird_linear;
		birds->bird_type = BIRD_TYPE_0 + i;
		birds->bird_count = kBirdCount[i];
		birds->pt_center = center_left;
		birds->is_red = false;

		entity = entity_manager_.new_entity();

		if (entity)
		{
			Action* act = new Action_Sequence(new Action_Delay(2.5f * (i + 1)), new Action_FuncNDD(fastdelegate::MakeDelegate(this, &GameTableLogic::round_linear), birds), NULL);
			action_manager_.add_action(act, entity, false);
		}
	}

	for (int i = 0; i < 14; i++)
	{
		birds = new round_bird_linear;
		birds->bird_type = BIRD_TYPE_0 + i;
		birds->bird_count = kBirdCount[i];
		birds->pt_center = center_right;
		birds->is_red = false;

		entity = entity_manager_.new_entity();

		if (entity)
		{
			Action* act = new Action_Sequence(new Action_Delay(2.5f * (i + 1)), new Action_FuncNDD(fastdelegate::MakeDelegate(this, &GameTableLogic::round_linear), birds), NULL);
			action_manager_.add_action(act, entity, false);
		}
	}
}

bool GameTableLogic::round_linear(Entity *target, void* data)
{
	if (bird_move_pause_is_ || timer_control_.scene_elapse_ > g_game_config_xml.scene_swith_time_ - 15)
		return false;

	round_bird_linear* birds = (round_bird_linear*)data;
	Bird bird;
	Action_Bird_Move* action = NULL;
	float cell_radian = 2 * M_PI / birds->bird_count;
	const float kRadius = 1800.f;
	Point pt;

	for (int i = 0; i < birds->bird_count; i++)
	{
		bird.set_id(INVALID_ID);
		bird.set_item(-1);
		bird.set_type(birds->bird_type);
		bird.set_speed(80);
		pt.x_ = birds->pt_center.x_ + kRadius * std::cos(i * cell_radian);
		pt.y_ = birds->pt_center.y_ + kRadius * std::sin(i * cell_radian);
		action = new Action_Bird_Move_Linear(bird.get_speed(), birds->pt_center, pt);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(birds->pt_center);
		bird.set_linear_end_pt(pt);
		bird.set_yuzhen_type(YUZHEN_LINEAR);
		build_scene_bird(bird, action);
	}

	//����
	delete birds;
	action_manager_.pause_target(target);
	action_manager_.remove_all_action(target);
	entity_manager_.delete_entity(target->get_id());

	//����������� ��ͻ��˷���
	send_bird_linear();

	return true;
}

bool GameTableLogic::build_bird_common(Bird &bird, Action_Bird_Move *action)
{
	Bird* bird_in_manager = bird_manager_.new_entity();
	if (!bird_in_manager)
	{
		//DBGPRINT("TableID=%d ** build_scene_bird() !bird_in_manager", m_pGameRoom->m_pITableFrame->GetTableID());
		return false;
	}

	bird_in_manager->set_item(bird.get_item());
	bird_in_manager->set_type(bird.get_type());
	bird_in_manager->set_position(bird.position());
	bird_in_manager->set_start_pt(bird.get_start_pt());
	bird_in_manager->set_linear_end_pt(bird.get_linear_end_pt());
	bird_in_manager->set_yuzhen_type(bird.get_yuzhen_type());
	bird_in_manager->set_round_radius(bird.get_round_radius());
	bird_in_manager->set_round_rotate_duration(bird.get_round_rotate_duration());
	bird_in_manager->set_round_start_angle(bird.get_round_start_angle());
	bird_in_manager->set_round_rotate_angle(bird.get_round_rotate_angle());
	bird_in_manager->set_round_move_duration(bird.get_round_move_duration());
	bird_in_manager->set_speed(bird.get_speed());
	bird_in_manager->set_action_id(action->get_tag());

	uint8_t type = bird.get_type();
	if (type > MAX_BIRD_TYPE)
	{
		type = bird.get_item();
	}

	//bird_in_manager->set_mulriple(Random(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max));

	uint32_t ml;
	if(g_game_config_xml.bird_config_[type].mulriple_min <= g_game_config_xml.bird_config_[type].mulriple_max)
	{
		std::uniform_int_distribution<> uid(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max);
		ml = uid(m_gen);
	}
	else
	{
		ml = g_game_config_xml.bird_config_[type].mulriple_min;
	}

	bird_in_manager->set_mulriple(ml);

	/*int64_t lAllStock=INSTANCE(CBYStockConfig)->GetCurrentAllStock();
    int64_t lStockA=INSTANCE(CBYStockConfig)->GetValueA();
    int64_t lStockB=INSTANCE(CBYStockConfig)->GetValueB();
    int64_t lStockC=INSTANCE(CBYStockConfig)->GetValueC();
    int64_t lStockD=INSTANCE(CBYStockConfig)->GetValueD();*/

	int64_t lAllStock=g_bystockconfig.GetCurValue();
	int64_t lStockA=g_bystockconfig.GetValueA();
	int64_t lStockB=g_bystockconfig.GetValueB();
	int64_t lStockC=g_bystockconfig.GetValueC();
	int64_t lStockD=g_bystockconfig.GetValueD();

	//���㵱ǰ����ϵ��
	int iFishDieRatio=bird_in_manager->get_mulriple();

	//ɱ��
	if(lAllStock<=lStockB)
	{
		iFishDieRatio= iFishDieRatio + ((lStockB-lAllStock)*iFishDieRatio)/ (lStockB-lStockA);
		//SHOW("����ɱ�֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d  iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
		//����
		//bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);
		if(0 <= iFishDieRatio-1)
		{
			std::uniform_int_distribution<> dis(0,iFishDieRatio-1);
			bird_in_manager->m_lRandNumber = dis(m_gen);
		}
		else
		{
			bird_in_manager->m_lRandNumber = 0;
		}
	}
		//�ŷ�
	else if(lAllStock>=lStockC)
	{
		iFishDieRatio= iFishDieRatio -((lAllStock-lStockC)*iFishDieRatio)/ (lStockD-lStockC);
		//SHOW("�����ŷ֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
		//bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

		if(0 <= iFishDieRatio-1)
		{
			std::uniform_int_distribution<> dis(0,iFishDieRatio-1);
			bird_in_manager->m_lRandNumber = dis(m_gen);
		}
		else
		{
			bird_in_manager->m_lRandNumber = 0;
		}
	}
	//���ò���ϵ��
	bird_in_manager->SetFishDieRatio(iFishDieRatio);
	//�������Ѫ��
	SetFishScore(bird_in_manager, type);

	Action* act = new Action_Sequence(new Action_Delay(bird.get_path_delay()), action, new Action_Func1(fastdelegate::MakeDelegate(this, &GameTableLogic::func_bird_end), 0), 0);
	act->set_speed(1);
	//act->set_speed(bird.get_speed());
	action_manager_.add_action(act, bird_in_manager, false);
	bird_in_manager->set_action_id(act->get_tag());

	//���Ӵ�������
	bird.set_id(bird_in_manager->get_id());
	//birds_factory_.add_special_birds(bird);

	return true;
}

bool GameTableLogic::build_scene_bird(Bird bird, Action_Bird_Move* action)
{
	Bird* bird_in_manager = bird_manager_.new_entity();
	if (!bird_in_manager) 
	{
		//DBGPRINT("TableID=%d ** build_scene_bird() !bird_in_manager", m_pGameRoom->m_pITableFrame->GetTableID());
		return false;
	}

	bird_in_manager->set_item(bird.get_item());
	bird_in_manager->set_type(bird.get_type());
	bird_in_manager->set_position(bird.position());
	bird_in_manager->set_start_pt(bird.get_start_pt());
	bird_in_manager->set_linear_end_pt(bird.get_linear_end_pt());
	bird_in_manager->set_yuzhen_type(bird.get_yuzhen_type());
	bird_in_manager->set_round_radius(bird.get_round_radius());
	bird_in_manager->set_round_rotate_duration(bird.get_round_rotate_duration());
	bird_in_manager->set_round_start_angle(bird.get_round_start_angle());
	bird_in_manager->set_round_rotate_angle(bird.get_round_rotate_angle());
	bird_in_manager->set_round_move_duration(bird.get_round_move_duration());
	bird_in_manager->set_speed(bird.get_speed());
	bird_in_manager->set_action_id(action->get_tag());

	uint8_t type = bird.get_type();
	if (type > MAX_BIRD_TYPE)
	{
		type = bird.get_item();
	}

	//bird_in_manager->set_mulriple(Random(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max));
	uint32_t  ml;
	if(g_game_config_xml.bird_config_[type].mulriple_min <= g_game_config_xml.bird_config_[type].mulriple_max)
	{
		std::uniform_int_distribution<> uid(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max);
		ml = uid(m_gen);
	}
	else
	{
		ml = g_game_config_xml.bird_config_[type].mulriple_min;
	}

	bird_in_manager->set_mulriple(ml);

    /*int64_t lAllStock=INSTANCE(CBYStockConfig)->GetCurrentAllStock();
	int64_t lStockA=INSTANCE(CBYStockConfig)->GetValueA();
	int64_t lStockB=INSTANCE(CBYStockConfig)->GetValueB();
	int64_t lStockC=INSTANCE(CBYStockConfig)->GetValueC();
	int64_t lStockD=INSTANCE(CBYStockConfig)->GetValueD();*/

	int64_t lAllStock=g_bystockconfig.GetCurValue();
	int64_t lStockA=g_bystockconfig.GetValueA();
	int64_t lStockB=g_bystockconfig.GetValueB();
	int64_t lStockC=g_bystockconfig.GetValueC();
	int64_t lStockD=g_bystockconfig.GetValueD();

    //���㵱ǰ����ϵ��
	int iFishDieRatio=bird_in_manager->get_mulriple();

	//ɱ��
    if(lAllStock<=lStockB)
	{
      iFishDieRatio= iFishDieRatio + ((lStockB-lAllStock)*iFishDieRatio)/ (lStockB-lStockA);
	  //SHOW("����ɱ�֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d  iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
      //����
	  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

	  if(0 <= iFishDieRatio-1)
	  {
		  std::uniform_int_distribution<> dis(0,iFishDieRatio-1);
		  bird_in_manager->m_lRandNumber = dis(m_gen);
	  }
	  else
	  {
		  bird_in_manager->m_lRandNumber = 0;
	  }
	}
	//�ŷ�
	else if(lAllStock>=lStockC)
	{
      iFishDieRatio= iFishDieRatio -((lAllStock-lStockC)*iFishDieRatio)/ (lStockD-lStockC);
	  //SHOW("�����ŷ֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
	  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

	  if(0 <= iFishDieRatio-1)
	  {
		  std::uniform_int_distribution<> dis(0,iFishDieRatio-1);
		  bird_in_manager->m_lRandNumber = dis(m_gen);
	  }
	  else
	  {
		  bird_in_manager->m_lRandNumber = 0;
	  }
	}
	//���ò���ϵ��
    bird_in_manager->SetFishDieRatio(iFishDieRatio);
	//�������Ѫ��
	SetFishScore(bird_in_manager, type);

	Action* act = new Action_Sequence(new Action_Delay(bird.get_path_delay()), action, new Action_Func1(fastdelegate::MakeDelegate(this, &GameTableLogic::func_bird_end), 0), 0);
	act->set_speed(1);
	//act->set_speed(bird.get_speed());
	action_manager_.add_action(act, bird_in_manager, false);
	bird_in_manager->set_action_id(act->get_tag());

	//���Ӵ�������
	bird.set_id(bird_in_manager->get_id());
	birds_factory_.add_special_birds(bird);

	
	return true;
}

void GameTableLogic::send_scene_array_four()
{
	Entity* entity;
	for (int i = 0; i <= 8; i++)
	{
		entity = entity_manager_.new_entity();
		if (entity)
		{
			Action* act = new Action_Sequence(new Action_Delay(4.0f * (i + 1)), new Action_Func1(fastdelegate::MakeDelegate(this, &GameTableLogic::up_down_linear), i%4), NULL);
			action_manager_.add_action(act, entity, false);
		}
	}
}

bool GameTableLogic::up_down_linear(Entity *target, uint32_t data)
{
	//��������
	float cell_interval = 0.f;			//���
	float speed = 80.f;					//�ٶ�
	float vertical_end = 0.f;			//�������λ��
	Point pt_start(0.f, 0.f);			//��ʼλ��
	Point pt_end(0.f, 0.f);				//����λ��
	int bird_count = 40;

	//����
	int nRed = 0;

	Action_Bird_Move* action = NULL;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_LINEAR);
	bird.set_speed(speed);

	cell_interval = kScreenWidth / bird_count;

	//����
	nRed = rand()%bird_count;
 	vertical_end = kScreenHeight + 300;
 	for (int i = 0; i < bird_count; i++)
 	{
 		pt_start.x_ = i * cell_interval + 25;
 		pt_start.y_ = 0.f - 300;
 		pt_end.x_ = pt_start.x_;
 		pt_end.y_ = vertical_end;
 		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_start_pt(pt_start);
 		bird.set_linear_end_pt(pt_end);
		bird.set_type(data);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/
 
 		build_scene_bird(bird, action);
 	}

  	//����
	nRed = rand()%bird_count;
  	vertical_end = 0.f - 300;
  	for (int i = 0; i < bird_count; i++)
  	{
  		pt_start.x_ = i * cell_interval + 25;
  		pt_start.y_ = kScreenHeight + 300;
  		pt_end.x_ = pt_start.x_;
  		pt_end.y_ = vertical_end;
  		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
  		bird.set_position(action->start_position());
  		bird.set_path_delay(0);
  		bird.set_start_pt(pt_start);
  		bird.set_linear_end_pt(pt_end);
		bird.set_type(data + 4);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/
  
  		build_scene_bird(bird, action);
  	}

	send_bird_linear();

	action_manager_.pause_target(target);
	action_manager_.remove_all_action(target);
	entity_manager_.delete_entity(target->get_id());

	return true;
}

void GameTableLogic::send_bird_small_clean_sweep()
{
	Bird bird;
	Bird *bird_in_manager;
	CMD_S_Send_Bird send_bird;

	if(birds_factory_.get_small_clean_sweep_bird(&bird))
	{
		bird_in_manager = bird_manager_.new_entity();

		if (!bird_in_manager) 
		{
			//DBGPRINT("TableID=%d ** send_bird_small_clean_sweep() !bird_in_manager", m_pGameRoom->m_pITableFrame->GetTableID());
			return;
		}

		bird_in_manager->set_item(bird.get_item());
		bird_in_manager->set_type(bird.get_type());
		bird_in_manager->set_path_id(bird.get_path_id());
		bird_in_manager->set_path_type(bird.get_path_type());
		bird_in_manager->set_path_delay(bird.get_path_delay());
		bird_in_manager->set_path_offset(bird.get_path_offset());
		bird_in_manager->set_speed(bird.get_speed());

		uint8_t type = bird.get_type();
		if (type >= MAX_BIRD_TYPE)
		{
			type = bird.get_item();
		}

		//bird_in_manager->set_mulriple(Random(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max));
		uint32_t  ml;
		if(g_game_config_xml.bird_config_[type].mulriple_min <= g_game_config_xml.bird_config_[type].mulriple_max)
		{
			std::uniform_int_distribution<> dis(g_game_config_xml.bird_config_[type].mulriple_min,g_game_config_xml.bird_config_[type].mulriple_max);
			ml = dis(m_gen);
		}
		else
		{
			ml = g_game_config_xml.bird_config_[type].mulriple_min;
		}

		bird_in_manager->set_mulriple(ml);
		bird_in_manager->set_position(Point(-500,-500));
        
//		int64_t lAllStock=INSTANCE(CBYStockConfig)->GetCurrentAllStock();
//		int64_t lStockA=INSTANCE(CBYStockConfig)->GetValueA();
//		int64_t lStockB=INSTANCE(CBYStockConfig)->GetValueB();
//		int64_t lStockC=INSTANCE(CBYStockConfig)->GetValueC();
//		int64_t lStockD=INSTANCE(CBYStockConfig)->GetValueD();
		int64_t lAllStock=g_bystockconfig.GetCurValue();
		int64_t lStockA=g_bystockconfig.GetValueA();
		int64_t lStockB=g_bystockconfig.GetValueB();
		int64_t lStockC=g_bystockconfig.GetValueC();
		int64_t lStockD=g_bystockconfig.GetValueD();

        //���㵱ǰ����ϵ��
		int iFishDieRatio=bird_in_manager->get_mulriple();

		//ɱ��
        if(lAllStock<=lStockB)
		{
          iFishDieRatio= iFishDieRatio + ((lStockB-lAllStock)*iFishDieRatio)/ (lStockB-lStockA);
		  //SHOW("����ɱ�֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d  iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
          //����
		  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

		  if(0 <= iFishDieRatio-1)
		  {
			  std::uniform_int_distribution<> dis(0,iFishDieRatio-1);
			  bird_in_manager->m_lRandNumber = dis(m_gen);
		  }
		  else
		  {
			  bird_in_manager->m_lRandNumber = 0;
		  }
		}
		//�ŷ�
		else if(lAllStock>=lStockC)
		{
          iFishDieRatio= iFishDieRatio -((lAllStock-lStockC)*iFishDieRatio)/ (lStockD-lStockC);
		  //SHOW("�����ŷ֣���ǰ���;%I64d A;%I64d B;%I64d C;%I64d D;%I64d iFishDieRatio:%d mulriple:%d ",lAllStock,lStockA,lStockB,lStockC,lStockD,iFishDieRatio,bird_in_manager->get_mulriple());
		  //bird_in_manager->m_lRandNumber =Random(0,iFishDieRatio-1);

		  if(0 <= iFishDieRatio-1)
		  {
			  std::uniform_int_distribution<> dis(0,iFishDieRatio-1);
			  bird_in_manager->m_lRandNumber = dis(m_gen);
		  }
		  else
		  {
			  bird_in_manager->m_lRandNumber = 0;
		  }
		}
		//���ò���ϵ��
        bird_in_manager->SetFishDieRatio(iFishDieRatio);
		//�������Ѫ��
		SetFishScore(bird_in_manager, type);

		Move_Points &move_points = path_manager_.get_paths(bird.get_path_id(), bird.get_path_type());
		Action *act = new Action_Sequence(new Action_Delay(bird.get_path_delay()), new Action_Move_Point(0.26, move_points, bird.get_path_offset()),
			new Action_Func1(fastdelegate::MakeDelegate(this, &GameTableLogic::func_bird_end), 0), 0);
		action_manager_.add_action(act, bird_in_manager, false);

		bird_in_manager->set_action_id(act->get_tag());

		send_bird.id_ = bird_in_manager->get_id();
		send_bird.type_ = bird_in_manager->get_type();
		send_bird.item_ = bird_in_manager->get_item();
		send_bird.path_id_ = bird_in_manager->get_path_id();
		send_bird.path_type_ = bird_in_manager->get_path_type();
		send_bird.path_delay_= bird_in_manager->get_path_delay();
		send_bird.path_offset_ =  bird_in_manager->get_path_offset();
		send_bird.elapsed_ = 0; 
		send_bird.speed_ = bird_in_manager->get_speed();
		send_bird.time_ = Utility::GetTick();//GetTickCount();

		CMD_S_Send_Bird cssb;
		cssb.id_ = bird_in_manager->get_id();
		cssb.type_ = bird_in_manager->get_type();
		cssb.item_ = bird_in_manager->get_item();
		cssb.path_id_ = bird_in_manager->get_path_id();
		cssb.path_type_ = bird_in_manager->get_path_type();
		cssb.path_delay_ = bird_in_manager->get_path_delay();
		cssb.path_offset_ = bird_in_manager->get_path_offset();
		cssb.elapsed_ = 0;
		cssb.speed_ = bird_in_manager->get_speed();
		cssb.time_ = Utility::GetTick();
		m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD, &cssb, sizeof(CMD_S_Send_Bird));

		/*SVar sSend;
		sSend["id_"] = (int64_t)bird_in_manager->get_id();
		sSend["type_"] = bird_in_manager->get_type();
		sSend["item_"] = bird_in_manager->get_item();
		sSend["path_id_"] = bird_in_manager->get_path_id();
		sSend["path_type_"] = bird_in_manager->get_path_type();
		sSend["path_delay_"] = bird_in_manager->get_path_delay();
		sSend["path_offset_x"] = bird_in_manager->get_path_offset().x_;
		sSend["path_offset_y"] = bird_in_manager->get_path_offset().y_;
		sSend["elapsed_"] = 0;
		sSend["speed_"] = bird_in_manager->get_speed();
		sSend["time_"] = (int64_t)GetTickCount();
		m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD,&sSend);*/
	}
}

//û�е��� ����
/*
void GameTableLogic::send_scene_entity(GamePlayer * pGamePlayer)
{
	if (scene_special_time_ > 0)
		return;

	Bird *bird;
	for (uint32_t j = 0; j < bird_manager_.get_count(); j++)
	{
		bird = bird_manager_.get_index_entity(j);
		if (!bird)
		{
			//DBGPRINT("TableID=%d ** send_scene_entity() !bird_manager_.get_index_entity", m_pGameRoom->m_pITableFrame->GetTableID());
			continue;
		}
		if (bird->get_index() == INVALID_ID)
			continue;

		if (!inside_screen(bird))
			continue;
		
		//������Ѫ��
		SetFishScore(bird, bird->get_type());

		Action_Interval * bird_act = ((Action_Interval*)action_manager_.get_action(bird->action_id()));
		if (bird->get_yuzhen_type() == YUZHEN_LINEAR)
		{
			CMD_S_Send_Bird_Linear cssbl;
			cssbl.id_        	 = bird->get_id();
			cssbl.type_       = bird->get_type();
			cssbl.item_       = bird->get_item();
			cssbl.path_delay_ = bird->get_path_delay();
			cssbl.elapsed_    = 0;
			cssbl.speed_      = bird->get_speed();
			cssbl.start_.x_     = bird->get_start_pt().x_;
			cssbl.start_.y_     = bird->get_start_pt().y_;
			cssbl.end_.x_       = bird->get_linear_end_pt().x_;
			cssbl.end_.y_       = bird->get_linear_end_pt().y_;
			cssbl.time_       = Utility::GetTick();
			m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_LINEAR, &cssbl, sizeof(cssbl));

			*//*SVar sSend;
			sSend["id_"]         = (int64_t)bird->get_id();
			sSend["type_"]       = bird->get_type();
			sSend["item_"]       = bird->get_item();
			sSend["path_delay_"] = bird->get_path_delay();
			sSend["elapsed_"]    = 0;
			sSend["speed_"]      = bird->get_speed();
			sSend["start_x"]     = bird->get_start_pt().x_;
			sSend["start_y"]     = bird->get_start_pt().y_;
			sSend["end_x"]       = bird->get_linear_end_pt().x_;
			sSend["end_y"]       = bird->get_linear_end_pt().y_;
			sSend["time_"]       = (int64_t)GetTickCount();
			m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD_LINEAR,&sSend);*//*

		}
		else if (bird->get_yuzhen_type() == YUZHEN_ROUND)
		{
			CMD_S_Send_Bird_Round cssbr;
			cssbr.id_               = bird->get_id();
			cssbr.type_             = bird->get_type();
			cssbr.item_            = bird->get_item();
			cssbr.path_delay_       = bird->get_path_delay();
			cssbr.speed_            = bird->get_speed();
			cssbr.radius_          = bird->get_round_radius();
			cssbr.rotate_duration_  = bird->get_round_rotate_duration();
			cssbr.center_.x_ = bird->get_start_pt().x_;
			cssbr.center_.y_ = bird->get_start_pt().y_;
			cssbr.start_angle_      = bird->get_round_start_angle();
			cssbr.rotate_angle_     = bird->get_round_rotate_angle();
			cssbr.move_duration_    = bird->get_round_move_duration();
			cssbr.time_             = Utility::GetTick();
			m_pGame_Process->broadcast_all(SUB_S_SEND_BIRD_ROUND, &cssbr, sizeof(CMD_S_Send_Bird_Round));

			*//*SVar sSend;
			sSend["id_"]               = (int64_t)bird->get_id();
			sSend["type_"]             = bird->get_type();
			sSend["item_"]             = bird->get_item();
			sSend["path_delay_"]       = bird->get_path_delay();
			sSend["speed_"]            = bird->get_speed();
			sSend["radius_"]           = bird->get_round_radius();
			sSend["center_"]           = bird->get_round_rotate_duration();
			sSend["rotate_duration_x"] = bird->get_start_pt().x_;
			sSend["rotate_duration_y"] = bird->get_start_pt().y_;
			sSend["start_angle_"]      = bird->get_round_start_angle();
			sSend["rotate_angle_"]     = bird->get_round_rotate_angle();
			sSend["move_duration_"]    = bird->get_round_move_duration();
			sSend["time_"]             =(int64_t)GetTickCount();
			m_pGameRoom->Bordcast_All(SUB_S_SEND_BIRD_ROUND,&sSend);*//*
		}
	}

	return;
}*/

bool GameTableLogic::func_bird_end(Entity *target, uint32_t data)
{
	if (bird_manager_.get_entity(target->get_id()))
	{
		if (bird_manager_.get_entity(target->get_id())->get_type() == BOSS_FISH)
		{
			return true;
		}
		
		Bird *pBird =  bird_manager_.get_entity(target->get_id());
		//����ִ����ϣ�����ִ�ж����������
		if(pBird != NULL)
		{	
			//����עѪ
		//	RecovePlusScore(pBird, pBird->get_type());
			pBird->reset();
			//�������д���
           if(pBird->get_index() != INVALID_ID)
           {
               bird_manager_.delete_entity(target->get_id());
           }
		}
		
		//CTraceService::TraceStringEx(TraceLevel_Debug, L"func_bird-endɾ����ID=%d",target->get_id());
	}

	return true;
}

void GameTableLogic::check_outtime_bullet()
{
	bullet_manager_.check_bullet_time_out();
}

//��ʱ�ַ���
void GameTableLogic::GameLoop(float dt)
{
	try
	{
		//������
		ClearFish(dt);
		//�ַ���
		DistributeFish(dt);
		//���¶����������� ������������
		action_manager_.update(dt);
		//����ӵ������Ƿ��г�ʱ���ӵ�
		m_check_timeou_bullet_interval -= dt;
		if(m_check_timeou_bullet_interval < 0)
		{
			m_check_timeou_bullet_interval = 5.0f;
			check_outtime_bullet();
		}
	}
	catch(...)
	{
		//DBGPRINT("TableID=%d ** throw_normal_error(distribute_bird)", m_pGameRoom->m_pITableFrame->GetTableID());
		//goto TABLE_distribute_bird;
	}

	return;
}

void GameTableLogic::user_fire_bullet()
{
	//����û�����ã�����
	/*for (unsigned short i = 0; i < GAME_PLAYER; i++)
	{
		GamePlayer * user_item = m_pGameRoom->GetPlayer(i);
		if (user_fire[i].bUserFire == true)
		{
			OnSubPlayerFire(user_item, &user_fire[i].userFire, true);
			user_fire[i].bUserFire = false;
		}
	}*/
}
void GameTableLogic::DeleteInvalidBullet(unsigned short wChairID)
{
	//SCORE clearScore = 0, forceScore = 0;
	//user_item->GetControlPlusInfo(forceScore, clearScore);


	//������Ϊû���κ�����
//	Bullet *bullet = bullet_manager_.get_entity_Bullet(wChairID);
//	if (bullet == NULL)
//	{
//		return;
//	}


	//forceScore += bullet->bullet_mulriple();
	//user_item->SetControlPlusInfo(forceScore, clearScore);
}

//������������
int GameTableLogic::fish_multiple_control(int fishKind)
{
	int totalOdds = 0, randValue = 0, tempOdds = 0, selectIndex = 0, selectMultiple = 0;
	for (int i = 0; i < 4; i++)
	{
		totalOdds += g_game_config_xml.fish_multiple_control[i];
	}
	randValue = rand() % totalOdds;
	for (int i = 0; i < 4; i++)
	{
		tempOdds += g_game_config_xml.fish_multiple_control[i];
		if (tempOdds >= randValue)
		{
			selectIndex = i;
			break;
		}
	}
	if (selectIndex == 0)
		selectMultiple = rand() % (g_game_config_xml.bird_config_[fishKind].mulriple_extend[selectIndex] - g_game_config_xml.bird_config_[fishKind].mulriple_min) + g_game_config_xml.bird_config_[fishKind].mulriple_min + 1;
	else
		selectMultiple = rand() % (g_game_config_xml.bird_config_[fishKind].mulriple_extend[selectIndex] - g_game_config_xml.bird_config_[fishKind].mulriple_extend[selectIndex - 1]) + g_game_config_xml.bird_config_[fishKind].mulriple_extend[selectIndex - 1] + 1;
	return selectMultiple;
}
//����ʱ�����ѡ����ĸ���
void GameTableLogic::selectFishByTime(int &fishKind, int &fishMultiple)
{
	//û�е��� ����
	/*int randFish[MAX_BIRD_TYPE] = { 0 }, randCount = 0;
	int randSelect = rand() % 4;
	for (int i = 0; i < MAX_BIRD_TYPE; i++)
	{
		//25%ѡ��С��
		if (randSelect == 0)
		{
			if (g_game_config_xml.bird_config_[i].mulriple_min < BIRD_TYPE_12)
				randFish[randCount++] = i;
		}
		//25%ѡ��BOSS
		else if (randSelect == 1)
		{
			fishKind == BOSS_FISH;
		}
		else
		{
			if (g_game_config_xml.bird_config_[i].mulriple_min >= BIRD_TYPE_12 && //������ 
				g_game_config_xml.bird_config_[i].mulriple_min <= BIRD_TYPE_24 && //����
				g_game_config_xml.bird_config_[i].mulriple_min != BIRD_PAUSE)     //����ը��
			{
				randFish[randCount++] = i;
			}
		}
	}

	if (randCount != 0)
	{
		fishKind = randFish[rand() % randCount];
	}


	if (g_game_config_xml.bird_config_[fishKind].mulriple_extend[0] != 0)
	{
		fishMultiple = fish_multiple_control(fishKind);
	}
	else
	{
		fishMultiple = g_game_config_xml.bird_config_[fishKind].mulriple_min;
	}*/
}

////����ץ����
//void GameTableLogic::ProcessCatchFish(uint8_t type, uint32_t nBulletMul, GamePlayer * pGamePlayer, bool isAddFireProb)
//{
//	bool pause_is    = false;
//	bool isPenetrate = false;
//	int judgeCount   = 1;
//	Bird *bird       = NULL;
//	if (pGamePlayer == NULL)
//	{
//		return;
//	}
//
//	uint16_t wChairId = pGamePlayer->GetSeat();
//	//cout << "����" << wChairId << endl;
//	for (int judge = 0; judge < judgeCount; judge++)
//	{
//		//��ȡ�ӵ�����
//		Bullet *bullet = bullet_manager_.get_entity_Bullet(wChairId);
//		if (bullet == NULL)
//		{
//			SHOW("[������] ��Ч�ӵ�!");
//			return;
//		}
//
//		nBulletMul = bullet->bullet_mulriple();
//		//�ͷ��ӵ�
//		if(!bullet_manager_.delete_entity_Bullet(wChairId))
//		{
//			SHOW("[������] �ӵ�������û����ô���ӵ�!");
//			return;
//		}
//
//		//������е���
//		bird = bird_manager_.get_entity(role_[wChairId].fish_id_);
//		//������
//		uint16_t bird_type = bird->get_type();
//
//		if (bird  == NULL || bird->get_index() == INVALID_ID)
//		{
//			SHOW("[������] ��Ч�� %I64d", pGamePlayer->GetPlayerID());
//
//			INSTANCE(CBYStockConfig)->SetBigStock(nBulletMul);
//			//�ܿ��
//			INSTANCE(CBYStockConfig)->SetCurrentStock(nBulletMul);
//
//			//����ʧ,���ӵ����ظ����
//			//��ҽ�ұ��
//			//pGamePlayer->ChangeCoin(nBulletMul,false);
//			////��¼�����Ӯ
//			//pGamePlayer->SetPlayerWinScore(lCatchScore);
//			DeleteInvalidBullet(wChairId, pGamePlayer);
//			return;
//		}
//
//		//������
//		int nFishKind     = bird->get_type();
//		//�㱶��
//		int nFishMultiple = bird->get_mulriple();
//
//		//У��������
//		if( (bird->get_type() < BIRD_TYPE_0 ) || (bird->get_type() >= MAX_BIRD_TYPE && bird->get_type() < BIRD_TYPE_ONE ) || ( bird->get_type() > BIRD_TYPE_FIVE))
//		{
//			SHOW("[������] �������͵��� %I64d", pGamePlayer->GetPlayerID());
//
//			INSTANCE(CBYStockConfig)->SetBigStock(nBulletMul);
//			//�ܿ��
//			INSTANCE(CBYStockConfig)->SetCurrentStock(nBulletMul);
//
//			//ɾ����Ч���ӵ�
//			DeleteInvalidBullet(wChairId, pGamePlayer);
//			//����עѪ
//			RecovePlusScore(bird, nFishKind);
//			//����������
//			bird->reset();
//			//�����������ɾ����
//			bird_manager_.delete_entity(role_[wChairId].fish_id_);
//			return;
//		}
//		
//		//�Ƿ���������
//		bool bSuccess = false; 
//		//BOSS��ʶ
//		bool bIsBoss  = false;
//		//��˫��
//		bool bIsDouble = false;
//
//		//����ʱ�䲢����BOSS
//		if (scene_special_time_ > 0 && nFishKind == BOSS_FISH)
//		{
//			selectFishByTime(nFishKind, nFishMultiple);
//			bIsBoss = true;
//			/*SHOW("FindID:%d FindKind:%d",  role_[wChairId].fish_id_, nFishKind);*/
//		}
//
//		//������(һ��һ��,һ��˫��,һ������,һ���ĵ�,һ�����)
//		if (nFishKind >= BIRD_TYPE_ONE && nFishKind <= BIRD_TYPE_FIVE)
//		{
//			nFishMultiple = nFishMultiple *			//�������㱶��
//				g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count *		//�����������
//				g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].mulriple;		//����
//		}
//		
//		//˰�ձ���(���֮)
//		//int nRevenueMul = 6;
//		//int64_t lRevenue = nBulletMul - (int)((10000 - nRevenueMul) / 10000.0f * nBulletMul);
//		////pGamePlayer->SetChouShui(lRevenue);
//		//pGamePlayer->SetPlayerRevenue(lRevenue);
//
//		//ʣ���ң�������
//		int64_t lRemainScore = nBulletMul;
//
//		if( INSTANCE(CBYStockConfig)->GetRevenueSwitch() >=  INSTANCE(CBYStockConfig)->GetRevenueCount())
//		{
//			//����
//			INSTANCE(CBYStockConfig)->ResetRevenueSwitch();
//			//��¼˰��
//			INSTANCE(CBYStockConfig)->SetCureentRevenue(lRemainScore);
//			lRemainScore = 0L;
//		}
//		else
//		{
//			//����
//			INSTANCE(CBYStockConfig)->SetRevenueSwitch();
//		}
//
//		//С����
//		if (nFishKind <= SMALL_BIRD_FLAG)
//		{
//			INSTANCE(CBYStockConfig)->SetSmallStock(lRemainScore);
//		}
//		//������
//		else
//		{
//			INSTANCE(CBYStockConfig)->SetBigStock(lRemainScore);
//		}
//		//�ܿ��
//		INSTANCE(CBYStockConfig)->SetCurrentStock(lRemainScore);
//		
//		//Logger PlayerLog("��Ҽ�¼", m_pGameRoom->GetRoomID());
//		//PlayerLog.TraceInfo("�������:%.2f ���:%.2f", (double)(lRemainScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );	
//
//		//�����������
//		int nMaxBullet = bird->GetMaxBullet();
//		if (nBulletMul > nMaxBullet)
//		{
//			bird->SetMaxBullet(nBulletMul);
//		}
//
//		//��¼��һ������
//		bird->SetChairScore(wChairId, nBulletMul);
//		//��¼��������¼
//		bird->SetHitScore(nBulletMul);
//
//		//PlayerLog.TraceInfo("[עѪǰ] ��:%I64d �������:%.2f ���:%.2f", bird->get_id(),(double)(lRemainScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );	
//		//עѪ
//		SetFishPlusScore(pGamePlayer, bird, nBulletMul, nFishMultiple, nFishKind);
//
//		//PlayerLog.TraceInfo("[עѪ��] ��:%I64d �������:%.2f ���:%.2f", bird->get_id(), (double)(lRemainScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );	
//
//		//�ж��Ƿ����
//		bSuccess = ComputeProbability(pGamePlayer, nBulletMul, nFishKind, nFishMultiple, wChairId, bird, bIsDouble);
//
//		//����עѪ
//		RecovePlusScore(bird, nFishKind);
//
//		//PlayerLog.TraceInfo("[���պ�] ��:%I64d �������:%.2f ���:%.2f", bird->get_id(), (double)(lRemainScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );	
//		if (bSuccess)
//		{
//			//char logArray[50];
//			bird = bird_manager_.get_entity(role_[wChairId].fish_id_);
//			if (bird == NULL|| bird->get_index() == INVALID_ID)
//			{
//				//DBGPRINT("TableID=%d ** bird enum bird manager failed! id=%d", m_pGameRoom->m_pITableFrame->GetTableID(), remove_birds_[m]);
//				return;
//			}
//
//			//���е÷�
//			SCORE lCatchScore =  nFishMultiple * nBulletMul;
//
//			//�����ж�
//			if(CheckBaoJi(pGamePlayer, nFishKind, lCatchScore))
//			{
//				lCatchScore *= 2;
//				bIsDouble = true;
//			}
//
//			role_[wChairId].set_gold(lCatchScore + role_[wChairId].get_gold());
//
//			//�ṹ���Ǯ
//			//int64_t lTmpScore = lCatchScore;
//
//			//С����
//			if (nFishKind <= SMALL_BIRD_FLAG)
//			{
//				INSTANCE(CBYStockConfig)->SetSmallStock(-lCatchScore);
//			}
//			//������
//			else
//			{
//				INSTANCE(CBYStockConfig)->SetBigStock(-lCatchScore);
//			}
//
//			//�ܿ��
//			INSTANCE(CBYStockConfig)->SetCurrentStock(-lCatchScore);
//
//			//PlayerLog.TraceInfo("[������] ������:%.2f ���:%.2f", (double)(lCatchScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );	
//
//			//��¼�����ˮ
//			int64_t lStreamValue = (lCatchScore - nBulletMul);
//			pGamePlayer->SetPlayerStream(lStreamValue);
//			//PlayerLog.TraceInfo("[������] ��:%I64d ��ˮ:%.2f ", bird->get_id(), (double)(pGamePlayer->GetPlayerStream() / GOLD_RATIO));	
//			
//			//��ҽ�ұ��
//			pGamePlayer->ChangeCoin(lCatchScore,false);
//			//��¼�����Ӯ
//			pGamePlayer->SetPlayerWinScore(lCatchScore);
//		
//			cout << "��ˮ:" << pGamePlayer->GetPlayerStream() << "|" << pGamePlayer->GetPlayerWinScore() << endl;
//
//			Catch_Bird catch_bird;
//			catch_bird.bird_id_     = bird->get_id();
//			catch_bird.catch_gold_  = lCatchScore;
//			catch_bird.isDouble     = bIsDouble;
//			catch_bird.now_money    = role_[wChairId].get_gold();
//			catch_bird.chair_id_    = wChairId;
//			if (bIsBoss && nFishKind != BOSS_FISH)
//			{
//				catch_bird.is_die = 0;
//			}
//			else
//			{
//				catch_bird.is_die = 1;
//			}
//			
//			if (catch_bird.is_die == 1)
//			{
//				//SHOW("[������] BOSS�� %I64d ������ %d_%d", pGamePlayer->GetPlayerID(), role_[wChairId].fish_id_, nFishKind);
//				//������
//				action_manager_.pause_target(bird);
//				action_manager_.remove_all_action(bird);
//
//				//����������
//				bird->reset();
//				//���������ͷ���
//				bird_manager_.delete_entity(role_[wChairId].fish_id_);
//			}
//
//			SVar sSend;
//			//��ID
//			sSend["bird_id_"]    = (int64_t)bird->get_id();
//			//�����ý����
//			sSend["catch_gold_"] = lCatchScore;
//			//�Ƿ񱩻�(˫��)
//			sSend["isDouble"]    = bIsDouble;
//			//��ǰ�����
//			sSend["now_money"]   = role_[wChairId].get_gold();
//			//����ID
//			sSend["chair_id_"]   = wChairId;
//			if (bIsBoss && nFishKind != BOSS_FISH)
//			{
//				sSend["is_die"] = 0;
//			}
//			else
//			{
//				sSend["is_die"] = 1;
//			}
//			m_pGameRoom->Bordcast_All(SUB_S_CATCH_BIRD,&sSend);
//
//			//������֮�� ���������ж��Ƿ������������͵���
//			AddFishRefresh(bird_type);
//		}
//		else
//		{
//			//��¼�����ˮ
//			pGamePlayer->SetPlayerStream(nBulletMul);
//		//	cout << "��ˮ:" << pGamePlayer->GetPlayerStream() << endl;
//			cout << "��ˮ:" << pGamePlayer->GetPlayerStream() << "|" << pGamePlayer->GetPlayerWinScore() << endl;
//
//			//PlayerLog.TraceInfo("[δ������] ��:%I64d ��ˮ: %.2f ", bird->get_id(), (double)(pGamePlayer->GetPlayerStream() / GOLD_RATIO));	
//		}
//
//		//���Ӯȡ����
//		role_[wChairId].maxScore = role_[wChairId].maxScore > role_[wChairId].get_gold() ? role_[wChairId].maxScore : role_[wChairId].get_gold();
//	}
//}

//û�е�������
//�������ж�
//bool GameTableLogic::CheckBaoJi(GamePlayer*pGamePlayer, int nFishKind, int64_t lCatchScore)
//{
//	srand((unsigned int)((unsigned int)time(NULL) + (unsigned int)pGamePlayer->GetSeat()));
//
//	//ֻ�п�� > 0���ܱ���
//	//if(INSTANCE(CBYStockConfig)->GetCurrentStock() > 0L)
//	if(g_bystockconfig.GetCurValue() > 0L)
//	{
//		//��ȡ�����Ӯ���
//		int64_t lPlayerWinScore = pGamePlayer->GetPlayerWinScore();
//		int64_t lBaoJiScore     = lCatchScore * 2;
//		//�������ܻ��տ��
//		if(lBaoJiScore > INSTANCE(CBYStockConfig)->GetCurrentStock())
//		{
//			return false;
//		}
//
//		//ӮǮ״̬
//		if(pGamePlayer->GetPlayerWinScore() >= 0L)
//		{
//			if(rand() % 100 < 20)
//			{
//				return true;
//			}
//		}
//		//��Ǯ״̬
//		else
//		{
//			if(rand() % 100 < 40)
//			{
//				return true;
//			}
//			////�������ܴ�ƽ
//			//if(lBaoJiScore + pGamePlayer->GetPlayerWinScore() > 0L)
//			//{
//			//	return false;
//			//}
//		}
//
//	}
//	return false;
//}

//��������������
void GameTableLogic::AddFishRefresh(int nFishType)
{
	if (nFishType < BIRD_TYPE_0 || nFishType >= MAX_BIRD_TYPE)
	{
		return;
	}

	//�����ʱ�򲻳���  
	if (scene_special_time_ > 0)
	{
		return;
	}

	//������֮�󲹳�ĸ���
	int nAddPro        = g_game_config_xml.add_fish_refresh_config_[nFishType].refresh_probability;
	//��������
	if (nAddPro == 0)
	{
		return;
	}
	
	//ˢ�µ���С������룩
	int    nInterval        = g_game_config_xml.add_fish_refresh_config_[nFishType].refresh_interval;
	time_t tNowTime         = time(NULL);
	time_t tLastRefreshTime = add_fish_interval[nFishType];

	if ( ((rand() % 100) < nAddPro) && ((tNowTime - tLastRefreshTime) > nInterval) )
	{
		//CTraceService::TraceStringEx(TraceLevel_Debug, L"ˢ�³ɹ� ��ǰʱ�䣺%I64d  �ϴ�ˢ��ʱ�䣺%I64d \n�����%I64d  ���� ˢ�¼����%d ", nowTime, add_fish_interval[fishType], nowTime - add_fish_interval[fishType], interval);
		add_fish_interval[nFishType] = tNowTime;
	}
	else
	{
		//CTraceService::TraceStringEx(TraceLevel_Debug, L"����ˢ�� ��ǰʱ�䣺%I64d  �ϴ�ˢ��ʱ�䣺%I64d \n�����%I64d  С�� ˢ�¼����%d ", nowTime, add_fish_interval[fishType], nowTime - add_fish_interval[fishType], interval);
		return;
	}

	SendSingleFish(nFishType);
}

//���������
void GameTableLogic::ClearFish(float dt)
{
	time_t tNowTime         = time(NULL);
	//���ڻ��չ�����
	for (uint32_t j = 0; j < bird_manager_.get_count(); j++)
	{
		Bird *pFish = bird_manager_.get_index_entity(j);
		if (pFish == NULL) continue;
		if (pFish->get_index() == INVALID_ID) continue;
		
		if(tNowTime - pFish->m_tCreatTime > (3 * 60))
		{
			//����������
			pFish->reset();
			if(pFish->get_index() != INVALID_ID)
			{
				//���������ͷ���
				bird_manager_.delete_entity(pFish->get_id());
			}
		}
	}
}

//�ַ���
void GameTableLogic::DistributeFish(float dt)
{
	//����ʱ��
	timer_control_.scene_elapse_ += dt;

	//SHOW("�����¼�:%.2f �����л��¼�:%.2f", timer_control_.scene_elapse_, g_game_config_xml.scene_swith_time_);
	//SHOW("����ʱ��:%.2f", scene_special_time_);
	if (scene_special_time_ > 0 && scene_special_time_ < 200)
	{
		scene_special_time_ -= dt;

		if(initScene_Time == 0)
		{
			initScene_Time = scene_special_time_;
		}

		//if(!yuzhen) yuzhen=true;
		if (initScene_Time - scene_special_time_ >= 6 && yuzhen)
		{
			send_scene_yuzhen(NULL,0);
			yuzhen = false;

            m_small_fish_interval_time = 0.5f;
            m_big_fish_interval_time = 1.5f;
            m_b_send_boss[0] = false;
			m_b_send_boss[1] = false;
			m_b_send_boss[2] = false;
			m_b_send_boss[3] = false;
			birds_factory_.get_special_birds_boss(&m_cur_Boss);
            m_b_once = false;
			m_b_boss_die = false;
		}

		if(yuzhen == false)
		{
			m_small_fish_interval_time -= dt;
			m_big_fish_interval_time -= dt;
			if(m_small_fish_interval_time < 0.0f)
			{
			    //����С��
				send_special_birds(1);
				send_special_birds(2);
				send_special_birds(5);
				send_special_birds(6);
				m_small_fish_interval_time = 0.5f;
			}
			if(m_big_fish_interval_time < 0.0f)
			{
			    //���ͽ���������Ů
				send_special_birds(3);
				send_special_birds(4);
				m_big_fish_interval_time = 1.5f;
			}

			if(m_b_boss_die == false)
			{
				for(int i = 0; i < 4; i++)
				{
					if(m_b_once == false)
					{
						if(m_b_send_boss[i] == false && m_pGame_Process->is_exist_user(i))
						{
							m_b_send_boss[i] = true;
							send_special_boss(i, m_cur_Boss);
						}
					}
					else
					{
						if(m_b_send_boss[i] == false && m_pGame_Process->is_exist_user(i))
						{
							m_b_send_boss[i] = true;

							Point front_delta_ = m_cur_Boss.get_pause_pt() - m_cur_Boss.get_start_pt();
							float length = std::sqrt(front_delta_.x_ * front_delta_.x_ + front_delta_.y_ * front_delta_.y_);
							float front_time_ = length / m_cur_Boss.get_speed();
							float fdeltatime = (front_time_ + m_cur_Boss.get_pause_time()) - (float)(time(NULL) - m_cur_Boss.GetCreateTime()) - 5.0f;
							if(fdeltatime > 0.0f)
							{
								float pause_time = fdeltatime - front_time_;
								if(pause_time <= 0.0f)
								{
									m_cur_Boss.set_pause_time(0.0f);
								}
								else
								{
									m_cur_Boss.set_pause_time(pause_time);
								}

								send_special_boss(i, m_cur_Boss);
							}
						}
					}
				}
			}

			m_b_once = true;
		}

		return;
	}
	else if(scene_special_time_ != 0)
	{
		scene_special_time_ = 0;
	}

	yuzhen = true;
	initScene_Time= 0 ;
	//��ͨ�泡
	if (timer_control_.scene_elapse_ < g_game_config_xml.scene_swith_time_)
	{
		//�л�����ǰ8�벻��������
		if (timer_control_.scene_elapse_ > g_game_config_xml.scene_swith_time_ - 10)
		{
			return;
		}

		//������Ⱥ
 		if (timer_control_.special_bird_elapse_ <= 0)
		{
 			//timer_control_.special_bird_elapse_ = Random(g_game_config_xml.bird_special_config_.distribute_interval_min, g_game_config_xml.bird_special_config_.distribute_interval_max);
 			if(g_game_config_xml.bird_special_config_.distribute_interval_min <= g_game_config_xml.bird_special_config_.distribute_interval_max)
			{
				std::uniform_int_distribution<> uid(g_game_config_xml.bird_special_config_.distribute_interval_min, g_game_config_xml.bird_special_config_.distribute_interval_max);
				timer_control_.special_bird_elapse_ = uid(m_gen);
			}
			else
			{
				timer_control_.special_bird_elapse_ = g_game_config_xml.bird_special_config_.distribute_interval_min;
			}
		}

 		timer_control_.special_bird_elapse_ -= dt;
 		if (timer_control_.special_bird_elapse_ <= 0)
 		{
 			SendSpecialFish();
 			timer_control_.special_bird_elapse_ = 0;
 		}

  		//���ɵ�����
   		for (int i = 0; i < MAX_BIRD_TYPE; i++)
   		{
			if(i == BOSS_FISH)
			{
				continue;
			}
   			//���ü��ʱ��
   			if (timer_control_.distribute_elapsed_[i] <= 0 && (g_game_config_xml.bird_config_[i].distribute_interval_min < g_game_config_xml.bird_config_[i].distribute_interval_max))
			{
   				//timer_control_.distribute_elapsed_[i] = Random(g_game_config_xml.bird_config_[i].distribute_interval_min,g_game_config_xml.bird_config_[i].distribute_interval_max);
   				if(g_game_config_xml.bird_config_[i].distribute_interval_min <= g_game_config_xml.bird_config_[i].distribute_interval_max)
				{
					std::uniform_int_distribution<> uid(g_game_config_xml.bird_config_[i].distribute_interval_min,g_game_config_xml.bird_config_[i].distribute_interval_max);
					timer_control_.distribute_elapsed_[i] = uid(m_gen);
				}
				else
				{
					timer_control_.distribute_elapsed_[i] = g_game_config_xml.bird_config_[i].distribute_interval_min;
				}
			}

   			timer_control_.distribute_elapsed_[i] -= dt;
   			//����
   			if(timer_control_.distribute_elapsed_[i] <= 0)
   			{
   				SendSingleFish(i);
   			}
   		}

   		//һ�����
   		for (int i = 0; i <= BIRD_TYPE_FIVE - BIRD_TYPE_ONE; i++)
   		{
   			if (timer_control_.bird_same_elapse_[i] <= 0 && (g_game_config_xml.bird_same_config_[i].distribute_interval_min < g_game_config_xml.bird_same_config_[i].distribute_interval_max))
			{
   				//timer_control_.bird_same_elapse_[i] = Random(g_game_config_xml.bird_same_config_[i].distribute_interval_min, g_game_config_xml.bird_same_config_[i].distribute_interval_max);
				if(g_game_config_xml.bird_same_config_[i].distribute_interval_min <= g_game_config_xml.bird_same_config_[i].distribute_interval_max)
				{
					std::uniform_int_distribution<> dis(g_game_config_xml.bird_same_config_[i].distribute_interval_min, g_game_config_xml.bird_same_config_[i].distribute_interval_max);
					timer_control_.bird_same_elapse_[i] = dis(m_gen);
				}
				else
				{
					timer_control_.bird_same_elapse_[i] = g_game_config_xml.bird_same_config_[i].distribute_interval_min;
				}
			}

   			timer_control_.bird_same_elapse_[i] -= dt;
   			if (timer_control_.bird_same_elapse_[i] <= 0)
   			{
  				SendSingleFish(i + BIRD_TYPE_ONE);
   			}
   		}
  
   		//С��Ⱥ
   		if (timer_control_.group_bird_elapse_ <= 0 && (g_game_config_xml.bird_group_config_.distribute_interval_min < g_game_config_xml.bird_group_config_.distribute_interval_max))
		{
   			//timer_control_.group_bird_elapse_ = Random(g_game_config_xml.bird_group_config_.distribute_interval_min, g_game_config_xml.bird_group_config_.distribute_interval_max);

   			if(g_game_config_xml.bird_group_config_.distribute_interval_min <= g_game_config_xml.bird_group_config_.distribute_interval_max)
			{
				std::uniform_int_distribution<> uid(g_game_config_xml.bird_group_config_.distribute_interval_min, g_game_config_xml.bird_group_config_.distribute_interval_max);
				timer_control_.group_bird_elapse_ = uid(m_gen);
			}
			else
			{
				timer_control_.group_bird_elapse_ = g_game_config_xml.bird_group_config_.distribute_interval_min;
			}
		}

   		timer_control_.group_bird_elapse_ -= dt;
   		if (timer_control_.group_bird_elapse_ <= 0)
		{
   			SendGroupFish();
		}
  
   		//��������
   		if (timer_control_.bird_red_series_elapse_ <= 0 && (g_game_config_xml.bird_red_series_config_.distribute_interval_min < g_game_config_xml.bird_red_series_config_.distribute_interval_max))
		{
   			//timer_control_.bird_red_series_elapse_ = Random(g_game_config_xml.bird_red_series_config_.distribute_interval_min, g_game_config_xml.bird_red_series_config_.distribute_interval_max);
			if(g_game_config_xml.bird_red_series_config_.distribute_interval_min <= g_game_config_xml.bird_red_series_config_.distribute_interval_max)
			{
				std::uniform_int_distribution<> uid(g_game_config_xml.bird_red_series_config_.distribute_interval_min, g_game_config_xml.bird_red_series_config_.distribute_interval_max);
				timer_control_.bird_red_series_elapse_ = uid(m_gen);
			}
			else
			{
				timer_control_.bird_red_series_elapse_ = g_game_config_xml.bird_red_series_config_.distribute_interval_min;
			}
		}

   		timer_control_.bird_red_series_elapse_ -= dt;

   		if (timer_control_.bird_red_series_elapse_ <= 0)
		{
   			SendSingleFish(BIRD_TYPE_RED);
		}
 
 		//�ĳ�����
 		if (timer_control_.bird_red_bloating_elapse_ <= 0 && (g_game_config_xml.bird_red_bloating_config_.distribute_interval_min < g_game_config_xml.bird_red_bloating_config_.distribute_interval_max))
		{
 			//timer_control_.bird_red_bloating_elapse_ = Random(g_game_config_xml.bird_red_bloating_config_.distribute_interval_min, g_game_config_xml.bird_red_bloating_config_.distribute_interval_max);

 			if(g_game_config_xml.bird_red_bloating_config_.distribute_interval_min <= g_game_config_xml.bird_red_bloating_config_.distribute_interval_max)
			{
				std::uniform_int_distribution<> dis(g_game_config_xml.bird_red_bloating_config_.distribute_interval_min, g_game_config_xml.bird_red_bloating_config_.distribute_interval_max);
				timer_control_.bird_red_bloating_elapse_ = dis(m_gen);
			}
			else
			{
				timer_control_.bird_red_bloating_elapse_ = g_game_config_xml.bird_red_bloating_config_.distribute_interval_min;
			}
		}
 		timer_control_.bird_red_bloating_elapse_ -= dt;

 		if (timer_control_.bird_red_bloating_elapse_ <= 0)
		{
 			SendRedBloating();
		}
	}
	//������
 	else
 	{
		//SHOW("�����¼�:%.2f �����л��¼�:%.2f", timer_control_.scene_elapse_, g_game_config_xml.scene_swith_time_);
		//SHOW("�����л�");
		//�л�����
 		ChangeScene();
 	}

	SendFish();
}

bool GameTableLogic::is_valid_bullet_id(int chair_id_p, uint32_t bullet_id)
{
	if(chair_id_p == 0)
	{
		if(bullet_id < 0 || bullet_id > 200)
		{
			LOG(ERROR) << "player:"<<m_pGame_Process->get_user_id(chair_id_p) << " chair_id: "<<chair_id_p << " bullet_id:"<<bullet_id;
			return false;
		}
	}
	else if(chair_id_p == 1)
	{
		if(bullet_id < 201 || bullet_id > 400)
		{
			LOG(ERROR) << "player:"<<m_pGame_Process->get_user_id(chair_id_p) << " chair_id: "<<chair_id_p << " bullet_id:"<<bullet_id;
			return false;
		}
	}
	else if(chair_id_p == 2)
	{
		if(bullet_id < 401 || bullet_id > 600)
		{
			LOG(ERROR) << "player:"<<m_pGame_Process->get_user_id(chair_id_p) << " chair_id: "<<chair_id_p << " bullet_id:"<<bullet_id;
			return false;
		}
	}
	else if(chair_id_p == 3)
	{
		if(bullet_id < 601 || bullet_id > 800)
		{
			LOG(ERROR) << "player:"<<m_pGame_Process->get_user_id(chair_id_p) << " chair_id: "<<chair_id_p << " bullet_id:"<<bullet_id;
			return false;
		}
	}
	else
	{
		LOG(ERROR) << "OnSubPlayerFire invalid chair_id:"<<chair_id_p;
		return false;
	}

	return true;
}

//�û�����
void GameTableLogic::OnSubPlayerFire(unsigned short chair_id_p, CMD_C_Fire *pFire, bool bLoop /* = false */)
{
	if(m_pGame_Process->is_exist_user(chair_id_p) == false)
	{
		return;
	}

	//�ӵ�IDУ��
	if(is_valid_bullet_id(chair_id_p, pFire->bullet_id) == false)
	{
		LOG(ERROR) << "player:"<<m_pGame_Process->get_user_id(chair_id_p) << " can not send bullet:"<<pFire->bullet_id;
		SendFireFailed(chair_id_p);
		return;
	}

	//�ӵ���ЧУ��
	if(bullet_manager_.is_using(pFire->bullet_id))
	{
		LOG(ERROR) << "player:"<<m_pGame_Process->get_user_id(chair_id_p) << " chair ID: "<<chair_id_p << " bullet ID:"<<pFire->bullet_id << " is using";
		SendFireFailed(chair_id_p);
		return;
	}

	uint16_t chair_id = chair_id_p;
	//uint16_t cannon   = role_[chair_id].get_cannon();
	SCORE finalGold   = 0L;
	SCORE gold        = role_[chair_id].get_gold();
	float rotation    = pFire->rote_;//����Ƕ�
	
	//�ӵ�����
	int bullet_count = bullet_manager_.get_count(chair_id_p);

	pFire->bullet_mulriple_ = pFire->bullet_mulriple_ * 100;
	//�ڵ�У��
	int bulletLevel = pFire->bullet_mulriple_ / g_game_config_xml.cannon_mulriple_[0];
	if (bulletLevel >= 1 && bulletLevel <= 10)
	{
		pFire->bullet_mulriple_ = bulletLevel * g_game_config_xml.cannon_mulriple_[0];
	}
	else
	{
		SendFireFailed(chair_id);
		return;
	}

	int bulletCount = 1;

	//˫�� ��˫��Ǯ  ����Ҫ�����ӵ�����
	if ((pFire->bullet_type_ & BULLET_DOUBLE) != 0)
	{
		bulletCount *= 2;
	}
	//��ֻ�ǿͻ��˼��٣����������ô���
	/*if (pFire->bullet_type_ & BULLET_FURY != 0)
	{
		bulletCount *= 2;
	}*/

	uint32_t temp_bullet_id = 0;
	//�����ӵ�����������Ҫ�� �����ӵ�
	for (int i = 0; i < bulletCount; i++)
	{
		//��ȡ���ʣ����
		gold = role_[chair_id].get_gold();

		//У���Һ���Ļ�ϵ��ӵ���Ŀ
		if (gold >= pFire->bullet_mulriple_ && bullet_count < MAX_BULLET_COUNT)
		{
			//���ظı�
			//PondChange();

			if(i == 0)
			{
				temp_bullet_id = pFire->bullet_id;
			}
			else if(i == 1)
			{
				temp_bullet_id = pFire->bullet_idTwo;
			}
			else
			{
				LOG(ERROR) << "OnSubPlayerFire player:" << m_pGame_Process->get_user_id(chair_id) << " i:"<<i;
				SendFireFailed(chair_id);
				return;
			}

			//�����ӵ�����
			Bullet *bullet = (Bullet *)bullet_manager_.new_entity(chair_id_p, temp_bullet_id);

			//�����ӵ��������
			if (!bullet)
			{
				//LOG(ERROR) << "player: "<< m_pGame_Process->get_user_id(chair_id_p) << " create bullet failture!";
				SendFireFailed(chair_id);
				return;
			}
			else
			{
				//LOG(ERROR) <<"player:"<< m_pGame_Process->get_user_id(chair_id_p) << " create bullet id:" << bullet->get_bullet_id() << " succesfully!";
			}

			//���ӱ��
			bullet->set_chair_id(chair_id);
			//�ӵ��Ƕ�
			bullet->set_rotation(pFire->rote_ - M_PI_2);
			//�ӵ�����
			bullet->set_bullet_mulriple(pFire->bullet_mulriple_);

			//ʣ�������� ���� �ӵ���ֵ
			if (gold > pFire->bullet_mulriple_)
			{
				finalGold = (gold - pFire->bullet_mulriple_);
			}
			else
			{
				finalGold = 0;
			}

			//���㿪��
			//CalculateFire(chair_id, pFire->bullet_mulriple_, pGamePlayer);

			//���������Ϣ
			//��ҽ��
			role_[chair_id].set_gold(finalGold);
			//�ڼ�ֵ
			role_[chair_id].set_cannon_mulriple(pFire->bullet_mulriple_);
			//������ID
			role_[chair_id].set_lock_bird_id(pFire->lock_bird_id_);
			//����Ӯȡ����
			role_[chair_id].minScore = role_[chair_id].minScore < role_[chair_id].get_gold() ? role_[chair_id].minScore : role_[chair_id].get_gold();

			//ִ�нṹ���Ǯ
			//int64_t lUseScore = finalGold > 0 ? (-(int)pFire->bullet_mulriple_): - pGamePlayer->GetCoin();
			int64_t lUseScore = finalGold > 0 ? (-(int)pFire->bullet_mulriple_): - m_pGame_Process->get_user_score(chair_id_p);

			//pGamePlayer->ChangeCoin(lUseScore,false);
			m_pGame_Process->change_user_score(chair_id_p, lUseScore);
			//��¼�����Ӯ
			//pGamePlayer->SetPlayerWinScore(lUseScore);
			m_pGame_Process->set_user_win_lose_score(chair_id_p, lUseScore);
            //��ҷ��ڲ��ұ���Ǯ�˲���һ����Ч�ķ���
            m_pGame_Process->set_user_last_fire_time(chair_id, time(NULL));
			//////////////////////////////////////////
			//if(lUseScore > 0)
			//{
			//	//˰�ձ���(���֮)
			//	int nRevenueMul = 40;
			//	int64_t lRevenue = lUseScore - (int)((10000 - nRevenueMul) / 10000.0f * lUseScore);
			//	pGamePlayer->SetChouShui(lRevenue);

			//	//��¼˰��
			//	INSTANCE(CBYStockConfig)->SetCureentRevenue(lRevenue);
			//	//ʣ���ң�������
			//	int64_t lRemainScore = lUseScore - lRevenue;

			//	//С����
			//	if (nFishKind <= SMALL_BIRD_FLAG)
			//	{
			//		INSTANCE(CBYStockConfig)->SetSmallStock(lRemainScore);
			//	}
			//	//������
			//	else
			//	{
			//		INSTANCE(CBYStockConfig)->SetBigStock(lRemainScore);
			//	}
			//	//�ܿ��
			//	INSTANCE(CBYStockConfig)->SetCurrentStock(lRemainScore);
			//}
			/////////////////////////////

		}
		else
		{
			SendFireFailed(chair_id);
			return;
		}
	}

	/*SVar sSend;
	sSend["rotation_"]        = rotation;
	sSend["chair_id_"]        = chair_id;
	sSend["bullet_mulriple_"] = pFire->bullet_mulriple_;
	sSend["lock_bird_id_"]    = pFire->lock_bird_id_;
	sSend["cur_gold_"]        = role_[chair_id].get_gold();
	sSend["bullet_kind"]      = pFire->bullet_type_;
	m_pGameRoom->Bordcast_All(SUB_S_SEND_BULLET,&sSend);*/

	CMD_S_Send_Bullet sFire;
    sFire.chair_id_ = chair_id;
    sFire.cur_gold_ = (double)role_[chair_id].get_gold() / 100.0f;
    sFire.lock_bird_id_ = pFire->lock_bird_id_;
    sFire.bullet_mulriple_ = (double)pFire->bullet_mulriple_ / 100.0f;
    sFire.bullet_kind = pFire->bullet_type_;
    sFire.rotation_ = rotation;
    sFire.bullet_id = pFire->bullet_id;

	/*CMD_S_Fire sFire;
	sFire.rote_        		= rotation;
	sFire.chair_id_         = chair_id;
	sFire.bullet_mulriple_ = pFire->bullet_mulriple_;
	sFire.lock_bird_id_    = pFire->lock_bird_id_;
	sFire.cur_gold_        = role_[chair_id].get_gold();
	sFire.bullet_type_      = pFire->bullet_type_;*/
	m_pGame_Process->broadcast_all(SUB_S_SEND_BULLET, &sFire, sizeof(sFire));
	//LOG(ERROR) << "player:" << chair_id << " fire,lock_fish_id: "<< sFire.lock_bird_id_ << " bullet_mulriple:"<<sFire.bullet_mulriple_ << " cur_glod:" <<sFire.cur_gold_;

	//����ͳ��
	//��ˮ/����ӵ����ܽ��/Ѻע�ܶ�/��֧��
	role_[chair_id].Defray      += pFire->bullet_mulriple_ * bulletCount;
	//֧������/�����ӵ���/Ѻע����
	role_[chair_id].DefrayTimes += bulletCount;
	//�޽�ɫ���ֵ���Ϸ��Ĭ��Ϊ0����ע��Ϸ��Ϊׯ0����1
	role_[chair_id].RoleType    = 0;
	//m_pGameRoom->m_pITableFrame->SendTableData(INVALID_CHAIR, SUB_S_SEND_BULLET, &send_bullet, sizeof(CMD_S_Send_Bullet));

	return ;
}

//û�е��� ����
/*void GameTableLogic::CalculateFire(unsigned short wChairId, uint32_t multiple, GamePlayer* pGamePlayer)
{
	*//*
	//ÿһ�ھ��ֳ�ˮ
	SCORE lProClear = (int)((10000 - g_game_control.m_foceTaxodd) / 10000.f * multiple);

	AccumulateFireCount(wChairId, multiple, pGamePlayer);

	//ǿ�Ʒ�
	SCORE lForceScore = 0L;
	//����
	SCORE lClearScore = 0L;

	//��ÿ��Ʒ���
	pGamePlayer->GetControlPlusInfo(lForceScore, lClearScore);

	//ÿһ�ڳ�ˮ��������˽���
	lClearScore -= lProClear;
	
	//���ÿ��Ʒ���
	pGamePlayer->SetControlPlusInfo(lForceScore, lClearScore);

	int64_t tmpGold = multiple;
	if (tmpGold > 0)
	{
		int r = Random(1,10000);
		if(r < g_game_control.m_SystemPondTaxodd)
		{
			GameRoom::setTax(tmpGold);
			pGamePlayer->m_lTax += tmpGold;
			return;
		}
		//tmpGold = (double)(10000 - g_game_control.m_SystemPondTaxodd) / 10000 * tmpGold;
	}

	//��ˮ֮�����ȫ�ֽ���
	GameRoom::setPond(tmpGold);
	*//*
}*/

void GameTableLogic::PondChange()
{
	//û�е��� ����
	/*if (GameRoom::loadPond() >= g_game_control.MaxinvLine)
	{
		GameRoom::setPond(g_game_control.AutoClear, true);
	}

	//lockArray m_meLock(&lockSection);
	m_mutex.Lock();
	if (g_game_control.m_currentKeepTimes == 0)
	{
		if (g_game_control.m_cofficientKeepTimesMax > g_game_control.m_cofficientKeepTimesMin)
		{
			//g_game_control.m_currentKeepTimes = Random(g_game_control.m_cofficientKeepTimesMin, g_game_control.m_cofficientKeepTimesMax);
			std::uniform_int_distribution<> uid(g_game_control.m_cofficientKeepTimesMin, g_game_control.m_cofficientKeepTimesMax);
			g_game_control.m_currentKeepTimes = uid(m_gen);
		}
		else
		{
			//CTraceService::TraceString(L"����keepϵ������", TraceLevel_Exception);
			g_game_control.m_currentKeepTimes = 5000;
		}
		calculatorPondCoefficient();
	}
	else
	{
		g_game_control.m_currentKeepTimes--;
	}
	m_mutex.Unlock();*/
}

//�ۻ����ڴ��� �ж��Ƿ������Ӯǿ��
//û�е��� ����
//void GameTableLogic::AccumulateFireCount(unsigned short chairID, int bulletMultiple, GamePlayer* pGamePlayer)
//{
	////���ڼ���
	//role_[chairID].fireCount++;

	////�����������  ͬʱ����㹻  �������׻���
	//if (pGamePlayer == NULL)
	//{
	//	return;
	//}

	////enterScorePercent���볡�����ٷֱ�
	////
	//if (role_[chairID].get_gold() < m_pGameRoom->GetLimitInMoney() * (g_game_control.forceScoreProtect.enterScorePercent / 100.f) ||
	//	role_[chairID].get_gold() < bulletMultiple * g_game_control.forceScoreProtect.nowBulletMultiple)
	//{
	//	if (role_[chairID].fireCount - role_[chairID].lastOpenForceSwitch > g_game_control.forceScoreProtect.touchInterval || role_[chairID].isFirstTouchForceSwitch)
	//	{
	//		//�Ƿ��һ�δ򿪿���
	//		role_[chairID].isFirstTouchForceSwitch = false;
	//		//��һ�δ򿪿��ص���������
	//		role_[chairID].lastOpenForceSwitch     = role_[chairID].fireCount;
	//		//ǿ��״̬����
	//		role_[chairID].FoceSwitch              = true;
	//	}
	//}

	////cout <<"�ӳ�ʱ��"<< role_[chairID].timesDelay <<endl;
	//SHOW("[�ӳ�ʱ��] %d", role_[chairID].timesDelay);
	////�����ڿ��� �����ӳٽ׶�
	//if (role_[chairID].timesDelay != 0)
	//{
	//	int lastDelay = role_[chairID].timesDelay;
	//	//������㿪�ڴ�����  ������Ӯǿ�ػ���
	//	SHOW("[�ӳ�ʱ��] %d_%d_%d",role_[chairID].fireCount, role_[chairID].lastTouchForceControl, role_[chairID].timesDelay);
	//	if (role_[chairID].fireCount - role_[chairID].lastTouchForceControl > role_[chairID].timesDelay)
	//	{
	//		role_[chairID].timesDelay = 0;

	//		WinForceControl selectConfig = selectConfigByPond(pGamePlayer);
	//		SCORE wlControlScore = 0, lastWlcontrolScore = 0;
	//		int wlProbability = 0, lastWlProbability = 0;
	//		pGamePlayer->GetWinLoseCtr(lastWlcontrolScore, lastWlProbability);
	//		wlControlScore = min(Random(selectConfig.ControlptsMin, selectConfig.ControlptsMax), Random(selectConfig.ControlperMin, selectConfig.ControlperMax) / 100.f * (role_[chairID].userEnterScore));
	//		wlProbability = selectConfig.addodds;

	//		/*char logArray[256];
	//		sprintf(logArray, "\n�������ControlptsMin:%I64d, ControlptsMax:%I64d, ControlperMin:%d, ControlperMax:%d, currentGold:%I64d, Score:%I64d,Pro:%d, lastScore:%I64d, fireCount:%d, lastTouchForceControl:%d, lastDelay:%d\n",
	//																																																	selectConfig.ControlptsMin, 
	//																																																	selectConfig.ControlptsMax, 
	//																																																	selectConfig.ControlperMin,
	//																																																	selectConfig.ControlperMax, 
	//																																																	role_[chairID].get_gold(), 
	//																																																	wlControlScore, 
	//																																																	wlProbability, 
	//																																																	lastWlcontrolScore,
	//																																																	role_[chairID].fireCount, 
	//																																																	role_[chairID].lastTouchForceControl,
	//																																																	lastDelay);*/
	//		//debugLog(logArray, userItem->GetUserID());
	//		//�������Ƿŷ� ����һ�εĸ����Ƽӽ���
	//		if (wlProbability > 100)
	//		{
	//			wlControlScore += lastWlcontrolScore;
	//		}
	//		else
	//		{
	//			wlControlScore -= lastWlcontrolScore;
	//		}
	//		pGamePlayer->SetWinLoseCtr(wlControlScore, wlProbability);
	//		//sprintf(logArray, "\nʵ������wlScore%I64d pro:%d\n", wlControlScore, wlProbability);
	//		//debugLog(logArray, userItem->GetUserID());
	//	}
	//}
	//else
	//{
	//	//��д���Ʒ���
	//	SCORE lSpecialScore = 0L;
	//	int   nProbability  = 0;
	//	pGamePlayer->GetSpecialCtr(lSpecialScore, nProbability);
	//	if (lSpecialScore > 0 && nProbability > 0)
	//	{
	//		pGamePlayer->SetSpecialCtr( -bulletMultiple );
	//	}
	//	else
	//	{
	//		SCORE lWinLoseScore = 0;
	//		int   lWinLoseProbability = 0;
	//		pGamePlayer->GetWinLoseCtr(lWinLoseScore, lWinLoseProbability);
	//		if (lWinLoseScore > 0)
	//		{
	//			if (lWinLoseProbability > 100)
	//			{
	//				pGamePlayer->SetWinLoseCtr(bulletMultiple);
	//			}
	//			else
	//			{
	//				pGamePlayer->SetWinLoseCtr(-bulletMultiple);
	//			}
	//		}
	//		else
	//		{
	//			char logArray[256];
	//			bool isHaveComp = false; // �Ƿ���ƥ���
	//									 //���ƽ���  Ѱ�����Ӧ���ӳٵ���Ŀ  Ŀǰ�ĸ��� ȥƥ������������  ƥ�䲻�� �����볡�ӳ����һ��
	//			for (int index = 0; index < MAX_CONTROL_NUM; index++)
	//			{
	//				if (g_game_control.winForceControlArr[index].addodds != 0 && g_game_control.winForceControlArr[index].addodds == lWinLoseProbability)
	//				{
	//					//�볡�ӳ�
	//					role_[chairID].timesDelay = Random(g_game_control.winForceControlArr[index].NextDelayMin, g_game_control.winForceControlArr[index].NextDelayMax);
	//					isHaveComp = true;

	//					//sprintf(logArray, "\n���ƽ��� �ٴ�����ӳٴ��� NextDelayMin:%d, NextDelayMax:%d, timesDelay:%d,  Score:%I64d,Pro:%d\n", g_game_control.winForceControlArr[index].NextDelayMin, g_game_control.winForceControlArr[index].NextDelayMax, role_[chairID].timesDelay, wlScore, wlProbability);
	//					//debugLog(logArray, userItem->GetUserID());
	//				}
	//			}
	//			if (!isHaveComp)
	//			{
	//				//�볡�ӳ�
	//				role_[chairID].timesDelay = Random(g_game_control.timesDelay.min, g_game_control.timesDelay.max);

	//				//sprintf(logArray, "\nû�ҵ�ƥ�� NextDelayMin:%d, NextDelayMax:%d, timesDelay:%d,  Score:%I64d,Pro:%d\n", g_game_control.timesDelay.min, g_game_control.timesDelay.max, role_[chairID].timesDelay, wlScore, wlProbability);
	//				//debugLog(logArray, userItem->GetUserID());
	//			}
	//			//sprintf(logArray, "\nlastTouchForceControl:%d, fireCount:%d\n", role_[chairID].lastTouchForceControl, role_[chairID].fireCount);
	//			//debugLog(logArray, userItem->GetUserID());
	//			role_[chairID].lastTouchForceControl = role_[chairID].fireCount;
	//		}
	//	}
	//}
//}

//Բ��
int GameTableLogic::RoundDesk(int count, int weightArr[])
{
	int sumWeight = 0, tempWeight = 0, index = 0, randWeight = 0;
	for (index = 0; index < count; index++)
		sumWeight += weightArr[index];
	randWeight = rand() % sumWeight;
	for (index = 0; index < count; index++)
	{
		tempWeight += weightArr[index];
		if (tempWeight > randWeight)
			return index;
	}
	return 0;
}
//WinForceControl GameTableLogic::selectConfigByPond(GamePlayer* userItem)
//{
//	/*
//	SCORE wlScore = 0;
//	int wlProbility = 0;
//	float wlAddProb = 1.0;  //ǿ�ص��Ӹ��� 
//	userItem->GetWinLoseCtr(wlScore, wlProbility);
//	SCORE nowPond = GameRoom::loadPond();
//	int selectRang = 0;
//	for (int index = 0; index < g_game_control.winForcePondLine.lineCount; index++)
//	{
//		if (nowPond <= g_game_control.winForcePondLine.pondLine[index])
//		{
//			selectRang = index;
//			break;
//		}
//		if (index == g_game_control.winForcePondLine.lineCount - 1 && nowPond > g_game_control.winForcePondLine.pondLine[index])
//		{
//			selectRang = index + 1;
//		}
//	}
//	int roundWight[MAX_CONTROL_NUM] = { 0 }, roundIndex[MAX_CONTROL_NUM] = { 0 }, selectIndex = 0;
//	WinForceSelectConfig selectConfig = g_game_control.winForceSelectConfig[selectRang];
//	if (wlProbility >= 100)
//	{
//		wlAddProb = g_game_control.WinToLoseAddProb;
//	}
//	else if (wlProbility != 0)
//	{
//		wlAddProb = g_game_control.LoseToWinAddProb;
//	}
//
//	for (int index = 0; index < selectConfig.groupCount; index++)
//	{
//		roundIndex[index] = selectConfig.groupIndex[index];
//		roundWight[index] = g_game_control.winForceControlArr[roundIndex[index]].prob;
//		if (wlProbility >= 100 && g_game_control.winForceControlArr[roundIndex[index]].addodds <= 100)
//		{
//			roundWight[index] *= wlAddProb;
//		}
//		if (wlProbility < 100 && g_game_control.winForceControlArr[roundIndex[index]].addodds > 100)
//		{
//			roundWight[index] *= wlAddProb;
//		}
//	}
//	selectIndex = roundIndex[RoundDesk(selectConfig.groupCount, roundWight)];
//
//	//char logArray[256];
//	//sprintf(logArray, "\n�ϴ�ǿ��Score:%I64d,Prob:%d,���õ���Prob:%f,ѡ������Index:%d\n", wlScore, wlProbility, wlAddProb, selectIndex);
//	//debugLog(logArray, userItem->GetUserID());
//	return g_game_control.winForceControlArr[selectIndex];
//	*/
//}

//����ʧ��
void GameTableLogic::SendFireFailed(unsigned short chairID)
{
	/*GamePlayer *pPlayer = m_pGameRoom->GetPlayer(chairID);
	if(pPlayer != NULL)
	{
		SVar sSend;
		sSend["chair_id_"] = chairID;
		sSend["nowGlod"]   = role_[chairID].get_gold();
		pPlayer->Send(SUB_S_FIRE_FAILED,&sSend);
	}*/

	if(m_pGame_Process->is_exist_user(chairID))
	{
		CMD_S_Fire_Failed csff;
		csff.chair_id_ = chairID;
		csff.nowGlod = (double)role_[chairID].get_gold() / 100.0f;
		m_pGame_Process->send_player_data(chairID, SUB_S_FIRE_FAILED, &csff, sizeof(CMD_S_Fire_Failed));
	}
}	

bool GameTableLogic::compute_collision(float bird_x, float bird_y, float bird_width, float bird_height, float bird_rotation,
								   float bullet_x, float bullet_y, float bullet_radius)
{
	float sint, cost;
	float w, h, rx, ry, r;

	cost = cosf(bird_rotation);
	sint = sinf(bird_rotation);

	w = bird_width;
	h = bird_height;
	r = bullet_radius;
	rx = (bullet_x - bird_x) * cost + (bullet_y - bird_y) * sint;
	ry = -(bullet_x - bird_x) * sint + (bullet_y - bird_y) * cost;

	float dx = std::min( rx, w*0.5f);
	dx = std::max( dx, -w*0.5f);

	float dy = std::min( ry, h*0.5f);
	dy = std::max( dy, -h*0.5f);

	return (rx - dx)*(rx - dx) + (ry - dy)*(ry - dy) <= r*r;
}

bool GameTableLogic::compute_energy_collision(float bird_x, float bird_y, float bird_width, float bird_height, float bird_rotation, 
										  float bullet_x, float bullet_y, float bullet_width, float bullet_height, float bullet_rotation)
{
	float bird_x1, bird_x2, bird_y1, bird_y2, bullet_x1, bullet_x2, bullet_y1, bullet_y2;

	float bird_r = std::sqrt((bird_width/2) * (bird_width/2) + (bird_height/2) * (bird_height/2));
	bird_x1 = bird_x + bird_r*cosf(bird_rotation);
	bird_y1 = bird_y + bird_r*sinf(bird_rotation);
	if (bird_x1 > bird_x)
		bird_x2 = bird_x - (bird_x1 - bird_x);
	else
		bird_x2 = bird_x + (bird_x1 - bird_x);
	if (bird_y1 > bird_y)
		bird_y2 = bird_y + (bird_y1 - bird_y);
	else
		bird_y2 = bird_y + (bird_y1 - bird_y);

	float bullet_r = kScreenHeight / sinf(bullet_rotation);
	bullet_x2 = bullet_x + bullet_r* cosf(bullet_rotation);
	bullet_y2 = bullet_y + bullet_r* sinf(bullet_rotation);
	bullet_x1 = bullet_x + 10;
	bullet_y1 = bullet_y;
	bullet_x2 = bullet_x2 -= 10;

	if (bird_x1 >= bullet_x1 && bird_x1 >= bullet_x2)
		return false;
	else if (bird_x1 <= bullet_x1 && bird_x2 <= bullet_x)
		return false;
	else if (bird_y1 >= bullet_y1 && bird_y2 >= bullet_y1)
		return false;
	else if (bird_y1 <= bullet_y1 && bird_y2 <= bullet_y1)
		return false;

	return true;
}

int GameTableLogic::compute_distance(float x1, float y1, float x2, float y2)
{
	return (x1 - x2) * (x1 - x2) + (y1 - y2) * (y1 - y2) ;
}
//����ȫ�ֽ���ϵ��
void GameTableLogic::calculatorPondCoefficient()
{
	//û�е�������
	/*SCORE nowPond = GameRoom::loadPond();
	if (nowPond < pond_control.m_systemPondLineOne)
	{
		g_game_control.m_nowPondCoefficient = Random_float(pond_control.m_systemPondCoefficientOneMin,pond_control.m_systemPondCoefficientOneMax);
	}
	else if (pond_control.m_systemPondLineOne <= nowPond && nowPond < pond_control.m_systemPondLineTwo)
	{
		g_game_control.m_nowPondCoefficient = Random_float(pond_control.m_systemPondCoefficientTwoMin, pond_control.m_systemPondCoefficientTwoMax);
	}
	else if (pond_control.m_systemPondLineTwo <= nowPond && nowPond < pond_control.m_systemPondLineThree)
	{
		g_game_control.m_nowPondCoefficient = Random_float(pond_control.m_systemPondCoefficientThreeMin, pond_control.m_systemPondCoefficientThreeMax);
	}
	else if (pond_control.m_systemPondLineThree <= nowPond && nowPond < pond_control.m_systemPondLineFour)
	{
		g_game_control.m_nowPondCoefficient = Random_float(pond_control.m_systemPondCoefficientFourMin, pond_control.m_systemPondCoefficientFourMax);
	}
	else if (pond_control.m_systemPondLineFour <= nowPond)
	{
		g_game_control.m_nowPondCoefficient = Random_float(pond_control.m_systemPondCoefficientFiveMin, pond_control.m_systemPondCoefficientFiveMax);
	}*/
}

//���㵱ǰ��ҵľ�����ʵ��ֵ û�е��� ����
/*void GameTableLogic::calculatorForceThreshold(unsigned short wChairID, GamePlayer * pGamePlayer)
{
	*//*
	//����󾻷���ֵ�� ����볡���*������ȡ�ϴ�ֵ ��ֵ��Ϊ����
	m_userMaxForceThreshold[wChairID] = max(g_game_control.m_MaxForceThreshold, pGamePlayer->GetPlayerID() * g_game_control.m_MaxForceThresholdAndGold);
	//����С������ֵ�� ����볡���*������ȡ��Сֵ ��ֵ��Ϊ����
	m_userMinForceThreshold[wChairID] = min(g_game_control.m_MinForceThreshold, pGamePlayer->GetPlayerID() * g_game_control.m_MinForceThresholdAndGold);
	//�ж���û����Ӯǿ�ط���
	SCORE wlScore = 0;
	int wlProbability = 0;
	pGamePlayer->GetWinLoseCtr(wlScore, wlProbability);
	char logArray[256];
	//sprintf(logArray, "\n����,��ֵmax:%I64d,min:%I64d,ǿ��score:%I64d,prob:%d\n", m_userMaxForceThreshold[wChairID], m_userMinForceThreshold[wChairID], wlScore, wlProbability);
	//debugLog(logArray, pGamePlayer->GetUserID());
	
	//���û����Ӯǿ�Ʒֿ���  Ҫ���ϡ��� 0
	if (wlScore <= 0 || wlProbability <= 0)
	{
		role_[wChairID].timesDelay = Random(g_game_control.timesDelay.min, g_game_control.timesDelay.max);
		//sprintf(logArray, "\nû�п��� ����timesDelay:%d", role_[wChairID].timesDelay);
		//debugLog(logArray, pGamePlayer->GetUserID());
	}
	*//*
}*/

//��ҵ�� û�е��� ����
/*bool GameTableLogic::CheckPlayer(GamePlayer *pGamePlayer)
{
	//if(pGamePlayer == NULL) return false;

	////����������Ӯ
	//int64_t lPlayerWinScore = pGamePlayer->GetPlayerWinScore();

	////�������䲻Ӯ�������
	//if(lPlayerWinScore <= 0)
	//{
	//	return true;
	//}
	////���ӮǮ,��ʼ���
	//else
	//{

	//}
	return true;
}*/

////�ж����Ƿ��ܹ�������
//bool GameTableLogic::ComputeProbability(GamePlayer *pGamePlayer,int nBulletMul, int nFishKind, int nFishMul, int nChairID, Bird* pFish, bool bIsDouble)
//{
//	//Logger PlayerLog("��Ҽ�¼", m_pGameRoom->GetRoomID());
//	//�ܻ����
//	int64_t lHitScore  = pFish->GetHitScore();
//	//עѪ���
//	int64_t lPlusScore = pFish->GetPlusScore();
//	//����������
//	int nFishDieMul  = pFish->GetFishDieMul();
//	//����������ӵ���
//	//int nMaxBullet   = pFish->GetMaxBullet();
//
//	if(BIRD_TYPE_TWO == nFishKind)
//	{
//		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
//	}
//	else if(BIRD_TYPE_THREE == nFishKind)
//	{
//		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
//	}
//	else if(BIRD_TYPE_FOUR == nFishKind)
//	{
//		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
//	}
//	else if(BIRD_TYPE_FIVE == nFishKind)
//	{
//		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
//	}
//
//	//cout << lHitScore <<" + " << lPlusScore <<"|" << nFishDieMul << "|" << nMaxBullet <<endl;
//	//��������+����Ѫ
//	if ((lHitScore + lPlusScore) < (nFishDieMul * nBulletMul))
//	{/*
//		PlayerLog.TraceInfo("[�����ж�] Ѫֵδ���� ��:%I64d ����%.2f עѪ:%.2f ��������%d ���:%.2f", pFish->get_id(),
//			                                                                                           (double)(lHitScore / GOLD_RATIO), 
//																									   (double)(lPlusScore / GOLD_RATIO), 
//																									   nFishDieMul,
//																									   (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );	*/
//		return false;
//	}
//
//	//��������,�жϸ���
//	srand((unsigned int)((unsigned int)time(NULL) + (unsigned int)pGamePlayer->GetSeat()));
//
//	//��ҵ��
//	if(!CheckPlayer(pGamePlayer))
//	{
//		return false;
//	}
//
//	//������һ�������Ľ��
//	int64_t lPlayerHitScore = pFish->GetChairScore(nChairID);
//
//	unsigned short wPlayerProbability = (lPlayerHitScore * 1000 / lHitScore);
//	if (rand() % 1000 > wPlayerProbability)
//	{
//		//PlayerLog.TraceInfo("[�����ж�] ����δ����", pFish->get_id());	
//		return false;
//	}
//
//	return true;
//}

//bool GameTableLogic::ComputeProbability(int bulltMul, int fishKind, int fishMultiple, int nChairID, Bird *bird, int superpositionProbability)
//{
//	GamePlayer* pGamePlayer = m_pGameRoom->GetPlayer(nChairID);
//	if (pGamePlayer == NULL)
//	{
//		return false;
//	}
//
//	int probability = Random(1,10000);
//	SCORE forceScore    = 0L;
//	SCORE clearScore    = 0L;
//	SCORE calClearScore = 0L;	//ǿ���� ����
//
//	pGamePlayer->GetControlPlusInfo(forceScore, clearScore);
//	calClearScore = clearScore; //����Ӱ��ľ���
//	if (calClearScore > 0)
//	{
//		//��������ڱ�
//		calClearScore *= g_game_control.m_roomMaxbet / bulltMul;
//	}
//
//	//���·��
//	int byTheWay = 0;
//	//������֮�� ��ʼ�����ж��߼�
//	
//	//ǿ����ˮ
//	int _tmpRuleQFCS = 0;
//	//���ֿ���
//	int _tmpRuleJFKZ = 0;
//	//ǿ���ز�
//	int _tmpRuleQFHB = 0;
//	//�Ƿ�ǿ���ز���������
//	bool isTouchQFHB = false;
//	//����Ӱ��
//	double forceInfluence = 1.0;
//	//��Ļ����������  Ŀǰ�����õ���С�������
//	double baseFishProbability, bird_probability, pondCoefficient;
//	//����ȫ�ָ���Ӱ��
//	pondCoefficient = ((g_game_control.m_nowPondCoefficient - 1) * bulltMul / g_game_control.m_roomMaxbet) + 1;
//
//	//���и���
//	if (fishKind < MAX_BIRD_TYPE)
//	{
//		//��ȡĬ�ϲ�����ĸ���
//		bird_probability = g_game_config_xml.bird_config_[fishKind].capture_probability;
//	}
//	else
//	{
//		//����
//		if (fishKind == BIRD_TYPE_RED)
//		{
//			if (bird->get_yuzhen_type() == YUZHEN_LINEAR)
//			{
//				bird_probability = g_game_config_xml.bird_red_bloating_config_.capture_probability[bird->get_item()];
//			}
//			else
//			{
//				bird_probability = g_game_config_xml.bird_red_series_config_.capture_probability[bird->get_item()];
//			}
//		}
//		else if (bird->get_type() >= BIRD_TYPE_ONE && bird->get_type() <= BIRD_TYPE_FIVE)
//		{
//			bird_probability = g_game_config_xml.bird_same_config_[bird->get_type() - BIRD_TYPE_ONE].capture_probability[bird->get_item()];
//		}
//	}
//	baseFishProbability = bird_probability;
//	int randProbability = Random(1,10000);
//	//ϵͳ��ˮ�ж� 
//	int tmpTaxOdd = Random(1,10000);
//	//��λ
//	int bDeskStation = pGamePlayer->GetSeat();
//	//�����
//	int64_t fish_score = 0;
//	//���������
//	//һ������������
//	//һ������������
//	
//	fish_score = fishMultiple * bulltMul;
//
//	if (tmpTaxOdd <= g_game_control.m_Taxodd)
//	{
//		//  GameTaxoddSwitch
//		if (g_game_control.m_TaxoddSwitch == 1)  //������ϵͳ��ˮ����
//		{
//			//��ˮ���� ��ȫ���ı���Ҿ��� �Լ�����
//			int tempFoce = (int)((10000 - g_game_control.m_foceTaxodd) / 10000.f * bulltMul);
//			clearScore += tempFoce;
//			int tempBulltMul = (10000 - g_game_control.m_SystemPondTaxodd) / 10000.f * bulltMul;
//			int64_t tmpGold =-tempBulltMul;
//			if (tmpGold > 0)
//			{
//				tmpGold = (double)(10000 - g_game_control.m_SystemPondTaxodd) / 10000 * tmpGold;
//			}
//			GameRoom::setPond(tmpGold);
//		}
//		fish_score = 0;
//		byTheWay = 1;
//		goto Account;
//	}
//	
//	//�������ļ������õ�ÿ��������Զ����� �����������ȥ�ж�ǿ����ˮ
//	if (fishKind < MAX_BIRD_TYPE)
//	{
//		_tmpRuleQFCS = g_game_control.m_arrayFishContorlRule[fishKind].ForceChouShui;
//		_tmpRuleQFHB = g_game_control.m_arrayFishContorlRule[fishKind].ForceHuiBu;
//		_tmpRuleJFKZ = g_game_control.m_arrayFishContorlRule[fishKind].ScoreContorl;
//	}
//	//ǿ�����ˮ�ж�
//	int tmpInOdd = Random(1, 10000);
//	//if (m_i64fish_SupperControl_Money_[bDeskStation].limit_score_ == 0)//���ڳ��˿���
//	{
//		byTheWay = 2;
//		if (_tmpRuleQFCS == 1)//����
//		{
//			if (tmpInOdd <= g_game_control.m_bForceBigFishOdd) //���ӵ�����ǿ����ļ���
//			{
//				forceScore += bulltMul;
//				fish_score = 0;
//				goto Account;
//			}
//		}
//		else if (_tmpRuleQFCS == 0)//С��
//		{
//			if (tmpInOdd <= g_game_control.m_bForceSmallFishOdd)//���ӵ�����ǿ����(20%)�ļ���
//			{
//				forceScore += bulltMul;
//				fish_score = 0;
//				goto Account;
//			}
//		}
//	}
//	// �������������ʼ���
//	//�Ƿ���ǿ���ز�����ж�
//	if (_tmpRuleQFHB == 1)
//	{
//		//ǿ������
//		if (role_[nChairID].FoceSwitch)
//		{
//			if (forceScore >= fish_score)
//			{
//				role_[nChairID].FoceSwitch = true;
//				bird_probability = bird_probability * g_game_control.m_bForceUpgradeMulti;
//				isTouchQFHB = true;
//			}
//			else
//			{
//				role_[nChairID].FoceSwitch = false;
//			}
//		}
//		else
//		{
//			//ǿ���� <= ��ı���*�ӵ�����*ǿ�������ϵ��
//			if (forceScore <= fish_score *  g_game_control.m_bForceMustOdd)
//			{
//				//����ʧ�� �ӵ����
//				forceScore += bulltMul;
//				//��¼δ��������
//				m_iUserFireCount[bDeskStation]++;
//				fish_score = 0;
//				byTheWay = 3;
//				goto Account;
//			}
//			else
//			{
//				if (forceScore >= g_game_control.m_fishMulitiple * bulltMul * g_game_control.m_bForceMulti)
//				{
//					role_[nChairID].FoceSwitch = true;
//					bird_probability = bird_probability * g_game_control.m_bForceUpgradeMulti;
//					isTouchQFHB = true;
//				}
//				else
//				{
//					if (forceScore >= fish_score && m_iUserFireCount[bDeskStation] >= g_game_control.m_bForceMissCount)
//					{
//						bird_probability = bird_probability * g_game_control.m_bForceUpgradeMulti;
//						isTouchQFHB = true;
//					}
//					else
//						role_[nChairID].FoceSwitch = false;
//				}
//			}
//		}
//
//		//���������ǿ���ز� ��ôֱ�Ӽ���������ĸ���  * ȫ�ָ���
//		if (isTouchQFHB)
//		{
//			bird_probability = bird_probability * pondCoefficient;
//		}
//	}
//
//	//�������ǿ���������� ����ǿ������ʧ��
//	if (!isTouchQFHB || _tmpRuleQFHB == 0)
//	{
//		//����Ǿ��ֿ��Ƶ���
//		if (_tmpRuleJFKZ == 1 /*&& m_i64fish_SupperControl_Money_[bDeskStation].limit_score_ == 0*/)
//		{
//			double  forceMaxValue, forceMinValue;
//			forceMaxValue = forceMinValue = 0.0;
//			//С����󾻷���ֵ
//			if (calClearScore < 0 && calClearScore <= m_userMaxForceThreshold[bDeskStation])
//			{
//				//���ֿ���Ӱ�� = 1 + ((��󾻷�ϵ�� / ����ϵ��) * (-1) * (����ֵ - �����ֵ) / ����������ڱ��� * ��������㱶�� * ��󾻷�ϵ��������)
//				forceInfluence = 1 + ((g_game_control.m_maxWinratio / g_game_control.m_losSratio) * (-1) *(double)(calClearScore - m_userMaxForceThreshold[bDeskStation]) / (g_game_control.m_roomMaxbet * g_game_control.m_fishMulitiple * g_game_control.m_wininf));
//				forceMaxValue = 1 + g_game_control.m_maxWinratio / g_game_control.m_losSratio;
//				forceInfluence = min(forceInfluence, forceMaxValue);
//				bird_probability = bird_probability * forceInfluence;
//			}
//			//������С������ֵ
//			else if (calClearScore > 0 && calClearScore >= m_userMinForceThreshold[bDeskStation])
//			{
//				//���ֿ���Ӱ�� = 1 - ((��С����ϵ�� / ����ϵ��)  * (����ֵ - ��С��ֵ) / ����������ڱ��� * ��������㱶�� * ��С����ϵ��������)
//				forceInfluence = 1 - ((g_game_control.m_minWinratio / g_game_control.m_losSratio) * (double)(calClearScore - m_userMinForceThreshold[bDeskStation]) / (g_game_control.m_roomMaxbet * g_game_control.m_fishMulitiple * g_game_control.m_loseinf));
//				forceMinValue = 1 - g_game_control.m_minWinratio / g_game_control.m_losSratio;
//				forceInfluence = max(forceInfluence, forceMinValue);
//				bird_probability = bird_probability * forceInfluence;
//			}
//			else
//			{
//				//û�дﵽ���Ƶ���ֵ
//			}
//
//			bird_probability = bird_probability * pondCoefficient;
//		}
//		//������Ǿ��ֿ��Ƶ���
//		else if (_tmpRuleJFKZ == 0 /*&& m_i64fish_SupperControl_Money_[bDeskStation].limit_score_ == 0*/)
//		{
//			bird_probability = bird_probability * pondCoefficient;
//		}
//	}
//	// �����жϲ�����
//	//����50%��ֱ��50%
//	bird_probability = min(0.5, bird_probability * superpositionProbability / 100.0);
//	//�����ж�
//	if (randProbability < bird_probability * 10000)
//	{
//		//����ɹ� ��������
//		byTheWay = 4;
//		if (_tmpRuleQFHB && isTouchQFHB)
//		{
//			//ǿ�����޸�
//			forceScore -= fish_score;
//			m_iUserFireCount[bDeskStation] = 0;
//			isTouchQFHB = 0;
//			byTheWay = 5;
//		}
//		else if (_tmpRuleQFHB && !isTouchQFHB)
//		{
//			m_iUserFireCount[bDeskStation] = 0;
//			byTheWay = 6;
//		}
//		goto Account;
//	}
//	else
//	{
//		byTheWay = 7;
//		if (_tmpRuleQFHB)
//		{
//			byTheWay = 8;
//			//����ʧ�� ǿ���ز����� ǿ��δ������+1
//			m_iUserFireCount[bDeskStation] += 1;
//
//		}
//		fish_score = 0;
//		goto Account;
//
//	}
//	//���ּ���
//Account:
//	//��־����
//	if (g_game_control.logSwitch)
//	{
//		char logArray[1024];//ǿ������������%d,
//		/*sprintf(logArray, "�㣺%d\t�ڣ�%d\t·��%d\t�棺%d\t����%d\t�գ�%d\tǿ��%I64d\t����%I64d\t��Ӱ�죺%f\t���أ�%I64d\t����ϵ����%f\t��ң�%I64d",
//			fishMultiple, bulltMul, byTheWay, randProbability, (int)(baseFishProbability * 10000), (int)(bird_probability * 10000), forceScore, clearScore,  forceInfluence, m_pGameRoom->loadPond(), pondCoefficient, role_[user_item->GetChairID()].get_gold());*/
//		// �㱶 �ڱ�  �ɹ���ʧ�ܵı�ǣ�·���� ���ֵ ��Ļ������� ���ո��� ǿ���� ǿ��Ӱ��(��������) ���� ����Ӱ�� ȫ�ֽ��� ȫ�ֽ��ص�ϵ�� ��ǰ�����
//		//debugLog(logArray, user_item->GetUserID());
//	}
//	pGamePlayer->SetControlPlusInfo(forceScore, clearScore);
//	if (fish_score == 0)
//	{
//		return false;
//	}
//	//�����˲����㽱��
//	
//	return true;
//
//}

//�����־
void GameTableLogic::debugLog(char *logStr, unsigned int userID)
{
	//if (!g_game_control.logSwitch)
	//{
	//	return;
	//}
	//char szFilename[256];
	////sprintf(szFilename, "E:\\yunding\\dntgLog\\%d_%d_%S.txt", m_pGameRoom->m_pGameServiceOption->wServerID, userID, CTime::GetCurrentTime().Format("%d-%H"));
	//FILE *fp = fopen(szFilename, "a");
	//if (NULL == fp)
	//{
	//	return;
	//}
	//fprintf(fp, "[%S]--%s\n", CTime::GetCurrentTime().Format("%H:%M:%S"),logStr);

	//fclose(fp);
}


//����
void GameTableLogic::red_bird_event(Bullet* bullet, Bird* bird, unsigned short charid)
{

}

void GameTableLogic::bomb_check(Bullet* bullet, Bird *bomb_bird)
{
	
}

//û�е��� ����
/*void GameTableLogic::CalGameScore(unsigned short wChairID, GamePlayer* pGamePlayer)
{
	
}*/

void GameTableLogic::android_update(float delta_time)
{
	return;
}

//*****������� ���� ���� ����******
//��λ�ü��
bool GameTableLogic::inside_screen(Bird* bird)
{
	Point position = bird->position();
	Size sz_bird;
	if (bird->get_type() < MAX_BIRD_TYPE)
	{
		sz_bird = g_game_config_xml.bird_config_[bird->get_type()].bird_size;
	}
	else
	{
		if (bird->get_type() == BIRD_TYPE_CHAIN || bird->get_type() == BIRD_TYPE_RED)
		{
			sz_bird = g_game_config_xml.bird_config_[bird->get_item()].bird_size;
		}
		else
		{
			sz_bird = g_game_config_xml.bird_same_config_[bird->get_type() - BIRD_TYPE_ONE].bird_size;
		}
	}

	if ((position.x_ - sz_bird.width_/2 >= 0 && position.x_ + sz_bird.width_/2 <= kScreenWidth) || 
		(position.y_ - sz_bird.height_/2 >= 0 && position.y_ + sz_bird.height_/2 <= kScreenHeight))
	{
		return true;
	}
	return false;
}
//ȡ������
bool GameTableLogic::cancel_bullet_lock(Entity * target, uint32_t data)
{
	Bullet* bullet = static_cast<Bullet*>(target);
	if (!bullet)
	{
		return false;
	}

	bullet->set_lock_bird_id(-1);
	Action* action = new Action_Bullet_Move(bullet->rotation(), bullet->bullet_speed());
	//Action* action = new Action_Sequence(new Action_Bullet_Move(bullet->rotation(), bullet->bullet_speed()), new Action_Func1(fastdelegate::MakeDelegate(this, &Game_Table::cancel_bullet_Temp), 0), 0);
	action_manager_.add_action(action, bullet, false);
	bullet->set_action_id(action->get_tag());

	return true;
}

bool GameTableLogic::cancel_bullet_Temp(Entity * target, uint32_t data)
{
	return true;
}

float GameTableLogic::bullet_move_duration(const Point& start, const Point end, float bullet_speed)
{
	Point delta = end - start;
	float length = std::sqrt(delta.x_ * delta.x_ + delta.y_ * delta.y_);
	return length / bullet_speed;
}

//��ȡ���������
int GameTableLogic::GetFishBaseMul(int nFishType)
{
	int nFishMul = 0;

	//BOSS
	if(nFishType == BOSS_FISH)
	{
		nFishMul = g_game_config_xml.bird_config_[nFishType].mulriple_max;
	}
	//С��ը��
	else if(nFishType == BOMB_SMALL)
	{
		nFishMul = g_game_config_xml.bird_config_[nFishType].mulriple_max;
	}
	//����ը��
	else if(nFishType == BOMB_LARGE)
	{
		nFishMul = 30;
	}
	//ȫ��ը��(ը��)
	//else if(nFishType == BOMB_ULTIMATELY)
	//{
	//	nFishMul = 600;
	//}
	else
	{
		nFishMul = g_game_config_xml.bird_config_[nFishType].mulriple_max;
	}

	return nFishMul;
}


//����Ѫ��
void GameTableLogic::SetFishScore(Bird *pFish, int nFishType)
{
	//��¼��������
	pFish->m_tCreatTime = time(NULL);

	//��ȡ��Ļ�������
	uint32_t nFishBaseMul = pFish->get_mulriple();/*GetFishBaseMul(nFishType);	*/
	pFish->SetFishBaseMul(nFishBaseMul);

	//��������
	uint32_t nFishDieMul = nFishBaseMul + (rand() % 2 + 1);
	pFish->SetFishDieMul(nFishDieMul);

	//�����е������
	uint32_t nMinBullet = g_game_config_xml.cannon_mulriple_[CANNON_TYPE_0];
	pFish->SetMaxBullet(nMinBullet);

	//��������
	pFish->SetPlusScore(0L);
	//������
	pFish->SetHitScore(0L);
	//��������,����
	pFish->ZeroChairScore();
}

//û�е�������
/*//�����㸽��Ѫ��
void GameTableLogic::SetFishPlusScore(GamePlayer *pGamePlayer, Bird *pFish, int nBulletMul,int nFishMul, int nFishType)
{
	if(pGamePlayer == NULL)
	{
		return;
	}

	if(pFish == NULL) 
	{
		return;
	}
	//Logger PlayerLog("��Ҽ�¼", m_pGameRoom->GetRoomID());

	//PlayerLog.TraceInfo("[עѪ0] ��:%I64d ����Ѫ:%.2f ���:%.2f", pFish->get_id(), (double)(pFish->GetPlusScore() / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );
	//С��
	if(nFishType <= SMALL_BIRD_FLAG)
	{
		//С����
		int64_t lCurrentSmallStock = INSTANCE(CBYStockConfig)->GetSmallStock();
		//С�ڿ��עѪ��ֵ
		int64_t lSmallStockLimit   = INSTANCE(CBYStockConfig)->GetSmallStockLimit();

		//С����Ϊ0
		if(lCurrentSmallStock < 0L)
		{
			//PlayerLog.TraceInfo("[עѪ1] С����Ϊ0 %.2f",  (double)(lCurrentSmallStock / GOLD_RATIO));
			return;
		}
		//С����ﲻ��עѪ����
		if(lCurrentSmallStock < lSmallStockLimit)
		{
			//PlayerLog.TraceInfo("[עѪ1] С��ﲻ��עˮ���� %.2f_%.2f",  (double)(lCurrentSmallStock / GOLD_RATIO), (double)(lSmallStockLimit / GOLD_RATIO));
			return;
		}

		//��ȡ�����Ӯ���
		int64_t lPlayerWinScore = pGamePlayer->GetPlayerWinScore();

		//�����ӮǮ״̬,��עѪ
		if(lPlayerWinScore >= 0L)
		{
			if(rand() % 100 < INSTANCE(CBYStockConfig)->GetWinSmallPlusRate())
			{
				//��ȡ�㱻��������
				int64_t lHitScore       = pFish->GetHitScore();
				//ץ�����ý����
				//int64_t lCatchScore     = pFish->GetFishDieMul() * nBulletMul;
				//�����ܻ�ý����
				int64_t lCatchScore     = pFish->GetFishDieMul() * pFish->GetMaxBullet();
				//��ʣ���ٽ�ҽ����ܱ�����
				int64_t lDiffScore      = lCatchScore -  lHitScore;

				//���ȫ��ע����Ȼ��������������ע��
				if(lCurrentSmallStock <= lDiffScore)
				{
					pFish->SetPlusScore(0L);
					return;
				}

				//���Ѿ����㱻�������������ע
				if(lDiffScore < 0L)
				{
					pFish->SetPlusScore(0L);
					return;
				}
				else
				{
					//עѪ����(С��)
					int nSmallPlusRate = INSTANCE(CBYStockConfig)->GetSmallPlusRate();
					//�жϸ���
					if (rand() % 100 < nSmallPlusRate)
					{
						int64_t lReduScore = 0L;
						if((lPlayerWinScore  + lCatchScore) < 0)
						{	
							lReduScore = lDiffScore;
							//��עѪ
							pFish->SetPlusScore(lReduScore);
	
							//�����
							INSTANCE(CBYStockConfig)->SetSmallStock(-lReduScore);
							//�ܿ��
							INSTANCE(CBYStockConfig)->SetCurrentStock(-lReduScore);

							//PlayerLog.TraceInfo("[עѪ1] ��:%I64d ������:%.2f ���:%.2f", pFish->get_id(), (double)(lReduScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );
						}
						else
						{
							//PlayerLog.TraceInfo("[עѪ4] ���ӮǮ %.2f_%.2f", (double)(lPlayerWinScore/GOLD_RATIO), (double)(lCatchScore/GOLD_RATIO));
						}
					}
				}
			}
			else
			{
				//PlayerLog.TraceInfo("[עѪ1] ���ӮǮ,��עѪ %d",  INSTANCE(CBYStockConfig)->GetWinSmallPlusRate());
			}
			return;
		}
		//�����Ǯ״̬
		else
		{
			//��ȡ�㱻��������
			int64_t lHitScore       = pFish->GetHitScore();
			//ץ�����ý����
			//int64_t lCatchScore     = pFish->GetFishDieMul() * nBulletMul;
			//�����ܻ�ý����
			int64_t lCatchScore     = pFish->GetFishDieMul() * pFish->GetMaxBullet();
			//��ʣ���ٽ�ҽ����ܱ�����
			int64_t lDiffScore      = lCatchScore -  lHitScore;

			//���ȫ��ע����Ȼ��������������ע��
			if(lCurrentSmallStock <= lDiffScore)
			{
				//PlayerLog.TraceInfo("[עѪ2] ���ȫ��ע����Ȼ��������������ע %.2f_%I64d", (double)(lCurrentSmallStock / GOLD_RATIO), lDiffScore);
				pFish->SetPlusScore(0L);
				return;
			}

			//���Ѿ����㱻�������������ע
			if(lDiffScore < 0L)
			{
				//PlayerLog.TraceInfo("[עѪ2]���Ѿ����㱻�������������ע %I64d", lDiffScore);
				pFish->SetPlusScore(0L);
				return;
			}
			//�㻹�����㱻��������������������Ӯ��ע��
			else
			{
				//עѪ����(С��)
				int nSmallPlusRate = INSTANCE(CBYStockConfig)->GetSmallPlusRate();
				//�жϸ���
				if (rand() % 100 < nSmallPlusRate)
				{
					int64_t lReduScore = 0L;
					if((lPlayerWinScore  + lCatchScore) < 0)
					{	
						lReduScore = lDiffScore;
						//��עѪ
						pFish->SetPlusScore(lReduScore);

						//�����
						INSTANCE(CBYStockConfig)->SetSmallStock(-lReduScore);
						//�ܿ��
						INSTANCE(CBYStockConfig)->SetCurrentStock(-lReduScore);

						//PlayerLog.TraceInfo("[עѪ2]��:%I64d ������:%.2f ���:%.2f",  pFish->get_id(), (double)(lReduScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );
					}
				}
				else
				{
					//PlayerLog.TraceInfo("[עѪ2]�����Ǯ,ע�����ʧ�� %d", INSTANCE(CBYStockConfig)->GetSmallPlusRate());
				}
			}
		}
	}
	else
	{
		//������
		int64_t lCurrentBigStock = INSTANCE(CBYStockConfig)->GetBigStock();
		//С����עѪ��ֵ
		int64_t lBigStockLimit   = INSTANCE(CBYStockConfig)->GetBigStockLimit();

		//С����Ϊ0
		if(lCurrentBigStock < 0L)
		{
			//PlayerLog.TraceInfo("[עѪ4] ������Ϊ0 %.2f",  (double)(lCurrentBigStock / GOLD_RATIO));
			return;
		}
		//С����ﲻ��עѪ����
		if(lCurrentBigStock < lBigStockLimit)
		{
			//PlayerLog.TraceInfo("[עѪ4] ����ﲻ��עˮ���� %.2f_%.2f",  (double)(lCurrentBigStock / GOLD_RATIO), (double)(lBigStockLimit / GOLD_RATIO));
			return;
		}

		//��ȡ�����Ӯ���
		int64_t lPlayerWinScore = pGamePlayer->GetPlayerWinScore();

		//�����ӮǮ״̬,��עѪ
		if(lPlayerWinScore >= 0L)
		{
			//���ӮǮ��ƽ�⣬��25%�ļ���עѪ
			if(rand() % 100 < INSTANCE(CBYStockConfig)->GetWinBigPlusRate())
			{
				//��ȡ�㱻��������
				int64_t lHitScore       = pFish->GetHitScore();
				//ץ�����ý����
				//int64_t lCatchScore     = pFish->GetFishDieMul() * nBulletMul;
				
				//�����ܻ�ý����
				int64_t lCatchScore     = pFish->GetFishDieMul() * pFish->GetMaxBullet();

				if(nFishType == BIRD_TYPE_TWO)
				{
					//PlayerLog.TraceInfo("[עѪ4_1] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
					lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
				}
				else if(nFishType == BIRD_TYPE_THREE)
				{
					//PlayerLog.TraceInfo("[עѪ4_2] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
					lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
				}
				else if(nFishType == BIRD_TYPE_FOUR)
				{
					//PlayerLog.TraceInfo("[עѪ4_3] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
					lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
				}
				else if(nFishType == BIRD_TYPE_FIVE)
				{
					//PlayerLog.TraceInfo("[עѪ4_4] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
					lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
				}

				//��ʣ���ٽ�ҽ����ܱ�����
				int64_t lDiffScore      = lCatchScore -  lHitScore;
				
				//���ȫ��ע����Ȼ��������������ע��
				if(lCurrentBigStock <= lDiffScore)
				{
					//PlayerLog.TraceInfo("[עѪ4_3] ���ȫ��ע����Ȼ��������������ע��");
					pFish->SetPlusScore(0L);
					return;
				}

				//���Ѿ����㱻�������������ע
				if(lDiffScore < 0L)
				{
					//PlayerLog.TraceInfo("[עѪ4_3] ���ȫ��ע����Ȼ��������������ע��" );
					pFish->SetPlusScore(0L);
					return;
				}
				else
				{
					//עѪ����(����)
					int nBigPlusRate = INSTANCE(CBYStockConfig)->GetBigPlusRate();
					if (rand() % 100 < nBigPlusRate)
					{
						int64_t lReduScore = 0L;
						if((lPlayerWinScore  + lCatchScore) < 0)
						{	
							lReduScore = lDiffScore;
							//��עѪ
							pFish->SetPlusScore(lReduScore);
							//�����
							INSTANCE(CBYStockConfig)->SetBigStock(-lReduScore);
							//�ܿ��
							INSTANCE(CBYStockConfig)->SetCurrentStock(-lReduScore);

							//PlayerLog.TraceInfo("[עѪ4] ��%I64d ������:%.2f ���:%.2f",  pFish->get_id(), (double)(lReduScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );
						}
						else
						{
							//PlayerLog.TraceInfo("[עѪ4] ���ӮǮ %.2f_%.2f", (double)(lPlayerWinScore/GOLD_RATIO), (double)(lCatchScore/GOLD_RATIO));
						}
					}
				}
			}
			else
			{
				//PlayerLog.TraceInfo("[עѪ4]���ӮǮ,����ʧ�� %d",  INSTANCE(CBYStockConfig)->GetBigPlusRate());
			}
			return;
		}
		else
		{
			//��ȡ�㱻��������
			int64_t lHitScore       = pFish->GetHitScore();
			//ץ�����ý����
			//int64_t lCatchScore     = pFish->GetFishDieMul() * nBulletMul;
			//�����ܻ�ý����
			int64_t lCatchScore     = pFish->GetFishDieMul() * pFish->GetMaxBullet();

			if(nFishType == BIRD_TYPE_TWO)
			{
				//PlayerLog.TraceInfo("[עѪ4_1] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
				lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
			}
			else if(nFishType == BIRD_TYPE_THREE)
			{
				//PlayerLog.TraceInfo("[עѪ4_2] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
				lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
			}
			else if(nFishType == BIRD_TYPE_FOUR)
			{
				//PlayerLog.TraceInfo("[עѪ4_3] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
				lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
			}
			else if(nFishType == BIRD_TYPE_FIVE)
			{
				//PlayerLog.TraceInfo("[עѪ4_4] %d_%d", nFishType,  g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count);
				lCatchScore *= g_game_config_xml.bird_same_config_[nFishType - BIRD_TYPE_ONE].bird_count; //�����������
			}

			//��ʣ���ٽ�ҽ����ܱ�����
			int64_t lDiffScore      = lCatchScore -  lHitScore;

			//���ȫ��ע����Ȼ��������������ע��
			if(lCurrentBigStock <= lDiffScore)
			{
				//PlayerLog.TraceInfo("[עѪ4_5] ���ȫ��ע����Ȼ��������������ע��");
				pFish->SetPlusScore(0L);
				return;
			}

			//���Ѿ����㱻�������������ע
			if(lDiffScore < 0L)
			{
				//PlayerLog.TraceInfo("[עѪ4_6] ���ȫ��ע����Ȼ��������������ע��");
				pFish->SetPlusScore(0L);
				return;
			}
			//�㻹�����㱻��������������������Ӯ��ע��
			else
			{
				//עѪ����(����)
				int nBigPlusRate = INSTANCE(CBYStockConfig)->GetBigPlusRate();
				if (rand() % 100 < nBigPlusRate)
				{
					int64_t lReduScore = 0L;
					if((lPlayerWinScore  + lCatchScore) < 0)
					{	
						lReduScore = lDiffScore;
						//��עѪ
						pFish->SetPlusScore(lReduScore);
						//�����
						INSTANCE(CBYStockConfig)->SetBigStock(-lReduScore);
						//�ܿ��
						INSTANCE(CBYStockConfig)->SetCurrentStock(-lReduScore);

						//PlayerLog.TraceInfo("[עѪ4_7] ��%I64d ������:%.2f ���:%.2f", pFish->get_id(), (double)(lReduScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );
					}
				}
				else
				{
					//PlayerLog.TraceInfo("[עѪ4_8]��Ҹ���,����ʧ�� %d", INSTANCE(CBYStockConfig)->GetBigPlusRate());
				}
			}
		}
	}

	//PlayerLog.TraceInfo("[עѪ5] ��:%I64d ����Ѫ:%.2f ���:%.2f", pFish->get_id(), (double)(pFish->GetPlusScore() / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );
	//SHOW("[��עѪ] %.2f_%.2f", (double)(INSTANCE(CBYStockConfig)->GetSmallStock() / 100.00),  (double)(INSTANCE(CBYStockConfig)->GetBigStock() / 100.00));
}*/

//û�е�������
//����עѪ
/*void GameTableLogic::RecovePlusScore(Bird *pFish, int nFishType)
{
	//Logger PlayerLog("��Ҽ�¼", m_pGameRoom->GetRoomID());
	if(pFish == NULL) return;
	
	int64_t lPlushScore = pFish->GetPlusScore();
	if(lPlushScore <= 0) return;

	//С����
	if(nFishType <= SMALL_BIRD_FLAG)
	{
		//С����
		INSTANCE(CBYStockConfig)->SetSmallStock(lPlushScore);
	}
	else
	{
		//������
		INSTANCE(CBYStockConfig)->SetBigStock(lPlushScore);
	}

	//�����㸽��Ѫ
	pFish->SetPlusScore(0L);
	//�ܿ��
	INSTANCE(CBYStockConfig)->SetCurrentStock(lPlushScore);

	//PlayerLog.TraceInfo("[����Ѫ] ��:%I64d עѪ:%.2f ���:%.2f", pFish->get_id(), (double)(lPlushScore / GOLD_RATIO), (double)(INSTANCE(CBYStockConfig)->GetCurrentStock() / GOLD_RATIO) );
	//SHOW("[����עѪ] %.2f_%.2f", (double)(INSTANCE(CBYStockConfig)->GetSmallStock() / 100.00),  (double)(INSTANCE(CBYStockConfig)->GetBigStock() / 100.00));
}*/


////����ץ����
//void GameTableLogic::ProcessCatchFish(uint8_t type, uint32_t nBulletMul, GamePlayer * pGamePlayer, bool isAddFireProb)
//{
//	bool pause_is    = false;
//	bool isPenetrate = false;
//	int judgeCount   = 1;
//	Bird *bird       = NULL;
//	if (pGamePlayer == NULL)
//	{
//		return;
//	}
//
//	uint16_t wChairId = pGamePlayer->GetSeat();
//	//cout << "����" << wChairId << endl;
//	for (int judge = 0; judge < judgeCount; judge++)
//	{
//		//��ȡ�ӵ�����
//		Bullet *bullet = bullet_manager_.get_entity_Bullet(wChairId);
//		if (bullet == NULL)
//		{
//			SHOW("[������] ��Ч�ӵ�!");
//			return;
//		}
//
//		nBulletMul = bullet->bullet_mulriple();
//		//�ͷ��ӵ�
//		if(!bullet_manager_.delete_entity_Bullet(wChairId))
//		{
//			SHOW("[������] �ӵ�������û����ô���ӵ�!");
//			return;
//		}
//
//		//������е���
//		bird = bird_manager_.get_entity(role_[wChairId].fish_id_);
//		//������
//		uint16_t bird_type = bird->get_type();
//
//		if (bird  == NULL || bird->get_index() == INVALID_ID)
//		{
//			SHOW("[������] ��Ч�� %I64d", pGamePlayer->GetPlayerID());
//
//			DeleteInvalidBullet(wChairId, pGamePlayer);
//			//���miss��
//            pGamePlayer->SetPlayerMissStock(nBulletMul);
//			return;
//		}
//
//		//������
//		int nFishKind     = bird->get_type();
//		//�㱶��
//		int nFishMultiple = bird->get_mulriple();
//
//		//У��������
//		if( (bird->get_type() < BIRD_TYPE_0 ) || (bird->get_type() >= MAX_BIRD_TYPE && bird->get_type() < BIRD_TYPE_ONE ) || ( bird->get_type() > BIRD_TYPE_FIVE))
//		{
//			SHOW("[������] �������͵��� %I64d", pGamePlayer->GetPlayerID());
//
//			//ɾ����Ч���ӵ�
//			DeleteInvalidBullet(wChairId, pGamePlayer);
//			//���miss��
//            pGamePlayer->SetPlayerMissStock(nBulletMul);
//			//����������
//			bird->reset();
//			//�����������ɾ����
//			bird_manager_.delete_entity(role_[wChairId].fish_id_);
//			return;
//		}
//		
//		//�Ƿ���������
//		bool bSuccess = false; 
//		//BOSS��ʶ
//		bool bIsBoss  = false;
//		//��˫��
//		bool bIsDouble = false;
//
//		//����ʱ�䲢����BOSS
//		if (scene_special_time_ > 0 && nFishKind == BOSS_FISH)
//		{
//			selectFishByTime(nFishKind, nFishMultiple);
//			bIsBoss = true;
//			/*SHOW("FindID:%d FindKind:%d",  role_[wChairId].fish_id_, nFishKind);*/
//		}
//
//		//������(һ��һ��,һ��˫��,һ������,һ���ĵ�,һ�����)
//		if (nFishKind >= BIRD_TYPE_ONE && nFishKind <= BIRD_TYPE_FIVE)
//		{
//			nFishMultiple = nFishMultiple *			//�������㱶��
//				g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count *		//�����������
//				g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].mulriple;		//����
//		}
//			
//		//ʣ���ң�������
//		int64_t lRemainScore = nBulletMul;
//
//		if( INSTANCE(CBYStockConfig)->GetRevenueSwitch() >=  INSTANCE(CBYStockConfig)->GetRevenueCount())
//		{
//			//����
//			INSTANCE(CBYStockConfig)->ResetRevenueSwitch();
//			//��¼˰��
//			INSTANCE(CBYStockConfig)->SetCureentRevenue(lRemainScore);
//			SHOW("�Ե��ӵ�%I64d lRemainScore:%I64d ",pGamePlayer->GetPlayerID(),lRemainScore);
//            return;
//		}
//		else
//		{
//			//����
//			INSTANCE(CBYStockConfig)->SetRevenueSwitch();
//		}
//		//�����������
//		int nMaxBullet = bird->GetMaxBullet();
//		if (nBulletMul > nMaxBullet)
//		{
//			bird->SetMaxBullet(nBulletMul);
//		}
//
//		//�ж��Ƿ����
//		bSuccess = ComputeProbability(pGamePlayer, nBulletMul, nFishKind, nFishMultiple, wChairId, bird, bIsDouble);
//	
//		if (bSuccess)
//		{
//			bird = bird_manager_.get_entity(role_[wChairId].fish_id_);
//			if (bird == NULL|| bird->get_index() == INVALID_ID)
//			{
//				return;
//			}
//
//			//���е÷�
//			SCORE lCatchScore =  nFishMultiple * nBulletMul;
//
//			//�����ж�
//			//if(CheckBaoJi(pGamePlayer, nFishKind, lCatchScore))
//			//{
//			//	lCatchScore *= 2;
//			//	bIsDouble = true;
//			//}
//
//			role_[wChairId].set_gold(lCatchScore + role_[wChairId].get_gold());
//
//            if(BIRD_TYPE_29==nFishKind)
//			{
//			  //�ܿ��
//			  INSTANCE(CBYStockConfig)->SetCurrentStock(-lCatchScore);
//			}
//
//			//��¼�����ˮ
//			int64_t lStreamValue = (lCatchScore - nBulletMul);
//			pGamePlayer->SetPlayerStream(lStreamValue);
//			
//
//			//��ҽ�ұ��
//			pGamePlayer->ChangeCoin(lCatchScore,false);
//			//��¼�����Ӯ
//			pGamePlayer->SetPlayerWinScore(lCatchScore);
//
//
//			Catch_Bird catch_bird;
//			catch_bird.bird_id_     = bird->get_id();
//			catch_bird.catch_gold_  = lCatchScore;
//			catch_bird.isDouble     = bIsDouble;
//			catch_bird.now_money    = role_[wChairId].get_gold();
//			catch_bird.chair_id_    = wChairId;
//			if (bIsBoss && nFishKind != BOSS_FISH)
//			{
//				catch_bird.is_die = 0;
//			}
//			else
//			{
//				catch_bird.is_die = 1;
//			}
//			
//			if (catch_bird.is_die == 1)
//			{
//				//SHOW("[������] BOSS�� %I64d ������ %d_%d", pGamePlayer->GetPlayerID(), role_[wChairId].fish_id_, nFishKind);
//				//������
//				action_manager_.pause_target(bird);
//				action_manager_.remove_all_action(bird);
//
//				//����������
//				bird->reset();
//				//���������ͷ���
//				bird_manager_.delete_entity(role_[wChairId].fish_id_);
//			}
//
//			SVar sSend;
//			//��ID
//			sSend["bird_id_"]    = (int64_t)bird->get_id();
//			//�����ý����
//			sSend["catch_gold_"] = lCatchScore;
//			//�Ƿ񱩻�(˫��)
//			sSend["isDouble"]    = bIsDouble;
//			//��ǰ�����
//			sSend["now_money"]   = role_[wChairId].get_gold();
//			//����ID
//			sSend["chair_id_"]   = wChairId;
//			if (bIsBoss && nFishKind != BOSS_FISH)
//			{
//				sSend["is_die"] = 0;
//			}
//			else
//			{
//				sSend["is_die"] = 1;
//			}
//			m_pGameRoom->Bordcast_All(SUB_S_CATCH_BIRD,&sSend);
//
//			//������֮�� ���������ж��Ƿ������������͵���
//			AddFishRefresh(bird_type);
//		}
//		else
//		{
//			//��¼�����ˮ
//			pGamePlayer->SetPlayerStream(nBulletMul);
//		}
//
//		//���Ӯȡ����
//		role_[wChairId].maxScore = role_[wChairId].maxScore > role_[wChairId].get_gold() ? role_[wChairId].maxScore : role_[wChairId].get_gold();
//	}
//}

//����ץ����
void GameTableLogic::ProcessCatchFish(unsigned short chair_id ,uint8_t type, uint32_t nBulletMul, uint32_t bullet_id, bool isAddFireProb)
{
	//bool pause_is    = false;
	//bool isPenetrate = false;
	int judgeCount   = 1;
	Bird *bird       = NULL;
	if(m_pGame_Process->is_exist_user(chair_id) == false)
	{
		return;
	}

	//У���ӵ��Ƿ�Ϸ�
	if(is_valid_bullet_id(chair_id, bullet_id) == false)
	{
		//LOG(ERROR) << "player:" << m_pGame_Process->get_user_id(chair_id) << " catch fish ,bullet_id: " << bullet_id << " is invalid!";
		return;
	}

	//����Ƿ�������ӵ�
	if(bullet_manager_.is_player_have_bullet(chair_id, bullet_id) == false)
	{
		//LOG(ERROR) << "player:"<<m_pGame_Process->get_user_id(chair_id) << " have no bullet_id:" << bullet_id;
		return;
	}

	uint16_t wChairId = chair_id;//pGamePlayer->GetSeat();
	for (int judge = 0; judge < judgeCount; judge++)
	{
		//��ȡ�ӵ�����
		Bullet *bullet = bullet_manager_.get_entity_Bullet(wChairId, bullet_id);
		if (bullet == NULL)
		{
			//LOG(ERROR) << "player: "<<m_pGame_Process->get_user_id(chair_id) << " bullet_id:" << bullet_id <<  " can not find bullet object";
			return;
		}

		nBulletMul = bullet->bullet_mulriple();
		//�ͷ��ӵ�
		if(!bullet_manager_.delete_entity_Bullet(wChairId, bullet_id))
		{
			//LOG(ERROR) << "player: "<< m_pGame_Process->get_user_id(chair_id) << " delete bullet failture" << " bullet_id:" << bullet_id;
			return;
		}
		else
		{
			//LOG(ERROR) << "player: "<<m_pGame_Process->get_user_id(chair_id) << " delete bullet:" << bullet_id <<" succesfull!";
		}

		//������е���
		bird = bird_manager_.get_entity(role_[wChairId].fish_id_);
		//������
		uint16_t bird_type = bird->get_type();

		if (bird  == NULL || bird->get_index() == INVALID_ID)
		{
			//SHOW("[������] ��Ч�� %I64d", pGamePlayer->GetPlayerID());
			//LOG(ERROR) <<"[������] ��Ч��:" << m_pGame_Process->get_user_id(chair_id);
           	//���miss��
            //pGamePlayer->SetPlayerMissStock(nBulletMul);
            m_pGame_Process->set_user_miss_stock(chair_id, nBulletMul);
			DeleteInvalidBullet(wChairId);
			return;
		}

		//������
		int nFishKind     = bird->get_type();
		//�㱶��
		int nFishMultiple = bird->get_mulriple();

		//У��������
		if( (bird->get_type() < BIRD_TYPE_0 ) || (bird->get_type() >= MAX_BIRD_TYPE && bird->get_type() < BIRD_TYPE_ONE ) || ( bird->get_type() > BIRD_TYPE_FIVE))
		{
			//SHOW("[������] �������͵��� %I64d", pGamePlayer->GetPlayerID());
			//LOG(ERROR) << "[������] �������͵���: "<< pGamePlayer->GetPlayerID();

			//ɾ����Ч���ӵ�
			DeleteInvalidBullet(wChairId);
			//���miss��
            //pGamePlayer->SetPlayerMissStock(nBulletMul);
			m_pGame_Process->set_user_miss_stock(chair_id, nBulletMul);
			//����������
			bird->reset();
			//�����������ɾ����
			bird_manager_.delete_entity(role_[wChairId].fish_id_);
			return;
		}
		//�Ƿ���������
		bool bSuccess = false; 
		//BOSS��ʶ
		bool bIsBoss  = false;
		//��˫��
		bool bIsDouble = false;

		//����ʱ�䲢����BOSS
		//if (scene_special_time_ > 0 && nFishKind == BOSS_FISH)
		//{
			//selectFishByTime(nFishKind, nFishMultiple);
			//bIsBoss = true;
		//}

		//������(һ��һ��,һ��˫��,һ������,һ���ĵ�,һ�����)
		if (nFishKind >= BIRD_TYPE_ONE && nFishKind <= BIRD_TYPE_FIVE)
		{
			nFishMultiple = nFishMultiple *			//�������㱶��
				g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count *		//�����������
				g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].mulriple;		//����
		}
		
		//ʣ���ң�������
		int64_t lRemainScore = nBulletMul;

		m_pGame_Process->set_user_send_count(chair_id);

		//��˰�յ��ӵ� ��������ӵ� �����ϵͳ��Ӯ
		g_bystockconfig.AddSysWinScore(nBulletMul);
		g_bystockconfig.m_llTodayTotalWinScore += nBulletMul;

		if(g_bystockconfig.GetRevenueSwitch() >=  g_bystockconfig.GetRevenueCount())
		{
			//����
			g_bystockconfig.ResetRevenueSwitch();
			//��¼˰��
			g_bystockconfig.SetCureentRevenue(lRemainScore);
			g_bystockconfig.m_llTodayTotalTax += lRemainScore;
			m_pGame_Process->set_user_tax(chair_id, lRemainScore);
			return;
		}
		else
		{
			//����
			g_bystockconfig.SetRevenueSwitch();
		}

		//�����������
		int nMaxBullet = bird->GetMaxBullet();
		if (nBulletMul > nMaxBullet)
		{
			bird->SetMaxBullet(nBulletMul);
		};

		//LOG(ERROR) << "player:"<<chair_id << " touch_fish_id:" << role_[wChairId].fish_id_;
		//�ж��Ƿ����
		bSuccess = ComputeProbability(chair_id, nBulletMul, nFishKind, nFishMultiple, wChairId, bird, bIsDouble);

		if (bSuccess)
		{
			bird = bird_manager_.get_entity(role_[wChairId].fish_id_);
			if (bird == NULL|| bird->get_index() == INVALID_ID)
			{
				return;
			}

			//���е÷�
			SCORE lCatchScore =  nFishMultiple * nBulletMul;
			//��������
            if(bIsDouble)
			{   
                lCatchScore*=2;
				//cout << "����:"<<lCatchScore<<endl;
			}
			role_[wChairId].set_gold(lCatchScore + role_[wChairId].get_gold());

            //if(BIRD_TYPE_29==nFishKind)
			//{
			  //�������
			//  INSTANCE(CBYStockConfig)->SetCurrentStock(-lCatchScore);
			//}
			//�ܿ��
			//INSTANCE(CBYStockConfig)->SetCurrentAllStock(-lCatchScore);
			g_bystockconfig.SetCurValue(-lCatchScore);
			g_bystockconfig.m_llTodayTotalKuCun -= lCatchScore;
			//��¼�����ˮ
			int64_t lStreamValue = (lCatchScore - nBulletMul);
			//pGamePlayer->SetPlayerStream(lStreamValue);
			m_pGame_Process->set_user_stream(chair_id, lStreamValue);
			
			//��ҽ�ұ��
			//pGamePlayer->ChangeCoin(lCatchScore,false);
			m_pGame_Process->change_user_score(chair_id, lCatchScore);
			g_bystockconfig.AddSysLoseScore(lCatchScore);
			g_bystockconfig.m_llTodayTotalLoseScore += lCatchScore;
			//��¼�����Ӯ
			//pGamePlayer->SetPlayerWinScore(lCatchScore);
			m_pGame_Process->set_user_win_lose_score(chair_id, lCatchScore);
		
			//cout << "��ˮ:" << pGamePlayer->GetPlayerStream() << "|" << pGamePlayer->GetPlayerWinScore() << endl;

			Catch_Bird catch_bird;
			catch_bird.bird_id_     = bird->get_id();
			catch_bird.catch_gold_  = (double)lCatchScore / 100.0f;
			catch_bird.isDouble     = bIsDouble;
			catch_bird.now_money    = (double)role_[wChairId].get_gold() / 100.0f;
			catch_bird.chair_id_    = wChairId;
			if (bIsBoss && nFishKind != BOSS_FISH)
			{
				catch_bird.is_die = 0;
			}
			else
			{
				catch_bird.is_die = 1;
			}

			if(bird->get_id() == m_cur_Boss.get_id() && bird->get_type() == BOSS_FISH)
            {
				m_b_boss_die = true;
            }

			if (catch_bird.is_die == 1)
			{
				//������
				action_manager_.pause_target(bird);
				action_manager_.remove_all_action(bird);

				//����������
				bird->reset();
				if(bird->get_index() != INVALID_ID)
				{
					//���������ͷ���
					bird_manager_.delete_entity(role_[wChairId].fish_id_);
				}
			}
			m_pGame_Process->broadcast_all(SUB_S_CATCH_BIRD, &catch_bird, sizeof(Catch_Bird));

			//LOG(ERROR) << "player: "<<chair_id << " kill_fish:"<<catch_bird.bird_id_ << " cur_glod: "<<catch_bird.now_money;
/*			SVar sSend;
			//��ID
			sSend["bird_id_"]    = (int64_t)bird->get_id();
			//�����ý����
			sSend["catch_gold_"] = lCatchScore;
			//�Ƿ񱩻�(˫��)
			
			sSend["isDouble"]    = bIsDouble ? 1 :0;
			//��ǰ�����
			sSend["now_money"]   = role_[wChairId].get_gold();
			//����ID
			sSend["chair_id_"]   = wChairId;
			if (bIsBoss && nFishKind != BOSS_FISH)
			{
				sSend["is_die"] = 0;
			}
			else
			{
				sSend["is_die"] = 1;
			}
			m_pGameRoom->Bordcast_All(SUB_S_CATCH_BIRD,&sSend);
			*/

			//������֮�� ���������ж��Ƿ������������͵���
			AddFishRefresh(bird_type);
		}
		else
		{
			//��¼�����ˮ
			//pGamePlayer->SetPlayerStream(nBulletMul);
			m_pGame_Process->set_user_stream(chair_id, nBulletMul);
			//cout << "��ˮ:" << pGamePlayer->GetPlayerStream() << "|" << pGamePlayer->GetPlayerWinScore() << endl;
		}

		//���Ӯȡ����
		role_[wChairId].maxScore = role_[wChairId].maxScore > role_[wChairId].get_gold() ? role_[wChairId].maxScore : role_[wChairId].get_gold();
	}
}
//�ж����Ƿ��ܹ�������
bool GameTableLogic::ComputeProbability(unsigned short chair_id, int nBulletMul, int nFishKind, int nFishMul, int nChairID, Bird* pFish, bool &bIsDouble)
{
	//Logger PlayerLog("��Ҽ�¼", m_pGameRoom->GetRoomID());

	//�ܻ����
	//int64_t lHitScore  = pFish->GetHitScore();
	//עѪ���
	//int64_t lPlusScore = pFish->GetPlusScore();
	//����������     
	int nFishDieMul  =  pFish->GetFishDieRatio(); 
	//����������ӵ���
	//int nMaxBullet   = pFish->GetMaxBullet();

	if(BIRD_TYPE_TWO == nFishKind)
	{
		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
	}
	else if(BIRD_TYPE_THREE == nFishKind)
	{
		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
	}
	else if(BIRD_TYPE_FOUR == nFishKind)
	{
		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
	}
	else if(BIRD_TYPE_FIVE == nFishKind)
	{
		nFishDieMul *= g_game_config_xml.bird_same_config_[nFishKind - BIRD_TYPE_ONE].bird_count; //�����������
	}
	//int iMissCount= pGamePlayer->GetPlayerMissStock()/nBulletMul;
	int iMissCount = m_pGame_Process->get_user_miss_stock(chair_id) / nBulletMul;
	//int iMissRatio= Random(0,1000);
	std::uniform_int_distribution<> dis(0, 1000);
	int iMissRatio = dis(m_gen);
	bool bTrigger=false;
	if(iMissRatio<(iMissCount*5)&&nFishDieMul>=30)
	{
        bTrigger=true;
	    //SHOW("����miss��%I64d ��ǰmiss���%I64d,iMissCount;%d",pGamePlayer->GetPlayerID(),pGamePlayer->GetPlayerMissStock(),iMissCount);
		if(iMissCount>100)
		{
          iMissCount=100;
		}
	}

	bool bRunMissStock = false;

	do 
	{
		//����������
		//INSTANCE(CBYStockConfig)->SetCurrentAllStock(nBulletMul);
		g_bystockconfig.SetCurValue(nBulletMul);
		g_bystockconfig.m_llTodayTotalKuCun += nBulletMul;
		////����
		//if(BIRD_TYPE_29==nFishKind)
		//{     
		//	  int64_t lCurrentStock=INSTANCE(CBYStockConfig)->GetCurrentStock();
		//	  int64_t lMulripleMax= g_game_config_xml.cannon_mulriple_[0]*10;
		//	  int64_t lCurrentRatio =(lCurrentStock/ lMulripleMax)- nFishDieMul;
		//	  //�ܿ��
		//      INSTANCE(CBYStockConfig)->SetCurrentStock(nBulletMul);
		//		//1/2*nFishDieMul +(CurrentRatio*5)/(nFishDieMul*1000);
		//		//500+(CurrentRatio*5)/1000*nFishDieMul
		//	  int64_t lRatio= (rand() %(200*nFishDieMul));
		//	  if(lRatio<(100+lCurrentRatio)&&lCurrentRatio>=0)
		//	  {   
		//		   SHOW("���񵽴��㣺CurrentRatio:%I64d lRatio:%I64d lCurrentStock:%I64d nFishDieMul:%d lMulripleMax:%I64d",(lCurrentRatio+100),lRatio,lCurrentStock,nFishDieMul,lMulripleMax);
		//		   return true;
		//	  }
		//	 // SHOW("δ���񵽴��㣺CurrentRatio:%I64d lRatio:%I64d lCurrentStock:%I64d nFishDieMul:%d lMulripleMax:%I64d",(lCurrentRatio+100),lRatio,lCurrentStock,nFishDieMul,lMulripleMax);
		//}
		//else
		//{   
			//�Ƿ񴥷�����
	        //int iBaojiRatio=Random(0,20);
	        std::uniform_int_distribution<> dis(0, 20);
	        int iBaojiRatio = dis(m_gen);
			int64_t lDouble=1;
	        if(iBaojiRatio==8&&nFishDieMul>=30&&BIRD_TYPE_29!=nFishKind)
			{
				lDouble=2;
			}   
			//int64_t lUserID= pGamePlayer->GetPlayerID();
			int64_t lUserID = m_pGame_Process->get_user_id(chair_id);
			int64_t lRatio= Randfun::GetRandom((int64_t)(nFishDieMul*lDouble),0);
			if(lRatio==pFish->m_lRandNumber)
			{   
				if (BIRD_TYPE_29==nFishKind)
				{
				  //SHOW("����boss��nFishDieMul:%d lRatio:%I64d RandNumber:%I64d,FisIDh:%d",nFishDieMul,  lRatio,pFish->m_lRandNumber,pFish->get_id());
				}
				if(lDouble==2)
                   bIsDouble=true;

				if(lUserID==1036414||lUserID==1022189)
				{
					//SHOW("%I64d���������nFishDieMul:%d lRatio:%I64d RandNumber:%I64d,FisID:%I64d nFishKind:%d nBulletMul:%d",lUserID,nFishDieMul,lRatio,pFish->m_lRandNumber,pFish->get_id(),nFishKind,nBulletMul);
				}
				return true;
			}
			else
			{
				if(lUserID==1036414||lUserID==1022189)
				{
					//SHOW("%I64dδ���������nFishDieMul:%d lRatio:%I64d RandNumber:%I64d,FisID:%I64d, nFishKind:%d nBulletMul:%d",lUserID,nFishDieMul,lRatio,pFish->m_lRandNumber,pFish->get_id(),nFishKind,nBulletMul);
				}
			}
			//SHOW("δ����С�㣺nFishDieMul:%d lRatio:%I64d  RandNumber:%I64d FisIDh:%d",nFishDieMul,  lRatio,pFish->m_lRandNumber,pFish->get_id());
		//}
		//û�д����˻���û�д���
		if(iMissCount==0||bTrigger==false) 
			break;

		bRunMissStock = true;
		iMissCount--;
		//pGamePlayer->SetPlayerMissStock(-nBulletMul);
		m_pGame_Process->set_user_miss_stock(chair_id, -nBulletMul);

        //miss���е��ӵ�������(���ܴ�����  ����û������  ���ܱ���˰) �����ϵͳ��Ӯ
        g_bystockconfig.AddSysWinScore(nBulletMul);
        g_bystockconfig.m_llTodayTotalWinScore += nBulletMul;

        if( g_bystockconfig.GetRevenueSwitch() >=  g_bystockconfig.GetRevenueCount())
		{
			//����
			g_bystockconfig.ResetRevenueSwitch();
			//��¼˰��
			g_bystockconfig.SetCureentRevenue(nBulletMul);
			g_bystockconfig.m_llTodayTotalTax += nBulletMul;
			m_pGame_Process->set_user_tax(chair_id, nBulletMul);
			 // SHOW("�Ե�miss��%I64d ��ǰmiss���%I64d,iMissCount;%d",pGamePlayer->GetPlayerID(),pGamePlayer->GetPlayerMissStock(),iMissCount);
			if(iMissCount==0)
            {
                break;
            }

			iMissCount--;
			//pGamePlayer->SetPlayerMissStock(-nBulletMul);
			m_pGame_Process->set_user_miss_stock(chair_id, -nBulletMul);

            //miss���е��ӵ�������(���ܴ�����  ����û������  ���ܱ���˰) �����ϵͳ��Ӯ
            g_bystockconfig.AddSysWinScore(nBulletMul);
            g_bystockconfig.m_llTodayTotalWinScore += nBulletMul;
		}
		else
		{
			//����
			g_bystockconfig.SetRevenueSwitch();
		}
      //  SHOW("ʹ��miss��%I64d ��ǰmiss���%I64d,iMissCount;%d",pGamePlayer->GetPlayerID(),pGamePlayer->GetPlayerMissStock(),iMissCount);
	}while(true);
    
    return false;
}
///////////////////////////////////////////////////////////////////////////

