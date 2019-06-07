#ifndef __ROLE_H__
#define __ROLE_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////
class Role
{
public:
	Role();
	~Role();

public:
	void initialise(uint16_t id);
    void cleanup();

	uint16_t get_id() const { return chair_id_; }
	void set_id(uint16_t id) ;

	SCORE get_gold() const { return gold_; }
	bool set_gold(SCORE gold) 
	{
		if (gold < 0)
			return false;
		gold_ = gold;
		return true;
	}

	SCORE get_exchange_gold() const { return exchange_gold_; }
	void set_exchange_gold(SCORE exchange_gold) { exchange_gold_ = exchange_gold; }

	uint16_t get_cannon() const { return cannon_type_; }
	void set_cannon(uint16_t cannon) { cannon_type_ = cannon; }

	Point get_position() const { return position_; }
	void set_position(Point position) { position_ = position; }

	uint32_t get_cannon_mulriple() const { return cannon_mulriple_; }
	void set_cannon_mulriple(uint32_t cannon_mulriple) { cannon_mulriple_ = cannon_mulriple; }

	int get_lock_bird_id() const { return lock_bird_id_; }
	void set_lock_bird_id( int lock_bird_id ) { lock_bird_id_ = lock_bird_id; }

	uint16_t get_energy_value() const { return energy_value_; }
	void set_energy_value( uint16_t energy_value ) { energy_value_ = energy_value; }


	SCORE get_award_gold() const { return award_gold_; }
	void set_award_gold( SCORE award_gold ) { award_gold_ = award_gold; }

	
private:
    uint16_t chair_id_;
    SCORE gold_;					//��ǰ���
	SCORE exchange_gold_;			//�һ����
	SCORE award_gold_;				//������
    uint16_t cannon_type_; 
	uint32_t cannon_mulriple_;
	int lock_bird_id_;				//������

	uint16_t energy_value_;			//����ֵ

	Point position_;
public:
	bool isCatchFish_;
	uint32_t 					fish_id_;					//��ID
	long long					Defray;						//��ˮ/����ӵ����ܽ��/Ѻע�ܶ�/��֧��
	long long					DefrayTimes;				//֧������/�����ӵ���/Ѻע����
	int							RoleType;					//�޽�ɫ���ֵ���Ϸ��Ĭ��Ϊ0����ע��Ϸ��Ϊׯ0����1
	long long					StartScore;					//��ʼ�����
	long long					maxScore;					//���Ӯȡ����
	long long					minScore;					//��СӮȡ����
	bool						FoceSwitch;					//ǿ��״̬����
	int							fireCount;					//���ڼ���
	int							timesDelay;					//�ӳټ���			
	int							lastOpenForceSwitch;		//��һ�δ򿪿��ص���������
	bool						isFirstTouchForceSwitch;	//�Ƿ��һ�δ򿪿���
	int							lastTouchForceControl;		//��һ�δ���ǿ��
	SCORE						userEnterScore;				//��ҽ�����
	SCORE						RecoverperScore;			//�ص�����
	int							Recoveraddodds;				//�ص�����
	int							RecoverDelay;				//�ص�֮����һ�εļ��
	bool						isRecoverStatus;			//�Ƿ�ص�״̬
	time_t						LastFireTime;				//�ϴο���ʱ��
	float						EnterScoreAddProb;			//�볡��ҵ��Ӹ���
	float						FireAddProb;
};

///////////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////////

#endif