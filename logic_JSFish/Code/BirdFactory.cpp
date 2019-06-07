#include "stdafx.h"
#include "Exception.h"
#include "GameTable.h"
#include "BirdFactory.h"
#include <stdlib.h>

//Game_Config_Xml* g_game_config_xml;
///////////////////////////////////////////////////////////////////////////
Bird_Factory::Bird_Factory():m_random_gen(m_random_device())
{

}
// Bird_Factory::Bird_Factory(Game_Table *table)
// {
// 	table_ = table;
//     special_type_ = 0;
// }

Bird_Factory::~Bird_Factory()
{
    //cleanup();
}

void Bird_Factory::initialise(GameTableLogic *pTableFrameSink)
{
	m_pTableFrameSink = pTableFrameSink;
    special_type_ = 0;
}

void Bird_Factory::cleanup()
{
    special_type_ = 0;
	if (!birds_.empty())
	{
		birds_.clear();
	}
	if (!small_clean_sweep_bird_.empty())
	{
		small_clean_sweep_bird_.clear();
	}
	if (!shape_birds_.empty())
	{
		shape_birds_.clear();
	}

	if (!special_birds_.empty())
	{
		special_birds_.clear();
	}

	if(!special_birds_1.empty())
	{
		special_birds_1.clear();
	}
	if(!special_birds_2.empty())
	{
		special_birds_2.clear();
	}
	if(!special_birds_3.empty())
	{
		special_birds_3.clear();
	}
	if(!special_birds_4.empty())
	{
		special_birds_4.clear();
	}
	if(!special_birds_5.empty())
	{
		special_birds_5.clear();
	}
	if(!special_birds_6.empty())
	{
		special_birds_6.clear();
	}
	if(!special_birds_boss.empty())
	{
		special_birds_boss.clear();
	}
}

void Bird_Factory::create_scene_left_bird()
{
// 	Bird bird;
// 	int index  = rand() % table_->get_path_manager()->get_scene_shape_count(0);
// 	index = 3;
// 	Move_Points &points = table_->get_path_manager()->get_scene_shape(index, 0);
// 
// 	Point pt;
// 	float delay = 0.0f;
// 	int i = points.size();
// 	for (int i = 0; i < 120; i++)
// 	{
// 		bird.set_id(INVALID_ID);
// 		bird.set_item(INVALID_ID);
// 		bird.set_type(0);
// 		bird.set_path_id(3);
// 		bird.set_path_type(PATH_TYPE_SCENE);
// 
// 
// // 		pt.x_ = Random(20, 50);
// // 		pt.y_ = Random(20, 50);
// 
// 		delay += 1.5;
// 
// 		//bird.set_path_offset(points[i].position_);
// 		bird.set_path_offset(Point(0,0));
// 		bird.set_path_delay(delay);
// 		bird.set_speed(3.5);
// 
// 		shape_birds_.push_back(bird);
// 	}


   	Bird bird;
   	int index  = rand() % m_pTableFrameSink->get_path_manager()->get_scene_shape_count(0);
   	//index = 3;
   	Move_Points &points = m_pTableFrameSink->get_path_manager()->get_scene_shape(index, 0);
   
   	for (size_t i = 0; i < points.size(); i++)
   	{
   		bird.set_id(INVALID_ID);
   		bird.set_item(INVALID_ID);
   		bird.set_type(points[i].angle_);
   		bird.set_path_id(0);
   		bird.set_path_type(PATH_TYPE_SCENE);
   		bird.set_path_offset(points[i].position_);
   		bird.set_path_delay(0);
   		bird.set_speed(2);
   
   		shape_birds_.push_back(bird);
   	}
}

void Bird_Factory::create_scene_right_bird()
{
	Bird bird;
	int index  = rand() % m_pTableFrameSink->get_path_manager()->get_scene_shape_count(1);
	//index = 3;
	Move_Points &points = m_pTableFrameSink->get_path_manager()->get_scene_shape(index, 1);

	for (size_t i = 0; i < points.size(); i++)
	{
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(points[i].angle_);
		bird.set_path_id(1);
		bird.set_path_type(PATH_TYPE_SCENE);
		bird.set_path_offset(points[i].position_);
		bird.set_path_delay(0);
		bird.set_speed(2);

		shape_birds_.push_back(bird);
	}
}

void Bird_Factory::create_normal_bird()
{
     int small_path[MAX_SMALL_PATH];
     rand_numer(small_path, MAX_SMALL_PATH);
     int big_path[MAX_BIG_PATH];
     rand_numer(big_path, MAX_BIG_PATH);
     int huge_path[MAX_HUGE_PATH];
     rand_numer(huge_path, MAX_HUGE_PATH);
 
 	int count = 10;
 	uint8_t bird_type;
 	uint16_t path_id;
 	uint8_t path_type;
 
     for (int i = 0; i < count; i++)
     {
         //bird_type = Random(4, MAX_BIRD_TYPE-1);
         std::uniform_int_distribution<> uid(4, MAX_BIRD_TYPE-1);
         bird_type = uid(m_random_gen);
 
         if (bird_type >= 15)
         {
             path_id = huge_path[i];
 			path_type = PATH_TYPE_HUGE;
         }
         else if (bird_type > 11 && bird_type < 15)
         {
             path_id = big_path[i];
 			path_type = PATH_TYPE_BIG;
         }
         else
         {
             path_id = small_path[i];
 			path_type = PATH_TYPE_SMALL;
         }
 
        Bird bird;
 
 		bird.set_id(INVALID_ID);
 		bird.set_item(INVALID_ID);
 		bird.set_type(bird_type);
 		bird.set_path_id(path_id);
 		bird.set_path_type(path_type);
 		bird.set_path_offset(Point(0,0));
 		bird.set_path_delay(0);
 
 		birds_.push_back(bird);
     }
}

void Bird_Factory::create_small_bird()
{
    int small_path[MAX_SMALL_PATH];
    rand_numer(small_path, MAX_SMALL_PATH);

	int count = 10;
	uint8_t bird_type;
	uint16_t path_id;
	uint8_t path_type;

    for (int i = 0; i < count; i++)
    {
        //bird_type = Random(4, 11);
        std::uniform_int_distribution<> uid(4,11);
        bird_type = uid(m_random_gen);

        path_id = small_path[i];
		path_type = PATH_TYPE_SMALL;
        
        Bird bird;

		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_path_id(path_id);
		bird.set_path_type(path_type);
		bird.set_path_offset(Point(0,0));
		bird.set_path_delay(0);

		birds_.push_back(bird);
    }
}

void Bird_Factory::create_big_bird()
{
    int big_path[MAX_BIG_PATH];
    rand_numer(big_path, MAX_BIG_PATH);

	int count = 6;
	uint8_t bird_type;
	uint16_t path_id;
	uint8_t path_type;

    for (int i = 0; i < count; i++)
    {
		//bird_type = Random(12, 14);
		std::uniform_int_distribution<> uid(12,14);
		bird_type = uid(m_random_gen);

        path_id = big_path[i];
		path_type = PATH_TYPE_BIG;
        
        Bird bird;

		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_path_id(path_id);
		bird.set_path_type(path_type);
		bird.set_path_offset(Point(0,0));
		bird.set_path_delay(0);

		birds_.push_back(bird);
    }
}

void Bird_Factory::create_huge_bird()
{
    int huge_path[MAX_HUGE_PATH];
    rand_numer(huge_path, MAX_HUGE_PATH);

	int count = 2;
	uint8_t bird_type;
	uint16_t path_id;
	uint8_t path_type;

    for (int i = 0; i < count; i++)
    {
		//bird_type = Random(15, MAX_BIRD_TYPE-3);
		std::uniform_int_distribution<> dis(15, MAX_BIRD_TYPE-3);
		bird_type = dis(m_random_gen);

        path_id = huge_path[i];
		path_type = PATH_TYPE_HUGE;
        
        Bird bird;

		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_path_id(path_id);
		bird.set_path_type(path_type);
		bird.set_path_offset(Point(0,0));
		bird.set_path_delay(0);

		birds_.push_back(bird);
    }
}

void Bird_Factory::create_small_clean_sweep_bird()
{
    int big_path[MAX_BIG_PATH];
    rand_numer(big_path, MAX_BIG_PATH);

//	int count = 3;
	uint8_t bird_type;
	uint16_t path_id;
	uint8_t path_type;
	uint8_t item = rand() % (11 );

	bird_type = 22;

	path_id = big_path[0];
	path_type = PATH_TYPE_BIG;

	Bird bird;

	bird.set_id(INVALID_ID);
	bird.set_item(item);
	bird.set_type(bird_type);
	bird.set_path_id(path_id);
	bird.set_path_type(path_type);
	bird.set_path_offset(Point(0,0));
	bird.set_path_delay(0);

	small_clean_sweep_bird_.push_back(bird);

// 	bird.set_id(INVALID_ID);
// 	bird.set_item(item);
// 	bird.set_type(bird_type);
// 	bird.set_path_id(path_id);
// 	bird.set_path_type(path_type);
// 	bird.set_path_offset(Point(0,150));
// 	bird.set_path_delay(0);
// 
// 	small_clean_sweep_bird_.push_back(bird);
// 
// 	bird.set_id(INVALID_ID);
// 	bird.set_item(item);
// 	bird.set_type(bird_type);
// 	bird.set_path_id(path_id);
// 	bird.set_path_type(path_type);
// 	bird.set_path_offset(Point(0,-150));
// 	bird.set_path_delay(0);
// 
// 	small_clean_sweep_bird_.push_back(bird);
}

