#ifndef __BIRD_FACTORY_H__
#define __BIRD_FACTORY_H__

///////////////////////////////////////////////////////////////////////////////////////////
#include "Prereqs.h"
#include "Bird.h"

///////////////////////////////////////////////////////////////////////////////////////////
class GameTableLogic ;

///////////////////////////////////////////////////////////////////////////////////////////
class Bird_Factory
{
public:
	typedef std::list<Bird > Birds;

	//��������
public:
	//���캯��
	Bird_Factory();
	virtual ~Bird_Factory();

public:
	void initialise(GameTableLogic *pTableFrameSink);
    void cleanup();

	//�����
public:
	//���ɵ�������
	void create_onetype_bird(int bird_type);
	//����������
	void create_special_bird();

	void create_scene_left_bird();
	void create_scene_right_bird();
	void create_normal_bird();
	void create_small_bird();
	void create_big_bird();
	void create_huge_bird();
	void create_small_clean_sweep_bird();
    void create_clean_sweep_bird();
    void create_group_bird();
	void create_special_type();
	
	void create_boss_bird();



	//�������
public:
	//����
	void create_scene_round();				//Բ����
	void create_scene_bloating();			//�ĳ���
	void create_scene_double_round();		//����Բ����
	void create_scene_double_bloating();	//�����ĳ���
	//��������
	void create_scene_array_one(bool is_left = true);	//һ�������С��
	void create_scene_array_two(bool is_left = true);	//�����������뻷 ÿ����һ������
	void create_scene_array_three(bool is_left = true);	//�໷��
	void create_scene_array_four();	//���¶��Σ�ÿ�Ŵ�һ�����㣩 �м��δ���
	void create_scene_array_five(); //���¶��� �γ�ʱ��ͣ��һ��
	void create_scene_boss_one(); //BOSS����
	/*
	 *	����ͣС�㣬�м�ͣ����
	 */
	void create_scene_array_six();
	/*
	 *	����ͣ����
	 */
	void create_scene_array_seven();

	//��ȡ��������
	bool get_bird(Bird *bird);
	//�����
	void bird_clear() { birds_.clear(); }

	bool get_small_clean_sweep_bird(Bird *bird);

	bool get_shape_bird(Bird * bird);
	
	//������
	void add_special_birds(Bird bird);
	bool get_special_bird(Bird * bird);

	void add_special_birds_1(Bird &bird);
	bool get_special_bird_1(Bird * bird);

	void add_special_birds_2(Bird &bird);
	bool get_special_bird_2(Bird * bird);

	void add_special_birds_3(Bird &bird);
	bool get_special_bird_3(Bird * bird);

	void add_special_birds_4(Bird &bird);
	bool get_special_bird_4(Bird * bird);

	void add_special_birds_5(Bird &bird);
	bool get_special_bird_5(Bird * bird);

	void add_special_birds_6(Bird &bird);
	bool get_special_bird_6(Bird * bird);

	void add_special_birds_boss(Bird &bird);
	bool get_special_birds_boss(Bird *bird);
	void rand_numer(int paths[], int count);
    uint32_t  get_bird_count(){return birds_.size();}


	Birds& get_all_bird_1() {return special_birds_1;}
	Birds& get_all_bird_2() {return special_birds_2;}
	Birds& get_all_bird_3() {return special_birds_3;}
	Birds& get_all_bird_4() {return special_birds_4;}
	Birds& get_all_bird_5() {return special_birds_5;}
	Birds& get_all_bird_6() {return special_birds_6;}
	Birds& get_all_bird_7() {return special_birds_boss;}
private:
	std::random_device 				m_random_device;
	std::mt19937 					m_random_gen;
	int special_type_;
	Birds birds_;
	Birds small_clean_sweep_bird_;
	Birds shape_birds_;
	Birds special_birds_;

	Birds special_birds_1;
	Birds special_birds_2;
	Birds special_birds_3;
	Birds special_birds_4;
	Birds special_birds_5;
	Birds special_birds_6;
	Birds special_birds_boss;
	GameTableLogic *m_pTableFrameSink;
};

///////////////////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////

#endif