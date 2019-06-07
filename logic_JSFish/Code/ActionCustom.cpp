#include "stdafx.h"
#include "Exception.h"
#include "Entity.h"
#include "ActionCustom.h"
#include "../Cmd/CMD_CatchBird.h"

const float kServerSpeed = kSpeed;

static float CalculateRotateAngle(Point pntBegin, Point pntNext)
{
	float dRotateAngle = atan2(fabs((float)pntBegin.x_ - pntNext.x_), fabs((float)pntBegin.y_ - pntNext.y_));

	//如果下一点的横坐标大于前一点(在第一和第四象限)
	if (pntNext.x_ >= pntBegin.x_)
	{
		if (pntNext.y_ >= pntBegin.y_)
			//不做任何处理
			dRotateAngle=M_PI-dRotateAngle;
		else
		{
			//dRotateAngle=dRotateAngle;
		}
	}
	else
	{
		//第二象限
		if (pntNext.y_ >= pntBegin.y_)
			dRotateAngle=M_PI+dRotateAngle;
		else//第三象限
			dRotateAngle=2*M_PI-dRotateAngle;
	}
	dRotateAngle = dRotateAngle * 180 / M_PI;
	return dRotateAngle;
}

/////////////////////////////////////////////////////////////////////////////////////////
// Move_Point::Move_Point():angle_(0) 
// { 
// }
// 
// Move_Point::Move_Point(const Point &position, float angle)
// :position_(position), angle_(angle)
// {
// }
// 
// Move_Point::~Move_Point()
// {
// }

/////////////////////////////////////////////////////////////////////////////////////////
Action_Move_Point::Action_Move_Point(float d, Move_Points &points, const Point &offset) :Action_Interval(d)
{
	Move_Point move_point;

	for (Move_Points::iterator i=points.begin(); i!=points.end(); ++i)
	{
		if (offset.x_!=0 || offset.y_!=0)
		{
			move_point = *i;
			move_point.position_ += offset;
			move_points_.push_back(move_point);

		}
		else
		{
			move_points_.push_back((*i));
		}
	}

	duration_ = d*move_points_.size();
}

Action_Move_Point::~Action_Move_Point()
{
}

void Action_Move_Point::update(float time)
{
	float fDiff;
	float fIndex = time * move_points_.size();
	int index = fIndex;

	fDiff = fIndex - index;

	if (index >= move_points_.size())
	{
		index = move_points_.size() - 1;
	}

	Move_Point move_point;

	if (index<move_points_.size()-1)
	{
		Move_Point move_point1 = move_points_.at(index);
		Move_Point move_point2 = move_points_.at(index+1);

		move_point.position_ = move_point1.position_*(1.0-fDiff)+ move_point2.position_*fDiff;
		move_point.angle_ = move_point1.angle_*(1.0-fDiff)+ move_point2.angle_*fDiff;

		if (std::abs(move_point1.angle_-move_point2.angle_) > 180.0)
		{
			move_point.angle_ = move_point1.angle_;
		}
	}
	else
	{
		move_point = move_points_.at(index);
	}

	target_->set_position(move_point.position_);
	target_->set_rotation((360.0-move_point.angle_)*M_PI/180.0);
}

Move_Point Action_Move_Point::move_to(float elapsed)
{
	float time =  std::min(1.0f, (elapsed_ + elapsed) / duration_);
	float fIndex = time * move_points_.size();
	int index = fIndex;

	float fDiff = fIndex - index;
	if (index >= move_points_.size())
		index = move_points_.size() -1;

	Move_Point move_to;

	if (index < move_points_.size() -1)
	{
		 Move_Point move_point1 = move_points_.at(index);
		 Move_Point move_point2 = move_points_.at(index + 1);
		 move_to.position_ = move_point1.position_*(1.0 - fDiff) + move_point2.position_ * fDiff;
		 move_to.angle_ = CalculateRotateAngle(move_point1.position_, move_point2.position_);
	}
	else
	{
		move_to.position_ = move_points_.at(index).position_;
		move_to.angle_ = move_points_.at(index).angle_;
	}

	return move_to;
}

///////////////////////////////////////////////////////////////////////////
Action_Bullet_Move::Action_Bullet_Move(float angle, float speed) : Action_Interval(1.0), angle_(angle), bullet_speed_(speed)
{
	dx_ = std::cos(angle_ - M_PI_2);
	dy_ = std::sin(angle_ - M_PI_2);
}