void Bird_Factory::create_clean_sweep_bird()
{
    int big_path[MAX_BIG_PATH];
    rand_numer(big_path, MAX_BIG_PATH);

//	int count = 3;
	uint8_t bird_type;
	uint16_t path_id;
	uint8_t path_type;
	uint8_t item = rand() % (11);

	bird_type = 23;

	path_id = big_path[0];
	path_type = PATH_TYPE_BIG;

	Bird bird;

	bird.set_id(INVALID_ID);
	bird.set_item(item);
	bird.set_type(bird_type);
	bird.set_path_id(path_id);
	bird.set_path_type(path_type);
	bird.set_path_offset(Point(0,0));
	bird.set_path_delay(0);

	birds_.push_back(bird);
}

void Bird_Factory::create_group_bird()
{
	int type = rand() % 8;
	Point pt;
	float delay = 0.0f;
	//int count = Random(g_game_config_xml.bird_group_config_.distribute_count_min, g_game_config_xml.bird_group_config_.distribute_count_max);
	std::uniform_int_distribution<> dis(g_game_config_xml.bird_group_config_.distribute_count_min, g_game_config_xml.bird_group_config_.distribute_count_max);
	int count = dis(m_random_gen);

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_type(type);
	bird.set_path_id(rand() % MAX_SMALL_PATH);
	bird.set_path_type(PATH_TYPE_SMALL);
	bird.set_speed(g_game_config_xml.bird_config_[type].speed);

	for (int i = 0; i < count; i++)
	{
		if (type == 0)
		{
			if (i == 0)
			{
			}
			else if (i % 2 == 0)
			{
				//pt.x_ = Random(20, 50);
				//pt.y_ = Random(20, 50);
				std::uniform_int_distribution<> dis(20,50);
				pt.x_ = dis(m_random_gen);
				pt.y_ = dis(m_random_gen);

				//delay += Random_float(0.5f, 0.7f);
				std::uniform_real_distribution<> dis_f(0.5f, 0.7f);
				delay += dis_f(m_random_gen);
			}
			else 
			{
				//pt.x_ = -Random(20, 50);
				//pt.y_ = -Random(20, 50);
				std::uniform_int_distribution<> dis(20,50);
				pt.x_ = -dis(m_random_gen);
				pt.y_ = -dis(m_random_gen);

				//delay +=  Random_float(-0.24f, 0.24f);
				std::uniform_real_distribution<> dis_f(-0.24f, 0.24f);
				delay += dis_f(m_random_gen);
			}
		}
		else if (type == 1)
		{
			if (i == 0)
			{
			}
			else if (i % 2 == 0)
			{
				//pt.x_ = Random(20, 50);
				//pt.y_ = Random(20, 50);
				std::uniform_int_distribution<> dis(20,50);
				pt.x_ = dis(m_random_gen);
				pt.y_ = dis(m_random_gen);

				//delay += Random_float(0.5f, 0.7f);
				std::uniform_real_distribution<> dis_f(0.5f, 0.7f);
				delay += dis_f(m_random_gen);
			}
			else 
			{
				/*pt.x_ = -Random(20, 50);
				pt.y_ = -Random(20, 50);*/
				std::uniform_int_distribution<> dis(20,50);
				pt.x_ = -dis(m_random_gen);
				pt.y_ = -dis(m_random_gen);

				//delay +=  Random_float(-0.24f, 0.24f);
				std::uniform_real_distribution<> dis_f(-0.24f, 0.24f);
				delay += dis_f(m_random_gen);
			}

		}
		else
		{
			if (i == 0)
			{
			}
			else if (i % 3 == 1)
			{
				//pt.x_ = - Random(5, 10);
				//pt.y_ = - Random(5, 10);
				std::uniform_int_distribution<> dis(5,10);
				pt.x_ = -dis(m_random_gen);
				pt.y_ = -dis(m_random_gen);

				//delay +=  Random_float(0.6f, 0.8f);
				std::uniform_real_distribution<> dis_f(0.6f, 0.8f);
				delay += dis_f(m_random_gen);
			}
			else if (i % 3 == 2)
			{
				//pt.x_ = - Random(30, 50);
				//pt.y_ = - Random(30, 50);
				std::uniform_int_distribution<> dis(30,50);
				pt.x_ = -dis(m_random_gen);
				pt.y_ = -dis(m_random_gen);

				//delay +=  Random_float(-0.24f, 0.24f);
				std::uniform_real_distribution<> dis_f(-0.24f, 0.24f);
				delay += dis_f(m_random_gen);
			}
			else if (i % 3 == 0)
			{
//				pt.x_ = Random(30, 50);
//				pt.y_ = Random(30, 50);
				std::uniform_int_distribution<> dis(30,50);
				pt.x_ = dis(m_random_gen);
				pt.y_ = dis(m_random_gen);

				//delay +=  Random_float(-0.24f, 0.24f);
				std::uniform_real_distribution<> dis_f(-0.24f, 0.24f);
				delay += dis_f(m_random_gen);
			}
		}

		bird.set_path_offset(pt);
		bird.set_path_delay(delay);

		birds_.push_back(bird);
	}
}

void Bird_Factory::create_special_type()
{
    special_type_ ++;
}

void Bird_Factory::create_special_bird()
{
	int type = special_type_ % 3;
	special_type_++;
	int bird_type = rand() % 8 + 4;
//	float delay = 0.0f;
//	int path = 0;

	if (bird_type == 9)
		bird_type = 10;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_type(bird_type);
    bird.set_path_type(PATH_TYPE_SPECIAL);
	bird.set_speed(g_game_config_xml.bird_config_[bird_type].speed);

	if (type == 0)
	{
		for (int i = 0; i < 4; i++)
		{
			bird.set_path_id(i);

			for (int j = 0; j < 6; j++)
			{
				bird.set_path_delay(3.f * j + 0.8f);
                birds_.push_back(bird);
			}
		}
	}
	else if (type == 1)
	{
		for (int i = 0; i < 8; i++)
		{
			bird.set_path_id(4 + i);

			for (int j = 0; j < 6; j++)
			{
				bird.set_path_delay(3.f * j + 0.8f);
                birds_.push_back(bird);
			}
		}
	}
	else if(type == 2)
	{
		for (int i = 0; i < 12; i++)
		{
			bird.set_path_id(12 + i);
			
			for (int j = 0; j < 6; j++)
			{
				bird.set_path_delay(3.f * j + 0.8f);
				birds_.push_back(bird);
			}
		}
	}
}

void Bird_Factory::create_boss_bird()
{
	uint8_t bird_type = BIRD_TYPE_12;
	uint16_t path_id = rand() % MAX_HUGE_PATH;
	uint8_t path_type = PATH_TYPE_HUGE;

	Bird bird;

	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_type(bird_type);
	bird.set_path_id(path_id);
	bird.set_path_type(path_type);
	bird.set_path_offset(Point(0,0));
	bird.set_path_delay(0);

	birds_.push_back(bird);
}

