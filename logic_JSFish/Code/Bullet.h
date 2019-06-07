#ifndef __BULLET_H__
#define __BULLET_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Entity.h"

///////////////////////////////////////////////////////////////////////////////////////////
class Bullet : public Entity
{
public:
	Bullet();
	virtual ~Bullet();

public:
    uint16_t get_chair_id() const { return chair_id_; }
	void set_chair_id(uint16_t chair_id) { chair_id_ = chair_id; }

	uint16_t get_cannon_type() const { return cannon_type_; }
	void set_cannon_type(uint16_t cannon_type) { cannon_type_ = cannon_type; }
	
	float bullet_speed() const {return bullet_speed_;}
	void set_bullet_speed(float bullet_speed) {bullet_speed_ = bullet_speed; }

	int lock_bird_id() const {return lock_bird_id_; }
	void set_lock_bird_id( int lock_bird_id) {lock_bird_id_ = lock_bird_id; }

	uint32_t range_index() const {return range_index_; }
	void set_range_index(uint32_t range_index) {range_index_ = range_index; }

	uint32_t bullet_mulriple() const { return bullet_mulriple_; }
	void set_bullet_mulriple( uint32_t bullet_mulriple ) { bullet_mulriple_ = bullet_mulriple; }

	void  set_bullet_id(uint32_t id) {bullet_id = id;}
	uint32_t  get_bullet_id(){return bullet_id;}

	void set_create_time(time_t t){create_time = t;}
	time_t get_create_time(){return create_time;}
private:
    uint16_t chair_id_;
    uint32_t bullet_id;
    uint16_t cannon_type_;

	float bullet_speed_;
	int lock_bird_id_;
	uint32_t range_index_;	//��淶Χ����
	uint32_t bullet_mulriple_;
};

///////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Bullet *> Bullets;

///////////////////////////////////////////////////////////////////////////////////////////

#endif