Action_Bullet_Move::~Action_Bullet_Move()
{
}

void Action_Bullet_Move::step(float dt)
{
	if(firstTick_)
	{
		firstTick_ = false;
		elapsed_ = 0;
	}
	else
	{

		elapsed_ += dt;
	}
	Point pt(target_->position());

	pt.x_ += bullet_speed_ * dx_ * dt * speed_;
	pt.y_ += bullet_speed_ * dy_ * dt * speed_;

    if (pt.x_ < 0.0f) { pt.x_ = 0 + (0 - pt.x_); dx_ = -dx_; angle_ =  - angle_; }
    if (pt.x_ > kScreenWidth)  {pt.x_ = kScreenWidth - (pt.x_ - kScreenWidth); dx_ = -dx_; angle_ =  - angle_;}
		
	if (pt.y_ < 0.0f) { pt.y_ = 0 + (0 - pt.y_); dy_ = -dy_; angle_ = M_PI - angle_;}
	if (pt.y_ > kScreenHeight)  {pt.y_ = kScreenHeight - (pt.y_ - kScreenHeight); dy_ = -dy_; angle_ = M_PI - angle_;}

	target_->set_rotation(angle_);
	target_->set_position(pt);
}

Move_Point Action_Bullet_Move::move_to(float elapsed)
{
	Point pt(target_->position());

	pt.x_ += bullet_speed_ * dx_ * elapsed * speed_;
	pt.y_ += bullet_speed_ * dy_ * elapsed * speed_;

	if (pt.x_ > kScreenWidth)  {pt.x_ = kScreenWidth - (pt.x_ - kScreenWidth);}
	if (pt.y_ > kScreenHeight)  {pt.y_ = kScreenHeight - (pt.y_ - kScreenHeight);}

    Move_Point move_to;
	move_to.position_ = pt;
	move_to.angle_ = angle_;

	return move_to;
}

//////////////////////////////////////////////////////////////////////////
Action_Bullet_Move_To::Action_Bullet_Move_To(const Point& start, const Point end, float angle, float speed)
:Action_Interval(1.0), bullet_speed_(speed),angle_(angle - M_PI_2), start_(start)
{
	delta_ = end - start;
	float length = std::sqrt(delta_.x_ * delta_.x_ + delta_.y_ * delta_.y_);
	length_ = length;
	duration_ = length / bullet_speed_;

	dx_ = std::cos(angle_ - M_PI_2);
	dy_ = std::sin(angle_ - M_PI_2);
}

 void Action_Bullet_Move_To::update(float dt)
 {
 	Point pt(target_->position());
 
 	pt.x_ = start_.x_ + delta_.x_ * dt;
 	pt.y_ = start_.y_ + delta_.y_ * dt;
 	
 	target_->set_rotation(angle_);
 	target_->set_position(pt);
 }


 Move_Point Action_Bullet_Move_To::move_to(float elapsed)
 {
	 float time = std::min(1.0f, (elapsed_ + elapsed)/ duration_);
	 return Move_Point(Point(start_.x_ + delta_.x_ * time, start_.y_ + delta_.y_ * time), target_->rotation());
 }

// void Action_Bullet_Move_To::step(float dt)
// {
// 	float time = elapsed_;
// 	//static float _preTime = dt;
// 	if (target_ && time <= duration_/* && !isAutoMove*/)
// 	{
// 		Point pt;
// 		///<  求百分比
// 		float precent = time / duration_;
// 		///< 求现在因走的距离
// 		float now_length = precent * length_;
// 		///< 现在应到的位置
// 		pt.x_ = start_.x_ + delta_.x_ * time;
// 		pt.y_ = start_.y_ + delta_.y_ * time;
// 		target_->set_position(pt);
// 		target_->set_rotation((angle_ - M_PI_2) * 180 / M_PI);
// 	}
// }

//////////////////////////////////////////////////////////////////////////
void Action_Bird_Move::move_angle()
{
	float temp_value = 0.f;
	temp_value = CalculateRotateAngle(m_ptCurrent, m_ptLast);

	target_->set_rotation(temp_value);
}

