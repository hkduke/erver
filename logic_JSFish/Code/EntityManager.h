#ifndef __GOLD_MANAGER_H__
#define __GOLD_MANAGER_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Exception.h"
#include "Entity.h"

///////////////////////////////////////////////////////////////////////////////////////////
template<class Entity>
class Entity_Manager 
{
public:
	Entity_Manager() :ids_(NULL), entitys_(NULL), count_(0), position_(0), max_count_(0) 
	{
	}
	~Entity_Manager() { cleanup(); }

	bool initialise(uint32_t max_count) 
	{
		ids_ = new int[max_count];
		//memset(ids_, -1, max_count * sizeof(DWORD));
		entitys_ = new Entity[max_count];
		for (int i = 0; i < max_count; ++i) {
			entitys_[i].set_id(i);
			entitys_[i].set_index(INVALID_ID);
			ids_[i] = -1;
		}
		count_ = 0;
		position_ = 0;
		max_count_ = max_count;
		return true;
	}

	void cleanup() 
	{
		if (ids_ != NULL)
		{
			delete[]ids_;
			ids_ = NULL;
		}

		if (entitys_ != NULL)
		{
			delete []entitys_;
			entitys_ = NULL;
		}

		count_ = 0;
		position_ = 0;
		max_count_ = 0;
	}

	Entity* new_entity() 
	{
		int ret = 0;
		for (int i = 0; i < max_count_; ++i) 
		{
			if (entitys_[position_].get_index() == INVALID_ID) 
			{
				ret = position_;
				if ((++position_) >= max_count_)
				{
					position_ = 0;
				}
				if (count_ < 0) assert(false);
				ids_[count_] = ret;
				entitys_[ret].set_index(count_);
				++count_;
				return &(entitys_[ret]);
			}

			if ((++position_) >= max_count_)
				position_ = 0;
		}
		return NULL;
	}

	void delete_entity(uint32_t id) 
	{
		
		if (id == INVALID_ID || id >= max_count_) 
		{
			assert(false);
			return;
		}
		Entity* entity = get_entity(id);
		assert(entity);
		uint32_t index = entity->get_index();
		if (index < 0 || count_ - 1 < 0) assert(false);
		entity->set_index(INVALID_ID);
		if (count_ > 0) {
			ids_[index] = ids_[count_ - 1];
			if (index != count_ - 1)
			{
				if (ids_[index] < 0) assert(false);
				entity = get_entity(ids_[index]);
				entity->set_index(index);
				ids_[count_ - 1] = INVALID_ID;
			}
			--count_;
		}
	}
	/*void delete_entity_Bullet(uint32_t id)
	{
		if (id == INVALID_ID || id >= max_count_)
		{
			assert(false);
			return;
		}
		Entity * entity = get_entity_Bullet(id);
		if (entity == NULL) return;
		int index = entity->get_index();
		entity->set_index(INVALID_ID);
		if (count_ > 0)
		{
			ids_[index] = ids_[count_ - 1];

			if (index != (count_ - 1))
			{
				entity = get_entity(ids_[index]);

				assert(entity);
				entity->set_index(index);
				ids_[count_ - 1] = INVALID_ID;
			}

			--count_;
		}
	}*/
	void delete_all() 
	{
		//memset(ids_, -1, max_count_ * sizeof(int));
		for (int i = 0; i < max_count_; ++i) 
		{
			entitys_[i].set_index(INVALID_ID);
			ids_[i] = INVALID_ID;
		}

		count_ = 0;
		//position_ = 0;
	}

	Entity* get_entity(uint32_t id) 
	{
		if (id == INVALID_ID || id >= max_count_) 
		{
			assert(false);
			return NULL;
		}
		return &(entitys_[id]);
	}

	Entity* get_index_entity(uint32_t index) 
	{
		if (index == INVALID_ID || index >= count_) 
		{
			assert(false);
			return NULL;
		}
		return get_entity(ids_[index]);
	}
	Entity *get_entity_Bullet(uint32_t id)
	{
		if (id == INVALID_ID) return 0;
		for (int i = 0; i < max_count_; ++i)
		{
			if (entitys_[i].get_id() == id)
			{
				if (entitys_[i].get_index() == -1) 
				{
					return NULL;
				}
				return &(entitys_[i]);
			}
		}
		return 0;
	}

	uint32_t get_count() const { return count_; }

protected:
	int *ids_;
    Entity* entitys_;

	int count_ ;
	int position_ ;
	int max_count_ ;
};



///////////////////////////////////////////////////////////////////////////////////////////


#endif