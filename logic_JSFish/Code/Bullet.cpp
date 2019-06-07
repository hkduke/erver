#include "stdafx.h"
#include "Exception.h"
#include "Bullet.h"

///////////////////////////////////////////////////////////////////////////
Bullet::Bullet()
{
    chair_id_ = INVALID_ID;
    cannon_type_ = CANNON_TYPE_NULL;
	range_index_ = 0;
	lock_bird_id_ = -1;
	bullet_speed_ = 0;
	bullet_mulriple_ = 0;

	//move_action_id_ = 0;
}

Bullet::~Bullet()
{
}

///////////////////////////////////////////////////////////////////////////