//////////////////////////////////////////////////////////////////////////
Action_Bird_Move_Linear::Action_Bird_Move_Linear(float bird_speed, const Point& start, const Point& end)
:start_(start), end_(end), bird_speed_(bird_speed)
{
	delta_ = end_ - start_;
	float length = std::sqrt(delta_.x_ * delta_.x_ + delta_.y_ * delta_.y_);
	duration_ = length / bird_speed_;
}

Action_Bird_Move_Linear::~Action_Bird_Move_Linear() {}

void Action_Bird_Move_Linear::start_with_target(Entity* entity)
{
	//start_ = entity->position();
// 	delta_ = end_ - start_;
// 	float length = std::sqrt(delta_.x_ * delta_.x_ + delta_.y_ * delta_.y_);
// 	if (length > 0)
// 	{
// 		if (delta_.y_ > 0)
// 			entity->set_rotation(std::acos(delta_.x_ / length));
// 		else
// 			entity->set_rotation(-std::acos(delta_.x_ / length));
// 	}	
	Action::start_with_target(entity);
}

void Action_Bird_Move_Linear::update(float time)
{
	Point pt(start_.x_ + delta_.x_ * time, start_.y_ + delta_.y_ * time);
	target_->set_position(pt);

	m_ptCurrent = pt;
	move_angle();
	m_ptLast = pt;
}

Move_Point Action_Bird_Move_Linear::move_to(float elapsed)
{
	float time = std::min(1.0f, (elapsed_ + elapsed)/ duration_);
	return Move_Point(Point(start_.x_ + delta_.x_ * time, start_.y_ + delta_.y_ * time), target_->rotation());
}

////////////////////////////////////////////////////////////////////////
Action_Bird_Move_Pause_Linear::Action_Bird_Move_Pause_Linear(float bird_speed, float pause_time, const Point& start, const Point& pause, const Point& end, float start_angle /* = 0.f */)
:start_(start), end_(end), pause_(pause), start_angle_(start_angle),  bird_speed_(bird_speed), pause_time_(pause_time)
{
	front_delta_ = pause_ - start_;
	float length = std::sqrt(front_delta_.x_ * front_delta_.x_ + front_delta_.y_ * front_delta_.y_);
	front_time_ = length / bird_speed;
	back_delta_ = end_ - pause_;
	length = std::sqrt(back_delta_.x_ * back_delta_.x_ + back_delta_.y_ * back_delta_.y_);
	back_time_ = length / bird_speed;
	duration_ = front_time_ + pause_time_ + back_time_;
}

Action_Bird_Move_Pause_Linear::~Action_Bird_Move_Pause_Linear() {}

void Action_Bird_Move_Pause_Linear::start_with_target(Entity *target)
{
	Action::start_with_target(target);
}

void Action_Bird_Move_Pause_Linear::update(float time)
{
	Point pt(0.0f, 0.0f);
	float elapsed = time * duration_;
	if (elapsed <= front_time_)
	{
		time = elapsed / front_time_;
		pt.x_ = start_.x_ + front_delta_.x_ * time;
		pt.y_ = start_.y_ + front_delta_.y_ * time;
		target_->set_position(pt);

		m_ptCurrent = pt;
		move_angle();
		m_ptLast = pt;
	}
	else if (elapsed > front_time_ + pause_time_)
	{
		time = (elapsed - (front_time_ + pause_time_)) / back_time_;
		pt.x_ = pause_.x_ + back_delta_.x_ * time;
		pt.y_ = pause_.y_ + back_delta_.y_ * time;
		target_->set_position(pt);

		m_ptCurrent = pt;
		move_angle();
		m_ptLast = pt;
	}
	else if (start_ == pause_)
	{
		pt = pause_;
		target_->set_position(pt);
		target_->set_rotation((360.0-start_angle_)*M_PI/180.0);
	}
}

Move_Point Action_Bird_Move_Pause_Linear::move_to(float elapsed)
{
	float time = 0.0;
	if ( (elapsed_ + elapsed) <= front_time_)
	{
		float f1 = 1.0f;
		float f2 = (elapsed_ + elapsed) / duration_;
		//time = std::min(1.0, (elapsed_ + elapsed) / duration_);
		time = std::min(f1, f2);
		return Move_Point(Point(start_.x_ + front_delta_.x_ * time, start_.y_ + front_delta_.y_ * time), target_->rotation());
	}
	else if ((elapsed_ + elapsed) > front_time_ + pause_time_)
	{
		float f1 = 1.0f;
		float f2 = ((elapsed_ + elapsed) - (front_time_ + pause_time_)) / back_time_;
		//time = std::min(1.0, ((elapsed_ + elapsed) - (front_time_ + pause_time_)) / back_time_);
		time = std::min(f1, f2);
		return Move_Point(Point(pause_.x_ + back_delta_.x_ * time, pause_.y_ + back_delta_.y_ + back_delta_.y_ * time), target_->rotation());
	}

	return Move_Point(Point(pause_.x_, pause_.y_), target_->rotation());
}

