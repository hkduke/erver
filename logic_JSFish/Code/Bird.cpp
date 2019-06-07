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

	//��
	//���������
	m_nFishBaseMul  = 0;
	//����������
	m_nFishDieMul   = 0;
	 //����������
	m_nMaxBullet    = 0;
	//�������
	m_lPlusScore    = 0L;   
	 //������
	m_lHitScore     = 0L;   
	//��������
	//ZeroMemory(m_lChairScore, sizeof(m_lChairScore));
	memset(m_lChairScore, 0, sizeof(m_lChairScore));

	//��������
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

	//������Ҫ������
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

	//��
	//���������
	m_nFishBaseMul  = 0;
	//����������
	m_nFishDieMul   = 0;
	//����������
	m_nMaxBullet    = 0;
	//�������
	m_lPlusScore    = 0L;   
	//������
	m_lHitScore     = 0L;   
	//��������
	//ZeroMemory(m_lChairScore, sizeof(m_lChairScore));
	memset(m_lChairScore, 0, sizeof(m_lChairScore));

	//��������
	m_tCreatTime = 0;
}

///////////////////////////////////////////////////////////////////////////
