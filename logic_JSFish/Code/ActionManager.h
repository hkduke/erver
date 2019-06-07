#ifndef __ACTION_MANAGER_H__
#define __ACTION_MANAGER_H__

//////////////////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Entity.h"
#include "Action.h"

////////////////////////////////////////////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Manager 
{
public:
    typedef std::map<uint32_t, Action*> Elements;
//     typedef std::vector<Action*> Erases;
//     typedef std::vector<Action*> Inserts;
	typedef std::vector<Action*> ActionsVector;

public:
    Action_Manager();
    virtual ~Action_Manager();

public:
	void initialise();
	void cleanup();

	void prepare();
    void update(float dt);

    void pause_target(Entity *target);
    void resume_targer(uint32_t tag);

    int number_of_actions(Entity *target);
    Action *get_action(uint32_t tag);
    bool add_action(Action *action, Entity *target, bool pause);
    bool remove_action(uint32_t tag) ;
    void remove_all_action(Entity *target) ;

protected:
	uint32_t tag_factory();

private:
	uint32_t tag_;

	ActionsVector erases_;
	ActionsVector inserts_;
    Elements elements_;
};

////////////////////////////////////////////////////////////////////////////////////////////////////


#endif