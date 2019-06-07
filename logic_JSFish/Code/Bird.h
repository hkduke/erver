#ifndef __BIRD_H__
#define __BIRD_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Entity.h"
#include <random>

///////////////////////////////////////////////////////////////////////////////////////////
class Bird : public Entity
{
public:
	Bird();
	virtual ~Bird();

public:
	void reset();

    uint8_t get_type() const { return type_; }
	void set_type(uint8_t type) { type_ = type; }

	uint8_t get_item() const { return item_; }
	void set_item(uint8_t item) { item_ = item; }

	uint16_t get_path_id() const { return path_id_; }
	void set_path_id(uint16_t path_id) { path_id_ = path_id; }

	uint8_t get_path_type() const { return path_type_; }
	void set_path_type(uint8_t path_type) { path_type_ = path_type; }

	float get_path_delay() const { return path_delay_; }
	void set_path_delay(float path_delay) { path_delay_ = path_delay; }

	Point get_path_offset() const { return path_offset_; }
	void set_path_offset(const Point &path_offset) { path_offset_ = path_offset; }

	float get_speed() const { return speed_; }
	void set_speed( float speed ) { speed_ = speed; }

	uint32_t get_mulriple() const {return mulriple_;}
	void set_mulriple(const uint32_t mulriple) 
	{
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_int_distribution<> dis(0,mulriple-1);
        m_lRandNumber = dis(gen);
        //m_lRandNumber = Random(0,mulriple-1);
	    mulriple_ = mulriple;
	}

	//鱼阵需要的数据
	uint8_t get_yuzhen_type() const { return yuzhen_type_; }
	void set_yuzhen_type( const uint8_t yuzhen_type ) { yuzhen_type_ = yuzhen_type; }
	//Linear
	Point get_start_pt() const { return start_; }
	void set_start_pt( const Point &start ) { start_ = start; }
	Point get_linear_end_pt() const { return end_; }
	void set_linear_end_pt( const Point &end ) { end_ = end; }
	//Pause
	Point get_pause_pt() const { return pause_; }
	void set_pause_pt( const Point& pause ) { pause_ = pause; }
	float get_pause_time() const { return pause_time_; }
	void set_pause_time( const float& pause_time ) { pause_time_ = pause_time; }

	//round
	float get_round_radius() const { return radius_; }
	void set_round_radius(const float radius) { radius_ = radius; }
	float get_round_rotate_duration() const { return rotate_duration_; }
	void set_round_rotate_duration( const float rotate_duration ) { rotate_duration_ = rotate_duration; }
	float get_round_start_angle() const { return start_angle_; }
	void set_round_start_angle( const float start_angle ) { start_angle_ = start_angle; }
	float get_round_rotate_angle() const { return rotate_angle_; }
	void set_round_rotate_angle( const float rotate_angle ) { rotate_angle_ = rotate_angle; }
	float get_round_move_duration() const { return move_duration_; }
	void set_round_move_duration( const float move_duration ) { move_duration_ = move_duration; }

	//血量设置
public:
	//鱼基础倍率
	int  GetFishBaseMul(){ return m_nFishBaseMul; }
	void SetFishBaseMul(uint32_t nFishBaseMul) {   m_nFishBaseMul = nFishBaseMul; }
	
	//鱼死亡倍率
	int GetFishDieMul() { return m_nFishDieMul; }
	void SetFishDieMul(uint32_t nFishDieMul) 
	{ m_nFishDieMul = nFishDieMul; }

	//被击中的最大炮
	int GetMaxBullet() { return m_nMaxBullet; }
	void SetMaxBullet(uint32_t nMaxBullet) { m_nMaxBullet = nMaxBullet; }

	//附加鱼币
	SCORE GetPlusScore() { return m_lPlusScore; }
	void SetPlusScore(SCORE lPlusScore) { m_lPlusScore = lPlusScore; }

	//总消耗
	SCORE GetHitScore() { return m_lHitScore; }
	void SetHitScore(SCORE lHitScore) { m_lHitScore += lHitScore; }

	time_t GetCreateTime(){return m_tCreatTime;}
	void SetCreateTime(time_t create_time) {m_tCreatTime = create_time;}
	//座椅消耗
	void ZeroChairScore()
	{
		//ZeroMemory(m_lChairScore, sizeof(m_lChairScore));
		memset(m_lChairScore, 0, sizeof(m_lChairScore));
	}

	SCORE GetChairScore(unsigned short wChairID)
	{
		return m_lChairScore[wChairID];
	}

	void SetChairScore(unsigned short wChairID, SCORE lChairScore)
	{
		m_lChairScore[wChairID] += lChairScore;
	}
	//新库存机制
public:
	//获取捕鱼系数
	int GetFishDieRatio(){return m_iFishDieRatio;}
	//设置捕鱼系数
	void SetFishDieRatio(int iFishDieRatio){m_iFishDieRatio=iFishDieRatio;};
	//血量相关
private:
	uint32_t  m_nFishBaseMul; //鱼基础倍率
	uint32_t  m_nFishDieMul;  //鱼死亡倍率
	uint32_t  m_nMaxBullet;   //命中最大炮
	SCORE m_lPlusScore;       //附加鱼币
	SCORE m_lHitScore;        //总消耗
	SCORE m_lChairScore[GAME_PLAYER]; //座椅消耗

public:
	//创建日期
	time_t m_tCreatTime;
	int64_t  m_lRandNumber;
private:
    uint8_t type_;
	uint8_t item_ ;

    uint16_t path_id_;
	uint8_t path_type_;
    float path_delay_;
	Point path_offset_;
	float speed_;
	uint32_t mulriple_;

	//鱼阵需要的数据
	uint8_t yuzhen_type_;	
	//Linear
	Point start_;
	Point end_;
	//Pause
	Point pause_;
	float pause_time_;

	//round
	float radius_;
	float rotate_duration_;
	float start_angle_;
	float rotate_angle_;
	float move_duration_;
public:
    int   m_iFishDieRatio;
};

///////////////////////////////////////////////////////////////////////////////////////////
typedef std::vector<Bird *> Birds;

///////////////////////////////////////////////////////////////////////////////////////////

#endif