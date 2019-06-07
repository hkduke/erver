#ifndef __ACTION_COMBINE_H__
#define __ACTION_COMBINE_H__

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Action.h"
#include "ActionInterval.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Sequence : public Action_Interval
{
private:
    class Sequence : public Action_Interval
    {
    public:
        Sequence(Action *act1, Action *act2);
        ~Sequence();

    public:
        virtual void update(float time);
        virtual void stop();

        virtual void start_with_target(Entity *target);

		virtual Move_Point move_to(float elapsed);

    private:
        float split_;
        int last_;
        Action_Interval *actions_[2];
    };

public:
    Action_Sequence(Action *act1, ...);
    virtual ~Action_Sequence();

public:
    virtual void update(float time);
    virtual void stop();

    virtual void start_with_target(Entity *target);

	virtual Move_Point move_to(float elapsed);

private:
    Sequence *sequence_;

};

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Spawn : public Action_Interval
{
private:
    class Spawn : public Action_Interval
    {
    public:
        Spawn(Action *act1, Action *act2);
        ~Spawn();

    public:
        virtual void update(float time);
        virtual void stop();

        virtual void start_with_target(Entity *target);

    private:
//        float split_;
//        int last_;
        Action_Interval *actions_[2];
    };

public:
    Action_Spawn(Action *act1, ...);
    virtual ~Action_Spawn();

public:
    virtual void update(float time);
    virtual void stop();

    virtual void start_with_target(Entity *target);

private:
    Spawn *spawn_;
};

///////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Repeat_Forever : public Action
{
public:
    Action_Repeat_Forever(Action *a);
    virtual ~Action_Repeat_Forever();

public:
    virtual bool is_done() { return stop_; }
    virtual void stop() { other_->stop(); stop_ = true; target_ = 0; }

    virtual void step(float dt);
    virtual void start_with_target(Entity *target);

private:
    bool stop_;
    Action *other_;
};

/////////////////////////////////////////////////////////////////////////////////////////////////


#endif