void Bird_Factory::create_onetype_bird(int bird_type)
{
// 	if (birds_.size() > 20)
// 		return;

	int small_path[MAX_SMALL_PATH];
	rand_numer(small_path, MAX_SMALL_PATH);
	int big_path[MAX_BIG_PATH];
	rand_numer(big_path, MAX_BIG_PATH);
	int huge_path[MAX_HUGE_PATH];
	rand_numer(huge_path, MAX_HUGE_PATH);

	if (bird_type < MAX_BIRD_TYPE)
	{
		BirdConfig bird_config = g_game_config_xml.bird_config_[bird_type];
		//int count = Random(bird_config.distribute_count_min, bird_config.distribute_count_max);
		std::uniform_int_distribution<> dis(bird_config.distribute_count_min, bird_config.distribute_count_max);
		int count = dis(m_random_gen);
		for (int i=0; i<count; i++)
		{
			Bird bird;
			bird.set_id(INVALID_ID);
			bird.set_type(bird_type);
			bird.set_path_type(bird_config.path_type);
			bird.set_path_offset(Point(0,0));
			bird.set_path_delay(0);
			bird.set_speed(bird_config.speed);

			//����Ч��
// 			if (bird_type > 18)
// 			{
// 
// 				if (bird_type >20 && bird_type < 24)
// 					bird.set_item(rand()%2?BIRD_ITEM_SPECIAL_TOR:BIRD_ITEM_SPECIAL_EEL);
// 				else if (bird_type == 24)
// 					bird.set_item(BIRD_ITEM_SPECIAL_DING);
// 				else
// 					bird.set_item(rand()%(BIRD_ITEM_SPECIAL_BOMB_3) + 1);
// 			}
// 			else
				bird.set_item(INVALID_ID);

			//·������
			if (bird_config.path_type == PATH_TYPE_SMALL)
				bird.set_path_id(small_path[rand()%MAX_SMALL_PATH]);
			else if(bird_config.path_type == PATH_TYPE_BIG)
				bird.set_path_id(big_path[rand()%MAX_BIG_PATH]);
			else
				bird.set_path_id(huge_path[rand()%MAX_HUGE_PATH]);

			birds_.push_back(bird);
		}
	}
	else
	{
		//һ�����
		if (bird_type >= BIRD_TYPE_ONE && bird_type <= BIRD_TYPE_FIVE)
		{
			BirdSameConfig bird_config = g_game_config_xml.bird_same_config_[bird_type - BIRD_TYPE_ONE];
			//int count = Random(bird_config.distribute_count_min, bird_config.distribute_count_max);
			std::uniform_int_distribution<> dis(bird_config.distribute_count_min, bird_config.distribute_count_max);
			int count = dis(m_random_gen);
			for (int i = 0; i < count; i++)
			{
				int type = rand()%bird_config.bird_type_max;
				Bird bird;
				bird.set_id(INVALID_ID);
				bird.set_type(bird_type);
				bird.set_path_type(bird_config.path_type);
				bird.set_path_offset(Point(0,0));
				bird.set_path_delay(0);
				bird.set_speed(bird_config.speed);
				bird.set_item(type);		//������

				//·������
				if (bird_config.path_type == PATH_TYPE_SMALL)
					bird.set_path_id(small_path[rand()%MAX_SMALL_PATH]);
				else if (bird_config.path_type == PATH_TYPE_BIG)
					bird.set_path_id(big_path[rand()%MAX_BIG_PATH]);
				else
					bird.set_path_id(huge_path[rand()%MAX_HUGE_PATH]);

				birds_.push_back(bird);
			}
		}
		////������
		//else if (bird_type == BIRD_TYPE_CHAIN)
		//{
		//	BirdChainConfig bird_config = g_game_config_xml.bird_chain_config_;
		//	int count = Random(bird_config.distribute_count_min, bird_config.distribute_count_max);
		//	for (int i = 0; i < count; i++)
		//	{
		//		int type = rand()%bird_config.bird_type_max;
		//		Bird bird;
		//		bird.set_id(INVALID_ID);
		//		bird.set_type(bird_type);
		//		bird.set_path_type(bird_config.path_type);
		//		bird.set_path_offset(Point(0,0));
		//		bird.set_path_delay(0);
		//		bird.set_speed(g_game_config_xml.bird_config_[type].speed);
		//		bird.set_item(type);		//����������

		//		//·������
		//		if (bird_config.path_type == PATH_TYPE_SMALL)
		//			bird.set_path_id(small_path[rand()%MAX_SMALL_PATH]);
		//		else if (bird_config.path_type == PATH_TYPE_BIG)
		//			bird.set_path_id(big_path[rand()%MAX_BIG_PATH]);
		//		else
		//			bird.set_path_id(huge_path[rand()%MAX_HUGE_PATH]);
		//		birds_.push_back(bird);
		//	}
		//}
		////Ԫ����
		//else if (bird_type == BIRD_TYPE_INGOT)
		//{
		//	BirdIngotConfig bird_config = g_game_config_xml.bird_ingot_config_;
		//	int count = Random(bird_config.distribute_count_min, bird_config.distribute_count_max);
		//	for (int i = 0; i < count; i++)
		//	{
		//		int type = rand()%bird_config.bird_type_max;
		//		Bird bird;
		//		bird.set_id(INVALID_ID);
		//		bird.set_type(bird_type);
		//		bird.set_path_type(bird_config.path_type);
		//		bird.set_path_offset(Point(0,0));
		//		bird.set_path_delay(0);
		//		bird.set_speed(g_game_config_xml.bird_config_[type].speed);
		//		bird.set_item(type);

		//		if (bird_config.path_type == PATH_TYPE_SMALL)
		//			bird.set_path_id(small_path[rand()%MAX_SMALL_PATH]);
		//		else if (bird_config.path_type == PATH_TYPE_BIG)
		//			bird.set_path_id(big_path[rand()%MAX_BIG_PATH]);
		//		else
		//			bird.set_path_id(huge_path[rand()%MAX_HUGE_PATH]);

		//		birds_.push_back(bird);
		//	}
		//}
		
		if (bird_type == BIRD_TYPE_RED)
		{
			BirdRedSeriesConfig bird_config = g_game_config_xml.bird_red_series_config_;
			//int count = Random(bird_config.distribute_count_min, bird_config.distribute_count_max);
			std::uniform_int_distribution<> dis_(bird_config.distribute_count_min, bird_config.distribute_count_max);
			int count = dis_(m_random_gen);
			int type = rand()%bird_config.bird_type_max;
			uint16_t path_id = 0;
			float path_delay = 5;
			if (bird_config.path_type == PATH_TYPE_SMALL)
				path_id = small_path[rand()%MAX_SMALL_PATH];
			if (bird_config.path_type == PATH_TYPE_BIG)
				path_id = big_path[rand()%MAX_BIG_PATH];
			else
				path_id = huge_path[rand()%MAX_HUGE_PATH];

			Point pt;
			//pt.x_ = Random(30, 70);
			//pt.y_ = Random(30, 70);
			std::uniform_int_distribution<> dis(30, 70);
			pt.x_ = dis(m_random_gen);
			pt.y_ = dis(m_random_gen);
			for (int i = 0; i < count; i++)
			{
				Bird bird;
				bird.set_id(INVALID_ID);
				bird.set_path_id(path_id);
				bird.set_path_type(bird_config.path_type);
				bird.set_speed(bird_config.speed);

				bird.set_type(type);
				bird.set_item(INVALID_ID);
				
				bird.set_path_offset(pt);
				bird.set_path_delay(path_delay * i);	//�ӳ�ʱ��

				birds_.push_back(bird);
			}
		}
	}
}

void Bird_Factory::create_scene_round()
{
	float round_time = 40.f;
	uint8_t bird_type;
	const float kBirdSpeed = 150.f;
	Action_Bird_Move* action = NULL;
	const Point center(kScreenWidth / 2.f, kScreenHeight / 2.f);
	float radius = 0.f;
	float cell_radian;
	float angle;
	float radius_max = 270.f;

	//��������
	uint8_t bird_big_type[10] = {BIRD_TYPE_16, BIRD_TYPE_17, BIRD_TYPE_18, BIRD_TYPE_19, BIRD_TYPE_20, BIRD_TYPE_21, BIRD_TYPE_22, BIRD_TYPE_23, BIRD_TYPE_24, BIRD_TYPE_25};

	Bird bird;

	bird.set_path_delay(0);
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_ROUND);
	bird.set_start_pt(center);
	bird.set_speed(kBirdSpeed);


	bird_type = BIRD_TYPE_1;
	radius = radius_max;		//270
	cell_radian = 2 * M_PI / 34;
	for (int i = 0; i< 34; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center, radius, round_time - 4, angle, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, kBirdSpeed);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 4);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_2;
	radius = radius_max - 40.f;			//230
	cell_radian = 2 * M_PI / 33;
	for (int i = 0; i< 33; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center, radius, round_time - 3, angle, 4 * M_PI /*- M_PI_2 * 2*/, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 3);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_3;
	radius = radius_max - 40.f - 45.f;			//198
	cell_radian = 2 * M_PI / 20;
	for (int i = 0; i < 20; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center, radius, round_time - 2, angle, 4 * M_PI, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 2);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_4;
	radius = radius_max - 40.f - 45.f - 45.f;
	cell_radian = 2 * M_PI / 11;
	for (int i = 0; i< 11; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center, radius, round_time - 1, angle, 4 * M_PI, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 1);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	radius = 0.f;
	bird_type = bird_big_type[rand() % sizeof(bird_big_type)];
 	action = new Action_Scene_Round_Move(center, 0, round_time, 0, 4 * M_PI + M_PI_2, 5.f, kBirdSpeed);
 	bird.set_item(rand()%(BIRD_ITEM_SPECIAL_BOMB_3) + 1);
 	bird.set_type(bird_type);
 	bird.set_position(action->start_position());
	bird.set_round_radius(radius);
	bird.set_round_rotate_duration(round_time);
	bird.set_round_start_angle(0);
	bird.set_round_rotate_angle(4 * M_PI + M_PI_2);
	bird.set_round_move_duration(5.f);
 	m_pTableFrameSink->build_scene_bird(bird, action);

	m_pTableFrameSink->send_bird_round();
}

void Bird_Factory::create_scene_bloating()
{
	m_pTableFrameSink->send_scene_bloating();
}

