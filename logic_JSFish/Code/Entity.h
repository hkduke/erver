#ifndef __ENTITY_H__
#define __ENTITY_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

class Entity
{
public:
	Entity();
	virtual ~Entity();

public:
	uint64_t get_id() const { return id_; }
	void set_id(uint32_t id) { id_ = id; }

	int get_index() const { return index_; }
	void set_index(int index) { index_ = index; }

	unsigned int action_id() const { return action_id_; }
	void set_action_id(unsigned int action_id ) { action_id_ = action_id; }

	Point position() const { return position_; }
    void set_position(const Point &position) { position_ = position; }

    Size size() const { return size_; }
    void set_size(const Size &size)  { size_ = size;  }

    float rotation() const { return rotation_; }
    void set_rotation(float rotation) { rotation_ = rotation; }

	uint64_t tag() const {return tag_;}
	void set_tag(uint64_t tag) {tag_ = tag;}

	void set_check(uint32_t check) {check_ = check;}
	uint32_t get_check(){return check_;}

protected:
	//entity创建时间
	time_t create_time;

private:
	uint64_t id_;
	int index_;
	unsigned int action_id_;
    float rotation_;
	Size size_;
    Point position_;

	uint64_t tag_;
	uint32_t check_;
};

///////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////

#endif