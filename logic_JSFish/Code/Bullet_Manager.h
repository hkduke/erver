#ifndef __GOLD_BulletManager_H__
#define __GOLD_BulletManager_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Exception.h"
#include "Entity.h"
#include <random>
#include "glog_wrapper.h"
///////////////////////////////////////////////////////////////////////////////////////////
template<class Entity>
class Bullet_Manager
{
public:
	Bullet_Manager() : entitys_(NULL), count_(0), position_(0), max_count_(0),m_gen(m_rd())
	{
	}
	~Bullet_Manager() { cleanup(); }

	bool initialise(uint32_t max_count) 
	{
		entitys_ = new Entity[max_count];
		for (int i = 0; i < max_count; ++i) 
		{
			entitys_[i].set_id(i);
			entitys_[i].set_index(INVALID_ID);
			entitys_[i].set_chair_id(INVALID_CHAIR_ID);
			entitys_[i].set_bullet_id(max_count);
            entitys_[i].set_create_time(INVALID_BULLET_CREATE_TIME);
		}
		for (int i=0;i<4;++i)
		{
			cout_[i] = 0;
		}
		count_ = 0;
		max_count_ = max_count;
		return true;
	}

	//获取玩家所有当前存活子弹的倍率
	uint32_t  get_player_all_bullet_mulriple(int chair_id)
	{
		uint32_t  bullet_total_mulriple = 0;
		for(int i = 0; i < max_count_; i++)
		{
			if(entitys_[i].get_chair_id() == chair_id)
			{
				bullet_total_mulriple += entitys_[i].bullet_mulriple();
			}
		}

		return bullet_total_mulriple;
	}

	void cleanup()
	{
		if (entitys_ != NULL)
		{
			delete[]entitys_;
			entitys_ = NULL;
		}
		for (int i = 0; i < 4; ++i)
		{
			cout_[i] = 0;		//计数			40个子弹上限	
		}
		count_ = 0;
		position_ = 0;
		max_count_ = 0;
	}

	Entity* new_entity(int chairId, uint32_t bullet_id)
	{
		if (cout_[chairId] >= 40) {
			return NULL;
		}
		if(bullet_id < 0 || bullet_id >= max_count_) {
			return NULL;
		}
		if(entitys_[bullet_id].get_index() != INVALID_ID) {
			return NULL;
		}

		//将这个位置的index设置为全局计数的ID
		entitys_[bullet_id].set_index(count_);
		//设置子弹被哪个玩家使用
		entitys_[bullet_id].set_chair_id(chairId);
		//记录子弹在entitys_数组中的索引，这个索引也是子弹的id
		entitys_[bullet_id].set_bullet_id(bullet_id);
		//设置子弹创建的时间
		entitys_[bullet_id].set_create_time(time(NULL));

		//玩家子弹数增加
		cout_[chairId]++;
		//全局计数ID增加
		++count_;

		if(count_ >= 88888888)
		{
			count_ = 0;
		}

		return &(entitys_[bullet_id]);
	}

	void check_bullet_time_out()
	{
		time_t cur_time = time(NULL);

		for(int i = 0; i < max_count_; i++)
		{
			if(entitys_[i].get_index() != INVALID_ID && entitys_[i].get_create_time() != INVALID_BULLET_CREATE_TIME && (cur_time - entitys_[i].get_create_time() > 20))
			{
				//LOG(ERROR) << "bullet_id : " << entitys_[i].get_id() << " belong to player: " << entitys_[i].get_chair_id() << " time_out";

				entitys_[i].set_index(INVALID_ID);
				entitys_[i].set_chair_id(INVALID_CHAIR_ID);
				entitys_[i].set_bullet_id(max_count_);
				entitys_[i].set_create_time(INVALID_BULLET_CREATE_TIME);

				int chair_id = entitys_[i].get_chair_id();
				if(chair_id >= 0 && chair_id <=3)
				{
					cout_[chair_id]--;
				}
			}
		}
	}

	bool is_using(uint32_t bullet_id)
	{
		if(bullet_id < 0 || bullet_id >= max_count_)
		{
			return true;
		}

		if(entitys_[bullet_id].get_index() == INVALID_ID)
		{
			return false;
		}
		else
		{
			return true;
		}
	}

	bool is_player_have_bullet(int chair_id, uint32_t bullet_id)
    {
	    if(chair_id < 0 || chair_id >= 4)
        {
	        return false;
        }
        if(bullet_id < 0 || bullet_id >= max_count_)
        {
            return false;
        }
        if(entitys_[bullet_id].get_index() == INVALID_ID)
        {
            return false;
        }

        if(entitys_[bullet_id].get_chair_id() == chair_id)
        {
            return true;
        }
        else
        {
            return false;
        }
    }

	//获取子弹实例子
	Entity *get_entity_Bullet(int chairId, int bullet_id)
	{
		if (chairId >= 4) return NULL;
		if (cout_[chairId] <= 0) return NULL;
		if (bullet_id < 0 || bullet_id >= max_count_) return NULL;

		Entity *bullet = &entitys_[bullet_id];
		assert(bullet);

		if(bullet->get_index() == INVALID_ID || bullet->get_chair_id() != chairId )
		{
			return NULL;
		}

		return bullet;
	}

	//删除子弹
	bool delete_entity_Bullet(int chairId, int bullet_id)
	{
		if (chairId == INVALID_ID || chairId >= 4)
		{
			assert(false);
			return false;
		}

		Entity * entity = get_entity_Bullet(chairId, bullet_id);
		if (entity == NULL)
		{
			return false;
		}
		entity->set_index(INVALID_ID);
		entity->set_chair_id(INVALID_CHAIR_ID);
		entity->set_bullet_id(max_count_);
		entity->set_create_time(INVALID_BULLET_CREATE_TIME);

		cout_[chairId]--;
		return true;
	}

	//删除所有子弹
	void delete_all()
	{
		//memset(ids_, -1, max_count_ * sizeof(int));
		for (int i = 0; i < max_count_; ++i)
		{
			entitys_[i].set_index(INVALID_ID);
			entitys_[i].set_chair_id(INVALID_CHAIR_ID);
			entitys_[i].set_bullet_id(max_count_);
			entitys_[i].set_create_time(INVALID_BULLET_CREATE_TIME);
		}		
		for (int i = 0; i < 4; ++i)
		{
			cout_[i] = 0;
		}
		count_ = 0;
		position_ = 0;
	}
	//删除单个玩家所有子弹
	void delete_chair(int chairId)
	{
		if (entitys_ == NULL)
			return;

		for(int i = 0; i < max_count_; i++)
		{
			if(entitys_[i].get_index() != INVALID_ID && entitys_[i].get_chair_id() == chairId)
			{
				entitys_[i].set_index(INVALID_ID);
				entitys_[i].set_chair_id(INVALID_CHAIR_ID);
				entitys_[i].set_bullet_id(max_count_);
				entitys_[i].set_create_time(INVALID_BULLET_CREATE_TIME);
			}
		}
		cout_[chairId] = 0;
	}
	uint32_t get_count(int chairId) const { return cout_[chairId]; }

protected:
	int cout_[4];		//子弹计数
	Entity* entitys_; //子弹实体数组
	int count_;		  //count是全局子弹的ID  一直增长
	int position_;	  //position是子弹实体数组的循环下标
	int max_count_;	  //最大个数 MAX_BULLET_IN_MANAGER 2048
	std::random_device m_rd;
	std::mt19937 m_gen;
};



///////////////////////////////////////////////////////////////////////////////////////////


#endif