void Bird_Factory::create_scene_double_round()
{
	//��������
	float round_time = 40.f;
	uint8_t bird_type;
	const float kBirdSpeed = 150;
	Action_Bird_Move* action = NULL;
	const Point center_left(kScreenWidth/4.f + 40, kScreenHeight/2.f);		//���
	const Point center_right(kScreenWidth/4.f * 3 - 40, kScreenHeight/2.f);	//�ұ�
	float radius = 0.f;		//Բ���뾶
	float cell_radian;		//�ֲ����
	float angle;			//λ�ýǶ�
	float radius_max = 270.f;

	//��������
	uint8_t bird_big_type[10] = {BIRD_TYPE_16, BIRD_TYPE_17, BIRD_TYPE_18, BIRD_TYPE_19, BIRD_TYPE_20, BIRD_TYPE_21, BIRD_TYPE_22, BIRD_TYPE_23, BIRD_TYPE_24, BIRD_TYPE_25};

	Bird bird;
	bird.set_path_delay(0);
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_ROUND);
	bird.set_speed(kBirdSpeed);

	//�������
	bird.set_start_pt(center_left);
	bird_type = BIRD_TYPE_1;
	radius = radius_max;
	cell_radian = 2 * M_PI / 28;
	for (int i = 0; i< 28; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_left, radius, round_time - 4, angle, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, kBirdSpeed);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 4);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_2;
	radius = radius_max - 40.f;
	cell_radian = 2 * M_PI / 30;
	for (int i = 0; i< 30; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_left, radius, round_time - 3, angle, 4 * M_PI /*- M_PI_2 * 2*/, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 3);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_3;
	radius = radius_max - 40.f - 45.f;
	cell_radian = 2 * M_PI / 18;
	for (int i = 0; i < 18; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_left, radius, round_time - 2, angle, 4 * M_PI, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 2);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_4;
	radius = radius_max - 40.f - 45.f - 45.f;
	cell_radian = 2 * M_PI / 10;
	for (int i = 0; i< 10; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_left, radius, round_time - 1, angle, 4 * M_PI, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 1);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	radius = 0.f;
	bird_type = bird_big_type[rand()%sizeof(bird_big_type)];
	action = new Action_Scene_Round_Move(center_left, 0, round_time, 0, 4 * M_PI + M_PI_2, 5.f, kBirdSpeed);
	bird.set_item(rand()%(BIRD_ITEM_SPECIAL_BOMB_3) + 1);
	bird.set_type(bird_type);
	bird.set_position(action->start_position());
	bird.set_round_radius(radius);
	bird.set_round_rotate_duration(round_time);
	bird.set_round_start_angle(0);
	bird.set_round_rotate_angle(4 * M_PI + M_PI_2);
	bird.set_round_move_duration(5.f);
	m_pTableFrameSink->build_scene_bird(bird, action);

	m_pTableFrameSink->send_bird_round();


	//�ұ�����
	bird.set_start_pt(center_right);
	bird_type = BIRD_TYPE_1;
	radius = radius_max;
	cell_radian = 2 * M_PI / 28;
	for (int i = 0; i< 28; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_right, radius, round_time - 4, angle, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, kBirdSpeed);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 4);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_2;
	radius = radius_max - 40.f;
	cell_radian = 2 * M_PI / 30;
	for (int i = 0; i< 30; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_right, radius, round_time - 3, angle, 4 * M_PI /*- M_PI_2 * 2*/, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 3);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_3;
	radius = radius_max - 40.f - 45.f;
	cell_radian = 2 * M_PI / 18;
	for (int i = 0; i < 18; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_right, radius, round_time - 2, angle, 4 * M_PI, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 2);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird_type = BIRD_TYPE_4;
	radius = radius_max - 40.f - 45.f - 45.f;
	cell_radian = 2 * M_PI / 10;
	for (int i = 0; i< 10; i++)
	{
		angle = i * cell_radian;
		action = new Action_Scene_Round_Move(center_right, radius, round_time - 1, angle, 4 * M_PI, 5.f, kBirdSpeed);
		bird.set_id(INVALID_ID);
		bird.set_item(INVALID_ID);
		bird.set_type(bird_type);
		bird.set_position(action->start_position());
		bird.set_round_radius(radius);
		bird.set_round_rotate_duration(round_time - 1);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	radius = 0.f;
	bird_type = bird_big_type[rand()%sizeof(bird_big_type)];
	action = new Action_Scene_Round_Move(center_right, 0, round_time, 0, 4 * M_PI + M_PI_2, 5.f, kBirdSpeed);
	bird.set_item(rand()%(BIRD_ITEM_SPECIAL_BOMB_3) + 1);
	bird.set_type(bird_type);
	bird.set_position(action->start_position());
	bird.set_round_radius(radius);
	bird.set_round_rotate_duration(round_time);
	bird.set_round_start_angle(0);
	bird.set_round_rotate_angle(4 * M_PI + M_PI_2);
	bird.set_round_move_duration(5.f);
	m_pTableFrameSink->build_scene_bird(bird, action);

	m_pTableFrameSink->send_bird_round();
}

void Bird_Factory::create_scene_double_bloating()
{
	m_pTableFrameSink->send_scene_double_bloating();
}

void Bird_Factory::create_scene_array_one(bool is_left)
{
	//��������
	float cell_radian = 0.f;				//�Ƕȼ��
	float speed = 60.f;						//�ٶ�
	float angle = 0.f;						//�Ƕ�λ��
	float radius = 270.f;					//��󻷰뾶
	float broadwise_end = 0.f;				//�����յ�λ��				
	Point pt_start(0.f, 0.f);				//��ʼλ��
	Point pt_end(0.f, 0.f);					//����λ��
	Point center(0.f, kScreenHeight / 2.f);	//������ĵ�
	Point center_temp(0.f, 0.f);
	Action_Bird_Move* action = NULL;
	
	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_LINEAR);
	bird.set_speed(speed);

	//���÷���
	if (is_left)		//������
	{
		broadwise_end = -radius * 2;
		center.x_ = kScreenWidth + radius;
	}
	else			//������
	{
		broadwise_end = kScreenWidth + radius * 2;
		center.x_ = -radius;
	}

	//���
	bird.set_type(BIRD_TYPE_0);
	cell_radian = 2 * M_PI / 70;
	for (int i = 0; i < 70; i++)
	{
		angle = i * cell_radian;
		pt_start.x_ = center.x_ + radius * std::cos(angle);
		pt_start.y_ = center.y_ + radius * std::sin(angle);
		pt_end.x_ = broadwise_end;
		pt_end.y_ = pt_start.y_;
		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(pt_start);
		bird.set_linear_end_pt(pt_end);

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	//������
	bird.set_type(BIRD_TYPE_2);
	cell_radian = 2 * M_PI / 25;
	radius = radius / 3;
	center_temp.x_ = center.x_ - radius;
	center_temp.y_ = center.y_ - radius;
	for (int i = 0; i < 25; i++)
	{
		angle = i * cell_radian;
		pt_start.x_ = center_temp.x_ + radius * std::cos(angle);
		pt_start.y_ = center_temp.y_ + radius * std::sin(angle);
		pt_end.x_ = broadwise_end;
		pt_end.y_ = pt_start.y_;
		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(pt_start);
		bird.set_linear_end_pt(pt_end);

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

 	//������
 	bird.set_type(BIRD_TYPE_4);
 	cell_radian = 2 * M_PI / 25;
 	center_temp.x_ = center.x_ + radius;
 	center_temp.y_ = center.y_ - radius;
 	for (int i = 0; i < 25; i++)
 	{
 		angle = i * cell_radian;
 		pt_start.x_ = center_temp.x_ + radius * std::cos(angle);
 		pt_start.y_ = center_temp.y_ + radius * std::sin(angle);
 		pt_end.x_ = broadwise_end;
 		pt_end.y_ = pt_start.y_;
 		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_start_pt(pt_start);
 		bird.set_linear_end_pt(pt_end);
 
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}
 
 	//������
 	bird.set_type(BIRD_TYPE_1);
 	cell_radian = 2 * M_PI / 25;
 	center_temp.x_ = center.x_ - radius;
 	center_temp.y_ = center.y_ + radius;
 	for (int i = 0; i < 25; i++)
 	{
 		angle = i * cell_radian;
 		pt_start.x_ = center_temp.x_ + radius * std::cos(angle);
 		pt_start.y_ = center_temp.y_ + radius * std::sin(angle);
 		pt_end.x_ = broadwise_end;
 		pt_end.y_ = pt_start.y_;
 		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_start_pt(pt_start);
 		bird.set_linear_end_pt(pt_end);
 
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}
 
 	//������
 	bird.set_type(BIRD_TYPE_3);
 	cell_radian = 2 * M_PI / 25;
 	center_temp.x_ = center.x_ + radius;
 	center_temp.y_ = center.y_ + radius;
 	for (int i = 0; i < 25; i++)
 	{
 		angle = i * cell_radian;
 		pt_start.x_ = center_temp.x_ + radius * std::cos(angle);
 		pt_start.y_ = center_temp.y_ + radius * std::sin(angle);
 		pt_end.x_ = broadwise_end;
 		pt_end.y_ = pt_start.y_;
 		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_start_pt(pt_start);
 		bird.set_linear_end_pt(pt_end);
 
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}

	//�����Ļ�
	bird.set_type(BIRD_TYPE_5);
	cell_radian = 2 * M_PI / 25;
	radius = radius / 7 * 5;
	for (int i = 0; i < 25; i++)
	{
		angle = i * cell_radian;
		pt_start.x_ = center.x_ + radius * std::cos(angle);
		pt_start.y_ = center.y_ + radius * std::sin(angle);
		pt_end.x_ = broadwise_end;
		pt_end.y_ = pt_start.y_;
		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(pt_start);
		bird.set_linear_end_pt(pt_end);

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	//������
	bird.set_type(BIRD_TYPE_19);
	pt_start.x_ = center.x_;
	pt_start.y_ = center.y_;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);

	m_pTableFrameSink->send_bird_linear();
}

void Bird_Factory::create_scene_array_two(bool is_left /* = true */)
{
	//��������
	float cell_radian = 0.f;				//�Ƕȼ��
	float speed = 50.f;						//�ٶ�
	float angle = 0.f;						//�Ƕ�λ��
	float radius = 270.f;					//�뾶
	float broadwise_end = 0.f;				//�����յ�λ��
	Point pt_start(0.f, 0.f);				//��ʼλ��
	Point pt_end(0.f, 0.f);					//����λ��
	Point center(0.f, kScreenHeight / 2.f);	//���ĵ�
	Point center_temp(0.f, 0.f);

	//����
	//int nRed = 0;

	Action_Bird_Move* action = NULL;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_LINEAR);
	bird.set_speed(speed);

	//���÷���
	if(is_left)
	{
		broadwise_end = -radius * 2;
		center.x_ = kScreenWidth + radius;
	}
	else
	{
		broadwise_end = kScreenWidth + radius * 2;
		center.x_ = -radius;
	}

 	//��
	//nRed = rand()%40;
 	cell_radian = 2 * M_PI / 40;
 	center_temp.x_ = center.x_ - radius;
 	center_temp.y_ = center.y_;
 	for (int i = 0; i< 40; i++)
 	{
 		angle = i * cell_radian;
 		pt_start.x_ = center_temp.x_ + radius * std::cos(angle);
 		pt_start.y_ = center_temp.y_ + radius * std::sin(angle);
 		pt_end.x_ = broadwise_end;
 		pt_end.y_ = pt_start.y_;
 		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_start_pt(pt_start);
 		bird.set_linear_end_pt(pt_end);
		bird.set_type(BIRD_TYPE_1);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/
 
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}
 
  	//�һ�
	//nRed = rand()%40;
  	cell_radian = 2 * M_PI / 40;
  	center_temp.x_ = center.x_ + radius;
  	center_temp.y_ = center.y_;
  	for (int i = 0; i< 40; i++)
  	{
  		angle = i * cell_radian;
  		pt_start.x_ = center_temp.x_ + radius * std::cos(angle);
  		pt_start.y_ = center_temp.y_ + radius * std::sin(angle);
  		pt_end.x_ = broadwise_end;
  		pt_end.y_ = pt_start.y_;
  		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
  		bird.set_position(action->start_position());
  		bird.set_path_delay(0);
  		bird.set_start_pt(pt_start);
  		bird.set_linear_end_pt(pt_end);
		bird.set_type(BIRD_TYPE_1);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/
  
  		m_pTableFrameSink->build_scene_bird(bird, action);
  	}

	//���Ұ뻷
	//nRed = rand()%40;
	cell_radian = 2 * M_PI / 40;
	center_temp.y_ = center.y_;
	for (int i = 0; i< 40; i++)
	{
		if ( i > 40 / 4 && i < 40 / 4 * 3 )
			center_temp.x_ = center.x_ - radius / 2 - 20;
		else
			center_temp.x_ = center.x_ + radius / 2 + 20;

		angle = i * cell_radian;
		pt_start.x_ = center_temp.x_ + (radius / 9 * 8) * std::cos(angle);
		pt_start.y_ = center_temp.y_ + (radius / 9 * 8) * std::sin(angle);
		pt_end.x_ = broadwise_end;
		pt_end.y_ = pt_start.y_;
		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(pt_start);
		bird.set_linear_end_pt(pt_end);
		bird.set_type(BIRD_TYPE_7);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/

		m_pTableFrameSink->build_scene_bird(bird, action);
	}
  
  	//�м价
	//nRed = rand()%40;
  	cell_radian = 2 * M_PI / 40;
  	center_temp.x_ = center.x_;
  	center_temp.y_ = center.y_;
  	for (int i = 0; i< 40; i++)
  	{
  		angle = i * cell_radian;
  		pt_start.x_ = center.x_ + radius * std::cos(angle);
  		pt_start.y_ = center.y_ + radius * std::sin(angle);
  		pt_end.x_ = broadwise_end;
  		pt_end.y_ = pt_start.y_;
  		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
  		bird.set_position(action->start_position());
  		bird.set_path_delay(0);
  		bird.set_start_pt(pt_start);
  		bird.set_linear_end_pt(pt_end);
		bird.set_type(BIRD_TYPE_2);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/
  
  		m_pTableFrameSink->build_scene_bird(bird, action);
  	}

	//����
	//���
	bird.set_type(rand()%4 + BIRD_TYPE_16);
	//����
	pt_start.x_ = center.x_ - radius;
	pt_start.y_ = center.y_ - radius / 2;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);
	//����
	pt_start.x_ = center.x_ - radius;
	pt_start.y_ = center.y_ + radius / 2;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);

	//�ұ�
	bird.set_type(rand()%4 + BIRD_TYPE_12);
	//����
	pt_start.x_ = center.x_ + radius;
	pt_start.y_ = center.y_ - radius / 2;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);
	//����
	pt_start.x_ = center.x_ + radius;
	pt_start.y_ = center.y_ + radius / 2;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);

	//�м�
	bird.set_type(rand()%3 + BIRD_TYPE_9);
	//����
	pt_start.x_ = center.x_;
	pt_start.y_ = center.y_ - radius / 4 *3;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);
	//����
	pt_start.x_ = center.x_;
	pt_start.y_ = center.y_ + radius / 4 *3;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);

	m_pTableFrameSink->send_bird_linear();
}

