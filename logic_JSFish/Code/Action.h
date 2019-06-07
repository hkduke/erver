#ifndef __ACTION_H__
#define __ACTION_H__

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
class Entity;

class Move_Point
{
public:
	Move_Point();
    Move_Point(const Point &position, float angle);

    ~Move_Point();

public:
    float angle_;
    Point position_;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action
{
public:
    Action();
    virtual ~Action();

public:
    uint32_t get_tag() const ;
    void set_tag(uint32_t tag);

	uint32_t get_data() const;
	void set_data(uint32_t data);

	bool is_pause() const ;
	void set_pause(bool pause);

	void pause();
    void resume();

    Entity *target() const ;
    virtual void start_with_target(Entity *target) ;

    virtual bool is_done() ;
    virtual void stop() ;

    virtual void step(float dt) ;
    virtual void update(float time);

	virtual float get_speed() const { return speed_; }
	virtual void set_speed(float speed) { speed_ = speed; }

	virtual float duration() const { return duration_; }
	virtual void set_duration(float duration) {duration_ = duration;}

	virtual float elapsed() const { return elapsed_; }
	virtual void set_elapsed( float elapsed ) { elapsed_ = elapsed; }

	virtual Point start_position() const {return start_; }
	virtual void set_start_point( Point start ) { start_ = start; }

	virtual Move_Point move_to(float elapsed) { return Move_Point(Point(-100, -100),0); }

    //virtual Action *clone() const ;
    //virtual Action *reverse() const ;

protected:
    uint32_t tag_;
	bool pause_;
	uint32_t data_;
    Entity *target_;
	float speed_;
	float duration_;
	float elapsed_;
	bool stop_;
	Point start_;
};

/////////////////////////////////////////////////////////////////////////////////////////////////


#endif