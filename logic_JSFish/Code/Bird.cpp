#include "stdafx.h"
#include "Exception.h"
#include "Bird.h"

///////////////////////////////////////////////////////////////////////////
Bird::Bird()
{
	type_ = BIRD_TYPE_NULL;
	item_ = ITEM_TYPE_NULL;
	path_id_ = 0;
	path_type_ = 0;
	path_delay_ = 0.0;
	speed_ = 0.0;

	yuzhen_type_ = YUZHEN_NORMAL;

	radius_ = 0.0;
	rotate_duration_ = 0.0;
	start_angle_ = 0.0;
	rotate_angle_ = 0.0;
	move_duration_ = 0.0;
	pause_time_ = 0.0;

	//鱼
	//鱼基础倍率
	m_nFishBaseMul  = 0;
	//鱼死亡倍率
	m_nFishDieMul   = 0;
	 //鱼死亡倍率
	m_nMaxBullet    = 0;
	//附加鱼币
	m_lPlusScore    = 0L;   
	 //总消耗
	m_lHitScore     = 0L;   
	//座椅消耗
	//ZeroMemory(m_lChairScore, sizeof(m_lChairScore));
	memset(m_lChairScore, 0, sizeof(m_lChairScore));

	//创建日期
	m_tCreatTime = 0;
}

Bird::~Bird()
{
}

void Bird::reset()
{
	type_ = BIRD_TYPE_NULL;
	item_ = ITEM_TYPE_NULL;

	path_id_ = 0;
	path_type_ = 0;
	path_delay_ = 0.0;
	speed_ = 0.0;
	mulriple_ = 0;

	//鱼阵需要的数据
	yuzhen_type_ = YUZHEN_NORMAL;
	pause_time_ = 0.0;

	radius_ = 0.0;
	rotate_duration_ = 0.0;
	start_angle_ = 0.0;
	rotate_angle_ = 0.0;
	move_duration_ = 0.0;

	start_ = Point(0.0,0.0);
	end_   = Point(0.0,0.0);
	pause_ = Point(0.0,0.0);

	//鱼
	//鱼基础倍率
	m_nFishBaseMul  = 0;
	//鱼死亡倍率
	m_nFishDieMul   = 0;
	//鱼死亡倍率
	m_nMaxBullet    = 0;
	//附加鱼币
	m_lPlusScore    = 0L;   
	//总消耗
	m_lHitScore     = 0L;   
	//座椅消耗
	//ZeroMemory(m_lChairScore, sizeof(m_lChairScore));
	memset(m_lChairScore, 0, sizeof(m_lChairScore));

	//创建日期
	m_tCreatTime = 0;
}

///////////////////////////////////////////////////////////////////////////
