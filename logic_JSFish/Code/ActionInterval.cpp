#include "stdafx.h"
#include "Exception.h"
#include "Entity.h"
#include "ActionInterval.h"
#include <cmath>
/////////////////////////////////////////////////////////////////////////////////////////


/////////////////////////////////////////////////////////////////////////////////////////
Action_Interval::Action_Interval(float d)
:duration_(d),
elapsed_(0),
stop_(false),
speed_(1.0),
firstTick_(true)
{
}

Action_Interval::~Action_Interval()
{
}

void Action_Interval::step(float dt)
{
    elapsed_ += dt*speed_;

    update(std::min(1.0f, ((float)elapsed_/duration_)));
}

void Action_Interval::update(float time)
{
}

void Action_Interval::start_with_target(Entity *target)
{
    elapsed_ = 0;
    stop_ = false;

    Action::start_with_target(target);
    //__super::start_with_target(target);
}



//////////////////////////////////////////////////////////////////////////////////////////////
Action_Move_To::Action_Move_To(float d, Point end)
:Action_Interval(d),
end_(end)
{
}

Action_Move_To::~Action_Move_To()
{
}

void Action_Move_To::start_with_target(Entity *target)
{
    start_ = target->position();
    delta_ = end_ - start_;

    Action_Interval::start_with_target(target);
    //__super::start_with_target(target);
}

void Action_Move_To::update(float time)
{
    Point pt(start_.x_+delta_.x_*time, start_.y_+delta_.y_*time);

    target_->set_position(pt);
}

//////////////////////////////////////////////////////////////////////////////////////////////
Action_Move_By::Action_Move_By(float d, Point delta)
:Action_Move_To(d, Point(0,0))
{
    delta_ = delta;
}

Action_Move_By::~Action_Move_By()
{
}

void Action_Move_By::start_with_target(Entity *target)
{
    Point temp = delta_;

    Action_Move_To::start_with_target(target);
    //__super::start_with_target(target);
    delta_ = temp;
}

////////////////////////////////////////////////////////////////////////////////////////////
Action_Rorate_To::Action_Rorate_To(float d, float end)
:Action_Interval(d),
end_angle_(end)
{
}

Action_Rorate_To::~Action_Rorate_To()
{
}

void Action_Rorate_To::update(float time)
{
    target_->set_rotation(start_angle_+diff_angle_*time); 
}

void Action_Rorate_To::start_with_target(Entity *target)
{
    //__super::start_with_target(target);
    Action_Interval::start_with_target(target);

    start_angle_ = target->rotation();

    if (start_angle_ > 0)
        start_angle_ = fmodf(start_angle_, M_PI);
    else
        start_angle_ = fmodf(start_angle_, -M_PI);

    diff_angle_ = end_angle_ - start_angle_;
    if (diff_angle_ > M_PI_2)
        diff_angle_ -= M_PI;
    if (diff_angle_ < -M_PI_2)
        diff_angle_ += M_PI;
}

////////////////////////////////////////////////////////////////////////////////////////////
Action_Rorate_By::Action_Rorate_By(float d, float angle)
:Action_Interval(d), diff_angle_(angle), start_angle_(0)
{
}

Action_Rorate_By::~Action_Rorate_By()
{
}

void Action_Rorate_By::update(float time)
{
    target_->set_rotation(start_angle_+diff_angle_*time); 
}

void Action_Rorate_By::start_with_target(Entity *target)
{
    //__super::start_with_target(target);
    Action_Interval::start_with_target(target);

    start_angle_ = target->rotation();

    if (start_angle_ > 0)
        start_angle_ = fmodf(start_angle_, M_PI);
    else
        start_angle_ = fmodf(start_angle_, -M_PI);
}


////////////////////////////////////////////////////////////////////////////////////////////
Action_Delay::Action_Delay(float d)
:Action_Interval(d)
{
}

Action_Delay::~Action_Delay()
{
}

void Action_Delay::start_with_target(Entity *target)
{
   // __super::start_with_target(target);
    Action_Interval::start_with_target(target);
}

////////////////////////////////////////////////////////////////////////////////////////////


