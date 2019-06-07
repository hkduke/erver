#include "stdafx.h"
#include "Exception.h"
#include "Role.h"
int platformCode = 0;
///////////////////////////////////////////////////////////////////////////
Role::Role()
{
    chair_id_ = INVALID_ID;
    gold_ = 0;
    cannon_type_ = CANNON_TYPE_NULL;
	cannon_mulriple_ = 0;
	lock_bird_id_ = -1;
	energy_value_ = 0;
	award_gold_ = 0;

}

Role::~Role()
{
}

void Role::initialise(uint16_t id)
{
	set_id (id);
	gold_ = 0;
	exchange_gold_ = 0;
	cannon_type_ = CANNON_TYPE_0;
	cannon_mulriple_ = 0;
	lock_bird_id_ = -1;
	energy_value_ = 0;
	award_gold_ = 0;
	Defray = 0;
	DefrayTimes = 0;
	RoleType = 0;
	maxScore = 0;
	minScore = 0;
	FoceSwitch = false;
	fireCount = 0;
	timesDelay = 0;
	lastOpenForceSwitch = 0;
	isFirstTouchForceSwitch = true;
	lastTouchForceControl = 0;
	userEnterScore = 0;
	isRecoverStatus = false;
	RecoverperScore = 0;
	Recoveraddodds = 0;
	RecoverDelay = 0;
	LastFireTime = Utility::GetTick(); //GetTickCount();
	EnterScoreAddProb = 1.0;
}

void Role::cleanup()
{
	set_id (0xFFFF);
    gold_ = 0;
	exchange_gold_ = 0;
    cannon_type_ = CANNON_TYPE_NULL;
	cannon_mulriple_ = 0;
	lock_bird_id_ = -1;
	energy_value_ = 0;
	award_gold_ = 0;
	FoceSwitch = false;
	Defray = 0;
	DefrayTimes = 0;
	fireCount = 0;
	timesDelay = 0;
	lastOpenForceSwitch = 0;
	isFirstTouchForceSwitch = true;
	lastTouchForceControl = 0;
	userEnterScore = 0;
	isRecoverStatus = false;
	RecoverperScore = 0;
	Recoveraddodds = 0;
	RecoverDelay = 0;
	EnterScoreAddProb = 1.0;
}

void Role::set_id(uint16_t id) 
{
	chair_id_ = id; 

	if (chair_id_ < GAME_PLAYER)
	{
		if (platformCode == 1)
		{
			position_.x_ = CANNON_POSITION_1[chair_id_][0];
			position_.y_ = CANNON_POSITION_1[chair_id_][1];
		}
		else
		{
			position_.x_ = CANNON_POSITION[chair_id_][0];
			position_.y_ = CANNON_POSITION[chair_id_][1];
		}
		
	}
}



///////////////////////////////////////////////////////////////////////////