void Bird_Factory:: create_scene_array_three(bool is_left /* = true */)
{
	//��������
	float cell_radian = 0.f;			//�Ƕȼ��
	float speed = 50.f;					//�ٶ�
	float angle = 0.f;					//�Ƕ�λ��
	float radius = 270.f;				//�뾶
	float broadwise_end = 0.f;			//�������λ��
	Point pt_start(0.f, 0.f);			//��ʼλ��
	Point pt_end(0.f, 0.f);				//����λ��
	Point center(0.f, kScreenHeight / 2.f);		//���ĵ�
	Action_Bird_Move* action = NULL;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_LINEAR);
	bird.set_speed(speed);

	//���÷���
	if (is_left)
	{
		broadwise_end = -radius * 2;
		center.x_ = kScreenWidth + radius;
	} 
	else
	{
		broadwise_end = kScreenWidth + radius * 2;
		center.x_ = -radius;
	}

	bird.set_type(rand()%2);
	cell_radian = 2 * M_PI / 70;
	for (int i = 0; i< 70; i++)
	{
		angle = i * cell_radian;
		pt_start.x_ = center.x_ + radius * std::cos(angle);
		pt_start.y_ = center.y_ + radius * std::sin(angle);
		pt_end.x_ = broadwise_end;
		pt_end.y_ = pt_start.y_;
		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(pt_start);
		bird.set_linear_end_pt(pt_end);

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird.set_type(rand()%2 + BIRD_TYPE_2);
	radius -= 80.f;
	cell_radian = 2 * M_PI / 50;
	for (int i = 0; i < 50; i++)
	{
		angle = i * cell_radian;
		pt_start.x_ = center.x_ + radius * std::cos(angle);
		pt_start.y_ = center.y_ + radius * std::sin(angle);
		pt_end.x_ = broadwise_end;
		pt_end.y_ = pt_start.y_;
		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(pt_start);
		bird.set_linear_end_pt(pt_end);

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	bird.set_type(rand()%2 + BIRD_TYPE_4);
	radius -= 80.f;
	cell_radian = 2 * M_PI / 30;
	for (int i = 0; i < 30; i++)
	{
		angle = i * cell_radian;
		pt_start.x_ = center.x_ + radius * std::cos(angle);
		pt_start.y_ = center.y_ + radius * std::sin(angle);
		pt_end.x_ = broadwise_end;
		pt_end.y_ = pt_start.y_;
		action = new Action_Bird_Move_Linear(bird.get_speed(), pt_start, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_start_pt(pt_start);
		bird.set_linear_end_pt(pt_end);

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	//������
	bird.set_type(rand()%5 + BIRD_TYPE_15);
	pt_start.x_ = center.x_;;
	pt_start.y_ = center.y_;
	pt_end.x_ = broadwise_end;
	pt_end.y_ = pt_start.y_;
	action = new Action_Bird_Move_Linear(bird.get_speed(), center, pt_end);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_start_pt(pt_start);
	bird.set_linear_end_pt(pt_end);
	m_pTableFrameSink->build_scene_bird(bird, action);

	m_pTableFrameSink->send_bird_linear();
}

void Bird_Factory::create_scene_array_four()
{
	//���¶���
	m_pTableFrameSink->send_scene_array_four();

	//�м����

	//�����յ�λ��
	Point pt_left(-500, kScreenHeight / 2.f);
	Point pt_right(kScreenWidth + 500, kScreenHeight / 2.f);

	Action_Bird_Move* action = NULL;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_LINEAR);
	bird.set_speed(40);

 	//���ٲ���
 	for (uint16_t i = 0; i < 4; i++)
 	{
 		if ( i < 2)
 		{
			if (i == 0)
 				bird.set_type(BIRD_TYPE_21);
			else
				bird.set_type(BIRD_TYPE_22);
 			action = new Action_Bird_Move_Linear(bird.get_speed(), pt_left, pt_right);
 			bird.set_position(action->start_position());
 			bird.set_path_delay(i%2*10 + 8);
 			bird.set_start_pt(pt_left);
			bird.set_linear_end_pt(pt_right);
 		}
 		else
 		{
			if ( i == 2)
				bird.set_type(BIRD_TYPE_23);
			else
				bird.set_type(BIRD_TYPE_24);
			action = new Action_Bird_Move_Linear(bird.get_speed(), pt_right, pt_left);
			bird.set_position(action->start_position());
			bird.set_path_delay(i%2*8 + 8);
			bird.set_start_pt(pt_right);
			bird.set_linear_end_pt(pt_left);
 		}

		m_pTableFrameSink->build_scene_bird(bird, action);
 	}

	m_pTableFrameSink->send_bird_linear();
}

void Bird_Factory::create_scene_array_five()
{
	//��������
	float cell_interval = 0.f;			//��� 
	float speed = 80.f;					//�ٶ�
	float time_pause = 25.f;					//ͣ��ʱ��
	float vertical_end = 0.f;			//�������λ��
	Point pt_start(0.f, 0.f);			//��ʼλ��
	Point pt_pause(0.f, 0.f);			//ͣ��λ��
	Point pt_end(0.f, 0.f);				//����λ��
	int bird_count = 40;
	uint8_t type = rand()%4;

	//����
	//int nRed = 0;

	Action_Bird_Move* action = NULL;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_PAUSE_LINEAR);
	bird.set_speed(speed);

	cell_interval = kScreenWidth / bird_count;

	//����
//	nRed = rand()%bird_count;
	vertical_end = kScreenHeight + 300; 
	for (int i = 0; i < bird_count; i++)
	{
		pt_start.x_ = i * cell_interval + 25;
		pt_start.y_ = 0.f - 300;
		pt_end.x_ = pt_start.x_;
		pt_end.y_ = vertical_end;
		pt_pause.x_ = pt_start.x_;
		pt_pause.y_ = 120;
		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end, 0);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);
		bird.set_type(type);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	//����
//	nRed = rand()%bird_count;
	vertical_end = 0.f;
	for (int i = 0; i < bird_count; i++)
	{
		pt_start.x_ = i * cell_interval + 25;
		pt_start.y_ = kScreenHeight + 300;
		pt_end.x_ = pt_start.x_;
		pt_end.y_ = vertical_end;
		pt_pause.x_ = pt_start.x_;
		pt_pause.y_ = kScreenHeight - 120;
		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);
		bird.set_type(type + 1);
		/*if (i == nRed)
		{
			bird.set_item(bird.get_type());
			bird.set_type(BIRD_TYPE_RED);
		}*/

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	m_pTableFrameSink->send_bird_pause_linear();

	//�м����

	//�����յ�λ��
	Point pt_left(0 - 350, kScreenHeight / 2.f);
	Point pt_right(kScreenWidth + 350, kScreenHeight / 2.f);

	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_LINEAR);
	bird.set_speed(60);

	//�ĺ�����
	for (uint16_t i = 0; i < 4; i++)
	{
		if ( i < 2)
		{
			if (i == 0)
				bird.set_type(BIRD_TYPE_27);
			else
				bird.set_type(BIRD_TYPE_24);
			action = new Action_Bird_Move_Linear(bird.get_speed(), pt_left, pt_right);
			bird.set_position(action->start_position());
			bird.set_path_delay(i%2*10);
			bird.set_start_pt(pt_left);
			bird.set_linear_end_pt(pt_right);
		}
		else
		{
			if ( i == 2)
				bird.set_type(BIRD_TYPE_26);
			else
				bird.set_type(BIRD_TYPE_25);
			action = new Action_Bird_Move_Linear(bird.get_speed(), pt_right, pt_left);
			bird.set_position(action->start_position());
			bird.set_path_delay(i%2*10);
			bird.set_start_pt(pt_right);
			bird.set_linear_end_pt(pt_left);
		}

		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	m_pTableFrameSink->send_bird_linear();
}
 
