#include "stdafx.h"
#include "Exception.h"
#include "Entity.h"
#include "ActionInstant.h"

/////////////////////////////////////////////////////////////////////////////////////////////////

/////////////////////////////////////////////////////////////////////////////////////////////////
void Action_Func::start_with_target(Entity *target)
{
    //__super::start_with_target(target);
    Action_Instant::start_with_target(target);

		(*slot_func_)(target, data_);
}

void Action_Func1::start_with_target(Entity *target)
{
	//__super::start_with_target(target);
    Action_Instant::start_with_target(target);

	(slot_func_)(target, data_);
}

 void Action_FuncND::start_with_target(Entity *target)
 {
 	//__super::start_with_target(target);
     Action_Instant::start_with_target(target);
 
 	(*slot_func_nd_)(target, pdata_);
 }

 void Action_FuncNDD::start_with_target(Entity *target)
 {
	 //__super::start_with_target(target);
     Action_Instant::start_with_target(target);

	 (slot_func_nd_)(target, pdata_);
 }

/////////////////////////////////////////////////////////////////////////////////////////////////
