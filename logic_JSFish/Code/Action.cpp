#include "Exception.h"
#include "Action.h"

/////////////////////////////////////////////////////////////////////////////////////////

Move_Point::Move_Point():angle_(0) 
{ 
}

Move_Point::Move_Point(const Point &position, float angle)
:angle_(angle), position_(position)
{
}

Move_Point::~Move_Point()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
Action::Action()
:tag_(0), target_(0)
{
	pause_ =false;
	data_ = 0;
	speed_ = 0.0;
	duration_ = 0.0;
	elapsed_ = 0.0;
}

Action::~Action()
{
}

uint32_t Action::get_tag() const 
{
    return tag_;
}

void Action::set_tag(uint32_t tag)
{
    tag_ = tag;
}

uint32_t Action::get_data() const
{
	return data_;
}

void Action::set_data(uint32_t data)
{
	data_ = data;
}

bool Action::is_pause() const 
{
	return pause_;
}

void Action::set_pause(bool pause)
{
	pause_ = pause;
}

void Action::pause()
{
	pause_ = true;
}

void Action::resume()
{
	pause_ = false;
}

bool Action::is_done() 
{
    //return true;
	return (elapsed_ >= duration_ || stop_);
}

void Action::stop() 
{
	stop_ = true;
	target_ = NULL;
}

void Action::step(float dt) 
{
	elapsed_ += dt * speed_;
	update(std::min(1.0f, elapsed_ / duration_));
}

void Action::update(float time)
{
}

Entity *Action::target() const 
{
    return target_;
}

void Action::start_with_target(Entity *target)
{
    target_  = target; 
}

////////////////////////////////////////////////////////////////////////////////////////////