void Bird_Factory::create_scene_array_six()
{
	float cell_interval = 0.f;		//���
	float speed = 80.f;				//�ٶ�
	float time_pause = 30.f;		//ͣ��ʱ��
	Point pt_start(0.f,0.f);		//��ʼλ��
	Point pt_pause(0.f,0.f);		//ͣ��λ��
	Point pt_end(0.f,0.f);			//����λ��
	int bird_count = 60;			//����
	float width_air = 120.f;			//������������
	float height_air = 120.f;		//������������
	float external_dis = 300;		//�ⲿ����
	uint8_t type = rand()%3 + 2;	//����

	//����λ��
//	int red_position = rand()%4;
	//int nRed = rand()%bird_count;

	Action_Bird_Move* action = NULL;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_PAUSE_LINEAR);
	bird.set_speed(speed);

	cell_interval = (kScreenWidth - width_air * 2) / bird_count;

	//��
	for (int i = 0; i < bird_count; i++)
	{
		pt_start.x_ = i * cell_interval + width_air;
		pt_start.y_ = 0.f - external_dis;
		pt_end = pt_start;
		pt_pause.x_ = pt_start.x_;
		pt_pause.y_ = height_air;
		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);
		bird.set_type(type);
		/*if (red_position == 0)
		{
			if (i == nRed)
			{
				bird.set_item(bird.get_type());
				bird.set_type(BIRD_TYPE_RED);
			}
		}*/
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	//��
	for (int i = 0; i < bird_count; i++)
	{
		pt_start.x_ = i * cell_interval + width_air;
		pt_start.y_ = kScreenHeight + external_dis;
		pt_end = pt_start;
		pt_pause.x_ = pt_start.x_;
		pt_pause.y_ = kScreenHeight - height_air;
		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);
		bird.set_type(type);
		/*if (red_position == 1)
		{
			if (i == nRed)
			{
				bird.set_item(bird.get_type());
				bird.set_type(BIRD_TYPE_RED);
			}
		}*/
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

	m_pTableFrameSink->send_bird_pause_linear();

	//����
	bird_count = 30;
	cell_interval = (kScreenHeight - height_air * 2 - 40)/ bird_count;
//	nRed = rand()%bird_count;
	time_pause -= 10.0f;
	//��
	for (int i = 0; i < bird_count; i++)
	{
		pt_start.x_ = 0 - ( external_dis + 180 + 148);
		pt_start.y_ = i * cell_interval + height_air + 25;
		pt_end = pt_start;
		pt_pause.x_ = width_air;
		pt_pause.y_ = pt_start.y_;
		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);
		bird.set_type(type);
		/*if (red_position == 2)
		{
			if (i == nRed)
			{
				bird.set_item(bird.get_type());
				bird.set_type(BIRD_TYPE_RED);
			}
		}*/
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

 	//��
 	for (int i = 0; i < bird_count; i++)
 	{
 		pt_start.x_ = kScreenWidth + ( external_dis + 180 + 148);
 		pt_start.y_ = i * cell_interval + height_air+ 25;
 		pt_end = pt_start;
 		pt_pause.x_ = kScreenWidth - width_air;
 		pt_pause.y_ = pt_start.y_;
 		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_pause_time(time_pause);
 		bird.set_start_pt(pt_start);
 		bird.set_pause_pt(pt_pause);
 		bird.set_linear_end_pt(pt_end);
 		bird.set_type(type);
 		/*if (red_position == 3)
 		{
 			if (i == nRed)
 			{
 				bird.set_item(bird.get_type());
 				bird.set_type(BIRD_TYPE_RED);
 			}
 		}*/
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}

	m_pTableFrameSink->send_bird_pause_linear();

	time_pause -= 4.0f;
 	//�м����
 	type = rand() % 5 + BIRD_TYPE_11;
 	uint8_t type_max = rand() % 3 + BIRD_TYPE_16;
	//external_dis + 150;
	for (int i = 0; i < 6; i++)
	{
		if (i < 3)
		{
			if (i == 0)
				pt_start.y_ = kScreenHeight / 3;
			else
				pt_start.y_ = kScreenHeight / 3 * 2;

			pt_start.x_ = 0 - ( external_dis + 148);
			pt_end.x_ = kScreenWidth + ( external_dis + 148);
			pt_end.y_ = pt_pause.y_ = pt_start.y_;
			pt_pause.x_ = kScreenWidth/2 - 300;
			bird.set_type(type);

			if ( i == 2)
			{
				pt_pause.x_ += 180;
				pt_start.x_ += 180;
				pt_start.y_ = pt_end.y_ = pt_pause.y_ = kScreenHeight / 2;
				bird.set_type(type_max);
			}
		}
		else
		{
			if (i == 3)
				pt_start.y_ = kScreenHeight / 3;
			else
				pt_start.y_ = kScreenHeight / 3 * 2;

			pt_start.x_ = kScreenWidth + ( external_dis + 148 );
			pt_end.x_ = 0 - ( external_dis + 148 );
			pt_end.y_ = pt_pause.y_ = pt_start.y_;
			pt_pause.x_ = kScreenWidth/2 + 300;
			bird.set_type(type);

			if ( i == 5)
			{
				pt_pause.x_ -= 180;
				pt_start.x_ -= 180;
				pt_start.y_ = pt_end.y_ = pt_pause.y_ = kScreenHeight / 2;
				bird.set_type(type_max);
			}
		}
 
 		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_pause_time(time_pause);
 		bird.set_start_pt(pt_start);
 		bird.set_pause_pt(pt_pause);
 		bird.set_linear_end_pt(pt_end);
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}

	m_pTableFrameSink->send_bird_pause_linear();
}

