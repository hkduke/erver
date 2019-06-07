#include "stdafx.h"
#include "Exception.h"
#include "ActionManager.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
Action_Manager::Action_Manager(): tag_(0) {}

Action_Manager::~Action_Manager()
{
	cleanup();
}

void Action_Manager::initialise()
{
	tag_ = 0;
}

void Action_Manager::cleanup() 
{
	tag_ = 0;
	prepare();
	for (Elements::iterator i = elements_.begin(); i != elements_.end(); ++i) 
	{
		delete i->second;
	}
	elements_.clear();
}

void Action_Manager::update(float dt) {
	prepare();

	Action *action;
	for (Elements::iterator it = elements_.begin(); it != elements_.end(); ++it) {
		action = it->second;
		if (action->is_done()) {
			ActionsVector::iterator j = std::find(erases_.begin(), erases_.end(), it->second);
			if (j == erases_.end())
				erases_.push_back(action);
		}
		else {
			if (!action->is_pause())
				action->step(dt);
		}
	}

	prepare();
}

void Action_Manager::prepare() 
{
	ActionsVector::iterator it;
	uint32_t action_id;
	for (it = erases_.begin(); it != erases_.end(); ++it) 
	{
		action_id =  (*it)->get_tag();
		delete (*it);
		elements_.erase(action_id);
	}
	erases_.clear();

	for (it = inserts_.begin(); it != inserts_.end(); ++it) 
	{
		elements_.insert(std::make_pair((*it)->get_tag(), *it));
	}
	inserts_.clear();
}

//void Action_Manager::pause_target(uint32_t tag) {
void Action_Manager::pause_target(Entity *target) {

	/*Elements::iterator it = elements_.find(tag);
	if (it == elements_.end())
		return;

	it->second->pause();*/
	Elements::iterator i;
	Elements::iterator iend = elements_.end();

	for (i = elements_.begin(); i != iend; ++i)
	{
		if (i->second->target() == target)
			i->second->pause();
	}
}

void Action_Manager::resume_targer(uint32_t tag)
{
	Elements::iterator it = elements_.find(tag);
	if (it == elements_.end())
		return;

	it->second->set_pause(false);
}

int Action_Manager::number_of_actions(Entity *target)
{
	int count = 0;

    Elements::iterator i;
    Elements::iterator iend = elements_.end();

    for (i = elements_.begin(); i != iend; ++i)
    {
		if (i->second->target() == target)
            count ++;
    }

    return count;
}

Action* Action_Manager::get_action(uint32_t action_id) {
	Elements::iterator it = elements_.find(action_id);
	if (it == elements_.end())
		return NULL;

	for (ActionsVector::iterator j = erases_.begin(); j != erases_.end(); ++j) {
		if ((*j)->get_tag() == action_id)
			return NULL;
	}

	return it->second;
}

bool Action_Manager::add_action(Action* action, Entity* target, bool pause) {
	uint32_t tag = tag_factory();
	Elements::iterator it = elements_.find(tag);
	if (it != elements_.end())
		return false;
	action->set_tag(tag);
	action->set_pause(pause);
	target->set_action_id(tag);
	action->start_with_target(target);

	inserts_.push_back(action);

	return true;
}

bool Action_Manager::remove_action(uint32_t tag) 
{
	Elements::iterator it = elements_.find(tag);
	if (it == elements_.end()) 
	{
		return false;
	}

	ActionsVector::iterator j = std::find(erases_.begin(), erases_.end(), it->second);
	if (j == erases_.end())
		erases_.push_back(it->second);
	return true;
}

void Action_Manager::remove_all_action(Entity *target)
{
    Elements::iterator i;
    Elements::iterator iend = elements_.end();

	for (i = elements_.begin(); i != iend; ++i)
    {
		if (i->second->target() == target)
		{
			ActionsVector::iterator j = std::find(erases_.begin(), erases_.end(), i->second);
			if (j == erases_.end())
                erases_.push_back(i->second);
		}
    }
}

uint32_t Action_Manager::tag_factory() {
	if ((++tag_) == 0)
		tag_ = 1;
	return tag_;
}

/////////////////////////////////////////////////////////////////////////////////////////////////
