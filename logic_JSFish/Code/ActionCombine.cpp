#include "stdafx.h"
#include "Exception.h"
#include "ActionInterval.h"
#include "ActionCombine.h"
#include <cstdarg>
/////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
Action_Sequence::Sequence::Sequence(Action *act1, Action *act2)
:Action_Interval(0)
{
    actions_[0] = (Action_Interval *)act1;
    actions_[1] = (Action_Interval *)act2;

    duration_ = actions_[0]->duration() + actions_[1]->duration();
}

Action_Sequence::Sequence::~Sequence()
{
    delete actions_[0];
    delete actions_[1];
}

void Action_Sequence::Sequence::update(float time)
{
    int found = 0;
    float new_t = 0.0f;	

    if( time >= split_ )
    {		
        found = 1;
        if ( split_ == 1 )
            new_t = 1;
        else
            new_t = (time-split_) / (1 - split_ );
    } else 
    {
        found = 0;
        if( split_ != 0 )
            new_t = time / split_;
        else
            new_t = 1;
    }

    if (last_ == -1 && found==1)
    {
        actions_[0]->start_with_target(target_);
        actions_[0]->update(1.0f);
        actions_[0]->stop();
    }

    if (last_ != found )
    {
        if( last_ != -1 )
        {
            actions_[last_]->update(1.0f);
            actions_[last_]->stop();
        }

        actions_[found]->start_with_target(target_);
    }

    actions_[found]->update(new_t);

    last_ = found;
}

void Action_Sequence::Sequence::stop()
{
    actions_[0]->stop();
    actions_[1]->stop();

    Action_Interval::stop();
    //__super::stop();
}

void Action_Sequence::Sequence::start_with_target(Entity *target)
{
   // __super:: start_with_target(target);
    Action_Interval::start_with_target(target);

    split_ = actions_[0]->duration() / duration_;
    last_ = -1;
}

Move_Point Action_Sequence::Sequence::move_to(float elapsed)
{
	//actions_[0]->move_to(elapsed + elapsed_);
	if (last_ != -1)
		return actions_[last_]->move_to(elapsed + elapsed_);
	else
		return Move_Point(Point(-1,-1), 0);
}

Action_Sequence::Action_Sequence(Action *act1, ...)
:Action_Interval(0)
{
    va_list params;
    va_start(params,act1);	

    Action_Interval *now;
    Action_Interval *prev = (Action_Interval *)act1;

    while( act1 ) 
    {
        now = va_arg(params,Action_Interval*);
        if ( now )
        {
            prev = new Sequence(prev, now);
        }
        else
            break;
    }

    va_end(params);

    duration_ = prev->duration();

    sequence_ = (Sequence *)prev;
}

Action_Sequence::~Action_Sequence()
{
    delete sequence_;
}

void Action_Sequence::stop()
{
    sequence_->stop();

    Action_Interval::stop();
    //__super::stop();
}

void Action_Sequence::update(float time)
{
    sequence_->update(time);
}

void Action_Sequence::start_with_target(Entity *target)
{
    //__super::start_with_target(target);
    Action_Interval::start_with_target(target);

    sequence_->start_with_target(target);
}

Move_Point Action_Sequence::move_to(float elapsed)
{
	sequence_->set_elapse(elapsed_);
	return sequence_->move_to(elapsed);
}

///////////////////////////////////////////////////////////////////////////////////////////
Action_Spawn::Spawn::Spawn(Action *act1, Action *act2)
:Action_Interval(0)
{
    actions_[0] = (Action_Interval *)act1;
    actions_[1] = (Action_Interval *)act2;

	float d1, d2;

	d1 = actions_[0]->duration();
	d2 = actions_[1]->duration();

	if (d1 > d2)
	{
        Action_Delay ad(d1 - d2);
		/*actions_[1] = new Action_Sequence(actions_[1], Action_Delay(d1 - d2), 0);*/
        actions_[1] = new Action_Sequence(actions_[1], &ad, 0);
	}
	else if (d1 < d2)
	{
        Action_Delay ad(d2 - d1);
		/*actions_[0] = new Action_Sequence(actions_[0], Action_Delay(d2 - d1), 0);*/
        actions_[0] = new Action_Sequence(actions_[0], &ad, 0);
	}

	duration_ = std::max((actions_[0]->duration()), (actions_[1]->duration()));
}

Action_Spawn::Spawn::~Spawn()
{
    delete actions_[0];
    delete actions_[1];
}

void Action_Spawn::Spawn::update(float time)
{
	actions_[0]->update(time);
	actions_[1]->update(time);
}

void Action_Spawn::Spawn::stop()
{
    actions_[0]->stop();
    actions_[1]->stop();

    Action_Interval::stop();
   // __super::stop();
}

void Action_Spawn::Spawn::start_with_target(Entity *target)
{
    Action_Interval::start_with_target(target);
    //__super:: start_with_target(target);

    actions_[0]->start_with_target(target);
    actions_[1]->start_with_target(target);
}

Action_Spawn::Action_Spawn(Action *act1, ...)
:Action_Interval(0)
{
    va_list params;
    va_start(params,act1);	

    Action_Interval *now;
    Action_Interval *prev = (Action_Interval *)act1;

    while( act1 ) 
    {
        now = va_arg(params,Action_Interval*);
        if ( now )
        {
            prev = new Spawn(prev, now);
        }
        else
            break;
    }

    va_end(params);

    duration_ = prev->duration();

    spawn_ = (Spawn *)prev;
}

Action_Spawn::~Action_Spawn()
{
    delete spawn_;
}

void Action_Spawn::stop()
{
    spawn_->stop();

    Action_Interval::stop();
    //__super::stop();
}

void Action_Spawn::update(float time)
{
    spawn_->update(time);
}

void Action_Spawn::start_with_target(Entity *target)
{
    //__super::start_with_target(target);
    Action_Interval::start_with_target(target);

    spawn_->start_with_target(target);
}

/////////////////////////////////////////////////////////////////////////////////////////
Action_Repeat_Forever::Action_Repeat_Forever(Action *a)
:stop_(false),
other_(a)
{
}

Action_Repeat_Forever::~Action_Repeat_Forever()
{
    if (other_)
    {
        delete other_;
    }
}

void Action_Repeat_Forever::step(float dt)
{
    other_->step(dt);
    if( other_->is_done()) 
    {
        other_->stop();
        other_->start_with_target(target_);
    }
}

void Action_Repeat_Forever::start_with_target(Entity *target)
{
    other_->start_with_target(target);
    //__super::start_with_target(target);

    Action::start_with_target(target);
}

/////////////////////////////////////////////////////////////////////////////////////////
