#ifndef __ACTION_CUSTOM_H__
#define __ACTION_CUSTOM_H__

/////////////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "ActionInterval.h"

/////////////////////////////////////////////////////////////////////////////////////////////////
// class Move_Point
// {
// public:
// 	Move_Point();
//     Move_Point(const Point &position, float angle);
// 
//     ~Move_Point();
// 
// public:
//     float angle_;
//     Point position_;
// };

typedef std::vector<Move_Point> Move_Points; 

/////////////////////////////////////////////////////////////////////////////////////////////////
class  Action_Move_Point  : public Action_Interval
{
public:
    Action_Move_Point(float d, Move_Points &points, const Point &offse);
    virtual ~Action_Move_Point();

public:
	virtual void update(float time);

	virtual Move_Point move_to(float elapsed);

private:
	Move_Points move_points_;
};


///////////////////////////////////////////////////////////////////////////////////////////
class Action_Bullet_Move : public Action_Interval
{
public:
    Action_Bullet_Move(float angle, float speed);
    virtual ~Action_Bullet_Move();

public:
	virtual void step(float dt);
    virtual bool is_done() { return false; }

	virtual Move_Point move_to( float elapsed );

private:
	float angle_;
	float bullet_speed_;

	float dx_;
	float dy_;
};

class Action_Bullet_Move_To : public Action_Interval
{
public:
	Action_Bullet_Move_To(const Point& start, const Point end, float angle, float speed);
	virtual ~Action_Bullet_Move_To() {}

	virtual void update(float dt);
	virtual Move_Point move_to( float elapsed );
	//virtual void step(float dt);

private:
	float bullet_speed_;
	float angle_;
	Point start_;
	Point delta_;
	float length_;
	float dx_;
	float dy_;
};

class Action_Bird_Move : public Action_Interval
{
public:
	Action_Bird_Move() :Action_Interval(0.26),m_ptLast(0,0), m_ptCurrent(0,0) {}

protected:
	void move_angle();

protected:
	Point m_ptLast;
	Point m_ptCurrent;

};

//场景鱼移动 直线
class Action_Bird_Move_Linear : public Action_Bird_Move
{
public:
	Action_Bird_Move_Linear(float bird_speed, const Point& start, const Point& end);
	virtual ~Action_Bird_Move_Linear();

	virtual void start_with_target(Entity* entity);

	virtual void update(float time);

	virtual Move_Point move_to(float elapsed);

protected:
	Point start_;
	Point end_;
	Point delta_;
	float bird_speed_;
};

//场景鱼阵动画 直线 中间停顿
class Action_Bird_Move_Pause_Linear : public Action_Bird_Move
{
public:
	Action_Bird_Move_Pause_Linear(float bird_speed, float pause_time, const Point& start, const Point& pause, const Point& end, float start_angle = 0.f);
	virtual ~Action_Bird_Move_Pause_Linear();

	virtual void start_with_target(Entity *target);
	virtual void update(float time);

	virtual Move_Point move_to(float elapsed);

protected:
	Point start_;
	Point end_;
	Point front_delta_;
	Point back_delta_;
	Point pause_;			//暂停点
	float start_angle_;		//开始点等于暂停点时使用此角度
	float bird_speed_;
	float pause_time_;		//暂停时间
	float front_time_;		//暂停之前的动作时间
	float back_time_;		//暂停之后的动作时间
};

//场景鱼阵动画 圆
class Action_Scene_Round_Move : public Action_Bird_Move
{
public:
	Action_Scene_Round_Move(const Point& center, float radius, float rotate_duration, float start_angle, float rotate_angle, float move_duration, float bird_speed);
	virtual ~Action_Scene_Round_Move();

	virtual void start_with_target(Entity* entity);
	virtual void update(float time);
	virtual Move_Point move_to(float elapsed);
	Point start_position() const { return start_; }

private:
	Point center_;
	float radius_;
	float rotate_duration_;
	float start_angle_;
	float rotate_angle_;
	float move_duration_;
	Point delta_;
	float bird_speed_;
	Point start_;
	int stage_;
};

/////////////////////////////////////////////////////////////////////////////////////////////////
#endif