void Bird_Factory::create_scene_array_seven()
{
	float cell_radian = 0.f;						//�Ƕȼ��
	float speed = 70.f;								//�ٶ�
	float angle = 0.f;								//�Ƕ�λ��
	float radius_big = kScreenHeight / 2 + 200;		//��뾶
	float radius_small = kScreenHeight / 3 + 15;	//С�뾶
	float time_pause = 30.f;
	Point center(0.f, kScreenHeight / 2);			//���ĵ�
	Point pt_start(0.f,0.f);						//��ʼ��
	Point pt_pause(0.f,0.f);						//ͣ�ٵ�
	Point pt_end(0.f,0.f);							//�����㣨�γ��㣩
	uint8_t type = rand()%3;						//����
	int bird_count = 60;

	//����λ��
//	int red_position = rand()%4;
//	int nRed = rand()%bird_count;

	Action_Bird_Move* action = NULL;

	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_PAUSE_LINEAR);
	bird.set_speed(speed);

	cell_radian = 2 * M_PI / bird_count;
	bird.set_type(type);
	for (int i = 0; i  < bird_count; i++)
	{
		angle = i * cell_radian;
		if ( i >= bird_count / 4 && i <= bird_count / 4 * 3 )
		{
			center.x_ = kScreenWidth / 9 * 2;
			pt_start.x_ = center.x_ + radius_small * std::cos(angle);
			pt_start.y_ = center.y_ + radius_small * std::sin(angle);
			bird.set_round_start_angle(0);
		}
		else
		{
			center.x_ = kScreenWidth / 9 * 7;
			pt_start.x_ = center.x_ + radius_small * std::cos(angle);
			pt_start.y_ = center.y_ + radius_small * std::sin(angle);
			bird.set_round_start_angle(M_PI);
		}
		pt_pause.x_ = center.x_ + radius_small * std::cos(angle);
		pt_pause.y_ = center.y_ + radius_small * std::sin(angle);
		pt_end.x_ = center.x_ + radius_big * std::cos(angle);
		pt_end.y_ = center.y_ + radius_big * std::sin(angle);
		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end, bird.get_round_start_angle());
		bird.set_position(action->start_position());
        bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);
		m_pTableFrameSink->build_scene_bird(bird, action);
	}

 	float cell_interval = 0.f;		//���
 	float width_air = 300.f;		//������������
 	float height_air = 120.f;		//������������
 	float external_dis = 300;		//�ⲿ����
	bird_count = 30;
 
 	cell_interval = (kScreenWidth - width_air * 2) / bird_count;
 
 	//��
 	for (int i = 0; i < bird_count; i++)
 	{
 		pt_start.x_ = i * cell_interval + width_air + 20;
 		pt_start.y_ = height_air;
 		pt_end.x_ = pt_start.x_;
		pt_end.y_ = 0.f - external_dis;
 		pt_pause.x_ = pt_start.x_;
 		pt_pause.y_ = height_air;
 		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end, 0);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_pause_time(time_pause);
 		bird.set_start_pt(pt_start);
 		bird.set_pause_pt(pt_pause);
 		bird.set_linear_end_pt(pt_end);
 		bird.set_type(type);
		bird.set_round_start_angle(0);
 		/*if (red_position == 0)
 		{
 			if (i == nRed)
 			{
 				bird.set_item(bird.get_type());
 				bird.set_type(BIRD_TYPE_RED);
 			}
 		}*/
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}
 
 	//��
 	for (int i = 0; i < bird_count; i++)
 	{
 		pt_start.x_ = i * cell_interval + width_air + 20;
 		pt_start.y_ = kScreenHeight - height_air;
 		pt_end.x_ = pt_start.x_;
		pt_end.y_ = kScreenHeight + external_dis;
 		pt_pause.x_ = pt_start.x_;
 		pt_pause.y_ = kScreenHeight - height_air;
 		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end, M_PI);
 		bird.set_position(action->start_position());
 		bird.set_path_delay(0);
 		bird.set_pause_time(time_pause);
 		bird.set_start_pt(pt_start);
 		bird.set_pause_pt(pt_pause);
 		bird.set_linear_end_pt(pt_end);
		bird.set_round_start_angle(M_PI);
 		bird.set_type(type);
 		/*if (red_position == 1)
 		{
 			if (i == nRed)
 			{
 				bird.set_item(bird.get_type());
 				bird.set_type(BIRD_TYPE_RED);
 			}
 		}*/
 		m_pTableFrameSink->build_scene_bird(bird, action);
 	}

	m_pTableFrameSink->send_bird_pause_linear();

	//�ڲ�СԲȦ
	float round_time = time_pause + 4;		//��תʱ��
	float rount_radius = 190;				//�뾶
	type = rand()%3 + BIRD_TYPE_3;
	bird_count = 25;
	cell_radian = 2 * M_PI / bird_count;
	for (int i = 0; i < bird_count; i++)
	{
		angle = i * cell_radian;

		center.x_ = kScreenWidth / 4 - 55;
		center.y_ = kScreenHeight / 2;
		action = new Action_Scene_Round_Move(center, rount_radius, round_time - 4, angle, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, speed);
		bird.set_start_pt(center);
		bird.set_type(type);
		bird.set_position(action->start_position());
		bird.set_round_radius(rount_radius);
		bird.set_round_rotate_duration(round_time - 4);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);

		if (i == 0)
		{
			action = new Action_Scene_Round_Move(center, 0, round_time - 4, 0, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, speed);
			bird.set_start_pt(center);
			bird.set_type(BIRD_TYPE_16);
			bird.set_position(action->start_position());
			bird.set_round_radius(0);
			bird.set_round_rotate_duration(round_time - 4);
			bird.set_round_start_angle(0);
			bird.set_round_rotate_angle(-4 * M_PI);
			bird.set_round_move_duration(5.f);
			m_pTableFrameSink->build_scene_bird(bird, action);
		}

		center.x_ = kScreenWidth / 2;
		center.y_ = kScreenHeight / 2;
		action = new Action_Scene_Round_Move(center, rount_radius, round_time - 4, angle, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, speed);
		bird.set_start_pt(center);
		bird.set_type(type);
		bird.set_position(action->start_position());
		bird.set_round_radius(rount_radius);
		bird.set_round_rotate_duration(round_time - 4);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);

		if (i == 0)
		{
			action = new Action_Scene_Round_Move(center, 0, round_time - 4, 0, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, speed);
			bird.set_start_pt(center);
			bird.set_type(BIRD_TYPE_18);
			bird.set_position(action->start_position());
			bird.set_round_radius(0);
			bird.set_round_rotate_duration(round_time - 4);
			bird.set_round_start_angle(0);
			bird.set_round_rotate_angle(-4 * M_PI);
			bird.set_round_move_duration(5.f);
			m_pTableFrameSink->build_scene_bird(bird, action);
		}

		center.x_ = kScreenWidth / 4 * 3 + 55;
		center.y_ = kScreenHeight / 2;
		action = new Action_Scene_Round_Move(center, rount_radius, round_time - 4, angle, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, speed);
		bird.set_start_pt(center);
		bird.set_type(type);
		bird.set_position(action->start_position());
		bird.set_round_radius(rount_radius);
		bird.set_round_rotate_duration(round_time - 4);
		bird.set_round_start_angle(angle);
		bird.set_round_rotate_angle(4 * M_PI);
		bird.set_round_move_duration(5.f);
		m_pTableFrameSink->build_scene_bird(bird, action);

		if (i == 0)
		{
			action = new Action_Scene_Round_Move(center, 0, round_time - 4, 0, 4 * M_PI /*- M_PI_2 * 3*/, 5.f, speed);
			bird.set_start_pt(center);
			bird.set_type(BIRD_TYPE_17);
			bird.set_position(action->start_position());
			bird.set_round_radius(0);
			bird.set_round_rotate_duration(round_time - 4);
			bird.set_round_start_angle(0);
			bird.set_round_rotate_angle(-4 * M_PI);
			bird.set_round_move_duration(5.f);
			m_pTableFrameSink->build_scene_bird(bird, action);
		}
	}

	m_pTableFrameSink->send_bird_round();
}

