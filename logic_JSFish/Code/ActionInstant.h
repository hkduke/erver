#ifndef __ACTION_INSTANT_H__
#define __ACTION_INSTANT_H__

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "rak/functional_fun.h"
#include "FastDelegate.h"
#include "Prereqs.h"
#include "Point.h"
#include "Action.h"
#include "ActionInterval.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Instant : public Action_Interval
{
public:
    Action_Instant():Action_Interval(0) {}
    virtual ~Action_Instant() {}

public:
    virtual void step(float dt) { update(1.0f); }
};


/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Func : public Action_Instant
{
public:
    typedef rak::function_base2<bool, Entity*, uint32_t> *Slot_Func;

public:
    Action_Func(Slot_Func slot, uint32_t data):slot_func_(slot) { data_ = data; }
    virtual void start_with_target(Entity *target);

protected:
    Slot_Func slot_func_;
};

class  Action_Func1 : public Action_Instant
{
public:
	typedef fastdelegate::FastDelegate2<Entity*, uint32_t, bool> Slot_Func1;

public:
	Action_Func1(Slot_Func1 slot, uint32_t data):slot_func_(slot) { data_ = data; }
	virtual void start_with_target(Entity *target);

protected:
	Slot_Func1 slot_func_;
};


typedef rak::function_base2<bool, Entity*, void*> *Slot_FuncND;
class  Action_FuncND : public Action_Instant
{
public:
	Action_FuncND(Slot_FuncND slot, void* data) : slot_func_nd_(slot) { pdata_ = data; }
	virtual void start_with_target(Entity *target);

private:
	void* pdata_;
	Slot_FuncND slot_func_nd_;
};

class  Action_FuncNDD : public Action_Instant
{
public:
	typedef fastdelegate::FastDelegate2<Entity*, void*, bool> Slot_FuncNDD;
	Action_FuncNDD(Slot_FuncNDD slot, void* data) :slot_func_nd_(slot) { pdata_ = data; }
	virtual void start_with_target(Entity *target);

private:
	void* pdata_;
	Slot_FuncNDD slot_func_nd_;
};

/////////////////////////////////////////////////////////////////////////////////////////////////

#endif