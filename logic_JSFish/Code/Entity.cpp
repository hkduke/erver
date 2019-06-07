#include "stdafx.h"
#include "Exception.h"
#include "Entity.h"

///////////////////////////////////////////////////////////////////////////
Entity::Entity()
:id_(INVALID_ID),
index_(INVALID_ID),
action_id_(0),
rotation_(0.0f),	
size_(0,0),
position_(0,0),
tag_(0)
{
}

Entity::~Entity()
{
}

///////////////////////////////////////////////////////////////////////////