void Bird_Factory::create_scene_boss_one()
{
	//��������
	//float cell_radian = 0.f;			//�Ƕȼ��
	float speed = 200.f;				//�ٶ�
	//float angle = 0.f;					//�Ƕ�λ��
//	float radius = 270.f;				//�뾶
//	float broadwise_end = 0.f;			//�������λ��
	Point pt_start(0.f, 0.f);			//��ʼλ��
	Point pt_end(0.f, 0.f);				//����λ��
	Point center(-300.f, -300.f);		//���ĵ�
	Action_Bird_Move* action = NULL;
	Point pt_pause(0.f,0.f);			//ͣ��λ��
	Bird bird;
	bird.set_id(INVALID_ID);
	bird.set_item(INVALID_ID);
	bird.set_yuzhen_type(YUZHEN_LINEAR);
	bird.set_speed(speed);
	float time_pause = 0.f;
//	float widthEnd = kScreenWidth -100.f, heightEnd = kScreenHeight + 100.f;
	bird.set_type(2);
	//cell_radian = 2 * M_PI / 70;
	int xincreas = 80, yincreas = 80;
	float intervel = 0.5;
	for (int i = 0; i < 120; i++)
	{
//		angle = i * cell_radian;
		if (i < 60)
		{
			pt_start.x_ = center.x_ - xincreas -30;
			pt_start.y_ = center.y_ + yincreas + 30;
			pt_end.x_ = kScreenWidth - xincreas -30;
			pt_end.y_ = kScreenHeight + yincreas+30;
			pt_pause.x_ = pt_start.x_;
			pt_pause.y_ = pt_start.y_;
			time_pause += intervel;
		}
		else
		{
			if (i == 60)
				time_pause = 0;
			pt_start.x_ = center.x_ + xincreas;
			pt_start.y_ = center.y_ - yincreas;
			pt_end.x_ = kScreenWidth + xincreas;
			pt_end.y_ = kScreenHeight - yincreas;
			pt_pause.x_ = pt_start.x_;
			pt_pause.y_ = pt_start.y_;
			time_pause += intervel;
		}

		time_pause = 0;

		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause,  pt_start, pt_pause, pt_end, 0.12);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);

		m_pTableFrameSink->build_bird_common(bird, action);
		if(i < 60)
		{
			add_special_birds_1(bird);
		}
		else
		{
			add_special_birds_2(bird);
		}
		//m_pTableFrameSink->build_scene_bird(bird, action);
	}
	
	xincreas = 120;
	yincreas = 120;
	intervel = 1.5;
	time_pause = 0;
	for (int i = 0; i < 40; i++)
	{
		if (i % 2 == 0)
		{
			bird.set_type(23);
		}
		else
		{
			bird.set_type(26);
		}
//		angle = i * cell_radian;
		if (i < 20)
		{
			pt_start.x_ = center.x_ - xincreas -30;
			pt_start.y_ = center.y_ + yincreas +30;
			pt_end.x_ = kScreenWidth - xincreas-30;
			pt_end.y_ = kScreenHeight + yincreas+30;
			pt_pause.x_ = pt_start.x_;
			pt_pause.y_ = pt_start.y_;
			time_pause += intervel;
		}
		else
		{
			if (i == 20)
				time_pause = 0;
			pt_start.x_ = center.x_ + xincreas;
			pt_start.y_ = center.y_ - yincreas;
			pt_end.x_ = kScreenWidth + xincreas;
			pt_end.y_ = kScreenHeight - yincreas;
			pt_pause.x_ = pt_start.x_;
			pt_pause.y_ = pt_start.y_;
			time_pause += intervel;
		}

		time_pause = 0;

		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end, 0.12);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);

		//m_pTableFrameSink->build_scene_bird(bird, action);
		m_pTableFrameSink->build_bird_common(bird, action);
		if(i < 20)
		{
			add_special_birds_3(bird);
		}
		else
		{
			add_special_birds_4(bird);
		}
	}

	bird.set_type(2);
	intervel = 0.5;
	xincreas = 200;
	yincreas = 200;
	time_pause = 0;
	for (int i = 0; i < 120; i++)
	{
//		angle = i * cell_radian;
		if (i < 60)
		{
			pt_start.x_ = center.x_ - xincreas -30;
			pt_start.y_ = center.y_ + yincreas + 30;
			pt_end.x_ = kScreenWidth - xincreas -30;
			pt_end.y_ = kScreenHeight + yincreas +30;
			pt_pause.x_ = pt_start.x_;
			pt_pause.y_ = pt_start.y_;
			time_pause += intervel;
		}
		else
		{
			if (i == 60)
				time_pause = 0;
			pt_start.x_ = center.x_ + xincreas;
			pt_start.y_ = center.y_ - yincreas;
			pt_end.x_ = kScreenWidth + xincreas;
			pt_end.y_ = kScreenHeight - yincreas;
			pt_pause.x_ = pt_start.x_;
			pt_pause.y_ = pt_start.y_;
			time_pause += intervel;
		}

		time_pause = 0;

		action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, pt_start, pt_pause, pt_end, 0.12);
		bird.set_position(action->start_position());
		bird.set_path_delay(0);
		bird.set_pause_time(time_pause);
		bird.set_start_pt(pt_start);
		bird.set_pause_pt(pt_pause);
		bird.set_linear_end_pt(pt_end);

		//m_pTableFrameSink->build_scene_bird(bird, action);
		m_pTableFrameSink->build_bird_common(bird, action);
		if(i < 60)
		{
			add_special_birds_5(bird);
		}
		else
		{
			add_special_birds_6(bird);
		}
	}

	//������
	bird.set_type(BOSS_FISH);
	speed = 80;
	pt_start.x_ = center.x_ + 80;
	pt_start.y_ = center.y_ - 80;
	pt_end.x_ = kScreenWidth + 80;
	pt_end.y_ = kScreenHeight - 80;
	pt_pause.x_ = (pt_start.x_ + pt_end.x_) / 2 +120 ;
	pt_pause.y_ = (pt_start.y_ + pt_end.y_) / 2 +120;
	time_pause = 10;
	action = new Action_Bird_Move_Pause_Linear(bird.get_speed(), time_pause, center,pt_pause, pt_end,0.12);
	bird.set_position(action->start_position());
	bird.set_path_delay(0);
	bird.set_speed(speed);
	bird.set_pause_time(time_pause);
	bird.set_start_pt(pt_start);
	bird.set_pause_pt(pt_pause);
	bird.set_linear_end_pt(pt_end);
    m_pTableFrameSink->build_bird_common(bird, action);
    add_special_birds_boss(bird);

	//m_pTableFrameSink->build_scene_bird(bird, action);
	//m_pTableFrameSink->send_bird_pause_linear();
}

bool Bird_Factory::get_bird(Bird *bird)
{
	if (birds_.empty())
	{
	    return false;
	}

	*bird = birds_.front();
	 birds_.pop_front();

	 return true;
}

bool Bird_Factory::get_small_clean_sweep_bird(Bird *bird)
{
	if (small_clean_sweep_bird_.empty())
	{
	    return false;
	}

	*bird = small_clean_sweep_bird_.front();
	 small_clean_sweep_bird_.pop_front();

	 return true;
}

bool Bird_Factory::get_shape_bird(Bird * bird)
{
	if (shape_birds_.empty())
	{
		return false;
	}

	*bird = shape_birds_.front();
	shape_birds_.pop_front();

	return true;
}

void Bird_Factory::add_special_birds(Bird bird)
{
	special_birds_.push_back(bird);
}

bool Bird_Factory::get_special_bird(Bird * bird)
{
	if (special_birds_.empty())
	{
		return false;
	}
	
	//��ȡͷ������
	*bird = special_birds_.front();
	//ɾ��ͷ������
	special_birds_.pop_front();

	return true;
}

void Bird_Factory::add_special_birds_1(Bird &bird)
{
	bird.SetCreateTime(time(NULL));
	special_birds_1.push_back(bird);
}

bool Bird_Factory::get_special_bird_1(Bird * bird)
{
	if (special_birds_1.empty())
	{
		return false;
	}

	*bird = special_birds_1.front();
	special_birds_1.pop_front();

	return true;
}

void Bird_Factory::add_special_birds_2(Bird &bird)
{
	bird.SetCreateTime(time(NULL));
	special_birds_2.push_back(bird);
}

bool Bird_Factory::get_special_bird_2(Bird * bird)
{
	if (special_birds_2.empty())
	{
		return false;
	}

	*bird = special_birds_2.front();
	special_birds_2.pop_front();
	return true;
}

void Bird_Factory::add_special_birds_3(Bird &bird)
{
	bird.SetCreateTime(time(NULL));
	special_birds_3.push_back(bird);
}

bool Bird_Factory::get_special_bird_3(Bird * bird)
{
	if (special_birds_3.empty())
	{
		return false;
	}

	*bird = special_birds_3.front();
	special_birds_3.pop_front();
	return true;
}

void Bird_Factory::add_special_birds_4(Bird &bird)
{
	bird.SetCreateTime(time(NULL));
	special_birds_4.push_back(bird);
}

bool Bird_Factory::get_special_bird_4(Bird * bird)
{
	if (special_birds_4.empty())
	{
		return false;
	}

	*bird = special_birds_4.front();
	special_birds_4.pop_front();
	return true;
}

void Bird_Factory::add_special_birds_5(Bird &bird)
{
	bird.SetCreateTime(time(NULL));
	special_birds_5.push_back(bird);
}

bool Bird_Factory::get_special_bird_5(Bird * bird)
{
	if (special_birds_5.empty())
	{
		return false;
	}

	*bird = special_birds_5.front();
	special_birds_5.pop_front();
	return true;
}

void Bird_Factory::add_special_birds_6(Bird &bird)
{
	bird.SetCreateTime(time(NULL));
	special_birds_6.push_back(bird);
}

bool Bird_Factory::get_special_bird_6(Bird * bird)
{
	if (special_birds_6.empty())
	{
		return false;
	}

	*bird = special_birds_6.front();
	special_birds_6.pop_front();
	return true;
}

void Bird_Factory::add_special_birds_boss(Bird &bird)
{
	bird.SetCreateTime(time(NULL));
    special_birds_boss.push_back(bird);
}

bool Bird_Factory::get_special_birds_boss(Bird *bird)
{
    if (special_birds_boss.empty())
    {
        return false;
    }

    *bird = special_birds_boss.front();
    special_birds_boss.pop_front();
    return true;
}

void Bird_Factory::rand_numer(int paths[], int count)
{
    for(int i=0; i<count; ++i) paths[i]=i;
    for(int i=count-1; i>=1; --i) std::swap(paths[i], paths[rand()%i]);
}


///////////////////////////////////////////////////////////////////////////