////////////////////////////////////////////////////////////////////////
Action_Scene_Round_Move::Action_Scene_Round_Move(const Point& center, float radius, float rotate_duration, float start_angle, float rotate_angle, float move_duration, float bird_speed)
	:center_(center), radius_(radius), rotate_duration_(rotate_duration), start_angle_(start_angle), rotate_angle_(rotate_angle), move_duration_(move_duration),bird_speed_(bird_speed),stage_(0)
{
	duration_ = rotate_duration + move_duration_;
	start_.x_ = center_.x_ + radius_ * std::cos(start_angle_);
	start_.y_ = center_.y_ + radius_ * std::sin(start_angle_);
}

Action_Scene_Round_Move::~Action_Scene_Round_Move()
{

}

void Action_Scene_Round_Move::start_with_target(Entity* entity)
{
	Action::start_with_target(entity);
	stage_ = 0;
	start_.x_ = center_.x_ + radius_ * std::cos(start_angle_);
	start_.y_ = center_.y_ + radius_ * std::sin(start_angle_);
}

void Action_Scene_Round_Move::update(float time)
{
  	if (stage_ == 0 && time * duration_ >= rotate_duration_)
  	{
  		stage_ = 1;
  		float angle = target_->rotation();
  		delta_.x_ = std::cos(angle);
  		delta_.y_ = std::sin(angle);
  	}
	if (stage_ == 0)
	{
		Point position;
		float angle = start_angle_ + rotate_angle_ * time;
		position.x_ = center_.x_ + radius_ * std::cos(angle);
		position.y_ = center_.y_ + radius_ * std::sin(angle);
		target_->set_position(position);

		m_ptCurrent = position;
		move_angle();
		m_ptLast = position;
	}
 	else
 	{
 		Point position = target_->position();
 		position.x_ += bird_speed_ * (time / 4) * delta_.x_;
 		position.y_ += bird_speed_ * (time / 4) * delta_.y_;
 		target_->set_position(position);
 	}
}

 Move_Point Action_Scene_Round_Move::move_to(float elapsed)
 {
 	if (stage_ == 0)
 	{
 		if (elapsed_ + elapsed >= duration_)
 		{
 			float angle = start_angle_ + rotate_angle_;
 			Move_Point move_to(Point(center_.x_ + radius_ * std::cos(angle), center_.y_ + radius_ * std::sin(angle)),angle);
 			float el = elapsed_ + elapsed - duration_;
 			int count = int(el / kServerSpeed);
 			while ((count--) > 0)
 			{
 				move_to.position_.x_ += bird_speed_ * kServerSpeed * delta_.x_ * speed_;
 				move_to.position_.y_ += bird_speed_ * kServerSpeed * delta_.y_ * speed_;
 			}
 			return move_to;
 		}
 		else
 		{
 			float f1 = 1.0f;
 			float f2 = (elapsed_ + elapsed) / duration_;
 			//float time = min(1.0f, (elapsed_ + elapsed) / duration_);
 			float time = std::min(f1,f2);
 			float angle = start_angle_ + rotate_angle_ * time;
 			return Move_Point(Point(center_.x_ + radius_ * std::cos(angle), center_.y_ + radius_ * std::sin(angle)),angle);
 		}
 	}
 	else
 	{
 		int count = int(elapsed / kServerSpeed);
 		Move_Point move_to;
		move_to.position_ = target_->position();
 		while ((count--) > 0)
 		{
 			move_to.position_.x_ += bird_speed_ * kServerSpeed * delta_.x_ * speed_;
 			move_to.position_.y_ += bird_speed_ * kServerSpeed * delta_.y_ * speed_;
			move_to.angle_ = target_->rotation();
 		}
 		return move_to;
 	}
 }

////////////////////////////////////////////////////////////////////////////////////////////