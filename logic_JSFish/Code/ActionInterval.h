#ifndef __ACTION_INTERVAL_H__
#define __ACTION_INTERVAL_H__

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Point.h"
#include "Action.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Interval : public Action
{
public:
    Action_Interval(float d);
    virtual ~Action_Interval();

public:
    virtual float duration() const { return duration_; }
    virtual void set_duration(float d) { duration_ = d; }

    virtual float elapse() const { return elapsed_; }
    virtual void set_elapse(float e) { elapsed_ = e; }

    virtual bool is_done() { return ((elapsed_ >= duration_) || stop_); }
    virtual void stop() { stop_ = true; target_ = 0; }

    virtual void step(float dt);
    virtual void update(float time);

    virtual float speed() const { return speed_; }
    virtual void set_speed(float speed) { speed_ = speed; }

    virtual void start_with_target(Entity *target);

protected:
    float duration_;
    float elapsed_;
    bool stop_;
    float speed_;
	bool firstTick_;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Move_To : public Action_Interval
{
public:
	Action_Move_To(float d, Point end);
    virtual ~Action_Move_To();

public:
    virtual void update(float time);
    virtual void start_with_target(Entity *target);

protected:
	Point start_;
    Point end_;
    Point delta_;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Move_By : public Action_Move_To
{
public:
    Action_Move_By(float d, Point delta);
    virtual ~Action_Move_By();

public:
    virtual void start_with_target(Entity *target);
};


////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Rorate_To : public Action_Interval
{
public:
    Action_Rorate_To(float d, float end);
    virtual ~Action_Rorate_To();

public:
    virtual void update(float time);
    virtual void start_with_target(Entity *target);

private:
    float start_angle_;
    float end_angle_;
    float diff_angle_;
};

////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Rorate_By : public Action_Interval
{
public:
    Action_Rorate_By(float d, float angle);
    virtual ~Action_Rorate_By();

public:
    virtual void update(float time);
    virtual void start_with_target(Entity *target);

private:
    float diff_angle_;
    float start_angle_;
};


////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Delay : public Action_Interval
{
public:
    Action_Delay(float d);
    virtual ~Action_Delay();

public:
    virtual void start_with_target(Entity *target);
};


/////////////////////////////////////////////////////////////////////////////////////////////////


#endif